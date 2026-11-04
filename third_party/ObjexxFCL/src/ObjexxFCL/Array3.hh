#ifndef ObjexxFCL_Array3_hh_INCLUDED
#define ObjexxFCL_Array3_hh_INCLUDED

// Array3: Row-Major 3D Array Abstract Base Class
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
#include <ObjexxFCL/Array3.fwd.hh>
#include <ObjexxFCL/Array.hh>

namespace ObjexxFCL {

// Forward
template< typename > class Array3D;

// Array3: Row-Major 3D Array Abstract Base Class
template< typename T >
class Array3 : public Array< T >
{

private: // Types

	typedef  Array< T >  Super;

private: // Friend

	template< typename > friend class Array3;
	template< typename > friend class Array3D;

protected: // Types

	typedef  internal::InitializerSentinel  InitializerSentinel;

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

	using Super::isize;
	using Super::npos;
	using Super::overlap;
	using Super::size;

protected: // Types

	using Super::size_of;
	using Super::swapB;

	using Super::data_;
	using Super::sdata_;
	using Super::shift_;
	using Super::size_;

protected: // Creation

	// Default Constructor
	Array3() :
	 z1_( 0u ),
	 z2_( 0u ),
	 z3_( 0u )
	{}

	// Copy Constructor
	Array3( Array3 const & a ) :
	 Super( a ),
	 I1_( a.I1_ ),
	 I2_( a.I2_ ),
	 I3_( a.I3_ ),
	 z1_( a.z1_ ),
	 z2_( a.z2_ ),
	 z3_( a.z3_ )
	{}

	// Move Constructor
	Array3( Array3 && a ) noexcept :
	 Super( std::move( a ) ),
	 I1_( a.I1_ ),
	 I2_( a.I2_ ),
	 I3_( a.I3_ ),
	 z1_( a.z1_ ),
	 z2_( a.z2_ ),
	 z3_( a.z3_ )
	{
		a.clear_move();
	}

	// Copy Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array3( Array3< U > const & a ) :
	 Super( a ),
	 I1_( a.I1_ ),
	 I2_( a.I2_ ),
	 I3_( a.I3_ ),
	 z1_( a.z1_ ),
	 z2_( a.z2_ ),
	 z3_( a.z3_ )
	{}

	// IndexRange Constructor
	Array3( IR const & I1, IR const & I2, IR const & I3 ) :
	 Super( size_of( I1, I2, I3 ) ),
	 I1_( I1 ),
	 I2_( I2 ),
	 I3_( I3 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() ),
	 z3_( I3_.size() )
	{}

	// IndexRange + InitializerSentinel Constructor
	Array3( IR const & I1, IR const & I2, IR const & I3, InitializerSentinel initialized ) :
	 Super( size_of( I1, I2, I3 ), initialized ),
	 I1_( I1 ),
	 I2_( I2 ),
	 I3_( I3 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() ),
	 z3_( I3_.size() )
	{}

	// IndexRange + Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array3( IR const & I1, IR const & I2, IR const & I3, std::initializer_list< U > const l ) :
	 Super( l ),
	 I1_( I1 ),
	 I2_( I2 ),
	 I3_( I3 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() ),
	 z3_( I3_.size() )
	{
		assert( size_of( I1, I2, I3 ) == l.size() );
	}


public: // Creation

	// Destructor
	virtual
	~Array3() = default;

public: // Assignment: Array

	// Copy Assignment
	Array3 &
	operator =( Array3 const & a )
	{
		if ( this != &a ) {
			if ( ( conformable( a ) ) || ( ! dimension_assign( a.I1_, a.I2_, a.I3_ ) ) ) {
				Super::operator =( a );
			} else {
				Super::initialize( a );
			}
		}
		return *this;
	}

	// Copy Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array3 &
	operator =( Array3< U > const & a )
	{
		if ( ( conformable( a ) ) || ( ! dimension_assign( a.I1_, a.I2_, a.I3_ ) ) ) {
			Super::operator =( a );
		} else {
			Super::initialize( a );
		}
		return *this;
	}

	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array3 &
	operator =( std::initializer_list< U > const l )
	{
		Super::operator =( l );
		return *this;
	}


public: // Assignment: Value

	// = Value
	Array3 &
	operator =( T const & t )
	{
		Super::operator =( t );
		return *this;
	}

public: // Subscript

	// array( i1, i2, i3 ) const
	T const &
	operator ()( int const i1, int const i2, int const i3 ) const
	{
		assert( contains( i1, i2, i3 ) );
		return sdata_[ ( ( ( i1 * z2_ ) + i2 ) * z3_ ) + i3 ];
	}

	// array( i1, i2, i3 )
	T &
	operator ()( int const i1, int const i2, int const i3 )
	{
		assert( contains( i1, i2, i3 ) );
		return sdata_[ ( ( ( i1 * z2_ ) + i2 ) * z3_ ) + i3 ];
	}

	// Linear Index
	size_type
	index( int const i1, int const i2, int const i3 ) const
	{
		return ( ( ( ( i1 * z2_ ) + i2 ) * z3_ ) + i3 ) - shift_;
	}

public: // Predicate

