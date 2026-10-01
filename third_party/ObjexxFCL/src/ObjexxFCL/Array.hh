#ifndef ObjexxFCL_Array_hh_INCLUDED
#define ObjexxFCL_Array_hh_INCLUDED

// Array: Array Abstract Base Class
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
#include <ObjexxFCL/Array.fwd.hh>
#include <ObjexxFCL/BArray.hh>
#include <ObjexxFCL/AlignedAllocator.hh>
#include <ObjexxFCL/ArrayS.hh>
#include <ObjexxFCL/CArrayA.hh>
#include <ObjexxFCL/InitializerSentinel.hh>
#include <ObjexxFCL/MArray.hh>
#include <ObjexxFCL/ProxySentinel.hh>
#include <ObjexxFCL/TypeTraits.hh>
#include <ObjexxFCL/Vector2.hh>
#include <ObjexxFCL/Vector3.hh>
#include <ObjexxFCL/Vector4.hh>

// C++ Headers
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iomanip>
#include <istream>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <ostream>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace ObjexxFCL {

// Array: Array Abstract Base Class
//
// Note:
//  Can hold numeric or non-numeric values: Numeric operations on non-numeric values won't compile
//  Any meaningful array index ranges can be specified as in Fortran
//  Zero-sized arrays are supported but have no valid indices
//  Argument/proxy arrays can have unbounded/unknown size
//  For efficiency constructors without initializer function or value do not initialize the array
//  For efficiency bounds checking is only active via asserts in debug builds
template< typename T >
class Array : public BArray
{

private: // Friend

	template< typename > friend class Array;

protected: // Types

	typedef  internal::InitializerSentinel  InitializerSentinel;
	typedef  internal::ProxySentinel  ProxySentinel;

public: // Types

	typedef  Array< T >  Base;
	typedef  AlignedAllocator< T >  Aligned;
	typedef  TypeTraits< T >  Traits;

	// STL style
	typedef  T  value_type;
	typedef  T &  reference;
	typedef  T const &  const_reference;
	typedef  T *  pointer;
	typedef  T const *  const_pointer;
	typedef  T *  iterator;
	typedef  T const *  const_iterator;
	typedef  std::reverse_iterator< T * >  reverse_iterator;
	typedef  std::reverse_iterator< T const * >  const_reverse_iterator;

	// C++ style
	typedef  T  Value;
	typedef  T &  Reference;
	typedef  T const &  ConstReference;
	typedef  T *  Pointer;
	typedef  T const *  ConstPointer;
	typedef  T *  Iterator;
	typedef  T const *  ConstIterator;
	typedef  std::reverse_iterator< T * >  ReverseIterator;
	typedef  std::reverse_iterator< T const * >  ConstReverseIterator;

protected: // Creation

