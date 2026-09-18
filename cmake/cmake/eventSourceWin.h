#pragma once

#include "event.h"
#include "eventSource.h"

class WinEventSource : public EventSource
{
public:
    Event getEvent() override;
};
