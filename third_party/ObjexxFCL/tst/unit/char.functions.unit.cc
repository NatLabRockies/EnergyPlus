// ObjexxFCL::char.functions Unit Tests
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
#include <ObjexxFCL/char.functions.hh>
#include "ObjexxFCL.unit.hh"

using namespace ObjexxFCL;

TEST( charFunctionsTest, Comparison )
{
	EXPECT_TRUE( equali( 'a', 'A' ) );
	EXPECT_FALSE( equali( 'a', 'X' ) );
}

TEST( charFunctionsTest, Modifier )
{
	char s( 'f' );
	EXPECT_EQ( 'f', s );
	uppercase( s );
	EXPECT_EQ( 'F', s );
	EXPECT_TRUE( equali( s, 'F' ) );
	EXPECT_TRUE( equali( s, 'f' ) );
	EXPECT_EQ( 'F', uppercase( s ) );
}

TEST( charFunctionsTest, Generator )
{
	char const s( 'F' );
	EXPECT_EQ( 'f', to_lower( s ) );
	EXPECT_EQ( 'F', to_upper( s ) );
}
