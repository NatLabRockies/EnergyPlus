#ifndef ObjexxFCL_IndexRange_hh_INCLUDED
#define ObjexxFCL_IndexRange_hh_INCLUDED

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
#include <ObjexxFCL/Index.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace ObjexxFCL {

// IndexRange: Index Range Class
//
// Note:
//  Zero-size range is indicated by ( l - 1 == u ) and ( size == 0 )
//  Upper-unbounded range is indicated by ( l - 2 == u ) and ( size == npos )
//  Legal ranges have ( l - 2 <= u ) with l and u in their allowed ranges
class IndexRange
{

public: // Types

	// STL style
	typedef  std::size_t  size_type;

	// C++ style
	typedef  std::size_t  Size;

public: // Creation

	// Default Constructor
	IndexRange() :
	 l_( 1 ),
	 u_( 0 ),
	 size_( 0u )
	{}

	// Copy Constructor
	IndexRange( IndexRange const & I ) :
	 l_( I.l_ ),
	 u_( I.u_ ),
	 size_( I.size_ )
	{}

	// Upper Index Constructor
	IndexRange( int const u ) :
	 l_( 1 ),
	 u_( clean_u( u ) ),
	 size_( u_ )
	{
		assert( legal() );
	}

	// Index Range Constructor
	IndexRange( int const l, int const u ) :
	 l_( l ),
	 u_( clean_u( u ) ),
	 size_( computed_size() )
	{
		assert( legal() );
	}

	// Initializer List of Integer Constructor
	template< typename U, class = typename std::enable_if< std::is_constructible< int, U >::value >::type >
	IndexRange( std::initializer_list< U > const lu ) :
	 l_( 1 ),
	 u_( 0 ),
	 size_( 0u )
	{
		size_type const n( lu.size() );
		assert( n <= 2 );
		auto i( lu.begin() );
		switch ( n ) {
		case 0: // {}
			l_ = 1;
			u_ = 0;
			break;
		case 1: // {l}
			l_ = int( *i );
			u_ = l_ - 2; // Unbounded
			break;
		case 2: // {l,u}
			l_ = int( *i );
			u_ = clean_u( *(++i) );
			break;
		}
		size_ = computed_size();
		assert( legal() );
	}

	// Initializer List of Index Constructor
	IndexRange( std::initializer_list< Index > const lu ) :
	 l_( 1 ),
	 u_( 0 ),
	 size_( 0u )
	{
		size_type const n( lu.size() );
		assert( n <= 2 );
		auto i( lu.begin() );
		switch ( n ) {
		case 0: // {}
			l_ = 1;
			u_ = 0;
			break;
		case 1: // {l}
			l_ = i->initialized() ? int( *i ) : 1;
			u_ = l_ - 2; // Unbounded
			break;
		case 2: // {l,u}
			l_ = i->initialized() ? int( *i ) : 1;
			u_ = (++i)->initialized() ? clean_u( int( *i ) ) : l_ - 2; // Omit => Unbounded
			break;
		}
		size_ = computed_size();
		assert( legal() );
	}

	// Omit Constructor
	IndexRange( Omit const ) :
	 l_( 1 ),
	 u_( -1 ),
	 size_( npos )
	{}

	// Lower Index + Omit Constructor
	IndexRange( int const l, Omit const ) :
	 l_( l ),
	 u_( l_ - 2 ),
	 size_( npos )
	{}

	// Omit + Upper Index Constructor
	IndexRange( Omit const, int const u ) :
	 l_( u + 2 ),
	 u_( u ),
	 size_( npos )
	{
		assert( legal() );
	}

	// Omit + Omit Constructor
	IndexRange( Omit const, Omit const ) :
	 l_( 1 ),
	 u_( -1 ),
	 size_( npos )
	{}

	// Destructor
	~IndexRange()
	{}

public: // Assignment

	// Copy Assignment
	IndexRange &
	operator =( IndexRange const & I )
	{
		if ( this != &I ) {
			l_ = I.l_;
			u_ = I.u_;
			size_ = I.size_;
		}
		assert( legal() );
		return *this;
	}

