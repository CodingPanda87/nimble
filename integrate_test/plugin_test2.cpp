#include "itf/i_plugin.hpp"
#include "itf/i_ctx.hpp"
#include "itf/i_log.hpp"
#include "itf/i_evt.hpp"
#include <string>
#include <iostream>
#include "api.hpp"


namespace nb {

I_Log* g_log = nullptr;

class TestPlugin2 : public I_Plugin {
public:
    TestPlugin2() : counter(0) {}
    ~TestPlugin2() override = default;

    x::Result init(I_Ctx* ctx) override {
        g_log = ctx->log();
        ctx->evt()->sub("food", "TestPlugin2",
            [](const EvtMsg& msg, const x::Struct& d) -> x::Result {
                LOG_INFO("eat food",_fmt("num:{}",d.getOnly1<int>()));
                return x::Result::OK();
            });
        return x::Result::OK();
    }

    bool pump() override {
        counter++;
        return true;
    }

    void uninit() override {
        LOG_INFO("uninit","Test Plugin 2 uninit");
    }

    x::cStr& info() override {
        static std::string info = "Consumer plugin";
        return info;
    }

    x::cStr& name() override {
        static std::string name = "TestPlugin2";
        return name;
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
