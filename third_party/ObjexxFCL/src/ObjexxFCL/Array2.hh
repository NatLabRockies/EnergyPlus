#ifndef ObjexxFCL_Array2_hh_INCLUDED
#define ObjexxFCL_Array2_hh_INCLUDED

// Array2: Row-Major 2D Array Abstract Base Class
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
#include <ObjexxFCL/Array2.fwd.hh>
#include <ObjexxFCL/Array.hh>
#include <ObjexxFCL/Array2S.hh>

namespace ObjexxFCL {

// Forward
template< typename > class Array1D; // For project-specific member array methods
template< typename > class Array2D;
template< typename > class Array2A;

// Array2: Row-Major 2D Array Abstract Base Class
template< typename T >
class Array2 : public Array< T >
{

private: // Types

	typedef  Array< T >  Super;

private: // Friend

	template< typename > friend class Array2;
	template< typename > friend class Array2D;
	template< typename > friend class Array2A;

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

	using Super::isize;
	using Super::npos;
	using Super::overlap;
	using Super::size;

protected: // Types

	using Super::size_of;
	using Super::slice_k;
	using Super::swapB;

	using Super::data_;
	using Super::sdata_;
	using Super::shift_;
	using Super::size_;

protected: // Creation

	// Default Constructor
	Array2() :
	 z1_( 0u ),
	 z2_( 0u )
	{}

	// Copy Constructor
	Array2( Array2 const & a ) :
	 Super( a ),
	 I1_( a.I1_ ),
	 I2_( a.I2_ ),
	 z1_( a.z1_ ),
	 z2_( a.z2_ )
	{}

	// Move Constructor
	Array2( Array2 && a ) noexcept :
	 Super( std::move( a ) ),
	 I1_( a.I1_ ),
	 I2_( a.I2_ ),
	 z1_( a.z1_ ),
	 z2_( a.z2_ )
	{
		a.clear_move();
	}

	// Copy Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array2( Array2< U > const & a ) :
	 Super( a ),
	 I1_( a.I1_ ),
	 I2_( a.I2_ ),
	 z1_( a.z1_ ),
	 z2_( a.z2_ )
	{}

	// Slice Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array2( Array2S< U > const & a ) :
	 Super( a ),
	 I1_( a.u1() ),
	 I2_( a.u2() ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() )
	{}

	// IndexRange Constructor
	Array2( IR const & I1, IR const & I2 ) :
	 Super( size_of( I1, I2 ) ),
	 I1_( I1 ),
	 I2_( I2 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() )
	{}

	// IndexRange + InitializerSentinel Constructor
	Array2( IR const & I1, IR const & I2, InitializerSentinel initialized ) :
	 Super( size_of( I1, I2 ), initialized ),
	 I1_( I1 ),
	 I2_( I2 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() )
	{}

	// IndexRange + Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array2( IR const & I1, IR const & I2, std::initializer_list< U > const l ) :
	 Super( l ),
	 I1_( I1 ),
	 I2_( I2 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() )
	{
		assert( size_of( I1, I2 ) == l.size() );
	}

	// Default Proxy Constructor
	Array2( ProxySentinel proxy ) :
	 Super( proxy ),
	 z1_( 0u ),
	 z2_( 0u )
	{}

	// Copy Proxy Constructor
	Array2( Array2 const & a, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I1_( a.I1_ ),
	 I2_( a.I2_ ),
	 z1_( a.z1_ ),
	 z2_( a.z2_ )
	{}

	// Slice Proxy Constructor
	Array2( Array2S< T > const & a, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I1_( a.u1() ),
	 I2_( a.u2() ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() )
	{}

	// Base Proxy Constructor
	Array2( Base const & a, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I1_( a.isize() ),
	 I2_( 1 ),
	 z1_( I1_.size() ),
	 z2_( 1u )
	{}

	// Value Proxy Constructor
	Array2( T const & t, ProxySentinel proxy ) :
	 Super( t, proxy ),
	 I1_( _ ),
	 I2_( 1 ),
	 z1_( I1_.size() ),
	 z2_( 1u )
	{}

	// Copy + IndexRange Proxy Constructor
	Array2( Array2 const & a, IR const & I1, IR const & I2, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I1_( I1 ),
	 I2_( I2 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() )
	{}

	// Slice + IndexRange Proxy Constructor
	Array2( Array2S< T > const & a, IR const & I1, IR const & I2, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I1_( I1 ),
	 I2_( I2 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() )
	{}

	// Base + IndexRange Proxy Constructor
	Array2( Base const & a, IR const & I1, IR const & I2, ProxySentinel proxy ) :
	 Super( a, proxy ),
	 I1_( I1 ),
	 I2_( I2 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() )
	{}

	// Value + IndexRange Proxy Constructor
	Array2( T const & t, IR const & I1, IR const & I2, ProxySentinel proxy ) :
	 Super( t, proxy ),
	 I1_( I1 ),
	 I2_( I2 ),
	 z1_( I1_.size() ),
	 z2_( I2_.size() )
	{}

public: // Creation

	// Destructor
	virtual
	~Array2() = default;

public: // Assignment: Array

	// Copy Assignment
	Array2 &
	operator =( Array2 const & a )
	{
		if ( this != &a ) {
			if ( ( conformable( a ) ) || ( ! dimension_assign( a.I1_, a.I2_ ) ) ) {
				Super::operator =( a );
			} else {
				Super::initialize( a );
			}
		}
		return *this;
	}

	// Copy Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array2 &
	operator =( Array2< U > const & a )
	{
		if ( ( conformable( a ) ) || ( ! dimension_assign( a.I1_, a.I2_ ) ) ) {
			Super::operator =( a );
		} else {
			Super::initialize( a );
		}
		return *this;
	}

	// Slice Assignment
	Array2 &
	operator =( Array2S< T > const & a )
	{
		size_type l( 0u );
		if ( ( conformable( a ) ) || ( ! dimension_assign( a.I1(), a.I2() ) ) ) {
			if ( overlap( a ) ) { // Overlap-safe
				CArrayA< T > c( a.size() );
				for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
					for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2, ++l ) {
						c[ l ] = a( i1, i2 );
					}
				}
				for ( size_type i = 0; i < c.size(); ++i ) {
					data_[ i ] = c[ i ];
				}
			} else { // Not overlap-safe
				for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
					for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2, ++l ) {
						data_[ l ] = a( i1, i2 );
					}
				}
			}
		} else {
			for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
				for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2, ++l ) {
					new ( data_ + l ) T( a( i1, i2 ) );
				}
			}
		}
		return *this;
	}

