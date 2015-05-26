// Includes:
#include "reliablePacketManager.h"
#include "player.h"
#include "networkEngine.h"
#include "packets.h"

#include "../application/application.h"

// Standard library:
//#include <iostream>

//#include <queue>

// Namespace(s):
using namespace std;

namespace iosync
{
	namespace networking
	{
		// Functions:
		// Nothing so far.

		// Structures:

		// reliablePacketManager:

		// Constructor(s):
		reliablePacketManager::reliablePacketManager(bool calculateSnapshot)
		{
			if (calculateSnapshot)
				updateSnapshot();
		}

		reliablePacketManager::~reliablePacketManager()
		{
			// Nothing so far.
		}

		// Methods:
		high_resolution_clock::time_point reliablePacketManager::updateSnapshot()
		{
			// Update the connection-snapshot, then return it.
			return (connectionSnapshot = high_resolution_clock::now());
		}

		// player:

		// Constructor(s):
		player::player(const address& remote, bool calculateSnapshot)
			: reliablePacketManager(calculateSnapshot), remoteAddress(remote), confirmedPackets(), ping((connectionPing)PING_UNAVAILABLE) { /* Nothing so far. */ }

		// Destructor(s):
		player::~player()
		{
			// Nothing so far.
		}

		// Methods:
		address player::vaddr() const
		{
			return address();
		}

		bool player::hasVirtualAddress() const
		{
			return false;
		}

		bool player::hasReliablePackets() const
		{
			return (!confirmedPackets.empty());
		}

		void player::pruneEarliestPacket()
		{
			updateConfirmedPacketTimer();

			if (confirmedPackets.empty())
				return;

			confirmedPackets.erase(confirmedPackets.begin());

			return;
		}

		bool player::pruneReliablePacket(milliseconds requiredTime)
		{
			if (hasReliablePackets() && confirmedPacketTime() >= requiredTime)
			{
				// Remove the earliest reliable packet-ID.
				pruneEarliestPacket();

				// Tell the user that a packet was removed.
				return true;
			}

			// Return the default response. (Nothing was removed)
			return false;
		}

		void player::addReliablePacket(packetID ID)
		{
			if (confirmedPackets.empty())
				updateConfirmedPacketTimer();

			confirmedPackets.insert(ID);

			return;
		}
		
		void player::removeReliablePacket(packetID ID)
		{
			confirmedPackets.erase(ID);

			return;
		}

		bool player::hasReliablePacket(packetID ID) const
		{
			return (confirmedPackets.find(ID) != confirmedPackets.end());
		}

		void player::outputAddressInfo(ostream& os, bool endLine)
		{
			os << remoteAddress;

			if (endLine)
				os << endl;

			return;
		}

		// Operators:
		bool player::operator==(const address& addr) const
		{
			return (remoteAddress == addr);
		}

		// indirect_player:

		// Constructor(s):
		indirect_player::indirect_player(const address& representativeAddress, const address& resolvedAddress, bool calculateSnapshot)
			: player(representativeAddress, calculateSnapshot), realAddress(resolvedAddress) { /* Nothing so far. */ }

		// Destructor(s):
		indirect_player::~indirect_player()
		{
			// Nothing so far.
		}

		// Methods:
		void indirect_player::outputAddressInfo(ostream& os, bool endLine)
		{
			player::outputAddressInfo(os, false);

			if (realAddress.isSet())
			{
				os << ", " << realAddress;
			}

			if (endLine)
				os << endl;

			return;
		}

		address indirect_player::vaddr() const
		{
			return realAddress;
		}

		bool indirect_player::hasVirtualAddress() const
		{
			return (realAddress.isSet()); // true;
		}

		// Operators:
		bool indirect_player::operator==(const address& addr) const
		{
			return (player::operator==(addr) || (realAddress == addr));
		}

		// Classes:

		// networkMetrics:

		// Constructor(s):
		networkMetrics::networkMetrics(milliseconds poll, milliseconds connection, milliseconds reliableIDTime, milliseconds reliableResend, milliseconds ping)
			: pollTimeout(poll), connectionTimeout(connection), reliablePruneTime(reliableIDTime), reliableResendTime(reliableResend), pingInterval(ping) { /* Nothing so far. */ }

		// networkEngine:

		// Functions:
		// Nothing so far.

		// Constructor(s):
		networkEngine::networkEngine(application& parent, const networkMetrics netMetrics)
			: parentProgram(parent), metrics(netMetrics), isHostNode(false), isMaster(false), nextReliableID(PACKET_ID_FIRST) { /* Nothing so far. */ }

		bool networkEngine::open()
		{
			updateSnapshot();

			// Return the default response.
			return true;
		}

		// Destructor(s):
		bool networkEngine::close()
		{
			// Close the internal socket.
			socket.close();

			// Clear the list of packets in transit.
			packetsInTransit.clear();

			// Return the default response.
			return true;
		}

