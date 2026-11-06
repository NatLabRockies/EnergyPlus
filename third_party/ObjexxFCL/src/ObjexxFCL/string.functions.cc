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
#include <ObjexxFCL/string.functions.hh>

namespace ObjexxFCL {

// Modifier /////

// Uppercase a string
std::string &
uppercase( std::string & s )
{
	std::string::size_type const s_len( s.length() );
	for ( std::string::size_type i = 0; i < s_len; ++i ) {
		uppercase( s[ i ] );
	}
	return s;
}

// Strip Space from a string's Tails
std::string &
strip( std::string & s )
{
	if ( ! s.empty() ) {
		std::string::size_type const ib( s.find_first_not_of( ' ' ) );
		std::string::size_type const ie( s.find_last_not_of( ' ' ) );
		if ( ( ib == std::string::npos ) || ( ie == std::string::npos ) ) { // All of string is ' '
			s.clear();
		} else {
			if ( ie < s.length() - 1 ) s.erase( ie + 1 );
			if ( ib > 0 ) s.erase( 0, ib );
		}
	}
	return s;
}

// Strip Space from a string's Right Tail
std::string &
rstrip( std::string & s )
{
	if ( ! s.empty() ) {
		std::string::size_type const ie( s.find_last_not_of( ' ' ) );
		if ( ie == std::string::npos ) { // All of string is ' '
			s.clear();
		} else {
			if ( ie < s.length() - 1 ) s.erase( ie + 1 );
		}
	}
	return s;
}

// Generator /////

// Uppercased Copy of a string
std::string uppercased(std::string_view const s) {
  std::string t(s);
  std::string::size_type const t_len(t.length());
  for (std::string::size_type i = 0; i < t_len; ++i) {
    uppercase(t[i]);
  }
  return t;
}

// Left-Justified Copy of a string
std::string ljustified(std::string_view const s) {
  std::string::size_type const off(s.find_first_not_of(' '));
  if ((off > 0) && (off != std::string::npos)) {
    return std::string{s.substr(off)}.append(off, ' ');
  } else {
    return std::string{s};
  }
}

// Right-Justified Copy of a string
std::string rjustified(std::string_view const s) {
  std::string::size_type const s_len_trim(len_trim(s));
  std::string::size_type const off(s.length() - s_len_trim);
  if (off > 0) {
    return std::string(off, ' ').append(s.substr(0, s_len_trim));
  } else {
    return std::string{s};
  }
}

// Trailing Space Trimmed Copy of a string
std::string trimmed(std::string_view const s) {
  if (s.empty()) { // Empty string
    return std::string{};
  } else {
    std::string::size_type const ie(s.find_last_not_of(' '));
    if (ie == std::string::npos) { // Blank string: return empty string
      return std::string();
    } else if (ie < s.length() - 1) { // Trimmed
      return std::string{s.substr(0, ie + 1)};
    } else { // Unchanged
      return std::string{s};
    }
  }
}

// Space Stripped from a string's Tails Copy of a string
std::string stripped(std::string_view const s) {
  if (s.empty()) {
    return std::string{};
  } else {
    std::string::size_type const ib(s.find_first_not_of(' '));
    std::string::size_type const ie(s.find_last_not_of(' '));
    if ((ib == std::string::npos) ||
        (ie == std::string::npos)) { // All of string is ' '
      return std::string();          // Return empty string
    } else {
      return std::string{s.substr(ib, ie - ib + 1)};
    }
  }
}

// Sized to a Specified Length Copy of a string
std::string sized(std::string_view const s, std::string::size_type const len) {
  std::string::size_type const s_len(s.length());
  if (s_len < len) { // Right-padded
    return std::string{s} + std::string(len - s_len, ' ');
  } else if (s_len == len) { // Unchanged
    return std::string{s};
  } else { // Truncated
    return std::string{s.substr(0, len)};
  }
}

} // ObjexxFCL
