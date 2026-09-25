// ObjexxFCL::Array1 Unit Tests
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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244) // Suppress conversion warnings: Intentional narrowing assignments present
#endif

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1.all.hh>
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Vector2.hh>
#include <ObjexxFCL/Vector3.hh>
#include "ObjexxFCL.unit.hh"

// C++ Headers
#include <array>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace ObjexxFCL;
typedef  IndexRange  IR;

TEST( Array1Test, ConstructionEmpty )
{
	Array1D_int v;
	EXPECT_EQ( 0u, v.size() );
	EXPECT_EQ( 0u, v.size1() );
	EXPECT_EQ( 1, v.l() );
	EXPECT_EQ( 1, v.l1() );
	EXPECT_EQ( 0, v.u() );
	EXPECT_EQ( 0, v.u1() );
	EXPECT_EQ( Array1D_int::IR(), v.I() );
	EXPECT_EQ( Array1D_int::IR(), v.I1() );
}

TEST( Array1Test, AllocateZero )
{
	Array1D_int i;
	EXPECT_FALSE( i.allocated() );
	EXPECT_FALSE( allocated( i ) );
	i.allocate( 0 );
	EXPECT_TRUE( i.allocated() );
	EXPECT_TRUE( allocated( i ) );
	EXPECT_EQ( 0u, i.size() );
	EXPECT_EQ( 0u, size( i ) );
}

TEST( Array1Test, ConstructionCopyEmpty )
{
	Array1D_int v;
	Array1D_int c( v );
	EXPECT_EQ( v.size(), c.size() );
	EXPECT_EQ( v.size1(), c.size1() );
	EXPECT_EQ( v.l(),  c.l() );
	EXPECT_EQ( v.l1(), c.l1() );
	EXPECT_EQ( v.u(),  c.u() );
	EXPECT_EQ( v.u1(), c.u1() );
	EXPECT_EQ( v.I(),  c.I() );
	EXPECT_EQ( v.I1(), c.I1() );
}

TEST( Array1Test, ConstructionUninitialized )
{
	Array1D_int v( 22 );
	EXPECT_EQ( 22u, v.size() );
	EXPECT_EQ( 1, v.l() );
	EXPECT_EQ( 1, lbound( v, 1 ) );
	EXPECT_EQ( 22, v.u() );
	EXPECT_EQ( 22, ubound( v, 1 ) );
}

TEST( Array1Test, ConstructionValueInitialized )
{
	Array1D_int v( 22, 33 );
	EXPECT_EQ( 22u, v.size() );
	EXPECT_EQ( 1, v.l() );
	EXPECT_EQ( 22, v.u() );
	for ( int i = 1; i <= 22; ++i ) {
		EXPECT_EQ( 33, v( i ) );
	}
}

TEST( Array1Test, ConstructionInitializerListIndexRange )
{
//	Array1D_double r( Array1D_double::IR( { 1, 3 } ) ); // Unambiguous
	Array1D_double r{ 1, 3 }; // Calls the initializer_list constructor but special code in that treats it as an IndexRange
	EXPECT_EQ( 1, r.l1() );
	EXPECT_EQ( 3, r.u1() );
}

TEST( Array1Test, ConstructionInitializerListOnlyInt )
{
	Array1D_double r{ 1, 2, 3 };
	EXPECT_EQ( 1, r.l1() );
	EXPECT_EQ( 3, r.u1() );
	EXPECT_EQ( 1.0, r( 1 ) );
	EXPECT_EQ( 2.0, r( 2 ) );
	EXPECT_EQ( 3.0, r( 3 ) );
	int v( 0 );
	for ( auto const e : r ) {
		EXPECT_EQ( ++v, e );
	}
}

TEST( Array1Test, ConstructionInitializerListOnlyUnsigned )
{
	Array1D_double r{ 1u, 2u, 3u };
	EXPECT_EQ( 1, r.l1() );
	EXPECT_EQ( 3, r.u1() );
	EXPECT_EQ( 1.0, r( 1 ) );
	EXPECT_EQ( 2.0, r( 2 ) );
	EXPECT_EQ( 3.0, r( 3 ) );
}

TEST( Array1Test, ConstructionInitializerListOnlyDouble )
{
	Array1D_double r{ 1.0, 2.0, 3.0 };
	EXPECT_EQ( 1, r.l1() );
	EXPECT_EQ( 3, r.u1() );
	EXPECT_EQ( 1.0, r( 1 ) );
	EXPECT_EQ( 2.0, r( 2 ) );
	EXPECT_EQ( 3.0, r( 3 ) );
}

TEST( Array1Test, ConstructionInitializerListOnlyString )
{
	Array1D_string r{ "Food", "Hat", "Eggs" };
	EXPECT_EQ( 1, r.l1() );
	EXPECT_EQ( 3, r.u1() );
	EXPECT_EQ( 4u, r( 1 ).length() );
	EXPECT_EQ( 3u, r( 2 ).length() );
	EXPECT_EQ( 4u, r( 3 ).length() );
	EXPECT_EQ( "Food", r( 1 ) );
	EXPECT_EQ( "Hat", r( 2 ) );
	EXPECT_EQ( "Eggs", r( 3 ) );
}

TEST( Array1Test, ConstructionStdArray )
{
	Array1D_int v( std::array< int, 3 >{ { 11, 22, 33 } } );
	EXPECT_EQ( 3u, v.size() );
	EXPECT_EQ( 3u, v.size1() );
	EXPECT_EQ( 1, v.l() );
	EXPECT_EQ( 1, v.l1() );
	EXPECT_EQ( 3, v.u() );
	EXPECT_EQ( 3, v.u1() );
	EXPECT_EQ( 11, v( 1 ) );
	EXPECT_EQ( 22, v( 2 ) );
	EXPECT_EQ( 33, v( 3 ) );
}

TEST( Array1Test, ConstructionStdVector )
{
	Array1D_int v( std::vector< int >{ 11, 22, 33 } );
	EXPECT_EQ( 3u, v.size() );
	EXPECT_EQ( 3u, v.size1() );
	EXPECT_EQ( 1, v.l() );
	EXPECT_EQ( 1, v.l1() );
	EXPECT_EQ( 3, v.u() );
	EXPECT_EQ( 3, v.u1() );
	EXPECT_EQ( Array1D_int::IR( 1, 3 ), v.I() );
	EXPECT_EQ( Array1D_int::IR( 1, 3 ), v.I1() );
	EXPECT_EQ( 11, v( 1 ) );
	EXPECT_EQ( 22, v( 2 ) );
	EXPECT_EQ( 33, v( 3 ) );
}

TEST( Array1Test, ConstructionVector2 )
{
	Array1D_int v( Vector2<int>{ { 11, 22 } } );
	EXPECT_EQ( 2u, v.size() );
	EXPECT_EQ( 2u, v.size1() );
	EXPECT_EQ( 1, v.l() );
	EXPECT_EQ( 1, v.l1() );
	EXPECT_EQ( 2, v.u() );
	EXPECT_EQ( 2, v.u1() );
	EXPECT_EQ( 11, v( 1 ) );
	EXPECT_EQ( 22, v( 2 ) );
}

TEST( Array1Test, ConstructionVector3 )
{
	Array1D_int v( Vector3<int>{ { 11, 22, 33 } } );
	EXPECT_EQ( 3u, v.size() );
	EXPECT_EQ( 3u, v.size1() );
	EXPECT_EQ( 1, v.l() );
	EXPECT_EQ( 1, v.l1() );
	EXPECT_EQ( 3, v.u() );
	EXPECT_EQ( 3, v.u1() );
	EXPECT_EQ( 11, v( 1 ) );
	EXPECT_EQ( 22, v( 2 ) );
	EXPECT_EQ( 33, v( 3 ) );
}

