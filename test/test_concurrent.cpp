#include <gtest/gtest.h>
#include "core/concurrent.hpp"
#include "core/thread_pool.hpp"
#include <thread>
#include <chrono>
#include <atomic>

using namespace nb;

class ConcurrentTest : public ::testing::Test {
protected:
    void SetUp() override {
        thread_pool_ = std::make_unique<ThreadPool>(2);
    }

    void TearDown() override {
        thread_pool_.reset();
    }

    std::unique_ptr<ThreadPool> thread_pool_;
};

// Test basic task creation and properties
TEST_F(ConcurrentTest, TaskBasicProperties) {
    Task task("test_task");
    
    // Test that task is not done initially
    EXPECT_FALSE(task.is_done());
    EXPECT_TRUE(task.isValid());
    EXPECT_EQ(task.key(), "test_task");
    
    // Test task completion
    task.done();
    EXPECT_TRUE(task.is_done());
    EXPECT_FALSE(task.isError());
}

// Test task error handling
TEST_F(ConcurrentTest, TaskErrorHandling) {
    Task task("error_task");
    
    task.done(-1, "Test error");
    EXPECT_TRUE(task.is_done());
    EXPECT_TRUE(task.isError());
    EXPECT_EQ(task.error(), "Test error");
}

// Test task timing
TEST_F(ConcurrentTest, TaskTiming) {
    Task task("timing_task");
    
    task.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    task.done();
    
    EXPECT_GT(task.take_time(), 0);
    EXPECT_TRUE(task.is_done());
}

// Test task equality operators
TEST_F(ConcurrentTest, TaskEqualityOperators) {
    Task task1("task1");
    Task task2("task2");
    
    // Different tasks should not be equal
    EXPECT_FALSE(task1 == task2);
    EXPECT_TRUE(task1 != task2);
    
    // Task should equal itself
    EXPECT_TRUE(task1 == task1);
    EXPECT_FALSE(task1 != task1);
}

// Test Concurrent class with empty task list
TEST_F(ConcurrentTest, EmptyTaskList) {
    Concurrent concurrent("test");
    
    // Waiting on empty task list should return true immediately
    EXPECT_TRUE(concurrent.wait_all());
    EXPECT_TRUE(concurrent.wait_all(100)); // With timeout
    EXPECT_EQ(concurrent.task_count(), 0);
    EXPECT_EQ(concurrent.completed_count(), 0);
}

// Test adding and completing a single task
TEST_F(ConcurrentTest, SingleTaskCompletion) {
    Concurrent concurrent("single_task_test");
    
    std::atomic<bool> task_executed{false};
    bool result = concurrent.add_task("test_task", [&task_executed]() {
        task_executed = true;
    }, thread_pool_.get());
    
    EXPECT_TRUE(result);
    EXPECT_EQ(concurrent.task_count(), 1);
    
    // Wait for task to complete
    EXPECT_TRUE(concurrent.wait_all(1000));
    EXPECT_TRUE(task_executed);
    EXPECT_EQ(concurrent.completed_count(), 1);
    EXPECT_FALSE(concurrent.isError());
}

// Test multiple tasks completion
TEST_F(ConcurrentTest, MultipleTasksCompletion) {
    Concurrent concurrent("multi_task_test");
    const int num_tasks = 5;
    std::atomic<int> completed_count{0};
    
    // Add multiple tasks
    for (int i = 0; i < num_tasks; ++i) {
        bool result = concurrent.add_task(_fmt("task_{}", i), [&completed_count]() {
            completed_count++;
        }, thread_pool_.get());
        EXPECT_TRUE(result);
    }
    
    EXPECT_EQ(concurrent.task_count(), num_tasks);
    
    // Wait for all tasks to complete
    EXPECT_TRUE(concurrent.wait_all(2000));
    EXPECT_EQ(completed_count, num_tasks);
    EXPECT_EQ(concurrent.completed_count(), num_tasks);
    EXPECT_FALSE(concurrent.isError());
}

// Test timeout behavior
TEST_F(ConcurrentTest, TimeoutBehavior) {
    Concurrent concurrent("timeout_test");
    
    // Add a task that takes some time
    bool result = concurrent.add_task("slow_task", []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }, thread_pool_.get());
    
    EXPECT_TRUE(result);
    
    // Should timeout since task takes 100ms
    EXPECT_FALSE(concurrent.wait_all(10)); // 10ms timeout
    
    // Wait longer and it should succeed
    EXPECT_TRUE(concurrent.wait_all(200));
}

// Test stop functionality
TEST_F(ConcurrentTest, StopFunctionality) {
    Concurrent concurrent("stop_test");
    
    // Add a task that would normally take a long time
    bool result = concurrent.add_task("long_task", []() {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }, thread_pool_.get());
    
    EXPECT_TRUE(result);
    
    // Start waiting in a separate thread
    std::atomic<bool> wait_completed{false};
    std::thread wait_thread([&]() {
        bool result = concurrent.wait_all();
        wait_completed.store(result);
    });
    
    // Give the wait thread time to start waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Stop the concurrent processing
    concurrent.stop();
    
    // Wait for the thread to complete
    wait_thread.join();
    
    // wait_all should return false when stopped
    EXPECT_FALSE(wait_completed.load());
}

