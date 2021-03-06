#pragma once

// Preprocessor related:
#ifdef _DEBUG
	#define IOSYNC_TESTMODE
#endif

/*
	This determines if command-line input is
	enabled when no configuration-file is present.

	This does not stop the user from manually enabling
	console-input through their configuration-file.
*/

#define IOSYNC_CONSOLE_INPUT
#define IOSYNC_SAFE

#ifdef IOSYNC_TESTMODE
	//#define IOSYNC_FAST_TESTMODE
	
	#ifdef IOSYNC_FAST_TESTMODE
		#define IOSYNC_FAST_TESTMODE_SINGLE_INPUT
		#define IOSYNC_FAST_TESTMODE_DOLPHIN_TEST
	#endif
#endif

#define IOSYNC_DEVICE_GAMEPAD

#ifdef IOSYNC_DEVICE_GAMEPAD
	// Gamepads are currently auto-detected by default.
	#define IOSYNC_DEVICE_GAMEPAD_AUTODETECT
#endif

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
#define _CRT_NON_CONFORMING_SWPRINTFS

// Includes:
#include "platform.h"

#include "application/application.h"

#include "exceptions.h"
#include "iosync_application_exceptions.h"

#include "networking/networking.h"
#include "networking/networkEngine.h"

#include "devices/devices.h"
#include "devices/keyboard.h"
#include "devices/gamepad.h"

// QuickLib:
#include <QuickLib/QuickINI/QuickINI.h>

// Standard library:
#include <string>
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <list>
#include <map>

#include <chrono>

#if defined(IOSYNC_ALLOW_ASYNC_EXECUTE) || defined(IOSYNC_LIVE_COMMANDS)
	#include <mutex>
	#include <thread>
#endif

// This is just so endless keyboard loops don't occur:
#if !defined(IOSYNC_FAST_TESTMODE) && defined(KEYBOARD_IMPLEMENTED)
	#define IOSYNC_DEVICE_KEYBOARD
#endif

// Windows-specific:
#ifdef PLATFORM_WINDOWS
	// This specifies if 'sharedWindow' is implemented.
	#define IOSYNC_SHAREDWINDOW_IMPLEMENTED
	
	// If disabled, fallbacks may be used to compensate for the lack of wide I/O streams.
	// Behavior with this disabled is partially undefined.
	#define IOSYNC_WIDE_IO
#endif

#ifdef PROCESS_MANAGER_IMPLEMENTED
	// This specifies if this application supports management of external processes.
	#define IOSYNC_PROCESSES_AVAILABLE
	
	#ifdef PROCESS_MANAGER_SYNCHRONIZED_PROCESS_IMPLEMENTED
		// This specifies that process synchronization is allowed.
		#define IOSYNC_ALLOW_PROCESS_SYNCHRONIZATION
	#endif
#endif

// Libraries:
#ifdef PLATFORM_WINDOWS
	// Required for advanced process management.
	#pragma comment(lib, "psapi.lib")
#endif

// Namespace(s):
using namespace std;
using namespace chrono;
using namespace quickLib;
using namespace iosync::networking;
using namespace iosync::exceptions::applicationExceptions;

//using namespace quickLib::sockets;

namespace iosync
{
	// Typedefs:
	typedef unsigned short applicationMode; // unsigned char
	
	// The 'processID' typedef allows for platform independent process management:
	#ifdef IOSYNC_PROCESSES_AVAILABLE
		#ifdef PROCESS_MANAGER_IMPLEMENTED
			typedef process::nativeID processID;
		#else
			#error Unable to resolve native process-identifier.
		#endif

		typedef vector<processID> processes;

		#ifdef IOSYNC_ALLOW_PROCESS_SYNCHRONIZATION
			typedef vector<process::synchronized_process> synchronized_processes;
		#endif
	#endif

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
		// Namespace(s):
		using namespace devices;

		// Typedefs:
		typedef unsigned char deviceType;

		// The type used for device sub-messages.
		typedef unsigned char deviceMessageType; // unsigned short

		// Aliases:
		using kbd = keyboard;
		using gp = gamepad;

		// Enumerator(s):
		enum deviceTypes : deviceType
		{
			DEVICE_TYPE_NOT_FOUND,
			DEVICE_TYPE_KEYBOARD,
			DEVICE_TYPE_GAMEPAD,
		};

