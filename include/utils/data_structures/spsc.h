// spsc.h
#pragma once
#include <atomic>
#include <cstdint>
#include <vector>

// Single-Producer, Single-Consumer ring buffer
template <typename T>
class SPSC
{
public:
    explicit SPSC(size_t cap_pow2) : mask_(cap_pow2 - 1), buf_(cap_pow2)
    {
        // cap must be power of two
    }

    bool push(const T &v)
    {
        auto h = head_.load(std::memory_order_relaxed);
        auto n = h + 1;
        if (n - tail_.load(std::memory_order_acquire) > buf_.size())
            return false;
        buf_[h & mask_] = v;
        head_.store(n, std::memory_order_release);
        return true;
    }

    bool pop(T &out)
    {
        auto t = tail_.load(std::memory_order_relaxed);
        if (head_.load(std::memory_order_acquire) == t)
            return false;
        out = buf_[t & mask_];
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }

private:
    size_t mask_;
    std::vector<T> buf_;
    std::atomic<uint64_t> head_{0}, tail_{0};
};
