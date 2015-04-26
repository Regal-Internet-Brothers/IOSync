// Includes:
#include "gamepad.h"

#include "../networking/networking.h"
#include "../application/application.h"

// Namespace(s):
namespace iosync
{
	namespace devices
	{
		// Structures:

		// gamepadState:

		// Constructor(s):
		gamepadState::gamepadState(nativeGamepad rep) : native(rep)
		{
			// Nothing so far.
		}

		// Destructor(s):
		// Nothing so far.

		// Methods:
		void gamepadState::readFrom(QSocket& socket)
		{
			#ifdef GAMEPAD_NETWORK_SAFE
				//auto startPosition = socket.readOffset;
			#endif

			#ifdef GAMEPAD_FAST_NETWORK_IO
				native = socket.read<nativeGamepad>();
			#else
				#ifdef PLATFORM_WINDOWS
					native.dwPacketNumber = socket.read<DWORD>();
					
					native.Gamepad.wButtons = socket.read<WORD>();
					
					native.Gamepad.bLeftTrigger = socket.read<BYTE>();
					native.Gamepad.bRightTrigger = socket.read<BYTE>();
					
					native.Gamepad.sThumbLX = socket.read<SHORT>();
					native.Gamepad.sThumbLY = socket.read<SHORT>();

					native.Gamepad.sThumbRX = socket.read<SHORT>();
					native.Gamepad.sThumbRY = socket.read<SHORT>();
				#endif
			#endif

			#ifdef GAMEPAD_NETWORK_SAFE
				/*
				// Calculate the amount read from the input:
				auto amountRead = socket.readOffset-startPosition;
						
				// Check if there are still bytes left.
				if (amountRead < serializedNativeGamepadSize)
				{
					socket.readBytes((uqchar*)&native, (serializedNativeGamepadSize-amountRead), amountRead);
				}
				*/
			#endif

			return;
		}

		void gamepadState::writeTo(QSocket& socket)
		{
			#ifdef GAMEPAD_NETWORK_SAFE
				//auto startPosition = socket.writeOffset;
			#endif

			#ifdef GAMEPAD_FAST_NETWORK_IO
				socket.write<nativeGamepad>(native);
			#else
				#ifdef PLATFORM_WINDOWS
					socket.write<DWORD>(native.dwPacketNumber);

					socket.write<WORD>(native.Gamepad.wButtons);
					
					socket.write<BYTE>(native.Gamepad.bLeftTrigger);
					socket.write<BYTE>(native.Gamepad.bRightTrigger);
					
					socket.write<SHORT>(native.Gamepad.sThumbLX);
					socket.write<SHORT>(native.Gamepad.sThumbLY);
					
					socket.write<SHORT>(native.Gamepad.sThumbRX);
					socket.write<SHORT>(native.Gamepad.sThumbRY);
				#endif
			#endif

			#ifdef GAMEPAD_NETWORK_SAFE
				/*
				// Calculate the amount read from the input:
				auto amountWritten = socket.readOffset-startPosition;
						
				// Check if there are still bytes left.
				if (amountWritten < serializedNativeGamepadSize)
				{
					socket.padBytes(serializedNativeGamepadSize-amountWritten);
				}
				*/
			#endif

			return;
		}

		// Classes:

		// gamepad:

		// Global variable(s):
		#ifdef PLATFORM_WINDOWS
			HANDLE gamepad::sharedMemory = HANDLE();
		#endif

		// Functions:

		// Windows-specific extensions:
		#ifdef PLATFORM_WINDOWS
			bool gamepad::__winnt__injectLibrary(DWORD processID, CPUArchitecture process_Architecture)
			{
				// Nested functions/lambdas:
				auto injectionStr = [&]() -> string
				{
					stringstream injectionStream;
					injectionStream << "IGNORE " << XINPUT_INJECTION_ARGUMENT << " " << processID;

					return injectionStream.str();
				};

				if (processID == 0)
					return false;

				switch (process_Architecture)
				{
					#if defined(PLATFORM_X86) || defined(PLATFORM_X64)
						case x86:
							#if defined(PLATFORM_X86)
								return application::__winnt__injectLibrary(INJECTION_DLL_NAME_X86, processID);
							#else
								return application::__winnt__startProcess(TEXT("IOSync_x86.exe"), injectionStr());
							#endif
						case x64:
							#if defined(PLATFORM_X64)
								return application::__winnt__injectLibrary(INJECTION_DLL_NAME_X64, processID);
							#else
								return application::__winnt__startProcess(TEXT("IOSync_x64.exe"), injectionStr());
							#endif
					#elif defined(PLATFORM_ARM) || defined(PLATFORM_ARM64)
						case ARM:
							return application::__winnt__injectLibrary(INJECTION_DLL_NAME_ARM, processID);
						case ARM64:
							return application::__winnt__injectLibrary(INJECTION_DLL_NAME_ARM64, processID);
					#endif
				}

				// This architecture is unsupported.
				return false;
			}

