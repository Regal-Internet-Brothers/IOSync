// Preprocessor related:
#define _CRT_SECURE_NO_WARNINGS

// Includes:
#include <windows.h>

#include "XInput_Injector.h"

#include <string>
#include <sstream>
#include <climits>

// Standard gamepad functionality.
#include "devices/gamepad.h"

#if defined(PLATFORM_X86)
	#define FILE_NAME INJECTION_DLL_NAME_X86
#elif defined(PLATFORM_X64)
	#define FILE_NAME INJECTION_DLL_NAME_X64
#elif defined(PLATFORM_ARM)
	#define INJECTION_DLL_NAME_ARM "xinput_injector_ARM.dll"
#elif defined(PLATFORM_ARM64)
	#define INJECTION_DLL_NAME_ARM64 "xinput_injector_ARM64.dll"
#endif

// Namespace(s):
using namespace iosync::devices;

// Internal:

// Global variable(s):

// Shared-memory related:
LPVOID sharedBuffer = NULL;

XINPUT_STATE* gamepadStates = nullptr;
bool* pluggedIn = nullptr;

HANDLE gamepad::sharedMemory = NULL;

// Functions:
void detachSharedMemory()
{
	// Un-map the shared memory segment.
	gamepad::__winnt__unmapSharedMemory(sharedBuffer);

	// Close the shared memory connection.
	gamepad::__winnt__closeSharedMemory();

	return;
}

