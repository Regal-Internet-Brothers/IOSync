// Includes:
#include "iosync.h"

// Standard library:
#include <stack>

// Namespace(s):
namespace iosync
{
	namespace sharedWindow
	{
		// Global variable(s):
		nativeWindow windowInstance = WINDOW_NONE;

		#ifdef PLATFORM_WINDOWS
			bool __winnt__classRegisted = false;
		#endif

		// Functions:
		nativeWindow open(OSINFO OSInfo)
		{
			#ifdef PLATFORM_WINDOWS_EXTENSIONS
				if (isOpen())
					return windowInstance;

				if (!__winnt__classRegisted)
					__winnt__registerClass(OSInfo);

				windowInstance = CreateWindowEx
				(
					WS_EX_CLIENTEDGE,
					__winnt__windowClassName,
					NULL, WS_OVERLAPPEDWINDOW,
					CW_USEDEFAULT, CW_USEDEFAULT,

					128, 128, NULL, NULL,
					OSInfo.hInstance, NULL
				);

				if (windowInstance == WINDOW_NONE)
					MessageBox(NULL, "O_O", "Snooping as usual, I see.", MB_ICONEXCLAMATION | MB_OK);

				//ShowWindow(windowInstance, OSInfo.nCmdShow);
				UpdateWindow(windowInstance);
			#endif

			return windowInstance;
		}

		void update(application* program)
		{
			MSG message;

			// The 'windowInstance' argument may cause problems.
			GetMessage(&message, windowInstance, 0, 0);

			TranslateMessage(&message);
			DispatchMessage(&message);

			return;
		}

		bool isOpen()
		{
			return (windowInstance != WINDOW_NONE);
		}

		// Windows-specific:
		#ifdef PLATFORM_WINDOWS
			LRESULT CALLBACK __winnt__WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
			{
				//networkLog << "Message: " << msg << endl;

				switch(msg)
				{
					case WM_CLOSE:
						DestroyWindow(hwnd);

						break;
					case WM_DESTROY:
						PostQuitMessage(0);

						break;
					default:
						return DefWindowProc(hwnd, msg, wParam, lParam);
				}

				return 0;
			}

			bool __winnt__registerClass(OSINFO OSInfo)
			{
				WNDCLASSEX wc;

				wc.cbSize        = sizeof(WNDCLASSEX);
				wc.style         = 0;
				wc.lpfnWndProc   = &__winnt__WndProc;
				wc.cbClsExtra    = 0;
				wc.cbWndExtra    = 0;
				wc.hInstance     = OSInfo.hInstance;
				wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
				wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
				wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
				wc.lpszMenuName  = NULL;
				wc.lpszClassName = __winnt__windowClassName;
				wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

				// Attempt to register the window-class, and return the result.
				return __winnt__classRegisted = (RegisterClassEx(&wc) == ATOM());
			}
		#endif
	}

	// Exception types:
	namespace exceptions
	{
		namespace applicationExceptions
		{
			// Classes:

			// noWindowException:

			// Constructor(s):
			noWindowException::noWindowException(application* program)
				: applicationException(program) { /* Nothing so far. */ }

			// Methods:
			const string noWindowException::message() const throw()
			{
				return "Unable to find/open native window-instance.";
			}
		}
	}

	namespace deviceManagement
	{
		// Structures:

		// connectedDevices:

		// Constructor(s):
		connectedDevices::connectedDevices() : keyboard(nullptr)
		{
			for (auto i = 0; i < MAX_GAMEPADS; i++)
				gamepads[i] = nullptr;
		}

		// Destructor(s):
		connectedDevices::~connectedDevices()
		{
			disconnect();
		}

		void connectedDevices::destroyKeyboard()
		{
			deviceInfo << "Attempting to destroy keyboard device-instance..." << endl;

			delete keyboard; keyboard = nullptr;

			deviceInfo << "Device-instance destroyed." << endl;

			return;
		}

		void connectedDevices::destroyGamepad(gp* pad, bool checkArray)
		{
			delete pad;

			if (checkArray)
			{
				for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
				{
					if (gamepads[i] == pad)
					{
						gamepads[i] = nullptr;

						break;
					}
				}
			}

			return;
		}

		void connectedDevices::destroyGamepad(gamepadID identifier)
		{
			deviceInfo << "Attempting to destroy gamepad device-instance..." << endl;

			destroyGamepad(gamepads[identifier], false); gamepads[identifier] = nullptr;

			deviceInfo << "Device-instance destroyed." << endl;

			return;
		}

		// Methods:
		void connectedDevices::update(iosync_application* program)
		{
			updateKeyboard(program);
			updateGamepads(program);

			return;
		}

		void connectedDevices::updateKeyboard(iosync_application* program)
		{
			if (keyboardConnected())
				keyboard->update(program);

			return;
		}

