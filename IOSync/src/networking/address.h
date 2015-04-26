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
			static const char ADDRESS_SEPARATOR = '|';
		#else
			static const char ADDRESS_SEPARATOR = ':';
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

			// Other:

			// The return value of this command specifies if both a port, and an IP address/hostname was parsed.
			template <typename characterType=char, typename characterTraits=char_traits<characterType>, typename strAlloc=allocator<characterType>>
			inline bool parse(const basic_string<characterType, characterTraits, strAlloc>& input, addressPort default_port)
			{
				auto addressDivider = input.find(ADDRESS_SEPARATOR);

				// Ensure we have a separator.
				if (addressDivider != basic_string<characterType, characterTraits, strAlloc>::npos)
				{
					IP = QSocket::StringToIntIP(quickLib::INI::abstractStringToDefault(input.substr(0, addressDivider)));

					try
					{
						port = stoi(input.substr(addressDivider+1));

						if (port == 0)
							port = default_port;

						return true;
					}
					catch (invalid_argument&)
					{
						//port = 0;
					}
				}
				else
				{
					IP = QSocket::nonNativeToNativeIP(quickLib::INI::abstractStringToDefault(input));
				}

				port = default_port;

				// Return the default response.
				return false;
			}

			template <typename characterType=char, typename characterTraits=char_traits<characterType>, typename strAlloc=allocator<characterType>>
			void encodeTo(basic_string<characterType, characterTraits, strAlloc>& out_str) const
			{
				basic_stringstream<characterType, characterTraits, strAlloc> ss;
				basic_string<characterType, characterTraits, strAlloc> str;

				quickLib::INI::correctString(QSocket::representIP(IP) + ADDRESS_SEPARATOR, str);

				ss << str << port;

				out_str = ss.str();

				return;
			}

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

		struct representativeAddress
		{
			// Constructor(s):
			representativeAddress(string hostname="", addressPort remotePort=0);

			// Methods:
			bool isSet() const;

			// The return value of this command specifies if both a port, and an IP address/hostname was parsed.
			template <typename characterType=char, typename characterTraits=char_traits<characterType>, typename strAlloc=allocator<characterType>>
			inline bool parse(const basic_string<characterType, characterTraits, strAlloc>& input, addressPort default_port)
			{
				auto addressDivider = input.find(ADDRESS_SEPARATOR);

				// Ensure we have a separator.
				if (addressDivider != basic_string<characterType, characterTraits, strAlloc>::npos)
				{
					IP = quickLib::INI::abstractStringToDefault(input.substr(0, addressDivider));

					try
					{
						port = stoi(input.substr(addressDivider+1));

						if (port == 0)
							port = default_port;

						return true;
					}
					catch (invalid_argument&)
					{
						//port = 0;
					}
				}
				else
				{
					IP = quickLib::INI::abstractStringToDefault(input);
				}

				port = default_port;

				// Return the default response.
				return false;
			}

			template <typename characterType=char, typename characterTraits=char_traits<characterType>, typename strAlloc=allocator<characterType>>
			void encodeTo(basic_string<characterType, characterTraits, strAlloc>& out_str) const
			{
				basic_stringstream<characterType, characterTraits, strAlloc> ss;
				basic_string<characterType, characterTraits, strAlloc> str;

				quickLib::INI::correctString(IP, str);

				ss << str << ADDRESS_SEPARATOR << port;

				out_str = ss.str();

				return;
			}

			// Fields:
			string IP;
			addressPort port;
		};
	}
}