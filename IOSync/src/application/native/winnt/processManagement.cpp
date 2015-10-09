/*
	PLEASE VIEW THIS FILE'S PRIMARY HEADER, BEFORE USING THIS FUNCTIONALITY.
*/

// Includes:
#include "processManagement.h"

// Standard library:
#include <sstream>

#ifdef PROCESS_MANAGER_DEBUG
	#include <iostream>
#endif

// Windows-specific:
#include <tlhelp32.h>

// Namespace(s):
namespace process
{
	// Namespace(s):
	using namespace std;

	// Classes:

	// synchronized_process:

	// Functions:
	#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
		synchronized_process::Wow64SuspendThread_t synchronized_process::fnWow64SuspendThread = NULL;
		//synchronized_process::Wow64ResumeThread_t synchronized_process::fnWow64ResumeThread = NULL;
	#endif

	void synchronized_process::initializeSharedFunctions()
	{
		#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
			if (fnWow64SuspendThread == NULL) // nullptr
			{
				// Local variable(s):
				auto k32Handle = GetModuleHandle(TEXT("kernel32"));

				if (k32Handle != NULL)
				{
					fnWow64SuspendThread = (Wow64SuspendThread_t)GetProcAddress(k32Handle, "Wow64SuspendThread");
					//fnWow64ResumeThread = (Wow64ResumeThread_t)GetProcAddress(k32Handle, "Wow64ResumeThread");
				}
			}
		#endif
	}

	// Constructor(s):
	#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
		synchronized_process::synchronized_process(nativeID ID, bool retrieveSnapshot, bool is32b)
			: PID(ID), is32bit(is32b) { /* Nothing so far. */ }
	#endif

	synchronized_process::synchronized_process(nativeID ID, bool retrieveSnapshot) : PID(ID)
	{
		// Retrieve information regarding this process.
		retrieveProcessInfo();

		// Retrieve a thread-snapshot.
		if (retrieveSnapshot)
		{
			retrieveThreadSnapshot();
		}
	}

	// Destructor(s):
	synchronized_process::~synchronized_process()
	{
		// Release the current thread-snapshot.
		releaseThreadSnapshot();
	}

	// Methods:
	void synchronized_process::retrieveProcessInfo()
	{
		#if defined(PROCESS_MANAGER_WOW) && !defined(PROCESS_MANAGER_DEBUG_SUSPEND)
			is32bit = process32bit(PID);

			if (is32bit)
			{
				initializeSharedFunctions();
			}
		#endif

		return;
	}

	void synchronized_process::retrieveThreadSnapshot()
	{
		#ifndef PROCESS_MANAGER_DEBUG_SUSPEND
			//processHandle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, PID); // PROCESS_ALL_ACCESS
		#endif

		openThreads();

		return;
	}

	void synchronized_process::openThreads()
	{
		#ifndef PROCESS_MANAGER_DEBUG_SUSPEND
			//threads.clear();
			
			auto threadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, PID); // 0
			
			// Local variable(s):
			THREADENTRY32 currentEntry;

			ZeroVariable(currentEntry);

			currentEntry.dwSize = sizeof(THREADENTRY32);

			if (!Thread32First(threadSnapshot, &currentEntry))
			{
				releaseThreadSnapshot();

				return;
			}

			do
			{
				if (currentEntry.th32OwnerProcessID == PID)
				{
					threads.push_back(currentEntry.th32ThreadID);
				}
			} while (Thread32Next(threadSnapshot, &currentEntry));

			CloseHandle(threadSnapshot);
		#endif

