// ObjexxFCL::Array3 Unit Tests
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
#include <ObjexxFCL/Array3.hh>
#include <ObjexxFCL/Array.functions.hh>
#include "ObjexxFCL.unit.hh"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace ObjexxFCL;

TEST( Array3Test, ConstructDefault )
{
	Array3D_int A;
	EXPECT_EQ( 0u, A.size() );
	EXPECT_EQ( 0u, A.size1() );
	EXPECT_EQ( 0u, A.size2() );
	EXPECT_EQ( 0u, A.size3() );
	EXPECT_EQ( 1, A.l1() );
	EXPECT_EQ( 1, A.l2() );
	EXPECT_EQ( 1, A.l3() );
	EXPECT_EQ( 0, A.u1() );
	EXPECT_EQ( 0, A.u2() );
	EXPECT_EQ( 0, A.u3() );
	EXPECT_EQ( Array3D_int::IR(), A.I1() );
	EXPECT_EQ( Array3D_int::IR(), A.I2() );
	EXPECT_EQ( Array3D_int::IR(), A.I3() );
}

TEST( Array3Test, ConstructCopy )
{
	Array3D_int A;
	Array3D_int B( A );
	EXPECT_EQ( A.size(), B.size() );
	EXPECT_EQ( A.size1(), B.size1() );
	EXPECT_EQ( A.size2(), B.size2() );
	EXPECT_EQ( A.size3(), B.size3() );
	EXPECT_EQ( A.l1(), B.l1() );
	EXPECT_EQ( A.l2(), B.l2() );
	EXPECT_EQ( A.l3(), B.l3() );
	EXPECT_EQ( A.u1(), B.u1() );
	EXPECT_EQ( A.u2(), B.u2() );
	EXPECT_EQ( A.u3(), B.u3() );
	EXPECT_EQ( A.I1(), B.I1() );
	EXPECT_EQ( A.I2(), B.I2() );
	EXPECT_EQ( A.I3(), B.I3() );
	EXPECT_TRUE( conformable( A, B ) );
	EXPECT_TRUE( equal_dimensions( A, B ) );
}

TEST( Array3Test, ConstructOtherData )
{
	Array3D<double> A( 2, 2, 2 );
	for ( int i1 = A.l1(); i1 <= A.u1(); ++i1 ) {
		for ( int i2 = A.l2(); i2 <= A.u2(); ++i2 ) {
			for ( int i3 = A.l3(); i3 <= A.u3(); ++i3 ) {
				A( i1, i2, i3 ) = i1 + i2 +i3;
			}
		}
	}
	Array3D_int B( A );
	EXPECT_EQ( A.I1(), B.I1() );
	EXPECT_EQ( A.I2(), B.I2() );
	EXPECT_EQ( A.I3(), B.I3() );
	EXPECT_TRUE( conformable( A, B ) );
	EXPECT_TRUE( equal_dimensions( A, B ) );
	for ( int i1 = A.l1(); i1 <= A.u1(); ++i1 ) {
		for ( int i2 = A.l2(); i2 <= A.u2(); ++i2 ) {
			for ( int i3 = A.l3(); i3 <= A.u3(); ++i3 ) {
				EXPECT_EQ( int( A( i1, i2, i3 ) ), B( i1, i2, i3 ) );
				EXPECT_DOUBLE_EQ( A( i1, i2, i3 ), double( B( i1, i2, i3 ) ) ); // Works because they are all integer values
			}
		}
	}
}

TEST( Array3Test, ConstructIndexes )
{
	Array3D_int A( 3, 3, 3 );
	EXPECT_EQ( 27u, A.size() );
	EXPECT_EQ( 3u, A.size1() );
	EXPECT_EQ( 3u, A.size2() );
	EXPECT_EQ( 3u, A.size3() );
	EXPECT_EQ( 1, A.l1() );
	EXPECT_EQ( 1, A.l2() );
	EXPECT_EQ( 1, A.l3() );
	EXPECT_EQ( 3, A.u1() );
	EXPECT_EQ( 3, A.u2() );
	EXPECT_EQ( 3, A.u3() );
	EXPECT_EQ( Array3D_int::IR( 1, 3 ), A.I1() );
	EXPECT_EQ( Array3D_int::IR( 1, 3 ), A.I2() );
	EXPECT_EQ( Array3D_int::IR( 1, 3 ), A.I3() );
}

TEST( Array3Test, RangeBasedFor )
{
	Array3D_int A( 2, 2, 2, { 1, 2, 3, 4, 5, 6, 7, 8 } );
	int v( 0 );
	for ( auto const e : A ) {
		EXPECT_EQ( ++v, e );
	}
}

TEST( Array3Test, Subscript )
{
	Array3D_int A( 2, 2, 2, {
	 111,
	 112,
	 121,
	 122,
	 211,
	 212,
	 221,
	 222
	} );
	EXPECT_EQ( 8u, A.size() );

	// Linear indexing
	EXPECT_EQ( 111, A[ 0 ] );
	EXPECT_EQ( 112, A[ 1 ] );
	EXPECT_EQ( 121, A[ 2 ] );
	EXPECT_EQ( 122, A[ 3 ] );
	EXPECT_EQ( 211, A[ 4 ] );
	EXPECT_EQ( 212, A[ 5 ] );
	EXPECT_EQ( 221, A[ 6 ] );
	EXPECT_EQ( 222, A[ 7 ] );

	// Array indexing
	EXPECT_EQ( 111, A( 1, 1, 1 ) );
	EXPECT_EQ( 112, A( 1, 1, 2 ) );
	EXPECT_EQ( 121, A( 1, 2, 1 ) );
	EXPECT_EQ( 122, A( 1, 2, 2 ) );
	EXPECT_EQ( 211, A( 2, 1, 1 ) );
	EXPECT_EQ( 212, A( 2, 1, 2 ) );
	EXPECT_EQ( 221, A( 2, 2, 1 ) );
	EXPECT_EQ( 222, A( 2, 2, 2 ) );
}

TEST( Array3Test, Predicates )
{
	Array3D_int A1;
	EXPECT_FALSE( A1.active() );
	EXPECT_FALSE( A1.allocated() );
	EXPECT_TRUE( A1.empty() );
	EXPECT_TRUE( A1.size_bounded() );
	EXPECT_TRUE( A1.owner() );
	EXPECT_FALSE( A1.proxy() );

	Array3D_int A2( 2, 3, 2 ); // Uninitialized
	EXPECT_TRUE( A2.active() );
	EXPECT_TRUE( A2.allocated() );
	EXPECT_FALSE( A2.empty() );
	EXPECT_TRUE( A2.owner() );
	EXPECT_FALSE( A2.proxy() );

	Array3D_int A3( 2, 3, 2, 31459 );
	EXPECT_TRUE( A3.active() );
	EXPECT_TRUE( A3.allocated() );
	EXPECT_FALSE( A3.empty() );
	EXPECT_TRUE( A3.owner() );
	EXPECT_FALSE( A3.proxy() );

	Array3D_int A4( 2, 2, 2, { 111, 112, 121, 122, 211, 212, 221, 222 } );
	EXPECT_TRUE( A4.active() );
	EXPECT_TRUE( A4.allocated() );
	EXPECT_FALSE( A4.empty() );
	EXPECT_TRUE( A4.owner() );
	EXPECT_FALSE( A4.proxy() );
}

