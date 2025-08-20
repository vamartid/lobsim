#include <gtest/gtest.h>
#include "engine/side/OrderBookSide.h"
#include "engine/match/PriceTimePriorityStrategy.h"
#include "test_utils/OrderFactory.h"

// 1. Fills in FIFO (already done)

// 2. Partial fill only
TEST(PriceTimePriorityStrategyTest, PartialFill)
{
    AskBookSide sell_side;
    std::vector<Order> sell_orders = {
        TestOrderFactory::CreateSell(1, 100.0, 5)};

    for (auto &o : sell_orders)
        sell_side.add_order(o);

    Order incoming = TestOrderFactory::CreateBuy(99, 100.0, 10);

    PriceTimePriorityStrategy strat;
    std::vector<FillOp> fills;

    const IOrderBookSideView &view = sell_side;
    MatchResult result = strat.match(incoming, view, fills);

    ASSERT_EQ(fills.size(), 1);
    EXPECT_EQ(fills[0].quantity, 5); // partially fills
    EXPECT_EQ(incoming.quantity, 5); // remaining qty
    EXPECT_EQ(fills[0].price, 100);
    EXPECT_EQ(result.filledQty, 5);
}

// 3. No fill (price too low)
TEST(PriceTimePriorityStrategyTest, NoFillPriceTooLow)
{
    AskBookSide sell_side;
    sell_side.add_order(TestOrderFactory::CreateSell(1, 101.0, 10));

    Order incoming = TestOrderFactory::CreateBuy(99, 100.0, 10);

    PriceTimePriorityStrategy strat;
    std::vector<FillOp> fills;

    const IOrderBookSideView &view = sell_side;
    MatchResult result = strat.match(incoming, view, fills);

    EXPECT_TRUE(fills.empty());
    EXPECT_EQ(incoming.quantity, 10);
    EXPECT_EQ(result.filledQty, 0);
}

// 4. Exact match
TEST(PriceTimePriorityStrategyTest, ExactMatch)
{
    AskBookSide sell_side;
    sell_side.add_order(TestOrderFactory::CreateSell(1, 100.0, 10));

    Order incoming = TestOrderFactory::CreateBuy(99, 100.0, 10);

    PriceTimePriorityStrategy strat;
    std::vector<FillOp> fills;

    const IOrderBookSideView &view = sell_side;
    MatchResult result = strat.match(incoming, view, fills);

    ASSERT_EQ(fills.size(), 1);
    EXPECT_EQ(fills[0].quantity, 10);
    EXPECT_EQ(incoming.quantity, 0);
    EXPECT_EQ(fills[0].price, 100);
    EXPECT_EQ(result.filledQty, 10);
}

// 5. Fill across multiple price levels
TEST(PriceTimePriorityStrategyTest, FillsMultiplePriceLevels)
{
    AskBookSide sell_side;
    std::vector<Order> sell_orders = {
        TestOrderFactory::CreateSell(1, 100.0, 5),
        TestOrderFactory::CreateSell(2, 101.0, 10)};
    for (auto &o : sell_orders)
        sell_side.add_order(o);

    Order incoming = TestOrderFactory::CreateBuy(99, 101.0, 12);

    PriceTimePriorityStrategy strat;
    std::vector<FillOp> fills;

    const IOrderBookSideView &view = sell_side;
    MatchResult result = strat.match(incoming, view, fills);

    ASSERT_EQ(fills.size(), 2);
    EXPECT_EQ(fills[0].quantity, 5); // filled at 100
    EXPECT_EQ(fills[1].quantity, 7); // partially filled at 101
    EXPECT_EQ(incoming.quantity, 0);
    EXPECT_EQ(fills[0].price, 100);
    EXPECT_EQ(fills[1].price, 101);
    EXPECT_EQ(result.filledQty, 12);
}

// 6. Incoming order smaller than top of book
TEST(PriceTimePriorityStrategyTest, IncomingSmallerThanTopOfBook)
{
    AskBookSide sell_side;
    sell_side.add_order(TestOrderFactory::CreateSell(1, 100.0, 10));

    Order incoming = TestOrderFactory::CreateBuy(99, 100.0, 3);

    PriceTimePriorityStrategy strat;
    std::vector<FillOp> fills;

    const IOrderBookSideView &view = sell_side;
    MatchResult result = strat.match(incoming, view, fills);

    ASSERT_EQ(fills.size(), 1);
    EXPECT_EQ(fills[0].quantity, 3);
    EXPECT_EQ(incoming.quantity, 0);
    EXPECT_EQ(fills[0].price, 100);
    EXPECT_EQ(result.filledQty, 3);
}
