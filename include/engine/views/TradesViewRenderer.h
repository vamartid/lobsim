#pragma once
#include "engine/views/IView.h"
#include <memory>
#include <ostream>
#include <format>
#include <span>

class TradesViewRenderer : public IView
{
public:
    using WallTime = uint32_t;
    explicit TradesViewRenderer(std::shared_ptr<TradeBuffer> buffer, std::span<const WallTime> tick_times)
        : buffer_(std::move(buffer)), tick_times_(tick_times) {}

    size_t render(std::ostream &os) override
    {
        if (!buffer_)
            return 0;

        auto trades = buffer_->snapshot(3);
        if (trades.empty())
            return 0;

        os << "=== Trades View (last " << trades.size() << ") ===\n";

        size_t lines = 1;
        for (const auto &t : trades)
        {
            os << std::format("{} Timestamp:{}\n", t, tick_times_[t.ts]);
            ++lines;
        }

        return lines;
    }

private:
    std::shared_ptr<TradeBuffer> buffer_;
    std::span<const WallTime> tick_times_; // store span
};
