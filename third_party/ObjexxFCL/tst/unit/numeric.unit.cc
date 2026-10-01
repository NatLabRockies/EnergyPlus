// ObjexxFCL::numeric Unit Tests
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
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/numeric.hh>
#include <ObjexxFCL/Array1D.hh>
#include "ObjexxFCL.unit.hh"

using namespace ObjexxFCL;

#define ARRAY_LENGTH(a) ( sizeof(a) / sizeof(a[0]) )

// Spaces in type names don't play well with some of the tests.
typedef  signed char         schar;
typedef  unsigned char       uchar;
typedef  signed short        sshort;
typedef  unsigned short      ushort;
typedef  signed int          sint;
typedef  unsigned int        uint;
typedef  signed long         slong;
typedef  unsigned long       ulong;
typedef  long long           longlong;
typedef  signed long long    slonglong;
typedef  unsigned long long  ulonglong;
typedef  long double         longdouble;

// Stash numeric limits to improve readability
//const bool bool_min = std::numeric_limits< bool >::min();
bool const bool_max( std::numeric_limits< bool >::max() );

//char const char_min( std::numeric_limits< char >::min() );
char const char_max( std::numeric_limits< char >::max() );
//schar const schar_min( std::numeric_limits< schar >::min() );
schar const schar_max( std::numeric_limits< schar >::max() );
//uchar const uchar_min( std::numeric_limits< uchar >::min() );
uchar const uchar_max( std::numeric_limits< uchar >::max() );

//short const short_min( std::numeric_limits< short >::min() );
short const short_max( std::numeric_limits< short >::max() );
//sshort const sshort_min( std::numeric_limits< sshort >::min() );
sshort const sshort_max( std::numeric_limits< sshort >::max() );
//ushort const ushort_min( std::numeric_limits< ushort >::min() );
ushort const ushort_max( std::numeric_limits< ushort >::max() );

//int const int_min( std::numeric_limits< int >::min() );
int const int_max( std::numeric_limits< int >::max() );
//sint const sint_min( std::numeric_limits< sint >::min() );
sint const sint_max( std::numeric_limits< sint >::max() );
//uint const uint_min( std::numeric_limits< uint >::min() );
uint const uint_max( std::numeric_limits< uint >::max() );

//long const long_min( std::numeric_limits< long >::min() );
long const long_max( std::numeric_limits< long >::max() );
//slong const slong_min( std::numeric_limits< slong >::min() );
slong const slong_max( std::numeric_limits< slong >::max() );
//ulong const ulong_min( std::numeric_limits< ulong >::min() );
ulong const ulong_max( std::numeric_limits< ulong >::max() );

//longlong const longlong_min( std::numeric_limits< longlong >::min() );
longlong const longlong_max( std::numeric_limits< longlong >::max() );
//slonglong const slonglong_min( std::numeric_limits< slonglong >::min() );
slonglong const slonglong_max( std::numeric_limits< slonglong >::max() );
//ulonglong const ulonglong_min( std::numeric_limits< ulonglong >::min() );
ulonglong const ulonglong_max( std::numeric_limits< ulonglong >::max() );

float const float_min( std::numeric_limits< float >::min() );
float const float_max( std::numeric_limits< float >::max() );
double const double_min( std::numeric_limits< double >::min() );
double const double_max( std::numeric_limits< double >::max() );
longdouble const longdouble_min( std::numeric_limits< longdouble >::min() );
longdouble const longdouble_max( std::numeric_limits< longdouble >::max() );

TEST( NumericTest, RADIX )
{
	EXPECT_EQ( std::numeric_limits< bool >::radix, RADIX( bool() ) );
	EXPECT_EQ( std::numeric_limits< char >::radix, RADIX( char() ) );
	EXPECT_EQ( std::numeric_limits< schar >::radix, RADIX( schar() ) );
	EXPECT_EQ( std::numeric_limits< uchar >::radix, RADIX( uchar() ) );
	EXPECT_EQ( std::numeric_limits< wchar_t >::radix, RADIX( wchar_t() ) );
	EXPECT_EQ( std::numeric_limits< char16_t >::radix, RADIX( char16_t() ) );
	EXPECT_EQ( std::numeric_limits< char32_t >::radix, RADIX( char32_t() ) );
	EXPECT_EQ( std::numeric_limits< short >::radix, RADIX( short() ) );
	EXPECT_EQ( std::numeric_limits< sshort >::radix, RADIX( sshort() ) );
	EXPECT_EQ( std::numeric_limits< ushort >::radix, RADIX( ushort() ) );
	EXPECT_EQ( std::numeric_limits< int >::radix, RADIX( int() ) );
	EXPECT_EQ( std::numeric_limits< sint >::radix, RADIX( sint() ) );
	EXPECT_EQ( std::numeric_limits< uint >::radix, RADIX( uint() ) );
	EXPECT_EQ( std::numeric_limits< long >::radix, RADIX( long() ) );
	EXPECT_EQ( std::numeric_limits< slong >::radix, RADIX( slong() ) );
	EXPECT_EQ( std::numeric_limits< ulong >::radix, RADIX( ulong() ) );
	EXPECT_EQ( std::numeric_limits< longlong >::radix, RADIX( longlong() ) );
	EXPECT_EQ( std::numeric_limits< slonglong >::radix, RADIX( slonglong() ) );
	EXPECT_EQ( std::numeric_limits< ulonglong >::radix, RADIX( ulonglong() ) );
	EXPECT_EQ( std::numeric_limits< float >::radix, RADIX( float() ) );
	EXPECT_EQ( std::numeric_limits< double >::radix, RADIX( double() ) );
	EXPECT_EQ( std::numeric_limits< longdouble >::radix, RADIX( longdouble() ) );
}

