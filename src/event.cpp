// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2025-02-10
// [Describe]	Event System Implementation
// [Copyright]  xiong.qiang
// [Brief]      Implementation of event subscription and publication
// *************************************************************************
#include "../_inc/event.hpp"
#include "x/x.hpp"
#include "core/thread_pool.hpp"

namespace nb {

void Event::sub(const x::str& evtName, I_EvtSub* sub, const MSG_HANDLE& type) noexcept {
    if (!sub) return;
    
    std::unique_lock<std::shared_mutex> lock(mutex_);
    subscribers_[evtName].emplace_back(evtName, sub, type);
}

void Event::sub(x::cStr& evtName, x::cStr& subName, const EvtCB& cb,
         const MSG_HANDLE& type) noexcept {
    if (!cb) return;
    std::unique_lock<std::shared_mutex> lock(mutex_);
    subscribers_[evtName].emplace_back(evtName, subName, type, cb);
}

void Event::unsub(x::cStr& evtName, x::cStr& subName)          noexcept {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto it = subscribers_.find(evtName);
    if (it != subscribers_.end()) {
        auto& subs = it->second;
        subs.erase(std::remove_if(subs.begin(), subs.end(),
            [subName](const Suber& s) { return s.name == subName; }), subs.end());
    }
}    

void Event::unsub(const x::str& evtName, I_EvtSub* sub) noexcept {
    if (!sub) return;
    
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto it = subscribers_.find(evtName);
    if (it != subscribers_.end()) {
        auto& subs = it->second;
        subs.erase(std::remove_if(subs.begin(), subs.end(),
            [sub](const Suber& s) { return s.sub == sub; }), subs.end());
    }
}

void Event::unsub(I_EvtSub* sub) noexcept {
    if (!sub) return;
    
    std::unique_lock<std::shared_mutex> lock(mutex_);
    for (auto& [_, subs] : subscribers_) {
        subs.erase(std::remove_if(subs.begin(), subs.end(),
            [sub](const Suber& s) { return s.sub == sub; }), subs.end());
    }
}

void Event::pub(const EvtMsg& msg, const x::Struct& d) noexcept {
    if(stopflag_) return;

    std::vector<Suber> *subers = nullptr;
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = subscribers_.find(msg.evt());
        if (it != subscribers_.end())
            subers = &it->second;
    }

    if(subers == nullptr) return;
    for (const auto& sub : *subers) {
        if(sub.type == MSG_HANDLE::DIRECT)
            sub.sub ? sub.sub->onEvt(const_cast<EvtMsg&>(msg), d) : sub.cb(msg, d);
        else if(pool_)
            pool_->commit([sub, msg = const_cast<EvtMsg&>(msg), d] {
                sub.sub ? sub.sub->onEvt(msg, d) : sub.cb(msg, d);
            });
    }
}

} // namespace nb
