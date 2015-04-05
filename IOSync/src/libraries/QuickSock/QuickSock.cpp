#include <iostream>

// Includes:
#ifndef QSOCK_MONKEYMODE
	#include "QuickSock.h"
		
	#if QSOCK_THROW_EXCEPTIONS
		#include "Exceptions.h"
	#endif
#endif

#ifndef WORDS_BIGENDIAN
	#if defined(QSOCK_WINDOWS_LEGACY) || defined(CFG_GLFW_USE_MINGW) && defined(QSOCK_MONKEYMODE)
		uqint htonf(const qfloat inFloat)
		{
			// Local variable(s):
			uqint retVal = 0;

			uqchar* floatToConvert = (uqchar*)&inFloat;
			uqchar* returnFloat = (uqchar*)&retVal;
		
			for (uqchar i = 0; i < sizeof(inFloat); i++)
				returnFloat[i] = floatToConvert[(sizeof(inFloat)-1)-i];
								
			return retVal;
		}
		
		qfloat ntohf(const uqint inFloat)
		{
			// Local variable(s):
			qfloat retVal = 0.0;

			uqchar* floatToConvert = (uqchar*) & inFloat;
			uqchar* returnFloat = (uqchar*) & retVal;
		
			for (uqchar i = 0; i < sizeof(inFloat); i++)
				returnFloat[i] = floatToConvert[(sizeof(inFloat)-1)-i];
								
			return retVal;
		}
		
		inline QSOCK_UINT64 htond(const double inFloat)
		{
			// Local variable(s):
			QSOCK_UINT64 retVal = 0;

			uqchar* floatToConvert = (uqchar*)&inFloat;
			uqchar* returnFloat = (uqchar*)&retVal;
		
			for (uqchar i = 0; i < sizeof(inFloat); i++)
				returnFloat[i] = floatToConvert[(sizeof(inFloat)-1)-i];
								
			return retVal;
		}
		
		inline qdouble ntohd(const QSOCK_UINT64 inFloat)
		{
			// Local variable(s):
			qdouble retVal = 0.0;

			uqchar* floatToConvert = (uqchar*)&inFloat;
			uqchar* returnFloat = (uqchar*)&retVal;
		
			for (uqchar i = 0; i < sizeof(inFloat); i++)
				returnFloat[i] = floatToConvert[(sizeof(inFloat)-1)-i];
								
			return retVal;
		}
	#endif
#endif

// Namespace(s):
namespace quickLib
{
	namespace sockets
	{
		// Classes:

		// Exceptions:

		// QSOCK_EXCEPTION:

		// Constructor(s):
		QSOCK_EXCEPTION::QSOCK_EXCEPTION(const QSocket* target) : exception(), socket(target) { /* Nothing so far. */ }

		// Methods:
		const char* QSOCK_EXCEPTION::what() const throw()
		{
			return message().c_str();
		}

		// QSOCK_CONSTRUCTION_EXCEPTION:

		// Constructor(s):
		QSOCK_CONSTRUCTION_EXCEPTION::QSOCK_CONSTRUCTION_EXCEPTION(const QSocket* target) : QSOCK_EXCEPTION(target) { /* Nothing so far. */ }

		// QSOCK_OUT_OF_BOUNDS_EXCEPTION:

		// Constructor(s):
		QSOCK_OUT_OF_BOUNDS_EXCEPTION::QSOCK_OUT_OF_BOUNDS_EXCEPTION(const QSocket* target, void* targetedBuffer, size_t bufferSize, size_t offsetInBuffer)
			: QSOCK_EXCEPTION(target), buffer(targetedBuffer), sizeofBuffer(bufferSize), relativePosition(offsetInBuffer) { /* Nothing so far. */ }

		// QSOCK_SEEK_EXCEPTION:

		// Constructor(s):
		QSOCK_SEEK_EXCEPTION::QSOCK_SEEK_EXCEPTION(const QSocket* target, size_t bufferSize, size_t offsetInBuffer, seekMode mode, void* targetedBuffer)
			: QSOCK_OUT_OF_BOUNDS_EXCEPTION(target, targetedBuffer, bufferSize, offsetInBuffer) { /* Nothing so far. */ }

		QSOCK_SEEK_EXCEPTION::QSOCK_SEEK_EXCEPTION(const QSocket* target, seekMode mode, size_t position)
			: QSOCK_OUT_OF_BOUNDS_EXCEPTION(target,
				(mode == SEEK_MODE_IN) ?
					(target->inbuffer) : (target->outbuffer),
				(mode == SEEK_MODE_IN) ?
					(target->inbufferlen) : (target->outbufferlen),
				position) { /* Nothing so far. */ }

		// QSOCK_READ_EXCEPTION:

		// Constructor(s):
		QSOCK_READ_EXCEPTION::QSOCK_READ_EXCEPTION(const QSocket* target, void* targetedBuffer, size_t bufferSize, size_t offsetInBuffer)
			: QSOCK_OUT_OF_BOUNDS_EXCEPTION(target, targetedBuffer, bufferSize, offsetInBuffer) { /* Nothing so far. */ }

