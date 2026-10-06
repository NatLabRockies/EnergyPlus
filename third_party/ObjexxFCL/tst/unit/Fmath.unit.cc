// ObjexxFCL::Fmath Unit Tests
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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244) // Suppress conversion warnings: Intentional narrowing assignments present
#endif

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>
#include "ObjexxFCL.unit.hh"

// C++ Headers
#include <cstddef> // size_t
#include <cstdlib> // abs

#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace ObjexxFCL;

typedef unsigned short ushort;

struct S
{
	S( int const x_ = 11 ) :
		x( x_ )
	{}

	friend
	S
	operator *( S const & s1, S const & s2 )
	{
		return S( s1.x * s2.x );
	}

	int x;
};

TEST( FmathTest, Min )
{
	// Note: cast to invoke short overload, as there are no short literals
	EXPECT_EQ( short( 4 ), min( short( 4 ), short( 9 ) ) );

	EXPECT_EQ( 3u, min( 9u, 3u ) );

	EXPECT_EQ( 3ul, min( 4ul, 9ul, 3ul, 11ul ) );

	EXPECT_EQ( 4.0f, min( 4.0f, 9.0f ) );
	EXPECT_EQ( -9.0f, min( 3.0f, 5.0f, 1.0f, -9.0f ) );

	EXPECT_EQ( 4.0, min( 4.0, 9.0 ) );
	EXPECT_EQ( 1.0, min( 5.0, 1.0, 9.0 ) );
	EXPECT_EQ( -9.0, min( 3.0, 5.0, 1.0, -9.0 ) );

	EXPECT_EQ( 4.0l, min( 4.0l, 9.0l ) );
}

TEST( FmathTest, Max )
{
	// Note: cast to invoke short overload, as there are no short literals
	EXPECT_EQ( short( 9 ), max( short( 3 ), short( 5 ), short( 1 ), short( 9 ) ) );
	EXPECT_EQ( short( 5 ), max( short( 3 ), short( 5 ), short( 1 ), short( -9 ) ) );
	EXPECT_EQ( ushort( 9 ), max( ushort( 9 ), ushort( 3 ) ) );
	EXPECT_EQ( ushort( 11 ), max( ushort( 4 ), ushort( 9 ), ushort( 3 ), ushort( 11 ) ) );

	EXPECT_EQ( 9, max( 4, 9 ) );
	EXPECT_EQ( 9, max( 5, 1, 9 ) );
	EXPECT_EQ( 5, max( 3, 5, 1, -9 ) );
	EXPECT_EQ( 9u, max( 9u, 3u ) );

	EXPECT_EQ( 9l, max( 4l, 9l ) );
	EXPECT_EQ( 9ul, max( 9ul, 3ul ) );

	EXPECT_EQ( 9.0f, max( 4.0f, 9.0f ) );
	EXPECT_EQ( 5.0f, max( 3.0f, 5.0f, 1.0f, -9.0f ) );

	EXPECT_EQ( 9.0, max( 4.0, 9.0 ) );
	EXPECT_EQ( 5.0, max( 3.0, 5.0, 1.0, -9.0 ) );
}

TEST( FmathTest, Abs )
{
	using std::abs;
	EXPECT_EQ( 44, abs( -44 ) );
	EXPECT_EQ( 44l, abs( -44l ) );
	EXPECT_EQ( 44.4f, abs( -44.4f ) );
	EXPECT_EQ( 44.4, abs( -44.4 ) );
	EXPECT_EQ( 44l, ObjexxFCL::abs( -44l ) );
	EXPECT_EQ( 44.4f, ObjexxFCL::abs( -44.4f ) );
	EXPECT_EQ( 44.4, ObjexxFCL::abs( -44.4 ) );
	EXPECT_EQ( 44.4l, ObjexxFCL::abs( -44.4l ) );

	EXPECT_EQ( 44, std::abs( -44 ) );
	EXPECT_EQ( 44l, std::abs( -44l ) );
	EXPECT_EQ( 44.4f, std::abs( -44.4f ) );
	EXPECT_EQ( 44.4, std::abs( -44.4 ) );
	EXPECT_EQ( 44.4l, std::abs( -44.4l ) );
}

TEST( FmathTest, Ceiling )
{
	EXPECT_EQ( 42, CEILING( 42.0f ) );
	EXPECT_EQ( 92, CEILING( 91.1f ) );
	EXPECT_EQ( -2, CEILING( -2.3f ) );

	EXPECT_EQ( 42, CEILING( 42.0 ) );
	EXPECT_EQ( -2, CEILING( -2.3 ) );

	EXPECT_EQ( 42, CEILING( 42.0l ) );
	EXPECT_EQ( 92, CEILING( 91.1l ) );
	EXPECT_EQ( -2, CEILING( -2.3l ) );
}

