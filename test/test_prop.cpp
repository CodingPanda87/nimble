#include "gtest/gtest.h"
#include "prop.hpp"
#include "x/x.hpp"

class PropertyTest : public ::testing::Test {
protected:
    void SetUp() override {
        prop = new nb::Property("test_module");
    }

    void TearDown() override {
        delete prop;
    }

    nb::Property* prop;
};

TEST_F(PropertyTest, AddGetProp) {
    // 测试添加和获取基本类型属性
    EXPECT_TRUE(prop->addProp("int_val", 42).ok());
    auto int_val = prop->getProp<int>("int_val");
    EXPECT_TRUE(int_val.has_value());
    EXPECT_EQ(int_val.value(), 42);

    // 测试添加和获取字符串类型属性
    EXPECT_TRUE(prop->addProp("str_val", std::string("test")).ok());
    auto str_val = prop->getProp<std::string>("str_val");
    EXPECT_TRUE(str_val.has_value());
    EXPECT_EQ(str_val.value(), "test");

    // 测试获取不存在的属性
    auto not_exist = prop->getProp<int>("not_exist");
    EXPECT_FALSE(not_exist.has_value());
}

TEST_F(PropertyTest, SetProp) {
    // 初始设置属性
    EXPECT_TRUE(prop->addProp("int_val", 42).ok());
    
    // 测试修改属性值
    EXPECT_TRUE(prop->setProp("int_val", 100).ok());
    auto int_val = prop->getProp<int>("int_val");
    EXPECT_TRUE(int_val.has_value());
    EXPECT_EQ(int_val.value(), 100);

    // 测试修改不存在的属性
    auto result = prop->setProp("not_exist", 100);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.code(), 1);
    EXPECT_EQ(result.message(), "no prop = not_exist");
}

class TestListener : public nb::PropListener {
public:
    void onPropChanged(x::cStr& module, x::cStr& key, const std::any& value) override{
        lastModule = module;
        lastKey = key;
        lastValue = value;
    }

    x::str lastModule;
    x::str lastKey;
    std::any lastValue;
};

TEST_F(PropertyTest, PropListener) {
    TestListener listener;
    bool callbackCalled = false;
    prop->clearListener();
    
    // 添加监听器回调
    auto callback = [&](x::cStr& module, x::cStr& key, const std::any& value) {
        callbackCalled = true;
        listener.onPropChanged(module, key, value);
    };
    
    // 添加属性和监听器
    EXPECT_TRUE(prop->addProp("test_key", 0).ok());
     EXPECT_TRUE(prop->addProp("test_key2", 0).ok());
    EXPECT_TRUE(prop->addPropListener("test_key", callback).ok());
   
    
    // 修改属性值，验证监听器被调用
    EXPECT_TRUE(prop->setProp("test_key", 42).ok());
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(listener.lastModule, "test_module");
    EXPECT_EQ(listener.lastKey, "test_key");
    EXPECT_EQ(std::any_cast<int>(listener.lastValue), 42);
    
    // 验证监听器自动注销
    {
        TestListener tempListener;
        EXPECT_TRUE(prop->addPropListener("test_key", &tempListener).ok());
        EXPECT_TRUE(prop->addPropListener("test_key2", &tempListener).ok());
        EXPECT_EQ(prop->sizeListener(), 3); 
    }
    // tempListener已析构，应该自动注销
    EXPECT_EQ(prop->sizeListener(), 1); // 只剩下之前的listener
}

TEST_F(PropertyTest, WildcardListener) {
    TestListener listener;
    int callbackCount = 0;
    
    // 添加通配符监听器
    auto callback = [&](x::cStr& module, x::cStr& key, const std::any& value) {
        callbackCount++;
        listener.onPropChanged(module, key, value);
    };
    
    EXPECT_TRUE(prop->addProp("key1", 0).ok());
    EXPECT_TRUE(prop->addProp("key2", 0).ok());
    EXPECT_TRUE(prop->addPropListener("*", callback).ok());
    
    // 修改多个属性值，验证通配符监听器被调用
    EXPECT_TRUE(prop->setProp("key1", 10).ok());
    EXPECT_TRUE(prop->setProp("key2", 20).ok());
    EXPECT_EQ(callbackCount, 2);
}

