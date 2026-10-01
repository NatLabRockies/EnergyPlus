#ifndef ObjexxFCL_CArrayA_hh_INCLUDED
#define ObjexxFCL_CArrayA_hh_INCLUDED

// CArrayA: Memory-Managed C Array Wrapper with Alignment Support
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
#include <ObjexxFCL/CArrayA.fwd.hh>
#include <ObjexxFCL/AlignedAllocator.hh>
#include <ObjexxFCL/TypeTraits.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <iomanip>
#include <istream>
#include <iterator>
#include <new>
#include <ostream>
#include <type_traits>
#include <utility>

namespace ObjexxFCL {

// CArrayA: Memory-Managed C Array Wrapper with Alignment Support
template< typename T >
class CArrayA
{

private: // Friend

	template< typename > friend class CArrayA; // Friendship across value types

public: // Types

	typedef  AlignedAllocator< T >  Aligned;
	typedef  TypeTraits< T >  Traits;
	typedef  typename std::conditional< std::is_scalar< T >::value, T const, T const & >::type  Tc;
	typedef  typename std::conditional< std::is_scalar< T >::value, typename std::remove_const< T >::type, T const & >::type  Tr;

	// STL Style
	typedef  T  value_type;
	typedef  T &  reference;
	typedef  T const &  const_reference;
	typedef  T *  pointer;
	typedef  T const *  const_pointer;
	typedef  T *  iterator;
	typedef  T const *  const_iterator;
	typedef  std::reverse_iterator< T * >  reverse_iterator;
	typedef  std::reverse_iterator< T const * >  const_reverse_iterator;
	typedef  std::size_t  size_type;
	typedef  std::ptrdiff_t  difference_type;

	// C++ Style
	typedef  T  Value;
	typedef  T &  Reference;
	typedef  T const &  ConstReference;
	typedef  T *  Pointer;
	typedef  T const *  ConstPointer;
	typedef  T *  Iterator;
	typedef  T const *  ConstIterator;
	typedef  std::reverse_iterator< T * >  ReverseIterator;
	typedef  std::reverse_iterator< T const * >  ConstReverseIterator;
	typedef  std::size_t  Size;
	typedef  std::ptrdiff_t  Difference;

	// Types to prevent compile failure when std::distance is in scope
	typedef  void  iterator_category;

public: // Creation

	// Default Constructor
	CArrayA() :
	 size_( 0u ),
	 mem_( nullptr ),
	 data_( nullptr )
	{}

	// Copy Constructor
	CArrayA( CArrayA const & a ) :
	 size_( a.size_ ),
	 mem_( Aligned::allocate( size_ ) ),
	 data_( Aligned::data( mem_ ) )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( a.data_[ i ] );
		}
	}

	// Move Constructor
	CArrayA( CArrayA && a ) noexcept :
	 size_( a.size_ ),
	 mem_( a.mem_ ),
	 data_( a.data_ )
	{
		a.size_ = 0u;
		a.mem_ = a.data_ = nullptr;
	}

	// Copy Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	CArrayA( CArrayA< U > const & a ) :
	 size_( a.size_ ),
	 mem_( Aligned::allocate( size_ ) ),
	 data_( Aligned::data( mem_ ) )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( a.data_[ i ] );
		}
	}

	// Pointer + Size Constructor
	CArrayA(
	 T const * const p,
	 size_type const size
	) :
	 size_( size ),
	 mem_( Aligned::allocate( size_ ) ),
	 data_( Aligned::data( mem_ ) )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( p[ i ] );
		}
	}

	// Pointer + Size Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	CArrayA(
	 U const * const p,
	 size_type const size
	) :
	 size_( size ),
	 mem_( Aligned::allocate( size_ ) ),
	 data_( Aligned::data( mem_ ) )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( p[ i ] );
		}
	}

	// Iterator Range Constructor Template
	template< typename InputIterator >
	CArrayA(
	 InputIterator const beg,
	 InputIterator const end
	) :
	 size_( end - beg ),
	 mem_( Aligned::allocate( size_ ) ),
	 data_( Aligned::data( mem_ ) )
	{
		if ( size_ > 0u ) {
			InputIterator k( beg );
			for ( size_type i = 0; i < size_; ++i, ++k ) {
				new ( data_ + i ) T( *k );
			}
		}
	}

	// Size Constructor: Built-in types are default, not zero, initialized for performance
	explicit
	CArrayA( size_type const size ) :
	 size_( size ),
	 mem_( Aligned::allocate( size_ ) ),
	 data_( Aligned::data( mem_ ) )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T;
		}
	}

	// Size + Uniform Value Constructor
	CArrayA(
	 size_type const size,
	 Tc t
	) :
	 size_( size ),
	 mem_( Aligned::allocate( size_ ) ),
	 data_( Aligned::data( mem_ ) )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( t );
		}
	}

	// Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	CArrayA( std::initializer_list< U > const l ) :
	 size_( l.size() ),
	 mem_( Aligned::allocate( size_ ) ),
	 data_( Aligned::data( mem_ ) )
	{
		auto li( l.begin() );
		for ( size_type i = 0; i < size_; ++i, ++li ) {
			new ( data_ + i ) T( *li );
		}
	}

	// Destructor
	~CArrayA()
	{
		destroy();
	}