		void connectedDevices::updateGamepads(iosync_application* program)
		{
			#if defined(XINPUT_DEVICE_GAMEPAD) && defined(XINPUT_DEVICE_GAMEPAD_AUTODETECT)
				// Check if we're detecting gamepads:
				if (program->allowDeviceDetection())
				{
					// Check for state changes:
					for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
					{
						bool connected = gamepadConnected(i);

						if (!connected && gamepadReserved(i))
						{
							continue;
						}

						// Check if a gamepad with the 'i' identifier is connected:
						if (!connected)
						{
							if (gp::__winnt__pluggedIn(i))
							{
								// Tell the remote host that a gamepad was connected.
								sendGamepadConnectMessage(*program->network, program->network->socket, DESTINATION_HOST);

								//sendGamepadConnectMessage(*program->network, program->network->socket, i, DESTINATION_HOST);

								// Add this identifier, so we don't bother trying to connect with it redundantly.
								reservedGamepads.insert(i);
							}
						}
						else
						{
							// Check for a real device-disconnection:
							if (!gamepads[i]->connected_real()) // gp::__winnt__pluggedIn(i)
							{
								// Attempt to disconnect the virtual device:
								if (disconnectLocalGamepad(program, i))
								{
									// Disconnection was successful, tell the remote host.
									sendGamepadDisconnectMessage(*program->network, program->network->socket, i, DESTINATION_HOST);

									// Remove this identifier, so it may be used again.
									reservedGamepads.erase(i);
								}
							}
						}
					}
				}
			#endif

			for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
			{
				if (gamepadConnected(i))
					gamepads[i]->update(program);
			}

			return;
		}

		void connectedDevices::connect(iosync_application* program)
		{
			// Connect all devices:
			connectKeyboard(program);
			connectGamepads(program);

			return;
		}

		void connectedDevices::disconnect()
		{
			// Disconnect all devices:
			disconnectKeyboard();

			return;
		}

		void connectedDevices::onGamepadConnected(iosync_application* program, gp* pad)
		{
			#ifdef PLATFORM_WINDOWS
				if (program->allowDeviceSimulation())
					gp::__winnt__activateGamepad(pad->localGamepadNumber);
			#endif

			return;
		}

		void connectedDevices::onGamepadDisconnected(iosync_application* program, gp* pad)
		{
			#ifdef PLATFORM_WINDOWS
				if (program->allowDeviceSimulation())
				{
					gp::__winnt__deactivateGamepad(pad->localGamepadNumber);

					// We don't have any gamepads connected,
					// close the shared memory segment:
					if (!hasGamepadConnected())
						gp::__winnt__closeSharedMemory();
				}
			#endif

			return;
		}

		bool connectedDevices::disconnectLocalGamepad(iosync_application* program, gamepadID identifier)
		{
			if (gamepads[identifier]->disconnect())
			{
				// Execute the gamepad disconnection call-back.
				onGamepadDisconnected(program, gamepads[identifier]);

				destroyGamepad(identifier);

				return true;
			}

			// Return the default response.
			return false;
		}

		// This will disconnect a gamepad using its "remote-identifier".
		// To use a local/"real" identifier, please use 'disconnectLocalGamepad'.
		bool connectedDevices::disconnectGamepad(iosync_application* program, gamepadID identifier)
		{
			for (gamepadID i = 0; i < MAX_GAMEPADS; i++)
			{
				if (!gamepadConnected(i))
					continue;

				if (gamepads[i]->remoteGamepadNumber == identifier)
				{
					return disconnectLocalGamepad(program, identifier);
				}
			}

			// Return the default response.
			return false;
		}

		kbd* connectedDevices::connectKeyboard(iosync_application* program)
		{
			// Check if we need to create a new 'gp' object:
			if (keyboard == nullptr)
			{
				deviceInfo << "Attempting to create keyboard-instance..." << endl;

				// Allocate a new 'kbd' on the heap.
				keyboard = new kbd(program->allowDeviceDetection(), program->allowDeviceSimulation());

				deviceInfo << "Keyboard-instance created." << endl;
			}
			else if (keyboard->connected())
			{
				deviceInfo << "Keyboard already connected, continuing anyway..." << endl;

				return keyboard;
			}

			deviceInfo << "Attempting to connect keyboard-device..." << endl;

			// Attempt to connect our keyboard:
			if (!keyboard->connect())
			{
				deviceInfo << "Failed to connect keyboard, destroying device-instance..." << endl;

				// We couldn't connect it, destroy the object.
				destroyKeyboard();

				deviceInfo << "Device-instance destroyed." << endl;

				// Tell the user we couldn't connect it.
				return nullptr;
			}

			deviceInfo << "Keyboard connected." << endl;

			// Return the 'keyboard' object to the user.
			return keyboard;
		}

