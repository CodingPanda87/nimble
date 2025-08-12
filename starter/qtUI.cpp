#include <QApplication>
#include "nb.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    auto& platform =  *nb::Platform::instance();
    if(!platform.init("cfg.json",true)){
        std::cout << "platform init failed" << std::endl;
        return 1;
    }
    platform.evt()->pub(_make_msg("ui.main.init","main"),x::Struct());

    QObject::connect(&app, &QApplication::aboutToQuit, []() {
        nb::Platform::instance()->stop();
    });
    
    return app.exec();
}