TEST( Array1Test, ConstructionIterator )
{
	std::vector< int > v{ { 11, 22, 33 } };
	Array1D_int a( v.begin(), v.end() );
	EXPECT_EQ( 3u, a.size() );
	EXPECT_EQ( 3u, a.size1() );
	EXPECT_EQ( 1, a.l() );
	EXPECT_EQ( 1, a.l1() );
	EXPECT_EQ( 3, a.u() );
	EXPECT_EQ( 3, a.u1() );
	EXPECT_EQ( Array1D_int::IR( 1, 3 ), a.I() );
	EXPECT_EQ( Array1D_int::IR( 1, 3 ), a.I1() );
	EXPECT_EQ( 11, a( 1 ) );
	EXPECT_EQ( 22, a( 2 ) );
	EXPECT_EQ( 33, a( 3 ) );
}

static void initializer_function( Array1D_string & a )
{
	a( 1 ) = "This";
	a( 2 ) = "is";
	a( 3 ) = "a";
	a( 4 ) = "string";
}

template< typename T >
static void initializer_function_template( Array1D< T > & a )
{
	a( 1 ) = T( 1 );
	a( 2 ) = T( 2 );
	a( 3 ) = T( 3 );
	a( 4 ) = T( 4 );
}

TEST( Array1Test, ConstructionInitializerFunction )
{
	Array1D_string r( 4, { "This", "is", "a", "stub" } );
	initializer_function( r );
	EXPECT_EQ( "This", r( 1 ) );
	EXPECT_EQ( "is", r( 2 ) );
	EXPECT_EQ( "a", r( 3 ) );
	EXPECT_EQ( "string", r( 4 ) );

	Array1D_string const cr( 4, initializer_function );
	EXPECT_EQ( "This", cr( 1 ) );
	EXPECT_EQ( "is", cr( 2 ) );
	EXPECT_EQ( "a", cr( 3 ) );
	EXPECT_EQ( "string", cr( 4 ) );

	Array1D_double const tr( 4, initializer_function_template< double > );
	EXPECT_EQ( 1.0, tr( 1 ) );
	EXPECT_EQ( 2.0, tr( 2 ) );
	EXPECT_EQ( 3.0, tr( 3 ) );
	EXPECT_EQ( 4.0, tr( 4 ) );
}

TEST( Array1Test, ConstructionIndexRange )
{
	Array1D_int r( IR( 1, 5 ), 33 );
	EXPECT_EQ( 1, r.l() );
	EXPECT_EQ( 5, r.u() );
	for ( int i = 1; i <= 5; ++i ) {
		EXPECT_EQ( 33, r( i ) );
		EXPECT_EQ( 33, r[ i - 1 ] );
	}
	Array1A_int a( r, IR( 1, 3 ) );
	EXPECT_EQ( 1, a.l() );
	EXPECT_EQ( 3, a.u() );
	EXPECT_EQ( 33, a[ 0 ] );
	EXPECT_EQ( 33, a[ 2 ] );
}

TEST( Array1Test, ConstructionIndexRangeList )
{
	Array1D_int r( {1,5}, 33 );
	EXPECT_EQ( 1, r.l() );
	EXPECT_EQ( 5, r.u() );
	for ( int i = 1; i <= 5; ++i ) {
		EXPECT_EQ( 33, r( i ) );
		EXPECT_EQ( 33, r[ i - 1 ] );
	}
}

TEST( Array1Test, ConstructionString )
{
	Array1D_string r( 3, std::string( 3, ' ' ) );
	EXPECT_EQ( 1, r.l1() );
	EXPECT_EQ( 3, r.u1() );
	EXPECT_EQ( "   ", r( 1 ) );
}

TEST( Array1Test, ConstructionStringInitializerList )
{
	Array1D_string r( 3, { "Food", "Hat", "Eggs" } );
	EXPECT_EQ( 1, r.l1() );
	EXPECT_EQ( 3, r.u1() );
	EXPECT_EQ( 4u, r( 1 ).length() );
	EXPECT_EQ( 3u, r( 2 ).length() );
	EXPECT_EQ( 4u, r( 3 ).length() );
	EXPECT_EQ( "Food", r( 1 ) );
	EXPECT_EQ( "Hat", r( 2 ) );
	EXPECT_EQ( "Eggs", r( 3 ) );
}

TEST( Array1Test, ConstructionIndexRangeInitializerList )
{
	Array1D_int r( IR( -1, 1 ), { 1, 2, 3 } );
	EXPECT_EQ( -1, r.l() );
	EXPECT_EQ( 1, r.u() );
	for ( int i = -1; i <= 1; ++i ) {
		EXPECT_EQ( i + 2, r( i ) );
	}
}

TEST( Array1Test, ConstructionIndexRangeInitializerArray )
{
	Array1D_int r( {-1,1}, Array1D_int( 3, 33 ) );
	EXPECT_EQ( -1, r.l() );
	EXPECT_EQ( 1, r.u() );
	EXPECT_EQ( 33, r( -1 ) );
	EXPECT_EQ( 33, r( 0 ) );
	EXPECT_EQ( 33, r( 1 ) );
}

TEST( Array1Test, ConstructionRange )
{
	Array1D_int c( 3 );
	c( 1 ) = 11;
	c( 2 ) = 22;
	c( 3 ) = 33;
	Array1D_int r1( Array1D_int::range( c ) );
	EXPECT_EQ( 3u, r1.size() );
	EXPECT_EQ( 3u, r1.size1() );
	EXPECT_EQ( 1, r1.l() );
	EXPECT_EQ( 1, r1.l1() );
	EXPECT_EQ( 3, r1.u() );
	EXPECT_EQ( 3, r1.u1() );
	// Values remain uninitialized
	Array1D_int r2( Array1D_int::range( c, 17 ) );
	EXPECT_EQ( 3u, r2.size() );
	EXPECT_EQ( 3u, r2.size1() );
	EXPECT_EQ( 1, r2.l() );
	EXPECT_EQ( 1, r2.l1() );
	EXPECT_EQ( 3, r2.u() );
	EXPECT_EQ( 3, r2.u1() );
	EXPECT_EQ( 17, r2( 1 ) );
	EXPECT_EQ( 17, r2( 2 ) );
	EXPECT_EQ( 17, r2( 3 ) );
}

TEST( Array1Test, ConstructionShape )
{
	Array1D_int c( 3 );
	c( 1 ) = 11;
	c( 2 ) = 22;
	c( 3 ) = 33;
	Array1D_int r1( Array1D_int::shape( c ) );
	EXPECT_EQ( 3u, r1.size() );
	EXPECT_EQ( 3u, r1.size1() );
	EXPECT_EQ( 1, r1.l() );
	EXPECT_EQ( 1, r1.l1() );
	EXPECT_EQ( 3, r1.u() );
	EXPECT_EQ( 3, r1.u1() );
	// Values remain uninitialized
	Array1D_int r2( Array1D_int::shape( c, 17 ) );
	EXPECT_EQ( 3u, r2.size() );
	EXPECT_EQ( 3u, r2.size1() );
	EXPECT_EQ( 1, r2.l() );
	EXPECT_EQ( 1, r2.l1() );
	EXPECT_EQ( 3, r2.u() );
	EXPECT_EQ( 3, r2.u1() );
	EXPECT_EQ( 17, r2( 1 ) );
	EXPECT_EQ( 17, r2( 2 ) );
	EXPECT_EQ( 17, r2( 3 ) );
}

