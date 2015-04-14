#pragma once

// Includes:
#include <chrono>

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
}