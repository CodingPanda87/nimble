#include <QApplication>
#include "nb.hpp"

#ifdef WIN32
#include <windows.h>
// 控制台事件处理函数
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
    case CTRL_C_EVENT:         // Ctrl+C
    case CTRL_BREAK_EVENT:     // Ctrl+Break
    case CTRL_CLOSE_EVENT:     // 控制台关闭
    case CTRL_LOGOFF_EVENT:    // 用户注销
    case CTRL_SHUTDOWN_EVENT:  // 系统关机
        QApplication::exit(0);
        return TRUE;  // 表示已处理该事件
    default:
        return FALSE; // 未处理的事件
    }
}
#endif

int main(int argc, char *argv[]) {

#ifdef WIN32
    // if start in console ,auto attach to parent
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);

        std::cout.clear();
        std::cerr.clear();
        std::cin.clear();

        std::cout << "console opened!" << std::endl;
        // 设置控制台事件处理函数
        SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    }
#endif

    // only dump plugin info
    if(argc > 2){
        if(argv[1] == std::string("--info")){
            const auto info = std::format("[Path] = {}\n[Info] = \n\n{}\n\n",argv[2],
                                          nb::Platform::instance()->pluginInfo(argv[2])); 
            std::cerr << info << std::endl;
            return 0;
        }
        else{
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
    platform.log()->info("test",_fmt("argc = {}",argc),_code_info());
    if(argc == 2)
        platform.log()->info("test",_fmt("argv = {}",argv[1]),_code_info());

    QObject::connect(&app, &QApplication::aboutToQuit, []() {
        nb::Platform::instance()->stop();
    });
    
    return app.exec();
}