TEST( Array1Test, ConstructionOneBased )
{
	Array1D_int c( 3 );
	c( 1 ) = 11;
	c( 2 ) = 22;
	c( 3 ) = 33;
	Array1D_int r1( Array1D_int::one_based( c ) );
	EXPECT_EQ( 3u, r1.size() );
	EXPECT_EQ( 3u, r1.size1() );
	EXPECT_EQ( 1, r1.l() );
	EXPECT_EQ( 1, r1.l1() );
	EXPECT_EQ( 3, r1.u() );
	EXPECT_EQ( 3, r1.u1() );
	// Values remain uninitialized
}

TEST( Array1Test, ConstructionOneBasedInitializerList )
{
	Array1D_int c( 3 );
	c( 1 ) = 11;
	c( 2 ) = 22;
	c( 3 ) = 33;
	Array1D_int r( Array1D_int::one_based( { 11, 22, 33 } ) );
	EXPECT_EQ( 3u, r.size() );
	EXPECT_EQ( 3u, r.size1() );
	EXPECT_EQ( 1, r.l() );
	EXPECT_EQ( 1, r.l1() );
	EXPECT_EQ( 3, r.u() );
	EXPECT_EQ( 3, r.u1() );
	EXPECT_EQ( 11, r( 1 ) );
	EXPECT_EQ( 22, r( 2 ) );
	EXPECT_EQ( 33, r( 3 ) );
}

TEST( Array1Test, AssignmentCopy )
{
	Array1D_double v( 22, 55.5 );
	Array1D_double const w( 13, 6.789 );
	v = w;
	EXPECT_TRUE( eq( w, v ) );
	v = 45.5;
	EXPECT_EQ( 13u, v.size() );
	EXPECT_TRUE( eq( v, 45.5 ) );
}

TEST( Array1Test, AssignmentMove )
{
	Array1D_double v( 22, 55.5 );
	v = Array1D_double( 13, 6.75 );
	EXPECT_EQ( 13u, v.size() );
	EXPECT_TRUE( eq( v, 6.75 ) );
	Array1D_double w;
	w = std::move( v );
	EXPECT_EQ( 0u, v.size() );
	EXPECT_EQ( 13u, w.size() );
	EXPECT_TRUE( eq( w, 6.75 ) );
}

TEST( Array1Test, ArgConstruct )
{
	Array1D_int u( 10, 22 );
	Array1A_int a( u );
	EXPECT_EQ( u.I(), a.I() );
	EXPECT_EQ( u( 3 ), a( 3 ) );
	EXPECT_TRUE( eq( Array1D_int( 10, 22 ), a ) );
	++a( 3 );
	EXPECT_EQ( u( 3 ), 23 );
	EXPECT_EQ( u( 3 ), a( 3 ) );
}

TEST( Array1Test, ConstArgConstruct )
{
	Array1D_int const u( 10, 22 );
	Array1A_int const a( u );
	EXPECT_EQ( u.I(), a.I() );
	EXPECT_EQ( u( 3 ), a( 3 ) );
	EXPECT_TRUE( eq( Array1D_int( 10, 22 ), a ) );
}

TEST( Array1Test, Index )
{
	Array1D_int A( 3, 6 );
	Array1D_int const C( 3, 6 );

	EXPECT_EQ( 2u, A.index( 3 ) );
	EXPECT_EQ( 5u, A.index( 6 ) );
	EXPECT_EQ( 2u, C.index( 3 ) );
	EXPECT_EQ( 5u, C.index( 6 ) );

	EXPECT_EQ( 1u, A.index( 2 ) );
}

TEST( Array1Test, OperatorBrackets )
{
	Array1D_int A( 3 );
	A[ 0 ] = 11;
	A[ 1 ] = 22;
	A[ 2 ] = 33;
	EXPECT_EQ( 11, A[ 0 ] );
	EXPECT_EQ( 22, A[ 1 ] );
	EXPECT_EQ( 33, A[ 2 ] );

	Array1D_int const C( 3, { 11, 22, 33 } );
	EXPECT_EQ( 11, C[ 0 ] );
	EXPECT_EQ( 22, C[ 1 ] );
	EXPECT_EQ( 33, C[ 2 ] );
}

TEST( Array1Test, Contains )
{
	Array1D_int A( { 11, 22, 33 } );

	EXPECT_TRUE( A.contains( 1 ) && A.contains( 2 ) && A.contains( 3 ) );
	EXPECT_FALSE( ! A.contains( 11 ) && A.contains( 22 ) && A.contains( 33 ) );
	EXPECT_FALSE( A.contains( 4 ) );
	EXPECT_FALSE( A.contains( -1 ) );
}

TEST( Array1Test, AllocateDeallocate )
{
	Array1D_double A1;
	EXPECT_EQ( 0u, A1.size() );
	EXPECT_EQ( 1, A1.l() );
	EXPECT_EQ( 0, A1.u() );
	EXPECT_FALSE( A1.allocated() );
	A1.allocate( 3 );
	EXPECT_EQ( 3u, A1.size() );
	EXPECT_EQ( 1, A1.l() );
	EXPECT_EQ( 3, A1.u() );
	EXPECT_TRUE( A1.allocated() );
	A1.deallocate();
	EXPECT_EQ( 0u, A1.size() );
	EXPECT_EQ( 1, A1.l() );
	EXPECT_EQ( 0, A1.u() );
	EXPECT_FALSE( allocated( A1 ) );

	Array1D_double A2( { 1.1, 2.2, 3.3 } );
	EXPECT_EQ( 3u, A2.size() );
	EXPECT_EQ( 1, A2.l() );
	EXPECT_EQ( 3, A2.u() );
	EXPECT_TRUE( allocated( A2 ) );
	deallocate( A2 );
	EXPECT_EQ( 0u, A1.size() );
	EXPECT_EQ( 1, A1.l() );
	EXPECT_EQ( 0, A1.u() );
	EXPECT_FALSE( allocated( A2 ) );
}

static void dimension_initializer_function( Array1D_int & A )
{
	for ( int i = A.l(); i <= A.u(); ++i ) {
		A( i ) = i * 10 + i;
	}
}

