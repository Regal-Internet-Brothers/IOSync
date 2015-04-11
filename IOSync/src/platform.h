#pragma once

// Definitions:
#define WIN32_LEAN_AND_MEAN

#if defined(_WIN32) || defined(_WIN64)
	#define PLATFORM_WINDOWS
	#define PLATFORM_WINDOWS_EXTENSIONS
#endif

#ifdef _M_AMD64
	#define PLATFORM_X64
	#define PLATFORM_64
#endif

#ifdef _M_IX86
	#define PLATFORM_X86
	#define PLATFORM_32
#endif

#ifdef PLATFORM_WINDOWS
	#include <windows.h>
#endif

//#ifndef PLATFORM_WINDOWS
#define ZeroVariable(X) memset(&X, 0, sizeof(X))
//#else
	//#define ZeroVariable(X) ZeroMemory(&X, sizeof(X))
//#endif

// Namespace(s):
namespace iosync
{
	// Enumerator(s):
	enum CPUArchitecture
	{
		x86,
		x64,
		ARM,
	};
}