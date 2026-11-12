#ifndef ObjexxFCL_BArray_hh_INCLUDED
#define ObjexxFCL_BArray_hh_INCLUDED

// BArray: Array Abstract Base Class
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
#include <ObjexxFCL/IndexRange.hh>

namespace ObjexxFCL {

// BArray: Array Abstract Base Class
class BArray
{

public: // Types

	typedef  IndexRange  IR;

	// STL style
	typedef  std::size_t  size_type;
	typedef  std::ptrdiff_t  difference_type;

	// C++ style
	typedef  std::size_t  Size;
	typedef  std::ptrdiff_t  Difference;

protected: // Creation

	// Default Constructor
	constexpr BArray() = default;

	// Copy Constructor
	constexpr BArray( BArray const & ) = default;

	// Move Constructor
	constexpr BArray( BArray && ) noexcept = default;

public: // Creation

	// Destructor
	virtual
	~BArray() = default;

protected: // Assignment

	// Copy Assignment
	BArray &
	operator =( BArray const & ) = default;

	// Move Assignment
	BArray &
	operator =( BArray && ) noexcept = default;

public: // Static Data

	static constexpr size_type const npos = static_cast<size_type>(-1); // Unbounded "size"
	static constexpr size_type const max_size = static_cast<size_type>(-1) - static_cast<size_type>(1); // Max array size

}; // BArray

} // ObjexxFCL

#endif // ObjexxFCL_BArray_hh_INCLUDED