public: // Conversion

	// Active?
	operator bool() const
	{
		return ( data_ != nullptr );
	}

	// Data
	operator T const *() const
	{
		return data_;
	}

	// Data
	operator T *()
	{
		return data_;
	}

public: // Assignment

	// Copy Assignment
	CArrayA &
	operator =( CArrayA const & a )
	{
		if ( this != &a ) {
			if ( size_ != a.size_ ) {
				destroy();
				size_ = a.size_;
				mem_ = Aligned::allocate( size_ );
				data_ = Aligned::data( mem_ );
				for ( size_type i = 0; i < size_; ++i ) {
					new ( data_ + i ) T( a.data_[ i ] );
				}
			} else {
				for ( size_type i = 0; i < size_; ++i ) {
					data_[ i ] = a.data_[ i ];
				}
			}
		}
		return *this;
	}

	// Move Assignment
	CArrayA &
	operator =( CArrayA && a ) noexcept
	{
		assert( this != &a );
		destroy();
		size_ = a.size_;
		mem_ = a.mem_;
		data_ = a.data_;
		a.size_ = 0u;
		a.mem_ = a.data_ = nullptr;
		return *this;
	}

	// Copy Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	CArrayA &
	operator =( CArrayA< U > const & a )
	{
		if ( size_ != a.size_ ) {
			destroy();
			size_ = a.size_;
			mem_ = Aligned::allocate( size_ );
			data_ = Aligned::data( mem_ );
			for ( size_type i = 0; i < size_; ++i ) {
				new ( data_ + i ) T( a.data_[ i ] );
			}
		} else {
			for ( size_type i = 0; i < size_; ++i ) {
				data_[ i ] = T( a.data_[ i ] );
			}
		}
		return *this;
	}

	// Uniform Value Assignment
	CArrayA &
	operator =( Tc t )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] = t;
		}
		return *this;
	}

	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	CArrayA &
	operator =( std::initializer_list< U > const l )
	{
		assert( l.size() == size_ );
		std::copy( l.begin(), l.end(), data_ );
		return *this;
	}

	// Pointer + Size Assignment
	CArrayA &
	assign(
	 T const * const p,
	 size_type const size
	)
	{
		if ( size_ != size ) {
			destroy();
			size_ = size;
			mem_ = Aligned::allocate( size_ );
			data_ = Aligned::data( mem_ );
			for ( size_type i = 0; i < size_; ++i ) {
				new ( data_ + i ) T( p[ i ] );
			}
		} else {
			for ( size_type i = 0; i < size_; ++i ) {
				data_[ i ] = p[ i ];
			}
		}
		return *this;
	}

	// Pointer + Size Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	CArrayA &
	assign(
	 U const * const p,
	 size_type const size
	)
	{
		if ( size_ != size ) {
			destroy();
			size_ = size;
			mem_ = Aligned::allocate( size_ );
			data_ = Aligned::data( mem_ );
			for ( size_type i = 0; i < size_; ++i ) {
				new ( data_ + i ) T( p[ i ] );
			}
		} else {
			for ( size_type i = 0; i < size_; ++i ) {
				data_[ i ] = T( p[ i ] );
			}
		}
		return *this;
	}

	// Iterator Range Assignment Template
	template< typename InputIterator >
	CArrayA &
	assign(
	 InputIterator const beg,
	 InputIterator const end
	)
	{
		size_type const size( end - beg );
		if ( size_ != size ) {
			destroy();
			size_ = size;
			mem_ = Aligned::allocate( size_ );
			data_ = Aligned::data( mem_ );
			if ( size_ > 0u ) {
				InputIterator k( beg );
				for ( size_type i = 0; i < size_; ++i, ++k ) {
					new ( data_ + i ) T( *k );
				}
			}
		} else {
			if ( size_ > 0u ) {
				InputIterator k( beg );
				for ( size_type i = 0; i < size_; ++i, ++k ) {
					data_[ i ] = T( *k );
				}
			}
		}
		return *this;
	}

	// Size + Value Assignment
	CArrayA &
	assign(
	 size_type const size,
	 Tc t
	)
	{
		if ( size_ != size ) { // Set to new array with uniform values
			CArrayA( size, t ).swap( *this );
		} else { // Set to uniform value
			(*this) = t;
		}
		return *this;
	}

