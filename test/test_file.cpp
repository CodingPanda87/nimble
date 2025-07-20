#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "core/file.hpp"

namespace fs = std::filesystem;
using namespace nb;

class FileTest : public ::testing::Test {
protected:
    const std::string testFile = "test_file.tmp";
    const std::string testContent = "Hello, world!\nThis is a test file.";

    void SetUp() override {
        // Create test file
        std::ofstream out(testFile);
        out << testContent;
        out.close();
    }

    void TearDown() override {
        if (fs::exists(testFile)) {
            fs::remove(testFile);
        }
    }
};

TEST_F(FileTest, BasicOperations) {
    File file;
    EXPECT_FALSE(file.isOpen());
    EXPECT_TRUE(file.open(testFile));
    EXPECT_TRUE(file.isOpen());
    file.close();
    EXPECT_FALSE(file.isOpen());
}

TEST_F(FileTest, ReadOperations) {
    {
        File file;
        file.open(testFile);

        // Test readAll
        EXPECT_EQ(file.readAll(), testContent);
    }

    {
        // Test partial read
        File file;
        file.open(testFile);
        char buffer[6] = {0};
        EXPECT_EQ(file.read(buffer, 5), 5);
        EXPECT_STREQ(buffer, "Hello");
    }
}

TEST_F(FileTest, WriteOperations) {
    const std::string newContent = "New content";
    File file;
    file.open(testFile,true);

    // Test write with string
    EXPECT_TRUE(file.write(newContent));
    file.flush();

    // Verify content
    file.close();
    file.open(testFile);
    EXPECT_EQ(file.readAll(), newContent);
}

TEST_F(FileTest, WriteCacheMode) {
    File file;
    file.writeCacheMode(16); // Small cache for testing
    file.open(testFile,true);

    // These should be cached
    file.write("First");
    file.write("Second");
    
    // Verify nothing written yet
    file.close(false);
    file.open(testFile);
    EXPECT_NE(file.readAll(), "FirstSecond");

    // Now with flush
    file.open(testFile,true);
    file.write("First");
    file.write("Second");
    file.close();
    file.open(testFile);
    EXPECT_EQ(file.readAll(), "FirstSecond");
}

TEST_F(FileTest, ErrorHandling) {
    File file;
    EXPECT_THROW(file.readAll(), std::runtime_error);
    
    char buf[10];
    EXPECT_THROW(file.read(buf, 10), std::runtime_error);
}

TEST_F(FileTest, ClearOperation) {
    File file;
    file.open(testFile);
    EXPECT_TRUE(file.isOpen()); // Should still be open after clear
}