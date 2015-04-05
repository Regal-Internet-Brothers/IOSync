// Includes:
#include "exceptions.h"

// Namespace(s):
namespace iosync
{
	namespace exceptions
	{
		// Classes:

		// iosync_exception:

		// Constructor(s):
		iosync_exception::iosync_exception() : exception()
		{
			// Nothing so far.
		}

		// Methods:
		const char* iosync_exception::what() const throw()
		{
			return message().c_str();
		}
	}
}