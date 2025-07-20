#include "gtest/gtest.h"
#include "../_inc/mem_pool.hpp"
#include "../include/itf/i_mempool.hpp"

using namespace nb;

class MemPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        try {
            pool_ = std::make_shared<MemPool>();
        } 
        catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown exception" << std::endl;
        }
    }

    std::shared_ptr<MemPool> pool_;

};

class IMemPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Using MemPool as concrete implementation for testing I_MemPool
        pool_ = std::make_shared<MemPool>();
    }

    std::shared_ptr<I_MemPool> pool_;
};

TEST_F(MemPoolTest, BasicAllocation) {
    auto buf = pool_->get(1024);
    EXPECT_NE(buf, nullptr);
    EXPECT_EQ(buf->size(), 1024);
}

TEST_F(IMemPoolTest, InterfaceBasicAllocation) {
    auto buf = pool_->get(1024);
    EXPECT_NE(buf, nullptr);
    EXPECT_EQ(buf->size(), 1024);
}

TEST_F(MemPoolTest, BufferReuse) {
    auto buf1 = pool_->get(1024);
    pool_->giveback(buf1);
    auto buf2 = pool_->get(1024);
    EXPECT_EQ(buf1.get(), buf2.get());  // Should reuse same buffer
}

TEST_F(IMemPoolTest, InterfaceBufferReuse) {
    auto buf1 = pool_->get(1024);
    pool_->giveback(buf1);
    auto buf2 = pool_->get(1024);
    EXPECT_EQ(buf1, buf2);
}

TEST_F(MemPoolTest, SizeMatching) {
    pool_->setMatchFactor(0.1f);
    
    // Create buffer slightly larger than requested
    auto buf = pool_->get(1000);
    pool_->giveback(buf);
    
    // Should match with slightly larger request
    auto matched = pool_->get(950);
    EXPECT_EQ(buf, matched);
    
    // Should not match with too small request
    auto notMatched = pool_->get(800);
    EXPECT_NE(buf, notMatched);
}

TEST_F(MemPoolTest, PoolSizeTracking) {
    EXPECT_EQ(pool_->size(), 0);
    
    auto buf1 = pool_->get(1024);
    EXPECT_EQ(pool_->size(), 1024);
    
    auto buf2 = pool_->get(2048);
    EXPECT_EQ(pool_->size(), 1024 + 2048);
    
    pool_->giveback(buf1);
    EXPECT_EQ(pool_->size(), 1024 + 2048);  // Still in pool
}

TEST_F(IMemPoolTest, InterfaceSizeTracking) {
    EXPECT_EQ(pool_->size(), 0);
    auto buf = pool_->get(1024);
    EXPECT_EQ(pool_->size(), 1024);
    pool_->giveback(buf);
    EXPECT_EQ(pool_->size(), 1024);
}

TEST_F(MemPoolTest, TimeBasedCleanup) {
    pool_->setFreeBufTime(100);  // 100ms timeout
    
    auto buf = pool_->get(1024);
    pool_->giveback(buf);
    
    // Buffer should still be there
    EXPECT_EQ(pool_->size(), 1024);
    
    // Simulate time passing (would need mock in real test)
    // Here we just test the check() function
    pool_->check();
    EXPECT_EQ(pool_->size(), 1024);  // Not enough time passed
    
    // In real test would need to mock timestamp_ms()
}

TEST_F(MemPoolTest, BufferContent) {
    auto buf = pool_->get(1024);
    const char* testData = "test data";
    EXPECT_TRUE(buf->input(testData, strlen(testData)));
    
    pool_->giveback(buf);
    auto buf2 = pool_->get(1024);
    EXPECT_EQ(memcmp(buf2->get(), testData, strlen(testData)), 0);
}

TEST(BufferTest, InputValidation) {
    Buffer buf;
    const char* testData = "test";
    
    // Test invalid cases
    EXPECT_FALSE(buf.input(nullptr, 4));
    EXPECT_FALSE(buf.input(testData, 0));
}

TEST_F(MemPoolTest, MultipleBuffers) {
    std::vector<BufferPtr> buffers;
    for (int i = 0; i < 10; i++) {
        buffers.push_back(pool_->get(512 + i * 100));
    }
    
    for (auto& buf : buffers) {
        pool_->giveback(buf);
    }
    
    EXPECT_EQ(pool_->size(), 512*10 + 100*45);  // Sum of arithmetic series
}

TEST_F(MemPoolTest, MatchFactorEdgeCases) {
    pool_->setMatchFactor(0.01f);
    auto buf = pool_->get(1000);
    pool_->giveback(buf);
    
    // Should match with very close size
    auto matched = pool_->get(992);
    EXPECT_EQ(buf, matched);
    EXPECT_EQ(matched->size(), 1000);
    
    // Should not match with slightly larger difference
    pool_->giveback(matched);
    auto notMatched = pool_->get(1020);
    EXPECT_NE(buf, notMatched);
    EXPECT_EQ(notMatched->size(), 1020);
}

TEST_F(MemPoolTest, EdgeCases) {
    // Zero size
    auto zeroBuf = pool_->get(0);
    EXPECT_EQ(zeroBuf, nullptr);
    
    // Very large size
    auto largeBuf = pool_->get(1024 * 1024 * 1024);
    EXPECT_NE(largeBuf, nullptr);
    
    // Give back nullptr
    pool_->giveback(nullptr);  // Should not crash
    
    // Give back same buffer multiple times
    auto buf = pool_->get(1024);
    pool_->giveback(buf);
    pool_->giveback(buf);  // Should handle gracefully
    EXPECT_EQ(pool_->size(), 1024 * 1024 * 1024 + 1024);
}

TEST_F(IMemPoolTest, InterfaceEdgeCases) {
    // Zero size
    auto zeroBuf = pool_->get(0);
    EXPECT_FALSE(zeroBuf);
    
    // Give back nullptr
    pool_->giveback(nullptr);
    
    // Invalid buffer operations
    BufferPtr invalidBuf;
    pool_->giveback(invalidBuf);
}

TEST_F(IMemPoolTest, VersionInfo) {
    EXPECT_EQ(pool_->version(), VER_MEM);
    EXPECT_EQ(pool_->minVersion(), VER_MEM_MIN);
    EXPECT_EQ(pool_->name(), "I_MemPool");
}