		gamepad* connectedDevices::connectGamepad(iosync_application* program, gamepadID identifier, gamepadID remoteIdentifier)
		{
			// Check if we need to create a new 'gp' object:
			if (gamepads[identifier] == nullptr)
			{
				#ifdef PLATFORM_WINDOWS
					if (program->allowDeviceSimulation())
					{
						if (!gp::__winnt__autoOpenSharedMemory())
							return nullptr;
					}
				#endif

				deviceInfo << "Attempting to create gamepad-instance..." << endl;

				// Allocate a new 'gp' on the heap.
				gamepads[identifier] = new gp(identifier, remoteIdentifier, program->allowDeviceDetection(), program->allowDeviceSimulation());

				deviceInfo << "Gamepad-instance created." << endl;
			}
			else if (gamepads[identifier]->connected())
			{
				deviceInfo << "Gamepad already connected, continuing anyway..." << endl;

				return gamepads[identifier];
			}

			deviceInfo << "Attempting to connect gamepad-device..." << endl;

			// Attempt to connect our gamepad:
			if (!gamepads[identifier]->connect())
			{
				deviceInfo << "Failed to connect gamepad, destroying device-instance..." << endl;

				// We couldn't connect it, destroy the object.
				destroyGamepad(identifier);

				deviceInfo << "Device-instance destroyed." << endl;

				// Tell the user we couldn't connect it.
				return nullptr;
			}

			onGamepadConnected(program, gamepads[identifier]);

			deviceInfo << "Gamepad connected." << endl;

			// Return the newly connected 'gamepad' object to the user.
			return gamepads[identifier];
		}

		gamepad* connectedDevices::connectGamepad(iosync_application* program, gamepadID remoteIdentifier)
		{
			// Local variable(s):

			// Get the next gamepad identifier.
			auto identifier = getNextGamepadID(program->reserveLocalGamepads());

			// Check if we can connect this device:
			if (identifier == GAMEPAD_ID_NONE)
			{
				// We are unable to connect more gamepads.
				return nullptr;
			}

			return connectGamepad(program, identifier, remoteIdentifier);
		}

		// This implementation is commonly used by servers.
		gamepad* connectedDevices::connectGamepad(iosync_application* program)
		{
			// Local variable(s):

			// Get the next gamepad identifier.
			auto identifier = getNextGamepadID(program->reserveLocalGamepads());

			// Check if we can connect this device:
			if (identifier == GAMEPAD_ID_NONE)
			{
				// We are unable to connect more gamepads.
				return nullptr;
			}

			return connectGamepad(program, identifier, identifier);
		}

		// Networking related:
		bool connectedDevices::connectDeviceThroughNetwork(iosync_application* program, QSocket& socket, deviceType devType, bool extended)
		{
			deviceInfo << "Connecting device (Type: " << (unsigned short)devType << ")..." << endl;

			switch (devType)
			{
				case DEVICE_TYPE_KEYBOARD:
					if (connectKeyboard(program) == nullptr)
					{
						return false;
					}

					if (program->mode == iosync_application::MODE_SERVER)
					{
						sendConnectMessage(*program->network, socket, devType, DESTINATION_REPLY);
					}

					break;
				case DEVICE_TYPE_GAMEPAD:
					{
						gamepadID identifier;

						if (extended)
						{
							identifier = (gamepadID)socket.read<serializedGamepadID>();
							bool remotelyConnected = remoteGamepadConnected(identifier);

							if (program->allowDeviceDetection() && remotelyConnected)
							{
								reservedGamepads.erase(identifier);
							}

							if (remotelyConnected)
							{
								sendGamepadDisconnectMessage(*program->network, socket, identifier, DESTINATION_REPLY);

								return false;
							}
						}
						else
						{
							identifier = getNextGamepadID(program->reserveLocalGamepads());
						}

						if (identifier == GAMEPAD_ID_NONE)
						{
							return false;
						}

						// Connect a gamepad using the identifier specified.
						if (!connectGamepad(program, identifier))
						{
							return false;
						}

						if (program->mode == iosync_application::MODE_SERVER)
						{
							sendGamepadConnectMessage(*program->network, socket, identifier, DESTINATION_REPLY);
						}
					}

					break;
				default:
					// This device-type is not supported.
					return false;
			}

			// Return the default response.
			return true;
		}

		bool connectedDevices::disconnectDeviceThroughNetwork(iosync_application* program, QSocket& socket, deviceType devType, bool extended)
		{
			if (!program->networkingEnabled() || !program->network->hasRemoteConnection())
			{
				deviceInfo << "Disconnecting device (Type: " << isprint(devType) << ")..." << endl;

				bool sendDefaultMessage = true;

				switch (devType)
				{
					case DEVICE_TYPE_KEYBOARD:
						if (!disconnectKeyboard())
						{
							return false;
						}

						break;
					case DEVICE_TYPE_GAMEPAD:
						{
							auto identifier = (gamepadID)socket.read<serializedGamepadID>();

							if (program->allowDeviceDetection() && !remoteGamepadConnected(identifier))
							{
								reservedGamepads.insert(identifier);

								return true; // false;
							}

							if (!disconnectGamepad(program, identifier))
							{
								return false;
							}

							sendDefaultMessage = false;

							sendGamepadDisconnectMessage(*program->network, socket, identifier, DESTINATION_ALL);
						}

						break;
					default:
						// This device-type is not supported.
						return false;
				}

				if (sendDefaultMessage)
				{
					sendDisconnectMessage(*program->network, socket, devType, DESTINATION_ALL);
				}
			}
			else
			{
				deviceNetInfo << "Disconnection request rejected; other players may desire to use it." << endl;
			}

			// Return the default response.
			return true;
		}

