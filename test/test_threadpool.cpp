#include "gtest/gtest.h"
#define MAXTIME_THREAD_IN_DEAD 1
#include "core/thread_pool.hpp"

using namespace nb;

TEST(ThreadPoolTest, BasicTaskExecution) {
    ThreadPool pool(2);
    auto future = pool.commit([](){ return 42; });
    EXPECT_EQ(future.get(), 42);
}

TEST(ThreadPoolTest, MultipleTasks) {
    ThreadPool pool(2);
    std::vector<std::future<int>> results;
    for (int i = 0; i < 10; ++i) {
        results.emplace_back(pool.commit([i](){ return i * i; }));
    }
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(results[i].get(), i * i);
    }
}

TEST(ThreadPoolTest, PoolSize) {
    ThreadPool pool(4);
    EXPECT_EQ(pool.poolSize(), 4);
    EXPECT_GE(pool.idlSize(), 0);
}

TEST(ThreadPoolTest, TaskQueue) {
    ThreadPool pool(1); // Single thread to test queue behavior
    std::atomic<int> counter{0};
    
    // Add two tasks - second should wait in queue
    pool.commitNoRet([&counter](){ 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        counter++;
    });
    pool.commitNoRet([&counter](){ counter++; });
    
    // Give some time for tasks to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(counter.load(), 2);
}

TEST(ThreadPoolTest, ThreadActivityTracking) {
    ThreadPool pool(2);
    auto future = pool.commit([](){ 
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return 1; 
    });
    
    // Initially should have idle threads
    EXPECT_GT(pool.idlSize(), 0);
    
    // After task starts, idle count should decrease
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_LT(pool.idlSize(), 2);
    
    future.get();
    // After task completes, idle count should increase again
    EXPECT_GT(pool.idlSize(), 0);
}

TEST(ThreadPoolTest, DestroyWithPendingTasks) {
    ThreadPool pool(2);
    std::atomic<bool> taskStarted{false};
    std::atomic<bool> taskCompleted{false};
    
    pool.commitNoRet([&](){
        taskStarted = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        taskCompleted = true;
    });
    
    // Wait for task to start
    while (!taskStarted) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Destroy pool while task is running
    pool.destroy();
    
    // Task may or may not complete - but pool should shutdown cleanly
    EXPECT_TRUE(taskCompleted);
}

TEST(ThreadPoolAdminTest, CreateAndManagePools) {
    auto manager = ThreadPoolAdmin::instance();
    auto pool = manager->creat(2);
    ASSERT_NE(pool, nullptr);
    EXPECT_LE(pool->poolSize(), 2);
    
    // Test basic operation through manager
    auto future = pool->commit([](){ return 123; });
    EXPECT_EQ(future.get(), 123);
    
    manager->clear();
}


TEST(ThreadPoolAdminTest, DeadThreadNum) {
    auto manager = ThreadPoolAdmin::instance();
    auto pool = manager->creat(2);
    ASSERT_NE(pool, nullptr);
    EXPECT_LE(pool->poolSize(), 2);
    
    // Test basic operation through manager
    auto future = pool->commit([](){ std::this_thread::sleep_for(std::chrono::milliseconds(2500)); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    EXPECT_EQ(pool->deadThreadNum(), 1);
    auto future2 = pool->commit([](){ std::this_thread::sleep_for(std::chrono::milliseconds(1200)); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(pool->deadThreadNum(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EXPECT_EQ(pool->deadThreadNum(), 2);
    EXPECT_EQ(manager->deadThreadNum(), 2);
    manager->clear();
}



