// ObjexxFCL::Reference+Optional Unit Tests
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
#include <ObjexxFCL/Optional.hh>
#include "ObjexxFCL.unit.hh"

using namespace ObjexxFCL;

TEST( ReferenceOptionalTest, ConstReferenceConstOptional )
{
	int i( 42 );
	int const & j( i );
	Optional_int_const o( j );
	Reference_int_const r( o );
	EXPECT_EQ( 42, o() );
	EXPECT_EQ( 42, r() );
	i = 56;
	EXPECT_EQ( 56, o() );
	EXPECT_EQ( 56, r() );
}

TEST( ReferenceOptionalTest, ConstReferenceConstOptionalValue )
{
	int i( 42 );
	int const & j( i );
	Optional_int_const o( j );
	Reference_int_const r( o() );
	EXPECT_EQ( 42, o() );
	EXPECT_EQ( 42, r() );
	i = 56;
	EXPECT_EQ( 56, o() );
	EXPECT_EQ( 56, r() );
}

TEST( ReferenceOptionalTest, ReferenceConstOptional )
{
	int i( 42 );
	int const & j( i );
	Optional_int_const o( j );
	Reference< int > r( o );
	EXPECT_EQ( 42, o() );
	EXPECT_EQ( 42, r() );
	i = 56;
	EXPECT_EQ( 56, o() );
	EXPECT_EQ( 56, r() );
}

TEST( ReferenceOptionalTest, ReferenceConstOptionalValue )
{
	int i( 42 );
	int const & j( i );
	Optional_int_const o( j );
	Reference< int > r( o() );
	EXPECT_EQ( 42, o() );
	EXPECT_EQ( 42, r() );
	i = 56;
	EXPECT_EQ( 56, o() );
	EXPECT_EQ( 56, r() );
}

TEST( ReferenceOptionalTest, ReferenceAttachConstOptional )
{
	int i( 42 );
	Optional_int_const o( i );
	Reference< int > r;
	r >>= o;
	EXPECT_EQ( 42, o() );
	EXPECT_EQ( 42, r() );
	i = 56;
	EXPECT_EQ( 56, o() );
	EXPECT_EQ( 56, r() );
}
