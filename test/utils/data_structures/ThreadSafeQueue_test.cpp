#include <gtest/gtest.h>

#include "utils/data_structures/ThreadSafeQueue.h"

#include <thread>
#include <chrono>
#include <mutex>

TEST(ThreadSafeQueueTest, PushAndPop)
{
    ThreadSafeQueue<int> queue;
    queue.push(42);
    auto result = queue.pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

TEST(ThreadSafeQueueTest, PopFromEmptyReturnsNullopt)
{
    ThreadSafeQueue<int> queue;
    auto result = queue.pop();
    EXPECT_FALSE(result.has_value());
}

TEST(ThreadSafeQueueTest, WaitAndPopBlocksUntilAvailable)
{
    ThreadSafeQueue<int> queue;
    int result = -1;

    std::thread consumer([&]
                         { result = queue.wait_and_pop(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.push(99);
    consumer.join();

    EXPECT_EQ(result, 99);
}

TEST(ThreadSafeQueueTest, MultiplePushMaintainsOrder)
{
    ThreadSafeQueue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);

    EXPECT_EQ(queue.pop().value(), 1);
    EXPECT_EQ(queue.pop().value(), 2);
    EXPECT_EQ(queue.pop().value(), 3);
}

TEST(ThreadSafeQueueTest, ConcurrentPushAndWaitAndPop)
{
    ThreadSafeQueue<int> queue;
    std::vector<int> results;
    std::mutex results_mutex;

    std::thread producer([&]
                         {
        for (int i = 0; i < 5; ++i)
        {
            queue.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } });

    std::thread consumer([&]
                         {
        for (int i = 0; i < 5; ++i)
        {
            int val = queue.wait_and_pop();
            std::lock_guard<std::mutex> lock(results_mutex);
            results.push_back(val);
        } });

    producer.join();
    consumer.join();

    std::lock_guard<std::mutex> lock(results_mutex);
    ASSERT_EQ(results.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(results[i], i);
    }
}

TEST(ThreadSafeQueueTest, MultipleProducersConsumers)
{
    ThreadSafeQueue<int> queue;
    const int num_producers = 4;
    const int num_consumers = 4;
    const int items_per_producer = 1000;
    std::atomic<int> total_popped = 0;

    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p)
    {
        producers.emplace_back([&, p]
                               {
            for (int i = 0; i < items_per_producer; ++i) {
                queue.push(p * items_per_producer + i);
            } });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c)
    {
        consumers.emplace_back([&]
                               {
            for (;;) {
                auto item = queue.pop();
                if (item.has_value()) {
                    ++total_popped;
                } else if (total_popped >= num_producers * items_per_producer) {
                    break;
                }
            } });
    }

    for (auto &p : producers)
        p.join();
    for (auto &c : consumers)
        c.join();

    EXPECT_EQ(total_popped.load(), num_producers * items_per_producer);
}

TEST(ThreadSafeQueueTest, WaitAndPopDelayedProducer)
{
    ThreadSafeQueue<int> queue;
    std::vector<int> results;

    std::thread consumer([&]
                         {
        for (int i = 0; i < 3; ++i) {
            results.push_back(queue.wait_and_pop());
        } });

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // let consumer block

    std::thread producer([&]
                         {
        queue.push(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.push(2);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.push(3); });

    producer.join();
    consumer.join();

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], 1);
    EXPECT_EQ(results[1], 2);
    EXPECT_EQ(results[2], 3);
}

TEST(ThreadSafeQueueTest, InterleavedPushPopRaceCondition)
{
    ThreadSafeQueue<int> queue;
    std::atomic<int> popped_count = 0;
    const int total = 10000;

    std::thread producer([&]
                         {
        for (int i = 0; i < total; ++i) {
            queue.push(i);
        } });

    std::thread consumer([&]
                         {
        while (popped_count < total) {
            auto val = queue.pop();
            if (val.has_value()) {
                ++popped_count;
            }
        } });

    producer.join();
    consumer.join();
    EXPECT_EQ(popped_count.load(), total);
}
