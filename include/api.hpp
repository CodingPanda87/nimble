// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-10-12
// [Describe]	API Export Macros
// [Copyright]  xiong.qiang
// [Brief]      Platform-specific API export/import definitions
// *************************************************************************
#pragma once

#ifdef NB_EXPORT
#ifdef _WIN32
#define NB_API  __declspec(dllexport)
#else
#define NB_API 
#endif
#else 
#ifdef _WIN32
#define NB_API  __declspec(dllimport)
#else
#define NB_API 
#endif
#endif // !NB_EXPORT
