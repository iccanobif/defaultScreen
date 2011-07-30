#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <windows.h>
#include <winuser.h>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>

void initDisplayDevice(DISPLAY_DEVICE *DisplayDevice)
{
    ZeroMemory(DisplayDevice, sizeof(DISPLAY_DEVICE));
    DisplayDevice->cb = sizeof(DISPLAY_DEVICE);
}

void initDevMode(DEVMODE *DevMode)
{
    ZeroMemory(DevMode, sizeof(DEVMODE));
    DevMode->dmSize = sizeof(DEVMODE);
}

void stampaRisultato(int result)
{
    switch (result)
    {
     case DISP_CHANGE_SUCCESSFUL:  qDebug() << "DISP_CHANGE_SUCCESSFUL";   break;
     case DISP_CHANGE_BADDUALVIEW: qDebug() << "DISP_CHANGE_BADDUALVIEW "; break;
     case DISP_CHANGE_BADFLAGS:    qDebug() << "DISP_CHANGE_BADFLAGS ";    break;
     case DISP_CHANGE_BADMODE:     qDebug() << "DISP_CHANGE_BADMODE ";     break;
     case DISP_CHANGE_BADPARAM:    qDebug() << "DISP_CHANGE_BADPARAM ";    break;
     case DISP_CHANGE_FAILED:      qDebug() << "DISP_CHANGE_FAILED ";      break;
     case DISP_CHANGE_NOTUPDATED:  qDebug() << "DISP_CHANGE_NOTUPDATED ";  break;
     case DISP_CHANGE_RESTART:     qDebug() << "DISP_CHANGE_RESTART ";     break;
    }
}

QRect getScreenRect(const QString &deviceName)
{
    DEVMODE DevMode;
    WCHAR arrDeviceName[deviceName.length()+1];

    deviceName.toWCharArray(arrDeviceName);
    arrDeviceName[deviceName.length()] = '\0';

    initDevMode(&DevMode);
    if ( !EnumDisplaySettings(arrDeviceName, ENUM_REGISTRY_SETTINGS, &DevMode) )
    {
        QMessageBox::critical(NULL, "Error", "Cannot find settings for display " + deviceName);
        return QRect();
    }

    return QRect(DevMode.dmPosition.x, DevMode.dmPosition.y, DevMode.dmPelsWidth, DevMode.dmPelsHeight);
}

int getScreenRefreshRate(const QString &deviceName)
{
    DEVMODE DevMode;
    WCHAR arrDeviceName[deviceName.length()+1];

    deviceName.toWCharArray(arrDeviceName);
    arrDeviceName[deviceName.length()] = '\0';

    initDevMode(&DevMode);
    if ( !EnumDisplaySettings(arrDeviceName, ENUM_REGISTRY_SETTINGS, &DevMode) )
    {
        QMessageBox::critical(NULL, "Error", "Cannot find settings for display " + deviceName);
        return -1;
    }

    return DevMode.dmDisplayFrequency;
}

int setScreenRect(const QString &deviceName, int x, int y, int width, int height, int refreshRate)
{
    qDebug() << deviceName << "refRate:" << refreshRate;
    DEVMODE DevMode;
    WCHAR arrDeviceName[deviceName.length()+1];

    deviceName.toWCharArray(arrDeviceName);
    arrDeviceName[deviceName.length()] = '\0';

    initDevMode(&DevMode);
    if ( !EnumDisplaySettings(arrDeviceName, ENUM_REGISTRY_SETTINGS, &DevMode) )
    {
        return -1;
    }

    DevMode.dmFields = DM_POSITION | DM_PELSHEIGHT | DM_PELSWIDTH;

//    qDebug() << deviceName;
//    qDebug() << "Going from" <<  DevMode.dmPosition.x << "x" << DevMode.dmPosition.y << "to" << x << "x" << y;

    DevMode.dmPosition.x = x;
    DevMode.dmPosition.y = y;
    DevMode.dmPelsHeight = height;
    DevMode.dmPelsWidth = width;
    DevMode.dmDisplayFrequency = refreshRate;

    if (x == 0 && y == 0)
        return ChangeDisplaySettingsEx(arrDeviceName,
                                       &DevMode,
                                       NULL,
                                       CDS_UPDATEREGISTRY | CDS_NORESET | CDS_SET_PRIMARY,
                                       NULL);
    else
        return ChangeDisplaySettingsEx(arrDeviceName,
                                       &DevMode,
                                       NULL,
                                       CDS_UPDATEREGISTRY | CDS_NORESET,
                                       NULL);

}

