#ifndef QSOCK_MONKEYMODE
	#pragma once
#endif
	
// Preprocessor related:

// QuickSock related:
#if defined(_WIN32) || defined(_WIN64)
	#define QSOCK_WINDOWS
#endif
	
#if !defined(QSOCK_WINDOWS_LEGACY)
	#if defined(QSOCK_WINDOWS) && defined(_MSC_VER) && (_MSC_VER <= 1600)
		#define QSOCK_WINDOWS_LEGACY
	#endif
#endif

#if defined(QSOCK_DLL)
	#if defined(QSOCK_MONKEYMODE) || !defined(QSOCK_WINDOWS)
		#undef QSOCK_DLL
	#endif
#endif

// Check if exceptions are enabled:
#if defined(QSOCK_THROW_EXCEPTIONS) && !defined(QSOCK_MONKEYMODE)
	#define qthrow(X) throw X
#else
	#define qthrow(X)
#endif

#if !defined(QSOCK_IPVABSTRACT)
	// This allows the IPV4-backend to resolve hostnames.
	// Using this may cause extra overhead, but this isn't
	// used very often by the IPV4 implementation to begin with.
	#define QSOCK_IPV4_RESOLVE_HOSTNAMES
	
	//#define QSOCK_IPVABSTRACT // By defining this, 'QuickSock' will use a 'modern' abstract approach to IP-version handling (May cause problems).
	//#define QSOCK_IPV6 // IPV6 support is now handled with IPVABSTRACT.
#else
	#if defined(QSOCK_IPV6)
		#undef QSOCK_IPV6
	#endif
#endif

#if defined(QSOCK_DLL)
	#define DLLImport   __declspec( dllimport )
	#define DLLExport   __declspec( dllexport )
#else
	#define DLLExport
	#define DLLImport
#endif

#ifndef QSOCK_ERROR
	// These can be found in the includes:
	#if defined(QSOCK_WINDOWS)
		#define QSOCK_ERROR WSAGetLastError()
	#else
		#define QSOCK_ERROR errno
	#endif
#endif

#ifdef WORDS_BIGENDIAN
	#define QSOCK_BIGENDIAN
#endif

// Includes (Internal):
#ifndef QSOCK_MONKEYMODE
	#include "QuickTypes.h"
	#include "Exceptions.h"
#endif

// Includes (External):

// Standard Library:

// C Standard library:
#include <cstdlib>
#include <algorithm>

// C++ Standard Library:
#include <string>
#include <sstream>

// Windows specific includes:
#if defined(QSOCK_WINDOWS)
	// WinSock related:
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
		#define QSOCK_WIN32_L_A_M
	#endif

	//#if (!defined(QSOCK_IPV6) && !defined(QSOCK_IPVABSTRACT))
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	//#endif
	
	// Make sure to link with the required library files:
	#pragma comment (lib, "Ws2_32.lib")
	//#pragma comment (lib, "Mswsock.lib")
	//#pragma comment (lib, "AdvApi32.lib")
	
	// Windows Includes (Much nicer than the Linux includes section):
	#include <windows.h>
	
	#if !defined(QSOCK_MONKEYMODE)
		#include <winsock2.h>
		#include <ws2tcpip.h>
		#include <WSPiApi.h>
		#include <iphlpapi.h>

		#if defined(QSOCK_IPVABSTRACT)
			#include <mstcpip.h>
		#endif
	#else
		#include <winsock.h>

		typedef int socklen_t;
	#endif

	// WinSock's manifest constants for the 'shutdown' command:
	#ifndef _WINSOCK2API_
		#define SD_RECEIVE      0x00
		#define SD_SEND         0x01
		#define SD_BOTH         0x02
	#endif

	#ifdef QSOCK_WIN32_L_A_M
		#undef QSOCK_WIN32_L_A_M

		#undef WIN32_LEAN_AND_MEAN
	#endif
#endif
//#endif

// Linux, BSD, Mac OS X, etc:
#if !defined(QSOCK_WINDOWS)
	// Includes:

	// Oh god, what am I looking at:
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <stdio.h>
	#include <arpa/inet.h>
	#include <stdint.h>
	#include <stdlib.h>
	#include <errno.h>

	// Don't mind me, just supporting things I don't need to:
		
	// Special thanks to Nanno Langstraat for this:
	#if defined(__linux__)
		// Includes:
		#include <endian.h>
	#elif defined(__FreeBSD__) || defined(__NetBSD__)
		// Includes:
		#include <sys/endian.h>
	#elif defined(__OpenBSD__)
		// Includes:
		//#include <sys/types.h>

		#define be16toh(x) betoh16(x)
		#define be32toh(x) betoh32(x)
		#define be64toh(x) betoh64(x)
	#endif

	// For the sake of laziness, I want to keep 'htonll' and 'ntohll':
	#if !defined(__APPLE__) && !defined(__MACH__)
		#define htonll(x) htobe64(x)
		#define ntohll(x) be64toh(x)
	#endif

	// Special thanks to Adam Banko for this part of the header.
	// In the end, I still needed to use this:
	
	//#ifndef QSOCK_MONKEYMODE
	// Based on code by Adam Banko:
	//#include "external/byteorder.h"

	// If we're compiling for a processor that's big-endian, disregard these commands:
	#ifdef WORDS_BIGENDIAN
		#define htons(x) (x)
		#define ntohs(x) (x)
		#define htonl(x) (x)
		#define ntohl(x) (x)
		#define htonf(x) (x)
		#define ntohf(x) (x)
		#define htond(x) (x)
		#define ntohd(x) (x)
	#else
		#ifdef PHP_WIN32
			#ifndef WINNT
				#define WINNT 1
			#endif
		#endif

		#ifndef htonf
			union float_long
			{
				QSOCK_FLOAT32 f;
				QSOCK_UINT32_LONG l;
			};

			inline QSOCK_UINT32_LONG htonf(QSOCK_FLOAT32 x)
			{
				union float_long fl;

				fl.f = x;

				return htonl(fl.l);
			}

			inline QSOCK_FLOAT32 ntohf(QSOCK_UINT32_LONG x)
			{
				union float_long fl;

				fl.l = ntohl(x);

				return fl.f;
			}
		#endif

		#ifndef htond
			#ifdef LINUX
				// Includes:
				#include <asm/byteorder.h>
					
				#define htond(x) __arch__swab64(x)
				#define ntohd(x) __arch__swab64(x)
			#else
				inline static QSOCK_FLOAT64 safe_swab64(QSOCK_FLOAT64 in)
				{
					QSOCK_FLOAT64 out;

					qchar* inP  = (qchar*)&in;
					qchar* outP = ((qchar*)&out) + sizeof(QSOCK_FLOAT64);

					for (uqint i=0; i< sizeof(QSOCK_FLOAT64); i++)
					{
						*(inP++) = *(--outP);
					}

					return out;
				}

				#define htond(x) safe_swab64(x)
				#define ntohd(x) safe_swab64(x)
			#endif
		#endif
	#endif
	//#endif
