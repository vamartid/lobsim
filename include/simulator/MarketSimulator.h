// MarketSimulator.h
#pragma once

#include "utils/data_structures/ThreadSafeQueue.h"
#include "utils/OrderTracker.h"
#include "core/MarketFeeder.h"
#include "engine/OrderBookEngine.h"
#include "utils/random/IRNG.h"

#include <thread>
#include <atomic>

class MarketSimulator
{
public:
    MarketSimulator();
    void start();
    void stop();

    void enable_live_view(bool enable);
    void start_live_view_thread();
    void stop_live_view_thread();

private:
    void engine_loop();

    ThreadSafeQueue<Order> order_queue_;
    std::shared_ptr<IRNG> rng_;
    // MarketFeeder feeder_; // Use default RNG template param
    std::vector<std::unique_ptr<MarketFeeder>> feeders_; // multiple feeders
    OrderBookEngine engine_;

    std::atomic<bool> live_view_enabled_{false}; // atomic for thread safety
    std::shared_ptr<OrderTracker> order_tracker_;
    std::thread live_view_thread_;

    std::atomic<bool> running_;
    std::thread engine_thread_;
};
