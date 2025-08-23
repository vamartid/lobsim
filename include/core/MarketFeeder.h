#pragma once

#include "core/Order.h"
#include "utils/data_structures/ThreadSafeQueue.h"
#include "utils/random/IRNG.h"
#include "utils/OrderTracker.h"

#include <thread>
#include <atomic>
#include <random>
#include <chrono>

class MarketFeeder
{
public:
    MarketFeeder(ThreadSafeQueue<Order> &queue, std::shared_ptr<IRNG> rng, uint16_t feeder_id = 0, uint32_t delay = 0);
    void start();
    void stop();

private:
    static constexpr int DELAY_MIN = 45;
    static constexpr int DELAY_MAX = 70;
    static constexpr int DELAY_JITTER = 5;

    static constexpr double PRICE_MIN = 100.0;
    static constexpr double PRICE_MAX = 105.0;

    static constexpr int QTY_MIN = 1;
    static constexpr int QTY_MAX = 100;

    static constexpr int SIDE_MIN = 0;
    static constexpr int SIDE_MAX = 1;

    void run();
    Order generate_order();

    std::atomic<bool> running_;
    std::thread worker_;
    ThreadSafeQueue<Order> &queue_; // no moves just reference binding
    uint32_t delay_;                // Shift of the delay initial (DELAY_MIN-DELAY_MAX)
    uint16_t feeder_id_;
    uint64_t order_id_;

    // Random generators
    std::shared_ptr<IRNG> rng_; // store RNG interface
    std::uniform_real_distribution<double> price_dist_;
    std::uniform_int_distribution<int> qty_dist_;
    std::uniform_int_distribution<int> side_dist_;
};
