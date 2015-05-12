// Includes:
#include "keyboard.h"

// Standard networking functionality.
#include "../networking/networking.h"

// Required for shared-window access.
#include "../iosync.h"

// Standard library:
#include <map>

// Namespace(s):
namespace iosync
{
	namespace devices
	{
		// Structures:

		// keyboardAction:

		// Constructor(s):
		keyboardAction::keyboardAction(keyboardKey keyValue, keyboardActionType actionType)
			: key(keyValue), type(actionType) { /* Nothing so far. */ }

		// Methods:
		void keyboardAction::readFrom(QSocket& socket)
		{
			key = socket.read<keyboardKey>();
			type = socket.read<keyboardActionType>();
			
			return;
		}

		void keyboardAction::writeTo(QSocket& socket)
		{
			socket.write<keyboardKey>(key);
			socket.write<keyboardActionType>(type);

			return;
		}

		// Classes:

		// keyboard:

		// Global variable(s):
		#ifdef KEYBOARD_GLOBAL_INPUT
			bool keyboard::registered = false;
		#endif

		#ifdef KEYBOARD_ALLOW_UNLINK
			size_t keyboard::globalDeviceCounter = 0;
		#endif

		#if defined(PLATFORM_WINDOWS)
			//HHOOK deviceHook = HHOOK();
		#elif defined(PLATFORM_LINUX)
			int keyboard::keyboardDescriptor = -1;
		#endif

		// Functions:
		#if defined(PLATFORM_WINDOWS)
			nativeResponseCode keyboard::sendNativeKeyboardInput(nativeDeviceInterface* items, size_t itemCount, nativeFlags flags)
			{
				for (size_t index = 0; index < itemCount; index++)
				{
					items[index].ki.dwFlags = (DWORD)flags;
				}

				return sendNativeSystemInput(items, itemCount);
			}

			bool keyboard::__winnt__registerHardware(HWND window)
			{
				if (registered)
					return false;

				RAWINPUTDEVICE kbd;

				kbd.usUsagePage = 0x1;
				kbd.usUsage = 0x06;
				kbd.dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY | RIDEV_DEVNOTIFY; // WM_INPUT_DEVICE_CHANGE
				kbd.hwndTarget = window;

				registered = (RegisterRawInputDevices(&kbd, 1, sizeof(kbd)) == TRUE);

				if (!registered)
				{
					clog << "Unable to register raw-device(s): " << GetLastError() << endl;
				}
				
				return registered;
			}
		#elif defined(PLATFORM_LINUX)
			nativeResponseCode keyboard::sendNativeKeyboardInput(nativeDeviceInterface* items, size_t itemCount, nativeFlags flags)
			{
				for (size_t index = 0; index < itemCount; index++)
				{
					items[index].input.value = (__s32)flags;
				}

				return sendNativeSystemInput(items, itemCount);
			}

			bool keyboard::__linux__registerHardware()
			{
				// Constant variable(s):
				static const int ERROR_FILE = -1;

				if (registered)
					return false;

				// Open/create a new keyboard device.
				keyboardDescriptor = open("/dev/uinput", O_WRONLY|O_NONBLOCK); // Alternative: /dev/input/uinput

				if (keyboardDescriptor != ERROR_FILE)
				{
					ioctl(keyboardDescriptor, UI_SET_EVBIT, EV_KEY);

					// Local variable(s):
					struct uinput_user_dev uidev;

					ZeroVariable(uidev);

					snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "iosync-virtual-keyboard");

					//uidev.id.bustype = 0;
					//uidev.id.vendor = ...;
					//uidev.product = ...;
					//uidev.version = 1;

					write(keyboardDescriptor, &uidev, sizeof(uidev));
					ioctl(keyboardDescriptor, UI_DEV_CREATE);

					registered = true;

					return true;
				}

				// Return the default response.
				return false;
			}

			bool keyboard::__linux__unregisterHardware()
			{
				// Constant variable(s):
				static const int DEFAULT_FILE = -1;

				if (!registered)
					return false;

				// Tell the system we're disconnecting the device.
				ioctl(keyboardDescriptor, UI_DEV_DESTROY);

				// Close our handle to the device-stream.
				close(keyboardDescriptor);
				
				keyboardDescriptor = DEFAULT_FILE;

				// Return the default response.
				return true;
			}