	// Default Constructor
	Array() :
	 owner_( true ),
	 capacity_( 0u ),
	 size_( 0u ),
	 mem_( nullptr ),
	 data_( nullptr ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// Copy Constructor
	Array( Array const & a ) :
	 BArray( a ),
	 owner_( true ),
	 capacity_( size_of( a.size_ ) ),
	 size_( capacity_ ),
	 mem_( a.data_ != nullptr ? Aligned::allocate_zero( capacity_ ) : nullptr ),
	 data_( a.data_ != nullptr ? Aligned::data( mem_ ) : nullptr ),
	 shift_( a.shift_ ),
	 sdata_( data_ - shift_ )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( a.data_[ i ] );
		}
	}

	// Move Constructor
	Array( Array && a ) noexcept :
	 BArray( std::move( a ) ),
	 owner_( a.owner_ ),
	 capacity_( a.capacity_ ),
	 size_( a.size_ ),
	 mem_( a.mem_ ),
	 data_( a.data_ ),
	 shift_( a.shift_ ),
	 sdata_( a.sdata_ )
	{
		a.capacity_ = a.size_ = 0u;
		a.mem_ = a.data_ = a.sdata_ = nullptr;
		a.shift_ = 0;
	}

	// Copy Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array( Array< U > const & a ) :
	 owner_( true ),
	 capacity_( size_of( a.size() ) ),
	 size_( capacity_ ),
	 mem_( a.data_ != nullptr ? Aligned::allocate_zero( capacity_ ) : nullptr ),
	 data_( a.data_ != nullptr ? Aligned::data( mem_ ) : nullptr ),
	 shift_( a.shift_ ),
	 sdata_( data_ - shift_ )
	{
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( a.data_[ i ] );
		}
	}

	// Slice Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	explicit
	Array( ArrayS< U > const & a ) :
	 owner_( true ),
	 capacity_( size_of( a.size() ) ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// MArray Constructor Template
	template< class A, typename M >
	explicit
	Array( MArray< A, M > const & a ) :
	 owner_( true ),
	 capacity_( size_of( a.size() ) ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// Size Constructor
	explicit
	Array( size_type const size ) :
	 owner_( true ),
	 capacity_( size_of( size ) ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
		T const fill( Traits::initial_array_value() );
#endif
		for ( size_type i = 0; i < size_; ++i ) {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
			new ( data_ + i ) T( fill );
#else
			new ( data_ + i ) T;
#endif
		}
	}

	// Size + InitializerSentinel Constructor
	Array( size_type const size, InitializerSentinel ) :
	 owner_( true ),
	 capacity_( size_of( size ) ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// Initializer List Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array( std::initializer_list< U > const l ) :
	 owner_( true ),
	 capacity_( l.size() ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		auto il( l.begin() );
		for ( size_type i = 0; i < size_; ++i, ++il ) {
			new ( data_ + i ) T( *il );
		}
	}

	// std::array Constructor Template
	template< typename U, Size s, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array( std::array< U, s > const & a ) :
	 owner_( true ),
	 capacity_( s ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		auto ia( a.begin() );
		for ( size_type i = 0; i < size_; ++i, ++ia ) {
			new ( data_ + i ) T( *ia );
		}
	}

	// std::vector Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array( std::vector< U > const & v ) :
	 owner_( true ),
	 capacity_( v.size() ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		auto iv( v.begin() );
		for ( size_type i = 0; i < size_; ++i, ++iv ) {
			new ( data_ + i ) T( *iv );
		}
	}

	// Vector2 Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array( Vector2< U > const & v ) :
	 owner_( true ),
	 capacity_( 2u ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		new ( &data_[ 0 ] ) T( v.x );
		new ( &data_[ 1 ] ) T( v.y );
	}

	// Vector3 Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array( Vector3< U > const & v ) :
	 owner_( true ),
	 capacity_( 3u ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		new ( &data_[ 0 ] ) T( v.x );
		new ( &data_[ 1 ] ) T( v.y );
		new ( &data_[ 2 ] ) T( v.z );
	}

	// Vector4 Constructor Template
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	Array( Vector4< U > const & v ) :
	 owner_( true ),
	 capacity_( 4u ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		new ( &data_[ 0 ] ) T( v.x );
		new ( &data_[ 1 ] ) T( v.y );
		new ( &data_[ 2 ] ) T( v.z );
		new ( &data_[ 3 ] ) T( v.w );
	}

	// Iterator Range Constructor Template
	template< class Iterator, typename = decltype( *std::declval< Iterator & >(), void(), ++std::declval< Iterator & >(), void() ) >
	Array( Iterator const beg, Iterator const end ) :
	 owner_( true ),
	 capacity_( end - beg ),
	 size_( capacity_ ),
	 mem_( Aligned::allocate_zero( capacity_ ) ),
	 data_( Aligned::data( mem_ ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		size_type i( 0u );
		for ( Iterator ii = beg; ii != end; ++ii, ++i ) {
			new ( data_ + i ) T( *ii );
		}
	}

	// Default Proxy Constructor
	Array( ProxySentinel ) :
	 owner_( false ),
	 capacity_( 0u ),
	 size_( 0u ),
	 mem_( nullptr ),
	 data_( nullptr ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// Array Proxy Constructor
	Array( Array const & a, ProxySentinel ) :
	 owner_( false ),
	 capacity_( a.capacity_ ),
	 size_( a.size_ ),
	 mem_( nullptr ),
	 data_( a.data_ ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

	// Slice Proxy Constructor
	Array( ArrayS< T > const & a, ProxySentinel ) :
	 owner_( false ),
	 capacity_( a.size() ),
	 size_( a.size() ),
	 mem_( nullptr ),
	 data_( a.data_beg_ ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{
		assert( a.contiguous() );
	}


	// Value Proxy Constructor
	Array( T const & t, ProxySentinel ) :
	 owner_( false ),
	 capacity_( npos ), // Unknown
	 size_( npos ), // Unbounded
	 mem_( nullptr ),
	 data_( const_cast< T * >( &t ) ),
	 shift_( 0 ),
	 sdata_( nullptr )
	{}

public: // Creation

	// Destructor
	virtual
	~Array()
	{
		if ( owner_ ) destroy();
	}

protected: // Assignment: Array

	// Copy Assignment
	void
	operator =( Array const & a )
	{
		assert( size_bounded() );
		assert( size_ == a.size_ );
		if ( overlap( a ) ) { // Overlap-safe
			CArrayA< T > c( size_ );
			for ( size_type i = 0; i < size_; ++i ) {
				c[ i ] = a[ i ];
			}
			for ( size_type i = 0; i < size_; ++i ) {
				data_[ i ] = c[ i ];
			}
		} else { // Not overlap-safe
			for ( size_type i = 0; i < size_; ++i ) {
				data_[ i ] = a[ i ];
			}
		}
	}

	// Move Assignment
	void
	operator =( Array && a ) noexcept
	{
		assert( this != &a );
		assert( owner_ == a.owner_ );
		if ( owner_ ) destroy();
		capacity_ = a.capacity_;
		size_ = a.size_;
		mem_ = a.mem_;
		data_ = a.data_;
		shift_ = a.shift_;
		sdata_ = a.sdata_;
		a.capacity_ = a.size_ = 0u;
		a.mem_ = a.data_ = a.sdata_ = nullptr;
		a.shift_ = 0;
	}

	// Copy Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( Array< U > const & a )
	{
		assert( size_bounded() );
		assert( size_ == a.size() );
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] = a[ i ];
		}
	}

	// Initializer List Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( std::initializer_list< U > const l )
	{
		assert( size_ == l.size() );
		std::copy( l.begin(), l.end(), data_ );
	}

	// std::array Assignment Template
	template< typename U, Size s, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( std::array< U, s > const & a )
	{
		assert( size_ == s );
		std::copy( a.begin(), a.end(), data_ );
	}

	// std::vector Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( std::vector< U > const & v )
	{
		assert( size_ == v.size() );
		std::copy( v.begin(), v.end(), data_ );
	}

	// Vector2 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( Vector2< U > const & v )
	{
		assert( size_ == 2u );
		operator []( 0 ) = v.x;
		operator []( 1 ) = v.y;
	}

	// Vector3 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( Vector3< U > const & v )
	{
		assert( size_ == 3u );
		operator []( 0 ) = v.x;
		operator []( 1 ) = v.y;
		operator []( 2 ) = v.z;
	}

	// Vector4 Assignment Template
	template< typename U, class = typename std::enable_if< std::is_assignable< T&, U >::value >::type >
	void
	operator =( Vector4< U > const & v )
	{
		assert( size_ == 4u );
		operator []( 0 ) = v.x;
		operator []( 1 ) = v.y;
		operator []( 2 ) = v.z;
		operator []( 3 ) = v.w;
	}

public: // Assignment: Value

	// = Value
	Array &
	operator =( T const & t )
	{
		assert( size_bounded() );
		if ( data_ ) std::fill_n( data_, size_, t );
		return *this;
	}


public: // Subscript

	// array[ i ] const: Linear Subscript
	T const &
	operator []( size_type const i ) const
	{
		assert( ( i < size_ ) || ( size_ == npos ) );
		return data_[ i ];
	}

	// array[ i ]: Linear Subscript
	T &
	operator []( size_type const i )
	{
		assert( ( i < size_ ) || ( size_ == npos ) );
		return data_[ i ];
	}

public: // Predicate

	// Owner?
	bool
	owner() const
	{
		return owner_;
	}

	// Proxy?
	bool
	proxy() const
	{
		return ! owner_;
	}

	// Active?
	bool
	active() const
	{
		return ( data_ != nullptr );
	}

	// Allocated
	bool
	allocated() const
	{
		return ( data_ != nullptr );
	}

	// Active Array Empty?
	bool
	empty() const
	{
		return ( size_ == 0u );
	}

	// Active Array Size Bounded?
	bool
	size_bounded() const
	{
		return ( size_ != npos );
	}

	// Memory Can Overlap a Range?
	bool
	overlap( T const * const b, T const * const e ) const
	{
		if ( ( data_ == nullptr ) || ( b == nullptr ) || ( e == nullptr ) ) { // No active memory range(s)
			return false;
		} else if ( size_ == 0u ) { // No finite memory range
			return false;
		} else if ( size_ == npos ) { // No memory upper bound
			return ( e >= data_ );
		} else { // Bounded ranges
			assert( b <= e );
			T const * const de( data_ + size_ - 1 ); // Data end pointer
			return ( ( data_ >= b ? data_ : b ) <= ( de <= e ? de : e ) );
		}
	}

	// Memory Can Overlap an Array?
	template< template< typename > class A >
	bool
	overlap( A< T > const & a ) const
	{
		if ( data_ == nullptr ) { // No active memory range
			return false;
		} else { // Bounded ranges
			return overlap( a.data_beg(), a.data_end() );
		}
	}

public: // Inspector

	// Rank
	virtual
	int
	rank() const = 0;

	// Data Size
	size_type
	capacity() const
	{
		return capacity_;
	}

	// Active Array Size
	size_type
	size() const
	{
		return size_;
	}

	// Active Array Size
	int
	isize() const
	{
		assert( size_ != npos );
		return static_cast< int >( size_ );
	}

	// IndexRange of a Dimension
	virtual
	IR const &
	I( int const d ) const = 0;

	// Lower Index of a Dimension
	virtual
	int
	l( int const d ) const = 0;

	// Upper Index of a Dimension
	virtual
	int
	u( int const d ) const = 0;

	// Size of a Dimension
	virtual
	size_type
	size( int const d ) const = 0;

	// Size of a Dimension
	virtual
	int
	isize( int const d ) const = 0;

	// Begin Iterator
	const_iterator
	begin() const
	{
		return data_;
	}

	// Begin Iterator
	iterator
	begin()
	{
		return data_;
	}

	// End Iterator
	const_iterator
	end() const
	{
		return ( ( data_ != nullptr ) && ( size_ != npos ) ? data_ + size_ : nullptr );
	}

	// End Iterator
	iterator
	end()
	{
		return ( ( data_ != nullptr ) && ( size_ != npos ) ? data_ + size_ : nullptr );
	}

	// Reverse Begin Iterator
	const_reverse_iterator
	rbegin() const
	{
		return const_reverse_iterator( ( data_ != nullptr ) && ( size_ != npos ) ? data_ + size_ : nullptr );
	}

	// Reverse Begin Iterator
	reverse_iterator
	rbegin()
	{
		return reverse_iterator( ( data_ != nullptr ) && ( size_ != npos ) ? data_ + size_ : nullptr );
	}

	// Reverse End Iterator
	const_reverse_iterator
	rend() const
	{
		return const_reverse_iterator( data_ );
	}

	// Reverse End Iterator
	reverse_iterator
	rend()
	{
		return reverse_iterator( data_ );
	}

	// Data Pointer
	T const *
	data() const
	{
		return data_;
	}

	// Data Pointer
	T *
	data()
	{
		return data_;
	}

	// Data Begin Pointer
	T const *
	data_beg() const
	{
		return data_;
	}

	// Data Begin Pointer
	T *
	data_beg()
	{
		return data_;
	}

	// Data End Pointer
	T const *
	data_end() const
	{
		return ( ( data_ != nullptr ) && ( size_ > 0u ) && ( size_ != npos ) ? data_ + size_ - 1 : nullptr );
	}

	// Data End Pointer
	T *
	data_end()
	{
		return ( ( data_ != nullptr ) && ( size_ > 0u ) && ( size_ != npos ) ? data_ + size_ - 1 : nullptr );
	}

public: // Modifier

	// Clear
	virtual
	Array &
	clear()
	{
		if ( owner_ ) destroy();
		capacity_ = size_ = 0u;
		mem_ = data_ = sdata_ = nullptr;
		shift_ = 0;
		return *this;
	}


public: // Comparison: Predicate

	// Array == Value
	friend
	bool
	eq( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return true;
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( ! ( a[ i ] == t ) ) return false;
		}
		return true;
	}

	// Value == Array
	friend
	bool
	eq( T const & t, Array const & a )
	{
		return eq( a, t );
	}


public: // Comparison: Predicate: Any

	// Any Array == Value
	friend
	bool
	any_eq( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return false;
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( a[ i ] == t ) return true;
		}
		return false;
	}

	// Any Value == Array
	friend
	bool
	any_eq( T const & t, Array const & a )
	{
		return any_eq( a, t );
	}

	// Any Array != Value
	friend
	bool
	any_ne( Array const & a, T const & t )
	{
		return ! eq( a, t );
	}

	// Any Value != Array
	friend
	bool
	any_ne( T const & t, Array const & a )
	{
		return ! eq( a, t );
	}

	// Any Array < Value
	friend
	bool
	any_lt( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return false;
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( a[ i ] < t ) return true;
		}
		return false;
	}

	// Any Value < Array
	friend
	bool
	any_lt( T const & t, Array const & a )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return false;
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( t < a[ i ] ) return true;
		}
		return false;
	}

	// Any Array <= Value
	friend
	bool
	any_le( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return false;
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( a[ i ] <= t ) return true;
		}
		return false;
	}

	// Any Value <= Array
	friend
	bool
	any_le( T const & t, Array const & a )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return false;
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( t <= a[ i ] ) return true;
		}
		return false;
	}

	// Any Array > Value
	friend
	bool
	any_gt( Array const & a, T const & t )
	{
		return any_lt( t, a );
	}

	// Any Value > Array
	friend
	bool
	any_gt( T const & t, Array const & a )
	{
		return any_lt( a, t );
	}

	// Any Array >= Value
	friend
	bool
	any_ge( Array const & a, T const & t )
	{
		return any_le( t, a );
	}

	// Any Value >= Array
	friend
	bool
	any_ge( T const & t, Array const & a )
	{
		return any_le( a, t );
	}