#endif

// Compatibility definitions:

// Just for the sake of laziness, we're keeping the WinSock naming-scheme for errors:
#ifndef WSAENETRESET
	#define WSAENETRESET 10052
#endif

#ifndef WSAECONNRESET
	#define WSAECONNRESET 10054
#endif

#ifndef INET_ADDRSTRLEN
	#define INET_ADDRSTRLEN 22
#endif

#ifndef INET6_ADDRSTRLEN
	#define INET6_ADDRSTRLEN 65
#endif

#ifndef SOCKET_ERROR
	#define SOCKET_ERROR -1
#endif

#ifndef INVALID_SOCKET
	#define INVALID_SOCKET -1
#endif

// Namespace(s):
namespace quickLib
{
	namespace sockets
	{
		// Typedefs/Aliases:
		typedef unsigned short nativePort;

		#if !defined(QSOCK_MONKEYMODE)
			typedef std::string nativeString;
		#else
			typedef String nativeString;
		#endif

		typedef wchar_t Char;

		#if !defined(QSOCK_IPVABSTRACT)
			typedef

			#ifdef QSOCK_MONKEYMODE
				QSOCK_UINT32_LONG
			#else
				QSOCK_INT32
			#endif

			nativeIP;

			typedef nativeString nonNativeIP;
		#else
			typedef nativeString nativeIP;
			typedef QSOCK_UINT32_LONG nonNativeIP;
		#endif

		// The address used internally by 'QuickSock':
		typedef

		#if !defined(QSOCK_IPVABSTRACT)
			sockaddr_in
		#else
			sockaddr
		#endif

		socketAddress;

		// The type used for stream locations/offset.
		typedef size_t streamLocation;

		// Functions:

		// Check if the 64-bit byte-order commands are already supported, if not, implement them:
		#ifndef WORDS_BIGENDIAN
			#if defined(__APPLE__) && defined(__MACH__) || defined(QSOCK_WINDOWS_LEGACY) || defined(QSOCK_MONKEYMODE) && defined(CFG_GLFW_USE_MINGW)
				inline QSOCK_UINT64 htonll(QSOCK_UINT64 inInt)
				{
					// Check if we're running on a big-endian system,
					// even if the compiler didn't know if we were:
					if (isBigEndian())
						return inInt;

					// Local variable(s):

					// The 64-bit integer used as a return-value.
					QSOCK_UINT64 retVal = 0;

					uqchar* intToConvert = (qchar*)&inInt;
					uqchar* returnInt = (qchar*)&retVal;
	
					for (uqchar i = 0; i < 7; i++)
					{
						returnInt[i] = intToConvert[7-i];
					}

					return retVal;
				}
		
				inline QSOCK_UINT64 ntohll(QSOCK_UINT64 inInt)
				{
					// Check if we're running on a big-endian system,
					// even if the compiler didn't know if we were:
					if (isBigEndian())
						return inInt;

					// Local variable(s):

					// The 64-bit integer used as a return-value.
					QSOCK_UINT64 retVal = htonll(inInt);

					((uqchar*)&retVal)[0] = (((uqchar*)&inInt)[0] >> 1);
	
					return retVal;
				}
			#endif
		#endif
		
		// If we don't have prototypes for these byte-order related commands, declare them here:
		#ifndef WORDS_BIGENDIAN
			#if defined(QSOCK_WINDOWS_LEGACY) || defined(QSOCK_MONKEYMODE) && defined(CFG_GLFW_USE_MINGW)
				unsigned long htonf(float inFloat);
				float ntohf(unsigned long inFloat);
				unsigned long long htond(double inFloat);
				double ntohd(unsigned long long inFloat);
			#endif
		#endif

		inline bool isBigEndian()
		{
			#if defined(QSOCK_BIGENDIAN)
				return true;
			#else
				// Local variable(s):
				QSOCK_INT32 temp(1);

				return ((*(char*)&temp) != 1);
			#endif
		}

		template<typename T>
		inline void ZeroVar(T& X)
		{
			memset(&X, 0, sizeof(X));

			return;
		}

		// Classes:
		#ifndef QSOCK_MONKEYMODE
		class DLLExport QSocket
		{
		#else
		class QSocket : public Object
		{
		#endif
			public:
				// Enumerator(s):
				enum addressType : uqchar
				{
					ADDRESS_TYPE_INTEGER = 0,
					ADDRESS_TYPE_STRING = 1,
				};

