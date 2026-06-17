#ifndef ObjexxFCL_ArrayS_functions_hh_INCLUDED
#define ObjexxFCL_ArrayS_functions_hh_INCLUDED

// ArrayS Functions
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
#include <ObjexxFCL/Array1S.hh>
#include <ObjexxFCL/Array2S.hh>
#include <ObjexxFCL/Array3S.hh>
#include <ObjexxFCL/Array4S.hh>
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
allocated( ArrayS< T > const & a )
{
	return a.allocated();
}

// all /////

inline
bool
all( Array1S< bool > const & a )
{
	if ( a.empty() ) return true;
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		if ( ! a( i ) ) return false;
	}
	return true;
}

inline
bool
all( Array2S< bool > const & a )
{
	if ( a.empty() ) return true;
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			if ( ! a( i1, i2 ) ) return false;
		}
	}
	return true;
}

inline
bool
all( Array3S< bool > const & a )
{
	if ( a.empty() ) return true;
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				if ( ! a( i1, i2, i3 ) ) return false;
			}
		}
	}
	return true;
}

inline
bool
all( Array4S< bool > const & a )
{
	if ( a.empty() ) return true;
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				for ( int i4 = 1, e4 = a.u4(); i4 <= e4; ++i4 ) {
					if ( ! a( i1, i2, i3, i4 ) ) return false;
				}
			}
		}
	}
	return true;
}

// any /////

inline
bool
any( Array1S< bool > const & a )
{
	if ( a.empty() ) return false;
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		if ( a( i ) ) return true;
	}
	return false;
}

inline
bool
any( Array2S< bool > const & a )
{
	if ( a.empty() ) return false;
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			if ( a( i1, i2 ) ) return true;
		}
	}
	return false;
}

inline
bool
any( Array3S< bool > const & a )
{
	if ( a.empty() ) return false;
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				if ( a( i1, i2, i3 ) ) return true;
			}
		}
	}
	return false;
}

inline
bool
any( Array4S< bool > const & a )
{
	if ( a.empty() ) return false;
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				for ( int i4 = 1, e4 = a.u4(); i4 <= e4; ++i4 ) {
					if ( a( i1, i2, i3, i4 ) ) return true;
				}
			}
		}
	}
	return false;
}

// abs /////

template< typename T >
inline
Array1D< T >
abs( Array1S< T > const & a )
{
	Array1D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = std::abs( r[ i ] );
	}
	return r;
}

// Negation /////

inline
Array1D< bool >
operator !( Array1S< bool > const & a )
{
	Array1D< bool > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = ! r[ i ];
	}
	return r;
}

inline
Array2D< bool >
operator !( Array2S< bool > const & a )
{
	Array2D< bool > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = ! r[ i ];
	}
	return r;
}

inline
Array3D< bool >
operator !( Array3S< bool > const & a )
{
	Array3D< bool > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = ! r[ i ];
	}
	return r;
}

inline
Array4D< bool >
operator !( Array4S< bool > const & a )
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
pow( Array1S< T > const & a, X const & x )
{
	Array1D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = T( std::pow( r[ i ], x ) );
	}
	return r;
}

template< typename T, typename X >
inline
Array2D< T >
pow( Array2S< T > const & a, X const & x )
{
	Array2D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = T( std::pow( r[ i ], x ) );
	}
	return r;
}

template< typename T, typename X >
inline
Array3D< T >
pow( Array3S< T > const & a, X const & x )
{
	Array3D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = T( std::pow( r[ i ], x ) );
	}
	return r;
}

template< typename T, typename X >
inline
Array4D< T >
pow( Array4S< T > const & a, X const & x )
{
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
sign( Array1S< T > const & a, X const & x )
{
	Array1D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( r[ i ], x );
	}
	return r;
}

template< typename T, typename X >
inline
Array2D< T >
sign( Array2S< T > const & a, X const & x )
{
	Array2D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( r[ i ], x );
	}
	return r;
}

