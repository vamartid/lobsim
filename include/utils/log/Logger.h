#pragma once

#include "engine/events/IEventListener.h"
#include <iostream>

struct Logger : IEventListener
{
    void on_event(const Event &e) override;
};