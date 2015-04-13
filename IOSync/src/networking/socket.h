#pragma once

// Includes:
#include <QuickLib/QuickSock/QuickSock.h>

#include "../platform.h"

// Libraries:
#ifdef PLATFORM_WINDOWS
	#if defined(PLATFORM_X86)
		#ifdef _DEBUG
			#pragma comment(lib, "QuickLib/x86/QuickSock/Debug/QuickSock.lib")
		#else
			#pragma comment(lib, "QuickLib/x86/QuickSock/Release/QuickSock.lib")
		#endif
	#elif defined(PLATFORM_X64)
		#ifdef _DEBUG
			#pragma comment(lib, "QuickLib/x64/QuickSock/Debug/QuickSock.lib")
		#else
			#pragma comment(lib, "QuickLib/x64/QuickSock/Release/QuickSock.lib")
		#endif
	#endif
#endif