		enum deviceMessageTypes : deviceMessageType
		{
			DEVICE_NETWORK_MESSAGE_CONNECT,
			DEVICE_NETWORK_MESSAGE_DISCONNECT,
			DEVICE_NETWORK_MESSAGE_ENTRIES,
			DEVICE_NETWORK_MESSAGE_INVALID,
		};

		enum gamepadMetrics : unsigned long long
		{
			GAMEPAD_DEFAULT_TIMEOUT = 15000,
		};

		// Structures:
		struct deviceConfiguration
		{
			// Constructor(s):
			deviceConfiguration
			(
				bool kbdEnabled=false,
				bool gpdsEnabled=true,
				unsigned char maximum_gpds=(unsigned char)MAX_GAMEPADS

				// Extensions:
				#ifdef GAMEPAD_VJOY_ENABLED
					, bool vJoy=true, UINT vJoy_DevOffset=0
				#endif
			);

			// Fields:
			unsigned char max_gamepads; // gamepadID

			bool keyboardEnabled;
			bool gamepadsEnabled;

			#ifdef GAMEPAD_VJOY_ENABLED
				UINT vJoy_DeviceOffset;
				
				bool vJoyEnabled;
			#endif
		};

		struct connectedDevices final : deviceConfiguration
		{
			// Fields:

			// Devices:
			kbd* keyboard;
			gp* gamepads[MAX_GAMEPADS];

			// A set of reserved gamepad device-identifiers. This is used
			// to ensure clients don't repeatedly request to connect a gamepad.
			unordered_set<gamepadID> reservedGamepads;

			// The "timeout" for gamepads.
			milliseconds gamepadTimeout;

			// Constructor(s):
			connectedDevices
			(
				milliseconds gamepadTimeout=(milliseconds)GAMEPAD_DEFAULT_TIMEOUT,
				bool kbdEnabled=false,
				bool gpdsEnabled=true,
				unsigned char max_gpds=(unsigned char)MAX_GAMEPADS
			);
			
			// Destructor(s):
			~connectedDevices();

			void destroyKeyboard();

			void destroyGamepad(gp* pad, bool checkArray=true);
			void destroyGamepad(gamepadID identifier);

			inline void destroyGamepads()
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					destroyGamepad(i);
				}

				// Clear the reserved-gamepad container.
				reservedGamepads.clear();

