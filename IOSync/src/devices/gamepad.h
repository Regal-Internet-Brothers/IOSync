#pragma once

// Preprocessor related:
#define GAMEPAD_DEBUG
#define GAMEPAD_DEBUG_RUMBLE

#define GAMEPAD_NETWORK_SAFE

#ifndef GAMEPAD_NETWORK_SAFE
	#define GAMEPAD_FAST_NETWORK_IO
#endif

// Includes:
#include "../platform.h"

#if defined(PLATFORM_X86) || defined(PLATFORM_X64)
	#define INJECTION_DLL_NAME_X86 "xinput_injector_x86.dll"
	#define INJECTION_DLL_NAME_X64 "xinput_injector_x64.dll"
#elif defined(PLATFORM_ARM) || defined(PLATFORM_ARM64)
	#define INJECTION_DLL_NAME_ARM "xinput_injector_ARM.dll"
	#define INJECTION_DLL_NAME_ARM64 "xinput_injector_ARM64.dll"
#endif

#include "devices.h"

// Standard library:
#include <thread>
#include <chrono>

// DirectX:
#ifdef GAMEPAD_EXTERNAL_ORIGIN
	#include "XInput_Injector.h"

	using namespace REAL_XINPUT;
#endif

#if defined(PLATFORM_WINDOWS) && !defined(GAMEPAD_EXTERNAL_ORIGIN)
	//#include <Xinput.h>
	#include "native/winnt/Real_XInput_Wrapper.h"
#endif

// Standard library:
#include <climits>

// Libraries:
#if defined(PLATFORM_WINDOWS) && !defined(GAMEPAD_EXTERNAL_ORIGIN)
	//#pragma comment(lib, "Xinput.lib")
#endif

// Namespace(s):
namespace iosync
{
	namespace devices
	{
		// Namespace(s):
		using namespace std;
		using namespace chrono;

		#ifdef PLATFORM_WINDOWS
			//using namespace REAL_XINPUT;
		#endif

		// Forward declarations:
		struct gamepadState;

		// Typedefs:
		#ifdef PLATFORM_WINDOWS
			// This program describes the 'nativeGamepad'
			// as the XInput API's 'XINPUT_STATE' structure.
			typedef XINPUT_STATE nativeGamepad;

			typedef DWORD gamepadID;
		#else
			#error 'nativeGamepad' implementation required.
			
			typedef unsigned char gamepadID;
		#endif

		typedef unsigned char serializedGamepadID;

		// Enumerator(s):
		enum gamepadIDs : gamepadID
		{
			FIRST_PLAYER,
			SECOND_PLAYER,
			THIRD_PLAYER,
			FOURTH_PLAYER,
			GAMEPAD_ID_NONE,
		};

		enum metrics : unsigned long long
		{
			#ifdef PLATFORM_WINDOWS
				MAX_GAMEPADS = 4,
			#else
				MAX_GAMEPADS = 1, // 0,
			#endif

			DEFAULT_DEBUG_RUMBLE_TIME = 500,
		};

		#ifdef PLATFORM_WINDOWS
			enum sharedMemoryState
			{
				SHARED_MEMORY_UNAVAILABLE,
				SHARED_MEMORY_ALREADY_ALLOCATED,
				SHARED_MEMORY_ALLOCATED,
			};
		#endif

		// Constant variable(s):
		static const size_t serializedNativeGamepadSize = sizeof(nativeGamepad); // sizeof(nativeGamepad);

		#ifdef PLATFORM_WINDOWS
			static LPCTSTR SHARED_GAMEPAD_MEMORY_NAME = "IOSYNC_GAMEPAD_BUFFER";

			static const size_t SHARED_SIZEOF_PLUGGED_IN_SEGMENT = (sizeof(bool)*MAX_GAMEPADS);
			static const size_t SHARED_SIZEOF_GAMEPAD_STATES = (sizeof(nativeGamepad)*MAX_GAMEPADS);