public: // Predicate

	// Active?
	bool
	active() const
	{
		return ( data_ != nullptr );
	}

	// Empty?
	bool
	empty() const
	{
		return ( size_ == 0u );
	}

public: // Inspector

	// Size
	size_type
	size() const
	{
		return size_;
	}

	// Lower Index
	size_type
	l() const
	{
		return 0u;
	}

	// Upper index
	size_type
	u() const
	{
		return size_ - 1u; // npos if size_ == 0
	}

	// First Element
	Tr
	front() const
	{
		assert( size_ > 0u );
		return data_[ 0 ];
	}

	// Last Element
	Tr
	back() const
	{
		assert( size_ > 0u );
		return data_[ size_ - 1 ];
	}

	// Length
	T
	length() const
	{
		T length_sq( T( 0 ) );
		for ( size_type i = 0; i < size_; ++i ) {
			length_sq += data_[ i ] * data_[ i ];
		}
		return std::sqrt( length_sq );
	}

	// Length Squared
	T
	length_squared() const
	{
		T length_sq( T( 0 ) );
		for ( size_type i = 0; i < size_; ++i ) {
			length_sq += data_[ i ] * data_[ i ];
		}
		return length_sq;
	}

public: // Modifier

	// First Element
	T &
	front()
	{
		assert( size_ > 0u );
		return data_[ 0 ];
	}

	// Last Element
	T &
	back()
	{
		assert( size_ > 0u );
		return data_[ size_ - 1 ];
	}

	// Resize: Values not Preserved
	// Built-in values are uninitialized if size changes
	CArrayA &
	size( size_type const size )
	{
		if ( size_ != size ) { // Set to new array
			CArrayA( size ).swap( *this );
		}
		return *this;
	}

	// Resize to Size With Fill Value: Values Preserved
	CArrayA &
	resize(
	 size_type const size,
	 Tc fill = T()
	)
	{
		if ( size_ < size ) {
			CArrayA a( size, fill ); // New array: Elements set to fill fill
			for ( size_type i = 0; i < size_; ++i ) { // Copy current values
				a.data_[ i ] = data_[ i ];
			}
			swap( a ); // Swap in new array
		} else if ( size_ > size ) {
			CArrayA a( size ); // New array
			for ( size_type i = 0; i < size; ++i ) { // Copy current values within new range
				a.data_[ i ] = data_[ i ];
			}
			swap( a ); // Swap in new array
		}
		return *this;
	}

	// Swap
	void
	swap( CArrayA & a )
	{
		std::swap( size_, a.size_ );
		std::swap( mem_, a.mem_ );
		std::swap( data_, a.data_ );
	}

	// Clear
	CArrayA &
	clear()
	{
		destroy();
		size_ = 0u;
		mem_ = data_ = nullptr;
		return *this;
	}

	// Normalize to Unit Length
	CArrayA &
	normalize()
	{
		T const length_( length() );
		assert( length_ > T( 0 ) );
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] /= length_;
		}
		return *this;
	}

