#include "engine/listeners/OrderBookView.h"

void OrderBookView::on_event(const Event &e)
{
    if (e.type != EventType::LevelAgg)
        return;

    std::lock_guard lock(mtx_);
    const auto &lvl = e.d.level;

    if (lvl.side == Order::Side::Buy)
    {
        if (lvl.aggQty > 0)
            bid_levels_[lvl.px] = lvl.aggQty;
        else
            bid_levels_.erase(lvl.px);
    }
    else
    {
        if (lvl.aggQty > 0)
            ask_levels_[lvl.px] = lvl.aggQty;
        else
            ask_levels_.erase(lvl.px);
    }
}

std::optional<int64_t> OrderBookView::get_qty_at_price(Order::Side side, double px) const
{
    std::lock_guard lock(mtx_);
    if (side == Order::Side::Buy)
    {
        auto it = bid_levels_.find(px);
        if (it != bid_levels_.end())
            return it->second;
    }
    else
    {
        auto it = ask_levels_.find(px);
        if (it != ask_levels_.end())
            return it->second;
    }
    return std::nullopt;
}

std::vector<PriceLevelView> OrderBookView::top_n(Order::Side side, size_t n) const
{
    std::lock_guard lock(mtx_);
    std::vector<PriceLevelView> result;
    result.reserve(n);

    if (side == Order::Side::Buy)
    {
        size_t count = 0;
        for (const auto &[price, qty] : bid_levels_)
        {
            result.push_back(PriceLevelView{price, 0, static_cast<uint32_t>(qty)});
            if (++count >= n)
                break;
        }
    }
    else
    {
        size_t count = 0;
        for (const auto &[price, qty] : ask_levels_)
        {
            result.push_back(PriceLevelView{price, 0, static_cast<uint32_t>(qty)});
            if (++count >= n)
                break;
        }
    }

    return result;
}
