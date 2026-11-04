// ObjexxFCL::Array Unit Tests
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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244) // Suppress conversion warnings: Intentional narrowing assignments present
#endif
#include <ObjexxFCL/Array.all.hh>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include "ObjexxFCL.unit.hh"

// C++ Headers
#include <array>

using namespace ObjexxFCL;
typedef  IndexRange  IR;

TEST( ArrayTest, DefaultConstruction )
{
	Array2D_int A, B;
	EXPECT_EQ( 0u, A.size() );
}

TEST( ArrayTest, Construction2DIndexRangeInitializerList )
{
	Array2D_int r( IR( -1, 1 ), IR( -1, 1 ), { 1, 2, 3, 4, 5, 6, 7, 8, 9 } );
	EXPECT_EQ( -1, r.l1() );
	EXPECT_EQ( 1, r.u1() );
	EXPECT_EQ( -1, r.l2() );
	EXPECT_EQ( 1, r.u2() );
	for ( int i = -1, k = 1; i <= 1; ++i ) {
		for ( int j = -1; j <= 1; ++j, ++k ) {
			EXPECT_EQ( k, r( i, j ) );
		}
	}
	int v( 0 );
	for ( auto const e : r ) {
		EXPECT_EQ( ++v, e );
	}
	v = 10;
	for ( auto & e : r ) {
		e = ++v;
		EXPECT_EQ( v, e );
	}
}

TEST( ArrayTest, Swap3D )
{
	Array3D_int A( 4, 4, 4, 44 );
	Array3D_int( 5, 5, 5, 55 ).swap( A );
	EXPECT_EQ( IR( 1, 5 ), A.I1() );
	EXPECT_EQ( IR( 1, 5 ), A.I2() );
	EXPECT_EQ( IR( 1, 5 ), A.I3() );
	EXPECT_EQ( 5u, A.size1() );
	EXPECT_EQ( 5u, A.size2() );
	EXPECT_EQ( 5u, A.size3() );
	EXPECT_EQ( 5u * 5u * 5u, A.size() );
	for ( std::size_t i = 0; i < A.size(); ++i ) {
		EXPECT_EQ( 55, A[ i ] );
	}
}

TEST( ArrayTest, LogicalNegation )
{
	Array2D_bool const F( 2, 2, false );
	EXPECT_FALSE( F( 1, 1 ) );
	EXPECT_FALSE( F( 1, 2 ) );
	EXPECT_FALSE( F( 2, 1 ) );
	EXPECT_FALSE( F( 2, 2 ) );
}

TEST( ArrayTest, EmptyComparisonElemental )
{
	Array1D_int a( 0 ), b( 0 ); // Empty
	EXPECT_EQ( 0u, a.size() );
	EXPECT_EQ( 0u, b.size() );
}

TEST( ArrayTest, Unallocated )
{
	Array1D_int a; // Empty
	EXPECT_FALSE( a.allocated() );
	EXPECT_FALSE( allocated( a ) );
	EXPECT_DEBUG_DEATH( a( 1 ), ".*Assertion.*" );
}