// External / Exported:
extern "C"
{
	// Global variable(s):

	// This is used to determine if a module is an injector.
	extern BOOL XINPUT_INJECTOR_VALIDATOR = TRUE;

	// A global variable representing the real 'XInput' module.
	HMODULE XINPUT_MODULE = NULL;

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

		// Nothing so far.

		return ERROR_EMPTY;
	}

	DWORD WINAPI XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId, PUINT renderCount, LPWSTR captureDeviceId, PUINT captureCount)
	{
		if (!pluggedIn[dwUserIndex])
		{
			return REAL_XINPUT::XInputGetAudioDeviceIds(dwUserIndex, pRenderDeviceId, renderCount, captureDeviceId, captureCount);
		}

		// Nothing so far.

		return ERROR_SUCCESS;
	}

	DWORD WINAPI XInputSetState(DWORD dwUserIndex, PXINPUT_VIBRATION pVibration)
	{
		if (!pluggedIn[dwUserIndex])
		{
			return REAL_XINPUT::XInputSetState(dwUserIndex, pVibration);
		}

		//ZeroVariable(*pVibration);

		return ERROR_SUCCESS;
	}

	DWORD WINAPI XInputGetState(DWORD dwUserIndex, PXINPUT_STATE pState)
	{
		if (pluggedIn[dwUserIndex])
		{
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
			pCapabilities->Flags = XINPUT_CAPS_FFB_SUPPORTED; // 0

			pCapabilities->Gamepad.sThumbLX = SHRT_MAX;
			pCapabilities->Gamepad.sThumbLY = SHRT_MAX;
			pCapabilities->Gamepad.sThumbRX = SHRT_MAX;
			pCapabilities->Gamepad.sThumbRY = SHRT_MAX;
			pCapabilities->Gamepad.bLeftTrigger = UCHAR_MAX;
			pCapabilities->Gamepad.bRightTrigger = UCHAR_MAX;

			pCapabilities->Gamepad.wButtons = USHRT_MAX;

			return ERROR_SUCCESS;
		}

		return REAL_XINPUT::XInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);
	}

	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
	{
		// Namespace(s):
		using namespace std;
		using namespace process;

		switch (fdwReason)
		{
			case DLL_PROCESS_ATTACH:
				{
					#ifdef DLL_DEBUG
						AllocConsole();
						freopen("CONOUT$", "w", stdout);
					#endif

					auto sharedMemoryResponse = gamepad::__winnt__openSharedMemory();

					if (sharedMemoryResponse == SHARED_MEMORY_ALLOCATED || sharedMemoryResponse == SHARED_MEMORY_ALREADY_ALLOCATED)
					{
						#ifdef DLL_DEBUG
							auto PID = getPID();
						#endif

						sharedBuffer = gamepad::__winnt__mapSharedMemory(FILE_MAP_READ);

						if (sharedBuffer == NULL)
							return FALSE;

						pluggedIn = (bool*)gamepad::__winnt__sharedMemory_getPluggedInOffset(sharedBuffer);
						gamepadStates = (nativeGamepad*)gamepad::__winnt__sharedMemory_getStatesOffset(sharedBuffer);

						bool injected = false;
						bool shouldInject = true;

						char DLLName[XINPUT_DLL_NAMELENGTH];

						for (auto i = XINPUT_MAX_SUBVERSION; i > 0; i--)
						{
							if (i == 0)
							{
								sprintf(DLLName, XINPUT_COMPATIBILITY_DLL);
							}
							else
							{
								sprintf(DLLName, "xinput1_%d.dll", i);
							}

							#ifdef DLL_DEBUG
								cout << "Attempting to find DLL: " << "\"" << DLLName  << "\"..." << endl;
							#endif

							HMODULE hDLL = GetModuleHandleA((LPCSTR)DLLName);

							if (hDLL != NULL)
							{
								#ifdef DLL_DEBUG
									cout << "Requested DLL found." << endl;
								#endif

								// Check if this DLL was used for injection:
								if (hDLL == hinstDLL || isInjectionDLL(hDLL))
								{
									#ifdef DLL_DEBUG
										cout << "Injection unneeded; resolving real module." << endl;
									#endif

									// Make sure we don't inject into this module.
									shouldInject = false;
								}

								// Check if we need to inject into the active library:
								if (shouldInject)
								{
									#ifdef DLL_DEBUG
										cout << "Injecting functions..." << endl;
									#endif
									//jumpSegment
									// Remap the active functions in the remote library:
									REAL_XINPUT::mapRemoteFunction("XInputEnable", (LPVOID)::XInputEnable, hDLL);
									REAL_XINPUT::mapRemoteFunction("XInputGetState", (LPVOID)::XInputGetState, hDLL);
									REAL_XINPUT::mapRemoteFunction("XInputSetState", (LPVOID)::XInputSetState, hDLL);
									REAL_XINPUT::mapRemoteFunction("XInputGetCapabilities", (LPVOID)::XInputGetCapabilities, hDLL);
									REAL_XINPUT::mapRemoteFunction("XInputGetKeystroke", (LPVOID)::XInputGetKeystroke, hDLL);
									REAL_XINPUT::mapRemoteFunction("XInputGetBatteryInformation", (LPVOID)::XInputGetBatteryInformation, hDLL);
									REAL_XINPUT::mapRemoteFunction("XInputGetAudioDeviceIds", (LPVOID)::XInputGetAudioDeviceIds, hDLL);

									// Extensions:
									REAL_XINPUT::mapRemoteFunction(REAL_XINPUT::XInputGetStateEx_Ordinal(), (LPVOID)::XInputGetStateEx, hDLL); // "XInputGetStateEx"

									#ifdef DLL_DEBUG
										cout << "Functions injected." << endl;

										cout << "Attempting to link with real 'XInput' module..." << endl;
									#endif

									// Safely link to the real module.
									REAL_XINPUT::linkTo(hDLL);

									// Set the global XInput-module.
									XINPUT_MODULE = hDLL;

									// Set the injection-flag to 'true'.
									injected = true;
								}
								else
								{
									// Link to an 'XInput' DLL in the system-folder.
									XINPUT_MODULE = REAL_XINPUT::linkTo((PCSTR)DLLName, TRUE);
								}

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
								cout << "Injected into process (PID): " << PID << endl;
							#endif
						}
						else if (shouldInject)
						{
							#ifdef DLL_DEBUG
								cout << "All attempts failed; unable to inject into process (PID): " << PID << endl;
							#endif

							return FALSE;
						}

						#ifdef DLL_CONFIRMATION_MESSAGE
							MessageBox(NULL, TEXT("XInput Initialized."), TEXT("IOSync"), MB_OK);
						#endif
					}
				}

				break;
			case DLL_PROCESS_DETACH:
				// Make sure to free this module,
				// otherwise, if the host program tries to link with XInput
				// again, it might automatically use this module instead.
				if (XINPUT_MODULE != NULL)
					FreeLibrary(XINPUT_MODULE);

				#ifdef DLL_DEBUG
					cout << "Disconnecting from process." << endl;
					
					fclose(stdout);
					FreeConsole();
				#endif

				detachSharedMemory();

				break;
			case DLL_THREAD_ATTACH:
				// Nothing so far.

				break;
			case DLL_THREAD_DETACH:
				// Nothing so far.

				break;
		}

		return TRUE;
	}
}

/*
int main()
{
	// Nothing so far.

	return 0;
}
*/