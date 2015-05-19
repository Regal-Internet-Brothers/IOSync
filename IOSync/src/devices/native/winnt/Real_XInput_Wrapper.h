#pragma once

// Includes:
#include "../../../application/native/winnt/processManagement.h"

#include <windows.h>

#ifdef XINPUT_INJECTOR
	#include "XinputInterface.h"
#else
	#include <Xinput.h>
#endif

// Standard library:
#include <map>

// Namespace(s):
namespace REAL_XINPUT
{
	// Namespace(s):
	using namespace std;

	// Typedefs:
	typedef pair<LPVOID, process::jumpSegment> jumpEntry;
	typedef map<LPVOID, process::jumpSegment> jumpMap;

	// Global variable(s):
	extern jumpMap jumpStates;

	// Functions:
	inline bool shouldFixJumps()
	{
		return !jumpStates.empty();
	}

	inline void swapJumps(jumpEntry& entry)
	{
		entry.second = process::swapJumpInstruction(entry.first, entry.second);

		return;
	}

	inline LPCSTR XInputGetStateEx_Ordinal()
	{
		return (LPCSTR)100;
	}

	process::jumpSegment mapRemoteFunction(LPCSTR name, LPCVOID function, HMODULE hDLL);

	// This command restores the internal function-pointers to their original state.
	BOOL restoreFunctions();

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