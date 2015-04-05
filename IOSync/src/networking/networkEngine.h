#pragma once

// Includes:
#include "networking.h"

#include "address.h"
#include "reliablePacketManager.h"
#include "player.h"
#include "packets.h"
#include "messages.h"

// Standard library:
#include <list>
#include <string>

// Namespace(s):
using namespace std;

namespace iosync
{
	// Forward declarations:
	class application;

	namespace networking
	{
		// Constant variable(s):

		// Error/warning log headers:

		// The log-header displayed when a message is unsupported.
		static const char* UNSUPPORTED_MESSAGE = "Attempted to managed unsupported message: ";
		static const char* EXTRA_BYTES_DETECTED = "Extra bytes detected in message: ";
		static const char* UNABLE_TO_PARSE_MESSAGE = "Unable to parse incoming message: ";
		static const char* UNABLE_TO_FORWARD_PACKET = "Unable to forward packet: ";

		// Structures:

		// Standard date and time functionality used by 'networkEngines'.
		struct networkMetrics
		{
			// Constructor(s):
			networkMetrics
			(
				milliseconds poll,
				milliseconds connection,
				milliseconds reliableIDTime,
				milliseconds reliableResend,
				milliseconds ping = duration_cast<milliseconds>((seconds)1)
			);

			// Methods:
			// Nothing so far.

			// Fields:

			// The maximum number of milliseconds required to
			// wait when polling network information.
			milliseconds pollTimeout;

			// The maximum number of milliseconds a connection can take,
			// before the side requesting a "pong" message closes the connection.
			milliseconds connectionTimeout;

			// The number of milliseconds required to wait before pruning
			// the earliest reliable packet-identifier of a 'player' object.
			milliseconds reliablePruneTime;

			// The number of milliseconds reliable-packets need to wait before resending.
			milliseconds reliableResendTime;

			// The number of milliseconds required to wait before a ping message is sent.
			// Ideally, this would be a relatively long amount of time.
			milliseconds pingInterval;
		};

		// Classes:
		class networkEngine : public reliablePacketManager
		{
			public:
				// Enumerator(s):
				enum messageTypes : messageType
				{
					// This is used by clients when joining a server.
					MESSAGE_TYPE_JOIN,

					// This is used by clients when leaving a server.
					// In addition, this may also be used to formally
					// request that clients leave the server.
					MESSAGE_TYPE_LEAVE,

					// This is used to send a "ping" message.
					MESSAGE_TYPE_PING,

					// This is used to send a "pong" (Response to "ping") message.
					MESSAGE_TYPE_PONG,

					// This is used to confirm reliable packets.
					MESSAGE_TYPE_CONFIRM_PACKET,

					// Custom message-types should start at this location.
					MESSAGE_TYPE_CUSTOM_LOCATION,
				};

				enum reservedBytes : packetSize_t
				{
					//PING_MESSAGE_RESERVED_BYTES = 0,
					//PONG_MESSAGE_RESERVED_BYTES = PING_MESSAGE_RESERVED_BYTES,
				};

				enum disconnectionReasons : disconnectionReason
				{
					// The server/message-origin has forced you to disconnect.
					DISCONNECTION_REASON_FORCE = DISCONNECTION_REASON_CUSTOM_LOCATION,

					// This is used by clients to accept a disconnection gracefully.
					// Leave-messages made with this reason are not always received, and normally unneeded.
					DISCONNECTION_REASON_ACCEPT,

					// Used in messages sent as last-ditch efforts to formally disconnect a player.
					DISCONNECTION_REASON_TIMEDOUT,

					// This is used when the connection was manually closed.
					DISCONNECTION_REASON_CLOSE,
				};

				// Functions:
				// Nothing so far.

				// Constructor(s):
				networkEngine(const networkMetrics metrics);

				virtual bool open();

				// Destructor(s):
				virtual ~networkEngine();

				virtual bool close(application* program);

				// Methods:
				virtual void update(application* program);

				bool updateSocket(QSocket& socket);

				inline bool updateSocket()
				{
					return updateSocket(this->socket);
				}

				virtual void updatePacketsInTransit(QSocket& socket);

				inline void updatePacketsInTransit()
				{
					updatePacketsInTransit(this->socket);

					return;
				}

				packetID generateReliableID();

				headerInfo beginMessage(QSocket& socket, messageType msgType);

				void finishMessage(QSocket& socket, const headerInfo header_Information);
				messageFooter finishMessage(QSocket& socket, const headerInfo header_information, const address forwardAddress, const packetID ID=PACKET_ID_UNRELIABLE);

