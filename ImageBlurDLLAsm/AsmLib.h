//// MathLibrary.h - Contains declarations of math functions
#pragma once
#include <Windows.h>

#ifdef IMAGEBLURDLLASM_EXPORTS
#define IMAGEBLURDLLASM_API __declspec(dllexport)
#else
#define IMAGEBLURDLLASM_API __declspec(dllimport)
#endif

extern "C" IMAGEBLURDLLASM_API int MyProc1(DWORD a, DWORD b);