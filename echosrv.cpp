
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
using namespace std;

int main(void) {
    char str[100];
    fd_set readset;
    struct timeval tv;
    struct termios ttystate, ttysave;

    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
    ttysave = ttystate;
    //turn off canonical mode and echo
    ttystate.c_lflag &= ~(ICANON | ECHO);
    //minimum of number input read.
    ttystate.c_cc[VMIN] = 1;

    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    while(1)
    {
		int clearingBuffer = 1;

		while(clearingBuffer)
		{
		    tv.tv_sec = 0;
		    tv.tv_usec = 1000;

		    FD_ZERO(&readset);
		    FD_SET(fileno(stdin), &readset);

		    select(fileno(stdin)+1, &readset, NULL, NULL, &tv);
		    if(FD_ISSET(fileno(stdin), &readset))
		    {
		        cout << (char)fgetc (stdin) << endl;
				cout.flush();
		    }
			else
				clearingBuffer = 0;
		}

    }

    ttystate.c_lflag |= ICANON | ECHO;
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    // report success
    return 0;
}

