#pragma once
#include "engine/events/IEventListener.h"
#include "engine/side/PriceLevelView.h"

#include <map>
#include <mutex>
#include <vector>
#include <optional>

/**
 * @brief Maintains an incremental L2 snapshot of the order book.
 *        Listens to EventBus and updates internal state using LevelAgg events.
 */
class OrderBookView : public IEventListener
{
public:
    OrderBookView() = default;

    // IEventListener interface
    void on_event(const Event &e) override;

    // Query methods
    std::optional<int64_t> get_qty_at_price(Order::Side side, double px) const;
    std::vector<PriceLevelView> top_n(Order::Side side, size_t n) const;

private:
    mutable std::mutex mtx_;
    std::map<double, int64_t, std::greater<>> bid_levels_; // descending (best bid = begin)
    std::map<double, int64_t, std::less<>> ask_levels_;    // ascending (best ask = begin)
};
