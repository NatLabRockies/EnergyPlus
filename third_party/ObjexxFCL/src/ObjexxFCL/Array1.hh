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

// C++ Headers
#include <cmath>

namespace ObjexxFCL {

// Forward
template< typename > class Array1D;

// Array1: 1D Array Abstract Base Class
template< typename T >
class Array1 : public Array< T >
{

private: // Types

	typedef  Array< T >  Super;

private: // Friend

	template< typename > friend class Array1;
	template< typename > friend class Array1D;

protected: // Types

	typedef  internal::InitializerSentinel  InitializerSentinel;
	typedef  internal::ProxySentinel  ProxySentinel;

public: // Types

	typedef  typename Super::Base  Base;
	typedef  typename Super::IR  IR;

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

	// Default Proxy Constructor
	Array1( ProxySentinel proxy ) :
	 Super( proxy )
	{}

	// Copy Proxy Constructor
	Array1( Array1 const & a, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I_( a.I_ )
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

	// Equal Dimensions?
	template< typename U >
	bool
	equal_dimensions( Array1< U > const & a ) const
	{
		return ( I_ == a.I_ );
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

public: // Comparison: Predicate


public: // Comparison: Predicate: Any


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

// Equal Dimensions?
template< typename U, typename V >
inline
bool
equal_dimensions( Array1< U > const & a, Array1< V > const & b )
{
	return a.equal_dimensions( b );
}

} // ObjexxFCL

#endif // ObjexxFCL_Array1_hh_INCLUDED
