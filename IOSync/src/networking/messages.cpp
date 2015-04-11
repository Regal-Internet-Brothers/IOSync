// Includes:
#include "messages.h"

// Namespace(s):
namespace iosync
{
	namespace networking
	{
		// Structures:

		// messageHeader:

		// Functions:
		// Nothing so far.

		// Constructor(s):
		messageHeader::messageHeader(messageType msgType, packetSize_t dataSize) : type(msgType), packetSize(dataSize), directedHere(true)
		{
			// Nothing so far.
		}

		messageHeader::messageHeader(messageType msgType, packetSize_t dataSize, QSocket& socket) : type(msgType), packetSize(dataSize), directedHere(true)
		{
			writeTo(socket);
		}

		messageHeader::messageHeader(messageType msgType, packetSize_t dataSize, QSocket& socket, headerInfo& info_out) : type(msgType), packetSize(dataSize), directedHere(true)
		{
			writeTo(socket, info_out);
		}

		// Methods:

		// I/O related:
		bool messageHeader::readFrom(QSocket& socket, messageFooter& footer)
		{
			// Local variable(s):
			bool hasFooter;

			// Read the type of message.
			type = socket.read<messageType>();

			hasFooter = socket.readBool();

			// Read the size of the size of the data-segment.
			packetSize = socket.read<packetSize_t>();

			if (hasFooter)
			{
				// Read the footer, then restore the stream-position.
				footer.readFrom(socket, socket.readOffset+packetSize, true);

				// Check if the packet is being forwarded.
				directedHere = !footer.forwardAddressSet();

				// Tell the user that we loaded a footer.
				return true;
			}

			// Tell the user that a footer was not found.
			return false;
		}

		void messageHeader::writeTo(QSocket& socket, headerInfo& info_out)
		{
			// Local variable(s):
			// Nothing so far.

			// Write the type of this message.
			socket.write<messageType>(type);

			// Retrieve the position where we'll write the footer's location.
			info_out.footerLocation_position = socket.writeOffset;

			// By default, messages to not have footers, to specify
			// one, please use the 'messageFooter' struct and/or
			// the 'networkEngine' class's "finishMessage" methods.
			socket.writeBool(false);

			// Retrieve the position where we'll write the
			// raw-data's size, then write the size in question:
			info_out.packetSize_position = socket.writeOffset;

			// This must be placed at the end of the serialized header.
			socket.write<packetSize_t>(packetSize);

			return;
		}

		// messageFooter:

		// Constructor(s):
		messageFooter::messageFooter(address forward, packetID identifier)
			: reliableIdentifier(identifier), forwardAddress(forward), serializedSize(0) { /* Nothing so far. */ }

		// The socket specified will be immediately read from.
		messageFooter::messageFooter(QSocket& socket)
		{
			// Read from the socket that was specified.
			readFrom(socket);
		}

		messageFooter::messageFooter(QSocket& socket, streamLocation position)
		{
			// Read from the socket specified, using the position specified.
			readFrom(socket);
		}

		// Methods:

		// I/O related:
		void messageFooter::readFrom(QSocket& socket, streamLocation position, bool restoreSeekPosition)
		{
			// Local variable(s):
			streamLocation readPosition;
			
			// Check if we need to log the read-offset:
			if (restoreSeekPosition)
			{
				// Store the current read-offset on the stack.
				readPosition = socket.readOffset;
			}

			// Seek to the beginning of the footer.
			socket.inSeek(position);

			// Call the main implementation.
			readFrom(socket);

			// Check if we should seek back to where we were:
			if (restoreSeekPosition)
			{
				// Seek to the location we stored on the stack.
				socket.inSeek(readPosition);
			}

			return;
		}

		void messageFooter::readFrom(QSocket& socket)
		{
			// Local variable(s):
			streamLocation startPosition;

			bool addressAvail;
			bool isReliable;

			// Store the start-position of the footer.
			startPosition = socket.readOffset;

			isReliable = socket.readBool();
			addressAvail = socket.readBool();

			// Check if this packet is reliable:
			if (isReliable)
			{
				// Read the specified "reliable-identifier".
				reliableIdentifier = socket.read<packetID>();
			}
			else
			{
				reliableIdentifier = PACKET_ID_UNRELIABLE;
			}

			// Check for a "forward address":
			if (addressAvail)
				forwardAddress.readFrom(socket);
			else
				forwardAddress = address();

			// Calculate the serialized size of the footer.
			serializedSize = (packetSize_t)(socket.readOffset-startPosition);

			return;
		}

		void messageFooter::writeTo(QSocket& socket, const headerInfo information, bool forceAddress)
		{
			// Update the header:

			// Update the projected "data-segment" size of the header:
			if (information.shouldUpdate_packetSize())
				messageHeader::markCurrentSize(socket, information.entryPoint+information.packetSize_position);

			// Update the serialized-header to reflect the existence of this footer:
			if (information.shouldUpdate_footerLocation())
				messageHeader::markBoolean(socket, information.entryPoint+information.footerLocation_position, true);

			// Local variable(s):
			bool isReliable = (reliableIdentifier != PACKET_ID_UNRELIABLE);
			bool addressAvail = (forceAddress || forwardAddress.isSet());

			// Write the packet-reliability state to the output.
			socket.writeBool(isReliable);
			
			// Write the address's availability-flag to the output.
			socket.writeBool(addressAvail);

			if (isReliable)
			{
				socket.write<packetID>(reliableIdentifier);
			}

			// Check if the "forward-address" has been set:
			if (addressAvail)
			{
				// Write 'forwardAddress' to the output.
				forwardAddress.writeTo(socket);
			}

			return;
		}
	}
}