template< typename T, typename X >
inline
Array3D< T >
sign( Array3S< T > const & a, X const & x )
{
	Array3D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( r[ i ], x );
	}
	return r;
}

template< typename T, typename X >
inline
Array4D< T >
sign( Array4S< T > const & a, X const & x )
{
	Array4D< T > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( r[ i ], x );
	}
	return r;
}

template< typename X, typename T >
inline
Array1D< X >
sign( X const & x, Array1S< T > const & a )
{
	Array1D< X > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( x, r[ i ] );
	}
	return r;
}

template< typename X, typename T >
inline
Array2D< X >
sign( X const & x, Array2S< T > const & a )
{
	Array2D< X > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( x, r[ i ] );
	}
	return r;
}

template< typename X, typename T >
inline
Array3D< X >
sign( X const & x, Array3S< T > const & a )
{
	Array3D< X > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( x, r[ i ] );
	}
	return r;
}

template< typename X, typename T >
inline
Array4D< X >
sign( X const & x, Array4S< T > const & a )
{
	Array4D< X > r( a );
	for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
		r[ i ] = sign( x, r[ i ] );
	}
	return r;
}

// count /////

inline
BArray::size_type
count( Array1S< bool > const & a )
{
	BArray::size_type c( 0u );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		if ( a( i ) ) ++c;
	}
	return c;
}

inline
BArray::size_type
count( Array2S< bool > const & a )
{
	BArray::size_type c( 0u );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			if ( a( i1, i2 ) ) ++c;
		}
	}
	return c;
}

inline
BArray::size_type
count( Array3S< bool > const & a )
{
	BArray::size_type c( 0u );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				if ( a( i1, i2, i3 ) ) ++c;
			}
		}
	}
	return c;
}

inline
BArray::size_type
count( Array4S< bool > const & a )
{
	BArray::size_type c( 0u );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				for ( int i4 = 1, e4 = a.u4(); i4 <= e4; ++i4 ) {
					if ( a( i1, i2, i3, i4 ) ) ++c;
				}
			}
		}
	}
	return c;
}

inline
BArray::size_type
count( Array1S< bool > const & a, int const dim )
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
count( Array2S< bool > const & a, int const dim )
{
	typedef  BArray::size_type  size_type;
	switch ( dim ) {
	case 1:
		{
		Array1D< size_type > v( a.isize2() );
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			size_type c( 0u );
			for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
				if ( a( i1, i2 ) ) ++c;
			}
			v( i2 ) = c;
		}
		return v;
		}
	case 2:
		{
		Array1D< size_type > v( a.isize1() );
		for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
			size_type c( 0u );
			for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
				if ( a( i1, i2 ) ) ++c;
			}
			v( i1 ) = c;
		}
		return v;
		}
	default:
		assert( false );
		return Array1D< size_type >();
	}
}

// contiguous /////

template< typename T >
inline
bool
contiguous( ArrayS< T > const & a )
{
	return a.contiguous();
}

// lbound /////

template< typename T >
inline
Array1D< int >
lbound( Array1S< T > const & )
{
	return Array1D< int >( 1, 1 );
}

template< typename T >
inline
Array1D< int >
lbound( Array2S< T > const & )
{
	return Array1D< int >( 2, 1 );
}

template< typename T >
inline
Array1D< int >
lbound( Array3S< T > const & )
{
	return Array1D< int >( 3, 1 );
}

template< typename T >
inline
Array1D< int >
lbound( Array4S< T > const & )
{
	return Array1D< int >( 4, 1 );
}

