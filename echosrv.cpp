//g++ -std=c++0x -pthread echosrv.cpp -o echosrv
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <mutex>
#include <unistd.h>
using namespace std;

int gRunning = 1;
int gPaused = 0;
std::mutex gRunningMutex;

class Worker
{
public:
    void operator()() const
    {
		gRunningMutex.lock();
		int running = gRunning;
		int paused = gPaused;
		gRunningMutex.unlock();
		while(running)
		{
			if(!paused)
			{
	        	cout << "PROGRESS=0.5" << endl;
				usleep(1000000);
			}
			else
			{
				usleep(1000);
			}

			gRunningMutex.lock();
			running = gRunning;
			paused = gPaused;
			gRunningMutex.unlock();
		}
		
    }
};

int main(int argc, char *argv[])
{
	std::thread thr((Worker()));
	gRunningMutex.lock();
	int running = gRunning;
	gRunningMutex.unlock();

	while(running)
	{
		//Read standard input
		std::string mystring;
		cin >> mystring;

		//Process commands
		if(mystring == "QUIT")
		{
			gRunningMutex.lock();
			gRunning = 0;
			gRunningMutex.unlock();
		}

		if(mystring == "PAUSE")
		{
			gRunningMutex.lock();
			gPaused = 1;
			gRunningMutex.unlock();
		}

		if(mystring == "RUN")
		{
			gRunningMutex.lock();
			gPaused = 0;
			gRunningMutex.unlock();
		}
		//if(mystring == "GET_PROGRESS")
		//	cout << "PROGRESS=0.5" << endl;

		gRunningMutex.lock();
		running = gRunning;
		gRunningMutex.unlock();
	}
	
	thr.join(); //This waits until worker thread finishes
	cout << "FINISHED" << endl;
}