public: // Subscript

	// CArrayA[ i ] const: 0-Based Indexing
	template< typename I, class = typename std::enable_if< std::is_integral< I >::value && std::is_unsigned< I >::value && std::is_const< T >::value >::type >
	Tr
	operator []( I const i ) const
	{
		assert( i < size_ );
		return data_[ i ];
	}

	// CArrayA[ i ] const: 0-Based Indexing
	template< typename I, class = typename std::enable_if< std::is_integral< I >::value && std::is_signed< I >::value && std::is_const< T >::value >::type, typename = void >
	Tr
	operator []( I const i ) const
	{
		assert( ( i >= 0 ) && ( static_cast< size_type >( i ) < size_ ) );
		return data_[ i ];
	}

	// CArrayA[ i ]: 0-Based Indexing
	template< typename I, class = typename std::enable_if< std::is_integral< I >::value && std::is_unsigned< I >::value >::type >
	T &
	operator []( I const i )
	{
		assert( i < size_ );
		return data_[ i ];
	}

	// CArrayA[ i ]: 0-Based Indexing
	template< typename I, class = typename std::enable_if< std::is_integral< I >::value && std::is_signed< I >::value >::type, typename = void >
	T &
	operator []( I const i )
	{
		assert( ( i >= 0 ) && ( static_cast< size_type >( i ) < size_ ) );
		return data_[ i ];
	}

	// CArrayA( i ) const: 1-Based Indexing
	template< typename I, class = typename std::enable_if< std::is_integral< I >::value && std::is_unsigned< I >::value && std::is_const< T >::value >::type >
	Tr
	operator ()( I const i ) const
	{
		assert( ( i > 0u ) && ( i <= size_ ) );
		return data_[ i - 1 ];
	}

	// CArrayA( i ) const: 1-Based Indexing
	template< typename I, class = typename std::enable_if< std::is_integral< I >::value && std::is_signed< I >::value && std::is_const< T >::value >::type, typename = void >
	Tr
	operator ()( I const i ) const
	{
		assert( ( i > 0 ) && ( static_cast< size_type >( i ) <= size_ ) );
		return data_[ i - 1 ];
	}

	// CArrayA( i ): 1-Based Indexing
	template< typename I, class = typename std::enable_if< std::is_integral< I >::value && std::is_unsigned< I >::value >::type >
	T &
	operator ()( I const i )
	{
		assert( ( i > 0u ) && ( i <= size_ ) );
		return data_[ i - 1 ];
	}

	// CArrayA( i ): 1-Based Indexing
	template< typename I, class = typename std::enable_if< std::is_integral< I >::value && std::is_signed< I >::value >::type, typename = void >
	T &
	operator ()( I const i )
	{
		assert( ( i > 0 ) && ( static_cast< size_type >( i ) <= size_ ) );
		return data_[ i - 1 ];
	}

public: // Iterator

	// Begin Iterator
	const_iterator
	begin() const
	{
		return data_;
	}

	// Begin Iterator
	iterator
	begin()
	{
		return data_;
	}

	// End Iterator
	const_iterator
	end() const
	{
		return ( data_ != nullptr ? data_ + size_ : nullptr );
	}

	// End Iterator
	iterator
	end()
	{
		return ( data_ != nullptr ? data_ + size_ : nullptr );
	}

	// Reverse Begin Iterator
	const_reverse_iterator
	rbegin() const
	{
		return const_reverse_iterator( data_ != nullptr ? data_ + size_ : nullptr );
	}

	// Reverse Begin Iterator
	reverse_iterator
	rbegin()
	{
		return reverse_iterator( data_ != nullptr ? data_ + size_ : nullptr );
	}

	// Reverse End Iterator
	const_reverse_iterator
	rend() const
	{
		return const_reverse_iterator( data_ );
	}

	// Reverse End Iterator
	reverse_iterator
	rend()
	{
		return reverse_iterator( data_ );
	}

public: // Array Accessor

	// C Array const Accessor
	T const *
	operator ()() const
	{
		return data_;
	}

	// C Array Non-const Accessor
	T *
	operator ()()
	{
		return data_;
	}

private: // Methods

	// Destruct Elements and Delete Array Memory (Doesn't Nullify Pointers)
	void
	destroy()
	{
		size_type i( size_ );
		while ( i ) data_[ --i ].~T();
		::operator delete( mem_ );
	}

