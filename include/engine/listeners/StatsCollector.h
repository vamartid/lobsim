#pragma once
#include "engine/events/IEventListener.h"
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <cstdint>

/**
 * @brief Collects live statistics from the EventBus
 *        e.g., total orders, fills, cancellations, top-of-book updates.
 */
class StatsCollector : public IEventListener
{
public:
    StatsCollector() = default;

    void on_event(const Event &e) override;

    uint64_t total_orders() const { return total_orders_.load(); }
    uint64_t total_fills() const { return total_fills_.load(); }
    uint64_t total_cancels() const { return total_cancels_.load(); }

    std::optional<int64_t> last_best_bid() const;
    std::optional<int64_t> last_best_ask() const;

    // --- Add these ---
    uint64_t trade_count() const { return total_fills(); } // simple alias
    double average_spread() const;

private:
    std::atomic<uint64_t> total_orders_ = 0;
    std::atomic<uint64_t> total_fills_ = 0;
    std::atomic<uint64_t> total_cancels_ = 0;

    mutable std::mutex mtx_;
    int64_t best_bid_ = 0;
    int64_t best_ask_ = 0;
};
