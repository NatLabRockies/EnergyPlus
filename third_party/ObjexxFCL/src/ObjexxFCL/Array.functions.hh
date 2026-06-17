#ifndef ObjexxFCL_Array_functions_hh_INCLUDED
#define ObjexxFCL_Array_functions_hh_INCLUDED

// Array Functions
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
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Array3D.hh>
#include <ObjexxFCL/Array4D.hh>
#include <ObjexxFCL/Fmath.hh>

// C++ Headers
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>

namespace ObjexxFCL {

// allocation /////

template< typename T >
inline
bool
allocated( Array< T > const & a )
{
	return a.allocated();
}

template< typename T >
inline
void
allocate( Array1D< T > & a, IndexRange const & I )
{
	a.allocate( I );
}

template< typename T >
inline
void
allocate( Array2D< T > & a, IndexRange const & I1, IndexRange const & I2 )
{
	a.allocate( I1, I2 );
}

template< typename T >
inline
void
allocate( Array3D< T > & a, IndexRange const & I1, IndexRange const & I2, IndexRange const & I3 )
{
	a.allocate( I1, I2, I3 );
}

template< typename T >
inline
void
allocate( Array4D< T > & a, IndexRange const & I1, IndexRange const & I2, IndexRange const & I3, IndexRange const & I4 )
{
	a.allocate( I1, I2, I3, I4 );
}

template< typename T >
inline
void
deallocate( Array< T > & a )
{
	a.clear();
}

// all /////

inline
bool
all( Array< bool > const & a )
{
	assert( a.size_bounded() );
	if ( a.empty() ) return true;
	for ( BArray::size_type i = 0; i < a.size(); ++i ) {
		if ( ! a[ i ] ) return false;
	}
	return true;
}

// any /////

inline
bool
any( Array< bool > const & a )
{
	assert( a.size_bounded() );
	if ( a.empty() ) return false;
	for ( BArray::size_type i = 0; i < a.size(); ++i ) {
		if ( a[ i ] ) return true;
	}
	return false;
}

// abs /////

template< typename T >
inline
Array1D< T >
abs( Array1< T > const & a )
{
	assert( a.size_bounded() );
	Array1D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = std::abs( r[ i ] );
	}
	return r;
}

// Negation /////

inline
Array1D< bool >
operator !( Array1< bool > const & a )
{
	Array1D< bool > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = ! r[ i ];
	}
	return r;
}

inline
Array2D< bool >
operator !( Array2< bool > const & a )
{
	Array2D< bool > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = ! r[ i ];
	}
	return r;
}

inline
Array3D< bool >
operator !( Array3< bool > const & a )
{
	Array3D< bool > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = ! r[ i ];
	}
	return r;
}

inline
Array4D< bool >
operator !( Array4< bool > const & a )
{
	Array4D< bool > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = ! r[ i ];
	}
	return r;
}



// pow /////

template< typename T, typename X >
inline
Array1D< T >
pow( Array1< T > const & a, X const & x )
{
	assert( a.size_bounded() );
	Array1D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = T( std::pow( r[ i ], x ) );
	}
	return r;
}

template< typename T, typename X >
inline
Array2D< T >
pow( Array2< T > const & a, X const & x )
{
	assert( a.size_bounded() );
	Array2D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = T( std::pow( r[ i ], x ) );
	}
	return r;
}

template< typename T, typename X >
inline
Array3D< T >
pow( Array3< T > const & a, X const & x )
{
	assert( a.size_bounded() );
	Array3D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = T( std::pow( r[ i ], x ) );
	}
	return r;
}

template< typename T, typename X >
inline
Array4D< T >
pow( Array4< T > const & a, X const & x )
{
	assert( a.size_bounded() );
	Array4D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = T( std::pow( r[ i ], x ) );
	}
	return r;
}

// sign /////

template< typename T, typename X >
inline
Array1D< T >
sign( Array1< T > const & a, X const & x )
{
	assert( a.size_bounded() );
	Array1D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( r[ i ], x );
	}
	return r;
}

template< typename T, typename X >
inline
Array2D< T >
sign( Array2< T > const & a, X const & x )
{
	assert( a.size_bounded() );
	Array2D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( r[ i ], x );
	}
	return r;
}

template< typename T, typename X >
inline
Array3D< T >
sign( Array3< T > const & a, X const & x )
{
	assert( a.size_bounded() );
	Array3D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( r[ i ], x );
	}
	return r;
}

template< typename T, typename X >
inline
Array4D< T >
sign( Array4< T > const & a, X const & x )
{
	assert( a.size_bounded() );
	Array4D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( r[ i ], x );
	}
	return r;
}

