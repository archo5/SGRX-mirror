

#include <utility>
#include <stdio.h>

#define USE_VEC3
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE

#include "utils.hpp"


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



/* string -> number conversion */

typedef const char CCH;

static inline bool hexchar( char c )
	{ return ( (c) >= '0' && (c) <= '9' ) ||
	( (c) >= 'a' && (c) <= 'f' ) || ( (c) >= 'A' && (c) <= 'F' ); }
static inline int gethex( char c )
	{ return ( (c) >= '0' && (c) <= '9' ) ? ( (c) - '0' ) :
	( ( (c) >= 'a' && (c) <= 'f' ) ? ( (c) - 'a' + 10 ) : ( (c) - 'A' + 10 ) ); }
static inline bool decchar( char c ){ return c >= '0' && c <= '9'; }
static inline int getdec( char c ){ return c - '0'; }
static inline bool octchar( char c ){ return c >= '0' && c <= '7'; }
static inline int getoct( char c ){ return c - '0'; }
static inline bool binchar( char c ){ return c == '0' || c == '1'; }
static inline int getbin( char c ){ return c - '0'; }

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

float String_ParseFloat( const StringView& sv, bool* success )
{
	double val = 0;
	const char* begin = sv.begin(), *end = sv.end();
	bool suc = strtonum_real( &begin, end, &val );
	if( success )
		*success = suc;
	return val;
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

#define SPACE_CHARS " \t\n"
#define HSPACE_CHARS " \t"
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
	if( ht_si.size() != 0 ) return 501; // empty
	if( ht_si.getcopy( tskey, 0 ) ) return 502; // miss
	ht_si.set( tskey, 12345 );
	if( ht_si.size() == 0 ) return 503; // not empty
	if( ht_si.item(0).key.x != tskey.x || ht_si.item(0).value != 12345 ) return 504; // created item
	if( !ht_si.getraw( tskey ) ) return 505; // can find item
	if( ht_si.getcopy( tskey ) != 12345 ) return 506; // returns right value
	
	return 0;
}


