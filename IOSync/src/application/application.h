#pragma once

// Preprocessor related:
#define WINDOW_NONE iosync::nativeWindow()

// Includes:
#include "application_exceptions.h"

#include "../platform.h"
#include "../networking/networking.h"

#include <vector>
#include <string>
#include <codecvt>

// Namespace(s):
using namespace iosync::networking;

namespace iosync
{
	// Forward declarations:
	namespace networking
	{
		// Structures:
		struct messageHeader;
		struct messageFooter;

		// Classes:
		class networkEngine;
	}

	// Typedefs:
	typedef unsigned short rate;

	typedef std::vector<std::wstring> arguments;

	// The type of 'nativeWindow' is platform-dependent,
	// and may not be needed for some platforms:
	#ifdef PLATFORM_WINDOWS
		typedef HWND nativeWindow;
	#else
		typedef void* nativeWindow;
	#endif

	// Structures:
	struct OSINFO
	{
		// Constructor(s):
		#ifdef PLATFORM_WINDOWS_EXTENSIONS
			OSINFO(HINSTANCE hInstance=HINSTANCE(), HINSTANCE hPrevInstance=HINSTANCE(), LPSTR lpCmdLine=LPSTR(), int nCmdShow=0);
		#else
			OSINFO(int argCount=0, char** argVector=nullptr);
		#endif

		// Fields:
		#ifdef PLATFORM_WINDOWS_EXTENSIONS
			HINSTANCE hInstance;
			HINSTANCE hPrevInstance;
			LPSTR lpCmdLine;
			int nCmdShow;
		#else
			int argc;
			char** argv;
		#endif
	};

	// Enumerator(s):
	enum rates : rate
	{
		DEFAULT_UPDATERATE = 60,
	};

	// Classes:
	class application
	{
		public:
			// Global variable(s):
			static std::wstring_convert<std::codecvt_utf8<wchar_t>> stringConverter;

			static std::wstring path;

			// Functions:
			string wideStringToDefault(const wstring wstr)
			{
				return stringConverter.to_bytes(wstr);
			}

			wstring defaultStringToWide(const string str)
			{
				return stringConverter.from_bytes(str);
			}

			// Constructor(s):
			application(rate updateRate = DEFAULT_UPDATERATE, OSINFO info=OSINFO());

			// Destructor(s):
			virtual ~application()
			{
				// Nothing so far.
			}

			// Methods:
			virtual int execute();

			// This method is abstract, and must be implemented.
			virtual void update(rate frameNumber=0) = 0;

			virtual nativeWindow getWindow() const;

			// Networking related:

			// Parsing/deserialization related:

			// This command provides message handling for 'application' objects.
			// Applications do not have to support networking, but a default implementation is provided.
			// This command may be used to virtually handle networking-messages from the perspective of an 'application'.
			virtual bool parseNetworkMessage(QSocket& socket, const messageHeader& header, const messageFooter& footer);

			// Call-backs:
			virtual void onNetworkConnected(networkEngine& engine);
			virtual void onNetworkClosed(networkEngine& engine);

			// Fields:

			// This acts as our standard argument container.
			arguments args;

			// The information supplied by the operating system.
			OSINFO OSInfo;

			// Rates:
			rate updateRate;

			// Booleans / Flags:
			bool isRunning;
	};
}