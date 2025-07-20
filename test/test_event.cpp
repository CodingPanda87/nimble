#include "gtest/gtest.h"
#include "../_inc/event.hpp"
#include "../include/core/thread_pool.hpp"

using namespace nb;

// Mock subscriber for testing
class MockSubscriber : public I_EvtSub {
public:
    MockSubscriber():I_EvtSub(){

    }
    x::Result onEvt(const EvtMsg& msg, const x::Struct& d) override {
        lastMsg = msg;
        lastData = d;
        lastHandle = currentHandle;
        callCount++;
        threadId = x::id_thread();
        return x::Result::OK();
    }

    EvtMsg lastMsg;
    x::Struct lastData;
    MSG_HANDLE lastHandle = MSG_HANDLE::ASYNC;
    int callCount = 0;
    MSG_HANDLE currentHandle = MSG_HANDLE::ASYNC;
    size_t threadId = 0;
};

TEST(EventTest, SubscribeUnsubscribe) {
    Event event;
    ThreadPool pool;
    event.setThreadPool(&pool);
    MockSubscriber sub1, sub2;

    // Test single subscription
    event.sub("test.event", &sub1);
    event.pub(EvtMsg{"test.event","1","2"});
    x::sleep(10);
    EXPECT_EQ(sub1.callCount, 1);

    // Test multiple subscribers
    event.sub("test.event", &sub2);
    event.pub(EvtMsg{"test.event","1","2"});
    x::sleep(10);
    EXPECT_EQ(sub1.callCount, 2);
    EXPECT_EQ(sub2.callCount, 1);

    // Test unsubscribe by event name
    event.unsub("test.event", &sub1);
    event.pub(EvtMsg{"test.event","1","2"});
    x::sleep(10);
    EXPECT_EQ(sub1.callCount, 2); // Should not increase
    EXPECT_EQ(sub2.callCount, 2); // Should increase

    // Test unsubscribe all for subscriber
    event.unsub(&sub2);
    event.pub(EvtMsg{"test.event","1","2"});
    x::sleep(10);
    EXPECT_EQ(sub2.callCount, 2); // Should not increase
}

TEST(EventTest, PublishWithData) {
    Event event;
    ThreadPool pool;
    event.setThreadPool(&pool);
    MockSubscriber sub;
    x::Struct testData;
    testData.add("key",_s("value"));

    event.sub("data.event", &sub);
    event.pub(EvtMsg{"data.event","1","2"}, testData);
    x::sleep(10);

    EXPECT_EQ(sub.callCount, 1);
    EXPECT_EQ(sub.lastMsg.evt(), "data.event");
    EXPECT_EQ(sub.lastData.get<std::string>("key"), "value");
}

TEST(EventTest, MultipleEventTypes) {
    Event event;
    ThreadPool pool;
    event.setThreadPool(&pool);
    MockSubscriber sub;

    event.sub("event.type1", &sub);
    event.sub("event.type2", &sub);
    
    event.pub(EvtMsg{"event.type1","1","2"});
    event.pub(EvtMsg{"event.type2","1","2"});
    x::sleep(10);
    
    EXPECT_EQ(sub.callCount, 2);
}

TEST(EventTest, MessageHandleTypes) {
    Event event;
    ThreadPool pool;
    event.setThreadPool(&pool);
    MockSubscriber sub1, sub2;

    sub1.currentHandle = MSG_HANDLE::DIRECT;
    sub2.currentHandle = MSG_HANDLE::ASYNC;
    
    event.sub("handle.test", &sub1, MSG_HANDLE::DIRECT);
    event.sub("handle.test", &sub2, MSG_HANDLE::ASYNC);
    
    event.pub(EvtMsg{"handle.test","1","2"});
    x::sleep(10);
    EXPECT_EQ(sub1.callCount, 1);
    EXPECT_EQ(sub2.callCount, 1);
    EXPECT_EQ(sub1.threadId, x::id_thread());
    EXPECT_NE(sub2.threadId, x::id_thread());
    EXPECT_EQ(sub1.lastHandle, MSG_HANDLE::DIRECT);
    EXPECT_EQ(sub2.lastHandle, MSG_HANDLE::ASYNC);
}

