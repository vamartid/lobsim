// MarketSimulator.h
#pragma once

#include "utils/data_structures/ThreadSafeQueue.h"
#include "utils/OrderTracker.h"
#include "core/MarketFeeder.h"
#include "engine/OrderBookEngine.h"
#include "utils/random/IRNG.h"
#include "engine/listeners/OrderBookView.h"
#include "engine/listeners/StatsCollector.h"
#include "engine/listeners/MarketDataPublisher.h"

#include <thread>
#include <atomic>

class MarketSimulator
{
public:
    MarketSimulator();
    void start();
    void stop();

    void enable_live_view(bool enable);

    size_t add_listener(EventBus::Callback cb);

private:
    void engine_loop();

    ThreadSafeQueue<Order> order_queue_;
    std::atomic<bool> running_{false};

    EventBus bus_;           // central event dispatcher
    OrderBookEngine engine_; // engine now subscribes to EventBus

    // Event-driven live view components
    std::shared_ptr<OrderBookView> live_view_; // optional live L2 view
    std::shared_ptr<StatsCollector> stats_;
    std::shared_ptr<MarketDataPublisher> publisher_;
    std::vector<std::pair<std::shared_ptr<void>, size_t>> live_view_listeners_;

    std::thread engine_thread_;
    std::vector<std::unique_ptr<MarketFeeder>> feeders_; // multiple feeders default RNG

    template <typename ListenerType, typename... Args>
    std::shared_ptr<ListenerType> make_and_register_listener(
        std::vector<std::pair<std::shared_ptr<void>, size_t>> &container,
        Args &&...args)
    {
        auto listener = std::make_shared<ListenerType>(std::forward<Args>(args)...);
        size_t id = bus_.add_listener([listener](const Event &e)
                                      { listener->on_event(e); });
        container.emplace_back(listener, id);
        return listener;
    }
};