		// Methods (Public):

		// Update routines:
		void networkEngine::update()
		{
			if (connectedToOthers())
			{
				if (connectionTime() >= metrics.pingInterval)
				{
					pingRemoteConnection(socket);

					// Reset/update the main connection-snapshot.
					reliablePacketManager::updateSnapshot();
				}
			}

			updatePacketsInTransit();

			handleMessages(this->socket);

			return;
		}

		bool networkEngine::updateSocket(QSocket& socket)
		{
			auto received = socket.update();

			return (socket.msgAvail());
		}

		void networkEngine::updatePacketsInTransit(QSocket& socket)
		{
			list<outbound_packet>::iterator packetIterator = packetsInTransit.begin();

			while (packetIterator != packetsInTransit.end())
			{
				// Used for the sake of reducing boilerplate.
				auto& packetInTransit = *packetIterator;

				if (packetInTransit.time() > metrics.reliablePruneTime)
				{
					packetsInTransit.erase(packetIterator++);

					continue;
				}

				if (packetInTransit.resendTime() >= metrics.reliableResendTime)
				{
					packetInTransit.updateOutputSnapshot();

					packetInTransit.sendTo(*this, socket, false);
				}

				packetIterator++;
			}

			return;
		}

		packetID networkEngine::generateReliableID()
		{
			// Get the next ID.
			auto ID = nextReliableID;

			// Add to the reliable-identifier counter.
			nextReliableID++;

			// Return the next ID.
			return ID;
		}

		headerInfo networkEngine::beginMessage(QSocket& socket, messageType msgType)
		{
			return messageHeader(msgType).writeTo(socket);
		}

		void networkEngine::finishMessage(QSocket& socket, const headerInfo header_Information)
		{
			messageHeader::markCurrentSize(socket, header_Information.packetSize_position);

			return;
		}

		messageFooter networkEngine::finishMessage(QSocket& socket, const headerInfo header_information, const address& forwardAddress, const packetID ID)
		{
			messageFooter footer(forwardAddress, ID);

			footer.writeTo(socket, header_information);

			return footer;
		}

		outbound_packet networkEngine::finishReliableMessage(QSocket& socket, const address& realAddress, const headerInfo header_Information, const address& forwardAddress, const packetID ID)
		{
			// Local variable(s):
			auto identifier = (ID != PACKET_ID_AUTOMATIC) ? ID : generateReliableID();

			auto footer = finishMessage(socket, header_Information, forwardAddress, identifier);

			outbound_packet output((realAddress.isSet()) ? realAddress : address(socket), nullptr, identifier);
			output.parodySerializedOutputMessage(socket, header_Information, footer.serializedSize, false);

			return output;
		}

		bool networkEngine::rewriteMessage(QSocket& socket, const messageHeader& header, const messageFooter& footer, messageHeader& header_out, messageFooter& footer_out)
		{
			// Read the message's data from the socket. (Simulated)
			outbound_packet messageData(socket, header.packetSize, generateReliableID(), footer.forwardAddress, true);

			// Check for errors:
			if (messageData.data == nullptr)
				return false;

			// Copy the contents of the message's header and footer:
			header_out = header;
			footer_out = footer;

			// Prepare the output-header:
			header_out.directedHere = true;

			// Prepare the output-footer:

			//footer_out.reliableIdentifier = footer.reliableIdentifier;

			footer_out.forwardAddress = address();
			footer_out.serializedSize = 0;

			// Allocate an intermediate header-information structure.
			headerInfo intermediateInformation;

			// Write the new message:
			header_out.writeTo(socket, intermediateInformation);

			messageData.writeTo(socket, false);

			footer_out.writeTo(socket, intermediateInformation);

			// Return the default response.
			return true;
		}

		size_t networkEngine::sendMessage(QSocket& socket, networkDestinationCode destination, bool resetLength)
		{
			switch (destination)
			{
				case DESTINATION_HOST:
					// Check if this is the master, otherwise,
					// continue on to the "reply" implementation:
					if (isMaster)
					{
						if (resetLength)
						{
							socket.flushOutput();
						}

						break;
					}
				case DESTINATION_REPLY:
					finalizeOutput(socket, destination);

					return socket.sendMsg(resetLength);
				case DESTINATION_ALL:
				case DESTINATION_EVERYONE:
					return broadcastMessage(socket, destination, resetLength);
			}

			return (size_t)SOCKET_ERROR;
		}

		size_t networkEngine::sendMessage(QSocket& socket, outbound_packet packet, bool alreadyInOutput)
		{
			// Add the specified packet to the reliable-packet container.
			addReliablePacket(packet);

			if (!alreadyInOutput)
			{
				socket.flushOutput();

				return packet.sendTo(*this, socket);
			}
			
			return packet.sendFor(*this, socket);
		}