	// Slice Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array2 &
	operator =( Array2S< U > const & a )
	{
		size_type l( 0u );
		if ( ( conformable( a ) ) || ( ! dimension_assign( a.I1(), a.I2() ) ) ) {
			for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
				for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2, ++l ) {
					data_[ l ] = a( i1, i2 );
				}
			}
		} else {
			for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
				for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2, ++l ) {
					new ( data_ + l ) T( a( i1, i2 ) );
				}
			}
		}
		return *this;
	}

	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	Array2 &
	operator =( std::initializer_list< U > const l )
	{
		Super::operator =( l );
		return *this;
	}

public: // Assignment: Value

	// = Value
	Array2 &
	operator =( T const & t )
	{
		Super::operator =( t );
		return *this;
	}

public: // Subscript

	// array( i1, i2 ) const
	T const &
	operator ()( int const i1, int const i2 ) const
	{
		assert( contains( i1, i2 ) );
		return sdata_[ ( i1 * z2_ ) + i2 ];
	}

	// array( i1, i2 )
	T &
	operator ()( int const i1, int const i2 )
	{
		assert( contains( i1, i2 ) );
		return sdata_[ ( i1 * z2_ ) + i2 ];
	}

	// Linear Index
	size_type
	index( int const i1, int const i2 ) const
	{
		return ( ( i1 * z2_ ) + i2 ) - shift_;
	}

public: // Slice Proxy Generators

	// array( s1, s2 ) const
	Array2S< T >
	operator ()( IS const & s1, IS const & s2 ) const
	{
		DS const d1( I1_, s1, z2_ );
		DS const d2( I2_, s2 );
		return Array2S< T >( data_, -shift_, d1, d2 );
	}

	// array( i1, s2 ) const
	Array1S< T >
	operator ()( int const i1, IS const & s2 ) const
	{
		std::int64_t k( -shift_ );
		k += slice_k( I1_, i1, z2_ );
		DS const d2( I2_, s2 );
		return Array1S< T >( data_, k, d2 );
	}

	// array( s1, i2 ) const
	Array1S< T >
	operator ()( IS const & s1, int const i2 ) const
	{
		std::int64_t k( -shift_ );
		DS const d1( I1_, s1, z2_ );
		k += slice_k( I2_, i2 );
		return Array1S< T >( data_, k, d1 );
	}

	// array( s1, s2 )
	Array2S< T >
	operator ()( IS const & s1, IS const & s2 )
	{
		DS const d1( I1_, s1, z2_ );
		DS const d2( I2_, s2 );
		return Array2S< T >( data_, -shift_, d1, d2 );
	}

	// array( i1, s2 )
	Array1S< T >
	operator ()( int const i1, IS const & s2 )
	{
		std::int64_t k( -shift_ );
		k += slice_k( I1_, i1, z2_ );
		DS const d2( I2_, s2 );
		return Array1S< T >( data_, k, d2 );
	}

	// array( s1, i2 )
	Array1S< T >
	operator ()( IS const & s1, int const i2 )
	{
		std::int64_t k( -shift_ );
		DS const d1( I1_, s1, z2_ );
		k += slice_k( I2_, i2 );
		return Array1S< T >( data_, k, d1 );
	}