// Test concurrent task completion from multiple threads
TEST_F(ConcurrentTest, ConcurrentTaskCompletion) {
    Concurrent concurrent("concurrent_test");
    const int num_tasks = 3;
    std::atomic<int> execution_order{0};
    
    for (int i = 0; i < num_tasks; ++i) {
        bool result = concurrent.add_task(_fmt("concurrent_task_{}", i), 
            [i, &execution_order]() {
                std::this_thread::sleep_for(std::chrono::milliseconds((i + 1) * 20));
                execution_order++;
            }, thread_pool_.get());
        EXPECT_TRUE(result);
    }
    
    // Wait for all tasks to complete
    EXPECT_TRUE(concurrent.wait_all(1000)); // 1 second timeout should be sufficient
    EXPECT_EQ(execution_order, num_tasks);
    EXPECT_FALSE(concurrent.isError());
}

// Test task validity
TEST_F(ConcurrentTest, TaskValidity) {
    Task valid_task("valid_task");
    EXPECT_TRUE(valid_task.isValid());
}

// Test that wait_all returns immediately when all tasks are already done
TEST_F(ConcurrentTest, WaitAllWithCompletedTasks) {
    Concurrent concurrent("completed_test");
    
    // Add a task that completes immediately
    bool result = concurrent.add_task("instant_task", []() {
        // Empty function that completes immediately
    }, thread_pool_.get());
    
    EXPECT_TRUE(result);
    
    // Should return true immediately since task completes quickly
    EXPECT_TRUE(concurrent.wait_all());
}

// Test negative timeout (wait forever)
TEST_F(ConcurrentTest, NegativeTimeoutWaitForever) {
    Concurrent concurrent("forever_test");
    
    // Add a task that completes after a short delay
    bool result = concurrent.add_task("delayed_task", []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }, thread_pool_.get());
    
    EXPECT_TRUE(result);
    
    // Start waiting with negative timeout (wait forever)
    std::atomic<bool> wait_completed{false};
    std::thread wait_thread([&]() {
        bool result = concurrent.wait_all(-1);
        wait_completed.store(result);
    });
    
    // Wait for the thread to complete
    wait_thread.join();
    
    // Should return true when task completes
    EXPECT_TRUE(wait_completed.load());
}

// Test error handling in tasks
TEST_F(ConcurrentTest, TaskErrorHandlingInConcurrent) {
    Concurrent concurrent("error_handling_test");
    
    // Add a task that throws an exception
    bool result = concurrent.add_task("error_task", []() {
        throw std::runtime_error("Test exception");
    }, thread_pool_.get());
    
    EXPECT_TRUE(result);
    
    // Wait for task to complete
    EXPECT_TRUE(concurrent.wait_all(1000));
    EXPECT_TRUE(concurrent.isError());
}

// Test print_info functionality
TEST_F(ConcurrentTest, PrintInfoTest) {
    Concurrent concurrent("print_test");
    
    // Add a couple of tasks
    concurrent.add_task("task1", []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }, thread_pool_.get());
    
    concurrent.add_task("task2", []() {
        throw std::runtime_error("Task failed");
    }, thread_pool_.get());
    
    // Wait for tasks to complete
    concurrent.wait_all(1000);
    
    std::string info = concurrent.print_info();
    EXPECT_TRUE(info.find("print_test") != std::string::npos);
    EXPECT_TRUE(info.find("task1") != std::string::npos);
    EXPECT_TRUE(info.find("task2") != std::string::npos);
    EXPECT_TRUE(info.find("ERROR") != std::string::npos);
}

// Test clear functionality
TEST_F(ConcurrentTest, ClearTest) {
    Concurrent concurrent("clear_test");
    
    // Add some tasks
    concurrent.add_task("task1", []() {}, thread_pool_.get());
    concurrent.add_task("task2", []() {}, thread_pool_.get());
    
    EXPECT_EQ(concurrent.task_count(), 2);
    
    // Clear and verify
    concurrent.clear();
    EXPECT_EQ(concurrent.task_count(), 0);
    EXPECT_EQ(concurrent.completed_count(), 0);
    
    // Should be able to add new tasks after clear
    concurrent.add_task("new_task", []() {}, thread_pool_.get());
    EXPECT_EQ(concurrent.task_count(), 1);
}

// Test null thread pool handling
TEST_F(ConcurrentTest, NullThreadPoolTest) {
    Concurrent concurrent("null_pool_test");
    
    // Try to add task with null thread pool
    bool result = concurrent.add_task("null_task", []() {}, nullptr);
    EXPECT_FALSE(result);
    EXPECT_EQ(concurrent.task_count(), 0);
}
