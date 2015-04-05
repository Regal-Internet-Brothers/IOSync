#pragma once

/* This provides a basic exception framework for the application. */

// Includes:
#include <exception>
#include <string>

// Namespace(s):
using namespace std;

namespace iosync
{
	namespace exceptions
	{
		// Classes:
		class iosync_exception : public exception
		{
			public:
				// Constructor(s):
				iosync_exception();

				// Methods:
				virtual const string message() const throw() = 0;

				// This currently acts as a standard-compliant wrapper for the 'message' command.
				virtual const char* what() const throw() override;

				// Fields:
				// Nothing so far.
		};
	}
}