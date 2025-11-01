#include "nlohmann/json_helper.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace std;
using json = nlohmann::json;

// 测试用的基础结构体
struct TestAddress {
    string street = "Unknown Street";
    string city = "Unknown City";
    int zip_code = 0;
    
    JSON_DEFINE_TYPE_INTRUSIVE(TestAddress, street, city, zip_code)
};

struct TestPerson {
    string name;
    int age = 0;
    vector<string> hobbies;
    string nickname;
    TestAddress address;
    double salary = 0.0;
    bool employed = false;
    
    JSON_DEFINE_TYPE_INTRUSIVE(TestPerson, name, age, hobbies, nickname, address, salary, employed)
};

// 非侵入式版本
struct TestPersonNonIntrusive {
    string name;
    int age = 0;
    vector<string> hobbies;
};
JSON_DEFINE_TYPE_NON_INTRUSIVE(TestPersonNonIntrusive, name, age, hobbies)

// 枚举测试
enum class TestStatus {
    Pending,
    Active,
    Inactive
};

NLOHMANN_JSON_SERIALIZE_ENUM(TestStatus, {
    {TestStatus::Pending, "pending"},
    {TestStatus::Active, "active"},
    {TestStatus::Inactive, "inactive"}
})

struct TestWithEnum {
    string name;
    TestStatus status = TestStatus::Pending;
    
    JSON_DEFINE_TYPE_INTRUSIVE(TestWithEnum, name, status)
};

class JsonHelperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 完整的测试数据
        complete_json = R"({
            "name": "Alice",
            "age": 30,
            "hobbies": ["reading", "swimming"],
            "nickname": "Ali",
            "address": {
                "street": "123 Main St",
                "city": "New York",
                "zip_code": 10001
            },
            "salary": 50000.5,
            "employed": true
        })"_json;
        
        // 不完整的测试数据
        incomplete_json = R"({
            "name": "Bob"
        })"_json;
        
        // 类型错误的测试数据
        wrong_type_json = R"({
            "name": "Charlie",
            "age": "not_a_number",
            "hobbies": "not_an_array",
            "salary": "not_a_double"
        })"_json;
    }
    
    json complete_json;
    json incomplete_json;
    json wrong_type_json;
};

// 测试正常情况下的序列化/反序列化
TEST_F(JsonHelperTest, CompleteDataSerialization) {
    TestPerson person;
    EXPECT_NO_THROW(person = complete_json.get<TestPerson>());
    
    EXPECT_EQ(person.name, "Alice");
    EXPECT_EQ(person.age, 30);
    EXPECT_EQ(person.hobbies.size(), 2);
    EXPECT_EQ(person.hobbies[0], "reading");
    EXPECT_EQ(person.hobbies[1], "swimming");
    EXPECT_EQ(person.nickname, "Ali");
    EXPECT_EQ(person.address.street, "123 Main St");
    EXPECT_EQ(person.address.city, "New York");
    EXPECT_EQ(person.address.zip_code, 10001);
    EXPECT_DOUBLE_EQ(person.salary, 50000.5);
    EXPECT_TRUE(person.employed);
}

// 测试不完整数据的反序列化（应该使用默认值）
TEST_F(JsonHelperTest, IncompleteDataUsesDefaults) {
    TestPerson person;
    EXPECT_NO_THROW(person = incomplete_json.get<TestPerson>());
    
    EXPECT_EQ(person.name, "Bob");
    EXPECT_EQ(person.age, 0);  // 默认值
    EXPECT_EQ(person.hobbies.size(), 0);  // 默认空vector
    EXPECT_TRUE(person.nickname == "");  
    EXPECT_EQ(person.address.street, "Unknown Street");  // 嵌套默认值
    EXPECT_EQ(person.address.city, "Unknown City");
    EXPECT_EQ(person.address.zip_code, 0);
    EXPECT_DOUBLE_EQ(person.salary, 0.0);  // 默认值
    EXPECT_FALSE(person.employed);  // 默认值
}