	// Contains Indexed Element?
	bool
	contains( int const i1, int const i2, int const i3 ) const
	{
		return ( I1_.contains( i1 ) && I2_.contains( i2 ) && I3_.contains( i3 ) );
	}

	// Conformable?
	template< typename U >
	bool
	conformable( Array3< U > const & a ) const
	{
		return ( ( z1_ == a.z1_ ) && ( z2_ == a.z2_ ) && ( z3_ == a.z3_ ) );
	}

	// Equal Dimensions?
	template< typename U >
	bool
	equal_dimensions( Array3< U > const & a ) const
	{
		return ( ( I1_ == a.I1_ ) && ( I2_ == a.I2_ ) && ( I3_ == a.I3_ ) );
	}

public: // Inspector

	// Rank
	int
	rank() const
	{
		return 3;
	}

	// IndexRange of a Dimension
	IR const &
	I( int const d ) const
	{
		switch ( d ) {
		case 1:
			return I1_;
		case 2:
			return I2_;
		case 3:
			return I3_;
		default:
			assert( false );
			return I1_;
		}
	}

	// Lower Index of a Dimension
	int
	l( int const d ) const
	{
		switch ( d ) {
		case 1:
			return l1();
		case 2:
			return l2();
		case 3:
			return l3();
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
		case 2:
			return u2();
		case 3:
			return u3();
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
			return z1_;
		case 2:
			return z2_;
		case 3:
			return z3_;
		default:
			assert( false );
			return z1_;
		}
	}

	// Size of a Dimension
	int
	isize( int const d ) const
	{
		switch ( d ) {
		case 1:
			return isize1();
		case 2:
			return isize2();
		case 3:
			return isize3();
		default:
			assert( false );
			return isize1();
		}
	}

	// IndexRange of Dimension 1
	IR const &
	I1() const
	{
		return I1_;
	}

	// Lower Index of Dimension 1
	int
	l1() const
	{
		return I1_.l();
	}

	// Upper Index of Dimension 1
	int
	u1() const
	{
		return I1_.u();
	}

	// Size of Dimension 1
	size_type
	size1() const
	{
		return z1_;
	}

	// Size of Dimension 1
	int
	isize1() const
	{
		return static_cast< int >( z1_ );
	}

	// IndexRange of Dimension 2
	IR const &
	I2() const
	{
		return I2_;
	}

	// Lower Index of Dimension 2
	int
	l2() const
	{
		return I2_.l();
	}

	// Upper Index of Dimension 2
	int
	u2() const
	{
		return I2_.u();
	}

	// Size of Dimension 2
	size_type
	size2() const
	{
		return z2_;
	}

	// Size of Dimension 2
	int
	isize2() const
	{
		return static_cast< int >( z2_ );
	}

	// IndexRange of Dimension 3
	IR const &
	I3() const
	{
		return I3_;
	}

	// Lower Index of Dimension 3
	int
	l3() const
	{
		return I3_.l();
	}

	// Upper Index of Dimension 3
	int
	u3() const
	{
		return I3_.u();
	}

	// Size of Dimension 3
	size_type
	size3() const
	{
		return z3_;
	}

	// Size of Dimension 3
	int
	isize3() const
	{
		return static_cast< int >( z3_ );
	}

public: // Modifier

	// Clear
	Array3 &
	clear()
	{
		Super::clear();
		I1_.clear();
		I2_.clear();
		I3_.clear();
		z1_ = z2_ = z3_ = 0u;
		return *this;
	}


protected: // Functions

	// Dimension by IndexRange
	virtual
	bool
	dimension_assign( IR const & I1, IR const & I2, IR const & I3 ) = 0;

	// Clear on Move
	void
	clear_move()
	{
		I1_.clear();
		I2_.clear();
		I3_.clear();
		z1_ = z2_ = z3_ = 0u;
	}

	// Swap
	void
	swap3( Array3 & v )
	{
		swapB( v );
		I1_.swap( v.I1_ );
		I2_.swap( v.I2_ );
		I3_.swap( v.I3_ );
		std::swap( z1_, v.z1_ );
		std::swap( z2_, v.z2_ );
		std::swap( z3_, v.z3_ );
	}

protected: // Data

	IR I1_; // Index range of dim 1
	IR I2_; // Index range of dim 2
	IR I3_; // Index range of dim 3

	size_type z1_; // Size of dim 1
	size_type z2_; // Size of dim 2
	size_type z3_; // Size of dim 3

}; // Array3

// Conformable?
template< typename U, typename V >
inline
bool
conformable( Array3< U > const & a, Array3< V > const & b )
{
	return a.conformable( b );
}

// Equal Dimensions?
template< typename U, typename V >
inline
bool
equal_dimensions( Array3< U > const & a, Array3< V > const & b )
{
	return a.equal_dimensions( b );
}

} // ObjexxFCL

#endif // ObjexxFCL_Array3_hh_INCLUDED
