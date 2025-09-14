// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-07-01
// [Describe]	Logging System
// [Copyright]  xiong.qiang
// [Brief]      Logging functionality with filtering and output handling
// *************************************************************************
#pragma once

#include "x/x.hpp"
#include "itf/i_log.hpp"
#include <regex>
#include <mutex>
#include <deque>
#include "nlohmann/json.hpp"

namespace nb{

enum class LogType {
    CONSOLE,
    FILE,
    DATABASE
};

enum class LogicOperator{
    AND,
    OR
};

enum class LogicField{
    None,
    Level,
    Time,
    RegexTitle,
    RegexMsg,
    RegexCodeinfo
};

enum class ExprSymbol{
    EQUAL,
    NOT_EQUAL,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL,
    LESS_THAN_OR_EQUAL,
    LESS_THAN,
    REGEX
};

struct LogExpr {
    LogicField      field;
    ExprSymbol      symbol;
    std::any        ref;

    bool isOK(const LogItem &it) const {
        switch (field) {
            case LogicField::Level:{
                switch (symbol) {
                    case ExprSymbol::EQUAL:
                        return it.level == std::any_cast<LogLevel>(ref);
                    case ExprSymbol::NOT_EQUAL:
                        return it.level != std::any_cast<LogLevel>(ref);
                    case ExprSymbol::GREATER_THAN:
                        return it.level > std::any_cast<LogLevel>(ref);
                    case ExprSymbol::LESS_THAN:
                        return it.level < std::any_cast<LogLevel>(ref);
                    case ExprSymbol::GREATER_THAN_OR_EQUAL:
                        return it.level >= std::any_cast<LogLevel>(ref);
                    case ExprSymbol::LESS_THAN_OR_EQUAL:
                        return it.level <= std::any_cast<LogLevel>(ref);
                    default: return false;
                }
            }
            case LogicField::Time:{
                switch (symbol) {
                    case ExprSymbol::EQUAL:
                        return it.time == std::any_cast<x::Time>(ref);
                    case ExprSymbol::NOT_EQUAL:
                        return it.time != std::any_cast<x::Time>(ref);
                    case ExprSymbol::GREATER_THAN:
                        return it.time > std::any_cast<x::Time>(ref);
                    case ExprSymbol::LESS_THAN:
                        return it.time < std::any_cast<x::Time>(ref);
                    case ExprSymbol::GREATER_THAN_OR_EQUAL:
                        return it.time >= std::any_cast<x::Time>(ref);
                    case ExprSymbol::LESS_THAN_OR_EQUAL:
                        return it.time <= std::any_cast<x::Time>(ref);
                    default: return false;
                }
            }
            case LogicField::RegexTitle:{
                std::regex r(std::any_cast<x::str>(ref));
                return std::regex_match(it.title, r);
            }
            case LogicField::RegexMsg:{
                std::regex r(std::any_cast<x::str>(ref));
                return std::regex_match(it.msg, r);
            }
            case LogicField::RegexCodeinfo:{
                std::regex r(std::any_cast<x::str>(ref));
                return std::regex_match(it.codeInfo, std::regex(std::any_cast<x::str>(ref)));
            }
            default: return false;
        }
    }
};

struct LogFilter {
    bool isAll = false;
    std::vector<LogExpr> exprs;
    std::vector<LogicOperator> operators;
    I_LogListener* listener = nullptr;

    bool check(const LogItem &it) const{
        if(isAll) return true;
        if(exprs.empty()) return false;
        auto i = 0;
        bool rest = exprs[i++].isOK(it);
        for(auto const &op:operators){
            if(op == LogicOperator::AND)
                rest = rest && exprs[i].isOK(it);
            else
                rest = rest || exprs[i].isOK(it);
            i++;
        }
        return rest;
    }