public: // Comparison: Predicate: All

	// All Array != Value
	friend
	bool
	all_ne( Array const & a, T const & t )
	{
		return ! any_eq( a, t );
	}

	// All Value != Array
	friend
	bool
	all_ne( T const & t, Array const & a )
	{
		return ! any_eq( a, t );
	}

	// All Array <= Value
	friend
	bool
	all_le( Array const & a, T const & t )
	{
		return le( a, t );
	}

	// All Value <= Array
	friend
	bool
	all_le( T const & t, Array const & a )
	{
		return le( t, a );
	}

	// All Value > Array
	friend
	bool
	all_gt( T const & t, Array const & a )
	{
		return gt( t, a );
	}

	// All Value >= Array
	friend
	bool
	all_ge( T const & t, Array const & a )
	{
		return ge( t, a );
	}

public: // Comparison: Count

	// Count Array == Value
	friend
	size_type
	count_eq( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return 0;
		size_type n( 0u );
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( a[ i ] == t ) ++n;
		}
		return n;
	}

	// Count Value == Array
	friend
	size_type
	count_eq( T const & t, Array const & a )
	{
		return count_eq( a, t );
	}

	// Count Array != Value
	friend
	size_type
	count_ne( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return 0;
		size_type n( 0u );
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( a[ i ] != t ) ++n;
		}
		return n;
	}

	// Count Value != Array
	friend
	size_type
	count_ne( T const & t, Array const & a )
	{
		return count_ne( a, t );
	}

	// Count Array < Value
	friend
	size_type
	count_lt( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return 0;
		size_type n( 0u );
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( a[ i ] < t ) ++n;
		}
		return n;
	}

	// Count Value < Array
	friend
	size_type
	count_lt( T const & t, Array const & a )
	{
		return count_gt( a, t );
	}

	// Count Array <= Value
	friend
	size_type
	count_le( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return 0;
		size_type n( 0u );
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( a[ i ] <= t ) ++n;
		}
		return n;
	}

	// Count Value <= Array
	friend
	size_type
	count_le( T const & t, Array const & a )
	{
		return count_ge( a, t );
	}

	// Count Array > Value
	friend
	size_type
	count_gt( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return 0;
		size_type n( 0u );
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( a[ i ] > t ) ++n;
		}
		return n;
	}

	// Count Value > Array
	friend
	size_type
	count_gt( T const & t, Array const & a )
	{
		return count_lt( a, t );
	}

	// Count Array >= Value
	friend
	size_type
	count_ge( Array const & a, T const & t )
	{
		assert( a.size_bounded() );
		if ( a.empty() ) return 0;
		size_type n( 0u );
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( a[ i ] >= t ) ++n;
		}
		return n;
	}

	// Count Value >= Array
	friend
	size_type
	count_ge( T const & t, Array const & a )
	{
		return count_le( a, t );
	}

