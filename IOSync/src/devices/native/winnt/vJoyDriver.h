#pragma once

// Includes:
#include "../../gamepad.h"

#ifndef GAMEPAD_VJOY_ENABLED
	#error Please enable vJoy-support before including this file.
#endif

#include <vJoy/vjoyinterface.h>
#include <vJoy/public.h>

// Standard library:
#include <map>
//#include <list>

// Namespace(s):
namespace iosync
{
	namespace devices
	{
		namespace vJoy
		{
			// Structures:

			/*
				NOTES:
					* When using 'vJoyDevices' in any way, please
					be sure you have proper control over it first.

					This "driver" does not currently handle this for you.
			*/

			struct vJoyDriver final
			{
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
							if (GetVJDAxisExist(deviceIdentifier, axis))
							{
								LONG axis_min, axis_max;

								GetVJDAxisMin(deviceIdentifier, axis, &axis_min);
								GetVJDAxisMax(deviceIdentifier, axis, &axis_max);

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
						contPOVNumber = GetVJDContPovNumber(deviceIdentifier);
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

				// This will create, then update a "device"; use at your own risk.
				inline vJoyDevice& createAndUpdateDevice(const UINT deviceID)
				{
					// Local variable(s):

					// Allocate a new device object.
					auto device = vJoyDevice { deviceID };

					// Update the device-object.
					device.update();

					// Add the device, then return it.
					return devices[deviceID] = device;
				}

				// Please refrain from calling this very often.
				inline vJoyDevices::iterator updateDevice(const UINT deviceID)
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

				inline vJoyDevice& getDevice(const UINT deviceID)
				{
					auto devIterator = hasDevice_iterator(deviceID);

					if (devIterator != devices.end())
					{
						return devIterator->second;
					}

					return createAndUpdateDevice(deviceID);
				}
			};

			// Classes:
			// Nothing so far.
		}
	}
}