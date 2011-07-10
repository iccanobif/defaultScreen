#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <windows.h>
#include <winuser.h>
#include <QMessageBox>
#include <QDebug>

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

int getNumberOfDevices()
{
    int i = 0;
    int out = 0;
    DISPLAY_DEVICE dd;
    initDisplayDevice(&dd);
    while (EnumDisplayDevices(NULL, i++, &dd, 1))
    {
        if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) && !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
            out++;
        initDisplayDevice(&dd);
    }
    return out;
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

int setScreenRect(const QString &deviceName, int x, int y, int width, int height)
{
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
    {
        return -1;
    }

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

    //get old position for every visible screen
    int i = 0;
    initDisplayDevice(&DisplayDevice);
    while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 1))
    {
        if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) && !(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
        {
            QString devName = QString::fromWCharArray(DisplayDevice.DeviceName);
            oldRects.insert(devName, getScreenRect(devName));
            detachScreen(devName);
        }
    }

    QMapIterator<QString, QRect> it(oldRects);

    while (it.hasNext())
    {
        it.next();
        setScreenRect(it.key(),
                      it.value().left() - primaryScreenOldRect.left(),
                      it.value().top() - primaryScreenOldRect.top(),
                      it.value().width(),
                      it.value().height());
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
}

void MainWindow::handleTrayMenu(QAction* action) {
    changePrimaryScreen(action->data().toString());
    loadScreenInfos();
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
    loadScreenInfos();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lstDevices_itemDoubleClicked(QListWidgetItem *item)
{
    changePrimaryScreen(item->data(Qt::UserRole).toString());
    loadScreenInfos();
}