		headerInfo connectedDevices::beginDeviceMessage(networkEngine& engine, QSocket& socket, deviceType devType)
		{
			headerInfo info = engine.beginMessage(socket, iosync_application::MESSAGE_TYPE_DEVICE);

			socket.write<deviceType>(devType);

			return info;
		}

		// Serialization related:
		void connectedDevices::serializeTo(networkEngine& engine, QSocket& socket)
		{
			if (keyboardConnected())
				serializeKeyboard(engine, socket);

			// Serialize every connected 'gamepad'.
			serializeGamepads(engine, socket);

			return;
		}

		void connectedDevices::serializeKeyboard(networkEngine& engine, QSocket& socket)
		{
			auto headerInformation = beginDeviceMessage(engine, socket, DEVICE_TYPE_KEYBOARD, DEVICE_NETWORK_MESSAGE_ENTRIES);

			serializeIODevice(socket, keyboard);

			engine.finishMessage(socket, headerInformation);

			return;
		}

		void connectedDevices::serializeGamepad(networkEngine& engine, QSocket& socket, gamepadID identifier, gamepadID remoteIdentifier)
		{
			auto headerInformation = beginGamepadDeviceMessage(engine, socket, remoteIdentifier, DEVICE_NETWORK_MESSAGE_ENTRIES);

			serializeIODevice(socket, gamepads[identifier]);

			engine.finishMessage(socket, headerInformation);

			return;
		}

		void connectedDevices::serializeConnectMessage(QSocket& socket, deviceType device)
		{
			// Mark this message as non-extension based.
			socket.writeBool(false);

			return;
		}

		void connectedDevices::serializeDisconnectMessage(QSocket& socket, deviceType device)
		{
			// Mark this message as non-extension based.
			socket.writeBool(false);

			return;
		}

		void connectedDevices::serializeGamepadConnectMessage(QSocket& socket, gamepadID identifier)
		{
			// Mark this message as extension based.
			socket.writeBool(true);

			return;
		}

		void connectedDevices::serializeGamepadDisconnectMessage(QSocket& socket, gamepadID identifier)
		{
			// Mark this message as extension based.
			socket.writeBool(true);

			return;
		}

		void connectedDevices::serializeIODevice(QSocket& socket, IODevice* device)
		{
			// Write the keyboard's action-queue to the output.
			device->writeTo(socket);

			return;
		}

		// Message generation related:
		outbound_packet connectedDevices::generateConnectMessage(networkEngine& engine, QSocket& socket, deviceType device, const address realAddress, address forwardAddress)
		{
			auto headerInformation = beginDeviceMessage(engine, socket, device, DEVICE_NETWORK_MESSAGE_CONNECT);

			serializeConnectMessage(socket, device);

			return engine.finishReliableMessage(socket, realAddress, headerInformation, forwardAddress);
		}

		outbound_packet connectedDevices::generateDisconnectMessage(networkEngine& engine, QSocket& socket, deviceType device, const address realAddress, address forwardAddress)
		{
			auto headerInformation = beginDeviceMessage(engine, socket, device, DEVICE_NETWORK_MESSAGE_DISCONNECT);

			serializeDisconnectMessage(socket, device);

			return engine.finishReliableMessage(socket, realAddress, headerInformation, forwardAddress);
		}

		outbound_packet connectedDevices::generateGamepadConnectMessage(networkEngine& engine, QSocket& socket, gamepadID identifier, const address realAddress, address forwardAddress)
		{
			auto headerInformation = beginGamepadDeviceMessage(engine, socket, identifier, DEVICE_NETWORK_MESSAGE_CONNECT);

			serializeGamepadConnectMessage(socket, identifier);

			return engine.finishReliableMessage(socket, realAddress, headerInformation, forwardAddress);
		}

		outbound_packet connectedDevices::generateGamepadDisconnectMessage(networkEngine& engine, QSocket& socket, gamepadID identifier, const address realAddress, address forwardAddress)
		{
			auto headerInformation = beginGamepadDeviceMessage(engine, socket, identifier, DEVICE_NETWORK_MESSAGE_DISCONNECT);

			serializeGamepadDisconnectMessage(socket, identifier);

			return engine.finishReliableMessage(socket, realAddress, headerInformation, forwardAddress);
		}

