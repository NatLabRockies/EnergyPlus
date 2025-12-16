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

	// string is Empty?
	constexpr
		bool
		empty(std::string_view const s)
	{
		return s.empty();
	}

	// string is Blank?
	constexpr
		bool
		is_blank(std::string_view const s)
	{
		return (s.empty() ? true : s.find_first_not_of(' ') == std::string::npos);
	}

	// string is Not Blank?
	constexpr
		bool
		not_blank(std::string_view const s)
	{
		return !is_blank(s);
	}

	// string is Whitespace?
	constexpr
		bool
		is_whitespace(std::string_view const s)
	{
		return (s.empty() ? true : s.find_last_not_of(" \t\000") == std::string::npos);
	}

	// string is Digits?
	inline
		bool
		is_digit(std::string_view const s)
	{
		if (s.empty()) {
			return false;
		}
		else {
			for (char const c : s) {
				if (std::isdigit(c) == 0) return false;
			}
			return true;
		}
	}

	// string is Lowercase Alphabetic?
	inline
		bool
		is_lower(std::string_view const s)
	{
		if (s.empty()) {
			return false;
		}
		else {
			for (char const c : s) {
				if (std::islower(c) == 0) return false;
			}
			return true;
		}
	}

	// string is Uppercase Alphabetic?
	inline
		bool
		is_upper(std::string_view const s)
	{
		if (s.empty()) {
			return false;
		}
		else {
			for (char const c : s) {
				if (std::isupper(c) == 0) return false;
			}
			return true;
		}
	}

	// string has a Lowercase Character?
	inline
		bool
		has_lower(std::string_view const s)
	{
		for (char const c : s) {
			if (std::islower(c) != 0) return true;
		}
		return false;
	}

	// string has an Uppercase Character?
	inline
		bool
		has_upper(std::string_view const s)
	{
		for (char const c : s) {
			if (std::isupper(c) != 0) return true;
		}
		return false;
	}

	// string has a string?
	constexpr
		bool
		has(std::string_view const s, std::string_view const t)
	{
		return (s.find(t) != std::string::npos);
	}

	// string has a Character?
	inline
		bool
		has(std::string_view const s, char const c)
	{
		return (s.find(c) != std::string::npos);
	}

	// string has a String/Character Case-Insensitively?
	template< typename T >
	bool
		hasi(std::string_view const s, T const& t); // Implementation below

	// string has any Character of a string_view?
	constexpr
		bool
		has_any_of(std::string_view const s, std::string_view const t)
	{
		return (s.find_first_of(t) != std::string::npos);
	}


	// Has a Prefix Case-Optionally?
	constexpr
		bool
		has_prefix(std::string_view const s, std::string_view const pre, bool const exact_case = true)
	{
		std::string::size_type const pre_len(pre.length());
		if (pre_len == 0) {
			return false;
		}
		else if (s.length() < pre_len) {
			return false;
		}
		else if (exact_case) {
			for (std::string::size_type i = 0; i < pre_len; ++i) {
				if (s[i] != pre[i]) return false;
			}
			return true;
		}
		else {
			for (std::string::size_type i = 0; i < pre_len; ++i) {
				if (!equali(s[i], pre[i])) return false;
			}
			return true;
		}
	}


	// Has a Prefix Case-Optionally?
	constexpr
		bool
		has_prefix(std::string_view const s, char const pre, bool const exact_case = true)
	{
		if (s.length() == 0) {
			return false;
		}
		else if (exact_case) {
			return (s[0] == pre);
		}
		else {
			return equali(s[0], pre);
		}
	}


	// Has a Prefix Case-Insensitively?
	template< typename S >
	constexpr
		bool
		has_prefixi(std::string_view const s, S const& pre)
	{
		return has_prefix(s, pre, false);
	}

	// Trailing Whitespace Trimmed Copy of a string
	std::string
		trimmed_whitespace(std::string_view const s); // Declaration copy for use below

	// string is Readable as a Type Supporting Stream Input?
	template< typename T >
	inline
		bool
		is_type(std::string const& s)
	{
		if (is_whitespace(s)) { // Don't accept empty or whitespace string
			return false;
		}
		else { // Try to read the string as a T
			std::istringstream t_stream(trimmed_whitespace(s));
			T t;
			t_stream >> t;
			return (t_stream && t_stream.eof());
		}
	}

	// string is Readable as a Type Supporting Stream Input?
	template< typename T >
	inline
		bool
		is_type(char const* const s)
	{
		return is_type< T >(std::string(s));
	}

	// string is Readable as a bool?
	template<>
	inline
		bool
		is_type< bool >(std::string const& s)
	{
		std::string const t(trimmed_whitespace(s));
		if (t.empty()) { // Don't accept empty or whitespace string
			return false;
		}
		else if ((t == "T") || (t == "t") || (t == "true")) { // Accept for true
			return true;
		}
		else if ((t == "F") || (t == "f") || (t == "false")) { // Accept for false
			return true;
		}
		else { // Try to read the string as 0/1 bool
			std::istringstream b_stream(t);
			bool b;
			b_stream >> b;
			return (b_stream && b_stream.eof());
		}
	}

	// Is char Pointer Pointing to String Whitespace Tail
	inline
		bool
		is_tail(char* end)
	{
		if (end == nullptr) return false;
		while (std::isspace(*end)) ++end;
		return (*end == '\0');
	}

	// string is Readable as a short int?
	template<>
	inline
		bool
		is_type< short int >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		long int const i(std::strtol(str, &end, 10));
		return ((end != str) && is_tail(end) && (std::numeric_limits< short int >::min() <= i) && (i <= std::numeric_limits< short int >::max()));
	}

	// string is Readable as an int?
	template<>
	inline
		bool
		is_type< int >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		long int const i(std::strtol(str, &end, 10));
		return ((end != str) && is_tail(end) && (std::numeric_limits< int >::min() <= i) && (i <= std::numeric_limits< int >::max()));
	}

	// string is Readable as a long int?
	template<>
	inline
		bool
		is_type< long int >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		static_cast<void>(std::strtol(str, &end, 10));
		return ((end != str) && is_tail(end));
	}

	// string is Readable as a long long int?
	template<>
	inline
		bool
		is_type< long long int >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		static_cast<void>(std::strtoll(str, &end, 10));
		return ((end != str) && is_tail(end));
	}

	// string is Readable as an unsigned short int?
	template<>
	inline
		bool
		is_type< unsigned short int >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		unsigned long int const i(std::strtoul(str, &end, 10));
		return ((end != str) && is_tail(end) && (i <= std::numeric_limits< unsigned short int >::max()));
	}

	// string is Readable as an unsigned int?
	template<>
	inline
		bool
		is_type< unsigned int >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		unsigned long int const i(std::strtoul(str, &end, 10));
		return ((end != str) && is_tail(end) && (i <= std::numeric_limits< unsigned int >::max()));
	}

	// string is Readable as an unsigned long int?
	template<>
	inline
		bool
		is_type< unsigned long int >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		static_cast<void>(std::strtoul(str, &end, 10));
		return ((end != str) && is_tail(end));
	}

	// string is Readable as an unsigned long long int?
	template<>
	inline
		bool
		is_type< unsigned long long int >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		static_cast<void>(std::strtoull(str, &end, 10));
		return ((end != str) && is_tail(end));
	}

	// string is Readable as a float?
	template<>
	inline
		bool
		is_type< float >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		static_cast<void>(std::strtof(str, &end));
		return ((end != str) && is_tail(end));
	}

	// string is Readable as a double?
	template<>
	inline
		bool
		is_type< double >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		static_cast<void>(std::strtod(str, &end));
		return ((end != str) && is_tail(end));
	}

	// string is Readable as a long double?
	template<>
	inline
		bool
		is_type< long double >(std::string const& s)
	{
		char const* str(s.c_str());
		char* end;
		static_cast<void>(std::strtold(str, &end));
		return ((end != str) && is_tail(end));
	}

	// string is Readable as a char?
	template<>
	inline
		bool
		is_type< char >(std::string const& s)
	{
		return (s.length() == 1);
	}

	// string is Readable as a bool?
	inline
		bool
		is_bool(std::string const& s)
	{
		return is_type< bool >(s);
	}

	// string is Readable as a short int?
	inline
		bool
		is_short(std::string const& s)
	{
		return is_type< short int >(s);
	}

	// string is Readable as an int?
	inline
		bool
		is_int(std::string const& s)
	{
		return is_type< int >(s);
	}

	// string is Readable as a long long int?
	inline
		bool
		is_longlong(std::string const& s)
	{
		return is_type< long long int >(s);
	}

	// string is Readable as an unsigned short int?
	inline
		bool
		is_ushort(std::string const& s)
	{
		return is_type< unsigned short int >(s);
	}

	// string is Readable as a float?
	inline
		bool
		is_float(std::string const& s)
	{
		return is_type< float >(s);
	}

	// string is Readable as a double?
	inline
		bool
		is_double(std::string const& s)
	{
		return is_type< double >(s);
	}

	// string is Readable as a char?
	inline
		bool
		is_char(std::string const& s)
	{
		return is_type< char >(s);
	}

	// string is Readable as a Decimal Integer?
	inline
		bool
		is_decimal(std::string const& s, bool const allow_sign = true)
	{
		char const* str(s.c_str());
		char* end;
		long int const i(std::strtol(str, &end, 10));
		return ((end != str) && is_tail(end) && (allow_sign || (i >= 0)));
	}

	// string is Readable as a Binary Integer?
	inline
		bool
		is_binary(std::string const& s, bool const allow_sign = true)
	{
		char const* str(s.c_str());
		char* end;
		long int const i(std::strtol(str, &end, 2));
		return ((end != str) && is_tail(end) && (allow_sign || (i >= 0)));
	}

	// string is Readable as a bool?
	inline
		bool
		is_bool(char const* const s)
	{
		return is_type< bool >(s);
	}

	// string is Readable as a short int?
	inline
		bool
		is_short(char const* const s)
	{
		return is_type< short int >(s);
	}

	// string is Readable as an int?
	inline
		bool
		is_int(char const* const s)
	{
		return is_type< int >(s);
	}

	// string is Readable as a long long int?
	inline
		bool
		is_longlong(char const* const s)
	{
		return is_type< long long int >(s);
	}

	// string is Readable as an unsigned short int?
	inline
		bool
		is_ushort(char const* const s)
	{
		return is_type< unsigned short int >(s);
	}

	// string is Readable as a float?
	inline
		bool
		is_float(char const* const s)
	{
		return is_type< float >(s);
	}

	// string is Readable as a double?
	inline
		bool
		is_double(char const* const s)
	{
		return is_type< double >(s);
	}

	// string is Readable as a char?
	inline
		bool
		is_char(char const* const s)
	{
		return is_type< char >(s);
	}

	// string is Readable as a Decimal Integer?
	inline
		bool
		is_decimal(char const* const s, bool const allow_sign = true)
	{
		char* end;
		long int const i(std::strtol(s, &end, 10));
		return ((end != s) && is_tail(end) && (allow_sign || (i >= 0)));
	}

	// string is Readable as a Binary Integer?
	inline
		bool
		is_binary(char const* const s, bool const allow_sign = true)
	{
		char* end;
		long int const i(std::strtol(s, &end, 2));
		return ((end != s) && is_tail(end) && (allow_sign || (i >= 0)));
	}

	// Comparison /////

	// char == char Case-Insensitively?
	constexpr
		bool
		equali_char(char const c, char const d)
	{
		if (c == d) {
			//Same character
			return true;
		}
		//Without passing or changing the character, check if it's the same, ignoring case
		else if (c >= 'A' && c <= 'Z') {
			if (c == (d - 32)) {
				return true;
			}
			else {
				return false;
			}
		}
		else if (c >= 'a' && c <= 'z') {
			if (c == (d + 32)) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}

	// string == string Case-Insensitively?
	constexpr
		bool
		equali(std::string_view const s, std::string_view const t)
	{
#if defined(__linux__) || defined(__INTEL_COMPILER) // This is faster
		std::string::size_type const s_len(s.length());
		if (s_len != t.length()) return false;
		if (s_len == 0) {
			//Both are empty strings, return true
			return true;
		}
		else if (to_lower(s[(int)s.length() / 2]) != to_lower(t[(int)t.length() / 2])) {
			//Check the middle character for a quick false
			return false;
		}
		for (std::string::size_type i = 0; i < s_len; ++i) {
			if (to_lower(s[i]) != to_lower(t[i])) return false;
			if (++i == s_len) break; // Unroll
			if (to_lower(s[i]) != to_lower(t[i])) return false;
			if (++i == s_len) break; // Unroll
			if (to_lower(s[i]) != to_lower(t[i])) return false;
		}
		return true;
#else
		if (s.length() == t.length()) {
			if (s.length() == 0) {
				//Both are empty strings, return true
				return true;
			}
			else if (!equali_char(s[(int)s.length() / 2], t[(int)t.length() / 2])) {
				//Check the middle character for a quick false
				return false;
			}
			//Do case insensitive compare
			return std::equal(s.begin(), s.end(), t.begin(), equali_char);
		}
		return false;
#endif
	}


	// string == string Case-Optionally?
	inline
		bool
		equal(std::string_view const s, std::string_view const t, bool const exact_case = true)
	{
		return (exact_case ? s == t : equali(s, t));
	}


	// char < char Case-Sensitively?
	inline
		bool
		lessthan_char(char const c, char const d)
	{
		return c < d;
	}

	// char < char Case-Insensitively?
	inline
		bool
		lessthani_char(char const c, char const d)
	{
		return (to_lower(c) < to_lower(d));
	}

	// string < string Case-Insensitively?
	inline
		bool
		lessthan(std::string_view const s, std::string_view const t, bool const exact_case = true)
	{
		if (exact_case) {
			return std::lexicographical_compare(s.begin(), s.end(), t.begin(), t.end(), lessthan_char);
		}
		else {
			return std::lexicographical_compare(s.begin(), s.end(), t.begin(), t.end(), lessthani_char);
		}
	}

	// string < string Case-Insensitively?
	inline
		bool
		lessthani(std::string_view const s, std::string_view const t)
	{
		return std::lexicographical_compare(s.begin(), s.end(), t.begin(), t.end(), lessthani_char);
	}

	// ASCII Lexical < Comparison
	inline
		bool
		llt(std::string_view const s, std::string_view const t)
	{
		return (s < t);
	}

	// ASCII Lexical <= Comparison
	inline
		bool
		lle(std::string_view const s, std::string_view const t)
	{
		return (s <= t);
	}

	// ASCII Lexical >= Comparison
	inline
		bool
		lge(std::string_view const s, std::string_view const t)
	{
		return (s >= t);
	}

	// ASCII Lexical < Comparison
	inline
		bool
		lgt(std::string_view const s, std::string_view const t)
	{
		return (s > t);
	}

	// Inspector /////

	// Length
	constexpr
		std::string::size_type
		len(std::string_view const s)
	{
		return s.length();
	}


	// Length Space-Trimmed
	constexpr
		std::string::size_type
		len_trim(std::string_view const s)
	{
		return s.find_last_not_of(' ') + 1; // Works if npos returned: npos + 1 == 0
	}

	// Index of a Substring
	template< typename T >
	constexpr
		std::string::size_type
		index(std::string_view const s, T const& t, bool const last = false)
	{
		return (last ? s.rfind(t) : s.find(t));
	}

	// Last Index of a Substring
	template< typename T >
	constexpr
		std::string::size_type
		rindex(std::string_view const s, T const& t)
	{
		return s.rfind(t);
	}

	// Index of a Substring Case-Insensitively
	inline
		std::string::size_type
		indexi(std::string_view const s, std::string_view const t, bool const last = false)
	{
		if (last) {
			auto const i(std::find_end(s.begin(), s.end(), t.begin(), t.end(), equali_char));
			return (i == s.end() ? std::string::npos : static_cast<std::string::size_type>(i - s.begin()));
		}
		else {
			auto const i(std::search(s.begin(), s.end(), t.begin(), t.end(), equali_char));
			return (i == s.end() ? std::string::npos : static_cast<std::string::size_type>(i - s.begin()));
		}
	}

	// Index of a Character Case-Insensitively
	constexpr
		std::string::size_type
		indexi(std::string_view const s, char const c, bool const last = false)
	{
		if (last) {
			for (std::string::size_type i = s.length() - 1, e = 0; i >= e; --i) {
				if (equali_char(s[i], c)) return i;
			}
		}
		else {
			for (std::string::size_type i = 0, e = s.length(); i < e; ++i) {
				if (equali_char(s[i], c)) return i;
			}
		}
		return std::string::npos;
	}

	// Last Index of a Substring Case-Insensitively
	template< typename T >
	inline
		std::string::size_type
		rindexi(std::string_view const s, T const& t)
	{
		return indexi(s, t, true);
	}

	// string has a String/Character Case-Insensitively?
	template< typename T >
	inline
		bool
		hasi(std::string_view const s, T const& t) // Declaration above
	{
		return (indexi(s, t) != std::string::npos);
	}

	// Find any Characters of Another String
	template< typename T >
	inline
		std::string::size_type
		scan(std::string_view const s, T const& t, bool const last = false)
	{
		return (last ? s.find_last_of(t) : s.find_first_of(t));
	}

	// Find any Characters not of Another String
	template< typename T >
	inline
		std::string::size_type
		verify(std::string_view const s, T const& t, bool const last = false)
	{
		return (last ? s.find_last_not_of(t) : s.find_first_not_of(t));
	}

	// Conversion /////

	// Integer Value of a string
	inline
		int
		ichar(std::string_view const s)
	{
		return (!s.empty() ? static_cast<int>(s[0]) : 0);
	}

	// Conversion to Numeric Types /////

	// Modifier /////

	// Uppercase a string
	std::string&
		uppercase(std::string& s);

	// Trim Trailing Space from a string
	std::string&
		trim(std::string& s);

	// Strip Specified Characters from a string's Tails
	std::string&
		strip(std::string& s, std::string const& chars);

	// Strip Specified Characters from a string's Right Tail
	std::string&
		rstrip(std::string& s, std::string const& chars);

	// Strip Space from a string's Tails
	std::string&
		strip(std::string& s);

	// Strip Space from a string's Right Tail
	std::string&
		rstrip(std::string& s);

	// Pad a string to a Specified Length
	inline
		std::string&
		pad(std::string& s, std::string::size_type const len)
	{
		std::string::size_type const s_len(s.length());
		if (s_len < len) s.append(len - s_len, ' '); // Pad
		return s;
	}

	// Pare a string to a Specified Length
	inline
		std::string&
		pare(std::string& s, std::string::size_type const len)
	{
		if (s.length() > len) s.erase(len); // Pare
		return s;
	}

	// Size a string to a Specified Length
	std::string&
		size(std::string& s, std::string::size_type const len);

	// Center a string wrt its Whitespace
	std::string&
		center(std::string& s);

	// Center a string with a Specified Length
	std::string&
		center(std::string& s, std::string::size_type const len);

	// Remove Repeat Characters from a Possibly Unsorted string Preserving Order
	std::string&
		unique(std::string& s);

	// Substring Replaced in a string
	std::string&
		replace(std::string& s, std::string_view const a, std::string_view const b);

	// Wrap in Double Quotes
	inline
		std::string&
		quote(std::string& s)
	{
		s = '"' + s + '"';
		return s;
	}

	// Overlay a string with Another string, Expanding Size as Needed
	std::string&
		overlay(std::string& s, std::string_view const t, std::string::size_type const pos = 0);

	// Generator /////

	// Blank string of Specified Length
	inline
		std::string
		blank(std::string::size_type const len)
	{
		return std::string(len, ' ');
	}

	// Uppercased Copy of a string
	std::string
		uppercased(std::string_view const s);

	// Left-Justified Copy of a string
	std::string
		ljustified(std::string_view const s);

	// Right-Justified Copy of a string
	std::string
		rjustified(std::string_view const s);

	// Trailing Space Trimmed Copy of a string
	std::string
		trimmed(std::string_view const s);

	// Trailing Whitespace Trimmed Copy of a string
	std::string
		trimmed_whitespace(std::string_view const s);

	// Specified Characters Stripped from a string's Tails Copy of a string
	std::string
		stripped(std::string_view const s, std::string_view const chars);

	// Space Stripped from a string's Tails Copy of a string
	std::string
		stripped(std::string_view const s);

	// Whitespace Stripped from a string's Tails Copy of a string
	std::string
		stripped_whitespace(std::string_view const s);

	// Sized to a Specified Length Copy of a string
	std::string
		sized(std::string_view const s, std::string::size_type const len);

	// Centered in a string of Specified Length Copy of a string
	std::string
		centered(std::string_view const s, std::string::size_type const len);

	// Centered wrt Whitespace Copy of a string
	inline
		std::string
		centered(std::string_view const s)
	{
		return centered(stripped_whitespace(s), s.length());
	}

	// Substring Replaced Copy of a string
	inline
		std::string
		replaced(std::string_view const s, std::string_view const a, std::string_view const b)
	{
		std::string r(s);
		replace(r, a, b);
		return r;
	}

	// Overlayed string with Another string, Expanding Size as Needed
	std::string
		overlayed(std::string_view const s, std::string_view const t, std::string::size_type const pos = 0);

	// Repeated Copies
	std::string
		repeated(std::string_view const s, int const n);

	// Repeated Copies
	std::string
		repeat(std::string_view const s, int const n);

	// Space-Free Head Copy of a string
	std::string
		head(std::string_view const s);

	// Concatenation: Non-template to Support Conversions
	inline
		std::string
		operator +(std::string const& s, std::string const& t)
	{
		return std::string(s) += t;
	}

	// Concatenation: Non-template to Support Conversions
	inline
		std::string
		operator +(char const* const s, std::string const& t)
	{
		return std::string(s) += t;
	}

	// Concatenation: Non-template to Support Conversions
	inline
		std::string
		operator +(std::string const& s, char const* const t)
	{
		return s + std::string(t);
	}

	// Conversion To std::string

} // ObjexxFCL

#endif // ObjexxFCL_string_functions_hh_INCLUDED