	// Upper Index Assignment
	IndexRange &
	operator =( int const u )
	{
		l_ = 1;
		u_ = clean_u( u );
		size_ = u_;
		assert( legal() );
		return *this;
	}

	// Initializer List of int Assignment
	template< typename U, class = typename std::enable_if< std::is_assignable< int&, U >::value >::type >
	IndexRange &
	operator =( std::initializer_list< U > const lu )
	{
		size_type const n( lu.size() );
		assert( n <= 2 );
		auto i( lu.begin() );
		switch ( n ) {
		case 0: // {}
			l_ = 1;
			u_ = 0;
			break;
		case 1: // {l}
			l_ = int( *i );
			u_ = l_ - 2; // Unbounded
			break;
		case 2: // {l,u}
			l_ = int( *i );
			u_ = clean_u( *(++i) );
			break;
		}
		size_ = computed_size();
		assert( legal() );
		return *this;
	}

	// Initializer List of Index Assignment
	IndexRange &
	operator =( std::initializer_list< Index > const lu )
	{
		size_type const n( lu.size() );
		assert( n <= 2 );
		auto i( lu.begin() );
		switch ( n ) {
		case 0: // {}
			l_ = 1;
			u_ = 0;
			break;
		case 1: // {l}
			l_ = i->initialized() ? int( *i ) : 1;
			u_ = l_ - 2; // Unbounded
			break;
		case 2: // {l,u}
			l_ = i->initialized() ? int( *i ) : 1;
			u_ = (++i)->initialized() ? clean_u( int( *i ) ) : l_ - 2; // Omit => Unbounded
			break;
		}
		size_ = computed_size();
		assert( legal() );
		return *this;
	}

	// IndexRange Assignment
	IndexRange &
	assign( IndexRange const & I )
	{
		l_ = I.l_;
		u_ = I.u_;
		size_ = I.size_;
		assert( legal() );
		return *this;
	}

	// Upper Index Assignment
	IndexRange &
	assign( int const u )
	{
		l_ = 1;
		u_ = clean_u( u );
		size_ = u_;
		assert( legal() );
		return *this;
	}

	// Index Range Assignment
	IndexRange &
	assign( int const l, int const u )
	{
		l_ = l;
		u_ = clean_u( u );
		size_ = computed_size();
		assert( legal() );
		return *this;
	}

public: // Subscript

	// IndexRange( i ): Internal Index for i
	int
	operator ()( int const i ) const
	{
		return l_ + ( i - 1 );
	}

	// IndexRange[ i ]: Internal Index for Zero-Based i
	int
	operator []( int const i ) const
	{
		return l_ + i;
	}

public: // Predicate

	// Legal?
	bool
	legal() const
	{
		return ( ( l_ >= l_min ) && ( u_ <= u_max ) && ( l_ - 2 <= u_ ) );
	}

	// Bounded?
	bool
	bounded() const
	{
		return ( l_ - 1 <= u_ );
	}

	// Unbounded?
	bool
	unbounded() const
	{
		return ( l_ - 2 == u_ );
	}

	// Empty?
	bool
	empty() const
	{
		return ( l_ - 1 == u_ );
	}

	// Non-Empty?
	bool
	non_empty() const
	{
		return ( l_ <= u_ );
	}

	// Bounded with Positive Size?
	bool
	positive() const
	{
		return ( l_ <= u_ );
	}

	// Contains an Index?
	bool
	contains( int const i ) const
	{
		return ( ( l_ <= i ) && ( ( i <= u_ ) || ( size_ == npos ) ) );
	}

	// Contains Two Indexes?
	bool
	contains( int const i, int const j ) const
	{
		return ( contains( i ) && contains( j ) );
	}

public: // Inspector

	// Lower Index
	int
	l() const
	{
		return l_;
	}

	// Upper Index
	int
	u() const
	{
		return u_;
	}

	// Size
	size_type
	size() const
	{
		return size_; // Unbounded => npos
	}

	// Size
	int
	isize() const
	{
		assert( size_ != npos );
		return static_cast< int >( size_ );
	}

	// Offset of an Index
	int
	offset( int const i ) const
	{
		return ( i - l_ ); // Doesn't check/require that IndexRange includes i
	}

	// Last Index
	int
	last() const
	{
		return std::max( l_, u_ );
	}

