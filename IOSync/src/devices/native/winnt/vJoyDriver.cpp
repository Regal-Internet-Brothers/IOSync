// Preprocessor related:
#define _CRT_SECURE_NO_WARNINGS

// Includes:
#include "vJoyDriver.h"

#ifdef GAMEPAD_VJOY_ENABLED
	// Windows API:
	#include <Shlobj.h>
	#include <tchar.h>

	// Namespace(s):
	namespace iosync
	{
		namespace devices
		{
			namespace vJoy
			{
				namespace REAL_VJOY
				{
					// Typedefs:

					// Function types:
					typedef decltype(&::vJoyEnabled) _vJoyEnabled_t;
					typedef decltype(&::DriverMatch) _DriverMatch_t;
					typedef decltype(&::AcquireVJD) _AcquireVJD_t;
					typedef decltype(&::RelinquishVJD) _RelinquishVJD_t;
					typedef decltype(&::UpdateVJD) _UpdateVJD_t;
					typedef decltype(&::GetVJDStatus) _GetVJDStatus_t;
					typedef decltype(&::GetVJDAxisExist) _GetVJDAxisExist_t;
					typedef decltype(&::GetVJDAxisMin) _GetVJDAxisMin_t;
					typedef decltype(&::GetVJDAxisMax) _GetVJDAxisMax_t;
					typedef decltype(&::GetVJDContPovNumber) _GetVJDContPovNumber_t;

					// Global variable(s):

					// Function pointers:
					_vJoyEnabled_t _vJoyEnabled;
					_DriverMatch_t _DriverMatch;
					_AcquireVJD_t _AcquireVJD;
					_RelinquishVJD_t _RelinquishVJD;
					_UpdateVJD_t _UpdateVJD;
					_GetVJDStatus_t _GetVJDStatus;
					_GetVJDAxisExist_t _GetVJDAxisExist;
					_GetVJDAxisMin_t _GetVJDAxisMin;
					_GetVJDAxisMax_t _GetVJDAxisMax;
					_GetVJDContPovNumber_t _GetVJDContPovNumber;

					// Functions:
					BOOL restoreFunctions()
					{
						// Set every function-pointer back to 'nullptr':
						_vJoyEnabled = nullptr;
						_DriverMatch = nullptr;
						_AcquireVJD = nullptr;
						_RelinquishVJD = nullptr;
						_UpdateVJD = nullptr;
						_GetVJDStatus = nullptr;
						_GetVJDAxisExist = nullptr;
						_GetVJDAxisMin = nullptr;
						_GetVJDAxisMax = nullptr;
						_GetVJDContPovNumber = nullptr;

						// Return the default response.
						return TRUE;
					}

					BOOL linkTo(HMODULE hDLL)
					{
						// This routine requires that every vJoy command used must be present:
						if ((_vJoyEnabled = (_vJoyEnabled_t)GetProcAddress(hDLL, "vJoyEnabled")) == nullptr)
							return !restoreFunctions();
						else if ((_DriverMatch = (_DriverMatch_t)GetProcAddress(hDLL, "DriverMatch")) == nullptr)
							return !restoreFunctions();
						else if ((_AcquireVJD = (_AcquireVJD_t)GetProcAddress(hDLL, "AcquireVJD")) == nullptr)
							return !restoreFunctions();
						else if ((_RelinquishVJD = (_RelinquishVJD_t)GetProcAddress(hDLL, "RelinquishVJD")) == nullptr)
							return !restoreFunctions();
						else if ((_UpdateVJD = (_UpdateVJD_t)GetProcAddress(hDLL, "UpdateVJD")) == nullptr)
							return !restoreFunctions();
						else if ((_GetVJDStatus = (_GetVJDStatus_t)GetProcAddress(hDLL, "GetVJDStatus")) == nullptr)
							return !restoreFunctions();
						else if ((_GetVJDAxisExist = (_GetVJDAxisExist_t)GetProcAddress(hDLL, "GetVJDAxisExist")) == nullptr)
							return !restoreFunctions();
						else if ((_GetVJDAxisMin = (_GetVJDAxisMin_t)GetProcAddress(hDLL, "GetVJDAxisMin")) == nullptr)
							return !restoreFunctions();
						else if ((_GetVJDAxisMax = (_GetVJDAxisMax_t)GetProcAddress(hDLL, "GetVJDAxisMax")) == nullptr)
							return !restoreFunctions();
						else if ((_GetVJDContPovNumber = (_GetVJDContPovNumber_t)GetProcAddress(hDLL, "GetVJDContPovNumber")) == nullptr)
							return !restoreFunctions();

						// Tell the user there wasn't any errors.
						return TRUE;
					}

					HMODULE linkTo(LPCTSTR path)
					{
						HMODULE module = LoadLibrary(path);

						if (module != NULL)
						{
							if (!linkTo(module))
							{
								FreeLibrary(module);
							}
							else
							{
								return module;
							}
						}

						return NULL;
					}

					HMODULE linkTo()
					{
						// Local variable(s):
						HMODULE module = NULL;

						if ((module = linkTo(TEXT(VJOY_INTERFACE_DLL))) != NULL)
							return module;
						else
						{
							// Attempt to link to the module at 'file':
							#ifdef PLATFORM_X64
								if ((module = globalLinkTo(1, TEXT(VJOY_INTERFACE_DLL_GLOBAL))) != NULL)
								{
									return module;
								}
							#else
								if ((module = globalLinkTo(2, TEXT(VJOY_INTERFACE_DLL_GLOBAL), TEXT(VJOY_INTERFACE_DLL_GLOBAL_ALT))) != NULL)
								{
									return module;
								}
							#endif
						}

						// Return the default response.
						return NULL;
					}

					HMODULE globalLinkTo(size_t paths, ...)
					{
						// Local variable(s):
						va_list local_paths;

						TCHAR file[MAX_PATH];
						TCHAR programFiles[MAX_PATH];

						HMODULE module = NULL;

						// Attempt to get the program-files directory:
						if (!SUCCEEDED(SHGetSpecialFolderPath(NULL, (LPTSTR)programFiles, CSIDL_PROGRAM_FILES, FALSE)))
							return NULL;

						// Start reading dynamic arguments.
						va_start(local_paths, paths);

						// Output to the file-path buffer.
						for (size_t i = 0; i < paths; i++)
						{
							_stprintf(file, TEXT("%s%s%s"), programFiles, TEXT("\\"), va_arg(local_paths, LPCTSTR));

							module = linkTo(file);

							if (module != NULL)
								break;
						}

						// Stop reading dynamic arguments.
						va_end(local_paths);

						// Return the loaded module.
						return module;
					}

					// vJoy API:
					BOOL __cdecl vJoyEnabled()
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_vJoyEnabled != nullptr)
							{
								return _vJoyEnabled();
							}

							return FALSE;
						#else
							return ::vJoyEnabled();
						#endif
					}

					BOOL __cdecl DriverMatch(WORD* DLLVer, WORD* DrvVer)
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_DriverMatch != nullptr)
							{
								return _DriverMatch(DLLVer, DrvVer);
							}

							return FALSE;
						#else
							return ::DriverMatch(DLLVer, DrvVer);
						#endif
					}

					BOOL __cdecl AcquireVJD(UINT rID)
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_AcquireVJD != nullptr)
							{
								return _AcquireVJD(rID);
							}

							return FALSE;
						#else
							return ::AcquireVJD(rID);
						#endif
					}

					VOID __cdecl RelinquishVJD(UINT rID)
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_RelinquishVJD != nullptr)
							{
								_RelinquishVJD(rID);

								return;
							}

							return;
						#else
							::RelinquishVJD(rID);
						
							return;
						#endif
					}

					BOOL __cdecl UpdateVJD(UINT rID, PVOID pData)
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_UpdateVJD != nullptr)
							{
								return _UpdateVJD(rID, pData);
							}

							return FALSE;
						#else
							return ::UpdateVJD(rID, pData);
						#endif
					}

					enum VjdStat __cdecl GetVJDStatus(UINT rID)
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_GetVJDStatus != nullptr)
							{
								return _GetVJDStatus(rID);
							}

							return VjdStat::VJD_STAT_MISS;
						#else
							return ::GetVJDStatus(rID);
						#endif
					}

					BOOL __cdecl GetVJDAxisExist(UINT rID, UINT axis)
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_GetVJDAxisExist != nullptr)
							{
								return _GetVJDAxisExist(rID, axis);
							}

							return FALSE;
						#else
							return ::GetVJDAxisExist(rID, axis);
						#endif
					}

					BOOL __cdecl GetVJDAxisMin(UINT rID, UINT axis, LONG* min)
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_GetVJDAxisMin != nullptr)
							{
								return _GetVJDAxisMin(rID, axis, min);
							}

							return FALSE;
						#else
							return ::GetVJDAxisMin(rID, axis, min);
						#endif
					}

					BOOL __cdecl GetVJDAxisMax(UINT rID, UINT axis, LONG* max)
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_GetVJDAxisMax != nullptr)
							{
								return _GetVJDAxisMax(rID, axis, max);
							}

							return FALSE;
						#else
							return ::GetVJDAxisMax(rID, axis, max);
						#endif
					}

					int __cdecl GetVJDContPovNumber(UINT rID)
					{
						#ifdef GAMEPAD_VJOY_DYNAMIC_LINK
							if (_GetVJDContPovNumber != nullptr)
							{
								return _GetVJDContPovNumber(rID);
							}

							return 0;
						#else
							return ::GetVJDContPovNumber(rID);
						#endif
					}
				}

				// Classes:

				// vJoyDriver:

				// Methods:
				vJoyDriver::vJoyDriverState vJoyDriver::init()
				{
					// Check for errors:
					if (state == VJOY_ENABLED)
						return state;

					// Resolve the current "driver" state:
					if (REAL_VJOY::vJoyEnabled())
					{
						WORD DLLVer, DrvVer;

						if (!REAL_VJOY::DriverMatch(&DLLVer, &DrvVer))
						{
							// Set the internal state, then return it.
							return (state = VJOY_DISABLED);
						}

						state = VJOY_ENABLED;
					}
					else
					{
						state = VJOY_DISABLED;
					}

					return state;
				}

				vJoyDriver::vJoyDriverState vJoyDriver::deinit()
				{
					// Check for errors:
					if (state != vJoyDriver::VJOY_ENABLED)
						return state;

					// "Flush" all vJoy representations.
					flushDevices();

					// Set the internal state, then return it.
					return state = vJoyDriver::VJOY_UNDEFINED; // vJoyDriver::VJOY_DISABLED;
				}

				vJoyDriver::vJoyDevice& vJoyDriver::createAndUpdateDevice(const UINT deviceID)
				{
					// Local variable(s):

					// Allocate a new device object.
					auto device = vJoyDevice { deviceID };

					// Update the device-object.
					device.update();

					// Add the device, then return it.
					return devices[deviceID] = device;
				}

				vJoyDriver::vJoyDevices::iterator vJoyDriver::updateDevice(const UINT deviceID)
				{
					auto devIterator = hasDevice_iterator(deviceID);

					if (devIterator == devices.end())
					{
						createAndUpdateDevice(deviceID);

						return hasDevice_iterator(deviceID);
					}

					devIterator->second.update();

					return devIterator;
				}

				vJoyDriver::vJoyDevice& vJoyDriver::getDevice(const UINT deviceID)
				{
					auto devIterator = hasDevice_iterator(deviceID);

					if (deviceFound(devIterator))
					{
						return devIterator->second;
					}

					return createAndUpdateDevice(deviceID);
				}

				// Structures:

			}
		}
	}
#endif