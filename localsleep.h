
#ifndef _LOCAL_SLEEP_H_
#define _LOCAL_SLEEP_H_

#include <QtCore/QThread>

class LocalSleep : public QThread
{
public:
	static void sleep(unsigned long secs) {
		QThread::sleep(secs);
	}
	static void msleep(unsigned long msecs) {
		QThread::msleep(msecs);
	}
	static void usleep(unsigned long usecs) {
		QThread::usleep(usecs);
	}
};

#endif //_LOCAL_SLEEP_H_
