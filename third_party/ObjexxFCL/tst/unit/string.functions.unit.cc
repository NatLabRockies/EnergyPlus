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
	EXPECT_TRUE( empty( string( "" ) ) );
	EXPECT_FALSE( empty( string( "X" ) ) );
	EXPECT_TRUE( is_blank( string( "" ) ) );
	EXPECT_TRUE( is_blank( string( " " ) ) );
	EXPECT_TRUE( is_blank( string( "  " ) ) );
	EXPECT_FALSE( is_blank( string( "x" ) ) );
	EXPECT_TRUE( not_blank( string( "x" ) ) );
	EXPECT_TRUE( is_whitespace( string( " " ) ) );
	EXPECT_TRUE( is_whitespace( string( "\t" ) ) );
	EXPECT_FALSE( is_whitespace( string( "x" ) ) );
	EXPECT_TRUE( is_digit( string( "4" ) ) );
	EXPECT_FALSE( is_digit( string( "P" ) ) );
	EXPECT_TRUE( is_lower( string( "e" ) ) );
	EXPECT_FALSE( is_lower( string( "E" ) ) );
	EXPECT_TRUE( is_upper( string( "B" ) ) );
	EXPECT_FALSE( is_upper( string( "b" ) ) );
	EXPECT_TRUE( has_lower( string( "eYe" ) ) );
	EXPECT_FALSE( has_lower( string( "EXO" ) ) );
	EXPECT_TRUE( has_upper( string( "Boo" ) ) );
	EXPECT_FALSE( has_upper( string( "boo" ) ) );
	EXPECT_TRUE( has( string( "cake" ), string( "ak" ) ) );
	EXPECT_TRUE( has( string( "cake" ), "ak" ) );
	EXPECT_TRUE( has( string( "cake" ), 'a' ) );
	EXPECT_TRUE( hasi( string( "cake" ), 'a' ) );
	EXPECT_TRUE( hasi( string( "cake" ), 'K' ) );
	EXPECT_FALSE( has( string( "cake" ), string( "ax" ) ) );
	EXPECT_FALSE( has( string( "cake" ), "ax" ) );
	EXPECT_FALSE( has( string( "cake" ), 'M' ) );
	EXPECT_FALSE( hasi( string( "cake" ), 'Z' ) );
	EXPECT_TRUE( has_any_of( string( "x" ), string( "xyz" ) ) );
	EXPECT_TRUE( has_any_of( string( "x" ), "xyz" ) );
	EXPECT_FALSE( has_any_of( string( "b" ), string( "xyz" ) ) );
	EXPECT_FALSE( has_any_of( string( "b" ), "xyz" ) );
	EXPECT_TRUE( has_prefix( string( "Cat and Dog" ), string( "Cat" ) ) );
	EXPECT_TRUE( has_prefix( string( "Cat and Dog" ), "Cat" ) );
	EXPECT_TRUE( has_prefix( string( "Cat and Dog" ), 'C' ) );
	EXPECT_FALSE( has_prefix( string( "Cat and Dog" ), string( "Bat" ) ) );
	EXPECT_FALSE( has_prefix( string( "Cat and Dog" ), "Bat" ) );
	EXPECT_FALSE( has_prefix( string( "Cat and Dog" ), 'B' ) );
	EXPECT_TRUE( has_prefixi( string( "Cat and Dog" ), "CAT" ) );
	EXPECT_FALSE( has_prefixi( string( "Cat and Dog" ), "BAT" ) );
	string const s( "Fish Tank" );
	EXPECT_TRUE( has_prefix( s, "Fi" ) );
	EXPECT_TRUE( has_prefixi( s, "FIsh" ) );
	EXPECT_FALSE( has_prefix( s, "Fin" ) );
	EXPECT_FALSE( has_prefixi( s, "Fin" ) );
	string const t( "A cat is a cat" );

	EXPECT_TRUE( is_type< int >( string( "123" ) ) );
	EXPECT_FALSE( is_type< int >( string( "Fish" ) ) );
	EXPECT_TRUE( is_type< short int >( string( "123" ) ) );
	EXPECT_TRUE( is_type< long int >( string( "123" ) ) );
	EXPECT_TRUE( is_type< float >( string( "123" ) ) );
	EXPECT_FALSE( is_type< float >( string( "123Go" ) ) );
	EXPECT_TRUE( is_type< double >( string( "123" ) ) );
	EXPECT_TRUE( is_type< float >( string( "123.456" ) ) );
	EXPECT_TRUE( is_type< double >( string( "123.456" ) ) );
	EXPECT_TRUE( is_type< double >( string( "123.456e2" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "0" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "00" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "1" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "01" ) ) );
	EXPECT_FALSE( is_type< bool >( string( "11" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "T" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "t" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "F" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "f" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "true" ) ) );
	EXPECT_FALSE( is_type< bool >( string( "true X" ) ) );
	EXPECT_TRUE( is_type< bool >( string( "false" ) ) );
	EXPECT_FALSE( is_type< bool >( string( "false X" ) ) );
	EXPECT_FALSE( is_type< bool >( string( "X" ) ) );
	EXPECT_TRUE( is_type< char >( string( "F" ) ) );
	EXPECT_FALSE( is_type< char >( string( "Foo" ) ) );
	EXPECT_TRUE( is_decimal( string( "123" ) ) );
	EXPECT_FALSE( is_decimal( string( "123Hats" ) ) );
	EXPECT_FALSE( is_decimal( string( "123 Hats" ) ) );
	EXPECT_TRUE( is_binary( string( "101100" ) ) );
	EXPECT_FALSE( is_binary( string( "123" ) ) );

	EXPECT_TRUE( is_type< bool >( "0" ) );
	EXPECT_TRUE( is_type< bool >( "00" ) );
	EXPECT_TRUE( is_type< bool >( "1" ) );
	EXPECT_TRUE( is_type< bool >( "01" ) );
	EXPECT_FALSE( is_type< bool >( "11" ) );
	EXPECT_TRUE( is_type< bool >( "T" ) );
	EXPECT_TRUE( is_type< bool >( "t" ) );
	EXPECT_TRUE( is_type< bool >( "F" ) );
	EXPECT_TRUE( is_type< bool >( "f" ) );
	EXPECT_TRUE( is_type< bool >( "true" ) );
	EXPECT_FALSE( is_type< bool >( "true X" ) );
	EXPECT_TRUE( is_type< bool >( "false" ) );
	EXPECT_FALSE( is_type< bool >( "false X" ) );
	EXPECT_FALSE( is_type< bool >( "X" ) );
	EXPECT_TRUE( is_type< short int >( "123" ) );
	EXPECT_TRUE( is_type< int >( "123" ) );
	EXPECT_FALSE( is_type< int >( "Fish" ) );
	EXPECT_TRUE( is_type< long int >( "123" ) );
	EXPECT_TRUE( is_type< float >( "123" ) );
	EXPECT_FALSE( is_type< float >( "123Go" ) );
	EXPECT_TRUE( is_type< double >( "123" ) );
	EXPECT_TRUE( is_type< long double >( "123" ) );
	EXPECT_TRUE( is_type< float >( "123.456" ) );
	EXPECT_TRUE( is_type< double >( "123.456" ) );
	EXPECT_TRUE( is_type< double >( "123.456e2" ) );
	EXPECT_TRUE( is_type< long double >( "123.456" ) );
	EXPECT_TRUE( is_type< long double >( "123.456e2" ) );
	EXPECT_TRUE( is_type< char >( "F" ) );
	EXPECT_FALSE( is_type< char >( "Foo" ) );

	EXPECT_TRUE( is_bool( "0" ) );
	EXPECT_TRUE( is_bool( "00" ) );
	EXPECT_TRUE( is_bool( "1" ) );
	EXPECT_TRUE( is_bool( "01" ) );
	EXPECT_FALSE( is_bool( "11" ) );
	EXPECT_TRUE( is_bool( "T" ) );
	EXPECT_TRUE( is_bool( "t" ) );
	EXPECT_TRUE( is_bool( "F" ) );
	EXPECT_TRUE( is_bool( "f" ) );
	EXPECT_TRUE( is_bool( "true" ) );
	EXPECT_FALSE( is_bool( "true X" ) );
	EXPECT_TRUE( is_bool( "false" ) );
	EXPECT_FALSE( is_bool( "false X" ) );
	EXPECT_FALSE( is_bool( "X" ) );
	EXPECT_TRUE( is_short( "123" ) );
	EXPECT_TRUE( is_int( "123" ) );
	EXPECT_FALSE( is_int( "Fish" ) );
	EXPECT_TRUE( is_float( "123" ) );
	EXPECT_FALSE( is_float( "123Go" ) );
	EXPECT_TRUE( is_double( "123" ) );
	EXPECT_TRUE( is_float( "123.456" ) );
	EXPECT_TRUE( is_double( "123.456" ) );
	EXPECT_TRUE( is_double( "123.456e2" ) );
	EXPECT_TRUE( is_char( "F" ) );
	EXPECT_FALSE( is_char( "Foo" ) );
	EXPECT_TRUE( is_decimal( "123" ) );
	EXPECT_FALSE( is_decimal( "123Hats" ) );
	EXPECT_FALSE( is_decimal( "123 Hats" ) );
	EXPECT_TRUE( is_binary( "101100" ) );
	EXPECT_FALSE( is_binary( "123" ) );
}