				inline void finishMessage(QSocket& socket, const headerInfo info, player* p)
				{
					if (p->hasVirtualAddress())
						finishMessage(socket, info, p->vaddr());
					else
						finishMessage(socket, info);

					return;
				}

				outbound_packet finishReliableMessage(QSocket& socket, const address realAddress, const headerInfo header_information, const address forwardAddress=address(), const packetID ID=PACKET_ID_AUTOMATIC);

				/*
					The 'header' and 'footer' arguments should be the message's current arguments.
					The read-position should also be after the header (Between the header and footer).
					Under normal circumstances, this will be the case.

					The "_out" arguments are used as pre-allocated output-variables for the new message.
					Those structures should not be pre-constructed if possible.

					If you do not wish to allocate these yourself, do not specify them.

					If this command returns 'false', then one or more operations could not be completed.
				*/

				bool rewriteMessage(QSocket& socket, const messageHeader& header, const messageFooter& footer, messageHeader& header_out, messageFooter& footer_out);

				// This overload provides pre-allocated header and footer objects.
				// The main arguments are the same; please follow the primary implementation's documentation.
				inline bool rewriteMessage(QSocket& socket, const messageHeader& header, const messageFooter& footer)
				{
					// Allocate the output-structures (Also useful for debugging):
					messageHeader header_out;
					messageFooter footer_out;

					// Call the main implementation, then return its response.
					return rewriteMessage(socket, header, footer, header_out, footer_out);
				}

				virtual size_t broadcastMessage(QSocket& socket, bool resetLength=true) = 0;

				// This is a general-purpose automatic send routine for sockets.
				// This is useful for situations where an address isn't needed.
				virtual size_t sendMessage(QSocket& socket, networkDestinationCode destination=DEFAULT_DESTINATION, bool resetLength=true);

				// This method may be overridden by inheriting classes.
				// For example, must specify extra information in order
				// to send messages to specific addresses.
				virtual size_t sendMessage(QSocket& socket, address remote, bool resetLength=true);
				virtual size_t sendMessage(networkDestinationCode destination=DEFAULT_DESTINATION, bool resetLength=true);

				// Reliable message related:

				// This command is called every time a reliable message is found.
				// If this returns 'false', the message will not be parsed, and it will be discarded.
				virtual bool onReliableMessage(QSocket& socket, address remoteAddress, const messageHeader& header, const messageFooter& footer, application* program);

				bool addReliablePacket(outbound_packet p);

				bool hasReliablePackets() const override;

				void removeReliablePacket(packetID ID) override;
				bool hasPacket(packetID ID) const override;

				// This method passes the message represented by the header specified.
				// This will only pass the data-segment of the message; the footer will not be passed.
				inline void passMessage(const messageHeader& header)
				{
					socket.inSeekForward(header.packetSize);

					return;
				}

				// This method passes the message represented by the header and footer specified.
				// This is used when unable to read a message.
				inline void passMessage(const messageHeader& header, const messageFooter& footer)
				{
					socket.inSeekForward(header.packetSize + footer.serializedSize);

					return;
				}

				// This will only pass the serialized size of the footer.
				inline void passFooter(const messageFooter& footer)
				{
					socket.inSeekForward(footer.serializedSize);

					return;
				}

				virtual bool onForwardPacket(QSocket& socket, streamLocation startPosition, address remoteAddress, const messageHeader& header, const messageFooter& footer, application* program);

				// The return value of this method indicates the number of messages that were received.
				size_t handleMessages(QSocket& socket, application* program);

				// Parsing/deserialization related:

				// When overriding this method, please "call up" to your super-class's implementation.
				// The order you do this in is up to you, but it is recommended that you do this first.
				virtual bool parseMessage(QSocket& socket, address remoteAddress, const messageHeader& header, const messageFooter& footer, application* program);

				// Player/connection management functionality:
				inline bool timedOut(milliseconds connectionTime) const
				{
					return (connectionTime >= metrics.connectionTimeout);
				}

				// Serialization related:
				void serializePlayerConnectionMessage(QSocket& socket, wstring name);
				void serializeLeaveNotice(QSocket& socket, disconnectionReason reason);
				void serializePacketConfirmationMessage(QSocket& socket, packetID ID);

				// Message generation:

				// This command will produce a "ping" message directed at 'realAddress'.
				inline outbound_packet generatePingMessage(QSocket& socket, const address realAddress=address(), address forwardAddress=address())
				{
					return finishReliableMessage(socket, realAddress, beginMessage(socket, MESSAGE_TYPE_PING), forwardAddress);
				}

