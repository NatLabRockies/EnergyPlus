#ifndef ObjexxFCL_Array1_hh_INCLUDED
#define ObjexxFCL_Array1_hh_INCLUDED

// Array1: 1D Array Abstract Base Class
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
#include <ObjexxFCL/Array1.fwd.hh>
#include <ObjexxFCL/Array.hh>
#include <ObjexxFCL/Array1S.hh>
#include <ObjexxFCL/MArray1.hh>

// C++ Headers
#include <cmath>

namespace ObjexxFCL {

// Forward
template< typename > class Array1D;
template< typename > class Array1A;

// Array1: 1D Array Abstract Base Class
template< typename T >
class Array1 : public Array< T >
{

private: // Types

	typedef  Array< T >  Super;

private: // Friend

	template< typename > friend class Array1;
	template< typename > friend class Array1D;
	template< typename > friend class Array1A;

protected: // Types

	typedef  internal::InitializerSentinel  InitializerSentinel;
	typedef  internal::ProxySentinel  ProxySentinel;

public: // Types

	typedef  typename Super::Base  Base;
	typedef  typename Super::IR  IR;
	typedef  typename Super::IS  IS;
	typedef  typename Super::DS  DS;

	// STL Style
	typedef  typename Super::value_type  value_type;
	typedef  typename Super::reference  reference;
	typedef  typename Super::const_reference  const_reference;
	typedef  typename Super::pointer  pointer;
	typedef  typename Super::const_pointer  const_pointer;
	typedef  typename Super::iterator  iterator;
	typedef  typename Super::const_iterator  const_iterator;
	typedef  typename Super::reverse_iterator  reverse_iterator;
	typedef  typename Super::const_reverse_iterator  const_reverse_iterator;
	typedef  typename Super::size_type  size_type;
	typedef  typename Super::difference_type  difference_type;

	// C++ Style
	typedef  typename Super::Value  Value;
	typedef  typename Super::Reference  Reference;
	typedef  typename Super::ConstReference  ConstReference;
	typedef  typename Super::Pointer  Pointer;
	typedef  typename Super::ConstPointer  ConstPointer;
	typedef  typename Super::Iterator  Iterator;
	typedef  typename Super::ConstIterator  ConstIterator;
	typedef  typename Super::ReverseIterator  ReverseIterator;
	typedef  typename Super::ConstReverseIterator  ConstReverseIterator;
	typedef  typename Super::Size  Size;
	typedef  typename Super::Difference  Difference;

	typedef  void  iterator_category; // Prevent compile failure when std::distance is in scope

	using Super::isize;
	using Super::npos;
	using Super::overlap;
	using Super::size;

protected: // Types

	using Super::shift_set;
	using Super::size_of;
	using Super::slice_k;
	using Super::swapB;

	using Super::capacity_;
	using Super::data_;
	using Super::sdata_;
	using Super::shift_;
	using Super::size_;

protected: // Creation

	// Default Constructor
	Array1() = default;

	// Copy Constructor
	Array1( Array1 const & a ) :
	 Super( a ),
	 I_( a.I_ )
	{}

	// Move Constructor
	Array1( Array1 && a ) noexcept :
	 Super( std::move( a ) ),
	 I_( a.I_ )
	{
		a.clear_move();
	}

	// Copy Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array1( Array1< U > const & a ) :
	 Super( a ),
	 I_( a.I_ )
	{}

	// Slice Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array1( Array1S< U > const & a ) :
	 Super( a ),
	 I_( a.u() )
	{}

	// MArray Constructor Template
	template< class A, typename M >
	explicit
	Array1( MArray1< A, M > const & a ) :
	 Super( a ),
	 I_( a.u() )
	{}

	// IndexRange Constructor
	explicit
	Array1( IR const & I ) :
	 Super( size_of( I ) ),
	 I_( I )
	{}

	// IndexRange + InitializerSentinel Constructor
	Array1( IR const & I, InitializerSentinel initialized ) :
	 Super( size_of( I ), initialized ),
	 I_( I )
	{}

	// Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1( std::initializer_list< U > const l ) :
	 Super( l ),
	 I_( static_cast< int >( l.size() ) )
	{}

	// Index Range + Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1( IR const & I, std::initializer_list< U > const l ) :
	 Super( l ),
	 I_( I )
	{
		assert( size_of( I ) == l.size() );
	}

	// std::array Constructor Template
	template< typename U, Size s, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1( std::array< U, s > const & a ) :
	 Super( a ),
	 I_( static_cast< int >( s ) )
	{}

	// std::vector Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1( std::vector< U > const & v ) :
	 Super( v ),
	 I_( static_cast< int >( v.size() ) )
	{}

	// Vector2 Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1( Vector2< U > const & v ) :
	 Super( v ),
	 I_( 2 )
	{}

	// Vector3 Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1( Vector3< U > const & v ) :
	 Super( v ),
	 I_( 3 )
	{}

	// Vector4 Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array1( Vector4< U > const & v ) :
	 Super( v ),
	 I_( 4 )
	{}

	// Iterator Range Constructor Template
	template< class Iterator, typename = decltype( *std::declval< Iterator & >(), void(), ++std::declval< Iterator & >(), void() ) >
	Array1( Iterator const beg, Iterator const end ) :
	 Super( beg, end ),
	 I_( static_cast< int >( size_ ) )
	{}

	// Default Proxy Constructor
	Array1( ProxySentinel proxy ) :
	 Super( proxy )
	{}

	// Copy Proxy Constructor
	Array1( Array1 const & a, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I_( a.I_ )
	{}

	// Slice Proxy Constructor
	Array1( Array1S< T > const & a, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I_( a.u() )
	{}

	// Base Proxy Constructor
	Array1( Base const & a, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I_( a.isize() )
	{}

	// Value Proxy Constructor
	Array1( T const & t, ProxySentinel proxy ) :
	 Super( t, proxy ),
	 I_( _ )
	{}

	// Copy + IndexRange Proxy Constructor
	Array1( Array1 const & a, IR const & I, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I_( I )
	{}

	// Slice + IndexRange Proxy Constructor
	Array1( Array1S< T > const & a, IR const & I, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I_( I )
	{}

	// Base + IndexRange Proxy Constructor
	Array1( Base const & a, IR const & I, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I_( I )
	{}

	// Value + IndexRange Proxy Constructor
	Array1( T const & t, IR const & I, ProxySentinel proxy ) :
	 Super( t, proxy ),
	 I_( I )
	{}

public: // Creation

	// Destructor
	virtual
	~Array1() = default;

public: // Assignment: Array

	// Copy Assignment
	Array1 &
	operator =( Array1 const & a )
	{
		if ( this != &a ) {
			if ( ( conformable( a ) ) || ( ! dimension_assign( a.I_ ) ) ) {
				Super::operator =( a );
			} else {
				Super::initialize( a );
			}
		}
		return *this;
	}

	// Copy Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1 &
	operator =( Array1< U > const & a )
	{
		if ( ( conformable( a ) ) || ( ! dimension_assign( a.I_ ) ) ) {
			Super::operator =( a );
		} else {
			Super::initialize( a );
		}
		return *this;
	}

	// Slice Assignment
	Array1 &
	operator =( Array1S< T > const & a )
	{
		size_type l( 0u );
		if ( ( conformable( a ) ) || ( ! dimension_assign( a.I() ) ) ) {
			if ( overlap( a ) ) { // Overlap-safe
				CArrayA< T > c( a.size() );
				for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
					c[ l ] = a( i );
				}
				for ( size_type i = 0; i < c.size(); ++i ) {
					data_[ i ] = c[ i ];
				}
			} else { // Not overlap-safe
				for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
					data_[ l ] = a( i );
				}
			}
		} else {
			for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
				new ( data_ + l ) T( a( i ) );
			}
		}
		return *this;
	}

	// Slice Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1 &
	operator =( Array1S< U > const & a )
	{
		size_type l( 0u );
		if ( ( conformable( a ) ) || ( ! dimension_assign( a.I() ) ) ) {
			for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
				data_[ l ] = a( i );
			}
		} else {
			for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
				new ( data_ + l ) T( a( i ) );
			}
		}
		return *this;
	}


	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1 &
	operator =( std::initializer_list< U > const l )
	{
		Super::operator =( l );
		return *this;
	}

	// std::array Assignment Template
	template< typename U, Size s, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1 &
	operator =( std::array< U, s > const & a )
	{
		Super::operator =( a );
		return *this;
	}

	// std::vector Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1 &
	operator =( std::vector< U > const & v )
	{
		Super::operator =( v );
		return *this;
	}

	// Vector2 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1 &
	operator =( Vector2< U > const & v )
	{
		Super::operator =( v );
		return *this;
	}

	// Vector3 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1 &
	operator =( Vector3< U > const & v )
	{
		Super::operator =( v );
		return *this;
	}

	// Vector4 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array1 &
	operator =( Vector4< U > const & v )
	{
		Super::operator =( v );
		return *this;
	}

public: // Assignment: Value

	// = Value
	Array1 &
	operator =( T const & t )
	{
		Super::operator =( t );
		return *this;
	}

public: // Subscript

	// array( i ) const
	T const &
	operator ()( int const i ) const
	{
		assert( contains( i ) );
		return sdata_[ i ];
	}

	// array( i )
	T &
	operator ()( int const i )
	{
		assert( contains( i ) );
		return sdata_[ i ];
	}

	// Linear Index
	size_type
	index( int const i ) const
	{
		return i - shift_;
	}


public: // Slice Proxy Generators

	// array( s ) const
	Array1S< T >
	operator ()( IS const & s ) const
	{
		DS const d( I_, s );
		return Array1S< T >( data_, -shift_, d );
	}

	// array( s )
	Array1S< T >
	operator ()( IS const & s )
	{
		DS const d( I_, s );
		return Array1S< T >( data_, -shift_, d );
	}

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) // VC++2013 bug work-around

