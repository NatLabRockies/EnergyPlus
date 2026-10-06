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

// this file contains the changes for function approximation:
// power function, and min/max
typedef  std::intmax_t  SSize;
/*
float myxor(float x, float y)
{
    unsigned long long* xPtr = (unsigned long long*)&x;
    unsigned long long* yPtr = (unsigned long long*)&y;
    unsigned long long uXor = (*xPtr) ^ (*yPtr);
    float dXor = *((float*)&uXor);
    return dXor;
}*/


// min /////

// min( short, short )

 inline
short int
min( short int const a, short int const b )
{
    //	return ( b ^ ((a ^ b) & -(a < b)) );
    return a*(a<b) + b*(b<=a);
}
/*
short int min(short int const x, short int const y)
{
    return y ^ ((x ^ y) & -(x < y));
}*/

// min( int, int )
inline
int
min( int const a, int const b )
{
	//return ( b ^ ((a ^ b) & -(a < b)) );
     return a*(a<b) + b*(b<=a);
}
/*
int min(int const x, int const y)
{
    return y ^ ((x ^ y) & -(x < y));
}*/

// min( long, long )
inline
long int
min( long int const a, long int const b )
{
	//return ( b ^ ((a ^ b) & -(a < b)) );
    return a*(a<b) + b*(b<=a);
}
/*
long int min(long int const x, long int const y)
{
    return y ^ ((x ^ y) & -(x < y));
}
*/
// min( unsigned short, unsigned short )
inline
unsigned short int
min( unsigned short int const a, unsigned short int const b )
{
	//return (b ^ ((a ^ b) & -(a < b)));
    return a*(a<b) + b*(b<=a);
}
/*
unsigned short int min(unsigned short int const x, unsigned short int const y)
{
    return y ^ ((x ^ y) & -(x < y));
}  */

// min( unsigned, unsigned )
inline
unsigned int
min( unsigned int const a, unsigned int const b )
{
	//return ( b ^ ((a ^ b) & -(a < b)) );
    return a*(a<b) + b*(b<=a);
}
/*
unsigned int min(unsigned int const x, unsigned int const y)
{
    return y ^ ((x ^ y) & -(x < y));
}*/

// min( unsigned long, unsigned long )
inline
unsigned long int
min( unsigned long int const a, unsigned long int const b )
{
	//return ( b ^ ((a ^ b) & -(a < b)) );
    return a*(a<b) + b*(b<=a);
}

/*
unsigned long int min(unsigned long int const x, unsigned long int const y)
{
    return y ^ ((x ^ y) & -(x < y));
}*/
  

// min( float, float )

inline
float
min( float const a, float const b )
{
	//return ( a < b ? a : b );
    return a*(a<b) + b*(b<=a);
}
/*
float min(float const x, float const y)
{
   return y ^ ((x ^ y) & -(x < y));
}*/


// min( double, double )

inline
double
min( double const a, double const b )
{
	//return ( a < b ? a : b );
    return a*(a<b) + b*(b<=a);
}
/*
double min(double const x, double const y)
{
    return y ^ ((x ^ y) & -(x < y));
}
*/

// min( long double, long double )

 inline
long double
min( long double const a, long double const b )
{
	//return ( a < b ? a : b );
    return a*(a<b) + b*(b<=a);
}
 
/*
long double min(long double const x, long double const y)
{
    return y ^ ((x ^ y) & -(x < y));
}
*/


// Use std::min for 2 arguments not covered by the above overloads
using std::min;

// min( short, short, short )

inline
short int
min( short int const a, short int const b, short int const c )
{
	return ( min(a, min(b,c)) );
}
/*
short int min(short int const x, short int const y, short int const z)
{
    return min(x, min(y,z));
}*/