    // "*" or 
    // "(level>debug)&&(time>2023-07-18 12:34:56)||(regex_msg==abc)||(regex_codeinfo==*abc)||(regex_title==*abc)"
    static bool fromString(const x::str &s, LogFilter &f){
        f.exprs.clear();
        f.operators.clear();

        if(s == "*"){
            f.isAll = true;
            return true;
        }

        // Split into conditions and operators
        std::vector<x::str> conditions;
        std::vector<x::str> ops;
        size_t start = 0;
        size_t end = s.find("&&");
        while(end != x::str::npos) {
            conditions.push_back(x::trim(s.substr(start, end-start)));
            ops.push_back("&&");
            start = end + 2;
            end = s.find("&&", start);
        }
        end = s.find("||", start);
        while(end != x::str::npos) {
            conditions.push_back(x::trim(s.substr(start, end-start)));
            ops.push_back("||");
            start = end + 2;
            end = s.find("||", start);
        }
        conditions.push_back(x::trim(s.substr(start)));

        // Parse each condition
        for(auto& cond : conditions) {
            // Remove surrounding parentheses
            if(cond.empty() || cond.front() != '(' || cond.back() != ')') 
                return false;
            cond = cond.substr(1, cond.size()-2);

            // Split into field, operator, value
            size_t op_pos = cond.find_first_of("><=!");
            if(op_pos == x::str::npos) return false;
            
            x::str field = cond.substr(0, op_pos);
            x::str op;
            x::str value;
            
            // Handle multi-character operators (==, !=, >=, <=)
            if(op_pos+1 < cond.size() && cond[op_pos+1] == '=') {
                op = cond.substr(op_pos, 2);
                value = cond.substr(op_pos+2);
            } else {
                op = cond.substr(op_pos, 1);
                value = cond.substr(op_pos+1);
            }

            // Create expression
            LogExpr expr;
            if(field == "level") {
                expr.field = LogicField::Level;
                expr.ref = ParseLogLevel(value);
            } else if(field == "time") {
                expr.field = LogicField::Time;
                if(value.find(":") == x::str::npos)
                    value += " 00:00:00";
                expr.ref = x::Time(value);
            } else if(field == "regex_msg") {
                expr.field = LogicField::RegexMsg;
                expr.ref = value;
            } else if(field == "regex_codeinfo") {
                expr.field = LogicField::RegexCodeinfo;
                expr.ref = value;
            } else if(field == "regex_title") {
                expr.field = LogicField::RegexTitle;
                expr.ref = value;
            } else 
                return false;
            
            // Set operator
            if(op == "==")      expr.symbol = ExprSymbol::EQUAL;
            else if(op == "!=") expr.symbol = ExprSymbol::NOT_EQUAL;
            else if(op == ">")  expr.symbol = ExprSymbol::GREATER_THAN;
            else if(op == ">=") expr.symbol = ExprSymbol::GREATER_THAN_OR_EQUAL;
            else if(op == "<")  expr.symbol = ExprSymbol::LESS_THAN;
            else if(op == "<=") expr.symbol = ExprSymbol::LESS_THAN_OR_EQUAL;
            else return false;
            
            f.exprs.push_back(expr);
        }

        // Set logical operators
        for(auto& op : ops) {
            if(op == "&&") f.operators.push_back(LogicOperator::AND);
            else if(op == "||") f.operators.push_back(LogicOperator::OR);
            else return false;
        }

        return true;
    }
};

class LogFile:public I_LogListener {
public:
    LogFile() = default;

    ~LogFile(){
        close();
    }

    bool open(const x::str& path){
        path_ = path;
        file_.open(path, std::ios::out | std::ios::binary | std::ios::app);
        auto ret = file_.is_open();
        if(ret){
             file_.seekg(0, std::ios::end);
             fileSize_ = file_.tellg();
        }
        return ret;
    }

    void close(){
        if(file_.is_open()){
            file_.flush();
            file_.close();
        }
    }