template< typename T >
inline
int
lbound( Array1S< T > const &, int const dim )
{
	switch ( dim ) {
	case 1:
		return 1;
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
lbound( Array2S< T > const &, int const dim )
{
	switch ( dim ) {
	case 1:
		return 1;
	case 2:
		return 1;
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
lbound( Array3S< T > const &, int const dim )
{
	switch ( dim ) {
	case 1:
		return 1;
	case 2:
		return 1;
	case 3:
		return 1;
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
lbound( Array4S< T > const &, int const dim )
{
	switch ( dim ) {
	case 1:
		return 1;
	case 2:
		return 1;
	case 3:
		return 1;
	case 4:
		return 1;
	default:
		assert( false );
		return 0;
	}
}

// ubound /////

template< typename T >
inline
Array1D< int >
ubound( Array1S< T > const & a )
{
	return Array1D< int >( 1, a.u1() );
}

template< typename T >
inline
Array1D< int >
ubound( Array2S< T > const & a )
{
	return Array1D< int >( 2, { a.u1(), a.u2() } );
}

template< typename T >
inline
Array1D< int >
ubound( Array3S< T > const & a )
{
	return Array1D< int >( 3, { a.u1(), a.u2(), a.u3() } );
}

template< typename T >
inline
Array1D< int >
ubound( Array4S< T > const & a )
{
	return Array1D< int >( 4, { a.u1(), a.u2(), a.u3(), a.u4() } );
}

template< typename T >
inline
int
ubound( Array1S< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		return a.u1();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
int
ubound( Array2S< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
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
ubound( Array3S< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
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
ubound( Array4S< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
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
shape( Array1S< T > const & a )
{
	return Array1D< int >( 1, a.isize1() );
}

template< typename T >
inline
Array1D< int >
shape( Array2S< T > const & a )
{
	return Array1D< int >( 2, { a.isize1(), a.isize2() } );
}

template< typename T >
inline
Array1D< int >
shape( Array3S< T > const & a )
{
	return Array1D< int >( 3, { a.isize1(), a.isize2(), a.isize3() } );
}

template< typename T >
inline
Array1D< int >
shape( Array4S< T > const & a )
{
	return Array1D< int >( 4, { a.isize1(), a.isize2(), a.isize3(), a.isize4() } );
}

// size /////

template< typename T >
inline
BArray::size_type
size( ArrayS< T > const & a )
{
	return a.size();
}

template< typename T >
inline
BArray::size_type
size( Array1S< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		return a.size1();
	default:
		assert( false );
		return 0;
	}
}

template< typename T >
inline
BArray::size_type
size( Array2S< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
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
size( Array3S< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
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
size( Array4S< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
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
isize( ArrayS< T > const & a )
{
	return static_cast< int >( size( a ) );
}

template< template< typename > class A, typename T, class = typename std::enable_if< std::is_base_of< ArrayS< T >, A< T > >::value >::type >
inline
int
isize( A< T > const & a, int const dim )
{
	return static_cast< int >( size( a, dim ) );
}

// contig /////

template< typename T >
inline
Array1D< T >
contig( Array1S< T > const & a )
{
	return Array1D< T >( a );
}

// reshape /////

template< typename T >
inline
Array1D< T >
reshape( Array1S< T > const & a )
{
	return Array1D< T >( a );
}

template< typename T, typename I >
inline
Array1D< T >
reshape( Array1S< T > const & a, std::array< I, 1 > const & shape )
{
	typedef  BArray::size_type  size_type;
	Array1D< T > r( shape[ 0 ] );
	size_type l( 0u );
	size_type const s( r.size() );
	for ( int i = 1, e = a.u(); ( ( i <= e ) && ( l < s ) ); ++i, ++l ) {
		r[ l ] = a( i );
	}
	return r;
}

template< typename T, typename I >
inline
Array2D< T >
reshape( Array1S< T > const & a, std::array< I, 2 > const & shape )
{
	typedef  BArray::size_type  size_type;
	Array2D< T > r( shape[ 0 ], shape[ 1 ] );
	size_type l( 0u );
	size_type const s( r.size() );
	for ( int i = 1, e = a.u(); ( ( i <= e ) && ( l < s ) ); ++i, ++l ) {
		r[ l ] = a( i );
	}
	return r;
}

template< typename T, typename I >
inline
Array3D< T >
reshape( Array1S< T > const & a, std::array< I, 3 > const & shape )
{
	typedef  BArray::size_type  size_type;
	Array3D< T > r( shape[ 0 ], shape[ 1 ], shape[ 2 ] );
	size_type l( 0u );
	size_type const s( r.size() );
	for ( int i = 1, e = a.u(); ( ( i <= e ) && ( l < s ) ); ++i, ++l ) {
		r[ l ] = a( i );
	}
	return r;
}

template< typename T, typename I >
inline
Array4D< T >
reshape( Array1S< T > const & a, std::array< I, 4 > const & shape )
{
	typedef  BArray::size_type  size_type;
	Array4D< T > r( shape[ 0 ], shape[ 1 ], shape[ 2 ], shape[ 3 ] );
	size_type l( 0u );
	size_type const s( r.size() );
	for ( int i = 1, e = a.u(); ( ( i <= e ) && ( l < s ) ); ++i, ++l ) {
		r[ l ] = a( i );
	}
	return r;
}

template< typename T, typename I >
inline
Array1D< T >
reshape( Array2S< T > const & a, std::array< I, 1 > const & shape )
{
	typedef  BArray::size_type  size_type;
	Array1D< T > r( shape[ 0 ] );
	size_type l( 0u );
	size_type const s( r.size() );
	for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
		for ( int i1 = 1, e1 = a.u1(); ( ( i1 <= e1 ) && ( l < s ) ); ++i1, ++l ) {
			r[ l ] = a( i1, i2 );
		}
	}
	return r;
}

template< typename T, typename I >
inline
Array2D< T >
reshape( Array2S< T > const & a, std::array< I, 2 > const & shape )
{
	typedef  BArray::size_type  size_type;
	Array2D< T > r( shape[ 0 ], shape[ 1 ] );
	size_type l( 0u );
	size_type const s( r.size() );
	for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
		for ( int i1 = 1, e1 = a.u1(); ( ( i1 <= e1 ) && ( l < s ) ); ++i1, ++l ) {
			r[ l ] = a( i1, i2 );
		}
	}
	return r;
}

// Deferred: More reshape overloads

// pack /////

template< typename T >
inline
Array1D< T >
pack( Array1S< T > const & a, Array1< bool > const & mask )
{
	assert( conformable( a, mask ) );
	typedef  BArray::size_type  size_type;
	size_type n( 0u );
	for ( size_type l = 0, e = mask.size(); l < e; ++l ) {
		if ( mask[ l ] ) ++n;
	}
	Array1D< T > r( static_cast< int >( n ) );
	size_type l( 0u ), k( 0u );
	for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
		if ( mask[ l ] ) r[ k++ ] = a( i );
	}
	return r;
}

// unpack /////

template< typename T >
inline
Array1D< T >
unpack( Array1< T > const & a, Array1< bool > const & mask, Array1S< T > const & f )
{
	assert( mask.size_bounded() );
	assert( mask.conformable( f ) );
	typedef  BArray::size_type  size_type;
	Array1D< T > r( f );
	size_type i( 0u );
	for ( size_type l = 0, e = mask.size(); l < e; ++l ) {
		if ( mask[ l ] ) {
			r[ l ] = a[ i ];
			++i;
		}
	}
	return r;
}


// eoshift /////

template< typename T >
inline
Array1D< T >
eoshift( Array1S< T > const & a, int const shift, T const bdy = TypeTraits< T >::initial_value(), int const dim = 1 )
{
	assert( dim == 1 );
#ifdef NDEBUG
	static_cast< void >( dim ); // Suppress unused warning
#endif
	Array1D< T > o( Array1D< T >::shape( a, bdy ) );
	int const b( 1 + std::max( shift, 0 ) ), e( a.u() + std::min( shift, 0 ) );
	for ( int i = b, j = std::max( 1 - shift, 1 ); i <= e; ++i, ++j ) {
		o( j ) = a( i );
	}
	return o;
}

// sum /////

template< typename T >
inline
T
sum( Array1S< T > const & a )
{
	T r( 0 );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		r += a( i );
	}
	return r;
}

template< typename T >
inline
T
sum( Array2S< T > const & a )
{
	T r( 0 );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			r += a( i1, i2 );
		}
	}
	return r;
}

template< typename T >
inline
T
sum( Array3S< T > const & a )
{
	T r( 0 );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				r += a( i1, i2, i3 );
			}
		}
	}
	return r;
}

template< typename T >
inline
T
sum( Array4S< T > const & a )
{
	T r( 0 );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				for ( int i4 = 1, e4 = a.u4(); i4 <= e4; ++i4 ) {
					r += a( i1, i2, i3, i4 );
				}
			}
		}
	}
	return r;
}

