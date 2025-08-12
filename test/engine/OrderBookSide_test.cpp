#include <gtest/gtest.h>

#include "engine/OrderBookSide.h"
#include "utils/Comparator.h"

TEST(OrderBookSideTest, EmptyOrderBookSide)
{
    OrderBookSide<utils::comparator::Descending> side(Side::Buy);
    EXPECT_FALSE(side.best_price().has_value());
    EXPECT_TRUE(side.best_orders().empty());
}

TEST(OrderBookSideTest, RemoveNonExistentOrderDoesNothing)
{
    OrderBookSide<utils::comparator::Ascending> side(Side::Sell);
    side.add_order({1, 0, 50.0, 10, Side::Sell});
    side.remove_order(9999); // non-existent ID

    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 50.0);

    const auto &best_orders = side.best_orders();
    EXPECT_EQ(best_orders.size(), 1);
}

TEST(OrderBookSideTest, RemoveLastOrderRemovesPriceLevel)
{
    OrderBookSide<utils::comparator::Descending> side(Side::Buy);
    side.add_order({1, 0, 100.0, 10, Side::Buy});

    side.remove_order(1);

    EXPECT_FALSE(side.best_price().has_value());
    EXPECT_TRUE(side.best_orders().empty());
}

TEST(OrderBookSideTest, RemoveOrderRemovesCorrectly)
{
    OrderBookSide<utils::comparator::Descending> buy_side(Side::Buy);

    std::vector<Order> orders = {
        {1, 0, 100.0, 10, Side::Buy},
        {2, 0, 100.0, 5, Side::Buy},
        {3, 0, 100.0, 20, Side::Buy}};

    for (const Order &order : orders)
        buy_side.add_order(order);

    // Before removal: best price should be 100.0
    auto best_price = buy_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0);

    // All three orders should be at best price
    const auto &best_orders = buy_side.best_orders();
    EXPECT_EQ(best_orders.size(), 3);

    // Now remove order with id 2
    buy_side.remove_order(2);

    // Best price still 100.0 because other orders remain
    best_price = buy_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0);

    // Now best orders should be 2 orders (id 1 and 3)
    const auto &updated_best_orders = buy_side.best_orders();
    EXPECT_EQ(updated_best_orders.size(), 2);

    // Check that order with id 2 is removed
    for (const auto &order : updated_best_orders)
    {
        EXPECT_NE(order.id, 2);
    }
}

TEST(OrderBookSideTest, AddAndRetrieveBestPriceBuySide)
{
    OrderBookSide<utils::comparator::Descending> buy_side(Side::Buy);

    EXPECT_FALSE(buy_side.best_price().has_value());

    std::vector<Order> orders = {
        {1, 0, 100.0, 10, Side::Buy},
        {2, 0, 101.0, 5, Side::Buy},
        {3, 0, 99.0, 20, Side::Buy}};

    for (const Order &order : orders)
        buy_side.add_order(order);

    // Buy side best price is max price
    auto best_price = buy_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 101.0);

    // Orders at best price
    const auto &best_orders = buy_side.best_orders();
    ASSERT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 2);
}

TEST(OrderBookSideTest, AddAndRetrieveBestPriceSellSide)
{
    OrderBookSide<utils::comparator::Ascending> sell_side(Side::Sell);

    EXPECT_FALSE(sell_side.best_price().has_value());

    std::vector<Order> orders = {
        {1, 0, 100.0, 10, Side::Sell},
        {2, 0, 101.0, 5, Side::Sell},
        {3, 0, 99.0, 20, Side::Sell}};

    for (const Order &order : orders)
        sell_side.add_order(order);

    // Sell side best price is min price
    auto best_price = sell_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 99.0);

    // Orders at best price
    const auto &best_orders = sell_side.best_orders();
    ASSERT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 3);
}

TEST(OrderBookSideTest, MultiplePriceLevelsBestPrice)
{
    OrderBookSide<utils::comparator::Descending> buy_side(Side::Buy);
    buy_side.add_order({1, 0, 100.0, 10, Side::Buy});
    buy_side.add_order({2, 0, 105.0, 5, Side::Buy});
    buy_side.add_order({3, 0, 102.0, 20, Side::Buy});

    auto best_price = buy_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 105.0);

    OrderBookSide<utils::comparator::Ascending> sell_side(Side::Sell);
    sell_side.add_order({4, 0, 200.0, 10, Side::Sell});
    sell_side.add_order({5, 0, 195.0, 5, Side::Sell});
    sell_side.add_order({6, 0, 198.0, 20, Side::Sell});

    best_price = sell_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 195.0);
}

TEST(OrderBookSideTest, BestPriceAndOrdersEmptySide)
{
    OrderBookSide<utils::comparator::Ascending> side(Side::Sell);

    EXPECT_FALSE(side.best_price().has_value());

    const auto &best_orders = side.best_orders();
    EXPECT_TRUE(best_orders.empty());
}

TEST(OrderBookSideTest, AddOrdersWithDuplicateIDs)
{
    OrderBookSide<utils::comparator::Descending> side(Side::Buy);

    side.add_order({1, 0, 100.0, 10, Side::Buy});
    side.add_order({1, 0, 101.0, 5, Side::Buy}); // Same ID, different price

    // Depending on your design, you may:
    // - allow duplicates, then best_price() should be 101.0 (max price for Buy)
    // - or expect only one order per ID (you might want to reject duplicates)

    // Here we just test current behavior (likely both exist)
    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 101.0);

    // Check how many orders at price 100 and 101
    // We don't have direct access; let's check best_orders at best price (101)
    const auto &best_orders = side.best_orders();
    EXPECT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 1);

    // To be thorough, try removing order ID=1 and check remaining orders
    side.remove_order(1); // Should remove the order at price 101

    best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0);

    const auto &orders_at_100 = side.best_orders();
    EXPECT_EQ(orders_at_100.size(), 1);
    EXPECT_EQ(orders_at_100.front().id, 1); // The other order with same ID still here
}

TEST(OrderBookSideTest, LargeNumberOfOrders)
{
    OrderBookSide<utils::comparator::Ascending> side(Side::Sell);

    constexpr int N = 100000;

    // Add N orders with incremental prices and IDs
    for (int i = 0; i < N; ++i)
    {
        side.add_order({static_cast<uint64_t>(i), 0, 100.0 + i * 0.01, 1, Side::Sell});
    }

    // Best price for Sell side should be the smallest price
    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0);

    // Remove half of the orders
    for (int i = 0; i < N / 2; ++i)
    {
        side.remove_order(i);
    }

    // Best price should now be the next smallest price (100.0 + (N/2)*0.01)
    best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0 + (N / 2) * 0.01);
}

TEST(OrderBookSideTest, FloatingPointPricePrecision)
{
    OrderBookSide<utils::comparator::Descending> side(Side::Buy);

    // Add orders with very close prices
    side.add_order({1, 0, 100.000001, 10, Side::Buy});
    side.add_order({2, 0, 100.000002, 20, Side::Buy});
    side.add_order({3, 0, 100.000000, 30, Side::Buy});

    // Best price for Buy is max price, should be 100.000002
    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_NEAR(best_price.value(), 100.000002, 1e-9);

    const auto &best_orders = side.best_orders();
    EXPECT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 2);
}