TEST(EventTest, CallbackSubscription) {
    Event event;
    ThreadPool pool;
    event.setThreadPool(&pool);
    
    int callbackCount = 0;
    EvtMsg lastMsg;
    x::Struct lastData;
    
    // Test basic callback
    auto cb = [&](const EvtMsg& msg, const x::Struct& d) -> x::Result {
        callbackCount++;
        lastMsg = msg;
        lastData = d;
        return x::Result::OK();
    };
    
    event.sub("callback.test", "test1", cb);
    event.pub(EvtMsg{"callback.test","1","2"});
    x::sleep(10);
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(lastMsg.evt(), "callback.test");
    
    // Test with data
    x::Struct testData;
    testData.add("key", _s("value"));
    event.pub(EvtMsg{"callback.test","1","2"}, testData);
    x::sleep(10);
    EXPECT_EQ(callbackCount, 2);
    EXPECT_EQ(lastData.get<std::string>("key"), "value");
    
    // Test unsubscribe by name
    event.unsub("callback.test", "test1");
    event.pub(EvtMsg{"callback.test","1","2"});
    x::sleep(10);
    EXPECT_EQ(callbackCount, 2); // Should not increase
}

TEST(EventTest, NullSubscriber) {
    Event event;
    
    // Should not crash
    event.sub("null.test", nullptr);
    event.unsub("null.test", nullptr);
    event.unsub(static_cast<I_EvtSub*>(nullptr));
    event.pub(EvtMsg{"null.test","1","2"});
}

TEST(EventTest, ThreadSafety) {
    Event event;
    ThreadPool pool(4); // Use 4 threads for testing
    event.setThreadPool(&pool);
    MockSubscriber sub1, sub2, sub3;

    // Concurrent subscribe/unsubscribe
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&] {
            event.sub("concurrent.test", &sub1);
            event.sub("concurrent.test", &sub2);
            event.unsub("concurrent.test", &sub1);
        });
    }

    // Concurrent publish while modifying subscriptions
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&] {
            event.pub(EvtMsg{"concurrent.test","1","2"});
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Verify no crashes and basic functionality
    event.sub("concurrent.test", &sub3);
    event.pub(EvtMsg{"concurrent.test","1","2"});
    x::sleep(10);
    EXPECT_GE(sub3.callCount, 1);
}

TEST(EventTest, StressTest) {
    Event event;
    ThreadPool pool(8); // Use more threads for stress test
    event.setThreadPool(&pool);
    
    const int NUM_SUBSCRIBERS = 1000;
    std::vector<std::unique_ptr<MockSubscriber>> subscribers;
    subscribers.reserve(NUM_SUBSCRIBERS);

    // Create and subscribe many subscribers
    for (int i = 0; i < NUM_SUBSCRIBERS; ++i) {
        subscribers.emplace_back(std::make_unique<MockSubscriber>());
        event.sub("stress.test", subscribers.back().get());
    }

    // Publish events to all subscribers
    for (int i = 0; i < 10; ++i) {
        event.pub(EvtMsg{"stress.test","1","2"});
    }
    x::sleep(100); // Give more time for processing

    // Verify all subscribers received events
    for (const auto& sub : subscribers) {
        EXPECT_GE(sub->callCount, 1);
    }

    // Unsubscribe all
    for (const auto& sub : subscribers) {
        event.unsub("stress.test", sub.get());
    }

    // Verify unsubscription worked
    event.pub(EvtMsg{"stress.test","1","2"});
    x::sleep(10);
    for (const auto& sub : subscribers) {
        EXPECT_EQ(sub->callCount, 10); // Should not have increased
    }
}

TEST(EventTest, CleanShutdown) {
    Event event;
    ThreadPool pool;
    event.setThreadPool(&pool);
    MockSubscriber sub;
    
    event.sub("shutdown.test", &sub);
    event.eixt();
    event.pub(EvtMsg{"shutdown.test","1","2"});
    x::sleep(10);
    
    EXPECT_EQ(sub.callCount, 0);
}
