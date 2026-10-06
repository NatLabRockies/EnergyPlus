#ifndef ObjexxFCL_align_hh_INCLUDED
#define ObjexxFCL_align_hh_INCLUDED

// Alignment Support
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
#include <cassert>
#include <cstddef>
#include <cstdint>

// Auto-Selected Alignment
#if defined(__MIC__)
#define OBJEXXFCL_ALIGN_AUTO 64
#elif defined(__AVX__)
#define OBJEXXFCL_ALIGN_AUTO 32
#elif defined(__SSE__)
#define OBJEXXFCL_ALIGN_AUTO 16
#else
#define OBJEXXFCL_ALIGN_AUTO 16
#endif

// Specified Alignment
#ifdef OBJEXXFCL_ALIGN
#if OBJEXXFCL_ALIGN < OBJEXXFCL_ALIGN_AUTO
#undef OBJEXXFCL_ALIGN
#define OBJEXXFCL_ALIGN OBJEXXFCL_ALIGN_AUTO
#endif
static_assert( ( OBJEXXFCL_ALIGN & ( OBJEXXFCL_ALIGN - 1 ) ) == 0, "OBJEXXFCL_ALIGN must be a power of 2" );
#endif

#endif // ObjexxFCL_align_hh_INCLUDED
