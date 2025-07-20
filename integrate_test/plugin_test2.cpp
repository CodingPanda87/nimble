#include "itf/i_plugin.hpp"
#include "itf/i_ctx.hpp"
#include <string>
#include <iostream>
#include "api.hpp"
#include "itf/i_log.hpp"

namespace nb {

I_Log* g_log = nullptr;

class TestPlugin2 : public I_Plugin {
public:
    TestPlugin2() : counter(0) {}
    ~TestPlugin2() override = default;

    x::Result init(I_Ctx* ctx) override {
        std::cout << "TestPlugin2 initialized" << std::endl;
        return x::Result::OK();
    }

    void pump() override {
        counter++;
    }

    void uninit() override {
        std::cout << "TestPlugin2 uninitialized, counter was: " << counter << std::endl;
    }

    x::cStr& info() override {
        static std::string info = "Test Plugin 2 with counter functionality";
        return info;
    }

    x::cStr& name() override {
        static std::string name = "TestPlugin2";
        return name;
    }

    void* itfPtr(x::cStr& name) const noexcept override {
        return nullptr;
    }

private:
    int counter;
};

// Plugin entry point
extern "C" NB_API void* instanceNB() {
    static TestPlugin2 plugin;
    return &plugin;
}

extern "C" NB_API const char* infoNB() {
    return "Test Plugin 2 for nimble framework";
}

extern "C" NB_API void clearNB() {
    LOG_INFO("PluginRelease","TestPlugin2 release");
}

} // namespace nb
