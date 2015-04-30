// Includes:
#include "gamepad.h"

#include "../platform.h"
#include "../networking/networking.h"
#include "../application/application.h"
#include "../names.h"

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

		#ifdef GAMEPAD_VJOY_ENABLED
			vJoy::vJoyDriver gamepad::vJoyInfo = { vJoy::vJoyDriver::VJOY_UNDEFINED };
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
								return application::__winnt__startProcess(TEXT(EXECUTABLE_NAME_X86), injectionStr());
							#endif
						case x64:
							#if defined(PLATFORM_X64)
								return application::__winnt__injectLibrary(INJECTION_DLL_NAME_X64, processID);
							#else
								return application::__winnt__startProcess(TEXT(EXECUTABLE_NAME_X64), injectionStr());
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

			#ifdef GAMEPAD_VJOY_ENABLED
				vJoy::vJoyDriver::vJoyDriverState gamepad::__winnt__vJoy__init()
				{
					// Namespace(s):
					using namespace vJoy;

					vJoyInfo.state = ((vJoyEnabled() == TRUE) ? vJoyDriver::VJOY_ENABLED : vJoyDriver::VJOY_DISABLED);

					if (vJoyInfo.state == vJoyDriver::VJOY_ENABLED)
					{
						WORD DLLVer, driverVer;

						if (!DriverMatch(&DLLVer, &driverVer))
						{
							vJoyInfo.state = vJoyDriver::VJOY_DISABLED;
						}
					}

					return vJoyInfo.state;
				}

				vJoy::vJoyDriver::vJoyDriverState gamepad::__winnt__vJoy__deinit()
				{
					// Namespace(s):
					using namespace vJoy;

					if (vJoyInfo.state == vJoyDriver::VJOY_ENABLED)
					{
						// Nothing so far.

						vJoyInfo.state = vJoyDriver::VJOY_DISABLED;
					}

					return vJoyInfo.state;
				}

				VjdStat gamepad::__winnt__vJoy__getStatus(const gamepadID internal_identifier)
				{
					return GetVJDStatus(__winnt__vJoy__vDevice(internal_identifier));
				}

				LONG gamepad::__winnt__vJoy__capAxis(const UINT vJoyID, const LONG value, const UINT axis, const UINT maximum_value)
				{
					#ifdef GAMEPAD_VJOY_SAFE
						auto& device = vJoyInfo.getDevice(vJoyID);
						auto axisInfo = device.getAxis(axis)->second;
						auto axis_mid = ((axisInfo.max - axisInfo.min) / 2);

						//return axis_mid + value;
						//return axis_mid + max(axis_min, min(value, axis_max));
						
						auto scalar = max(axis_mid / maximum_value, 1);

						return max(axisInfo.min, min((LONG)(axis_mid + (value * scalar)), axisInfo.max));
					#else
						return value;
					#endif
				}

				void gamepad::__winnt__vJoy__transferAxis(const UINT vJoyID, const gamepadState& state, JOYSTICK_POSITION& vState, const UINT axis)
				{
					switch (axis)
					{
						case HID_USAGE_X:
							vState.wAxisX = __winnt__vJoy__capAxis(vJoyID, state.native.Gamepad.sThumbLX, axis);

							break;
						case HID_USAGE_Y:
							vState.wAxisY = __winnt__vJoy__capAxis(vJoyID, -state.native.Gamepad.sThumbLY, axis);

							break;
						case HID_USAGE_Z:
							{
								LONG m = vJoyInfo.getDevice(vJoyID).axisMax(axis);

								m = ((m+1)/2);
								//m = m+1;

								auto value = m;
								
								int scalar = max(m / (UCHAR_MAX), 1);

								if (state.native.Gamepad.bRightTrigger != 0)
									value += ((state.native.Gamepad.bRightTrigger * scalar) + scalar);

								if (state.native.Gamepad.bLeftTrigger != 0)
									value -= ((state.native.Gamepad.bLeftTrigger * scalar) + scalar);

								vState.wAxisZ = value;
								//vState.wAxisZ = __winnt__vJoy__capAxis(vJoyID, value, axis);
							}

							break;
						case HID_USAGE_RX:
							vState.wAxisXRot = __winnt__vJoy__capAxis(vJoyID, state.native.Gamepad.sThumbRX, axis);

							break;
						case HID_USAGE_RY:
							vState.wAxisYRot = __winnt__vJoy__capAxis(vJoyID, -state.native.Gamepad.sThumbRY, axis);

							break;
						case HID_USAGE_POV:
							{
								// Local variable(s):

								// Get a reference to the vJoy-device.
								auto& device = vJoyInfo.getDevice(vJoyID);

								// Check the POV mode:
								if (device.contPOVNumber > 0)
								{
									if ((state.native.Gamepad.wButtons & gamepadState::padMask) > 0)
									{
										vState.bHats = (((int)joyHat(state))*100);
									}
									else
									{
										// Disable the HAT.
										vState.bHats = 36001;
									}
								}
								else
								{
									vState.bHats = 4;

									if (state.native.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
										vState.bHats = 1;
									else if (state.native.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
										vState.bHats = 3;
									else if (state.native.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
										vState.bHats = 2;
									else if (state.native.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
										vState.bHats = 0;
								}
							}

							break;
					}

					return;
				}

				void gamepad::__winnt__vJoy__autoTransferAxis(const UINT vJoyID, const gamepadState& state, JOYSTICK_POSITION& vState, const UINT axis)
				{
					// Local variable(s):
					auto& device = vJoyInfo.getDevice(vJoyID);

					if (device.hasAxis(axis))
					{
						__winnt__vJoy__transferAxis(vJoyID, state, vState, axis);
					}

					return;
				}

				void gamepad::__winnt__vJoy__simulateState(const gamepadState& state, const UINT vJoyID, const VjdStat status)
				{
					//ResetVJD(vJoyID);
					
					// Local variable(s):
					JOYSTICK_POSITION vState;

					ZeroVar(vState);

					vState.bDevice = vJoyID;

					// Automatically transfer the axes to the output-state:
					__winnt__vJoy__autoTransferAxis(vJoyID, state, vState, HID_USAGE_X);
					__winnt__vJoy__autoTransferAxis(vJoyID, state, vState, HID_USAGE_Y);

					__winnt__vJoy__autoTransferAxis(vJoyID, state, vState, HID_USAGE_RX);
					__winnt__vJoy__autoTransferAxis(vJoyID, state, vState, HID_USAGE_RY);

					__winnt__vJoy__autoTransferAxis(vJoyID, state, vState, HID_USAGE_POV);
					__winnt__vJoy__autoTransferAxis(vJoyID, state, vState, HID_USAGE_Z);

					vState.lButtons = (state.native.Gamepad.wButtons ^ (state.native.Gamepad.wButtons & gamepadState::padMask));
					vState.lButtons = ((((vState.lButtons >> 12) & SHRT_MAX) | (vState.lButtons ^ (vState.lButtons & gamepadState::faceMask))) & SHRT_MAX);

					//cout << "vState.lButtons: "  << vState.lButtons << endl;

					// Update the vJoy-device.
					UpdateVJD(vJoyID, &vState);

					return;
				}
			#endif
		#endif

		void gamepad::simulateState(const gamepadState& state, const gamepadID localIdentifier)
		{
			#ifdef PLATFORM_WINDOWS
				if (__winnt__sharedMemoryOpen())
				{
					return;

					auto buffer = __winnt__map_States_Memory(FILE_MAP_WRITE);

					nativeGamepad* statePtr = (((nativeGamepad*)__winnt__sharedMemory_getStatesOffset(buffer)) + localIdentifier); // FILE_MAP_ALL_ACCESS

					*statePtr = state.native;
				
					__winnt__unmapSharedMemory(buffer);
				}
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
				#ifdef GAMEPAD_VJOY_ENABLED
					// Check the current state of vJoy:
					if (vJoyInfo.state == vJoy::vJoyDriver::VJOY_ENABLED)
					{
						// Calculate the internal status.
						switch (vJoy_status)
						{
							case VJD_STAT_OWN:
							case VJD_STAT_FREE:
								__winnt__vJoy__simulateState(state, __winnt__vJoy__vDevice(localGamepadNumber), vJoy_status);

								break;
							case VJD_STAT_BUSY:
								break;
							case VJD_STAT_MISS:
								break;
							default:
								break;
						}
					}
				#endif
				
				__winnt__lastPacketNumber = state.native.dwPacketNumber;
			#endif

			// Return the default response.
			return true;
		}

		#ifdef GAMEPAD_VJOY_ENABLED
			VjdStat gamepad::__winnt__vJoy__calculateStatus()
			{
				// Assign the internal vJoy status, then return it.
				return vJoy_status = __winnt__vJoy__getStatus(localGamepadNumber);
			}
		#endif
	}
}