		return;
	}

	void synchronized_process::releaseThreads()
	{
		#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
			if (suspended)
			{
				resume();
			}
		#endif

		#ifndef PROCESS_MANAGER_DEBUG_SUSPEND
			threads.clear();
		#endif

		return;
	}

	void synchronized_process::releaseThreadSnapshot()
	{
		releaseThreads();

		if (processHandle != NULL)
		{
			CloseHandle(processHandle); processHandle = NULL;
		}

		return;
	}

	void synchronized_process::updateThreadSnapshot()
	{
		releaseThreadSnapshot();
		retrieveThreadSnapshot();

		return;
	}

	void synchronized_process::suspend()
	{
		#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
			if (suspended)
				return;
		#endif

		#ifdef PROCESS_MANAGER_DEBUG_SUSPEND
			DebugActiveProcess(PID);
		#else
			for (auto& thread : threads)
			{
				// Open the current thread.
				HANDLE nativeThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, thread); // THREAD_ALL_ACCESS

				// Suspend execution of this thread:
				#ifdef PROCESS_MANAGER_WOW
					if (is32bit) // (fnWow64SuspendThread != NULL)
					{
						fnWow64SuspendThread(nativeThread);
					}
					else
				#endif
					{
						SuspendThread(nativeThread);
					}

				// Close our temporary handle to the current thread.
				CloseHandle(nativeThread);
			}
		#endif

		#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
			suspended = true;
		#endif

		return;
	}

	void synchronized_process::resume()
	{
		#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
		if (!suspended)
			return;
		#endif

		#ifdef PROCESS_MANAGER_DEBUG_SUSPEND
			DebugActiveProcessStop(PID);
		#else
			for (auto& thread : threads)
			{
				// Open the current thread.
				HANDLE nativeThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, thread); // THREAD_ALL_ACCESS

				/*
				#ifdef PROCESS_MANAGER_WOW
					if (!is32bit) // (fnWow64ResumeThread != NULL)
					{
						fnWow64ResumeThread(nativeThread);
					}
					else
				#endif
					{
						// ...
					}
				*/

				// Resume execution of this thread.
				ResumeThread(nativeThread);

				// Close our temporary handle to the current thread.
				CloseHandle(nativeThread);
			}
		#endif

		#if !defined(PROCESS_MANAGER_DEBUG_SUSPEND) && defined(PROCESS_MANAGER_WOW)
			suspended = false;
		#endif

		return;
	}

	// Functions:

	// Portable API:
	nativeID getPID()
	{
		return GetProcessId(GetCurrentProcess());
	}

	// Optional:
	bool process32bit(nativeID processID)
	{
		//#ifdef PROCESS_MANAGER_WOW
		// Typedefs:
		typedef decltype(&IsWow64Process) IsWow64Process_t;

		// Local variable(s):
		auto k32Handle = GetModuleHandle(TEXT("kernel32"));

		// Attempt to retrieve a remote kernel function.
		IsWow64Process_t fnIsWow64Process = NULL;

		if (k32Handle != NULL)
		{
			fnIsWow64Process = (IsWow64Process_t)GetProcAddress(k32Handle, "IsWow64Process");
		}

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
		//#endif

		return true;
	}

	string getName()
	{
		return getName(GetModuleHandle(NULL));
	}

	string getUniqueName()
	{
		stringstream s;

		s << getName() << rand();

		return s.str();
	}

	// Platform-specific:
	bool isInjectionDLL(HMODULE hDLL)
	{
		return (GetProcAddress(hDLL, "XINPUT_INJECTOR_VALIDATOR") != NULL);
	}

	string resolveSystemPath(LPCSTR path)
	{	
		// A temporary buffer for the system-path.
		char buffer[MAX_PATH];

		// This will not have a trailing back-slash.
		GetSystemDirectoryA(buffer, MAX_PATH);

		return string(buffer) + "\\" + path;
	}

	string getName(HMODULE module)
	{
		char name[MAX_PATH];

		GetModuleFileName(module, (LPSTR)name, MAX_PATH);

		return string(name);
	}

	bool startProcess(LPCTSTR applicationName, const string& commandLine, DWORD flags)
	{
		// Local variable(s):
		STARTUPINFO si;     
		PROCESS_INFORMATION pi;

		// Set the size of the structures:
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));

		si.cb = sizeof(si);

		CHAR cmd[MAX_PATH];

		memcpy(cmd, commandLine.c_str(), std::min((size_t)commandLine.size(), (size_t)MAX_PATH));
		cmd[commandLine.length()] = '\0';

		// Start the specified program:
		if
		(
			CreateProcess
			(
				applicationName,
				(LPSTR)cmd,
				NULL,
				NULL,
				FALSE,
				flags,
				NULL,
				NULL,
				&si,
				&pi
			) == FALSE
		)
		{
			return false;
		}

		// Close the process and thread handles:
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		// Return the default response.
		return true;
	}

	// Hardware-specific:
	void readJumpSegment(jumpSegment& output, LPCVOID address)
	{
		ReadProcessMemory(GetCurrentProcess(), address, output.data(), output.size(), NULL);

		return;
	}
	
	void writeJumpSegment(const jumpSegment& segment, LPVOID writeaddress)
	{
		WriteProcessMemory(GetCurrentProcess(), writeaddress, segment.data(), JUMP_SEGMENT_SIZE, NULL);

		return;
	}

	jumpSegment swapJumpInstruction(LPVOID writeaddress, const jumpSegment& segment)
	{
		jumpSegment currentData;

		readJumpSegment(currentData, writeaddress);

		writeJumpSegment(segment, writeaddress);

		return currentData;
	}

	jumpSegment writeJump(LPVOID writeaddress, LPCVOID funcaddress)
	{
		// Local variable(s):

		// This will act as the original segment, before the jump was injected.
		jumpSegment currentData;

		// Read the segment of memory safely.
		readJumpSegment(currentData, writeaddress);

		// Allocate a temporary buffer for jump-injection.
		unsigned char data[JUMP_SEGMENT_SIZE];

		// Generate the proper instruction:
		data[0] = 0xE9; // E8; // 9A;

		*(signed int *)(data + 1) = (unsigned int)funcaddress - ((unsigned int)writeaddress + 5);

		// Write over the targeted binary with our new instruction.
		WriteProcessMemory(GetCurrentProcess(), writeaddress, data, JUMP_SEGMENT_SIZE, NULL);

		// Return the original binary, now that we've replaced it.
		return currentData;
	}

	LPVOID getRemoteFunction(LPCSTR name, HMODULE hDLL)
	{
		return (LPVOID)GetProcAddress(hDLL, name);
	}

	LPVOID getRemoteFunction(LPCSTR name, LPCSTR DLL)
	{
		return getRemoteFunction(name, GetModuleHandleA(DLL));
	}

	jumpSegment mapRemoteFunction(LPCSTR name, LPCVOID function, HMODULE hDLL)
	{
		#ifdef PROCESS_MANAGER_DEBUG
			cout << "Attempting to map '" << function << "' to: '" << name << "'" << endl;
		#endif

		auto remoteFunction = getRemoteFunction(name, hDLL);

		if (remoteFunction == NULL)
		{
			#ifdef PROCESS_MANAGER_DEBUG
				cout << "Remapping failed; unable to find remote function." << endl;
			#endif

			// For now, do nothing.
			return jumpSegment();
		}

		return writeJump(remoteFunction, function);
	}

	jumpSegment mapRemoteFunction(LPCSTR name, LPCVOID function, LPCSTR DLL)
	{
		return writeJump(getRemoteFunction(name, DLL), function);
	}
}