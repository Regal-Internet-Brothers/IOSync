#pragma once

// Preprocessor related:

// Includes:
#include "socket.h"

// Standard library:
#include <climits>

#include <chrono>

// Namespaces:
using namespace std;
using namespace chrono;

namespace iosync
{
	namespace networking
	{
		using namespace quickLib::sockets;

		// Typedefs:
		typedef nativeIP addressIP;
		typedef nativePort addressPort;

		typedef unsigned short packetSize_t;
		typedef unsigned short messageType;
		typedef unsigned short packetID; // unsigned long long

		typedef unsigned short disconnectionReason;

		typedef milliseconds connectionPing;

		/*
		// Structures:
		struct player;
		struct address;

		// Message types:
		struct messageHeader;
		struct messageFooter;

		// Packet types:
		struct packet;

		struct outbound_packet;
		struct symbolic_packet;

		// Networking time-information.
		struct networkMetrics;

		// Classes:

		// Internal:
		class networkEngine;
		class clientNetworkEngine;
		class serverNetworkEngine;
		*/

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
			DEFAULT_SERVER_RELIABLE_RESEND = 200, // 0.2 seconds.

			DEFAULT_CLIENT_CONNECTION_TIMEOUT = 15000, // 15 seconds.
			DEFAULT_CLIENT_PING_INTERVAL = 8000, // 8 seconds.
			DEFAULT_CLIENT_RELIABLE_RESEND = 100, // 0.1 seconds.

			DEFAULT_CONNECTION_POLL_TIMEOUT = 64, // 0.1 seconds.
			DEFAULT_RELIABLE_PACKET_WAIT_TIME = 2000, // 2 seconds.
		};

		// These are "destination codes" used to send context-sensitive messages:
		enum networkDestinationCode : uqchar
		{
			DESTINATION_HOST = 0,
			DESTINATION_REPLY = 1,
			DESTINATION_DIRECT = 2,
			DESTINATION_ALL = 3,

			DEFAULT_DESTINATION = DESTINATION_REPLY,
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

		// Functions:

		// This command returns the number of milliseconds that have passed since 't' was updated last.
		inline static milliseconds elapsed(high_resolution_clock::time_point t)
		{
			return duration_cast<milliseconds>(high_resolution_clock::now() - t);
		}
	}
}