			// The position where the "plugged in" segment begins.
			static const size_t SHARED_GAMEPAD_MEMORY_PLUGGED_IN_OFFSET = 0;

			// The position where the "states" segment begins.
			static const size_t SHARED_GAMEPAD_MEMORY_STATES_OFFSET = SHARED_SIZEOF_PLUGGED_IN_SEGMENT;

			// The position where the "plugged in" segment ends.
			static const size_t SHARED_GAMEPAD_MEMORY_PLUGGED_IN_MAX_SCOPE = SHARED_GAMEPAD_MEMORY_PLUGGED_IN_OFFSET+SHARED_SIZEOF_PLUGGED_IN_SEGMENT;

			// The position where the "states" segment ends.
			static const size_t SHARED_GAMEPAD_MEMORY_STATES_MAX_SCOPE = SHARED_GAMEPAD_MEMORY_STATES_OFFSET+SHARED_SIZEOF_GAMEPAD_STATES;

			// The size of the shared memory area.
			static const DWORD SHARED_GAMEPAD_MEMORY_SIZE = (SHARED_SIZEOF_PLUGGED_IN_SEGMENT+SHARED_SIZEOF_GAMEPAD_STATES);
		#endif

		// Structures:
		struct gamepadState
		{
			// Global variable(s):
			// Nothing so far.

			// Constructor(s):
			gamepadState(nativeGamepad rep=nativeGamepad());

			// Destructor(s):
			// Nothing so far.

			// Methods:
			void readFrom(QSocket& socket);
			void writeTo(QSocket& socket);

			// Fields:
			nativeGamepad native;
		};

		// Classes:
		class gamepad : public IODevice
		{
			public:
				// Global variable(s):
				#ifdef PLATFORM_WINDOWS
					static HANDLE sharedMemory;
				#endif

				// Functions:
				#ifdef PLATFORM_WINDOWS
					static inline DWORD __winnt__realDeviceState(gamepadID identifier, XINPUT_STATE& state)
					{
						// Make sure to "zero-initialize" our 'state' variable.
						ZeroVariable(state);

						return REAL_XINPUT::XInputGetStateEx((DWORD)identifier, &state); // XInputGetStateEx
					}

					static inline DWORD __winnt__realDeviceStateResponse(gamepadID identifier)
					{
						// Local variable(s):

						// Our targeted Xinput-device's state.
						XINPUT_STATE state;
					
						// Return the response-code from the main state-detection routine.
						return __winnt__realDeviceState(identifier, state);
					}

					// This command may be used to detect if a real gamepad is connected on the current system.
					static inline bool __winnt__pluggedIn(gamepadID identifier)
					{
						/*
						XINPUT_CAPABILITIES pCapabilities;
						ZeroVariable(pCapabilities);

						return (XInputGetCapabilities((DWORD)identifier, XINPUT_FLAG_GAMEPAD, &pCapabilities) != ERROR_DEVICE_NOT_CONNECTED);
						*/

						return (__winnt__realDeviceStateResponse(identifier) != ERROR_DEVICE_NOT_CONNECTED);
					}

					static inline DWORD __winnt__rumbleDevice(gamepadID identifier, XINPUT_VIBRATION vibration, milliseconds ms=(milliseconds)DEFAULT_DEBUG_RUMBLE_TIME)
					{
						// Local variable(s):
						auto response = REAL_XINPUT::XInputSetState(identifier, &vibration);

						if (response != ERROR_SUCCESS)
							return response;

						ZeroVariable(vibration);

						this_thread::sleep_for(ms);

						response = REAL_XINPUT::XInputSetState(identifier, &vibration);

						if (response != ERROR_SUCCESS)
							return response;

						return ERROR_SUCCESS;
					}

