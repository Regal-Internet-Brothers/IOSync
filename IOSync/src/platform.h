#pragma once

// Definitions:
#define WIN32_LEAN_AND_MEAN

#if defined(_WIN32) || defined(_WIN64)
	#define PLATFORM_WINDOWS
	#define PLATFORM_WINDOWS_EXTENSIONS
#endif

#ifdef PLATFORM_WINDOWS
	#include <windows.h>
#endif