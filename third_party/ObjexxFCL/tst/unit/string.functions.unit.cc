// ObjexxFCL::string.functions Unit Tests
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
#include <ObjexxFCL/string.functions.hh>
#include "ObjexxFCL.unit.hh"

using namespace ObjexxFCL;
using std::string;

TEST( StringFunctionsTest, Predicate )
{
	EXPECT_TRUE( is_blank( string( "" ) ) );
	EXPECT_TRUE( is_blank( string( " " ) ) );
	EXPECT_TRUE( is_blank( string( "  " ) ) );
	EXPECT_FALSE( is_blank( string( "x" ) ) );
	EXPECT_TRUE( not_blank( string( "x" ) ) );
	EXPECT_TRUE( has( string( "cake" ), string( "ak" ) ) );
	EXPECT_TRUE( has( string( "cake" ), "ak" ) );
	EXPECT_TRUE( has( string( "cake" ), 'a' ) );
	EXPECT_TRUE( hasi( string( "cake" ), 'a' ) );
	EXPECT_TRUE( hasi( string( "cake" ), 'K' ) );
	EXPECT_FALSE( has( string( "cake" ), string( "ax" ) ) );
	EXPECT_FALSE( has( string( "cake" ), "ax" ) );
	EXPECT_FALSE( has( string( "cake" ), 'M' ) );
	EXPECT_FALSE( hasi( string( "cake" ), 'Z' ) );
	EXPECT_TRUE( has_prefix( string( "Cat and Dog" ), string( "Cat" ) ) );
	EXPECT_TRUE( has_prefix( string( "Cat and Dog" ), "Cat" ) );
	EXPECT_FALSE( has_prefix( string( "Cat and Dog" ), string( "Bat" ) ) );
	EXPECT_FALSE( has_prefix( string( "Cat and Dog" ), "Bat" ) );
	EXPECT_TRUE( has_prefixi( string( "Cat and Dog" ), "CAT" ) );
	EXPECT_FALSE( has_prefixi( string( "Cat and Dog" ), "BAT" ) );
	string const s( "Fish Tank" );
	EXPECT_TRUE( has_prefix( s, "Fi" ) );
	EXPECT_TRUE( has_prefixi( s, "FIsh" ) );
	EXPECT_FALSE( has_prefix( s, "Fin" ) );
	EXPECT_FALSE( has_prefixi( s, "Fin" ) );
}

TEST( StringFunctionsTest, Comparison )
{
	EXPECT_TRUE( equali( string( "a" ), "A" ) );
	EXPECT_FALSE( equali( string( "a" ), "X" ) );
	EXPECT_TRUE( lessthani( string( "a" ), "b" ) );
	EXPECT_TRUE( lessthani( "a", string( "B" ) ) );
	EXPECT_TRUE( lessthani( string( "A" ), "b" ) );

	string s( "Fish" );
	string t( "FishY" );
	EXPECT_EQ( "Fish", s );
	EXPECT_TRUE( equali( s, "fiSh" ) );
	EXPECT_FALSE( equali( s, "fiShY" ) );
	EXPECT_FALSE( equali( s, t ) );
	EXPECT_TRUE( equali( "fiSh", s ) );
	EXPECT_FALSE( equali( "fiShY", s ) );
	EXPECT_TRUE( equali( "FISH", "fiSh" ) );
	EXPECT_FALSE( equali( "FISH", "fiShY" ) );
	EXPECT_TRUE( equali( "FISH ", "fiSh " ) );
	EXPECT_FALSE( equali( "FISH", "fiSh " ) );
	EXPECT_FALSE( equali( "FISH ", "fiSh" ) );
	uppercase( s );
	EXPECT_EQ( "FISH", s );
	EXPECT_EQ( "FISH", uppercased( s ) );
	EXPECT_TRUE( lessthani( s, "fiShY" ) );
	EXPECT_FALSE( lessthani( s, "fiSh" ) );
	EXPECT_TRUE( lessthani( s, "GISH" ) );
	EXPECT_TRUE( lessthani( "aaa", "Z" ) );
}

TEST( StringFunctionsTest, Inspector )
{
	EXPECT_EQ( 7u, len( string( "Zip Cat" ) ) );
	EXPECT_EQ( 4u, len( string( "Zip " ) ) );
	EXPECT_EQ( 3u, len_trim( string( "Zip " ) ) );
	EXPECT_EQ( 1u, index( string( "Cat in Hat" ), "at" ) );
	EXPECT_EQ( 8u, index( string( "Cat in Hat" ), "at", true ) );
	EXPECT_EQ( 1u, indexi( string( "Cat in Hat" ), "AT" ) );
	EXPECT_EQ( 8u, indexi( string( "Cat in Hat" ), "AT", true ) );
	EXPECT_EQ( 1u, scan( string( "Cat in Hat" ), "XaBt" ) );
	EXPECT_EQ( 9u, scan( string( "Cat in Hat" ), "XaBt", true ) );

	EXPECT_EQ( 7u, len( "Zip Cat" ) );
	EXPECT_EQ( 4u, len( "Zip " ) );
	EXPECT_EQ( 3u, len_trim( "Zip " ) );
	EXPECT_EQ( 1u, index( "Cat in Hat", "at" ) );
	EXPECT_EQ( 8u, index( "Cat in Hat", "at", true ) );
	EXPECT_EQ( 1u, indexi( "Cat in Hat", "AT" ) );
	EXPECT_EQ( 8u, indexi( "Cat in Hat", "AT", true ) );
	EXPECT_EQ( 1u, scan( "Cat in Hat", "XaBt" ) );
	EXPECT_EQ( 9u, scan( "Cat in Hat", "XaBt", true ) );
}

TEST( StringFunctionsTest, Modifier )
{
	string s( "Big Dog" );
	uppercase( s );
	EXPECT_EQ( "BIG DOG", s );

	s = " Dog   ";
	EXPECT_EQ( "Dog", strip( s ) );
	s = " Dog   ";
	EXPECT_EQ( " Dog", rstrip( s ) );
	s = "Doggy";
	EXPECT_EQ( "Dog", pare( s, 3u ) );
}

TEST( StringFunctionsTest, Generator )
{
	EXPECT_EQ( "BIG DOG", uppercased( string( "Big Dog" ) ) );
	EXPECT_EQ( "Dog  ", ljustified( string( "  Dog" ) ) );
	EXPECT_EQ( "  Dog", rjustified( string( "Dog  " ) ) );
	EXPECT_EQ( "Dog", trimmed( string( "Dog  " ) ) );
	EXPECT_EQ( "Dog", stripped( string( " Dog   " ) ) );
	EXPECT_EQ( "A lo", sized( string( "A long story" ), 4u ) );
}

TEST( StringFunctionsTest, StripSpace )
{
	string s( "  Fish " );
	EXPECT_EQ( "Fish", stripped( s ) );
	rstrip( s );
	EXPECT_EQ( "  Fish", s );
	s = "  Cow  ";
	strip( s );
	EXPECT_EQ( "Cow", s );
}

TEST( StringFunctionsTest, StripWhitespace )
{
	string s( " \0\0\t \t\0\t Fish \t\0 ", 17 );
	EXPECT_EQ( string( "\0\0\t \t\0\t Fish \t\0", 15 ), stripped( s ) );
}

TEST( StringFunctionsTest, Pare )
{
	string s( "Fish Tank" );
	EXPECT_EQ( "Fish Tank", pare( s, 12 ) );
	EXPECT_EQ( "Fish", pare( s, 4 ) );
}
