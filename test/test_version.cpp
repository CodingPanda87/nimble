#include <gtest/gtest.h>
#include "core/version.hpp"

namespace nb {

class VersionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(VersionTest, DefaultConstructor) {
    Version v;
    EXPECT_FALSE(v.valid());
}

TEST_F(VersionTest, ParameterizedConstructor) {
    Version v(1, 2, 3);
    EXPECT_TRUE(v.valid());
}

TEST_F(VersionTest, IsValid) {
    Version v1;
    EXPECT_FALSE(v1.valid());

    Version v2(1, 0, 0);
    EXPECT_TRUE(v2.valid());

    Version v3(0, 1, 0);
    EXPECT_TRUE(v3.valid());
}

TEST_F(VersionTest, EqualityOperators) {
    Version v1(1, 2, 3);
    Version v2(1, 2, 3);
    Version v3(1, 2, 4);

    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
    EXPECT_FALSE(v1 != v2);
    EXPECT_TRUE(v1 != v3);
}

TEST_F(VersionTest, ComparisonOperators) {
    Version v1(1, 2, 3);
    Version v2(1, 2, 4);
    Version v3(1, 3, 0);
    Version v4(2, 0, 0);

    EXPECT_TRUE(v1 < v2);
    EXPECT_TRUE(v1 < v3);
    EXPECT_TRUE(v1 < v4);
    EXPECT_TRUE(v1 <= v1);
    EXPECT_TRUE(v1 <= v2);
    EXPECT_TRUE(v2 > v1);
    EXPECT_TRUE(v3 > v1);
    EXPECT_TRUE(v4 > v1);
    EXPECT_TRUE(v1 >= v1);
    EXPECT_TRUE(v2 >= v1);
}

TEST_F(VersionTest, ToString) {
    Version v(1, 2, 3);
    EXPECT_EQ(v.toString(), "1.2.3");
}

TEST_F(VersionTest, FromString) {
    auto v1 = Version::fromString("1.2.3");
    EXPECT_TRUE(v1.valid());
    EXPECT_EQ(v1.toString(), "1.2.3");
    EXPECT_FALSE(v1 == Version());
    EXPECT_TRUE(v1 == Version(1,2,3));
    auto v2 = Version::fromString("invalid");
    EXPECT_FALSE(v2.valid());
}

TEST_F(VersionTest, CompileTimeVersion) {
    constexpr Version v = {1, 2, 3};
    static_assert(v == Version(1, 2, 3));
}

} // namespace nb