			__u16 keyboard::__linux__virtualToRealKey(__u16 virtualKeyCode) // unsigned short
			{
				// Global variable(s):

				static const map<__u16, __u16> vKeyMap =
				{
					{0x08, KEY_BACKSPACE}, // VK_BACK
					{0x09, KEY_TAB}, // VK_TAB

					{0x0C, KEY_CLEAR}, // VK_CLEAR
					{0x0D, KEY_ENTER}, // VK_RETURN

					{0x10, KEY_LEFTSHIFT}, // VK_SHIFT
					{0x11, KEY_LEFTCTRL}, // VK_CONTROL
					{0x12, KEY_MENU}, // VK_MENU
					{0x13, KEY_PAUSE}, // VK_PAUSE
					{0x14, KEY_CAPSLOCK}, // VK_CAPITAL

					/*
						{0x15, }, // VK_KANA/VK_HANGEUL
						{0x15, }, // VK_HANGUL
						{0x17, }, // VK_JUNJA
						{0x18, }, // VK_FINAL
						{0x19, }, // VK_HANJA
						{0x19, }, // VK_KANJI
					*/

					{0x1B, KEY_ESC}, // VK_ESCAPE

					/* IME keys:
						{0x1C, }, // VK_CONVERT
						{0x1D, }, // VK_NONCONVERT
						{0x1E, }, // VK_ACCEPT
						{0x1F, }, // VK_MODECHANGE
					*/

					{0x20, KEY_SPACE}, // VK_SPACE
					{0x21, KEY_PAGEUP}, // VK_PRIOR
					{0x22, KEY_PAGEDOWN}, // VK_NEXT
					{0x23, KEY_END}, // VK_END
					{0x24, KEY_HOME}, // VK_HOME
					{0x25, KEY_LEFT}, // VK_LEFT
					{0x26, KEY_UP}, // VK_UP
					{0x27, KEY_RIGHT}, // VK_RIGHT
					{0x28, KEY_DOWN}, // VK_DOWN
					{0x29, KEY_SELECT}, // VK_SELECT
					{0x2A, KEY_PRINT}, // VK_PRINT
					// {0x2B, }, // VK_EXECUTE
					{0x2C, KEY_SCREEN}, // VK_SNAPSHOT
					{0x2D, KEY_INSERT}, // VK_INSERT
					{0x2E, KEY_DELETE}, // VK_DELETE
					{0x2F, KEY_HELP}, // VK_HELP

					{0x30, KEY_0}, // '0'
					{0x31, KEY_1}, // '1'
					{0x32, KEY_2}, // '2'
					{0x33, KEY_3}, // '3'
					{0x34, KEY_4}, // '4'
					{0x35, KEY_5}, // '5'
					{0x36, KEY_6}, // '6'
					{0x37, KEY_7}, // '7'
					{0x38, KEY_8}, // '8'
					{0x39, KEY_9}, // '9'
						   
					{0x41, KEY_A}, // 'A'
					{0x42, KEY_B}, // 'B'
					{0x43, KEY_C}, // 'C'
					{0x44, KEY_D}, // 'D'
					{0x45, KEY_E}, // 'E'
					{0x46, KEY_F}, // 'F'
					{0x47, KEY_G}, // 'G'
					{0x48, KEY_H}, // 'H'
					{0x49, KEY_I}, // 'I'
					{0x4A, KEY_J}, // 'J'
					{0x4B, KEY_K}, // 'K'
					{0x4C, KEY_L}, // 'L'
					{0x4D, KEY_M}, // 'M'
					{0x4E, KEY_N}, // 'N'
					{0x4F, KEY_O}, // 'O'
					{0x50, KEY_P}, // 'P'
					{0x51, KEY_Q}, // 'Q'
					{0x52, KEY_R}, // 'R'
					{0x53, KEY_S}, // 'S'
					{0x54, KEY_T}, // 'T'
					{0x55, KEY_U}, // 'U'
					{0x56, KEY_V}, // 'V'
					{0x57, KEY_W}, // 'W'
					{0x58, KEY_X}, // 'X'
					{0x59, KEY_Y}, // 'Y'
					{0x5A, KEY_Z}, // 'Z'

					/*
						{0x5B, }, // VK_LWIN
						{0x5C, }, // VK_RWIN
						{0x5D, }, // VK_APPS
					*/

					{0x5F, KEY_SLEEP}, // VK_SLEEP

					{0x60, KEY_0}, // VK_NUMPAD0
					{0x61, KEY_1}, // VK_NUMPAD1
					{0x62, KEY_2}, // VK_NUMPAD2
					{0x63, KEY_3}, // VK_NUMPAD3
					{0x64, KEY_4}, // VK_NUMPAD4
					{0x65, KEY_5}, // VK_NUMPAD5
					{0x66, KEY_6}, // VK_NUMPAD6
					{0x67, KEY_7}, // VK_NUMPAD7
					{0x68, KEY_8}, // VK_NUMPAD8
					{0x69, KEY_9}, // VK_NUMPAD9

					/*
					{0x6A, }, // VK_MULTIPLY
					{0x6B, }, // VK_ADD
					{0x6C, }, // VK_SEPARATOR
					{0x6D, }, // VK_SUBTRACT
					{0x6E, }, // VK_DECIMAL
					{0x6F, }, // VK_DIVIDE
					*/

					{0x70, KEY_F1}, // VK_F1
					{0x71, KEY_F2}, // VK_F2
					{0x72, KEY_F3}, // VK_F3
					{0x73, KEY_F4}, // VK_F4
					{0x74, KEY_F5}, // VK_F5
					{0x75, KEY_F6}, // VK_F6
					{0x76, KEY_F7}, // VK_F7
					{0x77, KEY_F8}, // VK_F8
					{0x78, KEY_F9}, // VK_F9
					{0x79, KEY_F10}, // VK_F10
					{0x7A, KEY_F11}, // VK_F11
					{0x7B, KEY_F12}, // VK_F12
					{0x7C, KEY_F13}, // VK_F13
					{0x7D, KEY_F14}, // VK_F14
					{0x7E, KEY_F15}, // VK_F15
					{0x7F, KEY_F16}, // VK_F16
					{0x80, KEY_F17}, // VK_F17
					{0x81, KEY_F18}, // VK_F18
					{0x82, KEY_F19}, // VK_F19
					{0x83, KEY_F20}, // VK_F20
					{0x84, KEY_F21}, // VK_F21
					{0x85, KEY_F22}, // VK_F22
					{0x86, KEY_F23}, // VK_F23
					{0x87, KEY_F24}, // VK_F24

					{0x90, KEY_NUMLOCK}, // VK_NUMLOCK
					{0x91, KEY_SCROLLLOCK}, // VK_SCROLL

					{0xA0, KEY_LEFTSHIFT}, // VK_LSHIFT
					{0xA1, KEY_RIGHTSHIFT}, // VK_RSHIFT
					{0xA2, KEY_LEFTCTRL}, // VK_LCONTROL
					{0xA3, KEY_RIGHTCTRL}, // VK_RCONTROL
					{0xA4, KEY_MENU}, // VK_LMENU
					{0xA5, KEY_MENU}, // VK_RMENU

					{0xA6, KEY_BACK}, // VK_BROWSER_BACK
					{0xA7, KEY_FORWARD}, // VK_BROWSER_FORWARD
					{0xA8, KEY_REFRESH}, // VK_BROWSER_REFRESH
					{0xA9, KEY_STOP}, // VK_BROWSER_STOP
					{0xAA, KEY_SEARCH}, // VK_BROWSER_SEARCH
					{0xAB, KEY_BOOKMARKS}, // VK_BROWSER_FAVORITES
					{0xAC, KEY_HOMEPAGE}, // VK_BROWSER_HOME
					
					{0xAD, KEY_MUTE}, // VK_VOLUME_MUTE
					{0xAE, KEY_VOLUMEDOWN}, // VK_VOLUME_DOWN
					{0xAF, KEY_VOLUMEUP}, // VK_VOLUME_UP
					{0xB0, KEY_NEXTSONG}, // VK_MEDIA_NEXT_TRACK
					{0xB1, KEY_PREVIOUSSONG}, // VK_MEDIA_PREV_TRACK
					{0xB2, KEY_STOPCD}, // VK_MEDIA_STOP
					{0xB3, KEY_PLAYPAUSE}, // VK_MEDIA_PLAY_PAUSE
					{0xB4, KEY_EMAIL}, // VK_LAUNCH_MAIL
					{0xB5, KEY_MEDIA}, // VK_LAUNCH_MEDIA_SELECT

					/*
					{0xB6, }, // VK_LAUNCH_APP1
					{0xB7, }, // VK_LAUNCH_APP2
					*/
				};

				auto keyIterator = vKeyMap.find(virtualKeyCode);

				if (keyIterator != vKeyMap.end())
				{
					return keyIterator->second;
				}

				// Return the default response.
				return 0;
			}
		#endif

