#include "core/MarketFeeder.h"
#include "utils/GeneralUtils.h"

#include <random>
#include <thread>
#include <chrono>

// Macro for order ID assignment: use feeder_id if available, else simple increment
#ifdef FEEDER_ID_DEFINED
#define ASSIGN_ORDER_ID(order, feeder_id, order_id) \
    (order.id = utils::general::encode_order_id(feeder_id, order_id++))
#else
#define ASSIGN_ORDER_ID(order, feeder_id, order_id) \
    (order.id = order_id++)
#endif

// std::random_device Realistic randomness Default for simulations
// Fixed seed Reproducible tests or benchmarks
MarketFeeder::MarketFeeder(ThreadSafeQueue<Order> &queue, std::shared_ptr<IRNG> rng, uint16_t feeder_id, std::shared_ptr<OrderTracker> order_tracker)
    : queue_(queue), running_(false), order_id_(0), rng_(std::move(rng)), feeder_id_(feeder_id), order_tracker_(std::move(order_tracker))
{
}

void MarketFeeder::start()
{
    running_ = true;
    worker_ = std::thread(&MarketFeeder::run, this);
}

void MarketFeeder::stop()
{
    running_ = false;
    if (worker_.joinable())
        worker_.join();
}

void MarketFeeder::run()
{
    auto now = std::chrono::steady_clock::now();
    auto seed = static_cast<unsigned int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    std::mt19937 sleep_rng(seed);
    std::uniform_int_distribution<int> sleep_dist(45, 70);

    while (running_)
    {
        Order order = generate_order();
        queue_.push(std::move(order));
        int time_to_sleep = sleep_dist(sleep_rng);
        std::this_thread::sleep_for(std::chrono::microseconds(time_to_sleep)); // Simulate market frequency
    }
}

Order MarketFeeder::generate_order()
{
    Order order;
    ASSIGN_ORDER_ID(order, feeder_id_, order_id_);
    order.timestamp_ns = std::chrono::steady_clock::now().time_since_epoch().count();
    order.price = rng_->uniform_real(PRICE_MIN, PRICE_MAX);
    order.quantity = static_cast<uint32_t>(rng_->uniform_int(QTY_MIN, QTY_MAX));
    order.side = static_cast<Side>(rng_->uniform_int(SIDE_MIN, SIDE_MAX));
    if (order_tracker_ && order_tracker_->is_enabled())
    {
        order_tracker_->add_order(order, feeder_id_);
    }
    return order;
}