template< typename T >
inline
T
sum( Array1S< T > const & a, int const dim )
{
	assert( dim == 1 );
#ifdef NDEBUG
	static_cast< void >( dim ); // Suppress unused warning
#endif
	T r( 0 );
	for ( int i = 1, e = a.u(); i <= e; ++i ) {
		r += a( i );
	}
	return r;
}

template< typename T >
inline
Array1D< T >
sum( Array2S< T > const & a, int const dim )
{
	switch ( dim ) {
	case 1:
		{
		Array1D< T > r( a.isize2() );
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			T s( 0 );
			for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
				s += a( i1, i2 );
			}
			r( i2 ) = s;
		}
		return r;
		}
	case 2:
		{
		Array1D< T > r( a.isize1() );
		for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
			T s( 0 );
			for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
				s += a( i1, i2 );
			}
			r( i1 ) = s;
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
sum( Array1S< T > const & a, Array1< bool > const & mask )
{
	assert( conformable( a, mask ) );
	typedef  BArray::size_type  size_type;
	T r( 0 );
	size_type l( 0u );
	for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
		if ( mask[ l ] ) r += a( i );
	}
	return r;
}

template< typename T >
inline
T
sum( Array2S< T > const & a, Array2< bool > const & mask )
{
	assert( conformable( a, mask ) );
	typedef  BArray::size_type  size_type;
	T r( 0 );
	size_type l( 0u );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2, ++l ) {
			if ( mask[ l ] ) r += a( i1, i2 );
		}
	}
	return r;
}