				// This command will produce a "pong" message directed at 'realAddress'.
				inline outbound_packet generatePongMessage(QSocket& socket, const address realAddress=address(), address forwardAddress=address())
				{
					return finishReliableMessage(socket, realAddress, beginMessage(socket, MESSAGE_TYPE_PONG), forwardAddress);
				}

				inline outbound_packet generateLeaveNotice(QSocket& socket, disconnectionReason reason, const address realAddress = address(), const address forwardAddress = address())
				{
					auto info = beginMessage(socket, MESSAGE_TYPE_LEAVE);

					serializeLeaveNotice(socket, reason);

					return finishReliableMessage(socket, realAddress, info, forwardAddress);
				}

				inline outbound_packet generatePlayerConnectionMessage(QSocket& socket, wstring name, const address realAddress = address(), const address forwardAddress = address())
				{
					auto info = beginMessage(socket, MESSAGE_TYPE_JOIN);

					serializePlayerConnectionMessage(socket, name);

					return finishReliableMessage(socket, realAddress, info, forwardAddress);
				}

				inline void generatePacketConfirmationMessage(QSocket& socket, packetID ID)
				{
					auto info = beginMessage(socket, MESSAGE_TYPE_CONFIRM_PACKET);

					serializePacketConfirmationMessage(socket, ID);

					finishMessage(socket, info);

					return;
				}

				// Parsing/deserialization related:
				virtual disconnectionReason parseLeaveNotice(QSocket& socket, address remoteAddress, address forwardAddress=address());

				virtual packetID parsePacketConfirmationMessage(QSocket& socket, address remoteAddress, const messageHeader& header, const messageFooter& footer);

				// Sending related:
				inline size_t sendPing(QSocket& socket, networkDestinationCode destination=DEFAULT_DESTINATION, bool resetLength=true)
				{
					// Generate a message header for the output.
					generatePingMessage(socket);

					// Send the newly generated message.
					return networkEngine::sendMessage(socket, destination, resetLength);
				}

				inline size_t sendPong(QSocket& socket, networkDestinationCode destination=DEFAULT_DESTINATION, bool resetLength=true)
				{
					// Generate a message header for the output.
					generatePongMessage(socket);

					// Send the newly generated message.
					return networkEngine::sendMessage(socket, destination, resetLength);
				}

				virtual void pingRemoteConnection(QSocket& socket) = 0;

				// Operators:

				// This "engine" may be used in place of a 'QSocket'.
				inline operator QSocket&()
				{
					return socket;
				}

				// Fields (Public):

				// The primary socket of this "engine".
				QSocket socket;
			protected:
				// Fields (Protected):

				// Standard network time-metrics.
				networkMetrics metrics;

				// A list of outbound packets in transit.
				list<outbound_packet> packetsInTransit;

				// The next 'packetID' used for reliable packet-handling.
				packetID nextReliableID;

				// Booleans / Flags:

				// This variable describes if this "engine" is able to act as a "node".
				// This also changes for the real host of the session.
				bool isHostNode;

				// This field specifies if this "engine" is the "master server".
				bool isMaster;
		};

		class clientNetworkEngine : public networkEngine
		{
			public:
				// Constructor(s):
				clientNetworkEngine
				(
					wstring username,

					const networkMetrics metrics = networkMetrics
					(
						(milliseconds)DEFAULT_CONNECTION_POLL_TIMEOUT,
						(milliseconds)DEFAULT_CLIENT_CONNECTION_TIMEOUT,
						(milliseconds)DEFAULT_RELIABLE_PACKET_WAIT_TIME,
						(milliseconds)DEFAULT_CLIENT_RELIABLE_RESEND,
						(milliseconds)DEFAULT_CLIENT_PING_INTERVAL
					)
				);

				bool open(string remoteAddress, addressPort remotePort=DEFAULT_PORT, addressPort localPort=DEFAULT_LOCAL_PORT); // address remoteAddress

				// Destructor(s):
				virtual bool close(application* program) override;

				// Methods:
				virtual void update(application* program) override;

				high_resolution_clock::time_point updateSnapshot() override;

				virtual void updatePacketsInTransit(QSocket& socket) override;

				virtual bool onForwardPacket(QSocket& socket, streamLocation startPosition, address remoteAddress, const messageHeader& header, const messageFooter& footer, application* program) override;

				// Parsing/deserialization related:

