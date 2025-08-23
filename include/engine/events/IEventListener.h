#pragma once
#include "engine/events/Events.h"

struct IEventListener
{
    virtual ~IEventListener() = default;
    virtual void on_event(const Event &) = 0;
};
