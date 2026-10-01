// ObjexxFCL::CArray Unit Tests
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

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/CArray.hh>
#include "ObjexxFCL.unit.hh"

using namespace ObjexxFCL;

TEST( CArrayTest, Construction )
{
	{ // Copy constructor and assignment
		CArray_int v( 10u, 22 );
		CArray_int w( v );
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
		CArray_int v( 10u, 22 );
		CArray<float> f( v );
		EXPECT_EQ( CArray<float>( 10u, 22.0f ), f );
		v = 23;
		EXPECT_EQ( CArray_int( 10u, 23 ), v );
		f = v;
		EXPECT_EQ( CArray<float>( 10u, 23.0f ), f );
	}

	{ // Size constructor
		CArray_int v( 10 ); // Uninitialized
		EXPECT_EQ( 10u, v.size() );
	}

	{ // Size + value constructor
		CArray_int v( 10u, 22 );
		EXPECT_EQ( 10u, v.size() );
		EXPECT_EQ( 22, v[ 0u ] );
		EXPECT_EQ( 22, v[ 9u ] );
	}
}

TEST( CArrayTest, Assignment )
{
	CArray_int v( 10u, 22 );
	v = CArray_int( 20u, 33 );
	EXPECT_EQ( CArray_int( 20u, 33 ), v );
	v = 55;
	EXPECT_EQ( CArray_int( 20u, 55 ), v );
}

TEST( CArrayTest, Subscripting )
{
	CArray_int v( 10u, 22 );
	v[ 3u ] = 33;
	EXPECT_EQ( 22, v[ 0u ] );
	EXPECT_EQ( 33, v[ 3u ] );
	EXPECT_EQ( 22, v[ 9u ] );
	EXPECT_EQ( 33, v( 4 ) );
	v( 5 ) = 44;
	EXPECT_EQ( 44, v( 5 ) );
}

TEST( CArrayTest, Functions )
{
	CArray_int u{ 1, 2, 3 };
	CArray_int v{ 2, 3, 4 };
	EXPECT_EQ( 14, magnitude_squared( u ) );
	EXPECT_EQ( 3, distance_squared( u, v ) );
	EXPECT_EQ( 20, dot( u, v ) );
}

TEST( CArrayTest, Swap )
{
	CArray_int a( 10u, 22 ), A( a );
	CArray_int b( 8u, 33 ), B( b );
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
