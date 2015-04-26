// Includes:
#include "processManagement.h"

// Standard library:
#include <sstream>

#ifdef PROCESS_MANAGER_DEBUG
	#include <iostream>
#endif

// Namespace(s):
namespace process
{
	// Namespace(s):
	using namespace std;

	// Functions:
	DWORD getPID()
	{
		return GetProcessId(GetCurrentProcess());
	}

	BOOL isInjectionDLL(HMODULE hDLL)
	{
		return (BOOL)(GetProcAddress(hDLL, "XINPUT_INJECTOR_VALIDATOR") != NULL);
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

	string getName()
	{
		return getName(GetModuleHandle(NULL));
	}

	string getUniqueName()
	{
		stringstream s;
		string name = getName();

		s << name << rand();

		return s.str();
	}

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
				cout << "Remapping failed; unable to find remote function."
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