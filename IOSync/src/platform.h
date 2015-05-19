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

#if defined(_M_AMD64) || defined(_M_X64) || defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
	#define PLATFORM_X64
#elif defined(_M_IX86) || defined(_X86_) || defined(i386) || defined (__i386__) || defined(__i386)
	#define PLATFORM_X86
#elif defined(_M_ARM)
	#define PLATFORM_ARM
#elif defined(_M_ARM64)
	#define PLATFORM_ARM64
#endif

#if defined(_WIN64) || defined(PLATFORM_X64) || defined(PLATFORM_ARM64)
	#define PLATFORM_64
#elif defined(_WIN32) || defined(PLATFORM_X86) || defined(PLATFORM_ARM)
	#define PLATFORM_32
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