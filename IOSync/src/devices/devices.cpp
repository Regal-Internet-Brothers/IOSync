// Includes:
#include "devices.h"

#include "../iosync.h"

// Namespace(s):
namespace iosync
{
	namespace devices
	{
		// Classes:

		// deviceManager:

		// Constructor(s):
		deviceManager::deviceManager() : flags(FLAG_NONE)
		{
			// Nothing so far.
		}

		deviceManager::deviceManager(deviceFlags flagsToAdd)
		{
			// Apply the flags specified.
			flags |= flagsToAdd;
		}

		// Destructor(s):
		deviceManager::~deviceManager()
		{
			// Automatically disconnect.
			//autoDisconnect();
		}

		// Methods:
		bool deviceManager::connect()
		{
			// Set this device as connected.
			flags |= FLAG_CONNECTED;

			// Return the default response.
			return true;
		}

		bool deviceManager::disconnect()
		{
			// Set this device as disconnected.
			flags &= ~FLAG_CONNECTED;
			//flags ~= FLAG_CONNECTED;

			// Return the default response.
			return true;
		}

		bool deviceManager::autoDisconnect()
		{
			if (connected())
				return disconnect();

			return false;
		}

		bool deviceManager::autoConnect()
		{
			if (disconnected())
				return connect();

			return false;
		}

		bool deviceManager::connected() const
		{
			return ((flags & FLAG_CONNECTED) > 0);
		}

		bool deviceManager::disconnected() const
		{
			return (!connected());
		}

		#ifdef PLATFORM_WINDOWS
			void deviceManager::__winnt__rawRead(RAWINPUT* rawDevice)
			{
				// Nothing so far.

				return;
			}
		#endif

		// inputDevice:

		// Constructor(s):
		inputDevice::inputDevice() : deviceManager(FLAG_CAN_DETECT)
		{
			// Nothing so far.
		}

		// Methods:
		void inputDevice::update(iosync_application& program)
		{
			// Check if we can detect input:
			if (canDetect() && !asyncDetect())
			{
				// Detect actions, and output to the action-queue.
				detect(program);
			}

			return;
		}

		// outputDevice:
		
		// Functions:
		nativeResponseCode outputDevice::sendNativeSystemInput(nativeDeviceInterface* items, size_t itemCount)
		{
			#if defined(PLATFORM_WINDOWS)
				return SendInput((UINT)itemCount, (LPINPUT)items, (int)sizeof(nativeDeviceInterface));
			#elif defined(PLATFORM_LINUX)
				nativeResponseCode response = nativeResponseCode();
				
				// Send each of the input-operations specified.
				for (size_t i = 0; i < itemCount; i++)
				{
					response += write(items[i].deviceDescriptor, &items[i].input, sizeof(items[i].input));
				}

				return response;
			#else
				return (nativeResponseCode)0;
			#endif
		}

		// Constructor(s):
		outputDevice::outputDevice() : deviceManager(FLAG_CAN_SIMULATE)
		{
			// Nothing so far.
		}

		// Methods:
		void outputDevice::update(iosync_application& program)
		{
			// Check if we can simulate input:
			if (canSimulate() && !asyncSimulate())
			{
				// Simulate the actions in the action-queue.
				simulate(program);
			}

			return;
		}

		// IODevice:

		// Constructor(s):
		IODevice::IODevice() : deviceManager(FLAG_CAN_DETECT|FLAG_CAN_SIMULATE), inputDevice(), outputDevice()
		{
			// Nothing so far.
		}

		IODevice::IODevice(deviceFlags flagsToAdd) : deviceManager(flagsToAdd), inputDevice(), outputDevice() // IODevice()
		{
			// Nothing so far.
		}

		IODevice::IODevice(bool canDetect, bool canSimulate, deviceFlags flagsToAdd) : deviceManager(flagsToAdd)
		{
			if (canDetect)
				flags |= FLAG_CAN_DETECT;

			if (canSimulate)
				flags |= FLAG_CAN_SIMULATE;
		}

		// Methods:
		void IODevice::update(iosync_application& program)
		{
			// Call the primary implementations:
			inputDevice::update(program);
			outputDevice::update(program);

			// Nothing else so far.

			return;
		}

		void IODevice::simulate(iosync_application& program)
		{
			// Nothing so far.

			return;
		}

		void IODevice::detect(iosync_application& program)
		{
			// Nothing so far.

			return;
		}
	}
}