template< typename X, typename T >
inline
Array1D< X >
sign( X const & x, Array1< T > const & a )
{
	assert( a.size_bounded() );
	Array1D< X > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( x, r[ i ] );
	}
	return r;
}

template< typename X, typename T >
inline
Array2D< X >
sign( X const & x, Array2< T > const & a )
{
	assert( a.size_bounded() );
	Array2D< X > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( x, r[ i ] );
	}
	return r;
}

template< typename X, typename T >
inline
Array3D< X >
sign( X const & x, Array3< T > const & a )
{
	assert( a.size_bounded() );
	Array3D< X > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( x, r[ i ] );
	}
	return r;
}

template< typename X, typename T >
inline
Array4D< X >
sign( X const & x, Array4< T > const & a )
{
	assert( a.size_bounded() );
	Array4D< X > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( x, r[ i ] );
	}
	return r;
}

// count /////

inline
BArray::size_type
count( Array< bool > const & a )
{
	assert( a.size_bounded() );
	BArray::size_type c( 0u );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		if ( a[ i ] ) ++c;
	}
	return c;
}

inline
BArray::size_type
count( Array1< bool > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		return count( a );
	default:
		assert( false );
		return 0;
	}
}

inline
Array1D< BArray::size_type >
count( Array2< bool > const & a, int const dim )
{
	assert( a.size_bounded() );
	typedef  BArray::size_type  size_type;
	size_type const as1( a.size1() );
	size_type const as2( a.size2() );
	switch ( dim ) {
	case 1:
		{
		Array1D< size_type > r( static_cast< int >( as2 ) );
		for ( size_type i2 = 0; i2 < as2; ++i2 ) {
			size_type c( 0u );
			for ( size_type i1 = 0, l = i2; i1 < as1; ++i1, l += as2 ) {
				if ( a[ l ] ) ++c;
			}
			r[ i2 ] = c;
		}
		return r;
		}
	case 2:
		{
		Array1D< size_type > r( static_cast< int >( as1 ) );
		for ( size_type i1 = 0, l = 0; i1 < as1; ++i1 ) {
			size_type c( 0u );
			for ( size_type i2 = 0; i2 < as2; ++i2, ++l ) {
				if ( a[ l ] ) ++c;
			}
			r[ i1 ] = c;
		}
		return r;
		}
	default:
		assert( false );
		return Array1D< size_type >();
	}
}

// contiguous /////

// lbound /////

template< typename T >
inline
Array1D< int >
lbound( Array1< T > const & a )
{
	return Array1D< int >( 1, a.l1() );
}

template< typename T >
inline
Array1D< int >
lbound( Array2< T > const & a )
{
	return Array1D< int >( 2, { a.l1(), a.l2() } );
}

template< typename T >
inline
Array1D< int >
lbound( Array3< T > const & a )
{
	return Array1D< int >( 3, { a.l1(), a.l2(), a.l3() } );
}

template< typename T >
inline
Array1D< int >
lbound( Array4< T > const & a )
{
	return Array1D< int >( 4, { a.l1(), a.l2(), a.l3(), a.l4() } );
}