				return;
			}

			// Methods:

			// This acts as the main update routine for this structure.
			void update(iosync_application* program);

			// This will update the 'keyboard' object.
			void updateKeyboard(iosync_application* program);

			// This will update every 'gamepad' that's connected.
			void updateGamepads(iosync_application* program);

			// This command will connect every device.
			void connect(iosync_application* program);

			// This command will disconnect every device.
			void disconnect();

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

			// These call-backs are executed whenever their specific device-type's connection has been disrupted:
			void onGamepadConnectionDisrupted(iosync_application* program, gamepadID identifier, gamepadID remoteIdentifier) const;
			void onKeyboardConnectionDisrupted(iosync_application* program) const;

			// This command specifies if a gamepad-connection should be interrupted.
			// In the event the device was interrupted, operations on, or to create that device should not continue.
			inline bool interruptGamepad(iosync_application* program, gamepadID identifier, gamepadID remoteIdentifier) const
			{
				// Check if everything's normal:
				if (gamepadsEnabled && gamepadsConnected() < max_gamepads)
					return false;

				onGamepadConnectionDisrupted(program, identifier, remoteIdentifier);

				// Return the default response.
				return true;
			}

			// This command specifies if a keyboard-connection should be interrupted.
			// In the event the device was interrupted, operations on, or to create that device should not continue.
			inline bool interruptKeyboard(iosync_application* program) const
			{
				// Check if everything's normal:
				if (keyboardEnabled)
					return false;

				onKeyboardConnectionDisrupted(program);

				// Return the default response.
				return true;
			}

			// These are call-backs which get executed when a gamepad is connected, or disconnected, respectively:
			void onGamepadConnected(iosync_application* program, gp* pad);
			void onGamepadDisconnected(iosync_application* program, gp* pad);

			// This command is "unsafe", please check against 'gamepadConnected' before using this.
			bool disconnectLocalGamepad(iosync_application* program, gamepadID identifier);

			inline bool disconnectLocalGamepads(iosync_application* program, player* owner)
			{
				bool response = false;

				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepads[i] != nullptr && gamepads[i]->owner == owner)
					{
						if (disconnectGamepad(program, gamepads[i]->localGamepadNumber))
						{
							response = true;

							gamepads[i] = nullptr;
						}
					}
				}

				// Return the calculated response.
				return response;
			}

			// Unlike 'disconnectLocalGamepad', this will check if a device is connected, so it is "safer".
			// If you need to manually disconnect a gamepad using its local identifier, please use 'disconnectLocalGamepad' instead.
			bool disconnectGamepad(iosync_application* program, gamepadID identifier);

			// The return-value of this command determines if one or more gamepads are still connected.
			inline bool disconnectGamepads(iosync_application* program, gamepadID identifier)
			{
				// Local variable(s):
				bool response = true;

				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (!disconnectGamepad(program, identifier))
					{
						response = false;
					}
				}

				return response;
			}

			// This command allows you to manually connect the 'keyboard' object.
			kbd* connectKeyboard(iosync_application* program);

			// This command allows you to manually connect a 'gamepad' object.
			gamepad* connectGamepad(iosync_application* program, gamepadID identifier, gamepadID remoteIdentifier);

			gamepad* connectGamepad(iosync_application* program, gamepadID remoteIdentifier, bool checkRealDevices);

			// This implementation is commonly used by servers.
			gamepad* connectGamepad(iosync_application* program, bool checkRealDevices);

			gamepad* connectGamepad(iosync_application* program, gamepadID remoteIdentifier);

			gamepad* connectGamepad(iosync_application* program);

			// This command will manually connect every 'gamepad' possible.
			// Clients should only ever use this for testing purposes.
			// Servers may call this at their own leisure.
			// This will return a pointer to the internal 'gamepads' array.
			inline gamepad** connectGamepads(iosync_application* program)
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					connectGamepad(program, i, i);
				}

				return gamepads;
			}

			inline gamepadID getNextGamepadID(bool checkRealDevices) const
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if
					(
						(gamepads[i] == nullptr)

						#ifdef PLATFORM_WINDOWS
							// If requested, ensure a real device isn't plugged in.
							&& (!checkRealDevices || (!gp::__winnt__pluggedIn(i)))
						#endif
					)
					{
						return i;
					}
				}

				return GAMEPAD_ID_NONE;
			}

			// This will map a remote gamepad-identifier to a local 'gamepad' object.
			// If a device could not be found, this will return 'nullptr'.
			inline gamepad* getGamepad(gamepadID identifier) const
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i) && gamepads[i]->remoteGamepadNumber == identifier)
					{
						return gamepads[i];
					}
				}

				return nullptr;
			}

			// This will retrieve a gamepad using its local identifier.
			// This is mainly provided for future-proofing,
			// there isn't much of a point to calling this yet.
			// If a device couldn't be found, this will return 'nullptr'.
			inline gamepad* getLocalGamepad(gamepadID identifier)
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i) && gamepads[i]->localGamepadNumber == identifier)
					{
						return gamepads[i];
					}
				}

				return nullptr;
			}

			inline bool hasGamepadConnected()
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i))
						return true;
				}

				// Return the default response.
				return false;
			}

			inline size_t gamepadsConnected() const
			{
				size_t count = 0;

				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i))
						count += 1;
				}

				return count;
			}

			// Networking related:

			// This may be used to connect a specific device based on its 'deviceType',
			// as well as potentially reading from the 'socket' object specified for extension-specific input.
			// The 'extended' argument specifies if this routine is allowed to read forward in the 'socket' object's input-stream.
			bool connectDeviceThroughNetwork(iosync_application* program, QSocket& socket, deviceType devType, bool extended);

			// Similar to 'connectDevice', this will disconnect the device of the type specified,
			// using the 'socket' object for extension-specific data. Extensions may not be applied if 'extended' is set to 'false'.
			bool disconnectDeviceThroughNetwork(iosync_application* program, QSocket& socket, deviceType devType, bool extended);

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

			// This command is used to begin a device-message for a 'gamepad' object.
			// Please note that this command is only needed when referring to specific 'gamepads',
			// for messages which deal with all 'gamepads', this is not needed, and may cause undefined behavior.
			// When using this, please do not supply an "extension flag", this is already handled for you.
			inline headerInfo beginGamepadDeviceMessage(networkEngine& engine, QSocket& socket, gamepadID identifier, deviceMessageType devMsgType)
			{
				// Get the header information from the main routine.
				auto info = beginDeviceMessage(engine, socket, DEVICE_TYPE_GAMEPAD, devMsgType);

				// Mark this message as "extended".
				socket.writeBool(true);

				// Write the gamepad identifier specified.
				socket.write<serializedGamepadID>((serializedGamepadID)identifier);

				// Return the header-information.
				return info;
			}

			// Serialization related:
			void serializeTo(networkEngine& engine, QSocket& socket);
			void serializeKeyboard(networkEngine& engine, QSocket& socket);
			void serializeGamepad(networkEngine& engine, QSocket& socket, gamepadID gamepadLocation, gamepadID remoteIdentifier);

			inline void serializeGamepads(networkEngine& engine, QSocket& socket)
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i) && gamepads[i]->canDetect() && gamepads[i]->hasRealState()) // gamepads[i]->canSimulate() || hasState()
					{
						serializeGamepad(engine, socket, i, gamepads[i]->remoteGamepadNumber);
					}
				}

				return;
			}

			inline void serializeTo(networkEngine& engine)
			{
				serializeTo(engine, engine.socket);

				return;
			}

			void serializeConnectMessage(QSocket& socket, deviceType device);
			void serializeDisconnectMessage(QSocket& socket, deviceType device);

			// The 'existenceCheck' argument specifies if this connection
			// message is meant to notify a client of a gamepad's existence.
			// If unsure, don't explicitly set/supply this argument.
			void serializeGamepadConnectMessage(QSocket& socket, gamepadID identifier, bool existenceCheck=false);
			void serializeGamepadDisconnectMessage(QSocket& socket, gamepadID identifier);

			void serializeIODevice(QSocket& socket, IODevice* device);

			// Message generation related:

			// These act as the default connection/disconnection message-generators for devices:
			outbound_packet generateConnectMessage(networkEngine& engine, QSocket& socket, deviceType device, const address& realAddress=address(), const address& forwardAddress=address());
			outbound_packet generateDisconnectMessage(networkEngine& engine, QSocket& socket, deviceType device, const address& realAddress=address(), const address& forwardAddress=address());

			// These may be used to explicitly specify gamepad identifiers when connecting and disconnecting gamepads:
			outbound_packet generateGamepadConnectMessage(networkEngine& engine, QSocket& socket, gamepadID identifier, const address& realAddress=address(), const address& forwardAddress=address());
			outbound_packet generateGamepadDisconnectMessage(networkEngine& engine, QSocket& socket, gamepadID identifier, const address& realAddress=address(), const address& forwardAddress=address());

			// This can be used to notify people of a gamepad already being connected.
			// This is useful when a client doesn't know about connected gamepads.
			// "Exists" messages are an alternate form of connection-message. Disconnection is still required.
			outbound_packet generateGamepadExistsMessage(networkEngine& engine, QSocket& socket, gamepadID identifier, const address& realAddress=address(), const address& forwardAddress=address());

			// These provide reliable packets for keyboard and gamepad states:
			outbound_packet generateKeyboardState(networkEngine& engine, QSocket& socket, const address& realAddress=address(), const address& forwardAddress=address());
			outbound_packet generateGamepadState(networkEngine& engine, QSocket& socket, gamepadID identifier, gamepadID remoteIdentifier, const address& realAddress=address(), const address& forwardAddress=address());

			// Parsing/deserialization related:
			deviceType parseDeviceMessage(iosync_application* program, QSocket& socket, const messageHeader& header, const messageFooter& footer);

			bool parseConnectMessage(iosync_application* program, QSocket& socket, deviceType device, const messageHeader& header, const messageFooter& footer);
			bool parseDisconnectMessage(iosync_application* program, QSocket& socket, deviceType device, const messageHeader& header, const messageFooter& footer);

			deviceMessageType parseKeyboardMessage(iosync_application* program, QSocket& socket, deviceMessageType subMessageType, const messageHeader& header, const messageFooter& footer);
			deviceMessageType parseGamepadMessage(iosync_application* program, QSocket& socket, deviceMessageType subMessageType, const messageHeader& header, const messageFooter& footer);

			void parseIODevice(iosync_application* program, QSocket& socket, IODevice* device, const messageHeader& header, const messageFooter& footer);

			// Sending related:

			// This may be used by hosts to connect all active devices to a client.
			// This is very useful when dealing with setups where clients and hosts both handle device I/O.
			size_t sendActiveDevices(networkEngine& engine, QSocket& socket, const player& p);

			inline size_t sendConnectMessage(networkEngine& engine, QSocket& socket, deviceType device, networkDestinationCode destination = DEFAULT_DESTINATION, bool resetLength=true)
			{
				return engine.sendMessage(socket, generateConnectMessage(engine, socket, device), destination, resetLength);
			}

			inline size_t sendDisconnectMessage(networkEngine& engine, QSocket& socket, deviceType device, networkDestinationCode destination = DEFAULT_DESTINATION, bool resetLength=true)
			{
				return engine.sendMessage(socket, generateDisconnectMessage(engine, socket, device), destination, resetLength);
			}

			inline size_t sendGamepadConnectMessage(networkEngine& engine, QSocket& socket, networkDestinationCode destination = DEFAULT_DESTINATION, bool resetLength = true)
			{
				return engine.sendMessage(socket, generateConnectMessage(engine, socket, DEVICE_TYPE_GAMEPAD), destination, resetLength);
			}

			// This can be used to manually request connection of a 'gamepad' device.
			inline size_t sendGamepadConnectMessage(networkEngine& engine, QSocket& socket, gamepadID identifier, networkDestinationCode destination = DEFAULT_DESTINATION, bool resetLength=true)
			{
				return engine.sendMessage(socket, generateGamepadConnectMessage(engine, socket, identifier), destination, resetLength);
			}

			inline size_t sendGamepadDisconnectMessage(networkEngine& engine, QSocket& socket, gamepadID identifier, networkDestinationCode destination = DEFAULT_DESTINATION, bool resetLength=true)
			{
				return engine.sendMessage(socket, generateGamepadDisconnectMessage(engine, socket, identifier), destination, resetLength);
			}

			// This acts as a macro for requesting device-connections.
			inline size_t sendConnectionRequests(networkEngine& engine, networkDestinationCode destination = DEFAULT_DESTINATION)
			{
				// Local variable(s):

				// The number of bytes sent.
				size_t sent = 0;

				// The socket we'll use to send messages.
				auto& socket = engine.socket;

				#if defined(IOSYNC_DEVICE_KEYBOARD)
					if (keyboardEnabled)
					{
						// Request for remote-keyboard access.
						sendConnectMessage(engine, socket, DEVICE_TYPE_KEYBOARD, destination);
					}
				#endif

				#if defined(IOSYNC_DEVICE_GAMEPAD) && !defined(IOSYNC_DEVICE_GAMEPAD_AUTODETECT)
					if (gamepadsEnabled)
					{
						// Attempt to reserve a remote gamepad-identifier,
						// connecting the virtual gamepad if possible.
						sendGamepadConnectMessage(engine, socket, destination);
					}
				#endif

				// Return the number of bytes sent.
				return sent;
			}

			// This will send the active serializable data this manager produces.
			// Please call 'sendConnectionRequests' (Or similar) before calling this.
			size_t sendTo(networkEngine& engine, networkDestinationCode destination);
			size_t reliableSendTo(networkEngine& engine, networkDestinationCode destination);

			size_t sendTo(iosync_application* program, networkEngine& engine);
			size_t reliableSendTo(iosync_application* program, networkEngine& engine);

			inline bool keyboardConnected() const
			{
				return ((keyboard != nullptr) && keyboard->connected());
			}

			// This will tell you if the local gamepad at the "location" specified is connected.
			// To check if a gamepad is connected using a remote-identifier, please use 'remoteGamepadConnected'.
			// This is primarily useful for routines which iterate through the 'gamepads' array.
			inline bool gamepadConnected(const gamepadID gamepadLocation) const
			{
				return ((gamepads[gamepadLocation] != nullptr) && gamepads[gamepadLocation]->connected());
			}

			// This will tell you if a connected gamepad has the local-identifier specified.
			inline bool localGamepadConnected(const gamepadID identifier) const
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i) && gamepads[i]->localGamepadNumber == identifier)
					{
						return true;
					}
				}

				// Return the default response.
				return false;
			}

			// This will tell you if a connected gamepad has the remote-identifier specified.
			inline bool remoteGamepadConnected(const gamepadID identifier) const
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i))
					{
						if (gamepads[i]->remoteGamepadNumber == identifier)
						{
							return true;
						}
					}
				}

				// Return the default response.
				return false;
			}

			// This will determine if a local gamepad identifier is reserved, or otherwise taken.
			inline bool gamepadReserved(const gamepadID identifier) const
			{
				// Check if the 'reservedGamepads' container has this identifier:
				if (reservedGamepads.find(identifier) != reservedGamepads.end())
				{
					// This is explicitly reserved; we can not use it.
					return true;
				}

				gamepadID lowestRemoteID = MAX_GAMEPADS;

				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i) && gamepads[i]->remoteGamepadNumber < lowestRemoteID)
					{
						lowestRemoteID = gamepads[i]->remoteGamepadNumber;
					}
				}

				return (identifier > lowestRemoteID);
			}

			// This will tell the caller if at least one virtual gamepad is connected.
			inline bool gamepadConnected() const
			{
				// Check if at least one gamepad is connected:
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i))
						return true;
				}

				// Return the default response.
				return false;
			}

			inline bool hasDeviceConnected() const
			{
				return keyboardConnected() || gamepadConnected();
			}

			inline bool playerHasDevice(player* p) const
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepadConnected(i))
					{
						if (gamepads[i]->owner == p)
						{
							return true;
						}
					}
				}

				// Return the default response.
				return false;
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

				MODE_DIRECT_CLIENT = 2,
				MODE_DIRECT_SERVER = 3,
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
			
			struct applicationConfiguration final : deviceManagement::deviceConfiguration
			{
				// Constant variable(s):
				static const wstring DEFAULT_PATH;

				// INI sections:
				static const wstring APPLICATION_SECTION;
				static const wstring DEVICES_SECTION;
				static const wstring NETWORK_SECTION;

				#ifdef PLATFORM_WINDOWS
					static const wstring XINPUT_SECTION;
				#endif

				// INI properties:

				// Application:
				static const wstring APPLICATION_MODE;
				static const wstring APPLICATION_USECMD;

				// This is used to redirect to another configuration-file.
				static const wstring APPLICATION_CONFIG;

				// This is used to describe applications which are synchronized between systems.
				static const wstring APPLICATION_SYNCHRONIZED_APPLICATIONS;

				// Devices:
				static const wstring DEVICES_KEYBOARD;
				static const wstring DEVICES_GAMEPADS;
				static const wstring DEVICES_MAX_GAMEPADS;

				#ifdef GAMEPAD_VJOY_ENABLED
					static const wstring DEVICES_VJOY;
					static const wstring DEVICES_VJOY_OFFSET;
				#endif

				// Networking:

				// This is represented with an IP address / hostname, and optionally, a port.
				// A port may also be supplied without an address if a separator is specified. (Usually ':')
				static const wstring NETWORK_ADDRESS;

				// This may be used to manually supply a port, rather than using 'NETWORK_ADDRESS'.
				// There is no hostname/IP equivalent, as 'NETWORK_ADDRESS' already covers it.
				static const wstring NETWORK_PORT;

				static const wstring NETWORK_USERNAME;

				// Windows-specific
				#ifdef PLATFORM_WINDOWS
					// XInput:
					static const wstring XINPUT_PROCESSES;
				#endif

				// Other:

				// Networking:
				static const wstring NETWORKING_DEFAULT_PORT;

				// Devices:
				static const wstring DEVICES_GAMEPADS_MAXIMUM;

				// Constructor(s):
				applicationConfiguration
				(
					applicationMode internal_mode=MODE_SERVER,
					bool cmdOnly=false,
					bool kbdEnabled=false,
					bool gpdsEnabled=true,
					unsigned char max_gpds=(unsigned char)devices::metrics::MAX_GAMEPADS
				);

				// Destructor(s):
				~applicationConfiguration();

				// Methods:
				void load(const wstring& path=DEFAULT_PATH);
				void save(const wstring& path=DEFAULT_PATH);

				void read(wistream& is);
				void write(wostream& os);

				void read(const INI::INIVariables<wstring>& variables);
				void write(INI::INIVariables<wstring>& variables);

				// Fields:
				representativeAddress remoteAddress;

				wstring username;

				applicationMode mode;

				#ifdef PLATFORM_WINDOWS
					queue<DWORD> PIDs;
				#endif

				#ifdef IOSYNC_ALLOW_PROCESS_SYNCHRONIZATION
					synchronized_processes synchronizedApplications;
				#endif

				// Booleans / Flags:
				bool useCmd;
			};

			// Constant variable(s):
			static const wstring IOSYNC_APPDATA_FOLDER;

			// Global variable(s):

			// The number of application still linked to dynamic libraries.
			// When an application attempts to un-link when there are no other
			// applications linked, all dynamic modules will be discarded.
			static size_t dynamicLinks;

			#ifdef PLATFORM_WINDOWS
				static HMODULE xInputModule;

				#ifdef GAMEPAD_VJOY_DYNAMIC_LINK // GAMEPAD_VJOY_ENABLED
					static HMODULE vJoyModule;
				#endif
			#endif

			#ifdef IOSYNC_LIVE_COMMANDS
				static programList commandTargets;
				static queue<applicationCommand> commandQueue;
				
				static mutex commandMutex;
				static thread commandThread;

				static bool commandThreadRunning;
			#endif

			// Functions:
			static void dynamicLink(iosync_application& application);
			static void dynamicUnlink(iosync_application& application);

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
				transformToLower(portStr);

				// Check if the user didn't request the default port:
				if (portStr != "default" || portStr == "d")
				{
					// Convert the port-string into an integer.
					return (nativePort)stoi(portStr);
				}

				return DEFAULT_PORT;
			}

			static inline nativePort portFromInput(istream& is=cin)
			{
				string portStr;

				is >> portStr;

				return portFromString(portStr);
			}

			// The return-value of this command follows the behavior of the 'representativeAddress' structure's 'parse' command.
			static inline bool addressFromInput(representativeAddress& address, istream& is = cin, addressPort default_port=DEFAULT_PORT)
			{
				string addr_str;

				is >> addr_str;

				return address.parse(addr_str, default_port);
			}

			// This command accepts input from the user, then mutates the remote-address using that data.
			static inline void requestAddress(representativeAddress& remoteAddress, istream& is=cin, ostream& os=cout, addressPort default_port=DEFAULT_PORT)
			{
				do
				{
					os << "Please supply an address: ";

					if (!addressFromInput(remoteAddress, is, default_port))
					{
						remoteAddress.port = requestPort(is, os);
					}
				} while (remoteAddress.IP.empty());

				return;
			}

			static inline nativePort requestPort(istream& is=cin, ostream& os=cout)
			{
				os << "Please supply a port: ";

				return portFromInput(is);
			}

			#ifdef PLATFORM_WINDOWS
				// This command appends the application-data path to the beginning of the string specified.
				// This command may not assume that 'local_path' and 'new_path_out' are different.
				// The return-value of this command determines if it succeeded.
				// If this fails, the 'new_path_out' argument will not be modified.
				static bool __winnt__applyAppDataW(const wstring& local_path, wstring& new_path_out);

				// This command retrieves a PID from a valid process-identification entry.
				static DWORD __winnt__getPIDW(const wstring& entry);

				// This command returns true if the folder already exists, or if a new one was created.
				static bool __winnt__createAppDataFolder();

				// This implementation acts as shorthand for the main implementation.
				// The rules applied to that implementation apply here.
				static inline bool __winnt__applyAppDataW(wstring& new_path_out)
				{
					return __winnt__applyAppDataW(new_path_out, new_path_out);
				}
			#endif

			// Constructor(s):
			iosync_application(rate updateRate = DEFAULT_UPDATERATE, OSINFO OSInfo=OSINFO());

			// Destructor(s):
			~iosync_application();

			// Methods:
			int execute();
			int execute(const addressPort port, const applicationMode mode=MODE_SERVER);
			int execute(const wstring& username, const string& remoteAddress, const addressPort remotePort = DEFAULT_PORT, const addressPort localPort = DEFAULT_LOCAL_PORT, const applicationMode mode=MODE_CLIENT);

			#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
				void executeAsync();
			#endif

			// This command may be used in order to execute/start an application using a configuration-object.
			int applyConfiguration(applicationConfiguration& configuration);

			// This command polls information from the user, rather than the disk.
			// If the 'logChoices' argument is set to 'true', this may write to the disk.
			// The 'configuration' argument should be a valid container
			// used to output the settings provided by the user.
			int applyCommandlineConfiguration(applicationConfiguration& configuration, const bool logChoices=false, const wstring& output_file=wstring());

			void onCreate(applicationMode mode=MODE_SERVER);
			void onClose();

			void disconnectDevices();
			void closeNetwork();

			void update(rate frameNumber=0) override;
			
			void updateNetwork();
			void updateDevices();
			
			#ifdef IOSYNC_ALLOW_PROCESS_SYNCHRONIZATION
				void updateSynchronizedApplications();

				// Every call to this command should have a matching call to 'resumeSynchronizedApplications'
				void suspendSynchronizedApplications();

				// This should only be called after using 'suspendSynchronizedApplications'.
				void resumeSynchronizedApplications();
			#endif
			
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

			// Parsing/deserialization related:
			bool parseNetworkMessage(QSocket& socket, const messageHeader& header, const messageFooter& footer) override;

			// Sending related:
			// Nothing so far.

			// Call-backs:
			void onNetworkConnected(networkEngine& engine) override;
			void onNetworkClientConnected(networkEngine& engine, player& p) override;
			void onNetworkClientTimedOut(networkEngine& engine, player& p) override;
			void onNetworkClosed(networkEngine& engine) override;

			#ifdef IOSYNC_SHAREDWINDOW_IMPLEMENTED
				nativeWindow getWindow() const override;
			#endif

			// Platform-specific extensions:
			#ifdef PLATFORM_WINDOWS
				void __winnt__readFromDevice(RAWINPUT* rawDevice);
			#endif

			inline bool applicationsSuspended() const
			{
				#ifdef IOSYNC_ALLOW_PROCESS_SYNCHRONIZATION
					return synchronizedApplications_suspended;

					/*
					if (synchronizedApplications.empty())
						return false;

					// For now, we'll just check the first application-entry.
					return synchronizedApplications[0].suspended;
					*/
				#else
					return false;
				#endif
			}

			inline bool closed() const
			{
				return !isRunning;
			}

			inline bool multiWayHost() const
			{
				return (mode == MODE_DIRECT_SERVER);
			}

			inline bool multiWayClient() const
			{
				return (mode == MODE_DIRECT_CLIENT);
			}

			inline bool multiWayOperations() const
			{
				return (multiWayClient() || multiWayHost());
			}

			inline bool allowDeviceDetection() const
			{
				return (mode == MODE_CLIENT || multiWayOperations());
			}

			inline bool allowDeviceSimulation() const
			{
				return (mode == MODE_SERVER || multiWayOperations());
			}

			inline bool reserveLocalGamepads() const
			{
				return allowDeviceSimulation();
			}

			inline bool synchronizeApplications() const
			{
				#ifdef IOSYNC_ALLOW_PROCESS_SYNCHRONIZATION
					return multiWayOperations();
				#else
					return false;
				#endif
			}

			// Fields:

			// Network I/O.
			networkEngine* network;

			// Input devices.
			deviceManagement::connectedDevices devices;

			// The "mode" of this application.
			applicationMode mode;

			#ifdef IOSYNC_SHAREDWINDOW_IMPLEMENTED
				// The window this application is tied to.
				nativeWindow window;
			#endif

			#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
				mutex asyncExecutionMutex;
			#endif
		protected:
			// Fields (Protected):
			#ifdef IOSYNC_ALLOW_PROCESS_SYNCHRONIZATION
				// Process identifiers representing synchronized applications.
				synchronized_processes synchronizedApplications;

				bool synchronizedApplications_suspended = false;

				// To be refactored.
				const milliseconds synchronizedApplicationRefreshTime = (milliseconds)500;

				high_resolution_clock::time_point synchronizedApplicationRefreshTimer; // synchronizedApplicationThreadEnumerationTimer
			#endif
	};
}