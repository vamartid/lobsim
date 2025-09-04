#pragma once

#include "engine/events/Events.h"
#include "engine/views/Dashboard.h"

#include <memory>
#include <vector>
#include <ostream>
#include <atomic>
#include <thread>

// MarketDataPublisher subscribes to EventBus and renders a terminal UI
class MarketDataPublisher
{
public:
    explicit MarketDataPublisher(std::unique_ptr<Dashboard> dashboard);
    ~MarketDataPublisher();
    // Called on each event via EventBus
    void on_event(const Event &e);
    void handle_key(char key);

private:
    void refresh_terminal();
    void enable_ansi_escape_codes();
    std::unique_ptr<Dashboard> dashboard_;
    std::atomic<bool> running_;
    std::thread input_thread_;
    void input_loop();

    // For throttling refresh (optional)
    size_t event_counter_ = 0;
    size_t refresh_interval_ = 10; // refresh every N events
    bool first_time_ = true;
};
