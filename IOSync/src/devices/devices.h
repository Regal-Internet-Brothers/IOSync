#pragma once

// Includes:
#include "../platform.h"
#include "../networking/networking.h"

// Namespace(s):
namespace iosync
{
	// Forward declarations:
	class application;

	// Namespace(s):
	namespace devices
	{
		// Namespace(s):
		using namespace iosync::networking;

		// Typedefs:
		typedef unsigned int deviceFlags;

		// Platform-specific:
		#ifdef PLATFORM_WINDOWS
			typedef DWORD nativeFlags;
			typedef INPUT nativeDeviceInterface;
			typedef UINT nativeResponseCode;
		#else
			typedef unsigned long nativeFlags;
			typedef void* nativeDeviceInterface;
			typedef unsigned int nativeResponseCode;
		#endif

		// Enumerator(s):
		enum deviceFlag : deviceFlags
		{
			FLAG_NONE = 0,

			// This flag describes that a device can simulate
			// "real device input" on the system.
			FLAG_CAN_SIMULATE = 1,

			// This flag describes that a device can detect/read
			// "real device input" from the system.
			FLAG_CAN_DETECT = 2,

			/*
				The asynchronous flags effectively state if the 'simulate' and
				'detect' methods will be called when updating normally.
				
				When using these flags are enabled, these commands may
				or may not be called from external sources. This is up to
				the code managing this framework's utilities.

				For example, input-detection may be handled
				through platform-specific extensions.

				Simulation could also be handled using external utilities,
				even if the device has functionality to handle this.
			*/

			// This is used to describe if a device polls for input itself,
			// or if it retrieves it from an external/asyncronous source.
			// Platform-specific extensions may also be used in order to
			// "asynchronously" handle device messages, and general I/O.
			FLAG_ASYNC_DETECTION = 4,

			// Like 'FLAG_ASYNC_DETECTION', this describes if output/simulation
			// is done through an asynchronous/external routine.
			FLAG_ASYNC_SIMULATION = 8,

			// This flag is used to tell if a device has been connected.
			FLAG_CONNECTED = 16,

			// This may be used by inheriting devices as an offset for custom flags.
			FLAG_CUSTOM_LOCATION = 32,

			// This acts as a macro to describe that a device's detection
			// and simulation functionality will be handled externally.
			FLAG_IS_ASYNCHRONOUS = FLAG_ASYNC_DETECTION|FLAG_ASYNC_SIMULATION,
		};

		// Classes:
		class deviceManager
		{
			public:
				// Constructor(s):
				deviceManager();
				deviceManager(deviceFlags flags);

				// Destructor(s):
				virtual ~deviceManager();

				// Methods:
				virtual bool connect() = 0;
				virtual bool disconnect() = 0;

				virtual bool autoDisconnect();
				virtual bool autoConnect();

				// This method is used to interface with the device.
				virtual void update(application* program) = 0;

				virtual bool connected() const;
				virtual bool disconnected() const;

				#ifdef PLATFORM_WINDOWS
					virtual void __winnt__rawRead(RAWINPUT* rawDevice);
				#endif

				inline bool canDetect() const
				{
					return ((flags & FLAG_CAN_DETECT) > 0);
				}

				inline bool canSimulate() const
				{
					return ((flags & FLAG_CAN_SIMULATE) > 0);
				}

				inline bool asyncDetect() const
				{
					return ((flags & FLAG_ASYNC_DETECTION) > 0);
				}

				inline bool asyncSimulate() const
				{
					return ((flags & FLAG_ASYNC_SIMULATION) > 0);
				}

				inline bool isAsync() const
				{
					return (asyncDetect() || asyncSimulate());
				}

				// Fields:

				// Booleans / Flags:
				deviceFlags flags;
		};

		class inputDevice : virtual public deviceManager
		{
			public:
				// Constructor(s):
				inputDevice();

				// Destructor(s):
				// Nothing so far.

				// Methods:

				/*
					This acts as the primary device-detection command.

					Any operations you wish to make regarding polling/detecting of a specific
					"virtual device" should be done in your own implementation of this.

					You should only "call up" to this implementation if your super-class says to do so.
				*/

				virtual void detect(application* program) = 0;

				// Networking related:
				virtual void readFrom(QSocket& socket) = 0;

				// When overriding this implementation, please "call up" to this one.
				virtual void update(application* program) override;
		};

		class outputDevice : virtual public deviceManager
		{
			public:
				// Functions:

				// This command allows the user to "send" a message into the platform's device stream(s).
				// This command is largely unmanaged, and may not be available on some platforms.
				static nativeResponseCode sendNativeSystemInput(nativeDeviceInterface* items, size_t itemCount=1);

				// Constructor(s):
				outputDevice();

				// Destructor(s):
				// Nothing so far.

				// Methods:

				/*
					This acts as the primary device-simulation command.
				
					Any operations you wish to make upon a specific "virtual device"
					should be done in your own implementation of this.

					You should only "call up" to this implementation if your super-class says to do so.
				*/

				virtual void simulate(application* program) = 0;

				// Networking related:
				virtual void writeTo(QSocket& socket) = 0;

				// When overriding this implementation, please "call up" to this one.
				virtual void update(application* program) override;
		};

		class IODevice : virtual public inputDevice, virtual public outputDevice
		{
			public:
				// Constructor(s):
				IODevice();
				IODevice(deviceFlags flagsToAdd);

				// Destructor(s):
				// Nothing so far.

				// Methods:

				// When overriding this implementation, please "call up" to this one.
				virtual void update(application* program) override;

				// Overriding these is only recommended if you are not handling I/O asynchronously.
				// By default, this isn't the case, so you'll want to override these.
				// Asynchronous I/O is handled in two parts, so this is really just device-dependent.
				// You do not need to "call up" to these implementations.

				virtual void simulate(application* program) override;
				virtual void detect(application* program) override;

				// Fields:
				// Nothing so far.
		};
	}
}