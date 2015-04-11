// Includes:
#include "keyboard.h"

// Standard networking functionality.
#include "../networking/networking.h"

// Required for shared-window access.
#include "../iosync.h"

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
		#ifdef KETBOARD_WINDOWS
			//HHOOK keyboard::deviceHook = HHOOK();
		#endif

		#ifdef KEYBOARD_GLOBAL_INPUT
			bool keyboard::registered = false;
		#endif

		// Functions:
		#ifdef PLATFORM_WINDOWS
			nativeResponseCode keyboard::sendNativeKeyboardInput(nativeDeviceInterface* items, nativeFlags flags, size_t itemCount)
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
		#endif

		void keyboard::simulateAction(keyboardAction action)
		{
			// Windows-specific implementation:
			#ifdef PLATFORM_WINDOWS
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
						sendNativeKeyboardInput(&deviceAction, KEYEVENTF_SCANCODE|extraFlags); // KEYEVENTF_EXTENDEDKEY
					case ACTION_TYPE_RELEASE:
						// Send an input event to release the key.
						sendNativeKeyboardInput(&deviceAction, KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP|extraFlags); // KEYEVENTF_EXTENDEDKEY

						break;
					case ACTION_TYPE_DOWN:
						// Send a single input-event telling the system to hold the key.
						sendNativeKeyboardInput(&deviceAction, KEYEVENTF_SCANCODE|extraFlags); // KEYEVENTF_EXTENDEDKEY

						break;
				}
			#endif

			return;
		}

		void keyboard::linkKeyboard()
		{
			#ifdef PLATFORM_WINDOWS
				//__winnt_set_keyboard_hook();
				__winnt__registerHardware(sharedWindow::windowInstance);
			#endif

			return;
		}

		void keyboard::unlinkKeyboard()
		{
			#ifdef PLATFORM_WINDOWS
				//__winnt_unhook_keyboard();
			#endif

			return;
		}

		bool keyboard::keyboardLinked()
		{
			#ifdef PLATFORM_WINDOWS
				//return (deviceHook != HHOOK());
				return registered;
			#else
				return true;
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