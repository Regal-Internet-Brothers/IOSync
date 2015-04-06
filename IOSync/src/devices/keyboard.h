#pragma once

#include "../platform.h"

#ifdef PLATFORM_WINDOWS
	#define KEYBOARD_WINDOWS
#endif

#ifndef KEYBOARD_WINDOWS
	#error Keyboard support is currently Windows-only.
#else
	#define KEYBOARD_GLOBAL_INPUT
	//#define KEYBOARD_ALLOW_UNLINK
#endif

// Includes:
#include "devices.h"

// Standard library:
#include <deque>
#include <chrono>

// Threading / safety:
#include <thread>
#include <mutex>
#include <atomic>

// Namespace(s):
namespace iosync
{
	namespace devices
	{
		// Namespace(s):
		using namespace std;

		// Forward declarations:
		struct keyboardAction;

		// Typedefs:
		typedef DWORD keyboardKey;
		typedef size_t keyboardReferenceCount;
		typedef deque<keyboardAction> keyboardActionQueue;

		// Enumerator(s):
		enum keyboardActionType : unsigned char
		{
			ACTION_TYPE_NONE = 0,

			// This is used for key presses.
			ACTION_TYPE_HIT = 1,

			// This is used when a key is being held down.
			ACTION_TYPE_DOWN = 2,

			// This is used when a key is no longer being held down.
			ACTION_TYPE_RELEASE = 3,
		};

		enum keyboardDeviceFlag : deviceFlags
		{
			FLAG_TESTMODE = FLAG_CUSTOM_LOCATION,
		};

		// Structures:
		struct keyboardAction
		{
			// Constructor(s):
			keyboardAction(keyboardKey keyValue=keyboardKey(), keyboardActionType actionType=ACTION_TYPE_NONE);

			// Methods:
			void readFrom(QSocket& socket);
			void writeTo(QSocket& socket);

			// Fields:
			keyboardKey key;
			keyboardActionType type;
		};

		// Classes:

		// On Windows, this class currently uses hooks.
		class keyboard : public IODevice
		{
			public:
				// Global variable(s):
				#ifdef KEYBOARD_WINDOWS
					// Reserved keyboard hook.
					//static HHOOK deviceHook;
				#endif

				#ifdef KEYBOARD_GLOBAL_INPUT
					static bool registered;
				#endif

				// Functions:
				
				// This overload is a platform-specific extension to the standard implementation:
				#ifdef PLATFORM_WINDOWS
					static nativeResponseCode sendNativeKeyboardInput(nativeDeviceInterface* items, nativeFlags flags, size_t itemCount=1);

					static inline nativeResponseCode sendNativeKeyboardInput(nativeDeviceInterface& item)
					{
						return sendNativeSystemInput(&item, 1);
					}

					static bool __winnt__registerHardware(HWND window);

					/*
						Keyboard hook related:

						static LRESULT CALLBACK __winnt_hook_keyboard(INT code, WPARAM wParam, LPARAM lParam);
						static HHOOK __winnt_set_keyboard_hook(HINSTANCE hook_proc=HINSTANCE());
						static bool __winnt_unhook_keyboard();
					*/
				#endif

				// This command simulates the specified action.
				static void simulateAction(keyboardAction action);

				static void linkKeyboard();
				static void unlinkKeyboard();

				static bool keyboardLinked();

				static inline bool keyboardUnlinked()
				{
					return !keyboardLinked();
				}

				static inline bool autoLinkKeyboard()
				{
					if (keyboardUnlinked())
						linkKeyboard();

					if (keyboardLinked())
					{
						#ifdef KEYBOARD_ALLOW_UNLINK
							globalDeviceCounter++;
						#endif

						return true;
					}

					return false;
				}

				static inline bool autoUnlinkKeyboard()
				{
					#ifdef KEYBOARD_ALLOW_UNLINK
						if (globalDeviceCounter > 0 && keyboardLinked())
						{
							globalDeviceCounter--;
						}

						if (globalDeviceCounter == 0)
						{
							unlinkKeyboard();

							return keyboardUnlinked();
						}
					#endif

					return false;
				}

				// Call this when unsure of the states of active keyboard devices,
				// but you want to make sure everything's cleaned up.
				static inline bool cleanUp()
				{
					return autoUnlinkKeyboard();
				}

				// Constructor(s):
				keyboard(bool canDetect=true, bool canSimulate=true, deviceFlags flagsToAdd=deviceFlags());

				// Destructor(s):
				// Nothing so far.

				// Methods:
				virtual bool connect() override;
				virtual bool disconnect() override;

				virtual void detect(application* program) override;
				virtual void simulate(application* program) override;

				// These commands may be used to serialize and deserialize this device's "action-queue":
				virtual void readFrom(QSocket& socket) override;
				virtual void writeTo(QSocket& socket) override;

				// This command simulates the action at the top of the "action-queue".
				// The return value of this command indicates if the internal
				// action-queue is empty after the action was simulated.
				virtual bool simulateAction();

				inline bool hasAction() const
				{
					return (!actionQueue.empty());
				}

				#ifdef PLATFORM_WINDOWS
					virtual void __winnt__rawRead(RAWINPUT* rawDevice) override;

					void __winnt__readKeyboard(RAWINPUT* rawDevice);
				#endif

				inline bool testMode() const
				{
					return ((flags & FLAG_TESTMODE) > 0);
				}

				// Fields:
				keyboardActionQueue actionQueue;
		};
	}
}