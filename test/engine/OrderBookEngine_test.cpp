#include <gtest/gtest.h>

#include "engine/OrderBookEngine.h"

#include <sstream>
#include <iostream>

class OrderBookEngineTest : public ::testing::Test
{
protected:
    OrderBookEngine engine;

    // Helper to capture std::cout output
    std::string capture_output(std::function<void()> func)
    {
        std::stringstream buffer;
        std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());
        func();
        std::cout.rdbuf(old); // restore original buffer
        return buffer.str();
    }
};

TEST_F(OrderBookEngineTest, AddOrderAddsToCorrectSide)
{
    Order buy_order{1, 0, 100.0, 10, Side::Buy};
    Order sell_order{2, 0, 101.0, 5, Side::Sell};

    engine.add_order(buy_order);
    engine.add_order(sell_order);

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
    Order buy_order{1, 0, 100.0, 10, Side::Buy};
    engine.add_order(buy_order);

    ASSERT_TRUE(engine.bids().best_price().has_value());

    engine.cancel_order(1);

    EXPECT_FALSE(engine.bids().best_price().has_value());
}

TEST_F(OrderBookEngineTest, MatchOrderPrintsMatch)
{
    // Add sell order first
    Order sell_order{1, 0, 100.0, 10, Side::Sell};
    engine.add_order(sell_order);

    // Incoming buy order matches the sell order price
    Order buy_order{2, 0, 100.0, 5, Side::Buy};

    std::string output = capture_output([&]()
                                        { engine.match_order(buy_order); });

    std::string clean_output = utils::string::strip_ansi(output);
    EXPECT_NE(clean_output.find("Match Detail:"), std::string::npos);
    EXPECT_NE(clean_output.find("BUY order (ID: 2)"), std::string::npos);
    EXPECT_NE(clean_output.find("SELL order (ID: 1)"), std::string::npos);
    EXPECT_NE(clean_output.find("for 5 units"), std::string::npos);
    EXPECT_NE(clean_output.find("at price 100.00"), std::string::npos);
}

TEST_F(OrderBookEngineTest, NoMatchIfPricesDontCross)
{
    // Add sell order at price 102
    Order sell_order{1, 0, 102.0, 10, Side::Sell};
    engine.add_order(sell_order);

    // Incoming buy order at lower price 100 - no match expected
    Order buy_order{2, 0, 100.0, 5, Side::Buy};

    std::string output = capture_output([&]()
                                        { engine.add_order(buy_order); });

    EXPECT_TRUE(output.empty());
}
