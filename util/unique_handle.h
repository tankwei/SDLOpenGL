#pragma once

namespace util
{
	template <class T, T NULL_VALUE, class U, void (*FREE)(U)>
	class UniqueHandle
	{
	public:

		// Default constructor.
		UniqueHandle()
			: t(NULL_VALUE)
		{
		}

		// Copy constructor.
		UniqueHandle(UniqueHandle &other)
			: t(other.t)
		{
			other.t = NULL_VALUE;
		}

		// Construction from raw handle.
		UniqueHandle(T t)
			: t(t)
		{
		}

		// Destructor.
		~UniqueHandle()
		{
			// Free the current value.
			if (t != NULL_VALUE)
			{
				(*FREE)(t);
			}
		}

		// Conversion to bool.
		operator bool() const
		{
			return t != NULL_VALUE;
		}

		// Assignment operator.
		UniqueHandle &operator=(UniqueHandle &other)
		{
			// Free the current value.
			if (t != NULL_VALUE)
			{
				(*FREE)(t);
			}

			// Take over the new value.
			t = other.t;
			other.t = NULL_VALUE;

			return *this;
		}

		// Access.
		T get() const
		{
			return t;
		}
		
		// Release.
		void reset()
		{
			// Free the current value.
			if (t != NULL_VALUE)
			{
				(*FREE)(t);
				t = NULL_VALUE;
			}
		}

	private:
		
		T t;
	};
}