    bool write(const x::str& data){ 
        if(!file_.is_open()) return false;
        file_ << data + "\n";
        fileSize_ += data.size();
        if(fileSize_ > 50 * 1024 * 1024)
            switchFile();
        else
            time2Save();
        return true;
    }

    bool switchFile(){
        close();
        std::filesystem::remove(path_ + ".bak");
        std::filesystem::rename(path_,path_ + ".bak");
        return open(path_);
    }

    // ---------------- I_LogListener ----------------
    void onLog(const LogItem& item) noexcept override{
        write(item.toString());
    }

protected:
    void time2Save(){
        const auto nowTime = x::timestamp_ms();
        if(nowTime - lastSaveTime_ > 100){
            lastSaveTime_ = nowTime;
            file_.flush();
            file_.close();
            open(path_);
        }
    }

private:
    x::str             path_;
    std::fstream       file_;
    size_t             fileSize_ = 0;
    size_t             lastSaveTime_ = 0;
};

class Log : public I_Log,public I_LogListener
{
public:
    explicit Log(const size_t& cacheSize = 10000){
        logs_.resize(cacheSize);
        logs_.clear();
        addListener("console",this);
    }
    ~Log() override {};

    // same thread
    void pump();

    void exit(){
        enabled_ = false;
        pump();
        filters_.clear();
        listeners_.clear();
        logFiles_.clear();
    }

    // --------------- I_Log ----------------
    
    void enable(bool en)                                           noexcept override{
        enabled_ = en;
    }
    void setCache(int size)                                        noexcept override{
        if(size > 0) {
            logs_.resize(size);
            logs_.clear();
        }
    }
    void flush()                                                   noexcept override{
        pump();
    }
    // { "key" : "filter","expr":"" , "listener":{"key"= "123","path":"d:/log.txt"}}
    x::Result addFilter(x::cStr& filter)                           noexcept override;
    void addListener(x::cStr& key,I_LogListener* listener)         noexcept override{
        listeners_[key] = listener;
    }

    void debug(x::cStr& title,x::cStr& message,x::cStr& codeInfo)  noexcept override{
        if(enabled_) pushLog(LogLevel::DEBUG,title,message,codeInfo);
    }
    void  info(x::cStr& title,x::cStr& message,x::cStr& codeInfo)  noexcept override{
        if(enabled_) pushLog(LogLevel::INFO,title,message,codeInfo);
    }
    void warn(x::cStr& title,x::cStr& message,x::cStr& codeInfo)   noexcept override{
        if(enabled_) pushLog(LogLevel::WARN,title,message,codeInfo);
    }
    void error(x::cStr& title,x::cStr& message,x::cStr& codeInfo)  noexcept override{
        if(enabled_) pushLog(LogLevel::ERROR,title,message,codeInfo);
    }
    void fatal(x::cStr& title,x::cStr& message,x::cStr& codeInfo)  noexcept override{
        if(enabled_) pushLog(LogLevel::FATAL,title,message,codeInfo);
    }

    // --------------- I_LogListener ----------------
    void onLog(const LogItem & s) noexcept override; // for console

protected:
    std::mutex                                  mtxLog_;
    std::mutex                                  mtxConsole_;

    _unmap<x::str,I_LogListener*>               listeners_;
    _vec<LogFilter>                             filters_;
    std::deque<LogItem>                         logs_;
    std::atomic<bool>                           enabled_ = true;

    _unmap<x::str,std::shared_ptr<LogFile>>     logFiles_;


    void pushLog(LogLevel level,x::cStr& title,x::cStr& msg,x::cStr& codeInfo){
        LogItem item{
            .title = title,
            .msg = msg,
            .codeInfo = codeInfo,
            .time = x::Time::now(),
            .level = level
        };
        std::unique_lock lock(mtxLog_);
        if(logs_.size() == 10000)
            logs_.pop_front();
        logs_.push_back(item);
    }
};
} // namespace nb
