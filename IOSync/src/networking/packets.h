#pragma once

// Includes:
#include "networking.h"
#include "address.h"

// Standard library:
#include <memory>
#include <chrono>

// Namespace(s):
using namespace std;
using namespace chrono;

namespace iosync
{
	namespace networking
	{
		// Forward declarations:

		// Structures:
		struct messageHeader;
		struct headerInfo;

		//struct messageFooter;

		struct player;

		// Classes:
		class networkEngine;

		//using rawPacket = uqchar*;
		typedef uqchar* rawPacket;

		// Functions:

		// Structures:

		// 'packets' are used for two main purposes; reliable message-handling, and message "routing".
		// This structure is used for representing an already formed packet, not creating packets.
		// A 'packet' object is meant to represent a packet's data and meta-data, until it is no longer needed.
		// In the case of reliable packets, this means packets (With, or without an internal buffer) can be used to represent a message.

		// Please see the derrived packet-structures for details.
		struct packet
		{
			// Typedefs:

			// We use standard shared-pointers due to the fact that 'packets' can be moved
			// around, and two 'packet' containers can reference the same memory.
			typedef shared_ptr<uqchar> sharedMemory;

			// Constructor(s):
			explicit packet(rawPacket rawData = (rawPacket)nullptr, size_t rawSize = 0, bool canFreeRawData = true);
			explicit packet(QSocket& socket, size_t readSize = 0);

			// Destructor(s):
			virtual ~packet();

			// This command will agnostically "free"/"discard" the 'data' buffer.
			// The return value indicates if the buffer was freed/deleted.
			// In the event this returns 'false', then the previously
			// referenced data can be assumed as externally managed.
			virtual bool freeData();

			// Methods:

			// This command marks a pointer as shared.
			// Do not use this unless you expect the data to be deleted at a later date.
			inline sharedMemory shareMemory(rawPacket memory)
			{
				shared = sharedMemory(memory);
				data = memory;

				return shared;
			}

			/*
				This method can be used to load all (Or part) of a 'socket' object's incoming data.
				This is useful for "routed" packets, which use the server
				(Or another client/node) to send messages to others.
				For normal message handling, this application uses QuickSock's internal storage and utilities.

				The 'simulatedRead' argument specifies that the read operation will use volatile data.
				This means the data is temporary, and will likely be corrupted.
				Use that option when you need temporary storage, but parody and modification are unneeded.
			*/

			virtual void readFrom(QSocket& socket, size_t readSize = 0, bool useExistingData = true, bool simulatedRead = false);

			/*
				This command can be used to retrieve a copy of a message's serialized form.
				This is useful for reliable packet-handling, where a message
				needs to be automatically sent several times under poor conditions.
				
				Simulated parodies should only be made when the integrity of the data is temporary.
				This should be avoided for most situations, but options are available to do this.

				If you wrote a footer, please set 'footerSize' to the 'serializedSize'
				field of that 'messageFooter' object. Not doing this could be problematic.
			*/

			void parodySerializedOutputMessage(QSocket& socket, const headerInfo positionInformation, packetSize_t footerSize=0, bool simulated=false);

			// This method provides a standard routine for writing a pre-saved packet onto the specified socket.
			// To automatically destroy the internal data of this packet after sending, you can enable 'destroyDataAfter'.
			// Writing to a 'socket' is not the same as sending. To send this packet, please use 'sendTo'.
			// External handling of 'packet' objects has no direct relation to the 'sendTo' command.
			bool writeTo(QSocket& socket, bool destroyDataAfter = false);

			// This method does the same thing as 'writeTo', then it sends the message using 'destinationAddress'.
			// DO NOT use this method, unless you are sending a completely raw packet.
			// In other words, this must only be used when the 'data' field points
			// to a raw-packet containing the message's data, a header, and optionally, a footer.
			packetSize_t sendTo(QSocket& socket, const address& destinationAddress, bool destroyDataAfter = false);

			// Operators:
			inline bool operator==(const packet& p) const
			{
				if (size != p.size)
					return false;

				return (data == p.data || (shared && shared == p.shared));
			}

