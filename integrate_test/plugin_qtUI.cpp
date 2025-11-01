#include "nb.hpp"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

#define TEST_DUMP

NB_GLOBAL()

class TestWnd: public QMainWindow {
public:
    TestWnd(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Test Qt Window");
        auto vlayout = new QVBoxLayout();
        auto btn = new QPushButton("fire", this);
        connect(btn, &QPushButton::clicked, this, [this]() {
#ifdef TEST_DUMP
            int * a = nullptr;
            *a = 100;
#else
            cnt_++;
            PUB_EVT_ONE("ui.test.fire", "btn",cnt_);
            LOG_INFO("TestQtUI", _fmt("Button clicked, count: {}", cnt_));
#endif
        });
        SUB_EVT("ui.test.fire", "TestQtUI", [this](const nb::EvtMsg& msg, const x::Struct& d)->x::Result {
            LOG_INFO("TestQtUI", _fmt("Received fire event, count: {}", d.getOnly1<int>()));
            label_->setText(_fmt("shot : {}",d.getOnly1<int>()).c_str());
            return x::Result::OK();
        });
        label_ = new QLabel("Hello, Qt!", this);
        auto centralWidget = new QWidget(this);
        vlayout->addWidget(btn);
        vlayout->addWidget(label_);
        centralWidget->setLayout(vlayout);
        this->setCentralWidget(centralWidget);
        resize(800, 600);
    }

    QLabel *label_ = nullptr;
    int cnt_ = 0;
};

namespace nb {

class TesQtUI : public I_Plugin {
public:
    TesQtUI() = default;
    ~TesQtUI() override = default;
    TestWnd *mainWindow = nullptr;

    x::Result init(I_Ctx* ctx) override {
        NB_GLOBAL_INIT(ctx);
        if (!g_log) 
            return x::Result(1,"Log interface not found");
        ctx->evt()->sub("ui.main.init", "QtUI", [this,ctx](const EvtMsg& msg, const x::Struct& d)->x::Result {
            if(mainWindow == nullptr) {
                mainWindow = new TestWnd();
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
