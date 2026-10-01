// ObjexxFCL::MArray Unit Tests
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
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include "ObjexxFCL.unit.hh"

using namespace ObjexxFCL;

struct C
{
	C( int const m_ = 0, float const x_ = 0.0f ) :
	 m( m_ ),
	 x( x_ )
	{}

	int m;
	float x;
};

TEST( MArrayTest, Basic1D )
{
	Array1D< C > a( 5 );
	for ( int i = a.l(); i <= a.u(); ++i ) a( i ).m = i;
//	int C::*pm( &C::m ); // Can give the pointer to member a name like this also
//	MArray1< Array1D< C >, int > ma( a, pm );
	MArray1< Array1D< C >, int > ma( a, &C::m );
	EXPECT_EQ( a( 1 ).m, ma( 1 ) );
	EXPECT_EQ( a( 2 ).m, ma( 2 ) );
	EXPECT_EQ( a( 3 ).m, ma( 3 ) );
	EXPECT_EQ( a( 4 ).m, ma( 4 ) );
	EXPECT_EQ( a( 5 ).m, ma( 5 ) );
	EXPECT_EQ( 1, ma( 1 ) );
	EXPECT_EQ( 2, ma( 2 ) );
	EXPECT_EQ( 3, ma( 3 ) );
	EXPECT_EQ( 4, ma( 4 ) );
	EXPECT_EQ( 5, ma( 5 ) );
}

TEST( MArrayTest, Range1D )
{
	Array1D< C > a( Array1D< C >::IR( -3, 3 ) );
	for ( int i = a.l(); i <= a.u(); ++i ) a( i ).m = i;
	MArray1< Array1D< C >, int > ma( a, &C::m ); // MArray has 1-based indexing
	EXPECT_EQ( 1, ma.l() );
	EXPECT_EQ( 1, ma.l1() );
	EXPECT_EQ( 1, ma.l( 1 ) );
	EXPECT_EQ( 7, ma.u() );
	EXPECT_EQ( 7, ma.u1() );
	EXPECT_EQ( 7, ma.u( 1 ) );
	EXPECT_EQ( a( -3 ).m, ma( 1 ) );
	EXPECT_EQ( a( -2 ).m, ma( 2 ) );
	EXPECT_EQ( a( -1 ).m, ma( 3 ) );
	EXPECT_EQ( a( 0 ).m, ma( 4 ) );
	EXPECT_EQ( a( 1 ).m, ma( 5 ) );
	EXPECT_EQ( a( 2 ).m, ma( 6 ) );
	EXPECT_EQ( a( 3 ).m, ma( 7 ) );
	EXPECT_EQ( -3, ma( 1 ) );
	EXPECT_EQ( -2, ma( 2 ) );
	EXPECT_EQ( -1, ma( 3 ) );
	EXPECT_EQ( 0, ma( 4 ) );
	EXPECT_EQ( 1, ma( 5 ) );
	EXPECT_EQ( 2, ma( 6 ) );
	EXPECT_EQ( 3, ma( 7 ) );
}

TEST( MArrayTest, MakerMethod1D )
{
	Array1D< C > a( 5 );
	for ( int i = 1; i <= a.u(); ++i ) a( i ).m = i;
	auto ma( a.ma( &C::m ) ); // Array ma maker method
//	auto ma( a.ma( &decltype( a )::Value::m ) ); // Array ma maker method when you don't want to look up the value type of a // Intel C++ 14.x doesn't support this yet
	EXPECT_EQ( a( 1 ).m, ma( 1 ) );
	EXPECT_EQ( a( 2 ).m, ma( 2 ) );
	EXPECT_EQ( a( 3 ).m, ma( 3 ) );
	EXPECT_EQ( a( 4 ).m, ma( 4 ) );
	EXPECT_EQ( a( 5 ).m, ma( 5 ) );
}

//TEST( MArrayTest, EoshiftPos1D )
//{
//	Array1D< C > a( 5 );
//	for ( int i = a.l(); i <= a.u(); ++i ) a( i ).m = i;
//	auto A( make_MArray1( a, &C::m ) );
//	EXPECT_TRUE( eq( Array1D_int( { 3, 4, 5, 0, 0 } ), eoshift( A, 2 ) ) );
//	EXPECT_TRUE( eq( Array1D_int( { 3, 4, 5, 9, 9 } ), eoshift( A, 2, 9 ) ) );
//}
