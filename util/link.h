#pragma once

#include "dll.h"

namespace util
{
	class Link
	{
	public:

		//UTIL_DLL
		 Link();
		//UTIL_DLL
		 ~Link();

		Link *previous;
		Link *next;

	private:

		Link(const Link &);
		Link &operator=(const Link &);
	};
}