	// Next Index
	int
	next( int const i ) const
	{
		return i + 1; // Doesn't check that this is a valid index
	}

public: // Modifier

	// Lower Index Set
	IndexRange &
	l( int const l )
	{
		if ( l_ - 2 == u_ ) { // Unbounded range
			l_ = l;
			u_ = l_ - 2; // Reset u_ to maintain unbounded state
		} else { // Bounded
			l_ = l;
			size_ = computed_size();
			clean();
		}
		return *this;
	}

	// Upper Index Set
	IndexRange &
	u( int const u )
	{
		u_ = clean_u( u );
		size_ = computed_size();
		return *this;
	}

	// Grow Upper
	IndexRange &
	grow( int const n = 1 )
	{
		assert( n >= 0 );
		assert( u_ <= u_max - n );
		assert( u_ >= l_ - 1 );
		u_ += n;
		size_ = computed_size();
		return *this;
	}

	// Shrink Upper
	IndexRange &
	shrink( int const n = 1 )
	{
		assert( n >= 0 );
		assert( u_ >= l_ - 1 );
		u_ = clean_u( u_ - n );
		size_ = computed_size();
		return *this;
	}

	// Expand to Contain an Index
	IndexRange &
	contain( int const i )
	{
		if ( l_ - 1 <= u_ ) { // Bounded
			if ( l_ > i ) l_ = i;
			if ( u_ < i ) u_ = i;
			size_ = computed_size();
		} else { // Unbounded
			if ( l_ > i ) {
				l_ = i;
				u_ = l_ - 2; // Reset u_ to maintain unbounded state
			}
		}
		assert( legal() );
		return *this;
	}

	// Clear
	IndexRange &
	clear()
	{
		l_ = 1;
		u_ = 0;
		size_ = 0u;
		return *this;
	}

	// Clean
	IndexRange &
	clean()
	{
		if ( l_ > u_ ) {
			l_ = 1;
			u_ = 0;
			size_ = 0u;
		}
		return *this;
	}

	// Swap
	IndexRange &
	swap( IndexRange & I )
	{
		if ( this != &I ) {
			std::swap( l_, I.l_ );
			std::swap( u_, I.u_ );
			std::swap( size_, I.size_ );
		}
		return *this;
	}

private: // Methods

	// Computed Size
	size_type
	computed_size() const
	{
		return std::max( u_ - l_ + 1, -1 );
	}

	// Clean Upper Index Value After Lower Index Set
	int
	clean_u( int const u )
	{
		return std::max( u, l_ - 1 );
	}

public: // Data

	static size_type const npos; // Unbounded "size"
	static int const l_min; // Min lower index
	static int const u_max; // Max upper index

private: // Data

	int l_; // Lower index
	int u_; // Upper index
	size_type size_; // Size (npos iff unbounded)

}; // IndexRange

// Functions

// IndexRange == IndexRange
inline
bool
operator ==( IndexRange const & I, IndexRange const & J )
{
	return ( ( I.l() == J.l() ) && ( I.u() == J.u() ) );
}

// IndexRange != IndexRange
inline
bool
operator !=( IndexRange const & I, IndexRange const & J )
{
	return !( I == J );
}

// IndexRange < IndexRange
inline
bool
operator <( IndexRange const & I, IndexRange const & J )
{
	return ( I.positive() && J.positive() && ( I.u() < J.l() ) );
}

// IndexRange <= IndexRange
inline
bool
operator <=( IndexRange const & I, IndexRange const & J )
{
	return ( I.positive() && J.positive() && ( I.u() <= J.l() ) );
}

// IndexRange > IndexRange
inline
bool
operator >( IndexRange const & I, IndexRange const & J )
{
	return ( I.positive() && J.positive() && ( I.l() > J.u() ) );
}

// IndexRange >= IndexRange
inline
bool
operator >=( IndexRange const & I, IndexRange const & J )
{
	return ( I.positive() && J.positive() && ( I.l() >= J.u() ) );
}

// Swap
inline
void
swap( IndexRange & a, IndexRange & b )
{
	a.swap( b );
}

} // ObjexxFCL

#endif // ObjexxFCL_IndexRange_hh_INCLUDED
