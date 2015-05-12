#pragma once

// Preprocessor related:
#define XINPUT_INJECTOR

#define GAMEPAD_EXTERNAL_ORIGIN

// XInput macros:
#define XINPUT_DEVTYPE_GAMEPAD			0x01
#define XINPUT_DEVSUBTYPE_GAMEPAD		0x01
#define XINPUT_CAPS_FFB_SUPPORTED       0x0001

// Other:
//#define DLLExport // __declspec( dllexport )

//#define DLL_DEBUG
//#define DLL_CONFIRMATION_MESSAGE

// Includes:

// Windows-specific:
#include <windows.h>

// XInput related:
#include "XinputInterface.h"
#include "devices/native/winnt/Real_XInput_Wrapper.h"

// Other:
#include "native/winnt/processManagement.h"