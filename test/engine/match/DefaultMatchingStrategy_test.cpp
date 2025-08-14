#include <gtest/gtest.h>

#include "core/Order.h"
#include "engine/match/DefaultMatchingStrategy.h"
#include "test_utils/OrderFactory.h"

#include <unordered_map>
#include <list>
#include <optional>

using namespace std;

class DefaultMatchingStrategyTest : public ::testing::Test
{
protected:
    DefaultMatchingStrategy strategy;
    OrderBookSide<utils::comparator::Descending> bids{Order::Side::Buy};
    OrderBookSide<utils::comparator::Ascending> asks{Order::Side::Sell};

    unordered_map<uint64_t, tuple<Order::Side, double, list<Order>::iterator>> id_lookup;

    // Add order and update id_lookup by searching internal price_levels_
    void add_order(OrderBookSide<utils::comparator::Descending> &book, const Order &order)
    {
        book.add_order(order);
        auto &price_level = book.price_levels()[order.price];
        for (auto it = price_level.begin(); it != price_level.end(); ++it)
        {
            if (it->id == order.id)
            {
                id_lookup[order.id] = make_tuple(order.side(), order.price, it);
                break;
            }
        }
    }

    void add_order(OrderBookSide<utils::comparator::Ascending> &book, const Order &order)
    {
        book.add_order(order);
        auto &price_level = book.price_levels()[order.price];
        for (auto it = price_level.begin(); it != price_level.end(); ++it)
        {
            if (it->id == order.id)
            {
                id_lookup[order.id] = make_tuple(order.side(), order.price, it);
                break;
            }
        }
    }

    uint32_t get_quantity(uint64_t id)
    {
        auto it = id_lookup.find(id);
        if (it == id_lookup.end())
            return 0;
        return get<2>(it->second)->quantity;
    }

    bool has_order(uint64_t id)
    {
        return id_lookup.find(id) != id_lookup.end();
    }
};

// 1. Empty order books => no matches
TEST_F(DefaultMatchingStrategyTest, HandlesEmptyOrderBooks)
{
    strategy.match(bids, asks, id_lookup);
    EXPECT_TRUE(bids.best_price() == std::nullopt);
    EXPECT_TRUE(asks.best_price() == std::nullopt);
}

// 2. No price overlap => no trade
TEST_F(DefaultMatchingStrategyTest, NoMatchIfBestBidLessThanBestAsk)
{
    add_order(bids, TestOrderFactory::CreateBuy(1, 99.0, 10));
    add_order(asks, TestOrderFactory::CreateSell(2, 100.0, 10));
    strategy.match(bids, asks, id_lookup);
    EXPECT_EQ(get_quantity(1), 10);
    EXPECT_EQ(get_quantity(2), 10);
}

// 3. Partial fills
TEST_F(DefaultMatchingStrategyTest, PartialFillReducesQuantity)
{
    add_order(bids, TestOrderFactory::CreateBuy(1, 101.0, 10));
    add_order(asks, TestOrderFactory::CreateSell(2, 100.0, 5));
    strategy.match(bids, asks, id_lookup);
    EXPECT_EQ(get_quantity(1), 5);
    EXPECT_FALSE(has_order(2));
}

// 4. Full fills remove orders
TEST_F(DefaultMatchingStrategyTest, FullFillRemovesOrder)
{
    add_order(bids, TestOrderFactory::CreateBuy(1, 101.0, 5));
    add_order(asks, TestOrderFactory::CreateSell(2, 100.0, 5));
    strategy.match(bids, asks, id_lookup);
    EXPECT_FALSE(has_order(1));
    EXPECT_FALSE(has_order(2));
}

// 5. Multiple orders at same price FIFO matching
TEST_F(DefaultMatchingStrategyTest, MatchesMultipleOrdersAtSamePriceFIFO)
{
    add_order(bids, TestOrderFactory::CreateBuy(1, 101.0, 5));
    add_order(bids, TestOrderFactory::CreateBuy(2, 101.0, 5));
    add_order(asks, TestOrderFactory::CreateSell(3, 100.0, 8));
    strategy.match(bids, asks, id_lookup);
    EXPECT_EQ(get_quantity(2), 2);
    EXPECT_FALSE(has_order(1));
    EXPECT_FALSE(has_order(3));
}

// 6. Large quantity handling (no overflow)
TEST_F(DefaultMatchingStrategyTest, HandlesLargeQuantities)
{
    uint32_t large_qty = std::numeric_limits<uint32_t>::max() / 2;
    add_order(bids, TestOrderFactory::CreateBuy(1, 101.0, large_qty));
    add_order(asks, TestOrderFactory::CreateSell(2, 100.0, large_qty));
    strategy.match(bids, asks, id_lookup);
    EXPECT_FALSE(has_order(1));
    EXPECT_FALSE(has_order(2));
}

// 7. Multiple price levels matched in one call
TEST_F(DefaultMatchingStrategyTest, MatchesMultiplePriceLevelsInLoop)
{
    add_order(bids, TestOrderFactory::CreateBuy(1, 103.0, 5));
    add_order(bids, TestOrderFactory::CreateBuy(2, 102.0, 5));
    add_order(asks, TestOrderFactory::CreateSell(3, 101.0, 6));
    add_order(asks, TestOrderFactory::CreateSell(4, 102.0, 4));
    strategy.match(bids, asks, id_lookup);
    EXPECT_FALSE(has_order(2)); // order #2 fully filled
    EXPECT_FALSE(has_order(3));
    EXPECT_FALSE(has_order(4));
}