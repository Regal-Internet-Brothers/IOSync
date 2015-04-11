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

	BOOL writeJump(LPVOID writeaddress, LPVOID funcaddress)
	{
		unsigned char data[5];

		data[0] = 0xE9; // E8; // 9A;

		*(signed int *)(data + 1) = (unsigned int)funcaddress - ((unsigned int)writeaddress + 5);

		return WriteProcessMemory(GetCurrentProcess(), writeaddress, data, 5, NULL);
	}

	LPVOID getRemoteFunction(LPCSTR name, HMODULE hDLL)
	{
		return (LPVOID*)GetProcAddress(hDLL, name);
	}

	LPVOID getRemoteFunction(LPCSTR name, LPCSTR DLL)
	{
		return getRemoteFunction(name, GetModuleHandleA(DLL));
	}

	BOOL mapRemoteFunction(LPCSTR name, LPVOID function, HMODULE hDLL)
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

			return FALSE;
		}

		return writeJump(remoteFunction, function);
	}

	BOOL mapRemoteFunction(LPCSTR name, LPVOID function, LPCSTR DLL)
	{
		return writeJump(getRemoteFunction(name, DLL), function);
	}
}