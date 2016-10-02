

#pragma once
#define __STDC_FORMAT_MACROS 1
#define _USE_MATH_DEFINES
#include <inttypes.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <new>


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
#define ALLOC_ALIGNED16(sz) _aligned_malloc( sz, 16 )
#define FREE_ALIGNED(ptr) _aligned_free( ptr )
#else
#define IF_GCC(x) x
#define IF_MSVC(x)
#define FINLINE inline __attribute__((__always_inline__))
#define ALIGN16(a) a __attribute__ ((aligned (16)))
#endif

ENGINE_EXPORT void* sgrx_aligned_malloc( size_t size, size_t align );
ENGINE_EXPORT void sgrx_aligned_free( void* ptr );

#define CLASS_ALIGNED_ALLOC() \
	FINLINE void* operator new( size_t sz )           { return sgrx_aligned_malloc( sz, 16 ); } \
	FINLINE void  operator delete( void* ptr )        { sgrx_aligned_free( ptr ); } \
	FINLINE void* operator new( size_t, void* ptr )   { return ptr; } \
	FINLINE void  operator delete( void*, void* )     {} \
	FINLINE void* operator new[]( size_t sz )         { return sgrx_aligned_malloc( sz, 16 ); } \
	FINLINE void  operator delete[]( void* ptr )      { sgrx_aligned_free( ptr ); } \
	FINLINE void* operator new[]( size_t, void* ptr ) { return ptr; } \
	FINLINE void  operator delete[]( void*, void* )   {}

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

#define SAFE_DELETE( x ) if(x){ delete x; x = NULL; }
#define SGRX_STR_EXPAND( tok ) #tok
#define SGRX_STR( tok ) SGRX_STR_EXPAND( tok )
#define SGRX_CAST( t, to, from ) t to = (t) from
#define SGRX_ARRAY_SIZE( arr ) (sizeof(arr)/sizeof((arr)[0]))
#ifndef UNUSED
#  define UNUSED( x ) (void) x
#endif
#define STRLIT_LEN( x ) (sizeof(x)-1)
#define STRLIT_BUF( x ) x, STRLIT_LEN( x )
#define SGRX_GLUE(a,b) __SGRX_GLUE(a,b)
#define __SGRX_GLUE(a,b) a ## b
#define SGRX_CASSERT(expr, msg) typedef char SGRX_GLUE (compiler_verify_, msg) [(expr) ? (+1) : (-1)]

extern const char* SGRX_HEX_LOWER;
extern const char* SGRX_HEX_UPPER;


// compiler/toolchain padding
#define streq( a, b ) (!strcmp(a,b))
#define strpeq( a, bp ) (!strncmp(a,bp,strlen(bp)))
ENGINE_EXPORT void NOP( int x );
ENGINE_EXPORT uint32_t sgrx_crc32( const void* buf, size_t len, uint32_t in_crc );
ENGINE_EXPORT void sgrx_quicksort( void* array, size_t length, size_t size,
	int(*compare)(const void*, const void*, void*), void* userdata );
ENGINE_EXPORT void sgrx_combsort( void* array, size_t length, size_t size,
	bool(*compless)(const void*, const void*, void*), void* userdata );

inline size_t sgrx_snlen( const char* str, size_t ilen = (size_t) -1 )
{
	for( size_t i = 0; i < ilen; ++i )
		if( str[ i ] == '\0' )
			return i;
	return ilen;
}

inline int sgrx_sncopy( char* buf, size_t len, const char* str, size_t ilen = (size_t) -1 )
{
	if( len == 0 )
		return -1;
	len--;
	if( len > ilen )
		len = ilen;
	memcpy( buf, str, len );
	buf[ len ] = 0;
	return 0;
}

inline int sgrx_vsnprintf( char* buf, size_t len, const char* fmt, va_list args )
{
	if( len == 0 )
		return -1;
	int ret = vsnprintf( buf, len, fmt, args );
	buf[ len - 1 ] = 0;
	return ret;
}

inline int sgrx_snprintf( char* buf, size_t len, const char* fmt, ... )
{
	if( len == 0 )
		return -1;
	va_list args;
	va_start( args, fmt );
	int ret = vsnprintf( buf, len, fmt, args );
	va_end( args );
	buf[ len - 1 ] = 0;
	return ret;
}

// RT system padding
ENGINE_EXPORT double sgrx_hqtime();


//
// THREADING
//


ENGINE_EXPORT int32_t sgrx_atomic_inc( volatile int32_t* ptr );
ENGINE_EXPORT int32_t sgrx_atomic_dec( volatile int32_t* ptr );
ENGINE_EXPORT int32_t sgrx_atomic_cmpxchg( volatile int32_t* ptr, int32_t test, int32_t val );

ENGINE_EXPORT void sgrx_sleep( uint32_t ms );
ENGINE_EXPORT int sgrx_numcpus();

struct IF_GCC(ENGINE_EXPORT) SGRX_Thread
{
	typedef void (*Proc)(void*);
	ENGINE_EXPORT SGRX_Thread();
	ENGINE_EXPORT ~SGRX_Thread();
	ENGINE_EXPORT void Start( Proc fn, void* data );
	ENGINE_EXPORT void Join();
	
	void* handle;
	Proc m_nextproc;
	void* m_nextdata;
};

struct SGRX_Mutex
{
	SGRX_Mutex() : lock(0){}
	bool TryLock(){ return 0 == sgrx_atomic_cmpxchg( &lock, 0, 1 ); }
	void Lock(){ while( TryLock() == false ); }
	void Unlock(){ sgrx_atomic_cmpxchg( &lock, 1, 0 ); }
	
	volatile int32_t lock;
};

struct SGRX_ScopedMtxLock
{
	SGRX_ScopedMtxLock( SGRX_Mutex* mtx ) : m_mutex( mtx ) { mtx->Lock(); }
	~SGRX_ScopedMtxLock(){ m_mutex->Unlock(); }
	SGRX_Mutex* m_mutex;
};


//
// MATH etc
//


#define SMALL_FLOAT 0.001f
#define ENGINE_MAX_PATH 256

#define SPACE_CHARS " \t\n"
#define HSPACE_CHARS " \t"

#define FLT_PI float(M_PI)
#define DBL_PI M_PI
#define F_DEG2RAD (FLT_PI/180.0f)
#define F_RAD2DEG (180.0f/FLT_PI)
#define DEG2RAD( x ) ((x)*F_DEG2RAD)
#define RAD2DEG( x ) ((x)*F_RAD2DEG)

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
FINLINE double safe_ddiv( double x, double y ){ if( y == 0 ) return 0; return x / y; }
FINLINE double safe_dmod( double x, double y ){ if( y == 0 ) return 0; return fmod( x, y ); }
inline float normalize_angle( float x ){ x = fmodf( x, (float) FLT_PI * 2.0f ); return IIF( x < 0.0f, x + (float) FLT_PI * 2.0f, x ); }
inline float normalize_angle2( float x ){ x = normalize_angle( x ); if( x >= FLT_PI ) x -= FLT_PI * 2.0f; return x; }
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


template< class T > FINLINE void TSET_FLAG( T& var, T flag, bool val )
{
	if( val )
		var |= flag;
	else
		var &= ~flag;
}
template< class T > FINLINE T TMIN( const T& a, const T& b ){ return a < b ? a : b; }
template< class T > FINLINE T TMAX( const T& a, const T& b ){ return a > b ? a : b; }
template< class T > FINLINE T TCLAMP( const T& v, const T& vmin, const T& vmax )
{
	return v < vmin ? vmin : ( v > vmax ? vmax : v );
}
template< class T > FINLINE void TMEMSET( T* a, size_t c, const T& v )
{
	for( size_t i = 0; i < c; ++i )
		a[ i ] = v;
}
template< class T > FINLINE void TSWAP( T& a, T& b ){ T tmp( a ); a = b; b = tmp; }
template< class T > void TMEMSWAP( T& a, T& b )
{
	char tmp[ sizeof(T) ];
	memcpy( tmp, &a, sizeof(T) );
	memcpy( &a, &b, sizeof(T) );
	memcpy( &b, tmp, sizeof(T) );
}
template< class T, class S > FINLINE T TLERP( const T& a, const T& b, const S& s ){ return a * ( S(1) - s ) + b * s; }
template< class S, class T > FINLINE S TREVLERP( const T& a, const T& b, const T& s ){ if( a == b ) return a; return ( s - a ) / ( b - a ); }

template< class T > FINLINE T DefaultValue(){ return T(0); }

