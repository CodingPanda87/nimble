// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-06-10
// [Describe]	Base Interface
// [Copyright]  xiong.qiang
// [Brief]      Core interface class for all NB framework interfaces
// *************************************************************************
#pragma once

#include "core/version.hpp"

namespace nb{

inline constexpr int MAGIC_ITF = 0xAB87CD11;

class ITF{
private:
    int     flag_ = MAGIC_ITF;

public:
    virtual ~ITF() = default;
    
    bool    valid()             const {
        return flag_ == MAGIC_ITF;
    }

    const   x::str&  name()     const {
        return name_;
    }
    const   x::str&  info()     const {
        return info_;
    }

    const Version& version()    const {
        return version_;
    }
    const Version& minVersion() const {
        return minVersion_;
    }

    bool compatible(const Version& ver) const {
        return ver >= minVersion_;
    }

    template <typename T>
    T* obj() noexcept {
        return dynamic_cast<T*>(this);
    }

    template <typename T>
    T* obj(const Version& ver) noexcept {
        auto p = dynamic_cast<T*>(this);
        if(p && p->compatible(ver))
            return p;
        return nullptr;
    }

protected:
    ITF(x::cStr& name,const Version& ver,const Version& min):
    name_(name),version_(ver),minVersion_(min){};
    ITF() = delete;
    ITF(const ITF&) = delete;
    ITF& operator=(const ITF&) = delete;
    ITF(ITF&&) = delete;
    ITF& operator=(ITF&&) = delete;

    x::str  name_;
    x::str  info_;
    Version version_;
    Version minVersion_;
};

} // namspace nb