					static inline DWORD __winnt__rumbleDevice(gamepadID identifier, milliseconds ms=(milliseconds)DEFAULT_DEBUG_RUMBLE_TIME, WORD LMS=USHRT_MAX, WORD RMS=USHRT_MAX)
					{
						// Local variable(s):
						XINPUT_VIBRATION vibration;

						// "Zero out" the 'vibration' variable.
						ZeroVariable(vibration);

						// Set the vibration settings to what was specified.
						vibration.wLeftMotorSpeed = LMS;
						vibration.wRightMotorSpeed = RMS;

						// Call the main implementation.
						return __winnt__rumbleDevice(identifier, vibration, ms);
					}

					// This will inject an injection DLL/module based on the architecture specified.
					static bool __winnt__injectLibrary(DWORD processID, CPUArchitecture process_Architecture);

					// This will detect the architecture of the process specified, then use the correct DLL/module accordingly.
					static bool __winnt__injectLibrary(DWORD processID);

					static inline sharedMemoryState __winnt__openSharedMemory()
					{
						sharedMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_GAMEPAD_MEMORY_NAME);

						if (sharedMemory == NULL)
						{
							sharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHARED_GAMEPAD_MEMORY_SIZE, SHARED_GAMEPAD_MEMORY_NAME);

							if (sharedMemory == NULL)
								return SHARED_MEMORY_UNAVAILABLE;
						}
						else
						{
							return SHARED_MEMORY_ALREADY_ALLOCATED;
						}

						// Return the default response.
						return SHARED_MEMORY_ALLOCATED;
					}

					static inline bool gamepad::__winnt__closeSharedMemory()
					{
						// Close the shared memory handle.
						CloseHandle(sharedMemory);
				
						// Make sure we don't keep a reference to the shared memory.
						sharedMemory = NULL;

						// Return the default response.
						return true;
					}

					static inline bool __winnt__sharedMemoryOpen()
					{
						return (sharedMemory != NULL);
					}

					// This will only return 'false' if this
					// fails to open the shared memory segment.
					static inline bool __winnt__autoOpenSharedMemory()
					{
						if (__winnt__sharedMemoryOpen())
							return true;

						return (__winnt__openSharedMemory() != SHARED_MEMORY_UNAVAILABLE);
					}

					static inline LPVOID __winnt__mapSharedMemory(DWORD desiredAccess=FILE_MAP_READ, SIZE_T size=SHARED_GAMEPAD_MEMORY_SIZE, DWORD highFileOffset=0, DWORD lowFileOffset=0)
					{
						return MapViewOfFile(sharedMemory, desiredAccess, highFileOffset, lowFileOffset, size);
					}

					static inline void __winnt__unmapSharedMemory(LPCVOID memory)
					{
						UnmapViewOfFile(memory);

						return;
					}

					// This command will return a pointer to the origin of the
					// shared-buffer, up to the end of the "plugged in" segment.
					// This will not offset the buffer, as it needs to be "unmapped".
					// This can be done using the '__winnt__unmapSharedMemory' command.
					static inline LPVOID __winnt__map_PluggedIn_Memory(DWORD desiredAccess=FILE_MAP_READ)
					{
						return __winnt__mapSharedMemory(desiredAccess, SHARED_GAMEPAD_MEMORY_PLUGGED_IN_MAX_SCOPE);
					}

					// This is the same as '__winnt__map_PluggedIn_Memory', only for the "states" segment.
					// Please follow the documentation for that command, barring the "scope" differences.
					static inline LPVOID __winnt__map_States_Memory(DWORD desiredAccess=FILE_MAP_READ)
					{
						return __winnt__mapSharedMemory(desiredAccess, SHARED_GAMEPAD_MEMORY_STATES_MAX_SCOPE);
					}

					static inline bool __winnt__setGamepadConnected(gamepadID identifier, bool value)
					{
						if (!__winnt__sharedMemoryOpen())
							return false;

						// Get direct buffer access.
						auto buffer = __winnt__mapSharedMemory(FILE_MAP_WRITE, SHARED_GAMEPAD_MEMORY_PLUGGED_IN_MAX_SCOPE);

						bool* pluggedIn = (bool*)__winnt__sharedMemory_getPluggedInOffset(buffer);

						pluggedIn[identifier] = value;

						// Release usage of the buffer.
						__winnt__unmapSharedMemory(buffer);

						// Return the default response.
						return true;
					}