		size_t networkEngine::sendMessage(QSocket& socket, const address& remote, bool resetLength, networkDestinationCode destinationCode)
		{
			finalizeOutput(socket, destinationCode);

			return (size_t)socket.sendMsg(remote.IP, remote.port, resetLength);
		}

		size_t networkEngine::sendMessage(networkDestinationCode destination, bool resetLength)
		{
			return sendMessage(this->socket, destination, resetLength);
		}

		bool networkEngine::hasRemoteConnection() const
		{
			// Return the default response.
			return true;
		}

		bool networkEngine::canBroadcastLocally() const
		{
			// Return the default response.
			return false;
		}

		player* networkEngine::getPlayer(QSocket& socket)
		{
			return nullptr;
		}

		// Reliable message related:
		bool networkEngine::onReliableMessage(QSocket& socket, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Tell the user that reliable messages aren't supported.
			return false;
		}

		bool networkEngine::addReliablePacket(outbound_packet p)
		{
			// Reset the timers of this packet.
			p.resetTimers();

			// Add the packet specified into
			packetsInTransit.push_back(p);

			// Return the default response.
			return true;
		}

		bool networkEngine::hasReliablePackets() const
		{
			return !packetsInTransit.empty();
		}

		void networkEngine::removeReliablePacket(packetID ID)
		{
			for (auto& packetInTransit : packetsInTransit)
			{
				if (packetInTransit.identifier == ID)
				{
					packetsInTransit.remove(packetInTransit);

					break;
				}
			}

			return;
		}

		bool networkEngine::hasReliablePacket(packetID ID) const
		{
			for (auto& packetInTransit : packetsInTransit)
			{
				if (packetInTransit.identifier == ID)
				{
					return true;
				}
			}

			// Return the default response.
			return false;
		}

		bool networkEngine::removeReliablePacket(const address& remoteAddress, packetID ID)
		{
			for (auto& packetInTransit : packetsInTransit)
			{
				if (packetInTransit.identifier == ID)
				{
					if (packetInTransit.waitingConnections.empty())
					{
						removeReliablePacket(ID);

						return true;
					}
					else
					{
						// Retrieve the corrisponding player-object to the 'remoteAddress' argument.
						auto p = getPlayer(packetInTransit.waitingConnections, remoteAddress);

						if (p != nullptr)
						{
							return removeReliablePacket(p, packetInTransit);
						}

						break;
					}
				}
			}

			// Return the default response.
			return false;
		}

		bool networkEngine::removeReliablePacket(player* p, outbound_packet& packetInTransit)
		{
			if (!packetInTransit.waitingConnections.empty())
			{
				packetInTransit.waitingConnections.remove(p);

				// Check if this container still isn't empty:
				if (!packetInTransit.waitingConnections.empty())
				{
					// Tell the user we aren't done yet.
					return false;
				}
			}

			packetsInTransit.remove(packetInTransit);

			// Tell the user we found the packet, and successfully removed it.
			return true;
		}

		size_t networkEngine::handleMessages(QSocket& socket)
		{
			// Local variable(s):
			size_t messages = 0;

			// This will act as our standard poll-timer.
			high_resolution_clock::time_point timer = high_resolution_clock::now();

			// Check for incoming messages:
			while (updateSocket(socket))
			{
				#ifdef QSOCK_TESTMODE
					streamLocation initialPosition = socket.readOffset;
				#endif

				while (socket.canRead())
				{
					// Local variable(s):
					streamLocation startPosition = socket.readOffset;

					messageHeader header;
					messageFooter footer;

					header.readFrom(socket, footer);

					if (header.directedHere)
					{
						if (footer.isReliable())
						{
							if (!onReliableMessage(socket, address(socket), header, footer))
							{
								// Reliable messages aren't supported, or this message
								// has already been received, skip this message:
								passMessage(header, footer);

								// Continue to the next message.
								continue;
							}
						}

						// Store the current read-position.
						auto parsePosition = socket.readOffset;

						// Attempt to parse the message:
						if (!parseMessage(socket, address(socket), header, footer))
						{
							// Move back to the beginning of the message.
							socket.inSeek(parsePosition);

							// Report to the standard log.
							clog << UNABLE_TO_PARSE_MESSAGE << header.type << endl;

							// We were unable to parse this message, skip it:
							passMessage(header, footer);

							// Continue to the next message.
							continue;
						}

						// Add to the message-counter.
						messages += 1;

						// Calculate the number of bytes read while parsing.
						auto bytesRead = socket.readOffset-parsePosition;

						if (bytesRead < header.packetSize)
						{
							clog << EXTRA_BYTES_DETECTED << bytesRead << "/" << header.packetSize;
							clog << " (" << (header.packetSize-bytesRead) << " bytes left)." << endl;

							//socket.inSeek(parsePosition+header.packetSize+footer.serializedSize);

							// Formally pass the message:
							socket.inSeek(parsePosition);

							passMessage(header, footer);

							// Continue to the next message.
							continue;
						}

						// Pass this message's footer; already read it.
						passFooter(footer);
					}
					else
					{
						auto addrOfSocket = address(socket);

						if (!onForwardPacket(socket, startPosition, addrOfSocket, header, footer))
						{
							clog << UNABLE_TO_FORWARD_PACKET << addrOfSocket << endl;

							// Packet forwarding could not be done.
							passMessage(header, footer);

							// Continue to the next message.
							continue;
						}
					}
				}

				// Make sure we don't spend all of our time reading messages:
				/*
				if (elapsed(timer) > metrics.pollTimeout)
				{
					break;
				}
				*/
			};

			return messages;
		}

