

#include <utility>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#define USE_SERIALIZATION

#include "utils.hpp"


const Quat Quat::Identity = { 0, 0, 0, 1 };


bool Mat4::InvertTo( Mat4& out )
{
	float inv[16], det;
	int i;
	
	inv[0] = a[5]  * a[10] * a[15] -
			 a[5]  * a[11] * a[14] -
			 a[9]  * a[6]  * a[15] +
			 a[9]  * a[7]  * a[14] +
			 a[13] * a[6]  * a[11] -
			 a[13] * a[7]  * a[10];
	
	inv[4] = -a[4]  * a[10] * a[15] +
			  a[4]  * a[11] * a[14] +
			  a[8]  * a[6]  * a[15] -
			  a[8]  * a[7]  * a[14] -
			  a[12] * a[6]  * a[11] +
			  a[12] * a[7]  * a[10];
	
	inv[8] = a[4]  * a[9] * a[15] -
			 a[4]  * a[11] * a[13] -
			 a[8]  * a[5] * a[15] +
			 a[8]  * a[7] * a[13] +
			 a[12] * a[5] * a[11] -
			 a[12] * a[7] * a[9];
	
	inv[12] = -a[4]  * a[9] * a[14] +
			   a[4]  * a[10] * a[13] +
			   a[8]  * a[5] * a[14] -
			   a[8]  * a[6] * a[13] -
			   a[12] * a[5] * a[10] +
			   a[12] * a[6] * a[9];
	
	inv[1] = -a[1]  * a[10] * a[15] +
			  a[1]  * a[11] * a[14] +
			  a[9]  * a[2] * a[15] -
			  a[9]  * a[3] * a[14] -
			  a[13] * a[2] * a[11] +
			  a[13] * a[3] * a[10];
	
	inv[5] = a[0]  * a[10] * a[15] -
			 a[0]  * a[11] * a[14] -
			 a[8]  * a[2] * a[15] +
			 a[8]  * a[3] * a[14] +
			 a[12] * a[2] * a[11] -
			 a[12] * a[3] * a[10];
	
	inv[9] = -a[0]  * a[9] * a[15] +
			  a[0]  * a[11] * a[13] +
			  a[8]  * a[1] * a[15] -
			  a[8]  * a[3] * a[13] -
			  a[12] * a[1] * a[11] +
			  a[12] * a[3] * a[9];
	
	inv[13] = a[0]  * a[9] * a[14] -
			  a[0]  * a[10] * a[13] -
			  a[8]  * a[1] * a[14] +
			  a[8]  * a[2] * a[13] +
			  a[12] * a[1] * a[10] -
			  a[12] * a[2] * a[9];
	
	inv[2] = a[1]  * a[6] * a[15] -
			 a[1]  * a[7] * a[14] -
			 a[5]  * a[2] * a[15] +
			 a[5]  * a[3] * a[14] +
			 a[13] * a[2] * a[7] -
			 a[13] * a[3] * a[6];
	
	inv[6] = -a[0]  * a[6] * a[15] +
			  a[0]  * a[7] * a[14] +
			  a[4]  * a[2] * a[15] -
			  a[4]  * a[3] * a[14] -
			  a[12] * a[2] * a[7] +
			  a[12] * a[3] * a[6];
	
	inv[10] = a[0]  * a[5] * a[15] -
			  a[0]  * a[7] * a[13] -
			  a[4]  * a[1] * a[15] +
			  a[4]  * a[3] * a[13] +
			  a[12] * a[1] * a[7] -
			  a[12] * a[3] * a[5];
	
	inv[14] = -a[0]  * a[5] * a[14] +
			   a[0]  * a[6] * a[13] +
			   a[4]  * a[1] * a[14] -
			   a[4]  * a[2] * a[13] -
			   a[12] * a[1] * a[6] +
			   a[12] * a[2] * a[5];
	
	inv[3] = -a[1] * a[6] * a[11] +
			  a[1] * a[7] * a[10] +
			  a[5] * a[2] * a[11] -
			  a[5] * a[3] * a[10] -
			  a[9] * a[2] * a[7] +
			  a[9] * a[3] * a[6];
	
	inv[7] = a[0] * a[6] * a[11] -
			 a[0] * a[7] * a[10] -
			 a[4] * a[2] * a[11] +
			 a[4] * a[3] * a[10] +
			 a[8] * a[2] * a[7] -
			 a[8] * a[3] * a[6];
	
	inv[11] = -a[0] * a[5] * a[11] +
			   a[0] * a[7] * a[9] +
			   a[4] * a[1] * a[11] -
			   a[4] * a[3] * a[9] -
			   a[8] * a[1] * a[7] +
			   a[8] * a[3] * a[5];
	
	inv[15] = a[0] * a[5] * a[10] -
			  a[0] * a[6] * a[9] -
			  a[4] * a[1] * a[10] +
			  a[4] * a[2] * a[9] +
			  a[8] * a[1] * a[6] -
			  a[8] * a[2] * a[5];
	
	det = a[0] * inv[0] + a[1] * inv[4] + a[2] * inv[8] + a[3] * inv[12];
	
	if( det == 0 )
		return false;
	
	det = 1.0f / det;
	
	for( i = 0; i < 16; ++i )
		out.a[ i ] = inv[ i ] * det;
	
	return true;
}