private: // Data

	size_type size_; // Number of array elements
	void * mem_; // Pointer to raw memory
	T * data_; // Pointer to data

}; // CArrayA

// Functions

// Magnitude
template< typename T >
inline
T
magnitude( CArrayA< T > const & a )
{
	T mag_sq( T( 0 ) );
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		mag_sq += a[ i ] * a[ i ];
	}
	return std::sqrt( mag_sq );
}

// Magnitude Squared
template< typename T >
inline
T
magnitude_squared( CArrayA< T > const & a )
{
	T mag_sq( T( 0 ) );
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		mag_sq += a[ i ] * a[ i ];
	}
	return mag_sq;
}

// Distance
template< typename T >
inline
T
distance( CArrayA< T > const & a, CArrayA< T > const & b )
{
	assert( a.size() == b.size() );
	T distance_sq( T( 0 ) );
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		T const distance_i( a[ i ] - b[ i ] );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance Squared
template< typename T >
inline
T
distance_squared( CArrayA< T > const & a, CArrayA< T > const & b )
{
	assert( a.size() == b.size() );
	T distance_sq( T( 0 ) );
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		T const distance_i( a[ i ] - b[ i ] );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Dot Product
template< typename T >
inline
T
dot( CArrayA< T > const & a, CArrayA< T > const & b )
{
	assert( a.size() == b.size() );
	T sum( T( 0 ) );
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		sum += a[ i ] * b[ i ];
	}
	return sum;
}

// Dot Product
template< typename T >
inline
T
dot_product( CArrayA< T > const & a, CArrayA< T > const & b )
{
	assert( a.size() == b.size() );
	T sum( T( 0 ) );
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		sum += a[ i ] * b[ i ];
	}
	return sum;
}

// Swap
template< typename T >
inline
void
swap( CArrayA< T > & a, CArrayA< T > & b )
{
	a.swap( b );
}

// Comparison

// Are Two CArrayAs Comparable?
template< typename T >
inline
bool
comparable( CArrayA< T > const & a, CArrayA< T > const & b )
{
	return ( a.size() == b.size() );
}

// CArrayA == CArrayA
template< typename T >
inline
bool
operator ==( CArrayA< T > const & a, CArrayA< T > const & b )
{
	if ( &a == &b ) { // Same objects
		return true;
	} else if ( a.size() != b.size() ) { // Sizes differ
		return false;
	} else { // Compare values
		for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
			if ( !( a[ i ] == b[ i ] ) ) return false;
		}
		return true;
	}
}

// CArrayA != CArrayA
template< typename T >
inline
bool
operator !=( CArrayA< T > const & a, CArrayA< T > const & b )
{
	return !( a == b );
}

// CArrayA < CArrayA
template< typename T >
inline
bool
operator <( CArrayA< T > const & a, CArrayA< T > const & b )
{
	if ( &a == &b ) { // Same objects
		return false;
	} else if ( a.size() != b.size() ) { // Sizes differ
		return false;
	} else { // Compare values
		for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
			if ( !( a[ i ] < b[ i ] ) ) return false;
		}
		return true;
	}
}

// CArrayA <= CArrayA
template< typename T >
inline
bool
operator <=( CArrayA< T > const & a, CArrayA< T > const & b )
{
	if ( &a == &b ) { // Same objects
		return true;
	} else if ( a.size() != b.size() ) { // Sizes differ
		return false;
	} else { // Compare values
		for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
			if ( !( a[ i ] <= b[ i ] ) ) return false;
		}
		return true;
	}
}

// CArrayA >= CArrayA
template< typename T >
inline
bool
operator >=( CArrayA< T > const & a, CArrayA< T > const & b )
{
	if ( &a == &b ) { // Same objects
		return true;
	} else if ( a.size() != b.size() ) { // Sizes differ
		return false;
	} else { // Compare values
		for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
			if ( !( a[ i ] >= b[ i ] ) ) return false;
		}
		return true;
	}
}

// CArrayA > CArrayA
template< typename T >
inline
bool
operator >( CArrayA< T > const & a, CArrayA< T > const & b )
{
	if ( &a == &b ) { // Same objects
		return false;
	} else if ( a.size() != b.size() ) { // Sizes differ
		return false;
	} else { // Compare values
		for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
			if ( !( a[ i ] > b[ i ] ) ) return false;
		}
		return true;
	}
}