TEST( Array1Test, Dimension )
{
	{
		Array1D_int A;
		EXPECT_EQ( 1, A.l() );
		EXPECT_EQ( 0, A.u() );
		EXPECT_EQ( 0u, A.size() );

		A.dimension( 9 );
		EXPECT_EQ( 1, A.l() );
		EXPECT_EQ( 9, A.u() );
		EXPECT_EQ( 9u, A.size() );
	}

	{
		Array1D_int A( 3 );
		EXPECT_EQ( 1, A.l() );
		EXPECT_EQ( 3, A.u() );
		EXPECT_EQ( 3u, A.size() );

		A.dimension( { 3, 7 } );
		EXPECT_EQ( 3, A.l() );
		EXPECT_EQ( 7, A.u() );
		EXPECT_EQ( 5u, A.size() );

		A.dimension( { 2, 4 }, 17 );
		EXPECT_EQ( 2, A.l() );
		EXPECT_EQ( 4, A.u() );
		EXPECT_EQ( 3u, A.size() );
		EXPECT_EQ( 17, A( 2 ) );
		EXPECT_EQ( 17, A( 3 ) );
		EXPECT_EQ( 17, A( 4 ) );

		A.dimension( { 1, 5 }, 42 );
		EXPECT_EQ( 1, A.l() );
		EXPECT_EQ( 5, A.u() );
		EXPECT_EQ( 5u, A.size() );
		EXPECT_EQ( 42, A( 1 ) );
		EXPECT_EQ( 42, A( 2 ) );
		EXPECT_EQ( 42, A( 3 ) );
		EXPECT_EQ( 42, A( 4 ) );
		EXPECT_EQ( 42, A( 5 ) );

		A.dimension( { 4, 6 }, dimension_initializer_function );
		EXPECT_EQ( 4, A.l() );
		EXPECT_EQ( 6, A.u() );
		EXPECT_EQ( 3u, A.size() );
		EXPECT_EQ( 44, A( 4 ) );
		EXPECT_EQ( 55, A( 5 ) );
		EXPECT_EQ( 66, A( 6 ) );

		A.dimension( Array1D_int( { 3, 7 } ) );
		EXPECT_EQ( 3, A.l() );
		EXPECT_EQ( 7, A.u() );
		EXPECT_EQ( 5u, A.size() );

		A.dimension( Array1D_int( { 2, 4 } ), 17 );
		EXPECT_EQ( 2, A.l() );
		EXPECT_EQ( 4, A.u() );
		EXPECT_EQ( 3u, A.size() );
		EXPECT_EQ( 17, A( 2 ) );
		EXPECT_EQ( 17, A( 3 ) );
		EXPECT_EQ( 17, A( 4 ) );

		A.dimension( Array1D_int( { 4, 6 } ), dimension_initializer_function );
		EXPECT_EQ( 4, A.l() );
		EXPECT_EQ( 6, A.u() );
		EXPECT_EQ( 3u, A.size() );
		EXPECT_EQ( 44, A( 4 ) );
		EXPECT_EQ( 55, A( 5 ) );
		EXPECT_EQ( 66, A( 6 ) );
	}
}

TEST( Array1Test, Redimension )
{
	Array1D_int A( 5, { 11, 22, 33, 44, 55 } );

	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 5, A.u() );
	EXPECT_EQ( 5u, A.size() );
	EXPECT_EQ( 11, A( 1 ) );
	EXPECT_EQ( 22, A( 2 ) );
	EXPECT_EQ( 33, A( 3 ) );
	EXPECT_EQ( 44, A( 4 ) );
	EXPECT_EQ( 55, A( 5 ) );

	A.redimension( { 3, 7 } );
	EXPECT_EQ( 3, A.l() );
	EXPECT_EQ( 7, A.u() );
	EXPECT_EQ( 5u, A.size() );
	EXPECT_EQ( 33, A( 3 ) );
	EXPECT_EQ( 44, A( 4 ) );
	EXPECT_EQ( 55, A( 5 ) );

	A.redimension( { 0, 4 }, 17 );
	EXPECT_EQ( 0, A.l() );
	EXPECT_EQ( 4, A.u() );
	EXPECT_EQ( 5u, A.size() );
	EXPECT_EQ( 17, A( 0 ) );
	EXPECT_EQ( 17, A( 1 ) );
	EXPECT_EQ( 17, A( 2 ) );
	EXPECT_EQ( 33, A( 3 ) );
	EXPECT_EQ( 44, A( 4 ) );

	Array1D_int B( 5, { 11, 22, 33, 44, 55 } );

	B.redimension( Array1D_int( { 3, 7 } ) );
	EXPECT_EQ( 3, B.l() );
	EXPECT_EQ( 7, B.u() );
	EXPECT_EQ( 5u, B.size() );
	EXPECT_EQ( 33, B( 3 ) );
	EXPECT_EQ( 44, B( 4 ) );
	EXPECT_EQ( 55, B( 5 ) );

	B.redimension( Array1D_int( { 0, 4 } ), 17 );
	EXPECT_EQ( 0, B.l() );
	EXPECT_EQ( 4, B.u() );
	EXPECT_EQ( 5u, B.size() );
	EXPECT_EQ( 17, B( 0 ) );
	EXPECT_EQ( 17, B( 1 ) );
	EXPECT_EQ( 17, B( 2 ) );
	EXPECT_EQ( 33, B( 3 ) );
	EXPECT_EQ( 44, B( 4 ) );

	{
		Array1D_int A( 5, 1 );
		A.redimension( { 1, 5 }, 2 );
		EXPECT_EQ( 1, A.l() );
		EXPECT_EQ( 5, A.u() );
		EXPECT_TRUE( eq( A, 1 ) );
	}

	{
		Array1D_int A( 5, 1 );
		A.redimension( { 2, 4 }, 2 );
		EXPECT_EQ( 2, A.l() );
		EXPECT_EQ( 4, A.u() );
		EXPECT_TRUE( eq( A, 1 ) );
	}

	{
		Array1D_int A( 5, 1 );
		A.redimension( { -2, 0 }, 2 );
		EXPECT_EQ( -2, A.l() );
		EXPECT_EQ( 0, A.u() );
		EXPECT_TRUE( eq( A, 2 ) );
	}

	{
		Array1D_int A( 5, 1 );
		A.redimension( { 7, 9 }, 2 );
		EXPECT_EQ( 7, A.l() );
		EXPECT_EQ( 9, A.u() );
		EXPECT_TRUE( eq( A, 2 ) );
	}

	{
		Array1D_int A( 2, 1 );
		A.redimension( { -1, 4 }, 2 );
		EXPECT_EQ( -1, A.l() );
		EXPECT_EQ( 4, A.u() );
		EXPECT_EQ( 2, A( -1 ) );
		EXPECT_EQ( 2, A( 0 ) );
		EXPECT_EQ( 1, A( 1 ) );
		EXPECT_EQ( 1, A( 2 ) );
		EXPECT_EQ( 2, A( 3 ) );
		EXPECT_EQ( 2, A( 4 ) );
	}

	{
		Array1D_int A( 2, 1 );
		A.redimension( { -1, 2 }, 2 );
		EXPECT_EQ( -1, A.l() );
		EXPECT_EQ( 2, A.u() );
		EXPECT_EQ( 2, A( -1 ) );
		EXPECT_EQ( 2, A( 0 ) );
		EXPECT_EQ( 1, A( 1 ) );
		EXPECT_EQ( 1, A( 2 ) );
	}

	{
		Array1D_int A( 2, 1 );
		A.redimension( { -1, 1 }, 2 );
		EXPECT_EQ( -1, A.l() );
		EXPECT_EQ( 1, A.u() );
		EXPECT_EQ( 2, A( -1 ) );
		EXPECT_EQ( 2, A( 0 ) );
		EXPECT_EQ( 1, A( 1 ) );
	}

	{
		Array1D_int A( 2, 1 );
		A.redimension( { 2, 4 }, 2 );
		EXPECT_EQ( 2, A.l() );
		EXPECT_EQ( 4, A.u() );
		EXPECT_EQ( 1, A( 2 ) );
		EXPECT_EQ( 2, A( 3 ) );
		EXPECT_EQ( 2, A( 4 ) );
	}

	{ // No moving
		Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
		A.redimension( 4 );
		EXPECT_EQ( 1, A.l() );
		EXPECT_EQ( 4, A.u() );
		EXPECT_EQ( 1, A( 1 ) );
		EXPECT_EQ( 2, A( 2 ) );
		EXPECT_EQ( 3, A( 3 ) );
		EXPECT_EQ( 4, A( 4 ) );
		A.redimension( 5, 6 );
		EXPECT_EQ( 1, A.l() );
		EXPECT_EQ( 5, A.u() );
		EXPECT_EQ( 1, A( 1 ) );
		EXPECT_EQ( 2, A( 2 ) );
		EXPECT_EQ( 3, A( 3 ) );
		EXPECT_EQ( 4, A( 4 ) );
		EXPECT_EQ( 6, A( 5 ) );
		EXPECT_EQ( 5u, A.capacity() );
		A.redimension( 6, 7 ); // Reallocates
		EXPECT_EQ( 6u, A.capacity() );
		EXPECT_EQ( 1, A.l() );
		EXPECT_EQ( 6, A.u() );
		EXPECT_EQ( 1, A( 1 ) );
		EXPECT_EQ( 2, A( 2 ) );
		EXPECT_EQ( 3, A( 3 ) );
		EXPECT_EQ( 4, A( 4 ) );
		EXPECT_EQ( 6, A( 5 ) );
		EXPECT_EQ( 7, A( 6 ) );
	}

	{ // No overlap
		Array1D_int A( { 1, 2, 3, 4, 5 } );
		A.redimension( { -5, -1 }, 3 );
		EXPECT_EQ( -5, A.l() );
		EXPECT_EQ( -1, A.u() );
		EXPECT_EQ( 3, A( -5 ) );
		EXPECT_EQ( 3, A( -4 ) );
		EXPECT_EQ( 3, A( -3 ) );
		EXPECT_EQ( 3, A( -2 ) );
		EXPECT_EQ( 3, A( -1 ) );
	}

	{ // No overlap
		Array1D_int A( { 1, 2, 3, 4, 5 } );
		A.redimension( { 11, 15 }, 3 );
		EXPECT_EQ( 11, A.l() );
		EXPECT_EQ( 15, A.u() );
		EXPECT_EQ( 3, A( 11 ) );
		EXPECT_EQ( 3, A( 12 ) );
		EXPECT_EQ( 3, A( 13 ) );
		EXPECT_EQ( 3, A( 14 ) );
		EXPECT_EQ( 3, A( 15 ) );
	}

	{ // Up 1 overlap
		Array1D_int A( { 1, 2, 3, 4, 5 } );
		A.redimension( { 5, 9 }, 3 );
		EXPECT_EQ( 5, A.l() );
		EXPECT_EQ( 9, A.u() );
		EXPECT_EQ( 5, A( 5 ) );
		EXPECT_EQ( 3, A( 6 ) );
		EXPECT_EQ( 3, A( 7 ) );
		EXPECT_EQ( 3, A( 8 ) );
		EXPECT_EQ( 3, A( 9 ) );
	}

	{ // Down 1 overlap
		Array1D_int A( { 1, 2, 3, 4, 5 } );
		A.redimension( { -3, 1 }, 3 );
		EXPECT_EQ( -3, A.l() );
		EXPECT_EQ( 1, A.u() );
		EXPECT_EQ( 3, A( -3 ) );
		EXPECT_EQ( 3, A( -2 ) );
		EXPECT_EQ( 3, A( -1 ) );
		EXPECT_EQ( 3, A( 0 ) );
		EXPECT_EQ( 1, A( 1 ) );
	}

	{ // Up 1 overlap with reallocation
		Array1D_int A( { 1, 2, 3, 4, 5 } );
		A.redimension( { 5, 10 }, 3 );
		EXPECT_EQ( 5, A.l() );
		EXPECT_EQ( 10, A.u() );
		EXPECT_EQ( 5, A( 5 ) );
		EXPECT_EQ( 3, A( 6 ) );
		EXPECT_EQ( 3, A( 7 ) );
		EXPECT_EQ( 3, A( 8 ) );
		EXPECT_EQ( 3, A( 9 ) );
		EXPECT_EQ( 3, A( 10 ) );
	}

	{ // Down 1 overlap with reallocation
		Array1D_int A( { 1, 2, 3, 4, 5 } );
		A.redimension( { -4, 1 }, 3 );
		EXPECT_EQ( -4, A.l() );
		EXPECT_EQ( 1, A.u() );
		EXPECT_EQ( 3, A( -4 ) );
		EXPECT_EQ( 3, A( -3 ) );
		EXPECT_EQ( 3, A( -2 ) );
		EXPECT_EQ( 3, A( -1 ) );
		EXPECT_EQ( 3, A( 0 ) );
		EXPECT_EQ( 1, A( 1 ) );
	}
}