// 测试类型错误数据的反序列化（应该使用默认值）
TEST_F(JsonHelperTest, WrongTypeDataUsesDefaults) {
    TestPerson person;
    EXPECT_NO_THROW(person = wrong_type_json.get<TestPerson>());
    
    EXPECT_EQ(person.name, "Charlie");
    EXPECT_EQ(person.age, 0);  // 类型错误，使用默认值
    EXPECT_EQ(person.hobbies.size(), 0);  // 类型错误，使用默认空vector
    EXPECT_DOUBLE_EQ(person.salary, 0.0);  // 类型错误，使用默认值
}

// 测试序列化功能
TEST_F(JsonHelperTest, SerializationWorks) {
    TestPerson person;
    person.name = "David";
    person.age = 25;
    person.hobbies = {"coding", "gaming"};
    person.nickname = "Dave";
    person.address.street = "456 Oak Ave";
    person.address.city = "Boston";
    person.address.zip_code = 02115;
    person.salary = 75000.0;
    person.employed = true;
    
    json j = person;
    
    EXPECT_EQ(j["name"], "David");
    EXPECT_EQ(j["age"], 25);
    EXPECT_EQ(j["hobbies"].size(), 2);
    EXPECT_EQ(j["hobbies"][0], "coding");
    EXPECT_EQ(j["hobbies"][1], "gaming");
    EXPECT_EQ(j["nickname"], "Dave");
    EXPECT_EQ(j["address"]["street"], "456 Oak Ave");
    EXPECT_EQ(j["address"]["city"], "Boston");
    EXPECT_EQ(j["address"]["zip_code"], 02115);
    EXPECT_DOUBLE_EQ(j["salary"], 75000.0);
    EXPECT_TRUE(j["employed"]);
}

// 测试非侵入式宏
TEST_F(JsonHelperTest, NonIntrusiveMacroWorks) {
    TestPersonNonIntrusive person;
    person.name = "Eve";
    person.age = 28;
    person.hobbies = {"hiking"};
    
    // 序列化
    json j = person;
    EXPECT_EQ(j["name"], "Eve");
    EXPECT_EQ(j["age"], 28);
    EXPECT_EQ(j["hobbies"][0], "hiking");
    
    // 反序列化
    json j2 = R"({"name": "Frank", "age": 32})"_json;
    TestPersonNonIntrusive person2 = j2.get<TestPersonNonIntrusive>();
    
    EXPECT_EQ(person2.name, "Frank");
    EXPECT_EQ(person2.age, 32);
    EXPECT_EQ(person2.hobbies.size(), 0);  // 默认空vector
}

// 测试枚举支持
TEST_F(JsonHelperTest, EnumSupportWorks) {
    TestWithEnum obj;
    obj.name = "Test";
    obj.status = TestStatus::Active;
    
    // 序列化
    json j = obj;
    EXPECT_EQ(j["name"], "Test");
    EXPECT_EQ(j["status"], "active");
    
    // 反序列化正常值
    json j2 = R"({"name": "Test2", "status": "inactive"})"_json;
    TestWithEnum obj2 = j2.get<TestWithEnum>();
    EXPECT_EQ(obj2.name, "Test2");
    EXPECT_EQ(obj2.status, TestStatus::Inactive);
    
    // 反序列化缺失枚举字段（使用默认值）
    json j3 = R"({"name": "Test3"})"_json;
    TestWithEnum obj3 = j3.get<TestWithEnum>();
    EXPECT_EQ(obj3.name, "Test3");
    EXPECT_EQ(obj3.status, TestStatus::Pending);  // 默认值
}