		void keyboard::simulateAction(keyboardAction action)
		{
			// Windows-specific implementation:
			#if defined(PLATFORM_WINDOWS)
				// Local variable(s):
				INPUT deviceAction;

				// Set up the input-device descriptor:
				deviceAction.type = INPUT_KEYBOARD;
				deviceAction.ki.time = 0;
				deviceAction.ki.wVk = 0; // (DWORD)action.key
				deviceAction.ki.dwExtraInfo = 0;
				deviceAction.ki.wScan = MapVirtualKey(action.key, MAPVK_VK_TO_VSC);

				// Check if this is a key-release event:
				if (action.type == ACTION_TYPE_HIT && (GetAsyncKeyState(action.key) & 0x8000) > 0)
					action.type = ACTION_TYPE_RELEASE;

				#ifdef KEYBOARD_DEBUG
					deviceInfo << "Simuating key: " << action.key << ", " << action.type << " -- wScan: " << deviceAction.ki.wScan << endl;
				#endif

				// Extra flags that apply to the key event(s) we're about to make.
				nativeFlags extraFlags = 0;

				// Check if this is an "extended key":
				if ((action.key >= 33 && action.key <= 46) || (action.key >= 91 && action.key <= 93))
					extraFlags = KEYEVENTF_EXTENDEDKEY; // |=

				switch (action.type)
				{
					case ACTION_TYPE_HIT:
						// Send the initial input-event, then immediately send a release event:
						sendNativeKeyboardInput(&deviceAction, 1, KEYEVENTF_SCANCODE|extraFlags); // KEYEVENTF_EXTENDEDKEY
					case ACTION_TYPE_RELEASE:
						// Send an input event to release the key.
						sendNativeKeyboardInput(&deviceAction, 1, KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP|extraFlags); // KEYEVENTF_EXTENDEDKEY

						break;
					case ACTION_TYPE_DOWN:
						// Send a single input-event telling the system to hold the key.
						sendNativeKeyboardInput(&deviceAction, 1, KEYEVENTF_SCANCODE|extraFlags); // KEYEVENTF_EXTENDEDKEY

						break;
				}
			#elif defined(PLATFORM_LINUX)
				input_event virtualInput;
				
				ZeroVariable(virtualInput);

				virtualInput.type = EV_KEY;
				virtualInput.code = __linux__virtualToRealKey(action.key);

				#ifdef KEYBOARD_DEBUG
					deviceInfo << "Simuating key: " << action.key << ", " << action.type << " -- Scan code: " << virtualInput.code << endl;
				#endif

				nativeDeviceInterface deviceAction
				{
					virtualInput,
					keyboardDescriptor
				};

				switch (action.type)
				{
					case ACTION_TYPE_HIT:
						// Send the initial input-event, then immediately send a release event:
						sendNativeKeyboardInput(&deviceAction, 1, 1); // KEYEVENTF_EXTENDEDKEY
					case ACTION_TYPE_RELEASE:
						// Send an input event to release the key.
						sendNativeKeyboardInput(&deviceAction, 1, 0); // KEYEVENTF_EXTENDEDKEY

						break;
					case ACTION_TYPE_DOWN:
						// Send a single input-event telling the system to hold the key.
						sendNativeKeyboardInput(&deviceAction, 1, 1); // KEYEVENTF_EXTENDEDKEY

						break;
				}
			#endif

			return;
		}

