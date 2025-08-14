#pragma once
#include <atomic>
#include <cstddef>
#include <new>
#include <utility>
#include <type_traits>

// Simple Lock-Free MPSC Ring Buffer (bounded, single consumer)
// - Multiple producers can push concurrently without locks
// - Single consumer pops
// - Fixed capacity, contiguous storage
// - Head/tail indices managed with atomics for safe concurrent access
//
// Memory ordering basics in this context:
// - memory_order_relaxed: No ordering constraints, just atomicity. Used when we don't care about visibility ordering.
// - memory_order_acquire: Ensures that reads/writes before this load in other threads are visible after this load.
// - memory_order_release: Ensures that all prior writes are visible to threads that perform an acquire load on the same variable.
//
// In a producer-consumer queue:
// Producer: load head with acquire (see latest consumer progress), store tail with release (publish new item)
// Consumer: load tail with acquire (see latest producer progress), store head with release (publish slot freed)
//
// Pseudocode (producer):
//   t = tail.load(relaxed)
//   h = head.load(acquire)
//   if full -> fail
//   write item to slot
//   tail.store(t+1, release)  // publish
//
// Pseudocode (consumer):
//   h = head.load(relaxed)
//   t = tail.load(acquire)
//   if empty -> fail
//   read item from slot
//   head.store(h+1, release)  // publish free slot

template <typename T>
class SimpleMpscRingBuffer
{
public:
    explicit SimpleMpscRingBuffer(size_t capacity)
        : capacity_(capacity), mask_(capacity - 1), buffer_(static_cast<T *>(::operator new[](capacity * sizeof(T)))), head_(0), tail_(0) {}

    ~SimpleMpscRingBuffer()
    {
        // Destroy any remaining constructed elements
        T tmp;
        while (try_pop(tmp))
        {
        }
        ::operator delete[](buffer_);
    }

    SimpleMpscRingBuffer(const SimpleMpscRingBuffer &) = delete;
    SimpleMpscRingBuffer &operator=(const SimpleMpscRingBuffer &) = delete;

    // Producer: try to construct an element in-place.
    // Returns false if buffer is full.
    template <class... Args>
    bool try_emplace(Args &&...args)
    {
        // Read tail without ordering constraints (just need the number)
        size_t t = tail_.load(std::memory_order_relaxed);
        // Acquire head so we see any updates from the consumer
        size_t h = head_.load(std::memory_order_acquire);
        if (t - h >= capacity_)
            return false; // full
        size_t pos = t & mask_;
        // Placement-new into the slot
        new (&buffer_[pos]) T(std::forward<Args>(args)...);
        // Release so consumer sees the new element
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }

    // Consumer: try to pop an element.
    // Returns false if buffer is empty.
    bool try_pop(T &out)
    {
        // Read head without ordering constraints
        size_t h = head_.load(std::memory_order_relaxed);
        // Acquire tail so we see any updates from producers
        size_t t = tail_.load(std::memory_order_acquire);
        if (h == t)
            return false; // empty
        size_t pos = h & mask_;
        T *ptr = &buffer_[pos];
        out = std::move(*ptr);
        ptr->~T();
        // Release so producers see the freed slot
        head_.store(h + 1, std::memory_order_release);
        return true;
    }

private:
    const size_t capacity_;    // total number of slots
    const size_t mask_;        // capacity - 1, for fast wrap-around (capacity must be power of two)
    T *buffer_;                // contiguous storage
    std::atomic<size_t> head_; // consumer index
    std::atomic<size_t> tail_; // producer index
};

// Example usage:
// SimpleMpscRingBuffer<int> q(8);
// Producer threads: q.try_emplace(42);
// Consumer thread: int v; while (q.try_pop(v)) { /* process v */ }
