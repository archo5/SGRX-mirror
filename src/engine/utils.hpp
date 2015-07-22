

#pragma once
#define __STDC_FORMAT_MACROS 1
#define _USE_MATH_DEFINES
#include <inttypes.h>
#include <float.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <new>


#define USE_VEC3
#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY


#define EXPORT __declspec(dllexport)
#ifdef ENGINE_BUILDING
#  define ENGINE_EXPORT __declspec(dllexport)
#else
#  define ENGINE_EXPORT __declspec(dllimport)
#endif


ENGINE_EXPORT void sgrx_assert_func( const char* code, const char* file, int line );
#define ASSERT( x ) do{ if(!(x)) sgrx_assert_func( #x, __FILE__, __LINE__ ); }while(0)
#ifdef _MSC_VER
#define IF_GCC(x)
#define IF_MSVC(x) x
#define FINLINE __forceinline
#define ALIGN16(a) __declspec(align(16)) a
#else
#define IF_GCC(x) x
#define IF_MSVC(x)
#define FINLINE inline __attribute__((__always_inline__))
#define ALIGN16(a) a __attribute__ ((aligned (16)))
#endif

#ifndef THREAD_LOCAL
# if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
#  define THREAD_LOCAL _Thread_local
# elif defined _WIN32 && ( \
       defined _MSC_VER || \
       defined __ICL || \
       defined __DMC__ || \
       defined __BORLANDC__ )
#  define THREAD_LOCAL __declspec(thread) 
/* note that ICC (linux) and Clang are covered by __GNUC__ */
# elif defined __GNUC__ || \
       defined __SUNPRO_C || \
       defined __xlC__
#  define THREAD_LOCAL __thread
# else
#  error "Cannot define THREAD_LOCAL"
# endif
#endif

#define SGRX_CAST( t, to, from ) t to = (t) from
#define SGRX_ARRAY_SIZE( arr ) (sizeof(arr)/sizeof((arr)[0]))
#ifndef UNUSED
#  define UNUSED( x ) (void) x
#endif
#define STRLIT_LEN( x ) (sizeof(x)-1)
#define STRLIT_BUF( x ) x, STRLIT_LEN( x )


// compiler/toolchain padding
ENGINE_EXPORT void NOP( int x );
ENGINE_EXPORT int sgrx_sncopy( char* buf, size_t len, const char* str, size_t ilen = (size_t) -1 );
ENGINE_EXPORT int sgrx_snprintf( char* buf, size_t len, const char* fmt, ... );
ENGINE_EXPORT uint32_t sgrx_crc32( const void* buf, size_t len, uint32_t in_crc );
ENGINE_EXPORT void sgrx_quicksort( void* array, size_t length, size_t size,
	int(*compare)(const void*, const void*, void*), void* userdata );
ENGINE_EXPORT void sgrx_combsort( void* array, size_t length, size_t size,
	bool(*compless)(const void*, const void*, void*), void* userdata );

// RT system padding
ENGINE_EXPORT double sgrx_hqtime();


#define SMALL_FLOAT 0.001f
#define ENGINE_MAX_PATH 256

#define SPACE_CHARS " \t\n"
#define HSPACE_CHARS " \t"

#define DEG2RAD( x ) ((x)/180.0f*(float)M_PI)
#define RAD2DEG( x ) ((x)*180.0f/(float)M_PI)

#define COLOR_F2B( x ) (uint8_t( clamp( x, 0, 1 ) * 255 ))
#define COLOR_RGBA(r,g,b,a) ((uint32_t)((((int)(a)&0xff)<<24)|(((int)(b)&0xff)<<16)|(((int)(g)&0xff)<<8)|((int)(r)&0xff)))
#define COLOR_RGB(r,g,b) COLOR_RGBA(r,g,b,0xff)
#define COLOR_EXTRACT_( c, off ) (((c)>>(off))&0xff)
#define COLOR_EXTRACT_R( c ) COLOR_EXTRACT_( c, 0 )
#define COLOR_EXTRACT_G( c ) COLOR_EXTRACT_( c, 8 )
#define COLOR_EXTRACT_B( c ) COLOR_EXTRACT_( c, 16 )
#define COLOR_EXTRACT_A( c ) COLOR_EXTRACT_( c, 24 )

inline size_t divideup( size_t x, int d ){ return ( x + d - 1 ) / d; }

#define IIF( cond, a, b ) ((cond)?(a):(b))
inline float clamp( float v, float vmin, float vmax ){ return IIF( v < vmin, vmin, IIF( v > vmax, vmax, v ) ); }
inline float lerp( float a, float b, float t ){ return a * (1.0f-t) + b * t; }
inline float sign( float x ){ return IIF( x == 0.0f, 0.0f, IIF( x < 0.0f, -1.0f, 1.0f ) ); }
FINLINE int safe_idiv( int x, int y ){ if( y == 0 ) return 0; return x / y; }
FINLINE float safe_fdiv( float x, float y ){ if( y == 0 ) return 0; return x / y; }
inline float normalize_angle( float x ){ x = fmodf( x, (float) M_PI * 2.0f ); return IIF( x < 0.0f, x + (float) M_PI*2.0f, x ); }
inline float normalize_angle2( float x ){ x = normalize_angle( x ); if( x >= M_PI ) x -= M_PI * 2; return x; }
inline float saturate( float x ){ return IIF( x < 0.0f, 0.0f, IIF( x > 1.0f, 1.0f, x ) ); }
inline float smoothstep( float x ){ return x * x * ( 3.0f - 2.0f * x ); }
inline float smoothlerp_oneway( float t, float a, float b ){ if( b == a ) return 1.0f; return smoothstep( saturate( ( t - a ) / ( b - a ) ) ); }
inline float smoothlerp_range( float t, float a, float b, float c, float d ){ return smoothlerp_oneway( t, a, b ) * smoothlerp_oneway( t, d, c ); }
inline float randf(){ return (float) rand() / (float) RAND_MAX; }
inline float randf11(){ return randf() * 2.0f - 1.0f; }


inline bool hexchar( char c )
	{ return ( (c) >= '0' && (c) <= '9' ) ||
	( (c) >= 'a' && (c) <= 'f' ) || ( (c) >= 'A' && (c) <= 'F' ); }
inline int gethex( char c )
	{ return ( (c) >= '0' && (c) <= '9' ) ? ( (c) - '0' ) :
	( ( (c) >= 'a' && (c) <= 'f' ) ? ( (c) - 'a' + 10 ) : ( (c) - 'A' + 10 ) ); }
inline bool decchar( char c ){ return c >= '0' && c <= '9'; }
inline int getdec( char c ){ return c - '0'; }
inline bool octchar( char c ){ return c >= '0' && c <= '7'; }
inline int getoct( char c ){ return c - '0'; }
inline bool binchar( char c ){ return c == '0' || c == '1'; }
inline int getbin( char c ){ return c - '0'; }


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
template< class T > void TSWAP( T& a, T& b ){ T tmp( a ); a = b; b = tmp; }
template< class T > void TMEMSWAP( T& a, T& b )
{
	char tmp[ sizeof(T) ];
	memcpy( tmp, &a, sizeof(T) );
	memcpy( &a, &b, sizeof(T) );
	memcpy( &b, tmp, sizeof(T) );
}
template< class T, class S > T TLERP( const T& a, const T& b, const S& s ){ return a * ( S(1) - s ) + b * s; }
template< class S, class T > S TREVLERP( const T& a, const T& b, const T& s ){ if( a == b ) return a; return ( s - a ) / ( b - a ); }

template< class T > struct IVState
{
	FINLINE IVState( const T& start ) : prev(start), curr(start) {}
	FINLINE void Advance( const T& next ){ prev = curr; curr = next; }
	FINLINE T Get( float q ){ return TLERP( prev, curr, q ); }
	
	T prev;
	T curr;
};


//
// VEC2
//

struct ENGINE_EXPORT Vec2
{
	float x, y;
	
	static FINLINE Vec2 Create( float x ){ Vec2 v = { x, x }; return v; }
	static FINLINE Vec2 Create( float x, float y ){ Vec2 v = { x, y }; return v; }
	static FINLINE Vec2 CreateFromAngle( float a, float d = 1.0f ){ Vec2 v = { cosf( a ) * d, sinf( a ) * d }; return v; }
	static FINLINE Vec2 CreateFromPtr( const float* x ){ Vec2 v = { x[0], x[1] }; return v; }
	static FINLINE Vec2 Min( const Vec2& a, const Vec2& b ){ return Create( TMIN( a.x, b.x ), TMIN( a.y, b.y ) ); }
	static FINLINE Vec2 Max( const Vec2& a, const Vec2& b ){ return Create( TMAX( a.x, b.x ), TMAX( a.y, b.y ) ); }
	
	FINLINE Vec2 operator + () const { return *this; }
	FINLINE Vec2 operator - () const { Vec2 v = { -x, -y }; return v; }
	
	FINLINE Vec2 operator + ( const Vec2& o ) const { Vec2 v = { x + o.x, y + o.y }; return v; }
	FINLINE Vec2 operator - ( const Vec2& o ) const { Vec2 v = { x - o.x, y - o.y }; return v; }
	FINLINE Vec2 operator * ( const Vec2& o ) const { Vec2 v = { x * o.x, y * o.y }; return v; }
	FINLINE Vec2 operator / ( const Vec2& o ) const { Vec2 v = { x / o.x, y / o.y }; return v; }
	
	FINLINE Vec2 operator + ( float f ) const { Vec2 v = { x + f, y + f }; return v; }
	FINLINE Vec2 operator - ( float f ) const { Vec2 v = { x - f, y - f }; return v; }
	FINLINE Vec2 operator * ( float f ) const { Vec2 v = { x * f, y * f }; return v; }
	FINLINE Vec2 operator / ( float f ) const { Vec2 v = { x / f, y / f }; return v; }
	
	FINLINE Vec2& operator += ( const Vec2& o ){ x += o.x; y += o.y; return *this; }
	FINLINE Vec2& operator -= ( const Vec2& o ){ x -= o.x; y -= o.y; return *this; }
	FINLINE Vec2& operator *= ( const Vec2& o ){ x *= o.x; y *= o.y; return *this; }
	FINLINE Vec2& operator /= ( const Vec2& o ){ x /= o.x; y /= o.y; return *this; }
	
