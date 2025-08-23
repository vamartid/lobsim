#pragma once

#include <memory>
#include <vector>
#include "engine/listeners/OrderBookView.h"
#include "engine/listeners/StatsCollector.h"
#include "engine/events/Events.h"
#include <ostream>
// MarketDataPublisher subscribes to EventBus and renders a terminal UI
class MarketDataPublisher
{
public:
    MarketDataPublisher(std::shared_ptr<OrderBookView> book,
                        std::shared_ptr<StatsCollector> stats);

    // Called on each event via EventBus
    void on_event(const Event &e);

private:
    void refresh_terminal();
    size_t prepare_terminal_view(std::ostream &os);

    std::shared_ptr<OrderBookView> book_;
    std::shared_ptr<StatsCollector> stats_;

    // For throttling refresh (optional)
    size_t event_counter_ = 0;
    size_t refresh_interval_ = 10; // refresh every N events
    bool first_time_ = true;
};
