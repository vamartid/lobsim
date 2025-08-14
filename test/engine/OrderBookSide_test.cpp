#include <gtest/gtest.h>

#include "engine/OrderBookSide.h"
#include "utils/Comparator.h"
#include "test_utils/OrderFactory.h"

TEST(OrderBookSideTest, EmptyOrderBookSide)
{
    OrderBookSide<utils::comparator::Descending> side(Order::Side::Buy);
    EXPECT_FALSE(side.best_price().has_value());
    EXPECT_TRUE(side.best_orders().empty());
}

TEST(OrderBookSideTest, RemoveNonExistentOrderDoesNothing)
{
    OrderBookSide<utils::comparator::Ascending> side(Order::Side::Sell);
    side.add_order(TestOrderFactory::CreateSell(1, 50.0, 10));
    side.remove_order(9999); // non-existent ID

    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 50.0);

    const auto &best_orders = side.best_orders();
    EXPECT_EQ(best_orders.size(), 1);
}

TEST(OrderBookSideTest, RemoveLastOrderRemovesPriceLevel)
{
    OrderBookSide<utils::comparator::Descending> side(Order::Side::Buy);
    side.add_order(TestOrderFactory::CreateBuy(1, 100.0, 10));

    side.remove_order(1);

    EXPECT_FALSE(side.best_price().has_value());
    EXPECT_TRUE(side.best_orders().empty());
}

TEST(OrderBookSideTest, RemoveOrderRemovesCorrectly)
{
    OrderBookSide<utils::comparator::Descending> buy_side(Order::Side::Buy);

    std::vector<Order> orders = {
        TestOrderFactory::CreateBuy(1, 100.0, 10),
        TestOrderFactory::CreateBuy(2, 100.0, 5),
        TestOrderFactory::CreateBuy(3, 100.0, 20)};

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
    OrderBookSide<utils::comparator::Descending> buy_side(Order::Side::Buy);

    EXPECT_FALSE(buy_side.best_price().has_value());

    std::vector<Order> orders = {
        TestOrderFactory::CreateBuy(1, 100.0, 10),
        TestOrderFactory::CreateBuy(2, 101.0, 5),
        TestOrderFactory::CreateBuy(3, 99.0, 20)};

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
    OrderBookSide<utils::comparator::Ascending> sell_side(Order::Side::Sell);

    EXPECT_FALSE(sell_side.best_price().has_value());

    std::vector<Order> orders = {
        TestOrderFactory::CreateSell(1, 100.0, 10),
        TestOrderFactory::CreateSell(2, 101.0, 5),
        TestOrderFactory::CreateSell(3, 99.0, 20)};

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
    OrderBookSide<utils::comparator::Descending> buy_side(Order::Side::Buy);
    buy_side.add_order(TestOrderFactory::CreateBuy(1, 100.0, 10));
    buy_side.add_order(TestOrderFactory::CreateBuy(2, 105.0, 5));
    buy_side.add_order(TestOrderFactory::CreateBuy(3, 102.0, 20));

    auto best_price = buy_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 105.0);

    OrderBookSide<utils::comparator::Ascending> sell_side(Order::Side::Sell);
    sell_side.add_order(TestOrderFactory::CreateSell(4, 200.0, 10));
    sell_side.add_order(TestOrderFactory::CreateSell(5, 195.0, 5));
    sell_side.add_order(TestOrderFactory::CreateSell(6, 198.0, 20));

    best_price = sell_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 195.0);
}

TEST(OrderBookSideTest, BestPriceAndOrdersEmptySide)
{
    OrderBookSide<utils::comparator::Ascending> side(Order::Side::Sell);

    EXPECT_FALSE(side.best_price().has_value());

    const auto &best_orders = side.best_orders();
    EXPECT_TRUE(best_orders.empty());
}

TEST(OrderBookSideTest, AddOrdersWithDuplicateIDs)
{
    OrderBookSide<utils::comparator::Descending> side(Order::Side::Buy);

    side.add_order(TestOrderFactory::CreateBuy(1, 100.0, 10));
    side.add_order(TestOrderFactory::CreateBuy(1, 101.0, 5)); // Same ID, different price

    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 101.0);

    const auto &best_orders = side.best_orders();
    EXPECT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 1);

    // Remove the order at price 101
    side.remove_order(1);

    best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0);

    const auto &orders_at_100 = side.best_orders();
    EXPECT_EQ(orders_at_100.size(), 1);
    EXPECT_EQ(orders_at_100.front().id, 1);
}

TEST(OrderBookSideTest, LargeNumberOfOrders)
{
    OrderBookSide<utils::comparator::Ascending> side(Order::Side::Sell);

    constexpr int N = 100000;

    // Add N orders with incremental prices and IDs
    for (int i = 0; i < N; ++i)
    {
        side.add_order(TestOrderFactory::CreateSell(i, 100.0 + i * 0.01, 1));
    }

    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0);

    // Remove half of the orders
    for (int i = 0; i < N / 2; ++i)
    {
        side.remove_order(i);
    }

    best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0 + (N / 2) * 0.01);
}

TEST(OrderBookSideTest, FloatingPointPricePrecision)
{
    OrderBookSide<utils::comparator::Descending> side(Order::Side::Buy);

    side.add_order(TestOrderFactory::CreateBuy(1, 100.000001, 10));
    side.add_order(TestOrderFactory::CreateBuy(2, 100.000002, 20));
    side.add_order(TestOrderFactory::CreateBuy(3, 100.000000, 30));

    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_NEAR(best_price.value(), 100.000002, 1e-9);

    const auto &best_orders = side.best_orders();
    EXPECT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 2);
}
