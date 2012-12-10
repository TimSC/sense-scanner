
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
using namespace std;

int main(int argc, char *argv[])
{
	int running = 1;
	while(running)
	{
		std::string mystring;
		cin >> mystring;
		//cout << mystring << mystring.length() << endl;
		if(mystring == "QUIT")
			running = 0;
		if(mystring == "GET_PROGRESS")
			cout << "PROGRESS=0.5" << endl;
	}
	cout << "FINISHED" << endl;
}

