

#pragma once
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <new>

#define ASSERT assert
#define EXPORT __declspec(dllexport)
#ifdef _MSC_VER
#define FINLINE __forceinline
#else
#define FINLINE inline __attribute__((__always_inline__))
#endif

#define SGRX_CAST( t, to, from ) t to = (t) from
#define SGRX_ARRAY_SIZE( arr ) (sizeof(arr)/sizeof((arr)[0]))
#define UNUSED( x ) (void) x
#define STRLIT_LEN( x ) (sizeof(x)-1)
#define STRLIT_BUF( x ) x, STRLIT_LEN( x )


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

#define IIF( cond, a, b ) ((cond)?(a):(b))
inline float clamp( float v, float vmin, float vmax ){ return IIF( v < vmin, vmin, IIF( v > vmax, vmax, v ) ); }
inline float lerp( float a, float b, float t ){ return a * (1.0f-t) + b * t; }
inline float sign( float x ){ return IIF( x == 0, 0, IIF( x < 0, -1, 1 ) ); }
inline float normalize_angle( float x ){ x = fmodf( x, M_PI * 2 ); return IIF( x < 0, x + M_PI*2, x ); }
inline float saturate( float x ){ return IIF( x < 0, 0, IIF( x > 1, 1, x ) ); }
inline float smoothstep( float x ){ return x * x * ( 3 - 2 * x ); }
inline float smoothlerp_oneway( float t, float a, float b ){ if( b == a ) return 1.0f; return smoothstep( saturate( ( t - a ) / ( b - a ) ) ); }
inline float smoothlerp_range( float t, float a, float b, float c, float d ){ return smoothlerp_oneway( t, a, b ) * smoothlerp_oneway( t, d, c ); }
inline float randf(){ return (float) rand() / (float) RAND_MAX; }
inline float randf11(){ return randf() * 2 - 1; }


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


//
// VEC2
//

struct EXPORT Vec2
{
	float x, y;
	
#ifdef USE_VEC2
	static FINLINE Vec2 Create( float x ){ Vec2 v = { x, x }; return v; }
	static FINLINE Vec2 Create( float x, float y ){ Vec2 v = { x, y }; return v; }
	static FINLINE Vec2 CreateFromAngle( float a, float d = 1.0f ){ Vec2 v = { cos( a ) * d, sin( a ) * d }; return v; }
	static FINLINE Vec2 CreateFromPtr( const float* x ){ Vec2 v = { x[0], x[1] }; return v; }
	
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
	FINLINE void Normalize(){ *this = Normalized(); }
	FINLINE Vec2 Rotate( float angle ) const { return Rotate( Vec2::Create( cos( angle ), sin( angle ) ) ); }
	FINLINE Vec2 Rotate( const Vec2& dir ) const { return Vec2::Create( x * dir.x - y * dir.y, x * dir.y + y * dir.x ); }
	template< class T > void Serialize( T& arch )
	{
		if( T::IsText )
			arch.marker( "Vec2" );
		arch << x << y;
	}
#endif
};

#ifdef USE_VEC2
FINLINE Vec2 operator + ( float f, const Vec2& v ){ Vec2 out = { f + v.x, f + v.y }; return out; }
FINLINE Vec2 operator - ( float f, const Vec2& v ){ Vec2 out = { f - v.x, f - v.y }; return out; }
FINLINE Vec2 operator * ( float f, const Vec2& v ){ Vec2 out = { f * v.x, f * v.y }; return out; }
FINLINE Vec2 operator / ( float f, const Vec2& v ){ Vec2 out = { f / v.x, f / v.y }; return out; }

FINLINE float Vec2Dot( const Vec2& v1, const Vec2& v2 ){ return v1.x * v2.x + v1.y * v2.y; }
#endif


//
// VEC3
//

#ifdef USE_MAT4
#define USE_VEC3
#endif

