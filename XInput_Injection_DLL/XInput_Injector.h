#pragma once

// Preprocessor related:
#define GAMEPAD_EXTERNAL_ORIGIN

#define _CRT_SECURE_NO_WARNINGS

// XInput macros:
#define XINPUT_DEVTYPE_GAMEPAD			0x01
#define XINPUT_DEVSUBTYPE_GAMEPAD		0x01
#define XINPUT_CAPS_FFB_SUPPORTED       0x0001

// Other:
//#define DLLExport // __declspec( dllexport )

#define DLL_DEBUG

/*
#ifdef DLL_DEBUG
	#define PROCESS_MANAGER_DEBUG
#endif
*/

// Includes:
// Nothing so far.

// Windows-specific:
#include <windows.h>

// XInput related:
#include "XinputInterface.h"
#include "Real_XInput_Wrapper.h"

// Other:
#include "processManagement.h"