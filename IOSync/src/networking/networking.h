#pragma once

// Preprocessor related:
#define NETWORKING_LOG_TO_CONSOLE

// Output-stream macros:
#ifdef NETWORKING_LOG_TO_CONSOLE
	#define networkLogStream std::cout
	#define wnetworkLogStream std::wcout

	#define networkLog std::cout << "{NETWORK}: "
	#define wnetworkLog std::wcout << L"{NETWORK}: "
#else
	#define networkLogStream std::clog
	#define wnetworkLogStream std::wclog

	#define networkLog std::clog << "{NETWORK}: "
	#define wnetworkLog std::wclog << L"{NETWORK}: "
#endif

// Includes:
#include "socket.h"
#include "../util.h"

// Standard library:
#include <climits>

#include <iostream>
#include <list>
#include <chrono>

// Namespaces:
using namespace std;
using namespace chrono;

namespace iosync
{
	namespace networking
	{
		using namespace quickLib::sockets;

		// Forward declarations:
		struct player;

		// Typedefs:
		typedef nativeIP addressIP;
		typedef nativePort addressPort;

		typedef unsigned short packetSize_t;
		typedef unsigned short messageType;
		typedef unsigned short packetID; // unsigned long long

		// The type used to deduce/describe the type of a connection.
		typedef unsigned char connectionType;

		typedef unsigned short disconnectionReason;

		typedef milliseconds connectionPing;

		typedef list<player*> playerList;

		// Enumerator(s):

		// Useful port-macros:
		enum ports : addressPort
		{
			PORT_AUTOMATIC = 0,

			DEFAULT_PORT = 5029, // 27015
			DEFAULT_LOCAL_PORT = PORT_AUTOMATIC,
		};

		// General purpose error-codes (Placeholder):
		enum errors : int
		{
			ERROR_HOSTING = -1,
			ERROR_CONNECTING = -1,
		};

		// Time-management macros:

		// These macros are measured in milliseconds:
		enum time : unsigned long long
		{
			DEFAULT_SERVER_CONNECTION_TIMEOUT = 3000, // 3 seconds.
			DEFAULT_SERVER_PING_INTERVAL = 1000, // 1 second.
			DEFAULT_SERVER_RELIABLE_RESEND = 50, // 200, // 0.2 seconds.

			DEFAULT_CLIENT_CONNECTION_TIMEOUT = 15000, // 15 seconds.
			DEFAULT_CLIENT_PING_INTERVAL = 4000, // 4 seconds.

			DEFAULT_CLIENT_RELIABLE_RESEND = 50, // 100, // 0.1 seconds.

			DEFAULT_CONNECTION_POLL_TIMEOUT = 100, // 0.1 seconds.
			DEFAULT_RELIABLE_PACKET_WAIT_TIME = 2000, // 2 seconds.
		};

		// These are "destination codes" used to send context-sensitive messages:
		enum networkDestinationCode : uqchar
		{
			// This allows clients to directly contact the server.
			DESTINATION_HOST = 0,

			// This may be used to automatically reply to the address a packet came from.
			DESTINATION_REPLY = 1,

			// This allows you to specify every other connection as a recipient. (You don't receive the message you sent)
			DESTINATION_ALL = 3,

			// This allows you to specify every connection as a recipient. (You get your own message back, as well)
			DESTINATION_EVERYONE = 4,

			DESTINATION_DIRECT = DESTINATION_REPLY,
			DEFAULT_DESTINATION = DESTINATION_REPLY, // DESTINATION_DIRECT
		};

		// Reserved/useful packet identifier-macros:
		enum packetIdentifiers : packetID
		{
			PACKET_ID_UNRELIABLE = 0,
			PACKET_ID_FIRST = 1,

			PACKET_ID_AUTOMATIC = PACKET_ID_UNRELIABLE,
		};

		// Reasons a client may be disconnected.
		enum disconnectionReasons : disconnectionReason
		{
			// A reason was not specified, or another unknown reason.
			DISCONNECTION_REASON_UNKNOWN = 0,

			// Custom disconnection-reasons should start at this point.
			DISCONNECTION_REASON_CUSTOM_LOCATION,
		};

		// Reserved/useful ping macros:
		enum connectionPings : unsigned short
		{
			PING_UNAVAILABLE = USHRT_MAX
		};
	}
}