#pragma once

#include "link.h"

namespace util
{
	template <class T, Link T::*LINK>
	class List
	{
	public:

		List()
		{
		}

		~List()
		{
		}

		void insert(T *t)
		{
			Link *const link = t_to_link(t);
		}

	private:

		static Link *t_to_link(T *t)
		{
			return &(t->*LINK);
		}

		static T *link_to_t(Link *t);

		List(const List &);
		List &operator=(const List &);
	};
}
