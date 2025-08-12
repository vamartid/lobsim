#include <gtest/gtest.h>

#include "utils/random/RealRNG.h"
#include "test_utils/CheckHelpers.h"

TEST(RealRNGTest, DeterministicWithSeed)
{
    unsigned seed = 42;
    RealRNG rng1(seed);
    RealRNG rng2(seed);

    std::vector<double> seq1;
    std::vector<double> seq2;

    for (int i = 0; i < 10; ++i)
    {
        seq1.push_back(rng1.uniform_real(0.0, 1.0));
        seq2.push_back(rng2.uniform_real(0.0, 1.0));
    }

    EXPECT_EQ(seq1, seq2) << "Same seed should produce identical sequences";
}

TEST(RealRNGTest, NonDeterministicWithoutSeed)
{
    RealRNG rng1; // no seed
    RealRNG rng2; // no seed

    std::vector<double> seq1;
    std::vector<double> seq2;

    for (int i = 0; i < 10; ++i)
    {
        seq1.push_back(rng1.uniform_real(0.0, 1.0));
        seq2.push_back(rng2.uniform_real(0.0, 1.0));
    }

    // Very small chance of being equal, so this should almost always pass
    EXPECT_NE(seq1, seq2) << "Different seeds should produce different sequences";
}

TEST(RealRNGTest, UniformRealWithinRange)
{
    RealRNG rng(123);

    std::vector<double> values;
    for (int i = 0; i < 100; ++i)
    {
        values.push_back(rng.uniform_real(-5.0, 5.0));
    }

    EXPECT_TRUE(all_in_range<double>(values, -5.0, 5.0));
}

TEST(RealRNGTest, UniformIntWithinRange)
{
    RealRNG rng(456);

    std::vector<int> values;
    for (int i = 0; i < 100; ++i)
    {
        values.push_back(rng.uniform_int(10, 20));
    }

    EXPECT_TRUE(all_in_range<int>(values, 10, 20));
}
