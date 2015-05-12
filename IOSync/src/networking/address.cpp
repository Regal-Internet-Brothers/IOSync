// Includes:
#include "networking.h"
#include "address.h"
#include "player.h"

// QuickLib:
#include <QuickLib/QuickINI/QuickINI.h>

// Standard library:
#include <iostream>
#include <string>

// Namespace(s):
using namespace std;

namespace iosync
{
	namespace networking
	{
		// Structures:

		// address:

		// Functions:
		bool address::addressSet(string IP)
		{
			return (!IP.empty());
		}

		bool address::addressSet(QSOCK_UINT32_LONG IP)
		{
			return (IP > 0);
		}

		// Constructor(s):
		address::address() : IP(), port() { /* Nothing so far. */ }
		address::address(addressIP address, addressPort remotePort) : IP(address), port(remotePort) { /* Nothing so far. */ }

		address::address(nonNativeIP address, addressPort remotePort) : port(remotePort)
		{
			IP = basic_socket::nonNativeToNativeIP(address);
		}

		address::address(const QSocket& socket) : address(socket.msgIP(), socket.msgPort())
		{
			// Nothing so far.
		}

		address::address(const player* p) : address(p->remoteAddress)
		{
			// Nothing so far.
		}

		// Methods:
		bool address::isSet() const
		{
			return (port > 0 || addressSet(IP));
		}

		void address::readFrom(QSocket& socket)
		{
			IP = socket.readIP();
			port = socket.readPort();

			return;
		}

		void address::writeTo(QSocket& socket)
		{
			socket.writeIP(IP);
			socket.writePort(port);

			return;
		}

		// Operators:
		ostream& operator<<(ostream& os, const address& addr)
		{
			// Output 'addr' to the 'os' output-stream.
			os << QSocket::representIP(addr.IP) << ADDRESS_SEPARATOR << addr.port;

			// Return the output-stream so it may be chained.
			return os;
		}

		// representativeAddress:

		// Constructor(s):
		representativeAddress::representativeAddress(string hostname, addressPort remotePort) : IP(hostname), port(remotePort)
		{
			// Nothing so far.
		}

		// Methods:
		bool representativeAddress::isSet() const
		{
			return (port > 0 || (!IP.empty()));
		}
	}
}