		// Parsing/deserialization related:
		bool networkEngine::parseMessage(QSocket& socket, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			switch (header.type)
			{
				case MESSAGE_TYPE_META:
					parseMeta(socket, remoteAddress, header);

					break;
				case MESSAGE_TYPE_CONFIRM_PACKET:
					parsePacketConfirmationMessage(socket, remoteAddress, header, footer);

					break;
				case MESSAGE_TYPE_LEAVE:
					parseLeaveNotice(socket, remoteAddress, footer.forwardAddress);

					break;
				default:
					return false;
			}

			// Tell the user the message was loaded properly.
			return true;
		}

		bool networkEngine::onForwardPacket(QSocket& socket, streamLocation startPosition, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Tell the user that packet-forwarding isn't supported.
			return false;
		}

		// Message generation:
		// Nothing so far.

		// Serialization related:
		void networkEngine::serializeConnectionMessage(QSocket& socket, wstring name)
		{
			// Write the namne of the connecting player.
			socket.writeWideString(name);

			return;
		}

		void networkEngine::serializeLeaveNotice(QSocket& socket, disconnectionReason reason)
		{
			socket.write<disconnectionReason>(reason);

			return;
		}

		void networkEngine::serializePacketConfirmationMessage(QSocket& socket, packetID ID)
		{
			socket.write<packetID>(ID);

			return;
		}

		// Parsing/deserialization related:
		disconnectionReason networkEngine::parseLeaveNotice(QSocket& socket, const address& remoteAddress, const address& forwardAddress)
		{
			return socket.read<disconnectionReason>();
		}

		packetID networkEngine::parsePacketConfirmationMessage(QSocket& socket, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Local variable(s):

			// Read the identifier from the input.
			auto ID = socket.read<packetID>();

			// Remove the reliable-packet with this identifier.
			removeReliablePacket(remoteAddress, ID);

			// Return the packet-identifier.
			return ID;
		}

		// Methods (Protected):

		// Message generation:
		void networkEngine::finalizeOutput(QSocket& s, networkDestinationCode destinationCode)
		{
			// Check if we're a client:
			//if (!isHostNode)
			{
				// Create a final message:

				// Start a new message.
				auto headerInformation = beginMessage(s, MESSAGE_TYPE_META);

				// Serialize the destination-code.
				s << destinationCode;

				// Finish the message.
				finishMessage(s, headerInformation);
			}

			return;
		}

		// Parsing/deserialization related:
		networkDestinationCode networkEngine::parseMeta(QSocket& socket, const address& remoteAddress, const messageHeader& header)
		{
			return socket.read<networkDestinationCode>();
		}

		// clientNetworkEngine:

		// Constructor(s):
		clientNetworkEngine::clientNetworkEngine(application& parent, wstring username, networkMetrics metrics)
			: connection(), networkEngine(parent, metrics), connected(false)
		{
			// Set the name of this connection.
			connection.name = username;
		}

		bool clientNetworkEngine::open(string remoteAddress, addressPort remotePort, addressPort localPort)
		{
			if (!networkEngine::open())
				return false;

			if (!socket.connect(remoteAddress, remotePort, localPort))
				return false;

			master.IP = basic_socket::nonNativeToNativeIP(remoteAddress);
			master.port = remotePort;

			// Set the remote address of this object to the master-address.
			connection.remoteAddress = master;

			// Check if we have a username to work with, if not, assign one:
			if (connection.name.empty())
				connection.name = L"Unknown";

			sendConnectionMessage(socket, connection.name, DESTINATION_HOST);

			return true;
		}

		// Destructor(s):
		bool clientNetworkEngine::close()
		{
			this->connected = false;

			// Return the super-class's response.
			return networkEngine::close();
		}

		// Methods (Public):
		
		// Update routines:
		void clientNetworkEngine::update()
		{
			// Check if we've timed-out:
			if (timedOut())
			{
				throw exceptions::networkClosed(*this);

				return;
			}

			// Call the super-class's implementation.
			networkEngine::update();

			return;
		}

