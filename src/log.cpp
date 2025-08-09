// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2025-02-18
// [Describe]	Logging System Implementation
// [Copyright]  xiong.qiang
// [Brief]      Implementation of logging functionality with filtering
// *************************************************************************
#include "../_inc/log.hpp"
#include <sstream>
#include <chrono>

namespace nb {

// for console
void Log::onLog(const LogItem& s) noexcept
{
    std::unique_lock lock(mtxConsole_);
    std::cout << s.toString() << std::endl;
}

// { "key" : "filter","expr":"" , "listener":{"key" = "123","type"= "console","path":"d:/log.txt"}}
x::Result Log::addFilter(x::cStr& filter) noexcept
{
    try {
        nlohmann::json json = nlohmann::json::parse(filter);
        LogFilter newFilter;
        
        if(!LogFilter::fromString(json["expr"], newFilter)) 
            return x::Result(1, "Failed to parse filter expression = "
                   + json["expr"].get<std::string>());
        if(json["listener"].contains("key") && !json["listener"].contains("path")){
            auto it = listeners_.find(json["listener"]["key"]);
            if(it != listeners_.end()) 
                newFilter.listener = listeners_[json["listener"]["key"]];
        }
        
        if(json["listener"].contains("path")){
            auto p = std::make_shared<LogFile>();
            auto const &path = json["listener"]["path"].get<std::string>();
            if(!p->open(path))
                return x::Result(1, "open failed :" + path);
            newFilter.listener = p.get();
            logFiles_[json["key"].get<std::string>()] = p;
        }

        if(newFilter.listener == nullptr)
            return x::Result(1, "no log listener bind");

        filters_.push_back(newFilter);
    } catch(const std::exception& e) {
        return x::Result(2, _s("Failed to add filter: ")+e.what());
    }
    return x::Result(0);
}

void Log::pump()
{
    std::deque<LogItem> logsToProcess;
    {
        std::unique_lock lock(mtxLog_);
        if(logs_.empty()) return;
        logsToProcess.swap(logs_);
    }

    for(const auto& item : logsToProcess) {
        for(const auto& filter : filters_) {
            if(filter.check(item) && filter.listener) {
                filter.listener->onLog(item);
            }
        }
    }
}

} // namespace nb
