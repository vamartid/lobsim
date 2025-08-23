#include "simulator/MarketSimulator.h"
#include "utils/random/RealRNG.h"
#include <iostream>

MarketSimulator::MarketSimulator()
    : engine_(bus_)
{
    unsigned int num_cores = std::thread::hardware_concurrency();
    unsigned int num_feeders = (num_cores > 1) ? (num_cores - 1) : 1;
    feeders_.reserve(num_feeders);

    for (unsigned int i = 0; i < num_feeders; ++i)
    {
        auto rng = std::make_shared<RealRNG>();
        feeders_.emplace_back(std::make_unique<MarketFeeder>(
            order_queue_, rng, i + 1, num_feeders * 100));
    }
}

void MarketSimulator::start()
{
    running_ = true;

    // Start feeders
    for (auto &feeder : feeders_)
        feeder->start();

    // Start engine thread
    engine_thread_ = std::thread(&MarketSimulator::engine_loop, this);
}

void MarketSimulator::stop()
{
    running_ = false;

    for (auto &feeder : feeders_)
        feeder->stop();

    if (engine_thread_.joinable())
        engine_thread_.join();
}

void MarketSimulator::engine_loop()
{
    while (running_)
    {
        // Pop next order from the queue
        Order order = order_queue_.wait_and_pop();

        // Add order to engine (triggers matching, events published)
        engine_.add_order(order);
    }
}

void MarketSimulator::enable_live_view(bool enable)
{
    if (enable && live_view_listeners_.empty())
    {
        // Create live view, stats, and publisher if not already created
        if (!live_view_)
            live_view_ = make_and_register_listener<OrderBookView>(live_view_listeners_);
        if (!stats_)
            stats_ = make_and_register_listener<StatsCollector>(live_view_listeners_);
        if (!publisher_)
            publisher_ = make_and_register_listener<MarketDataPublisher>(live_view_listeners_, live_view_, stats_);
    }
    else if (!enable && !live_view_listeners_.empty())
    {
        // Remove registered listeners from EventBus
        for (auto &[listener, id] : live_view_listeners_)
            bus_.remove_listener(id);

        live_view_listeners_.clear();

        // Optional: reset pointers if you no longer need them
        live_view_.reset();
        stats_.reset();
        publisher_.reset();
    }
}

size_t MarketSimulator::add_listener(EventBus::Callback cb)
{
    // Returns listener ID so caller can remove later if needed
    return bus_.add_listener(std::move(cb));
}
