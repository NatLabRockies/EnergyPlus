#ifndef ObjexxFCL_time_hh_INCLUDED
#define ObjexxFCL_time_hh_INCLUDED

// Time and Date Functions
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
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// C++ Headers
#include <string>

namespace ObjexxFCL {

// Current Date and Time
void
date_and_time(
 Optional< std::string > date = _,
 Optional< std::string > time = _,
 Optional< std::string > zone = _,
 Optional< Array1D< int > > values = _
);

} // ObjexxFCL

#endif // ObjexxFCL_time_hh_INCLUDED
