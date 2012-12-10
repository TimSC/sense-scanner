#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
	cout << "Hello world" << endl;
	char buffer[1000];

	while(1)
	{
		usleep(1000000);

		long flags = fcntl(0, F_GETFL); /* get current file status flags */
		flags |= O_NONBLOCK;		/* turn off blocking flag */
		fcntl(0, F_SETFL, flags);		/* set up non-blocking read */

		char* ret = fgets(buffer, 1000, stdin);

		cout << "." << (ret == NULL) << endl;
	}

	return 0;
}

