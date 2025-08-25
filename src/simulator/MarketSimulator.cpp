#include "simulator/MarketSimulator.h"
#include "utils/random/RealRNG.h"
#include "engine/views/Dashboard.h"
#include "engine/views/OrderBookViewRenderer.h"
#include "engine/views/StatsViewRenderer.h"
#include "engine/views/TradesViewRenderer.h"

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

// MarketSimulator
// ├─ vector<listeners> live_view_listeners_
// │    ├─ shared_ptr<OrderBookView>            // owns core orderbook data
// │    ├─ shared_ptr<StatsCollector>           // owns core stats + trade buffer
// │    └─ shared_ptr<MarketDataPublisher>      // owns unique_ptr<Dashboard>
// │         └─ unique_ptr<Dashboard>           // owns the views
// │              ├─ unique_ptr<OrderBookViewRenderer>
// │              ├─ unique_ptr<StatsViewRenderer>
// │              └─ unique_ptr<TradesViewRenderer>
// └─ No separate publisher_ member needed      // ownership is in live_view_listeners_
// └─ No separate dashboard_ member needed      // owned by MarketDataPublisher
// └─ trade_buffer is shared_ptr because it is used by both StatsCollector and TradesViewRenderer
void MarketSimulator::enable_live_view(bool enable)
{
    if (enable && live_view_listeners_.empty())
    {
        // Core listeners (data, not UI)
        auto trade_buffer = std::make_shared<TradeBuffer>(1024);
        auto live_orderbook = make_and_add_listener_to_bus<OrderBookView>(live_view_listeners_);
        auto live_stats = make_and_add_listener_to_bus<StatsCollector>(live_view_listeners_, trade_buffer);

        // Dashboard + views (dashboard owns these views)
        auto dashboard = std::make_unique<Dashboard>();
        dashboard->add_view(std::make_unique<OrderBookViewRenderer>(live_orderbook));
        dashboard->add_view(std::make_unique<StatsViewRenderer>(live_stats));
        dashboard->add_view(std::make_unique<TradesViewRenderer>(trade_buffer, engine_.tick_wall_times()));

        // Publisher (wraps dashboard, connects to bus, owns dashboard)
        make_and_add_listener_to_bus<MarketDataPublisher>(live_view_listeners_, std::move(dashboard));
    }
    else if (!enable && !live_view_listeners_.empty())
    {
        // Remove all listeners from EventBus
        for (auto &[listener, id] : live_view_listeners_)
            bus_.remove_listener(id);
        live_view_listeners_.clear(); // destroys all shared_ptrs, freeing MarketDataPublisher -> Dashboard -> Views
    }
}

size_t MarketSimulator::add_listener(EventBus::Callback cb)
{
    // Returns listener ID so caller can remove later if needed
    return bus_.add_listener(std::move(cb));
}
