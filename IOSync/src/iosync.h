#pragma once

// Preprocessor related:
#define IOSYNC_TESTMODE
#define IOSYNC_FAST_TESTMODE

// Includes:
#include "exceptions.h"
#include "iosync_application_exceptions.h"

#include "application/application.h"
#include "networking/networkEngine.h"

#include "devices/devices.h"
#include "devices/keyboard.h"

// Standard library:
#include <string>
#include <iostream>

// Namespace(s):
using namespace std;
using namespace iosync::networking;
using namespace iosync::exceptions::applicationExceptions;

//using namespace quickLib::sockets;

namespace iosync
{
	// Typedefs:
	typedef unsigned short applicationMode; // unsigned char

	// Forward declarations:
	class iosync_application;

	// Namespace(s):
	namespace sharedWindow
	{
		// Constant variable(s):
		#ifdef PLATFORM_WINDOWS
			static const char* __winnt__windowClassName = "DefaultWindow";
		#endif

		// Global variable(s):
		extern nativeWindow windowInstance;

		#ifdef PLATFORM_WINDOWS
			extern bool __winnt__classRegisted;
		#endif

		// Functions:
		nativeWindow open(OSINFO info);

		void update(application* program);

		bool isOpen();

		// Windows:
		#ifdef PLATFORM_WINDOWS
			LRESULT CALLBACK __winnt__WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			bool __winnt__registerClass(OSINFO OSInfo);
		#endif
	};

	namespace deviceManagement
	{
		// Typedefs:
		typedef unsigned char deviceType;

		// The type used for device sub-messages.
		typedef unsigned char deviceMessageType; // unsigned short

		// Aliases:
		using kbd = devices::keyboard;

		// Enumerator(s):
		enum deviceTypes : deviceType
		{
			DEVICE_TYPE_NOT_FOUND,
			DEVICE_TYPE_KEYBOARD,
		};

		enum deviceMessageTypes : deviceMessageType
		{
			DEVICE_NETWORK_MESSAGE_CONNECT,
			DEVICE_NETWORK_MESSAGE_DISCONNECT,
			DEVICE_NETWORK_MESSAGE_ENTRIES,
		};

		// Structures:
		struct connectedDevices final
		{
			// Fields:
			kbd* keyboard;

			// Constructor(s):
			connectedDevices();

			// Destructor(s):
			~connectedDevices();

			inline void destroyKeyboard()
			{
				cout << "Attempting to destroy keyboard device-instance..." << endl;

				delete keyboard; keyboard = nullptr;

				cout << "Device-instance destroyed." << endl;

				return;
			}

			// Methods:
			void update(iosync_application* program);

			// This command will connect every device.
			void connect(iosync_application* program);

			// This command will disconnect every device.
			void disconnect();

			// This may be used to connect a specific device based on its 'deviceType'.
			bool connectDevice(deviceType devType, iosync_application* program);

			// Similar to 'connectDevice', this will disconnect the device of the type specified.
			bool disconnectDevice(deviceType devType, iosync_application* program);

			// This command allows you to manually disconnect the 'keyboard' object.
			inline bool disconnectKeyboard()
			{
				if (keyboard == nullptr)
					return false;

				if (keyboard->disconnect())
				{
					destroyKeyboard();

					return true;
				}

				// Return the default response.
				return false;
			}

			// This command allows you to manually connect the 'keyboard' object.
			kbd* connectKeyboard(iosync_application* program);

			// Networking related:

			// Unless you want to manually write a sub-message type, please use the other overload.
			// This acts as a macro for 'beginMessage', which specifies the targeted device.
			// There is no 'finishDeviceMessage', as it is not needed.
			// Please use the 'finishMessage' method, found in 'networkEngine'.
			headerInfo beginDeviceMessage(networkEngine& engine, QSocket& socket, deviceType devType);

			// This overload should be used instead of the main implementation, when you do not need to write a sub-message type yourself.
			// Normally, you shoul stick to this overload. Most, if not all documentation for them main implementation applies here.
			inline headerInfo beginDeviceMessage(networkEngine& engine, QSocket& socket, deviceType devType, deviceMessageType devMsgType)
			{
				// Get the header information from the main routine.
				auto info = beginDeviceMessage(engine, socket, devType);

				// Write the sub-message type specified.
				socket.write<deviceMessageType>(devMsgType);

				// Return the header-information.
				return info;
			}

			// Serialization related:
			void serializeTo(networkEngine& engine, QSocket& socket);
			void serializeKeyboard(networkEngine& engine, QSocket& socket);

			inline void serializeTo(networkEngine& engine)
			{
				serializeTo(engine, engine.socket);

				return;
			}

			void serializeConnectMessage(QSocket& socket, deviceType device);
			void serializeDisconnectMessage(QSocket& socket, deviceType device);

			void serializeKeyboardActions(QSocket& socket);

