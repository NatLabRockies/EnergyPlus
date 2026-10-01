// ObjexxFCL::Reference Unit Tests
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
#include <ObjexxFCL/Reference.hh>
#include "ObjexxFCL.unit.hh"

using namespace ObjexxFCL;

TEST( ReferenceTest, Basic )
{
	Reference_int_const r;
	//EXPECT_EQ( Reference_int_const(), r ); // Can't compare non-associated References for compat with Fortran
	EXPECT_FALSE( r.attached() );
	EXPECT_FALSE( r.associated() );
	int const j( 42 ), k( 59 );
	r.attach( j );
	EXPECT_EQ( 42, j );
	EXPECT_EQ( 42, r );
	EXPECT_TRUE( r.attached() );
	EXPECT_TRUE( r.associated() );
	EXPECT_TRUE( r.attached( j ) );
	EXPECT_TRUE( r.associated( j ) );
	EXPECT_FALSE( r.attached( k ) );
	EXPECT_FALSE( r.associated( k ) );
	Reference_int_const r2( j );
	EXPECT_TRUE( r.associated( r2 ) );
	EXPECT_TRUE( associated( r, r2 ) );
	r.detach();
	EXPECT_FALSE( r.attached() );
	EXPECT_FALSE( r.associated() );
	EXPECT_FALSE( r.attached( j ) );
	EXPECT_FALSE( r.associated( j ) );
	//r = k; // Doesn't compile since r is const ref
}

TEST( ReferenceTest, Advanced )
{
	int j( 42 ), k( 59 );
	Reference< int > r( j );
	EXPECT_TRUE( r.attached() );
	EXPECT_TRUE( r.associated() );
	EXPECT_TRUE( r.attached( j ) );
	EXPECT_TRUE( r.associated( j ) );
	EXPECT_FALSE( r.attached( k ) );
	EXPECT_FALSE( r.associated( k ) );
	r.detach();
	EXPECT_FALSE( r.attached() );
	EXPECT_FALSE( r.associated() );
	EXPECT_FALSE( r.attached( j ) );
	EXPECT_FALSE( r.associated( j ) );
	r.attach( j );
	r = k;
	EXPECT_EQ( 59, j );
	EXPECT_EQ( 59, r );
//	EXPECT_EQ( &r, &j );
//	EXPECT_NE( &r, &k );
	r >>= k;
	EXPECT_TRUE( r.attached( k ) );
	EXPECT_TRUE( r.associated( k ) );
	EXPECT_FALSE( r.attached( j ) );
	EXPECT_FALSE( r.associated( j ) );
	Reference< int > r1( r ); // Reference of Reference
	EXPECT_EQ( k, r1 );
	Reference< int > r2( r1 );
	EXPECT_EQ( k, r2 );
	r2 >>= j;
	EXPECT_EQ( j, r2 );
	EXPECT_EQ( k, r1 );
	r2 >>= r1;
	EXPECT_EQ( k, r2 );
}

TEST( ReferenceTest, ConstInt )
{
#ifndef OBJEXXFCL_CATCH_NONCONST_REFERENCE_TO_CONST
	int const j( 42 );
	Reference< int > r( j ); // Fortran allows POINTER to INTENT(IN) (const) arg
	EXPECT_TRUE( r.attached() );
	EXPECT_TRUE( r.associated() );
	EXPECT_TRUE( r.attached( j ) );
	EXPECT_TRUE( r.associated( j ) );
	EXPECT_EQ( j, r );
	r.detach();
	EXPECT_FALSE( r.attached() );
	EXPECT_FALSE( r.associated() );
	EXPECT_FALSE( r.attached( j ) );
	EXPECT_FALSE( r.associated( j ) );
#endif
}

TEST( ReferenceTest, String )
{
	std::string s( "A short phrase" );
	Reference_string r( s );
	EXPECT_TRUE( r.attached() );
	EXPECT_TRUE( r.associated() );
	EXPECT_TRUE( r.attached( s ) );
	EXPECT_EQ( s, r );
}

TEST( ReferenceTest, Allocate )
{
	{
		Reference< int > r;
		r.allocate();
		r = 123;
		EXPECT_TRUE( r.attached() );
		EXPECT_TRUE( r.associated() );
		EXPECT_TRUE( r.attached() );
		EXPECT_EQ( 123, r );
		r.deallocate();
		EXPECT_FALSE( r.attached() );
		EXPECT_FALSE( r.associated() );
		EXPECT_FALSE( r.attached() );
	}
	{
		struct S
		{
			int i, j, k;
		};
		Reference< S > r;
		r.allocate();
		r().i = 1;
		r().j = 2;
		r().k = 3;
		EXPECT_TRUE( r.attached() );
		EXPECT_TRUE( r.associated() );
		EXPECT_TRUE( r.attached() );
		EXPECT_EQ( 1, r().i );
		r.deallocate();
		EXPECT_FALSE( r.attached() );
		EXPECT_FALSE( r.associated() );
		EXPECT_FALSE( r.attached() );
	}
}
