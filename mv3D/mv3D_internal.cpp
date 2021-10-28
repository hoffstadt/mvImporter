#include "mv3D_internal.h"
#include <assert.h>

extern mvContext* GContext = nullptr;

void 
mvCreateContext()
{
	if (GContext)
	{
		assert(false && "Context already created.");
		return;
	}

	GContext = new mvContext();
}

void 
mvDestroyContext()
{
	if (GContext)
	{
		delete GContext;
		GContext = nullptr;
		return;
	}

	assert(false && "Context already destroyed.");
}
