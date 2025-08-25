#pragma once

#include <memory>
#include <vector>
#include <ostream>
#include "engine/events/Events.h"
#include "engine/views/Dashboard.h"

// MarketDataPublisher subscribes to EventBus and renders a terminal UI
class MarketDataPublisher
{
public:
    explicit MarketDataPublisher(std::unique_ptr<Dashboard> dashboard);
    // Called on each event via EventBus
    void on_event(const Event &e);

private:
    void refresh_terminal();

    std::unique_ptr<Dashboard> dashboard_;

    // For throttling refresh (optional)
    size_t event_counter_ = 0;
    size_t refresh_interval_ = 10; // refresh every N events
    bool first_time_ = true;
};