	FINLINE Vec2& operator += ( float f ){ x += f; y += f; return *this; }
	FINLINE Vec2& operator -= ( float f ){ x -= f; y -= f; return *this; }
	FINLINE Vec2& operator *= ( float f ){ x *= f; y *= f; return *this; }
	FINLINE Vec2& operator /= ( float f ){ x /= f; y /= f; return *this; }
	
	FINLINE bool operator == ( const Vec2& o ) const { return x == o.x && y == o.y; }
	FINLINE bool operator != ( const Vec2& o ) const { return x != o.x || y != o.y; }
	
	FINLINE bool IsZero() const { return x == 0 && y == 0; }
	FINLINE bool NearZero() const { return fabs(x) < SMALL_FLOAT && fabs(y) < SMALL_FLOAT; }
	FINLINE float LengthSq() const { return x * x + y * y; }
	FINLINE float Length() const { return sqrtf( LengthSq() ); }
	FINLINE float Angle() const { return atan2( y, x ); }
	FINLINE Vec2 Normalized() const
	{
		float lensq = LengthSq();
		if( lensq == 0 )
		{
			Vec2 v = { 0, 0 };
			return v;
		}
		float invlen = 1.0f / sqrtf( lensq );
		Vec2 v = { x * invlen, y * invlen };
		return v;
	}
	FINLINE Vec2 Perp() const { return Vec2::Create( y, -x ); }
	FINLINE Vec2 Perp2() const { return Vec2::Create( -y, x ); }
	FINLINE void Normalize(){ *this = Normalized(); }
	FINLINE Vec2 Rotate( float angle ) const { return Rotate( Vec2::Create( cos( angle ), sin( angle ) ) ); }
	FINLINE Vec2 Rotate( const Vec2& dir ) const { return Vec2::Create( x * dir.x - y * dir.y, x * dir.y + y * dir.x ); }
	template< class T > void Serialize( T& arch )
	{
		if( T::IsText )
			arch.marker( "Vec2" );
		arch << x << y;
	}
};

FINLINE Vec2 operator + ( float f, const Vec2& v ){ Vec2 out = { f + v.x, f + v.y }; return out; }
FINLINE Vec2 operator - ( float f, const Vec2& v ){ Vec2 out = { f - v.x, f - v.y }; return out; }
FINLINE Vec2 operator * ( float f, const Vec2& v ){ Vec2 out = { f * v.x, f * v.y }; return out; }
FINLINE Vec2 operator / ( float f, const Vec2& v ){ Vec2 out = { f / v.x, f / v.y }; return out; }

FINLINE Vec2 V2( float x ){ Vec2 v = { x, x }; return v; }
FINLINE Vec2 V2( float x, float y ){ Vec2 v = { x, y }; return v; }
FINLINE float Vec2Dot( const Vec2& v1, const Vec2& v2 ){ return v1.x * v2.x + v1.y * v2.y; }


//
// VEC3
//

struct ENGINE_EXPORT Vec3
{
	float x, y, z;
	
	static FINLINE Vec3 Create( float x ){ Vec3 v = { x, x, x }; return v; }
	static FINLINE Vec3 Create( float x, float y, float z ){ Vec3 v = { x, y, z }; return v; }
	static FINLINE Vec3 CreateFromPtr( const float* x ){ Vec3 v = { x[0], x[1], x[2] }; return v; }
	static FINLINE Vec3 Min( const Vec3& a, const Vec3& b ){ return Create( TMIN( a.x, b.x ), TMIN( a.y, b.y ), TMIN( a.z, b.z ) ); }
	static FINLINE Vec3 Max( const Vec3& a, const Vec3& b ){ return Create( TMAX( a.x, b.x ), TMAX( a.y, b.y ), TMAX( a.z, b.z ) ); }
	
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
	
	FINLINE Vec3 Shuffle() const { Vec3 v = { y, z, -x }; return v; }
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
	FINLINE void Normalize(){ *this = Normalized(); }
	template< class T > void Serialize( T& arch )
	{
		if( T::IsText )
			arch.marker( "Vec3" );
		arch << x << y << z;
	}
	FINLINE Vec3 Abs() const { Vec3 v = { fabsf( x ), fabsf( y ), fabsf( z ) }; return v; }
	FINLINE Vec3 Pow( float f ){ Vec3 v = { powf( x, f ), powf( y, f ), powf( z, f ) }; return v; }
	FINLINE void Set( float _x, float _y, float _z ){ x = _x; y = _y; z = _z; }
	FINLINE void SetXY( const Vec2& xy ){ x = xy.x; y = xy.y; }
	FINLINE Vec2 ToVec2() const { Vec2 v = { x, y }; return v; }
};

FINLINE Vec3 operator + ( float f, const Vec3& v ){ Vec3 out = { f + v.x, f + v.y, f + v.z }; return out; }
FINLINE Vec3 operator - ( float f, const Vec3& v ){ Vec3 out = { f - v.x, f - v.y, f - v.z }; return out; }
FINLINE Vec3 operator * ( float f, const Vec3& v ){ Vec3 out = { f * v.x, f * v.y, f * v.z }; return out; }
FINLINE Vec3 operator / ( float f, const Vec3& v ){ Vec3 out = { f / v.x, f / v.y, f / v.z }; return out; }

FINLINE Vec3 V3( float x ){ Vec3 v = { x, x, x }; return v; }
FINLINE Vec3 V3( float x, float y, float z ){ Vec3 v = { x, y, z }; return v; }
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
FINLINE float Vec3Angle( const Vec3& v1, const Vec3& v2 ){ return acos( clamp( Vec3Dot( v1.Normalized(), v2.Normalized() ), -1, 1 ) ); }
inline Vec3 Vec3Slerp( const Vec3& v1, const Vec3& v2, float q )
{
	float dot = clamp( Vec3Dot( v1, v2 ), -1, 1 );
	float ang = acosf( dot ) * q;
	Vec3 rel = v2 - v1 * dot;
	rel.Normalize();
	if( rel.NearZero() )
	{
		if( dot > 0 )
			return TLERP( v1, v2, q );
		rel = Vec3Cross( rel, rel.Shuffle() ).Normalized();
	}
	return v1 * cos( ang ) + rel * sin( ang );
}
inline Vec3 TLERP( const Vec3& a, const Vec3& b, const Vec3& s ){ return a * ( V3(1) - s ) + b * s; }
inline Vec3 TREVLERP( const Vec3& a, const Vec3& b, const Vec3& s )
{
	return V3
	(
		a.x == b.x ? a.x : ( s.x - a.x ) / ( b.x - a.x ),
		a.y == b.y ? a.y : ( s.y - a.y ) / ( b.y - a.y ),
		a.z == b.z ? a.z : ( s.z - a.z ) / ( b.z - a.z )
	);
}


//
// VEC4
//

struct ENGINE_EXPORT Vec4
{
	float x, y, z, w;
	
#ifdef USE_VEC4
	static FINLINE Vec4 Create( float x ){ Vec4 v = { x, x, x, x }; return v; }
	static FINLINE Vec4 Create( float x, float y, float z, float w ){ Vec4 v = { x, y, z, w }; return v; }
	static FINLINE Vec4 CreateFromPtr( const float* x ){ Vec4 v = { x[0], x[1], x[2], x[3] }; return v; }
	
	FINLINE Vec4 operator + () const { return *this; }
	FINLINE Vec4 operator - () const { Vec4 v = { -x, -y, -z, -w }; return v; }
	
	FINLINE Vec4 operator + ( const Vec4& o ) const { Vec4 v = { x + o.x, y + o.y, z + o.z, w + o.w }; return v; }
	FINLINE Vec4 operator - ( const Vec4& o ) const { Vec4 v = { x - o.x, y - o.y, z - o.z, w - o.w }; return v; }
	FINLINE Vec4 operator * ( const Vec4& o ) const { Vec4 v = { x * o.x, y * o.y, z * o.z, w * o.w }; return v; }
	FINLINE Vec4 operator / ( const Vec4& o ) const { Vec4 v = { x / o.x, y / o.y, z / o.z, w / o.w }; return v; }
	
	FINLINE Vec4 operator + ( float f ) const { Vec4 v = { x + f, y + f, z + f, w + f }; return v; }
	FINLINE Vec4 operator - ( float f ) const { Vec4 v = { x - f, y - f, z - f, w - f }; return v; }
	FINLINE Vec4 operator * ( float f ) const { Vec4 v = { x * f, y * f, z * f, w * f }; return v; }
	FINLINE Vec4 operator / ( float f ) const { Vec4 v = { x / f, y / f, z / f, w / f }; return v; }
	
