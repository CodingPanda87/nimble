// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-07-15
// [Describe]	Context Interface
// [Copyright]  xiong.qiang
// [Brief]      Core context interface for NB framework
// *************************************************************************
#pragma once

#include "itf.hpp"

namespace nb {

class I_Evt;
class I_Log;
class I_MemPool;
class I_PluginAdmin;
class ThreadPoolAdmin;

constexpr Version VER_CTX       = {1, 0, 0};
constexpr Version VER_CTX_MIN   = {1, 0, 0};

class I_Ctx : public ITF {
public:
    virtual ~I_Ctx() = default;
    
    virtual I_Evt*      evt()                   const noexcept = 0;
    virtual I_Log*      log()                   const noexcept = 0;
    virtual I_MemPool*  memPool()               const noexcept = 0;
    virtual ThreadPoolAdmin* 
                        threadPoolAdmin()       const noexcept = 0;
    virtual I_PluginAdmin* 
                        pluginAdmin()           const noexcept = 0;

    virtual void        exit(x::cStr &info = "")const noexcept = 0;
    
protected:
    I_Ctx():ITF("I_CTX",VER_CTX,VER_CTX_MIN) {}
};

} // namespace nb
