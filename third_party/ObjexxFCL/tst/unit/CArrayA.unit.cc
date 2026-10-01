// ObjexxFCL::CArrayA Unit Tests
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
#include <ObjexxFCL/CArrayA.hh>
#include "ObjexxFCL.unit.hh"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace ObjexxFCL;

TEST( CArrayATest, Construction )
{
	{ // Copy constructor and assignment
		CArrayA<int> v( 10u, 22 );
		CArrayA<int> w( v );
		EXPECT_EQ( v, w );
		EXPECT_EQ( w, v );
		v = w;
		EXPECT_EQ( v, w );
		EXPECT_EQ( w, v );
		EXPECT_TRUE( v == w );
		EXPECT_TRUE( v <= w );
		EXPECT_TRUE( v >= w );
		EXPECT_FALSE( v < w );
		EXPECT_FALSE( v > w );
	}

	{ // Copy constructor and assignment template
		CArrayA<int> v( 10u, 22 );
		CArrayA<float> f( v ); // May cause conversion warning
		EXPECT_EQ( CArrayA<float>( 10u, 22.0f ), f );
		v = 23;
		EXPECT_EQ( CArrayA<int>( 10u, 23 ), v );
		f = v;
		EXPECT_EQ( CArrayA<float>( 10u, 23.0f ), f );
	}

	{ // Size constructor
		CArrayA<int> v( 10 ); // Uninitialized
		EXPECT_EQ( 10u, v.size() );
	}

	{ // Size + value constructor
		CArrayA<int> v( 10u, 22 );
		EXPECT_EQ( 10u, v.size() );
		EXPECT_EQ( 22, v[ 0u ] );
		EXPECT_EQ( 22, v[ 9u ] );
	}
}

TEST( CArrayATest, Assignment )
{
	CArrayA<int> v( 10u, 22 );
	v = CArrayA<int>( 20u, 33 );
	EXPECT_EQ( CArrayA<int>( 20u, 33 ), v );
	v = 55;
	EXPECT_EQ( CArrayA<int>( 20u, 55 ), v );
}

TEST( CArrayATest, Subscripting )
{
	CArrayA<int> v( 10u, 22 );
	v[ 3u ] = 33;
	EXPECT_EQ( 22, v[ 0u ] );
	EXPECT_EQ( 33, v[ 3u ] );
	EXPECT_EQ( 22, v[ 9u ] );
	EXPECT_EQ( 33, v( 4 ) );
	v( 5 ) = 44;
	EXPECT_EQ( 44, v( 5 ) );
}

TEST( CArrayATest, Functions )
{
	CArrayA<int> u{ 1, 2, 3 };
	CArrayA<int> v{ 2, 3, 4 };
	EXPECT_EQ( 14, magnitude_squared( u ) );
	EXPECT_EQ( 3, distance_squared( u, v ) );
	EXPECT_EQ( 20, dot( u, v ) );
}

TEST( CArrayATest, Swap )
{
	CArrayA<int> a( 10u, 22 ), A( a );
	CArrayA<int> b( 8u, 33 ), B( b );
	a.swap( b );
	EXPECT_EQ( B, a );
	EXPECT_EQ( A, b );
	b.swap( a );
	EXPECT_EQ( A, a );
	EXPECT_EQ( B, b );
	swap( a, b );
	EXPECT_EQ( B, a );
	EXPECT_EQ( A, b );
}