int detachScreen(const QString &deviceName)
{
    DEVMODE DevMode;
    WCHAR arrDeviceName[deviceName.length()+1];

    deviceName.toWCharArray(arrDeviceName);
    arrDeviceName[deviceName.length()] = '\0';

    initDevMode(&DevMode);
    if ( !EnumDisplaySettings(arrDeviceName, ENUM_REGISTRY_SETTINGS, &DevMode) )
        return -1;

    DevMode.dmFields = DM_POSITION;

    DevMode.dmPelsHeight = 0;
    DevMode.dmPelsWidth = 0;

    return ChangeDisplaySettingsEx(arrDeviceName,
                                    &DevMode,
                                    NULL,
                                    CDS_UPDATEREGISTRY | CDS_NORESET,
                                    NULL);

}

int changePrimaryScreen(const QString &deviceName)
{
    DISPLAY_DEVICE DisplayDevice;

    QRect primaryScreenOldRect = getScreenRect(deviceName);
    QMap<QString,QRect> oldRects;
    QMap<QString,int> oldRefreshRates;

    //get old position for every visible screen
    int i = 0;
    initDisplayDevice(&DisplayDevice);
    while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 1))
    {
        if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) && !(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
        {
            QString devName = QString::fromWCharArray(DisplayDevice.DeviceName);
            oldRects.insert(devName, getScreenRect(devName));
            oldRefreshRates.insert(devName, getScreenRefreshRate(devName));
            detachScreen(devName);
        }
    }

    QMapIterator<QString, QRect> it(oldRects);
    QMapIterator<QString, int> itRefRate(oldRefreshRates);

    while (it.hasNext())
    {
        it.next();
        itRefRate.next();
        setScreenRect(it.key(),
                      it.value().left() - primaryScreenOldRect.left(),
                      it.value().top() - primaryScreenOldRect.top(),
                      it.value().width(),
                      it.value().height(),
                      itRefRate.value());
    }

    ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);

    return 1;
}

void MainWindow::loadScreenInfos()
{
    ui->lstDevices->clear();
    trayMenu->clear();

    DISPLAY_DEVICE DisplayDevice;
    int i = 0;

    initDisplayDevice(&DisplayDevice);
    while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 1))
    {
        if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) && !(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
        {
            DEVMODE DevMode;
            initDevMode(&DevMode);
            EnumDisplaySettings(DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &DevMode);

            QString devDescription;
            devDescription = QString::fromWCharArray(DisplayDevice.DeviceString);
            devDescription += " - ";
            devDescription += QString::number(DevMode.dmPelsWidth) + "x";
            devDescription += QString::number(DevMode.dmPelsHeight);
            devDescription += "; " + QString::number(DevMode.dmPosition.x);

            QListWidgetItem *item = new QListWidgetItem(devDescription);
            item->setData(Qt::UserRole, QString::fromWCharArray(DisplayDevice.DeviceName));

            if (DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
            {
                QFont newFont = item->font();
                newFont.setBold(true);
                item->setFont(newFont);
            }

            ui->lstDevices->addItem(item);

            QAction* newAction = new QAction(devDescription, trayMenu);
            newAction->setData(QString::fromWCharArray(DisplayDevice.DeviceName));
            if (DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
            {
                QFont newFont = newAction->font();
                newFont.setBold(true);
                newAction->setFont(newFont);
            }
            trayMenu->addAction(newAction);
        }
        initDisplayDevice(&DisplayDevice);
    }
    trayMenu->addSeparator();
    trayMenu->addAction("Quit")->setData("_DEFAULT_SCREEN_QUIT_");
}

void MainWindow::handleTrayMenu(QAction* action) {
    if (action->data().toString() == "_DEFAULT_SCREEN_QUIT_")
    {
        QApplication::quit();
        return;
    }
    changePrimaryScreen(action->data().toString());
    loadScreenInfos();
}

void MainWindow::handleTrayMenuActivation(QSystemTrayIcon::ActivationReason reason) {
//    if (reason == QSystemTrayIcon::DoubleClick)
//    {
//        if (this->isHidden())
//            showNormal();
//        else
//            this->hide();
//    }
}

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    trayMenu = new QMenu();
    tray = new QSystemTrayIcon();
    tray->setIcon(QIcon(":/icons/icon.png"));
    tray->setContextMenu(trayMenu);
    tray->setVisible(true);
    connect(trayMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(handleTrayMenu(QAction*)));
    connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(handleTrayMenuActivation(QSystemTrayIcon::ActivationReason)));
    loadScreenInfos();
}

void MainWindow::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
        {
            QApplication::processEvents();
            QTimer::singleShot(250, this, SLOT(hide()));
            e->ignore();
        }
    }
    QWidget::changeEvent(e);
}

MainWindow::~MainWindow()
{
    delete ui;
    tray->hide();
}

void MainWindow::on_lstDevices_itemDoubleClicked(QListWidgetItem *item)
{
    changePrimaryScreen(item->data(Qt::UserRole).toString());
    loadScreenInfos();
}
