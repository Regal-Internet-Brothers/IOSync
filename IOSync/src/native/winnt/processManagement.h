#pragma once

// Preprocessor related:
//#define XINPUT_DLL "xinput9_1_0.dll" // "Xinput.dll"
//#define INJECTOR_XINPUT_DEFAULT_DLL_W L"xinput1_3.dll" // L"xinput1_4.dll"

#define INJECTOR_XINPUT_DEFAULT_DLL_A "xinput1_3.dll" // "xinput9_1_0.dll" // xinput1_4.dll"
#define XINPUT_COMPATIBILITY_DLL "xinput9_1_0.dll"

#define XINPUT_MAX_SUBVERSION 4
#define XINPUT_DLL_NAMELENGTH  16 // xinput1_4.dll

//#define PROCESS_MANAGER_DEBUG

// Includes:
#include <windows.h>
#include <psapi.h>

// Standard library:
#include <string>
#include <array>

// Namespace(s):
namespace process
{
	// Namespace(s):
	using namespace std;

	// Constant variable(s):
	const size_t JUMP_SEGMENT_SIZE = 5;

	// Typedefs:

	// A "segment" of memory dedicated to a jump instruction.
	typedef array<unsigned char, JUMP_SEGMENT_SIZE> jumpSegment;
	
	// Functions:
	DWORD getPID();

	// This will tell you if the module specified is an injection module/DLL.
	BOOL isInjectionDLL(HMODULE hDLL);

	// This will resolve the proper system-path for the file specified.
	// This is useful for modules/DLLs that could be "misrepresented" by local implementations.
	string resolveSystemPath(LPCSTR path);

	// This will return the file-name of the module specified.
	string getName(HMODULE module);

	// This will return the name of this process's executable.
	string getName();

	// This will return the result of 'getName', plus an additional random-number.
	string getUniqueName();

	void readJumpSegment(jumpSegment& output, LPCVOID address);
	void writeJumpSegment(const jumpSegment& segment, LPVOID writeaddress);

	jumpSegment swapJumpInstruction(LPVOID writeaddress, const jumpSegment& segment);
	jumpSegment writeJump(LPVOID writeaddress, LPCVOID funcaddress);

	LPVOID getRemoteFunction(LPCSTR name, HMODULE hDLL);
	LPVOID getRemoteFunction(LPCSTR name, LPCSTR DLL=INJECTOR_XINPUT_DEFAULT_DLL_A);

	jumpSegment mapRemoteFunction(LPCSTR name, LPCVOID function, HMODULE hDLL);
	jumpSegment mapRemoteFunction(LPCSTR name, LPCVOID function, LPCSTR DLL=INJECTOR_XINPUT_DEFAULT_DLL_A);

	// This command requires you to link with "psapi.lib"; use at your own risk.
	template <const size_t max_processes=1024>
	inline DWORD PIDFromProcessNameW(const wstring& entry)
	{
		DWORD aProcesses[max_processes], cbNeeded;

		DWORD PID = 0;

		if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		{
			DWORD cProcesses = (cbNeeded / sizeof(DWORD));

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
						DWORD _cbNeeded;

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
}