	// array( {s} ) const
	Array1S< T >
	operator ()( std::initializer_list< int > const l ) const
	{
		IS const s( l );
		DS const d( I_, s );
		return Array1S< T >( data_, -shift_, d );
	}

	// array( {s} )
	Array1S< T >
	operator ()( std::initializer_list< int > const l )
	{
		IS const s( l );
		DS const d( I_, s );
		return Array1S< T >( data_, -shift_, d );
	}

#else

	// array( {s} ) const
	template< typename U, class = typename std::enable_if< std::is_constructible< int, U >::value >::type >
	Array1S< T >
	operator ()( std::initializer_list< U > const l ) const
	{
		IS const s( l );
		DS const d( I_, s );
		return Array1S< T >( data_, -shift_, d );
	}

	// array( {s} )
	template< typename U, class = typename std::enable_if< std::is_constructible< int, U >::value >::type >
	Array1S< T >
	operator ()( std::initializer_list< U > const l )
	{
		IS const s( l );
		DS const d( I_, s );
		return Array1S< T >( data_, -shift_, d );
	}

#endif

public: // Predicate

	// Contains Indexed Element?
	bool
	contains( int const i ) const
	{
		return I_.contains( i );
	}

	// Conformable?
	template< typename U >
	bool
	conformable( Array1< U > const & a ) const
	{
		return ( size_ == a.size() );
	}

	// Conformable?
	template< typename U >
	bool
	conformable( Array1S< U > const & a ) const
	{
		return ( size_ == a.size() );
	}

	// Equal Dimensions?
	template< typename U >
	bool
	equal_dimensions( Array1< U > const & a ) const
	{
		return ( I_ == a.I_ );
	}

	// Equal Dimensions?
	template< typename U >
	bool
	equal_dimensions( Array1S< U > const & a ) const
	{
		return ( ( l() == 1 ) && ( u() == a.u() ) );
	}

public: // Inspector

	// Rank
	int
	rank() const
	{
		return 1;
	}

	// IndexRange of a Dimension
	IR const &
	I( int const d ) const
	{
		switch ( d ) {
		case 1:
			return I_;
		default:
			assert( false );
			return I_;
		}
	}

