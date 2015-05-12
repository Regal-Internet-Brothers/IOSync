#pragma once

// Preprocessor related:
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
	#define IOSYNC_ADVANCED_STRING_CODECS
#endif

// Includes:
#include <chrono>
#include <string>

#include <algorithm>
#include <cctype>

// Namespace(s):
namespace iosync
{
	// Namespace(s):
	using namespace std;
	using namespace chrono;

	// Functions:

	// This command returns the number of milliseconds that have passed since 't' was updated last.
	static inline milliseconds elapsed(high_resolution_clock::time_point t)
	{
		return duration_cast<milliseconds>(high_resolution_clock::now() - t);
	}

	// String related (Basically what 'QuickINI' provides; may be changed later):
	string wideStringToDefault(const wstring& wstr);
	wstring defaultStringToWide(const string& str);

	inline void correctString(const string& str, wstring& output)
	{
		output = defaultStringToWide(str);

		return;
	}

	inline void correctString(const wstring& wstr, string& output)
	{
		output = wideStringToDefault(wstr);

		return;
	}

	inline void correctString(const string& str, string& output)
	{
		output = str;

		return;
	}

	inline void correctString(const wstring& wstr, wstring& output)
	{
		output = wstr;

		return;
	}

	template <typename inputStr, typename characterType, typename characterTraits = char_traits<characterType>, typename strAlloc = allocator<characterType>>
	inline basic_string<characterType, characterTraits, strAlloc> convertString(const inputStr& str)
	{
		basic_string<characterType, characterTraits, strAlloc> output;

		correctString(str, output);

		return output;
	}

	template <typename characterType, typename characterTraits = char_traits<characterType>, typename strAlloc = allocator<characterType>>
	inline basic_string<characterType, characterTraits, strAlloc> convertStringA(const char* str)
	{
		return convertString<const char*, characterType, characterTraits, strAlloc>(str);
	}

	inline string abstractStringToDefault(const string& str)
	{
		return str;
	}

	inline string abstractStringToDefault(const wstring& wstr)
	{
		return wideStringToDefault(wstr);
	}

	inline wstring abstractStringToWide(const wstring& wstr)
	{
		return wstr;
	}

	inline wstring abstractStringToWide(const string& str)
	{
		return defaultStringToWide(str);
	}

	template <typename characterType, typename characterTraits=char_traits<characterType>, typename strAlloc=allocator<characterType>>
	inline void transformToLower(basic_string<characterType, characterTraits, strAlloc>& out_str)
	{
		transform(out_str.begin(), out_str.end(), out_str.begin(), static_cast<int (*)(int)>(std::tolower));

		return;
	}

	template <typename characterType, typename characterTraits=char_traits<characterType>, typename strAlloc=allocator<characterType>>
	inline basic_string<characterType, characterTraits, strAlloc> toLower(const basic_string<characterType, characterTraits, strAlloc>& str)
	{
		auto output = str;

		transformToLower(output);

		return output;
	}
}