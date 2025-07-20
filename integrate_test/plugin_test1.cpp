#include "itf/i_plugin.hpp"
#include "itf/i_ctx.hpp"
#include "itf/i_log.hpp"
#include <string>
#include "api.hpp"

namespace nb {

I_Log* g_log = nullptr;

class TestPlugin1 : public I_Plugin {
public:
    TestPlugin1() = default;
    ~TestPlugin1() override = default;

    x::Result init(I_Ctx* ctx) override {
        g_log = ctx->log();
        if (!g_log) 
            return x::Result(1,"Log interface not found");
        return x::Result::OK();
    }

    void pump() override {
        LOG_INFO("PluginPump","Test Plugin 1 pump");
        // Test plugin 1 pump implementation
    }

    void uninit() override {
        LOG_INFO("uninit","Test Plugin 1 uninit");
        // Cleanup resources
    }

    x::cStr& info() override {
        static std::string info = "Test Plugin 1 for nimble framework";
        return info;
    }

    x::cStr& name() override {
        static std::string name = "TestPlugin1";
        return name;
    }

    void* itfPtr(x::cStr& name) const noexcept override {
        return nullptr;
    }
};

// Plugin entry point
extern "C" NB_API void* instanceNB() {
    static TestPlugin1 plugin;
    return &plugin;
}

extern "C" NB_API const char* infoNB() {
    static std::string info = "Test Plugin 1 for nimble framework";
    return info.c_str();
}

extern "C" NB_API void clearNB() {
    LOG_INFO("PluginRelease","TestPlugin1 release");
}

} // namespace nb
