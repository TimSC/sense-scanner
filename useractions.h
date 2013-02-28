#ifndef USERACTIONS_H
#define USERACTIONS_H

#include "eventloop.h"

class UserActions : public MessagableThread
{
public:
    UserActions();
    virtual ~UserActions();

    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);
    virtual void Update();

    void SetMediaInterface(class AvBinMedia* mediaInterfaceIn);
    int SaveAs(QString fina);
    void Load(QString fina);

protected:
    class AvBinMedia* mediaInterface;

};

#endif // USERACTIONS_H
