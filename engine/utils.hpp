

#pragma once
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <new>

#define ASSERT assert
#define EXPORT __declspec(dllexport)
#ifdef _MSC_VER
#define FINLINE __forceinline
#else
#define FINLINE inline __attribute__((__always_inline__))
#endif

#define SGRX_CAST( t, to, from ) t to = (t) from
#define UNUSED( x ) (void) x


#define SMALL_FLOAT 0.001f
#define ENGINE_MAX_PATH 256

#define DEG2RAD( x ) ((x)/180.0f*M_PI)
#define RAD2DEG( x ) ((x)*180.0f/M_PI)

#define COLOR_RGBA(r,g,b,a) ((uint32_t)((((int)(a)&0xff)<<24)|(((int)(b)&0xff)<<16)|(((int)(g)&0xff)<<8)|((int)(r)&0xff)))
#define COLOR_RGB(r,g,b,a) COLOR_RGBA(r,g,b,0xff)
#define COLOR_EXTRACT_( c, off ) (((c)>>(off))&0xff)
#define COLOR_EXTRACT_R( c ) COLOR_EXTRACT_( c, 0 )
#define COLOR_EXTRACT_G( c ) COLOR_EXTRACT_( c, 8 )
#define COLOR_EXTRACT_B( c ) COLOR_EXTRACT_( c, 16 )
#define COLOR_EXTRACT_A( c ) COLOR_EXTRACT_( c, 24 )

inline size_t divideup( size_t x, int d ){ return ( x + d - 1 ) / d; }


//
// TEMPLATES
//

template< class T > T TMIN( const T& a, const T& b ){ return a < b ? a : b; }
template< class T > T TMAX( const T& a, const T& b ){ return a > b ? a : b; }
template< class T > void TMEMSET( T* a, size_t c, const T& v )
{
	for( size_t i = 0; i < c; ++i )
		a[ i ] = v;
}


//
// VEC3
//

struct Vec3
{
	float x, y, z;
	
#ifdef USE_VEC3
	static FINLINE Vec3 Create( float x ){ Vec3 v = { x, x, x }; return v; }
	static FINLINE Vec3 Create( float x, float y, float z ){ Vec3 v = { x, y, z }; return v; }
	static FINLINE Vec3 CreateFromPtr( const float* x ){ Vec3 v = { x[0], x[1], x[2] }; return v; }
	
	FINLINE Vec3 operator + () const { return *this; }
	FINLINE Vec3 operator - () const { Vec3 v = { -x, -y, -z }; return v; }
	
	FINLINE Vec3 operator + ( const Vec3& o ) const { Vec3 v = { x + o.x, y + o.y, z + o.z }; return v; }
	FINLINE Vec3 operator - ( const Vec3& o ) const { Vec3 v = { x - o.x, y - o.y, z - o.z }; return v; }
	FINLINE Vec3 operator * ( const Vec3& o ) const { Vec3 v = { x * o.x, y * o.y, z * o.z }; return v; }
	FINLINE Vec3 operator / ( const Vec3& o ) const { Vec3 v = { x / o.x, y / o.y, z / o.z }; return v; }
	
	FINLINE Vec3 operator + ( float f ) const { Vec3 v = { x + f, y + f, z + f }; return v; }
	FINLINE Vec3 operator - ( float f ) const { Vec3 v = { x - f, y - f, z - f }; return v; }
	FINLINE Vec3 operator * ( float f ) const { Vec3 v = { x * f, y * f, z * f }; return v; }
	FINLINE Vec3 operator / ( float f ) const { Vec3 v = { x / f, y / f, z / f }; return v; }
	
	FINLINE Vec3& operator += ( const Vec3& o ){ x += o.x; y += o.y; z += o.z; return *this; }
	FINLINE Vec3& operator -= ( const Vec3& o ){ x -= o.x; y -= o.y; z -= o.z; return *this; }
	FINLINE Vec3& operator *= ( const Vec3& o ){ x *= o.x; y *= o.y; z *= o.z; return *this; }
	FINLINE Vec3& operator /= ( const Vec3& o ){ x /= o.x; y /= o.y; z /= o.z; return *this; }
	
