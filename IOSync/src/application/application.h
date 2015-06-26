#pragma once

// Preprocessor related:
#define WINDOW_NONE iosync::nativeWindow()

// Includes:
#include "application_exceptions.h"

#include "../platform.h"
#include "../util.h"
#include "../networking/networking.h"

// Platform-specific:
#ifdef PLATFORM_WINDOWS
	#include "native/winnt/processManagement.h"
#endif

// Standard library:
#include <cstdio>

#include <vector>
#include <string>
#include <iostream>

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
		DEFAULT_UPDATERATE = 60, // 120,
	};

	// Classes:
	class application
	{
		public:
			// Typedefs:
			typedef unsigned long long frameNumber;

			// Global variable(s):
			static std::wstring path;

			// Functions:
			static inline void clearConsole()
			{
				#ifdef PLATFORM_WINDOWS
					// System-dependent, but it works.
					system("CLS");
				#elif defined(PLATFORM_LINUX)
					system("clear");
				#else
					// Nothing so far.
				#endif

				return;
			}

			// This command tells you if the user's input-character is "true".
			static inline bool isEnabled(const int character)
			{
				if (character == '1') // (character == 1)
					return true;

				auto lcCharacter = tolower(character);

				return (lcCharacter == 'y' || lcCharacter == 't'); // (lcCharacter == 'a')
			}

			template <typename characterType=char, typename characterTraits=char_traits<characterType>>
			static inline bool userBoolean(std::basic_istream<characterType, characterTraits>& is=cin)
			{
				// Local variable(s):
				characterType choice; is >> choice;

				return isEnabled(choice);
			}

			template <typename characterType, typename characterTraits=char_traits<characterType>, typename strAlloc=allocator<characterType>>
			static inline bool strEnabled(const basic_string<characterType, characterTraits, strAlloc>& str)
			{
				return (!str.empty()) ? isEnabled(str[0]) : false;
			}

			static inline bool wstrEnabled(const wstring& wstr)
			{
				return strEnabled<wchar_t>(wstr);
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

					if (buffer == nullptr || GetFullPathNameA(library.c_str(), MAX_PATH, (LPSTR)buffer, NULL) == 0)
						return false;

					//const char* buffer = library.c_str();
					
					//cout << "Directory: " << buffer << endl;

					SIZE_T strLength = (SIZE_T)strlen(buffer);
					//buffer[strLength] = '\0';

					// Open the remote process with specific rights.
					remoteProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

					if (remoteProc == 0)
						return false;

					// Load the "kernel32.dll" module's location in the remote process.
					LoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

					// Allocate a buffer using the remote process.
					remoteLibraryName = (LPVOID)VirtualAllocEx(remoteProc, NULL, strLength, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); // strLength+1

					if (remoteLibraryName == NULL)
						return false;

					// Write the 'library' string to the newly allocated portion of memory.
					BOOL test = WriteProcessMemory(remoteProc, remoteLibraryName, buffer, strLength, NULL); // strLength+1

					HANDLE h;

					// Check for errors, then create a remote thread that will immediately load the library specified into the remote process:
					if (test == FALSE || (h = CreateRemoteThread(remoteProc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, remoteLibraryName, NULL, NULL)) == NULL)
					{
						// Free the memory we allocated.
						VirtualFreeEx(remoteProc, remoteLibraryName, 0, MEM_RELEASE); // MEM_DECOMMIT

						return false;
					}

					// Close our local handle to the remote thread.
					CloseHandle(h);

					// Close the handle to the remote process.
					CloseHandle(remoteProc);

					// Free the memory we allocated.
					VirtualFreeEx(remoteProc, remoteLibraryName, 0, MEM_RELEASE); // MEM_DECOMMIT

					delete[] buffer;

					// Return the default response.
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

			// Methods (Public):
			virtual int execute();

			// This method is abstract, and must be implemented.
			virtual void update(rate localFrame=0) = 0;

			virtual nativeWindow getWindow() const;

			inline frameNumber getFrame() const
			{
				return currentFrame;
			}

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

			// Fields (Public):

			// This acts as our standard argument container.
			arguments args;

			// The information supplied by the operating system.
			OSINFO OSInfo;

			// Rates:
			rate updateRate;

			// Booleans / Flags:
			bool isRunning;
		protected:
			// Fields (Protected):

			// The current "frame number" of this application.
			frameNumber currentFrame;
	};
}