TEST_F(PropertyTest, MultipleListeners) {
    prop->clearListener();
    TestListener listener;
    int callback1Count = 0, callback2Count = 0;
    
    auto callback1 = [&](x::cStr&, x::cStr&, const std::any&) { callback1Count++; };
    auto callback2 = [&](x::cStr&, x::cStr&, const std::any&) { callback2Count++; };
    
    EXPECT_TRUE(prop->addProp("test_key", 0).ok());
    EXPECT_TRUE(prop->addPropListener("test_key", callback1).ok());
    EXPECT_TRUE(prop->addPropListener("test_key", callback2).ok());
    EXPECT_TRUE(prop->addPropListener("test_key", &listener).ok());
    
    EXPECT_TRUE(prop->setProp("test_key", 42).ok());
    EXPECT_EQ(callback1Count, 1);
    EXPECT_EQ(callback2Count, 1);
    EXPECT_EQ(std::any_cast<int>(listener.lastValue), 42);
}

TEST_F(PropertyTest, AllPropListeners) {
    prop->clearListener();
    bool callbackCalled = false;
    TestListener listener;
    int callback1Count = 0, callback2Count = 0;
    
    auto callback1 = [&](x::cStr&, x::cStr&, const std::any&) { callback1Count++; };
    auto callback2 = [&](x::cStr&, x::cStr&, const std::any&) { callback2Count++; };
    
    EXPECT_TRUE(prop->addProp("test_key", 0).ok());
    EXPECT_TRUE(prop->addProp("test_key_2", 0).ok());
    EXPECT_TRUE(prop->addPropListener("test_key", callback1).ok());
    EXPECT_TRUE(prop->addPropListener("*", callback2).ok());
    EXPECT_TRUE(prop->addPropListener("test_key_2", &listener).ok());
    
    EXPECT_TRUE(prop->setProp("test_key", 42).ok());
    EXPECT_TRUE(prop->setProp("test_key_2", 43).ok());
    EXPECT_EQ(callback1Count, 1);
    EXPECT_EQ(callback2Count, 2);
    EXPECT_EQ(std::any_cast<int>(listener.lastValue), 43);
}

TEST_F(PropertyTest, ThreadSafety) {
    prop->clearListener();
    std::atomic_int total = 0;
    std::atomic_bool isWriteDone = false;
    prop->addProp("test_key", 0);
    std::mutex mtx;
    auto outMsg = [&](x::cStr& msg){
        std::lock_guard<std::mutex> lck(mtx);
        std::cout << msg.c_str() << std::endl;
    };
    try{
        std::thread writeThread([&](){
            try{
                for (int i = 0; i < 100; ++i) {
                    EXPECT_TRUE(prop->setProp("test_key", i + 1).ok());
                    x::sleep(30);
                }
                x::sleep(10);
                isWriteDone = true;
            }catch(const std::exception& e){
                outMsg(x::str("read except") + e.what());
            }
            outMsg("write done");
        });
        std::thread readThread([&](){
            int last = 0;
            int t2 = 0;
            try{
                while (!isWriteDone) {
                    auto value = prop->getProp<int>("test_key");
                    if(value.has_value()){
                        const auto v = value.value(); 
                        if(v != last){
                            last = v;
                            total += v;
                        }
                    }
                    x::sleep(10);
                }
            }
            catch(const std::exception& e){
                outMsg(x::str("read except") + e.what());
            }
            outMsg("read done");
        });
        writeThread.join();
        readThread.join();
    }catch(const std::exception& e){
        std::cout << e.what() << std::endl;
    }
    EXPECT_EQ(total, 5050);
}