		// QSOCK_WRITE_EXCEPTION:

		// Constructor(s):
		QSOCK_WRITE_EXCEPTION::QSOCK_WRITE_EXCEPTION(const QSocket* target, void* targetedBuffer, size_t bufferSize, size_t offsetInBuffer)
			: QSOCK_OUT_OF_BOUNDS_EXCEPTION(target, targetedBuffer, bufferSize, offsetInBuffer) { /* Nothing so far. */ }

		// QSocket:

		// Global variables:
		#if defined(QSOCK_WINDOWS)
			// An internal data-structure required for 'WinSock'. (Used for meta-data)
			WSADATA* QSocket::WSA_Data = nullptr;
		#endif

		bool QSocket::socketsInitialized = false;

		fd_set QSocket::fd;

		// Functions:
		// Nothing so far.

		// Constuctors & Destructors:

		// This command sets up an object(Usually called by the constructor):
		bool QSocket::setupObject(size_t bufferLength, bool fixBOrder)
		{
			#if defined(QSOCK_AUTOINIT_SOCKETS)
				if (!initSockets())
					return false;
			#endif

			// Initialize all of the pointers as 'nullptr':
			inbuffer = nullptr;
			result = nullptr;
			boundAddress = nullptr;
			outbuffer = nullptr;

			_socket = INVALID_SOCKET;

			// Initialize the in and out buffers:
			if (bufferLength == 0)
				bufferLength = DEFAULT_BUFFERLEN;

			_bufferlen = bufferLength;

			inbuffer = new uqchar[_bufferlen+1];
			inbuffer[_bufferlen] = '\0';

			outbuffer = new uqchar[_bufferlen+1];
			outbuffer[_bufferlen] = '\0';

			// Integer initialization:
			writeOffset = 0;
			readOffset = 0;
			outbufferlen = 0;
			inbufferlen = 0;
			port = (nativePort)0;

			// Initialize everything else we need to:

			// Initialize the timer-values.
			setTimeValues(false);

			// 'Zero-out' various fields:
			ZeroVar(hints);
			ZeroVar(si_Destination);
			ZeroVar(so_Destination);

			fixByteOrder = fixBOrder;

			// By default, we'll assume the socket isn't for a server.
			_isServer = false;

			// This is more or less unused at the moment.
			manualDelete = false;

			// Initialize the socket's closed-state variable.
			socketClosed = true;

			// Initialize the socket's 'broadcast' variable.
			broadcastSupported = false;

			//outputPort = (nativePort)0;

			#if !defined(QSOCK_IPVABSTRACT)
				//outputIP = 0;
			#else
				//outputIP = "";
			#endif

			// Return the default response (true).
			return true;
		}

		// This command deletes some odds and ends of an object before the object is deleted(Usually called by the destructor):
		bool QSocket::freeObject()
		{
			// Namespace(s):
			using namespace std;

			// Check to see if the socket has been closed, if not, close it.
			if (!socketClosed) closeSocket();

			// Free the in and out buffers:

			// Delete the input buffer.
			delete [] inbuffer;
	
			// Delete the output buffer.
			delete [] outbuffer;

			// Return the default response (true).
			return true;
		}

		QSocket::QSocket(size_t bufferlen, bool fixByteOrder)
		{
			// Ensure we're not using Monkey:
			#ifndef QSOCK_MONKEYMODE
				if (!setupObject(bufferlen, fixByteOrder))
				{
					qthrow(QSOCK_CONSTRUCTION_EXCEPTION(this));
				}
			#endif
		}

		QSocket::~QSocket()
		{
			// If we're deleting it manually, free the object.
			if (manualDelete != true)
				freeObject();
		}

		// Functions:

		bool QSocket::initSockets(size_t bufferlen)
		{
			// Namespace(s):
			using namespace std;

			// Check if we've initialized yet, if we have, do nothing.
			if (socketsInitialized)
				return false;
	
			#if defined(QSOCK_WINDOWS)
				WSA_Data = new WSADATA();

				QSOCK_INT32 iResult = WSAStartup(MAKEWORD(2,2), WSA_Data);

				if (iResult != 0)
				{
					// Cleanup everything related to WSA:

					// Delete the 'WSA_Data' variable.
					delete WSA_Data;

					WSACleanup();

					return false;
				}
			#endif

			socketsInitialized = true;

			// Initialize the 'File-descriptor set':
			FD_ZERO(&fd);

			// Return the default response.
			return true;
		}

		bool QSocket::deinitSockets()
		{
			// Namespace(s):
			using namespace std;

			if (!socketsInitialized)
				return false;

			// Free all of the objects we need to:
			#if defined(QSOCK_WINDOWS)
				delete WSA_Data; WSA_Data = nullptr;

				// Cleanup WinSock functionality.
				WSACleanup();
			#endif

			// Asign the socketsInitialized variable back to false.
			socketsInitialized = false;

			// Return something
			return true;
		}

