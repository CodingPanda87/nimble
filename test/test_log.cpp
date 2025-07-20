#include "gtest/gtest.h"
#include "../_inc/log.hpp"
#include <memory>

using namespace nb;
using namespace std::chrono_literals;

class LogTest : public ::testing::Test {
protected:
    void SetUp() override {
        log = std::make_unique<Log>();
        log->addFilter(R"({"key":"filter","expr":"*","listener":{"key":"console"}})");
    }

    void TearDown() override {
        log.reset();
    }

    std::unique_ptr<Log> log;
};

TEST_F(LogTest, LogLevelConversion) {
    EXPECT_EQ(LogLevelToString(LogLevel::DEBUG), "DEBUG");
    EXPECT_EQ(LogLevelToString(LogLevel::INFO), "INFO");
    EXPECT_EQ(LogLevelToString(LogLevel::WARN), "WARN");
    EXPECT_EQ(LogLevelToString(LogLevel::ERROR), "ERROR");
    EXPECT_EQ(LogLevelToString(LogLevel::FATAL), "FATAL");
    
    EXPECT_EQ(ParseLogLevel("debug"), LogLevel::DEBUG);
    EXPECT_EQ(ParseLogLevel("info"), LogLevel::INFO);
    EXPECT_EQ(ParseLogLevel("warn"), LogLevel::WARN);
    EXPECT_EQ(ParseLogLevel("error"), LogLevel::ERROR);
    EXPECT_EQ(ParseLogLevel("fatal"), LogLevel::FATAL);
    EXPECT_EQ(ParseLogLevel("unknown"), LogLevel::DEBUG);
}

TEST_F(LogTest, LogExprEvaluation) {
    LogItem testItem{
        .title = "Test",
        .msg = "Listener test",
        .codeInfo = "test.cpp:40",
        .time = x::Time::now(),
        .level = LogLevel::INFO
    };

    LogExpr expr;
    
    // Test level comparisons
    expr.field = LogicField::Level;
    expr.ref = LogLevel::INFO;
    expr.symbol = ExprSymbol::EQUAL;
    EXPECT_TRUE(expr.isOK(testItem));
    
    expr.symbol = ExprSymbol::NOT_EQUAL;
    EXPECT_FALSE(expr.isOK(testItem));
    
    // Test time comparisons
    expr.field = LogicField::Time;
    expr.ref = testItem.time;
    expr.symbol = ExprSymbol::EQUAL;
    EXPECT_TRUE(expr.isOK(testItem));
    
    // Test regex matching
    expr.field = LogicField::RegexTitle;
    expr.ref = x::str("Test.*");
    expr.symbol = ExprSymbol::REGEX;
    EXPECT_TRUE(expr.isOK(testItem));
}

TEST_F(LogTest, LogFilterParsing) {
    LogFilter filter;

    // Test new format simple condition
    EXPECT_TRUE(LogFilter::fromString("(level==info)", filter));
    EXPECT_EQ(filter.exprs.size(), 1);
    EXPECT_EQ(filter.operators.size(), 0);

    // Test new format complex conditions
    EXPECT_TRUE(LogFilter::fromString(
        "(level>debug)&&(time>2023-07-18 12:34:56)||(regex_msg==abc)||(regex_codeinfo==*abc)",
        filter));
    EXPECT_EQ(filter.exprs.size(), 4);
    EXPECT_EQ(filter.operators.size(), 3);

    // Test all operator types
    EXPECT_TRUE(LogFilter::fromString("(level>=debug)", filter));
    EXPECT_TRUE(LogFilter::fromString("(time<2023-01-01)", filter));
    EXPECT_TRUE(LogFilter::fromString("(regex_msg!=abc)", filter));

    // Test invalid formats
    EXPECT_FALSE(LogFilter::fromString("level==info", filter)); // Missing parentheses
    EXPECT_FALSE(LogFilter::fromString("(level=info)", filter)); // Invalid operator
    EXPECT_FALSE(LogFilter::fromString("(unknown>value)", filter)); // Unknown field
}

TEST_F(LogTest, ComplexFilterEvaluation) {
    LogFilter filter;
    x::Time testTime = x::Time("2023-07-19 10:00:00");
    x::Time oldTime = x::Time("2023-07-17 10:00:00");

    // Test AND logic
    EXPECT_TRUE(LogFilter::fromString("(level>debug)&&(time>2023-07-18)", filter));
    
    LogItem matchingItem{
        .title = "Test",
        .msg = "Message",
        .codeInfo = "test.cpp:1",
        .time = testTime,
        .level = LogLevel::INFO
    };
    EXPECT_TRUE(filter.check(matchingItem));

    LogItem nonMatchingLevel{
        .title = "Test",
        .msg = "Message", 
        .codeInfo = "test.cpp:1",
        .time = testTime,
        .level = LogLevel::DEBUG
    };
    EXPECT_FALSE(filter.check(nonMatchingLevel));

    LogItem nonMatchingTime{
        .title = "Test",
        .msg = "Message",
        .codeInfo = "test.cpp:1",
        .time = oldTime,
        .level = LogLevel::INFO
    };
    EXPECT_FALSE(filter.check(nonMatchingTime));

    // Test OR logic
    EXPECT_TRUE(LogFilter::fromString("(level==debug)||(regex_msg==.*error.*)", filter));
    
    LogItem debugItem{
        .title = "Test",
        .msg = "Debug message",
        .codeInfo = "test.cpp:1",
        .time = testTime,
        .level = LogLevel::DEBUG
    };
    EXPECT_TRUE(filter.check(debugItem));

    LogItem errorMsgItem{
        .title = "Test",
        .msg = "error occurred",
        .codeInfo = "test.cpp:1",
        .time = testTime,
        .level = LogLevel::INFO
    };
    EXPECT_TRUE(filter.check(errorMsgItem));

    LogItem nonMatchingItem{
        .title = "Test",
        .msg = "Normal message",
        .codeInfo = "test.cpp:1",
        .time = testTime,
        .level = LogLevel::INFO
    };
    EXPECT_FALSE(filter.check(nonMatchingItem));

    // Test complex combination
    EXPECT_TRUE(LogFilter::fromString(
        "(level>=info)&&(time>2023-07-18)||(regex_codeinfo==.*test.*)", 
        filter));

    LogItem complexMatch1{
        .title = "Test",
        .msg = "Message",
        .codeInfo = "test.cpp:1",
        .time = testTime,
        .level = LogLevel::WARN
    };
    EXPECT_TRUE(filter.check(complexMatch1));

    LogItem complexMatch2{
        .title = "Test",
        .msg = "Message",
        .codeInfo = "other.cpp:1",
        .time = oldTime,
        .level = LogLevel::DEBUG
    };
    EXPECT_FALSE(filter.check(complexMatch2));

    LogItem complexMatch3{
        .title = "Test",
        .msg = "Message",
        .codeInfo = "test_file.cpp:1",
        .time = oldTime,
        .level = LogLevel::DEBUG
    };
    EXPECT_TRUE(filter.check(complexMatch3));
}

