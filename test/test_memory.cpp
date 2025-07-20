#include "gtest/gtest.h"
#include "core/memory.hpp"

namespace nb {

Memory* g_memory = nullptr;

TEST(MemoryTest, SingletonInstance) {
    Memory* mem1 = Memory::instance();
    Memory* mem2 = Memory::instance();
    ASSERT_EQ(mem1, mem2);
}

TEST(MemoryTest, AddAndRemove) {
    Memory* mem = Memory::instance();
    InfoMemory info1(100, "test1");
    InfoMemory info2(200, "test2");
    
    mem->add(0x1000, info1);
    mem->add(0x2000, info2);
    ASSERT_EQ(mem->size(), 300);
    
    mem->remove(0x1000);
    ASSERT_EQ(mem->size(), 200);
    
    mem->remove(0x2000);
    ASSERT_EQ(mem->size(), 0);
}

TEST(MemoryTest, DumpOutput) {
    Memory* mem = Memory::instance();
    InfoMemory info(300, "test dump");
    mem->add(0x3000, info);
    
    std::string dump = mem->dump();
    ASSERT_TRUE(dump.find("test dump") != std::string::npos);
    ASSERT_TRUE(dump.find("300") != std::string::npos);
    
    mem->remove(0x3000);
    dump = mem->dump();
    ASSERT_TRUE(dump.find("Nice,memory all released") != std::string::npos);
}

TEST(MemoryTest, NewTemplate) {
    g_memory = Memory::instance();
    auto ptr = New<int>(10, "test new");
    ASSERT_NE(ptr.get(), nullptr);
    ASSERT_EQ(g_memory->size(), 10 * sizeof(int));
    
    ptr.reset();
    ASSERT_EQ(g_memory->size(), 0);
}

TEST(MemoryTest, ThreadSafety) {
    Memory* mem = Memory::instance();
    constexpr int kThreads = 4;
    constexpr int kIterations = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([mem, i]() {
            for (int j = 0; j < kIterations; ++j) {
                x::u64 addr = 0x1000 + i * kIterations + j;
                InfoMemory info(1, "thread_test");
                mem->add(addr, info);
                ASSERT_GE(mem->size(), 1);
                mem->remove(addr);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    ASSERT_EQ(mem->size(), 0);
}

TEST(MemoryTest, DuplicateAdd) {
    Memory* mem = Memory::instance();
    InfoMemory info1(100, "dup1");
    InfoMemory info2(200, "dup2");
    
    mem->add(0x1000, info1);
    ASSERT_EQ(mem->size(), 100);
    
    // Add same address with different info
    mem->add(0x1000, info2);
    ASSERT_EQ(mem->size(), 200); // Should overwrite
    
    mem->remove(0x1000);
    ASSERT_EQ(mem->size(), 0);
}

TEST(MemoryTest, NewError) {
    g_memory = nullptr;
    ASSERT_THROW(New<int>(10, "should_throw"), std::exception);
    g_memory = Memory::instance(); // Restore
}

TEST(MemoryTest, ZeroSize) {
    Memory* mem = Memory::instance();
    InfoMemory info(0, "zero_size");
    mem->add(0x1000, info);
    ASSERT_EQ(mem->size(), 0);
    mem->remove(0x1000);
}

TEST(MemoryTest, RemoveNonExistent) {
    Memory* mem = Memory::instance();
    // Should not crash or affect size
    mem->remove(0x9999);
    ASSERT_EQ(mem->size(), 0);
}

TEST(MemoryTest, DumpFormat) {
    Memory* mem = Memory::instance();
    // Test empty dump format
    std::string empty_dump = mem->dump();
    ASSERT_TRUE(empty_dump.find("^_^ Nice,memory all released. ^_^") != std::string::npos);
    
    // Test with one entry
    InfoMemory info(100, "dump_test");
    mem->add(0x1000, info);
    std::string one_dump = mem->dump();
    ASSERT_TRUE(one_dump.find("[Addr] = 0X1000") != std::string::npos);
    ASSERT_TRUE(one_dump.find("[Info] = dump_test") != std::string::npos);
    ASSERT_TRUE(one_dump.find("<Total> = 100 bytes") != std::string::npos);
    
    mem->remove(0x1000);
}

} // namespace nb