		void keyboard::linkKeyboard()
		{
			#if defined(PLATFORM_WINDOWS) && defined(IOSYNC_SHAREDWINDOW_IMPLEMENTED)
				//__winnt_set_keyboard_hook();
				__winnt__registerHardware(sharedWindow::windowInstance);
			#elif defined(PLATFORM_LINUX)
				__linux__registerHardware();
			#endif

			return;
		}

		void keyboard::unlinkKeyboard()
		{
			#if defined(PLATFORM_WINDOWS)
				//__winnt_unhook_keyboard();
			#elif defined(PLATFORM_LINUX)
				__linux__unregisterHardware();
			#endif

			return;
		}

		bool keyboard::keyboardLinked()
		{
			#ifdef KEYBOARD_GLOBAL_INPUT
				return registered;
			#else
				return true; // false;
			#endif
		}

		// Constructor(s):
		keyboard::keyboard(bool canDetect, bool canSimulate, deviceFlags flagsToAdd) : IODevice(canDetect, canSimulate, flagsToAdd|FLAG_ASYNC_DETECTION)
		{
			// Nothing so far.
		}

		// Destructor(s):
		// Nothing so far.

		// Methods:
		bool keyboard::connect()
		{
			// Make sure we aren't already connected:
			if (connected())
				return false;

			// Automatically link the keyboard:
			if (!autoLinkKeyboard())
				return false;

			// Call the super-class's implementation, then return its response.
			return deviceManager::connect();
		}