		// Parsing/deserialization related:
		deviceType connectedDevices::parseDeviceMessage(iosync_application* program, QSocket& socket, const messageHeader& header, const messageFooter& footer)
		{
			// Local variable(s):

			// Read the device-type from the input.
			auto devType = socket.read<deviceType>();

			// Read the sub-message type from the input.
			auto subMessageType = socket.read<deviceMessageType>();

			switch (subMessageType)
			{
				case DEVICE_NETWORK_MESSAGE_CONNECT:
					deviceNetInfo << "Discovered device-connection request, parsing..." << endl;

					if (parseConnectMessage(program, socket, devType, header, footer))
					{
						deviceNetInfo << "Parsing operations complete." << endl;

						deviceNetInfo << "Device connected successfully";

						switch (program->mode)
						{
							case iosync_application::MODE_CLIENT:
								deviceNetInfoStream << ", as the remote host intended";
							default:
								deviceNetInfoStream << ".";

								break;
						}

						deviceNetInfoStream << endl;
					}

					break;
				case DEVICE_NETWORK_MESSAGE_DISCONNECT:
					deviceNetInfo << "Discovered device-disconnection request, parsing..." << endl;

					if (parseDisconnectMessage(program, socket, devType, header, footer))
					{
						deviceNetInfo << "Parsing operations complete." << endl;

						deviceNetInfo << "Device disconnected successfully";

						switch (program->mode)
						{
							case iosync_application::MODE_CLIENT:
								deviceNetInfoStream << ", as the remote host intended";
							default:
								deviceNetInfoStream << ".";

								break;
						}

						deviceNetInfoStream << endl;
					}

					break;
				default:
					switch (devType)
					{
						case DEVICE_TYPE_KEYBOARD:
							if (keyboardConnected())
							{
								parseKeyboardMessage(program, socket, subMessageType, header, footer);

								break;
							}

							// The keyboard wasn't connected, tell the user.
							return DEVICE_TYPE_NOT_FOUND;
						case DEVICE_TYPE_GAMEPAD:
							if (parseGamepadMessage(program, socket, subMessageType, header, footer) == DEVICE_NETWORK_MESSAGE_INVALID)
							{
								return DEVICE_TYPE_NOT_FOUND;
							}
						default:
							// Tell the user we couldn't find a device.
							return DEVICE_TYPE_NOT_FOUND;
					}
			}

			// Tell the user which device was affected.
			return devType;
		}

		bool connectedDevices::parseConnectMessage(iosync_application* program, QSocket& socket, deviceType device, const messageHeader& header, const messageFooter& footer)
		{
			// Local variable(s):

			// Check if this message is relying upon device-specific extensions.
			auto extended = socket.readBool();

			// Connect the device specified, then return the connection-response.
			return connectDeviceThroughNetwork(program, socket, device, extended);
		}

		bool connectedDevices::parseDisconnectMessage(iosync_application* program, QSocket& socket, deviceType device, const messageHeader& header, const messageFooter& footer)
		{
			// Local variable(s):

			// Check if this message is relying upon device-specific extensions.
			auto extended = socket.readBool();

			// Disconnect the device specified, then return the disconnection-response.
			return disconnectDeviceThroughNetwork(program, socket, device, extended);
		}

		deviceMessageType connectedDevices::parseKeyboardMessage(iosync_application* program, QSocket& socket, deviceMessageType subMessageType, const messageHeader& header, const messageFooter& footer)
		{
			switch (subMessageType)
			{
				case DEVICE_NETWORK_MESSAGE_ENTRIES:
					parseIODevice(program, socket, keyboard, header, footer);

					break;
				default:
					// This sub-message type is unsupported, skip it.
					clog << "Invalid keyboard network-message detected." << endl;

					return DEVICE_NETWORK_MESSAGE_INVALID;
			}

			// Tell the user what kind of message this was.
			return subMessageType;
		}

		deviceMessageType connectedDevices::parseGamepadMessage(iosync_application* program, QSocket& socket, deviceMessageType subMessageType, const messageHeader& header, const messageFooter& footer)
		{
			// Local variable(s):

			// Read the identifier of the incoming 'gamepad'.
			gamepadID remoteIdentifier = (gamepadID)socket.read<serializedGamepadID>();
			gp* pad = getGamepad(remoteIdentifier);

			if (pad == nullptr)
				return DEVICE_NETWORK_MESSAGE_INVALID;

			switch (subMessageType)
			{
				case DEVICE_NETWORK_MESSAGE_ENTRIES:
					parseIODevice(program, socket, pad, header, footer);

					break;
				default:
					// This sub-message type is unsupported, skip it.
					clog << "Invalid gamepad network-message detected." << endl;

					return DEVICE_NETWORK_MESSAGE_INVALID;
			}

			return subMessageType;
		}

		void connectedDevices::parseIODevice(iosync_application* program, QSocket& socket, IODevice* device, const messageHeader& header, const messageFooter& footer)
		{
			device->readFrom(socket);

			return;
		}
	}

	// Classes:

	// iosync_application:

	// Structures:
	#ifdef IOSYNC_LIVE_COMMANDS
		// applicationCommand:

		// Constructor(s):
		iosync_application::applicationCommand::applicationCommand(messageType msgData, programList programs)
			: data(msgData), connectedPrograms(programs) { /* Nothing so far. */ }

		iosync_application::applicationCommand::applicationCommand(inputStream inStream, programList programs) : connectedPrograms(programs)
		{
			// Read from the input-stream specified.
			readFrom(inStream);
		}

		// Destructor(s):
		// Nothing so far.

		// Methods:
		void iosync_application::applicationCommand::readFrom(inputStream in)
		{
			// Read from the input-stream.
			in >> data;
			//in.sync();

			return;
		}
	#endif

