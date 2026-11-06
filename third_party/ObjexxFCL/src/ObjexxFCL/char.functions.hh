#ifndef ObjexxFCL_char_functions_hh_INCLUDED
#define ObjexxFCL_char_functions_hh_INCLUDED

// Character Functions
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

namespace ObjexxFCL {

// Comparison /////

// Lowercased 8-bit ASCII char
constexpr
char
to_lower( char const c ); // Defined below

// Uppercased 8-bit ASCII char
constexpr
char
to_upper( char const c ); // Defined below

// char == char Case-Insensitively
constexpr
bool
equali( char const c, char const d )
{
	return ( to_lower( c ) == to_lower( d ) );
}

// Modifier /////

// Uppercase a char
constexpr
char &
uppercase( char & c )
{
	c = to_upper( c );
	return c;
}

// Generator /////

// Lowercased 8-bit ASCII char
constexpr
char
to_lower( char const c )
{
	int const i( static_cast< int >( c ) );
	return ( ( ( 65 <= i ) && ( i <= 90 ) ) || ( ( 192 <= i ) && ( i <= 223 ) ) ? i + 32 : i );
}

// Uppercased 8-bit ASCII char
constexpr
char
to_upper( char const c )
{
	int const i( static_cast< int >( c ) );
	return ( ( ( 97 <= i ) && ( i <= 122 ) ) || ( ( 224 <= i ) && ( i <= 255 ) ) ? i - 32 : i );
}

} // ObjexxFCL

#endif // ObjexxFCL_char_functions_hh_INCLUDED