		high_resolution_clock::time_point clientNetworkEngine::updateSnapshot()
		{
			connection.updateSnapshot();

			// Call the super-class's implementation.
			return reliablePacketManager::updateSnapshot();
		}

		void clientNetworkEngine::updatePacketsInTransit(QSocket& socket)
		{
			// Call the super-class's implementation.
			networkEngine::updatePacketsInTransit(socket);

			// Call the "prune" routine for reliable packets.
			connection.pruneReliablePacket(metrics.reliablePruneTime);

			return;
		}

		bool clientNetworkEngine::onForwardPacket(QSocket& socket, streamLocation startPosition, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Re-write the message.
			rewriteMessage(socket, header, footer);

			sendMessage(socket, remoteAddress);

			// Tell the user that packet-forwarding isn't supported.
			return false;
		}

		// Reliable message related:
		bool clientNetworkEngine::onReliableMessage(QSocket& socket, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Local variable(s):

			// Just an alias to make my life easier.
			const auto& ID = footer.reliableIdentifier;

			bool response = !connection.hasReliablePacket(ID);

			if (response)
				connection.addReliablePacket(ID);

			generatePacketConfirmationMessage(socket, ID);

			//sendPacketConfirmationMessage(socket, ID);
			sendMessage(socket, remoteAddress);

			return response;
		}

		// Simple messages:
		void clientNetworkEngine::pingRemoteConnection(QSocket& socket)
		{
			// Send a "ping" messages to the host.
			sendPing(socket);

			connection.pinging = true;

			return;
		}

		// Parsing/deserialization related:
		bool clientNetworkEngine::parseMessage(QSocket& socket, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Call the super-class's implementation:
			if (networkEngine::parseMessage(socket, remoteAddress, header, footer))
				return true;

			switch (header.type)
			{
				case MESSAGE_TYPE_JOIN:
					if (!connected)
					{
						// Set the connection-flag.
						connected = true;

						updateSnapshot();

						parentProgram.onNetworkConnected(*this);
					}

					break;
				case MESSAGE_TYPE_PING:
					// Update the connection-time snapshot.
					updateSnapshot();

					// Send back a "pong" message.
					sendPong(socket);

					connection.pinging = false;

					break;
				case MESSAGE_TYPE_PONG:
					// Update the connection's ping.
					connection.ping = connection.connectionTime();

					updateSnapshot();

					//sendPong(socket);

					connection.pinging = false;

					break;
				case MESSAGE_TYPE_LEAVE:
					if (connected)
					{
						// Attempt to disconnect "gracefully". (Unreliable)
						sendCourtesyLeaveConfirmation(socket);

						// Close this network.
						close();
					}

					break;
				default:
					// Call the 'program' object's implementation.
					return parentProgram.parseNetworkMessage(socket, header, footer);
			}

			// Tell the user that the message was read.
			return true;
		}

		// Sending related:
		size_t clientNetworkEngine::broadcastMessage(QSocket& socket, networkDestinationCode destinationCode, bool resetLength)
		{
			return sendMessage(socket, connection.remoteAddress, resetLength, destinationCode);
		}

		size_t clientNetworkEngine::sendMessage(QSocket& socket, const address& remote, bool resetLength, networkDestinationCode destinationCode)
		{
			return networkEngine::sendMessage(socket, remote, resetLength, destinationCode);
		}

		bool clientNetworkEngine::hasRemoteConnection() const
		{
			return connected;
		}

		// Parsing/deserialization related:
		disconnectionReason clientNetworkEngine::parseLeaveNotice(QSocket& socket, const address& remoteAddress, const address& forwardAddress)
		{
			// Local variable(s):

			// Call the super-class's implementation, then store its "reason".
			auto reason = networkEngine::parseLeaveNotice(socket, remoteAddress, forwardAddress);
			
			// By default, clients will accept all leave-notices:
			networkEngine::sendMessage(socket, generateLeaveNotice(socket, DISCONNECTION_REASON_ACCEPT));

			// Return the "reason" given.
			return reason;
		}

		// Connection management functionality:
		player* clientNetworkEngine::getPlayer(QSocket& socket)
		{
			return &connection;
		}

		bool clientNetworkEngine::alone() const
		{
			return !connected;
		}

		size_t clientNetworkEngine::connections() const
		{
			return 1;
		}

		// Methods (Protected):
		/*
		networkDestinationCode clientNetworkEngine::parseMeta(QSocket& socket, const address& remoteAddress, const messageHeader& header)
		{
			// Call the super-class's implementation.
			auto destinationCode = networkEngine::parseMeta(socket, remoteAddress, header);

			switch (destinationCode)
			{
				case DESTINATION_EVERYONE:
					
			}
		}
		*/

		// serverNetworkEngine:

