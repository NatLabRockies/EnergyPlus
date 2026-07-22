#ifndef ObjexxFCL_AlignedAllocator_hh_INCLUDED
#define ObjexxFCL_AlignedAllocator_hh_INCLUDED

// Aligned Allocator
//
// Project: Objexx Fortran-C++ Library (ObjexxFCL)
//
// Version: 4.2.0
//
// Language: C++
//
// Copyright (c) 2000-2017 Objexx Engineering, Inc. All Rights Reserved.
// Use of this source code or any derivative of it is restricted by license.
// Licensing is available from Objexx Engineering, Inc.:  http://objexx.com

// ObjexxFCL Headers
#include <ObjexxFCL/align.hh>

// C++ Headers
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>

namespace ObjexxFCL {

// Aligned Allocator Class Template
template< typename T >
struct AlignedAllocator
{

public: // Types

	// STL Style
	using size_type = std::size_t;

	// C++ Style
	using Size = std::size_t;

public: // Static Methods

	// Raw allocation size including optional alignment padding
	static
	size_type
	allocation_size( size_type const n )
	{
#ifdef OBJEXXFCL_ALIGN
		constexpr size_type alignment_overhead( OBJEXXFCL_ALIGN > 0u ? OBJEXXFCL_ALIGN - 1u : 0u );
#else
		constexpr size_type alignment_overhead( 0u );
#endif
		constexpr size_type max_size( static_cast< size_type >( std::numeric_limits< std::ptrdiff_t >::max() ) );
		if ( n > ( max_size - alignment_overhead ) / sizeof( T ) ) {
			throw std::bad_array_new_length();
		}
		return ( n * sizeof( T ) ) + alignment_overhead;
	}

	// Allocate Raw Array Memory with ::operator new
	static
	void *
	allocate( size_type const n )
	{
		void * mem( n > 0u ? ::operator new( allocation_size( n ) ) : nullptr );
		assert( ( n == 0u ) || ( mem != nullptr ) );
		return mem;
	}

	// Allocate Raw Array Memory with ::operator new Even if Size is Zero
	static
	void *
	allocate_zero( size_type const n )
	{
		void * mem( ::operator new( allocation_size( n ) ) );
		assert( ( n == 0u ) || ( mem != nullptr ) );
		return mem;
	}

	// Aligned Data Pointer for a Given Memory Pointer
	static
	T *
	data( void * const mem )
	{
#ifdef OBJEXXFCL_ALIGN
		T * p( static_cast< T * >( ( mem == nullptr ) || ( OBJEXXFCL_ALIGN == 0u ) ? mem : reinterpret_cast< void * >( ( reinterpret_cast< std::uintptr_t >( mem ) + static_cast< std::uintptr_t >( OBJEXXFCL_ALIGN - 1 ) ) & ~static_cast< std::uintptr_t >( OBJEXXFCL_ALIGN - 1 ) ) ) );
		assert( p >= mem );
		assert( reinterpret_cast< std::uintptr_t >( p ) % OBJEXXFCL_ALIGN == 0 );
#else
		T * p( static_cast< T * >( mem ) );
#endif
		return p;
	}

}; // AlignedAllocator

} // ObjexxFCL

#endif // ObjexxFCL_AlignedAllocator_hh_INCLUDED
