#include "simulator/MarketSimulator.h"
#include "utils/random/RealRNG.h" // Your real RNG implementation

MarketSimulator::MarketSimulator()
    : engine_(),
      running_(false),
      order_tracker_(std::make_shared<OrderTracker>()) // create shared tracker
{
    unsigned int num_cores = std::thread::hardware_concurrency();
    unsigned int num_feeders = (num_cores > 1) ? (num_cores - 1) : 1;

    feeders_.reserve(num_feeders);

    for (unsigned int i = 0; i < num_feeders; ++i)
    {
        auto rng = std::make_shared<RealRNG>();

        // Pass order_tracker_ to feeders (you'll need to update MarketFeeder constructor)
        feeders_.push_back(std::make_unique<MarketFeeder>(order_queue_, rng, i + 1, order_tracker_));
    }

    // Also inject order_tracker_ into engine or matching strategy if needed
    // order_tracker_->enable(true);
    engine_.set_order_tracker(order_tracker_);
}

void MarketSimulator::start()
{
    running_ = true;
    for (auto &feeder : feeders_)
        feeder->start();
    engine_thread_ = std::thread(&MarketSimulator::engine_loop, this);
}

void MarketSimulator::stop()
{
    for (auto &feeder : feeders_)
        feeder->stop();
    running_ = false;
    if (engine_thread_.joinable())
        engine_thread_.join();
}

void MarketSimulator::engine_loop()
{
    while (running_)
    {
        Order order = order_queue_.wait_and_pop();
        engine_.match_order(order); // <-- triggers matching immediately
    }
}

void MarketSimulator::enable_live_view(bool enable)
{
    if (enable && !live_view_enabled_)
    {
        live_view_enabled_ = true;
        order_tracker_->enable(true); // Enable tracking here
        start_live_view_thread();
    }
    else if (!enable && live_view_enabled_)
    {
        live_view_enabled_ = false;
        stop_live_view_thread();
        order_tracker_->enable(false); // Disable tracking here
    }
}

void MarketSimulator::start_live_view_thread()
{
    live_view_thread_ = std::thread([this]()
                                    {
        while (live_view_enabled_) {
            order_tracker_->render_live_view();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } });
}

void MarketSimulator::stop_live_view_thread()
{
    if (live_view_thread_.joinable())
    {
        live_view_thread_.join();
    }
}