const Mat4 Mat4::Identity =
{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1,
};



float PointTriangleDistance( const Vec3& pt, const Vec3& t0, const Vec3& t1, const Vec3& t2 )
{
	Vec3 nrm = Vec3Cross( t1 - t0, t2 - t0 ).Normalized();
	Vec3 en0 = Vec3Cross( t1 - t0, nrm ).Normalized();
	Vec3 en1 = Vec3Cross( t2 - t1, nrm ).Normalized();
	Vec3 en2 = Vec3Cross( t0 - t2, nrm ).Normalized();
	float pd = fabsf( Vec3Dot( nrm, pt ) - Vec3Dot( nrm, t0 ) );
	float ptd0 = Vec3Dot( en0, pt ) - Vec3Dot( en0, t0 );
	float ptd1 = Vec3Dot( en1, pt ) - Vec3Dot( en1, t1 );
	float ptd2 = Vec3Dot( en2, pt ) - Vec3Dot( en2, t2 );
	if( ptd0 > 0 && ptd1 > 0 ) return ( pt - t1 ).Length();
	if( ptd1 > 0 && ptd2 > 0 ) return ( pt - t2 ).Length();
	if( ptd2 > 0 && ptd0 > 0 ) return ( pt - t0 ).Length();
	if( ptd0 > 0 ) return V2( pd, ptd0 ).Length();
	if( ptd1 > 0 ) return V2( pd, ptd1 ).Length();
	if( ptd2 > 0 ) return V2( pd, ptd2 ).Length();
	return pd;
}

bool TriangleIntersect( const Vec3& ta0, const Vec3& ta1, const Vec3& ta2, const Vec3& tb0, const Vec3& tb1, const Vec3& tb2 )
{
	Vec3 nrma = Vec3Cross( ta1 - ta0, ta2 - ta0 ).Normalized();
	Vec3 nrmb = Vec3Cross( tb1 - tb0, tb2 - tb0 ).Normalized();
	Vec3 axes[] =
	{
		nrma, Vec3Cross( ta1 - ta0, nrma ).Normalized(), Vec3Cross( ta2 - ta1, nrma ).Normalized(), Vec3Cross( ta0 - ta2, nrma ).Normalized(),
		nrmb, Vec3Cross( tb1 - tb0, nrmb ).Normalized(), Vec3Cross( tb2 - tb1, nrmb ).Normalized(), Vec3Cross( tb0 - tb2, nrmb ).Normalized(),
	};
	// increase plane overlap chance, decrease edge overlap chance
	static const float margins[] =
	{
		SMALL_FLOAT, -SMALL_FLOAT, -SMALL_FLOAT, -SMALL_FLOAT,
		SMALL_FLOAT, -SMALL_FLOAT, -SMALL_FLOAT, -SMALL_FLOAT,
	};
//	float minmargin = 1e8f;
	for( int i = 0; i < sizeof(axes)/sizeof(axes[0]); ++i )
	{
		Vec3 axis = axes[ i ];
		float margin = margins[ i ];
		
		float qa0 = Vec3Dot( axis, ta0 );
		float qa1 = Vec3Dot( axis, ta1 );
		float qa2 = Vec3Dot( axis, ta2 );
		float qb0 = Vec3Dot( axis, tb0 );
		float qb1 = Vec3Dot( axis, tb1 );
		float qb2 = Vec3Dot( axis, tb2 );
		
		float mina = TMIN( TMIN( qa0, qa1 ), qa2 ), maxa = TMAX( TMAX( qa0, qa1 ), qa2 );
		float minb = TMIN( TMIN( qb0, qb1 ), qb2 ), maxb = TMAX( TMAX( qb0, qb1 ), qb2 );
		if( mina > maxb + margin || minb > maxa + margin )
			return false;
//		if( i % 4 != 0 )
//			minmargin = TMIN( minmargin, TMIN( maxa - minb, maxb - mina ) );
	}
//	printf("ISECT minmargin=%f\n",minmargin);
//	printf("A p0 %g %g %g\nA p1 %g %g %g\nA p2 %g %g %g\n", ta0.x,ta0.y,ta0.z, ta1.x,ta1.y,ta1.z, ta2.x,ta2.y,ta2.z );
//	printf("B p0 %g %g %g\nB p1 %g %g %g\nB p2 %g %g %g\n", tb0.x,tb0.y,tb0.z, tb1.x,tb1.y,tb1.z, tb2.x,tb2.y,tb2.z );
	return true;
}