// 测试空JSON对象
TEST_F(JsonHelperTest, EmptyJsonObject) {
    json empty_json = json::object();
    
    TestPerson person;
    EXPECT_NO_THROW(person = empty_json.get<TestPerson>());
    
    // 所有字段都应该是默认值
    EXPECT_EQ(person.name, "");
    EXPECT_EQ(person.age, 0);
    EXPECT_EQ(person.hobbies.size(), 0);
    EXPECT_TRUE(person.nickname == "");
    EXPECT_EQ(person.address.street, "Unknown Street");
    EXPECT_EQ(person.address.city, "Unknown City");
    EXPECT_EQ(person.address.zip_code, 0);
    EXPECT_DOUBLE_EQ(person.salary, 0.0);
    EXPECT_FALSE(person.employed);
}

// 测试json_value_safe函数
TEST(JsonItemValueTest, BasicFunctionality) {
    json j = R"({
        "string_field": "test_value",
        "int_field": 42,
        "double_field": 3.14,
        "bool_field": true
    })"_json;
    
    // 正常取值
    EXPECT_EQ(nlohmann::json_value_safe(j, "string_field", string("default")), string("test_value"));
    EXPECT_EQ(nlohmann::json_value_safe(j, "int_field", 0), 42);
    EXPECT_DOUBLE_EQ(nlohmann::json_value_safe(j, "double_field", 0.0), 3.14);
    EXPECT_EQ(nlohmann::json_value_safe(j, "bool_field", false), true);
    
    // 缺失字段使用默认值
    EXPECT_EQ(nlohmann::json_value_safe(j, "missing_field", string("default")), string("default"));
    EXPECT_EQ(nlohmann::json_value_safe(j, "another_missing", 100), 100);
    
    // 类型错误使用默认值
    json wrong_type = R"({"int_field": "not_a_number"})"_json;
    EXPECT_EQ(nlohmann::json_value_safe(wrong_type, "int_field", 999), 999);
}

// 测试嵌套结构体的默认值
TEST_F(JsonHelperTest, NestedStructDefaults) {
    json partial_address = R"({
        "name": "Grace",
        "address": {
            "city": "Chicago"
        }
    })"_json;
    
    TestPerson person;
    EXPECT_NO_THROW(person = partial_address.get<TestPerson>());
    
    EXPECT_EQ(person.name, "Grace");
    EXPECT_EQ(person.address.city, "Chicago");
    EXPECT_EQ(person.address.street, "Unknown Street");  // 嵌套默认值
    EXPECT_EQ(person.address.zip_code, 0);  // 嵌套默认值
}

// 测试vector类型的默认行为
TEST_F(JsonHelperTest, VectorDefaultBehavior) {
    // 测试空数组
    json empty_array = R"({"name": "Test", "hobbies": []})"_json;
    TestPerson person1;
    EXPECT_NO_THROW(person1 = empty_array.get<TestPerson>());
    EXPECT_EQ(person1.hobbies.size(), 0);
    
    // 测试缺失数组字段
    json no_array = R"({"name": "Test"})"_json;
    TestPerson person2;
    EXPECT_NO_THROW(person2 = no_array.get<TestPerson>());
    EXPECT_EQ(person2.hobbies.size(), 0);  // 默认空vector
}

// 性能测试：确保不会抛出异常影响性能
TEST_F(JsonHelperTest, NoExceptionThrown) {
    // 这些操作都不应该抛出异常
    EXPECT_NO_THROW({
        TestPerson p1 = complete_json.get<TestPerson>();
        TestPerson p2 = incomplete_json.get<TestPerson>();
        TestPerson p3 = wrong_type_json.get<TestPerson>();
        TestPerson p4 = json::object().get<TestPerson>();
    });
}

// 边界值测试
TEST_F(JsonHelperTest, BoundaryValues) {
    // 测试数值边界
    json boundary_json = R"({
        "name": "BoundaryTest",
        "age": 2147483647,
        "salary": 1.7976931348623157e+308
    })"_json;
    
    TestPerson person;
    EXPECT_NO_THROW(person = boundary_json.get<TestPerson>());
    EXPECT_EQ(person.age, 2147483647);
    EXPECT_DOUBLE_EQ(person.salary, 1.7976931348623157e+308);
}
