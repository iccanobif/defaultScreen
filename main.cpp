#include <windows.h>
#include <winuser.h>
#include <string.h>
#include <QtGui>
#include <QDebug>
#include "mainwindow.hpp"

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    MainWindow w;
    w.show();
    return a.exec();
}