float PolyArea( const Vec2* points, int pointcount )
{
	float area = 0;
	if( pointcount < 3 )
		return area;
	
	for( int i = 0; i < pointcount; ++i )
	{
		int i1 = ( i + 1 ) % pointcount;
		area += points[i].x * points[i1].y - points[i1].x * points[i].y;
	}
	
	return area * 0.5f;
}

bool RayPlaneIntersect( const Vec3& pos, const Vec3& dir, const Vec4& plane, float dsts[2] )
{
	/* returns <distance to intersection, signed origin distance from plane>
			\ <false> on (near-)parallel */
	float sigdst = Vec3Dot( pos, plane.ToVec3() ) - plane.w;
	float dirdot = Vec3Dot( dir, plane.ToVec3() );
	
	if( fabs( dirdot ) < SMALL_FLOAT )
	{
		return false;
	}
	else
	{
		dsts[0] = -sigdst / dirdot;
		dsts[1] = sigdst;
		return true;
	}
}

bool PolyGetPlane( const Vec3* points, int pointcount, Vec4& plane )
{
	Vec3 dir = {0,0,0};
	for( int i = 2; i < pointcount; ++i )
	{
		Vec3 nrm = Vec3Cross( points[i-1] - points[0], points[i] - points[0] ).Normalized();
		dir += nrm;
	}
	float lendiff = dir.Length() - ( pointcount - 2 );
	if( fabs( lendiff ) > SMALL_FLOAT )
		return false;
	dir = dir.Normalized();
	plane.x = dir.x;
	plane.y = dir.y;
	plane.z = dir.z;
	plane.w = Vec3Dot( dir, points[0] );
	return true;
}

bool RayPolyIntersect( const Vec3& pos, const Vec3& dir, const Vec3* points, int pointcount, float dst[1] )
{
	float dsts[2];
	Vec4 plane;
	if( !PolyGetPlane( points, pointcount, plane ) )
		return false;
	if( !RayPlaneIntersect( pos, dir, plane, dsts ) || dsts[0] < 0 || dsts[1] < 0 )
		return false;
	Vec3 isp = pos + dir * dsts[0];
	Vec3 normal = plane.ToVec3();
	for( int i = 0; i < pointcount; ++i )
	{
		int i1 = ( i + 1 ) % pointcount;
		Vec3 edir = points[ i1 ] - points[ i ];
		Vec3 eout = Vec3Cross( edir, normal ).Normalized();
		if( Vec3Dot( eout, isp ) - Vec3Dot( eout, points[ i ] ) > SMALL_FLOAT )
			return false;
	}
	dst[0] = dsts[0];
	return true;
}