		bool keyboard::disconnect()
		{
			// Make sure we aren't already disconnected:
			if (disconnected())
				return false;

			#ifdef KEYBOARD_GLOBAL_INPUT
				// Automatically unlink the keyboard.
				if (!autoUnlinkKeyboard())
					return false;
			#endif

			// Call the super-class's implementation, then return its response.
			return deviceManager::disconnect();
		}

		void keyboard::detect(application* program)
		{
			// Nothing so far.

			return;
		}

		void keyboard::simulate(application* program)
		{
			/*
				keyboardKey key = VK_UP;

				actionQueue.push_back(keyboardAction(key, ACTION_TYPE_DOWN));
				simulateAction();

				this_thread::sleep_for((milliseconds)4000);

				actionQueue.push_back(keyboardAction(key, ACTION_TYPE_HIT));

				simulateAction();

				deviceInfo << "Done." << endl;
			*/

			while (simulateAction())
			{
				// Nothing so far.
			}

			return;
		}

		bool keyboard::simulateAction()
		{
			if (!hasAction())
				return false;

			simulateAction(actionQueue.front());

			// Pop this action off of the queue.
			actionQueue.pop_front();

			return actionQueue.empty();
		}

		void keyboard::readFrom(QSocket& socket)
		{
			// Local variable(s):
			auto items = socket.read<packetSize_t>();

			for (packetSize_t i = 1; i <= items; i++)
			{
				keyboardAction action;

				action.readFrom(socket);

				actionQueue.push_back(action);
			}

			return;
		}

		void keyboard::writeTo(QSocket& socket)
		{
			socket.write<packetSize_t>((packetSize_t)actionQueue.size());

			while (!actionQueue.empty())
			{
				actionQueue.front().writeTo(socket);

				actionQueue.pop_front();
			}

			return;
		}

		#ifdef PLATFORM_WINDOWS
			void keyboard::__winnt__rawRead(RAWINPUT* rawDevice)
			{
				switch (rawDevice->header.dwType)
				{
					//case RIM_TYPEHID:
					//case RIM_TYPEMOUSE:
					case RIM_TYPEKEYBOARD:
						__winnt__readKeyboard(rawDevice);

						break;
				}
				
				return;
			}

			void keyboard::__winnt__readKeyboard(RAWINPUT* rawDevice)
			{
				// Local variable(s):
				keyboardActionType actionType;

				auto key = (keyboardKey)rawDevice->data.keyboard.VKey;
				bool isDown = keyEnabled(key);
				
				if ((rawDevice->data.keyboard.Flags & RI_KEY_BREAK) > 0)
				{
					// If the key is currently up, release/disable it.
					if (isDown)
						disableKey(key);

					actionType = ACTION_TYPE_HIT;
				}
				else // if ((rawDevice->data.keyboard.Flags & RI_KEY_MAKE) > 0)
				{
					// Check if this event is worth logging:
					if (isDown)
						return;

					actionType = ACTION_TYPE_DOWN;

					// Enable/hold this key, so we don't
					// log it again until it's been released.
					enableKey(key);
				}

				actionQueue.push_back(keyboardAction(key, actionType));

				#ifdef KEYBOARD_DEBUG
					deviceInfo << "Detected key: " << rawDevice->data.keyboard.VKey << ", actionType: " << actionType << endl;
				#endif

				return;
			}
		#endif
	}
}