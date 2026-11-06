#ifndef ObjexxFCL_Fmath_hh_INCLUDED
#define ObjexxFCL_Fmath_hh_INCLUDED

// Fortran Intrinsic-Compatible and General Math Functions
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

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <type_traits>

namespace ObjexxFCL {

typedef  std::intmax_t  SSize;

// min /////

// min( int, int )
inline
int
min( int const a, int const b )
{
	return ( a < b ? a : b );
}

// min( unsigned long, unsigned long )
inline
unsigned long int
min( unsigned long int const a, unsigned long int const b )
{
	return ( a < b ? a : b );
}

// min( double, double )
inline
double
min( double const a, double const b )
{
	return ( a < b ? a : b );
}

// Use std::min for 2 arguments not covered by the above overloads
using std::min;

// min( unsigned long, unsigned long, unsigned long, unsigned long, ... )
template< typename... Ts >
inline
unsigned long int
min( unsigned long int const a, unsigned long int const b, unsigned long int const c, unsigned long int const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( double, double, double )
inline
double
min( double const a, double const b, double const c )
{
	return ( a < b ? ( a < c ? a : c ) : ( b < c ? b : c ) );
}

// min( double, double, double, double, ... )
template< typename... Ts >
inline
double
min( double const a, double const b, double const c, double const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( a, b, c )
template< typename T >
inline
T const &
min( T const & a, T const & b, T const & c )
{
	return ( a < b ? ( a < c ? a : c ) : ( b < c ? b : c ) );
}

// min( a, b, c, d, ... )
template< typename T, typename... Ts >
inline
T const &
min( T const & a, T const & b, T const & c, T const & d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// max /////

// max( int, int )
inline
int
max( int const a, int const b )
{
	return ( a < b ? b : a );
}

// max( unsigned long, unsigned long )
inline
unsigned long int
max( unsigned long int const a, unsigned long int const b )
{
	return ( a < b ? b : a );
}

// max( double, double )
inline
double
max( double const a, double const b )
{
	return ( a < b ? b : a );
}

// Use std::max for 2 arguments not covered by the above overloads
using std::max;

// max( int, int, int )
inline
int
max( int const a, int const b, int const c )
{
	return ( a < b ? ( b < c ? c : b ) : ( a < c ? c : a ) );
}

// max( int, int, int, int, ... )
template< typename... Ts >
inline
int
max( int const a, int const b, int const c, int const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( double, double, double )
inline
double
max( double const a, double const b, double const c )
{
	return ( a < b ? ( b < c ? c : b ) : ( a < c ? c : a ) );
}

// max( double, double, double, double, ... )
template< typename... Ts >
inline
double
max( double const a, double const b, double const c, double const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( a, b, c )
template< typename T >
inline
T const &
max( T const & a, T const & b, T const & c )
{
	return ( a < b ? ( b < c ? c : b ) : ( a < c ? c : a ) );
}

// max( a, b, c, d, ... )
template< typename T, typename... Ts >
inline
T const &
max( T const & a, T const & b, T const & c, T const & d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// General /////

// abs( x ) == | x |
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
abs( T const & x )
{
	return ( x < T( 0 ) ? -x : x );
}

// CEILING( x )
template< typename T >
inline
int
CEILING( T const & x )
{
	return int( std::ceil( x ) );
}

// sign( x )
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
int
sign( T const & x )
{
	return ( x >= T( 0 ) ? +1 : -1 );
}

// Sign Transfer from Second Argument to First Argument
template< typename X, typename Y, class = typename std::enable_if< std::is_arithmetic< X >::value && std::is_arithmetic< Y >::value, X >::type >
inline
X
sign( X const & x, Y const & y )
{
	return ( y >= Y( 0 ) ? abs( x ) : -abs( x ) );
}

// nint( x ): Nearest int
template< typename T >
inline
int
nint( T const & x )
{
	return static_cast< int >( x + ( sign( x ) * T( 0.5 ) ) );
}

// nlint( x ): Nearest long int
template< typename T >
inline
int64_t
nint64( T const & x )
{
	return static_cast< int64_t >( x + ( sign( x ) * T( 0.5 ) ) );
}

// Mod function selector class for non-integer types
template< typename T, bool >
struct ModSelector
{
	static
	T
	mod( T const & x, T const & y )
	{
		return ( y != T( 0 ) ? x - ( T( static_cast< SSize >( x / y ) ) * y ) : T( 0 ) );
	}
};

// Mod function selector class for integer types
//  When used with negative integer arguments this assumes integer division
//   rounds towards zero (de facto and future C++ standard)
template< typename T >
struct ModSelector< T, true >
{
	static
	T
	mod( T const & x, T const & y )
	{
		return ( y != T( 0 ) ? x - ( ( x / y ) * y ) : T( 0 ) );
	}
};

// x(mod y) computational modulo returning magnitude < | y | and sign of x
//  When used with negative integer arguments this assumes integer division
//   rounds towards zero (de facto and future C++ standard)
template< typename T >
inline
T
mod( T const & x, T const & y )
{
	return ModSelector< T, std::numeric_limits< T >::is_integer >::mod( x, y );
}

// i(mod n) : double Arguments
inline
double
mod( double const i, double const n )
{
	return ( n != 0.0 ? std::fmod( i, n ) : 0.0 );
}

// Power and Roots /////

// square( x ) == x^2
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
square( T const x )
{
	return x * x;
}

// pow_2( x ) == x^2
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_2( T const x )
{
	return x * x;
}

// pow_3( x ) == x^3
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_3( T const x )
{
	return x * x * x;
}

// pow_4( x ) == x^4
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_4( T const x )
{
	T const t( x * x );
	return t * t;
}

// pow_5( x ) == x^5
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_5( T const x )
{
	T const t( x * x );
	return t * t * x;
}

// pow_6( x ) == x^6
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_6( T const x )
{
	T const t( x * x * x );
	return t * t;
}

// pow_7( x ) == x^7
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_7( T const x )
{
	T const t( x * x * x );
	return t * t * x;
}

// root_4( x ) == x^(1/4)
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
root_4( T const x )
{
	return T( std::sqrt( std::sqrt( x ) ) );
}

} // ObjexxFCL

#endif // ObjexxFCL_Fmath_hh_INCLUDED