protected: // Comparison: Predicate

	// Array == Array
	friend
	bool
	eq( Array const & a, Array const & b )
	{
		assert( a.size_bounded() );
		assert( a.size_ == b.size_ );
		if ( ( &a == &b ) || a.empty() ) return true;
		for ( size_type i = 0, e = a.size_; i < e; ++i ) {
			if ( ! ( a[ i ] == b[ i ] ) ) return false;
		}
		return true;
	}


protected: // Comparison: Elemental

	// Array == Array
	friend
	void
	eq_elemental( Array const & a, Array const & b, Array< bool > & r )
	{
		assert( a.size() == b.size() );
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] == b[ i ] );
		}
	}

	// Array != Array
	friend
	void
	ne_elemental( Array const & a, Array const & b, Array< bool > & r )
	{
		assert( a.size() == b.size() );
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] != b[ i ] );
		}
	}

	// Array < Array
	friend
	void
	lt_elemental( Array const & a, Array const & b, Array< bool > & r )
	{
		assert( a.size() == b.size() );
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] < b[ i ] );
		}
	}

	// Array <= Array
	friend
	void
	le_elemental( Array const & a, Array const & b, Array< bool > & r )
	{
		assert( a.size() == b.size() );
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] <= b[ i ] );
		}
	}

	// Array > Array
	friend
	void
	gt_elemental( Array const & a, Array const & b, Array< bool > & r )
	{
		assert( a.size() == b.size() );
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] > b[ i ] );
		}
	}

	// Array >= Array
	friend
	void
	ge_elemental( Array const & a, Array const & b, Array< bool > & r )
	{
		assert( a.size() == b.size() );
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] >= b[ i ] );
		}
	}

	// Array == Value
	friend
	void
	eq_elemental( Array const & a, T const & t, Array< bool > & r )
	{
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] == t );
		}
	}

	// Array != Value
	friend
	void
	ne_elemental( Array const & a, T const & t, Array< bool > & r )
	{
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] != t );
		}
	}

	// Array < Value
	friend
	void
	lt_elemental( Array const & a, T const & t, Array< bool > & r )
	{
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] < t );
		}
	}

	// Array <= Value
	friend
	void
	le_elemental( Array const & a, T const & t, Array< bool > & r )
	{
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] <= t );
		}
	}

	// Array > Value
	friend
	void
	gt_elemental( Array const & a, T const & t, Array< bool > & r )
	{
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] > t );
		}
	}

	// Array >= Value
	friend
	void
	ge_elemental( Array const & a, T const & t, Array< bool > & r )
	{
		assert( a.size() == r.size() );
		for ( size_type i = 0, e = a.size(); i < e; ++i ) {
			r[ i ] = ( a[ i ] >= t );
		}
	}

	// Value == Array
	friend
	void
	eq_elemental( T const & t, Array const & b, Array< bool > & r )
	{
		assert( b.size() == r.size() );
		for ( size_type i = 0, e = b.size(); i < e; ++i ) {
			r[ i ] = ( t == b[ i ] );
		}
	}

	// Value != Array
	friend
	void
	ne_elemental( T const & t, Array const & b, Array< bool > & r )
	{
		assert( b.size() == r.size() );
		for ( size_type i = 0, e = b.size(); i < e; ++i ) {
			r[ i ] = ( t != b[ i ] );
		}
	}

	// Value < Array
	friend
	void
	lt_elemental( T const & t, Array const & b, Array< bool > & r )
	{
		assert( b.size() == r.size() );
		for ( size_type i = 0, e = b.size(); i < e; ++i ) {
			r[ i ] = ( t < b[ i ] );
		}
	}

	// Value <= Array
	friend
	void
	le_elemental( T const & t, Array const & b, Array< bool > & r )
	{
		assert( b.size() == r.size() );
		for ( size_type i = 0, e = b.size(); i < e; ++i ) {
			r[ i ] = ( t <= b[ i ] );
		}
	}

	// Value > Array
	friend
	void
	gt_elemental( T const & t, Array const & b, Array< bool > & r )
	{
		assert( b.size() == r.size() );
		for ( size_type i = 0, e = b.size(); i < e; ++i ) {
			r[ i ] = ( t > b[ i ] );
		}
	}

	// Value >= Array
	friend
	void
	ge_elemental( T const & t, Array const & b, Array< bool > & r )
	{
		assert( b.size() == r.size() );
		for ( size_type i = 0, e = b.size(); i < e; ++i ) {
			r[ i ] = ( t >= b[ i ] );
		}
	}

