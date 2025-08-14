#include <QApplication>
#include "nb.hpp"

int main(int argc, char *argv[]) {
    // only dump plugin info
    if(argc > 2){
        if(argv[1] == std::string("--info")){
            const auto info = std::format("[Path] = {}\n[Info] = \n\n{}\n\n",argv[2],
                                          nb::Platform::instance()->pluginInfo(argv[2])); 
            std::cerr << info << std::endl;
            return 0;
        }else{
            std::cout << "usage: " << argv[0] << " --info <plugin_path>" << std::endl;
            return 1;
        }
    }
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
