#include "gtest/gtest.h"
#include "itf/i_mempool.hpp"

using namespace nb;

TEST(BufferTest, Construction) {
    Buffer b1;
    EXPECT_FALSE(b1);
    EXPECT_EQ(b1.size(), 0);
    EXPECT_EQ(b1.get(), nullptr);

    x::u32 size = 100;
    x::str info = "test";
    Buffer b2(size, info);
    EXPECT_TRUE(b2);
    EXPECT_EQ(b2.size(), size);
    EXPECT_NE(b2.get(), nullptr);
}

TEST(BufferTest, MoveOperations) {
    x::u32 size = 100;
    x::str info = "test";
    Buffer b1(size, info);
    
    Buffer b2(std::move(b1));
    EXPECT_TRUE(b2);
    EXPECT_FALSE(b1);
    
    Buffer b3;
    b3 = std::move(b2);
    EXPECT_TRUE(b3);
    EXPECT_FALSE(b2);
}

TEST(BufferTest, Input) {
    x::u32 size = 100;
    x::str info = "test";
    Buffer b(size, info);
    
    std::string testData = "hello world";
    EXPECT_TRUE(b.input(testData.c_str(), testData.size()));
    EXPECT_EQ(memcmp(b.get(), testData.c_str(), testData.size()), 0);
    
    // Test invalid input
    EXPECT_FALSE(b.input(nullptr, 10));
    EXPECT_FALSE(b.input("data", size + 1));
}

class MockMemPool : public I_MemPool {
public:
    BufferPtr get(size_t size) noexcept override {
        if(size == 0)
            return nullptr;
        auto buf = std::shared_ptr<Buffer>(new Buffer((x::u32)size, "test"));
        return buf;
    }
    
    void giveback(const BufferPtr& b) noexcept override {
        // No-op for testing
    }
    
    size_t size() const noexcept override {
        return 0;
    }
};

class MemPoolTest_I : public ::testing::Test {
protected:
    MockMemPool pool_;
};

TEST_F(MemPoolTest_I, Interface) {
    // Test get()
    auto buf1 = pool_.get(100);
    EXPECT_TRUE(buf1);
    EXPECT_TRUE(*buf1);
    EXPECT_EQ(buf1->size(), 100);
    
    // Test giveback()
    EXPECT_NO_THROW(pool_.giveback(buf1));
    
    // Test size()
    EXPECT_EQ(pool_.size(), 0);
}

TEST(IIMemPoolTest, EdgeCases) {
    MockMemPool pool;
    
    // Test zero size
    auto buf1 = pool.get(0);
    EXPECT_FALSE(buf1);
    
    // Test giveback null
    EXPECT_NO_THROW(pool.giveback(nullptr));
}