struct EXPORT Vec3
{
	float x, y, z;
	
#ifdef USE_VEC3
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
	FINLINE Vec3 Pow( float f ){ Vec3 v = { pow( x, f ), pow( y, f ), pow( z, f ) }; return v; }
	FINLINE void Set( float _x, float _y, float _z ){ x = _x; y = _y; z = _z; }
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
// VEC4
//

struct EXPORT Vec4
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
	FINLINE Vec3 ToVec3(){ Vec3 v = { x, y, z }; return v; }
#endif
};

#ifdef USE_VEC4
FINLINE Vec4 operator + ( float f, const Vec4& v ){ Vec4 out = { f + v.x, f + v.y, f + v.z, f + v.w }; return out; }
FINLINE Vec4 operator - ( float f, const Vec4& v ){ Vec4 out = { f - v.x, f - v.y, f - v.z, f - v.w }; return out; }
FINLINE Vec4 operator * ( float f, const Vec4& v ){ Vec4 out = { f * v.x, f * v.y, f * v.z, f * v.w }; return out; }
FINLINE Vec4 operator / ( float f, const Vec4& v ){ Vec4 out = { f / v.x, f / v.y, f / v.z, f / v.w }; return out; }

FINLINE float Vec4Dot( const Vec4& v1, const Vec4& v2 ){ return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
#endif


//
// MAT4
//

struct EXPORT Mat4
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
			-x, y, 0.5f * size, 1
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
	
	static FINLINE Mat4 CreateTranslation( float x, float y, float z )
	{
		Mat4 out;
		out.SetIdentity();
		out.m[3][0] = x;
		out.m[3][1] = y;
		out.m[3][2] = z;
		return out;
	}
	static FINLINE Mat4 CreateTranslation( Vec3 v ){ return CreateTranslation( v.x, v.y, v.z ); }
	
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
	
	template< class T > void Serialize( T& arch )
	{
		if( T::IsText )
			arch.marker( "Mat4" );
		for( int i = 0; i < 16; ++i )
			arch << a[ i ];
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
	
	FINLINE size_t size() const { return m_size; }
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
	
	FINLINE void push_front( const T& v ){ insert( 0, v ); }
	FINLINE void push_back( const T& v ){ insert( size(), v ); }
	FINLINE void append( const T* v, size_t sz ){ insert( size(), v, sz ); }
	FINLINE void assign( const T* v, size_t sz ){ clear(); insert( size(), v, sz ); }
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

FINLINE size_t StringLength( const char* str ){ const char* o = str; while( *str ) str++; return str - o; }

struct String : Array< char >
{
#ifdef USE_ARRAY
	String() : Array(){}
	String( const char* str ) : Array( str, StringLength( str ) ){}
	String( const char* str, size_t size ) : Array( str, size ){}
	String( const String& s ) : Array( s ){}
	
	FINLINE void append( const char* str, size_t sz ){ Array::append( str, sz ); }
	FINLINE void append( const char* str ){ append( str, StringLength( str ) ); }
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
		for( size_t i = from; i < m_size - substr.m_size; ++i )
			if( !memcmp( m_str + i, substr.m_str, substr.m_size ) )
				return i;
		return defval;
	}
	FINLINE bool is_any( char c ) const
	{
		for( size_t i = 0; i < m_size; ++i )
			if( m_str[i] == c )
				return true;
		return false;
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
typedef StackString< ENGINE_MAX_PATH > StackPath;


EXPORT String String_Concat( const StringView& a, const StringView& b );
EXPORT String String_Replace( const StringView& base, const StringView& sub, const StringView& rep );

//
// PARSING
//

EXPORT int64_t String_ParseInt( const StringView& sv, bool* success = NULL );
EXPORT double String_ParseFloat( const StringView& sv, bool* success = NULL );
EXPORT Vec2 String_ParseVec2( const StringView& sv, bool* success = NULL );
EXPORT Vec3 String_ParseVec3( const StringView& sv, bool* success = NULL );


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
EXPORT Hash HashFunc( const char* str, size_t size );

inline Hash HashVar( int8_t v ){ return v; }
inline Hash HashVar( uint8_t v ){ return v; }
inline Hash HashVar( int16_t v ){ return v; }
inline Hash HashVar( uint16_t v ){ return v; }
inline Hash HashVar( int32_t v ){ return v; }
inline Hash HashVar( uint32_t v ){ return v; }
inline Hash HashVar( int64_t v ){ return v; }
inline Hash HashVar( uint64_t v ){ return v; }
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
	FINLINE const Var& item( size_t i ) const { ASSERT( i < m_size ); return m_vars[ i ]; }
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

#ifdef USE_SERIALIZATION
struct ByteReader
{
	ByteReader( ByteArray* ba, size_t p = 0 ) : input( ba ), pos( p ), error( false ){}
	enum { IsWriter = 0, IsReader = 1, IsText = 0, IsBinary = 1 };
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
	
	ByteArray* input;
	size_t pos;
	bool error;
};

struct ByteWriter
{
	ByteWriter( ByteArray* str ) : output( str ){}
	enum { IsWriter = 1, IsReader = 0, IsText = 0, IsBinary = 1 };
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
	FINLINE TextReader& operator << ( int8_t& v ){ v = String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( uint8_t& v ){ v = String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( int16_t& v ){ v = String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( uint16_t& v ){ v = String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( int32_t& v ){ v = String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( uint32_t& v ){ v = String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( int64_t& v ){ v = String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( uint64_t& v ){ v = String_ParseInt( _read() ); return *this; }
	FINLINE TextReader& operator << ( float& v ){ v = String_ParseFloat( _read() ); return *this; }
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
	FINLINE TextWriter& operator << ( int8_t& v ){ char bfr[ 32 ]; sprintf( bfr, "%d\n", (int)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( uint8_t& v ){ char bfr[ 32 ]; sprintf( bfr, "%u\n", (unsigned)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( int16_t& v ){ char bfr[ 32 ]; sprintf( bfr, "%d\n", (int)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( uint16_t& v ){ char bfr[ 32 ]; sprintf( bfr, "%u\n", (unsigned)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( int32_t& v ){ char bfr[ 32 ]; sprintf( bfr, "%d\n", (int)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( uint32_t& v ){ char bfr[ 32 ]; sprintf( bfr, "%u\n", (unsigned)v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( int64_t& v ){ char bfr[ 32 ]; sprintf( bfr, "%" PRId64 "\n", v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( uint64_t& v ){ char bfr[ 32 ]; sprintf( bfr, "%" PRIu64 "\n", v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( float& v ){ char bfr[ 32 ]; sprintf( bfr, "%f\n", v ); _write( bfr ); return *this; }
	FINLINE TextWriter& operator << ( double& v ){ char bfr[ 32 ]; sprintf( bfr, "%f\n", v ); _write( bfr ); return *this; }
	template< class T > TextWriter& operator << ( T& v ){ v.Serialize( *this ); return *this; }
	TextWriter& memory( void* ptr, size_t sz )
	{
		char bfr[ 32 ];
		uint8_t* p = (uint8_t*) ptr;
		for( size_t i = 0; i < sz; ++i )
		{
			sprintf( bfr, "%02X", (int) p[ i ] );
			_write( bfr );
		}
		return *this;
	}
	FINLINE TextWriter& marker( const char* str ){ return marker( str, StringLength( str ) ); }
	FINLINE TextWriter& marker( const void* ptr, size_t sz ){ output->append( (char*) ptr, sz ); output->push_back( '\n' ); return *this; }
	FINLINE TextWriter& padding( size_t sz ){ UNUSED( sz ); return *this; }
	FINLINE TextWriter& _write( const void* ptr ){ output->append( (char*) ptr, StringLength( (char*) ptr ) ); return *this; }
	
	String* output;
};
#endif


//
// FILES
//

EXPORT bool LoadBinaryFile( const StringView& path, ByteArray& out );
EXPORT bool LoadTextFile( const StringView& path, String& out );

struct IL_Item
{
	String name;
	StringTable params;
};
typedef Array< IL_Item > ItemList;

EXPORT bool LoadItemListFile( const StringView& path, ItemList& out );


//
// TESTS
//

int TestSystems();