		// Constructor(s):
		serverNetworkEngine::serverNetworkEngine(application& parent, networkMetrics metrics)
			: networkEngine(parent, metrics) { isHostNode = true; isMaster = true; }

		bool serverNetworkEngine::open(const addressPort port)
		{
			if (!networkEngine::open())
				return false;

			if (!socket.host(port))
				return false;

			// Return the default response.
			return true;
		}

		// Destructor(s):
		bool serverNetworkEngine::close()
		{
			forceDisconnectPlayers(this->socket, DISCONNECTION_REASON_CLOSE, false);

			// Call the super-class's implementation, then return its response.
			return networkEngine::close();
		}

		// Methods (Public):

		// Update routines:
		void serverNetworkEngine::update()
		{
			// Call the super-class's implementation.
			networkEngine::update();

			checkClientTimeouts();

			return;
		}

		void serverNetworkEngine::updatePacketsInTransit(QSocket& socket)
		{
			// Call the super-class's implementation.
			networkEngine::updatePacketsInTransit(socket);

			for (auto p : players)
			{
				// Remove the earliest reliable-packet if this
				// player's packet-timer has gone over our maximum-time.
				p->pruneReliablePacket(metrics.reliablePruneTime);
			}

			return;
		}

		void serverNetworkEngine::checkClientTimeouts(QSocket& socket)
		{
			// Local variable(s):
			auto p = players.begin();

			while (p != players.end())
			{
				if (timedOut(*p))
				{
					parentProgram.onNetworkClientTimedOut(*this, **p);

					forceDisconnectPlayer(socket, *p, DISCONNECTION_REASON_TIMEDOUT, false, false);

					// Remove the invalid "reference" from the 'players' container.
					players.erase(p++);

					continue;
				}

				p++;
			}

			return;
		}

		size_t serverNetworkEngine::broadcastMessage(QSocket& socket, playerList players, networkDestinationCode destinationCode, bool resetLength)
		{
			//return socket.broadcastMsg();

			// Finalize this packet.
			finalizeOutput(socket);
			
			size_t sent = 0;

			for (auto p : players)
			{
				 sent += networkEngine::sendMessage(socket, p->remoteAddress, false);
			}

			/*
			switch (destinationCode)
			{
				case DESTINATION_EVERYONE:
					// Send to ourself.
					sent += networkEngine::sendMessage(socket, address("127.0.0.1", 5029), false);

					break;
			}
			*/

			if (resetLength)
				socket.flushOutput();

			return sent;
		}

		size_t serverNetworkEngine::broadcastMessage(QSocket& socket, networkDestinationCode destinationCode, bool resetLength)
		{
			return broadcastMessage(socket, this->players, destinationCode, resetLength);
		}

		bool serverNetworkEngine::hasRemoteConnection() const
		{
			return hasPlayers();
		}

		bool serverNetworkEngine::canBroadcastLocally() const
		{
			return true;
		}

		player* serverNetworkEngine::getPlayer(QSocket& socket)
		{
			return getPlayer(address(socket));
		}

		bool serverNetworkEngine::alone() const
		{
			return players.empty();
		}

		size_t serverNetworkEngine::connections() const
		{
			return players.size();
		}

		// Reliable message related:
		bool serverNetworkEngine::onReliableMessage(QSocket& socket, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Local variable(s):
			bool response = true;
			player* p = getPlayer(remoteAddress);

			// Check for errors:
			if (p != nullptr)
			{
				if (!p->hasReliablePacket(footer.reliableIdentifier))
				{
					p->addReliablePacket(footer.reliableIdentifier);
				}
				else
				{
					response = false;
				}
			}

			generatePacketConfirmationMessage(socket, footer.reliableIdentifier);

			//sendPacketConfirmationMessage(socket, footer.reliableIdentifier);
			networkEngine::sendMessage(socket, remoteAddress);

			return response;
		}

		// Simple messages:
		void serverNetworkEngine::pingRemoteConnection(QSocket& socket)
		{
			// Send "ping" messages to every player:
			for (auto p : players)
			{
				if (!p->pinging)
				{
					// Send a message to this player.
					sendMessage(socket, generatePingMessage(socket, p, p->vaddr()));

					// Update this player's connection-snapshot.
					p->updateSnapshot();

					// Set this player as "pinging", so we don't send incorrectly.
					p->pinging = true;
				}
			}

			return;
		}

		bool serverNetworkEngine::onForwardPacket(QSocket& socket, streamLocation startPosition, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Tell the user that packet-forwarding isn't supported.
			return false;
		}

		size_t serverNetworkEngine::sendMessage(QSocket& socket, outbound_packet packet, bool alreadyInOutput)
		{
			if (packet.destinationCode == DESTINATION_ALL || packet.destinationCode == DESTINATION_EVERYONE)
			{
				// Make a copy of the current player-list.
				packet.waitingConnections = players;
			}

			return networkEngine::sendMessage(socket, packet, alreadyInOutput);
		}