template< class T > struct IVState
{
	FINLINE IVState() : prev(DefaultValue<T>()), curr(DefaultValue<T>()) {}
	FINLINE IVState( const T& start ) : prev(start), curr(start) {}
	FINLINE void Advance( const T& next ){ prev = curr; curr = next; }
	FINLINE void Set( const T& val ){ prev = val; curr = val; }
	FINLINE T Get( float q ){ return TLERP( prev, curr, q ); }
	
	T prev;
	T curr;
};

template< class T > struct TempSwapper
{
	TempSwapper( T& t, const T& v ) : tgt(t), backup(t){ tgt = v; }
	~TempSwapper(){ tgt = backup; }
	T& tgt;
	T backup;
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

template<> inline Vec2 DefaultValue<Vec2>(){ return V2(0); }


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
FINLINE Vec3 V3( Vec2 v2, float z ){ Vec3 v = { v2.x, v2.y, z }; return v; }
FINLINE Vec3 V3( float x, float y, float z ){ Vec3 v = { x, y, z }; return v; }
FINLINE Vec3 V3P( float* p ){ Vec3 v = { p[0], p[1], p[2] }; return v; }
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
FINLINE Vec3 Vec3Reflect( const Vec3& dir, const Vec3& nrm )
{
	return dir - 2 * nrm * Vec3Dot( dir, nrm );
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

template<> inline Vec3 DefaultValue<Vec3>(){ return V3(0); }


//
// VEC4
//

struct ENGINE_EXPORT Vec4
{
	float x, y, z, w;
	
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
};

FINLINE Vec4 operator + ( float f, const Vec4& v ){ Vec4 out = { f + v.x, f + v.y, f + v.z, f + v.w }; return out; }
FINLINE Vec4 operator - ( float f, const Vec4& v ){ Vec4 out = { f - v.x, f - v.y, f - v.z, f - v.w }; return out; }
FINLINE Vec4 operator * ( float f, const Vec4& v ){ Vec4 out = { f * v.x, f * v.y, f * v.z, f * v.w }; return out; }
FINLINE Vec4 operator / ( float f, const Vec4& v ){ Vec4 out = { f / v.x, f / v.y, f / v.z, f / v.w }; return out; }

FINLINE Vec4 V4( float x ){ Vec4 v = { x, x, x, x }; return v; }
FINLINE Vec4 V4( float x, float a ){ Vec4 v = { x, x, x, a }; return v; }
FINLINE Vec4 V4( const Vec3& v3, float w ){ Vec4 v = { v3.x, v3.y, v3.z, w }; return v; }
FINLINE Vec4 V4( float x, float y, float z, float w ){ Vec4 v = { x, y, z, w }; return v; }
FINLINE float Vec4Dot( const Vec4& v1, const Vec4& v2 ){ return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
inline Vec4 TLERP( const Vec4& a, const Vec4& b, const Vec4& s ){ return a * ( V4(1) - s ) + b * s; }

template<> inline Vec4 DefaultValue<Vec4>(){ return V4(0); }

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
	
	static FINLINE Quat CreateAxisAngle( const Vec3& v, float a ){ return CreateAxisAngle( v.x, v.y, v.z, a ); }
	static FINLINE Quat CreateAxisAxis( const Vec3& d0, const Vec3& d1 )
	{
		Vec3 axis = Vec3Cross( d0, d1 ).Normalized();
		float angle = acosf( clamp( Vec3Dot( d0, d1 ), -1, 1 ) );
		return CreateAxisAngle( axis, angle );
	}
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
	static Quat CreateFromXYZ( const Vec3& angles )
	{
		return ( Quat::CreateAxisAngle( 1,0,0, angles.x ) *
			Quat::CreateAxisAngle( 0,1,0, angles.y ) ) *
			Quat::CreateAxisAngle( 0,0,1, angles.z );
	}
	
	FINLINE Quat operator + ( const Quat& o ) const { Quat q = { x + o.x, y + o.y, z + o.z, w + o.w }; return q; }
	FINLINE Quat operator - ( const Quat& o ) const { Quat q = { x - o.x, y - o.y, z - o.z, w - o.w }; return q; }
	Quat operator * ( const Quat& o ) const
	{
		const Quat& q1 = o, &q2 = *this;
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
	FINLINE bool operator != ( const Quat& o ) const { return x != o.x || y != o.y || z != o.z || w != o.w; }
	
	Vec3 Transform( const Vec3& p ) const // TODO FIX
	{
		Quat qpos = { p.x, p.y, p.z, 0 };
		Quat qconj = Conjugate();
		Quat q_ = this->Normalized();
		q_ = qpos * q_;
		q_ = qconj * q_;
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
	Vec3 ToXYZ() const;
};
	
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

ENGINE_EXPORT Vec3 CalcAngularVelocity( const Quat& qa, const Quat& qb );

template<> inline Quat DefaultValue<Quat>(){ return Quat::Identity; }


//
// YAWPITCH
//

struct YawPitch
{
	float yaw, pitch;
	
	void Normalize()
	{
		yaw = normalize_angle( yaw );
		pitch = normalize_angle( pitch );
	}
	YawPitch Normalized() const
	{
		YawPitch out = { normalize_angle( yaw ), normalize_angle( pitch ) };
		return out;
	};
	YawPitch Abs() const
	{
		YawPitch out = { fabsf( yaw ), fabsf( pitch ) };
		return out;
	}
	YawPitch Scaled( float f ) const
	{
		YawPitch out = { yaw * f, pitch * f };
		return out;
	}
	
	Vec3 ToVec3() const
	{
		float cy = cosf( yaw ), sy = sinf( yaw );
		float cp = cosf( pitch ), sp = sinf( pitch );
		return V3( cy * cp, sy * cp, sp );
	}
	void TurnTo( const YawPitch& tgt, const YawPitch& speedDelta )
	{
		YawPitch ypend = tgt.Normalized();
		YawPitch ypstart = Normalized();
		if( fabs( ypend.yaw - ypstart.yaw ) > M_PI )
			ypstart.yaw += ypend.yaw > ypstart.yaw ? M_PI * 2 : -M_PI * 2;
		if( fabs( ypend.pitch - ypstart.pitch ) > M_PI )
			ypstart.pitch += ypend.pitch > ypstart.pitch ? M_PI * 2 : -M_PI * 2;
		yaw = ypstart.yaw + sign( ypend.yaw - ypstart.yaw ) * TMIN( fabsf( ypend.yaw - ypstart.yaw ), speedDelta.yaw );
		pitch = ypstart.pitch + sign( ypend.pitch - ypstart.pitch ) * TMIN( fabsf( ypend.pitch - ypstart.pitch ), speedDelta.pitch );
	}
};
FINLINE YawPitch YP( float y, float p ){ YawPitch out = { y, p }; return out; }
FINLINE YawPitch YP( float f ){ return YP( f, f ); }
inline YawPitch YP( const Vec3& v )
{
	return YP( normalize_angle( atan2( v.y, v.x ) ),
		normalize_angle( atan2( v.z, v.ToVec2().Length() ) ) );
}
inline YawPitch YawPitchDist( YawPitch from, YawPitch to )
{
	from = from.Normalized();
	to = to.Normalized();
	if( fabs( to.yaw - from.yaw ) > M_PI )
		from.yaw += to.yaw > from.yaw ? M_PI * 2 : -M_PI * 2;
	if( fabs( to.pitch - from.pitch ) > M_PI )
		from.pitch += to.pitch > from.pitch ? M_PI * 2 : -M_PI * 2;
	return YP( to.yaw - from.yaw, to.pitch - from.pitch );
}
inline bool YawPitchAlmostEqual( const YawPitch& a, const YawPitch& b )
{
	YawPitch dst = YawPitchDist( a, b ).Abs();
	return dst.yaw < SMALL_FLOAT && dst.pitch < SMALL_FLOAT;
}



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
	
	static Mat4 CreateFromPtr( const float* v16f )
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
	static Mat4 CreateUI( float x0, float y0, float x1, float y1, float z0 = -1.0f, float z1 = 1.0f )
	{
		float w = x1 - x0;
		float h = y1 - y0;
		Mat4 m =
		{
			2.0f / w, 0, 0, 0,
			0, -2.0f / h, 0, 0,
			0, 0, 1.0f / ( z1 - z0 ), 0,
			-1 - x0 / w * 2.0f, 1 + y0 / w * 2.0f, z0 / ( z0 - z1 ), 1
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
	static Mat4 CreateRotationBetweenVectors( const Vec3& a, const Vec3& b, float scale = 1.0f )
	{
		float angle = acosf( clamp( Vec3Dot( a, b ), -1, 1 ) );
		Vec3 axis = Vec3Cross( a, b );
		if( axis.LengthSq() < SMALL_FLOAT )
			axis = Vec3Cross( a, a.Shuffle() );
		return CreateRotationAxisAngle( axis.Normalized(), angle * scale );
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
		Mat4 out = Identity;
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
		Mat4 tmp = rot;
		// fast prescale
		tmp.m[0][0] *= scale.x;
		tmp.m[1][0] *= scale.x;
		tmp.m[2][0] *= scale.x;
		tmp.m[0][1] *= scale.y;
		tmp.m[1][1] *= scale.y;
		tmp.m[2][1] *= scale.y;
		tmp.m[0][2] *= scale.z;
		tmp.m[1][2] *= scale.z;
		tmp.m[2][2] *= scale.z;
		// fast translate
		tmp.SetTranslation( pos );
		return tmp;
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
	void SetTranslation( const Vec3& v )
	{
		m[3][0] = v.x;
		m[3][1] = v.y;
		m[3][2] = v.z;
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
	bool InvertTo( Mat4& out ) const;
	FINLINE Mat4 Inverted() const
	{
		Mat4 out = Mat4::Identity;
		InvertTo( out );
		return out;
	}
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
	Mat4& MultiplyAffine( const Mat4& A, const Mat4& B )
	{
		m[0][0] = A.m[0][0] * B.m[0][0] + A.m[0][1] * B.m[1][0] + A.m[0][2] * B.m[2][0];
		m[0][1] = A.m[0][0] * B.m[0][1] + A.m[0][1] * B.m[1][1] + A.m[0][2] * B.m[2][1];
		m[0][2] = A.m[0][0] * B.m[0][2] + A.m[0][1] * B.m[1][2] + A.m[0][2] * B.m[2][2];
		m[1][0] = A.m[1][0] * B.m[0][0] + A.m[1][1] * B.m[1][0] + A.m[1][2] * B.m[2][0];
		m[1][1] = A.m[1][0] * B.m[0][1] + A.m[1][1] * B.m[1][1] + A.m[1][2] * B.m[2][1];
		m[1][2] = A.m[1][0] * B.m[0][2] + A.m[1][1] * B.m[1][2] + A.m[1][2] * B.m[2][2];
		m[2][0] = A.m[2][0] * B.m[0][0] + A.m[2][1] * B.m[1][0] + A.m[2][2] * B.m[2][0];
		m[2][1] = A.m[2][0] * B.m[0][1] + A.m[2][1] * B.m[1][1] + A.m[2][2] * B.m[2][1];
		m[2][2] = A.m[2][0] * B.m[0][2] + A.m[2][1] * B.m[1][2] + A.m[2][2] * B.m[2][2];
		m[3][0] = A.m[3][0] * B.m[0][0] + A.m[3][1] * B.m[1][0] + A.m[3][2] * B.m[2][0] + B.m[3][0];
		m[3][1] = A.m[3][0] * B.m[0][1] + A.m[3][1] * B.m[1][1] + A.m[3][2] * B.m[2][1] + B.m[3][1];
		m[3][2] = A.m[3][0] * B.m[0][2] + A.m[3][1] * B.m[1][2] + A.m[3][2] * B.m[2][2] + B.m[3][2];
		m[0][3] = m[1][3] = m[2][3] = 0;
		m[3][3] = 1;
		return *this;
	}
	
	void Scale( Vec3 scale )
	{
		*this = Identity;
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
};
FINLINE Mat4 M4MulAff( const Mat4& a, const Mat4& b ){ Mat4 out; out.MultiplyAffine( a, b ); return out; }


struct ENGINE_EXPORT Mat4x3
{
	union
	{
		float a[12];
		float m[4][3];
	};
	
	static const Mat4x3 Identity;
	
	static Mat4x3 CreateFromPtr( const float* v12f )
	{
		Mat4x3 m;
		memcpy( m.a, v12f, sizeof(float[12]) );
		return m;
	}
	
	static Mat4x3 CreateScale( float x, float y, float z )
	{
		Mat4x3 m = Identity;
		m.m[0][0] = x;
		m.m[1][1] = y;
		m.m[2][2] = z;
		return m;
	}
	static FINLINE Mat4x3 CreateScale( const Vec3& v ){ return CreateScale( v.x, v.y, v.z ); }
	static Mat4x3 CreateRotationDefAxis( int axis0, int axis1, float angle )
	{
		float s = sin( angle );
		float c = cos( angle );
		Mat4x3 m = Identity;
		m.m[ axis0 ][ axis0 ] = c;
		m.m[ axis1 ][ axis0 ] = -s;
		m.m[ axis0 ][ axis1 ] = s;
		m.m[ axis1 ][ axis1 ] = c;
		return m;
	}
	static FINLINE Mat4x3 CreateRotationX( float angle ){ return CreateRotationDefAxis( 1, 2, angle ); }
	static FINLINE Mat4x3 CreateRotationY( float angle ){ return CreateRotationDefAxis( 2, 0, angle ); }
	static FINLINE Mat4x3 CreateRotationZ( float angle ){ return CreateRotationDefAxis( 0, 1, angle ); }
	static Mat4x3 CreateRotationXYZ( float x, float y, float z )
	{
		Mat4x3 rx = CreateRotationX( x );
		Mat4x3 ry = CreateRotationY( y );
		Mat4x3 rz = CreateRotationZ( z );
		Mat4x3 rot, rot2;
		rot2.Multiply( rx, ry );
		rot.Multiply( rot2, rz );
		return rot;
	}
	static FINLINE Mat4x3 CreateRotationXYZ( const Vec3& rot_angles ){ return CreateRotationXYZ( rot_angles.x, rot_angles.y, rot_angles.z ); }
	static Mat4x3 CreateRotationAxisAngle( float x, float y, float z, float angle )
	{
		float s = sin( angle );
		float c = cos( angle );
		float Ic = 1 - c;
		
		Mat4x3 m = Identity;
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
	static FINLINE Mat4x3 CreateRotationAxisAngle( const Vec3& axis, float angle ){ return CreateRotationAxisAngle( axis.x, axis.y, axis.z, angle ); }
	static Mat4x3 CreateRotationBetweenVectors( const Vec3& a, const Vec3& b, float scale = 1.0f )
	{
		float angle = acosf( clamp( Vec3Dot( a, b ), -1, 1 ) );
		Vec3 axis = Vec3Cross( a, b );
		if( axis.LengthSq() < SMALL_FLOAT )
			axis = Vec3Cross( a, a.Shuffle() );
		return CreateRotationAxisAngle( axis.Normalized(), angle * scale );
	}
	static Mat4x3 CreateRotationFromQuat( const Quat& q )
	{
		float a = -q.w;
		float b = q.x;
		float c = q.y;
		float d = q.z;
		float a2 = a*a;
		float b2 = b*b;
		float c2 = c*c;
		float d2 = d*d;
		
		Mat4x3 m;
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
		
		return m;
	}
	static FINLINE Mat4x3 CreateTranslation( float x, float y, float z )
	{
		Mat4x3 out = Identity;
		out.m[3][0] = x;
		out.m[3][1] = y;
		out.m[3][2] = z;
		return out;
	}
	static FINLINE Mat4x3 CreateTranslation( const Vec3& v ){ return CreateTranslation( v.x, v.y, v.z ); }
	static FINLINE Mat4x3 CreateSRT( const Vec3& scale, const Vec3& rot_angles, const Vec3& pos )
	{
		return CreateSXT( scale, CreateRotationXYZ( rot_angles ), pos );
	}
	static FINLINE Mat4x3 CreateSRT( const Vec3& scale, const Quat& rot, const Vec3& pos )
	{
		return CreateSXT( scale, CreateRotationFromQuat( rot ), pos );
	}
	static Mat4x3 CreateSXT( const Vec3& scale, const Mat4x3& rot, const Vec3& pos )
	{
		Mat4x3 mscl = CreateScale( scale );
		Mat4x3 mtrn = CreateTranslation( pos );
		Mat4x3 mtmp1, mtmp2;
		mtmp1.Multiply( mscl, rot );
		mtmp2.Multiply( mtmp1, mtrn );
		return mtmp2;
	}
	Vec3 GetTranslation() const
	{
		return V3( m[3][0], m[3][1], m[3][2] );
	}
	void SetTranslation( const Vec3& v )
	{
		m[3][0] = v.x;
		m[3][1] = v.y;
		m[3][2] = v.z;
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
			v.w,
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
		return out;
	}
	FINLINE Vec3 TransformNormal( const Vec3& nrm ) const { return Transform( nrm, 0.0f ); }
	
	FINLINE bool operator == ( const Mat4x3& o ) const
	{
		for( int i = 0; i < 12; ++i )
			if( a[ i ] != o.a[ i ] )
				return false;
		return true;
	}
	FINLINE bool operator != ( const Mat4x3& o ) const { return !( *this == o ); }
	
	bool InvertTo( Mat4x3& out ) const;
	FINLINE Mat4x3 Inverted() const
	{
		Mat4x3 out = Identity;
		InvertTo( out );
		return out;
	}
	
	Mat4x3& Multiply( const Mat4x3& A, const Mat4x3& B )
	{
		m[0][0] = A.m[0][0] * B.m[0][0] + A.m[0][1] * B.m[1][0] + A.m[0][2] * B.m[2][0];
		m[0][1] = A.m[0][0] * B.m[0][1] + A.m[0][1] * B.m[1][1] + A.m[0][2] * B.m[2][1];
		m[0][2] = A.m[0][0] * B.m[0][2] + A.m[0][1] * B.m[1][2] + A.m[0][2] * B.m[2][2];
		m[1][0] = A.m[1][0] * B.m[0][0] + A.m[1][1] * B.m[1][0] + A.m[1][2] * B.m[2][0];
		m[1][1] = A.m[1][0] * B.m[0][1] + A.m[1][1] * B.m[1][1] + A.m[1][2] * B.m[2][1];
		m[1][2] = A.m[1][0] * B.m[0][2] + A.m[1][1] * B.m[1][2] + A.m[1][2] * B.m[2][2];
		m[2][0] = A.m[2][0] * B.m[0][0] + A.m[2][1] * B.m[1][0] + A.m[2][2] * B.m[2][0];
		m[2][1] = A.m[2][0] * B.m[0][1] + A.m[2][1] * B.m[1][1] + A.m[2][2] * B.m[2][1];
		m[2][2] = A.m[2][0] * B.m[0][2] + A.m[2][1] * B.m[1][2] + A.m[2][2] * B.m[2][2];
		m[3][0] = A.m[3][0] * B.m[0][0] + A.m[3][1] * B.m[1][0] + A.m[3][2] * B.m[2][0] + B.m[3][0];
		m[3][1] = A.m[3][0] * B.m[0][1] + A.m[3][1] * B.m[1][1] + A.m[3][2] * B.m[2][1] + B.m[3][1];
		m[3][2] = A.m[3][0] * B.m[0][2] + A.m[3][1] * B.m[1][2] + A.m[3][2] * B.m[2][2] + B.m[3][2];
		return *this;
	}
	Mat4x3 operator * ( const Mat4x3& o ) const
	{
		Mat4x3 n;
		n.Multiply( *this, o );
		return n;
	}
	
	template< class T > void Serialize( T& arch )
	{
		if( T::IsText )
			arch.marker( "Mat4x3" );
		for( int i = 0; i < 12; ++i )
			arch << a[ i ];
	}
};

ENGINE_EXPORT Quat TransformQuaternion( const Quat& q, const Mat4& m );



ENGINE_EXPORT Vec3 PointLineDistance3( const Vec3& pt, const Vec3& l0, const Vec3& l1 );
FINLINE float PointLineDistance( const Vec3& pt, const Vec3& l0, const Vec3& l1 )
{
	return PointLineDistance3( pt, l0, l1 ).Length();
}
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
ENGINE_EXPORT void TransformAABB( Vec3& bbmin, Vec3& bbmax, const Mat4& mtx );



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
	template< class S > void Serialize( S& arch )
	{
		if( S::IsReader && !item )
		{
			item = T::UnserializeCreate( arch );
			ASSERT( item && "UnserializeCreate" );
			item->Acquire();
		}
		else
		{
			ASSERT( item );
		}
		arch << *item;
	}
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
	FINLINE void append( const Array& a ){ append( a.m_data, a.m_size ); }
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
	void move_item( size_t from, size_t to )
	{
		ASSERT( from < m_size );
		ASSERT( to < m_size );
		size_t incr = to > from ? 1 : size_t(-1);
		for( ; to != from; from += incr )
		{
			TSWAP( m_data[ from ], m_data[ from + incr ] );
		}
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
};

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

template< class T > void TSWAP( Array<T>& a, Array<T>& b )
{
	TSWAP( a.m_data, b.m_data );
	TSWAP( a.m_size, b.m_size );
	TSWAP( a.m_mem, b.m_mem );
}

typedef Array< uint8_t > UInt8Array;
typedef Array< uint16_t > UInt16Array;
typedef Array< uint32_t > UInt32Array;

struct ByteArray : UInt8Array
{
	ByteArray() : UInt8Array(){}
	ByteArray( const ByteArray& a ) : UInt8Array(a){}
	ByteArray( const void* v, size_t sz ) : UInt8Array( (const uint8_t*) v, sz ){}
	void insert( size_t at, const void* data, size_t size ){ UInt8Array::insert( at, (const uint8_t*) data, size ); }
	void assign( const void* data, size_t size ){ UInt8Array::assign( (const uint8_t*) data, size ); }
	void append( const void* data, size_t size ){ UInt8Array::append( (const uint8_t*) data, size ); }
	void append( const ByteArray& a ){ append( a.m_data, a.m_size ); }
	size_t find_first_bytes_at( const void* data, size_t size, size_t from = 0 )
	{
		size_t count = m_size / size;
		for( size_t i = from; i < count; ++i )
			if( memcmp( data, m_data + i * size, size ) == 0 )
				return i;
		return NOT_FOUND;
	}
	size_t find_or_add_bytes( const void* data, size_t size, size_t from = 0 )
	{
		size_t id = find_first_bytes_at( data, size, from );
		if( id == NOT_FOUND )
		{
			id = m_size / size;
			append( data, size );
		}
		return id;
	}
};


//
// STRINGS
//

FINLINE size_t StringLength( const char* str ){ const char* o = str; while( *str ) str++; return str - o; }

struct String : Array< char >
{
	String() : Array(){}
	String( const char* str ) : Array( str, StringLength( str ) ){}
	String( const char* str, size_t size ) : Array( str, size ){}
	String( const String& s ) : Array( s ){}
	
	FINLINE void append( const struct StringView& sv );
	FINLINE void append( const char* str, size_t sz ){ Array::append( str, sz ); }
	FINLINE void append( const char* str ){ append( str, StringLength( str ) ); }
	FINLINE void insert( size_t at, char val ){ Array::insert( at, val ); }
	FINLINE void insert( size_t at, const char* data, size_t size ){ Array::insert( at, data, size ); }
	FINLINE void insert( size_t i, const struct StringView& sv );
	template< class TA > void Serialize( TA& arch )
	{
		uint32_t sz = m_size;
		arch << sz;
		if( TA::IsReader )
			resize( sz );
		arch.charbuf( m_data, m_size );
	}
};

struct IF_GCC(ENGINE_EXPORT) StringView
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
	
	FINLINE const char& operator [] ( size_t i ) const { ASSERT( i < m_size ); return m_str[ i ]; }
	
	FINLINE bool operator == ( const StringView& sv ) const { return m_size == sv.m_size && !memcmp( m_str, sv.m_str, m_size ); }
	FINLINE bool operator != ( const StringView& sv ) const { return !( *this == sv ); }
	FINLINE int compare_to( const StringView& other ) const
	{
		int rv = memcmp( m_str, other.m_str, TMIN( m_size, other.m_size ) );
		if( rv )
			return rv;
		if( m_size < other.m_size )
			return -1;
		else if( m_size > other.m_size )
			return 1;
		return 0;
	};
	
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
	FINLINE int count( const StringView& substr ) const
	{
		int out = 0;
		size_t at = 0;
		for(;;)
		{
			at = find_first_at( substr, at );
			if( at != NOT_FOUND )
			{
				at++;
				out++;
			}
			else
				break;
		}
		return out;
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
	
	FINLINE StringView from( const StringView& substr, size_t from = 0 ) const
	{
		size_t pos = find_first_at( substr, from, m_size );
		return StringView( m_str + pos, m_size - pos );
	}
	FINLINE StringView after( const StringView& substr ) const
	{
		if( substr.m_size > m_size )
			return StringView( m_str + m_size, 0 );
		size_t pos = find_first_at( substr, 0, m_size - substr.m_size ) + substr.m_size;
		return StringView( m_str + pos, m_size - pos );
	}
	FINLINE StringView until( const StringView& substr, size_t from = 0 ) const
	{
		size_t pos = find_first_at( substr, from, m_size );
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
	FINLINE StringView from_last( const StringView& substr ) const
	{
		size_t pos = find_last_at( substr );
		return pos == NOT_FOUND ? StringView( m_str + m_size, 0 ) : part( pos );
	}
	FINLINE StringView after_last( const StringView& substr ) const
	{
		size_t pos = find_last_at( substr );
		return pos == NOT_FOUND ? StringView( m_str + m_size, 0 ) : part( pos + substr.size() );
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
	FINLINE StringView take( size_t n )
	{
		StringView p = part( 0, n );
		skip( n );
		return p;
	}
	FINLINE void ltrim( const StringView& chars )
	{
		while( m_size && chars.is_any( *m_str ) )
		{
			m_str++;
			m_size--;
		}
	}
	FINLINE void rtrim( const StringView& chars )
	{
		while( m_size && chars.is_any( m_str[ m_size - 1 ] ) )
			m_size--;
	}
	FINLINE void trim( const StringView& chars )
	{
		ltrim( chars );
		rtrim( chars );
	}
	
	int64_t parse_int();
	double parse_float();
	
	ENGINE_EXPORT bool equal_lower( const StringView& o );
	ENGINE_EXPORT bool match_loose( const StringView& substr );
	ENGINE_EXPORT bool match( const StringView& regex );
	
	FINLINE operator String () const { return String( m_str, m_size ); }
};

// shorthand single arg autoconversion
FINLINE StringView SV( const StringView& sv = StringView() ){ return sv; }

FINLINE void String::append( const StringView& sv )
{
	append( sv.m_str, sv.m_size );
}

FINLINE void String::insert( size_t i, const struct StringView& sv )
{
	insert( i, sv.m_str, sv.m_size );
}

struct SGRX_Regex
{
	ENGINE_EXPORT SGRX_Regex( const StringView& regex, const char* mods );
	ENGINE_EXPORT ~SGRX_Regex();
	FINLINE bool Valid(){ return m_R != NULL; }
	ENGINE_EXPORT bool Match( const StringView& str, size_t off = 0 );
	
	void* m_R;
};


template< class T > struct RemoveConst { typedef T type; };
template< class T > struct RemoveConst<const T> { typedef T type; };


template< class T > struct IF_GCC(ENGINE_EXPORT) ArrayView
{
	typedef T value_type;
	typedef const T const_type;
	typedef typename RemoveConst<T>::type mutable_type;
	
	T* m_data;
	size_t m_size;
	
	FINLINE ArrayView() : m_data( NULL ), m_size( 0 ){}
	FINLINE ArrayView( T* data, size_t size ) : m_data( data ), m_size( size ){}
	FINLINE ArrayView( const Array<const_type>& arr ) : m_data( arr.m_data ), m_size( arr.m_size ){}
	FINLINE ArrayView( const Array<mutable_type>& arr ) : m_data( arr.m_data ), m_size( arr.m_size ){}
	FINLINE ArrayView( const ArrayView& av ) : m_data( av.m_data ), m_size( av.m_size ){}
	
	FINLINE T* data() const{ return m_data; }
	FINLINE size_t size() const { return m_size; }
	FINLINE operator bool() const { return m_data && m_size; }
	FINLINE T* begin() const { return m_data; }
	FINLINE T* end() const { return m_data + m_size; }
	
	FINLINE T& operator [] ( size_t i ) const { ASSERT( i < m_size ); return m_data[ i ]; }
	
	FINLINE bool operator == ( const ArrayView& av ) const
	{
		if( m_size != av.m_size )
			return false;
		for( size_t i = 0; i < m_size; ++i )
		{
			if( !( m_data[ i ] == av.m_data[ i ] ) )
				return false;
		}
		return true;
	}
	FINLINE bool operator != ( const ArrayView& av ) const { return !( *this == av ); }
	
	FINLINE ArrayView part( size_t start, size_t count = NOT_FOUND ) const
	{
		if( start > m_size )
			start = m_size;
		if( count > m_size || start + count > m_size )
			count = m_size - start;
		return ArrayView( m_data + start, count );
	}
	FINLINE bool skip( size_t n )
	{
		if( n > m_size )
		{
			m_data += m_size;
			m_size = 0;
			return false;
		}
		m_data += n;
		m_size -= n;
		return true;
	}
	FINLINE ArrayView take( size_t n )
	{
		ArrayView p = part( 0, n );
		skip( n );
		return p;
	}
	
	FINLINE operator Array<T> () const { return Array<T>( m_data, m_size ); }
	FINLINE operator ArrayView<const T> () const { return ArrayView<const T>( m_data, m_size ); }
};

typedef ArrayView< uint8_t > ByteView;
typedef ArrayView< const uint8_t > CByteView;


template< class T > struct StridingArrayView
{
	typedef T value_type;
	
	const char* m_data;
	size_t m_count;
	size_t m_stride;
	
	FINLINE StridingArrayView() : m_data( NULL ), m_count( 0 ), m_stride( 0 ){}
	FINLINE StridingArrayView( const void* data, size_t count, size_t stride = sizeof(T) ) :
		m_data( (const char*) data ), m_count( count ), m_stride( stride ){}
	FINLINE StridingArrayView( const Array<T>& arr ) :
		m_data( (const char*) arr.m_data ), m_count( arr.m_size ), m_stride( sizeof(T) ){}
	FINLINE StridingArrayView( const ArrayView<T>& av ) :
		m_data( (const char*) av.m_data ), m_count( av.m_size ), m_stride( sizeof(T) ){}
	FINLINE StridingArrayView( const StridingArrayView& sav ) :
		m_data( sav.m_data ), m_count( sav.m_count ), m_stride( sav.m_stride ){}
	
	FINLINE const void* data() const { return m_data; }
	FINLINE size_t size() const { return m_count; }
	FINLINE size_t stride() const { return m_stride; }
	FINLINE operator bool() const { return m_data && m_count; }
	
	FINLINE T operator [] ( size_t i ) const
	{
		ASSERT( i < m_count );
		T out;
		memcpy( &out, m_data + m_stride * i, sizeof(T) );
		return out;
	}
	
	FINLINE StridingArrayView part( size_t start, size_t count = NOT_FOUND ) const
	{
		if( start > m_count )
			start = m_count;
		if( count > m_count || start + count > m_count )
			count = m_count - start;
		return StridingArrayView( m_data + start * m_stride, count, m_stride );
	}
	FINLINE bool skip( size_t n )
	{
		if( n > m_count )
		{
			m_data += m_count * m_stride;
			m_count = 0;
			return false;
		}
		m_data += n * m_stride;
		m_count -= n;
		return true;
	}
	FINLINE StridingArrayView take( size_t n )
	{
		StridingArrayView p = part( 0, n );
		skip( n );
		return p;
	}
};

typedef StridingArrayView< Vec3 > Vec3SAV;
typedef StridingArrayView< Quat > QuatSAV;


struct IF_GCC(ENGINE_EXPORT) IProcessor
{
	ENGINE_EXPORT virtual void Process( void* data ) = 0;
};

struct SGRX_RefCounted
{
	SGRX_RefCounted() : m_refcount(0){}
	virtual ~SGRX_RefCounted(){}
	
	SGRX_RefCounted( const SGRX_RefCounted& ) : m_refcount(0){} // DO NOT COPY REFCOUNT
	SGRX_RefCounted& operator = ( const SGRX_RefCounted& ){ return *this; } // DO NOT COPY REFCOUNT
	
	FINLINE void Acquire(){ sgrx_atomic_inc( &m_refcount ); }
	FINLINE void Release(){ if( sgrx_atomic_dec( &m_refcount ) <= 0 ) delete this; }
	FINLINE int32_t GetRefCount() const { return m_refcount; }
	
private:
	volatile int32_t m_refcount;
};

struct SGRX_RCRsrc : SGRX_RefCounted
{
	String m_key; // [storage for] hash table key
};

struct SGRX_RCXFItem : SGRX_RefCounted
{
	virtual void SetTransform( const Mat4& mtx ) = 0;
};

typedef Handle< SGRX_RefCounted > GenericHandle;
typedef Handle< SGRX_RCXFItem > XFItemHandle;


struct RCString_Data : SGRX_RefCounted
{
	RCString_Data( const StringView& sv ) : m_str( NULL )
	{
		m_size = sv.size();
		if( m_size )
		{
			m_str = new char[ m_size + 1 ];
			memcpy( m_str, sv.data(), m_size );
			m_str[ m_size ] = '\0';
		}
	}
	RCString_Data( uint32_t sz ) : m_str( NULL ), m_size( sz )
	{
		m_str = new char[ m_size + 1 ];
		m_str[ m_size ] = '\0';
	}
	~RCString_Data()
	{
		delete [] m_str;
	}
	StringView sv() const { return StringView( m_str, m_size ); }
	char* m_str;
	size_t m_size;
};
typedef Handle< RCString_Data > RCString_Handle;

struct RCString : RCString_Handle
{
	FINLINE RCString(){}
	FINLINE RCString( const RCString& o ) : RCString_Handle( o ){}
	FINLINE RCString( const char* s ) : RCString_Handle( new RCString_Data( s ) ){}
	FINLINE RCString( const StringView& sv ) : RCString_Handle( new RCString_Data( sv ) ){}
	FINLINE RCString( const String& s ) : RCString_Handle( new RCString_Data( s ) ){}
	FINLINE operator StringView () const { return item ? item->sv() : StringView(); }
	FINLINE operator String () const { return item ? String( item->sv() ) : String(); }
	FINLINE const char* c_str() const { return item && item->m_str ? item->m_str : ""; }
	FINLINE const char* data() const { return item ? item->m_str : NULL; }
	FINLINE size_t size() const { return item ? item->m_size : 0; }
	FINLINE StringView view() const { return item ? item->sv() : StringView(); }
	
	FINLINE bool operator == ( const StringView& o ) const { return view() == o; }
	FINLINE bool operator != ( const StringView& o ) const { return view() != o; }
	
	template< class T > void Serialize( T& arch )
	{
		uint32_t sz = size();
		arch << sz;
		if( T::IsReader )
		{
			if( item )
				item->Release();
			if( sz )
			{
				item = new RCString_Data( sz );
				item->Acquire();
			}
			else item = NULL;
		}
		arch.charbuf( item ? item->m_str : NULL, sz );
	}
};


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

ENGINE_EXPORT int util_strtonum( const char** at, const char* end, int64_t* outi, double* outf );
ENGINE_EXPORT int util_strtonum( const char** at, const char* end, int64_t* outi );
ENGINE_EXPORT int util_strtonum( const char** at, const char* end, double* outf );

ENGINE_EXPORT bool String_ParseBool( const StringView& sv );
ENGINE_EXPORT int64_t String_ParseInt( const StringView& sv, bool* success = NULL );
ENGINE_EXPORT double String_ParseFloat( const StringView& sv, bool* success = NULL );
ENGINE_EXPORT Vec2 String_ParseVec2( const StringView& sv, bool* success = NULL, StringView substr = ";" );
ENGINE_EXPORT Vec3 String_ParseVec3( const StringView& sv, bool* success = NULL, StringView substr = ";" );
ENGINE_EXPORT Vec4 String_ParseVec4( const StringView& sv, bool* success = NULL, StringView substr = ";" );


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

struct UTF8Iterator
{
	UTF8Iterator( const StringView& text ) :
		offset(0), codepoint(0), m_nextoff(0), m_text(text){}
	bool Advance();
	void SetOffset( size_t off );
	StringView ReadUntilEndOr( StringView s );
	
	size_t offset;
	uint32_t codepoint;
	
	size_t m_nextoff;
	StringView m_text;
};


//
// HASH TABLE
//

struct NoValue
{
	NoValue(){}
};

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
inline Hash HashVar( float v ){ Hash out; memcpy( &out, &v, sizeof(out) ); return out; }
inline Hash HashVar( double v ){ Hash out; memcpy( &out, &v, sizeof(out) ); return out; }
inline Hash HashVar( void* v ){ return (Hash) v; }
inline Hash HashVar( const String& s ){ return HashFunc( s.m_data, s.m_size ); }
inline Hash HashVar( const StringView& sv ){ return HashFunc( sv.m_str, sv.m_size ); }
inline Hash HashVar( const RCString& rcs ){ return HashVar( rcs.view() ); }

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
	bool isset( const K& key ) const
	{
		return _get_pair_id( key, HashVar( key ) ) >= 0;
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
			
			ASSERT( m_size != osize );
			
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
				ASSERT( i != -1 );
				
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
	
	FINLINE const void* at() const { return arch->at(); }
	
	FINLINE bool hasError() const { return arch->hasError(); }
	FINLINE SerializeVersionHelper& memory( void* ptr, size_t sz ){ arch->memory( ptr, sz ); return *this; }
	FINLINE SerializeVersionHelper& charbuf( char* ptr, size_t sz ){ arch->charbuf( ptr, sz ); return *this; }
	FINLINE SerializeVersionHelper& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE SerializeVersionHelper& marker( const char* ptr, size_t sz ){ arch->marker( ptr, sz ); return *this; }
	FINLINE SerializeVersionHelper& padding( size_t sz ){ arch->padding( sz ); return *this; }
	
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
	FINLINE SerializeVersionHelper& stringView( StringView& sv )
	{
		arch->stringView( sv );
		return *this;
	}
};

struct ByteReader
{
	ByteReader( CByteView bv, size_t p = 0 ) : input_ptr( bv.data() ), input_size( bv.size() ), pos( p ), error( false ){}
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
	FINLINE bool hasError() const { return error; }
	FINLINE ByteReader& memory( void* ptr, size_t sz ){ return _read( ptr, sz ); }
	FINLINE ByteReader& charbuf( char* ptr, size_t sz ){ return _read( ptr, sz ); }
	FINLINE ByteReader& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE ByteReader& marker( const char* ptr, size_t sz )
	{
		if( pos >= input_size || pos + sz > input_size )
			error = true;
		else if( 0 != memcmp( ptr, &input_ptr[ pos ], sz ) )
			error = true;
		if( !error )
			pos += sz;
		return *this;
	}
	FINLINE ByteReader& padding( size_t sz )
	{
		if( pos >= input_size || pos + sz > input_size )
			error = true;
		if( !error )
			pos += sz;
		return *this;
	}
	FINLINE ByteReader& _read( void* ptr, size_t sz )
	{
		if( pos >= input_size || pos + sz > input_size )
			error = true;
		if( !error )
		{
			memcpy( ptr, &input_ptr[ pos ], sz );
			pos += sz;
		}
		return *this;
	}
	FINLINE const void* at() const { return &input_ptr[ pos ]; }
	FINLINE bool atEnd() const { return pos >= input_size; }
	
	CByteView readChunk( const char* mkr )
	{
		marker( mkr );
		uint32_t size = 0;
		*this << size;
		if( pos + size > input_size )
		{
			error = true;
			return CByteView();
		}
		CByteView out( input_ptr + pos, size );
		pos += size;
		return out;
	}
	FINLINE uint32_t beginChunk( const char* mkr )
	{
		marker( mkr );
		padding( 4 ); // uint32_t
		return pos;
	}
	FINLINE void endChunk( uint32_t at )
	{
		if( at < 4 || at > pos )
		{
			error = true;
			return;
		}
		uint32_t size;
		memcpy( &size, &input_ptr[ at - 4 ], 4 );
		if( at + size != pos )
		{
		//	printf( "chunk err at=%d size=%d pos=%d\n", (int)at, (int)size, (int)pos );
			error = true;
		}
	}
	FINLINE ByteReader& smallString( String& out )
	{
		uint8_t size = 0;
		*this << size;
		out.resize( size );
		memory( out.data(), out.size() );
		return *this;
	}
	FINLINE ByteReader& stringView( StringView& sv )
	{
		uint32_t size = 0;
		*this << size;
		if( pos + size > input_size )
			error = true;
		sv.m_str = error ? NULL : (const char*) &input_ptr[ pos ];
		sv.m_size = error ? 0 : size;
		if( !error )
			pos += size;
		return *this;
	}
	
	const uint8_t* input_ptr;
	size_t input_size;
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
	template< class T > FINLINE ByteWriter& write( T v ){ (*this) << v; return *this; }
	FINLINE bool hasError() const { return false; }
	FINLINE ByteWriter& memory( const void* ptr, size_t sz ){ return _write( ptr, sz ); }
	FINLINE ByteWriter& charbuf( const char* ptr, size_t sz ){ return _write( ptr, sz ); }
	FINLINE ByteWriter& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE ByteWriter& marker( const void* ptr, size_t sz ){ output->append( (uint8_t*) ptr, sz ); return *this; }
	FINLINE ByteWriter& padding( size_t sz ){ output->reserve( output->size() + sz ); while( sz --> 0 ) output->push_back( 0 ); return *this; }
	FINLINE ByteWriter& _write( const void* ptr, size_t sz ){ output->append( (uint8_t*) ptr, sz ); return *this; }
	FINLINE void* at() const { return NULL; }
	
	FINLINE uint32_t beginChunk( const char* mkr )
	{
		marker( mkr );
		padding( 4 ); // uint32_t
		return output->size();
	}
	FINLINE void endChunk( uint32_t at )
	{
		ASSERT( at >= 4 && at <= output->size() );
		uint32_t size = output->size() - at;
		memcpy( &(*output)[ at - 4 ], &size, 4 ); // uint32_t
	}
	FINLINE ByteWriter& smallString( StringView str )
	{
		ASSERT( str.size() <= 255 );
		write< uint8_t >( (uint8_t) str.size() );
		memory( str.data(), str.size() );
		return *this;
	}
	FINLINE ByteWriter& stringView( StringView& sv )
	{
		uint32_t size = sv.size();
		*this << size;
		_write( sv.data(), size );
		return *this;
	}
	
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
	FINLINE bool hasError() const { return error; }
	TextReader& memory( void* ptr, size_t sz )
	{
		StringView it = _read();
		if( sz * 2 != it.size() )
			error = true;
		if( !error )
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
		if( !error )
			pos++;
		return *this;
	}
	FINLINE TextReader& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE TextReader& marker( const void* ptr, size_t sz ){ if( StringView( (const char*) ptr, sz ) != _read() ) error = true; return *this; }
	FINLINE TextReader& padding( size_t sz ){ UNUSED( sz ); return *this; }
	FINLINE StringView _read()
	{
		if( pos >= input->size() )
			error = true;
		if( !error )
		{
			StringView sv = StringView( &input->at( pos ), input->size() - pos ).until( "\n" );
			if( sv.size() == 0 )
				error = true;
			else
				pos += sv.size() + 1;
			return sv;
		}
		return StringView();
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
	FINLINE bool hasError() const { return false; }
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

ENGINE_EXPORT bool FSItemExists( const StringView& path );

ENGINE_EXPORT bool CWDGet( String& path );
ENGINE_EXPORT bool CWDSet( const StringView& path );

ENGINE_EXPORT String NormalizePath( const StringView& path );
ENGINE_EXPORT bool IsAbsPath( const StringView& path );
ENGINE_EXPORT bool PathIsUnder( const StringView& path, const StringView& base );
ENGINE_EXPORT String GetRelativePath( const StringView& path, const StringView& base );
ENGINE_EXPORT String AbsPath( const StringView& path );
ENGINE_EXPORT String RealPath( const StringView& path );

ENGINE_EXPORT bool LoadBinaryFile( const StringView& path, ByteArray& out );
ENGINE_EXPORT bool SaveBinaryFile( const StringView& path, const void* data, size_t size );
ENGINE_EXPORT bool LoadTextFile( const StringView& path, String& out );
ENGINE_EXPORT bool SaveTextFile( const StringView& path, const StringView& data );
ENGINE_EXPORT bool FileExists( const StringView& path );
ENGINE_EXPORT uint32_t FileModTime( const StringView& path );

struct IL_Item
{
	String name;
	StringTable params;
};
typedef Array< IL_Item > ItemList;

ENGINE_EXPORT bool LoadItemListFile( const StringView& path, ItemList& out );


//
// GUID
//

struct IF_GCC(ENGINE_EXPORT) SGRX_GUID
{
	union
	{
		uint8_t bytes[16];
		uint32_t u32[4];
		struct
		{
			uint32_t a;
			uint16_t b;
			uint16_t c;
			uint16_t d;
			uint8_t e[6];
		}
		fmt;
		struct
		{
			uint32_t a;
			uint16_t b;
			uint16_t c;
			uint8_t d[8];
		}
		winfmt;
	};
	
	static const SGRX_GUID Null;
	static FINLINE SGRX_GUID FromBytes( const uint8_t bytes[16] )
	{
		SGRX_GUID o;
		memcpy( &o, bytes, sizeof(o) );
		return o;
	}
	ENGINE_EXPORT static SGRX_GUID Generate();
	ENGINE_EXPORT static SGRX_GUID ParseString( StringView str );
	ENGINE_EXPORT void ToCharArray( char* out, bool upper = false, bool nul = true, char sep = '-' ) const;
	ENGINE_EXPORT String ToString( bool upper = false );
	
	void SetNull(){ *this = Null; }
	void SetGenerated(){ *this = Generate(); }
	
	SGRX_GUID(){ memset( u32, 0, sizeof(u32) ); }
	FINLINE bool operator == ( const SGRX_GUID& o ) const
	{
		return u32[0] == o.u32[0]
			&& u32[1] == o.u32[1]
			&& u32[2] == o.u32[2]
			&& u32[3] == o.u32[3];
	}
	FINLINE bool operator != ( const SGRX_GUID& o ) const { return !( *this == o ); }
	FINLINE bool IsNull() const { return *this == Null; }
	FINLINE bool NotNull() const { return *this != Null; }
	FINLINE int Compare( const SGRX_GUID& o ) const
	{
		if( u32[0] != o.u32[0] ) return u32[0] < o.u32[0] ? -1 : 1;
		if( u32[1] != o.u32[1] ) return u32[1] < o.u32[1] ? -1 : 1;
		if( u32[2] != o.u32[2] ) return u32[2] < o.u32[2] ? -1 : 1;
		if( u32[3] != o.u32[3] ) return u32[3] < o.u32[3] ? -1 : 1;
		return 0;
	}
	template< class T > void Serialize( T& arch )
	{
		if( T::IsText )
			arch.marker( "GUID" );
		arch << u32[0] << u32[1] << u32[2] << u32[3];
	}
};

inline Hash HashVar( const SGRX_GUID& v )
{
	return v.u32[0] | v.u32[1] | v.u32[2] | v.u32[3];
}



//
// SPATIAL PARTITIONING
//

struct AABB3
{
	Vec3 bbmin;
	Vec3 bbmax;
	
	FINLINE bool Valid() const { return bbmin.x <= bbmax.x && bbmin.y <= bbmax.y && bbmin.z <= bbmax.z; }
	FINLINE Vec3 Center() const { return ( bbmin + bbmax ) * 0.5f; }
	FINLINE float Volume() const { return ( bbmax.x - bbmin.x ) * ( bbmax.y - bbmin.y ) * ( bbmax.z - bbmin.z ); }
};

struct BaseRayQuery
{
	Vec3 ray_origin;
	float ray_len;
	Vec3 _ray_inv_dir;
	
	void SetRayDir( Vec3 dir )
	{
		dir = dir.Normalized();
		_ray_inv_dir = V3
		(
			safe_fdiv( 1, dir.x ),
			safe_fdiv( 1, dir.y ),
			safe_fdiv( 1, dir.z )
		);
	}
	void SetRay( const Vec3& r0, const Vec3& r1 )
	{
		ray_origin = r0;
		ray_len = ( r1 - r0 ).Length();
		SetRayDir( r1 - r0 );
	}
};

bool RayAABBTest( const Vec3& ro, const Vec3& inv_n, float len, const Vec3& bbmin, const Vec3& bbmax );

struct AABBTree
{
	struct Node // size = 8(3+3+2) * 4(float/int32)
	{
		Vec3 bbmin;
		Vec3 bbmax;
		int32_t ch; // ch0 = node + 1, ch1 = ch
		int32_t ido; // item data offset
	};
	
	// AABBs must be stored manually if necessary
	void SetAABBs( AABB3* aabbs, size_t count );
	
	void _printdepth( int depth )
	{
		for( int i = 0; i < depth; ++i )
			printf( "  " );
	}
	void Dump( int32_t node = 0, int depth = 0 )
	{
		AABBTree::Node& N = m_nodes[ node ];
		_printdepth(depth); printf( "node #%d (%d items, %.2f;%.2f;%.2f -> %.2f;%.2f;%.2f)",
			int(node), N.ido != -1 ? int(m_itemidx[ N.ido ]) : 0,
			N.bbmin.x, N.bbmin.y, N.bbmin.z, N.bbmax.x, N.bbmax.y, N.bbmax.z );
		if( N.ch != -1 )
		{
			printf( " {\n" );
			depth++;
			Dump( node + 1, depth );
			Dump( N.ch, depth );
			depth--;
			_printdepth(depth); printf( "}\n" );
		}
		else printf( "\n" );
	}
	
	template< class T > bool RayQuery( T& rq, int32_t node = 0 )
	{
		AABBTree::Node& N = m_nodes[ node ];
		if( RayAABBTest( rq.ray_origin, rq._ray_inv_dir, rq.ray_len, N.bbmin, N.bbmax ) == false )
			return true;
		
		if( N.ido != -1 )
		{
			if( rq( &m_itemidx[ N.ido + 1 ], m_itemidx[ N.ido ] ) == false )
				return false;
		}
		
		// child nodes
		if( N.ch != -1 )
		{
			if( RayQuery( rq, node + 1 ) == false ) return false;
			if( RayQuery( rq, N.ch ) == false ) return false;
		}
		
		return true;
	}
	
	template< class T > void DynBBQuery( T& bbq, int32_t node = 0 )
	{
		AABBTree::Node& N = m_nodes[ node ];
		if( bbq.bbmin.x > N.bbmax.x || bbq.bbmax.x < N.bbmin.x ||
			bbq.bbmin.y > N.bbmax.y || bbq.bbmax.y < N.bbmin.y ||
			bbq.bbmin.z > N.bbmax.z || bbq.bbmax.z < N.bbmin.z )
			return;
		
		// items
		if( N.ido != -1 )
		{
			bbq( &m_itemidx[ N.ido + 1 ], m_itemidx[ N.ido ] );
		}
		
		// child nodes
		if( N.ch != -1 )
		{
			DynBBQuery( bbq, node + 1 );
			DynBBQuery( bbq, N.ch );
		}
	}
	
	template< class T > void Query( const Vec3& qmin, const Vec3& qmax, T& out, int32_t node = 0 )
	{
		AABBTree::Node& N = m_nodes[ node ];
		if( qmin.x > N.bbmax.x || qmax.x < N.bbmin.x ||
			qmin.y > N.bbmax.y || qmax.y < N.bbmin.y ||
			qmin.z > N.bbmax.z || qmax.z < N.bbmin.z )
			return;
		
		// items
		if( N.ido != -1 )
		{
			out( &m_itemidx[ N.ido + 1 ], m_itemidx[ N.ido ] );
		}
		
		// child nodes
		if( N.ch != -1 )
		{
			Query( qmin, qmax, out, node + 1 );
			Query( qmin, qmax, out, N.ch );
		}
	}
	
	template< class T > void GetAll( T& out )
	{
		for( size_t i = 0; i < m_itemidx.size(); i += 1 + m_itemidx[ i ] )
		{
			out( &m_itemidx[ i + 1 ], m_itemidx[ i ] );
		}
	}
	
	void _MakeNode( int32_t node, AABB3* aabbs, int32_t* sampidx_data, size_t sampidx_count, int depth );
	
	// BVH
	Array< Node > m_nodes;
	Array< int32_t > m_itemidx; // format: <count> [ <item> x count ], ...
};

struct Triangle
{
	Vec3 P1, P2, P3;
	
	bool CheckIsUseful() const
	{
		Vec3 e1 = P2 - P1, e2 = P3 - P1;
		return !Vec3Cross( e1, e2 ).NearZero();
	}
	void GetAABB( AABB3& out ) const
	{
		out.bbmin = V3( TMIN( P1.x, TMIN( P2.x, P3.x ) ), TMIN( P1.y, TMIN( P2.y, P3.y ) ), TMIN( P1.z, TMIN( P2.z, P3.z ) ) );
		out.bbmax = V3( TMAX( P1.x, TMAX( P2.x, P3.x ) ), TMAX( P1.y, TMAX( P2.y, P3.y ) ), TMAX( P1.z, TMAX( P2.z, P3.z ) ) );
	}
	Vec3 GetNormal() const
	{
		return Vec3Cross( P3 - P1, P2 - P1 ).Normalized();
	}
};

float IntersectLineSegmentTriangle( const Vec3& L1, const Vec3& L2, const Vec3& P1, const Vec3& P2, const Vec3& P3 );

struct TriTree
{
	ENGINE_EXPORT void SetTris( Triangle* tris, size_t count );
	ENGINE_EXPORT bool IntersectRay( const Vec3& from, const Vec3& to );
	ENGINE_EXPORT float IntersectRayDist( const Vec3& from, const Vec3& to, int32_t* outtid );
	ENGINE_EXPORT float GetDistance( const Vec3& p, float dist );
	
	AABBTree m_bbTree;
	Array< Triangle > m_tris;
};



//
// LOGGING
//

// #define GATHER_STATS

struct IF_GCC(ENGINE_EXPORT) SGRX_LogOutput
{
	SGRX_LogOutput() : next(NULL){}
	virtual ~SGRX_LogOutput(){ Unregister(); }
	virtual void Write( const char* str, size_t sz ) = 0;
	ENGINE_EXPORT void Register();
	ENGINE_EXPORT void Unregister();
	SGRX_LogOutput* next;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_LogOutputStdout : SGRX_LogOutput
{
	ENGINE_EXPORT virtual void Write( const char* str, size_t sz );
};

struct IF_GCC(ENGINE_EXPORT) SGRX_Log
{
	template< class T > struct Loggable
	{
		void _LogMe( SGRX_Log& to ) const { ((const T*)this)->Log( to ); }
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
	struct IF_GCC(ENGINE_EXPORT) RegFunc
	{
#ifdef GATHER_STATS
		struct IF_GCC(ENGINE_EXPORT) STATS
		{
			FILE* f;
			ENGINE_EXPORT STATS();
			ENGINE_EXPORT ~STATS();
		};
		static STATS stats;
		double time;
		ENGINE_EXPORT void WriteStat( double dt );
#endif
		ENGINE_EXPORT RegFunc( const char* func, const char* file, int ln, StringView a = StringView() );
		ENGINE_EXPORT ~RegFunc();
		const char* funcname;
		const char* filename;
		int linenum;
		StringView arg;
		RegFunc* prev;
	};
	
	bool end_newline;
	bool need_sep;
	const char* sep;
	
	static THREAD_LOCAL RegFunc* lastfunc;
	static SGRX_LogOutput* out;
	
	ENGINE_EXPORT static void RegisterLogOutput( SGRX_LogOutput* o );
	ENGINE_EXPORT static void UnregisterLogOutput( SGRX_LogOutput* o );
	
	ENGINE_EXPORT SGRX_Log();
	ENGINE_EXPORT ~SGRX_Log();
	ENGINE_EXPORT void write( const char* str, size_t size );
	ENGINE_EXPORT void write( const char* str );
	ENGINE_EXPORT void writef( const char* str, ... );
	ENGINE_EXPORT void prelog();
	
	ENGINE_EXPORT SGRX_Log& operator << ( const Separator& );
	ENGINE_EXPORT SGRX_Log& operator << ( EMod_Partial );
	ENGINE_EXPORT SGRX_Log& operator << ( ESpec_Date );
	ENGINE_EXPORT SGRX_Log& operator << ( ESpec_CallStack );
	ENGINE_EXPORT SGRX_Log& operator << ( bool );
	ENGINE_EXPORT SGRX_Log& operator << ( int8_t );
	ENGINE_EXPORT SGRX_Log& operator << ( uint8_t );
	ENGINE_EXPORT SGRX_Log& operator << ( int16_t );
	ENGINE_EXPORT SGRX_Log& operator << ( uint16_t );
	ENGINE_EXPORT SGRX_Log& operator << ( int32_t );
	ENGINE_EXPORT SGRX_Log& operator << ( uint32_t );
	ENGINE_EXPORT SGRX_Log& operator << ( int64_t );
	ENGINE_EXPORT SGRX_Log& operator << ( uint64_t );
	ENGINE_EXPORT SGRX_Log& operator << ( float );
	ENGINE_EXPORT SGRX_Log& operator << ( double );
	ENGINE_EXPORT SGRX_Log& operator << ( const void* );
	ENGINE_EXPORT SGRX_Log& operator << ( const char* );
	ENGINE_EXPORT SGRX_Log& operator << ( const StringView& );
	ENGINE_EXPORT SGRX_Log& operator << ( const String& );
	ENGINE_EXPORT SGRX_Log& operator << ( const Vec2& );
	ENGINE_EXPORT SGRX_Log& operator << ( const Vec3& );
	ENGINE_EXPORT SGRX_Log& operator << ( const Vec4& );
	ENGINE_EXPORT SGRX_Log& operator << ( const Quat& );
	ENGINE_EXPORT SGRX_Log& operator << ( const Mat4& );
	ENGINE_EXPORT SGRX_Log& operator << ( const SGRX_GUID& );
	template< class T > SGRX_Log& operator << ( const Loggable<T>& val ){ val._LogMe( *this ); return *this; }
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
#define LOG_FUNCTION_ARG( x ) SGRX_Log::RegFunc __regfn( __FUNCTION__, __FILE__, __LINE__, x )
#define LOG_TIME( x ) { double _t0 = sgrx_hqtime(); { x; } double _t1 = sgrx_hqtime(); \
	LOG << "TIME[" #x "]: " << ( _t1 - _t0 ); }



//
// TESTS
//

int TestSystems();

