#pragma once

#include "engine/views/IView.h"
#include "engine/listeners/OrderBookView.h"
#include <memory>
#include <format>
#include <algorithm>

class OrderBookViewRenderer : public IView
{
public:
    explicit OrderBookViewRenderer(std::shared_ptr<OrderBookView> book)
        : book_(std::move(book)) {}

    size_t render(std::ostream &os) override
    {
        constexpr size_t depth = 10;
        auto bids = book_->top_n(Order::Side::Buy, depth);
        auto asks = book_->top_n(Order::Side::Sell, depth);

        size_t line_count = 0;

        os << "=== Order Book ===\n";
        line_count++;

        size_t max_rows = std::max(bids.size(), asks.size());
        for (size_t i = 0; i < max_rows; ++i)
        {
            if (i < bids.size())
                os << std::format("BID {:6} @ {:.2f}", bids[i].aggregate_qty, bids[i].price);
            else
                os << "                     ";

            if (i < asks.size())
                os << std::format("   ASK {:6} @ {:.2f}", asks[i].aggregate_qty, asks[i].price);

            os << "\n";
            line_count++;
        }

        return line_count;
    }

private:
    std::shared_ptr<OrderBookView> book_;
};