	FINLINE Vec4& operator += ( const Vec4& o ){ x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
	FINLINE Vec4& operator -= ( const Vec4& o ){ x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
	FINLINE Vec4& operator *= ( const Vec4& o ){ x *= o.x; y *= o.y; z *= o.z; w *= o.w; return *this; }
	FINLINE Vec4& operator /= ( const Vec4& o ){ x /= o.x; y /= o.y; z /= o.z; w /= o.w; return *this; }
	
	FINLINE Vec4& operator += ( float f ){ x += f; y += f; z += f; w += f; return *this; }
	FINLINE Vec4& operator -= ( float f ){ x -= f; y -= f; z -= f; w -= f; return *this; }
	FINLINE Vec4& operator *= ( float f ){ x *= f; y *= f; z *= f; w *= f; return *this; }
	FINLINE Vec4& operator /= ( float f ){ x /= f; y /= f; z /= f; w /= f; return *this; }
	
	FINLINE bool operator == ( const Vec4& o ) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
	FINLINE bool operator != ( const Vec4& o ) const { return x != o.x || y != o.y || z != o.z || w != o.w; }
	
	FINLINE bool IsZero() const { return x == 0 && y == 0 && z == 0 && w == 0; }
	FINLINE bool NearZero() const { return fabs(x) < SMALL_FLOAT && fabs(y) < SMALL_FLOAT && fabs(z) < SMALL_FLOAT && fabs(w) < SMALL_FLOAT; }
	FINLINE float LengthSq() const { return x * x + y * y + z * z + w * w; }
	FINLINE float Length() const { return sqrtf( LengthSq() ); }
	FINLINE Vec4 Normalized() const
	{
		float lensq = LengthSq();
		if( lensq == 0 )
		{
			Vec4 v = { 0, 0, 0, 0 };
			return v;
		}
		float invlen = 1.0f / sqrtf( lensq );
		Vec4 v = { x * invlen, y * invlen, z * invlen, w * invlen };
		return v;
	}
	FINLINE void Normalize(){ *this = Normalized(); }
	template< class T > void Serialize( T& arch )
	{
		if( T::IsText )
			arch.marker( "Vec4" );
		arch << x << y << z << w;
	}
	FINLINE void Set( float _x, float _y, float _z, float _w ){ x = _x; y = _y; z = _z; w = _w; }
	FINLINE Vec3 ToVec3() const { Vec3 v = { x, y, z }; return v; }
#endif
};

#ifdef USE_VEC4
FINLINE Vec4 operator + ( float f, const Vec4& v ){ Vec4 out = { f + v.x, f + v.y, f + v.z, f + v.w }; return out; }
FINLINE Vec4 operator - ( float f, const Vec4& v ){ Vec4 out = { f - v.x, f - v.y, f - v.z, f - v.w }; return out; }
FINLINE Vec4 operator * ( float f, const Vec4& v ){ Vec4 out = { f * v.x, f * v.y, f * v.z, f * v.w }; return out; }
FINLINE Vec4 operator / ( float f, const Vec4& v ){ Vec4 out = { f / v.x, f / v.y, f / v.z, f / v.w }; return out; }

FINLINE Vec4 V4( float x ){ Vec4 v = { x, x, x, x }; return v; }
FINLINE Vec4 V4( const Vec3& v3, float w ){ Vec4 v = { v3.x, v3.y, v3.z, w }; return v; }
FINLINE Vec4 V4( float x, float y, float z, float w ){ Vec4 v = { x, y, z, w }; return v; }
FINLINE float Vec4Dot( const Vec4& v1, const Vec4& v2 ){ return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
#endif
inline Vec4 TLERP( const Vec4& a, const Vec4& b, const Vec4& s ){ return a * ( V4(1) - s ) + b * s; }

// for interpolating color values only
static const float INV_255F = 1.0f / 255.0f;
struct BVec4
{
	union
	{
		uint8_t v[4];
		struct { uint8_t x, y, z, w; };
	};
	BVec4( const Vec4& v ) : x(v.x * 255.0f), y(v.y * 255.0f), z(v.z * 255.0f), w(v.w * 255.0f){}
	FINLINE Vec4 operator * ( float f ) const { f *= INV_255F; Vec4 v = { x * f, y * f, z * f, w * f }; return v; }
};
FINLINE Vec4 operator * ( float f, const BVec4& v ){ Vec4 out = { f * v.x, f * v.y, f * v.z, f * v.w }; return out; }


//
// QUAT
//

struct ENGINE_EXPORT Quat
{
	float x, y, z, w;
	
	static const Quat Identity;
	
#ifdef USE_QUAT
	static FINLINE Quat CreateAxisAngle( const Vec3& v, float a ){ return CreateAxisAngle( v.x, v.y, v.z, a ); }
	static Quat CreateAxisAngle( float x, float y, float z, float a )
	{
		float angsin = sinf( a / 2.0f );
		Quat q =
		{
			x * angsin,
			y * angsin,
			z * angsin,
			cosf( a / 2.0f ),
		};
		return q.Normalized();
	}
	
	FINLINE Quat operator + ( const Quat& o ) const { Quat q = { x + o.x, y + o.y, z + o.z, w + o.w }; return q; }
	FINLINE Quat operator - ( const Quat& o ) const { Quat q = { x - o.x, y - o.y, z - o.z, w - o.w }; return q; }
	Quat operator * ( const Quat& o ) const
	{
		const Quat& q1 = *this, &q2 = o;
		Quat q =
		{
			 q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x,
			-q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y,
			 q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z,
			-q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w,
		};
		return q;
	}
	FINLINE Quat operator * ( float f ) const { Quat q = { x * f, y * f, z * f, w * f }; return q; }
	FINLINE Quat operator - () const { Quat q = { -x, -y, -z, -w }; return q; }
	
	FINLINE bool operator == ( const Quat& o ) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
	FINLINE bool operator != ( const Quat& o ) const { return x != o.x && y != o.y && z != o.z && w != o.w; }
	
	Vec3 Transform( const Vec3& p ) const
	{
		Quat v_ = { p.x, p.y, p.z };
		Quat qin = Conjugate();
		Quat q_ = this->Normalized();
		q_ = v_ * q_;
		q_ = qin * q_;
		Vec3 out = { q_.x, q_.y, q_.z };
		return out;
	}
	
	FINLINE float LengthSq() const { return x * x + y * y + z * z + w * w; }
	FINLINE float Length() const { return sqrtf( LengthSq() ); }
	FINLINE Quat Normalized() const
	{
		float lensq = LengthSq();
		if( lensq == 0 )
		{
			Quat v = { 0, 0, 0, 0 };
			return v;
		}
		float invlen = 1.0f / sqrtf( lensq );
		Quat v = { x * invlen, y * invlen, z * invlen, w * invlen };
		return v;
	}
	FINLINE void Normalize(){ *this = Normalized(); }
	template< class T > void Serialize( T& arch )
	{
		if( T::IsText )
			arch.marker( "Quat" );
		arch << x << y << z << w;
	}
	FINLINE void Set( float _x, float _y, float _z, float _w ){ x = _x; y = _y; z = _z; w = _w; }
	FINLINE Quat Conjugate() const { Quat q = { -x, -y, -z, w }; return q; }
	FINLINE Quat Inverted() const { Quat q = { -x, -y, -z, w }; return q; }
	FINLINE float GetAngle() const { return 2 * acosf( w ); }
	inline Vec3 GetAxis() const
	{
		float iwsq = 1.0f - w * w;
		if( iwsq < 10 * SMALL_FLOAT )
			return V3(1,0,0);
		float s = 1.0f / iwsq;
		return V3( x * s, y * s, z * s );
	}
#endif
};
	
#ifdef USE_QUAT
FINLINE Quat operator * ( float f, const Quat& q ){ Quat out = { f * q.x, f * q.y, f * q.z, f * q.w }; return out; }

FINLINE Quat QUAT( float x, float y, float z, float w ){ Quat q = { x, y, z, w }; return q; }
FINLINE float QuatDot( const Quat& qa, const Quat& qb ){ return qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w; }
inline Quat QuatSlerp( const Quat& qa, const Quat& qo, float t )
{
	Quat qb = qo;
	float coshalfangle = qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w;
	if( coshalfangle < 0 )
	{
		qb.x = -qb.x;
		qb.y = -qb.y;
		qb.z = -qb.z;
		qb.w = -qb.w;
		coshalfangle = -coshalfangle;
	}
	
	if( fabsf( coshalfangle ) >= 1.0 )
		return qa;
	
	float halfangle = acosf( coshalfangle );
	float sinhalfangle = sqrtf( 1.0 - coshalfangle * coshalfangle );
	
	float fa, fb;
	if( fabsf( sinhalfangle ) > SMALL_FLOAT )
	{
		fa = sinf( ( 1 - t ) * halfangle ) / sinhalfangle;
		fb = sinf( t * halfangle ) / sinhalfangle; 
	}
	else
	{
		fa = 1 - t;
		fb = t;
	}
	
	Quat qout =
	{
		qa.x * fa + qb.x * fb,
		qa.y * fa + qb.y * fb,
		qa.z * fa + qb.z * fb,
		qa.w * fa + qb.w * fb,
	};
	return qout;
}
template< class S > Quat TLERP( const Quat& a, const Quat& b, const S& s ){ return QuatSlerp( a, b, s ); }
#endif

ENGINE_EXPORT Vec3 CalcAngularVelocity( const Quat& qa, const Quat& qb );


//
// MAT4
//

struct ENGINE_EXPORT Mat4
{
	union
	{
		float a[16];
		float m[4][4];
	};
	
	static const Mat4 Identity;
	
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
	static Mat4 CreateFromPtr( float* v16f )
	{
		Mat4 m;
		memcpy( m.a, v16f, sizeof(float[16]) );
		return m;
	}
	
	static Mat4 Create2DCamera( float x, float y, float size, float aspect, float angle = 0 )
	{
		size = 1.0f / size;
		x *= size / aspect;
		y *= size;
		Mat4 m =
		{
			size / aspect, 0, 0, 0,
			0, -size, 0, 0,
			0, 0, size, 0,
			-x, y, 0.5f * size, 1,
		};
		return m;
	}
	static Mat4 CreateUI( float x0, float y0, float x1, float y1 )
	{
		float w = x1 - x0;
		float h = y1 - y0;
		Mat4 m =
		{
			2.0f / w, 0, 0, 0,
			0, -2.0f / h, 0, 0,
			0, 0, 1, 0,
			-1 - x0 / w * 2.0f, 1 + y0 / w * 2.0f, 0, 1
		};
		return m;
	}
	
	static Mat4 CreateScale( float x, float y, float z )
	{
		Mat4 m = Identity;
		m.m[0][0] = x;
		m.m[1][1] = y;
		m.m[2][2] = z;
		return m;
	}
	static FINLINE Mat4 CreateScale( const Vec3& v ){ return CreateScale( v.x, v.y, v.z ); }
	static Mat4 CreateRotationDefAxis( int axis0, int axis1, float angle )
	{
		float s = sin( angle );
		float c = cos( angle );
		Mat4 m = Identity;
		m.m[ axis0 ][ axis0 ] = c;
		m.m[ axis1 ][ axis0 ] = -s;
		m.m[ axis0 ][ axis1 ] = s;
		m.m[ axis1 ][ axis1 ] = c;
		return m;
	}
	static FINLINE Mat4 CreateRotationX( float angle ){ return CreateRotationDefAxis( 1, 2, angle ); }
	static FINLINE Mat4 CreateRotationY( float angle ){ return CreateRotationDefAxis( 2, 0, angle ); }
	static FINLINE Mat4 CreateRotationZ( float angle ){ return CreateRotationDefAxis( 0, 1, angle ); }
	static Mat4 CreateRotationXYZ( float x, float y, float z )
	{
		Mat4 rx = Mat4::CreateRotationX( x );
		Mat4 ry = Mat4::CreateRotationY( y );
		Mat4 rz = Mat4::CreateRotationZ( z );
		Mat4 rot, rot2;
		rot2.Multiply( rx, ry );
		rot.Multiply( rot2, rz );
		return rot;
	}
	static FINLINE Mat4 CreateRotationXYZ( const Vec3& rot_angles ){ return CreateRotationXYZ( rot_angles.x, rot_angles.y, rot_angles.z ); }
	static Mat4 CreateRotationAxisAngle( float x, float y, float z, float angle )
	{
		float s = sin( angle );
		float c = cos( angle );
		float Ic = 1 - c;
		
		Mat4 m = Identity;
		m.m[0][0] = x * x * Ic + c;
		m.m[1][0] = y * x * Ic - z * s;
		m.m[2][0] = z * x * Ic + y * s;
		m.m[0][1] = x * y * Ic + z * s;
		m.m[1][1] = y * y * Ic + c;
		m.m[2][1] = z * y * Ic - x * s;
		m.m[0][2] = x * z * Ic - y * s;
		m.m[1][2] = y * z * Ic + x * s;
		m.m[2][2] = z * z * Ic + c;
		return m;
	}
	static FINLINE Mat4 CreateRotationAxisAngle( const Vec3& axis, float angle ){ return CreateRotationAxisAngle( axis.x, axis.y, axis.z, angle ); }
	static Mat4 CreateRotationBetweenVectors( const Vec3& a, const Vec3& b )
	{
		float angle = acosf( clamp( Vec3Dot( a, b ), -1, 1 ) );
		Vec3 axis = Vec3Cross( a, b );
		if( axis.LengthSq() < SMALL_FLOAT )
			axis = Vec3Cross( a, a.Shuffle() );
		return CreateRotationAxisAngle( axis.Normalized(), angle );
	}
	static Mat4 CreateRotationFromQuat( const Quat& q )
	{
		float a = -q.w;
		float b = q.x;
		float c = q.y;
		float d = q.z;
		float a2 = a*a;
		float b2 = b*b;
		float c2 = c*c;
		float d2 = d*d;
		
		Mat4 m;
		m.m[0][0] = a2 + b2 - c2 - d2;
		m.m[1][0] = 2*(b*c + a*d);
		m.m[2][0] = 2*(b*d - a*c);
		m.m[3][0] = 0.0f;
		
		m.m[0][1] = 2*(b*c - a*d);
		m.m[1][1] = a2 - b2 + c2 - d2;
		m.m[2][1] = 2*(c*d + a*b);
		m.m[3][1] = 0.0f;
		
		m.m[0][2] = 2*(b*d + a*c);
		m.m[1][2] = 2*(c*d - a*b);
		m.m[2][2] = a2 - b2 - c2 + d2;
		m.m[3][2] = 0.0f;
		
		m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.0f;
		m.m[3][3] = 1.0f;
		return m;
	}
	static FINLINE Mat4 CreateTranslation( float x, float y, float z )
	{
		Mat4 out;
		out.SetIdentity();
		out.m[3][0] = x;
		out.m[3][1] = y;
		out.m[3][2] = z;
		return out;
	}
	static FINLINE Mat4 CreateTranslation( const Vec3& v ){ return CreateTranslation( v.x, v.y, v.z ); }
	static FINLINE Mat4 CreateSRT( const Vec3& scale, const Vec3& rot_angles, const Vec3& pos )
	{
		return CreateSXT( scale, CreateRotationXYZ( rot_angles ), pos );
	}
	static FINLINE Mat4 CreateSRT( const Vec3& scale, const Quat& rot, const Vec3& pos )
	{
		return CreateSXT( scale, CreateRotationFromQuat( rot ), pos );
	}
	static Mat4 CreateSXT( const Vec3& scale, const Mat4& rot, const Vec3& pos )
	{
		Mat4 mscl = Mat4::CreateScale( scale );
		Mat4 mtrn = Mat4::CreateTranslation( pos );
		Mat4 mtmp1, mtmp2;
		mtmp1.Multiply( mscl, rot );
		mtmp2.Multiply( mtmp1, mtrn );
		return mtmp2;
	}
	
	static Mat4 CreateLookAt( Vec3 pos, Vec3 dir, Vec3 up )
	{
		Mat4 out;
		out.LookAt( pos, dir, up );
		return out;
	}
	static Mat4 CreatePerspective( float angle, float aspect, float aamix, float znear, float zfar )
	{
		Mat4 out;
		out.Perspective( angle, aspect, aamix, znear, zfar );
		return out;
	}
	static Mat4 CreateOrtho( const Vec3& bmin, const Vec3& bmax )
	{
		Mat4 out;
		out.Ortho( bmin, bmax );
		return out;
	}
	static Mat4 Basis( const Vec3& vx, const Vec3& vy, const Vec3& vz, bool cols = false )
	{
		Mat4 out =
		{
			vx.x, vx.y, vx.z, 0,
			vy.x, vy.y, vy.z, 0,
			vz.x, vz.y, vz.z, 0,
			0, 0, 0, 1
		};
		if( cols )
			out.Transpose();
		return out;
	}
	
	Vec3 GetTranslation() const
	{
		return V3( m[3][0], m[3][1], m[3][2] );
	}
	Vec3 GetScale() const
	{
		return V3(
			V3( m[0][0], m[1][0], m[2][0] ).Length(),
			V3( m[0][1], m[1][1], m[2][1] ).Length(),
			V3( m[0][2], m[1][2], m[2][2] ).Length()
		);
	}
	Quat GetRotationQuaternion() const;
	Vec3 GetXYZAngles() const
	{
		float q = sqrtf( m[1][2] * m[1][2] + m[2][2] * m[2][2] );
		return V3( atan2( m[1][2], m[2][2] ), atan2( -m[0][2], q ), atan2( m[0][1], m[0][0] ) );
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
	FINLINE Vec4 Transform( const Vec4& v ) const
	{
		Vec4 out =
		{
			v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + v.w * m[3][0],
			v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + v.w * m[3][1],
			v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + v.w * m[3][2],
			v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + v.w * m[3][3],
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

	FINLINE bool operator == ( const Mat4& o ) const
	{
		for( int i = 0; i < 16; ++i )
			if( a[ i ] != o.a[ i ] )
				return false;
		return true;
	}
	FINLINE bool operator != ( const Mat4& o ) const { return !( *this == o ); }
	
#define InvertTo InvertTo_ // linker errors
	bool InvertTo( Mat4& out );
	FINLINE void Transpose()
	{
		TSWAP( m[1][0], m[0][1] );
		TSWAP( m[2][0], m[0][2] );
		TSWAP( m[3][0], m[0][3] );
		TSWAP( m[2][1], m[1][2] );
		TSWAP( m[3][1], m[1][3] );
		TSWAP( m[3][2], m[2][3] );
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
	
	Mat4& Multiply( const Mat4& A, const Mat4& B )
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
		return *this;
	}
	Mat4 operator * ( const Mat4& o ) const
	{
		Mat4 n;
		n.Multiply( *this, o );
		return n;
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
	
	void Ortho( const Vec3& bmin, const Vec3& bmax )
	{
		m[0][0] = 2.0f / ( bmax.x - bmin.x );
		m[0][1] = m[0][2] = m[0][3] = 0;
		m[1][1] = 2.0f / ( bmax.y - bmin.y );
		m[1][0] = m[1][2] = m[1][3] = 0;
		m[2][2] = 1.0f / ( bmax.z - bmin.z );
		m[2][0] = m[2][1] = m[2][3] = 0;
		m[3][0] = ( bmin.x + bmax.x ) / ( bmin.x - bmax.x );
		m[3][1] = ( bmin.y + bmax.y ) / ( bmin.y - bmax.y );
		m[3][2] = ( bmin.z + bmax.z ) / ( bmin.z - bmax.z );
		m[3][3] = 1;
	}
	
	template< class T > void Serialize( T& arch )
	{
		if( T::IsText )
			arch.marker( "Mat4" );
		for( int i = 0; i < 16; ++i )
			arch << a[ i ];
	}
#endif
};

ENGINE_EXPORT Quat TransformQuaternion( const Quat& q, const Mat4& m );



ENGINE_EXPORT float PointTriangleDistance( const Vec3& pt, const Vec3& t0, const Vec3& t1, const Vec3& t2 );
ENGINE_EXPORT bool TriangleIntersect( const Vec3& ta0, const Vec3& ta1, const Vec3& ta2, const Vec3& tb0, const Vec3& tb1, const Vec3& tb2 );
ENGINE_EXPORT bool TriangleAABBIntersect( const Vec3& t0, const Vec3& t1, const Vec3& t2, const Vec3& bbmin, const Vec3& bbmax );
ENGINE_EXPORT float PolyArea( const Vec2* points, int pointcount );
ENGINE_EXPORT float TriangleArea( float a, float b, float c );
ENGINE_EXPORT float TriangleArea( const Vec3& p0, const Vec3& p1, const Vec3& p2 );
ENGINE_EXPORT bool RayPlaneIntersect( const Vec3& pos, const Vec3& dir, const Vec4& plane, float dsts[2] );
ENGINE_EXPORT bool PolyGetPlane( const Vec3* points, int pointcount, Vec4& plane );
ENGINE_EXPORT bool RayPolyIntersect( const Vec3& pos, const Vec3& dir, const Vec3* points, int pointcount, float dst[1] );
ENGINE_EXPORT bool RaySphereIntersect( const Vec3& pos, const Vec3& dir, const Vec3& spherePos, float sphereRadius, float dst[1] );
ENGINE_EXPORT bool SegmentAABBIntersect( const Vec3& p1, const Vec3& p2, const Vec3& bbmin, const Vec3& bbmax );
ENGINE_EXPORT bool SegmentAABBIntersect2( const Vec3& p1, const Vec3& p2, const Vec3& bbmin, const Vec3& bbmax, float dst[1] );


//
// COLOR
//

inline uint32_t Vec4ToCol32( const Vec4& colv4 )
{
	return COLOR_RGBA(
		255 * clamp( colv4.x, 0, 1 ),
		255 * clamp( colv4.y, 0, 1 ),
		255 * clamp( colv4.z, 0, 1 ),
		255 * clamp( colv4.w, 0, 1 )
	);
}
inline Vec4 Col32ToVec4( uint32_t colu32 )
{
	Vec4 out =
	{
		INV_255F * (float) COLOR_EXTRACT_R( colu32 ),
		INV_255F * (float) COLOR_EXTRACT_G( colu32 ),
		INV_255F * (float) COLOR_EXTRACT_B( colu32 ),
		INV_255F * (float) COLOR_EXTRACT_A( colu32 )
	};
	return out;
}

inline uint32_t Vec3ToCol32( const Vec3& colv3 )
{
	return COLOR_RGB(
		255 * clamp( colv3.x, 0, 1 ),
		255 * clamp( colv3.y, 0, 1 ),
		255 * clamp( colv3.z, 0, 1 )
	);
}
inline Vec3 Col32ToVec3( uint32_t colu32 )
{
	Vec3 out =
	{
		INV_255F * (float) COLOR_EXTRACT_R( colu32 ),
		INV_255F * (float) COLOR_EXTRACT_G( colu32 ),
		INV_255F * (float) COLOR_EXTRACT_B( colu32 )
	};
	return out;
}

// Alt. color interfaces
// - build from
inline Vec3 HSV( const Vec3& hsv )
{
	Vec3 cor = { fmodf( hsv.x, 1.0f ), hsv.y, hsv.z };
	int32_t hi = int32_t( floor( cor.x * 6 ) ) % 6;
	float f = ( cor.x * 6 ) - floor( cor.x * 6 );
	float p = cor.z * ( 1.0f - cor.y );
	float q = cor.z * ( 1.0f - ( f * cor.y ) );
	float t = cor.z * ( 1.0f - ( ( 1.0f - f ) * cor.y ) );
	switch( hi )
	{
	case 0: return V3( cor.z, t, p );
	case 1: return V3( q, cor.z, p );
	case 2: return V3( p, cor.z, t );
	case 3: return V3( p, q, cor.z );
	case 4: return V3( t, p, cor.z );
	case 5: return V3( cor.z, p, q );
	}
	return V3( 0, 0, 0 );
}
// - retrieve properties
// generics
inline float GetMax( const Vec3& rgbColor ) { return TMAX( rgbColor.x, TMAX( rgbColor.y, rgbColor.z ) ); }
inline float GetMin( const Vec3& rgbColor ) { return TMIN( rgbColor.x, TMIN( rgbColor.y, rgbColor.z ) ); }
inline float GetChroma( const Vec3& rgbColor ) { return GetMax( rgbColor ) - GetMin( rgbColor ); }
// hue
inline float GetHue( const Vec3& rgbColor )
{
	float M = GetMax( rgbColor );
	float m = GetMin( rgbColor );
	float C = M - m;
	if( C == 0 )
		return 0; // undefined, mapped to acceptable values
	float h = 0;
	if( M == rgbColor.x ) h = fmodf( ( rgbColor.y - rgbColor.z ) / C, 6.0f );
	else if( M == rgbColor.y ) h = ( rgbColor.z - rgbColor.x ) / C + 2.0f;
	else if( M == rgbColor.z ) h = ( rgbColor.x - rgbColor.y ) / C + 4.0f;
	return h / 6.0f;
}
// lightness/value/...
inline float GetIntensity( const Vec3& rgbColor ){ return ( rgbColor.x + rgbColor.y + rgbColor.z ) / 3.0f; }
inline float GetValue( const Vec3& rgbColor ){ return GetMax( rgbColor ); }
inline float GetLightness( const Vec3& rgbColor ){ return ( GetMax( rgbColor ) + GetMin( rgbColor ) ) * 0.5f; }
inline float GetLuma( const Vec3& rgbColor ){ return 0.3f * rgbColor.x + 0.59f * rgbColor.y + 0.11f * rgbColor.z; }
// saturation
inline float GetHSVSaturation( const Vec3& rgbColor )
{
	float M = GetMax( rgbColor );
	float m = GetMin( rgbColor );
	if( M == 0 ) return 0;
	else return ( M - m ) / M;
}
inline float GetHSLSaturation( const Vec3& rgbColor )
{
	float M = GetMax( rgbColor );
	float m = GetMin( rgbColor );
	float C = M - m;
	float L = ( M + m ) * 0.5f;
	if( L == 1 ) return 0;
	else return C / ( 1 - fabsf( 2 * L - 1 ) );
}
inline float GetHSISaturation( const Vec3& rgbColor )
{
	float I = GetIntensity( rgbColor );
	if( I == 0 ) return 0;
	else return 1 - GetMin( rgbColor ) / I;
}
// alt. color spaces
inline Vec3 GetHSV( const Vec3& rgbColor ){ return V3( GetHue( rgbColor ), GetHSVSaturation( rgbColor ), GetValue( rgbColor ) ); }
inline Vec3 GetHSL( const Vec3& rgbColor ){ return V3( GetHue( rgbColor ), GetHSLSaturation( rgbColor ), GetLightness( rgbColor ) ); }


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
	Handle& operator = ( const Handle& h ){ if( item ) item->Release(); item = h.item; if( item ) item->Acquire(); return *this; }
	T* operator -> () const { return item; }
	T& operator * () const { return *item; }
	operator T* () const { return item; }
};


//
// ARRAY
//

#ifdef USE_SERIALIZATION
#define USE_ARRAY
#endif

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
	Array( const Array& a ) : m_data(NULL), m_size(0), m_mem(0) { insert( 0, a.m_data, a.m_size ); }
	Array( const T* v, size_t sz ) : m_data(NULL), m_size(0), m_mem(0) { insert( 0, v, sz ); }
	~Array(){ clear(); free( m_data ); }
	Array& operator = ( const Array& a )
	{
		if( this == &a )
			return *this;
		clear();
		insert( 0, a.m_data, a.m_size );
		return *this;
	}
	
	FINLINE bool operator == ( const Array& other ) const
	{
		if( m_size != other.m_size )
			return false;
		for( size_t i = 0; i < other.m_size; ++i )
			if( !( m_data[ i ] == other.m_data[ i ] ) )
				return false;
		return true;
	}
	FINLINE bool operator != ( const Array& other ) const { return !( *this == other ); }
	
	FINLINE int compare_to( const Array& other ) const
	{
		int rv = memcmp( m_data, other.m_data, TMIN( m_size, other.m_size ) );
		if( rv )
			return rv;
		if( m_size < other.m_size )
			return -1;
		else if( m_size > other.m_size )
			return 1;
		return 0;
	};
	
	FINLINE size_t size() const { return m_size; }
	FINLINE size_t size_bytes() const { return m_size * sizeof( T ); }
	FINLINE size_t capacity() const { return m_mem; }
	FINLINE const T* data() const { return m_data; }
	FINLINE T* data(){ return m_data; }
	
	FINLINE T& first(){ ASSERT( m_size ); return m_data[0]; }
	FINLINE const T& first() const { ASSERT( m_size ); return m_data[0]; }
	FINLINE T& last(){ ASSERT( m_size ); return m_data[ m_size - 1 ]; }
	FINLINE const T& last() const { ASSERT( m_size ); return m_data[ m_size - 1 ]; }
	
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
	void uerase( size_t i )
	{
		if( i < size() - 1 )
			m_data[ i ] = m_data[ m_size - 1 ];
		pop_back();
	}
	
	FINLINE void reverse()
	{
		for( size_t i = 0; i < m_size / 2; ++i )
		{
			TSWAP( m_data[ i ], m_data[ m_size - 1 - i ] );
		}
	}
	
	FINLINE void push_front( const T& v ){ insert( 0, v ); }
	FINLINE void push_back( const T& v ){ insert( size(), v ); }
	FINLINE void pop_back(){ erase( size() - 1 ); }
	FINLINE void append( const T* v, size_t sz ){ insert( size(), v, sz ); }
	FINLINE void assign( const T* v, size_t sz ){ clear(); insert( size(), v, sz ); }
	FINLINE void clear(){ resize( 0 ); }
	
	FINLINE void insert( size_t at, const T& v ){ insert( at, &v, 1 ); }
	void erase( size_t from, size_t count = 1 );
	void resize( size_t sz );
	void resize_using( size_t sz, const T& val );
	void reserve( size_t sz );
	void insert( size_t at, const T* v, size_t count );
	size_t find_or_add( const T& what, size_t from = 0 )
	{
		size_t id = find_first_at( what, from );
		if( id == NOT_FOUND )
		{
			id = m_size;
			push_back( what );
		}
		return id;
	}
	
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
	
	template< class TA > void Serialize( TA& arch )
	{
		uint32_t sz = m_size;
		arch << sz;
		if( TA::IsReader )
			resize( sz );
		for( size_t i = 0; i < m_size; ++i )
			arch << m_data[ i ];
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
void Array<T>::resize_using( size_t sz, const T& val )
{
	reserve( sz );
	while( sz > m_size )
		new (&m_data[ m_size++ ]) T( val );
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
		size_t i = m_size;
		while( i > at )
		{
			i--;
			new (&m_data[ i + count ]) T( m_data[ i ] );
			m_data[ i ].~T();
		}
	}
	for( size_t i = 0; i < count; ++i )
		new (&m_data[ i + at ]) T( v[ i ] );
	m_size += count;
}

template< class T >
void Array<T>::erase( size_t from, size_t count )
{
	if( !count )
		return;
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

template< class T > void TSWAP( Array<T>& a, Array<T>& b )
{
	TSWAP( a.m_data, b.m_data );
	TSWAP( a.m_size, b.m_size );
	TSWAP( a.m_mem, b.m_mem );
}

typedef Array< uint8_t > ByteArray;
typedef Array< uint16_t > UInt16Array;
typedef Array< uint32_t > UInt32Array;


//
// STRINGS
//

FINLINE size_t StringLength( const char* str ){ const char* o = str; while( *str ) str++; return str - o; }

struct String : Array< char >
{
#ifdef USE_ARRAY
	String() : Array(){}
	String( const char* str ) : Array( str, StringLength( str ) ){}
	String( const char* str, size_t size ) : Array( str, size ){}
	String( const String& s ) : Array( s ){}
	
	FINLINE void append( const struct StringView& sv );
	FINLINE void append( const char* str, size_t sz ){ Array::append( str, sz ); }
	FINLINE void append( const char* str ){ append( str, StringLength( str ) ); }
	template< class TA > void Serialize( TA& arch )
	{
		uint32_t sz = m_size;
		arch << sz;
		if( TA::IsReader )
			resize( sz );
		arch.charbuf( m_data, m_size );
	}
#endif
};

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
	FINLINE const char* begin() const { return m_str; }
	FINLINE const char* end() const { return m_str + m_size; }
	
	FINLINE const char& operator [] ( size_t i ) const { assert( i < m_size ); return m_str[ i ]; }
	
	FINLINE bool operator == ( const StringView& sv ) const { return m_size == sv.m_size && !memcmp( m_str, sv.m_str, m_size ); }
	FINLINE bool operator != ( const StringView& sv ) const { return !( *this == sv ); }
	
	FINLINE char ch() const { if( m_size ) return *m_str; else return 0; }
	FINLINE bool contains( const StringView& substr ) const { return find_first_at( substr ) != NOT_FOUND; }
	FINLINE size_t find_first_at( const StringView& substr, size_t from = 0, size_t defval = NOT_FOUND ) const
	{
		if( substr.m_size > m_size )
			return defval;
		for( size_t i = from; i <= m_size - substr.m_size; ++i )
			if( !memcmp( m_str + i, substr.m_str, substr.m_size ) )
				return i;
		return defval;
	}
	FINLINE size_t substr_end( const StringView& substr ) const { return m_size - TMIN( m_size, substr.m_size ); }
	FINLINE size_t find_last_at( const StringView& substr, size_t from, size_t defval = NOT_FOUND ) const
	{
		if( substr.m_size > m_size )
			return defval;
		for( size_t i = from;; )
		{
			if( !memcmp( m_str + i, substr.m_str, substr.m_size ) )
				return i;
			if( i == 0 )
				break;
			i--;
		}
		return defval;
	}
	FINLINE size_t find_last_at( const StringView& substr ) const
	{
		return find_last_at( substr, substr_end( substr ) );
	}
	FINLINE bool is_any( char c ) const
	{
		for( size_t i = 0; i < m_size; ++i )
			if( m_str[i] == c )
				return true;
		return false;
	}
	FINLINE bool starts_with( const StringView& substr ) const
	{
		return m_size >= substr.m_size && !memcmp( m_str, substr.m_str, substr.m_size );
	}
	FINLINE bool ends_with( const StringView& substr ) const
	{
		return m_size >= substr.m_size && !memcmp( m_str + m_size - substr.m_size, substr.m_str, substr.m_size );
	}
	
	FINLINE StringView part( size_t start, size_t count = NOT_FOUND ) const
	{
		if( start > m_size )
			start = m_size;
		if( count > m_size || start + count > m_size )
			count = m_size - start;
		return StringView( m_str + start, count );
	}
	
	FINLINE StringView from( const StringView& substr ) const
	{
		size_t pos = find_first_at( substr, 0, m_size );
		return StringView( m_str + pos, m_size - pos );
	}
	FINLINE StringView after( const StringView& substr ) const
	{
		if( substr.m_size > m_size )
			return StringView( m_str + m_size, 0 );
		size_t pos = find_first_at( substr, 0, m_size - substr.m_size ) + substr.m_size;
		return StringView( m_str + pos, m_size - pos );
	}
	FINLINE StringView until( const StringView& substr ) const
	{
		size_t pos = find_first_at( substr, 0, m_size );
		return StringView( m_str, pos );
	}
	FINLINE StringView until_last( const StringView& substr ) const
	{
		size_t pos = find_last_at( substr, m_size - 1, 0 );
		return StringView( m_str, pos );
	}
	FINLINE StringView up_to_last( const StringView& substr ) const
	{
		size_t pos = find_last_at( substr );
		return pos == NOT_FOUND ? StringView( m_str, 0 ) : StringView( m_str, pos + substr.size() );
	}
	FINLINE StringView until_any( const StringView& chars ) const
	{
		const char *ptr = m_str, *end = m_str + m_size;
		while( ptr < end && !chars.is_any( *ptr ) )
			ptr++;
		return StringView( m_str, ptr - m_str );
	}
	FINLINE StringView after_all( const StringView& chars ) const
	{
		const char *ptr = m_str, *end = m_str + m_size;
		while( ptr < end && chars.is_any( *ptr ) )
			ptr++;
		return StringView( ptr, end - ptr );
	}
	FINLINE bool skip( size_t n )
	{
		if( n > m_size )
		{
			m_str += m_size;
			m_size = 0;
			return false;
		}
		m_str += n;
		m_size -= n;
		return true;
	}
	FINLINE void trim( const StringView& chars )
	{
		while( m_size && chars.is_any( *m_str ) )
		{
			m_str++;
			m_size--;
		}
		while( m_size && chars.is_any( m_str[ m_size - 1 ] ) )
			m_size--;
	}
	
#ifdef USE_ARRAY
	FINLINE operator String () const { return String( m_str, m_size ); }
#endif
};

FINLINE void String::append( const StringView& sv )
{
	append( sv.m_str, sv.m_size );
}


struct IF_GCC(ENGINE_EXPORT) IProcessor
{
	ENGINE_EXPORT virtual void Process( void* data ) = 0;
};

struct SGRX_RefCounted
{
	SGRX_RefCounted() : m_refcount(0){}
	virtual ~SGRX_RefCounted(){}
	
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	int32_t m_refcount;
};

struct SGRX_RCRsrc : SGRX_RefCounted
{
	String m_key; // [storage for] hash table key
};

struct SGRX_RCXFItem : SGRX_RefCounted
{
	virtual void SetTransform( const Mat4& mtx ) = 0;
};

typedef Handle< SGRX_RCXFItem > XFItemHandle;


struct ConfigReader
{
	ConfigReader( const StringView& _it ) : it( _it.after_all( SPACE_CHARS ) ){}
	bool Read( StringView& key, StringView& value )
	{
		while( it.size() )
		{
			key = it.until_any( SPACE_CHARS );
			if( key.ch() == '#' )
			{
				it = it.after( "\n" ).after_all( SPACE_CHARS );
				continue;
			}
			
			it.skip( key.size() );
			
			value = it.until( "\n" );
			it.skip( value.size() );
			value.trim( SPACE_CHARS );
			
			it = it.after_all( SPACE_CHARS );
			
			return key.size() != 0;
		}
		return false;
	}
	StringView it;
};

struct CmdListReader
{
	CmdListReader();
	bool Read( StringView& cmd )
	{
		// TODO
		return false;
	}
	bool GetParam( StringView& key, StringView& value )
	{
		// TODO
		return false;
	}
	int GetParamCount() const { return params.size() / 2; }
	Array< String > params;
	StringView it;
};

template< size_t N >
struct StackString
{
	char str[ N + 1 ];
	
	StackString( const StringView& sv ){ size_t sz = TMIN( sv.m_size, N ); if( sz ) memcpy( str, sv.m_str, sz ); str[ sz ] = 0; }
	operator const char* (){ return str; }
};
typedef StackString< ENGINE_MAX_PATH > StackPath;


ENGINE_EXPORT String String_Concat( const StringView& a, const StringView& b );
ENGINE_EXPORT String String_Replace( const StringView& base, const StringView& sub, const StringView& rep );

//
// PARSING
//

ENGINE_EXPORT bool String_ParseBool( const StringView& sv );
ENGINE_EXPORT int64_t String_ParseInt( const StringView& sv, bool* success = NULL );
ENGINE_EXPORT double String_ParseFloat( const StringView& sv, bool* success = NULL );
ENGINE_EXPORT Vec2 String_ParseVec2( const StringView& sv, bool* success = NULL );
ENGINE_EXPORT Vec3 String_ParseVec3( const StringView& sv, bool* success = NULL );
ENGINE_EXPORT Vec4 String_ParseVec4( const StringView& sv, bool* success = NULL );


//
// UNICODE
//
// - utf8_decode: returns number of bytes parsed (negated if input was invalid)
// - utf8_encode: returns number of bytes written (up to 4, make sure there's space)
//

#define UNICODE_INVCHAR 0xfffd
#define UNICODE_INVCHAR_STR "\xef\xbf\xbd"
#define UNICODE_INVCHAR_LEN 3
ENGINE_EXPORT int UTF8Decode( const char* buf, size_t size, uint32_t* outchar );
ENGINE_EXPORT int UTF8Encode( uint32_t ch, char* out );


//
// HASH TABLE
//

typedef uint32_t Hash;
ENGINE_EXPORT Hash HashFunc( const char* str, size_t size );

inline Hash HashVar( int8_t v ){ return v; }
inline Hash HashVar( uint8_t v ){ return v; }
inline Hash HashVar( int16_t v ){ return v; }
inline Hash HashVar( uint16_t v ){ return v; }
inline Hash HashVar( int32_t v ){ return v; }
inline Hash HashVar( uint32_t v ){ return v; }
inline Hash HashVar( int64_t v ){ return (Hash) v; }
inline Hash HashVar( uint64_t v ){ return (Hash) v; }
inline Hash HashVar( void* v ){ return (Hash) v; }
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
	HashTable( const HashTable& ht ) : m_pair_mem( ht.m_pair_mem ), m_var_mem( ht.m_var_mem ), m_size( ht.m_size ), m_num_removed( ht.m_num_removed )
	{
		m_pairs = new size_type[ m_pair_mem ];
		m_vars = (Var*) malloc( sizeof(Var) * m_var_mem );
		memcpy( m_pairs, ht.m_pairs, sizeof(size_type) * m_pair_mem );
		for( size_type i = 0; i < m_size; ++i )
		{
			new (&m_vars[ i ].key) K( ht.m_vars[ i ].key );
			new (&m_vars[ i ].value) V( ht.m_vars[ i ].value );
		}
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
	
	HashTable& operator = ( const HashTable& ht )
	{
		this->~HashTable();
		new (this) HashTable( ht );
		return *this;
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
	FINLINE Var& item( size_t i ){ ASSERT( (size_type) i < m_size ); return m_vars[ i ]; }
	FINLINE const Var& item( size_t i ) const { ASSERT( (size_type) i < m_size ); return m_vars[ i ]; }
	FINLINE V getcopy( const K& key, const V& defval = V() ) const { const Var* raw = getraw( key ); return raw ? raw->value : defval; }
	FINLINE V* getptr( const K& key, V* defval = NULL ){ Var* raw = getraw( key ); return raw ? &raw->value : defval; }
	FINLINE const V* getptr( const K& key, const V* defval = NULL ) const { const Var* raw = getraw( key ); return raw ? &raw->value : defval; }
	FINLINE V& operator [] ( const K& key ){ Var* raw = getraw( key ); if( !raw ) raw = set( key, V() ); return raw->value; }
	
	size_type _get_pair_id( const K& key, Hash hash ) const
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
			
			m_pairs[ i ] = REMOVED;
			
			m_num_removed++;
			m_size--;
			if( p < m_vars + m_size )
			{
				Var* ep = m_vars + m_size;
				i = _get_pair_id( ep->key, ep->hash );
				assert( i != -1 );
				
				p->key.~K();
				p->value.~V();
				new (&p->key) K( ep->key );
				new (&p->value) V( ep->value );
				ep->key.~K();
				ep->value.~V();
				p->hash = ep->hash;
				
				m_pairs[ i ] = idx;
			}
			else
			{
				p->key.~K();
				p->value.~V();
			}
			
			if( m_num_removed > m_var_mem * 0.25 + 16 )
			{
				reserve( (size_type) ( m_size * 0.75 + m_var_mem * 0.25 ) );
				rehash( (size_type) ( m_size * 0.5 + m_var_mem * 0.5 ) );
			}
			
			return true;
		}
		
		return false;
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
		TMEMSET( np, (size_t) size, (size_type) EMPTY );
		
	#if 0
		printf( "rehash %d -> %d (size = %d, mem = %d kB)\n", m_pair_mem, size, m_size,
			(size * sizeof(size_type) + m_var_mem * sizeof(Var)) / 1024 );
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
			p[ i ].hash = m_vars[ i ].hash;
		}
		free( m_vars );
		m_vars = p;
		m_var_mem = size;
	}
#endif
};

typedef HashTable< String, String > StringTable;


//
// SERIALIZATION
//

template< class T > struct SerializeVersionHelper
{
	enum { IsWriter = T::IsWriter, IsReader = T::IsReader, IsText = T::IsText, IsBinary = T::IsBinary };
	
	SerializeVersionHelper( T& a, uint16_t curver ) : version( curver ), arch( &a )
	{
		(*arch) << version;
	}
	uint16_t version;
	T* arch;
	template< class V > SerializeVersionHelper& _serialize( V& v, bool actually = true, const V& defval = V() )
	{
		if( !actually )
		{
			if( T::IsReader )
				v = defval;
			return *this;
		}
		(*arch) << v;
		return *this;
	}
	
	FINLINE SerializeVersionHelper& memory( void* ptr, size_t sz ){ arch->memory( ptr, sz ); return *this; }
	FINLINE SerializeVersionHelper& charbuf( char* ptr, size_t sz ){ arch->charbuf( ptr, sz ); return *this; }
	FINLINE SerializeVersionHelper& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE SerializeVersionHelper& marker( const char* ptr, size_t sz ){ arch->marker( ptr, sz ); return *this; }
	
	FINLINE SerializeVersionHelper& operator () ( bool& v, bool a = true, bool d = 0 ){ _serialize( v, a, d ); return *this; }
	FINLINE SerializeVersionHelper& operator () ( char& v, bool a = true, char d = 0 ){ _serialize( v, a, d ); return *this; }
	FINLINE SerializeVersionHelper& operator () ( int8_t& v, bool a = true, const int8_t d = 0 ){ return _serialize( v, a, d ); }
	FINLINE SerializeVersionHelper& operator () ( uint8_t& v, bool a = true, const uint8_t d = 0 ){ return _serialize( v, a, d ); }
	FINLINE SerializeVersionHelper& operator () ( int16_t& v, bool a = true, const int16_t d = 0 ){ return _serialize( v, a, d ); }
	FINLINE SerializeVersionHelper& operator () ( uint16_t& v, bool a = true, const uint16_t d = 0 ){ return _serialize( v, a, d ); }
	FINLINE SerializeVersionHelper& operator () ( int32_t& v, bool a = true, const int32_t d = 0 ){ return _serialize( v, a, d ); }
	FINLINE SerializeVersionHelper& operator () ( uint32_t& v, bool a = true, const uint32_t d = 0 ){ return _serialize( v, a, d ); }
	FINLINE SerializeVersionHelper& operator () ( int64_t& v, bool a = true, const int64_t d = 0 ){ return _serialize( v, a, d ); }
	FINLINE SerializeVersionHelper& operator () ( uint64_t& v, bool a = true, const uint64_t d = 0 ){ return _serialize( v, a, d ); }
	FINLINE SerializeVersionHelper& operator () ( float& v, bool a = true, const float d = 0 ){ return _serialize( v, a, d ); }
	FINLINE SerializeVersionHelper& operator () ( double& v, bool a = true, const double d = 0 ){ return _serialize( v, a, d ); }
	template< class ST > SerializeVersionHelper& operator () ( ST& v, bool actually = true, const ST& defval = ST() )
	{
		if( !actually )
		{
			if( T::IsReader )
				v = defval;
			return *this;
		}
		v.Serialize( *this );
		return *this;
	}
	
	FINLINE SerializeVersionHelper& operator << ( bool& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( char& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( int8_t& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( uint8_t& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( int16_t& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( uint16_t& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( int32_t& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( uint32_t& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( int64_t& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( uint64_t& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( float& v ){ _serialize( v ); return *this; }
	FINLINE SerializeVersionHelper& operator << ( double& v ){ _serialize( v ); return *this; }
	template< class ST > SerializeVersionHelper& operator << ( ST& v ){ v.Serialize( *this ); return *this; }
};

struct ByteReader
{
	ByteReader( ByteArray* ba, size_t p = 0 ) : input( ba ), pos( p ), error( false ){}
	enum { IsWriter = 0, IsReader = 1, IsText = 0, IsBinary = 1 };
	FINLINE ByteReader& operator << ( bool& v ){ uint8_t u; _read( &u, sizeof(u) ); v = !!u; return *this; }
	FINLINE ByteReader& operator << ( char& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( int8_t& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( uint8_t& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( int16_t& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( uint16_t& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( int32_t& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( uint32_t& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( int64_t& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( uint64_t& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( float& v ){ _read( &v, sizeof(v) ); return *this; }
	FINLINE ByteReader& operator << ( double& v ){ _read( &v, sizeof(v) ); return *this; }
	template< class T > ByteReader& operator << ( T& v ){ v.Serialize( *this ); return *this; }
	FINLINE ByteReader& memory( void* ptr, size_t sz ){ return _read( ptr, sz ); }
	FINLINE ByteReader& charbuf( char* ptr, size_t sz ){ return _read( ptr, sz ); }
	FINLINE ByteReader& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE ByteReader& marker( const char* ptr, size_t sz )
	{
		if( pos >= input->size() || pos + sz > input->size() )
			error = true;
		else if( 0 != memcmp( ptr, &input->at( pos ), sz ) )
			error = true;
		else
			pos += sz;
		return *this;
	}
	FINLINE ByteReader& padding( size_t sz )
	{
		if( pos >= input->size() || pos + sz > input->size() )
			error = true;
		else
			pos += sz;
		return *this;
	}
	FINLINE ByteReader& _read( void* ptr, size_t sz )
	{
		if( pos >= input->size() || pos + sz > input->size() )
			error = true;
		else
		{
			memcpy( ptr, &input->at( pos ), sz );
			pos += sz;
		}
		return *this;
	}
	FINLINE void* at() const { return &input->at( pos ); }
	
	ByteArray* input;
	size_t pos;
	bool error;
};

struct ByteWriter
{
	ByteWriter( ByteArray* str ) : output( str ){}
	enum { IsWriter = 1, IsReader = 0, IsText = 0, IsBinary = 1 };
	FINLINE ByteWriter& operator << ( bool& v ){ uint8_t u = v?1:0; _write( &u, sizeof(u) ); return *this; }
	FINLINE ByteWriter& operator << ( char& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( int8_t& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( uint8_t& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( int16_t& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( uint16_t& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( int32_t& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( uint32_t& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( int64_t& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( uint64_t& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( float& v ){ _write( &v, sizeof(v) ); return *this; }
	FINLINE ByteWriter& operator << ( double& v ){ _write( &v, sizeof(v) ); return *this; }
	template< class T > ByteWriter& operator << ( T& v ){ v.Serialize( *this ); return *this; }
	FINLINE ByteWriter& memory( void* ptr, size_t sz ){ return _write( ptr, sz ); }
	FINLINE ByteWriter& charbuf( char* ptr, size_t sz ){ return _write( ptr, sz ); }
	FINLINE ByteWriter& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE ByteWriter& marker( const void* ptr, size_t sz ){ output->append( (uint8_t*) ptr, sz ); return *this; }
	FINLINE ByteWriter& padding( size_t sz ){ output->reserve( output->size() + sz ); while( sz --> 0 ) output->push_back( 0 ); return *this; }
	FINLINE ByteWriter& _write( const void* ptr, size_t sz ){ output->append( (uint8_t*) ptr, sz ); return *this; }
	
	ByteArray* output;
};

struct TextReader
{
	TextReader( String* str, size_t p = 0 ) : input( str ), pos( p ), error( false ){}
	enum { IsWriter = 0, IsReader = 1, IsText = 1, IsBinary = 0 };
	FINLINE TextReader& operator << ( bool& v ){ v = !!String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( char& v ){ v = (char) String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( int8_t& v ){ v = (int8_t) String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( uint8_t& v ){ v = (uint8_t) String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( int16_t& v ){ v = (int16_t) String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( uint16_t& v ){ v = (uint16_t) String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( int32_t& v ){ v = (int32_t) String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( uint32_t& v ){ v = (uint32_t) String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( int64_t& v ){ v = String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( uint64_t& v ){ v = (uint64_t) String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( float& v ){ v = (float) String_ParseFloat( _read() ); return *this; }
	FINLINE TextReader& operator << ( double& v ){ v = String_ParseFloat( _read() ); return *this; }
	template< class T > TextReader& operator << ( T& v ){ v.Serialize( *this ); return *this; }
	TextReader& memory( void* ptr, size_t sz )
	{
		StringView it = _read();
		if( sz * 2 != it.size() )
			error = true;
		else
		{
			uint8_t* p = (uint8_t*) ptr;
			for( size_t i = 0; i < sz; ++i )
			{
				int hx0 = gethex( it[ i * 2 ] );
				int hx1 = gethex( it[ i * 2 + 1 ] );
				if( hx0 < 0 || hx0 >= 16 || hx1 < 0 || hx1 >= 16 )
				{
					error = true;
					return *this;
				}
				p[ i ] = ( hx0 << 4 ) | hx1;
			}
		}
		return *this;
	}
	TextReader& charbuf( char* ptr, size_t sz )
	{
		while( sz && pos < input->size() )
		{
			char ch = input->at( pos++ );
			if( ch == '\\' )
			{
				if( pos + 2 >= input->size() )
				{
					error = true;
					break;
				}
				char vv0 = input->at( pos++ );
				char vv1 = input->at( pos++ );
				if( !hexchar( vv0 ) || !hexchar( vv1 ) )
				{
					error = true;
					break;
				}
				ch = ( gethex( vv0 ) << 4 ) | gethex( vv1 );
			}
			*ptr++ = ch;
			sz--;
		}
		if( pos >= input->size() || sz || input->at( pos ) != '\n' )
			error = true;
		else
			pos++;
		return *this;
	}
	FINLINE TextReader& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE TextReader& marker( const void* ptr, size_t sz ){ if( StringView( (const char*) ptr, sz ) != _read() ) error = true; return *this; }
	FINLINE TextReader& padding( size_t sz ){ UNUSED( sz ); return *this; }
	FINLINE StringView _read()
	{
		if( pos >= input->size() )
		{
			error = true;
			return StringView();
		}
		else
		{
			StringView sv = StringView( &input->at( pos ), input->size() - pos ).until( "\n" );
			if( sv.size() == 0 )
				error = true;
			else
				pos += sv.size() + 1;
			return sv;
		}
	}
	
	String* input;
	size_t pos;
	bool error;
};

struct TextWriter
{
	TextWriter( String* str ) : output( str ){}
	enum { IsWriter = 1, IsReader = 0, IsText = 1, IsBinary = 0 };
	FINLINE TextWriter& operator << ( bool& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%u\n", v?1:0 ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( char& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%d\n", (int)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( int8_t& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%d\n", (int)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( uint8_t& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%u\n", (unsigned)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( int16_t& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%d\n", (int)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( uint16_t& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%u\n", (unsigned)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( int32_t& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%d\n", (int)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( uint32_t& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%u\n", (unsigned)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( int64_t& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%" PRId64 "\n", v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( uint64_t& v ){ char bfr[ 32 ]; sgrx_snprintf( bfr, 32, "%" PRIu64 "\n", v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( float& v ){ char bfr[ 1500 ]; sgrx_snprintf( bfr, 1500, "%.6g\n", v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( double& v ){ char bfr[ 1500 ]; sgrx_snprintf( bfr, 1500, "%.18g\n", v ); _write( bfr ); return *this; }
	template< class T > TextWriter& operator << ( T& v ){ v.Serialize( *this ); return *this; }
	TextWriter& memory( void* ptr, size_t sz )
	{
		char bfr[ 32 ];
		uint8_t* p = (uint8_t*) ptr;
		for( size_t i = 0; i < sz; ++i )
		{
			sgrx_snprintf( bfr, 32, "%02X", (int) p[ i ] );
			_write( bfr );
		}
		return *this;
	}
	TextWriter& charbuf( char* ptr, size_t sz )
	{
		for( size_t i = 0; i < sz; ++i )
		{
			if( ptr[ i ] > 0x20 && ptr[ i ] < 0x7f && ptr[ i ] != '\\' )
			{
				char bfr[2] = { ptr[i], 0 };
				_write( bfr );
			}
			else
			{
				char bfr[ 32 ];
				sgrx_snprintf( bfr, 32, "\\%02X", (int) (uint8_t) ptr[ i ] );
				_write( bfr );
			}
		}
		_write( "\n" );
		return *this;
	}
	FINLINE TextWriter& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE TextWriter& marker( const void* ptr, size_t sz ){ output->append( (char*) ptr, sz ); output->push_back( '\n' ); return *this; }
	FINLINE TextWriter& padding( size_t sz ){ UNUSED( sz ); return *this; }
	FINLINE TextWriter& _write( const char* ptr ){ output->append( ptr, StringLength( ptr ) ); return *this; }
	
	String* output;
};


//
// FILES
//

struct ENGINE_EXPORT DirectoryIterator
{
	DirectoryIterator( const StringView& path );
	~DirectoryIterator();
	
	bool Next();
	StringView Name();
	bool IsDirectory();
	
	struct _IntDirIter* m_int;
	
private:
	DirectoryIterator& operator = ( const DirectoryIterator& );
	DirectoryIterator( const DirectoryIterator& );
};

struct InLocalStorage
{
	ENGINE_EXPORT InLocalStorage( const StringView& path );
	ENGINE_EXPORT ~InLocalStorage();
	
	String m_path;
	
private:
	InLocalStorage& operator = ( const InLocalStorage& );
	InLocalStorage( const InLocalStorage& );
};

ENGINE_EXPORT bool DirExists( const StringView& path );
ENGINE_EXPORT bool DirCreate( const StringView& path );

ENGINE_EXPORT bool CWDGet( String& path );
ENGINE_EXPORT bool CWDSet( const StringView& path );

ENGINE_EXPORT bool LoadBinaryFile( const StringView& path, ByteArray& out );
ENGINE_EXPORT bool SaveBinaryFile( const StringView& path, const void* data, size_t size );
ENGINE_EXPORT bool LoadTextFile( const StringView& path, String& out );
ENGINE_EXPORT bool SaveTextFile( const StringView& path, const StringView& data );
ENGINE_EXPORT bool FileExists( const StringView& path );

struct IL_Item
{
	String name;
	StringTable params;
};
typedef Array< IL_Item > ItemList;

ENGINE_EXPORT bool LoadItemListFile( const StringView& path, ItemList& out );


//
// LOGGING
//


struct ENGINE_EXPORT SGRX_Log
{
	template< class T > struct Loggable
	{
		Loggable( const T& _v ) : v(_v){}
		const T& v;
	};
	template< class T > static Loggable<T> MakeLoggable( const T& v ){ return Loggable< T >( v ); }
	struct Separator
	{
		Separator( const char* s ) : sep( s ){}
		const char* sep;
	};
	enum EMod_Partial { Mod_Partial };
	enum ESpec_Date { Spec_Date };
	enum ESpec_CallStack { Spec_CallStack };
	struct RegFunc
	{
		RegFunc( const char* func, const char* file, int ln ) :
			funcname( func ), filename( file ), linenum( ln ), prev( lastfunc )
		{ lastfunc = this; }
		~RegFunc(){ lastfunc = prev; }
		const char* funcname;
		const char* filename;
		int linenum;
		RegFunc* prev;
	};
	
	bool end_newline;
	bool need_sep;
	const char* sep;
	
	static THREAD_LOCAL RegFunc* lastfunc;
	
	SGRX_Log();
	~SGRX_Log();
	void prelog();
	
	SGRX_Log& operator << ( const Separator& );
	SGRX_Log& operator << ( EMod_Partial );
	SGRX_Log& operator << ( ESpec_Date );
	SGRX_Log& operator << ( ESpec_CallStack );
	SGRX_Log& operator << ( bool );
	SGRX_Log& operator << ( int8_t );
	SGRX_Log& operator << ( uint8_t );
	SGRX_Log& operator << ( int16_t );
	SGRX_Log& operator << ( uint16_t );
	SGRX_Log& operator << ( int32_t );
	SGRX_Log& operator << ( uint32_t );
	SGRX_Log& operator << ( int64_t );
	SGRX_Log& operator << ( uint64_t );
	SGRX_Log& operator << ( float );
	SGRX_Log& operator << ( double );
	SGRX_Log& operator << ( const void* );
	SGRX_Log& operator << ( const char* );
	SGRX_Log& operator << ( const StringView& );
	SGRX_Log& operator << ( const String& );
	SGRX_Log& operator << ( const Vec2& );
	SGRX_Log& operator << ( const Vec3& );
	SGRX_Log& operator << ( const Vec4& );
	SGRX_Log& operator << ( const Quat& );
	SGRX_Log& operator << ( const Mat4& );
	template< class T > SGRX_Log& operator << ( const Loggable<T>& val ){ val.v.Log( *this ); return *this; }
	template< class T > SGRX_Log& operator << ( const Array<T>& arr ){ *this << "ARRAY";
		for( size_t i = 0; i < arr.size(); ++i ) *this << "\n\t" << i << ": " << arr[i]; return *this; }
};
#define LOG SGRX_Log()
#define LOG_ERROR SGRX_Log() << LOG_CALLSTACK << "\n!!! ERROR: "
#define LOG_WARNING SGRX_Log() << "WARNING: "
#define PARTIAL_LOG SGRX_Log::Mod_Partial
#define LOG_DATE SGRX_Log::Spec_Date
#define LOG_CALLSTACK SGRX_Log::Spec_CallStack
#define LOG_SEP( x ) SGRX_Log::Separator( x )
#define LOG_XTD( x ) SGRX_Log::MakeLoggable( x )
#define LOG_FUNCTION SGRX_Log::RegFunc __regfn( __FUNCTION__, __FILE__, __LINE__ )



//
// TESTS
//

int TestSystems();