				// When calling up to this implementation, it is best to ensure a connection has been properly made beforehand.
				virtual bool parseMessage(QSocket& socket, address remoteAddress, const messageHeader& header, const messageFooter& footer, application* program) override;

				inline outbound_packet finishReliableMessage(QSocket& socket, const address realAddress, const headerInfo header_information, const player* p, packetID ID = PACKET_ID_AUTOMATIC)
				{
					return networkEngine::finishReliableMessage(socket, realAddress, header_information, p->vaddr(), ID);
				}

				size_t broadcastMessage(QSocket& socket, bool resetLength=true) override;
				size_t sendMessage(QSocket& socket, address remote, bool resetLength=true) override;

				inline bool timedOut() const
				{
					return networkEngine::timedOut(connection.connectionTime());
				}

				// Reliable message related:
				virtual bool onReliableMessage(QSocket& socket, address remoteAddress, const messageHeader& header, const messageFooter& footer, application* program) override;

				// Simple messages:
				virtual void pingRemoteConnection(QSocket& socket) override;

				inline size_t sendLeaveNotice(QSocket& socket, disconnectionReason reason=DISCONNECTION_REASON_UNKNOWN, networkDestinationCode destination=DEFAULT_DESTINATION, bool resetLength=true)
				{
					generateLeaveNotice(socket, reason);

					return networkEngine::sendMessage(socket, destination, resetLength);
				}

				inline size_t sendCourtesyLeaveConfirmation(QSocket& socket, networkDestinationCode destination=DEFAULT_DESTINATION)
				{
					// Write a one-off message to accept the disconnection.
					auto info = beginMessage(socket, MESSAGE_TYPE_LEAVE);

					serializeLeaveNotice(socket, DISCONNECTION_REASON_ACCEPT);

					finishMessage(socket, info);

					return networkEngine::sendMessage(socket, destination);
				}

				inline size_t sendPlayerConnectionMessage(QSocket& socket, wstring playerName=L"Player", networkDestinationCode destination=DEFAULT_DESTINATION, bool resetLength=true)
				{
					generatePlayerConnectionMessage(socket, playerName);

					return networkEngine::sendMessage(socket, destination, resetLength);
				}

				inline size_t sendPlayerConnectionMessage(QSocket& socket, wstring playerName, address forwardAddress, networkDestinationCode destination=DEFAULT_DESTINATION, bool resetLength=true)
				{
					generatePlayerConnectionMessage(socket, playerName, socket, forwardAddress);

					return networkEngine::sendMessage(socket, destination, resetLength);
				}

				// Message generation:
				inline outbound_packet generateLeaveNotice(QSocket& socket, disconnectionReason reason, const address forwardAddress = address())
				{
					return networkEngine::generateLeaveNotice(socket, reason, connection.remoteAddress, forwardAddress);
				}

				// Serialization related:
				// Nothing so far.

				// Parsing/deserialization related:
				virtual disconnectionReason parseLeaveNotice(QSocket& socket, address remoteAddress, address forwardAddress=address()) override;

				// Operators:
				inline operator player&()
				{
					return connection;
				}

				// Fields:
				player connection;

				/*
					This represents the resolved host address. In the event
					of a direct connection, this will be the same as 'remote'.
					This is used to represent the master/origin server.

					This address may be messaged directly with no problems from previous parent-nodes.
					Parent-nodes have the option to query either the master/origin server, or the client itself.
					
					An improper disconnection from a non-esential node is unlikely, but it can happen.
					If such events arise, the 'master' address will be relied upon.

					Parent-nodes are assumed to be represented with 'master', until it can be evaluated.
				*/

				address master;

				// Booleans / Flags:
				bool connected;
		};

		class serverNetworkEngine : public networkEngine
		{
			public:
				// Constructor(s):
				serverNetworkEngine
				(
					const networkMetrics metrics = networkMetrics
					(
						(milliseconds)DEFAULT_CONNECTION_POLL_TIMEOUT,
						(milliseconds)DEFAULT_SERVER_CONNECTION_TIMEOUT,
						(milliseconds)DEFAULT_RELIABLE_PACKET_WAIT_TIME,
						(milliseconds)DEFAULT_SERVER_RELIABLE_RESEND,
						(milliseconds)DEFAULT_SERVER_PING_INTERVAL
					)
				);

				// Destructor(s):
				virtual bool close(application* program) override;

				bool open(addressPort port=DEFAULT_PORT);

				// Methods:
				virtual void update(application* program) override;

				virtual void updatePacketsInTransit(QSocket& socket) override;

