/*
	Native Windows implementation of IOSync's process management functionality.
	
	(x86/x64; some hardware specific behavior)

	This file, and any associated "native" files reserves the
	right to use platform-specific functionality, including types.
	
	Code relying upon this functionality is NOT guaranteed to be portable.
*/

#pragma once

// Preprocessor related:
//#define XINPUT_DLL "xinput9_1_0.dll" // "Xinput.dll"
//#define INJECTOR_XINPUT_DEFAULT_DLL_W L"xinput1_3.dll" // L"xinput1_4.dll"

#define INJECTOR_XINPUT_DEFAULT_DLL_A "xinput1_3.dll" // "xinput9_1_0.dll" // xinput1_4.dll"
#define XINPUT_COMPATIBILITY_DLL "xinput9_1_0.dll"

#define XINPUT_MAX_SUBVERSION 4
#define XINPUT_DLL_NAMELENGTH  16 // xinput1_4.dll

//#define PROCESS_MANAGER_DEBUG

#define PROCESS_MANAGER_IMPLEMENTED
#define PROCESS_MANAGER_SYNCHRONIZED_PROCESS_IMPLEMENTED

//#define PROCESS_MANAGER_DEBUG_SUSPEND

// Includes:
#include "../../../platform.h"

// Windows API:
#include <windows.h>
#include <psapi.h>

// Standard library:
#include <string>
#include <array>
#include <vector>
#include <queue>

#ifdef PLATFORM_64
	// "WOW" stands for "Windows on Windows"; a compatibility layer.
	#define PROCESS_MANAGER_WOW
#endif

// Namespace(s):
namespace process
{
	// Namespace(s):
	using namespace std;

	// Constant variable(s):
	const size_t JUMP_SEGMENT_SIZE = 5;

	// Typedefs:

	// A "segment" of memory dedicated to a jump instruction.
	typedef array<unsigned char, JUMP_SEGMENT_SIZE> jumpSegment; // uint8_t;

	// This platform's native process-identifier.
	typedef DWORD nativeID;
	typedef DWORD nativeThreadID;
	
	// Classes:

	// Optional API:
	//#ifdef PROCESS_MANAGER_SYNCHRONIZED_PROCESS_IMPLEMENTED
	class synchronized_process final
	{
		public:
			// Typedefs:
			#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
				typedef decltype(&Wow64SuspendThread) Wow64SuspendThread_t;
				//typedef decltype(&Wow64ResumeThread) Wow64ResumeThread_t;
			#endif

			// Functions:
			#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
				static Wow64SuspendThread_t fnWow64SuspendThread;
				//static Wow64ResumeThread_t fnWow64ResumeThread;
			#endif

			static void initializeSharedFunctions();

			// Constructor(s):
			#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
				synchronized_process(nativeID ID, bool retrieveSnapshot, bool is32b);
			#endif

			synchronized_process(nativeID ID, bool retrieveSnapshot=true);

			// Destructor(s):
			~synchronized_process();

			// Methods:
			void retrieveProcessInfo();

			// These may be used to manually retrieve and release thread handles:
			void openThreads();
			void releaseThreads();

			// Ideally, you'll use these to handle threads:
			void retrieveThreadSnapshot();
			void releaseThreadSnapshot();
			void updateThreadSnapshot();

			// Only use these after updating/retrieving a thread-snapshot:
			void suspend();
			void resume();

			// Fields:

			// The native process-identifier this object represents.
			nativeID PID;

			// Windows-specific:
			HANDLE processHandle = NULL;

			#ifndef PROCESS_MANAGER_DEBUG_SUSPEND
				// All of the currently open threads of this process.
				vector<nativeThreadID> threads;
			#endif

			// Booleans / Flags:
			#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
				bool is32bit; // = true;

				bool suspended = false;
			#endif
	};
	//#endif

	// Functions:

	// Portable API (Available, as long as this API is implemented):
	nativeID getPID();

	// Optional (Must at least provide default values):
	bool process32bit(nativeID processID);

	// This will return the name of this process's executable.
	string getName();

	// This will return the result of 'getName', plus an additional random-number.
	string getUniqueName();

