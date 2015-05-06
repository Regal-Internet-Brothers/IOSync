// Includes:
#include "util.h"

#ifdef IOSYNC_ADVANCED_STRING_CODECS
	#include <codecvt>
#endif

// Namespace(s):
namespace iosync
{
	// Functions:
	string wideStringToDefault(const wstring& wstr)
	{
		#ifdef IOSYNC_ADVANCED_STRING_CODECS
			wstring_convert<codecvt_utf8<wchar_t>> stringConverter;

			return stringConverter.to_bytes(wstr);
		#else
			return string(wstr.begin(), wstr.end());
		#endif
	}

	wstring defaultStringToWide(const string& str)
	{
		#ifdef IOSYNC_ADVANCED_STRING_CODECS
			wstring_convert<codecvt_utf8<wchar_t>> stringConverter;

			return stringConverter.from_bytes(str);
		#else
			return wstring(str.begin(), str.end());
		#endif
	}
}