TEST( StringFunctionsTest, Comparison )
{
	EXPECT_TRUE( equali( string( "a" ), "A" ) );
	EXPECT_FALSE( equali( string( "a" ), "X" ) );
	EXPECT_TRUE( equal( string( "a" ), "a" ) );
	EXPECT_TRUE( equal( "a", string( "a" ), true ) );
	EXPECT_FALSE( equal( "a", "A" ) );
	EXPECT_TRUE( equal( string( "a" ), "A", false ) );
	EXPECT_TRUE( lessthan( string( "a" ), "b" ) );
	EXPECT_TRUE( lessthan( "a", string( "b" ), true ) );
	EXPECT_FALSE( lessthan( "a", string( "B" ), true ) );
	EXPECT_TRUE( lessthan( string( "A" ), "b", false ) );
	EXPECT_FALSE( lessthan( "b", string( "A" ), false ) );
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
	EXPECT_TRUE( equal( s, "fiSh", false ) );
	EXPECT_TRUE( equal( s, "Fish" ) );
	EXPECT_TRUE( equal( s, "FiSh", false ) );
	EXPECT_FALSE( equal( s, "fiSh" ) );
	EXPECT_FALSE( equal( s, "fiSh", true ) );
	uppercase( s );
	EXPECT_EQ( "FISH", s );
	EXPECT_EQ( "FISH", uppercased( s ) );
	EXPECT_TRUE( lessthani( s, "fiShY" ) );
	EXPECT_FALSE( lessthani( s, "fiSh" ) );
	EXPECT_TRUE( lessthani( s, "GISH" ) );
	EXPECT_TRUE( lessthani( "aaa", "Z" ) );

	EXPECT_TRUE( llt( string( "a" ), string( "b" ) ) );
	EXPECT_FALSE( llt( string( "a" ), string( "a" ) ) );
	EXPECT_FALSE( llt( "b", string( "a" ) ) );
	EXPECT_TRUE( lle( string( "a" ), string( "b" ) ) );
	EXPECT_TRUE( lle( string( "a" ), string( "a" ) ) );
	EXPECT_FALSE( lle( "b", string( "a" ) ) );
	EXPECT_FALSE( lgt( string( "a" ), string( "b" ) ) );
	EXPECT_FALSE( lgt( string( "a" ), string( "a" ) ) );
	EXPECT_TRUE( lgt( "b", string( "a" ) ) );
	EXPECT_FALSE( lge( string( "a" ), string( "b" ) ) );
	EXPECT_TRUE( lge( string( "a" ), string( "a" ) ) );
	EXPECT_TRUE( lge( "b", string( "a" ) ) );
}

