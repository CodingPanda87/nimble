// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2025-01-20
// [Describe]	Platform Context
// [Copyright]  xiong.qiang
// [Brief]      Main platform context and interface implementation
// *************************************************************************
#pragma once

#include "itf/i_ctx.hpp"

namespace nb {

class Platform : public I_Ctx {
public:
    Platform() : I_Ctx() {}

    x::Result   init(x::cStr &cfgPath);
    void        stop();

    // ----------------------- I_Ctx -----------------------
    I_Evt*      evt()                   const noexcept override;
    I_Log*      log()                   const noexcept override;
    I_MemPool*  memPool()               const noexcept override;
    ThreadPoolAdmin* 
                threadPoolAdmin()       const noexcept override;
    I_PluginAdmin* 
                pluginAdmin()           const noexcept override;

    void        exit(x::cStr &info = "")const noexcept override;

protected:

};

} // namespace nb
