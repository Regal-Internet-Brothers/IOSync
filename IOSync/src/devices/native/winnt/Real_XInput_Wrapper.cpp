// Preprocessor related:

//#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

// This enables console-output when injected XInput commands are executed (Virtual or real).
// This also applies to IOSync's general XInput support.
//#define DEBUG_XINPUT_CALLS

// Includes:

// The main interface-header.
#include <windows.h>

// The actual Xinput header.
#include <Xinput.h>

#include "Real_XInput_Wrapper.h"

// Standard library:
#include <string>
//#include <sstream>

//#ifdef DEBUG_XINPUT_CALLS
#include <iostream>
//#endif

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

	// Global variable(s):
	jumpMap jumpStates;

	// Functions:

	// Meta:
	process::jumpSegment mapRemoteFunction(LPCSTR name, LPCVOID function, HMODULE hDLL)
	{
		using namespace process;

		#ifdef PROCESS_MANAGER_DEBUG
			cout << "{XINPUT}: Attempting to map '" << function << "' to: '" << name << "'" << endl;
		#endif

		auto remoteFunction = getRemoteFunction(name, hDLL);

		if (remoteFunction == NULL)
		{
			#ifdef PROCESS_MANAGER_DEBUG
				cout << "Mapping failed; unable to find remote function."
			#endif

			// For now, do nothing.
			return jumpSegment();
		}

		auto response = writeJump(remoteFunction, function);

		jumpStates[remoteFunction] = response;

		return response;
	}

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
		#ifdef DEBUG_XINPUT_CALLS
			cout << "XInputEnable(" << enable << ")" << endl;
		#endif

		if (_XInputEnable != nullptr)
		{
			if (shouldFixJumps())
			{
				auto entryIterator = jumpStates.find((LPVOID)_XInputEnable);

				if (entryIterator != jumpStates.end())
				{
					jumpEntry entry = *entryIterator;

					swapJumps(entry);
					
					_XInputEnable(enable);

					swapJumps(entry);

					return;
				}
			}

			_XInputEnable(enable);

			return;
		}

		return;
	}

	DWORD WINAPI XInputSetState(DWORD dwUserIndex, PXINPUT_VIBRATION pVibration)
	{
		#ifdef DEBUG_XINPUT_CALLS
			cout << "XInputSetState(" << dwUserIndex << ", " << pVibration << ")" << endl;
		#endif

		if (_XInputSetState != nullptr)
		{
			if (shouldFixJumps())
			{
				auto entryIterator = jumpStates.find((LPVOID)_XInputSetState);

				if (entryIterator != jumpStates.end())
				{
					jumpEntry entry = *entryIterator;

					swapJumps(entry);
					
					auto response = _XInputSetState(dwUserIndex, pVibration);

					swapJumps(entry);

					return response;
				}
			}

			return _XInputSetState(dwUserIndex, pVibration);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState)
	{
		#ifdef DEBUG_XINPUT_CALLS
			cout << "XInputGetState(" << dwUserIndex << ", " << pState << ")" << endl;
			cout << "_XInputGetState: " << _XInputGetState << endl;
		#endif

		if (_XInputGetState != nullptr)
		{
			if (shouldFixJumps())
			{
				#ifdef DEBUG_XINPUT_CALLS
					cout << "Jump fixing..." << endl;
				#endif

				auto entryIterator = jumpStates.find((LPVOID)_XInputGetState);

				if (entryIterator != jumpStates.end())
				{
					#ifdef DEBUG_XINPUT_CALLS
						cout << "__XInputGetState? - " << entryIterator->first << endl;
					#endif

					jumpEntry entry = *entryIterator;

					swapJumps(entry);
					
					auto response = _XInputGetState(dwUserIndex, pState);

					swapJumps(entry);

					return response;
				}
			}

			return _XInputGetState(dwUserIndex, pState);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, PXINPUT_CAPABILITIES pCapabilities)
	{
		#ifdef DEBUG_XINPUT_CALLS
			cout << "XInputGetCapabilities(" << dwUserIndex << ", " << dwFlags << ", " << pCapabilities << ")" << endl;
		#endif

		if (_XInputGetCapabilities != nullptr)
		{
			if (shouldFixJumps())
			{
				auto entryIterator = jumpStates.find((LPVOID)_XInputGetCapabilities);

				if (entryIterator != jumpStates.end())
				{
					jumpEntry entry = *entryIterator;

					swapJumps(entry);
					
					auto response = _XInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);

					swapJumps(entry);

					return response;
				}
			}

			return _XInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType, PXINPUT_BATTERY_INFORMATION battery)
	{
		#ifdef DEBUG_XINPUT_CALLS
			cout << "XInputGetBatteryInformation(" << dwUserIndex << ", " << devType << ", " << battery << ")" << endl;
		#endif

		if (_XInputGetBatteryInformation != nullptr)
		{
			if (shouldFixJumps())
			{
				auto entryIterator = jumpStates.find((LPVOID)_XInputGetBatteryInformation);

				if (entryIterator != jumpStates.end())
				{
					jumpEntry entry = *entryIterator;

					swapJumps(entry);
					
					auto response = _XInputGetBatteryInformation(dwUserIndex, devType, battery);

					swapJumps(entry);

					return response;
				}
			}

			return _XInputGetBatteryInformation(dwUserIndex, devType, battery);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeyStroke)
	{
		#ifdef DEBUG_XINPUT_CALLS
			cout << "XInputGetKeystroke(" << dwUserIndex << ", " << dwReserved << ", " << pKeyStroke << ")" << endl;
		#endif

		if (_XInputGetKeystroke != nullptr)
		{
			if (shouldFixJumps())
			{
				auto entryIterator = jumpStates.find((LPVOID)_XInputGetKeystroke);

				if (entryIterator != jumpStates.end())
				{
					jumpEntry entry = *entryIterator;

					swapJumps(entry);
					
					auto response = _XInputGetKeystroke(dwUserIndex, dwReserved, pKeyStroke);

					swapJumps(entry);

					return response;
				}
			}

			return _XInputGetKeystroke(dwUserIndex, dwReserved, pKeyStroke);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId, PUINT renderCount, LPWSTR captureDeviceId, PUINT captureCount)
	{
		#ifdef DEBUG_XINPUT_CALLS
			cout << "XInputGetAudioDeviceIds(" << dwUserIndex << ", " << pRenderDeviceId << ", " << renderCount << ", " << captureDeviceId << ", " << captureCount << ")" << endl;
		#endif

		if (_XInputGetAudioDeviceIds != nullptr)
		{
			if (shouldFixJumps())
			{
				auto entryIterator = jumpStates.find((LPVOID)_XInputGetAudioDeviceIds);

				if (entryIterator != jumpStates.end())
				{
					jumpEntry entry = *entryIterator;

					swapJumps(entry);
					
					auto response = _XInputGetAudioDeviceIds(dwUserIndex, pRenderDeviceId, renderCount, captureDeviceId, captureCount);

					swapJumps(entry);

					return response;
				}
			}

			return _XInputGetAudioDeviceIds(dwUserIndex, pRenderDeviceId, renderCount, captureDeviceId, captureCount);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}

	DWORD WINAPI XInputGetStateEx(DWORD dwUserIndex, PXINPUT_STATE pState)
	{
		#ifdef DEBUG_XINPUT_CALLS
			cout << "XInputGetStateEx(" << dwUserIndex << ", " << pState << ")" << endl;
		#endif

		if (_XInputGetStateEx != nullptr)
		{
			if (shouldFixJumps())
			{
				auto entryIterator = jumpStates.find((LPVOID)_XInputGetStateEx);

				if (entryIterator != jumpStates.end())
				{
					jumpEntry entry = *entryIterator;

					swapJumps(entry);
					
					auto response = _XInputGetStateEx(dwUserIndex, pState);

					swapJumps(entry);

					return response;
				}
			}

			return _XInputGetStateEx(dwUserIndex, pState);
		}

		return ERROR_DEVICE_NOT_CONNECTED;
	}
}