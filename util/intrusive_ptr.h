#pragma once

#include <assert.h>
#include <stdlib.h>

namespace util
{
	template <class T, void (*ADD_REF)(T *), void (*RELEASE)(T *)>
	class IntrusivePtr
	{
	public:

		// Default constructor.
		IntrusivePtr()
			: t(NULL)
		{
		}

		// Copy constructor.
		IntrusivePtr(const IntrusivePtr &other)
			: t(other.t)
		{
			// Add a reference.
			if (t != NULL)
			{
				(*ADD_REF)(t);
			}
		}

		// Construction from raw pointer.
		IntrusivePtr(T *t, bool add_ref)
			: t(t)
		{
			// Add a reference.
			if (add_ref && (t != NULL))
			{
				(*ADD_REF)(t);
			}
		}

		// Destructor.
		~IntrusivePtr()
		{
			reset();
		}

		// Conversion to bool.
		operator bool() const
		{
			return t != NULL;
		}

		// Assignment operator.
		IntrusivePtr &operator=(const IntrusivePtr &other)
		{
			// Add a reference to the new value.
			if (other.t != NULL)
			{
				(*ADD_REF)(other.t);
			}

			// Release the old value.
			if (t != NULL)
			{
				(*RELEASE)(t);
			}

			// Take over the new value.
			t = other.t;

			return *this;
		}

		// Dereference operator.
		T &operator*() const
		{
			// Sanity checks.
			assert(t != NULL);

			return *t;
		}

		// Dereference operator.
		T *operator->() const
		{
			// Sanity checks.
			assert(t != NULL);

			return t;
		}

		// Access.
		T *get() const
		{
			return t;
		}
		
		// Release.
		void reset()
		{
			if (t != NULL)
			{
				(*RELEASE)(t);
				t = NULL;
			}
		}

	private:

		T *t;
	};
}