template< typename T >
inline
T
sum( Array3S< T > const & a, Array3< bool > const & mask )
{
	assert( conformable( a, mask ) );
	typedef  BArray::size_type  size_type;
	T r( 0 );
	size_type l( 0u );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3, ++l ) {
				if ( mask[ l ] ) r += a( i1, i2, i3 );
			}
		}
	}
	return r;
}

template< typename T >
inline
T
sum( Array4S< T > const & a, Array4< bool > const & mask )
{
	assert( conformable( a, mask ) );
	typedef  BArray::size_type  size_type;
	T r( 0 );
	size_type l( 0u );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				for ( int i4 = 1, e4 = a.u4(); i4 <= e4; ++i4, ++l ) {
					if ( mask[ l ] ) r += a( i1, i2, i3, i4 );
				}
			}
		}
	}
	return r;
}


// minval /////

template< typename T >
inline
T
minval( Array1S< T > const & a )
{
	T r( a.empty() ? std::numeric_limits< T >::max() : a( 1 ) );
	for ( int i = 2, e = a.u(); i <= e; ++i ) {
		r = std::min( r, a( i ) );
	}
	return r;
}

template< typename T >
inline
T
minval( Array2S< T > const & a )
{
	T r( a.empty() ? std::numeric_limits< T >::max() : a( 1, 1 ) );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			r = std::min( r, a( i1, i2 ) );
		}
	}
	return r;
}

template< typename T >
inline
T
minval( Array3S< T > const & a )
{
	T r( a.empty() ? std::numeric_limits< T >::max() : a( 1, 1, 1 ) );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				r = std::min( r, a( i1, i2, i3 ) );
			}
		}
	}
	return r;
}

