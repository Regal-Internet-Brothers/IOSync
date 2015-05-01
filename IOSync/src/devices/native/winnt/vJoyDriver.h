#pragma once

// Preprocessor related:
#define VJOY_INTERFACE_LIB "VJOYINTERFACE"
#define VJOY_INTERFACE_DLL "VJOYINTERFACE.DLL"

// Includes
#include "../../../platform.h"

// The 'GAMEPAD_VJOY_ENABLED' preprocessor-variable should
// only ever be defined when 'PLATFORM_WINDOWS' is defined.
// Any other behavior should be considered non-standard.
#if defined(PLATFORM_WINDOWS) && !defined(GAMEPAD_EXTERNAL_ORIGIN)
	// Disable this if needed; toggles optional vJoy support.
	#define GAMEPAD_VJOY_ENABLED
	
	#ifdef GAMEPAD_VJOY_ENABLED
		#define GAMEPAD_VJOY_SAFE
	#endif
#endif

#ifdef GAMEPAD_VJOY_ENABLED
	#define GAMEPAD_VJOY_DYNAMIC_LINK	
	
	#ifdef PLATFORM_X64
		#define VJOY_INTERFACE_DLL_GLOBAL "vJoy\\x64\\vJoyInterface.dll"
	#else
		#define VJOY_INTERFACE_DLL_GLOBAL "vJoy\\x86\\vJoyInterface.dll"
		#define VJOY_INTERFACE_DLL_GLOBAL_ALT "vJoy\\vJoyInterface.dll"
	#endif

	#include <vJoy/vjoyinterface.h>
	#include <vJoy/public.h>

	// Standard library:
	#include <map>
	//#include <list>
	
	#include <cstdarg>

	// Libraries:
	#ifndef GAMEPAD_VJOY_DYNAMIC_LINK
		#pragma comment(lib, VJOY_INTERFACE_LIB)
	#endif

	// Namespace(s):
	namespace iosync
	{
		namespace devices
		{
			namespace vJoy
			{
				// Namespace(s):
				using namespace std;

				namespace REAL_VJOY
				{
					// Functions:

					// This command restores the internal function-pointers to their original state.
					BOOL restoreFunctions();

					BOOL linkTo(HMODULE hDLL);

					HMODULE linkTo(LPCTSTR path);
					HMODULE linkTo();

					HMODULE globalLinkTo(size_t paths, ...);

					// vJoy API:
					BOOL __cdecl vJoyEnabled();
					BOOL __cdecl DriverMatch(WORD* DLLVer, WORD* DrvVer);
					BOOL __cdecl AcquireVJD(UINT rID);
					VOID __cdecl RelinquishVJD(UINT rID);
					BOOL __cdecl UpdateVJD(UINT rID, PVOID pData);
					enum VjdStat __cdecl GetVJDStatus(UINT rID);
					BOOL __cdecl GetVJDAxisExist(UINT rID, UINT axis);
					BOOL __cdecl GetVJDAxisMin(UINT rID, UINT axis, LONG* min);
					BOOL __cdecl GetVJDAxisMax(UINT rID, UINT axis, LONG* max);
					int __cdecl GetVJDContPovNumber(UINT rID);
				}
				
				// Structures:

				/*
					NOTES:
						* When using 'vJoyDevices' in any way, please
						be sure you have proper control over it first.

						This "driver" does not currently handle this for you.
				*/

				class vJoyDriver final
				{
					public:
						// Enumerator(s):
						enum vJoyDriverState
						{
							VJOY_UNDEFINED,
							VJOY_ENABLED,
							VJOY_DISABLED,
						};

						// Structures:
						struct vJoyDevice
						{
							// Structures:
							struct axis final
							{
								// Fields:
								LONG min, max;
							};

							// Typedefs:
							typedef map<UINT, axis> axes_t;

							// Fields:
							UINT deviceIdentifier;
							int contPOVNumber;

							axes_t axes;

							// Methods:
							inline axes_t::iterator getAxis(const UINT axis)
							{
								auto axisIterator = axes.find(axis);

								if (axisIterator == axes.end())
								{
									if (REAL_VJOY::GetVJDAxisExist(deviceIdentifier, axis))
									{
										LONG axis_min, axis_max;

										REAL_VJOY::GetVJDAxisMin(deviceIdentifier, axis, &axis_min);
										REAL_VJOY::GetVJDAxisMax(deviceIdentifier, axis, &axis_max);

										axes[axis] = { axis_min, axis_max };

										return axes.find(axis);
									}
								}

								return axisIterator;
							}

							inline bool hasAxis(const UINT axis)
							{
								return (getAxis(axis) != axes.end());
							}
							
							inline LONG axisMin(const UINT axis)
							{
								auto aIterator = getAxis(axis);

								if (aIterator != axes.end())
								{
									return aIterator->second.min;
								}

								//return -LONG_MAX;
								return 0;
							}

							inline LONG axisMax(const UINT axis)
							{
								auto aIterator = getAxis(axis);

								if (aIterator != axes.end())
								{
									return aIterator->second.max;
								}

								//return LONG_MAX;
								return 0;
							}

							// This command should only be called when initializing / "refreshing" this device.
							inline void update()
							{
								// Retrieve proper axis metrics:
								getAxis(HID_USAGE_X);
								getAxis(HID_USAGE_Y);

								getAxis(HID_USAGE_RX);
								getAxis(HID_USAGE_RY);

								getAxis(HID_USAGE_POV);
								getAxis(HID_USAGE_Z);

								// Get the number of continuous POV HATs.
								contPOVNumber = REAL_VJOY::GetVJDContPovNumber(deviceIdentifier);
							}
						};

						// Typedefs:
						typedef map<UINT, vJoyDevice> vJoyDevices;

						// Fields:
						vJoyDriverState state;
						vJoyDevices devices;

						// Methods:
						inline bool deviceFound(vJoyDevices::iterator it) const
						{
							return (it != devices.end());
						}

						inline vJoyDevices::iterator hasDevice_iterator(const UINT deviceID)
						{
							return devices.find(deviceID);
						}

						inline bool hasDevice(const UINT deviceID)
						{
							return deviceFound(hasDevice_iterator(deviceID));
						}

						vJoyDriverState init();
						vJoyDriverState deinit();

						// This will create, then update a "device"; use at your own risk.
						vJoyDevice& createAndUpdateDevice(const UINT deviceID);

						// Please refrain from calling this very often.
						vJoyDevices::iterator updateDevice(const UINT deviceID);
						vJoyDevice& getDevice(const UINT deviceID);

						// This will destroy all active "device" objects.
						// This class does not currently handle acquiring "devices",
						// so this will not relinquish anything directly.
						inline void flushDevices()
						{
							// Clear the internal container.
							devices.clear();

							return;
						}
				};

				// Classes:
				// Nothing so far.
			}
		}
	}
#endif