				// Global variables:

				// These variables dictate the amount of time spent detecting incoming packets:
				static const QSOCK_INT32_LONG TIMEOUT_USEC = 15;
				static const QSOCK_INT32_LONG TIMEOUT_SEC = 0;

				// The default max buffer-length for sockets.
				static const size_t DEFAULT_BUFFERLEN = 1024;

				// A global boolean stating if sockets have been initialized or not.
				static bool socketsInitialized;

				// All meta-data specified by WinSock upon initialization (Windows only).
				#if defined(QSOCK_WINDOWS)
					static WSADATA* WSA_Data;
				#endif

				// Functions:

				// This will transfer the contents of 'source' to 'destination', using the sub-script operator.
				template<typename TypeA, typename TypeB = TypeA>
				static size_t smartTransfer(TypeA source, TypeB destination, size_t count, size_t sourceOffset = 0, size_t destinationOffset = 0)
				{
					for (uqint i = 0; i < count; i++)
						source[i + sourceOffset] = destination[i + destinationOffset];

					return count;
				}

				// The return value of this command is the number of bytes transferred.
				static size_t rawTransfer(const void* source, void* destination, size_t count, size_t sourceOffsetInBytes=0, size_t destinationOffsetInBytes=0)
				{
					memcpy((uqchar*)destination + destinationOffsetInBytes, (const uqchar*)source + sourceOffsetInBytes, count);

					return count;
				}

				static size_t lengthOfString(nativeString s)
				{
					#if !defined(QSOCK_MONKEYMODE)
						return s.size();
					#else
						return (size_t)s.Length();
					#endif
				}

				static size_t lengthOfString(std::wstring wstr)
				{
					return wstr.length();
				}

				// The real IP conversion commands:
				#ifdef QSOCK_MONKEYMODE
					static QSOCK_INT32 StringToIntIP(nativeString IP);
					static nativeString IntToStringIP(QSOCK_INT32 IP);
				#else
					static QSOCK_UINT32_LONG StringToIntIP(nativeString IP);
					static nativeString IntToStringIP(QSOCK_UINT32_LONG IP);
				#endif

				// These commands are here for the sake of compatibility:
				static inline QSOCK_UINT32_LONG StringToIntIP(QSOCK_UINT32_LONG IP)
				{
					// Return the input.
					return IP;
				}

				static inline nativeString IntToStringIP(nativeString IP)
				{
					// Return the input.
					return IP;
				}

				static inline nonNativeIP nativeToNonNativeIP(nativeIP input)
				{
					#if !defined(QSOCK_IPVABSTRACT)
						return IntToStringIP(input);
					#else
						return StringToIntIP(input);
					#endif
				}

				static inline nativeIP nonNativeToNativeIP(nonNativeIP input)
				{
					#if !defined(QSOCK_IPVABSTRACT)
						return StringToIntIP(input);
					#else
						return IntToStringIP(input);
					#endif
				}

				// These commands are here for the sake of compatibility:
				static inline nativeIP nonNativeToNativeIP(nativeIP input)
				{
					return input;
				}

				static inline nonNativeIP nativeToNonNativeIP(nonNativeIP input)
				{
					return input;
				}

				// Constructors & Destructors:
				QSocket(size_t bufferLength=0, bool fixByteOrder=true);
				~QSocket();

				// Fields (Public):

				// The input and output buffers:
				uqchar* inbuffer;
				uqchar* outbuffer;

				// The length variables for each buffer:
				size_t _bufferlen;
				size_t inbufferlen;
				size_t outbufferlen;

				// The read & write offsets:
				streamLocation readOffset;
				streamLocation writeOffset;

				// The destinations used for sending and receiving packets.
				socketAddress si_Destination, so_Destination;

				bool fixByteOrder;

				// Functions (Public):
				static bool initSockets(size_t bufferlen=DEFAULT_BUFFERLEN);
				static bool deinitSockets();

				// Methods (Public):

				// General methods:
				inline bool isServer() const { return _isServer; }
				inline bool isClosed() const { return socketClosed; }

				// Initialization related:
				#if !defined(QSOCK_MONKEYMODE)
					bool connect(nativeString address, const nativePort ePort, nativePort iPort=(nativePort)0);
				#else
					bool connect(nativeString address, const QSOCK_INT32 externalPort, const QSOCK_INT32 internalPort);
				#endif

				// Platform specific arguments:
				#if !defined(QSOCK_MONKEYMODE)
					// Return type/other ('connect').
					inline bool connect(QSOCK_INT32_LONG address, const nativePort ePort, const nativePort iPort=(nativePort)0)
					{
						return connect(IntToStringIP(ntohl(htonl((QSOCK_UINT32_LONG)address))), (nativePort)ePort, (nativePort)iPort);
					}
				#endif

				// Return type/other (directConnect).
				inline bool