template< typename T >
inline
int
lbound( Array1< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		return a.l1();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
lbound( Array2< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		return a.l1();
	case 2:
		return a.l2();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
lbound( Array3< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		return a.l1();
	case 2:
		return a.l2();
	case 3:
		return a.l3();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
lbound( Array4< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		return a.l1();
	case 2:
		return a.l2();
	case 3:
		return a.l3();
	case 4:
		return a.l4();
	default:
		assert( false );
		return 0;
	}
}

// ubound /////

template< typename T >
inline
Array1D< int >
ubound( Array1< T > const & a )
{
	assert( a.size_bounded() );
	return Array1D< int >( 1, a.u1() );
}

template< typename T >
inline
Array1D< int >
ubound( Array2< T > const & a )
{
	assert( a.size_bounded() );
	return Array1D< int >( 2, { a.u1(), a.u2() } );
}

template< typename T >
inline
Array1D< int >
ubound( Array3< T > const & a )
{
	assert( a.size_bounded() );
	return Array1D< int >( 3, { a.u1(), a.u2(), a.u3() } );
}

template< typename T >
inline
Array1D< int >
ubound( Array4< T > const & a )
{
	assert( a.size_bounded() );
	return Array1D< int >( 4, { a.u1(), a.u2(), a.u3(), a.u4() } );
}

template< typename T >
inline
int
ubound( Array1< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		assert( a.I1().bounded() );
		return a.u1();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
ubound( Array2< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		assert( a.I1().bounded() );
		return a.u1();
	case 2:
		return a.u2();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
ubound( Array3< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		assert( a.I1().bounded() );
		return a.u1();
	case 2:
		return a.u2();
	case 3:
		return a.u3();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
ubound( Array4< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		assert( a.I1().bounded() );
		return a.u1();
	case 2:
		return a.u2();
	case 3:
		return a.u3();
	case 4:
		return a.u4();
	default:
		assert( false );
		return 0;
	}
}

// shape /////

template< typename T >
inline
Array1D< int >
shape( Array1< T > const & a )
{
	assert( a.size_bounded() );
	return Array1D< int >( 1, a.isize1() );
}

template< typename T >
inline
Array1D< int >
shape( Array2< T > const & a )
{
	assert( a.size_bounded() );
	return Array1D< int >( 2, { a.isize1(), a.isize2() } );
}

template< typename T >
inline
Array1D< int >
shape( Array3< T > const & a )
{
	assert( a.size_bounded() );
	return Array1D< int >( 3, { a.isize1(), a.isize2(), a.isize3() } );
}

template< typename T >
inline
Array1D< int >
shape( Array4< T > const & a )
{
	assert( a.size_bounded() );
	return Array1D< int >( 4, { a.isize1(), a.isize2(), a.isize3(), a.isize4() } );
}

// size /////

template< typename T >
inline
BArray::size_type
size( Array< T > const & a )
{
	assert( a.size_bounded() );
	return a.size();
}

template< typename T >
inline
BArray::size_type
size( Array1< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		assert( a.I1().bounded() );
		return a.size1();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
BArray::size_type
size( Array2< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		assert( a.I1().bounded() );
		return a.size1();
	case 2:
		return a.size2();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
BArray::size_type
size( Array3< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		assert( a.I1().bounded() );
		return a.size1();
	case 2:
		return a.size2();
	case 3:
		return a.size3();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
BArray::size_type
size( Array4< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		assert( a.I1().bounded() );
		return a.size1();
	case 2:
		return a.size2();
	case 3:
		return a.size3();
	case 4:
		return a.size4();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
isize( Array< T > const & a )
{
	return static_cast< int >( size( a ) );
}

template< template< typename > class A, typename T >
inline
typename std::enable_if< std::is_base_of< Array< T >, A< T > >::value, int >::type
isize( A< T > const & a, int const dim )
{
	return static_cast< int >( size( a, dim ) );
}

// pack /////

template< typename T >
inline
Array1D< T >
pack( Array< T > const & a, bool const mask )
{
	assert( a.size_bounded() );
	if ( mask ) {
		return Array1D< T >( a, a.isize() ); // All elements
	} else {
		return Array1D< T >( 0 ); // Empty array
	}
}

template< typename T >
inline
Array1D< T >
pack( Array1< T > const & a, Array1< bool > const & mask )
{
	assert( a.size_bounded() );
	assert( conformable( a, mask ) );
	typedef  BArray::size_type  size_type;
	size_type n( 0u );
	for ( size_type i = 0, e = mask.size(); i < e; ++i ) {
		if ( mask[ i ] ) ++n;
	}
	Array1D< T > r( static_cast< int >( n ) );
	for ( size_type i = 0, e = mask.size(), k = 0; i < e; ++i ) {
		if ( mask[ i ] ) r[ k++ ] = a[ i ];
	}
	return r;
}


// eoshift /////

template< typename T >
inline
Array1D< T >
eoshift( Array1< T > const & a, int const shift, T const bdy = TypeTraits< T >::initial_value(), int const dim = 1 )
{
	assert( a.size_bounded() );
	assert( dim == 1 );
#ifdef NDEBUG
	static_cast< void >( dim ); // Suppress unused warning
#endif
	Array1D< T > o( Array1D< T >::shape( a, bdy ) );
	int const b( a.l() + std::max( shift, 0 ) ), e( a.u() + std::min( shift, 0 ) );
	for ( int i = b, j = std::max( 1 - shift, 1 ); i <= e; ++i, ++j ) {
		o( j ) = a( i );
	}
	return o;
}

// sum /////

template< typename T >
inline
T
sum( Array< T > const & a )
{
	assert( a.size_bounded() );
	typedef  BArray::size_type  size_type;
	T r( 0 );
	for ( size_type i = 0, e = a.size(); i < e; ++i ) {
		r += a[ i ];
	}
	return r;
}

template< typename T >
inline
T
sum( Array1< T > const & a, int const dim )
{
	assert( a.size_bounded() );
	assert( dim == 1 );
#ifdef NDEBUG
	static_cast< void >( dim ); // Suppress unused warning
#endif
	typedef  BArray::size_type  size_type;
	T r( 0 );
	for ( size_type i = 0, e = a.size(); i < e; ++i ) {
		r += a[ i ];
	}
	return r;
}

template< typename T >
inline
Array1D< T >
sum( Array2< T > const & a, int const dim )
{
	assert( a.size_bounded() );
	typedef  BArray::size_type  size_type;
	size_type const as1( a.size1() );
	size_type const as2( a.size2() );
	switch ( dim ) {
	case 1:
		{
		Array1D< T > r( static_cast< int >( as2 ), T( 0 ) );
		for ( size_type i1 = 0, l = 0; i1 < as1; ++i1 ) {
			for ( size_type i2 = 0; i2 < as2; ++i2, ++l ) {
				r[ i2 ] += a[ l ];
			}
		}
		return r;
		}
	case 2:
		{
		Array1D< T > r( static_cast< int >( as1 ) );
		for ( size_type i1 = 0, l = 0; i1 < as1; ++i1 ) {
			T s( 0 );
			for ( size_type i2 = 0; i2 < as2; ++i2, ++l ) {
				s += a[ l ];
			}
			r[ i1 ] = s;
		}
		return r;
		}
	default:
		assert( false );
		return Array1D< T >();
	}
}

template< typename T >
inline
T
sum( Array< T > const & a, Array< bool > const & mask )
{
	assert( a.size_bounded() );
	typedef  BArray::size_type  size_type;
	assert( a.size() == mask.size() ); // Fortran compliance requires conformable so this is looser
	T r( 0 );
	for ( size_type i = 0, e = a.size(); i < e; ++i ) {
		if ( mask[ i ] ) r += a[ i ];
	}
	return r;
}


// minval /////

template< typename T >
inline
T
minval( Array< T > const & a )
{
	assert( a.size_bounded() );
	typedef  BArray::size_type  size_type;
	T r( a.empty() ? std::numeric_limits< T >::max() : a[ 0 ] );
	for ( size_type i = 1; i < a.size(); ++i ) {
		r = std::min( r, a[ i ] );
	}
	return r;
}

// maxval /////

template< typename T >
inline
T
maxval( Array< T > const & a )
{
	assert( a.size_bounded() );
	typedef  BArray::size_type  size_type;
	T r( a.empty() ? std::numeric_limits< T >::lowest() : a[ 0 ] );
	for ( size_type i = 1; i < a.size(); ++i ) {
		r = std::max( r, a[ i ] );
	}
	return r;
}

// minloc /////

template< typename T >
inline
Array1D< int >
minloc( Array1< T > const & a )
{
	assert( a.size_bounded() );
	typedef  BArray::size_type  size_type;
	int const as( a.isize1() );
	Array1D< int > loc( 1, a.empty() ? 0 : 1 ); // F2008 standard => 0 for empty arrays
	T r( a.empty() ? std::numeric_limits< T >::max() : a[ 0 ] );
	size_type l( 1u );
	for ( int i = 2; i <= as; ++i, ++l ) {
		if ( a[ l ] < r ) {
			r = a[ l ];
			loc = { i };
		}
	}
	return loc;
}


template< typename T >
inline
int
minloc( Array1< T > const & a, int const dim )
{
	assert( a.size_bounded() );
	switch ( dim ) {
	case 1:
		return minloc( a )( 1 );
	default:
		assert( false );
		return 0;
	}
}


template< typename T >
inline
Array1D< int >
minloc( Array1< T > const & a, Array1< bool > const & mask )
{
	assert( a.size_bounded() );
	assert( conformable( a, mask ) );
	typedef  BArray::size_type  size_type;
	int const as( a.isize1() );
	Array1D< int > loc( 1, a.empty() ? 0 : 1 ); // F2008 standard => 0 for empty arrays
	T r( std::numeric_limits< T >::max() );
	size_type l( 0u );
	bool first( true );
	for ( int i = 1; i <= as; ++i, ++l ) {
		if ( mask[ l ] ) {
			if ( first ) {
				first = false;
				r = a[ l ];
				loc = { i };
			} else if ( a[ l ] < r ) {
				r = a[ l ];
				loc = { i };
			}
		}
	}
	return loc;
}



} // ObjexxFCL

#endif // ObjexxFCL_Array_functions_hh_INCLUDED