TEST( Array1Test, Append )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	A.append( 6 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 6, A.u() );
	EXPECT_EQ( 6u, A.size() );
	EXPECT_EQ( 6u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	A.reserve( 7 ); // So next append doesn't reallocate
	A.append( 7 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 7, A.u() );
	EXPECT_EQ( 7u, A.size() );
	EXPECT_EQ( 7u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	EXPECT_EQ( 7, A( 7 ) );
}

TEST( Array1Test, Front_And_Back )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	EXPECT_EQ( 1, A.front() );
	EXPECT_EQ( 5, A.back() );
}

TEST( Array1Test, Push_Back_Copy )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	int const i6( 6 ), i7( 7 ), i8( 8 ), i9( 9 );
	A.push_back( i6 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 6, A.u() );
	EXPECT_EQ( 6u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	A.push_back( i7 );
	A.push_back( i8 );
	A.push_back( i9 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 9, A.u() );
	EXPECT_EQ( 9u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	EXPECT_EQ( 7, A( 7 ) );
	EXPECT_EQ( 8, A( 8 ) );
	EXPECT_EQ( 9, A( 9 ) );
}

TEST( Array1Test, Push_Back_Move )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	A.push_back( 6 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 6, A.u() );
	EXPECT_EQ( 6u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	A.push_back( 7 );
	A.push_back( 8 );
	A.push_back( 9 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 9, A.u() );
	EXPECT_EQ( 9u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	EXPECT_EQ( 7, A( 7 ) );
	EXPECT_EQ( 8, A( 8 ) );
	EXPECT_EQ( 9, A( 9 ) );
}

TEST( Array1Test, Push_Back_Empty )
{
	Array1D_int A;
	A.push_back( 1 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 1, A.u() );
	EXPECT_EQ( 1u, A.size() );
	EXPECT_EQ( 1u, A.capacity() );
	EXPECT_EQ( 1, A[ 0 ] );
	EXPECT_EQ( 1, A( 1 ) );
}

TEST( Array1Test, Push_Back_Aggregate )
{
	struct agg { int a, b; };
	Array1D< agg > A;
	A.push_back( { 11, 22 } );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 1, A.u() );
	EXPECT_EQ( 1u, A.size() );
	EXPECT_EQ( 1u, A.capacity() );
	EXPECT_EQ( 11, A[ 0 ].a );
	EXPECT_EQ( 22, A( 1 ).b );
}

TEST( Array1Test, Push_Back_SelfRef )
{
	Array1D_int A( { 1 } );
	EXPECT_EQ( 1u, A.size() );
	EXPECT_EQ( 1u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	A.push_back( A( 1 ) );
	EXPECT_EQ( 2u, A.size() );
	EXPECT_EQ( 2u, A.capacity() );
	EXPECT_EQ( 1, A( 2 ) );
}

TEST( Array1Test, Pop_Back )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	EXPECT_EQ( 5, A.back() );
	A.pop_back();
	EXPECT_EQ( 4u, A.size() );
}

TEST( Array1Test, Insert_Copy )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	int const six( 6 ), seven( 7 ), nine( 9 );
	A.insert( A.end(), six );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 6, A.u() );
	EXPECT_EQ( 6u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	A.insert( A.end(), seven );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 7, A.u() );
	EXPECT_EQ( 7u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	EXPECT_EQ( 7, A( 7 ) );
	A.insert( A.begin(), A( 7 ) ); // Self-referential
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 8, A.u() );
	EXPECT_EQ( 8u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 7, A( 1 ) );
	EXPECT_EQ( 1, A( 2 ) );
	EXPECT_EQ( 2, A( 3 ) );
	EXPECT_EQ( 6, A( 7 ) );
	EXPECT_EQ( 7, A( 8 ) );
	A.insert( A.end(), nine );
	A.insert( A.end(), nine );
	A.insert( A.end(), nine );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 11, A.u() );
	EXPECT_EQ( 11u, A.size() );
	EXPECT_EQ( 20u, A.capacity() );
	EXPECT_EQ( 7, A( 1 ) );
	EXPECT_EQ( 1, A( 2 ) );
	EXPECT_EQ( 2, A( 3 ) );
	EXPECT_EQ( 6, A( 7 ) );
	EXPECT_EQ( 7, A( 8 ) );
	EXPECT_EQ( 9, A( 9 ) );
	EXPECT_EQ( 9, A( 10 ) );
	EXPECT_EQ( 9, A( 11 ) );
}