protected: // Methods

	// Shift Setup
	void
	shift_set( difference_type const shift )
	{
		shift_ = shift;
		sdata_ = data_ - shift_;
	}

	// Shift Setup Without Setting Shifted Data Pointer
	void
	shift_only_set( difference_type const shift )
	{
		shift_ = shift;
	}

	// Active Array Size Setup
	void
	size_set( size_type const size )
	{
		assert( size <= capacity_ );
		size_ = size;
	}

	// Resize a Real Array: Return Whether Reallocation Happened
	bool
	resize( size_type const size )
	{
		assert( owner_ );
		assert( size != npos );
		if ( ( data_ == nullptr ) || ( capacity_ < size ) || ( ( capacity_ == size_ ) && ( size != size_ ) ) ) {
			destroy();
			capacity_ = size_ = size;
			mem_ = Aligned::allocate_zero( capacity_ );
			data_ = Aligned::data( mem_ );
			sdata_ = data_ - shift_;
			return true; // Reallocated: Elements not constructed
		} else {
			size_type i( size_ );
			while ( i > size ) { // Destruct removed elements
				data_[ --i ].~T();
			}
			size_ = size;
			sdata_ = data_ - shift_;
			return false; // Not reallocated
		}
	}

	// Reserve Capacity in a Real Array
	void
	reserve_capacity( size_type const n )
	{
		assert( owner_ );
		assert( n <= max_size );
		if ( capacity_ < n ) {
			void * new_mem = Aligned::allocate_zero( n );
			T * new_data = Aligned::data( new_mem );
			if ( size_ > 0u ) uninitialized_move_or_copy( data_, data_ + size_, new_data );
			destroy();
			capacity_ = n;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
		}
	}

	// Shrink Capacity to Size in a Real Array
	void
	shrink_capacity()
	{
		assert( owner_ );
		if ( capacity_ > size_ ) {
			void * new_mem = Aligned::allocate_zero( size_ );
			T * new_data = Aligned::data( new_mem );
			if ( size_ > 0u ) uninitialized_move_or_copy( data_, data_ + size_, new_data );
			destroy();
			capacity_ = size_;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
		}
	}

	// Append Value by Copy in a Real Array
	void
	do_push_back_copy( T const & t )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			new ( &new_data[ size_ ] ) T( t );
			if ( size_ > 0u ) uninitialized_move_or_copy( data_, data_ + size_, new_data );
			destroy();
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
		} else {
			new ( &data_[ size_ ] ) T( t );
		}
		size_ = new_size;
	}

	// Append Value by Move in a Real Array
	void
	do_push_back_move( T && t )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			new ( &new_data[ size_ ] ) T( std::move( t ) );
			if ( size_ > 0u ) uninitialized_move_or_copy( data_, data_ + size_, new_data );
			destroy();
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
		} else {
			new ( &data_[ size_ ] ) T( std::move( t ) );
		}
		size_ = new_size;
	}

	// Append Value by Move in a Real Array
	void
	do_pop_back()
	{
		if ( size_ > 0u ) {
			--size_;
			data_[ size_ ].~T();
		}
	}

	// Insert Value by Copy in a Real Array
	iterator
	do_insert_copy( const_iterator pos, T const & t )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		assert( data_ <= pos );
		assert( pos <= end() );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		iterator const old_pos( data_ + ( pos - data_ ) );
		iterator const old_end( end() );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			iterator const new_pos( new_data + ( pos - data_ ) );
			new ( &*new_pos ) T( t );
			if ( data_ < pos ) uninitialized_move_or_copy( data_, old_pos, new_data );
			if ( pos < old_end ) uninitialized_move_or_copy( old_pos, old_end, new_pos + 1 );
			destroy();
			size_ = new_size;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
			return new_pos;
		} else {
			if ( pos == old_end ) {
				new ( &*old_end ) T( t );
			} else {
				T const tt( t );
				uninitialized_move_or_copy( old_end - 1, old_end, old_end );
				if ( size_ > 1u ) move_or_copy_backward( old_pos, old_end - 1, old_end );
				*old_pos = tt;
			}
			size_ = new_size;
			return old_pos;
		}
	}

	// Insert Value by Move in a Real Array
	iterator
	do_insert_move( const_iterator pos, T && t )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		assert( data_ <= pos );
		assert( pos <= end() );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		iterator const old_pos( data_ + ( pos - data_ ) );
		iterator const old_end( end() );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			iterator const new_pos( new_data + ( pos - data_ ) );
			new ( &*new_pos ) T( std::move( t ) );
			if ( data_ < pos ) uninitialized_move_or_copy( data_, old_pos, new_data );
			if ( pos < old_end ) uninitialized_move_or_copy( old_pos, old_end, new_pos + 1 );
			destroy();
			size_ = new_size;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
			return new_pos;
		} else {
			if ( pos == old_end ) {
				new ( &*old_end ) T( std::move( t ) );
			} else {
				uninitialized_move_or_copy( old_end - 1, old_end, old_end );
				if ( size_ > 1u ) move_or_copy_backward( old_pos, old_end - 1, old_end );
				*old_pos = std::move( t );
			}
			size_ = new_size;
			return old_pos;
		}
	}

	// Insert Multiples of a Value by Copy in a Real Array
	iterator
	do_insert_n( const_iterator pos, size_type n, T const & t )
	{
		assert( owner_ );
		assert( size_ < npos - n );
		assert( data_ <= pos );
		assert( pos <= end() );
		size_type const new_size( size_ + n );
		assert( new_size <= max_size );
		iterator const old_pos( data_ + ( pos - data_ ) );
		iterator const old_end( end() );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			iterator const new_pos( new_data + ( pos - data_ ) );
			std::uninitialized_fill_n( new_pos, n, t );
			if ( data_ < pos ) uninitialized_move_or_copy( data_, old_pos, new_data );
			if ( pos < old_end ) uninitialized_move_or_copy( old_pos, old_end, new_pos + n );
			destroy();
			size_ = new_size;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
			return new_pos;
		} else {
			if ( pos == old_end ) {
				std::uninitialized_fill_n( old_end, n, t );
			} else {
				T const tt( t );
				iterator const k( old_pos + n < old_end ? old_end - n : old_pos );
				uninitialized_move_or_copy( k, old_end, k + n );
				move_or_copy_backward( old_pos, k, old_end );
				size_type const ni( std::min( n, static_cast< size_type >( old_end - old_pos ) ) );
				std::fill_n( old_pos, ni, tt );
				if ( ni < n ) std::uninitialized_fill_n( old_end, n - ni, tt );
			}
			size_ = new_size;
			return old_pos;
		}
	}

	// Insert Iterator Range in a Real Array
	template< typename Iterator, class = typename std::enable_if<
	 std::is_same< typename std::iterator_traits< Iterator >::iterator_category, std::input_iterator_tag >::value ||
	 std::is_same< typename std::iterator_traits< Iterator >::iterator_category, std::forward_iterator_tag >::value ||
	 std::is_same< typename std::iterator_traits< Iterator >::iterator_category, std::bidirectional_iterator_tag >::value ||
	 std::is_same< typename std::iterator_traits< Iterator >::iterator_category, std::random_access_iterator_tag >::value
	 >::type >
	iterator
	do_insert_iterator( const_iterator pos, Iterator first, Iterator last ) // Like std containers first and last may not be iterators to this Array
	{
		assert( owner_ );
		size_type const n( std::distance( first, last ) );
		assert( size_ < npos - n );
		assert( data_ <= pos );
		assert( pos <= end() );
		size_type const new_size( size_ + n );
		assert( new_size <= max_size );
		iterator const old_pos( data_ + ( pos - data_ ) );
		iterator const old_end( end() );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			iterator const new_pos( new_data + ( pos - data_ ) );
			std::copy( first, last, new_pos );
			if ( data_ < pos ) uninitialized_move_or_copy( data_, old_pos, new_data );
			if ( pos < old_end ) uninitialized_move_or_copy( old_pos, old_end, new_pos + n );
			destroy();
			size_ = new_size;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
			return new_pos;
		} else {
			if ( pos == old_end ) {
				std::uninitialized_copy( first, last, old_pos );
			} else {
				iterator const k( old_pos + n < old_end ? old_end - n : old_pos );
				uninitialized_move_or_copy( k, old_end, k + n );
				move_or_copy_backward( old_pos, k, old_end );
				size_type const ni( std::min( n, static_cast< size_type >( old_end - old_pos ) ) );
				std::copy( first, first + ni, old_pos );
				if ( ni < n ) std::uninitialized_copy( first + ni, last, old_pos + ni );
			}
			size_ = new_size;
			return old_pos;
		}
	}

	// Insert Initializer List in a Real Array
	iterator
	do_insert_initializer_list( const_iterator pos, std::initializer_list< T > il )
	{
		assert( owner_ );
		size_type const n( il.size() );
		assert( size_ < npos - n );
		assert( data_ <= pos );
		assert( pos <= end() );
		size_type const new_size( size_ + n );
		assert( new_size <= max_size );
		iterator const old_pos( data_ + ( pos - data_ ) );
		iterator const old_end( end() );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			iterator const new_pos( new_data + ( pos - data_ ) );
			std::copy( il.begin(), il.end(), new_pos );
			if ( data_ < pos ) uninitialized_move_or_copy( data_, old_pos, new_data );
			if ( pos < old_end ) uninitialized_move_or_copy( old_pos, old_end, new_pos + n );
			destroy();
			size_ = new_size;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
			return new_pos;
		} else {
			if ( pos == old_end ) {
				std::uninitialized_copy( il.begin(), il.end(), old_pos );
			} else {
				iterator const k( old_pos + n < old_end ? old_end - n : old_pos );
				uninitialized_move_or_copy( k, old_end, k + n );
				move_or_copy_backward( old_pos, k, old_end );
				size_type const ni( std::min( n, static_cast< size_type >( old_end - old_pos ) ) );
				std::copy( il.begin(), il.begin() + ni, old_pos );
				if ( ni < n ) std::uninitialized_copy( il.begin() + ni, il.end(), old_pos + ni );
			}
			size_ = new_size;
			return old_pos;
		}
	}

	// Insert Value Constructed in Place in a Real Array
	template< typename... Args >
	iterator
	do_emplace( const_iterator pos, Args &&... args )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		assert( data_ <= pos );
		assert( pos <= end() );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		iterator const old_pos( data_ + ( pos - data_ ) );
		iterator const old_end( end() );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			iterator const new_pos( new_data + ( pos - data_ ) );
			new ( &*new_pos ) T( std::forward< Args >( args )... );
			if ( data_ < pos ) uninitialized_move_or_copy( data_, old_pos, new_data );
			if ( pos < old_end ) uninitialized_move_or_copy( old_pos, old_end, new_pos + 1 );
			destroy();
			size_ = new_size;
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
			return new_pos;
		} else {
			if ( pos == old_end ) {
				new ( &*pos ) T( std::forward< Args >( args )... );
			} else {
				new ( &data_[ size_ ] ) T( data_[ size_ - 1 ] );
				move_or_copy_backward( old_pos, old_end - 1, old_end );
				*pos = T( std::forward< Args >( args )... );
			}
			size_ = new_size;
			return old_pos;
		}
	}

	// Append Value Constructed in Place in a Real Array
	template< typename... Args >
	void
	do_emplace_back( Args &&... args )
	{
		assert( owner_ );
		assert( size_ < npos - 1 );
		size_type const new_size( size_ + 1 );
		assert( new_size <= max_size );
		if ( capacity_ < new_size ) {
			capacity_ = std::min( std::max( capacity_ << 1, new_size ), max_size );
			void * const new_mem = Aligned::allocate_zero( capacity_ );
			T * const new_data = Aligned::data( new_mem );
			new ( &new_data[ size_ ] ) T( std::forward< Args >( args )... );
			if ( size_ > 0u ) uninitialized_move_or_copy( data_, data_ + size_, new_data );
			destroy();
			mem_ = new_mem;
			data_ = new_data;
			sdata_ = data_ - shift_;
		} else {
			new ( &data_[ size_ ] ) T( std::forward< Args >( args )... );
		}
		size_ = new_size;
	}

	// Erase Iterator in a Real Array
	iterator
	do_erase( const_iterator pos )
	{
		assert( owner_ );
		assert( data_ <= pos );
		assert( pos < end() );
		iterator const old_pos( data_ + ( pos - data_ ) );
		iterator const old_end( end() );
		move_or_copy( old_pos + 1, old_end, old_pos );
		old_end->~T();
		--size_;
		return old_pos;
	}

	// Erase Iterator Range in a Real Array
	iterator
	do_erase_iterator( const_iterator first, const_iterator last )
	{
		assert( owner_ );
		assert( ( data_ <= first ) || ( first == last ) );
		assert( ( first <= last ) || ( first == last ) );
		assert( ( last <= end() ) || ( first == last ) );
		size_type const n( std::distance( first, last ) );
		iterator const start( data_ + ( first - data_ ) );
		if ( n > 0u ) {
			iterator const stop( data_ + ( last - data_ ) );
			iterator const old_end( end() );
			move_or_copy( stop, old_end, start );
			for ( auto i = stop; i != old_end; ++i ) i->~T();
			size_ -= n;
		}
		return start;
	}


	// Swap
	void
	swapB( Array & v )
	{
		assert( owner_ );
		assert( v.owner_ );
		std::swap( capacity_, v.capacity_ );
		std::swap( size_, v.size_ );
		std::swap( mem_, v.mem_ );
		std::swap( data_, v.data_ );
		std::swap( shift_, v.shift_ );
		std::swap( sdata_, v.sdata_ );
	}

	// Initialize by Uniform Value
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	initialize( U const & u )
	{
		assert( owner_ );
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( u );
		}
	}

	// Initialize by Array
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	initialize( Array< U > const & a )
	{
		assert( owner_ );
		assert( size_ == a.size_ );
		for ( size_type i = 0; i < size_; ++i ) {
			new ( data_ + i ) T( a[ i ] );
		}
	}

	// Initialize an Element
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	initialize( size_type const i, U const & u )
	{
		assert( owner_ );
		new ( data_ + i ) T( u );
	}

	// Uniform Assignment
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	assign( U const & u )
	{
		assert( owner_ );
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] = T( u );
		}
	}

	// Element Assignment
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	assign( size_type const i, U const & u )
	{
		data_[ i ] = T( u );
	}

	// Assign Array
	template< typename U, class = typename std::enable_if< std::is_constructible< T, U >::value >::type >
	void
	assign( Array< U > const & a )
	{
		assert( size_bounded() );
		assert( size_ == a.size_ );
		for ( size_type i = 0; i < size_; ++i ) {
			data_[ i ] = T( a[ i ] );
		}
	}

	// Switch to Size Construction
	void
	reconstruct_by_size( size_type const size )
	{
		assert( owner_ );
		destroy();
		capacity_ = size_ = size;
		assert( size_bounded() );
		mem_ = Aligned::allocate_zero( capacity_ );
		data_ = Aligned::data( mem_ );
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
		T const fill( Traits::initial_array_value() );
#endif
		for ( size_type i = 0; i < size_; ++i ) {
#if defined(OBJEXXFCL_ARRAY_INIT) || defined(OBJEXXFCL_ARRAY_INIT_DEBUG)
			new ( data_ + i ) T( fill );
#else
			new ( data_ + i ) T;
#endif
		}
	}

	// Conformable Move
	void
	conformable_move( Array & a )
	{
		assert( this != &a );
		assert( owner_ == a.owner_ );
		destroy();
		capacity_ = a.capacity_;
		mem_ = a.mem_;
		data_ = a.data_;
		sdata_ = data_ - shift_;
		a.capacity_ = a.size_ = 0u;
		a.mem_ = a.data_ = a.sdata_ = nullptr;
		a.shift_ = 0;
	}

