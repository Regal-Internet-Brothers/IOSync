#pragma once

// Includes:
#include <QuickLib/QuickSock/QuickSock.h>

#include "../platform.h"

// Libraries:
#ifdef PLATFORM_WINDOWS
	#if defined(PLATFORM_X86)
		#pragma comment(lib, "QuickLib/x86/QuickSock.lib")
	#elif defined(PLATFORM_X64)
		#pragma comment(lib, "QuickLib/x64/QuickSock.lib")
	#endif
#endif