TEST( StringFunctionsTest, Inspector )
{
	EXPECT_EQ( 7u, len( string( "Zip Cat" ) ) );
	EXPECT_EQ( 4u, len( string( "Zip " ) ) );
	EXPECT_EQ( 3u, len_trim( string( "Zip " ) ) );
	EXPECT_EQ( 1u, index( string( "Cat in Hat" ), "at" ) );
	EXPECT_EQ( 8u, index( string( "Cat in Hat" ), "at", true ) );
	EXPECT_EQ( 8u, rindex( string( "Cat in Hat" ), "at" ) );
	EXPECT_EQ( 1u, indexi( string( "Cat in Hat" ), "AT" ) );
	EXPECT_EQ( 8u, indexi( string( "Cat in Hat" ), "AT", true ) );
	EXPECT_EQ( 1u, scan( string( "Cat in Hat" ), "XaBt" ) );
	EXPECT_EQ( 9u, scan( string( "Cat in Hat" ), "XaBt", true ) );
	EXPECT_EQ( 2u, verify( string( "Cat in Hat" ), "CaBo" ) );
	EXPECT_EQ( 7u, verify( string( "Cat in Hat" ), "XaBt", true ) );

	EXPECT_EQ( 7u, len( "Zip Cat" ) );
	EXPECT_EQ( 4u, len( "Zip " ) );
	EXPECT_EQ( 3u, len_trim( "Zip " ) );
	EXPECT_EQ( 1u, index( "Cat in Hat", "at" ) );
	EXPECT_EQ( 8u, index( "Cat in Hat", "at", true ) );
	EXPECT_EQ( 8u, rindex( "Cat in Hat", "at" ) );
	EXPECT_EQ( 1u, indexi( "Cat in Hat", "AT" ) );
	EXPECT_EQ( 8u, indexi( "Cat in Hat", "AT", true ) );
	EXPECT_EQ( 1u, scan( "Cat in Hat", "XaBt" ) );
	EXPECT_EQ( 9u, scan( "Cat in Hat", "XaBt", true ) );
	EXPECT_EQ( 2u, verify( "Cat in Hat", "CaBo" ) );
	EXPECT_EQ( 7u, verify( "Cat in Hat", "XaBt", true ) );
}