TEST( Array1Test, Insert_Move )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	A.insert( A.end(), 6 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 6, A.u() );
	EXPECT_EQ( 6u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	A.insert( A.end(), 7 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 7, A.u() );
	EXPECT_EQ( 7u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	EXPECT_EQ( 7, A( 7 ) );
	A.insert( A.begin(), 0 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 8, A.u() );
	EXPECT_EQ( 8u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 0, A[ 0 ] );
	EXPECT_EQ( 1, A[ 1 ] );
	EXPECT_EQ( 6, A[ 6 ] );
	EXPECT_EQ( 7, A[ 7 ] );
	A.insert( A.begin() + 3, 3u, 3 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 11, A.u() );
	EXPECT_EQ( 11u, A.size() );
	EXPECT_EQ( 20u, A.capacity() );
	EXPECT_EQ( 0, A[ 0 ] );
	EXPECT_EQ( 1, A[ 1 ] );
	EXPECT_EQ( 2, A[ 2 ] );
	EXPECT_EQ( 3, A[ 3 ] );
	EXPECT_EQ( 3, A[ 4 ] );
	EXPECT_EQ( 3, A[ 5 ] );
	EXPECT_EQ( 3, A[ 6 ] );
	EXPECT_EQ( 4, A[ 7 ] );
	EXPECT_EQ( 5, A[ 8 ] );
	EXPECT_EQ( 6, A[ 9 ] );
	EXPECT_EQ( 7, A[ 10 ] );
}

TEST( Array1Test, Insert_Multiples )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	A.insert( A.end(), 3u, 6 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 8, A.u() );
	EXPECT_EQ( 8u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 6, A( 6 ) );
	EXPECT_EQ( 6, A( 7 ) );
	EXPECT_EQ( 6, A( 8 ) );
	A.insert( A.begin() + 3, 3u, 3 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 11, A.u() );
	EXPECT_EQ( 11u, A.size() );
	EXPECT_EQ( 20u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 2, A( 2 ) );
	EXPECT_EQ( 3, A( 3 ) );
	EXPECT_EQ( 3, A( 4 ) );
	EXPECT_EQ( 3, A( 5 ) );
	EXPECT_EQ( 3, A( 6 ) );
	EXPECT_EQ( 4, A( 7 ) );
	EXPECT_EQ( 5, A( 8 ) );
	EXPECT_EQ( 6, A( 9 ) );
	EXPECT_EQ( 6, A( 10 ) );
	EXPECT_EQ( 6, A( 11 ) );
	A.insert( A.begin(), 2u, A( 7 ) );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 13, A.u() );
	EXPECT_EQ( 13u, A.size() );
	EXPECT_EQ( 20u, A.capacity() );
	EXPECT_EQ( 4, A( 1 ) );
	EXPECT_EQ( 4, A( 2 ) );
	EXPECT_EQ( 1, A( 3 ) );
	EXPECT_EQ( 2, A( 4 ) );
	EXPECT_EQ( 5, A( 10 ) );
	EXPECT_EQ( 6, A( 11 ) );
	EXPECT_EQ( 6, A( 12 ) );
	EXPECT_EQ( 6, A( 13 ) );
}

TEST( Array1Test, Insert_Iterator )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	std::vector< int > const v( { 6, 7, 8 } );
	A.insert( A.end(), v.begin(), v.end() );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 8, A.u() );
	EXPECT_EQ( 8u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 2, A( 2 ) );
	EXPECT_EQ( 3, A( 3 ) );
	EXPECT_EQ( 4, A( 4 ) );
	EXPECT_EQ( 5, A( 5 ) );
	EXPECT_EQ( 6, A( 6 ) );
	EXPECT_EQ( 7, A( 7 ) );
	EXPECT_EQ( 8, A( 8 ) );
}

TEST( Array1Test, Insert_Initializer_List )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	A.insert( A.end(), { 6, 7, 8 } );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 8, A.u() );
	EXPECT_EQ( 8u, A.size() );
	EXPECT_EQ( 10u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 2, A( 2 ) );
	EXPECT_EQ( 3, A( 3 ) );
	EXPECT_EQ( 4, A( 4 ) );
	EXPECT_EQ( 5, A( 5 ) );
	EXPECT_EQ( 6, A( 6 ) );
	EXPECT_EQ( 7, A( 7 ) );
	EXPECT_EQ( 8, A( 8 ) );
}

TEST( Array1Test, Emplace )
{
	Array1D_int A( { 1 } );
	EXPECT_EQ( 1u, A.size() );
	EXPECT_EQ( 1, A( 1 ) );
	A.insert( A.end(), 2 );
	EXPECT_EQ( 2u, A.size() );
	EXPECT_EQ( 2, A( 2 ) );
}

TEST( Array1Test, Emplace_Back )
{
	Array1D_int A( { 1 } );
	EXPECT_EQ( 1u, A.size() );
	EXPECT_EQ( 1, A( 1 ) );
	A.emplace_back( 2 );
	EXPECT_EQ( 2u, A.size() );
	EXPECT_EQ( 2, A( 2 ) );
}

TEST( Array1Test, Emplace_Back_SelfRef )
{
	Array1D_int A( { 1 } );
	EXPECT_EQ( 1u, A.size() );
	EXPECT_EQ( 1u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	A.emplace_back( A( 1 ) );
	EXPECT_EQ( 2u, A.size() );
	EXPECT_EQ( 2u, A.capacity() );
	EXPECT_EQ( 1, A( 2 ) );
}

TEST( Array1Test, Erase )
{
	Array1D_int A( { 1, 2, 3 } );
	A.erase( A.begin() );
	EXPECT_EQ( 2u, A.size() );
	EXPECT_EQ( 2, A( 1 ) );
	EXPECT_EQ( 3, A( 2 ) );
}

TEST( Array1Test, Erase_Iterator )
{
	Array1D_int A( { 1, 2, 3, 4, 5, 6 } );
	A.erase( A.begin() + 1, A.begin() + 3 );
	EXPECT_EQ( 4u, A.size() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 4, A( 2 ) );
	EXPECT_EQ( 5, A( 3 ) );
	EXPECT_EQ( 6, A( 4 ) );
}

TEST( Array1Test, Reserve )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	EXPECT_EQ( 5u, A.size() );
	EXPECT_EQ( 5u, A.capacity() );
	A.reserve( 8u );
	EXPECT_EQ( 5u, A.size() );
	EXPECT_EQ( 8u, A.capacity() );
	EXPECT_EQ( 1, A( 1 ) );
	EXPECT_EQ( 2, A( 2 ) );
	EXPECT_EQ( 3, A( 3 ) );
	EXPECT_EQ( 4, A( 4 ) );
	EXPECT_EQ( 5, A( 5 ) );
	A.push_back( 6 );
	A.push_back( 7 );
	A.push_back( 8 );
	EXPECT_EQ( 8u, A.size() );
	EXPECT_EQ( 8u, A.capacity() );
	EXPECT_EQ( 6, A( 6 ) );
	EXPECT_EQ( 7, A( 7 ) );
	EXPECT_EQ( 8, A( 8 ) );
	A.push_back( 9 );
	EXPECT_EQ( 9u, A.size() );
	EXPECT_EQ( 16u, A.capacity() );
	EXPECT_EQ( 9, A( 9 ) );
}