		// Methods:
		qint QSocket::bindInternalSocket(qint filter)
		{
			// Namespace(s):
			using namespace std;

			// Local variable(s):
			qint bindResult = 0;

			// Used for socket options:
			QSOCK_INT32 bAllow = 1;
			QSOCK_INT32 bDisable = 0;

			#if !defined(QSOCK_IPVABSTRACT)
				_socket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
				bindResult = bind(_socket, (const sockaddr*)boundAddress, sizeof(socketAddress));
			#else
				// Bind the socket using the one of the addresses in 'result':
				for (boundAddress = result; boundAddress != nullptr; boundAddress = boundAddress->ai_next)
				{
					if (filter != -1 && boundAddress->ai_family != filter)
						continue;

					// Try to create a socket.
					_socket = socket(boundAddress->ai_family, boundAddress->ai_socktype, boundAddress->ai_protocol);

					// Check for errors while attempting socket creation:
					if (_socket == INVALID_SOCKET)
					{
						// Unable to create a socket, skip this attempt.
						continue;
					}

					// Assgin various socket options:
					#if defined(QSOCK_IPVABSTRACT)
						if
						(
							#if defined(QSOCK_IPVABSTRACT)
								boundAddress->ai_family == AF_INET6
							#else
								boundAddress->sin_family == AF_INET6
							#endif
						)
						{
							// Add support for IPV4:
							if (setsockopt(_socket, IPPROTO_IPV6, IPV6_V6ONLY, (const QSOCK_CHAR*)&bDisable, sizeof(bDisable)) != 0)
							{
								//return false;
							}
						}
					#endif

					if (isServer())
					{
						if
						(
							#if defined(QSOCK_IPVABSTRACT)
								boundAddress->ai_family != AF_INET
							#else
								boundAddress->sin_family != AF_INET
							#endif
							|| setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (const QSOCK_CHAR*)&bAllow, sizeof(bAllow)) < 0
						)
						{
							broadcastSupported = false;
        
							//return false;
						}
						else
						{
							broadcastSupported = true;
						}
					}

					// Attempt to bind the socket.
					bindResult = bind(_socket, boundAddress->ai_addr, boundAddress->ai_addrlen);

					// Check if the bind attempt was successful.
					if (bindResult == 0)
					{
						// The socket was successfully bound, exit the loop.
						break;
					}
					else
					{
						// Close the socket.
						shutdownInternalSocket();
					}
				}
			#endif

			// Return the error code produced from 'bind'.
			return bindResult;
		}

		bool QSocket::bindSocket(const nativePort internalPort)
		{
			// Namespace(s):
			using namespace std;

			// Setup the socket's information:
			#if !defined(QSOCK_IPVABSTRACT)
				hints.ai_family = AF_INET;
			#else
				// This will be changed to 'AF_UNSPEC' later on, assuming an IPV4 address couldn't be found.
				//hints.ai_family = AF_INET;
				hints.ai_family = AF_UNSPEC;
			#endif

			// Set the information for 'hints':
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_protocol = IPPROTO_UDP;

			#if !defined(QSOCK_MONKEYMODE)
				hints.ai_flags = AI_PASSIVE;
			#endif

			// Setup the 'result' socket 'address_in' with 'hints', the internal port, and 'INADDR_ANY' as the address:
			#if !defined(QSOCK_IPVABSTRACT)
				result = new socketAddress(); // Also known as 'sockaddr_in'

				#if defined(QSOCK_WINDOWS)
					result->sin_addr.S_un.S_addr =
				#else
					result->sin_addr.s_addr =
				#endif

				htonl(INADDR_ANY);

				result->sin_family = hints.ai_family;
				result->sin_port = htons(internalPort);
			#else
				{
					// Map the port agnostically to the 'result' address.

					// Convert the port to a string:
					stringstream portSStream; portSStream << internalPort;

					/*
					QSOCK_CHAR* portStr = new QSOCK_CHAR[portSStream.str().length()+1];
					memcpy(portStr, portSStream.str().c_str(), portSStream.str().length());
					portStr[portSStream.str().length()] = '\0';
					*/
			
					/*
					// Check for IPV4 addresses first:
					if (getaddrinfo(nullptr, (const QSOCK_CHAR*)portStr, &hints, &result) != 0) // portSStream.str().c_str()
					{
						// We weren't able to find an IPV4 address, clean things up, and try with any IP-version:
						if (result != nullptr) freeaddrinfo(result); result = nullptr; // delete [] result;
				
						// Assign the ip-family to unspecific.
						hints.ai_family = AF_INET;
					*/

					if (getaddrinfo(nullptr, portSStream.str().c_str(), &hints, &result) != 0) // portStr
					{
						// Delete the address-info result.
						if (result != nullptr) freeaddrinfo(result); //delete [] result;

						// Since we can't continue, tell the user.
						return false;
					}

					//delete [] portStr;
					//}

					//hints.ai_flags = 0;
				}
			#endif

			// Create the socket using 'hints':
			{
				// Local variable(s):

				// The result-variable used for the bind-result.
				qint bindResult = 0;

				#if !defined(QSOCK_IPVABSTRACT)
					// Assign the bound-address variable to 'result'.
					boundAddress = result;

					// Bind the socket, using the address 'boundAddress' is pointing to.
					bindResult = bindInternalSocket();
				#else
					// Try IPV6 before attempting to use any protocol.
					bindResult = bindInternalSocket(AF_INET6);

					// We weren't able to use IPV6, try any protocol:
					if (bindResult != 0)
					{
						bindResult = bindInternalSocket();
					}
				#endif

				// Check for any errors while creating the socket:
				if (_socket == INVALID_SOCKET)
				{
					closeSocket();

					return false;
				}

				// Check if our bind-attempt was successful:
				if (bindResult == SOCKET_ERROR)
				{
					// Close the internal socket.
					closeSocket();

					// Return a negative response.
					return false;
				}

				///* Do not set the family of so_Destination first, this will screw up checks made later on:
				// Assign the IO addresses' families:
				#if !defined(QSOCK_IPVABSTRACT)
					si_Destination.sin_family = boundAddress->sin_family;
					//so_Destination.sin_family = si_Destination.sin_family;
				#else
					si_Destination.sa_family = boundAddress->ai_addr->sa_family; //boundAddress->ai_family;
					//so_Destination.sa_family = si_Destination.sa_family;
				#endif
				//*/
			}

			// Return the default response.
			return true;
		}

