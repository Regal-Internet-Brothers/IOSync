// Preprocessor related:

// Includes:
#include "XInput_Injector.h"

// Standard library:
#include <iostream>

#include <thread>
#include <chrono>

// Standard gamepad functionality.
#include "gamepad.h"

#if defined(_WIN64) && defined(_M_AMD64)
	#define FILE_NAME INJECTION_DLL_NAME_X64
#elif defined(_WIN32) && defined(_M_IX86)
	#define FILE_NAME INJECTION_DLL_NAME_X86
#else
	#define FILE_NAME "xinput_injector.dll"
#endif

// Namespace(s):
using namespace iosync::devices;

// Forward declarations:
static inline LPCSTR XInputGetStateEx_Ordinal();

void detachSaredMemory();

// Exports:
extern "C"
{
	// Global variable(s):
	LPVOID sharedBuffer = NULL;
	XINPUT_STATE* gamepadStates = nullptr;
	bool* pluggedIn = nullptr; // MAX_GAMEPADS

	HANDLE gamepad::sharedMemory = NULL;
	HMODULE realModule = NULL;

	// This is used to determine if this DLL was injected as a replacement module.
	BOOL isMasterDLL = FALSE;

	BOOL XINPUT_INJECTOR_VALIDATOR = TRUE;

	// Functions:
	VOID WINAPI XInputEnable(BOOL enable)
	{
		REAL_XINPUT::XInputEnable(enable);

		return;
	}

	DWORD WINAPI XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType, PXINPUT_BATTERY_INFORMATION battery)
	{
		if (!pluggedIn[dwUserIndex])
		{
			return REAL_XINPUT::XInputGetBatteryInformation(dwUserIndex, devType, battery);
		}

		ZeroVariable(*battery);

		return ERROR_SUCCESS;
	}

	DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeyStroke)
	{
		if (!pluggedIn[dwUserIndex])
		{
			return REAL_XINPUT::XInputGetKeystroke(dwUserIndex, dwReserved, pKeyStroke);
		}

		return ERROR_EMPTY;
	}

	DWORD WINAPI XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId, PUINT renderCount, LPWSTR captureDeviceId, PUINT captureCount)
	{
		if (!pluggedIn[dwUserIndex])
		{
			return REAL_XINPUT::XInputGetAudioDeviceIds(dwUserIndex, pRenderDeviceId, renderCount, captureDeviceId, captureCount);
		}

		return ERROR_SUCCESS;
	}

	DWORD WINAPI XInputSetState(DWORD dwUserIndex, PXINPUT_VIBRATION pVibration)
	{
		//ZeroVariable(*pVibration);

		// Check if a virtual gamepad is plugged in:
		if (!pluggedIn[dwUserIndex])
		{
			// There isn't a virtual gamepad, send the
			// vibration information to the real device.
			return REAL_XINPUT::XInputSetState(dwUserIndex, pVibration);
		}

		return ERROR_SUCCESS;
	}

	DWORD WINAPI XInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState)
	{
		if (pluggedIn[dwUserIndex])
		{
			//ZeroMemory(pState, sizeof(XINPUT_STATE));
			*pState = gamepadStates[dwUserIndex];

			return ERROR_SUCCESS;
		}

		return REAL_XINPUT::XInputGetState(dwUserIndex, pState);
	}

	DWORD WINAPI XInputGetStateEx(DWORD dwUserIndex, PXINPUT_STATE pState)
	{
		if (pluggedIn[dwUserIndex])
		{
			return ::XInputGetState(dwUserIndex, pState);
		}

		return REAL_XINPUT::XInputGetStateEx(dwUserIndex, pState);
	}

	DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, PXINPUT_CAPABILITIES pCapabilities)
	{
		if (pluggedIn[dwUserIndex])
		{
			ZeroVariable(*pCapabilities);

			pCapabilities->Type = XINPUT_DEVTYPE_GAMEPAD;
			pCapabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
			pCapabilities->Flags = 0; // XINPUT_CAPS_FFB_SUPPORTED;

			XINPUT_STATE state;

			::XInputGetState(dwUserIndex, &state);

			pCapabilities->Gamepad = state.Gamepad;

			return ERROR_SUCCESS;
		}

		return REAL_XINPUT::XInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);
	}
	
	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
	{
		using namespace std;
		using namespace process;

		switch (fdwReason)
		{
			case DLL_PROCESS_ATTACH:
				{
					// Ensure there aren't other injection DLLs:
					{
						HMODULE copyModule = GetModuleHandleA(FILE_NAME);

						if (copyModule != NULL && copyModule != hinstDLL && isInjectionDLL(copyModule))
						{
							//isMasterDLL = FALSE;

							return FALSE;
						}
					}

					#ifdef DLL_DEBUG
						AllocConsole();
						freopen("CONOUT$", "w", stdout);
					#endif

					auto sharedMemoryResponse = gamepad::__winnt__openSharedMemory();

					if (sharedMemoryResponse == SHARED_MEMORY_ALLOCATED || sharedMemoryResponse == SHARED_MEMORY_ALREADY_ALLOCATED)
					{
						auto PID = getPID();

						LPVOID buffer = gamepad::__winnt__mapSharedMemory();

						if (buffer == NULL)
							return FALSE;

						sharedBuffer = buffer;

						gamepadStates = (nativeGamepad*)gamepad::__winnt__sharedMemory_getStatesOffset(buffer);
						pluggedIn = (bool*)gamepad::__winnt__sharedMemory_getPluggedInOffset(buffer);

						bool injected = false;

						char DLLName[XINPUT_DLL_NAMELENGTH];

						//sprintf(DLLName, INJECTOR_XINPUT_DEFAULT_DLL_A);

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

							HMODULE hDLL = GetModuleHandleA((LPCSTR)DLLName);

							// Check if this module is the same as the detected DLL:
							if (hinstDLL == hDLL || isInjectionDLL(hDLL) != NULL)
							{
								#ifdef DLL_DEBUG
									cout << "Infinite injection detected, continuing without remapping functions." << endl;
								#endif

								// It's the same as this module, fake the "injection".
								injected = true;

								isMasterDLL = TRUE;
								//blacklistedModuleName = DLLName;

								// Skip the rest of the loop.
								break;
							}

							#ifdef DLL_DEBUG
								cout << "Attempting to inject into DLL: " << "\"" << DLLName  << "\"..." << endl;
							#endif

							if (hDLL != NULL)
							{
								#ifdef DLL_DEBUG
									cout << "Module found, attempting to link commands..." << endl;
								#endif

								mapRemoteFunction(XInputGetStateEx_Ordinal(), (LPVOID)::XInputGetStateEx, hDLL); // 'XInputGetStateEx'.

								mapRemoteFunction("XInputEnable", (LPVOID)::XInputEnable, hDLL);
								mapRemoteFunction("XInputGetState", (LPVOID)::XInputGetState, hDLL);
								mapRemoteFunction("XInputSetState", (LPVOID)::XInputSetState, hDLL);
								mapRemoteFunction("XInputGetCapabilities", (LPVOID)::XInputGetCapabilities, hDLL);
								mapRemoteFunction("XInputGetBatteryInformation", (LPVOID)::XInputGetBatteryInformation, hDLL);
								mapRemoteFunction("XInputGetKeystroke", (LPVOID)::XInputGetKeystroke, hDLL);
								mapRemoteFunction("XInputGetAudioDeviceIds", (LPVOID)::XInputGetAudioDeviceIds, hDLL);

								injected = true;

								#ifdef DLL_DEBUG
									cout << "Commands linked." << endl;
								#endif

								break;
							}
							else
							{
								#ifdef DLL_DEBUG
									cout << "Unable to inject into DLL (\"" << DLLName << "\")" << endl;
								#endif
							}
						}

						if (injected)
						{
							#ifdef DLL_DEBUG
								cout << "Attempting to link with a real version of 'XInput'..." << endl;
							#endif

							realModule = REAL_XINPUT::linkTo(isMasterDLL);

							// Attempt to dynamically link to this XInput DLL.
							if (realModule != NULL) // INJECTOR_XINPUT_DEFAULT_DLL_A
							{
								#ifdef DLL_DEBUG
									cout << "Real version linked." << endl;
									cout << "Injected into process (PID): " << PID << endl;
								#endif
							}
							else
							{
								cout << "Unable to link to real 'XInput' version." << endl;
							}
						}
						else
						{
							#ifdef DLL_DEBUG
								cout << "All attempts failed; unable to inject into process (PID): " << PID << endl;
							#endif
						}
					}
					else
					{
						if (sharedMemoryResponse == SHARED_MEMORY_UNAVAILABLE)
						{
							cout << "Unable to use shared memory; injection skipped." << endl;
						}
						else
						{
							detachSaredMemory();

							#ifdef DLL_DEBUG
								cout << "An unknown error has occurred." << endl;
							#endif
						}
					}
				}

				break;
			case DLL_PROCESS_DETACH:
				if (isMasterDLL)
				{
					// This is a fix for programs that dynamically link this.
					// Not the best of hacks, but it works.
					return FALSE;
					//return TRUE;
				}

				#ifdef DLL_DEBUG
					//cout << "Disconnecting from process..." << endl;
				#endif

				if (realModule != NULL)
					FreeLibrary(realModule);

				detachSaredMemory();

				#ifdef DLL_DEBUG
					//FreeConsole();
				#endif

				break;
			case DLL_THREAD_ATTACH:
				// Nothing so far.

				break;
			case DLL_THREAD_DETACH:
				break;
		}

		return TRUE;
	}
}

// Non-exported functions:

// This does not normally return a proper string.
static inline LPCSTR XInputGetStateEx_Ordinal()
{
	return (LPCSTR)100;
}

void detachSaredMemory()
{
	#ifdef DLL_DEBUG
		//cout << "Detaching shared memory..." << endl;
	#endif

	gamepad::__winnt__unmapSharedMemory(sharedBuffer);
	gamepad::__winnt__closeSharedMemory();

	#ifdef DLL_DEBUG
		//cout << "Shared memory detached." << endl;
	#endif

	return;
}

/*
int main()
{
	// Nothing so far.

	return 0;
}
*/