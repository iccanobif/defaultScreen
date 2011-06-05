#include <windows.h>
#include <winuser.h>
#include <string.h>
#include <QDebug>

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

void initDevMode(DEVMODE * DevMode)
{
    ZeroMemory(DevMode, sizeof(DEVMODE));
    DevMode->dmSize = sizeof(DEVMODE);
}

int main() {
    DISPLAY_DEVICE DisplayDevice;
    DEVMODE    DevMode;
//    WCHAR nomeDevice[200];
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
              OutputDebugString(L"Store default failed\n");

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
    return 0;
}