TEST( NumericTest, Huge )
{
	EXPECT_EQ( bool_max, HUGE_( bool() ) );
	EXPECT_EQ( char_max, HUGE_( char() ) );
	EXPECT_EQ( schar_max, HUGE_( schar() ) );
	EXPECT_EQ( uchar_max, HUGE_( uchar() ) );
	EXPECT_EQ( std::numeric_limits< wchar_t >::max(), HUGE_( wchar_t() ) );
	EXPECT_EQ( std::numeric_limits< char16_t >::max(), HUGE_( char16_t() ) );
	EXPECT_EQ( std::numeric_limits< char32_t >::max(), HUGE_( char32_t() ) );
	EXPECT_EQ( short_max, HUGE_( short() ) );
	EXPECT_EQ( sshort_max, HUGE_( sshort() ) );
	EXPECT_EQ( ushort_max, HUGE_( ushort() ) );
	EXPECT_EQ( int_max, HUGE_( int() ) );
	EXPECT_EQ( sint_max, HUGE_( sint() ) );
	EXPECT_EQ( uint_max, HUGE_( uint() ) );
	EXPECT_EQ( long_max, HUGE_( long() ) );
	EXPECT_EQ( slong_max, HUGE_( slong() ) );
	EXPECT_EQ( ulong_max, HUGE_( ulong() ) );
	EXPECT_EQ( longlong_max, HUGE_( longlong() ) );
	EXPECT_EQ( slonglong_max, HUGE_( slonglong() ) );
	EXPECT_EQ( ulonglong_max, HUGE_( ulonglong() ) );
	EXPECT_EQ( float_max, HUGE_( float() ) );
	EXPECT_EQ( double_max, HUGE_( double() ) );
	EXPECT_EQ( longdouble_max, HUGE_( longdouble() ) );
}

TEST( NumericTest, TINY )
{
	EXPECT_EQ( float_min, TINY( float() ) );
	EXPECT_EQ( double_min, TINY( double() ) );
	EXPECT_EQ( longdouble_min, TINY( longdouble() ) );
}

TEST( NumericTest, EXPONENT )
{
	EXPECT_EQ( 0, EXPONENT( float() ) );
	EXPECT_EQ( -125, EXPONENT( float_min ) );
	EXPECT_EQ( 129, EXPONENT( float_max ) );
	EXPECT_EQ( 0, EXPONENT( double() ) );
	EXPECT_EQ( -1021, EXPONENT( double_min ) );
	EXPECT_EQ( 1025, EXPONENT( double_max ) );
	EXPECT_EQ( 0, EXPONENT( longdouble() ) );
#ifndef __INTEL_COMPILER // Avoid exception
#ifdef _MSC_VER // long double == double
	EXPECT_EQ( -1021, EXPONENT( longdouble_min ) );
	EXPECT_EQ( 1025, EXPONENT( longdouble_max ) );
#else
	EXPECT_EQ( -16381, EXPONENT( longdouble_min ) );
	EXPECT_EQ( 16385, EXPONENT( longdouble_max ) );
#endif
#endif
	EXPECT_EQ( 0, EXPONENT( float() ) );
	EXPECT_EQ( 0, EXPONENT( double() ) );
	EXPECT_EQ( 0, EXPONENT( longdouble() ) );
}

TEST( NumericTest, FRACTION )
{
	EXPECT_EQ( 0, FRACTION( float() ) );
	EXPECT_FLOAT_EQ( 0.5, FRACTION( float_min ) );
	EXPECT_FLOAT_EQ( 0.5, FRACTION( float_max ) );
	EXPECT_EQ( 0, FRACTION( double() ) );
	EXPECT_DOUBLE_EQ( 0.5, FRACTION( double_min ) );
	EXPECT_DOUBLE_EQ( 0.5, FRACTION( double_max ) );
	EXPECT_EQ( 0, FRACTION( longdouble() ) );
#ifndef __INTEL_COMPILER
	EXPECT_DOUBLE_EQ( 0.5, FRACTION( longdouble_min ) );
	EXPECT_DOUBLE_EQ( 0.5, FRACTION( longdouble_max ) );
#endif
	EXPECT_EQ( 0, FRACTION( float() ) );
	EXPECT_EQ( 0, FRACTION( double() ) );
	EXPECT_EQ( 0, FRACTION( longdouble() ) );
}

TEST( NumericTest, NEAREST )
{
	EXPECT_DOUBLE_EQ( 3.0 + std::pow( 2.0, -22 ), NEAREST( 3.0f, 2.0f ) );
	EXPECT_DOUBLE_EQ( 3.0 + std::pow( 2.0, -22 ), NEAREST( 3.0f, 2.0 ) );
	EXPECT_DOUBLE_EQ( 3.0 - std::pow( 2.0, -22 ), NEAREST( 3.0f, -2.0f ) );
	EXPECT_DOUBLE_EQ( 3.0 - std::pow( 2.0, -22 ), NEAREST( 3.0f, -2.0 ) );
	double const eps( std::pow( 2.0, -52 ) );
	EXPECT_DOUBLE_EQ( 1.0 + eps, NEAREST( 1.0, 1.0 ) );
	EXPECT_DOUBLE_EQ( 1.0 - eps, NEAREST( 1.0, -1.0 )  );
}
