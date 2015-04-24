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
		iosync_exception::iosync_exception(const string& exception_name) : runtime_error(exception_name)
		{
			// Nothing so far.
		}

		// Methods:
		// Nothing so far.
	}
}