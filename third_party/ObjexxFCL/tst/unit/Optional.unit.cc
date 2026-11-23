// ObjexxFCL::Optional Unit Tests
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
#include <ObjexxFCL/Optional.hh>
#include <ObjexxFCL/string.functions.hh> // For concatenation operators
#include "ObjexxFCL.unit.hh"

// C++ Headers
#include <string>

using namespace ObjexxFCL;

TEST( OptionalTest, ConstructionDefault )
{
	Optional_int_const o;
	EXPECT_EQ( Optional_int_const(), o );
	EXPECT_NE( Optional_int_const( 2 ), o );
	EXPECT_EQ( Optional_int_const( _ ), o );
	EXPECT_FALSE( o.present() );
}

TEST( OptionalTest, ConstructionOmit )
{
	Optional_int_const o( _ );
	EXPECT_EQ( Optional_int_const(), o );
	EXPECT_NE( Optional_int_const( 2 ), o );
	EXPECT_EQ( Optional_int_const( _ ), o );
	EXPECT_FALSE( o.present() );
}

TEST( OptionalTest, ConstructionValue )
{
	Optional_int_const o( 33 );
	EXPECT_EQ( 33, o() );
	EXPECT_EQ( 33, int( o ) ); // Conversion operator
	EXPECT_EQ( 33, o ); // Conversion operator
}

TEST( OptionalTest, ConstructionExpression )
{
	Optional_int_const o( 33 * 2 );
	EXPECT_EQ( 66, o() );
	EXPECT_EQ( 66, int( o ) ); // Conversion operator
	EXPECT_EQ( 66, o ); // Conversion operator
}

TEST( OptionalTest, ConstructionCopy )
{
	Optional_int o( 33 );
	Optional_int c( o );
	EXPECT_EQ( 33, c() );
	EXPECT_EQ( 33, int( c ) ); // Conversion operator
	EXPECT_EQ( 33, c ); // Conversion operator
}

TEST( OptionalTest, ConstructionCopyConst )
{
	Optional_int_const o( 33 );
	Optional_int_const c( o );
	EXPECT_EQ( 33, c() );
	EXPECT_EQ( 33, int( c ) ); // Conversion operator
	EXPECT_EQ( 33, c ); // Conversion operator
}

TEST( OptionalTest, AssignmentValue )
{
	int i( -3 );
	Optional_int o( i );
	o = 42;
	EXPECT_EQ( 42, o );
	EXPECT_EQ( 42, i );
}

TEST( OptionalTest, AssignmentOmit )
{
	int i( -3 );
	Optional_int o( i );
	o = _;
	EXPECT_EQ( Optional_int( _ ), o );
	EXPECT_FALSE( o.present() );
	EXPECT_NE( -3, o );
}

TEST( OptionalTest, StringFromLiteral )
{
	Optional_string_const o( "A literal string" );
	EXPECT_EQ( std::string( "A literal string" ), o );
	EXPECT_EQ( "A literal string", o() ); // Need the () on o() when types don't match exactly
	EXPECT_TRUE( o.present() );
	EXPECT_NE( std::string( "Some other string" ), o );
}

TEST( OptionalTest, StringAssignment )
{
	std::string s( "A literal string" );
	Optional_string o( s );
	EXPECT_EQ( std::string( "A literal string" ), o );
	EXPECT_EQ( "A literal string", o() ); // Need the () on o() when types don't match exactly
	EXPECT_TRUE( o.present() );
	EXPECT_NE( std::string( "Some other string" ), o );
	o = "New string";
	EXPECT_EQ( "New string", o() ); // Need the () on o() when types don't match exactly
	EXPECT_EQ( "New string", s );
}

TEST( OptionalTest, ConstReference )
{
	int i( 42 );
	int const & j( i );
	Optional_int_const o( j );
	EXPECT_EQ( 42, o() );
	i = 56;
	EXPECT_EQ( 56, o() );
}

TEST( OptionalTest, ConstOptionalFromNonPresentNonConstOptional )
{
	Optional_int o; // Not "present"
	Optional_int_const c( o );
	EXPECT_FALSE( c.present() );
}

TEST( OptionalTest, ConstOptionalFromConst )
{
	int i( 42 );
	Optional_int_const c( i );
	EXPECT_EQ( 42, c() );
	i = 56;
	EXPECT_EQ( 56, c() ); // Proves that c is not holding a local copy
	EXPECT_FALSE( c.own() ); // So does this
}

TEST( OptionalTest, ConstOptionalFromConstRef )
{
	int i( 42 );
	int const & r( i );
	Optional_int_const c( r );
	EXPECT_EQ( 42, c() );
	i = 56;
	EXPECT_EQ( 56, c() ); // Proves that c is not holding a local copy
	EXPECT_FALSE( c.own() ); // So does this
}

TEST( OptionalTest, ConstOptionalFromNonConstOptional )
{
	int i( 42 );
	Optional_int o( i );
	Optional_int_const c( o );
	EXPECT_EQ( 42, c() );
	i = 56;
	EXPECT_EQ( 56, c() ); // Proves that c is not holding a local copy
	EXPECT_FALSE( c.own() ); // So does this
	o = 123;
	EXPECT_EQ( 123, c() );
}