				void checkClientTimeouts(QSocket& socket);

				inline void checkClientTimeouts()
				{
					return checkClientTimeouts(this->socket);
				}

				size_t broadcastMessage(QSocket& socket, bool resetLength=true) override;

				inline size_t sendMessageTo(QSocket& socket, player* p)
				{
					return sendMessage(socket, p->remoteAddress);
				}

				// Reliable message related:
				virtual bool onReliableMessage(QSocket& socket, address remoteAddress, const messageHeader& header, const messageFooter& footer, application* program) override;

				virtual bool onForwardPacket(QSocket& socket, streamLocation startPosition, address remoteAddress, const messageHeader& header, const messageFooter& footer, application* program) override;

				// Parsing/deserialization related:
				virtual bool parseMessage(QSocket& socket, address remoteAddress, const messageHeader& header, const messageFooter& footer, application* program) override;

				// Serialization related:
				// Nothing so far.

				// Message generation:

				// This sends a blank 'MESSAGE_TYPE_JOIN' message reliably to the address specified.
				// This is used to notify a player that their connection request has been accepted.
				inline outbound_packet generatePlayerConfirmationMessage(QSocket& socket, const address realAddress = address(), const address forwardAddress = address())
				{
					return finishReliableMessage(socket, realAddress, beginMessage(socket, MESSAGE_TYPE_JOIN), forwardAddress);
				}

				inline outbound_packet generateLeaveNotice(QSocket& socket, disconnectionReason reason, player* p)
				{
					return networkEngine::generateLeaveNotice(socket, reason, p->remoteAddress, p->vaddr());
				}

				// Parsing/deserialization related:

				// The return value of this command specifies if the 'player' object was an "indirect" player or not.
				bool parsePlayerConnectionMessage(QSocket& socket, address remoteAddress, const messageHeader& header, const messageFooter& footer);

				virtual disconnectionReason parseLeaveNotice(QSocket& socket, address remoteAddress, address forwardAddress=address()) override;

				// Simple messages:
				virtual void pingRemoteConnection(QSocket& socket) override;

				// This simply sets a different default argument for this method.
				inline size_t sendPing(QSocket& socket, networkDestinationCode destination = DESTINATION_ALL, bool resetLength = true)
				{
					return networkEngine::sendPing(socket, destination, resetLength);
				}

				// Player/connection management functionality:
				inline bool timedOut(player* p)
				{
					// Call the super-class's implementation.
					return networkEngine::timedOut(p->connectionTime());
				}

				// This command allows you to retrieve a player-entry from the 'players' list.
				inline player* getPlayer(address addr, address vaddr = address())
				{
					for (auto p : players)
					{
						if (p->remoteAddress == addr && p->vaddr() == vaddr)
							return p;
					}

					return nullptr;
				}

				inline bool playerJoined(address addr)
				{
					return (getPlayer(addr) != nullptr);
				}

				// The return values of these commands indicate if they were successful:
				bool connectPlayer(QSocket& socket, player* p);
				bool disconnectPlayer(QSocket& socket, player* p, disconnectionReason reason=DISCONNECTION_REASON_ACCEPT);
				void forceDisconnectPlayer(QSocket& socket, player* p, disconnectionReason reason=DISCONNECTION_REASON_FORCE, bool reliable=false, bool autoRemove=true);

				// This will force-disconnect all connected players.
				void forceDisconnectPlayers(QSocket& socket, disconnectionReason reason=DISCONNECTION_REASON_FORCE, bool reliable=false);

				// This will formally disconnect all connected players.
				void disconnectPlayers(QSocket& socket, bool reliable=true);

				inline void disconnectPlayers()
				{
					disconnectPlayers(this->socket);

					return;
				}

				/*
				inline bool connectPlayer(player* p)
				{
					return connectPlayer(this->socket, p);
				}

				inline bool disconnectPlayer(player* p)
				{
					return disconnectPlayer(this->socket, p);
				}
				*/

				inline void addPlayer(player* p)
				{
					players.push_back(p);

					return;
				}

				inline void removePlayer(player* p, bool autoRemove=true)
				{
					if (autoRemove)
						players.remove(p);

					if (getPlayer(p->remoteAddress) == nullptr)
					{
						auto op = packetsInTransit.begin();

						while (op != packetsInTransit.end())
						{
							if ((*op).isSendingTo(p))
							{
								packetsInTransit.erase(op++);

								continue;
							}

							op++;
						}
					}

					return;
				}

				// Fields:
				list<player*> players;
		};
	}
}