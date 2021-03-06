﻿/*
	I/O Sync: Cross-system input events.

	TODO Format:
		X = Finished.
		/ = Partially implemented.
		* = On the bucket-list.
		? = Unsure and/or partially implemented.
	TODO:
		X Keyboard integration. (Windows)
		X Fix application-arguments for Windows "extended" builds.
		X XInput controller/gamepad integration. (Windows)

		/ Adjust network "metrics".

		* Proper support for user-commands.
		* Options for vibration timeouts.

		/ Options for button-hold timeouts. (In case of disconnection/other)
		/ Fix "const correctness" for some commands.

		? Mouse integration.
*/

// Preprocessor related:

// Includes:
#include "iosync.h"

// Standard library:
#include <iostream>
#include <exception>

/*
#include <cstdlib>
#include <cstdio>
*/

// Platform-specific:
#ifdef PLATFORM_WINDOWS
	#include <fcntl.h>
	#include <io.h>
#endif

// Namespace(s):
using namespace iosync;
using namespace std;

using namespace iosync::networking;
using namespace iosync::devices;

// Functions:
#ifdef IOSYNC_LIVE_COMMAND_INPUT_ON_MAIN_THREAD
iosync_application* runProgram(OSINFO OSInfo, rate updateRate=DEFAULT_UPDATERATE)
#else
int runProgram(OSINFO OSInfo, rate updateRate=DEFAULT_UPDATERATE)
#endif
{
	#ifdef IOSYNC_ALLOW_ASYNC_EXECUTE
		iosync_application* program = new iosync_application(updateRate, OSInfo);
		
		program->executeAsync();

		return program;
	#else
		auto program = iosync_application(updateRate, OSInfo);	
		
		try
		{
			// Execute the application:
			auto responseCode = program.execute();

			if (responseCode != 0)
			{
				cout << "Unable to continue operations; exiting..." << endl;
				cout << "Error code thrown by application: " << responseCode << endl;

				//cout << "Press any key to continue..."; getchar(); cout << endl;
				
				// Return the response-code.
				return responseCode;
			}
		}
		catch (exception& e)
		{
			cout << "Exception: " << endl << endl << e.what() << endl;
		}

		return 0;
	#endif
}

#ifdef PLATFORM_WINDOWS_EXTENSIONS
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	// Local variable(s):
	bool parentConsoleAttached = false;

	// Initialize networking functionality.
	QSocket::initSockets();

	#ifdef PLATFORM_WINDOWS_EXTENSIONS
		// Kept in its own scope due to large stack allocation(s):
		{
			WCHAR pathBuffer[MAX_PATH];

			GetModuleFileNameW(NULL, (LPWSTR)pathBuffer, MAX_PATH);

			auto tempStr = wstring(pathBuffer);

			string::size_type dividerLocation = tempStr.find_last_of(L"\\/");

			if (dividerLocation != string::npos)
			{
				application::path = tempStr.substr(0, dividerLocation);
			}

			//application::path = wstring((const wchar_t*)pathBuffer);
		}

		if (!AttachConsole(ATTACH_PARENT_PROCESS))
		{
			AllocConsole();
		}
		else
		{
			parentConsoleAttached = true;
		}

		auto consoleOutput = freopen("CONOUT$", "w", stdout);
		auto consoleInput = freopen("CONIN$", "r", stdin);

		//freopen("CONOUT$", "w", stderr);

		/*
		// Allow for UTF16 text I/O:
		char* locale = setlocale(LC_ALL, "English");
		std::locale engLocale(locale);

		setlocale(LC_ALL, locale);

		wcout.imbue(engLocale);
		//wcin.imbue(engLocale);

		_setmode(_fileno(stdout), _O_U16TEXT);
		_setmode(_fileno(stdin), _O_U16TEXT);

		_wfreopen(L"CONOUT$", L"w", stdout);
		_wfreopen(L"CONOUT$", L"r", stdin);

		wcout << L"ルイジ." << endl;

		wcin.get();
		*/

		//SetStdHandle(STD_INPUT_HANDLE, GetStdHandle(STD_INPUT_HANDLE)); // stdin
		//SetStdHandle(STD_OUTPUT_HANDLE, GetStdHandle(STD_OUTPUT_HANDLE)); // stdout
	#else
		application::path = defaultStringToWide(argv[0]);
	#endif

	if (!parentConsoleAttached)
	{
		//wcout << "Path: " << "\"" << application::path << "\"" << endl;
		cout << "Starting the application..." << endl;
	}

	// Allocate the main application using the stack's memory.
	// This effectively embeds 'program' into 'main'.

	OSINFO OSInfo;

	#ifdef PLATFORM_WINDOWS_EXTENSIONS
		OSInfo = OSINFO(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	#else
		OSInfo = OSINFO(argc - 1, ((argc > 1) ? (argv + 1) : nullptr));
	#endif

	auto programData = runProgram(OSInfo, DEFAULT_UPDATERATE);

	#if defined(IOSYNC_LIVE_COMMANDS)
		#if !defined(IOSYNC_LIVE_COMMAND_INPUT_ON_MAIN_THREAD)
			// Ensure the the 'iosync_application' class's command-thread is closed.
			iosync_application::forceCloseCommandThread();
		#else // elif defined(IOSYNC_ALLOW_ASYNC_EXECUTE)
			lock_guard<mutex> startLock(programData->asyncExecutionMutex);

			// Accept commands as the application runs on another thread.
			iosync_application::beginLocalCommandAccept();
		#endif
	#endif

	#ifdef PLATFORM_WINDOWS
		#ifdef PLATFORM_WINDOWS_EXTENSIONS
			// Close the console-streams:
			fclose(consoleInput);
			fclose(consoleOutput);
		#endif
	#endif

	// Deinitialize networking functionality.
	QSocket::deinitSockets();

	#if defined(IOSYNC_LIVE_COMMAND_INPUT_ON_MAIN_THREAD)
		delete programData;
		
		return 0;
	#else
		return programData;
	#endif
}