TEST( StringFunctionsTest, Conversion )
{
	EXPECT_EQ( 99, ichar( string( "c" ) ) );

	EXPECT_EQ( 99, ichar( "c" ) );
}

TEST( StringFunctionsTest, Modifier )
{
	string s( "Big Dog" );
	uppercase( s );
	EXPECT_EQ( "BIG DOG", s );

	s = "Dog  ";
	EXPECT_EQ( "Dog", trim( s ) );
	s = "aBana";
	EXPECT_EQ( "B", strip( s, "an" ) );
	s = "aBana";
	s = "aBana";
	EXPECT_EQ( "aB", rstrip( s, "an" ) );
	s = " Dog   ";
	EXPECT_EQ( "Dog", strip( s ) );
	s = " Dog   ";
	s = " Dog   ";
	EXPECT_EQ( " Dog", rstrip( s ) );
	s = "Dog";
	EXPECT_EQ( "Dog   ", pad( s, 6u ) );
	s = "Dog";
	s = "Dog";
	s = "Doggy";
	EXPECT_EQ( "Dog", pare( s, 3u ) );
	s = "a Dog";
	s = "Doggy";
	s = "A long story";
	EXPECT_EQ( "A lo", size( s, 4u ) );
	s = "A long story";
	s = "A long story";
	s = "   Center Me     ";
	EXPECT_EQ( "    Center Me    ", center( s ) );
	s = "   Center Me     ";
	EXPECT_EQ( "      Center Me      ", center( s, 21u ) );
	s = "   Center Me     ";
	EXPECT_EQ( "Center Me", center( s, 9u ) );
	s = "   Center Me     ";
	EXPECT_EQ( "enter M", center( s, 7u ) );
	s = "   Center Me     ";
	EXPECT_EQ( "nter ", center( s, 5u ) );
	s = "   Center Me     ";
	EXPECT_EQ( "ter", center( s, 3u ) );
	s = "Banana";
	EXPECT_EQ( "Ban", unique( s ) );
	s = "Banana";
	EXPECT_EQ( "Bonono", replace( s, "a", "o" ) );
	s = "Banana";
	EXPECT_EQ( "Bonona", replace( s, "an", "on" ) );
	s = "Bat";
	EXPECT_EQ( "\"Bat\"", quote( s ) );
	s = "Cat";
	EXPECT_EQ( "Dog", overlay( s, "Dog" ) );
	s = "Cat and Dog";
	EXPECT_EQ( "Cat Hat Dog", overlay( s, "Hat", 4 ) );
	s = "Cat and Dog";
	EXPECT_EQ( "Cat and Gator", overlay( s, "Gator", 8u ) );
}

