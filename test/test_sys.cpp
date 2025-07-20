#include "gtest/gtest.h"
#include "core/sys.hpp"

namespace nb::sys::test {

TEST(SysTest, ProcessInfo) {
    // Test proc_id() returns non-zero
    EXPECT_NE(proc_id(), 0);

    // Test proc_path() returns non-empty string
    auto path = proc_path();
    EXPECT_FALSE(path.empty());

    // Test proc_name() returns non-empty string
    auto name = proc_name();
    EXPECT_FALSE(name.empty());

    // Test proc_dir() returns non-empty string and matches path
    auto dir = proc_dir();
    EXPECT_FALSE(dir.empty());
    EXPECT_TRUE(path.starts_with(dir));
}

TEST(SysTest, EnvironmentVariables) {
    const x::str var_name = "NB_TEST_VAR";
    const x::str var_value = "test_value";

    // Test set_env and get_env
    EXPECT_TRUE(set_env(var_name, var_value));
    EXPECT_EQ(get_env(var_name), var_value);

    // Test get_env with non-existent variable
    EXPECT_TRUE(get_env("NON_EXISTENT_VAR").empty());
}

TEST(SysTest, CommandExecution) {
    // Test basic command execution
    auto result = run_cmd("echo hello");
    EXPECT_NE(result.find("hello"), std::string::npos);

    // Test command with error (should still return output)
    auto error_result = run_cmd("ls --invalid");
    EXPECT_FALSE(error_result.empty());
}

TEST(SysTest, MutexSemaphore) {
    const x::str mutex_name = "NB_TEST_MUTEX";

    // Test new_only creates a new mutex/semaphore
    void* mutex = new_only(mutex_name);
    EXPECT_NE(mutex, nullptr);

    // Test has_only detects existing mutex/semaphore
    EXPECT_TRUE(has_only(mutex_name));

    // Test rm_only removes the mutex/semaphore
    EXPECT_TRUE(rm_only(mutex,mutex_name));
    EXPECT_FALSE(has_only(mutex_name));

    // Test new_only fails when mutex/semaphore already exists
    void* mutex2 = new_only(mutex_name);
    EXPECT_NE(mutex2, nullptr);
    EXPECT_EQ(new_only(mutex_name), nullptr);
    std::thread t1([=](){
        EXPECT_TRUE(has_only(mutex_name));
    });
    t1.join();
    EXPECT_TRUE(rm_only(mutex2, mutex_name));
    std::thread t2([=](){
        EXPECT_FALSE(has_only(mutex_name));
    });
    t2.join();
}

} // namespace nb::sys::test
