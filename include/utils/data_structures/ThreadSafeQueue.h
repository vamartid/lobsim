#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

// thread-safe queue

template <typename T>
class ThreadSafeQueue
{
public:
    void push(T item)
    {
        // lock and notify when available
        std::lock_guard<std::mutex> lock(mutex_);
        // Pass-by-value-for-move-or-copy
        // item has it's own copy of the object so we can move it
        // cheap operation, avoids a potentially expensive memory allocation and copy.
        queue_.push(std::move(item));
        cv_.notify_one(); // notifies consumers waiting in wait_and_pop
    }

    std::optional<T> pop()
    {
        // non-blocking pop
        // only removes an element if one is available
        // it doesn’t need to notify anyone
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty())
        {
            return std::nullopt;
        }
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    T wait_and_pop()
    {
        // should I notify here?
        // No, you don’t need to call cv_.notify_one() inside wait_and_pop()
        // because nothing is waiting for the queue to become less full.
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]
                 { return !queue_.empty(); });
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
