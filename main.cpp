#include <QtGui/QApplication>
#include "mainwindow.h"
#include <iostream>
using namespace std;
#include <stdio.h>

#ifdef _MSC_VER
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <Windows.h>

#ifndef _DEBUG
int WINAPI WinMain ( HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show )
{
    int argc = 0;
    char **argv = NULL;
#else
int main(int argc, char *argv[])
{
#endif
#endif

#ifndef _MSC_VER
int main(int argc, char *argv[])
{
#endif

#ifdef _MSC_VER
	//http://stackoverflow.com/questions/9554252/c-c-is-it-possible-to-pass-binary-data-through-the-console
	// Set "stdin" to have binary mode:
	int result = _setmode( _fileno( stdin ), _O_BINARY );
	if( result == -1 )
		cout <<  "Cannot console set mode" << endl;
	else
		cout << "'stdin' successfully changed to binary mode"<< endl;
	result = _setmode( _fileno( stdout ), _O_BINARY );
	if( result == -1 )
		cout <<  "Cannot console set mode" << endl;
	else
		cout << "'stdout' successfully changed to binary mode"<< endl;

#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}

