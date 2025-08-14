// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2025-01-20
// [Describe]	Platform Context
// [Copyright]  xiong.qiang
// [Brief]      Main platform context and interface implementation
// *************************************************************************
#pragma once

#include "itf/i_ctx.hpp"
#include <shared_mutex>
#include <unordered_map>

namespace nb {

class Platform : public I_Ctx {
public:
    ~Platform() override {}

    static Platform*  instance();

    x::Result   init(x::cStr &cfgPath,const bool& isUI);
    void        pump(); // only for console
    void        stop();
    
    static std::string pluginInfo(x::cStr& path);

    // ----------------------- I_Ctx -----------------------
    I_Evt*      evt()                   const noexcept override;
    I_Log*      log()                   const noexcept override;
    I_MemPool*  memPool()               const noexcept override;
    ThreadPoolAdmin* 
                threadPoolAdmin()       const noexcept override;
    I_PluginAdmin* 
                pluginAdmin()           const noexcept override;

    bool        isRunning()             const noexcept override{
        return running_;
    }
    void        exit(x::cStr &info = "")      noexcept override;

    // --------------------- Admin ITF ---------------------
    x::Result   regItf(x::cStr& name, 
                       ITF *itf)              noexcept override;
    ITF*        getItf(x::cStr& name)   const noexcept override;
    void        unregItf(x::cStr& name)       noexcept override;

protected:
    Platform():I_Ctx() {}

    mutable std::shared_mutex           mtx_;
    std::unordered_map<x::str, ITF*>   itfs_;
    bool                               running_ = false;  
};

} // namespace nb