template< typename T >
inline
T
minval( Array4S< T > const & a )
{
	T r( a.empty() ? std::numeric_limits< T >::max() : a( 1, 1, 1, 1 ) );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				for ( int i4 = 1, e4 = a.u4(); i4 <= e4; ++i4 ) {
					r = std::min( r, a( i1, i2, i3, i4 ) );
				}
			}
		}
	}
	return r;
}

// maxval /////

template< typename T >
inline
T
maxval( Array1S< T > const & a )
{
	T r( a.empty() ? std::numeric_limits< T >::lowest() : a( 1 ) );
	for ( int i = 2, e = a.u(); i <= e; ++i ) {
		r = std::max( r, a( i ) );
	}
	return r;
}

template< typename T >
inline
T
maxval( Array2S< T > const & a )
{
	T r( a.empty() ? std::numeric_limits< T >::lowest() : a( 1, 1 ) );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			r = std::max( r, a( i1, i2 ) );
		}
	}
	return r;
}

template< typename T >
inline
T
maxval( Array3S< T > const & a )
{
	T r( a.empty() ? std::numeric_limits< T >::lowest() : a( 1, 1, 1 ) );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				r = std::max( r, a( i1, i2, i3 ) );
			}
		}
	}
	return r;
}

template< typename T >
inline
T
maxval( Array4S< T > const & a )
{
	T r( a.empty() ? std::numeric_limits< T >::lowest() : a( 1, 1, 1, 1 ) );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2 ) {
			for ( int i3 = 1, e3 = a.u3(); i3 <= e3; ++i3 ) {
				for ( int i4 = 1, e4 = a.u4(); i4 <= e4; ++i4 ) {
					r = std::max( r, a( i1, i2, i3, i4 ) );
				}
			}
		}
	}
	return r;
}

// minloc /////

template< typename T >
inline
Array1D< int >
minloc( Array1S< T > const & a )
{
	Array1D< int > loc( 1, a.empty() ? 0 : 1 ); // F2008 standard => 0 for empty arrays
	T r( a.empty() ? std::numeric_limits< T >::max() : a( 1 ) );
	for ( int i = 2, e = a.u(); i <= e; ++i ) {
		if ( a( i ) < r ) {
			r = a( i );
			loc = { i };
		}
	}
	return loc;
}

template< typename T >
inline
int
minloc( Array1S< T > const & a, int const dim )
{
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
minloc( Array1S< T > const & a, Array1< bool > const & mask )
{
	assert( conformable( a, mask ) );
	typedef  BArray::size_type  size_type;
	Array1D< int > loc( 1, a.empty() ? 0 : 1 ); // F2008 standard => 0 for empty arrays
	T r( std::numeric_limits< T >::max() );
	size_type l( 0u );
	bool first( true );
	for ( int i = 1, e = a.u(); i <= e; ++i, ++l ) {
		if ( mask[ l ] ) {
			if ( first ) {
				first = false;
				r = a( i );
				loc = { i };
			} else if ( a( i ) < r ) {
				r = a( i );
				loc = { i };
			}
		}
	}
	return loc;
}

template< typename T >
inline
Array1D< int >
minloc( Array2S< T > const & a, Array2< bool > const & mask )
{
	assert( conformable( a, mask ) );
	typedef  BArray::size_type  size_type;
	Array1D< int > loc( 2, 0 ); // F2008 standard => 0 for empty arrays
	T r( std::numeric_limits< T >::max() );
	size_type l( 0u );
	bool first( true );
	for ( int i1 = 1, e1 = a.u1(); i1 <= e1; ++i1 ) {
		for ( int i2 = 1, e2 = a.u2(); i2 <= e2; ++i2, ++l ) {
			if ( mask[ l ] ) {
				if ( first ) {
					first = false;
					r = a( i1, i2 );
					loc = { i1, i2 };
				} else if ( a( i1, i2 ) < r ) {
					r = a( i1, i2 );
					loc = { i1, i2 };
				}
			}
		}
	}
	return loc;
}


} // ObjexxFCL

#endif // ObjexxFCL_ArrayS_functions_hh_INCLUDED
