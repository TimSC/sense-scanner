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
std::mutex gRunningMutex;

class Worker
{
public:
    void operator()() const
    {
		gRunningMutex.lock();
		int running = gRunning;
		gRunningMutex.unlock();
		while(running)
		{
        	cout << "PROGRESS=0.5" << endl;
			usleep(1000000);

			gRunningMutex.lock();
			running = gRunning;
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
			gRunning = 0;
		//if(mystring == "GET_PROGRESS")
		//	cout << "PROGRESS=0.5" << endl;

		gRunningMutex.lock();
		running = gRunning;
		gRunningMutex.unlock();
	}
	
	thr.join();
	cout << "FINISHED" << endl;
}

