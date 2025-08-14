#include "engine/match/DefaultMatchingStrategy.h"
#include "utils/log/Printer.h"

#include <algorithm>
#include <sstream>
#include <iostream>

void DefaultMatchingStrategy::match(
    OrderBookSide<utils::comparator::Descending> &bids,
    OrderBookSide<utils::comparator::Ascending> &asks,
    std::unordered_map<uint64_t, std::tuple<Order::Side, double, std::list<Order>::iterator>> &id_lookup)
{
    while (true)
    {
        auto best_bid_price = bids.best_price();
        auto best_ask_price = asks.best_price();

        if (!best_bid_price || !best_ask_price)
            break;

        if (*best_bid_price < *best_ask_price)
            break;

        auto &bid_queue = bids.best_orders();
        auto &ask_queue = asks.best_orders();

        if (bid_queue.empty() || ask_queue.empty())
            break;

        Order &best_bid = bid_queue.front();
        Order &best_ask = ask_queue.front();

        uint64_t trade_qty = std::min(best_bid.quantity, best_ask.quantity);

        // printer::print_match(std::cout, best_ask, best_bid, trade_qty);
        // order_tracker.mark_matched(order_id);

        best_bid.quantity -= trade_qty;
        best_ask.quantity -= trade_qty;

        if (best_bid.quantity == 0)
        {
            id_lookup.erase(best_bid.id);
            bids.remove_order(best_bid.id);
        }
        if (best_ask.quantity == 0)
        {
            id_lookup.erase(best_ask.id);
            asks.remove_order(best_ask.id);
        }
    }
}