		#if !defined(QSOCK_MONKEYMODE)
		bool QSocket::connect(nativeString address, const nativePort externalPort, const nativePort internalPort)
		{
		#else
		bool QSocket::connect(nativeString _address, const QSOCK_INT32 _externalPort, const QSOCK_INT32 _internalPort)
		{
			// Locsl variable(s):
			const nativePort externalPort = (nativePort)_externalPort;
			const nativePort internalPort = (nativePort)_internalPort;
		#endif
			// Namespace(s):
			using namespace std;

			if (!socketClosed) return false;

			// If we're using Monkey, convert 'address' from a Monkey-based string to a normal 'nativeString':
			#if defined(QSOCK_MONKEYMODE)
				// Convert the monkey-based string into a standard-string:
				std::string address(_address.Length());

				// Convert the Monkey-native address-string to a char array.
				for (uqint index = 0; index < _address.Length(); index++)
					address[index] = _address[index];
			#endif

			// Check if we're using IPV6 or not:
			/*
			#if defined(QSOCK_IPV6)
				for (QSOCK_UINT32_LONG ipIndex = 0; ipIndex < (QSOCK_UINT32_LONG)strlen(address); ipIndex++)
				{
					if (address[ipIndex] == ':')
					{
						Manual_IPV6Enabled = true;
						break;
					}
				}
			#endif
			*/
	
			// Definitions:
	
			// Set the port variable.
			this->port = externalPort;
	
			// Set the 'isServer' variable.
			this->_isServer = false;

			// Bind and initialize the internal socket.
			if (!bindSocket(internalPort)) return false;
	
			// Setup our destination using the external port, and the address:
			if (!setupDestination(address, this->port)) return false;

			// Set the timer values.
			setTimeValues(true);

			// Finish doing any needed operations:

			// Set the socket as 'open'.
			this->socketClosed = false;

			// Return the default response.
			return true;
		}

		#if defined(QSOCK_MONKEYMODE)
		bool QSocket::host(const QSOCK_INT32 _externalPort)
		{
			nativePort externalPort = (nativePort)(_externalPort);
		#else
		bool QSocket::host(nativePort externalPort)
		{
		#endif
			// Namespace(s):
			using namespace std;

			// Definitions:
	
			// Set the port variable.
			this->port = externalPort;

			// Set the isServer variable:
			this->_isServer = true;

			// Bind and initialize the internal socket.
			if (!bindSocket(externalPort)) return false;

			// Set the timer values.
			setTimeValues(true);

			// Finish doing any needed operations:
			this->socketClosed = false;

			// Return the default response.
			return true;
		}

		bool QSocket::setupDestinationV4(QSOCK_UINT32_LONG address, nativePort externalPort)
		{
			#if !defined(QSOCK_IPVABSTRACT)
				if (externalPort == (nativePort)0)
				{
					externalPort = msgPort();

					// If we still don't have a port to work with, tell the user:
					if (externalPort == (nativePort)0)
						return false;
				}

				// If the address is set to zero, and we can't broadcast, try the IP of the last message received:
				if (address == 0 && !broadcastSupported)
				{
					// Get the IP from the last packet received.
					address = msgIP();

					// If we still don't have an IP to work with, tell the user:
					if (address == 0)
						return false;
				}
		
				// Assign the output-family to IPV4.
				so_Destination.sin_family = AF_INET;

				// Assign the output-port to 'externalPort'.
				so_Destination.sin_port = htons((uqshort)externalPort);

				// Assign the output-address to 'address':
				#if defined(QSOCK_WINDOWS)
					so_Destination.sin_addr.S_un.S_addr =
				#else
					so_Destination.sin_addr.s_addr =
				#endif

				htonl(address);

				// No errors were found, return as usual.
				return true;
			#else
				// Call the main 'QSOCK_IPVABSTRACT' compliant version of 'setupDestination'.
				return setupDestination(IntToStringIP(address), externalPort);
			#endif

			// This point should never be reached.
			return false;
		}

		bool QSocket::setupDestination(std::string address, nativePort externalPort)
		{
			// Namespace(s):
			using namespace std;

			// Local variable(s):
			bool response(false);

			#if !defined(QSOCK_IPVABSTRACT)
				// Call the IPV4 version of 'setupDestination'.
				response = setupDestination(

				#if !defined(QSOCK_MONKEYMODE)
					StringToIntIP(address),
				#else
					StringToIntIP(nativeString(address.c_str())),
				#endif

					externalPort);
			#else
				// If I end up adding to this function later, I'll want what's in here to be cleaned up first:
				{
					if (address.length() == 0)
					{
						address = (std::string)msgIP();

						if (address.length() == 0)
						{
							// We were unable to find a suitable address.
							return false;
						}
					}

					if (externalPort == (nativePort)0)
					{
						externalPort = msgPort();

						// Check if we've been given a valid port, and if not, return false:
						if (externalPort == (nativePort)0)
							return false;
					}

					// Local variable(s):
					addrinfo* so_Destination_result = nullptr;
					stringstream portSStream; portSStream << externalPort;

					// inet_pton(hints.sa_family, address, &so_Destination)

					if (getaddrinfo(address.c_str(), portSStream.str().c_str(), &hints, &so_Destination_result) != 0)
					{
						// Clean up the address-list.
						freeaddrinfo(so_Destination_result);

						// Since we can't continue, tell the user.
						return false;
					}

					// Copy the result from 'getaddrinfo' to 'so_Destination', then delete the old copy of the result:
					memcpy(&so_Destination, so_Destination_result->ai_addr, sizeof(so_Destination));

					/*
					if (!isBigEndian())
					{
						if (so_Destination.sa_family == AF_INET)
						{
							// Swap the byte-order of the IPV4 segment:
							QSOCK_UINT32_LONG* V4Address = (QSOCK_UINT32_LONG*)(so_Destination.sa_data+sizeof(nativePort));

							*V4Address = htonl(*V4Address);
						}
					}
					*/

					// Clean up the address-list.
					freeaddrinfo(so_Destination_result);

					// Set the response to 'true'.
					response = true;
				}
			#endif

			// Return the default response.
			return response;
		}

		// Update related:

		// This is our generic update command(It applies to both the client and server).
		QSOCK_INT32 QSocket::listenSocket()
		{
			// General update code:

			// Free the message buffers:
			clearInBuffer();
			clearOutBuffer();

			if (isServer())
			{
				// Run the server's update routine.
				return hostUpdate();
			}

			// Run the client's update routine.
			return clientUpdate();
		}

		// 'clientUpdate' & 'hostUpdate' have been moved to the main header.

		// These still hold all of their code, they're just private now:
		QSOCK_INT32 QSocket::hostUpdate()
		{
			// Namespace(s):
			using namespace std;

			// The main program:

			// Check for messages:
			readMsg();

			// Return the default response
			return 0;
		}

		QSOCK_INT32 QSocket::clientUpdate()
		{
			// Namespace(s):
			using namespace std;

			// The main program:

			// Check for messages:
			return readMsg();
		}

		// Input related:
		QSOCK_INT32 QSocket::readAvail()
		{
			// Namespace(s):
			using namespace std;

			// Definitions:
			QSOCK_INT32 response = 0;

			if (!FD_ISSET(_socket, &fd))
			{
				inbufferlen = 0;
				readOffset = 0;
			}
			else
			{
				// Unfortunately, this has to be an 'int', instead of a 'size_t'.
				int socketAddress_Length = (int)sizeof(socketAddress);

				response = (recvfrom(_socket, (QSOCK_CHAR*)inbuffer, _bufferlen, 0, (sockaddr*)&si_Destination, &socketAddress_Length));

				//readOffset = 0;

				// Check for errors:
				if (response == SOCKET_ERROR && QSOCK_ERROR != WSAENETRESET && QSOCK_ERROR != WSAECONNRESET)
				{
					return SOCKET_ERROR;
				}
				else if (response > 0)
				{
					inbufferlen = response;

					// Reset the length and offset for the input-buffer:
					readOffset = 0;

					// Assign 'so_Destination' to 'si_Destination'.
					memcpy(&so_Destination, &si_Destination, sizeof(socketAddress));
				}
			}

			// Check for messages:
			FD_ZERO(&fd);
			FD_SET(_socket, &fd);
	
			if (select(1, &fd, nullptr, nullptr, (timeval*)&tv) == SOCKET_ERROR)
			{
				return SOCKET_ERROR;
			}
	
			// Return the calculated response.
			return response;
		}

		QSOCK_INT32 QSocket::readMsg()
		{
			return readAvail();
		}

		bool QSocket::clearInBuffer()
		{
			// 'Zero-out' the inbound-message buffer.
			ZeroMemory(inbuffer, _bufferlen);

			// Set the read-offset and inbound-message length to zero.
			readOffset = 0;

			flushInput();

			// Return the default response (true).
			return true;
		}

		void QSocket::setTimeValues(bool init)
		{
			// Set the time values:
			if (init)
			{
				FD_SET(_socket, &fd);

				tv.tv_usec = 0;
				tv.tv_sec = 0;

				// DO NOT UNCOMMENT THIS.
				select(1, &fd, nullptr, nullptr, &tv);
			}

			tv.tv_sec = TIMEOUT_SEC;
			tv.tv_usec = TIMEOUT_USEC;

			return;
		}

		nativeString QSocket::strMsgIP() const
		{
			#if !defined(QSOCK_IPVABSTRACT)
				return QSocket::IntToStringIP(msgIP());
			#else
				// Namespace(s):
				using namespace std;
		
				// Local variable(s):
				//size_t outputLength(0);
				//void* dataPosition = nullptr;

				// Allocate the needed c-string(s).
				QSOCK_CHAR output_cstr[NI_MAXHOST];

				// Allocate the output 'nativeString'.
				nativeString output;

				// Convert the address to a string:
				//inet_ntop(si_Destination.sa_family, dataPosition, output_cstr, outputLength); // (void*)&si_Destination.sa_data

				if (getnameinfo(&so_Destination, sizeof(socketAddress), output_cstr, sizeof(output_cstr), nullptr, 0, 0) != 0) // sizeof(sockaddr_in6)
				{
					// Nothing so far.
				}

				// Convert the c-string to a 'nativeString':
				output = (nativeString)output_cstr;

				// Return the newly generated 'nativeString'.
				return output;
			#endif
		}

		QSOCK_UINT32_LONG QSocket::intMsgIP() const
		{
			return

			#ifdef QSOCK_MONKEYMODE
				(QSOCK_INT32)
			#endif

			#if !defined(QSOCK_IPVABSTRACT)
				ntohl
				(
				#if defined(QSOCK_WINDOWS)
					si_Destination.sin_addr.S_un.S_addr
				#else
					si_Destination.sin_addr.s_addr
				#endif
				);
			#else
				(si_Destination.sa_family == AF_INET) ? StringToIntIP(strMsgIP()) : 0;
			#endif
		}

		nativePort QSocket::msgPort() const
		{
			// Check the destination's family, and based on that, return the internal port.
			#if !defined(QSOCK_IPVABSTRACT)
				/*
				switch (si_Destination.sin_family)
				{
					default: // case AF_INET:
						return (nativePort)ntohs(((sockaddr_in*)&si_Destination)->sin_port);

						break;
				}
				*/
		
				return (nativePort)ntohs(((sockaddr_in*)&si_Destination)->sin_port);
			#else
				// Allocate the needed c-string(s).
				QSOCK_CHAR serverInfo[NI_MAXSERV];

				// Convert the service name to a 'nativePort'
				if (getnameinfo((const socketAddress*)&si_Destination, sizeof(socketAddress), nullptr, 0, serverInfo, NI_MAXSERV, NI_DGRAM|NI_NUMERICSERV) != 0)
				{
					return (nativePort)0;
				}

				return (nativePort)atol(serverInfo);
			#endif

			// If we couldn't calculate the port, return zero.
			return (nativePort)0;
		}

		// The generic reading command for raw data ('size' is in bytes):
		bool QSocket::readData(void* output, size_t size, size_t output_offset, bool checkRead)
		{
			// Ensure we can read more of the internal buffer:
			#ifndef QSOCK_THROW_EXCEPTIONS
				if (!checkRead || canRead(size))
			#endif
				{
					// Hold the current input-position, so we can attempt to seek safely.
					auto currentOffset = readOffset;

					// Seek forward the amount we're going to transfer.
					inSeekForward(size);
					
					// Transfer from the input-buffer to the output.
					size_t transferred = rawTransfer(inbuffer, output, size, currentOffset, output_offset);

					return (transferred == size); // true;
				}

			// Return the default response.
			return false;
		}

		bool QSocket::UreadBytes(uqchar* output, uqint count, size_t output_offset, bool checkRead)
		{
			return readData(output, (count == 0) ? inBytesLeft() : count, output_offset, checkRead);
		}

		uqchar* QSocket::UreadBytes(uqint count, bool zero_ended)
		{
			if (!canRead(count))
				return nullptr;

			uqchar* data;

			// Create a new array to use as an output:
			if (zero_ended)
			{
				data = new uqchar[count + 1];
				data[count] = (uqchar)'\0';
			}
			else
			{
				data = new uqchar[count];
			}

			// Call the main implementation.
			UreadBytes(data, count, 0, false);

			return data;
		}

		// Line related:
		nativeString QSocket::readLine()
		{
			// Namespace(s):
			using namespace std;

			// Definitions:
			QSOCK_INT32 count = inbufferlen-readOffset;

			if (count <= 0)
				return nativeString();

			QSOCK_UINT32 strLen = 0;
			uqchar* bytePosition = nullptr;

			for (uqint index = readOffset; index < (uqint)inbufferlen; index++)
			{
				bytePosition = inbuffer+index;
		
				// Search for the correct bytes to end the line:
				if (*bytePosition == (uqchar)'\r' && (index+1) <= (uqint)inbufferlen)
				{
					// If we've found the final byte, break.
					if (*(bytePosition++) == (uqchar)'\n')
						break;
				}

				// Add to the final string-length.
				strLen++;
			}

			// Allocate the output-string.
			nativeString output;

			#if !defined(QSOCK_MONKEYMODE)
				// Copy part of the contents of 'inbuffer' to our output 'string'.
				output.assign((QSOCK_CHAR*)(inbuffer+readOffset), strLen);
			#else
				// Allocate a 'string' for the output using 'tempBuffer'.
				output = String(inbuffer+readOffset, strLen);
			#endif

			// Add to the offset.
			readOffset += strLen;

			// Return the output.
			return output;
		}

		// Output related:
		QSOCK_INT32 QSocket::broadcastMsg(nativePort port, bool resetLength) // Only really useful for servers/hosts.
		{
			// Namespace(s):
			using namespace std;

			if (!isServer() || !broadcastSupported)
				return sendMsg(msgIP(), port);

			if (isServer() && broadcastSupported)
			{
				// Set the IPV4-destination.
				if (!setupDestination(INADDR_BROADCAST, port)) return SOCKET_ERROR;

				// Output the message.
				outputMessage(resetLength);
			}

			return 0;
		}

		#if defined(QSOCK_MONKEYMODE)
		QSOCK_INT32 QSocket::sendMsg(QSOCK_INT32 _IP, QSOCK_INT32 _port, bool resetLength)
		{
			QSOCK_UINT32_LONG IP = (QSOCK_UINT32_LONG)(_IP);
			nativePort port = (nativePort)(_port);
		#else
		QSOCK_INT32 QSocket::sendMsg(QSOCK_UINT32_LONG IP, nativePort port, bool resetLength)
		{
		#endif
			// Namespace(s):
			using namespace std;

			// Setup the output-address structure with the needed information.
			if (!setupDestination(IP, port)) return SOCKET_ERROR;

			// Output the message.
			return outputMessage(resetLength);
		}


		#if defined(QSOCK_MONKEYMODE)
		QSOCK_INT32 QSocket::sendMsg(nativeString strIP, QSOCK_INT32 _port, bool resetLength)
		{
			nativePort port = (nativePort)(_port);
	
			/*
			std::string strIP(_strIP.Length(), '\0');

			for (uqint index = 0; index < _strIP.Length(); index++)
			{
				strIP[index] = _strIP[index];
			}
			*/
		#else
		QSOCK_INT32 QSocket::sendMsg(nativeString strIP, nativePort port, bool resetLength)
		{
		#endif

			#if !defined(QSOCK_IPVABSTRACT)
				return sendMsg(StringToIntIP(strIP), port, resetLength);
			#else
				if (!setupDestination(strIP, port)) return SOCKET_ERROR;

				// Output the message.
				return outputMessage(resetLength);
			#endif

			// Return the default response.
			return 0;
		}

		QSOCK_INT32 QSocket::sendMsg(bool resetLength)
		{
			if
			(
				#if !defined(QSOCK_IPVABSTRACT)
					so_Destination.sin_family == 0
				#else
					so_Destination.sa_family == 0
				#endif
			)
			{
				if (!setupDestination(msgIP(), msgPort()))
				{
					return SOCKET_ERROR;
				}
			}

			return outputMessage(resetLength);
		}

		QSOCK_INT32 QSocket::outputMessage(bool resetLength)
		{
			// Namespace(s):
			using namespace std;

			// Ensure we have an address to work with.
			if
			(
				#if !defined(QSOCK_IPVABSTRACT)
					so_Destination.sin_family == 0
				#else
					so_Destination.sa_family == 0
				#endif
			)
			{
				return SOCKET_ERROR;
			}

			// Send the current 'outbuffer' to the IP address specified, on the port specified:
			QSOCK_INT32 transferred = sendto(_socket, (const QSOCK_CHAR*)outbuffer, outbufferlen, 0, (const sockaddr*)&so_Destination, sizeof(sockaddr)); // No flags added for now.

			// Check for errors:
			if (transferred == SOCKET_ERROR) 
			{
				return SOCKET_ERROR;
			}

			// Reset various write/output variables:
			if (resetLength)
				flushOutput();

			// Manually set the output-offset back to the default.
			writeOffset = 0;

			// Clear the output-buffer.
			//clearOutBuffer(); // Commented out for various reasons.

			// Tell the user how many bytes were transferred.
			return transferred;
		}

		bool QSocket::clearOutBuffer() { memset(outbuffer, 0, _bufferlen); flushOutput(); writeOffset = 0; return true; }

		// Buffer-writing related:

		// The generic writing command for raw data ('size' is in bytes):
		bool QSocket::writeData(const void* input, size_t size, size_t input_offset)
		{
			// Ensure we can write more into the internal buffer:
			#ifndef QSOCK_THROW_EXCEPTIONS
				if (canWrite(size))
			#endif
				{
					// Hold the current output-position, so we can attempt to seek safely.
					auto currentOffset = writeOffset;

					// Seek forward the amount we're going to transfer.
					outSeekForward(size);

					// Transfer from the input to the output-buffer.
					size_t transferred = rawTransfer(input, outbuffer, size, input_offset, currentOffset);

					return (transferred == size); // true;
				}

			// Return the default response.
			return false;
		}

		bool QSocket::writeBytes(const qchar* data, uqint dataSize) { return writeData(data, (dataSize == 0) ? strlen((const QSOCK_CHAR*)data) : dataSize); }
		bool QSocket::UwriteBytes(const uqchar* data, uqint dataSize) { return writeData(data, (dataSize == 0) ? strlen((const QSOCK_CHAR*)data) : dataSize); }

		// Line related:
		bool QSocket::writeLine(const QSOCK_CHAR* strIn, uqint length)
		{
			// Definition(s):
			bool response = false;

			// Write all of the bytes in strIn to the 'outbuffer'.
			response = writeBytes(strIn, length);
			if (!response) return response;

			// Setup the end of the line:
			response = writeChar('\r');
			if (!response) return response;
	
			response = writeChar('\n');

			// Return the response.
			return response;
		}

		// The rest of the commands:

		// This is the same as closeSocket.
		bool QSocket::close(QSOCK_INT32 nothing) { return closeSocket(); }

		// Close the internal socket:
		bool QSocket::closeSocket()
		{
			// Namespace(s):
			using namespace std;

			// Check if this socket has been closed already:
			if (socketClosed)
			{
				return false;
			}

			// Delete any extra data, clean things up, etc:
			#if defined(QSOCK_IPVABSTRACT)
				freeaddrinfo((addrinfo*)result); // delete [] result;
			#else
				delete result;
			#endif
	
			result = nullptr;
			boundAddress = nullptr;

			ZeroVar(si_Destination);
			ZeroVar(so_Destination);

			// Free the internal socket:
			#if defined(QSOCK_WINDOWS)
				if (_socket != 0 && closesocket(_socket) == SOCKET_ERROR)
			#else
				if (_socket != 0 && close(_socket) == SOCKET_ERROR)
			#endif
				{
					return false;
				}

			shutdownInternalSocket();

			// Set the socket as closed.
			socketClosed = true;

			// Return the default response.
			return true;
		}

		// These commands are mainly here for Monkey, but you can use them in C++ as well:

		#ifdef QSOCK_MONKEYMODE
			QSOCK_INT32 QSocket::StringToIntIP(nativeString IP)
		#else
			QSOCK_UINT32_LONG QSocket::StringToIntIP(nativeString IP)
		#endif
		{
			// Definition(s):
			QSOCK_UINT32_LONG intIP = 0;
	
			// Calculate the integer for the IP address:
			#ifdef QSOCK_MONKEYMODE
				// Local variable(s):

				// Allocate a new character array.
				char* data = new char[IP.Length()+1];
				data[IP.Length()] = '\0';
		
				// Convert the monkey-based string into a c-string:
		
				// Convert the Monkey-native string to a character array:
				wcstombs(data, IP.Data(), IP.Length());

				/*
				for (uqint index = 0; index < IP.Length(); index++)
					data[index] = IP[index];
				*/
		
				// Get the IPV4 address from 'data'.
				intIP = inet_addr(data);

				// Delete the temporary buffer.
				delete [] data;
			#else
				#ifdef QSOCK_IPV4_RESOLVE_HOSTNAMES
					// This implementation is rather...
					// Well, let's just say it's a terrible hack:
					addrinfo hints;
					addrinfo* info = nullptr;

					ZeroVar(hints);

					hints.ai_family = AF_INET;
					hints.ai_socktype = SOCK_DGRAM;
					hints.ai_protocol = IPPROTO_UDP;

					if (getaddrinfo(IP.c_str(), nullptr, &hints, &info) != 0)
						return 0;

					auto* v4 = ((sockaddr_in*)(info->ai_addr));

					intIP =

					#if defined(QSOCK_WINDOWS)
						v4->sin_addr.S_un.S_addr;
					#else
						v4->sin_addr.s_addr;
					#endif

					freeaddrinfo(info);
				#else
					intIP = inet_addr(IP.c_str());
				#endif
			#endif

			// Return the calculated IP address:
			return
			#ifdef QSOCK_MONKEYMODE
				(QSOCK_INT32)ntohl(intIP);
			#else
				ntohl(intIP);
			#endif
		}

		#ifdef QSOCK_MONKEYMODE
			nativeString QSocket::IntToStringIP(QSOCK_INT32 IP)
		#else
			nativeString QSocket::IntToStringIP(QSOCK_UINT32_LONG IP)
		#endif
		{
			// Definition(s):
			struct in_addr address;

			// Add the IP to 'address':
			#if defined(QSOCK_WINDOWS)
				address.S_un.S_addr =
			#else
				address.s_addr =
			#endif

			htonl(IP);

			// Return a nativeString of the IP address:
			#ifdef QSOCK_MONKEYMODE
				return nativeString(inet_ntoa(address));
			#else
				return inet_ntoa(address);
			#endif
		}
	}
}