// min( short, short, short, short, ... )
template< typename... Ts >
inline
short int
min( short int const a, short int const b, short int const c, short int const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( int, int, int )
inline
int
min( int const a, int const b, int const c )
{
	return ( min(a, min(b,c)) );
}
/*
int min(int x, int y, int z)
{
    return min(x, min(y,z));
}*/

// min( int, int, int, int, ... )
template< typename... Ts >
inline
int
min( int const a, int const b, int const c, int const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( long, long, long )
inline
long int
min( long int const a, long int const b, long int const c )
{
	return ( min(a, min(b,c)) );
}
/*
long int min(long int const x, long int const y, long int const z)
{
    return min(x, min(y,z));
}*/

// min( long, long, long, long, ... )
template< typename... Ts >
inline
long int
min( long int const a, long int const b, long int const c, long int const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( unsigned short, unsigned short, unsigned short )
inline
unsigned short int
min( unsigned short int const a, unsigned short int const b, unsigned short int const c )
{
	return ( min(a, min(b,c)));
}

/*
unsigned short int min(unsigned short int const x, unsigned short int const y, unsigned short int const z)
{
    return min(x, min(y,z));
}*/

// min( unsigned short, unsigned short, unsigned short, unsigned short, ... )
template< typename... Ts >
inline
unsigned short int
min( unsigned short int const a, unsigned short int const b, unsigned short int const c, unsigned short int const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( unsigned, unsigned, unsigned )
inline
unsigned int
min( unsigned int const a, unsigned int const b, unsigned int const c )
{
	//return ( a < b ? ( a < c ? a : c ) : ( b < c ? b : c ) );
    return min(min(a,b),c);
}

/*
unsigned int min(unsigned int const x, unsigned int const y)
{
    return y ^ ((x ^ y) & -(x < y));
}
*/

// min( unsigned, unsigned, unsigned, unsigned, ... )
template< typename... Ts >
inline
unsigned int
min( unsigned int const a, unsigned int const b, unsigned int const c, unsigned int const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( unsigned long, unsigned long, unsigned long )
inline
unsigned long int
min( unsigned long int const a, unsigned long int const b, unsigned long int const c )
{
	return (min(a,min(b,c)));
}
/*
unsigned long int min(unsigned long int const x, unsigned long int const y, unsigned long int const z)
{
    return min(x,min(y,z));
}*/

// min( unsigned long, unsigned long, unsigned long, unsigned long, ... )
template< typename... Ts >
inline
unsigned long int
min( unsigned long int const a, unsigned long int const b, unsigned long int const c, unsigned long int const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( float, float, float )
inline
float
min( float const a, float const b, float const c )
{
	return ( min(a, min(b,c)) );
}
/*
float min(float const x, float const y, float const z)
{
    return min(x,min(y,z));
}*/

// min( float, float, float, float, ... )
template< typename... Ts >
inline
float
min( float const a, float const b, float const c, float const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( double, double, double )
inline
double
min( double const a, double const b, double const c )
{
	return ( min(a, min(b,c)) );
}
/*
double min(double const x, double const y, double const z)
{
    return min(x,min(y,z));
}*/

// min( double, double, double, double, ... )
template< typename... Ts >
inline
double
min( double const a, double const b, double const c, double const d, Ts const &... o )
{
	return min( a < b ? a : b, c < d ? c : d, o... );
}

// min( long double, long double, long double )
inline
long double
min( long double const a, long double const b, long double const c )
{
	return ( min(a, min(b,c)) );
}
/*
long double min(long double const x, long double const y, long double const z)
{
    return min(x, min(y,z));
}*/


// min( long double, long double, long double, long double, ... )
template< typename... Ts >
inline
long double
min( long double const a, long double const b, long double const c, long double const d, Ts const &... o )
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

// max( short, short )
inline
short int
max( short int const a, short int const b )
{
	//return ( a ^ ((a ^ b) & -(a < b)) );
    return b*(a<b) + a*(b<=a);
}
/*
short int max(short int const x, short int const y)
{
    return x ^ ((x ^ y) & -(x < y));
}*/

// max( int, int )
inline
int
max( int const a, int const b )
{
	//return ( a ^ ((a ^ b) & -(a < b)) );
    return b*(a<b) + a*(b<=a);
}
/*
 int max( int const x, int const y)
{
    return x ^ ((x ^ y) & -(x < y));
}*/

// max( long, long )
inline
long int
max( long int const a, long int const b )
{
	//return ( a ^ ((a ^ b) & -(a < b)) );
    return b*(a<b) + a*(b<=a);
}
/*
long int max(long int const x, long int const y)
{
    return x ^ ((x ^ y) & -(x < y));
}
*/

// max( unsigned short, unsigned short )

inline
unsigned short int
max( unsigned short int const a, unsigned short int const b )
{
    //return ( a ^ ((a ^ b) & -(a < b)) );
    return b*(a<b) + a*(b<=a);
}
/*
unsigned short int max(unsigned short int const x, unsigned short int const y)
{
    return x ^ ((x ^ y) & -(x < y));
}*/


// max( unsigned, unsigned )

inline
unsigned int
max( unsigned int const a, unsigned int const b )
{
	//return ( a^((a^b) & -(a<b)) );
    return b*(a<b) + a*(b<=a);
}

/*
unsigned int max(unsigned int x, unsigned int y)
{
    return x ^ ((x ^ y) & -(x < y));
}*/


// max( unsigned long, unsigned long )
inline
unsigned long int
max( unsigned long int const a, unsigned long int const b )
{
	//return (a^((a^b) & -(a<b)));
    return b*(a<b) + a*(b<=a);
}
/*
unsigned long int max(unsigned long int const x, unsigned long int const y)
{
    return x ^ ((x ^ y) & -(x < y));
}*/


// max( float, float )
 inline
float
max( float const a, float const b )
{
	//return ( a < b ? b : a );
    return b*(a<b) + a*(b<=a);
} 
/*
float max(float const x, float const y)
{
    return x ^ ((x ^ y) & -(x < y));
}
*/

// max( double, double )
inline
double
max( double const a, double const b )
{
	//return ( a < b ? b : a );
    return b*(a<b) + a*(b<=a);
}

/*
double max(double const x, double const y)
{
    return x ^ ((x ^ y) & -(x < y));
}*/



// max( long double, long double )
inline
long double
max( long double const a, long double const b )
{
	//return ( a < b ? b : a );
    return b*(a<b) + a*(b<=a);
} 
/*
long double max(long double const x, long double const y)
{
    return x ^ ((x ^ y) & -(x < y));
}
*/

// Use std::max for 2 arguments not covered by the above overloads
using std::max;

// max( short, short, short )
inline
short int
max( short int const a, short int const b, short int const c )
{
	return ( max(a, max(b,c)) );
}

/*
short int max(short int const x, short int const y, short int const z)
{
    return max(x,max(y,z));
}
*/

// max( short, short, short, short, ... )
template< typename... Ts >
inline
short int
max( short int const a, short int const b, short int const c, short int const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( int, int, int )
inline
int
max( int const a, int const b, int const c )
{
	return ( max(a, max(b,c)) );
}
/*
int max( int const x, int const y, int const z)
{
    return max(x,max(y,z));
}*/


// max( int, int, int, int, ... )
template< typename... Ts >
inline
int
max( int const a, int const b, int const c, int const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( long, long, long )
inline
long int
max( long int const a, long int const b, long int const c )
{
	return ( max(a, max(b,c)) );
}

/*
long int max(long int const x, long int const y, long int const z)
{
    return max(x,max(y,z));
}*/


// max( long, long, long, long, ... )
template< typename... Ts >
inline
long int
max( long int const a, long int const b, long int const c, long int const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( unsigned short, unsigned short, unsigned short )
inline
unsigned short int
max( unsigned short int const a, unsigned short int const b, unsigned short int const c )
{
	return ( max(a, max(b,c)) );
}
/*
unsigned short int max(unsigned short int const x, unsigned short int const y, unsigned short int const z)
{
    return max(x,max(y,z));
}*/

// max( unsigned short, unsigned short, unsigned short, unsigned short, ... )
template< typename... Ts >
inline
unsigned short int
max( unsigned short int const a, unsigned short int const b, unsigned short int const c, unsigned short int const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( unsigned, unsigned, unsigned )
 inline
unsigned int
max( unsigned int const a, unsigned int const b, unsigned int const c )
{
	return ( max(a, max(b,c)) );
}
/*
unsigned int max(unsigned int const x, unsigned int const y, unsigned int const z)
{
    return max(x,max(y,z));
}*/


// max( unsigned, unsigned, unsigned, unsigned, ... )
template< typename... Ts >
inline
unsigned int
max( unsigned int const a, unsigned int const b, unsigned int const c, unsigned int const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( unsigned long, unsigned long, unsigned long )
 inline
unsigned long int
max( unsigned long int const a, unsigned long int const b, unsigned long int const c )
{
	return ( max(a, max(b,c)) );
}
/*
unsigned long int max(unsigned long int const x, unsigned long int const y, unsigned long int const z)
{
    return max(x,max(y,z));
}
*/

// max( unsigned long, unsigned long, unsigned long, unsigned long, ... )
template< typename... Ts >
inline
unsigned long int
max( unsigned long int const a, unsigned long int const b, unsigned long int const c, unsigned long int const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( float, float, float )
  inline
float
max( float const a, float const b, float const c )
{
	//return ( a < b ? ( b < c ? c : b ) : ( a < c ? c : a ) );
    return max(max(a,b),c);
}
/*
float max(float const x, float const y, float const z)
{
    return max(x,max(y,z));
}*/

// max( float, float, float, float, ... )
template< typename... Ts >
inline
float
max( float const a, float const b, float const c, float const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( double, double, double )
 inline
double
max( double const a, double const b, double const c )
{
	//return ( a < b ? ( b < c ? c : b ) : ( a < c ? c : a ) );
    return max(max(a,b),c)
}

/*
double max(double const x, double const y, double const z)
{
    return max(x,max(y,z));
}*/


// max( double, double, double, double, ... )
template< typename... Ts >
inline
double
max( double const a, double const b, double const c, double const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( long double, long double, long double )

inline
long double
max( long double const a, long double const b, long double const c )
{
	//return ( a < b ? ( b < c ? c : b ) : ( a < c ? c : a ) );
    return max(max(a,b),c)
}


/*
long double max(long double const x, long double const y, long double const z)
{
    return max(x,max(y,z));
}*/


// max( long double, long double, long double, long double, ... )
template< typename... Ts >
inline
long double
max( long double const a, long double const b, long double const c, long double const d, Ts const &... o )
{
	return max( a < b ? b : a, c < d ? d : c, o... );
}

// max( a, b, c )
template< typename T >
inline
T const &
max( T const & a, T const & b, T const & c )
{
	//return ( a < b ? ( b < c ? c : b ) : ( a < c ? c : a ) );
    return max(max(a,b),c)
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

// FLOOR( x )
template< typename T >
inline
int
FLOOR( T const & x )
{
	return int( std::floor( x ) );
}

// CEILING( x )
template< typename T >
inline
int
CEILING( T const & x )
{
	return int( std::ceil( x ) );
}

// signum( x )
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
int
signum( T const & x )
{
	return ( x > T( 0 ) ? +1 : ( x < T( 0 ) ? -1 : 0 ) );
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

// nsint( x ): Nearest short int
template< typename T >
inline
short int
nsint( T const & x )
{
	return static_cast< short int >( x + ( sign( x ) * T( 0.5 ) ) );
}

// nlint( x ): Nearest long int
template< typename T >
inline
long int
nlint( T const & x )
{
	return static_cast< long int >( x + ( sign( x ) * T( 0.5 ) ) );
}

// nlint( x ): Nearest long int
template< typename T >
inline
int64_t
nint64( T const & x )
{
	return static_cast< int64_t >( x + ( sign( x ) * T( 0.5 ) ) );
}

// Nearest function selector class for R non-integer or T integer
template< typename R, typename T, bool >
struct NearestSelector
{
	static
	R
	nearest( T const & x )
	{
		return R( x );
	}
};

// Nearest function selector class for R integer and T non-integer
template< typename R, typename T >
struct NearestSelector< R, T, true >
{
	static
	R
	nearest( T const & x )
	{
		return R( x + ( sign( x ) * T( 0.5 ) ) );
	}
};

// nearest< R >( x ): Nearest R
template< typename R, typename T >
inline
R
nearest( T const & x )
{
	return NearestSelector< R, T, ( ( std::numeric_limits< R >::is_integer ) && ( ! std::numeric_limits< T >::is_integer ) ) >::nearest( x );
}

// nearest_size( x ): Nearest std::size_t
template< typename T >
inline
std::size_t
nearest_size( T const & x )
{
	return std::size_t( x > T( 0 ) ? x + ( sign( x ) * T( 0.5 ) ) : 0 );
}

// nearest_ssize( x ): Nearest SSize
template< typename T >
inline
SSize
nearest_ssize( T const & x )
{
	return SSize( x + ( sign( x ) * T( 0.5 ) ) );
}

// nearest_int( x ): Nearest int
template< typename T >
inline
int
nearest_int( T const & x )
{
	return static_cast< int >( x + ( sign( x ) * T( 0.5 ) ) );
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

// i(mod n) : float Arguments
inline
float
mod( float const i, float const n )
{
	return ( n != 0.0f ? std::fmod( i, n ) : 0.0f );
}

// i(mod n) : double Arguments
inline
double
mod( double const i, double const n )
{
	return ( n != 0.0 ? std::fmod( i, n ) : 0.0 );
}

// i(mod n) : long double Arguments
inline
long double
mod( long double const & i, long double const & n )
{
	return ( n != 0.0l ? std::fmod( i, n ) : 0.0l );
}

// Modulo function selector class for non-integer types
template< typename T, bool >
struct ModuloSelector
{
	static
	T
	modulo( T const & x, T const & y )
	{
		return ( y != T( 0 ) ? x - ( std::floor( x / y ) * y ) : x ); // Intel Fortran behavior if y==0
	}
};

// Modulo function selector class for integer types
template< typename T >
struct ModuloSelector< T, true >
{
	static
	T
	modulo( T const & x, T const & y )
	{
		return ( y != T( 0 ) ? x - ( T( std::floor( static_cast< long double >( x ) / y ) ) * y ) : x ); // Intel Fortran behavior if y==0
	}
};

// x(mod y) mathematical modulo returning magnitude < | y | and sign of y
template< typename T >
inline
T
modulo( T const & x, T const & y )
{
	return ModuloSelector< T, std::numeric_limits< T >::is_integer >::modulo( x, y );
}

// Positive Difference
template< typename T >
inline
T
dim( T const & x, T const & y )
{
	return max( x - y, T( 0 ) );
}

// Greatest Common Divisor
template< typename T >
inline
T
gcd( T const & m, T const & n )
{
	T lo( min( m, n ) );
	T hi( max( m, n ) );
	while ( lo > T( 0 ) ) {
		T const rem( mod( hi, lo ) );
		hi = lo;
		lo = rem;
	}
	return hi;
}

// Complex /////

// Real Part
template< typename T >
inline
T
REAL( std::complex< T > const & c )
{
	return c.real();
}

// Real Conversion
template< typename T >
inline
T
REAL( T const & x )
{
	return float( x );
}

// Complex Maker
template< typename R, typename I, class = typename std::enable_if< std::is_integral< R >::value && std::is_integral< I >::value >::type >
inline
std::complex< float >
make_complex( R const & r, I const & i )
{
	return std::complex< float >( r, i );
}

// Complex Maker
template< typename R, typename I, class = typename std::enable_if< std::is_integral< R >::value && ( ! std::is_integral< I >::value ) >::type >
inline
std::complex< I >
make_complex( R const & r, I const & i )
{
	return std::complex< I >( r, i );
}

// Complex Maker
template< typename R, typename I, class = typename std::enable_if< ( ! std::is_integral< R >::value ) && std::is_integral< I >::value >::type >
inline
std::complex< R >
make_complex( R const & r, I const & i )
{
	return std::complex< R >( r, i );
}

// Complex Maker
template< typename R, typename I, class = typename std::enable_if< ( ! std::is_integral< R >::value ) && ( ! std::is_integral< I >::value ) && std::is_same< R, I >::value >::type, typename = void >
inline
std::complex< R >
make_complex( R const & r, I const & i )
{
	return std::complex< R >( r, i );
}

// Complex Maker
template< typename R, typename I, class = typename std::enable_if< ( ! std::is_integral< R >::value ) && ( ! std::is_integral< I >::value ) && ( ! std::is_same< R, I >::value ) >::type >
inline
std::complex< double >
make_complex( R const & r, I const & i )
{
	return std::complex< double >( r, i );
}

// Trigonometric /////

// pi
template< typename T >
inline
T
pi()
{
	static T const Pi( T( 4 ) * std::atan( T( 1 ) ) );
	return Pi;
}

// pi/2
template< typename T >
inline
T
pi_over_2()
{
	static T const Pi__2( T( 2 ) * std::atan( T( 1 ) ) );
	return Pi__2;
}

// pi/2
template< typename T >
inline
T
pi__2()
{
	static T const Pi__2( T( 2 ) * std::atan( T( 1 ) ) );
	return Pi__2;
}

// Radians to Degrees
template< typename T >
inline
T
degrees( T const & r )
{
	static T const r2d( T( 180 ) / pi< T >() );
	return r * r2d;
}

// Radians to Degrees
template< typename T >
inline
T
deg( T const & r )
{
	static T const r2d( T( 180 ) / pi< T >() );
	return r * r2d;
}

// Degrees to Radians
template< typename T >
inline
T
radians( T const & d )
{
	static T const d2r( pi< T >() / T( 180 ) );
	return d * d2r;
}

// Degrees to Radians
template< typename T >
inline
T
rad( T const & d )
{
	static T const d2r( pi< T >() / T( 180 ) );
	return d * d2r;
}

// Cotangent
template< typename T >
inline
T
cot( T const & r )
{
	if ( r == T( 0 ) ) {
		return std::numeric_limits< T >::infinity();
	} else {
		return std::tan( pi_over_2< T >() - r );
	}
}

// Arccotangent
template< typename T >
inline
T
acot( T const & x )
{
	return pi_over_2< T >() - std::atan( x );
}

// Tangent of Angle in Degrees
template< typename T >
inline
T
tand( T const & d )
{
	return std::tan( radians( d ) );
}

// Cotangent of Angle in Degrees
template< typename T >
inline
T
cotd( T const & d )
{
	return cot( radians( d ) );
}

// Arcsine in Degrees
template< typename T >
inline
T
asind( T const & x )
{
	return degrees( std::asin( x ) );
}

// Arccosine in Degrees
template< typename T >
inline
T
acosd( T const & x )
{
	return degrees( std::acos( x ) );
}

// Arctangent in Degrees
template< typename T >
inline
T
atand( T const & x )
{
	return degrees( std::atan( x ) );
}

// Arccotangent in Degrees
template< typename T >
inline
T
acotd( T const & x )
{
	return degrees( acot( x ) );
}

// Two-Argument Arctangent in Degrees
template< typename T >
inline
T
atan2d( T const & y, T const & x )
{
	return degrees( std::atan2( y, x ) );
}

// Error Function /////

// Two-Argument Arctangent in Degrees
template< typename T >
inline
T
erfcx( T const & x )
{
	return T( std::exp( x * x ) * std::erfc( x ) );
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

// cube( x ) == x^3
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
cube( T const x )
{
	return x * x * x;
}

// quad( x ) == x^4
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
quad( T const x )
{
	T const t( x * x );
	return t * t;
}

// pow_2( x ) == x^2
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_2( T const x )
{
	return x * x;
}
/*
double pow_2(double x)
{
    return exp(2*log(x));
}
 */
// pow_3( x ) == x^3

 template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_3( T const x )
{
	//return x * x * x;
    return exp(3*log(x));
}
/*
double pow_3(double x)
{
    return exp(3*log(x));
    
}*/

//// pow_4( x ) == x^4

template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_4( T const x )
{
	//T const t( x * x );
	//return t * t;
    return exp(4*log(x));
}

/*
double pow_4 (double x)
{
    return exp(4*log(x));
}
*/

// pow_5( x ) == x^5

 template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_5( T const x )
{
	//T const t( x * x );
	//return t * t * x;
    return exp(5*log(x));
}


// pow_6( x ) == x^6
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_6( T const x )
{
	//T const t( x * x * x );
	//return t * t;
    return exp(6*log(x));
    
}
/*
double pow_6(float x)
{
    return exp(6*log(x));
    
}*/


// pow_7( x ) == x^7

template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_7( T const x )
{
	//T const t( x * x * x );
	//return t * t * x;
    return exp(7*log(x));
}
 
/*
 double pow_7(float x)
 {
     return exp(7*log(x));
 }
*/
// pow_8( x ) == x^8
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_8( T const x )
{
	//T t( x * x );
	//t *= t;
	//return t * t;
    return exp(8*log(x));
}

// pow_9( x ) == x^9
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
pow_9( T const x )
{
	//T const t( x * x * x );
	//return t * t * t;
    return exp(9*log(x));
}



// root_4( x ) == x^(1/4)
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
root_4( T const x )
{
	return T( std::sqrt( std::sqrt( x ) ) );
}

// root_8( x ) == x^(1/8)
template< typename T, class = typename std::enable_if< std::is_arithmetic< T >::value >::type >
inline
T
root_8( T const x )
{
	return T( std::sqrt( std::sqrt( std::sqrt( x ) ) ) );
}

// square( x ) == x^2
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
square( T const & x )
{
	return x * x;
}

// cube( x ) == x^3
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
cube( T const & x )
{
	return x * x * x;
}

// quad( x ) == x^4
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
quad( T const & x )
{
	T const t( x * x );
	return t * t;
}

// pow_2( x ) == x^2
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
pow_2( T const & x )
{
    return  exp(2*log(x));
}
/*
double pow_2 (double x)
{

    return exp(2*log(x));
}*/

// pow_3( x ) == x^3
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
pow_3( T const & x )
{
	return exp(3*log(x));
}
/*
double pow_3 (double x)
{

    return exp(3*log(x));
}*/

// pow_4( x ) == x^4

template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
pow_4( T const & x )
{
	//T const t( x * x );
	return pow_2(pow_2(x));
}
 /*
double pow_4 (double x)
{

    return exp(4*log(x));
}*/

// pow_5( x ) == x^5
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
pow_5( T const & x )
{
	//T const t( x * x );
	//return t * t * x;
    return pow_2(x)*pow_3(x);
}
/*
double pow_5(double x)
{
    return exp(5*log(x));
    
}*/

// pow_6( x ) == x^6
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
pow_6( T const & x )
{
	//T const t( x * x * x );
	//return t * t;
    return pow_2(pow_3(x));
}
 /*
double pow_6(double x)
{
     return exp(6*log(x));
    
}*/

// pow_7( x ) == x^7

template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
pow_7( T const & x )
{
	//T const t( x * x * x );
	//return t * t * x;
    return pow_3(x)*pow_4(x);
}
/*
double pow_7(double x)
{
    return exp(7*log(x));
    
}*/

// pow_8( x ) == x^8
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
pow_8( T const & x )
{
	//T t( x * x );
	//t *= t;
	//return t * t;
    return pow_4(x)*pow_4(x);
}
 /*
double pow_8(double x)
{
    return exp(8*log(x));
    
}*/

// pow_9( x ) == x^9
template< typename T, class = typename std::enable_if< ! std::is_arithmetic< T >::value >::type >
inline
T
pow_9( T const & x )
{
	//T const t( x * x * x );
	//return t * t * t;
    return pow_4(x)*pow_5(x);
}
 /*

double pow_9(double x)
{
     return exp(9*log(x));
    
}*/

// Tolerant Comparisons /////

// x == y Within Specified Relative or Absolute Tolerances?
template< typename T >
inline
bool
eq_tol( T const & x, T const & y, T const & r_tol, T const & a_tol = T( 0 ) )
{
	using std::abs; // Can use std::abs or user-defined abs
	assert( r_tol >= T( 0 ) );
	assert( a_tol >= T( 0 ) );
	return ( abs( x - y ) <= max( r_tol * max( abs( x ), abs( y ) ), a_tol ) );
}

// x < y Within Specified Relative or Absolute Tolerances?
template< typename T >
inline
bool
lt_tol( T const & x, T const & y, T const & r_tol, T const & a_tol = T( 0 ) )
{
	using std::abs; // Can use std::abs or user-defined abs
	assert( r_tol >= T( 0 ) );
	assert( a_tol >= T( 0 ) );
	return ( x < y + max( r_tol * max( abs( x ), abs( y ) ), a_tol ) );
}

// x <= y Within Specified Relative or Absolute Tolerances?
template< typename T >
inline
bool
le_tol( T const & x, T const & y, T const & r_tol, T const & a_tol = T( 0 ) )
{
	using std::abs; // Can use std::abs or user-defined abs
	assert( r_tol >= T( 0 ) );
	assert( a_tol >= T( 0 ) );
	return ( x <= y + max( r_tol * max( abs( x ), abs( y ) ), a_tol ) );
}

// x >= y Within Specified Relative or Absolute Tolerances?
template< typename T >
inline
bool
ge_tol( T const & x, T const & y, T const & r_tol, T const & a_tol = T( 0 ) )
{
	using std::abs; // Can use std::abs or user-defined abs
	assert( r_tol >= T( 0 ) );
	assert( a_tol >= T( 0 ) );
	return ( x >= y - max( r_tol * max( abs( x ), abs( y ) ), a_tol ) );
}

// x > y Within Specified Relative or Absolute Tolerances?
template< typename T >
inline
bool
gt_tol( T const & x, T const & y, T const & r_tol, T const & a_tol = T( 0 ) )
{
	using std::abs; // Can use std::abs or user-defined abs
	assert( r_tol >= T( 0 ) );
	assert( a_tol >= T( 0 ) );
	return ( x > y - max( r_tol * max( abs( x ), abs( y ) ), a_tol ) );
}

} // ObjexxFCL

#endif // ObjexxFCL_Fmath_hh_INCLUDED

