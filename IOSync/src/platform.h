#pragma once

// Definitions:
#define WIN32_LEAN_AND_MEAN

#if defined(_WIN32) || defined(_WIN64)
	#define PLATFORM_WINDOWS
	#define PLATFORM_WINDOWS_EXTENSIONS
#elif defined(__linux__)
	#define PLATFORM_LINUX
#elif defined(__APPLE__)
	#define PLATFORM_MAC
#endif

#if defined(_WIN64)
	#define PLATFORM_64
#elif defined(_WIN32)
	#define PLATFORM_32
#endif

#if defined(_M_AMD64)
	#define PLATFORM_X64
#elif defined(_M_IX86)
	#define PLATFORM_X86
#elif defined(_M_ARM)
	#define PLATFORM_ARM
#endif

#ifdef PLATFORM_WINDOWS
	#define NOMINMAX
	
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
		ARM64,
	};
}