TEST_F(LogTest, BasicLogging) {
    testing::internal::CaptureStdout();
    
    log->debug("Test", "Debug message", "test.cpp:20");
    log->info("Test", "Info message", "test.cpp:21");
    log->warn("Test", "Warn message", "test.cpp:22");
    log->error("Test", "Error message", "test.cpp:23");
    log->fatal("Test", "Fatal message", "test.cpp:24");

    log->pump();
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(output.empty());
}

TEST_F(LogTest, LogFileOperations) {
    LogFile logFile;
    const x::str testPath = "test_log.txt";
    
    EXPECT_TRUE(logFile.open(testPath));
    EXPECT_TRUE(logFile.write("Test log entry\n"));
    
    LogItem testItem{
        .title = "FileTest",
        .msg = "Testing file logging",
        .codeInfo = "test.cpp:30",
        .time = x::Time::now(),
        .level = LogLevel::INFO
    };
    
    logFile.onLog(testItem);
    logFile.close();
    
    // Note: Would need file system checks here in a real test
}

TEST_F(LogTest, ListenerManagement) {
    class MockListener : public I_LogListener {
    public:
        void onLog(const LogItem&) noexcept override { called = true; }
        bool called = false;
    };
    
    MockListener listener;
    log->addListener("mock", &listener);
    log->addFilter(R"({"key":"filter","expr":"*","listener":{"key":"mock"}})");
    log->info("Test", "Listener test", "test.cpp:40");
    log->pump();
    
    EXPECT_TRUE(listener.called);
}

TEST_F(LogTest, TimeFilter) {
    class MockListener : public I_LogListener {
    public:
        void onLog(const LogItem&) noexcept override { 
            called++; 
            _prtln("called");
        }
        int called = 0;
    };
    MockListener listener;
    log->addListener("mock", &listener);
    auto now = x::Time::now();
    _prtln("now: {}", now.to_string());
    now = now + 100000us;
    _prtln("now: {}", now.to_string());
    log->addFilter(x::vfmt(R"({{"key":"filter","expr":" (time>{}) ","listener":{{"key":"mock"}}}})", now.to_string()));
    auto i = 6;
    while(i-->0){
        log->info("Test", "Listener test", "test.cpp:40");
        x::sleep(40);
    }
    log->pump();
    _prtln("called: {}", listener.called);
    EXPECT_TRUE(listener.called < 6 && listener.called > 0);
}

TEST_F(LogTest, FileFilter) { 
    std::filesystem::path path = "test_log.txt";
    std::filesystem::remove(path);
    auto ret = log->addFilter(R"({"key":"filter","expr":" (level==info) && (regex_msg==.*error.*) ","listener":{"path":"test_log.txt"}})");
    EXPECT_TRUE(ret);
    log->info("Test", "This is a test", "test.cpp:40");
    log->error("Test", "This is a test", "test.cpp:40");
    log->info("Test", "This is a test,error,hello", "test.cpp:40");
    log->pump();
    log->flush();
    log->exit();
    EXPECT_TRUE(std::filesystem::exists(path));
    std::ifstream file(path);
    EXPECT_TRUE(file.good());
    std::string line;
    std::getline(file, line);
    x::vprtln(line);
    EXPECT_TRUE(line.find("[INFO]") != std::string::npos);
}

TEST_F(LogTest, LogFile) { 
    LogFile logFile;
    x::remove_file("test_log1.txt");
    EXPECT_TRUE(logFile.open("test_log1.txt"));
    LogItem it{
        .title      = "FileTest",
        .msg        = "Testing file logging",
        .codeInfo   = "test.cpp:30",
        .time       = x::Time::now(),
        .level      = LogLevel::INFO
    };
    auto nw1 = x::Time::now();
    auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    for(int i = 0; i < 100000; ++i)
        logFile.onLog(it);
    auto nw2 = x::Time::now();
    auto t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    _prtln("cost time = {}", nw2 - nw1);
    _prtln("cost time = {}", (t2 - t1));
    logFile.close();
    EXPECT_TRUE(std::filesystem::exists("test_log1.txt"));
}

