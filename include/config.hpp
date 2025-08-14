// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-11-08
// [Describe]	Configuration Macros
// [Copyright]  xiong.qiang
// [Brief]      Global configuration paths and macros
// *************************************************************************
#pragma once

#define DIR_CFG         "config"
#define DIR_CFG_PLAT    DIR_CFG "/nimble"
#define DIR_CFG_BUSI    DIR_CFG "/business"
#define PATH_CFG_PLAT   DIR_CFG_PLAT "/plat.json"


#define NB_GLOBAL()     \
    namespace nb { \
        I_Ctx*      g_ctx       = nullptr; \
        I_Log*      g_log       = nullptr; \
        Memory*     g_memory    = nullptr; \
    } 

#define PUB_EVT(msg,src) \
    if(nb::g_ctx) { \
        nb::g_ctx->evt()->pub(_make_msg(msg,src)); \
    } else { \
        std::cerr << "Event publish failed: Context not initialized." << std::endl; \
    }

#define PUB_EVT_DAT(msg,dat,src) \
    if(nb::g_ctx) { \
        nb::g_ctx->evt()->pub(_make_msg(msg,src),dat); \
    } else { \
        std::cerr << "Event publish failed: Context not initialized." << std::endl; \
    }

#define SUB_EVT(msg,src,cb) \
    if(nb::g_ctx) { \
        nb::g_ctx->evt()->sub(msg,src,cb); \
    } else { \
        std::cerr << "Event subscription failed: Context not initialized." << std::endl; \
    }