	// This command requires you to link with "psapi.lib"; use at your own risk.
	template <const size_t max_processes=1024>
	inline nativeID PIDFromProcessNameW(const wstring& entry)
	{
		nativeID aProcesses[max_processes], cbNeeded;

		nativeID PID = 0;

		if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		{
			nativeID cProcesses = (cbNeeded / sizeof(nativeID));

			for (unsigned int i = 0; i < cProcesses; i++)
			{
				if (aProcesses[i] != 0) // NULL
				{
					HANDLE hProcess = OpenProcess
					(
						PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
						FALSE, aProcesses[i]
					);

					if (hProcess != NULL)
					{
						HMODULE hMod;
						nativeID _cbNeeded;

						if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &_cbNeeded))
						{
							WCHAR processName[MAX_PATH] = L"<unknown>";

							GetModuleBaseNameW(hProcess, hMod, (LPWSTR)processName, sizeof(processName)/sizeof(WCHAR));

							if (entry == processName)
							{
								PID = aProcesses[i];
							}
						}

						CloseHandle(hProcess);
					}
				}

				if (PID != 0)
					break;
			}
		}

		return PID;
	}

	template <typename callback, const size_t max_processes=1024>
	inline void PIDsFromProcessNameW(const wstring& entry, callback&& predicate)
	{
		nativeID aProcesses[max_processes], cbNeeded;

		if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		{
			nativeID cProcesses = (cbNeeded / sizeof(nativeID));

			for (unsigned int i = 0; i < cProcesses; i++)
			{
				if (aProcesses[i] != 0) // NULL
				{
					HANDLE hProcess = OpenProcess
					(
						PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
						FALSE, aProcesses[i]
					);

					if (hProcess != NULL)
					{
						HMODULE hMod;
						nativeID _cbNeeded;

						if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &_cbNeeded))
						{
							WCHAR processName[MAX_PATH] = L"<unknown>";

							GetModuleBaseNameW(hProcess, hMod, (LPWSTR)processName, sizeof(processName)/sizeof(WCHAR));

							if (entry == processName)
							{
								predicate(aProcesses[i]);
							}
						}

						CloseHandle(hProcess);
					}
				}
			}
		}

		return;
	}

	template <const size_t max_processes=1024>
	inline vector<nativeID> PIDsFromProcessNameW(const wstring& entry)
	{
		auto add = [&PIDs] (const nativeID& PID) { PIDs.push_back(PID); return; };

		vector<nativeID> PIDs;

		PIDsFromProcessNameW<decltype(add), max_processes>(entry, add);

		return PIDs;
	}

	// This has the same requirements as 'PIDFromProcessNameW'.
	template <const size_t max_processes=1024>
	inline nativeID resolvePIDW(const wstring& entry)
	{
		try
		{
			return (nativeID)stoi(entry); // PID;
		}
		catch (std::invalid_argument&)
		{
			// Local variable(s):
			nativeID PID = 0;

			GetWindowThreadProcessId(FindWindowW(NULL, (LPCWSTR)entry.c_str()), (LPDWORD)&PID);

			if (PID == 0)
			{
				return PIDFromProcessNameW<max_processes>(entry); // PID;
			}

			return PID;
		}

		return 0;
	}

	template <typename callback, const size_t max_processes=1024>
	inline void resolvePIDsW(const wstring& entry, callback&& predicate)
	{
		try
		{
			predicate((nativeID)stoi(entry)); // PID;
		}
		catch (std::invalid_argument&)
		{
			// Local variable(s):
			nativeID PID = 0;

			GetWindowThreadProcessId(FindWindowW(NULL, (LPCWSTR)entry.c_str()), (LPDWORD)&PID);

			if (PID == 0)
			{
				PIDsFromProcessNameW<callback&, max_processes>(entry, predicate);
			}
			else
			{
				predicate(PID);
			}
		}

		return;
	}

	// Platform-specific (Non-portable):

	// This will tell you if the module specified is an injection module/DLL.
	bool isInjectionDLL(HMODULE hDLL);

	// This will resolve the proper system-path for the file specified.
	// This is useful for modules/DLLs that could be "misrepresented" by local implementations.
	string resolveSystemPath(LPCSTR path);

	// This will return the file-name of the module specified.
	string getName(HMODULE module);

	bool startProcess(LPCTSTR applicationName, const string& commandLine=string(), DWORD flags=CREATE_NO_WINDOW);

	// Hardware-specific (Also platform-specific):
	void readJumpSegment(jumpSegment& output, LPCVOID address);
	void writeJumpSegment(const jumpSegment& segment, LPVOID writeaddress);

	jumpSegment swapJumpInstruction(LPVOID writeaddress, const jumpSegment& segment);
	jumpSegment writeJump(LPVOID writeaddress, LPCVOID funcaddress);

	LPVOID getRemoteFunction(LPCSTR name, HMODULE hDLL);
	LPVOID getRemoteFunction(LPCSTR name, LPCSTR DLL=INJECTOR_XINPUT_DEFAULT_DLL_A);

	jumpSegment mapRemoteFunction(LPCSTR name, LPCVOID function, HMODULE hDLL);
	jumpSegment mapRemoteFunction(LPCSTR name, LPCVOID function, LPCSTR DLL=INJECTOR_XINPUT_DEFAULT_DLL_A);
}