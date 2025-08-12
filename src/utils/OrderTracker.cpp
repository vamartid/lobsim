#include "core/Order.h"
#include "utils/OrderTracker.h"

#include <iostream> // for example output in render_live_view
#include <iomanip>  // For setw, fixed, setprecision
#include <sstream>
#include <string>
#include <algorithm>

void OrderTracker::enable(bool on)
{
    enabled_.store(on, std::memory_order_release);
    if (!on)
    {
        std::unique_lock lock(mutex_);
        orders_.clear();
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
        it->second = order; // replace if exists

    // Do NOT update order_feeder_map_ here

    updated_.store(true, std::memory_order_release);
}
void OrderTracker::add_order(const Order &order, uint16_t feeder_id)
{
    if (!enabled_.load(std::memory_order_acquire))
        return;

    std::unique_lock lock(mutex_);
    auto [it, inserted] = orders_.emplace(order.id, order);
    if (!inserted)
        it->second = order; // replace if exists

    order_feeder_map_[order.id] = feeder_id; // Track feeder id

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
    // else: order not found, ignore or optionally add
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

bool OrderTracker::has_updates()
{
    // Atomically check and reset updated flag
    return updated_.exchange(false, std::memory_order_acq_rel);
}

void OrderTracker::render_live_view()
{
    if (!is_enabled() || !has_updates())
        return;
    std::shared_lock lock(mutex_); // acquire shared lock to read orders_
    std::ostringstream oss;

    // Capture output separately
    std::stringstream ss_order_summary;
    std::stringstream ss_top_feeders;
    std::stringstream ss_orders_by_feeder;

    render_order_summary(ss_order_summary);
    render_top_feeders(ss_top_feeders, 5);
    render_orders_by_feeder(ss_orders_by_feeder);

    // Combine side-by-side and append to main output
    oss << utils::string::print_two_columns(std::move(ss_order_summary), std::move(utils::string::print_two_columns(std::move(ss_top_feeders), std::move(ss_orders_by_feeder), 40)), 40).str();

    render_order_flow_imbalance(oss);

    std::cout << oss.str();
}

void OrderTracker::render_order_summary(std::ostream &os) const
{
    size_t buy_count = 0, sell_count = 0;
    double buy_qty = 0, sell_qty = 0;
    double buy_price_sum = 0, sell_price_sum = 0;

    for (const auto &[id, order] : orders_)
    {
        if (order.side == Side::Buy)
        {
            ++buy_count;
            buy_qty += order.quantity;
            buy_price_sum += order.price;
        }
        else
        {
            ++sell_count;
            sell_qty += order.quantity;
            sell_price_sum += order.price;
        }
    }

    os << "=== Order Summary ===\n"
       << std::left << std::setw(10) << "Side"
       << std::setw(10) << "Count"
       << std::setw(15) << "Total Qty"
       << std::setw(15) << "Avg Price"
       << "\n";

    os << std::left << std::setw(10) << "BUY"
       << std::setw(10) << buy_count
       << std::setw(15) << buy_qty
       << std::setw(15) << (buy_count ? buy_price_sum / buy_count : 0)
       << "\n";

    os << std::left << std::setw(10) << "SELL"
       << std::setw(10) << sell_count
       << std::setw(15) << sell_qty
       << std::setw(15) << (sell_count ? sell_price_sum / sell_count : 0)
       << "\n\n";
}

void OrderTracker::render_top_feeders(std::ostream &os, size_t top_n) const
{
    std::unordered_map<uint16_t, size_t> volume_per_feeder;

    for (const auto &[id, order] : orders_)
    {
        auto it = order_feeder_map_.find(order.id);
        if (it == order_feeder_map_.end())
            continue; // or handle missing feeder ID differently
        uint16_t feeder_id = it->second;
        volume_per_feeder[feeder_id] += order.quantity;
    }

    std::vector<std::pair<uint16_t, size_t>> feeders(volume_per_feeder.begin(), volume_per_feeder.end());

    // Sort descending by volume (second value)
    std::sort(feeders.begin(), feeders.end(),
              [](auto &a, auto &b)
              { return a.second > b.second; });

    os << "=== Top Feeders ===\n";
    for (size_t i = 0; i < std::min(top_n, feeders.size()); ++i)
    {
        os << "Feeder " << feeders[i].first << ": " << feeders[i].second << " qty\n";
    }
    os << "\n";
}

void OrderTracker::render_orders_by_feeder(std::ostream &os) const
{
    std::unordered_map<uint16_t, size_t> order_count;
    for (const auto &[id, order] : orders_)
    {
        auto it = order_feeder_map_.find(order.id);
        if (it == order_feeder_map_.end())
            continue; // or handle missing feeder ID differently
        uint16_t feeder_id = it->second;
        ++order_count[feeder_id];
    }

    os << "=== Orders by Feeder ===\n";
    for (const auto &[feeder, count] : order_count)
    {
        os << "Feeder " << feeder << ": " << count << "\n";
    }
    os << "\n";
}

// void OrderTracker::render_order_book_ladder(std::ostream &os, size_t depth) const
// {
//     os << "=== Order Book Ladder ===\n";
//     os << std::setw(20) << "BUY" << " | " << "SELL\n";

//     auto buys = order_book_.get_levels(Side::Buy, depth);
//     auto sells = order_book_.get_levels(Side::Sell, depth);

//     for (size_t i = 0; i < depth; ++i)
//     {
//         std::ostringstream buy_str, sell_str;

//         if (i < buys.size())
//         {
//             buy_str << std::fixed << std::setprecision(2)
//                     << buys[i].price << "  " << buys[i].qty;
//         }
//         if (i < sells.size())
//         {
//             sell_str << std::fixed << std::setprecision(2)
//                      << sells[i].qty << "  " << sells[i].price;
//         }

//         os << std::setw(20) << buy_str.str()
//            << " | " << sell_str.str() << "\n";
//     }
//     os << "\n";
// }

// void OrderTracker::render_trade_blotter(std::ostream &os, size_t count) const
// {
//     os << "=== Trade Blotter ===\n";
//     os << std::left << std::setw(10) << "Time"
//        << std::setw(6) << "Side"
//        << std::setw(10) << "Price"
//        << std::setw(10) << "Qty"
//        << "\n";

//     auto trades = trade_history_.get_last(count);
//     for (const auto &trade : trades)
//     {
//         os << std::setw(10) << trade.timestamp
//            << std::setw(6) << (trade.side == Side::Buy ? "BUY" : "SELL")
//            << std::setw(10) << trade.price
//            << std::setw(10) << trade.quantity
//            << "\n";
//     }
//     os << "\n";
// }

// void OrderTracker::render_feeder_latency(std::ostream &os) const
// {
//     os << "=== Feeder Latency ===\n";
//     for (auto &[feeder, stats] : latency_stats_)
//     {
//         os << "Feeder " << feeder
//            << ": min=" << stats.min
//            << "ms, avg=" << stats.avg
//            << "ms, max=" << stats.max
//            << "ms\n";
//     }
//     os << "\n";
// }

void OrderTracker::render_order_flow_imbalance(std::ostream &os) const
{
    double buy_qty = 0, sell_qty = 0;
    for (const auto &[id, order] : orders_)
    {
        if (order.side == Side::Buy)
            buy_qty += order.quantity;
        else
            sell_qty += order.quantity;
    }

    double imbalance = (buy_qty - sell_qty) / (buy_qty + sell_qty) * 100.0;

    os << "=== Order Flow Imbalance ===\n";
    os << "Imbalance: " << std::fixed << std::setprecision(2) << imbalance << "%\n\n";
}

// void OrderTracker::render_position_summary(std::ostream &os) const
// {
//     os << "=== Position Summary ===\n";
//     for (auto &[symbol, pos] : positions_)
//     {
//         os << symbol << ": "
//            << "Pos=" << pos.qty
//            << ", Unrealized P&L=" << pos.unrealized
//            << ", Realized P&L=" << pos.realized << "\n";
//     }
//     os << "\n";
// }
