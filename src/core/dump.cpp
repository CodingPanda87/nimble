
#ifdef _WIN32

// windows_crash_handler.cpp
#include <windows.h>
#include <dbghelp.h>
#include <iostream>
#include <string>

#pragma comment(lib, "dbghelp.lib")

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    std::string dumpFile = "crash.dmp";
    HANDLE hFile = CreateFileA(
        dumpFile.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = ExceptionInfo;
    mdei.ClientPointers = FALSE;

    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpNormal,
        &mdei,
        nullptr,
        nullptr
    );

    CloseHandle(hFile);
    std::cout << "Dump written to: " << dumpFile << std::endl;

    return EXCEPTION_EXECUTE_HANDLER;
}

namespace nb{

void InstallCrashHandler() {
    SetUnhandledExceptionFilter(CrashHandler);
}

} // namespace nb

#elif __linux__

// linux_crash_handler.cpp
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ucontext.h>
#include <cstring>
#include <iostream>

void crashHandler(int sig) {
    void* array[50];
    size_t size = backtrace(array, 50);

    int fd = open("crash.log", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd != -1) {
        backtrace_symbols_fd(array, size, fd);
        close(fd);
    }

    std::cout << "Crash log written to crash.log" << std::endl;
    _exit(1);
}

namespace nb{

void InstallCrashHandler() {
    signal(SIGSEGV, crashHandler);
    signal(SIGABRT, crashHandler);
    signal(SIGFPE, crashHandler);
    signal(SIGILL, crashHandler);
}

} // namespace nb

#endif
