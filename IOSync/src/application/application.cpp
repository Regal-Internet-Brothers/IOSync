// Includes:
//#include "../iosync.h"
#include "application.h"

//#include "../networking/networking.h"
//#include "../networking/networkEngine.h"
#include "../networking/messages.h"
#include "../networking/player.h"

// Standard library:
#include <sstream>
#include <chrono>
#include <thread>

#include <iostream>

// Windows-specific:
#ifdef PLATFORM_WINDOWS
	#include <shellapi.h>
#endif

// Namespace(s):
using namespace std;

namespace iosync
{
	namespace exceptions
	{
		namespace applicationExceptions
		{
			// Classes:

			// applicationException:

			// Constructor(s):
			applicationException::applicationException(application* targetedProgram, const string& exception_name)
				: iosync_exception(exception_name), program(targetedProgram) { /* Nothing so far. */ }

			// Methods:
			// Nothing so far.
		}
	}

	// Structures:

	// OSINFO:
	#ifdef PLATFORM_WINDOWS_EXTENSIONS
		OSINFO::OSINFO(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
			: hInstance(instance), hPrevInstance(prevInstance), lpCmdLine(cmdLine), nCmdShow(cmdShow) { /* Nothing so far. */ }
	#else
		OSINFO::OSINFO(int argCount, char** argVector)
			: argc(argCount), argv(argVector) { /* Nothing so far. */ }
	#endif

	// Classes:

	// application:

	// Global variable(s):
	std::wstring application::path;

	// Constructor(s):
	application::application(rate updateRate, OSINFO info) : isRunning(false), OSInfo(info)
	{
		this->updateRate = updateRate;

		#ifdef PLATFORM_WINDOWS_EXTENSIONS
			LPWSTR* argv;
			int argc;

			argv = CommandLineToArgvW(GetCommandLineW(), &argc);

			if (argv != nullptr)
			{
				args = arguments(argv+1, argv+argc);
			}
		#else
			if (OSInfo.argc > 0)
			{
				args.clear();

				// Create a standard argument-container:
				for (unsigned i = 1; i < info.argc; i++)
				{
					args.push_back(defaultStringToWide(info.argv[i]));
				}

				//args = arguments(OSInfo.argv + 1, OSInfo.argv + OSInfo.argc);
			}
		#endif
	}

	// Methods:
	int application::execute()
	{
		// Set the execution-flag to 'true'.
		isRunning = true;

		rate frameNumber = 0;
		auto delayAmount = (chrono::milliseconds)(1000/updateRate);

		// This will act as our main loop:
		while (isRunning)
		{
			// Update this application.
			update(frameNumber);

			frameNumber += 1;

			if (frameNumber > updateRate)
			{
				//this_thread::sleep_for((chrono::seconds)1);

				frameNumber = 0;
			}

			this_thread::sleep_for(delayAmount);
		}

		// Return the default response.
		return 0;
	}

	nativeWindow application::getWindow() const
	{
		return WINDOW_NONE;
	}

	// Networking related:

	// Parsing/deserialization related:
	bool application::parseNetworkMessage(QSocket& socket, const messageHeader& header, const messageFooter& footer)
	{
		// By default, we can not accept network-messages.
		return false;
	}

	// Call-backs:

	// These are blank implementations, do not bother calling up to them.
	// Please implement this method as you see fit.

	void application::onNetworkConnected(networkEngine& engine)
	{
		return;
	}

	void application::onNetworkClientConnected(networkEngine& engine, player& p)
	{
		return;
	}

	void application::onNetworkClientTimedOut(networkEngine& engine, player& p)
	{
		return;
	}

	void application::onNetworkClosed(networkEngine& engine)
	{
		return;
	}
}