	FINLINE Vec3& operator += ( float f ){ x += f; y += f; z += f; return *this; }
	FINLINE Vec3& operator -= ( float f ){ x -= f; y -= f; z -= f; return *this; }
	FINLINE Vec3& operator *= ( float f ){ x *= f; y *= f; z *= f; return *this; }
	FINLINE Vec3& operator /= ( float f ){ x /= f; y /= f; z /= f; return *this; }
	
	FINLINE bool operator == ( const Vec3& o ) const { return x == o.x && y == o.y && z == o.z; }
	FINLINE bool operator != ( const Vec3& o ) const { return x != o.x || y != o.y || z != o.z; }
	
	FINLINE bool IsZero() const { return x == 0 && y == 0 && z == 0; }
	FINLINE bool NearZero() const { return fabs(x) < SMALL_FLOAT && fabs(y) < SMALL_FLOAT && fabs(z) < SMALL_FLOAT; }
	FINLINE float LengthSq() const { return x * x + y * y + z * z; }
	FINLINE float Length() const { return sqrtf( LengthSq() ); }
	FINLINE Vec3 Normalized() const
	{
		float lensq = LengthSq();
		if( lensq == 0 )
		{
			Vec3 v = { 0, 0, 0 };
			return v;
		}
		float invlen = 1.0f / sqrtf( lensq );
		Vec3 v = { x * invlen, y * invlen, z * invlen };
		return v;
	}
#endif
};

#ifdef USE_VEC3
FINLINE Vec3 operator + ( float f, const Vec3& v ){ Vec3 out = { f + v.x, f + v.y, f + v.z }; return out; }
FINLINE Vec3 operator - ( float f, const Vec3& v ){ Vec3 out = { f - v.x, f - v.y, f - v.z }; return out; }
FINLINE Vec3 operator * ( float f, const Vec3& v ){ Vec3 out = { f * v.x, f * v.y, f * v.z }; return out; }
FINLINE Vec3 operator / ( float f, const Vec3& v ){ Vec3 out = { f / v.x, f / v.y, f / v.z }; return out; }

