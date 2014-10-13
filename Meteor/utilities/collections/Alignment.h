// alignof library

// Copyright (C) 2003 Martin Buchholz
//
// Permission to copy, use, sell and distribute this software is granted
// provided this copyright notice appears in all copies.
// Permission to modify the code and to distribute modified code is granted
// provided this copyright notice appears in all copies, and a notice
// that the code was modified is included with the copyright notice.
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.
//

// ----------------------------------------------------------------
// This code is known to work with the following compilers:
// Linux x86 g++ 2.95
// Linux x86 g++ 3.2.1
// Linux x86 g++ 3.3 20030122 pre-release
// Linux x86 g++ 3.4 20030122 pre-release
// Linux x86 Intel compiler Version 7.0
// Windows x86 MS Visual C++ .NET Version 13.00

// To work with MS Visual C++, we avoid partial template specialization.
// ----------------------------------------------------------------

#ifndef ALIGNOF_HPP_INCLUDED_
#define ALIGNOF_HPP_INCLUDED_

#if defined(__GNUC__)

#define ALIGNOF(type) __alignof__(type)

#else

namespace Alignment
{
	// Implementing alignof:
	// We compute alignof using two separate algorithms, then take their min.

	namespace ffs
	{
		// alignof (T) must be a power of two which is a factor of sizeof (T).
		template <typename T>
		struct alignof
		{
			// Most common programmer interview question!
			enum { s = sizeof (T), value = s ^ (s & (s - 1)) };
		};
	}

	namespace QuantumJump
	{
		// Put T in a struct, keep adding chars until a "quantum jump" in
		// the size occurs.
		template <typename T> struct alignof;

		template <int size_diff>
		struct helper
		{
			template <typename T> struct Val { enum { value = size_diff }; };
		};

		template <>
		struct helper<0>
		{
			template <typename T> struct Val { enum { value = alignof<T>::value }; };
		};

		template <typename T>
		struct alignof
		{
			struct Big { T x; char c; };

			enum { diff = sizeof (Big) - sizeof (T),
				value = helper<diff>::template Val<Big>::value };
		};

	} // QuantumJump

	template <typename T>
	struct alignof
	{
		enum { x = QuantumJump::alignof<T>::value,
			y = ffs::alignof<T>::value,
			value = x < y ? x : y };
	};
}

/* ALIGNOF (type)
Return alignment of TYPE. */
#define ALIGNOF(type) Alignment::alignof<type>::value

#endif

#endif
