#include "engine/listeners/StatsCollector.h"
#include "engine/events/Events.h"

void StatsCollector::on_event(const Event &e)
{
    switch (e.type)
    {
    case EventType::Fill:
        total_fills_++;
        break;
    case EventType::OrderAdded:
        total_orders_++;
        break;
    case EventType::OrderRemoved:
        total_cancels_++;
        break;
    case EventType::LevelAgg:
    {
        std::lock_guard lock(mtx_);
        if (e.d.level.side == Order::Side::Buy)
            best_bid_ = e.d.level.px;
        else
            best_ask_ = e.d.level.px;
    }
    break;
    default:
        break;
    }
}

double StatsCollector::average_spread() const
{
    std::lock_guard lock(mtx_);
    if (best_bid_ == 0 || best_ask_ == 0)
        return 0.0;
    return static_cast<double>(best_ask_ - best_bid_);
}

std::optional<int64_t> StatsCollector::last_best_bid() const
{
    std::lock_guard lock(mtx_);
    return best_bid_ > 0 ? std::make_optional(best_bid_) : std::nullopt;
}

std::optional<int64_t> StatsCollector::last_best_ask() const
{
    std::lock_guard lock(mtx_);
    return best_ask_ > 0 ? std::make_optional(best_ask_) : std::nullopt;
}
