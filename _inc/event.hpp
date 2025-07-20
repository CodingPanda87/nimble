// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-06-15
// [Describe]	Event System
// [Copyright]  xiong.qiang
// [Brief]      Event subscription and publication system
// *************************************************************************
#pragma once

#include "x/x.hpp"
#include "itf/i_evt.hpp"
#include "nlohmann/json.hpp"
#include <shared_mutex>

namespace nb{

class ThreadPool;

struct Suber{
    x::str      evt;
    x::str      name;
    I_EvtSub*   sub = nullptr;
    MSG_HANDLE  type;
    EvtCB       cb;

    Suber(x::cStr& evt, I_EvtSub* sub, const MSG_HANDLE& type):
    evt(evt),sub(sub),type(type) {}

    Suber(x::cStr& evt, x::cStr& name,const MSG_HANDLE& type, const EvtCB& cb):
    evt(evt),name(name),type(type),cb(cb) {}

    bool isValid() const noexcept {
        return sub != nullptr || cb != nullptr;
    }
};

class Event : public I_Evt{
public:
    Event() : I_Evt() {}

    void setThreadPool(ThreadPool* pool) noexcept {
        pool_ = pool;
    }

    void eixt() noexcept {
        stopflag_ = true;
        pool_ = nullptr;
    }

    // ------------------- I_Evt -------------------
    void sub(x::cStr& evtName, I_EvtSub* sub,
             const MSG_HANDLE& type = MSG_HANDLE::DIRECT)   noexcept override;
    void sub(x::cStr& evtName,x::cStr& subName, const EvtCB& cb,
             const MSG_HANDLE& type = MSG_HANDLE::DIRECT)   noexcept override;
    void unsub(x::cStr& evtName, x::cStr& subName)          noexcept override;        
    void unsub(x::cStr& evtName, I_EvtSub* sub)             noexcept override;
    void unsub(I_EvtSub* sub)                               noexcept override;
    void pub(const EvtMsg& msg, const x::Struct& d = {})    noexcept override;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<x::str, std::vector<Suber>> subscribers_;
    ThreadPool * pool_       = nullptr;
    bool         stopflag_   = false;
};

} // namespace nb
