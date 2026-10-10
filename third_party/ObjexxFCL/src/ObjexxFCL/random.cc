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
#include <ObjexxFCL/random.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <ctime>
#include <random>
#include <vector>

namespace ObjexxFCL {

namespace { // Internal shared global
std::default_random_engine random_generator;
}

// Random double on [0,1)
void
RANDOM_NUMBER( double & harvest )
{
	static std::uniform_real_distribution< double > distribution( 0.0, 1.0 );
	harvest = distribution( random_generator );
}

// Random Seed Interface
void
RANDOM_SEED(
 Optional< int > size,
 Optional< Array1D< int > const > put,
 Optional< Array1D< int > > get
)
{
	static std::vector< int > seed_vals{ int( std::time( NULL ) ), int( std::time( NULL ) ) }; // C++ doesn't provide access to the seed values so we cache them here
	if ( size.present() ) {
		assert( ( ! put.present() ) && ( ! get.present() ) ); // At most one arg allowed
		size = static_cast< int >( seed_vals.size() );
	} else if ( put.present() ) {
		assert( ! get.present() ); // At most one arg allowed
		seed_vals.clear();
		for ( BArray::size_type i = 0, e = put().size(); i < e; ++i ) seed_vals.push_back( put()[ i ] );
		std::seed_seq seed_val_seq( seed_vals.begin(), seed_vals.end() );
		random_generator.seed( seed_val_seq ); // Not clear how to know how many seed values the generator is using
	} else if ( get.present() ) {
		get = 0; // In case it has more elements than seed_vals
		for ( std::vector< int >::size_type i = 0, e = std::min( seed_vals.size(), get().size() ); i < e; ++i ) get()[ i ] = seed_vals[ i ];
	} else { // No arguments
		seed_vals.assign( { int( std::time( NULL ) ), int( std::time( NULL ) ) } );
		std::seed_seq seed_val_seq( seed_vals.begin(), seed_vals.end() );
		random_generator.seed( seed_val_seq ); // Not clear how to know how many seed values the generator is using
	}
}

} // ObjexxFCL