	// Global variable(s):
	#ifdef IOSYNC_LIVE_COMMANDS
		iosync_application::programList iosync_application::commandTargets;
		queue<iosync_application::applicationCommand> iosync_application::commandQueue;
		
		mutex iosync_application::commandMutex;
		thread iosync_application::commandThread;

		bool iosync_application::commandThreadRunning;
	#endif

	// Functions:
	#ifdef IOSYNC_LIVE_COMMANDS
		void iosync_application::beginLocalCommandAccept()
		{
			commandMutex.lock();

			// Make sure to set the "running" flag.
			commandThreadRunning = true;

			commandMutex.unlock();

			acceptCommands();

			return;
		}

		void iosync_application::acceptCommands()
		{
			while (commandThreadRunning)
			{
				// Local variable(s):

				// Read the user's input from the standard input-stream:
				applicationCommand command
				(
					wcin, programList(commandTargets.begin(), commandTargets.end())
				);

				// Lock the command-mutex.
				commandMutex.lock();

				// Remove finished commands.
				pruneCommand(commandQueue);

				// Add this command to the queue.
				commandQueue.push(command);

				// Unlock the command-mutex.
				commandMutex.unlock();
			}

			return;
		}
		
		bool iosync_application::openCommandThread(iosync_application* program)
		{
			// Lock the command-thread's mutex.
			lock_guard<mutex> mutexLock(commandMutex);

			commandTargets.push_back(program);

			// Check if the thread is already running:
			if (commandThreadRunning)
				return true;

			// Set the "running" flag to 'true'.
			commandThreadRunning = true;

			// Start the command-thread; the "running"
			// flag must be set to 'true' beforehand.
			commandThread = thread(acceptCommands);

			// Return the default response.
			return true;
		}

		bool iosync_application::closeCommandThread(iosync_application* program)
		{
			// Local variable(s):

			// Lock the command-thread's mutex.
			lock_guard<mutex> mutexLock(commandMutex);

			commandTargets.remove(program);

			checkThreadViability();

			// Return the default response.
			return true;
		}

		void iosync_application::forceCloseCommandThread()
		{
			// Local variable(s):

			// Lock the command-thread's mutex.
			lock_guard<mutex> mutexLock(commandMutex);

			// Check if we're even running:
			if (!commandThreadRunning)
				return;

			// Clear the list of command-targets.
			commandTargets.clear();

			// Check if this thread is still viable.
			// The unsafe version is called because we alredy have the mutex locked.
			checkThreadViability_unsafe();

			return;
		}

		bool iosync_application::checkThreadViability()
		{
			// Local variable(s):

			// Lock the command-thread's mutex.
			lock_guard<mutex> mutexLock(commandMutex);

			// Execute the main routine.
			return checkThreadViability_unsafe();
		}

		bool iosync_application::checkThreadViability_unsafe()
		{
			// Check if the command-thread is running:
			if (!commandThreadRunning)
				return true;

			// Check if something can still use this thread:
			if (!commandTargets.empty())
				return false;

			commandThreadRunning = false;

			// Detach the command-thread.
			commandThread.detach();

			return true; // (commandThreadRunning == false)
		}
	#endif

	#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
		static stack<iosync_application*> asyncProgramStack;

		static inline void pushAsyncApplication(iosync_application* program)
		{
			asyncProgramStack.push(program);

			return;
		}

		static inline iosync_application* popAsyncApplication()
		{
			iosync_application* application = asyncProgramStack.top();

			asyncProgramStack.pop();

			return application;
		}

		void iosync_application::executeAsyncApplication()
		{
			if (asyncProgramStack.empty())
				return;

			auto program = popAsyncApplication();

			program->execute();

			return;
		}
	#endif

	// Constructor(s):
	iosync_application::iosync_application(rate updateRate, OSINFO OSInfo) : application(updateRate, OSInfo), network(nullptr)
	{
		// Nothing so far.
	}

	// Destructor(s):
	iosync_application::~iosync_application()
	{
		// Delete the "network engine" instance.
		delete network;

		// Close the application.
		if (!closed())
			onClose();
	}

	// Methods:
	int iosync_application::execute(const addressPort port)
	{
		cout << "Attempting to open server network (" << port << ")..." << endl << endl;

		// Allocate a new "networking engine".
		auto engine = new serverNetworkEngine(*this);

		// Attempt to host using the port specified:
		if (!engine->open(port))
		{
			cout << "Unable to open server network, cleaning up allocations..." << endl;

			// Delete the "engine" we allocated.
			delete engine;

			cout << "Network-object deallocated." << endl;

			return ERROR_HOSTING;
		}

		// Set the main "engine" instance to the one we allocated.
		this->network = engine;

		cout << "Server network started." << endl << endl;

		// Execute the creation call-back.
		onCreate(MODE_SERVER);

		// Call the super-class's implementation.
		auto responseCode = application::execute();

		// Execute the cleanup/close call-back.
		onClose();

		// Return the calculated response-code.
		return responseCode;
	}

