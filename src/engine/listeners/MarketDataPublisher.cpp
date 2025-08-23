#include "engine/listeners/MarketDataPublisher.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iostream>

MarketDataPublisher::MarketDataPublisher(std::shared_ptr<OrderBookView> book,
                                         std::shared_ptr<StatsCollector> stats)
    : book_(std::move(book)), stats_(std::move(stats)) {}

void MarketDataPublisher::on_event(const Event &e)
{
    // Throttle updates (every N events)
    if (++event_counter_ % refresh_interval_ == 0)
    {
        refresh_terminal();
    }
}

void MarketDataPublisher::refresh_terminal()
{
    // Caching the total number of lines from the previous frame.
    static size_t total_lines_last_frame = 0;

    // If it's not the first time, move the cursor up and clear the screen.
    if (total_lines_last_frame > 0)
    {
        // Move cursor up by total_lines and clear from cursor down
        std::cout << "\x1b[" << total_lines_last_frame << "A\x1b[J";
    }

    // Prepare and print the new frame directly to std::cout
    size_t current_total_lines = prepare_terminal_view(std::cout);

    // Update the line count for the next frame
    total_lines_last_frame = current_total_lines;

    // Flush the output buffer
    std::cout << std::flush;

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
}

// MarketDataPublisher.cpp
size_t MarketDataPublisher::prepare_terminal_view(std::ostream &os)
{
    constexpr size_t depth = 10;
    auto bids = book_->top_n(Order::Side::Buy, depth);
    auto asks = book_->top_n(Order::Side::Sell, depth);

    size_t line_count = 0;

    // Header
    os << "=== Live Market View ===\n\n";
    line_count += 2; // Title line + blank line

    // Order book data
    size_t max_rows = std::max(bids.size(), asks.size());
    for (size_t i = 0; i < max_rows; ++i)
    {
        if (i < bids.size())
            os << std::format("BID {:6} @ {:.2f}", bids[i].aggregate_qty, bids[i].price);
        else
            os << "                     "; // Match spacing for an empty bid side

        if (i < asks.size())
            os << std::format("   ASK {:6} @ {:.2f}", asks[i].aggregate_qty, asks[i].price);

        os << "\n";
        line_count++;
    }

    // Stats line
    os << std::format("\nTrades: {} | Avg Spread: {:.2f}\n",
                      stats_->trade_count(),
                      stats_->average_spread());
    line_count += 2; // Blank line + stats line

    return line_count;
}