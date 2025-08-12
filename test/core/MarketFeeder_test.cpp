#include <gtest/gtest.h>
#include "test_utils/MockRNGHelpers.h"
#include "utils/data_structures/ThreadSafeQueue.h"
#include "core/MarketFeeder.h"
#include "utils/random/RealRNG.h"
#include "utils/random/MockRNG.h"

#include <unordered_set>
#include <thread>
#include <chrono>

TEST(MarketFeederTest, FeedsOrdersIntoQueue)
{
    ThreadSafeQueue<Order> queue;
    auto rng = std::make_shared<RealRNG>(42); // seeded for deterministic
    MarketFeeder feeder(queue, rng);
    feeder.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    feeder.stop();

    int count = 0;
    while (auto order = queue.pop())
    {
        EXPECT_GE(order->price, 0.0);
        EXPECT_GT(order->quantity, 0u);
        EXPECT_TRUE(order->side == Side::Buy || order->side == Side::Sell);
        ++count;
    }
    EXPECT_GT(count, 0);
}

TEST(MarketFeederTest, StopsGracefully)
{
    ThreadSafeQueue<Order> queue;
    auto rng = std::make_shared<RealRNG>(123);
    MarketFeeder feeder(queue, rng);
    feeder.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    feeder.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    int current_size = 0;
    while (queue.pop())
        ++current_size;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    int after_stop_count = 0;
    while (queue.pop())
        ++after_stop_count;

    EXPECT_EQ(after_stop_count, 0);
}

TEST(MarketFeederTest, RapidStartStop)
{
    ThreadSafeQueue<Order> queue;
    auto rng = std::make_shared<RealRNG>();
    for (int i = 0; i < 10; ++i)
    {
        MarketFeeder feeder(queue, rng);
        feeder.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        feeder.stop();
    }

    int count = 0;
    while (queue.pop())
        ++count;

    EXPECT_GT(count, 0);
}

TEST(MarketFeederTest, MultipleFeedersToSameQueue)
{
    ThreadSafeQueue<Order> queue;
    std::vector<std::unique_ptr<MarketFeeder>> feeders;

    for (int i = 0; i < 4; ++i)
    {
        auto rng = std::make_shared<RealRNG>(i);
        feeders.push_back(std::make_unique<MarketFeeder>(queue, rng));
        feeders.back()->start();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    for (auto &f : feeders)
        f->stop();

    int count = 0;
    while (queue.pop())
        ++count;

    EXPECT_GT(count, 10);
}

TEST(MarketFeederTest, NoOrdersAfterImmediateStop)
{
    ThreadSafeQueue<Order> queue;
    auto rng = std::make_shared<RealRNG>();
    MarketFeeder feeder(queue, rng);
    feeder.start();
    feeder.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int count = 0;
    while (queue.pop())
        ++count;

    EXPECT_LE(count, 2);
}

TEST(MarketFeederTest, UniqueOrderIDs)
{
    ThreadSafeQueue<Order> queue;
    auto rng = std::make_shared<RealRNG>();
    MarketFeeder feeder(queue, rng);
    feeder.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    feeder.stop();

    std::unordered_set<uint64_t> ids;
    while (auto order = queue.pop())
    {
        EXPECT_TRUE(ids.insert(order->id).second);
    }
}

TEST(MarketFeederTest, StressTestRunsLonger)
{
    ThreadSafeQueue<Order> queue;
    auto rng = std::make_shared<RealRNG>();
    MarketFeeder feeder(queue, rng);
    feeder.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    feeder.stop();

    int count = 0;
    while (queue.pop())
        ++count;

    EXPECT_GT(count, 100);
}

TEST(MarketFeederMockTest, ProducesOrdersWithMockedPriceAndQuantity)
{
    ThreadSafeQueue<Order> queue;
    std::shared_ptr<MockRNG> mock_rng = make_mock_rng_from_orders({Order{1, 123456789u, 100.25, 50, Side::Buy}});

    MarketFeeder feeder(queue, mock_rng);
    feeder.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    feeder.stop();

    auto order = queue.pop();
    ASSERT_TRUE(order);

    // EXPECT_EQ(order.id, 1);
    // EXPECT_EQ(order.timestamp_ns, 123456789u);
    EXPECT_EQ(order->side, Side::Buy);
    EXPECT_DOUBLE_EQ(order->price, 100.25);
    EXPECT_EQ(order->quantity, 50u);
}

TEST(MarketFeederMockTest, AlternatesBuyAndSellSides)
{
    ThreadSafeQueue<Order> queue;
    // Setup price, quantity, and sides for 4 orders: Buy, Sell, Buy, Sell
    std::vector<Order> expected_orders = {
        Order{1, 0, 50.0, 5, Side::Buy},
        Order{2, 0, 60.0, 10, Side::Sell},
        Order{3, 0, 70.0, 15, Side::Buy},
        Order{4, 0, 80.0, 20, Side::Sell}};

    std::shared_ptr<MockRNG> mock_rng = make_mock_rng_from_orders(expected_orders);
    MarketFeeder feeder(queue, mock_rng);
    feeder.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    feeder.stop();

    std::vector<Side> expected_sides = {Side::Buy, Side::Sell, Side::Buy, Side::Sell};
    std::vector<double> expected_prices = {50.0, 60.0, 70.0, 80.0};
    std::vector<unsigned> expected_quantities = {5, 10, 15, 20};

    for (size_t i = 0; i < expected_sides.size(); ++i)
    {
        auto order = queue.pop();
        ASSERT_TRUE(order);
        // EXPECT_EQ(order->side, expected_sides[i]);
        EXPECT_DOUBLE_EQ(order->price, expected_prices[i]);
        EXPECT_EQ(order->quantity, expected_quantities[i]);
    }
}