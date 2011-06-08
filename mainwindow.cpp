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
   QMessageBox::information(NULL, "lol", item->data(Qt::UserRole).toString());
}
