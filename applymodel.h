#ifndef APPLYMODEL_H
#define APPLYMODEL_H

#include "eventloop.h"

class ApplyModel : public MessagableThread
{
public:
    ApplyModel();
    virtual ~ApplyModel();

    void Update();
    void HandleEvent(std::tr1::shared_ptr<class Event> ev);
};

#endif // APPLYMODEL_H
