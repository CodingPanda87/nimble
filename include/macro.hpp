// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-11-08
// [Describe]	Macros
// [Copyright]  xiong.qiang
// [Brief]      All macros
// *************************************************************************
#pragma once

#define NB_GLOBAL()     \
    namespace nb { \
        I_Ctx*      g_ctx       = nullptr; \
        I_Log*      g_log       = nullptr; \
        Memory*     g_memory    = nullptr; \
        I_Evt*      g_evt       = nullptr; \
    } 

#define NB_GLOBAL_INIT(ctx)                     \
    nb::g_ctx       = ctx;                      \
    nb::g_log       = nb::g_ctx->log();         \
    nb::g_memory    = nb::Memory::instance();   \
    nb::g_evt       = nb::g_ctx->evt(); 