TEST( FmathTest, Sign )
{
	EXPECT_EQ( 1, sign( 11 ) );
	EXPECT_EQ( 1, sign( 0 ) );
	EXPECT_EQ( -1, sign( -11 ) );
	EXPECT_EQ( 53.3, sign( 53.3, 11 ) );
	EXPECT_EQ( 53.3, sign( 53.3, 0 ) );
	EXPECT_EQ( -53.3, sign( 53.3, -11 ) );
	EXPECT_EQ( 53.3, sign( -53.3, 11 ) );
	EXPECT_EQ( 53.3, sign( -53.3, 0 ) );
	EXPECT_EQ( -53.3, sign( -53.3, -11 ) );
}

TEST( FmathTest, Nint )
{
	EXPECT_EQ( 3, nint( 3.123 ) );
	EXPECT_EQ( 3, nint( 3.4999 ) );
	EXPECT_EQ( 4, nint( 3.5 ) );

	EXPECT_EQ( 3, nint64( 3.123 ) );
	EXPECT_EQ( 3, nint64( 3.4999 ) );
	EXPECT_EQ( 4, nint64( 3.5 ) );
}

TEST( FmathTest, Mod )
{
	EXPECT_EQ( short( 3 ), mod( short( 33 ), short( 10 ) ) );
	EXPECT_EQ( ushort( 3 ), mod( ushort( 33 ), ushort( 10 ) ) );
	EXPECT_EQ( 3, mod( 33, 10 ) );
	EXPECT_EQ( 3u, mod( 33u, 10u ) );
	EXPECT_EQ( 3l, mod( 33l, 10l ) );
	EXPECT_EQ( 3ul, mod( 33ul, 10ul ) );
	EXPECT_EQ( size_t( 3u ), mod( size_t( 33u ), size_t( 10u ) ) );
	EXPECT_EQ( 3.0f, mod( 33.0f, 10.0f ) );
	EXPECT_EQ( 10.0f, mod( 32.0f, 11.0f ) );
	EXPECT_EQ( 3.0, mod( 33.0, 10.0 ) );
	EXPECT_EQ( 10.0, mod( 32.0l, 11.0l ) );
	EXPECT_EQ( 3.0l, mod( 33.0l, 10.0l ) );
	EXPECT_EQ( 10.0l, mod( 32.0l, 11.0l ) );
}

TEST( FmathTest, PiOver2  )
{
}

TEST( FmathTest, Square )
{
	EXPECT_EQ( short( 11 ) * short( 11 ), square( short( -11 ) ) );
	EXPECT_EQ( ushort( 11 ) * ushort( 11 ), square( ushort( 11 ) ) );
	EXPECT_EQ( 11 * 11, square( -11 ) );
	EXPECT_EQ( 11u * 11u, square( 11u ) );
	EXPECT_EQ( 11l * 11l, square( -11l ) );
	EXPECT_EQ( 11ul * 11ul, square( 11ul ) );
	EXPECT_EQ( 11.0f * 11.0f, square( -11.0f ) );
	EXPECT_EQ( 11.0 * 11.0, square( -11.0 ) );
	EXPECT_EQ( 11.0l * 11.0l, square( -11.0l ) );

}

TEST( FmathTest, Pow )
{
	EXPECT_EQ( 11*11, pow_2( -11 ) );
	EXPECT_EQ( -11*11*11, pow_3( -11 ) );
	EXPECT_EQ( 11u*11u*11u, pow_3( 11u ) );
	EXPECT_EQ( -11l*11l*11l, pow_3( -11l ) );
	EXPECT_EQ( 11ul*11ul*11ul, pow_3( 11ul ) );
	EXPECT_EQ( -11.0f * 11.0f * 11.0f, pow_3( -11.0f ) );
	EXPECT_EQ( -11.0f * 11.0 * 11.0, pow_3( -11.0 ) );
	EXPECT_EQ( -11.0l * 11.0l * 11.0l, pow_3( -11.0l ) );
	EXPECT_EQ( 16, pow_4( 2.0 ) );
	EXPECT_EQ( 32, pow_5( 2.0 ) );
	EXPECT_EQ( 64, pow_6( 2.0 ) );
	EXPECT_EQ( 128, pow_7( 2.0 ) );
}

TEST( FmathTest, Root )
{
	EXPECT_EQ( 3, root_4( 81 ) );
}

