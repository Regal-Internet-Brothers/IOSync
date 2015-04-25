#pragma once

// Includes:
#include <windows.h>

#ifdef XINPUT_INJECTOR
	#include "XinputInterface.h"
#else
	#include <Xinput.h>
#endif

// Namespace(s):
namespace REAL_XINPUT
{
	//#include <Xinput.h>

	// Functions:
	inline LPCSTR XInputGetStateEx_Ordinal()
	{
		return (LPCSTR)100;
	}

	VOID linkTo(HMODULE hDLL);
	
	// The 'resolveSystemPath' argument specifies if this routine
	// should add the system-directory to the path.
	HMODULE linkTo(LPCSTR DLL_Location, BOOL resolveSystemPath=FALSE);

	HMODULE linkTo(BOOL resolveSystemPath=FALSE);

	VOID WINAPI XInputEnable(BOOL enable);
	DWORD WINAPI XInputSetState(DWORD dwUserIndex, PXINPUT_VIBRATION pVibration);
	DWORD WINAPI XInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState);
	DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, PXINPUT_CAPABILITIES pCapabilities);
	DWORD WINAPI XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType, PXINPUT_BATTERY_INFORMATION battery);
	DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeyStroke);
	DWORD WINAPI XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId, PUINT renderCount, LPWSTR captureDeviceId, PUINT captureCount);
	DWORD WINAPI XInputGetStateEx(DWORD dwUserIndex, PXINPUT_STATE pState);
}