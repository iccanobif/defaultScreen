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

void initDevMode(DEVMODE * DevMode)
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

int changePrimaryScreen(const QString &deviceName)
{
    DEVMODE DevMode;
    WCHAR arrDeviceName[deviceName.length()+1];

    //Get the offset

    deviceName.toWCharArray(arrDeviceName);
    arrDeviceName[deviceName.length()] = '\0';

    initDevMode(&DevMode);
    if ( !EnumDisplaySettings(arrDeviceName, ENUM_REGISTRY_SETTINGS, &DevMode) )
    {
        QMessageBox::critical(NULL, "Error", "Cannot find settings for display " + deviceName);
        return 0;
    }
    int offsetx = DevMode.dmPosition.x;
    int offsety = DevMode.dmPosition.y;
}

void MainWindow::loadScreenInfos()
{
    DISPLAY_DEVICE DisplayDevice;
    int i = 0;

    initDisplayDevice(&DisplayDevice);
    while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 1))
    {
        if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) && !(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
        {
            QListWidgetItem *item = new QListWidgetItem(QString::fromWCharArray(DisplayDevice.DeviceString));
            item->setData(Qt::UserRole, QString::fromWCharArray(DisplayDevice.DeviceName));
            ui->lstDevices->addItem(item);
        }
        initDisplayDevice(&DisplayDevice);
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadScreenInfos();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lstDevices_itemDoubleClicked(QListWidgetItem *item)
{
    changePrimaryScreen(item->data(Qt::UserRole).toString());
}

void stampaRisultato(int result)
{
    switch (result)
    {
     case DISP_CHANGE_SUCCESSFUL:
        qDebug() << "DISP_CHANGE_SUCCESSFUL";
        break;
     case DISP_CHANGE_BADDUALVIEW :
        qDebug() << "DISP_CHANGE_BADDUALVIEW ";
        break;
     case DISP_CHANGE_BADFLAGS :
        qDebug() << "DISP_CHANGE_BADFLAGS ";
        break;
     case DISP_CHANGE_BADMODE :
        qDebug() << "DISP_CHANGE_BADMODE ";
        break;
     case DISP_CHANGE_BADPARAM :
        qDebug() << "DISP_CHANGE_BADPARAM ";
        break;
     case DISP_CHANGE_FAILED :
        qDebug() << "DISP_CHANGE_FAILED ";
        break;
     case DISP_CHANGE_NOTUPDATED :
        qDebug() << "DISP_CHANGE_NOTUPDATED ";
        break;
     case DISP_CHANGE_RESTART :
        qDebug() << "DISP_CHANGE_RESTART ";
        break;
    }
}

void cambiaSchermoVecchioStile()
{
    DISPLAY_DEVICE DisplayDevice;
    DEVMODE    DevMode;
    std::wstring nomeDevice;
    int i = 0, j = 0;

    ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
    DisplayDevice.cb = sizeof(DisplayDevice);
    while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 1 ))
        {
        j++;

        if (j == 2)
            nomeDevice = DisplayDevice.DeviceName;

        if (DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        {
            qDebug() << "Il monitor principale attualmente e' " << QString::fromWCharArray(DisplayDevice.DeviceName);
        }

        ZeroMemory(&DevMode, sizeof(DevMode));
        DevMode.dmSize = sizeof(DevMode);
        if ( !EnumDisplaySettings(DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &DevMode) )
              qDebug() << "Store default failed";

        qDebug() << QString::fromWCharArray(DisplayDevice.DeviceName) << DevMode.dmPelsWidth << "x" << DevMode.dmPelsHeight << "x:" << DevMode.dmPosition.x << " y:" << DevMode.dmPosition.y;

        ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
        DisplayDevice.cb = sizeof(DisplayDevice);
    }
    qDebug() << "sto per cosare il device " << QString::fromStdWString(nomeDevice);
    initDevMode(&DevMode);
    if ( !EnumDisplaySettings(L"\\\\.\\DISPLAY1", ENUM_REGISTRY_SETTINGS, &DevMode) )
        qDebug() << "store default failed";
    DevMode.dmPosition.x = 1024;
    DevMode.dmPosition.y = 0;

    stampaRisultato(ChangeDisplaySettingsEx(L"\\\\.\\DISPLAY1",
                                    &DevMode,
                                    NULL,
                                    CDS_UPDATEREGISTRY,
                                    NULL));

    initDevMode(&DevMode);
    if ( !EnumDisplaySettings(L"\\\\.\\DISPLAY2", ENUM_REGISTRY_SETTINGS, &DevMode) )
        qDebug() << "store default failed";
    DevMode.dmPosition.x = 1024 + 1920;
    DevMode.dmPosition.y = 0;
    stampaRisultato(ChangeDisplaySettingsEx(L"\\\\.\\DISPLAY2",
                                    &DevMode,
                                    NULL,
                                    CDS_UPDATEREGISTRY,
                                    NULL));

    initDevMode(&DevMode);
    if ( !EnumDisplaySettings(L"\\\\.\\DISPLAY3", ENUM_REGISTRY_SETTINGS, &DevMode) )
        qDebug() << "store default failed";
    DevMode.dmPosition.x = 0;
    DevMode.dmPosition.y = 0;
    stampaRisultato(ChangeDisplaySettingsEx(L"\\\\.\\DISPLAY3",
                                    &DevMode,
                                    NULL,
                                    CDS_SET_PRIMARY | CDS_UPDATEREGISTRY,
                                    NULL));

    stampaRisultato(ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL));
}
