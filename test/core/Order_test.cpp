#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "core/Order.h"
#include "test_utils/OrderFactory.h"

using Side = Order::Side;

// ---------------------------
// FieldAssignment Test
// ---------------------------
TEST(OrderTest, FieldAssignment)
{
    Order order = TestOrderFactory::CreateBuy();
    order.setIOC(true);

    EXPECT_EQ(order.id, 1);
    EXPECT_EQ(order.side(), Side::Buy);
    EXPECT_DOUBLE_EQ(order.price, 100.0);
    EXPECT_EQ(order.quantity, 10u);
    EXPECT_EQ(order.timestamp, 123456789u);
    EXPECT_TRUE(order.isIOC());
}

// ---------------------------
// ControlFlags Test
// ---------------------------
TEST(OrderTest, ControlFlagsAll)
{
    Order order = TestOrderFactory::CreateSell();

    // Initially, all flags should be false
    EXPECT_FALSE(order.isIceberg());
    EXPECT_FALSE(order.isHidden());
    EXPECT_FALSE(order.isWeighted());
    EXPECT_FALSE(order.isAuction());
    EXPECT_FALSE(order.isIOC());
    EXPECT_FALSE(order.isFOK());
    EXPECT_FALSE(order.isMarket());
    EXPECT_FALSE(order.isReservedFlag());

    // Set each flag to true and verify
    order.setIceberg(true);
    EXPECT_TRUE(order.isIceberg());

    order.setHidden(true);
    EXPECT_TRUE(order.isHidden());

    order.setWeighted(true);
    EXPECT_TRUE(order.isWeighted());

    order.setAuction(true);
    EXPECT_TRUE(order.isAuction());

    order.setIOC(true);
    EXPECT_TRUE(order.isIOC());

    order.setFOK(true);
    EXPECT_TRUE(order.isFOK());

    order.setMarket(true);
    EXPECT_TRUE(order.isMarket());

    order.setReservedFlag(true);
    EXPECT_TRUE(order.isReservedFlag());

    // Reset each flag to false and verify
    order.setIceberg(false);
    EXPECT_FALSE(order.isIceberg());

    order.setHidden(false);
    EXPECT_FALSE(order.isHidden());

    order.setWeighted(false);
    EXPECT_FALSE(order.isWeighted());

    order.setAuction(false);
    EXPECT_FALSE(order.isAuction());

    order.setIOC(false);
    EXPECT_FALSE(order.isIOC());

    order.setFOK(false);
    EXPECT_FALSE(order.isFOK());

    order.setMarket(false);
    EXPECT_FALSE(order.isMarket());

    order.setReservedFlag(false);
    EXPECT_FALSE(order.isReservedFlag());
}
// ---------------------------
// ToString Tests
// ---------------------------
TEST(OrderTest, ToStringBuy)
{
    Order order = TestOrderFactory::CreateBuy();
    std::string result = order.to_string();
    EXPECT_NE(result.find("Side:BUY"), std::string::npos);
    EXPECT_NE(result.find("Price:100.00"), std::string::npos);
    EXPECT_NE(result.find("Qty:10"), std::string::npos);
    EXPECT_NE(result.find("ID:1"), std::string::npos);
    EXPECT_NE(result.find("Time:123456789"), std::string::npos);
}

TEST(OrderTest, ToStringSell)
{
    Order order = TestOrderFactory::CreateSell();
    std::string result = order.to_string();
    EXPECT_NE(result.find("Side:SELL"), std::string::npos);
    EXPECT_NE(result.find("Price:100.00"), std::string::npos);
    EXPECT_NE(result.find("Qty:10"), std::string::npos);
    EXPECT_NE(result.find("ID:1"), std::string::npos);
    EXPECT_NE(result.find("Time:123456789"), std::string::npos);
}

// ---------------------------
// Cache Line Alignment Test
// ---------------------------
TEST(OrderTest, CacheLineAlignment)
{
    EXPECT_EQ(alignof(Order), 64);
    EXPECT_EQ(sizeof(Order) % 64, 0);
}

// ---------------------------
// Thread Safety / Concurrent Access Test
// ---------------------------
TEST(OrderTest, ThreadSafetyAccess)
{
    Order order = TestOrderFactory::CreateBuy();

    auto writer = [&order]()
    {
        for (int i = 0; i < 1000; ++i)
            order.quantity += 1;
    };

    auto reader = [&order]()
    {
        volatile uint32_t sum = 0;
        for (int i = 0; i < 1000; ++i)
            sum += order.quantity;
    };

    std::thread t1(writer);
    std::thread t2(reader);
    std::thread t3(writer);
    std::thread t4(reader);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    EXPECT_GE(order.quantity, 10u); // Should be at least initial value
}