				// Platform specific arguments:
				#if defined(QSOCK_MONKEYMODE)
					directConnect(QSOCK_INT32 address, const QSOCK_INT32 ePort, const QSOCK_INT32 iPort
				#else
					directConnect(QSOCK_INT32_LONG address, const nativePort ePort, const nativePort iPort
				#endif

				=(nativePort)0)
				{
					return connect(address, (nativePort)ePort, (nativePort)iPort);
				}

				bool host
				(
					#if defined(QSOCK_MONKEYMODE)
						const QSOCK_INT32 ePort
					#else
						const nativePort ePort
					#endif
				);

				// Update related:

				// These four do the same thing:
				inline QSOCK_INT32 updateSocket() { return listenSocket(); }
				inline QSOCK_INT32 update() { return listenSocket(); }
				inline QSOCK_INT32 listen() { return listenSocket(); }
				inline QSOCK_INT32 socketListen() { return listenSocket(); }

				QSOCK_INT32 listenSocket();

				inline QSOCK_INT32 hostUpdate();
				inline QSOCK_INT32 clientUpdate();

				// Internal (Don't use this unless you want platform dependant access):
				// This function is subject to change in the future.
				#if defined(QSOCK_WINDOWS)
					inline SOCKET* getSocket() { return &_socket; }
				#else
					inline unsigned int getSocket() { return _socket; }
				#endif

				// Input related:
				QSOCK_INT32 readAvail();
				QSOCK_INT32 readMsg();

				bool clearInBuffer();

				inline bool msgAvail() const
				{
					return ((inbufferlen-readOffset) > 0);
				}

				inline nativeIP msgIP() const
				{
					return nativeMsgIP();
				}

				nativeString strMsgIP() const;
				QSOCK_UINT32_LONG intMsgIP() const;

				inline nativeIP nativeMsgIP() const
				{
					#if !defined(QSOCK_IPVABSTRACT)
						return intMsgIP();
					#else
						return strMsgIP();
					#endif
				}

				inline nonNativeIP nonNativeMsgIP()
				{
					#if !defined(QSOCK_IPVABSTRACT)
						return strMsgIP();
					#else
						return intMsgIP();
					#endif
				}

				nativePort msgPort() const;

				// Return type/other ('msgAddr'):
				inline socketAddress msgAddr() const
				{
					return si_Destination;
				}

				// General purpose:
				inline bool canRead(size_t count) const
				{
					return ((count != 0) && (((size_t)readOffset) + count) <= inbufferlen);
				}

				inline bool canWrite(size_t count) const
				{
					return ((count != 0) && (((size_t)writeOffset) + count) <= _bufferlen);
				}

				inline bool canRead() const
				{
					return (readOffset < inbufferlen);
				}

				inline bool canWrite() const
				{
					return (writeOffset < _bufferlen);
				}

				size_t inBytesLeft() const
				{
					return (inbufferlen - (size_t)readOffset);
				}

				size_t outBytesLeft() const
				{
					return (_bufferlen - (size_t)writeOffset);
				}

				// This command will not "zero out" the internal input-buffer.
				// Instead, it will simply act as if the buffer doesn't contain anything.
				void flushInput()
				{
					inbufferlen = 0;
					readOffset = 0;

					return;
				}

				// This command will not "zero out" the internal output-buffer.
				// Instead, it will simply act as if the buffer doesn't contain anything.
				void flushOutput()
				{
					outbufferlen = 0;
					writeOffset = 0;

					return;
				}

				// This command will safely flush a region of the input-buffer.
				// This is useful when seeking around the input "stream".
				inline void flushInputRegion(streamLocation position, size_t amount)
				{
					if (position+amount > inbufferlen)
						amount = (inbufferlen-position);

					memset(inbuffer + position, 0, amount);

					return;
				}

				// This will "flush" the amount specified, starting at the current input-offset.
				inline void flushInputRegion(size_t amount)
				{
					memset(inbuffer + readOffset, 0, amount);

					return;
				}

				// This will flush all bytes following the current read-offset.
				inline void flushInputRegion()
				{
					flushInputRegion(inbufferlen-readOffset);

					return;
				}

				// This will set the input-length to the length specified.
				// The input-offset will be changed to the
				// length specified if it was out of bounds.
				inline void setInputLength(size_t length)
				{
					inbufferlen = min(length, inbufferlen);
					readOffset = min(readOffset, inbufferlen);

					return;
				}

				inline size_t inSeek(streamLocation position=0)
				{
					if (position > inbufferlen)
					{
						qthrow(QSOCK_SEEK_EXCEPTION(this, QSOCK_SEEK_EXCEPTION::seekMode::SEEK_MODE_IN, position));

						return 0;
					}

					// Assign the new read-offset, then return it.
					return readOffset = position;
				}

				inline streamLocation inSeekForward(streamLocation bytesForward)
				{
					return inSeek(readOffset+bytesForward);
				}

				inline streamLocation inSeekBackward(streamLocation bytesBackward)
				{
					return inSeek(readOffset-bytesBackward);
				}

				// This command resets the read-offset to the default/zero.
				inline void resetRead() { inSeek(0); return; }

				inline streamLocation outSeek(streamLocation position)
				{
					if (position > _bufferlen)
					{
						qthrow(QSOCK_SEEK_EXCEPTION(this, QSOCK_SEEK_EXCEPTION::seekMode::SEEK_MODE_OUT, position));

						return 0;
					}

					// Assign the new write-offset, then return it:
					writeOffset = position;

					outbufferlen = max(outbufferlen, writeOffset);

					return writeOffset;
				}

				inline streamLocation outSeekForward(streamLocation bytesForward)
				{
					return outSeek(writeOffset+bytesForward);
				}

				inline streamLocation outSeekBackward(streamLocation bytesBackward)
				{
					return outSeek(writeOffset-bytesBackward);
				}

				// This command resets the write-offset to the default/zero.
				inline void resetWrite() { outSeek(0); return; }

				bool readData(void* output, size_t size, size_t output_offset = 0, bool checkRead = true);

				// This command will read the type specified, but it will not
				// automatically swap the appropriate bytes of the type.
				// To do this, please use the 'read' command.
				template<typename type>
				inline type rawRead()
				{
					type data;
			
					readData(&data, sizeof(type));

					return data;
				}

				template<> inline QSOCK_UCHAR rawRead<QSOCK_UCHAR>()
				{
					#ifndef QSOCK_THROW_EXCEPTIONS
						if (canRead(sizeof(QSOCK_UCHAR)))
					#endif
						{
							// Hold the current read-offset.
							auto currentOffset = readOffset;

							// Attempt to seek forward.
							inSeekForward(sizeof(QSOCK_UCHAR));

							// Return the requested information.
							return (QSOCK_UCHAR)inbuffer[currentOffset];
						}

					return 0;
				}

				template<> inline QSOCK_CHAR rawRead<QSOCK_CHAR>()
				{
					return (QSOCK_CHAR)rawRead<QSOCK_UCHAR>();
				}

				template<> inline bool rawRead<bool>()
				{
					return ((rawRead<QSOCK_UCHAR>() != 0) ? true : false);
				}

				// This acts as an automatic reading command for the type specified.
				// When retrieving the data requested, the proper byte-order command will be used.
				// If the speicfied type isn't directly supported, it will not be automatically byte-swapped.
				template<typename type> inline type read()
				{
					return rawRead<type>();
				}

				template<> inline QSOCK_UINT16 read<QSOCK_UINT16>() { return (fixByteOrder) ? ntohs(rawRead<QSOCK_UINT16>()) : rawRead<QSOCK_UINT16>(); }
				template<> inline QSOCK_INT16 read<QSOCK_INT16>() { return (QSOCK_INT16)read<QSOCK_UINT16>(); }

				template<> inline QSOCK_UINT32_LONG read<QSOCK_UINT32_LONG>()
				{
					return ((fixByteOrder) ? ntohl(rawRead<QSOCK_UINT32_LONG>()) : rawRead<QSOCK_UINT32_LONG>());
				}

				template<> inline QSOCK_INT32_LONG read<QSOCK_INT32_LONG>() { return (QSOCK_INT32_LONG)read<QSOCK_UINT32_LONG>(); }

				template<> inline QSOCK_UINT32 read<QSOCK_UINT32>() { return (QSOCK_UINT32)read<QSOCK_UINT32_LONG>(); }
				template<> inline QSOCK_INT32 read<QSOCK_INT32>() { return (QSOCK_INT32)read<QSOCK_INT32_LONG>(); }

				template<> inline QSOCK_UINT64 read<QSOCK_UINT64>() { return (fixByteOrder) ? ntohll(rawRead<QSOCK_UINT64>()) : rawRead<QSOCK_UINT64>(); }

				template<> inline QSOCK_INT64 read<QSOCK_INT64>() { return (QSOCK_INT64)read<QSOCK_UINT64>(); }

				template<> inline QSOCK_FLOAT32 read<QSOCK_FLOAT32>() { return (fixByteOrder) ? ntohf(rawRead<QSOCK_UINT32>()) : rawRead<QSOCK_FLOAT32>(); }
				template<> inline QSOCK_FLOAT64 read<QSOCK_FLOAT64>() { return (fixByteOrder) ? ntohd(rawRead<QSOCK_UINT64>()) : rawRead<QSOCK_FLOAT64>(); }

				template<> inline nativeString read<nativeString>()
				{
					// Read the string's length, then the string itself.
					return readNativeString(readLengthOfString());
				}

				template<> inline std::wstring read<std::wstring>()
				{
					auto length = readLengthOfString();
					auto str = std::wstring((const wchar_t*)simulatedReadBytes(length*sizeof(std::wstring::value_type)), length);

					return str;
				}

				// Type-specific macros:
				inline uqchar readOct() { return read<uqchar>(); }
				inline uqchar readByte() { return readOct(); }
				inline qchar readChar() { return read<qchar>(); }
				inline bool readBool() { return read<bool>(); }

				inline qshort readShort() { return read<qshort>(); }
				inline qint readInt() { return read<qint>(); }
				inline qlong readLong() { return read<qlong>(); }

				inline qfloat readFloat() { return read<qfloat>(); }
				inline qdouble readDouble() { return read<qdouble>(); }

				inline nativePort readPort() { return read<nativePort>(); }

				inline size_t readLengthOfString()
				{
					return (size_t)read<uqshort>();
				}

				inline nativeIP readIP()
				{
					switch (read<addressType>())
					{
						case ADDRESS_TYPE_INTEGER:
							#if !defined(QSOCK_IPVABSTRACT)
								return read<nativeIP>();
							#else
								return nonNativeToNativeIP(read<nonNativeIP>());
							#endif
						case ADDRESS_TYPE_STRING:
							#if !defined(QSOCK_IPVABSTRACT)
								return nonNativeToNativeIP(readString());
							#else
								return (nativeIP)readString();
							#endif
					}

					// If all else fails, return a blank IP address.
					return nativeIP();
				}

				// This overload returns 'true' if operations were successful.
				// The 'checkRead' argument is generally reserved, and should not be used externally.
				// This is not enforced, but please take safety into account before using it.
				bool UreadBytes(uqchar* output, uqint count, size_t output_offset = 0, bool checkRead = true);

				// This overload of 'UreadBytes' produces a new 'uqchar' array, this array should be managed/deleted by the caller.
				// If an array was not produced, reading could not be done.
				uqchar* UreadBytes(uqint count=0, bool zero_ended=false);

				// Like 'UreadBytes', 'readBytes' produces a new 'qchar' array, and should be managed/deleted by the caller.
				qchar* readBytes(size_t count=0, bool zero_ended=false) { return (qchar*)UreadBytes(count, zero_ended); }

				inline bool readBytes(uqchar* output, uqint count, size_t output_offset = 0)
				{
					return UreadBytes(output, count, output_offset);
				}

				/*
					Simulated reading should only be done with full understanding
					that the memory provided may not be modified or deleted.
					Only use these commands for temporary representation of data.
					
					If reading can not be done, unlike a normal read,
					these will simply provide 'nullptr' as a return value.
					
					DO NOT perform any reading operations which may mutate the data pointed to.
					This mainly applies to checking for messages, and similar routines.
					Seeking should work without any problems.
				*/

				inline uqchar* simulatedUReadBytes(uqint count=0)
				{
					// Check for errors:
					if (count == 0 && !canRead(inBytesLeft()) || !canRead(count))
						return nullptr;

					uqchar* data = (uqchar*)(inbuffer+readOffset);

					// Seek forward the amount we're going to "transfer".
					inSeekForward(count);

					return data;
				}

				// This command simply wraps 'simulatedUReadBytes',
				// please read that command's documentation.
				inline qchar* simulatedReadBytes(uqint count = 0)
				{
					return (qchar*)simulatedUReadBytes(count);
				}

				inline nativeString readNativeString(size_t length=0)
				{
					const char* characters = (char*)(inbuffer + readOffset);

					#if !defined(QSOCK_MONKEYMODE)
						return nativeString(characters, characters+length);
					#else
						return nativeString(characters, length);
					#endif
				}

				inline nativeString readString()
				{
					return read<nativeString>();
				}

				inline nativeString readString(size_t length)
				{
					return readNativeString(length);
				}

				inline std::wstring readWideString()
				{
					return read<std::wstring>();
				}
				
				// Line related:
				nativeString readLine();

				inline nativeString readstdLine() { return readLine(); }

				// Output related:

				// This command only works on servers. Clients automatically call 'sendMsg':
				QSOCK_INT32 broadcastMsg(nativePort port=(nativePort)0, bool resetLength=true);
				inline QSOCK_INT32 sendBroadcastMsg(nativePort port, bool resetLength=true) { return broadcastMsg(port, resetLength); }
		
				#if defined(QSOCK_MONKEYMODE)
					// Comments can be found below.
					QSOCK_INT32 sendMsg(QSOCK_INT32 IP, QSOCK_INT32 port=0, bool resetLength=true);
					QSOCK_INT32 sendMsg(nativeString IP, QSOCK_INT32 port=0, bool resetLength=true);
				#else
					// This overload is used for raw/native IPV4 addresses.
					QSOCK_INT32 sendMsg(QSOCK_UINT32_LONG IP, nativePort port=(nativePort)0, bool resetLength=true);

					// This overload is used for string IP addresses. (IPV4, IPV6)
					QSOCK_INT32 sendMsg(nativeString strIP, nativePort port=(nativePort)0, bool resetLength=true);
				#endif

				QSOCK_INT32 sendMsg(bool resetLength=true);

				QSOCK_INT32 sendMsg(socketAddress* outboundAddress, uqint addressOffset=0, bool resetLength=true)
				{
					if (!setupDestination(outboundAddress, addressOffset)) return SOCKET_ERROR;

					return outputMessage(resetLength);
				}

				// This command will manually set the output-offset, blanking memory as it becomes "out of scope".
				// The region between the current offset, and the position specified will be "zeroed out".
				// Use this with caution; use 'outSeek' if the intention is to normally "seek".
				inline streamLocation setWrite(streamLocation position)
				{
					position = min(position, _bufferlen);

					if (position > writeOffset)
						flushOutputRegion(position-writeOffset);
					else
						flushOutputRegion(position, (writeOffset-position));

					return outSeek(position);
				}

				// This command will "snap" the output-length to the current offset.
				// This does not have the extra overhead of blanking memory.
				// This is different from 'setWrite', which simply seeks to the specified position, then blanks memory.
				// Use this to "snap" the output-length to the current offset.
				// This command reserves the right to change the output-offset, if it is out of bounds.
				inline streamLocation snapWriteLength()
				{
					// Assign the output-buffer's length, then return it:
					writeOffset = min(writeOffset, outbufferlen);
					outbufferlen = writeOffset;

					return outbufferlen;
				}

				bool clearOutBuffer();

				// Buffer writing related:

				// This command will safely flush a region of the output-buffer.
				// This is useful when seeking around the output "stream".
				inline void flushOutputRegion(streamLocation position, size_t amount)
				{
					if (position+amount > _bufferlen)
						amount = (_bufferlen-position);

					memset(outbuffer + position, 0, amount);

					return;
				}

				// This will "flush" the amount specified, starting at the current write-offset.
				inline void flushOutputRegion(size_t amount)
				{
					memset(outbuffer + writeOffset, 0, amount);

					return;
				}

				// This will flush all bytes following the current write-offset.
				inline void flushOutputRegion()
				{
					flushOutputRegion(_bufferlen-writeOffset);

					return;
				}

				// This will set the output-length to the length specified.
				// The write-offset will be changed to the
				// length specified if it was out of bounds.
				inline void setOutputLength(streamLocation length)
				{
					outbufferlen = min(length, _bufferlen);
					writeOffset = min(writeOffset, outbufferlen);

					return;
				}

				// This will set the output-length to the current write-offset.
				inline void setOutputLength()
				{
					setOutputLength(writeOffset);

					return;
				}
		
				// The generic writing command for raw data. ('size' is in bytes)
				bool writeData(const void* input, size_t size, size_t input_offset=0);

				// This command will write the type specified, but it will not
				// automatically swap the appropriate bytes of the type.
				// To do this, please use the 'write' command.
				template<typename type>
				inline bool rawWrite(type data)
				{
					return writeData(&data, sizeof(type));
				}

				template<> inline bool rawWrite<QSOCK_UCHAR>(QSOCK_UCHAR data)
				{
					#ifndef QSOCK_THROW_EXCEPTIONS
						if (canWrite(sizeof(data)))
					#endif
						{
							// Hold the current output-offset.
							auto currentOffset = writeOffset;

							// Attempt to seek forward.
							outSeekForward(sizeof(data));

							// Copy the specified data.
							outbuffer[currentOffset] = (uqchar)data;

							// Tell the user writing was successful.
							return true;
						}

					return false;
				}

				template<> inline bool rawWrite<QSOCK_CHAR>(QSOCK_CHAR data)
				{
					return rawWrite<QSOCK_UCHAR>((QSOCK_UCHAR)data);
				}

				template<> inline bool rawWrite<bool>(bool data)
				{
					return rawWrite<QSOCK_UCHAR>((data) ? 1 : 0);
				}

				template<typename type> inline bool write(type data)
				{
					return rawWrite<type>(data);
				}

				template<> inline bool write<QSOCK_UINT16>(QSOCK_UINT16 data) { return (fixByteOrder) ? rawWrite<QSOCK_UINT16>(htons(data)) : rawWrite<QSOCK_UINT16>(data); }
				template<> inline bool write<QSOCK_INT16>(QSOCK_INT16 data) { return write<QSOCK_UINT16>((QSOCK_UINT16)data); }

				template<> inline bool write<QSOCK_UINT32_LONG>(QSOCK_UINT32_LONG data)
				{
					return ((fixByteOrder) ? rawWrite<QSOCK_UINT32_LONG>(htonl(data)) : rawWrite<QSOCK_UINT32_LONG>(data));
				}

				template<> inline bool write<QSOCK_INT32_LONG>(QSOCK_INT32_LONG data) { return write<QSOCK_UINT32_LONG>((QSOCK_UINT32_LONG)data); }

				template<> inline bool write<QSOCK_UINT32>(QSOCK_UINT32 data) { return write<QSOCK_UINT32_LONG>((QSOCK_UINT32_LONG)data); }
				template<> inline bool write<QSOCK_INT32>(QSOCK_INT32 data) { return write<QSOCK_INT32_LONG>((QSOCK_INT32_LONG)data); }

				template<> inline bool write<QSOCK_UINT64>(QSOCK_UINT64 data) { return rawWrite<QSOCK_UINT64>((fixByteOrder) ? htonll(data) : data); }
				template<> inline bool write<QSOCK_INT64>(QSOCK_INT64 data) { return write<QSOCK_UINT64>((QSOCK_UINT64)data); }

				template<> inline bool write<QSOCK_FLOAT32>(QSOCK_FLOAT32 data) { return (fixByteOrder) ? write<QSOCK_UINT32>(htonf(data)) : rawWrite<QSOCK_FLOAT32>(data); }
				template<> inline bool write<QSOCK_FLOAT64>(QSOCK_FLOAT64 data) { return (fixByteOrder) ? write<QSOCK_UINT64>(htond(data)) : rawWrite<QSOCK_FLOAT64>(data); }

				inline bool writeLengthOfString(size_t length)
				{
					return write<uqshort>((uqshort)length);
				}

				inline bool writeLengthOfString(nativeString s)
				{
					return writeLengthOfString(lengthOfString(s));
				}

				inline bool writeLengthOfString(std::wstring wstr)
				{
					return writeLengthOfString(lengthOfString(wstr));
				}

				inline bool write(nativeString str, size_t length)
				{
					// Write the string and its length:
					writeLengthOfString(length);

					return writeNativeString(str.c_str(), (uqint)length);
				}

				template<> inline bool write<nativeString>(nativeString str)
				{
					return write(str, lengthOfString(str));
				}

				inline bool write(std::wstring wstr, size_t length)
				{
					writeLengthOfString(length);

					return writeData(wstr.c_str(), length*sizeof(std::wstring::value_type));
				}

				template<> inline bool write<std::wstring>(std::wstring wstr)
				{
					return write(wstr, lengthOfString(wstr));
				}

				inline bool writeIP(QSOCK_UINT32_LONG address)
				{
					write<addressType>(ADDRESS_TYPE_INTEGER);

					return write<QSOCK_UINT32_LONG>(address);
				}

				inline bool writeIP(nativeString address)
				{
					write<addressType>(ADDRESS_TYPE_STRING);
					return writeString(address);
				}

				inline bool writePort(nativePort port)
				{
					return write<nativePort>(port);
				}

				inline bool writeOct(uqchar data) { return write<uqchar>(data); }
				inline bool writeByte(uqchar data) { return writeOct(data); }
				inline bool writeChar(qchar data) { return write<qchar>(data); }
				inline bool writeBool(bool data) { return write<bool>(data); }

				inline bool writeShort(qshort data) { return write<qshort>(data); }
				inline bool writeInt(qint data) { return write<qint>(data); }
				inline bool writeLong(qlong data) { return write<qlong>(data); }

				inline bool writeFloat(qfloat data) { return write<qfloat>(data); }
				inline bool writeDouble(qdouble data) { return write<qdouble>(data); }
		
				bool UwriteBytes(const uqchar* data, uqint dataSize=0);
				bool writeBytes(const qchar* data, uqint dataSize=0);

				// Other types:

				// This does not serialize the length of the string, to do this, either use 'writeString', or use 'writeLine'.
				#if !defined(QSOCK_MONKEYMODE)
					inline bool writeNativeString(nativeString str) { return writeBytes((const qchar*)str.c_str(), str.length()); }
				#else
					inline bool writeNativeString(String s) { return writeBytes((const qchar*)s.ToCString<qchar>(), s.Length()); }
				#endif

				inline bool writeNativeString(const QSOCK_CHAR* str, uqint length=0) { return writeBytes((const qchar*)str, (length != 0) ? length : strlen(str)); }

				inline bool writeString(nativeString str)
				{
					return write<nativeString>(str);
				}

				inline bool writeString(nativeString str, size_t length)
				{
					return write(str, length);
				}

				inline bool writeWideString(std::wstring wstr)
				{
					return write<std::wstring>(wstr);
				}

				inline bool writeWideString(std::wstring wstr, size_t length)
				{
					return write(wstr, length);
				}

				// Standard-line related:
				bool writeLine(const QSOCK_CHAR* strIn, uqint length=0);

				inline bool writeLine(std::string str) { return writeLine(str.c_str(), str.length()); }
				inline bool writestdLine(std::string str) { return writeLine(str); }

				#if defined(QSOCK_MONKEYMODE)
					inline bool writeLine(String s)
					{
						return writeLine(s.ToCString<QSOCK_CHAR>(), (uqint)s.Length());
					}
				#endif

				// The rest of the methods:
				bool closeSocket();
				bool close(QSOCK_INT32 nothing=0); // The 'nothing' argument was added due to problems with Unix sockets.
		
				// Monkey garbage collection and debugging related:
				#if defined(QSOCK_MONKEYMODE)
					QSocket* m_new()
					{
						#if defined(CFG_QSOCK_DEBUG_ENABLED)
							DBG_ENTER("Native: QuickSock")
							//DBG_LOCAL((QSocket*)this, "Self")
							DBG_INFO("Unable to find location.");
						#endif
 
						return this;
					}

					void mark() { Object::mark(); return; }

					#if defined(CFG_QSOCK_DEBUG_ENABLED)
						nativeString debug()
						{
							nativeString t = L"(QSocket)\n";

							// Other:
							t += dbg_decl("Internal_Socket", (int*)&_socket);
							//t += dbg_decl("Port", &port);

							// Offsets:
							t += dbg_decl("ReadOffset", &readOffset);
							t += dbg_decl("WriteOffset", &writeOffset);

							// Flags/Booleans:
							/*
							t += dbg_decl("IsServer", &_isServer);
							t += dbg_decl("SocketClosed", &socketClosed);
							t += dbg_decl("Manually_Deleted", &manualDelete);
							t += dbg_decl("Broadcast_Supported", broadcastSupported);
							*/

							// Lengths:
							t += dbg_decl("In_Buffer_Length", &inbufferlen);
							t += dbg_decl("Out_Buffer_Length", &outbufferlen);
							t += dbg_decl("Maximum_Buffer_Length", &_bufferlen);

							return t;
						}

						nativeString dbg_type(QSocket**s) { return "QSocket"; }
					#endif
				#endif

				// Operators (This class is still not standard I/O stream compliant):
				template<typename T>
				inline const QSocket& operator<<(T data)
				{
					write<T>(data);

					return *this;
				}

				template<typename T>
				inline const QSocket& operator>>(T& outputVariable)
				{
					outputVariable = read<T>();

					return *this;
				}
			private:
				// Methods (Private):
				bool setupDestinationV4(QSOCK_UINT32_LONG address, nativePort externalPort);
				bool setupDestination(std::string address, nativePort externalPort);

				inline bool setupDestination(QSOCK_UINT32_LONG address, nativePort externalPort)
				{
					return setupDestinationV4(address, externalPort);
				}

				inline bool setupDestination(socketAddress* outboundAddress, uqint addressOffset=0)
				{
					static const size_t socketAddress_Length = sizeof(socketAddress);

					memcpy(&so_Destination, outboundAddress+socketAddress_Length, socketAddress_Length);

					// Return the default response.
					return true;
				}

				inline bool setupDestination()
				{
					return setupDestination(msgIP(), msgPort());
				}

				// When calling this method, or calling any of the general purpose send
				// methods, the output-offset will be restored to its default position.
				QSOCK_INT32 outputMessage(bool resetLength=true);

				void setTimeValues(bool init=false);

				// Update methods:
				// Nothing so far.

				// Fields (Private):
				// Nothing so far.
			protected:
				// Global variable(s) (Protected):

				// The 'file-descriptor set' used for check-timeouts.
				static fd_set fd;

				// Constructors & Destructors:
				bool setupObject(size_t bufferLength=0, bool fixByteOrder=true);
				bool freeObject();

				// Methods (Protected):

				// Initialization related:
				bool bindSocket(const nativePort port);
				qint bindInternalSocket(qint filter=-1);

				// Deinitialization related:
				inline qint shutdownInternalSocket()
				{
					qint response;

					// Shutdown the internal socket.
					response = shutdown(_socket, SD_BOTH);

					if (response == 0)
					{
						// Set the internal socket to an "invalid socket".
						_socket = INVALID_SOCKET;
					}

					return response;
				}

				// Fields (Protected):

				// The final address-result(s):

				// 'boundAddress' points to the element of 'result' which is used to bind the internal socket.
				#if !defined(QSOCK_IPVABSTRACT)
					socketAddress * result, * boundAddress;
				#else
					addrinfo * result, * boundAddress;
				#endif

				// The hints used to evaluate addresses (Protocols, 'IP families', etc).
				addrinfo hints;

				// Internal socket:
				#if defined(QSOCK_WINDOWS)
					SOCKET _socket;
				#else
					unsigned int _socket;
				#endif
		
				// Time-related field(s):
				timeval tv;

				// The connection port (Host: Local/Server port, Client: External / Remote server's port).
				nativePort port;

				// Booleans / Flags:

				// Currently doesn't do much, and it's inaccessible in most places.
				bool manualDelete;

				// A boolean describing the connection-mode; server or client.
				bool _isServer;

				// A simple boolean for the state of the socket.
				bool socketClosed;
				bool broadcastSupported;

				// Other:

				/*
				nativePort outputPort;
				nativeIP outputIP;
				*/
		};
	}
}