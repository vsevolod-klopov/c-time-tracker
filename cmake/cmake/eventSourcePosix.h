#pragma once

#include "event.h"
#include "eventSource.h"

class PosixEventSource : public EventSource
{
public:
	Event getEvent() override;
};