TEST( StringFunctionsTest, Generator )
{
	EXPECT_EQ( "     ", blank( 5 ) );
	EXPECT_EQ( "BIG DOG", uppercased( string( "Big Dog" ) ) );
	EXPECT_EQ( "Dog  ", ljustified( string( "  Dog" ) ) );
	EXPECT_EQ( "  Dog", rjustified( string( "Dog  " ) ) );
	EXPECT_EQ( "Dog", trimmed( string( "Dog  " ) ) );
	EXPECT_EQ( "Dog", trimmed_whitespace( string( "Dog \t \0  " ) ) );
	EXPECT_EQ( "B", stripped( string( "aBana" ), "an" ) );
	EXPECT_EQ( "Dog", stripped( string( " Dog   " ) ) );
	EXPECT_EQ( "Dog", stripped_whitespace( string( " Dog \t " ) ) );
	EXPECT_EQ( "A lo", sized( string( "A long story" ), 4u ) );
	EXPECT_EQ( "    Center Me    ", centered( string( "   Center Me     " ) ) );
	EXPECT_EQ( "      Center Me      ", centered( string( "   Center Me     " ), 21u ) );
	EXPECT_EQ( "Center Me", centered( string( "   Center Me     " ), 9u ) );
	EXPECT_EQ( "enter M", centered( string( "   Center Me     " ), 7u ) );
	EXPECT_EQ( "nter ", centered( string( "   Center Me     " ), 5u ) );
	EXPECT_EQ( "ter", centered( string( "   Center Me     " ), 3u ) );
	EXPECT_EQ( "Bonono", replaced( string( "Banana" ), "a", "o" ) );
	EXPECT_EQ( "Bonona", replaced( string( "Banana" ), "an", "on" ) );
	EXPECT_EQ( "HaHaHa", repeated( string( "Ha" ), 3 ) );
	EXPECT_EQ( "HaHaHa", repeat( string( "Ha" ), 3 ) );
	EXPECT_EQ( "Fish", head( string( "Fish gotta swim" ) ) );
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
	EXPECT_EQ( "Fish", stripped_whitespace( s ) );
}

TEST( StringFunctionsTest, StripSpecifiedCharacters )
{
	string s( "Fish" );
	EXPECT_EQ( "Fis", stripped( s, "h" ) );
	EXPECT_EQ( "ish", stripped( s, "F" ) );
	EXPECT_EQ( "is", stripped( s, "Fh" ) );
	rstrip( s, "sh" );
	EXPECT_EQ( "Fi", s );
}

TEST( StringFunctionsTest, Centering )
{
	string s( "    Fish Tank" );
	EXPECT_EQ( "  Fish Tank  ", centered( s ) );
	EXPECT_EQ( "  Fish Tank  ", center( s ) );
	EXPECT_EQ( "  Fish Tank  ", s );
}

TEST( StringFunctionsTest, Replace )
{
	string const s( "Fish Tank" );
	EXPECT_EQ( "Dish Tank", replaced( s, "F", "D" ) );
	EXPECT_EQ( "Foolish Tank", replaced( s, "ish", "oolish" ) );
	EXPECT_EQ( "Fish Truck", replaced( s, "ank", "ruck" ) );
	// VC++2013 can't handle some escape sequences in macros like EXPECT that use the # stringize operator so we define them out of line
	string const t( "A \\\"Fishy\\\" Story" );
	string const t2( "A \"Fishy\" Story" );
	string const slash( "\\" );
	string const dslash( "\\\\" );
	string const slash_dquote( "\\\"" );
	string const dquote( "\"" );
	EXPECT_EQ( t2, replaced( replaced( t, dslash, slash ), slash_dquote, dquote ) );
}

TEST( StringFunctionsTest, Repeated )
{
	string const s( "Fish" );
	EXPECT_EQ( "FishFishFish", repeated( s, 3 ) );
}

TEST( StringFunctionsTest, Pad )
{
	string s( "Fish Tank" );
	EXPECT_EQ( "Fish Tank", pad( s, 5 ) );
	EXPECT_EQ( "Fish Tank   ", pad( s, 12 ) );
}

TEST( StringFunctionsTest, Pare )
{
	string s( "Fish Tank" );
	EXPECT_EQ( "Fish Tank", pare( s, 12 ) );
	EXPECT_EQ( "Fish", pare( s, 4 ) );
}
