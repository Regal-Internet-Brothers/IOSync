// Includes:
#include "iosync.h"

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
				//cout << "Message: " << msg << endl;

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
			// Nothing so far.
		}

		// Destructor(s):
		connectedDevices::~connectedDevices()
		{
			disconnect();
		}

		// Methods:
		void connectedDevices::update(iosync_application* program)
		{
			if (keyboardConnected())
				keyboard->update(program);

			return;
		}

		void connectedDevices::connect(iosync_application* program)
		{
			// Connect all devices:
			connectKeyboard(program);

			return;
		}

		void connectedDevices::disconnect()
		{
			// Disconnect all devices:
			disconnectKeyboard();

			return;
		}

		bool connectedDevices::connectDevice(deviceType devType, iosync_application* program)
		{
			cout << "Connecting device (Type: " << isprint(devType) << ")..." << endl;

			switch (devType)
			{
				case DEVICE_TYPE_KEYBOARD:
					if (!connectKeyboard(program))
					{
						return false;
					}

					break;
				default:
					// This device-type is not supported.
					return false;
			}

			// Return the default response.
			return true;
		}

		bool connectedDevices::disconnectDevice(deviceType devType, iosync_application* program)
		{
			cout << "Disconnecting device (Type: " << isprint(devType) << ")..." << endl;

			switch (devType)
			{
				case DEVICE_TYPE_KEYBOARD:
					if (!disconnectKeyboard())
					{
						return false;
					}

					break;
				default:
					// This device-type is not supported.
					return false;
			}

			// Return the default response.
			return true;
		}

		kbd* connectedDevices::connectKeyboard(iosync_application* program)
		{
			cout << "Attempting to create keyboard-instance..." << endl;

			// Check if we need to create a new keyboard:
			if (keyboard == nullptr)
			{
				cout << "Creating keyboard instance." << endl;

				// Allocate a new keyboard on the heap.
				keyboard = new kbd(program->allowDeviceDetection(), program->allowDeviceSimulation());

				cout << "Keyboard instance created." << endl;
			}

			cout << "Attempting to connect keyboard..." << endl;

			// Attempt to connect our keyboard:
			if (!keyboard->connect())
			{
				cout << "Failed to connect keyboard, destroying device-instance..." << endl;

				// We couldn't connect it, destroy the object.
				destroyKeyboard();

				// Tell the user we couldn't connect it.
				return nullptr;
			}

			cout << "Keyboard connected." << endl;

			// Return the 'keyboard' object to the user.
			return keyboard;
		}

		// Networking related:
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

			return;
		}

		void connectedDevices::serializeKeyboard(networkEngine& engine, QSocket& socket)
		{
			auto headerInformation = beginDeviceMessage(engine, socket, DEVICE_TYPE_KEYBOARD, DEVICE_NETWORK_MESSAGE_ENTRIES);

			serializeKeyboardActions(socket);

			engine.finishMessage(socket, headerInformation);

			return;
		}

		void connectedDevices::serializeConnectMessage(QSocket& socket, deviceType device)
		{
			socket.write<deviceType>(device);

			return;
		}

		void connectedDevices::serializeDisconnectMessage(QSocket& socket, deviceType device)
		{
			socket.write<deviceType>(device);

			return;
		}

		void connectedDevices::serializeKeyboardActions(QSocket& socket)
		{
			// Write the keyboard's action-queue to the output.
			keyboard->writeTo(socket);

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
					cout << "Discovered device-connection request, parsing..." << endl;

					if (parseConnectMessage(program, socket, header, footer))
					{
						cout << "Parsing operations complete." << endl;

						switch (program->mode)
						{
							case iosync_application::MODE_SERVER:
								cout << "Sending device-connection notification to client..." << endl;

								sendConnectMessage(*(program->network), socket, devType, DESTINATION_REPLY);

								cout << "Sending operations complete." << endl;

								break;
							case iosync_application::MODE_CLIENT:
								cout << "Device connected successfully, as the remote host intended." << endl;

								break;
						}
					}

					break;
				case DEVICE_NETWORK_MESSAGE_DISCONNECT:
					cout << "Discovered device-disconnection request, parsing..." << endl;

					if (parseDisconnectMessage(program, socket, header, footer))
					{
						cout << "Parsing operations complete." << endl;

						switch (program->mode)
						{
							case iosync_application::MODE_SERVER:
								cout << "Sending device-disconnection notification to clients..." << endl;

								// Tell everyone this device has been disconnected.
								sendDisconnectMessage(*(program->network), socket, devType, DESTINATION_ALL);

								cout << "Sending operations complete." << endl;

								break;
							case iosync_application::MODE_CLIENT:
								cout << "Device disconnected successfully, as the remote host intended." << endl;

								break;
						}
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
						default:
							// Tell the user we couldn't find a device.
							return DEVICE_TYPE_NOT_FOUND;
					}
			}

			// Tell the user which device was affected.
			return devType;
		}

		bool connectedDevices::parseConnectMessage(iosync_application* program, QSocket& socket, const messageHeader& header, const messageFooter& footer)
		{
			auto device = socket.read<deviceType>();

			return connectDevice(device, program);
		}

		bool connectedDevices::parseDisconnectMessage(iosync_application* program, QSocket& socket, const messageHeader& header, const messageFooter& footer)
		{
			auto device = socket.read<deviceType>();

			return disconnectDevice(device, program);
		}

		deviceMessageType connectedDevices::parseKeyboardMessage(iosync_application* program, QSocket& socket, deviceMessageType subMessageType, const messageHeader& header, const messageFooter& footer)
		{
			switch (subMessageType)
			{
				case DEVICE_NETWORK_MESSAGE_ENTRIES:
					parseKeyboardActions(program, socket, header, footer);

					break;
				default:
					// This sub-message type is unsupported, skip it.
					clog << "Invalid keyboard network-message detected." << endl;

					break;
			}

			// Tell the user what kind of message this was.
			return subMessageType;
		}

		void connectedDevices::parseKeyboardActions(iosync_application* program, QSocket& socket, const messageHeader& header, const messageFooter& footer)
		{
			keyboard->readFrom(socket);

			return;
		}
	}

	// Classes:

	// iosync_application:

	// Global variable(s):
	// Nothing so far.

	// Functions:
	// Nothing so far.

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
		cout << "Opening server network (" << port << ")." << endl;

		// Allocate a new "networking engine".
		auto engine = new serverNetworkEngine();

		// Attempt to host using the port specified:
		if (!engine->open(port))
		{
			// Delete the "engine" we allocated.
			delete engine;

			return ERROR_HOSTING;
		}

		// Set the main "engine" instance to the one we allocated.
		this->network = engine;

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

				cout << "Mode: "; cin >> mode; //cout << endl;

				switch (mode)
				{
					case MODE_CLIENT:
						#ifndef IOSYNC_FAST_TESTMODE
							cout << "Address: "; cin >> hostname; cout << endl;

							cout << "Port: "; cin >> port; cout << endl;
							cout << "Username: "; wcin >> username; cout << endl;
						#endif

						return execute(username, hostname, port);
					case MODE_SERVER:
						#ifndef IOSYNC_FAST_TESTMODE
							cout << "Port: "; cin >> port; cout << endl;
						#endif

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
		cout << "Opening client network (" << remoteAddress << ":" << remotePort << ")." << endl;

		// Allocate a new "networking engine".
		auto engine = new clientNetworkEngine(username);

		// Attempt to connect to the address specified.
		if (!engine->open(remoteAddress, remotePort, localPort))
		{
			// Delete the "engine" we allocated.
			delete engine;

			return ERROR_CONNECTING;
		}

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

	// The main creation routine.
	void iosync_application::onCreate(applicationMode mode)
	{
		// Set the internal application-mode.
		this->mode = mode;

		// Attempt to open the internal window.
		this->window = sharedWindow::open(OSInfo);

		if (window == WINDOW_NONE)
			throw noWindowException(this);

		return;
	}

	void iosync_application::onClose()
	{
		disconnectDevices();

		closeNetwork();

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
		network->close(this);

		delete network;
		network = nullptr;

		return;
	}

	// Update routines:
	void iosync_application::update(rate frameNumber)
	{
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

		network->update(this);

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
			while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);

				//cout << "System Message: " << message.message << endl;

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

				//DispatchMessage(&message);
			}
		#endif

		return;
	}

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
		if (mode == MODE_CLIENT)
		{
			/* Handled through safe call-back messages now:
				// Connect all virtual-devices.
				devices.connect(this);
			*/

			// Tell the host to connect the devices we have.
			// If they accept, we will receive messages to connect our devices.
			devices.sendConnectMessages(engine, DESTINATION_HOST);
		}

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