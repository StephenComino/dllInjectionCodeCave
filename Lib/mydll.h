#ifdef DLL_EXPORT
#define DECLDIR extern "C"  __declspec(dllexport)
#else
#define DECLDIR extern "C" __declspec(dllimport)
#endif
DLL_EXPORT void Share();
DLL_EXPORT void Keep();
#pragma once
