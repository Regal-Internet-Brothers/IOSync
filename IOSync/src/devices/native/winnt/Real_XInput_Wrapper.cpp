// Preprocessor related:
//#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

// Includes:

// The main interface-header.
#include <windows.h>

// The actual Xinput header.
#include <Xinput.h>

#include "Real_XInput_Wrapper.h"
#include "../../../native/winnt/processManagement.h"

// Standard library:
#include <string>
//#include <sstream>

// Namespace(s):
namespace REAL_XINPUT
{
	// Typedefs:
	typedef decltype(&XInputEnable) _XInputEnable_t;
	typedef decltype(&XInputSetState) _XInputSetState_t;
	typedef decltype(&XInputGetState) _XInputGetState_t;
	typedef decltype(&XInputGetCapabilities) _XInputGetCapabilities_t;
	typedef decltype(&XInputGetBatteryInformation) _XInputGetBatteryInformation_t;
	typedef decltype(&XInputGetKeystroke) _XInputGetKeystroke_t;
	typedef decltype(&XInputGetAudioDeviceIds) _XInputGetAudioDeviceIds_t;
	typedef decltype(&XInputGetState) _XInputGetStateEx_t; // XInputGetStateEx

	_XInputEnable_t _XInputEnable;
	_XInputSetState_t _XInputSetState;
	_XInputGetState_t _XInputGetState;
	_XInputGetCapabilities_t _XInputGetCapabilities;
	_XInputGetBatteryInformation_t _XInputGetBatteryInformation;
	_XInputGetKeystroke_t _XInputGetKeystroke;
	_XInputGetAudioDeviceIds_t _XInputGetAudioDeviceIds;
	_XInputGetStateEx_t _XInputGetStateEx;

	// Functions:

	// Meta:
	VOID linkTo(HMODULE hDLL)
	{
		using namespace process;

		_XInputEnable = (_XInputEnable_t)getRemoteFunction("XInputEnable", hDLL);
		_XInputSetState = (_XInputSetState_t)getRemoteFunction("XInputSetState", hDLL);
		_XInputGetState = (_XInputGetState_t)getRemoteFunction("XInputGetState", hDLL);
		_XInputGetCapabilities = (_XInputGetCapabilities_t)getRemoteFunction("XInputGetCapabilities", hDLL);
		_XInputGetBatteryInformation = (_XInputGetBatteryInformation_t)getRemoteFunction("XInputGetBatteryInformation", hDLL);
		_XInputGetKeystroke = (_XInputGetKeystroke_t)getRemoteFunction("XInputGetKeystroke", hDLL);
		_XInputGetAudioDeviceIds = (_XInputGetAudioDeviceIds_t)getRemoteFunction("XInputGetAudioDeviceIds", hDLL);

		// Attempt to get the hidden 'XInputGetStateEx' function.
		_XInputGetStateEx = (_XInputGetStateEx_t)getRemoteFunction(XInputGetStateEx_Ordinal(), hDLL);

		// Check if we were able to find it:
		if (_XInputGetStateEx == nullptr)
		{
			// Try getting the function normally (Normally doesn't work):
			_XInputGetStateEx = (_XInputGetStateEx_t)getRemoteFunction("XInputGetStateEx", hDLL);

			if (_XInputGetStateEx == nullptr)
			{
				// If all else fails, attach to the standard version.
				_XInputGetStateEx = _XInputGetState;
			}
		}

		return;
	}

	HMODULE linkTo(LPCSTR DLL_Location, BOOL resolveSystemPath)
	{
		using namespace std;

		HMODULE library = NULL;

		if (resolveSystemPath == TRUE)
		{
			library = LoadLibraryA((process::resolveSystemPath(DLL_Location)).c_str());
		}
		else
		{
			library = LoadLibraryA(DLL_Location);
		}

		if (library == NULL)
			return NULL;

		// Call the main implementation using the loaded module.
		linkTo(library);

		// Return the loaded module.
		return library;
	}

	HMODULE linkTo(BOOL resolveSystemPath)
	{
		// Not the most efficient, but it works:
		char DLLName[XINPUT_DLL_NAMELENGTH];

		for (auto i = XINPUT_MAX_SUBVERSION; i >= 0; i--)
		{
			if (i == 0)
			{
				sprintf(DLLName, XINPUT_COMPATIBILITY_DLL);
			}
			else
			{
				sprintf(DLLName, "xinput1_%d.dll", i);
			}

			HMODULE module = linkTo(DLLName, resolveSystemPath);

			if (module != NULL)
			{
				return module;
			}

			//sprintf(DLLName, "xinput1_%d.dll", i);
		}

		return NULL;
	}

	// Wrapper / "shim":
	VOID WINAPI XInputEnable(BOOL enable)
	{
		if (_XInputEnable != nullptr)
		{	
			_XInputEnable(enable);

			return;
		}

		return;
	}

	DWORD WINAPI XInputSetState(DWORD dwUserIndex, PXINPUT_VIBRATION pVibration)
	{
		if (_XInputSetState != nullptr)
		{
			return _XInputSetState(dwUserIndex, pVibration);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState)
	{
		if (_XInputGetState != nullptr)
		{
			return _XInputGetState(dwUserIndex, pState);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, PXINPUT_CAPABILITIES pCapabilities)
	{
		using namespace std;

		if (_XInputGetCapabilities != nullptr)
		{
			return _XInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType, PXINPUT_BATTERY_INFORMATION battery)
	{
		if (_XInputGetBatteryInformation != nullptr)
		{
			return _XInputGetBatteryInformation(dwUserIndex, devType, battery);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeyStroke)
	{
		if (_XInputGetKeystroke != nullptr)
		{
			return _XInputGetKeystroke(dwUserIndex, dwReserved, pKeyStroke);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId, PUINT renderCount, LPWSTR captureDeviceId, PUINT captureCount)
	{
		if (_XInputGetAudioDeviceIds != nullptr)
		{
			return _XInputGetAudioDeviceIds(dwUserIndex, pRenderDeviceId, renderCount, captureDeviceId, captureCount);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetStateEx(DWORD dwUserIndex, PXINPUT_STATE pState)
	{
		if (_XInputGetState != nullptr)
		{
			return _XInputGetState(dwUserIndex, pState);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}
}