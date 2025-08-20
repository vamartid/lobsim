#include <gtest/gtest.h>

#include "engine/side/OrderBookSide.h"
#include "test_utils/OrderFactory.h"

TEST(OrderBookSideTest, EmptyOrderBookSide)
{
    BidBookSide side;
    EXPECT_FALSE(side.best_price().has_value());
}

TEST(OrderBookSideTest, RemoveNonExistentOrderDoesNothing)
{
    AskBookSide side;
    side.add_order(TestOrderFactory::CreateSell(1, 50.0, 10));

    // Try to remove a non-existent order manually
    double price = 50.0;
    auto &orders = side.get_orders_at_price(price);
    auto it = std::find_if(orders.begin(), orders.end(), [](const Order &o)
                           { return o.id == 9999; });
    if (it != orders.end())
        orders.erase(it); // should do nothing

    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 50.0);

    EXPECT_EQ(side.get_orders_at_price(best_price.value()).size(), 1);
}

TEST(OrderBookSideTest, RemoveLastOrderRemovesPriceLevel)
{
    BidBookSide side;
    side.add_order(TestOrderFactory::CreateBuy(1, 100.0, 10));

    // Remove the order
    auto &orders = side.get_orders_at_price(100.0);
    orders.clear();
    side.remove_price_level(100.0);

    EXPECT_FALSE(side.best_price().has_value());
}

TEST(OrderBookSideTest, RemoveOrderRemovesCorrectly)
{
    BidBookSide buy_side;

    std::vector<Order> orders = {
        TestOrderFactory::CreateBuy(1, 100.0, 10),
        TestOrderFactory::CreateBuy(2, 100.0, 5),
        TestOrderFactory::CreateBuy(3, 100.0, 20)};

    for (const auto &o : orders)
        buy_side.add_order(o);

    auto best_price = buy_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0);

    auto &best_orders = buy_side.get_orders_at_price(*best_price);
    EXPECT_EQ(best_orders.size(), 3);

    // Remove order with id 2
    auto it = std::find_if(best_orders.begin(), best_orders.end(), [](const Order &o)
                           { return o.id == 2; });
    if (it != best_orders.end())
        best_orders.erase(it);

    // Check remaining orders
    EXPECT_EQ(best_orders.size(), 2);
    for (const auto &o : best_orders)
        EXPECT_NE(o.id, 2);
}

TEST(OrderBookSideTest, AddAndRetrieveBestPriceBuySide)
{
    BidBookSide buy_side;

    std::vector<Order> orders = {
        TestOrderFactory::CreateBuy(1, 100.0, 10),
        TestOrderFactory::CreateBuy(2, 101.0, 5),
        TestOrderFactory::CreateBuy(3, 99.0, 20)};

    for (const auto &o : orders)
        buy_side.add_order(o);

    auto best_price = buy_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 101.0);

    auto &best_orders = buy_side.get_orders_at_price(*best_price);
    ASSERT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 2);
}

TEST(OrderBookSideTest, AddAndRetrieveBestPriceSellSide)
{
    AskBookSide sell_side;

    std::vector<Order> orders = {
        TestOrderFactory::CreateSell(1, 100.0, 10),
        TestOrderFactory::CreateSell(2, 101.0, 5),
        TestOrderFactory::CreateSell(3, 99.0, 20)};

    for (const auto &o : orders)
        sell_side.add_order(o);

    auto best_price = sell_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 99.0);

    auto &best_orders = sell_side.get_orders_at_price(*best_price);
    ASSERT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 3);
}

TEST(OrderBookSideTest, MultiplePriceLevelsBestPrice)
{
    BidBookSide buy_side;
    buy_side.add_order(TestOrderFactory::CreateBuy(1, 100.0, 10));
    buy_side.add_order(TestOrderFactory::CreateBuy(2, 105.0, 5));
    buy_side.add_order(TestOrderFactory::CreateBuy(3, 102.0, 20));

    auto best_price = buy_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 105.0);

    AskBookSide sell_side;
    sell_side.add_order(TestOrderFactory::CreateSell(4, 200.0, 10));
    sell_side.add_order(TestOrderFactory::CreateSell(5, 195.0, 5));
    sell_side.add_order(TestOrderFactory::CreateSell(6, 198.0, 20));

    best_price = sell_side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 195.0);
}

TEST(OrderBookSideTest, BestPriceAndOrdersEmptySide)
{
    AskBookSide side;

    EXPECT_FALSE(side.best_price().has_value());
}

TEST(OrderBookSideTest, AddOrdersWithDuplicateIDs)
{
    BidBookSide side;

    side.add_order(TestOrderFactory::CreateBuy(1, 100.0, 10));
    side.add_order(TestOrderFactory::CreateBuy(1, 101.0, 5));

    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 101.0);

    auto &best_orders = side.get_orders_at_price(*best_price);
    EXPECT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 1);

    // Remove order at 101
    best_orders.clear();
    side.remove_price_level(*best_price);

    best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    auto &orders_at_100 = side.get_orders_at_price(*best_price);
    EXPECT_EQ(orders_at_100.size(), 1);
    EXPECT_EQ(orders_at_100.front().id, 1);
}

TEST(OrderBookSideTest, LargeNumberOfOrders)
{
    AskBookSide side;

    constexpr int N = 100000;
    for (int i = 0; i < N; ++i)
        side.add_order(TestOrderFactory::CreateSell(i, 100.0 + i * 0.01, 1));

    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0);

    // Remove half of the orders
    for (int i = 0; i < N / 2; ++i)
    {
        auto &orders = side.get_orders_at_price(100.0 + i * 0.01);
        orders.clear();
        side.remove_price_level(100.0 + i * 0.01);
    }

    best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_DOUBLE_EQ(best_price.value(), 100.0 + (N / 2) * 0.01);
}

TEST(OrderBookSideTest, FloatingPointPricePrecision)
{
    BidBookSide side;

    side.add_order(TestOrderFactory::CreateBuy(1, 100.000001, 10));
    side.add_order(TestOrderFactory::CreateBuy(2, 100.000002, 20));
    side.add_order(TestOrderFactory::CreateBuy(3, 100.000000, 30));

    auto best_price = side.best_price();
    ASSERT_TRUE(best_price.has_value());
    EXPECT_NEAR(best_price.value(), 100.000002, 1e-9);

    auto &best_orders = side.get_orders_at_price(*best_price);
    EXPECT_EQ(best_orders.size(), 1);
    EXPECT_EQ(best_orders.front().id, 2);
}