			// Fields:

			// The size of this packet.
			size_t size;

			// A buffer containing the data of this packet.
			rawPacket data;

			// A managed pointer used to represent shared memory.
			// This is not always set, as memory could be directly handled by an external source.
			sharedMemory shared;
		};

		// 'outbound_packets' represent packets with described destinations.
		struct outbound_packet : packet
		{
			// Constructor(s):
			explicit outbound_packet(const address& remoteDestination, rawPacket rawData = (rawPacket)nullptr, packetID reliableIdentifier=PACKET_ID_UNRELIABLE, size_t rawSize = 0, bool canFreeRawData = true);

			explicit outbound_packet(networkDestinationCode destCode, rawPacket rawData = (rawPacket)nullptr, packetID reliableIdentifier=PACKET_ID_UNRELIABLE, size_t rawSize = 0, bool canFreeRawData = true);

			/*
				This overload will read from the socket specified.
				If you wish to hold a segment of the output-buffer,
				please use the other constructor, and 'parodySerializedOutputMessage' instead.
				The 'simulatedRead' argument is the same as the 'readFrom' command's version.
			*/
			explicit outbound_packet(QSocket& socket, size_t readSize = 0, packetID reliableIdentifier=PACKET_ID_UNRELIABLE, const address& destinationAddress=address(), bool simulatedRead=false);

			// Methods:

			// This overload provides an automated version of 'sendTo',
			// which uses the internal 'destination' address.
			packetSize_t sendTo(QSocket& socket, bool destroyDataAfter = false);

			// This allows you to send this packet using a 'networkEngine'.
			// This means the 'destinationCode' field may be used (If set by the user).
			packetSize_t sendTo(networkEngine& engine, QSocket& socket, bool destroyDataAfter = false);

			// This command will automatically change the 'destination'
			// address to the socket's message address, then send with it.
			// Ideally, you'll want to use 'sendTo', instead of this.
			packetSize_t autoSendTo(QSocket& socket, bool destroyDataAfter = false);

			// This will send a packet using the arguments specified. This will not copy the internal buffer into the 'socket' itself.
			// This is not guaranteed to use the 'engine' argument. However, it is guaranteed to use 'socket'.
			packetSize_t sendFor(networkEngine& engine, QSocket& socket);

			// This states if this packet is being sent to the address specified.
			bool isSendingTo(const address& addr) const;

			// This will update the time-snapshot used to represent when this packet was first sent.
			inline high_resolution_clock::time_point updateSnapshot()
			{
				// Update the connection-snapshot, then return it.
				return snapshot = high_resolution_clock::now();
			}

			// This will update the time-snapshot used to re-send unreceived packets.
			inline high_resolution_clock::time_point updateOutputSnapshot()
			{
				// Update the connection-snapshot, then return it.
				return resendSnapshot = high_resolution_clock::now();
			}

			inline void resetTimers()
			{
				updateSnapshot();
				updateOutputSnapshot();

				return;
			}

			// This command will tell you how long this packet has been in transit.
			inline milliseconds time() const
			{
				return elapsed(snapshot);
			}

			inline milliseconds resendTime() const
			{
				return elapsed(resendSnapshot);
			}

			// Operators:
			inline bool operator==(const outbound_packet& p) const
			{
				if (!packet::operator==(p))
					return false;

				if ((identifier != PACKET_ID_UNRELIABLE && identifier == p.identifier))
					return true;

				if (destination != p.destination)
					return false;

				if (destinationCode != p.destinationCode)
					return false;

				return (snapshot == p.snapshot);
			}

			// Fields:

			// The address this packet is going to.
			address destination;

			// Used when sending to a destination, without an explicit address.
			networkDestinationCode destinationCode;

			// A list of connections tied to this packet.
			playerList waitingConnections;

			// A "snapshot" of the time this packet was initially sent.
			high_resolution_clock::time_point snapshot;

			//  A "snapshot" of the time this packet was last sent.
			high_resolution_clock::time_point resendSnapshot;

			// The reliable identifier of this packet.
			packetID identifier;
		};
	}
}