TEST( Array1Test, ReserveAllocate )
{
	Array1D_int A;
	A.reserve( 6u );
	A.allocate( 6 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 6, A.u() );
	EXPECT_EQ( 6u, A.size() );
	EXPECT_EQ( 6u, A.capacity() );
	A.allocate( 3 );
	EXPECT_EQ( 1, A.l() );
	EXPECT_EQ( 3, A.u() );
	EXPECT_EQ( 3u, A.size() );
	EXPECT_EQ( 3u, A.capacity() );
}

TEST( Array1Test, Swap )
{
	Array1D_int A( 4, 11 );
	Array1D_int( 5, 22 ).swap( A );
	EXPECT_EQ( IR( 1, 5 ), A.I() );
	EXPECT_EQ( 5u, A.size() );
	for ( int i = A.l(); i <= A.u(); ++i ) {
		EXPECT_EQ( 22, A( i ) );
	}
}

TEST( Array1Test, Functions )
{
	Array1D_int u{ 1, 2, 3 };
	Array1D_int v{ 2, 3, 4 };
	EXPECT_EQ( 14, magnitude_squared( u ) );
	EXPECT_EQ( 3, distance_squared( u, v ) );
	EXPECT_EQ( 20, dot( u, v ) );
}

TEST( Array1Test, Dot )
{
	Array1D_int A( 3 );
	Array1D_int B( 3 );
	A( 1 ) = 4;
	A( 2 ) = 3;
	A( 3 ) = 5;
	B( 1 ) = 7;
	B( 2 ) = 5;
	B( 3 ) = 4;
	EXPECT_EQ( 63, dot( A, B ) );
	EXPECT_EQ( 63, dot( B, A ) );
	EXPECT_EQ( 63, dot_product( A, B ) );
	EXPECT_EQ( 63, dot_product( B, A ) );
}

TEST( Array1Test, EoshiftPos )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	EXPECT_TRUE( eq( Array1D_int( { 3, 4, 5, 0, 0 } ), eoshift( A, 2 ) ) );
	EXPECT_TRUE( eq( Array1D_int( { 3, 4, 5, 9, 9 } ), eoshift( A, 2, 9 ) ) );
}

TEST( Array1Test, EoshiftNeg )
{
	Array1D_int A( 5, { 1, 2, 3, 4, 5 } );
	EXPECT_TRUE( eq( Array1D_int( { 0, 0, 1, 2, 3 } ), eoshift( A, -2 ) ) );
	EXPECT_TRUE( eq( Array1D_int( { 8, 8, 1, 2, 3 } ), eoshift( A, -2, 8 ) ) );
}

TEST( Array1Test, EoshiftMoveAssignment )
{
	Array1D_int A( { 0, 4 }, { 0, 1, 2, 3, 4 } ); // Not 1-based
	A = eoshift( A, 2 ); // eoshift is 1-based but move assignment is conformable so A index ranges shouldn't change
	EXPECT_TRUE( eq( Array1D_int( { 0, 4 }, { 2, 3, 4, 0, 0 } ), A ) );
	EXPECT_TRUE( equal_dimensions( Array1D_int( { 0, 4 }, { 2, 3, 4, 0, 0 } ), A ) ); // Conformable move shouldn't change index ranges
}

TEST( Array1Test, Cross )
{
	Array1D_int A( 3, 33 ), B( 44 - A );
	EXPECT_TRUE( eq( cross( A, B ), cross_product( A, B ) ) );
}

TEST( Array1Test, ProxyConstCorrectness )
{
	Array1D_int const v( 3, 33 );
	Array1A_int p( v ); // Proxy for const array
	EXPECT_TRUE( eq( v, p ) );
}

TEST( Array1Test, Generators )
{
	Array1D_double A( 3, 22.0 ), B( 3, 11.0 );
	EXPECT_TRUE( eq( B, A / 2.0 ) );
	//EXPECT_TRUE( eq( B, A / 2 ) ); // This doesn't compile: Won't convert
}

TEST( Array1Test, Iterator )
{
	Array1D_int A{ 1, 2, 3 };
	int j( 0 );
	for ( Array1D_int::const_iterator i = A.begin(); i != A.end(); ++i ) {
		EXPECT_EQ( ++j, *i );
	}
}

TEST( Array1Test, ReverseIterator )
{
	Array1D_int A{ 1, 2, 3 };
	int j( 4 );
	for ( Array1D_int::const_reverse_iterator i = A.rbegin(); i != A.rend(); ++i ) {
		EXPECT_EQ( --j, *i );
	}
}

TEST( Array1Test, FunctionAllocateDeallocate )
{
	Array1D_double A1;
	EXPECT_FALSE( allocated( A1 ) );
	allocate( A1, 3 );
	EXPECT_TRUE( allocated( A1 ) );
	deallocate( A1 );
	EXPECT_FALSE( allocated( A1 ) );

	Array1D_double A2( { 1.0, 2.01, 3.012 } );
	EXPECT_TRUE( allocated( A2 ) );
	deallocate( A2 );
	EXPECT_FALSE( allocated( A2 ) );
}

TEST( Array1Test, FunctionAllAny )
{
	Array1D_bool A1( { true, true, true } );
	EXPECT_TRUE( all( A1 ) );
	EXPECT_TRUE( any( A1 ) );
	EXPECT_FALSE( all( ! A1 ) );
	EXPECT_FALSE( any( ! A1 ) );

	Array1D_bool A2( { false, false, false } );
	EXPECT_FALSE( all( A2 ) );
	EXPECT_FALSE( any( A2 ) );
	EXPECT_TRUE( all( ! A2 ) );
	EXPECT_TRUE( any( ! A2 ) );

	Array1D_bool A3( { false, false, true } );
	EXPECT_FALSE( all( A3 ) );
	EXPECT_TRUE( any( A3 ) );
	EXPECT_FALSE( all( ! A3 ) );
	EXPECT_TRUE( any( ! A3 ) );
}

TEST( Array1Test, FunctionAbs )
{
	Array1D_int const A( { -1, -2, -3 } );
	Array1D_int const E( { 1, 2, 3 } );
	EXPECT_TRUE( eq( E, abs( A ) ) );
}

TEST( Array1Test, FunctionNegation )
{
	Array1D_bool const A( { true, false, true } );
	Array1D_bool const E( { false, true, false } );
	EXPECT_TRUE( eq( E, !A ) );
}

TEST( Array1Test, FunctionPow )
{
	Array1D_int A( { 1, 2, 3 } );
	Array1D_int const E( { 1, 4, 9 } );
	EXPECT_TRUE( eq( E, pow( A, 2 ) ) );
}

