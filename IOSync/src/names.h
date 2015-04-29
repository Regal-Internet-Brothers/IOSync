#pragma once

#include "platform.h"

#if defined(PLATFORM_X86) || defined(PLATFORM_X64)
	#define EXECUTABLE_NAME_X86 "IOSync_x86.exe"
	#define EXECUTABLE_NAME_X64 "IOSync_x64.exe"
	
	#define INJECTION_DLL_NAME_X86 "xinput_injector_x86.dll"
	#define INJECTION_DLL_NAME_X64 "xinput_injector_x64.dll"
#elif defined(PLATFORM_ARM) || defined(PLATFORM_ARM64)
	#define EXECUTABLE_NAME_ARM "IOSync_ARM.exe"
	#define EXECUTABLE_NAME_ARM64 "IOSync_ARM64.exe"
	
	#define INJECTION_DLL_NAME_ARM "xinput_injector_ARM.dll"
	#define INJECTION_DLL_NAME_ARM64 "xinput_injector_ARM64.dll"
#endif