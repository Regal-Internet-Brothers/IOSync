#pragma once

/*
	This provides exception-types which deal with the 'application' and 'iosync_application' classes.

	For exceptions specific to 'application', please view the 'application_exceptions.h' header file.
*/

// Includes:
#include "exceptions.h"

#include "application/application_exceptions.h"

// Namespace(s):
namespace iosync
{
	namespace exceptions
	{
		namespace applicationExceptions
		{
			// Classes:
			class noWindowException final : public applicationException
			{
				public:
					// Constructor(s):
					noWindowException(application* program);

					// Methods:
					virtual const string message() const throw() override;
			};
		}
	}
}