		// Parsing/deserialization related:
		bool serverNetworkEngine::parseMessage(QSocket& socket, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Local variable(s):

			// Attempt to match a 'player' object to this message.
			// (If no 'forwardAddress' is specified, it won't be checked)
			player* p = getPlayer(remoteAddress, footer.forwardAddress);

			// Check if this player has joined:
			if (p == nullptr)
			{
				switch (header.type)
				{
					case MESSAGE_TYPE_JOIN:
						parseConnectionMessage(socket, remoteAddress, header, footer);

						return true;
					case MESSAGE_TYPE_CONFIRM_PACKET:
						// Pass through to the main routine.
						break;
					case MESSAGE_TYPE_LEAVE:
						// We received a formal leave-message, but
						// the player has already been removed internally.
						// At this point, it's better to do nothing.

						// Tell the user that the message could not be parsed.
						return false;
					default:
						return false;
				}
			}

			// Call the super-class's implementation, then hold its response:
			bool superResponse = networkEngine::parseMessage(socket, remoteAddress, header, footer);

			switch (header.type)
			{
				case MESSAGE_TYPE_PING:
					sendPong(socket, DESTINATION_REPLY);

					p->updateSnapshot();

					p->pinging = false;

					break;
				case MESSAGE_TYPE_PONG:
					// Set this player's ping to the number
					// of milliseconds since it was requested.
					p->ping = p->connectionTime();

					// Update the time-snapshot of this player, so they don't time-out.
					p->updateSnapshot();

					p->pinging = false;

					break;
				default:
					// Call the 'program' object's implementation.
					auto programResponse = parentProgram.parseNetworkMessage(socket, header, footer);

					// After calling the super-class's implementation,
					// calculate what our response should be.
					return (superResponse || programResponse);
			}

			// Tell the user that the message was read.
			return true;
		}

		// Parsing/deserialization related:
		bool serverNetworkEngine::parseConnectionMessage(QSocket& socket, const address& remoteAddress, const messageHeader& header, const messageFooter& footer)
		{
			// Local variable(s):
			player* p;

			bool response;
			connectionType type;

			type = CONNECTION_TYPE_PLAYER; // socket.read<connectionType>();

			switch (type)
			{
				case CONNECTION_TYPE_PLAYER:
					if (remoteAddress != socket)
					{
						p = new indirect_player(remoteAddress, footer.forwardAddress);

						response = true;

						if (!connectPlayer(socket, p))
						{
							delete p;

							return response;
						}
					}
					else
					{
						p = new player(remoteAddress);

						response = false;

						if (!connectPlayer(socket, p))
						{
							delete p;

							return response;
						}
					}

					// Read the player's name from the input.
					p->name = socket.readWideString();

					//wclog << L"Player connected: " << p->name << endl;

					parentProgram.onNetworkClientConnected(*this, *p);

					break;
			}

			// Return the calculated response-code.
			return response;
		}

		disconnectionReason serverNetworkEngine::parseLeaveNotice(QSocket& socket, const address& remoteAddress, const address& forwardAddress)
		{
			// Local variable(s):

			// Call the super-class's implementation, then store its "reason".
			auto reason = networkEngine::parseLeaveNotice(socket, remoteAddress, forwardAddress);
			
			player* p = getPlayer(remoteAddress, forwardAddress);

			switch (reason)
			{
				case DISCONNECTION_REASON_ACCEPT:
					// Send a formal disconnection message, just to be courteous.
					// This message should not be reliable, and the client should not need it.
					forceDisconnectPlayer(socket, p, DISCONNECTION_REASON_ACCEPT, false);

					break;
			}

			// Return the "reason" given.
			return reason;
		}

		bool serverNetworkEngine::connectPlayer(QSocket& socket, player* p)
		{
			// Add the player to the internal-container.
			addPlayer(p);

			networkEngine::sendMessage(socket, generatePlayerConfirmationMessage(socket, p, p->vaddr()));

			// Return the default response.
			return true;
		}

		bool serverNetworkEngine::disconnectPlayer(QSocket& socket, player* p, disconnectionReason reason)
		{
			// Request that the player disconnects.
			sendMessage(socket, generateLeaveNotice(socket, reason, p));

			// Return the default response.
			return true;
		}

		void serverNetworkEngine::forceDisconnectPlayer(QSocket& socket, player* p, disconnectionReason reason, bool reliable, bool autoRemove)
		{
			// Normally disconnect the player, first:
			auto info = beginMessage(socket, MESSAGE_TYPE_LEAVE);

			serializeLeaveNotice(socket, reason);

			finishMessage(socket, info, p);

			networkEngine::sendMessage(socket, p);

			// Force the player out of the internal-container.
			removePlayer(p, autoRemove);

			// Delete the 'player' object specified.
			delete p;

			return;
		}

