#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <windows.h>
#include <winuser.h>
#include <QMessageBox>

void initDisplayDevice(DISPLAY_DEVICE *DisplayDevice)
{
    ZeroMemory(DisplayDevice, sizeof(DISPLAY_DEVICE));
    DisplayDevice->cb = sizeof(DISPLAY_DEVICE);
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

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QMessageBox::information(NULL, "",QString::number(getNumberOfDevices()));

    DISPLAY_DEVICE DisplayDevice;
    int i = 0;

    initDisplayDevice(&DisplayDevice);
    while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 1))
    {
        if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) && !(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
            ui->lstDevices->addItem(QString::fromWCharArray(DisplayDevice.DeviceString));
        initDisplayDevice(&DisplayDevice);
    }

}


MainWindow::~MainWindow()
{
    delete ui;
}
