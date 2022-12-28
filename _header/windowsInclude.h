#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

typedef struct IUnknown IUnknown;

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <objbase.h>

// C RunTime Header Files
#include <cstdlib>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>