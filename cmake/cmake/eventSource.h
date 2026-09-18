//#ifndef EVENT_SOURCE_H
//#define EVENT_SOURCE_H
#pragma once
#include "event.h"


class EventSource
{
public:
    virtual Event getEvent() = 0;
    virtual ~EventSource() = default;
};

//#endif // EVENT_SOURCE_H