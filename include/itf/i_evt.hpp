// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-08-20
// [Describe]	Event Interface
// [Copyright]  xiong.qiang
// [Brief]      Event subscription and publication interface
// *************************************************************************
#pragma once

#include "itf.hpp"
#include "x/x.hpp"

namespace nb {

constexpr Version VER_EVT       = {1, 0, 0};
constexpr Version VER_EVT_MIN   = {1, 0, 0};

struct EvtMsg {
    EvtMsg() = default;

    EvtMsg(x::cStr& evt, x::cStr& src,x::cStr& codeInfo) : 
    evt_(evt), src_(src), codeInfo_(codeInfo),time_(x::Time::now()) {}

    void done()                     noexcept { done_ = true;    }
    bool isDone()           const   noexcept { return done_;    }

    x::cStr & evt()         const   noexcept { return evt_;     }
    x::cStr & src()         const   noexcept { return src_;     }
    x::cStr & codeInfo()    const   noexcept { return codeInfo_;}
    const x::Time & time()  const   noexcept { return time_;    }

    x::str  toString()      const   noexcept {
        return _fmt("EvtMsg: evt:{}, src:{}, codeInfo:{}, time:{}",
                    evt_,src_,codeInfo_,time_.to_string());
    }

private:
    bool    done_ = false;
    x::str  evt_;
    x::str  src_;
    x::str  codeInfo_;
    x::Time time_;
};

enum class MSG_HANDLE{
    DIRECT,
    ASYNC
};

#define _make_msg(msg,src) EvtMsg(msg,src,_code_info())

class I_EvtSub : public ITF {
public:
    virtual ~I_EvtSub() = default;

    virtual x::Result onEvt(const EvtMsg& msg, const x::Struct& d) = 0;
    
protected:
    I_EvtSub(): ITF("I_EvtSub",VER_EVT,VER_EVT_MIN) {}
};

typedef std::function<x::Result(const EvtMsg& msg, const x::Struct& d)> EvtCB;

class I_Evt : public ITF {
public:
    virtual ~I_Evt() = default;

    virtual void   sub(x::cStr& evtName, I_EvtSub* sub,
                       const MSG_HANDLE& type = MSG_HANDLE::DIRECT) noexcept = 0;
    virtual void   sub(x::cStr& evtName, x::cStr& subName, const EvtCB& cb,
                       const MSG_HANDLE& type = MSG_HANDLE::DIRECT) noexcept = 0;
    virtual void unsub(x::cStr& evtName, x::cStr& subName)          noexcept = 0;              
    virtual void unsub(x::cStr& evtName, I_EvtSub* sub)             noexcept = 0;
    virtual void unsub(I_EvtSub* sub)                               noexcept = 0;
    virtual void pub(const EvtMsg& msg, const x::Struct& d = {})    noexcept = 0;

protected:
    I_Evt(): ITF("I_Evt",VER_EVT,VER_EVT_MIN) { };
};

} // namespace nb
