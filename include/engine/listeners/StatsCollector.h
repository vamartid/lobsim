#pragma once
#include "engine/events/IEventListener.h"
#include "engine/listeners/TradeInfo.h"
#include "utils/data_structures/spsc.h"

#include <atomic>
#include <mutex>
#include <optional>
#include <cstdint>
#include <memory>

using TradeBuffer = SPSC<TradeInfo>;

/**
 * @brief Collects live statistics from the EventBus
 *        e.g., total orders, fills, cancellations, top-of-book updates.
 *        Also pushes executed trades into a TradeBuffer for renderers.
 */
class StatsCollector : public IEventListener
{
public:
    explicit StatsCollector(std::shared_ptr<TradeBuffer> trade_buffer = nullptr)
        : trade_buffer_(std::move(trade_buffer)) {}

    void on_event(const Event &e) override;

    uint64_t total_orders() const { return total_orders_.load(); }
    uint64_t total_fills() const { return total_fills_.load(); }
    uint64_t total_cancels() const { return total_cancels_.load(); }

    std::optional<int64_t> last_best_bid() const;
    std::optional<int64_t> last_best_ask() const;

    // --- Derived stats ---
    uint64_t trade_count() const { return total_fills(); }
    double average_spread() const;
    std::shared_ptr<TradeBuffer> trade_buffer() const { return trade_buffer_; }

private:
    std::atomic<uint64_t> total_orders_ = 0;
    std::atomic<uint64_t> total_fills_ = 0;
    std::atomic<uint64_t> total_cancels_ = 0;

    mutable std::mutex mtx_;
    int64_t best_bid_ = 0;
    int64_t best_ask_ = 0;

    std::shared_ptr<TradeBuffer> trade_buffer_;
};