TEST( Array1Test, FunctionSign )
{
	Array1D_int A1( { 1, 2, 3 } );
	Array1D_int const E11( { 1, 2, 3 } );
	Array1D_int const E12( { -1, -2, -3 } );
	EXPECT_TRUE( eq( E11, sign( A1, 1 ) ) );
	EXPECT_TRUE( eq( E11, sign( A1, 0 ) ) );
	EXPECT_TRUE( eq( E12, sign( A1, -1 ) ) );

	Array1D_int A2( { -1, -2, -3 } );
	Array1D_int const E21( { 1, 2, 3 } );
	Array1D_int const E22( { -1, -2, -3 } );
	EXPECT_TRUE( eq( E21, sign( A2, 1 ) ) );
	EXPECT_TRUE( eq( E21, sign( A2, 0 ) ) );
	EXPECT_TRUE( eq( E22, sign( A2, -1 ) ) );

	Array1D_int A3( { 1, -2, 3 } );
	Array1D_int const E31( { 1, -1, 1 } );
	Array1D_int const E32( { 0, 0, 0 } );
	EXPECT_TRUE( eq( E31, sign( 1,  A3 ) ) );
	EXPECT_TRUE( eq( E32, sign( 0,  A3 ) ) );
	EXPECT_TRUE( eq( E31, sign( -1, A3 ) ) );
}

TEST( Array1Test, FunctionCount )
{
	Array1D_bool A1( { true, true, true, true, true } );
	EXPECT_EQ( 5u, count( A1 ) );
	EXPECT_EQ( 5u, count( A1, 1 ) );

	Array1D_bool A2( { true, false, true, false, true } );
	EXPECT_EQ( 3u, count( A2 ) );
	EXPECT_EQ( 3u, count( A2, 1 ) );

	Array1D_bool A3( { false, false, false, false, false } );
	EXPECT_EQ( 0u, count( A3 ) );
	EXPECT_EQ( 0u, count( A3, 1 ) );
}

TEST( Array1Test, FunctionLUBound )
{
	Array1D_double A( { 1.0, 2.0, 3.0, 4.0, 5.0 } );
	Array1D_int E11( 1, { 1.0 } ); // May cause conversion warning
	Array1D_int E12( 1, { 5.0 } ); // May cause conversion warning
	EXPECT_TRUE( eq( E11, lbound( A ) ) );
	EXPECT_TRUE( eq( E12, ubound( A ) ) );
	int const E21( 1 );
	int const E22( 5 );
	EXPECT_EQ( E21, lbound( A, 1 ) );
	EXPECT_EQ( E22, ubound( A, 1 ) );
}

TEST( Array1Test, FunctionShape )
{
	Array1D_double A( { 1.0, 2.0, 3.0, 4.0, 5.0 } );
	Array1D_int E1( 1, 5 );
	EXPECT_TRUE( eq( E1, shape( A ) ) );
}

TEST( Array1Test, FunctionSize )
{
	int const N = 10;
	Array1D_double A( N );
	EXPECT_EQ( unsigned( N ), size( A ) );
	EXPECT_EQ( unsigned( N ), size( A, 1 ) );
	EXPECT_EQ( A.size(), size( A ) );
}

TEST( Array1Test, FunctionISize )
{
	int const N = 10;
	Array1D_double A( N );
	EXPECT_EQ( N, isize( A ) );
	EXPECT_EQ( N, isize( A, 1 ) );
	EXPECT_EQ( int( size( A ) ), isize( A ) );
}

TEST( Array1Test, FunctionPack )
{
	Array1D_double A1;
	Array1D_double const E1;
	EXPECT_TRUE( eq( E1, pack( A1, false ) ) );
	EXPECT_TRUE( eq( E1, pack( A1, true ) ) );

	Array1D_double A2( { 1.0, 2.0, 3.0, 4.0, 5.0 } );
	Array1D_double const E2( { 1.0, 2.0, 3.0, 4.0, 5.0 } );
	EXPECT_TRUE( eq( E1, pack( A2, false ) ) );
	EXPECT_TRUE( eq( E2, pack( A2, true ) ) );

	Array1D_double A3( { 1.0, 2.0, 3.0, 4.0, 5.0 } );
	Array1D_double const E3( { 2.0, 4.0 } );
	Array1D_bool const M( { false, true, false, true, false } );
	EXPECT_TRUE( eq( E3, pack( A3, M ) ) );
}

TEST( Array1Test, FunctionEOShift )
{
	Array1D_double A( { 1.0, 2.0, 3.0, 4.0, 5.0 } );
	EXPECT_TRUE( eq( A, eoshift( A, 0 ) ) );

	Array1D_double const E11( { 2.0, 3.0, 4.0, 5.0, 0.0 } );
	Array1D_double const E12( { 2.0, 3.0, 4.0, 5.0, 17.0 } );
	EXPECT_TRUE( eq( E11, eoshift( A, 1 ) ) );
	EXPECT_TRUE( eq( E12, eoshift( A, 1, 17.0 ) ) );
	EXPECT_TRUE( eq( E11, eoshift( A, 1, 0.0, 1 ) ) );

	Array1D_double const E21( { 3.0, 4.0, 5.0, 0.0, 0.0 } );
	Array1D_double const E22( { 3.0, 4.0, 5.0, 17.0, 17.0 } );
	EXPECT_TRUE( eq( E21, eoshift( A, 2 ) ) );
	EXPECT_TRUE( eq( E22, eoshift( A, 2, 17.0 ) ) );
	EXPECT_TRUE( eq( E21, eoshift( A, 2, 0.0, 1 ) ) );

	Array1D_double const E31( { 0.0, 1.0, 2.0, 3.0, 4.0 } );
	Array1D_double const E32( { 17.0, 1.0, 2.0, 3.0, 4.0 } );
	EXPECT_TRUE( eq( E31, eoshift( A, -1 ) ) );
	EXPECT_TRUE( eq( E32, eoshift( A, -1, 17.0 ) ) );
	EXPECT_TRUE( eq( E31, eoshift( A, -1, 0.0, 1 ) ) );
}

TEST( Array1Test, FunctionSum )
{
	Array1D_double A( { 1.0, 2.0, 3.0, 4.0, 5.0 } );
	double const E1 = 15.0;
	EXPECT_EQ( E1, sum( A ) );
	EXPECT_EQ( E1, sum( A, 1 ) );
	Array1D_bool M( { true, false, true, false, true } );
	double const E2 = 9.0;
	EXPECT_EQ( E2, sum( A, M ) );
}

TEST( Array1Test, FunctionMinVal )
{
	Array1D_int A( { -1000, -1, 0, 1, 1000 } );
	int const E = -1000;
	EXPECT_EQ( E, minval( A ) );
}

TEST( Array1Test, FunctionMaxVal )
{
	Array1D_int A( { -1000, -1, 0, 1, 1000 } );
	int const E = 1000;
	EXPECT_EQ( E, maxval( A ) );
}

TEST( Array1Test, FunctionMinLoc )
{
	Array1D_int A1( { 1, 2, 3 } );
	int const I1 = 1;
	Array1D_int const E1( 1, I1 );
	EXPECT_TRUE( eq( E1, minloc( A1 ) ) );
	EXPECT_EQ( I1, minloc( A1, 1 ) );

	Array1D_int A2( { 3, 2, 1 } );
	int const I2 = 3;
	Array1D_int const E2( 1, I2 );
	EXPECT_TRUE( eq( E2, minloc( A2 ) ) );
	EXPECT_EQ( I2, minloc( A2, 1 ) );

	Array1D_int A3( { 3, 2, 1, 2, 1 } );
	int const I3 = 3;
	Array1D_int const E3( 1, I3 );
	EXPECT_TRUE( eq( E3, minloc( A3 ) ) );
	EXPECT_EQ( I3, minloc( A3, 1 ) );

	Array1D_int A4( { 1, 2, 3, -2, -1 } );
	int const I4 = 4;
	Array1D_int const E4( 1, I4 );
	EXPECT_TRUE( eq( E4, minloc( A4 ) ) );
	EXPECT_EQ( I4, minloc( A4, 1 ) );
}
