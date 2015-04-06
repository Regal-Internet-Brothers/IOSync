#pragma once

// Preprocessor related:
#define IOSYNC_TESTMODE
//#define IOSYNC_FAST_TESTMODE

//#define IOSYNC_LIVE_COMMANDS

/*
#ifdef IOSYNC_LIVE_COMMANDS
	#define IOSYNC_LIVE_COMMAND_INPUT_ON_MAIN_THREAD
#endif
*/

#ifdef IOSYNC_LIVE_COMMAND_INPUT_ON_MAIN_THREAD
	#define IOSYNC_ALLOW_ASYNC_EXECUTE
#endif

#define _CRT_SECURE_NO_WARNINGS

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

#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
	#include <mutex>
	#include <thread>
#endif

#ifdef IOSYNC_LIVE_COMMANDS
	#include <queue>
	#include <vector>
	
	#include <thread>
	#include <mutex>
#endif

// Output-stream macros:
#define deviceNetInfoStream std::cout
#define wdeviceNetInfoStream std::wcout

#define deviceInfoStream std::cout
#define wdeviceInfoStream std::wcout

#define deviceNetInfo std::cout << "{NETWORK} {DEVICE}: " // networkInfo
#define wdeviceNetInfo std::wcout << L"{NETWORK} {DEVICE}: "

#define deviceInfo std::cout << "{DEVICE}: "
#define wdeviceInfo std::wcout << L"{DEVICE}: "

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

			void destroyKeyboard();

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
			// Typedefs:
			typedef list<iosync_application*> programList;

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

			// Structures:
			#ifdef IOSYNC_LIVE_COMMANDS
				struct applicationCommand
				{
					// Typedefs:
					typedef wstring messageType;
					typedef wistream& inputStream;

					// Constructor(s):
					applicationCommand(messageType msgData, programList programs=programList());

					// This command will immediately read from the stream specified.
					applicationCommand(inputStream inStream, programList programs);

					// Destructor(s):
					// Nothing so far.

					// Methods:
					void readFrom(inputStream in);

					// Fields:
					messageType data;
					programList connectedPrograms;
				};
			#endif

			// Global variable(s):
			#ifdef IOSYNC_LIVE_COMMANDS
				static programList commandTargets;
				static queue<applicationCommand> commandQueue;
				
				static mutex commandMutex;
				static thread commandThread;

				static bool commandThreadRunning;
			#endif

			// Functions:
			#ifdef IOSYNC_LIVE_COMMANDS
				static inline bool pruneCommand(queue<applicationCommand>& commandQueue)
				{
					if (!commandQueue.empty())
					{
						if (commandQueue.front().connectedPrograms.empty())
						{
							commandQueue.pop();

							if (!commandQueue.empty())
							{
								pruneCommand(commandQueue);

								return true;
							}
						}
					}

					return false;
				}

				// This command allows you to manually begin accepting
				// user-commands, by surrendering the calling thread.
				static void beginLocalCommandAccept();

				static void acceptCommands();
				static bool openCommandThread(iosync_application* program);
				static bool closeCommandThread(iosync_application* program);
				static void forceCloseCommandThread();

				// This command closes the command-thread if no other applications are using it.
				// This will return 'true' if it closed the thread, or if the thread was already closed.
				static bool checkThreadViability();

				// This is used internally for optimization purposes, please use 'checkThreadViability' instead.
				static bool checkThreadViability_unsafe();
			#endif

			#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
				static void executeAsyncApplication();
			#endif

			static inline nativePort portFromString(string portStr)
			{
				// Convert the port-string to lower-case.
				transform(portStr.begin(), portStr.end(), portStr.begin(), tolower);

				// Check if the user didn't request the default port:
				if (portStr != "default" || portStr == "d")
				{
					// Convert the port-string into an integer.
					return (nativePort)stoi(portStr);
				}

				return DEFAULT_PORT;
			}

			static inline nativePort portFromInput(istream& is)
			{
				string portStr;

				is >> portStr;

				return portFromString(portStr);
			}

			// Constructor(s):
			iosync_application(rate updateRate = DEFAULT_UPDATERATE, OSINFO OSInfo=OSINFO());

			// Destructor(s):
			~iosync_application();

			// Methods:
			int execute();
			int execute(const addressPort port);
			int execute(wstring username, string remoteAddress, addressPort remotePort = DEFAULT_PORT, addressPort localPort = DEFAULT_LOCAL_PORT);

			#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
				void executeAsync();
			#endif

			void onCreate(applicationMode mode=MODE_SERVER);
			void onClose();

			void disconnectDevices();
			void closeNetwork();

			void update(rate frameNumber=0) override;

			void updateNetwork();
			void updateDevices();

			void checkDeviceMessages();

			#ifdef IOSYNC_LIVE_COMMANDS
				// This routine is commonly called through 'parseCommands'.
				// Basically, this parses the live-command specified.
				bool parseCommand(applicationCommand command);
				
				// This acts as this application's main routine for user-command parsing.
				// To parse a single command, please use 'parseCommand'.
				void parseCommands();
			#endif

			// Networking related:
			inline bool networkingEnabled() const
			{
				return (network != nullptr);
			}

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
			void onNetworkClientConnected(networkEngine& engine, player& p) override;
			void onNetworkClientTimedOut(networkEngine& engine, player& p) override;
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

			#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
				mutex asyncExecutionMutex;
			#endif
	};
}