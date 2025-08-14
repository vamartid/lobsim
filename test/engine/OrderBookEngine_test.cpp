#include <gtest/gtest.h>

#include "engine/OrderBookEngine.h"
#include "test_utils/OrderFactory.h"
#include "utils/log/StreamUtils.h"
#include <ostream>
#include <iostream>

class OrderBookEngineTest : public ::testing::Test
{
protected:
    OrderBookEngine engine;
};

TEST_F(OrderBookEngineTest, AddOrderAddsToCorrectSide)
{
    engine.add_order(TestOrderFactory::CreateBuy(1, 100.0, 10));
    engine.add_order(TestOrderFactory::CreateSell(2, 101.0, 5));

    // Check best prices
    auto best_bid = engine.bids().best_price();
    auto best_ask = engine.asks().best_price();

    ASSERT_TRUE(best_bid.has_value());
    EXPECT_DOUBLE_EQ(best_bid.value(), 100.0);

    ASSERT_TRUE(best_ask.has_value());
    EXPECT_DOUBLE_EQ(best_ask.value(), 101.0);
}

TEST_F(OrderBookEngineTest, CancelOrderRemovesOrder)
{
    engine.add_order(TestOrderFactory::CreateBuy(1, 100.0, 10));

    ASSERT_TRUE(engine.bids().best_price().has_value());

    engine.cancel_order(1);

    EXPECT_FALSE(engine.bids().best_price().has_value());
}

TEST_F(OrderBookEngineTest, MatchOrderPrintsMatch)
{
    // Arrange
    engine.add_order(TestOrderFactory::CreateSell(1, 100.0, 10));

    // Act
    // Capture output to a stream
    std::ostringstream captured_os;

    utils::stream::capture_output(std::cout, captured_os, [&]()
                                  { engine.match_order(TestOrderFactory::CreateBuy(2, 100.0, 5)); });
    // Strip ANSI codes directly from the captured stream's buffer
    std::istringstream captured_is(captured_os.str());
    std::ostringstream clean_os;
    utils::stream::strip_ansi(clean_os, captured_is);

    // Search in the cleaned stream without creating a final string copy
    std::string line;
    bool found_match = false, found_buy = false, found_sell = false, found_qty = false, found_price = false;
    std::cout << clean_os.str() << '\n';
    std::istringstream clean_is(clean_os.str());
    while (std::getline(clean_is, line))
    {
        found_match |= line.find("Match Detail:") != std::string::npos;
        found_buy |= line.find("BUY order (ID: 2)") != std::string::npos;
        found_sell |= line.find("SELL order (ID: 1)") != std::string::npos;
        found_qty |= line.find("for 5 units") != std::string::npos;
        found_price |= line.find("at price 100") != std::string::npos;
    }

    // Assert
    EXPECT_TRUE(found_match);
    EXPECT_TRUE(found_buy);
    EXPECT_TRUE(found_sell);
    EXPECT_TRUE(found_qty);
    EXPECT_TRUE(found_price);
}

TEST_F(OrderBookEngineTest, NoMatchIfPricesDontCross)
{
    engine.add_order(TestOrderFactory::CreateSell(1, 102.0, 10));
    std::ostringstream captured_os;
    utils::stream::capture_output(std::cout, captured_os, [&]()
                                  { engine.match_order(TestOrderFactory::CreateBuy(2, 100.0, 5)); });
    // Check that nothing was printed
    EXPECT_TRUE(captured_os.str().empty());
}
