#ifndef ObjexxFCL_member_functions_hh_INCLUDED
#define ObjexxFCL_member_functions_hh_INCLUDED

// Container Member Functions
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

// C++ Headers
#include <algorithm>
#include <limits>

namespace ObjexxFCL {

// Sum of All Members of a Container
template< class Container, typename Member >
inline
Member
sum( Container const & c, Member Container::value_type::* pmem )
{
	Member s( 0 );
	for ( typename Container::const_iterator i = c.begin(), e = c.end(); i != e; ++i ) {
		s += i->*pmem;
	}
	return s;
}

// Sum of Members of an Array at Subscripts
template< class Array, typename Member, class SubArray >
inline
Member
sum_sub( Array const & a, Member Array::value_type::* pmem, SubArray const & sub )
{
	Member s( 0 );
	for ( int i = sub.l(), e = sub.u(); i <= e; ++i ) {
		s += a( sub( i ) ).*pmem;
	}
	return s;
}

// Sum of Product of Two Members of an Array at Subscripts
template< class Array, typename Member, class SubArray >
inline
Member
sum_product_sub( Array const & a, Member Array::value_type::* pmem1, Member Array::value_type::* pmem2, SubArray const & sub )
{
	Member s( 0 );
	for ( int i = sub.l(), e = sub.u(); i <= e; ++i ) {
		auto const & a_j( a( sub( i ) ) );
		s += a_j.*pmem1 * a_j.*pmem2;
	}
	return s;
}

// Sum of Product of Elements of an Array with Members of an Array at Subscripts
template< class Array1, class Array2, typename Member, class SubArray >
inline
Member
sum_product_sub( Array1 const & a, Array2 const & b, Member Array2::value_type::* pmem, SubArray const & sub )
{
	Member s( 0 );
	for ( int i = sub.l(), e = sub.u(); i <= e; ++i ) {
		int const j( sub( i ) );
		s += a( j ) * b( j ).*pmem;
	}
	return s;
}

// Sum of Product of Members of Elements of an Array with Members of an Array at Subscripts
template< class Array1, typename OuterMember, class Array2, typename Member, class SubArray >
inline
Member
sum_product_sub( Array1 const & a, OuterMember Array1::value_type::* pmemo, Member OuterMember::* pmem1, Array2 const & b, Member Array2::value_type::* pmem2, SubArray const & sub )
{
	Member s( 0 );
	for ( int i = sub.l(), e = sub.u(); i <= e; ++i ) {
		int const j( sub( i ) );
		s += a( j ).*pmemo.*pmem1 * b( j ).*pmem2;
	}
	return s;
}

// Array of Array Members at Subscripts
template< class Array, typename Member, class SubArray >
inline
Array1D< Member >
array_sub( Array const & a, Member Array::value_type::* pmem, SubArray const & sub )
{
	Array1D< Member > r( sub.isize() );
	for ( int i = sub.l(), e = sub.u(), k = 1; i <= e; ++i, ++k ) {
		r( k ) = a( sub( i ) ).*pmem;
	}
	return r;
}

// Minimum Value of All Members of a Container
template< class Container, typename Member >
inline
Member
minval( Container const & c, Member Container::value_type::* pmem )
{
	Member v( c.begin() == c.end() ? std::numeric_limits< Member >::max() : c.begin()->*pmem );
	for ( typename Container::const_iterator i = c.begin(), e = c.end(); i != e; ++i ) {
		v = std::min( v, i->*pmem );
	}
	return v;
}

// Maximum Value of All Members of a Container
template< class Container, typename Member >
inline
Member
maxval( Container const & c, Member Container::value_type::* pmem )
{
	Member v( c.begin() == c.end() ? std::numeric_limits< Member >::lowest() : c.begin()->*pmem );
	for ( typename Container::const_iterator i = c.begin(), e = c.end(); i != e; ++i ) {
		v = std::max( v, i->*pmem );
	}
	return v;
}

} // ObjexxFCL

#endif // ObjexxFCL_member_functions_hh_INCLUDED