			bool gamepad::__winnt__injectLibrary(DWORD processID)
			{
				if (processID == 0)
					return false;

				#if !defined(PLATFORM_ARM)
					return __winnt__injectLibrary(processID, (application::__winnt__process32bit(processID)) ? x86 : x64);
				#else
					return __winnt__injectLibrary(processID, (application::__winnt__process32bit(processID)) ? ARM : ARM64)
				#endif
			}
		#endif

		void gamepad::simulateState(gamepadState& state, gamepadID localIdentifier)
		{
			#ifdef PLATFORM_WINDOWS
				if (!__winnt__sharedMemoryOpen())
					return;

				auto buffer = __winnt__map_States_Memory(FILE_MAP_WRITE);

				nativeGamepad* statePtr = (((nativeGamepad*)__winnt__sharedMemory_getStatesOffset(buffer)) + localIdentifier); // FILE_MAP_ALL_ACCESS

				*statePtr = state.native;
				
				__winnt__unmapSharedMemory(buffer);
			#endif

			return;
		}
		
		// Constructor(s):
		gamepad::gamepad(gamepadID localIdentifier, gamepadID remoteIdentifier, bool canDetect, bool canSimulate, deviceFlags flagsToAdd)
			: IODevice(canDetect, canSimulate, flagsToAdd), localGamepadNumber(localIdentifier), remoteGamepadNumber(remoteIdentifier) { /* Nothing so far. */ }

		// Destructor(s):
		// Nothing so far.

		// Methods:
		bool gamepad::connect()
		{
			// Make sure we aren't already connected:
			if (connected())
				return false;

			#ifdef GAMEPAD_DEBUG_RUMBLE
				#ifdef PLATFORM_WINDOWS
					__winnt__rumbleDevice(localGamepadNumber);
				#endif
			#endif

			updateActivityTimer();

			// Call the super-class's implementation, then return its response.
			return deviceManager::connect();
		}

		bool gamepad::disconnect()
		{
			// Make sure we aren't already disconnected:
			if (disconnected())
				return false;

			// Call the super-class's implementation, then return its response.
			return deviceManager::disconnect();
		}

		void gamepad::detect(application* program)
		{
			#ifdef PLATFORM_WINDOWS
				__winnt__lastPacketNumber = state.native.dwPacketNumber;

				// Read the Xinput-device's state.
				auto result = __winnt__realDeviceState(localGamepadNumber, state.native);

				#ifdef GAMEPAD_DEBUG
					if (result != ERROR_SUCCESS)
					{
						if (result == ERROR_DEVICE_NOT_CONNECTED)
						{
							deviceInfo << "Attempted to detect device that is not connected: " << localGamepadNumber << endl;
						}
						else
						{
							deviceInfo << "Unable to detect 'gamepad' state: " << result << endl;
						}
					}
				#endif

				__winnt__state_meta = result;
			#endif

			// Update the activity timer.
			updateActivityTimer();

			return;
		}

		void gamepad::simulate(application* program)
		{
			#ifdef PLATFORM_WINDOWS
				if (!__winnt__sharedMemoryOpen())
					return;
			#endif

			simulateState();

			return;
		}

		void gamepad::readFrom(QSocket& socket)
		{
			state.readFrom(socket);

			// Update the activity timer.
			updateActivityTimer();

			return;
		}

		void gamepad::writeTo(QSocket& socket)
		{
			state.writeTo(socket);

			// Update the activity timer.
			updateActivityTimer();

			return;
		}

		bool gamepad::simulateState()
		{
			if (!hasState())
				return false;

			// Update the activity timer.
			updateActivityTimer();

			simulateState(state, localGamepadNumber);

			#ifdef PLATFORM_WINDOWS
				__winnt__lastPacketNumber = state.native.dwPacketNumber;
			#endif

			// Return the default response.
			return true;
		}
	}
}