					static inline bool __winnt__getGamepadConnected(gamepadID identifier)
					{
						if (!__winnt__sharedMemoryOpen())
							return false;

						// Get direct buffer access.
						auto buffer = __winnt__map_PluggedIn_Memory(FILE_MAP_READ);

						bool* pluggedIn = (bool*)__winnt__sharedMemory_getPluggedInOffset(buffer);

						auto value = pluggedIn[identifier];

						// Release usage of the buffer.
						__winnt__unmapSharedMemory(buffer);

						// Return the current connection-value.
						return value;
					}

					static inline bool __winnt__activateGamepad(gamepadID identifier)
					{
						return __winnt__setGamepadConnected(identifier, true);
					}

					static inline bool __winnt__deactivateGamepad(gamepadID identifier)
					{
						return __winnt__setGamepadConnected(identifier, false);
					}

					static inline size_t __winnt__sharedMemory_getStatesOffset()
					{
						return SHARED_GAMEPAD_MEMORY_STATES_OFFSET;
					}

					static inline size_t __winnt__sharedMemory_getPluggedInOffset()
					{
						return SHARED_GAMEPAD_MEMORY_PLUGGED_IN_OFFSET;
					}

					static inline LPVOID __winnt__sharedMemory_getStatesOffset(LPVOID memory)
					{
						return (LPVOID)(((unsigned char*)memory)+__winnt__sharedMemory_getStatesOffset());
					}

					static inline LPVOID __winnt__sharedMemory_getPluggedInOffset(LPVOID memory)
					{
						return (LPVOID)(((unsigned char*)memory)+__winnt__sharedMemory_getPluggedInOffset());
					}
				#endif

				static inline bool realDeviceConnected(gamepadID identifier)
				{
					#ifdef PLATFORM_WINDOWS
						return (__winnt__realDeviceStateResponse(identifier) == ERROR_SUCCESS);
					#else
						return false;
					#endif
				}

				// This command simulates the specified state.
				static void simulateState(gamepadState& state, gamepadID localIdentifier);

				// Constructor(s):
				gamepad(gamepadID localIdentifier, gamepadID remoteIdentifier, bool canDetect=true, bool canSimulate=true, deviceFlags flagsToAdd=deviceFlags());

				// Destructor(s):
				// Nothing so far.

				// Methods:
				virtual bool connect() override;
				virtual bool disconnect() override;

				virtual void detect(application* program) override;
				virtual void simulate(application* program) override;

				// These commands may be used to serialize and deserialize this device's 'state':
				virtual void readFrom(QSocket& socket) override;
				virtual void writeTo(QSocket& socket) override;

				// This command simulates the current state if 'hasState' specifies to do so.
				bool simulateState();

				inline high_resolution_clock::time_point updateActivityTimer()
				{
					// Set the activity snapshot, then return it to the user.
					return activitySnapshot = high_resolution_clock::now();
				}

				inline bool hasState() const
				{
					#ifdef PLATFORM_WINDOWS
						return (state.native.dwPacketNumber != __winnt__lastPacketNumber);
					#else
						return false;
					#endif
				}

				inline bool connected_real() const
				{
					#ifdef PLATFORM_WINDOWS
						return (__winnt__state_meta != ERROR_DEVICE_NOT_CONNECTED);
					#else
						return true;
					#endif
				}

				// This will tell you how much time has passed since this device was last used.
				inline milliseconds activityTime() const
				{
					return elapsed(activitySnapshot);
				}

				// Fields:
				gamepadID localGamepadNumber;
				gamepadID remoteGamepadNumber;

				gamepadState state;

				high_resolution_clock::time_point activitySnapshot;

				#ifdef PLATFORM_WINDOWS
					DWORD __winnt__lastPacketNumber;
					DWORD __winnt__state_meta;
				#endif
		};
	}
}