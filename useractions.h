#ifndef USERACTIONS_H
#define USERACTIONS_H

#include "eventloop.h"
#include <QtCore/QtCore>

class UserActions : public MessagableThread
{
public:
    UserActions();
    virtual ~UserActions();

    virtual void HandleEvent(std::tr1::shared_ptr<class Event> ev);
    virtual void Update();
    void SetEventLoop(class EventLoop *eventLoopIn);

    int SaveAs(QString fina);
    void Load(QString fina);
    void TrainModel(QList<QUuid> annotationUuids);

    void SetMediaInterface(QUuid mediaUuidIn);
protected:

    QUuid mediaUuid;
};

#endif // USERACTIONS_H
