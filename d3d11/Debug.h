#pragma once

#include <minwindef.h>

void DXTrace(HRESULT hr, const char* filename, const char* func, int line);
#define HR(hr) if(hr != S_OK) { DXTrace(hr, __FILE__, #hr, __LINE__); }
int DebugPrint(const char* format, ...);
int Print(const char* format, va_list args);