public: // Predicate

	// Contains Indexed Element?
	bool
	contains( int const i1, int const i2 ) const
	{
		return ( I1_.contains( i1 ) && I2_.contains( i2 ) );
	}

	// Conformable?
	template< typename U >
	bool
	conformable( Array2< U > const & a ) const
	{
		return ( ( z1_ == a.z1_ ) && ( z2_ == a.z2_ ) );
	}

	// Conformable?
	template< typename U >
	bool
	conformable( Array2S< U > const & a ) const
	{
		return ( ( z1_ == a.size1() ) && ( z2_ == a.size2() ) );
	}

	// Equal Dimensions?
	template< typename U >
	bool
	equal_dimensions( Array2< U > const & a ) const
	{
		return ( ( I1_ == a.I1_ ) && ( I2_ == a.I2_ ) );
	}

	// Equal Dimensions?
	template< typename U >
	bool
	equal_dimensions( Array2S< U > const & a ) const
	{
		return ( ( l1() == 1 ) && ( u1() == a.u1() ) && ( l2() == 1 ) && ( u2() == a.u2() ) );
	}

	// Is Identity?
	bool
	is_identity() const
	{
		static T const ZERO( 0 );
		static T const ONE( 1 );
		Array2 const & A( *this ); // Shorthand name
		if ( ! square() ) { // Non-square
			return false;
		} else { // Square
			for ( int i = l1(), ie = u1(); i <= ie; ++i ) {
				int const jd( l2() + ( i - l1() ) ); // Col index of diagonal
				for ( int j = l2(), je = u2(); j <= je; ++j ) {
					if ( A( i, j ) != ( j == jd ? ONE : ZERO ) ) return false;
				}
			}
			return true;
		}
	}

	// Square?
	bool
	square() const
	{
		return ( z1_ == z2_ );
	}

	// Symmetric?
	bool
	symmetric() const
	{
		Array2 const & A( *this ); // Shorthand name
		if ( I1_ != I2_ ) { // Unequal index ranges
			return false;
		} else { // Equal index ranges
			for ( int i = l1(), ie = u1(); i <= ie; ++i ) {
				size_type l( A.index( i, l2() ) );
				for ( int j = l2(); j < i; ++j, ++l ) {
					if ( A[ l ] != A( j, i ) ) return false;
				}
			}
			return true;
		}
	}

public: // Inspector

	// Rank
	int
	rank() const
	{
		return 2;
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

public: // Modifier

	// Clear
	Array2 &
	clear()
	{
		Super::clear();
		I1_.clear();
		I2_.clear();
		z1_ = z2_ = 0u;
		return *this;
	}

	// Set to the Identity Matrix
	Array2 &
	to_identity()
	{
		assert( square() );
		Array2 & A( *this ); // Shorthand name
		A = T( 0 ); // Zero the array
		T const One( T( 1 ) );
		for ( size_type l = 0, l_inc = z1_ + 1; l < size_; l += l_inc ) {
			A[ l ] = One;
		}
		return *this;
	}

	// Set to Diagonal Matrix with Uniform Value
	Array2 &
	to_diag( T const & d )
	{
		assert( square() );
		Array2 & A( *this ); // Shorthand name
		A = T( 0 ); // Zero the array
		for ( size_type l = 0, l_inc = z1_ + 1; l < size_; l += l_inc ) {
			A[ l ] = d;
		}
		return *this;
	}

	// Transpose
	Array2 &
	transpose()
	{
		using std::swap; // Allows std::swap to be used if no T version
		assert( square() ); // So dimensions aren't changed
		Array2 & A( *this ); // Shorthand name
		for ( size_type i = 0; i < z2_; ++i ) {
			for ( size_type j = 0, l = i * z2_, lT = i; j < i; ++j, ++l, lT += z2_ ) {
				swap( A[ lT ], A[ l ] );
			}
		}
		return *this;
	}


protected: // Functions

	// Dimension by IndexRange
	virtual
	bool
	dimension_assign( IR const & I1, IR const & I2 ) = 0;

	// Clear on Move
	void
	clear_move()
	{
		I1_.clear();
		I2_.clear();
		z1_ = z2_ = 0u;
	}

	// Swap
	void
	swap2( Array2 & v )
	{
		swapB( v );
		I1_.swap( v.I1_ );
		I2_.swap( v.I2_ );
		std::swap( z1_, v.z1_ );
		std::swap( z2_, v.z2_ );
	}

protected: // Data

	IR I1_; // Index range of dim 1
	IR I2_; // Index range of dim 2

	size_type z1_; // Size of dim 1
	size_type z2_; // Size of dim 2

}; // Array2

// Conformable?
template< typename U, typename V >
inline
bool
conformable( Array2< U > const & a, Array2< V > const & b )
{
	return a.conformable( b );
}

// Conformable?
template< typename U, typename V >
inline
bool
conformable( Array2< U > const & a, Array2S< V > const & b )
{
	return a.conformable( b );
}

// Conformable?
template< typename U, typename V >
inline
bool
conformable( Array2S< U > const & a, Array2< V > const & b )
{
	return b.conformable( a );
}

// Equal Dimensions?
template< typename U, typename V >
inline
bool
equal_dimensions( Array2< U > const & a, Array2< V > const & b )
{
	return a.equal_dimensions( b );
}

} // ObjexxFCL

#endif // ObjexxFCL_Array2_hh_INCLUDED
