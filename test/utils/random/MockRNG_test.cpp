#include <gtest/gtest.h>

#include "utils/random/MockRNG.h"

TEST(MockRNGTest, ReturnsValuesInOrder)
{
    std::vector<double> values = {1.5, 2.5, 3.5};
    MockRNG rng(values);

    EXPECT_DOUBLE_EQ(rng.uniform_real(0.0, 10.0), 1.5);
    EXPECT_DOUBLE_EQ(rng.uniform_real(-5.0, 5.0), 2.5);
    EXPECT_DOUBLE_EQ(rng.uniform_real(100.0, 200.0), 3.5);
}

TEST(MockRNGTest, UniformIntCastsCorrectly)
{
    std::vector<double> values = {42.0, 99.9, -3.1};
    MockRNG rng(values);

    EXPECT_EQ(rng.uniform_int(0, 100), 42); // 42.0 → 42
    EXPECT_EQ(rng.uniform_int(0, 100), 99); // 99.9 → 99
    EXPECT_EQ(rng.uniform_int(0, 100), -3); // -3.1 → -3
}

TEST(MockRNGTest, WrapsAroundWhenOutOfValues)
{
    std::vector<double> values = {7.0, 8.0};
    MockRNG rng(values);

    EXPECT_DOUBLE_EQ(rng.uniform_real(0.0, 1.0), 7.0);
    EXPECT_DOUBLE_EQ(rng.uniform_real(0.0, 1.0), 8.0);
    EXPECT_DOUBLE_EQ(rng.uniform_real(0.0, 1.0), 7.0); // Loops back
    EXPECT_DOUBLE_EQ(rng.uniform_real(0.0, 1.0), 8.0);
}

TEST(MockRNGTest, IgnoresMinMaxParameters)
{
    std::vector<double> values = {123.45};
    MockRNG rng(values);

    // Regardless of min/max, we get stored value
    EXPECT_DOUBLE_EQ(rng.uniform_real(-1000.0, -500.0), 123.45);
    EXPECT_EQ(rng.uniform_int(1000, 2000), 123);
}