		void serverNetworkEngine::onPlayerRemoved(player* p)
		{
			// Remove any references to this 'player':

			// Check for lingering packets sent directly to 'p':
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

			if (!packetsInTransit.empty())
			{
				auto packetInTransit = packetsInTransit.begin();

				// Remove multi-destination ties to this 'player' object:
				while (packetInTransit != packetsInTransit.end())
				{
					removeReliablePacket(p, *packetInTransit++);
				}
			}

			return;
		}

		void serverNetworkEngine::forceDisconnectPlayers(QSocket& socket, disconnectionReason reason, bool reliable)
		{
			for (auto p : players)
			{
				forceDisconnectPlayer(socket, p, DISCONNECTION_REASON_FORCE, reliable, false);
			}

			// Remove the invalid pointers from the 'players' container.
			players.clear();

			return;
		}

		void serverNetworkEngine::disconnectPlayers(QSocket& socket, bool reliable)
		{
			for (auto p : players)
			{
				disconnectPlayer(socket, p, reliable);
			}

			return;
		}

		// Methods (Protected):
		networkDestinationCode serverNetworkEngine::parseMeta(QSocket& socket, const address& remoteAddress, const messageHeader& header)
		{
			// Call the super-class's implementation.
			auto destinationCode = networkEngine::parseMeta(socket, remoteAddress, header);

			switch (destinationCode)
			{
				case DESTINATION_ALL:
				case DESTINATION_EVERYONE:
					if (!players.empty())
					{
						// Local variable(s):
						
						// Copy the contents of the 'players' container:
						playerList playerMask;

						if (destinationCode != DESTINATION_EVERYONE)
						{
							playerMask = players;

							auto p = networkEngine::getPlayer(playerMask, socket);

							// Remove the caller from this new container.
							playerMask.remove(p);
						}

						if (!playerMask.empty() || destinationCode == DESTINATION_EVERYONE)
						{
							auto returnPoint = socket.readOffset;

							// Seek back to the beginning.
							socket.resetRead();

							while (socket.canRead())
							{
								// Local variable(s):
								streamLocation startPosition = socket.readOffset;

								messageHeader header;
								messageFooter footer;

								header.readFrom(socket, footer);

								if (header.directedHere)
								{
									switch (header.type)
									{
										case MESSAGE_TYPE_META:
											// Skip this message; meta-data, same as this.
											socket.inSeekForward(header.packetSize);

											break;
										default:
											// Store the current read-position.
											auto parsePosition = socket.readOffset;

											// Start a new message:
											auto headerInformation = beginMessage(socket, header.type);

											// Write the bytes of this message into the output.
											socket.UwriteBytes(socket.simulatedUReadBytes(header.packetSize), header.packetSize);

											// Finish the message:
											if (!footer.isReliable())
											{
												finishMessage(socket, headerInformation);
											}
											else
											{
												auto p = finishReliableMessage(socket, address(), headerInformation);
									
												p.destinationCode = destinationCode;
												
												switch (destinationCode)
												{
													case DESTINATION_EVERYONE:
														p.waitingConnections = players;

														break;
													default:
														// Copy the generated container.
														p.waitingConnections = playerMask;

														break;
												}

												// Add the new packet to the internal container.
												addReliablePacket(p);
											}

											break;
									}

									// Pass this message's footer; already read it.
									passFooter(footer);
								}
								else
								{
									// Packet forwarding should not be done here.
									passMessage(header, footer);

									// Continue to the next message.
									continue;
								}
							}

							// Send out the message:
							switch (destinationCode)
							{
								case DESTINATION_EVERYONE:
									networkEngine::sendMessage(destinationCode, true); // DESTINATION_ALL

									break;
								default:
									broadcastMessage(socket, playerMask, destinationCode, true);
							}

							socket.inSeek(returnPoint);
						}
					}

					break;
				case DESTINATION_REPLY:
					// Nothing so far.

					break;
			}

			// Return the default response.
			return destinationCode;
		}
	}

	namespace exceptions
	{
		// networkEnded:

		// Constructor(s):
		networkEnded::networkEnded(networkEngine& target, const string& exception_name)
			: iosync_exception(exception_name), network(target) { /* Nothing so far. */ }

		// Methods:
		const string networkEnded::message() const throw()
		{
			return "Network session ended unexpectedly.";
		}

		// networkClosed:

		// Constructor(s):
		networkClosed::networkClosed(networkEngine& target, const string& exception_name)
			: networkEnded(target, exception_name) { /* Nothing so far. */ }

		// Methods:
		const string networkClosed::message() const throw()
		{
			return "Network session closed; disconnected, timed-out, etc.";
		}
	}
}