	// Lower Index of a Dimension
	int
	l( int const d ) const
	{
		switch ( d ) {
		case 1:
			return l1();
		default:
			assert( false );
			return l1();
		}
	}

	// Upper Index of a Dimension
	int
	u( int const d ) const
	{
		switch ( d ) {
		case 1:
			return u1();
		default:
			assert( false );
			return u1();
		}
	}

	// Size of a Dimension
	size_type
	size( int const d ) const
	{
		switch ( d ) {
		case 1:
			return size1();
		default:
			assert( false );
			return size1();
		}
	}

	// Size of a Dimension
	int
	isize( int const d ) const
	{
		switch ( d ) {
		case 1:
			return isize1();
		default:
			assert( false );
			return isize1();
		}
	}

	// IndexRange
	IR const &
	I() const
	{
		return I_;
	}

	// Lower Index
	int
	l() const
	{
		return I_.l();
	}

	// Upper Index
	int
	u() const
	{
		return I_.u();
	}

	// IndexRange of Dimension 1
	IR const &
	I1() const
	{
		return I_;
	}

	// Lower Index of Dimension 1
	int
	l1() const
	{
		return I_.l();
	}

	// Upper Index of Dimension 1
	int
	u1() const
	{
		return I_.u();
	}

	// Size of Dimension 1
	size_type
	size1() const
	{
		return I_.size();
	}

	// Size of Dimension 1
	int
	isize1() const
	{
		return I_.isize();
	}

	// Length
	T
	length() const
	{
		T length_sq( T( 0 ) );
		for ( int i = l(), e = u(); i <= e; ++i ) {
			T const length_i( sdata_[ i ] );
			length_sq += length_i * length_i;
		}
		return std::sqrt( length_sq );
	}

public: // Modifier

	// Clear
	Array1 &
	clear()
	{
		Super::clear();
		I_.clear();
		shift_set( 1 );
		return *this;
	}

	// Normalize to Unit Length
	Array1 &
	normalize()
	{
		T const length_( length() );
		assert( length_ > T( 0 ) );
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] /= length_;
		}
		return *this;
	}

public: // MArray Generators

	// Template Helpers
	template< typename U > class Wrapper {};
	typedef  typename std::conditional< std::is_class< T >::value, T, Wrapper< T > >::type  ClassT;

	// MArray Generator
	template< typename M >
	MArray1< Array1 const, M >
	ma( M ClassT::* pmem ) const
	{
		return MArray1< Array1 const, M >( *this, pmem );
	}

	// MArray Generator
	template< typename M >
	MArray1< Array1, M >
	ma( M ClassT::* pmem )
	{
		return MArray1< Array1, M >( *this, pmem );
	}

public: // Comparison: Predicate

	// Array1 == Array1
	friend
	bool
	eq( Array1 const & a, Array1 const & b )
	{
		assert( a.size_bounded() );
		assert( a.conformable( b ) );
		return eq( static_cast< Super const & >( a ), static_cast< Super const & >( b ) );
	}


public: // Comparison: Predicate: Any

	// Array1 == Array1
	friend
	bool
	any_eq( Array1 const & a, Array1 const & b )
	{
		assert( a.size_bounded() );
		assert( a.conformable( b ) );
		return any_eq( static_cast< Super const & >( a ), static_cast< Super const & >( b ) );
	}


public: // Comparison: Predicate: Slice

	// Array1 == Array1S
	friend
	bool
	eq( Array1 const & a, Array1S< T > const & b )
	{
		assert( a.size_bounded() );
		assert( a.conformable( b ) );
		if ( a.empty() ) return true;
		size_type l( 0u );
		for ( int i = 1, e = b.u(); i <= e; ++i, ++l ) {
			if ( ! ( a[ l ] == b( i ) ) ) return false;
		}
		return true;
	}

	// Array1S == Array1
	friend
	bool
	eq( Array1S< T > const & a, Array1 const & b )
	{
		return eq( b, a );
	}

