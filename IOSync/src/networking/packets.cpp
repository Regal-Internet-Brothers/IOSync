// Includes:
#include "packets.h"
#include "messages.h"
#include "player.h"

#include "networkEngine.h"

// Namespace(s):
namespace iosync
{
	namespace networking
	{
		// Structures:

		// packet:

		// Constructor(s):
		packet::packet(rawPacket rawData, size_t rawSize, bool canFreeRawData) : size(rawSize)
		{
			if (rawData != rawPacket())
			{
				if (canFreeRawData)
					shareMemory(rawData);
				else
					data = rawData;
			}
		}

		packet::packet(QSocket& socket, size_t readSize)
		{
			readFrom(socket, readSize, false);
		}
		
		// Destructor(s):
		packet::~packet()
		{
			freeData();
		}

		bool packet::freeData()
		{
			// Reset the raw-data reference.
			data = rawPacket(); // nullptr;

			// Reset the shared representation of memory, if needed:
			if (shared)
			{
				// Reset shared memory.
				shared.reset();

				return true;
			}

			return false;
		}

		// Methods:
		void packet::readFrom(QSocket& socket, size_t readSize, bool useExistingData, bool simulatedRead)
		{
			// Check if we're able to fit the new data into our current buffer:
			if (useExistingData && shared && readSize > size)
				useExistingData = false;

			if (readSize == 0)
			{
				freeData();

				size = 0;

				return;
			}

			// Check if we should release the existing buffer:
			if (!useExistingData)
			{
				freeData();

				if (simulatedRead)
				{
					data = socket.simulatedUReadBytes(readSize);
				}
				else
				{
					shared = shareMemory(socket.UreadBytes(readSize));
				}
			}
			else
			{
				socket.UreadBytes(data, readSize);
			}

			size = readSize;

			return;
		}

		// This implementation relies on raw memory access. However, these accesses should be
		// considered safe (Assuming the input is correct). Extra error-checking is not done,
		// so caution should be taken when using this command. That being said, this should be reasonably fast.
		void packet::parodySerializedOutputMessage(QSocket& socket, const headerInfo positionInformation, packetSize_t footerSize, bool simulated)
		{
			// Free the existing data, if there is any.
			freeData();

			// Local variable(s):
			auto streamData = (rawPacket)(socket.outbuffer + positionInformation.entryPoint);

			// Calculate the internal-size of this packet.
			size = (socket.writeOffset-positionInformation.entryPoint);

			// Check for buffer "simulation":
			if (simulated)
			{
				this->data = streamData;

				return;
			}

			// Allocate a new chunk for the parody, then share it.
			if (!shareMemory(new uqchar[size]))
			{
				size = 0;

				return;
			}

			// Copy the contents of 'streamData' into our newly created buffer.
			memcpy(data, streamData, size);

			return;
		}

		bool packet::writeTo(QSocket& socket, bool destroyDataAfter)
		{
			if (data == nullptr)
				return false;

			// Write the saved data to 
			return socket.UwriteBytes(data, size);
		}

		packetSize_t packet::sendTo(QSocket& socket, address destinationAddress, bool destroyDataAfter)
		{
			writeTo(socket, destroyDataAfter);

			return destinationAddress.sendWith(socket);
		}

		// outbound_packet:
		
		// Constructor(s):
		outbound_packet::outbound_packet(address remoteDestination, rawPacket rawData, packetID reliableIdentifier, size_t rawSize, bool canFreeRawData)
			: packet(rawData, rawSize, canFreeRawData), identifier(reliableIdentifier), destination(remoteDestination), destinationCode(DESTINATION_DIRECT) { /* Nothing so far. */ }

		outbound_packet::outbound_packet(networkDestinationCode destCode, rawPacket rawData, packetID reliableIdentifier, size_t rawSize, bool canFreeRawData)
			: packet(rawData, rawSize, canFreeRawData), identifier(reliableIdentifier), destinationCode(destCode), destination() { /* Nothing so far. */ }

		outbound_packet::outbound_packet(QSocket& socket, size_t readSize, packetID reliableIdentifier, address destinationAddress, bool simulatedRead)
			: packet(socket, readSize), identifier(reliableIdentifier), destination(destinationAddress), destinationCode(DESTINATION_DIRECT)
		{
			readFrom(socket, readSize, false);
		}

		// Methods:
		packetSize_t outbound_packet::sendTo(QSocket& socket, bool destroyDataAfter)
		{
			return packet::sendTo(socket, destination, destroyDataAfter);
		}

		packetSize_t outbound_packet::sendTo(networkEngine& engine, QSocket& socket, bool destroyDataAfter)
		{
			packet::writeTo(socket, destroyDataAfter);

			return sendFor(engine, socket);
		}

		packetSize_t outbound_packet::autoSendTo(QSocket& socket, bool destroyDataAfter)
		{
			destination.IP = socket.msgIP();
			destination.port = socket.msgPort();

			return packet::sendTo(socket, destination, destroyDataAfter);
		}

		packetSize_t outbound_packet::sendFor(networkEngine& engine, QSocket& socket)
		{
			if (destinationCode == DESTINATION_DIRECT)
			{
				if (destination.isSet()) // (destination == socket)
				{
					return sendTo(socket);
				}
				
				return 0; // (packetSize_t)-1;
			}

			// Send using the internal destination-code.
			return (packetSize_t)engine.sendMessage(socket, destinationCode);
		}

		bool outbound_packet::isSendingTo(const address addr) const
		{
			return (addr == destination);
		}
	}
}