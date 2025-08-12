#include <gtest/gtest.h>
#include "core/Order.h"

TEST(OrderTest, FieldAssignment)
{
    Order order = {1, 123456789u, 100.25, 50, Side::Buy};

    EXPECT_EQ(order.id, 1);
    EXPECT_EQ(order.side, Side::Buy);
    EXPECT_DOUBLE_EQ(order.price, 100.25);
    EXPECT_EQ(order.quantity, 50u);
    EXPECT_EQ(order.timestamp_ns, 123456789u);
}

TEST(OrderTest, ToStringBuy)
{
    Order order = {42, 987654321, 99.99, 100, Side::Buy};
    std::string result = order.to_string();

    EXPECT_NE(result.find("BUY"), std::string::npos);
    EXPECT_NE(result.find("Price: 99.99"), std::string::npos);
    EXPECT_NE(result.find("Qty: 100"), std::string::npos);
    EXPECT_NE(result.find("ID: 42"), std::string::npos);
    EXPECT_NE(result.find("Time: 987654321"), std::string::npos);
    EXPECT_NE(result.find("\033[0;32m"), std::string::npos); // Green for BUY
}

TEST(OrderTest, ToStringSell)
{
    Order order = {99, 111222333, 55.50, 10, Side::Sell};
    std::string result = order.to_string();

    EXPECT_NE(result.find("SELL"), std::string::npos);
    EXPECT_NE(result.find("Price: 55.50"), std::string::npos);
    EXPECT_NE(result.find("Qty: 10"), std::string::npos);
    EXPECT_NE(result.find("ID: 99"), std::string::npos);
    EXPECT_NE(result.find("Time: 111222333"), std::string::npos);
    EXPECT_NE(result.find("\033[0;31m"), std::string::npos); // Red for SELL
}
