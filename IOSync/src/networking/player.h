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
			player(const address& remote = address(), bool calculateSnapshot = true);

			// Destructor(s):
			virtual ~player();

			// Methods:
			bool pruneReliablePacket(milliseconds requiredTime);

			void addReliablePacket(packetID ID);
			void pruneEarliestPacket();

			virtual void removeReliablePacket(packetID ID) override;
			virtual bool hasReliablePacket(packetID ID) const override;

			// This command outputs address-information to the 'os' stream.
			virtual void outputAddressInfo(ostream& os, bool endLine=true);

			// This will return a blank address if a "virtual address" could not be found.
			// This is implementation dependent, if you wish to
			// supply a "virtual/forward" address, please override this.
			virtual address vaddr() const;

			// This specifies whether this 'player' object has a "virtual address".
			virtual bool hasVirtualAddress() const;
			virtual bool hasReliablePackets() const override;

			// Operators:
			virtual bool operator==(const QSocket& socket) const;

			inline bool operator!=(const QSocket& addr) const
			{
				return !operator==(addr);
			}

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

			// Booleans / Flags:

			// This specifies if this connection is being "pinged".
			bool pinging = false;
		};

		struct indirect_player : player
		{
			// Constructor(s):
			indirect_player(const address& representativeAddress=address(), const address& resolvedAddress=address(), bool calculateSnapshot = true);

			// Destructor(s):
			virtual ~indirect_player();

			// Methods:
			virtual void outputAddressInfo(ostream& os, bool endLine=true) override;

			// This will supply 'realAddress' to those who
			// request the "virtual" address of this 'player'.
			virtual address vaddr() const override;

			virtual bool hasVirtualAddress() const override;

			// Operators:
			virtual bool operator==(const QSocket& socket) const override;

			// Fields:
			address realAddress;
		};
	}
}