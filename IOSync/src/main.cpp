/*
	I/O Sync: Cross-system "local" multiplayer input handling, across the internet.
	Using both nodes, and standalone clients, I/O Sync can provide low input-latency.

	TODO:
		X Keyboard integration. (Windows)
		* Support for multi-threaded console input. (Commands, etc)
		* Fix application-arguments for Windows "extended" builds.
		* Options for button-hold timeouts. (In case of disconnection/other)
		* XInput controller/gamepad integration. (Windows)
		? Mouse integration.
*/

// Preprocessor related:
#define _CRT_SECURE_NO_WARNINGS

// Includes:
#include "iosync.h"

// Standard library:
#include <iostream>
#include <exception>

/*
#include <cstdlib>
#include <cstdio>
*/

// Namespace(s):
using namespace iosync;
using namespace std;

using namespace iosync::networking;
using namespace iosync::devices;

// Functions:
#ifdef PLATFORM_WINDOWS_EXTENSIONS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	// Initialize networking functionality.
	QSocket::initSockets();

	#ifdef PLATFORM_WINDOWS_EXTENSIONS
		AllocConsole();

		freopen("CONOUT$", "w", stdout);
		//freopen("CONOUT$", "w", stderr);
		freopen("CONIN$", "r", stdin);

		//SetStdHandle(STD_INPUT_HANDLE, GetStdHandle(STD_INPUT_HANDLE)); // stdin
		//SetStdHandle(STD_OUTPUT_HANDLE, GetStdHandle(STD_OUTPUT_HANDLE)); // stdout

		// Kept in its own scope due to large stack allocation(s):
		{
			WCHAR pathBuffer[MAX_PATH];

			GetModuleFileNameW(NULL, (LPWSTR)pathBuffer, MAX_PATH);

			/*
			auto tempStr = string(pathBuffer);

			string::size_type dividerLocation = tempStr.find_last_of("\\/");

			if (dividerLocation != string::npos)
			{
				application::path = tempStr.substr(0, dividerLocation);
			}
			*/

			application::path = wstring((const wchar_t*)pathBuffer);
		}
	#else
		application::path = defaultStringToWide(argv[0]);
	#endif

	//wcout << "Path: " << "\"" << application::path << "\"" << endl;
	cout << "Starting the application..." << endl;
	
	/*
	keyboard test_keyboard(keyboardDeviceFlag::FLAG_TESTMODE);

	if (!test_keyboard.connect())
		return -1;
	*/

	// Allocate the main application using the stack's memory.
	// This effectively embeds 'program' into 'main'.

	OSINFO OSInfo;

	#ifdef PLATFORM_WINDOWS_EXTENSIONS
		OSInfo = OSINFO(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	#else
		OSInfo = OSINFO(argc - 1, ((argc > 1) ? (argv + 1) : nullptr));
	#endif

	auto program = iosync_application(DEFAULT_UPDATERATE, OSInfo);

	try
	{
		// Execute the application.
		auto responseCode = program.execute();

		if (responseCode != 0)
		{
			cout << "Unable to continue operations, exiting..." << endl;
			cout << "Error code thrown by application: " << responseCode << endl;

			system("PAUSE");

			// Return the response-code.
			return responseCode;
		}
	}
	catch (exception& e)
	{
		cout << "Exception: " << endl << endl << e.what() << endl;
	}

	// Deinitialize networking functionality.
	QSocket::deinitSockets();
	
	return 0;
}