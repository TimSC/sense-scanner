#include <QtGui/QApplication>
#include "mainwindow.h"
#include <Windows.h>

int WINAPI WinMain ( HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show )
{
	int argc = 0;
	char **argv = NULL;
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
