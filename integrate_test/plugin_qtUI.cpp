#include "nb.hpp"
#include <QMainWindow>

NB_GLOBAL()

namespace nb {

class TesQtUI : public I_Plugin {
public:
    TesQtUI() = default;
    ~TesQtUI() override = default;
    QMainWindow *mainWindow = nullptr;

    x::Result init(I_Ctx* ctx) override {
        g_log = ctx->log();
        if (!g_log) 
            return x::Result(1,"Log interface not found");
        ctx->evt()->sub("ui.main.init", "QtUI", [this,ctx](const EvtMsg& msg, const x::Struct& d)->x::Result {
            if(mainWindow == nullptr) {
                mainWindow = new QMainWindow();
                mainWindow->show();
            }
            return x::Result::OK();
        });
        return x::Result::OK();
    }

    bool pump() override {
        LOG_INFO("PluginPump","Test Plugin QtUI pump");
        return true;
    }

    void uninit() override {
        if(mainWindow != nullptr)
            delete mainWindow;
        LOG_INFO("uninit","Test Plugin QtUI uninit");
    }

    x::cStr& info() override {
        static std::string info = "producer plugin";
        return info;
    }

    x::cStr& name() override {
        static std::string name = "QtUI";
        return name;
    }

};

// Plugin entry point
extern "C" NB_API void* instanceNB() {
    static TesQtUI plugin;
    return &plugin;
}

extern "C" NB_API const char* infoNB() {
    static std::string info = "Test Plugin Qt Main UI for nimble framework";
    return info.c_str();
}

extern "C" NB_API void clearNB() {
    LOG_INFO("PluginRelease","TestPlugin1 release");
}

} // namespace nb
