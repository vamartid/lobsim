#include "engine/listeners/MarketDataPublisher.h"

#include <iostream>
#include <thread>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif
MarketDataPublisher::MarketDataPublisher(std::unique_ptr<Dashboard> dashboard)
    : dashboard_(std::move(dashboard)), running_(true)
{
#ifdef _WIN32
    enable_ansi_escape_codes(); // Call this once at the start
#endif
    input_thread_ = std::thread([this]()
                                { input_loop(); });
}

MarketDataPublisher::~MarketDataPublisher()
{
    running_ = false;
    if (input_thread_.joinable())
        input_thread_.join();
}

void MarketDataPublisher::on_event(const Event &e)
{
    if (++event_counter_ % refresh_interval_ == 0)
    {
        refresh_terminal();
    }
}

void MarketDataPublisher::enable_ansi_escape_codes() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        return;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        return;
    }
}

void MarketDataPublisher::refresh_terminal()
{
    // The most reliable way to create a "frame" is to clear the entire screen
    // and reset the cursor position to the top-left corner (0,0).
    std::cout << "\x1b[2J\x1b[H";

    // Call rendering function.
    // It's crucial this function ONLY prints
    size_t current_total_lines = dashboard_->render_all(std::cout);

    // maybe try moves the cursor back up later
    
    // Flush the output buffer to ensure everything is written immediately.
    std::cout << std::flush;

    // Wait for the next frame.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void MarketDataPublisher::handle_key(char key)
{
    switch (key)
    {
    case '1':
        dashboard_->toggle_view(0);
        break; // toggle OrderBookViewRenderer
    case '2':
        dashboard_->toggle_view(1);
        break; // toggle StatsViewRenderer
    case '3':
        dashboard_->toggle_view(2);
        break; // toggle TradesViewRenderer
    }
}

void MarketDataPublisher::input_loop()
{
#ifdef _WIN32
    while (running_)
    {
        if (_kbhit())
        {
            char c = _getch();
            handle_key(c);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
#else
    // Linux / POSIX
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    while (running_)
    {
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0)
            handle_key(c);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}
