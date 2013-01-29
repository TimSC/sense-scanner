#include <QtGui/QApplication>
#include "mainwindow.h"
#include <iostream>
using namespace std;
#include <stdio.h>

#ifdef _MSC_VER
#include <Windows.h>

int WINAPI WinMain ( HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show )
{
    int argc = 0;
    char **argv = NULL;
#else
int main(int argc, char *argv[])
{
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
