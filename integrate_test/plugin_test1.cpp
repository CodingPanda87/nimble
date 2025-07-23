#include "itf/i_plugin.hpp"
#include "itf/i_ctx.hpp"
#include "itf/i_log.hpp"
#include "itf/i_evt.hpp"
#include <string>
#include "api.hpp"

namespace nb {

I_Log* g_log = nullptr;
std::thread g_thread;
bool   g_running = true;

class TestPlugin1 : public I_Plugin {
public:
    TestPlugin1() = default;
    ~TestPlugin1() override = default;

    x::Result init(I_Ctx* ctx) override {
        g_log = ctx->log();
        if (!g_log) 
            return x::Result(1,"Log interface not found");
        g_thread = std::thread([&,ctx](){
            int i = 0;
            while (g_running) {
                ctx->evt()->pub(_make_msg("food","src1"),x::Struct::One(i++));
                LOG_INFO("MakeFood",_fmt("Num:{}",i));
                int cnt = 0;
                while(g_running && cnt++ < 3)
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        g_thread.detach();
        return x::Result::OK();
    }

    bool pump() override {
        LOG_INFO("PluginPump","Test Plugin 1 pump");
        return true;
    }

    void uninit() override {
        LOG_INFO("uninit","Test Plugin 1 uninit");
        g_running = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if(g_thread.joinable())
            g_thread.join();
    }

    x::cStr& info() override {
        static std::string info = "producer plugin";
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
