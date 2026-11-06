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

// sign /////

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

// lbound /////

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

// ubound /////

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
int
isize( Array< T > const & a )
{
	return static_cast< int >( size( a ) );
}

// pack /////

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

} // ObjexxFCL

#endif // ObjexxFCL_Array_functions_hh_INCLUDED
