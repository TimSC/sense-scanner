#ifndef _LOCAL_MUTEX_H_
#define _LOCAL_MUTEX_H_

/*!
* This file provides platform specific aliases for the Mutex type.
*/

#ifdef _MSC_VER

//#include <QtCore/QMutex>
//typedef QMutex Mutex;

#include <windows.h>

class MsVsMutex
{
public:
	MsVsMutex()
	{
		this->mutex = CreateMutex(NULL,FALSE,NULL);
	}

	~MsVsMutex()
	{
		CloseHandle(this->mutex);
	}

	void lock()
	{
		WaitForSingleObject(this->mutex, INFINITE);
	}

	void unlock()
	{
		ReleaseMutex(this->mutex);
	}

protected:
	HANDLE mutex;
};

typedef MsVsMutex Mutex;

#else

#include <mutex>
typedef std::mutex Mutex;

#endif

#endif //_LOCAL_MUTEX_H_
