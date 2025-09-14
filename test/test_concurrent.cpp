#include <gtest/gtest.h>
#include "core/concurrent.hpp"
#include <thread>
#include <chrono>

using namespace nb;

class ConcurrentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test basic task creation and properties
TEST_F(ConcurrentTest, TaskBasicProperties) {
    Task task;
    
    // Test that task is not done initially
    EXPECT_FALSE(task.is_done());
    
    // Test task completion
    task.done();
    EXPECT_TRUE(task.is_done());
}

// Test task equality operators
TEST_F(ConcurrentTest, TaskEqualityOperators) {
    Task task1;
    Task task2;
    
    // Different tasks should not be equal
    EXPECT_FALSE(task1 == task2);
    EXPECT_TRUE(task1 != task2);
    
    // Task should equal itself
    EXPECT_TRUE(task1 == task1);
    EXPECT_FALSE(task1 != task1);
}

// Test Concurrent class with empty task list
TEST_F(ConcurrentTest, EmptyTaskList) {
    Concurrent concurrent;
    
    // Waiting on empty task list should return true immediately
    EXPECT_TRUE(concurrent.wait_all());
    EXPECT_TRUE(concurrent.wait_all(100)); // With timeout
}

// Test adding and completing a single task
TEST_F(ConcurrentTest, SingleTaskCompletion) {
    Concurrent concurrent;
    auto task = std::make_shared<Task>();
    
    concurrent.add_task(task);
    
    // Task should not be done initially
    EXPECT_FALSE(concurrent.wait_all(10)); // Should timeout since task not done
    
    // Mark task as done and verify wait_all succeeds
    task->done();
    EXPECT_TRUE(concurrent.wait_all());
}

// Test multiple tasks completion
TEST_F(ConcurrentTest, MultipleTasksCompletion) {
    Concurrent concurrent;
    const int num_tasks = 5;
    std::vector<std::shared_ptr<Task>> tasks;
    
    // Add multiple tasks
    for (int i = 0; i < num_tasks; ++i) {
        auto task = std::make_shared<Task>();
        tasks.push_back(task);
        concurrent.add_task(task);
    }
    
    // Initially should not be done
    EXPECT_FALSE(concurrent.wait_all(10));
    
    // Complete tasks one by one
    for (int i = 0; i < num_tasks; ++i) {
        tasks[i]->done();
        // Should only succeed when all tasks are done
        bool result = concurrent.wait_all(10);
        if (i == num_tasks - 1) {
            EXPECT_TRUE(result);
        } else {
            EXPECT_FALSE(result);
        }
    }
}

// Test timeout behavior
TEST_F(ConcurrentTest, TimeoutBehavior) {
    Concurrent concurrent;
    auto task = std::make_shared<Task>();
    concurrent.add_task(task);
    
    // Should timeout since task is not done
    EXPECT_FALSE(concurrent.wait_all(50)); // 50ms timeout
    
    // Complete task and verify no timeout
    task->done();
    EXPECT_TRUE(concurrent.wait_all(50));
}

// Test stop functionality
TEST_F(ConcurrentTest, StopFunctionality) {
    Concurrent concurrent;
    auto task = std::make_shared<Task>();
    concurrent.add_task(task);
    
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
    Concurrent concurrent;
    const int num_tasks = 3;
    std::vector<std::shared_ptr<Task>> tasks;
    
    for (int i = 0; i < num_tasks; ++i) {
        auto task = std::make_shared<Task>();
        tasks.push_back(task);
        concurrent.add_task(task);
    }
    
    // Start threads to complete tasks at different times
    std::vector<std::thread> completion_threads;
    for (int i = 0; i < num_tasks; ++i) {
        completion_threads.emplace_back([task = tasks[i], delay = (i + 1) * 20]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            task->done();
        });
    }
    
    // Wait for all tasks to complete
    EXPECT_TRUE(concurrent.wait_all(1000)); // 1 second timeout should be sufficient
    
    // Join all completion threads
    for (auto& thread : completion_threads) {
        thread.join();
    }
}

// Test task validity
TEST_F(ConcurrentTest, TaskValidity) {
    Task valid_task;
    EXPECT_TRUE(valid_task.isValid());
}

// Test that wait_all returns immediately when all tasks are already done
TEST_F(ConcurrentTest, WaitAllWithCompletedTasks) {
    Concurrent concurrent;
    auto task = std::make_shared<Task>();
    task->done();
    
    concurrent.add_task(task);
    
    // Should return true immediately since task is already done
    EXPECT_TRUE(concurrent.wait_all());
}

// Test negative timeout (wait forever)
TEST_F(ConcurrentTest, NegativeTimeoutWaitForever) {
    Concurrent concurrent;
    auto task = std::make_shared<Task>();
    concurrent.add_task(task);
    
    // Start waiting with negative timeout (wait forever)
    std::atomic<bool> wait_completed{false};
    std::thread wait_thread([&]() {
        bool result = concurrent.wait_all(-1);
        wait_completed.store(result);
    });
    
    // Give the wait thread time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Complete the task
    task->done();
    
    // Wait for the thread to complete
    wait_thread.join();
    
// Should return true when task completes
    EXPECT_TRUE(wait_completed.load());
}
