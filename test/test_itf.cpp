#include <gtest/gtest.h>
#include "itf/itf.hpp"

using namespace nb;

class TestITF : public ITF {
public:
    TestITF() : ITF("TestInterface", Version(1,0,0), Version(1,0,0)) {
        info_ = "Test Interface";
    }
};

class TestITF2 : public ITF {
public:
    TestITF2() : ITF("TestInterface2", Version(3,0,0), Version(2,0,0)) {
        info_ = "Test Interface2";
    }
};

class TestITF3 : public TestITF, public TestITF2 {
public:
    TestITF3() {}
};

TEST(ITFTest, BasicValidity) {
    TestITF itf;
    EXPECT_TRUE(itf.valid());
}

TEST(ITFTest, NameAndInfo) {
    TestITF itf;
    EXPECT_EQ(itf.name(), "TestInterface");
    EXPECT_EQ(itf.info(), "Test Interface");
}

TEST(ITFTest, VersionChecks) {
    TestITF itf;
    EXPECT_EQ(itf.version(), Version(1,0,0));
    EXPECT_EQ(itf.minVersion(), Version(1,0,0));
    EXPECT_TRUE(itf.compatible(Version(1,0,0)));
    EXPECT_TRUE(itf.compatible(Version(1,1,0)));
    EXPECT_FALSE(itf.compatible(Version(0,9,9)));
}

TEST(ITFTest, MoveAndCopySemantics) {
    TestITF original;
    // Verify copy operations are deleted
    EXPECT_FALSE(std::is_copy_constructible<ITF>::value);
    EXPECT_FALSE(std::is_copy_assignable<ITF>::value);
    // Verify move operations are deleted
    EXPECT_FALSE(std::is_move_constructible<ITF>::value);
    EXPECT_FALSE(std::is_move_assignable<ITF>::value);
}

TEST(ITFTest, VersionEdgeCases) {
    TestITF itf;
    // Test exact version match
    EXPECT_TRUE(itf.compatible(Version(1,0,0)));
    // Test minor version above
    EXPECT_TRUE(itf.compatible(Version(1,0,1)));
    // Test major version below
    EXPECT_FALSE(itf.compatible(Version(0,9,9)));
    // Test major version above
    EXPECT_TRUE(itf.compatible(Version(2,0,0)));
}

TEST(ITFTest, ValidityAfterModification) {
    struct Test{
        int a = 0;
        int b = 0;
    };
    Test t;
    auto itf =(ITF *)&t;
    EXPECT_FALSE(itf->valid());
}

TEST(ITFTest, DerivedClassBehavior) {
    class DerivedITF : public ITF {
    public:
        DerivedITF() : ITF("Derived", Version(2,0,0), Version(1,5,0)) {}
    };

    DerivedITF derived;
    EXPECT_TRUE(derived.valid());
    EXPECT_EQ(derived.name(), "Derived");
    EXPECT_TRUE(derived.compatible(Version(1,5,0)));
    EXPECT_FALSE(derived.compatible(Version(1,4,9)));
}

TEST(ITFTest, Implement2ITF) {
    TestITF3 itf;
    auto i = (TestITF2*)&itf;
    EXPECT_TRUE(i->valid());
    EXPECT_EQ(i->name(), "TestInterface2");
    EXPECT_EQ(i->info(), "Test Interface2");
    EXPECT_TRUE(i->compatible(Version(2,0,0)));

    auto i2 = (TestITF*)&itf;
    EXPECT_TRUE(i2->valid());
    EXPECT_EQ(i2->name(), "TestInterface");
    EXPECT_EQ(i2->info(), "Test Interface");

    auto i3 = (ITF*)i;
    EXPECT_TRUE(i3->valid());
    auto p = i3->obj<TestITF2>();
    EXPECT_TRUE(p);
    auto p2 = i3->obj<TestITF>();
    EXPECT_TRUE(p2);
    EXPECT_EQ(p->name(), "TestInterface2");
    EXPECT_EQ(p->info(), "Test Interface2");
    EXPECT_EQ(p2->name(), "TestInterface");
    EXPECT_EQ(p2->info(), "Test Interface");

    auto pp = i3->obj<TestITF2>(Version(3,0,0));
    EXPECT_TRUE(pp);
    EXPECT_EQ(pp->name(), "TestInterface2");
    EXPECT_EQ(pp->info(), "Test Interface2");
    EXPECT_FALSE(i3->obj<TestITF>(Version(0,1,0)));
}

