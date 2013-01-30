#include <QtGui/QApplication>
#include "mainwindow.h"
#include <iostream>
using namespace std;
#include <stdio.h>

#ifdef _MSC_VER
#include <Windows.h>
#ifndef _DEBUG
int WINAPI WinMain ( HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show )
{
    int argc = 0;
    char **argv = NULL;
#else
int main(int argc, char *argv[])
{

//http://stackoverflow.com/questions/9554252/c-c-is-it-possible-to-pass-binary-data-through-the-console
#endif
#endif

#ifndef _MSC_VER
int main(int argc, char *argv[])
{
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
