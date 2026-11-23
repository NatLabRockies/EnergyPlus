#ifndef ObjexxFCL_TypeTraits_hh_INCLUDED
#define ObjexxFCL_TypeTraits_hh_INCLUDED

// TypeTraits: Type Traits Template
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

// C++ Headers
#include <complex>
#include <cstddef>
#include <ios>
#include <limits>

namespace ObjexxFCL {

// TypeTraits: Type Traits Template
template< typename T >
struct TypeTraits
{
	typedef  T  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return traits_type(); // Use default constructor
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 0; // Field width
	static int const iwidth = 0; // Field width
};

// TypeTraits: Type Traits char Specialization
template<>
struct TypeTraits< char >
{
	typedef  char  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return ' ';
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 1; // Field width
	static int const iwidth = 1; // Field width
};

// TypeTraits: Type Traits signed char Specialization
template<>
struct TypeTraits< signed char >
{
	typedef  signed char  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return ' ';
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 1; // Field width
	static int const iwidth = 1; // Field width
};

// TypeTraits: Type Traits unsigned char Specialization
template<>
struct TypeTraits< unsigned char >
{
	typedef  unsigned char  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return ' ';
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 1; // Field width
	static int const iwidth = 1; // Field width
};

// TypeTraits: Type Traits bool Specialization
template<>
struct TypeTraits< bool >
{
	typedef  bool  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 2; // Field width
	static int const iwidth = 2; // Field width
};

// TypeTraits: Type Traits short int Specialization
template<>
struct TypeTraits< short int >
{
	typedef  short int  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 7; // Field width
	static int const iwidth = 7; // Field width
};

// TypeTraits: Type Traits unsigned short int Specialization
template<>
struct TypeTraits< unsigned short int >
{
	typedef  unsigned short int  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 7; // Field width
	static int const iwidth = 7; // Field width
};

// TypeTraits: Type Traits int Specialization
template<>
struct TypeTraits< int >
{
	typedef  int  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 12; // Field width
	static int const iwidth = 12; // Field width
};

// TypeTraits: Type Traits unsigned int Specialization
template<>
struct TypeTraits< unsigned int >
{
	typedef  unsigned int  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 12; // Field width
	static int const iwidth = 12; // Field width
};

// TypeTraits: Type Traits long int Specialization
template<>
struct TypeTraits< long int >
{
	typedef  long int  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 23; // Field width
	static int const iwidth = 23; // Field width
};

// TypeTraits: Type Traits unsigned long int Specialization
template<>
struct TypeTraits< unsigned long int >
{
	typedef  unsigned long int  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 23; // Field width
	static int const iwidth = 23; // Field width
};

// TypeTraits: Type Traits long long int Specialization
template<>
struct TypeTraits< long long int >
{
	typedef  long long int  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 23; // Field width
	static int const iwidth = 23; // Field width
};

// TypeTraits: Type Traits unsigned long long int Specialization
template<>
struct TypeTraits< unsigned long long int >
{
	typedef  unsigned long long int  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return std::numeric_limits< traits_type >::max();
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

	static std::streamsize const precision = 0; // Precision
	static Size const width = 23; // Field width
	static int const iwidth = 23; // Field width
};

// TypeTraits: Type Traits float Specialization
template<>
struct TypeTraits< float >
{
	typedef  float  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return ( std::numeric_limits< traits_type >::has_signaling_NaN ? std::numeric_limits< traits_type >::signaling_NaN() : std::numeric_limits< traits_type >::max() );
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

#ifdef OBJEXXFCL_TYPETRAITS_EXTRA_PRECISION
	static std::streamsize const precision = 9; // Precision
#else
	static std::streamsize const precision = 8; // Precision
#endif
	static Size const width = 15; // Field width
	static int const iwidth = 15; // Field width
};

// TypeTraits: Type Traits double Specialization
template<>
struct TypeTraits< double >
{
	typedef  double  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return ( std::numeric_limits< traits_type >::has_signaling_NaN ? std::numeric_limits< traits_type >::signaling_NaN() : std::numeric_limits< traits_type >::max() );
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

#ifdef OBJEXXFCL_TYPETRAITS_EXTRA_PRECISION
	static std::streamsize const precision = 17; // Precision
#else
	static std::streamsize const precision = 16; // Precision
#endif
	static Size const width = 23; // Field width
	static int const iwidth = 23; // Field width
};

// TypeTraits: Type Traits long double Specialization
template<>
struct TypeTraits< long double >
{
	typedef  long double  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return ( std::numeric_limits< traits_type >::has_signaling_NaN ? std::numeric_limits< traits_type >::signaling_NaN() : std::numeric_limits< traits_type >::max() );
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

#ifdef OBJEXXFCL_TYPETRAITS_EXTRA_PRECISION
	static std::streamsize const precision = 34; // Precision
#else
	static std::streamsize const precision = 33; // Precision
#endif
	static Size const width = 42; // Field width
	static int const iwidth = 42; // Field width
};

// TypeTraits: Type Traits std::complex< float > Specialization
template<>
struct TypeTraits< std::complex< float > >
{
	typedef  float  value_type;
	typedef  std::complex< float >  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return traits_type( TypeTraits< value_type >::debug_value(), TypeTraits< value_type >::debug_value() );
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

#ifdef OBJEXXFCL_TYPETRAITS_EXTRA_PRECISION
	static std::streamsize const precision = 9; // Precision
#else
	static std::streamsize const precision = 8; // Precision
#endif
	static Size const width = 33; // Field width
	static int const iwidth = 33; // Field width
};

// TypeTraits: Type Traits std::complex< double > Specialization
template<>
struct TypeTraits< std::complex< double > >
{
	typedef  double  value_type;
	typedef  std::complex< double >  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return traits_type( TypeTraits< value_type >::debug_value(), TypeTraits< value_type >::debug_value() );
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

#ifdef OBJEXXFCL_TYPETRAITS_EXTRA_PRECISION
	static std::streamsize const precision = 17; // Precision
#else
	static std::streamsize const precision = 16; // Precision
#endif
	static Size const width = 49; // Field width
	static int const iwidth = 49; // Field width
};

// TypeTraits: Type Traits std::complex< long double > Specialization
template<>
struct TypeTraits< std::complex< long double > >
{
	typedef  long double  value_type;
	typedef  std::complex< long double >  traits_type;
	typedef  std::size_t  Size;

	// Initial Value
	static
	traits_type
	initial_value()
	{
		return traits_type(); // Use default constructor
	}

	// Debug Value
	static
	traits_type
	debug_value()
	{
		return traits_type( TypeTraits< value_type >::debug_value(), TypeTraits< value_type >::debug_value() );
	}

	// Initial Array Value
	static
	traits_type
	initial_array_value()
	{
#ifdef OBJEXXFCL_ARRAY_INIT_DEBUG
		return debug_value();
#else
		return initial_value();
#endif
	}

#ifdef OBJEXXFCL_TYPETRAITS_EXTRA_PRECISION
	static std::streamsize const precision = 34; // Precision
#else
	static std::streamsize const precision = 33; // Precision
#endif
	static Size const width = 83; // Field width
	static int const iwidth = 83; // Field width
};

} // ObjexxFCL

#endif // ObjexxFCL_TypeTraits_hh_INCLUDED
