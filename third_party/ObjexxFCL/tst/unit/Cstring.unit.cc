// ObjexxFCL::Cstring Unit Tests
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
#include <ObjexxFCL/Cstring.hh>
#include "ObjexxFCL.unit.hh"

// C++ Headers
#include <cstring>

using namespace ObjexxFCL;

TEST( CstringTest, Construction )
{
	{ // Default constructor and assignment
		Cstring s;
		EXPECT_TRUE( s.empty() );
		EXPECT_EQ( 0u, s.length() );
		EXPECT_EQ( 0u, s.size() );
		EXPECT_EQ( 0u, std::strlen( s ) ); // Conversion to C string
		s = "Dog";
		EXPECT_EQ( "Dog", s );
		EXPECT_TRUE( s != "Cat" );
		EXPECT_EQ( std::string( "Dog" ), s );
		EXPECT_EQ( 3u, s.length() );
		EXPECT_EQ( 3u, std::strlen( s ) );
	}

	{ // C string and Copy constructors and assignment
		Cstring s( "Fish" );
		Cstring t( s );
		EXPECT_EQ( "Fish", s );
		EXPECT_EQ( s, t );
		EXPECT_EQ( t, s );
		s = "Hook";
		t = s;
		EXPECT_EQ( "Hook", s );
		EXPECT_EQ( s, t );
		EXPECT_EQ( t, s );
		Cstring u( s, 3 );
		EXPECT_EQ( "Hoo", u );
		EXPECT_EQ( 3u, u.length() );
	}

	{ // std::string constructors and assignment
		Cstring s( std::string( "Fish" ) );
		EXPECT_EQ( "Fish", s );
		s = std::string( "Freshwater" );
		EXPECT_EQ( "Freshwater", s );
		EXPECT_FALSE( s.empty() );
		EXPECT_FALSE( s.is_blank() );
		EXPECT_TRUE( s.not_blank() );
		EXPECT_TRUE( s.has( 't' ) );
		EXPECT_FALSE( s.has( 'T' ) );
	}

	{ // char constructors and assignment
		Cstring s( 'x' );
		EXPECT_EQ( 'x', s );
		s = 'z';
		EXPECT_EQ( 'z', s );
	}

	{ // Length constructors and assignment
		Cstring s( 5 );
		EXPECT_EQ( 5u, s.length() );
		EXPECT_EQ( "     ", s );
		EXPECT_TRUE( s.is_blank() );
		EXPECT_FALSE( s.not_blank() );
	}
}

TEST( CstringTest, Assignment )
{
	Cstring s( "Fish" );
	EXPECT_EQ( "Fish", s );
	s = "Cat";
	EXPECT_EQ( "Cat", s );
	EXPECT_EQ( "CatFish", s += "Fish" );
}

TEST( CstringTest, CaseChange )
{
	Cstring s( "Fish" );
	s.lowercase();
	EXPECT_EQ( "fish", s );
	s.uppercase();
	EXPECT_EQ( "FISH", s );
	EXPECT_TRUE( equali( s, "fiSh" ) );
	EXPECT_EQ( "fish", s.lowercased() );
	EXPECT_EQ( "FISH", s.uppercased() );
}

TEST( CstringTest, JustifyTrim )
{
	Cstring s( "  Fish " );
	EXPECT_EQ( 6u, s.len_trim() );
	s.ljustify();
	EXPECT_EQ( "Fish   ", s );
	s.rjustify();
	EXPECT_EQ( "   Fish", s );
	s.ljustify();
	EXPECT_EQ( "   Fish", s.rjustified() );
	s = "Bozo \t ";
	EXPECT_EQ( 6u, s.len_trim() );
	EXPECT_EQ( "Bozo \t", s.trimmed() );
}

TEST( CstringTest, Centering )
{
	Cstring s( "Fish  " );
	EXPECT_EQ( " Fish ", s.centered() );
	EXPECT_EQ( " Fish ", s.center() );
	EXPECT_EQ( " Fish ", s );
	s = "   Dog ";
	EXPECT_EQ( "  Dog  ", s.centered() );
	EXPECT_EQ( "  Dog  ", s.center() );
	EXPECT_EQ( "  Dog  ", s );
}

TEST( CstringTest, Subscripting )
{
	Cstring s( "Cat" );
	EXPECT_EQ( 'C', s[ 0 ] );
	EXPECT_EQ( 'a', s[ 1 ] );
	EXPECT_EQ( 't', s[ 2 ] );
	s[ 0 ] = 'B';
	EXPECT_EQ( "Bat", s );
}

TEST( CstringTest, Concatenation )
{
	Cstring s( "Cat" );
	Cstring t( "Fish" );
	EXPECT_EQ( "CatFish", s + t );
	s += t;
	EXPECT_EQ( "CatFish", s );
	s += 'X';
	EXPECT_EQ( "CatFishX", s );
	s += std::string( "Ray" );
	EXPECT_EQ( "CatFishXRay", s );
}
