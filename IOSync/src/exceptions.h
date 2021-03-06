#pragma once

/* This provides a basic exception framework for the application. */

// Includes:
#include <stdexcept>
#include <string>

// Namespace(s):
namespace iosync
{
	namespace exceptions
	{
		// Namespace(s):
		using namespace std;

		// Classes:
		class iosync_exception : public runtime_error
		{
			public:
				// Constructor(s):
				iosync_exception(const string& exception_name="IOSYNC: Exception.");

				// Methods:
				virtual const string message() const throw() = 0;

				// Fields:
				// Nothing so far.
		};
	}
}