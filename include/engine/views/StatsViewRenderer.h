#pragma once
#include "engine/views/IView.h"
#include "engine/listeners/StatsCollector.h"
#include <memory>
#include <ostream>
#include <format>

class StatsViewRenderer : public IView
{
public:
    explicit StatsViewRenderer(std::shared_ptr<StatsCollector> stats)
        : stats_(std::move(stats)) {}

    size_t render(std::ostream &os) override
    {
        if (!stats_)
            return 0;

        os << "=== Stats View ===\n";
        os << std::format("Total Orders: {}\n", stats_->total_orders());
        os << std::format("Total Fills:  {}\n", stats_->total_fills());
        os << std::format("Total Cancels: {}\n", stats_->total_cancels());

        if (auto bid = stats_->last_best_bid())
            os << std::format("Best Bid:    {}\n", *bid);
        if (auto ask = stats_->last_best_ask())
            os << std::format("Best Ask:    {}\n", *ask);

        os << std::format("Trade Count: {} | Avg Spread: {:.2f}\n",
                          stats_->trade_count(),
                          stats_->average_spread());

        return 8; // number of lines printed (adjust if you add/remove fields)
    }

private:
    std::shared_ptr<StatsCollector> stats_;
};