bool RaySphereIntersect( const Vec3& pos, const Vec3& dir, const Vec3& spherePos, float sphereRadius, float dst[1] )
{
	/* vec3 ray_pos, vec3 ray_dir, vec3 sphere_pos, float radius;
		returns <distance to intersection> \ false on no intersection */
	Vec3 r2s = spherePos - pos;
	float a = Vec3Dot( dir, r2s );
	if( a < 0 )
	{
		return false;
	}
	float b = Vec3Dot( r2s, r2s ) - a * a;
	if( b > sphereRadius * sphereRadius )
	{
		return false;
	}
	dst[0] = a - sqrtf( sphereRadius * sphereRadius - b );
	return true;
}



/* string -> number conversion */

typedef const char CCH;

static int strtonum_hex( CCH** at, CCH* end, int64_t* outi )
{
	int64_t val = 0;
	CCH* str = *at + 2;
	while( str < end && hexchar( *str ) )
	{
		val *= 16;
		val += gethex( *str );
		str++;
	}
	*at = str;
	*outi = val;
	return 1;
}

static int strtonum_oct( CCH** at, CCH* end, int64_t* outi )
{
	int64_t val = 0;
	CCH* str = *at + 2;
	while( str < end && octchar( *str ) )
	{
		val *= 8;
		val += getoct( *str );
		str++;
	}
	*at = str;
	*outi = val;
	return 1;
}

static int strtonum_bin( CCH** at, CCH* end, int64_t* outi )
{
	int64_t val = 0;
	CCH* str = *at + 2;
	while( str < end && binchar( *str ) )
	{
		val *= 2;
		val += getbin( *str );
		str++;
	}
	*at = str;
	*outi = val;
	return 1;
}

static int strtonum_real( CCH** at, CCH* end, double* outf )
{
	double val = 0;
	double vsign = 1;
	CCH* str = *at, *teststr;
	if( str == end )
		return 0;
	
	if( *str == '+' ) str++;
	else if( *str == '-' ){ vsign = -1; str++; }
	
	teststr = str;
	while( str < end && decchar( *str ) )
	{
		val *= 10;
		val += getdec( *str );
		str++;
	}
	if( str == teststr )
		return 0;
	if( str >= end )
		goto done;
	if( *str == '.' )
	{
		double mult = 1.0;
		str++;
		while( str < end && decchar( *str ) )
		{
			mult /= 10;
			val += getdec( *str ) * mult;
			str++;
		}
	}
	if( str < end && ( *str == 'e' || *str == 'E' ) )
	{
		double sign, e = 0;
		str++;
		if( str >= end || ( *str != '+' && *str != '-' ) )
			goto done;
		sign = *str++ == '-' ? -1 : 1;
		while( str < end && decchar( *str ) )
		{
			e *= 10;
			e += getdec( *str );
			str++;
		}
		val *= pow( 10, e * sign );
	}
	
done:
	*outf = val * vsign;
	*at = str;
	return 2;
}

static int strtonum_dec( CCH** at, CCH* end, int64_t* outi, double* outf )
{
	CCH* str = *at, *teststr;
	if( *str == '+' || *str == '-' ) str++;
	teststr = str;
	while( str < end && decchar( *str ) )
		str++;
	if( str == teststr )
		return 0;
	if( str < end && ( *str == '.' || *str == 'E' || *str == 'e' ) )
		return strtonum_real( at, end, outf );
	else
	{
		int64_t val = 0;
		int invsign = 0;
		
		str = *at;
		if( *str == '+' ) str++;
		else if( *str == '-' ){ invsign = 1; str++; }
		
		while( str < end && decchar( *str ) )
		{
			val *= 10;
			val += getdec( *str );
			str++;
		}
		if( invsign ) val = -val;
		*outi = val;
		*at = str;
		return 1;
	}
}

int util_strtonum( CCH** at, CCH* end, int64_t* outi, double* outf )
{
	CCH* str = *at;
	if( str >= end )
		return 0;
	if( end - str >= 3 && *str == '0' )
	{
		if( str[1] == 'x' ) return strtonum_hex( at, end, outi );
		else if( str[1] == 'o' ) return strtonum_oct( at, end, outi );
		else if( str[1] == 'b' ) return strtonum_bin( at, end, outi );
	}
	return strtonum_dec( at, end, outi, outf );
}