protected: // Static Methods

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1 )
	{
		assert( s1 != npos );
		return s1;
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2 )
	{
		assert( s1 != npos );
		assert( s2 != npos );
		assert( ( s2 == 0 ) || ( s1 <= max_size / s2 ) );
		return s1 * s2;
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2, size_type const s3 )
	{
		return size_of( size_of( s1, s2 ), s3 );
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2, size_type const s3, size_type const s4 )
	{
		return size_of( size_of( s1, s2 ), size_of( s3, s4 ) );
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2, size_type const s3, size_type const s4, size_type const s5 )
	{
		return size_of( size_of( size_of( s1, s2 ), size_of( s3, s4 ) ), s5 );
	}

	// Array Size Product of Specified Bounded Dimensional Sizes
	static
	size_type
	size_of( size_type const s1, size_type const s2, size_type const s3, size_type const s4, size_type const s5, size_type const s6 )
	{
		return size_of( size_of( size_of( s1, s2 ), size_of( s3, s4 ) ), size_of( s5, s6 ) );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1 )
	{
		return size_of( I1.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2 )
	{
		return size_of( I1.size(), I2.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2, IR const & I3 )
	{
		return size_of( I1.size(), I2.size(), I3.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2, IR const & I3, IR const & I4 )
	{
		return size_of( I1.size(), I2.size(), I3.size(), I4.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2, IR const & I3, IR const & I4, IR const & I5 )
	{
		return size_of( I1.size(), I2.size(), I3.size(), I4.size(), I5.size() );
	}

	// Array Size Product of Specified Bounded IndexRanges
	static
	size_type
	size_of( IR const & I1, IR const & I2, IR const & I3, IR const & I4, IR const & I5, IR const & I6 )
	{
		return size_of( I1.size(), I2.size(), I3.size(), I4.size(), I5.size(), I6.size() );
	}

	// Slice Constant for a Scalar Index
	static
	std::int64_t
	slice_k( IR const & range, int const i, std::int64_t const multiplier = 1 )
	{
		assert( range.contains( i ) );
		assert( multiplier <= std::numeric_limits< std::int64_t >::max() / std::abs( i ) );
		(void)range; // Suppress unused warning in release builds
		return i * multiplier;
	}

	// Slice Constant for a Scalar Index
	static
	std::int64_t
	slice_k( IR const & range, int const i, size_type const multiplier )
	{
		assert( range.contains( i ) );
		assert( multiplier <= size_type( std::numeric_limits< std::int64_t >::max() / std::abs( i ) ) );
		(void)range; // Suppress unused warning in release builds
		return i * multiplier;
	}

	// Slice Constant for a Scalar Index
	static
	std::int64_t
	slice_k( int const u, int const i, std::int64_t const multiplier = 1 )
	{
		assert( ( 1 <= i ) && ( i <= u ) );
		assert( multiplier <= std::numeric_limits< std::int64_t >::max() / std::abs( i ) );
		(void)u; // Suppress unused warning in release builds
		return i * multiplier;
	}

	// Slice Constant for a Scalar Index
	static
	std::int64_t
	slice_k( int const u, int const i, size_type const multiplier )
	{
		assert( ( 1 <= i ) && ( i <= u ) );
		assert( multiplier <= size_type( std::numeric_limits< std::int64_t >::max() / std::abs( i ) ) );
		(void)u; // Suppress unused warning in release builds
		return i * multiplier;
	}

	// Move if Movable: Movable Overload
	template< typename U, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	static
	U &&
	move_if( U & u )
	{
		return std::move( u );
	}

	// Move if Movable: Non-Movable Overload
	template< typename U, class = typename std::enable_if< ! std::is_move_assignable< U >::value >::type, typename = void >
	static
	U &
	move_if( U & u )
	{
		return u;
	}

	// Move or Copy: Move Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	static
	OutputIterator
	move_or_copy( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::move( beg, end, out );
	}

	// Move or Copy: Copy Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< ! std::is_move_assignable< U >::value >::type, typename = void >
	static
	OutputIterator
	move_or_copy( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::copy( beg, end, out );
	}

	// Move or Copy Backward: Move Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	static
	OutputIterator
	move_or_copy_backward( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::move_backward( beg, end, out );
	}

	// Move or Copy Backward: Copy Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< ! std::is_move_assignable< U >::value >::type, typename = void >
	static
	OutputIterator
	move_or_copy_backward( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::copy_backward( beg, end, out );
	}

	// Move or Copy: Move Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< std::is_move_assignable< U >::value >::type >
	static
	OutputIterator
	uninitialized_move_or_copy( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::uninitialized_copy( std::make_move_iterator( beg ), std::make_move_iterator( end ), out );
	}

	// Move or Copy: Copy Overload
	template< typename InputIterator, typename OutputIterator, typename U = T, class = typename std::enable_if< ! std::is_move_assignable< U >::value >::type, typename = void >
	static
	OutputIterator
	uninitialized_move_or_copy( InputIterator beg, InputIterator end, OutputIterator out )
	{
		return std::uninitialized_copy( beg, end, out );
	}

private: // Methods

	// Destruct Elements and Delete Array Memory (Doesn't Nullify Pointers)
	void
	destroy()
	{
		if ( data_ != nullptr ) {
			size_type i( size_ );
			while ( i ) {
				data_[ --i ].~T();
			}
		}
		::operator delete( mem_ );
	}

protected: // Data

	bool const owner_; // Owner of data array?
	size_type capacity_; // Size of data array
	size_type size_; // Size of active array
	void * mem_; // Pointer to raw memory
	T * data_; // Pointer to data array
	difference_type shift_; // Array shift
	T * sdata_; // Shifted pointer to data array

}; // Array

// Stream >> Array
template< typename T >
inline
std::istream &
operator >>( std::istream & stream, Array< T > & a )
{
	if ( stream && ( a.size() > 0u ) ) {
		for ( BArray::size_type i = 0, e = a.size(); i < e; ++i ) {
			stream >> a[ i ];
			if ( ! stream ) break;
		}
	}
	return stream;
}

// Stream << Array
template< typename T >
inline
std::ostream &
operator <<( std::ostream & stream, Array< T > const & a )
{
	typedef  TypeTraits< T >  Traits;
	if ( stream && ( a.size() > 0u ) ) {
		std::ios_base::fmtflags const old_flags( stream.flags() );
		std::streamsize const old_precision( stream.precision( Traits::precision ) );
		stream << std::right << std::showpoint << std::uppercase;
		int const w( Traits::iwidth );
		for ( BArray::size_type i = 0, e = a.size() - 1; i < e; ++i ) {
			stream << std::setw( w ) << a[ i ] << ' ';
			if ( ! stream ) break;
		} if ( stream ) stream << std::setw( w ) << a[ a.size() - 1 ];
		stream.precision( old_precision );
		stream.flags( old_flags );
	}
	return stream;
}


namespace fmt {

// List-Directed Format: Array
template< typename T >
inline
std::string
LD( Array< T > const & a )
{
	std::string s;
	std::size_t const n( a.size() );
	if ( n > 0u ) {
		s.reserve( n * TypeTraits< T >::width );
		for ( std::size_t i = 0; i < n; ++i ) {
			s.append( fmt::LD( a[ i ] ) );
		}
	}
	return s;
}

} // fmt

} // ObjexxFCL

#endif // ObjexxFCL_Array_hh_INCLUDED