			// Message generation related:
			outbound_packet generateConnectMessage(networkEngine& engine, QSocket& socket, deviceType device, const address realAddress=address(), address forwardAddress=address());
			outbound_packet generateDisconnectMessage(networkEngine& engine, QSocket& socket, deviceType device, const address realAddress=address(), address forwardAddress=address());

			// Parsing/deserialization related:
			deviceType parseDeviceMessage(iosync_application* program, QSocket& socket, const messageHeader& header, const messageFooter& footer);

			bool parseConnectMessage(iosync_application* program, QSocket& socket, const messageHeader& header, const messageFooter& footer);
			bool parseDisconnectMessage(iosync_application* program, QSocket& socket, const messageHeader& header, const messageFooter& footer);

			deviceMessageType parseKeyboardMessage(iosync_application* program, QSocket& socket, deviceMessageType subMessageType, const messageHeader& header, const messageFooter& footer);

			void parseKeyboardActions(iosync_application* program, QSocket& socket, const messageHeader& header, const messageFooter& footer);

			// Sending related:
			inline size_t sendConnectMessage(networkEngine& engine, QSocket& socket, deviceType device, networkDestinationCode destination = DEFAULT_DESTINATION, bool resetLength=true)
			{
				generateConnectMessage(engine, socket, device);

				return engine.sendMessage(socket, destination, resetLength);
			}

			inline size_t sendDisconnectMessage(networkEngine& engine, QSocket& socket, deviceType device, networkDestinationCode destination = DEFAULT_DESTINATION, bool resetLength=true)
			{
				generateDisconnectMessage(engine, socket, device);

				return engine.sendMessage(socket, destination, resetLength);
			}

			// This acts as a macro for requesting device-connections.
			inline size_t sendConnectMessages(networkEngine& engine, networkDestinationCode destination = DEFAULT_DESTINATION)
			{
				// Local variable(s):

				// The number of bytes sent.
				size_t sent = 0;

				sendConnectMessage(engine, engine.socket, DEVICE_TYPE_KEYBOARD, destination);

				// Return the number of bytes sent.
				return sent;
			}

			// This will send the active serializable data this manager produces.
			// Please call 'sendConnectMessages' (Or similar) before calling this.
			inline size_t sendTo(networkEngine& engine, networkDestinationCode destination=DEFAULT_DESTINATION)
			{
				serializeTo(engine);

				return engine.sendMessage(engine, destination);
			}

			inline bool keyboardConnected() const
			{
				return ((keyboard != nullptr) && keyboard->connected());
			}

			inline bool hasDeviceConnected() const
			{
				return keyboardConnected();
			}
		};
	}

	// Constant variable(s):

	// Player names are handled as wide-strings.
	static const wchar_t* DEFAULT_PLAYER_NAME = L"Player";

	// Classes:
	class iosync_application : public application
	{
		public:
			// Enumerator(s):
			enum applicationModes : applicationMode
			{
				MODE_CLIENT = 0,
				MODE_SERVER = 1,
			};

			enum messageTypes : messageType
			{
				MESSAGE_TYPE_DEVICE = networkEngine::messageTypes::MESSAGE_TYPE_CUSTOM_LOCATION,
			};

			// Global variable(s):
			static bool sharedWindowOpen;

			// Functions:
			// Nothing so far.

			// Constructor(s):
			iosync_application(rate updateRate = DEFAULT_UPDATERATE, OSINFO OSInfo=OSINFO());

			// Destructor(s):
			~iosync_application();

			// Methods:
			int execute();
			int execute(const addressPort port);
			int execute(wstring username, string remoteAddress, addressPort remotePort = DEFAULT_PORT, addressPort localPort = DEFAULT_LOCAL_PORT);

			void onCreate(applicationMode mode=MODE_SERVER);
			void onClose();

			void disconnectDevices();
			void closeNetwork();

			void update(rate frameNumber=0) override;

			void updateNetwork();
			void updateDevices();

			void checkDeviceMessages();

			// Networking related:

			// Serialization related:
			// Nothing so far.

			// Message generation related:
			// Nothing so far.

			// Parsing/deserialization related:
			bool parseNetworkMessage(QSocket& socket, const messageHeader& header, const messageFooter& footer) override;

			// Sending related:
			// Nothing so far.

			// Call-backs:
			void onNetworkConnected(networkEngine& engine) override;
			void onNetworkClosed(networkEngine& engine) override;

			nativeWindow getWindow() const override;

			// Platform-specific extensions:
			#ifdef PLATFORM_WINDOWS
				void __winnt__readFromDevice(RAWINPUT* rawDevice);
			#endif

			inline bool closed() const
			{
				return !isRunning;
			}

			inline bool allowDeviceDetection() const
			{
				return (mode == MODE_CLIENT);
			}

			inline bool allowDeviceSimulation() const
			{
				return (mode == MODE_SERVER);
			}

			// Fields:

			// Network I/O.
			networkEngine* network;

			// Input devices.
			deviceManagement::connectedDevices devices;

			// The "mode" of this application.
			applicationMode mode;

			// The window this application is tied to.
			nativeWindow window;
	};
}