String String_Concat( const StringView& a, const StringView& b )
{
	String out;
	out.resize( a.size() + b.size() );
	memcpy( out.data(), a.data(), a.size() );
	memcpy( out.data() + a.size(), b.data(), b.size() );
	return out;
}

String String_Replace( const StringView& base, const StringView& sub, const StringView& rep )
{
	String out;
	size_t at, cur = 0;
	while( ( at = base.find_first_at( sub, cur ) ) != NOT_FOUND )
	{
		out.append( &base[ cur ], at - cur );
		out.append( rep.data(), rep.size() );
		cur = at + sub.size();
	}
	if( cur < base.size() )
		out.append( &base[ cur ], base.size() - cur );
	return out;
}


int64_t String_ParseInt( const StringView& sv, bool* success )
{
	int64_t val = 0;
	double valdbl = 0;
	const char* begin = sv.begin(), *end = sv.end();
	int suc = util_strtonum( &begin, end, &val, &valdbl );
	if( success )
		*success = !!suc;
	return suc == 2 ? valdbl : val;
}

double String_ParseFloat( const StringView& sv, bool* success )
{
	double val = 0;
	const char* begin = sv.begin(), *end = sv.end();
	bool suc = strtonum_real( &begin, end, &val );
	if( success )
		*success = suc;
	return val;
}

