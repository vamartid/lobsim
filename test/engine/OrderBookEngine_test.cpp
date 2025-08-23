#include <gtest/gtest.h>
#include "engine/OrderBookEngine.h"
#include "engine/match/PriceTimePriorityStrategy.h"
#include "test_utils/OrderFactory.h"
#include "engine/events/EventBus.h"
#include "utils/log/Logger.h"

class OrderBookEngineTest : public ::testing::Test
{
protected:
    OrderBookEngineTest()
        : engine(bus, std::make_unique<PriceTimePriorityStrategy>())
    {
        bus.add_listener([&](const Event &e)
                         { logger.on_event(e); }, Backpressure::Drop);
    }
    Logger logger;
    EventBus bus; // dummy bus for tests
    OrderBookEngine engine;
};

// --- Basic add/cancel tests ---

TEST_F(OrderBookEngineTest, AddOrderAddsToCorrectSide)
{
    auto buy = TestOrderFactory::CreateBuy(1, 100.0, 10);
    auto sell = TestOrderFactory::CreateSell(2, 101.0, 5);

    engine.add_order(buy);
    engine.add_order(sell);

    auto best_bid = engine.bids().best_price();
    auto best_ask = engine.asks().best_price();

    ASSERT_TRUE(best_bid.has_value());
    EXPECT_DOUBLE_EQ(best_bid.value(), 100.0);

    ASSERT_TRUE(best_ask.has_value());
    EXPECT_DOUBLE_EQ(best_ask.value(), 101.0);
}

TEST_F(OrderBookEngineTest, CancelOrderRemovesOrder)
{
    auto buy = TestOrderFactory::CreateBuy(1, 100.0, 10);
    engine.add_order(buy);

    ASSERT_TRUE(engine.bids().best_price().has_value());

    engine.cancel_order(1);
    EXPECT_FALSE(engine.bids().best_price().has_value());
}

// --- Matching tests (PriceTimePriorityStrategy) ---

TEST_F(OrderBookEngineTest, BuyMatchesSellFIFO)
{
    // Arrange sells
    auto sells = {
        TestOrderFactory::CreateSell(1, 100.0, 10),
        TestOrderFactory::CreateSell(2, 100.0, 5),
        TestOrderFactory::CreateSell(3, 101.0, 20)};
    for (auto sell : sells)
        engine.add_order(sell);

    // Incoming buy
    auto incoming = TestOrderFactory::CreateBuy(99, 100.0, 12);
    engine.add_order(incoming);

    // Assert resting book
    auto best_ask = engine.asks().best_price();
    ASSERT_TRUE(best_ask.has_value());
    EXPECT_DOUBLE_EQ(best_ask.value(), 100.0); // partially filled sell #2

    // Check quantities
    // Sell #1 gone, Sell #2 partially
    EXPECT_EQ(engine.asks().get_orders_at_price(100.0).front().quantity, 3);
}

TEST_F(OrderBookEngineTest, NoMatchIfPricesDontCross)
{
    auto sell = TestOrderFactory::CreateSell(1, 102.0, 10);
    engine.add_order(sell);

    auto incoming = TestOrderFactory::CreateBuy(2, 100.0, 5);
    engine.add_order(incoming);

    // Incoming stays on bid side
    auto best_bid = engine.bids().best_price();
    ASSERT_TRUE(best_bid.has_value());
    EXPECT_DOUBLE_EQ(best_bid.value(), 100.0);

    // Sell untouched
    auto best_ask = engine.asks().best_price();
    ASSERT_TRUE(best_ask.has_value());
    EXPECT_DOUBLE_EQ(best_ask.value(), 102.0);
}

TEST_F(OrderBookEngineTest, BuyPartialFill)
{
    // Resting sell order larger than incoming buy
    auto sell = TestOrderFactory::CreateSell(1, 100.0, 10);
    engine.add_order(sell);

    // Incoming buy partially fills the resting sell
    auto incoming = TestOrderFactory::CreateBuy(2, 100.0, 5);
    engine.add_order(incoming);

    // Book state: resting sell partially filled
    auto best_ask = engine.asks().best_price();
    ASSERT_TRUE(best_ask.has_value());
    EXPECT_DOUBLE_EQ(best_ask.value(), 100.0);

    // Remaining quantity of resting sell
    EXPECT_EQ(engine.asks().get_orders_at_price(100.0).front().quantity, 5);

    // No buy orders remain in the book
    EXPECT_FALSE(engine.bids().best_price().has_value());
}

TEST_F(OrderBookEngineTest, BuyMatchesMultiplePriceLevels)
{
    auto sells = {TestOrderFactory::CreateSell(1, 100.0, 5), TestOrderFactory::CreateSell(2, 101.0, 10)};
    for (auto sell : sells)
        engine.add_order(sell);
    auto incoming = TestOrderFactory::CreateBuy(3, 101.0, 12);
    engine.add_order(incoming);

    // After: both sells reduced/removed
    EXPECT_TRUE(engine.asks().get_orders_at_price(100.0).empty());           // fully consumed
    EXPECT_EQ(engine.asks().get_orders_at_price(101.0).front().quantity, 3); // partial
}

TEST_F(OrderBookEngineTest, SellMatchesBuyFIFO)
{
    auto buys = {TestOrderFactory::CreateBuy(1, 101.0, 10), TestOrderFactory::CreateBuy(2, 100.0, 5)};
    for (auto buy : buys)
        engine.add_order(buy);
    auto incoming = TestOrderFactory::CreateSell(3, 100.0, 12);
    engine.add_order(incoming);

    // Buy #1 fully filled
    EXPECT_TRUE(engine.bids().get_orders_at_price(101.0).empty());
    // Buy #2 partially filled
    EXPECT_EQ(engine.bids().get_orders_at_price(100.0).front().quantity, 3);
}
