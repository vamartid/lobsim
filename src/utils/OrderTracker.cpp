#include "utils/OrderTracker.h"
#include "utils/OrderRenderer.h"
#include <mutex>
#include <iostream>
#include <charconv>
#include <cmath>
#include <algorithm>

#include <thread>
#include <chrono>

thread_local std::array<char, OrderTracker::BUFFER_SIZE> OrderTracker::final_buffer_;

// ===================== Enable / Updates =====================
void OrderTracker::enable(bool on)
{
    if (on)
        std::cout << "\033[2J\033[H";
    enabled_.store(on, std::memory_order_release);
    if (!on)
    {
        std::unique_lock lock(mutex_);
        orders_.clear();
        order_feeder_map_.clear();
        updated_ = false;
    }
}

bool OrderTracker::is_enabled() const
{
    return enabled_.load(std::memory_order_acquire);
}

void OrderTracker::add_order(const Order &order)
{
    if (!enabled_.load(std::memory_order_acquire))
        return;

    std::unique_lock lock(mutex_);
    auto [it, inserted] = orders_.emplace(order.id, order);
    if (!inserted)
        it->second = order;
    updated_.store(true, std::memory_order_release);
}

void OrderTracker::add_order(const Order &order, uint16_t feeder_id)
{
    if (!enabled_.load(std::memory_order_acquire))
        return;

    std::unique_lock lock(mutex_);
    auto [it, inserted] = orders_.emplace(order.id, order);
    if (!inserted)
        it->second = order;
    order_feeder_map_[order.id] = feeder_id;
    updated_.store(true, std::memory_order_release);
}

void OrderTracker::update_order(const Order &order)
{
    if (!enabled_.load(std::memory_order_acquire))
        return;

    std::unique_lock lock(mutex_);
    auto it = orders_.find(order.id);
    if (it != orders_.end())
    {
        it->second = order;
        updated_.store(true, std::memory_order_release);
    }
}

void OrderTracker::remove_order(uint64_t order_id)
{
    if (!enabled_.load(std::memory_order_acquire))
        return;

    std::unique_lock lock(mutex_);
    if (orders_.erase(order_id) > 0)
    {
        order_feeder_map_.erase(order_id);
        updated_.store(true, std::memory_order_release);
    }
}

std::vector<Order> OrderTracker::get_snapshot() const
{
    std::shared_lock lock(mutex_);
    std::vector<Order> snapshot;
    snapshot.reserve(orders_.size());
    for (const auto &pair : orders_)
        snapshot.push_back(pair.second);
    return snapshot;
}

bool OrderTracker::has_updates() const
{
    return updated_.exchange(false, std::memory_order_acq_rel);
}

// ===================== Render Live View =====================
void OrderTracker::render_live_view() const
{
    if (orders_.empty())
        return;

    std::shared_lock lock(mutex_);
    size_t pos = 0;

    OrderRenderer::build_side_by_side_view(*this, final_buffer_, pos);

    std::cout.write(final_buffer_.data(), pos);

    // Clear screen and move cursor to top-left
    // ANSI escape codes (cross-platform for most terminals)
    std::cout << "\033[H"; // Move cursor to top-left
    std::this_thread::sleep_for(std::chrono::milliseconds(180));
}