Vec2 String_ParseVec2( const StringView& sv, bool* success )
{
	bool suc = true;
	Vec2 out = {0,0};
	StringView substr = ";";
	if( success ) *success = true;
	
	out.x = String_ParseFloat( sv.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	StringView tmp = sv.after( substr );
	out.y = String_ParseFloat( tmp.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	return out;
}

Vec3 String_ParseVec3( const StringView& sv, bool* success )
{
	bool suc = true;
	Vec3 out = {0,0,0};
	StringView substr = ";";
	if( success ) *success = true;
	
	out.x = String_ParseFloat( sv.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	StringView tmp = sv.after( substr );
	out.y = String_ParseFloat( tmp.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	tmp = tmp.after( substr );
	out.z = String_ParseFloat( tmp.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	return out;
}



#define U8NFL( x ) ((x&0xC0)!=0x80)

int UTF8Decode( const char* buf, size_t size, uint32_t* outchar )
{
	char c;
	if( size == 0 )
		return 0;
	
	c = *buf;
	if( !( c & 0x80 ) )
	{
		*outchar = (uint32_t) c;
		return 1;
	}
	
	if( ( c & 0xE0 ) == 0xC0 )
	{
		if( size < 2 || U8NFL( buf[1] ) )
			return - (int) TMIN( size, (size_t) 2 );
		*outchar = (uint32_t) ( ( ((int)(buf[0]&0x1f)) << 6 ) | ((int)(buf[1]&0x3f)) );
		return 2;
	}
	
	if( ( c & 0xF0 ) == 0xE0 )
	{
		if( size < 3 || U8NFL( buf[1] ) || U8NFL( buf[2] ) )
			return - (int) TMIN( size, (size_t) 3 );
		*outchar = (uint32_t) ( ( ((int)(buf[0]&0x0f)) << 12 ) | ( ((int)(buf[1]&0x3f)) << 6 )
			| ((int)(buf[2]&0x3f)) );
		return 3;
	}
	
	if( ( c & 0xF8 ) == 0xF0 )
	{
		if( size < 4 || U8NFL( buf[1] ) || U8NFL( buf[2] ) || U8NFL( buf[3] ) )
			return - (int) TMIN( size, (size_t) 4 );
		*outchar = (uint32_t) ( ( ((int)(buf[0]&0x07)) << 18 ) | ( ((int)(buf[1]&0x3f)) << 12 )
				| ( ((int)(buf[2]&0x3f)) << 6 ) | ((int)(buf[3]&0x3f)) );
		return 4;
	}
	
	return -1;
}

int UTF8Encode( uint32_t ch, char* out )
{
	if( ch <= 0x7f )
	{
		*out = (char) ch;
		return 1;
	}
	if( ch <= 0x7ff )
	{
		out[ 0 ] = (char)( 0xc0 | ( ( ch >> 6 ) & 0x1f ) );
		out[ 1 ] = (char)( 0x80 | ( ch & 0x3f ) );
		return 2;
	}
	if( ch <= 0xffff )
	{
		out[ 0 ] = (char)( 0xe0 | ( ( ch >> 12 ) & 0x0f ) );
		out[ 1 ] = (char)( 0x80 | ( ( ch >> 6 ) & 0x3f ) );
		out[ 2 ] = (char)( 0x80 | ( ch & 0x3f ) );
		return 3;
	}
	if( ch <= 0x10ffff )
	{
		out[ 0 ] = (char)( 0xf0 | ( ( ch >> 18 ) & 0x07 ) );
		out[ 1 ] = (char)( 0x80 | ( ( ch >> 12 ) & 0x3f ) );
		out[ 2 ] = (char)( 0x80 | ( ( ch >> 6 ) & 0x3f ) );
		out[ 3 ] = (char)( 0x80 | ( ch & 0x3f ) );
		return 4;
	}

	return 0;
}


Hash HashFunc( const char* str, size_t size )
{
	size_t i, adv = size / 127 + 1;
	Hash h = 2166136261u;
	for( i = 0; i < size; i += adv )
	{
		h ^= (Hash) (uint8_t) str[ i ];
		h *= 16777619u;
	}
	return h;
}


#ifdef _WIN32
template< int N >
struct StackWString
{
	WCHAR str[ N + 1 ];
	
	StackWString( const StringView& sv )
	{
		int sz = MultiByteToWideChar( CP_UTF8, 0, sv.data(), sv.size(), str, N );
		str[ sz ] = 0;
	}
	operator const WCHAR* (){ return str; }
};
#endif


struct _IntDirIter
{
	_IntDirIter( const StringView& path ) :
		searched( false ),
		searchdir( path ),
		utf8file( "" ),
		hfind( INVALID_HANDLE_VALUE )
	{
		if( wcslen( searchdir ) <= MAX_PATH - 2 )
			wcscat( searchdir.str, L"\\*" );
	}
	~_IntDirIter()
	{
		if( searched && hfind != INVALID_HANDLE_VALUE )
			FindClose( hfind );
	}
	bool Next()
	{
		bool ret;
		if( searched )
			ret = FindNextFileW( hfind, &wfd );
		else
		{
			ret = ( hfind = FindFirstFileW( searchdir, &wfd ) ) != INVALID_HANDLE_VALUE;
			searched = true;
		}
		
		if( ret )
		{
			int sz = WideCharToMultiByte( CP_UTF8, 0, wfd.cFileName, wcslen( wfd.cFileName ), utf8file.str, MAX_PATH * 4, NULL, NULL );
			utf8file.str[ sz ] = 0;
		}
		return ret;
	}
	bool IsDirectory()
	{
		return !!( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY );
	}
	
	bool searched;
	StackWString< MAX_PATH > searchdir;
	StackString< MAX_PATH * 4 > utf8file;
	WIN32_FIND_DATAW wfd;
	HANDLE hfind;
};

DirectoryIterator::DirectoryIterator( const StringView& path )
{
	m_int = new _IntDirIter( path );
}

DirectoryIterator::~DirectoryIterator()
{
	delete m_int;
}

bool DirectoryIterator::Next()
{
	return m_int->Next();
}

StringView DirectoryIterator::Name()
{
	return m_int->utf8file.str;
}

bool DirectoryIterator::IsDirectory()
{
	return m_int->IsDirectory();
}


bool DirCreate( const StringView& path )
{
#ifdef _WIN32
	return CreateDirectoryW( StackWString< MAX_PATH >( path ), NULL );
#else
	return mkdir( StackString< 4096 >( path ) ) == 0;
#endif
}

bool CWDSet( const StringView& path )
{
#ifdef _WIN32
	return SetCurrentDirectoryW( StackWString< MAX_PATH >( path ) );
#else
	return chdir( StackString< 4096 >( path ) ) == 0;
#endif
}

bool LoadBinaryFile( const StringView& path, ByteArray& out )
{
	FILE* fp = fopen( StackPath( path ), "rb" );
	if( !fp )
		return false;
	
	fseek( fp, 0, SEEK_END );
	long len = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	
	if( len > 0x7fffffff )
	{
		fclose( fp );
		return false;
	}
	
	out.resize( len );
	bool ret = fread( out.data(), (size_t) len, 1, fp ) == 1;
	fclose( fp );
	return ret;
}

bool SaveBinaryFile( const StringView& path, const void* data, size_t size )
{
	FILE* fp = fopen( StackPath( path ), "wb" );
	if( !fp )
		return false;
	
	bool ret = fwrite( data, size, 1, fp ) == 1;
	fclose( fp );
	return ret;
}

bool LoadTextFile( const StringView& path, String& out )
{
	FILE* fp = fopen( StackPath( path ), "rb" );
	if( !fp )
		return false;
	
	fseek( fp, 0, SEEK_END );
	long len = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	
	if( len > 0x7fffffff )
	{
		fclose( fp );
		return false;
	}
	
	out.resize( len );
	bool ret = fread( out.data(), (size_t) len, 1, fp ) == 1;
	if( ret )
	{
		char* src = out.data();
		char* dst = src;
		char* end = src + out.size();
		while( src < end )
		{
			if( *src == '\r' )
			{
				if( src + 1 < end && src[1] == '\n' )
					src++;
				else
					*src = '\n';
			}
			if( src != dst )
				*dst = *src;
			src++;
			dst++;
		}
		out.resize( dst - out.data() );
	}
	fclose( fp );
	return ret;
}

bool SaveTextFile( const StringView& path, const StringView& data )
{
	FILE* fp = fopen( StackPath( path ), "wb" );
	if( !fp )
		return false;
	
#define TXT_WRITE_BUF 1024
	char buf[ TXT_WRITE_BUF ];
	char xbuf[ TXT_WRITE_BUF * 2 ];
	int xbufsz;
	
	bool ret = true;
	StringView it = data;
	while( it )
	{
		int readamount = TMIN( (int) it.size(), TXT_WRITE_BUF );
		memcpy( buf, it.data(), readamount );
		it.skip( readamount );
		
		xbufsz = 0;
		for( int i = 0; i < readamount; ++i )
		{
			if( buf[ i ] == '\n' )
				xbuf[ xbufsz++ ] = '\r';
			xbuf[ xbufsz++ ] = buf[ i ];
		}
		
		ret = ret && fwrite( xbuf, xbufsz, 1, fp ) == 1;
	}
	
	fclose( fp );
	return ret;
}

bool LoadItemListFile( const StringView& path, ItemList& out )
{
	String text;
	if( !LoadTextFile( path, text ) )
		return false;
	
	out.clear();
	
	StringView it = text;
	it = it.after_all( SPACE_CHARS );
	while( it.size() )
	{
		// prepare for in-place editing
		out.push_back( IL_Item() );
		IL_Item& outitem = out.last();
		
		// comment
		if( it.ch() == '#' )
		{
			it = it.from( "\n" ).after_all( SPACE_CHARS );
			continue;
		}
		
		// read name
		StringView value, name = it.until_any( SPACE_CHARS );
		outitem.name = name;
		
		// skip name and following spaces
		it.skip( name.size() );
		it = it.after_all( HSPACE_CHARS );
		
		// while not at end of line
		while( it.size() && it.ch() != '\n' )
		{
			String tmpval;
			
			// read and skip param name
			name = it.until( "=" );
			it.skip( name.size() + 1 );
			
			// read and skip param value
			if( it.ch() == '"' )
			{
				// parse quoted value
				const char* ptr = it.data(), *end = it.data() + it.size();
				while( ptr < end )
				{
					if( *ptr == '\n' )
					{
						// detected newline in parameter value
						return false;
					}
					if( *ptr == '\"' )
					{
						it.skip( ptr + 1 - it.data() );
						break;
					}
					if( ptr > it.m_str && *(ptr-1) == '\\' )
					{
						if( *ptr == 'n' )
							tmpval.push_back( '\n' );
						else if( *ptr == '"' )
							tmpval.push_back( '\"' );
						else
							tmpval.push_back( '\\' );
					}
					else
						tmpval.push_back( *ptr );
					ptr++;
				}
				
				value = tmpval;
			}
			else
				value = it.until_any( SPACE_CHARS );
			it.skip( value.size() );
			
			// add param
			outitem.params.set( name, value );
			
			it = it.after_all( HSPACE_CHARS );
		}
		
		it = it.after_all( SPACE_CHARS );
	}
	
	return true;
}



//
// TESTS
//

struct _teststr_ { const void* p; int x; };
inline bool operator == ( const _teststr_& a, const _teststr_& b ){ return a.p == b.p && a.x == b.x; }
inline Hash HashVar( const _teststr_& v ){ return (int)v.p + v.x; }
#define CLOSE(a,b) (fabsf((a)-(b))<SMALL_FLOAT)

#define TESTSER_SIZE (1+8+4+(4+4+4))
struct _testser_
{
	uint8_t a; int64_t b; float c; Vec3 d;
	bool operator != ( const _testser_& o ) const { return a != o.a || b != o.b || c != o.c || d != o.d; }
	template< class T > void Serialize( T& arch ){ if( T::IsText ) arch.marker( "TEST" ); arch << a << b << c << d; }
};

int TestSystems()
{
	HashTable< int, int > ht_ii;
	if( ht_ii.size() != 0 ) return 501; // empty
	if( ht_ii.getcopy( 0, 0 ) ) return 502; // miss
	ht_ii.set( 42, 12345 );
	if( ht_ii.size() == 0 ) return 503; // not empty
	if( ht_ii.item(0).key != 42 || ht_ii.item(0).value != 12345 ) return 504; // created item
	if( !ht_ii.getraw( 42 ) ) return 505; // can find item
	if( ht_ii.getcopy( 42 ) != 12345 ) return 506; // returns right value
	
	HashTable< _teststr_, int > ht_si;
	_teststr_ tskey = { "test", 42 };
	if( ht_si.size() != 0 ) return 551; // empty
	if( ht_si.getcopy( tskey, 0 ) ) return 552; // miss
	ht_si.set( tskey, 12345 );
	if( ht_si.size() == 0 ) return 553; // not empty
	if( ht_si.item(0).key.x != tskey.x || ht_si.item(0).value != 12345 ) return 554; // created item
	if( !ht_si.getraw( tskey ) ) return 555; // can find item
	if( ht_si.getcopy( tskey ) != 12345 ) return 556; // returns right value
	
	_testser_ dst, src = { 1, -2, 3, { 4, 5, 6 } };
	ByteArray barr;
	String carr;
	ByteWriter( &barr ) << src;
	if( barr.size() != TESTSER_SIZE ) return 601;
	TextWriter( &carr ) << src;
	if( !carr.size() ) return 602;
	ByteReader br( &barr );
	br << dst;
	if( br.error ) return 603;
	if( src != dst ) return 604;
	TextReader tr( &carr );
	tr << dst;
	if( tr.error ) return 605;
	if( src != dst ) return 606;
	
	Vec3 rot_angles = DEG2RAD( V3(25,50,75) );
	Mat4 rot_mtx = Mat4::CreateRotationXYZ( rot_angles );
	Vec3 out_rot_angles = rot_mtx.GetXYZAngles();
	if( !( rot_angles - out_rot_angles ).NearZero() ) return 701;
	
	Vec3 tri0[3] = { V3(0,0,0), V3(2,0,0), V3(0,3,0) };
	if( PointTriangleDistance( V3(0.5f,0.5f,0), tri0[0], tri0[1], tri0[2] ) != 0.0f ) return 801;
	if( PointTriangleDistance( V3(0,0,0), tri0[0], tri0[1], tri0[2] ) != 0.0f ) return 802;
	if( !CLOSE( PointTriangleDistance( V3(-1,-1,0), tri0[0], tri0[1], tri0[2] ), V3(-1,-1,0).Length() ) ) return 803;
	if( !CLOSE( PointTriangleDistance( V3(-1,0.5f,0), tri0[0], tri0[1], tri0[2] ), 1 ) ) return 804;
	if( !CLOSE( PointTriangleDistance( V3(0.5f,-1,0), tri0[0], tri0[1], tri0[2] ), 1 ) ) return 805;
	
	return 0;
}


