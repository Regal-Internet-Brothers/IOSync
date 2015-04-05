#pragma once

// Includes:
#include "networking.h"
#include "reliablePacketManager.h"
#include "address.h"

#include <set>

// Namespace(s):
using namespace std;

namespace iosync
{
	namespace networking
	{
		// Structures:
		struct player : reliablePacketManager
		{
			// Constructor(s):
			player(address remote = address(), bool calculateSnapshot = true);

			// Destructor(s):
			virtual ~player();

			// Methods:
			bool pruneReliablePacket(milliseconds requiredTime);

			void addReliablePacket(packetID ID);
			void pruneEarliestPacket();

			virtual void removeReliablePacket(packetID ID) override;
			virtual bool hasPacket(packetID ID) const override;

			// This will return a blank address if a "virtual address" could not be found.
			// This is implementation dependent, if you wish to
			// supply a "virtual/forward" address, please override this.
			virtual address vaddr() const;

			// This specifies whether this 'player' object has a "virtual address".
			virtual bool hasVirtualAddress() const;
			virtual bool hasReliablePackets() const override;

			// Fields:

			// This represents the remote address of this player object.
			// This address is used by inheriting classes, such as
			// 'clientNetworkEngine', for other purposes.
			address remoteAddress;

			// A set of reliable packet-identifiers that have been confirmed.
			// This basically represents what's been sent over from a remote 'networkEngine'.
			set<packetID> confirmedPackets;

			// The name of this 'player' object.
			wstring name;

			connectionPing ping;
		};

		struct indirect_player : player
		{
			// Constructor(s):
			indirect_player(address representativeAddress = address(), address resolvedAddress = address(), bool calculateSnapshot = true);

			// Destructor(s):
			virtual ~indirect_player();

			// Methods:

			// This will supply 'realAddress' to those who
			// request the "virtual" address of this 'player'.
			virtual address vaddr() const override;

			virtual bool hasVirtualAddress() const override;

			// Fields:
			address realAddress;
		};
	}
}