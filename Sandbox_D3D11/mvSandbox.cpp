#include "mvSandbox.h"
#include <assert.h>

extern mvContext* GContext = nullptr;

void 
create_context()
{
	if (GContext)
	{
		assert(false && "Context already created.");
		return;
	}

	GContext = new mvContext();
}

void 
destroy_context()
{
	if (GContext)
	{
		delete GContext;
		GContext = nullptr;
		return;
	}

	assert(false && "Context already destroyed.");
}
