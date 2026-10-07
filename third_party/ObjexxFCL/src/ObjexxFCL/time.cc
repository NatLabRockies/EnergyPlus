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
#include <ObjexxFCL/time.hh>

// C++ Headers
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace ObjexxFCL {

// Local Time Zone Offset from UTC in Seconds
int
time_zone_offset_seconds()
{
	std::time_t const current_time( std::time( nullptr ) );
	std::time_t const ltime( std::mktime( std::localtime( &current_time ) ) ); // Local
#if defined(_WIN32)
	std::time_t const gtime( _mkgmtime( std::localtime( &current_time ) ) ); // UTC
#elif defined(__linux__)
	std::time_t const gtime( timegm( std::localtime( &current_time ) ) ); // UTC
#else
	std::time_t const gtime( std::mktime( std::gmtime( &current_time ) ) ); // UTC
#endif
	return static_cast< int >( gtime - ltime );
}

// Current Date and Time
void
date_and_time(
 Optional< std::string > date,
 Optional< std::string > time,
 Optional< std::string > zone,
 Optional< Array1D< int > > values
)
{
	using std::chrono::system_clock;
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;
	using std::setw;
	system_clock::time_point const now( system_clock::now() );
	int const ms( static_cast< int >( duration_cast< milliseconds >( now.time_since_epoch() ).count() % 1000 ) ); // msec
	std::time_t const current_time( system_clock::to_time_t( now ) );
	std::tm const * const timeinfo( std::localtime( &current_time ) );

	int const DD( timeinfo->tm_mday ); // Day of month: 1-31
	int const MM( timeinfo->tm_mon + 1 ); // Month of year: 1-12
	int const YY( timeinfo->tm_year + 1900 ); // Year
	if ( date.present() ) {
		std::stringstream date_stream;
		date_stream << std::setfill( '0' ) << setw( 4 ) << YY << setw( 2 ) << MM << setw( 2 ) << DD;
		date = date_stream.str();
	}

	int const hh( timeinfo->tm_hour );
	int const mm( timeinfo->tm_min );
	int const ss( timeinfo->tm_sec );
	if ( time.present() ) {
		std::stringstream time_stream;
		time_stream << std::setfill( '0' ) << setw( 2 ) << hh << setw( 2 ) << mm << setw( 2 ) << ss << '.' << setw( 3 ) << ms;
		time = time_stream.str();
	}

	int const zs( time_zone_offset_seconds() );
	if ( zone.present() ) {
		int const zh( std::abs( zs ) / 3600 );
		int const zm( ( std::abs( zs ) - ( zh * 3600 ) ) / 60 );
		std::stringstream zone_stream;
		zone_stream << std::setfill( '0' ) << ( zs >= 0 ? '+' : '-' ) << setw( 2 ) << zh << setw( 2 ) << zm;
		zone = zone_stream.str();
	}

	if ( values.present() ) {
		assert( ( values().l() <= 1 ) && ( values().u() >= 8 ) );
		values()( 1 ) = YY;
		values()( 2 ) = MM;
		values()( 3 ) = DD;
		values()( 4 ) = zs / 60;
		values()( 5 ) = hh;
		values()( 6 ) = mm;
		values()( 7 ) = ss;
		values()( 8 ) = ms;
	}
}

} // ObjexxFCL
