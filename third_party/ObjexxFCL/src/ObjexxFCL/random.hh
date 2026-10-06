#ifndef ObjexxFCL_random_hh_INCLUDED
#define ObjexxFCL_random_hh_INCLUDED

// Random Number Functions
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

namespace ObjexxFCL {

// Random double on [0,1)
void
RANDOM_NUMBER( double & harvest );

// Random Seed Interface
void
RANDOM_SEED(
 Optional< int > size = _,
 Optional< Array1D< int > const > put = _,
 Optional< Array1D< int > > get = _
);

} // ObjexxFCL

#endif // ObjexxFCL_random_hh_INCLUDED