// CArrayA == Value
template< typename T >
inline
bool
operator ==( CArrayA< T > const & a, typename CArrayA< T >::Tc t )
{
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		if ( a[ i ] != t ) return false;
	}
	return true;
}

// CArrayA != Value
template< typename T >
inline
bool
operator !=( CArrayA< T > const & a, typename CArrayA< T >::Tc t )
{
	return !( a == t );
}

// CArrayA < Value
template< typename T >
inline
bool
operator <( CArrayA< T > const & a, typename CArrayA< T >::Tc t )
{
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		if ( !( a[ i ] < t ) ) return false;
	}
	return true;
}

// CArrayA <= Value
template< typename T >
inline
bool
operator <=( CArrayA< T > const & a, typename CArrayA< T >::Tc t )
{
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		if ( !( a[ i ] <= t ) ) return false;
	}
	return true;
}

// CArrayA >= Value
template< typename T >
inline
bool
operator >=( CArrayA< T > const & a, typename CArrayA< T >::Tc t )
{
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		if ( !( a[ i ] >= t ) ) return false;
	}
	return true;
}

// CArrayA > Value
template< typename T >
inline
bool
operator >( CArrayA< T > const & a, typename CArrayA< T >::Tc t )
{
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		if ( !( a[ i ] > t ) ) return false;
	}
	return true;
}

// Value == CArrayA
template< typename T >
inline
bool
operator ==( typename CArrayA< T >::Tc t, CArrayA< T > const & a )
{
	return ( a == t );
}

// Value != CArrayA
template< typename T >
inline
bool
operator !=( typename CArrayA< T >::Tc t, CArrayA< T > const & a )
{
	return !( t == a );
}

// Value < CArrayA
template< typename T >
inline
bool
operator <( typename CArrayA< T >::Tc t, CArrayA< T > const & a )
{
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		if ( !( t < a[ i ] ) ) return false;
	}
	return true;
}

// Value <= CArrayA
template< typename T >
inline
bool
operator <=( typename CArrayA< T >::Tc t, CArrayA< T > const & a )
{
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		if ( !( t <= a[ i ] ) ) return false;
	}
	return true;
}

// Value >= CArrayA
template< typename T >
inline
bool
operator >=( typename CArrayA< T >::Tc t, CArrayA< T > const & a )
{
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		if ( !( t >= a[ i ] ) ) return false;
	}
	return true;
}

// Value > CArrayA
template< typename T >
inline
bool
operator >( typename CArrayA< T >::Tc t, CArrayA< T > const & a )
{
	for ( typename CArrayA< T >::size_type i = 0, ie = a.size(); i < ie; ++i ) {
		if ( !( t > a[ i ] ) ) return false;
	}
	return true;
}

// Stream >> CArrayA
template< typename T >
inline
std::istream &
operator >>( std::istream & stream, CArrayA< T > & a )
{
	typedef  typename CArrayA< T >::size_type  size_type;
	if ( stream && ( ! a.emtpy() ) ) {
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			stream >> a[ i ];
			if ( ! stream ) break;
		}
	}
	return stream;
}

// Stream << CArrayA
template< typename T >
inline
std::ostream &
operator <<( std::ostream & stream, CArrayA< T > const & a )
{
	using std::setw;
	typedef  TypeTraits< T >  Traits;
	typedef  typename CArrayA< T >::size_type  size_type;
	if ( stream && ( ! a.emtpy() ) ) {
		std::ios_base::fmtflags const old_flags( stream.flags() );
		std::streamsize const old_precision( stream.precision( Traits::precision ) );
		stream << std::right << std::showpoint << std::uppercase;
		size_type const e( a.size() - 1 );
		int const w( Traits::iwidth );
		for ( size_type i = 0; i < e; ++i ) {
			stream << setw( w ) << a[ i ] << ' ';
		} stream << setw( w ) << a[ e ];
		stream.precision( old_precision );
		stream.flags( old_flags );
	}
	return stream;
}

} // ObjexxFCL

#endif // ObjexxFCL_CArrayA_hh_INCLUDED
