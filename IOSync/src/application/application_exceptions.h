#pragma once

// Includes:
#include "../exceptions.h"

// Namespace(s):
namespace iosync
{
	// Classes:
	class application;

	// Namespace(s):
	namespace exceptions
	{
		namespace applicationExceptions
		{
			// Classes:
			class applicationException : public iosync_exception
			{
				public:
					// Constructor(s):
					applicationException(application* targetedProgram, const string& exception_name="IOSYNC: Application exception.");

					// Methods:
					virtual const string message() const throw() override = 0;

					// Fields:
					application* program;
			};
		}
	}
}