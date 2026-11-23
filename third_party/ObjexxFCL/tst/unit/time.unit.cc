// ObjexxFCL::time Unit Tests
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
#include <ObjexxFCL/time.hh>
#include "ObjexxFCL.unit.hh"

// C++ Headers
#include <cctype>

using namespace ObjexxFCL;

TEST( TimeTest, DateAndTime )
{
	std::string d, t, z;
	Array1D_int v( 8 );
	date_and_time( d, t, z, v );
	EXPECT_EQ( d.length(), 8u );
	for ( int i = 0; i < 8; ++i ) {
		EXPECT_TRUE( std::isdigit( d[ i ] ) );
	}
	EXPECT_EQ( t.length(), 10u );
	EXPECT_TRUE( std::isdigit( t[ 0 ] ) );
	EXPECT_TRUE( std::isdigit( t[ 1 ] ) );
	EXPECT_TRUE( std::isdigit( t[ 2 ] ) );
	EXPECT_TRUE( std::isdigit( t[ 3 ] ) );
	EXPECT_TRUE( std::isdigit( t[ 4 ] ) );
	EXPECT_TRUE( std::isdigit( t[ 5 ] ) );
	EXPECT_EQ( t[ 6 ], '.' );
	EXPECT_TRUE( std::isdigit( t[ 7 ] ) );
	EXPECT_TRUE( std::isdigit( t[ 8 ] ) );
	EXPECT_TRUE( std::isdigit( t[ 9 ] ) );
	EXPECT_EQ( z.length(), 5u );
	EXPECT_TRUE( ( z[ 0 ] == '+' ) || ( z[ 0 ] == '-' ) );
	EXPECT_TRUE( std::isdigit( z[ 1 ] ) );
	EXPECT_TRUE( std::isdigit( z[ 2 ] ) );
	EXPECT_TRUE( std::isdigit( z[ 3 ] ) );
	EXPECT_TRUE( std::isdigit( z[ 4 ] ) );
	EXPECT_TRUE( 2000 <= v( 1 ) );
	EXPECT_TRUE( v( 1 ) <= 9999 );
	EXPECT_TRUE( 1 <= v( 2 ) );
	EXPECT_TRUE( v( 2 ) <= 12 );
	EXPECT_TRUE( 1 <= v( 3 ) );
	EXPECT_TRUE( v( 3 ) <= 31 );
	EXPECT_TRUE( v( 4 ) <= 840 ); // +14 h offset is max
	EXPECT_TRUE( 0 <= v( 5 ) );
	EXPECT_TRUE( v( 5 ) <= 59 );
	EXPECT_TRUE( 0 <= v( 6 ) );
	EXPECT_TRUE( v( 6 ) <= 59 );
	EXPECT_TRUE( 0 <= v( 7 ) );
	EXPECT_TRUE( v( 7 ) <= 999 );
}
