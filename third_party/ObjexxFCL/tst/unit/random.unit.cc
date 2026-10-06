// ObjexxFCL::random Unit Tests
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
#include <ObjexxFCL/random.hh>
#include <ObjexxFCL/Array1D.hh>
#include "ObjexxFCL.unit.hh"

using namespace ObjexxFCL;

TEST( RandomTest, RandomNumber )
{
	double r;
	RANDOM_NUMBER( r );
	EXPECT_TRUE( 0.0 <= r );
	EXPECT_TRUE( r < 1.0 );
}

TEST( RandomTest, RandomSeed )
{
	int size;
	Array1D< int > put( { 11, 22, 33 } );
	Array1D< int > get( 3 );
	// Just run them
	RANDOM_SEED();
	RANDOM_SEED( size );
	RANDOM_SEED( _, put );
	RANDOM_SEED( _, _, get );
}
