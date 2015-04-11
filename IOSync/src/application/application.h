#pragma once

// Preprocessor related:
#define WINDOW_NONE iosync::nativeWindow()

// Includes:
#include "application_exceptions.h"

#include "../platform.h"
#include "../networking/networking.h"

#include <cstdio>

#include <vector>
#include <string>
#include <codecvt>

// Namespace(s):
using namespace iosync::networking;

namespace iosync
{
	// Forward declarations:
	namespace networking
	{
		// Structures:
		struct messageHeader;
		struct messageFooter;

		struct player;

		// Classes:
		class networkEngine;
	}

	// Typedefs:
	typedef unsigned short rate;

	typedef std::vector<std::wstring> arguments;

	// The type of 'nativeWindow' is platform-dependent,
	// and may not be needed for some platforms:
	#ifdef PLATFORM_WINDOWS
		typedef HWND nativeWindow;
	#else
		typedef void* nativeWindow;
	#endif

	// Structures:
	struct OSINFO
	{
		// Constructor(s):
		#ifdef PLATFORM_WINDOWS_EXTENSIONS
			OSINFO(HINSTANCE hInstance=HINSTANCE(), HINSTANCE hPrevInstance=HINSTANCE(), LPSTR lpCmdLine=LPSTR(), int nCmdShow=0);
		#else
			OSINFO(int argCount=0, char** argVector=nullptr);
		#endif

		// Fields:
		#ifdef PLATFORM_WINDOWS_EXTENSIONS
			HINSTANCE hInstance;
			HINSTANCE hPrevInstance;
			LPSTR lpCmdLine;
			int nCmdShow;
		#else
			int argc;
			char** argv;
		#endif
	};

	// Enumerator(s):
	enum rates : rate
	{
		DEFAULT_UPDATERATE = 60,
	};

	// Classes:
	class application
	{
		public:
			// Global variable(s):
			static std::wstring_convert<std::codecvt_utf8<wchar_t>> stringConverter;

			static std::wstring path;

			// Functions:
			static inline string wideStringToDefault(const wstring wstr)
			{
				return stringConverter.to_bytes(wstr);
			}

			static inline wstring defaultStringToWide(const string str)
			{
				return stringConverter.from_bytes(str);
			}

			static inline void clearConsole()
			{
				// System-dependent, but it works.
				system("CLS");

				return;
			}

			#ifdef PLATFORM_WINDOWS
				// This command will inject the library specified into the process with the PID specified by 'processID'.
				// The return-value of this command indicates if injection was successful.
				static inline bool __winnt__injectLibrary(string library, DWORD processID)
				{
					// Local variable(s):
					HANDLE remoteProc;
					LPVOID remoteLibraryName;
					LPVOID LoadLibraryAddr;

					BOOL response = SetCurrentDirectoryW((LPCWSTR)path.c_str());

					char* buffer = new char[MAX_PATH];

					if (GetFullPathNameA(library.c_str(), MAX_PATH, (LPSTR)buffer, NULL) == 0)
						return false;

					//const char* buffer = library.c_str();
					
					//cout << "Directory: " << buffer << endl;

					SIZE_T strLength = (SIZE_T)strlen(buffer);

					// Open the remote process with specific rights.
					remoteProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

					if (remoteProc == 0)
						return false;

					// Load the "kernel32.dll" module's location in the remote process.
					LoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

					// Allocate a buffer using the remote process.
					remoteLibraryName = (LPVOID)VirtualAllocEx(remoteProc, NULL, strLength, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

					// Write the 'library' string to the newly allocated portion of memory.
					BOOL test = WriteProcessMemory(remoteProc, remoteLibraryName, buffer, strLength, NULL); // library.c_str()

					// Create a remote thread that will immediately load the library specified into the remote process.
					HANDLE h = CreateRemoteThread(remoteProc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, remoteLibraryName, NULL, NULL);

					// Close the handle to the remote process.
					CloseHandle(remoteProc);

					// Free the memory we allocated.
					VirtualFreeEx(remoteProc, remoteLibraryName, 0, MEM_RELEASE | MEM_DECOMMIT);

					delete buffer;

					// Return the default response.
					return true;
				}

				static inline bool __winnt__process32bit(DWORD processID)
				{
					// Typedefs:
					typedef decltype(&IsWow64Process) IsWow64Process_t;

					// Local variable(s):

					// Attempt to retrieve a remote kernel function.
					IsWow64Process_t fnIsWow64Process = (IsWow64Process_t)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

					if (fnIsWow64Process != NULL)
					{
						HANDLE p = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID); // PROCESS_VM_READ

						if (p == NULL)
							return true;

						BOOL is32bit;

						if (fnIsWow64Process(p, &is32bit) == FALSE)
						{
							return true;
						}

						CloseHandle(p);

						return (is32bit == TRUE);
					}

					return true;
				}
			#endif

			// Constructor(s):
			application(rate updateRate = DEFAULT_UPDATERATE, OSINFO info=OSINFO());

			// Destructor(s):
			virtual ~application()
			{
				// Nothing so far.
			}

			// Methods:
			virtual int execute();

			// This method is abstract, and must be implemented.
			virtual void update(rate frameNumber=0) = 0;

			virtual nativeWindow getWindow() const;

			// Networking related:

			// Parsing/deserialization related:

			// This command provides message handling for 'application' objects.
			// Applications do not have to support networking, but a default implementation is provided.
			// This command may be used to virtually handle networking-messages from the perspective of an 'application'.
			virtual bool parseNetworkMessage(QSocket& socket, const messageHeader& header, const messageFooter& footer);

			// Call-backs:

			/*
				This is called every time a client attempts to connect
				to a remote host, without receiving a preliminary error.
				
				This is not called when servers receive player join-requests.
				For that behavior, please use 'onNetworkClientConnected'.
			*/

			virtual void onNetworkConnected(networkEngine& engine);

			// This is called any time a client/player connects to the network.
			virtual void onNetworkClientConnected(networkEngine& engine, player& p);

			// This is called whenever a client times out.
			virtual void onNetworkClientTimedOut(networkEngine& engine, player& p);

			// This is called when a network-engine is closed.
			virtual void onNetworkClosed(networkEngine& engine);

			// Fields:

			// This acts as our standard argument container.
			arguments args;

			// The information supplied by the operating system.
			OSINFO OSInfo;

			// Rates:
			rate updateRate;

			// Booleans / Flags:
			bool isRunning;
	};
}