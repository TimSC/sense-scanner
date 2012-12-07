#ifndef ALGORITHM_H
#define ALGORITHM_H


class AlgThread
{
public:
    AlgThread();
    virtual ~AlgThread();

    int IsRunning();
    void Start();
    void Stop();
    void WaitUntilStopped();

protected:


};

class Algorithm : public AlgThread
{
public:
    Algorithm();
};

#endif // ALGORITHM_H
