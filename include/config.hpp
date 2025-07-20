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
    nb::I_Log*      nb::g_log       = nullptr; \
    nb::Memory*     nb::g_memory    = nullptr;