protected: // Functions

	// Dimension by IndexRange
	virtual
	bool
	dimension_assign( IR const & I ) = 0;

	// Clear on Move
	void
	clear_move()
	{
		I_.clear();
		shift_set( 1 );
	}

	// Swap
	void
	swap1( Array1 & v )
	{
		swapB( v );
		I_.swap( v.I_ );
	}

protected: // Data

	IR I_; // Index range

}; // Array1

// Conformable?
template< typename U, typename V >
inline
bool
conformable( Array1< U > const & a, Array1< V > const & b )
{
	return a.conformable( b );
}

// Conformable?
template< typename U, typename V >
inline
bool
conformable( Array1< U > const & a, Array1S< V > const & b )
{
	return a.conformable( b );
}

// Conformable?
template< typename U, typename V >
inline
bool
conformable( Array1S< U > const & a, Array1< V > const & b )
{
	return b.conformable( a );
}

// Equal Dimensions?
template< typename U, typename V >
inline
bool
equal_dimensions( Array1< U > const & a, Array1< V > const & b )
{
	return a.equal_dimensions( b );
}

// Magnitude
template< typename T >
inline
T
magnitude( Array1< T > const & a )
{
	T mag_sq( T( 0 ) );
	for ( int i = a.l(), e = a.u(); i <= e; ++i ) {
		T const mag_i( a( i ) );
		mag_sq += mag_i * mag_i;
	}
	return std::sqrt( mag_sq );
}

// Magnitude Squared
template< typename T >
inline
T
magnitude_squared( Array1< T > const & a )
{
	T mag_sq( T( 0 ) );
	for ( int i = a.l(), e = a.u(); i <= e; ++i ) {
		T const mag_i( a( i ) );
		mag_sq += mag_i * mag_i;
	}
	return mag_sq;
}

