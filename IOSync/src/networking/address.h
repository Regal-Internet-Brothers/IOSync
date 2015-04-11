#pragma once

// Includes:
#include "socket.h"
#include "networking.h"

// Used for I/O functionality for 'address' objects.
#include <iostream>
#include <string>

// Namespace(s):
using namespace std;

namespace iosync
{
	namespace networking
	{
		using namespace quickLib::sockets;

		// Forward declarations:
		struct player;

		// Constant variable(s):
		#ifdef QSOCK_IPVABSTRACT
			static const char* ADDRESS_SEPARATOR = "|";
		#else
			static const char* ADDRESS_SEPARATOR = ":";
		#endif

		// Structures:
		struct address
		{
			// Functions:

			// These commands tell you if an IP address has been "set".
			static bool addressSet(string IP);
			static bool addressSet(QSOCK_UINT32_LONG IP);

			// Constructor(s):
			address();
			address(addressIP address, addressPort remotePort);
			address(nonNativeIP address, addressPort remotePort);
			address(const QSocket& socket);
			address(const player* p);

			// Methods:
			bool isSet() const;

			inline packetSize_t sendWith(QSocket& socket)
			{
				return (packetSize_t)socket.sendMsg(IP, port);
			}

			// Network I/O:
			void readFrom(QSocket& socket);
			void writeTo(QSocket& socket);

			// Operators:
			inline bool operator==(const address* addr) const
			{
				return ((this->port == addr->port) && this->IP == addr->IP);
			}

			inline bool operator==(const address addr) const
			{
				return operator==(&addr);
			}

			inline bool operator==(const QSocket& socket) const
			{
				return operator==(address(socket));
			}

			inline bool operator!=(const address* addr) const
			{
				return !operator==(addr);
			}

			inline bool operator!=(const address addr) const
			{
				return operator!=(&addr);
			}

			inline bool operator!=(const QSocket& socket) const
			{
				return !(operator==(socket));
			}

			friend ostream& operator<<(ostream& os, const address& addr);

			// Fields:
			addressIP IP;
			addressPort port;
		};
	}
}