FINLINE float Vec3Dot( const Vec3& v1, const Vec3& v2 ){ return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
FINLINE Vec3 Vec3Cross( const Vec3& v1, const Vec3& v2 )
{
	Vec3 out =
	{
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x,
	};
	return out;
}
#endif


//
// MAT4
//

struct Mat4
{
	union
	{
		float a[16];
		float m[4][4];
	};
	
#ifdef USE_MAT4
	void SetIdentity()
	{
		for( int i = 0; i < 4; ++i )
			for( int j = 0; j < 4; ++j )
				m[i][j] = i == j;
	}
	static Mat4 CreateIdentity()
	{
		Mat4 m;
		m.SetIdentity();
		return m;
	}
	
	FINLINE Vec3 Transform( const Vec3& v, float w ) const
	{
		Vec3 out =
		{
			v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + m[3][0] * w,
			v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + m[3][1] * w,
			v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + m[3][2] * w,
		};
		return out;
	}
	FINLINE Vec3 TransformPos( const Vec3& v ) const
	{
		Vec3 out =
		{
			v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + m[3][0],
			v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + m[3][1],
			v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + m[3][2],
		};
		float q = 1.0f / ( v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + m[3][3] );
		return out * q;
	}
	FINLINE Vec3 TransformNormal( const Vec3& nrm ) const { return Transform( nrm, 0.0f ); }
	
	bool InvertTo( Mat4& out );
	FINLINE void Transpose()
	{
		std::swap( m[1][0], m[0][1] );
		std::swap( m[2][0], m[0][2] );
		std::swap( m[3][0], m[0][3] );
		std::swap( m[2][1], m[1][2] );
		std::swap( m[3][1], m[1][3] );
		std::swap( m[3][2], m[2][3] );
	}
	void GenNormalMatrix( Mat4& out ) const
	{
		out.m[0][0] = m[0][0]; out.m[0][1] = m[0][1]; out.m[0][2] = m[0][2];
		out.m[1][0] = m[1][0]; out.m[1][1] = m[1][1]; out.m[1][2] = m[1][2];
		out.m[2][0] = m[2][0]; out.m[2][1] = m[2][1]; out.m[2][2] = m[2][2];
		out.m[0][3] = out.m[1][3] = out.m[2][3] = 0;
		out.m[3][0] = out.m[3][1] = out.m[3][2] = 0;
		out.m[3][3] = 1;
		
		out.InvertTo( out );
		out.Transpose();
	}
	
	void Multiply( const Mat4& A, const Mat4& B )
	{
		m[0][0] = A.m[0][0] * B.m[0][0] + A.m[0][1] * B.m[1][0] + A.m[0][2] * B.m[2][0] + A.m[0][3] * B.m[3][0];
		m[0][1] = A.m[0][0] * B.m[0][1] + A.m[0][1] * B.m[1][1] + A.m[0][2] * B.m[2][1] + A.m[0][3] * B.m[3][1];
		m[0][2] = A.m[0][0] * B.m[0][2] + A.m[0][1] * B.m[1][2] + A.m[0][2] * B.m[2][2] + A.m[0][3] * B.m[3][2];
		m[0][3] = A.m[0][0] * B.m[0][3] + A.m[0][1] * B.m[1][3] + A.m[0][2] * B.m[2][3] + A.m[0][3] * B.m[3][3];
		m[1][0] = A.m[1][0] * B.m[0][0] + A.m[1][1] * B.m[1][0] + A.m[1][2] * B.m[2][0] + A.m[1][3] * B.m[3][0];
		m[1][1] = A.m[1][0] * B.m[0][1] + A.m[1][1] * B.m[1][1] + A.m[1][2] * B.m[2][1] + A.m[1][3] * B.m[3][1];
		m[1][2] = A.m[1][0] * B.m[0][2] + A.m[1][1] * B.m[1][2] + A.m[1][2] * B.m[2][2] + A.m[1][3] * B.m[3][2];
		m[1][3] = A.m[1][0] * B.m[0][3] + A.m[1][1] * B.m[1][3] + A.m[1][2] * B.m[2][3] + A.m[1][3] * B.m[3][3];
		m[2][0] = A.m[2][0] * B.m[0][0] + A.m[2][1] * B.m[1][0] + A.m[2][2] * B.m[2][0] + A.m[2][3] * B.m[3][0];
		m[2][1] = A.m[2][0] * B.m[0][1] + A.m[2][1] * B.m[1][1] + A.m[2][2] * B.m[2][1] + A.m[2][3] * B.m[3][1];
		m[2][2] = A.m[2][0] * B.m[0][2] + A.m[2][1] * B.m[1][2] + A.m[2][2] * B.m[2][2] + A.m[2][3] * B.m[3][2];
		m[2][3] = A.m[2][0] * B.m[0][3] + A.m[2][1] * B.m[1][3] + A.m[2][2] * B.m[2][3] + A.m[2][3] * B.m[3][3];
		m[3][0] = A.m[3][0] * B.m[0][0] + A.m[3][1] * B.m[1][0] + A.m[3][2] * B.m[2][0] + A.m[3][3] * B.m[3][0];
		m[3][1] = A.m[3][0] * B.m[0][1] + A.m[3][1] * B.m[1][1] + A.m[3][2] * B.m[2][1] + A.m[3][3] * B.m[3][1];
		m[3][2] = A.m[3][0] * B.m[0][2] + A.m[3][1] * B.m[1][2] + A.m[3][2] * B.m[2][2] + A.m[3][3] * B.m[3][2];
		m[3][3] = A.m[3][0] * B.m[0][3] + A.m[3][1] * B.m[1][3] + A.m[3][2] * B.m[2][3] + A.m[3][3] * B.m[3][3];
	}
	
	void Scale( Vec3 scale )
	{
		SetIdentity();
		m[0][0] = scale.x;
		m[1][1] = scale.y;
		m[2][2] = scale.z;
	}
	
	void LookAt( Vec3 pos, Vec3 dir, Vec3 up )
	{
		Vec3 zaxis, xaxis, yaxis;
		zaxis = dir.Normalized();
		xaxis = Vec3Cross( up, zaxis ).Normalized();
		yaxis = Vec3Cross( zaxis, xaxis );
		
		m[0][0] = xaxis.x; m[0][1] = yaxis.x; m[0][2] = zaxis.x; m[0][3] = 0;
		m[1][0] = xaxis.y; m[1][1] = yaxis.y; m[1][2] = zaxis.y; m[1][3] = 0;
		m[2][0] = xaxis.z; m[2][1] = yaxis.z; m[2][2] = zaxis.z; m[2][3] = 0;
		m[3][0] = -Vec3Dot( xaxis, pos );
		m[3][1] = -Vec3Dot( yaxis, pos );
		m[3][2] = -Vec3Dot( zaxis, pos );
		m[3][3] = 1;
	}
	
	void Perspective( float angle, float aspect, float aamix, float znear, float zfar )
	{
		float tha = tan( DEG2RAD( angle ) / 2.0f );
		if( tha < 0.001f ) tha = 0.001f;
		float itha = 1.0f / tha;
		float xscale = -itha / pow( aspect, aamix );
		float yscale = itha * pow( aspect, 1 - aamix );
		
		m[0][0] = xscale;
		m[0][1] = m[0][2] = m[0][3] = 0;
		m[1][1] = yscale;
		m[1][0] = m[1][2] = m[1][3] = 0;
		m[2][2] = zfar / ( zfar - znear );
		m[2][0] = m[2][1] = 0; m[2][3] = 1;
		m[3][2] = -znear * zfar / ( zfar - znear );
		m[3][0] = m[3][1] = m[3][3] = 0;
	}
#endif
};


//
// HANDLE
//

template< class T >
struct Handle
{
	typedef T subtype;
	T* item;
	
	Handle() : item( NULL ){}
	Handle( T* v ) : item( v ){ if( item ) item->Acquire(); }
	Handle( const Handle& h ) : item( h.item ){ if( item ) item->Acquire(); }
	~Handle(){ if( item ){ item->Release(); item = NULL; } }
	Handle& operator = ( const Handle& h ){ if( item ) item->Release(); item = h.item; if( item ) item->Acquire(); }
	T* operator -> () const { return item; }
	operator T* () const { return item; }
};


//
// ARRAY
//

#define NOT_FOUND ((size_t)-1)

template< class T >
struct Array
{
	typedef T subtype;
	
	T* m_data;
	size_t m_size;
	size_t m_mem;
	
#ifdef USE_ARRAY
	Array() : m_data(NULL), m_size(0), m_mem(0){}
	Array( const T* v, size_t sz ) : m_data(NULL), m_size(0), m_mem(0) { insert( 0, v, sz ); }
	~Array(){ free( m_data ); }
	
	FINLINE size_t size() const { return m_size; }
	FINLINE size_t capacity() const { return m_mem; }
	FINLINE const T* data() const { return m_data; }
	FINLINE T* data(){ return m_data; }
	
	FINLINE T& front(){ ASSERT( m_size ); return m_data[0]; }
	FINLINE const T& front() const { ASSERT( m_size ); return m_data[0]; }
	FINLINE T& back(){ ASSERT( m_size ); return m_data[ m_size - 1 ]; }
	FINLINE const T& back() const { ASSERT( m_size ); return m_data[ m_size - 1 ]; }
	
	FINLINE T& operator [] ( size_t i ){ ASSERT( i < m_size ); return m_data[ i ]; }
	FINLINE const T& operator [] ( size_t i ) const { ASSERT( i < m_size ); return m_data[ i ]; }
	FINLINE T& at( size_t i ){ ASSERT( i < m_size ); return m_data[ i ]; }
	FINLINE const T& at( size_t i ) const { ASSERT( i < m_size ); return m_data[ i ]; }
	
	FINLINE bool has( const T& v ){ return find_first_at( v ) != NOT_FOUND; }
	bool remove_first( const T& v, size_t from = 0 )
	{
		for( size_t i = from; i < m_size; ++i )
			if( v == m_data[ i ] )
			{
				erase( i );
				return true;
			}
		return false;
	}
	bool remove_all( const T& v )
	{
		bool removed = false;
		for( size_t i = 0; i < m_size; ++i )
			if( v == m_data[ i ] )
			{
				erase( i-- );
				removed = true;
			}
		return removed;
	}
	
	FINLINE void push_front( const T& v ){ insert( 0, v ); }
	FINLINE void push_back( const T& v ){ insert( size(), v ); }
	FINLINE void append( const T* v, size_t sz ){ insert( size(), v, sz ); }
	FINLINE void clear(){ resize( 0 ); }
	
	FINLINE void insert( size_t at, const T& v ){ insert( at, &v, 1 ); }
	void erase( size_t from, size_t count = 1 );
	void resize( size_t sz );
	void reserve( size_t sz );
	void insert( size_t at, const T* v, size_t count );
	
	size_t find_first_at( const T& what, size_t from = 0 ) const
	{
		for( size_t i = from; i < m_size; ++i )
			if( what == m_data[ i ] )
				return i;
		return NOT_FOUND;
	}
	T* find_first_ptr( const T& what, size_t from = 0 ) const
	{
		for( size_t i = from; i < m_size; ++i )
			if( what == m_data[ i ] )
				return &m_data[ i ];
		return NULL;
	}
#endif
};

#ifdef USE_ARRAY
template< class T >
void Array<T>::resize( size_t sz )
{
	reserve( sz );
	while( sz > m_size )
		new (&m_data[ m_size++ ]) T();
	while( sz < m_size )
		m_data[ --m_size ].~T();
}

template< class T >
void Array<T>::reserve( size_t sz )
{
	if( sz <= m_mem )
		return;
	m_mem = sz;
	T* newdata = (T*) malloc( sizeof(T) * sz );
	if( m_data )
	{
		for( size_t i = 0; i < m_size; ++i )
		{
			new (&newdata[ i ]) T( m_data[ i ] );
			m_data[ i ].~T();
		}
		free( m_data );
	}
	m_data = newdata;
}

template< class T >
void Array<T>::insert( size_t at, const T* v, size_t count )
{
	if( m_mem < m_size + count )
		reserve( TMAX( m_size + count, m_mem * 2 ) );
	if( at < m_size )
	{
		size_t i = m_size + count;
		while( i > m_size )
		{
			i--;
			new (&m_data[ i ]) T( m_data[ i - count ] );
			m_data[ i - count ].~T();
		}
	}
	for( size_t i = 0; i < count; ++i )
		new (&m_data[ i + at ]) T( v[ i ] );
	m_size += count;
}

template< class T >
void Array<T>::erase( size_t from, size_t count )
{
	ASSERT( from < m_size );
	ASSERT( from + count <= m_size );
	for( size_t i = 0; i < count; ++i )
		m_data[ from + i ].~T();
	for( size_t i = from + count; i < m_size; ++i )
	{
		new (&m_data[ i - count ]) T( m_data[ i ] );
		m_data[ i ].~T();
	}
	m_size -= count;
}
#endif

typedef Array< uint8_t > ByteArray;


//
// STRINGS
//

typedef Array< char > String;

FINLINE size_t StringLength( const char* str ){ const char* o = str; while( *str ) str++; return str - o; }

struct StringView
{
	const char* m_str;
	size_t m_size;
	
	StringView() : m_str( NULL ), m_size( 0 ){}
	StringView( const char* str ) : m_str( str ), m_size( StringLength( str ) ){}
	StringView( const char* str, size_t size ) : m_str( str ), m_size( size ){}
	StringView( const String& str ) : m_str( str.m_data ), m_size( str.m_size ){}
	StringView( const StringView& sv ) : m_str( sv.m_str ), m_size( sv.m_size ){}
	
	FINLINE const char* data() const { return m_str; }
	FINLINE size_t size() const { return m_size; }
	FINLINE operator bool() const { return m_str && m_size; }
	
	FINLINE bool contains( const StringView& substr ) const { return find_first_at( substr ) != NOT_FOUND; }
	size_t find_first_at( const StringView& substr, size_t defval = NOT_FOUND ) const
	{
		if( substr.m_size > m_size )
			return defval;
		for( size_t i = 0; i < m_size - substr.m_size; ++i )
			if( !memcmp( m_str + i, substr.m_str, substr.m_size ) )
				return i;
		return defval;
	}
	
	FINLINE StringView until( const StringView& substr ) const
	{
		size_t pos = find_first_at( substr, 0 );
		return StringView( m_str + pos, m_size - pos );
	}
	FINLINE StringView from( const StringView& substr ) const
	{
		size_t pos = find_first_at( substr, 0 );
		return StringView( m_str, pos );
	}
	FINLINE bool skip( size_t n )
	{
		if( n > m_size );
		{
			m_str += m_size;
			m_size = 0;
			return false;
		}
		m_str += n;
		m_size -= n;
		return true;
	}
	
#ifdef USE_ARRAY
	FINLINE operator String () const { return String( m_str, m_size ); }
#endif
};

template< size_t N >
struct StackString
{
	char str[ N + 1 ];
	
	StackString( const StringView& sv ){ size_t sz = TMIN( sv.m_size, N ); if( sz ) memcpy( str, sv.m_str, sz ); str[ sz ] = 0; }
	operator const char* (){ return str; }
};


//
// UNICODE
//
// - utf8_decode: returns number of bytes parsed (negated if input was invalid)
// - utf8_encode: returns number of bytes written (up to 4, make sure there's space)
//

#define UNICODE_INVCHAR 0xfffd
#define UNICODE_INVCHAR_STR "\xef\xbf\xbd"
#define UNICODE_INVCHAR_LEN 3
EXPORT int UTF8Decode( const char* buf, size_t size, uint32_t* outchar );
EXPORT int UTF8Encode( uint32_t ch, char* out );


//
// HASH TABLE
//

typedef uint32_t Hash;
Hash HashFunc( const char* str, size_t size );

inline Hash HashVar( const String& s ){ return HashFunc( s.m_data, s.m_size ); }
inline Hash HashVar( const StringView& sv ){ return HashFunc( sv.m_str, sv.m_size ); }

template< class K, class V >
struct HashTable
{
	typedef int32_t size_type;
	typedef K key_type;
	typedef V value_type;
	
	struct Var
	{
		Hash hash;
		K key;
		V value;
	};
	
	size_type* m_pairs;
	Var* m_vars;
	size_type m_pair_mem;
	size_type m_var_mem;
	size_type m_size;
	size_type m_num_removed;
	
#ifdef USE_HASHTABLE
	// special pair IDs
	enum { EMPTY = -1, REMOVED = -2 };
	
	HashTable( size_type initial_mem = 4 ) : m_pair_mem( initial_mem ), m_var_mem( initial_mem ), m_size( 0 ), m_num_removed( 0 )
	{
		m_pairs = new size_type[ m_pair_mem ];
		m_vars = (Var*) malloc( sizeof(Var) * m_var_mem );
		TMEMSET( m_pairs, m_pair_mem, (size_type) EMPTY );
	}
	~HashTable()
	{
		for( size_type i = 0; i < m_size; ++i )
		{
			m_vars[ i ].key.~K();
			m_vars[ i ].value.~V();
		}
		delete [] m_pairs;
		free( m_vars );
	}
	
	void clear()
	{
		for( size_type i = 0; i < m_size; ++i )
		{
			m_vars[ i ].key.~K();
			m_vars[ i ].value.~V();
		}
		m_num_removed += m_size;
		m_size = 0;
		TMEMSET( m_pairs, m_pair_mem, (size_type) EMPTY );
	}
	
	FINLINE size_t size() const { return m_size; }
	FINLINE Var& item( size_t i ){ ASSERT( i < m_size ); return m_vars[ i ]; }
	FINLINE V getcopy( const K& key, const V& defval = V() ) const { const Var* raw = getraw( key ); return raw ? raw->value : defval; }
	FINLINE V* getptr( const K& key, const V* defval = NULL ){ Var* raw = getraw( key ); return raw ? raw->value : defval; }
	FINLINE V& operator [] ( const K& key ){ Var* raw = getraw( key ); if( !raw ) raw = set( key, V() ); return raw->value; }
	
	size_type _get_pair_id( const K& key, Hash hash )
	{
		size_type i, sp = (size_type)( hash % (Hash) m_pair_mem );
		i = sp;
		do
		{
			size_type idx = m_pairs[ i ];
			if( idx == EMPTY )
				break;
			if( idx != REMOVED && m_vars[ idx ].key == key )
				return i;
			i++;
			if( i >= m_pair_mem )
				i = 0;
		}
		while( i != sp );
		return -1;
	}
	Var* getraw( const K& key )
	{
		size_type i = _get_pair_id( key, HashVar( key ) );
		if( i >= 0 )
			return &m_vars[ m_pairs[ i ] ];
		else
			return NULL;
	}
	const Var* getraw( const K& key ) const
	{
		size_type i = _get_pair_id( key, HashVar( key ) );
		if( i >= 0 )
			return &m_vars[ m_pairs[ i ] ];
		else
			return NULL;
	}
	Var* set( const K& key, const V& val )
	{
		Hash h = HashVar( key );
		size_type sp, i = _get_pair_id( key, h );
		if( i >= 0 )
		{
			Var& var = m_vars[ m_pairs[ i ] ];
			var.value.~V();
			new (&var.value) V( val );
			return &var;
		}
		else
		{
			size_type osize = m_size;
			UNUSED( osize );
			
			/* prefer to rehash if too many removed (num_rem) items are found */
			if( m_size + m_num_removed + 1.0 >= m_pair_mem * 0.7 )
				rehash( (size_type) TMAX<double>( m_pair_mem * 1.5, m_size + 16 ) );
			if( m_size >= m_var_mem )
				reserve( (size_type) TMAX<double>( m_size * 1.5, m_size + 16 ) );
			
			{
				Var& var = m_vars[ m_size ];
				new (&var.key) K( key );
				new (&var.value) V( val );
				var.hash = h;
			}
			
			sp = i = (size_type)( h % (Hash) m_pair_mem );
			do
			{
				size_type idx = m_pairs[ i ];
				if( idx == EMPTY || idx == REMOVED )
				{
					if( idx == REMOVED )
						m_num_removed--;
					m_pairs[ i ] = m_size;
					m_size++;
					break;
				}
				i++;
				if( i >= m_pair_mem )
					i = 0;
			}
			while( i != sp );
			
			assert( m_size != osize );
			
			return &m_vars[ m_size - 1 ];
		}
	}
	bool unset( const K& key )
	{
		Hash h = HashVar( key );
		size_type i = _get_pair_id( key, h );
		if( i >= 0 )
		{
			size_type idx = m_pairs[ i ];
			Var* p = &m_vars[ idx ];
			p->key.~K();
			p->value.~V();
			
			m_pairs[ i ] = REMOVED;
			
			m_num_removed++;
			m_size--;
			if( p < m_vars + m_size )
			{
				Var* ep = m_vars + m_size;
				i = _get_pair_id( &ep->key, ep->hash );
				assert( i != -1 );
				
				new (&p->key) K( ep->key );
				new (&p->value) V( ep->value );
				p->hash = ep->hash;
				
				m_pairs[ i ] = idx;
			}
			
			// for some reason originally old item was deleted after swap
		}
		
		if( m_num_removed > m_var_mem * 0.25 + 16 )
		{
			reserve( (size_type) ( m_size * 0.75 + m_var_mem * 0.25 ) );
			rehash( (size_type) ( m_size * 0.5 + m_var_mem * 0.5 ) );
		}
	}
	void rehash( size_type size )
	{
		Hash h;
		size_type i, si, sp, idx, *np;
		ASSERT( size >= m_size );
		
		if( size == m_pair_mem )
			return;
		if( size < 4 )
			size = 4;
		
		np = new size_type[ size ];
		memset( np, EMPTY, sizeof(size_type) * (size_t) size );
		
	#if 0
		printf( "rehash %d -> %d (size = %d, mem = %d kB)\n", m_pair_mem, size, m_size,
			(size * sizeof(size_type) + T->var_mem * sizeof(Var)) / 1024 );
	#endif
		
		for( si = 0; si < m_pair_mem; ++si )
		{
			idx = m_pairs[ si ];
			if( idx >= 0 )
			{
				h = m_vars[ idx ].hash;
				sp = i = (size_type)( h % (Hash) size );
				do
				{
					size_type nidx = np[ i ];
					if( nidx == EMPTY )
					{
						np[ i ] = idx;
						break;
					}
					i++;
					if( i >= size )
						i = 0;
				}
				while( i != sp );
			}
		}
		
		delete [] m_pairs;
		m_pairs = np;
		m_pair_mem = size;
		m_num_removed = 0;
	}
	void reserve( size_type size )
	{
		Var* p;
		
		ASSERT( size >= m_size );
		
		if( size == m_var_mem )
			return;
		if( size < 4 )
			size = 4;
		
	#if 0
		printf( "reserve %d -> %d (size = %d, mem = %d kB)\n", m_var_mem, size, m_size,
			(m_pair_mem * sizeof(size_type) + size * sizeof(Var)) / 1024 );
	#endif
		
		/* WP: hash table limit */
		p = (Var*) malloc( sizeof(Var) * size );
		for( size_type i = 0; i < m_size; ++i )
		{
			new (&p[ i ].key) K( m_vars[ i ].key );
			new (&p[ i ].value) V( m_vars[ i ].value );
			m_vars[ i ].key.~K();
			m_vars[ i ].value.~V();
		}
		free( m_vars );
		m_vars = p;
		m_var_mem = size;
	}
#endif
};


//
// FILES
//

bool LoadBinaryFile( const char* path, ByteArray& out );