// Distance
template< typename T >
inline
T
distance( Array1< T > const & a, Array1< T > const & b )
{
	assert( a.size() == b.size() );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance
template< typename T >
inline
T
distance( Array1< T > const & a, Array1S< T > const & b )
{
	assert( a.size() == b.size() );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance
template< typename T >
inline
T
distance( Array1S< T > const & a, Array1< T > const & b )
{
	return distance( b, a );
}

// Distance
template< typename T >
inline
T
distance( Array1< T > const & a, Vector2< T > const & b )
{
	assert( a.size() == 2u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance
template< typename T >
inline
T
distance( Vector2< T > const & a, Array1< T > const & b )
{
	return distance( b, a );
}

// Distance
template< typename T >
inline
T
distance( Array1< T > const & a, Vector3< T > const & b )
{
	assert( a.size() == 3u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance
template< typename T >
inline
T
distance( Vector3< T > const & a, Array1< T > const & b )
{
	return distance( b, a );
}

// Distance
template< typename T >
inline
T
distance( Array1< T > const & a, Vector4< T > const & b )
{
	assert( a.size() == 4u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return std::sqrt( distance_sq );
}

// Distance
template< typename T >
inline
T
distance( Vector4< T > const & a, Array1< T > const & b )
{
	return distance( b, a );
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1< T > const & a, Array1< T > const & b )
{
	assert( a.size() == b.size() );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1< T > const & a, Array1S< T > const & b )
{
	assert( a.size() == b.size() );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1S< T > const & a, Array1< T > const & b )
{
	return distance_squared( b, a );
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1< T > const & a, Vector2< T > const & b )
{
	assert( a.size() == 2u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Vector2< T > const & a, Array1< T > const & b )
{
	return distance_squared( b, a );
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1< T > const & a, Vector3< T > const & b )
{
	assert( a.size() == 3u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Vector3< T > const & a, Array1< T > const & b )
{
	return distance_squared( b, a );
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Array1< T > const & a, Vector4< T > const & b )
{
	assert( a.size() == 4u );
	T distance_sq( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		T const distance_i( a( i ) - b( j ) );
		distance_sq += distance_i * distance_i;
	}
	return distance_sq;
}

// Distance Squared
template< typename T >
inline
T
distance_squared( Vector4< T > const & a, Array1< T > const & b )
{
	return distance_squared( b, a );
}

// Dot Product
template< typename T >
inline
T
dot( Array1< T > const & a, Array1< T > const & b )
{
	assert( a.size() == b.size() );
	T result( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		result += a( i ) * b( j );
	}
	return result;
}

// Dot Product
template< typename T >
inline
T
dot( Array1< T > const & a, Array1S< T > const & b )
{
	assert( a.size() == b.size() );
	T result( T( 0 ) );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		result += a( i ) * b( j );
	}
	return result;
}

// Dot Product
template< typename T >
inline
T
dot( Array1S< T > const & a, Array1< T > const & b )
{
	return dot( b, a );
}

// Dot Product of Boolean Arrays
inline
bool
dot( Array1< bool > const & a, Array1< bool > const & b )
{
	assert( a.size() == b.size() );
	bool result( false );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		if ( a( i ) && b( j ) ) {
			result = true;
			break;
		}
	}
	return result;
}

// Dot Product of Boolean Arrays
inline
bool
dot( Array1< bool > const & a, Array1S< bool > const & b )
{
	assert( a.size() == b.size() );
	bool result( false );
	for ( int i = a.l(), j = b.l(), e = a.u(); i <= e; ++i, ++j ) {
		if ( a( i ) && b( j ) ) {
			result = true;
			break;
		}
	}
	return result;
}

// Dot Product of Boolean Arrays
inline
bool
dot( Array1S< bool > const & a, Array1< bool > const & b )
{
	return dot( b, a );
}

// Dot Product with Vector2
template< typename T >
inline
T
dot( Array1< T > const & a, Vector2< T > const & b )
{
	assert( a.size() == 2u );
	T result( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		result += a( i ) * b( i );
	}
	return result;
}

// Dot Product with Vector2
template< typename T >
inline
T
dot( Vector2< T > const & a, Array1< T > const & b )
{
	return dot( b, a );
}

// Dot Product with Vector3
template< typename T >
inline
T
dot( Array1< T > const & a, Vector3< T > const & b )
{
	assert( a.size() == 3u );
	T result( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		result += a( i ) * b( i );
	}
	return result;
}

// Dot Product with Vector3
template< typename T >
inline
T
dot( Vector3< T > const & a, Array1< T > const & b )
{
	return dot( b, a );
}

// Dot Product with Vector4
template< typename T >
inline
T
dot( Array1< T > const & a, Vector4< T > const & b )
{
	assert( a.size() == 4u );
	T result( T( 0 ) );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		result += a( i ) * b( i );
	}
	return result;
}

// Dot Product with Vector4
template< typename T >
inline
T
dot( Vector4< T > const & a, Array1< T > const & b )
{
	return dot( b, a );
}

// Dot Product (Fortran Intrinsic Name)
template< typename T >
inline
T
dot_product( Array1< T > const & a, Array1< T > const & b )
{
	return dot( a, b );
}

// Dot Product (Fortran Intrinsic Name)
template< typename T >
inline
T
dot_product( Array1< T > const & a, Array1S< T > const & b )
{
	return dot( a, b );
}

// Dot Product (Fortran Intrinsic Name)
template< typename T >
inline
T
dot_product( Array1S< T > const & a, Array1< T > const & b )
{
	return dot( a, b );
}

// Dot Product of Boolean Arrays (Fortran Intrinsic Name)
inline
bool
dot_product( Array1< bool > const & a, Array1< bool > const & b )
{
	return dot( a, b );
}

// Dot Product of Boolean Arrays (Fortran Intrinsic Name)
inline
bool
dot_product( Array1< bool > const & a, Array1S< bool > const & b )
{
	return dot( a, b );
}

// Dot Product of Boolean Arrays (Fortran Intrinsic Name)
inline
bool
dot_product( Array1S< bool > const & a, Array1< bool > const & b )
{
	return dot( a, b );
}

// Cross Product of 2-Tuples
template< typename T >
inline
T
cross2( Vector2< T > const & a, Array1< T > const & b )
{
	return cross2( b, a );
}

} // ObjexxFCL

#endif // ObjexxFCL_Array1_hh_INCLUDED