	// This acts as the default 'execute' implementation.
	int iosync_application::execute()
	{
		auto argCount = args.size();

		#ifdef IOSYNC_TESTMODE
			if (argCount == 0)
			{
				wstring username = DEFAULT_PLAYER_NAME;
				string hostname = "127.0.0.1";
				nativePort port = DEFAULT_PORT;

				//cout << endl;

				cout << "Application mode (" << MODE_CLIENT << " = Client, " << MODE_SERVER << " = Server): "; cin >> mode; //cout << endl;

				switch (mode)
				{
					case MODE_CLIENT:
						#ifndef IOSYNC_FAST_TESTMODE
							{
								string::size_type separatorPosition;

								cout << "Please supply a hostname: "; cin >> hostname; // cout << endl;

								separatorPosition = hostname.find(networking::ADDRESS_SEPARATOR);

								if (separatorPosition != string::npos && separatorPosition < hostname.length())
								{
									port = portFromString(hostname.substr(separatorPosition+1));

									hostname = hostname.substr(0, separatorPosition);
								}
								else
								{
									string portStr;

									cout << "Please supply a remote-port: ";
									port = portFromInput(cin); // cout << endl;
								}

								cout << "Please enter a username (No spaces): "; wcin >> username; // cout << endl;
							}
						#endif

						//clearConsole();

						return execute(username, hostname, port);
					case MODE_SERVER:
						{
							#ifndef IOSYNC_FAST_TESTMODE
								cout << "Please enter a port to host with: ";
								port = portFromInput(cin); // cout << endl;
							#endif

							DWORD PID; cout << "Please specify a PID to apply XInput injection (0 = None): "; cin >> PID;

							devices::gamepad::__winnt__injectLibrary(PID);
						}

						//clearConsole();

						return execute(port);
				}
			}
		#endif

		// Check if we have arguments to use:
		if (argCount > 0)
		{
			return execute((addressPort)stoi(args[0]));
		}
		else if (argCount > 1)
		{
			return execute((argCount > 2) ? args[2] : DEFAULT_PLAYER_NAME, wideStringToDefault(args[0]), (addressPort)stoi(args[1]));
		}

		// Call the main implementation.
		return execute(DEFAULT_PORT);
	}

	int iosync_application::execute(wstring username, string remoteAddress, addressPort remotePort, addressPort localPort)
	{
		cout << "Attempting to opening client network (" << remoteAddress << networking::ADDRESS_SEPARATOR << remotePort << ")..." << endl << endl;

		// Allocate a new "networking engine".
		auto engine = new clientNetworkEngine(*this, username);

		// Attempt to connect to the address specified.
		if (!engine->open(remoteAddress, remotePort, localPort))
		{
			cout << "Unable to open client network, cleaning up allocations..." << endl;

			// Delete the "engine" we allocated.
			delete engine;

			cout << "Network-object deallocated." << endl;

			return ERROR_CONNECTING;
		}

		//cout << "Preliminary processes complete, continuing..." << endl << endl;

		// Set the main "engine" instance to the one we allocated.
		this->network = engine;

		// Execute the creation call-back.
		onCreate(MODE_CLIENT);

		// Call the super-class's implementation.
		auto responseCode = application::execute();

		// Execute the cleanup/close call-back.
		onClose();

		// Return the calculated response-code.
		return responseCode;
	}

	#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
		void iosync_application::executeAsync()
		{
			#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
				asyncExecutionMutex.lock();
			#endif

			// Push this application to the top of the async-application stack.
			pushAsyncApplication(this);

			// Start the application asynchronously.
			thread t = thread(executeAsyncApplication);

			// Detach the application's thread.
			t.detach();

			return;
		}
	#endif

	// The main creation routine.
	void iosync_application::onCreate(applicationMode mode)
	{
		#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
			asyncExecutionMutex.unlock();
		#endif

		// Set the internal application-mode.
		this->mode = mode;

		// Attempt to open the internal window.
		this->window = sharedWindow::open(OSInfo);

		if (window == WINDOW_NONE)
			throw noWindowException(this);

		#ifdef IOSYNC_LIVE_COMMANDS
			openCommandThread(this);
		#endif

		return;
	}

	void iosync_application::onClose()
	{
		disconnectDevices();

		closeNetwork();

		//sharedWindow::close();

		#ifdef IOSYNC_LIVE_COMMANDS
			closeCommandThread(this);
		#endif

		return;
	}

	void iosync_application::disconnectDevices()
	{
		devices.disconnect();

		return;
	}

	void iosync_application::closeNetwork()
	{
		if (network == nullptr)
			return;

		// Manually close the network-engine.
		network->close();

		delete network;
		network = nullptr;

		return;
	}

	// Update routines:
	void iosync_application::update(rate frameNumber)
	{
		#ifdef IOSYNC_LIVE_COMMANDS
			parseCommands();
		#endif

		updateDevices();

		if (network != nullptr)
			updateNetwork();

		return;
	}

