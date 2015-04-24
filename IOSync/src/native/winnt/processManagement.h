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

// Standard library:
#include <string>

// Namespace(s):
namespace process
{
	// Namespace(s):
	using namespace std;

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

	BOOL writeJump(LPVOID writeaddress, LPVOID funcaddress);

	LPVOID getRemoteFunction(LPCSTR name, HMODULE hDLL);
	LPVOID getRemoteFunction(LPCSTR name, LPCSTR DLL=INJECTOR_XINPUT_DEFAULT_DLL_A);

	BOOL mapRemoteFunction(LPCSTR name, LPVOID function, HMODULE hDLL);
	BOOL mapRemoteFunction(LPCSTR name, LPVOID function, LPCSTR DLL=INJECTOR_XINPUT_DEFAULT_DLL_A);
}