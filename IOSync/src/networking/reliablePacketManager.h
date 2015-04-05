#pragma once

// Includes:
#include "networking.h"

#include <chrono>
#include <set>

// Namespace(s):
using namespace std;
using namespace chrono;

namespace iosync
{
	namespace networking
	{
		// Structures:
		struct reliablePacketManager
		{
			// Constructor(s):
			reliablePacketManager(bool calculateSnapshot=true);

			// Destructor(s):
			virtual ~reliablePacketManager();

			// Methods:
			virtual high_resolution_clock::time_point updateSnapshot();

			// This command tells you how many milliseconds have passed
			// since the last connection-time snapshot was created:
			inline milliseconds connectionTime() const
			{
				return elapsed(connectionSnapshot);
			}

			inline high_resolution_clock::time_point updateConfirmedPacketTimer()
			{
				// Update the confirmed-packet timer, then return it.
				return confirmedPacketTimer = high_resolution_clock::now();
			}

			inline milliseconds confirmedPacketTime() const
			{
				return elapsed(confirmedPacketTimer);
			}

			// Abstract methods:
			virtual bool hasReliablePackets() const = 0;

			virtual void removeReliablePacket(packetID ID) = 0;
			virtual bool hasPacket(packetID ID) const = 0;

			// Fields:

			// A timer used to tell if the earliest packet-identifier
			// in the 'confirmedPackets' container should be removed.
			high_resolution_clock::time_point confirmedPacketTimer;

			/*
				A "snapshot" of the time this object's "connection" was last pinged.

				This timer is used by 'serverNetworkEngines' in order to ping clients.

				This is also used by 'clientNetworkEngines' to detect
				the time since the last ping message was received.

				Furthermore, this can be used from the perspective of
				'player' objects, in order to detect connection-timeouts.
			*/

			high_resolution_clock::time_point connectionSnapshot;
		};
	}
}