	void iosync_application::updateNetwork()
	{
		if (devices.hasDeviceConnected())
		{
			switch (mode)
			{
				case MODE_SERVER:
					// Nothing so far.

					break;
				case MODE_CLIENT:
					devices.sendTo(*network);

					break;
			}
		}

		network->update();

		return;
	}

	void iosync_application::updateDevices()
	{
		// Check for platform-specific device-messages:
		if (mode == MODE_CLIENT && devices.hasDeviceConnected())
			checkDeviceMessages();

		// Update all of the devices:
		devices.update(this);

		return;
	}

	void iosync_application::checkDeviceMessages()
	{
		#ifdef PLATFORM_WINDOWS
			MSG message;
			LPBYTE lpb;
			UINT dwSize;

			// This still needs to be refactored; all system-messages
			// are handled by this routine, and that can be problematic:
			while (PeekMessage(&message, sharedWindow::windowInstance, 0, 0, PM_REMOVE)) // NULL
			{
				TranslateMessage(&message);

				//networkLog << "System Message: " << message.message << endl;

				switch (message.message)
				{
					case WM_INPUT:
						// This routine still needs to be optimized:
						GetRawInputData((HRAWINPUT)message.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

						lpb = new BYTE[dwSize];

						// Check if we could allocate the structure:
						if (lpb == NULL)
							continue;

						if (GetRawInputData((HRAWINPUT)message.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
						{
							delete[] lpb;

							continue;
						}

						// Read from the raw-input device.
						__winnt__readFromDevice((RAWINPUT*)lpb);

						// Delete the buffer we allocated.
						delete[] lpb;

						break;
					default:
						// Nothing so far.

						break;
				}

				DefWindowProc(sharedWindow::windowInstance, message.message, message.wParam, message.lParam);
				DispatchMessage(&message);
			}
		#endif

		return;
	}

	#ifdef IOSYNC_LIVE_COMMANDS
		bool iosync_application::parseCommand(applicationCommand command)
		{
			wcout << L"This is a test: " << command.data << endl;

			// Return the default response.
			return true;
		}
		
		void iosync_application::parseCommands()
		{
			// Local variable(s):
			lock_guard<mutex> mutexLock(commandMutex);

			while (!commandQueue.empty())
			{
				auto& connectedPrograms = commandQueue.front().connectedPrograms;

				// Search for this object
				if (find(connectedPrograms.begin(), connectedPrograms.end(), this) != connectedPrograms.end())
				{
					auto& command = commandQueue.front();

					parseCommand(command);

					command.connectedPrograms.remove(this);
				}
			}

			return;
		}
	#endif

	// Networking related:

	// Parsing/deserialization related:
	bool iosync_application::parseNetworkMessage(QSocket& socket, const messageHeader& header, const messageFooter& footer)
	{
		switch (header.type)
		{
			case MESSAGE_TYPE_DEVICE:
				// Route device packets to the 'devices' manager.
				// In the event a device wasn't found, tell our caller:
				if (devices.parseDeviceMessage(this, socket, header, footer) == deviceManagement::DEVICE_TYPE_NOT_FOUND)
				{
					// We couldn't find the device.
					return false;
				}

				break;
			default:
				return false;
		}

		// Return the default response.
		return true;
	}

	// Call-backs:
	void iosync_application::onNetworkConnected(networkEngine& engine)
	{
		/*
			// Handled through safe call-back messages now:

			// Connect all virtual-devices.
			devices.connect(this);
		*/

		// Tell the host to connect the devices we have.
		// If they accept, we will receive messages to connect our devices.
		devices.sendConnectionRequests(engine, DESTINATION_HOST);

		return;
	}

	void iosync_application::onNetworkClientConnected(networkEngine& engine, player& p)
	{
		wnetworkLog << "Player connected: " << p.name << endl;

		networkLog << "Player address: "; p.outputAddressInfo(networkLogStream, true);

		return;
	}

	void iosync_application::onNetworkClientTimedOut(networkEngine& engine, player& p)
	{
		wnetworkLog << "Player timed-out: " << p.name << endl;

		networkLog << "Timed-out player's address: "; p.outputAddressInfo(networkLogStream, true);

		return;
	}

	void iosync_application::onNetworkClosed(networkEngine& engine)
	{
		if (&engine == network)
		{
			clog << "Received request to destroy 'network' object..." << endl;

			delete network;

			network = nullptr;

			clog << "'network' object deleted." << endl;

			clog << "Setting execution-state to 'false'." << endl;

			isRunning = false;
		}
		else
		{
			// This isn't the best fallback, but it "works":
			clog << "Found unknown 'networkEngine' object, deleting anyway..." << endl;

			delete &engine;

			clog << "Unknown 'networkEngine' deleted." << endl;
		}

		return;
	}

	nativeWindow iosync_application::getWindow() const
	{
		return sharedWindow::windowInstance;
	}

	#ifdef PLATFORM_WINDOWS
		void iosync_application::__winnt__readFromDevice(RAWINPUT* rawDevice)
		{
			if (devices.keyboard->canDetect())
				devices.keyboard->__winnt__rawRead(rawDevice);

			return;
		}
	#endif
}