#pragma once

// Includes:
#include "networking.h"
#include "address.h"

// Namespace(s):
namespace iosync
{
	namespace networking
	{
		// Enumerator(s):
		enum footerLocations : streamLocation
		{
			FOOTER_LOCATION_NONE = 0,
		};

		// Structures:
		 
		// Forward declaration for 'messageHeader'.
		struct messageHeader;

		// This structure is dedicated to holding information about an unfinished header.
		struct headerInfo
		{
			// Functions:
			inline static bool positionSet(streamLocation position)
			{
				return (position == 0);
			}

			// Fields:

			// The "entry point" of the serialized header. (Commonly used for reliable messages)
			streamLocation entryPoint;

			// The position of the 'packetSize' entry in the serialized header.
			streamLocation packetSize_position;

			// The position of the "footer-location pointer" in the serialized header.
			streamLocation footerLocation_position;

			// Methods:
			inline bool shouldUpdate_packetSize() const
			{
				return !positionSet(packetSize_position);
			}

			inline bool shouldUpdate_footerLocation() const
			{
				return !positionSet(footerLocation_position);
			}
		};

		struct messageFooter
		{
			// Constructor(s):
			messageFooter(address forward = address(), packetID identifier = PACKET_ID_UNRELIABLE);
			messageFooter(QSocket& socket, streamLocation position);
			messageFooter(QSocket& socket);

			// Methods:
			inline bool isReliable() const
			{
				return (reliableIdentifier != PACKET_ID_UNRELIABLE);
			}

			// I/O related:
			void readFrom(QSocket& socket);
			void readFrom(QSocket& socket, streamLocation position, bool restoreSeekPosition=true);

			void writeTo(QSocket& socket, const headerInfo information, bool forceAddress = false);

			inline bool forwardAddressSet() const
			{
				return forwardAddress.isSet();
			}

			// Fields:
			packetID reliableIdentifier;

			// The "forward address" of a footer is the
			// address of the intended recipient.
			address forwardAddress;

			// The serialized size of this footer. This is useful when
			// passing invalid messages in a "real" packet.
			packetSize_t serializedSize;
		};

		struct messageHeader
		{
			// Functions:
			inline static packetSize_t getRelativeLocation(QSocket& socket, streamLocation origin)
			{
				return (socket.writeOffset-origin);
			}

			inline static bool markSize(QSocket& socket, streamLocation position, packetSize_t size)
			{
				// Store the current write-offset.
				auto offset = socket.writeOffset;

				// Seek to the position specified.
				socket.outSeek(position);
				
				// Write the packet-size to the output.
				auto response = socket.write<packetSize_t>(size);

				// Restore the original write-offset.
				socket.outSeek(offset);

				// Return the socket's response.
				return response;
			}

			// This command marks the 'location' value specified, at the stream-position specified.
			inline static bool markLocation(QSocket& socket, streamLocation position, streamLocation location)
			{
				// Store the current write-offset.
				auto offset = socket.writeOffset;

				// Seek to the position specified.
				socket.outSeek(position);
				
				// Write the packet-size to the output.
				auto response = socket.write<streamLocation>(location);

				// Restore the original write-offset.
				socket.outSeek(offset);

				// Return the socket's response.
				return response;
			}

			/*
				Like 'markSize', this will set the size of the packet.
				Unlike that command, however, this will use the current
				write-offset as the packet's size. This command should only
				be used when you're done writing the body of the message.

				The 'position' argument specifies where
				the write-operation should take place.
			*/

			inline static bool markCurrentSize(QSocket& socket, streamLocation sizePosition)
			{
				// This is offset so that the message-size doesn't reflect the storage for the size-entry.
				return markSize(socket, sizePosition, getRelativeLocation(socket, sizePosition+sizeof(packetSize_t)));
			}

			// This command will mark/write the current 
			inline static bool markCurrentLocation(QSocket& socket, streamLocation position)
			{
				return markLocation(socket, position, getRelativeLocation(socket, position));
			}

			inline static bool markBoolean(QSocket& socket, streamLocation position, bool value)
			{
				// Store the current write-offset.
				auto offset = socket.writeOffset;

				// Seek to the position specified.
				socket.outSeek(position);
				
				// Write the packet-size to the output.
				auto response = socket.writeBool(value);

				// Restore the original write-offset.
				socket.outSeek(offset);

				// Return the socket's response.
				return response;
			}

			// Constructor(s):
			messageHeader(messageType msgType = 0, packetSize_t dataSize = 0);

			// This constructor will immediately write to the socket specified.
			messageHeader(messageType msgType, packetSize_t dataSize, QSocket& socket);

			// This constructor will immediately write to the socket specified.
			messageHeader(messageType msgType, packetSize_t dataSize, QSocket& socket, headerInfo& info_out);

			// Methods:

			// I/O related:

			// This command will read the serialized header.
			// After reading the header, it may output a footer.
			// For this reason, you need to allocate a footer-object, then pass it in.
			// The return-value of this command indicates if a footer was found.
			bool readFrom(QSocket& socket, messageFooter& footer);

			/*
				This will write the serializable contents of this header.
				When finishing, this will produce a structure containing position-information
				for elements which can't be handled internally. Please finish writing the header.

				To finish writing the header, please use either 'markSize'
				(To manually finish the message), or use a 'messageFooter'.

				If a packet-size has already been specified
				(With the 'packetSize' field, or in 'information'),
				it will not be required to overwrite.
				
				By default, headers do not expect footers.
			*/

			void writeTo(QSocket& socket, headerInfo& information_out);

			// This overload will call the main implementation, then return a pre-allocated 'headerInfo' object.
			inline headerInfo writeTo(QSocket& socket)
			{
				headerInfo output = headerInfo();

				writeTo(socket, output);

				return output;
			}

			// Documentation is provided for the primary implementation.
			// This implementation simply uses the 'packetSize' field.
			// Do not use this if you do not intend to serialize the 'packetSize' field.
			inline bool markSize(QSocket& socket, streamLocation position)
			{
				return markSize(socket, position, this->packetSize);
			}

			// When manually finishing a message (Without a footer), please use this overload.
			inline bool markSize(QSocket& socket, const headerInfo info)
			{
				return markSize(socket, info.packetSize_position);
			}

			// Fields:
			packetSize_t packetSize;
			messageType type;

			// Booleans / Flags:

			// This is automatically handled by a footer.
			// If no footer was present, this will default to 'false'.
			bool directedHere;
		};
	}
}