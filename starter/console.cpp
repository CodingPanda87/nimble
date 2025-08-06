#include <iostream>
#include "nb.hpp"
#include <atomic>
#ifdef _WIN32
#include <Windows.h>
#else
#include <csignal>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#endif

// 全局标志用于通知主程序退出
std::atomic<bool> g_exit_requested(false);

#ifdef _WIN32
// 控制台事件处理函数
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
    case CTRL_C_EVENT:         // Ctrl+C
    case CTRL_BREAK_EVENT:     // Ctrl+Break
    case CTRL_CLOSE_EVENT:     // 控制台关闭
    case CTRL_LOGOFF_EVENT:    // 用户注销
    case CTRL_SHUTDOWN_EVENT:  // 系统关机
        std::cout << "\nwill close console..." << std::endl;
        g_exit_requested = true;
        return TRUE;  // 表示已处理该事件
    default:
        return FALSE; // 未处理的事件
    }
}
#else
// 信号处理函数
void signal_handler(int signal) {
    switch(signal) {
        case SIGHUP:    // 终端关闭
            std::cout << "\n[信号] 终端断开 (SIGHUP)" << std::endl;
            g_exit_requested = true;
            break;
            
        case SIGINT:    // Ctrl+C
            std::cout << "\n[信号] Ctrl+C 按下 (SIGINT)" << std::endl;
            g_exit_requested = true;
            break;
            
        case SIGTERM:   // kill 命令
            std::cout << "\n[信号] 终止请求 (SIGTERM)" << std::endl;
            g_exit_requested = true;
            break;
            
        case SIGQUIT:   // Ctrl+\
            std::cout << "\n[信号] 退出请求 (SIGQUIT)" << std::endl;
            g_exit_requested = true;
            break;
    }
}

// 守护进程初始化
void daemon_init() {
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "创建守护进程失败" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (pid > 0) { // 父进程退出
        exit(EXIT_SUCCESS);
    }
    
    // 子进程成为新会话的领导
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    
    // 忽略终端I/O信号
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    
    // 再次fork确保不是会话组长
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    // 设置文件权限掩码
    umask(0);
    
    // 改变工作目录
    chdir("/");
    
    // 关闭所有打开的文件描述符
    for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--) {
        close(fd);
    }
    
    // 重定向标准输入输出
    open("/dev/null", O_RDWR); // stdin
    dup(0); // stdout
    dup(0); // stderr
}
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // 设置控制台事件处理函数
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
#else
    // 检查是否以守护进程模式运行
    bool is_daemon = false;
    if (argc > 1 && std::string(argv[1]) == "--daemon") {
        is_daemon = true;
        daemon_init();
    }
    // 注册信号处理函数
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGHUP,  &sa, nullptr);   // 终端关闭
    sigaction(SIGINT,  &sa, nullptr);   // Ctrl+C
    sigaction(SIGTERM, &sa, nullptr);   // kill 命令
    sigaction(SIGQUIT, &sa, nullptr);   // Ctrl+[\]
#endif
    auto& platform =  *nb::Platform::instance();
    if(!platform.init("cfg.json",false)){
        std::cout << "platform init failed" << std::endl;
        return 1;
    }
    while(!g_exit_requested && platform.isRunning()){
        platform.pump();
        x::sleep(10);
    }
    platform.stop();
    std::cerr << x::Time::now().to_string() << std::endl;
    return 0;
}