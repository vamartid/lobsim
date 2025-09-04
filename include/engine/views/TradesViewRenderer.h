#pragma once

#include "engine/views/IView.h"

#include <deque>
#include <format>
#include <memory>
#include <ostream>
#include <span>

class TradesViewRenderer : public IView {
public:
  using WallTime = uint32_t;

  explicit TradesViewRenderer(std::shared_ptr<TradeBuffer> buffer, std::span<const WallTime> tick_times, size_t n = 5)
    : buffer_(std::move(buffer)), tick_times_(tick_times), N_(n) {}

  size_t render(std::ostream &os) override
  {
    // Drain new trades from buffer_
    TradeInfo t;
    while (buffer_->pop(t))
    {
      recent_.push_back(std::move(t));
      if (recent_.size() > N_)
      {
        recent_.pop_front(); // keep only last N
      }
    }

    if (recent_.empty())
      return 0;

    os << "=== Trades View (last " << N_ << ") ===\n";
    for (auto &trade : recent_)
    {
      os << std::format("{} Timestamp:{}\n", trade, tick_times_[trade.ts]);
    }
    return recent_.size();
  }

private:
  std::shared_ptr<TradeBuffer> buffer_;
  std::span<const WallTime> tick_times_; // store span of wall times
  std::deque<TradeInfo> recent_;         // rolling window of last N trades
  size_t N_;
};
