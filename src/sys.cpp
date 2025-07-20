#include "core/sys.hpp"

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <fcntl.h>
#endif

namespace nb::sys {

x::u64 proc_id() {
#ifdef _WIN32
    return static_cast<x::u64>(GetCurrentProcessId());
#else
    return static_cast<x::u64>(getpid());
#endif
}

x::str proc_path() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    return x::str(path);
#else
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path)-1);
    if (len != -1) {
        path[len] = '\0';
        return x::str(path);
    }
    return x::str();
#endif
}

x::str proc_name() {
    x::str path = proc_path();
    size_t pos = path.rfind(
#ifdef _WIN32
        '\\'
#else
        '/'
#endif
    );
    return pos != x::str::npos ? path.substr(pos+1) : path;
}

x::str proc_dir() {
    x::str path = proc_path();
    size_t pos = path.rfind(
#ifdef _WIN32
        '\\'
#else
        '/'
#endif
    );
    return pos != x::str::npos ? path.substr(0, pos) : x::str();
}

x::str get_env(x::cStr& name) {
#ifdef _WIN32
    // Convert narrow string to wide string for Windows API
    int wlen = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
    std::wstring wname(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, &wname[0], wlen);

    wchar_t* value = _wgetenv(wname.c_str());
    if (!value) return x::str();

    // Convert wide string result back to narrow string
    int len = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
    std::string result(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, value, -1, &result[0], len, nullptr, nullptr);
    return x::str(result.c_str());
#else
    char* value = getenv(name.c_str());
    return value ? x::str(value) : x::str();
#endif
}

bool set_env(x::cStr& name, x::cStr& value) {
#ifdef _WIN32
    // Convert name to wide string
    int name_wlen = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
    std::wstring wname(name_wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, &wname[0], name_wlen);

    // Convert value to wide string
    int value_wlen = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    std::wstring wvalue(value_wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, &wvalue[0], value_wlen);

    return _wputenv_s(wname.c_str(), wvalue.c_str()) == 0;
#else
    return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

x::str run_cmd(x::cStr& command) {
    if (command.empty()) {
        return x::str();
    }

#ifdef _WIN32
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hChildStdoutRd, hChildStdoutWr;
    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) {
        return x::str("Failed to create pipe");
    }
    if (!SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hChildStdoutRd);
        CloseHandle(hChildStdoutWr);
        return x::str("Failed to set pipe handle information");
    }

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFOW siStartInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOW));
    siStartInfo.cb = sizeof(STARTUPINFOW);
    siStartInfo.hStdError = hChildStdoutWr;
    siStartInfo.hStdOutput = hChildStdoutWr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Convert command to wide string
    int wlen = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, nullptr, 0);
    std::wstring wcommand(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wcommand[0], wlen);

    // Create a full command line with cmd.exe for better compatibility
    std::wstring fullCommand = L"cmd.exe /c " + wcommand;

    BOOL bSuccess = CreateProcessW(
        NULL,
        const_cast<wchar_t*>(fullCommand.c_str()),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo
    );

    if (!bSuccess) {
        CloseHandle(hChildStdoutWr);
        CloseHandle(hChildStdoutRd);
        return x::str("Failed to create process");
    }

    CloseHandle(hChildStdoutWr);
    CloseHandle(piProcInfo.hThread);

    DWORD dwRead;
    CHAR chBuf[4096];
    x::str result;

    while (true) {
        bSuccess = ReadFile(hChildStdoutRd, chBuf, sizeof(chBuf), &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;
        result.append(chBuf, dwRead);
    }

    DWORD exitCode;
    GetExitCodeProcess(piProcInfo.hProcess, &exitCode);

    CloseHandle(hChildStdoutRd);
    CloseHandle(piProcInfo.hProcess);

    if (exitCode != 0) 
        result = x::str("Command failed with exit code = ") + std::to_string(exitCode) + "\n" + result;
    return result;
#else
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return x::str("Failed to open pipe");

    char buffer[128];
    x::str result;

    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }

    int status = pclose(pipe);
    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);
        if (exitCode != 0)
            result = x::str("Command failed with exit code ") + std::to_string(exitCode) + "\n" + result;
    }
    return result;
#endif
}

void * new_only(x::cStr& name)
{
#ifdef _WIN32
    HANDLE hMutex = CreateMutexA(nullptr, FALSE, ("Global\\"+name).c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return nullptr; // exist
    }
    return hMutex;
#else
    sem_t* sem = sem_open(("/"+name).c_str(), O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED)
        return nullptr; 
    return sem;
#endif
}

bool   rm_only(void *obj, x::cStr& name)
{
    if(obj == nullptr)
        return true;
#ifdef _WIN32
    auto hMutex = (HANDLE)obj;
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
#else
    sem_close((sem_t*)obj);
    sem_unlink(("/"+name).c_str());
#endif
    return true;
}

bool   has_only(x::cStr& name)
{
#ifdef _WIN32
    HANDLE handle = OpenMutexA(SYNCHRONIZE, FALSE, ("Global\\"+name).c_str());
    if (handle) {
        CloseHandle(handle);
        return true;
    }
    return (GetLastError() != ERROR_FILE_NOT_FOUND);
#else
    sem_t* sem = sem_open(("/"+name).c_str(), O_RDONLY);
    if (sem != SEM_FAILED) {
        sem_close(sem);
        return true;
    }
    return (errno != ENOENT);
#endif
}

}
