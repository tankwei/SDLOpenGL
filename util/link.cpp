#include "util/link.h"

#include <stdlib.h>

namespace util
{
	Link::Link()
		: previous(NULL),
		next(NULL)
	{
		previous = this;
		next = this;
	}

	Link::~Link()
	{
		// Link the previous and next links to each other.
		previous->next = next;
		next->previous = previous;
	}
}
