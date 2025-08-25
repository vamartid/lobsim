#include "engine/listeners/MarketDataPublisher.h"

#include <iostream>
#include <thread>
#include <chrono>

MarketDataPublisher::MarketDataPublisher(std::unique_ptr<Dashboard> dashboard)
    : dashboard_(std::move(dashboard)) {}

void MarketDataPublisher::on_event(const Event &e)
{
    if (++event_counter_ % refresh_interval_ == 0)
    {
        refresh_terminal();
    }
}

void MarketDataPublisher::refresh_terminal()
{
    static size_t total_lines_last_frame = 0;

    // if (total_lines_last_frame > 0)
    // {
    //     // Move cursor up by total_lines and clear from cursor down
    //     std::cout << "\x1b[" << total_lines_last_frame << "A\x1b[J";
    // }

    size_t current_total_lines = dashboard_->render_all(std::cout);

    total_lines_last_frame = current_total_lines;

    std::cout << std::flush;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}
