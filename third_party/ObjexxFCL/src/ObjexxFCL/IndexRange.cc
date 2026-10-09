// IndexRange: Index Range Class
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

	// Static Data Member Definitions
	IndexRange::size_type const IndexRange::npos = static_cast< size_type >( -1 );
	int const IndexRange::l_min = -( static_cast< int >( ( static_cast< unsigned int >( -1 ) / 2u ) ) - 1 );
	int const IndexRange::u_max = static_cast< int >( ( static_cast< unsigned int >( -1 ) / 2u ) );

} // ObjexxFCL
