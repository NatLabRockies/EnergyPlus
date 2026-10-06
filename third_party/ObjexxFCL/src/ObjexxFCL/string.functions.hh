#ifndef ObjexxFCL_string_functions_hh_INCLUDED
#define ObjexxFCL_string_functions_hh_INCLUDED

// String Functions
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

// ObjexxFCL Headers
#include <ObjexxFCL/TypeTraits.hh>
#include <ObjexxFCL/char.functions.hh>

// C++ Headers
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace ObjexxFCL {

// Predicate /////

// string is Blank?
constexpr
bool
is_blank( std::string_view const s )
{
	return ( s.empty() ? true : s.find_first_not_of( ' ' ) == std::string::npos );
}

// string is Not Blank?
constexpr
bool
not_blank( std::string_view const s )
{
	return ! is_blank( s );
}

// string has a string?
constexpr
bool
has( std::string_view const s, std::string_view const t )
{
	return ( s.find( t ) != std::string::npos );
}

// string has a Character?
inline
bool
has( std::string_view const s, char const c )
{
	return ( s.find( c ) != std::string::npos );
}

// string has a String/Character Case-Insensitively?
template< typename T >
bool
hasi( std::string_view const s, T const & t ); // Implementation below

// Has a Prefix Case-Optionally?
constexpr
bool
has_prefix( std::string_view const s, std::string_view const pre, bool const exact_case = true )
{
	std::string::size_type const pre_len( pre.length() );
	if ( pre_len == 0 ) {
		return false;
	} else if ( s.length() < pre_len ) {
		return false;
	} else if ( exact_case ) {
		for ( std::string::size_type i = 0; i < pre_len; ++i ) {
			if ( s[ i ] != pre[ i ] ) return false;
		}
		return true;
	} else {
		for ( std::string::size_type i = 0; i < pre_len; ++i ) {
			if ( ! equali( s[ i ], pre[ i ] ) ) return false;
		}
		return true;
	}
}

// Has a Prefix Case-Insensitively?
template< typename S >
constexpr
bool
has_prefixi( std::string_view const s, S const & pre )
{
	return has_prefix( s, pre, false );
}

// Comparison /////

// char == char Case-Insensitively?
constexpr
bool
equali_char( char const c, char const d )
{
	return ( to_lower( c ) == to_lower( d ) );
}

// string == string Case-Insensitively?
constexpr
bool
equali( std::string_view const s, std::string_view const t )
{
#if defined(__linux__) || defined(__INTEL_COMPILER) // This is faster
	std::string::size_type const s_len( s.length() );
	if ( s_len != t.length() ) return false;
	for ( std::string::size_type i = 0; i < s_len; ++i ) {
		if ( to_lower( s[ i ] ) != to_lower( t[ i ] ) ) return false;
		if ( ++i == s_len ) break; // Unroll
		if ( to_lower( s[ i ] ) != to_lower( t[ i ] ) ) return false;
		if ( ++i == s_len ) break; // Unroll
		if ( to_lower( s[ i ] ) != to_lower( t[ i ] ) ) return false;
	}
	return true;
#else
	return ( s.length() == t.length() ? std::equal( s.begin(), s.end(), t.begin(), equali_char ) : false );
#endif
}

// char < char Case-Insensitively?
inline
bool
lessthani_char( char const c, char const d )
{
	return ( to_lower( c ) < to_lower( d ) );
}

// string < string Case-Insensitively?
inline
bool
lessthani( std::string_view const s, std::string_view const t )
{
	return std::lexicographical_compare( s.begin(), s.end(), t.begin(), t.end(), lessthani_char );
}

// Inspector /////

// Length
constexpr
std::string::size_type
len( std::string_view const s )
{
	return s.length();
}

// Length Space-Trimmed
constexpr
std::string::size_type
len_trim( std::string_view const s )
{
	return s.find_last_not_of( ' ' ) + 1; // Works if npos returned: npos + 1 == 0
}

// Index of a Substring
template< typename T >
constexpr
std::string::size_type
index( std::string_view const s, T const & t, bool const last = false )
{
	return ( last ? s.rfind( t ) : s.find( t ) );
}

// Index of a Substring Case-Insensitively
inline
std::string::size_type
indexi( std::string_view const s, std::string_view const t, bool const last = false )
{
	if ( last ) {
		auto const i( std::find_end( s.begin(), s.end(), t.begin(), t.end(), equali_char ) );
		return ( i == s.end() ? std::string::npos : static_cast< std::string::size_type >( i - s.begin() ) );
	} else {
		auto const i( std::search( s.begin(), s.end(), t.begin(), t.end(), equali_char ) );
		return ( i == s.end() ? std::string::npos : static_cast< std::string::size_type >( i - s.begin() ) );
	}
}

// Index of a Character Case-Insensitively
constexpr
std::string::size_type
indexi( std::string_view const s, char const c, bool const last = false )
{
	if ( last ) {
		for ( std::string::size_type i = s.length() - 1, e = 0; i >= e; --i ) {
			if ( equali_char( s[ i ], c ) ) return i;
		}
	} else {
		for ( std::string::size_type i = 0, e = s.length(); i < e; ++i ) {
			if ( equali_char( s[ i ], c ) ) return i;
		}
	}
	return std::string::npos;
}

// string has a String/Character Case-Insensitively?
template< typename T >
inline
bool
hasi( std::string_view const s, T const & t ) // Declaration above
{
	return ( indexi( s, t ) != std::string::npos );
}

// Find any Characters of Another String
template< typename T >
inline
std::string::size_type
scan( std::string_view const s, T const & t, bool const last = false )
{
	return ( last ? s.find_last_of( t ) : s.find_first_of( t ) );
}

// Modifier /////

// Uppercase a string
std::string &
uppercase( std::string & s );

// Strip Space from a string's Tails
std::string &
strip( std::string & s );

// Strip Space from a string's Right Tail
std::string &
rstrip( std::string & s );

// Pare a string to a Specified Length
inline
std::string &
pare( std::string & s, std::string::size_type const len )
{
	if ( s.length() > len ) s.erase( len ); // Pare
	return s;
}

// Generator /////

// Uppercased Copy of a string
std::string
uppercased( std::string_view const s );

// Left-Justified Copy of a string
std::string
ljustified( std::string_view const s );

// Right-Justified Copy of a string
std::string
rjustified( std::string_view const s );

// Trailing Space Trimmed Copy of a string
std::string
trimmed( std::string_view const s );

// Space Stripped from a string's Tails Copy of a string
std::string
stripped( std::string_view const s );

// Sized to a Specified Length Copy of a string
std::string
sized( std::string_view const s, std::string::size_type const len );

// Conversion To std::string

} // ObjexxFCL

#endif // ObjexxFCL_string_functions_hh_INCLUDED
