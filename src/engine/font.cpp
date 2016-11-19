

#include <ft2build.h>
#include FT_FREETYPE_H

#define NANOSVG_IMPLEMENTATION
#include <nanosvg/nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg/nanosvgrast.h>

#define FREETYPE_INCLUDED
#define NANOSVG_INCLUDED
#include "engine_int.hpp"

extern FontHashTable* g_LoadedFonts;



static FT_Library g_FTLib;
static NSVGrasterizer* g_NSVGRasterizer;

void sgrx_int_InitializeFontRendering()
{
	FT_Init_FreeType( &g_FTLib );
	g_NSVGRasterizer = nsvgCreateRasterizer();
}

void sgrx_int_FreeFontRendering()
{
	nsvgDeleteRasterizer( g_NSVGRasterizer );
	g_NSVGRasterizer = NULL;
}



static SystemFont g_SysFont( false );
static SystemFont g_SysFontOutlined( true );
SystemFont* sgrx_int_GetSystemFont( bool ol )
{
	return ol ? &g_SysFontOutlined : &g_SysFont;
}

static uint8_t g_SysFontLetters[] = {
	0x00, 0x00, 0x00, 0x00, 0x00,// (space)
	0x00, 0x00, 0x5F, 0x00, 0x00,// !
	0x00, 0x07, 0x00, 0x07, 0x00,// "
	0x14, 0x7F, 0x14, 0x7F, 0x14,// #
	0x24, 0x2A, 0x7F, 0x2A, 0x12,// $
	0x23, 0x13, 0x08, 0x64, 0x62,// %
	0x36, 0x49, 0x55, 0x22, 0x50,// &
	0x00, 0x05, 0x03, 0x00, 0x00,// '
	0x00, 0x1C, 0x22, 0x41, 0x00,// (
	0x00, 0x41, 0x22, 0x1C, 0x00,// )
	0x08, 0x2A, 0x1C, 0x2A, 0x08,// *
	0x08, 0x08, 0x3E, 0x08, 0x08,// +
	0x00, 0x50, 0x30, 0x00, 0x00,// ,
	0x08, 0x08, 0x08, 0x08, 0x08,// -
	0x00, 0x60, 0x60, 0x00, 0x00,// .
	0x20, 0x10, 0x08, 0x04, 0x02,// /
	0x3E, 0x51, 0x49, 0x45, 0x3E,// 0
	0x00, 0x42, 0x7F, 0x40, 0x00,// 1
	0x42, 0x61, 0x51, 0x49, 0x46,// 2
	0x21, 0x41, 0x45, 0x4B, 0x31,// 3
	0x18, 0x14, 0x12, 0x7F, 0x10,// 4
	0x27, 0x45, 0x45, 0x45, 0x39,// 5
	0x3C, 0x4A, 0x49, 0x49, 0x30,// 6
	0x01, 0x71, 0x09, 0x05, 0x03,// 7
	0x36, 0x49, 0x49, 0x49, 0x36,// 8
	0x06, 0x49, 0x49, 0x29, 0x1E,// 9
	0x00, 0x36, 0x36, 0x00, 0x00,// :
	0x00, 0x56, 0x36, 0x00, 0x00,// ;
	0x00, 0x08, 0x14, 0x22, 0x41,// <
	0x14, 0x14, 0x14, 0x14, 0x14,// =
	0x41, 0x22, 0x14, 0x08, 0x00,// >
	0x02, 0x01, 0x51, 0x09, 0x06,// ?
	0x32, 0x49, 0x79, 0x41, 0x3E,// @
	0x7E, 0x11, 0x11, 0x11, 0x7E,// A
	0x7F, 0x49, 0x49, 0x49, 0x36,// B
	0x3E, 0x41, 0x41, 0x41, 0x22,// C
	0x7F, 0x41, 0x41, 0x22, 0x1C,// D
	0x7F, 0x49, 0x49, 0x49, 0x41,// E
	0x7F, 0x09, 0x09, 0x01, 0x01,// F
	0x3E, 0x41, 0x41, 0x51, 0x32,// G
	0x7F, 0x08, 0x08, 0x08, 0x7F,// H
	0x00, 0x41, 0x7F, 0x41, 0x00,// I
	0x20, 0x40, 0x41, 0x3F, 0x01,// J
	0x7F, 0x08, 0x14, 0x22, 0x41,// K
	0x7F, 0x40, 0x40, 0x40, 0x40,// L
	0x7F, 0x02, 0x04, 0x02, 0x7F,// M
	0x7F, 0x04, 0x08, 0x10, 0x7F,// N
	0x3E, 0x41, 0x41, 0x41, 0x3E,// O
	0x7F, 0x09, 0x09, 0x09, 0x06,// P
	0x3E, 0x41, 0x51, 0x21, 0x5E,// Q
	0x7F, 0x09, 0x19, 0x29, 0x46,// R
	0x46, 0x49, 0x49, 0x49, 0x31,// S
	0x01, 0x01, 0x7F, 0x01, 0x01,// T
	0x3F, 0x40, 0x40, 0x40, 0x3F,// U
	0x1F, 0x20, 0x40, 0x20, 0x1F,// V
	0x7F, 0x20, 0x18, 0x20, 0x7F,// W
	0x63, 0x14, 0x08, 0x14, 0x63,// X
	0x03, 0x04, 0x78, 0x04, 0x03,// Y
	0x61, 0x51, 0x49, 0x45, 0x43,// Z
	0x00, 0x00, 0x7F, 0x41, 0x41,// [
	0x02, 0x04, 0x08, 0x10, 0x20,// "\"
	0x41, 0x41, 0x7F, 0x00, 0x00,// ]
	0x04, 0x02, 0x01, 0x02, 0x04,// ^
	0x40, 0x40, 0x40, 0x40, 0x40,// _
	0x00, 0x01, 0x02, 0x04, 0x00,// `
	0x20, 0x54, 0x54, 0x54, 0x78,// a
	0x7F, 0x48, 0x44, 0x44, 0x38,// b
	0x38, 0x44, 0x44, 0x44, 0x20,// c
	0x38, 0x44, 0x44, 0x48, 0x7F,// d
	0x38, 0x54, 0x54, 0x54, 0x18,// e
	0x08, 0x7E, 0x09, 0x01, 0x02,// f
	0x08, 0x14, 0x54, 0x54, 0x3C,// g
	0x7F, 0x08, 0x04, 0x04, 0x78,// h
	0x00, 0x44, 0x7D, 0x40, 0x00,// i
	0x20, 0x40, 0x44, 0x3D, 0x00,// j
	0x00, 0x7F, 0x10, 0x28, 0x44,// k
	0x00, 0x41, 0x7F, 0x40, 0x00,// l
	0x7C, 0x04, 0x18, 0x04, 0x78,// m
	0x7C, 0x08, 0x04, 0x04, 0x78,// n
	0x38, 0x44, 0x44, 0x44, 0x38,// o
	0x7C, 0x14, 0x14, 0x14, 0x08,// p
	0x08, 0x14, 0x14, 0x18, 0x7C,// q
	0x7C, 0x08, 0x04, 0x04, 0x08,// r
	0x48, 0x54, 0x54, 0x54, 0x20,// s
	0x04, 0x3F, 0x44, 0x40, 0x20,// t
	0x3C, 0x40, 0x40, 0x20, 0x7C,// u
	0x1C, 0x20, 0x40, 0x20, 0x1C,// v
	0x3C, 0x40, 0x30, 0x40, 0x3C,// w
	0x44, 0x28, 0x10, 0x28, 0x44,// x
	0x0C, 0x50, 0x50, 0x50, 0x3C,// y
	0x44, 0x64, 0x54, 0x4C, 0x44,// z
	0x00, 0x08, 0x36, 0x41, 0x00,// {
	0x00, 0x00, 0x7F, 0x00, 0x00,// |
	0x00, 0x41, 0x36, 0x08, 0x00,// }
	0x08, 0x08, 0x2A, 0x1C, 0x08,// ->
	0x08, 0x1C, 0x2A, 0x08, 0x08 // <-
};

void SystemFont::LoadGlyphInfo( int pxsize, uint32_t ch, SGRX_GlyphInfo* info )
{
	UNUSED( pxsize );
	info->glyph_kern_id = 0;
	info->bmsizex = 5;
	info->bmsizey = 7;
	info->bmoffx = 0;
	info->bmoffy = 0;
	info->advx = 6;
	info->advy = 8;
	if( outlined )
	{
		info->bmsizex += 2;
		info->bmsizey += 2;
		info->bmoffx -= 1;
		info->bmoffy -= 1;
	}
	if( ch < 32 || ch > 126 )
		ch = '?';
	loaded_glyph = g_SysFontLetters + ( ch - 32 ) * 5;
}

void SystemFont::GetGlyphBitmap( uint32_t* out, int pitch )
{
	if( outlined )
	{
		for( int y = 0; y < 9; ++y )
		{
			for( int x = 0; x < 7; ++x )
			{
				uint32_t& opx = out[ x + y * pitch ];
#define FHIT( x, y ) ((x)>=1&&(x)<=5&&(y)>=1&&(y)<=7&&((loaded_glyph[(x)-1]>>((y)-1))&1))
				if( FHIT( x, y ) )
					opx = 0xffffffff;
				else if( FHIT( x - 1, y ) || FHIT( x + 1, y )
					|| FHIT( x, y - 1 ) || FHIT( x, y + 1 ) )
					opx = 0xff000000;
				else
					opx = 0;
#undef FHIT
			}
		}
	}
	else
	{
		for( int y = 0; y < 7; ++y )
		{
			for( int x = 0; x < 5; ++x )
			{
				out[ x + y * pitch ] =
					( loaded_glyph[ x ] >> ( y ) ) & 1
					? 0xffffffff : 0;
			}
		}
	}
}


FTFont::FTFont() : face(NULL), rendersize(0), nohint(false)
{
}

FTFont::~FTFont()
{
	if( face )
		FT_Done_Face( face );
}

void FTFont::LoadGlyphInfo( int pxsize, uint32_t ch, SGRX_GlyphInfo* info )
{
	_Resize( pxsize );
	
	int flags = FT_LOAD_RENDER;
	if( nohint )
		flags |= FT_LOAD_NO_HINTING;
	FT_Load_Char( face, ch, flags );
	FT_GlyphSlot glyph = face->glyph;
	
	info->glyph_kern_id = FT_Get_Char_Index( face, ch );
	info->bmsizex = glyph->bitmap.width;
	info->bmsizey = glyph->bitmap.rows;
	info->bmoffx = glyph->bitmap_left;
	info->bmoffy = glyph->bitmap_top;
	info->advx = glyph->advance.x >> 6;
	info->advy = glyph->advance.y >> 6;
}

void FTFont::GetGlyphBitmap( uint32_t* out, int pitch )
{
	FT_GlyphSlot glyph = face->glyph;
	int width = glyph->bitmap.width;
	int height = glyph->bitmap.rows;
	int ftbpitch = glyph->bitmap.pitch;
	for( int y = 0; y < height; ++y )
	{
		for( int x = 0; x < width; ++x )
			out[ x + y * pitch ] = COLOR_RGBA( 255, 255, 255, glyph->bitmap.buffer[ x + y * ftbpitch ] );
	}
}

int FTFont::GetKerning( uint32_t ic1, uint32_t ic2 )
{
	if( FT_HAS_KERNING( face ) )
	{
		FT_Vector delta;
		FT_Get_Kerning( face, ic1, ic2, FT_KERNING_DEFAULT, &delta );
		return delta.x >> 6;
	}
	return 0;
}

int FTFont::GetYOffset( int pxsize )
{
	_Resize( pxsize );
	FT_Size_Metrics m = face->size->metrics;
	return -( ( ( m.ascender + abs( m.descender ) ) >> 6 ) - m.y_ppem ) / 2;
}

void FTFont::_Resize( int pxsize )
{
	if( pxsize != rendersize )
	{
		FT_Set_Pixel_Sizes( face, pxsize, 0 );
		rendersize = pxsize;
	}
}

FTFont* sgrx_int_CreateFont( const StringView& path )
{
	StringView filename = path.until( ":" );
	StringView flagdata = path.from( ":" );
	
	FTFont* font = new FTFont;
	font->nohint = flagdata.contains( ":nohint" );
	FT_Face face;
	if( FS_LoadBinaryFile( filename, font->data ) == false )
	{
		LOG_ERROR << LOG_DATE << "  Could not find font file: " << path;
		delete font;
		return NULL;
	}
	if( FT_New_Memory_Face( g_FTLib, font->data.data(), font->data.size(), 0, &face ) != 0 )
	{
		LOG_ERROR << LOG_DATE << "  Could not initialize FreeType font: " << path;
		delete font;
		return NULL;
	}
	font->face = face;
	if( g_VerboseLogging )
		LOG << "Loaded new font: " << path;
	return font;
}


SVGIconFont::SVGIconFont() : m_loaded_img(NULL), m_loaded_width(0), m_loaded_height(0), m_rendersize(0)
{
}

SVGIconFont::~SVGIconFont()
{
	for( size_t i = 0; i < m_icons.size(); ++i )
	{
		nsvgDelete( m_icons.item( i ).value );
	}
}

void SVGIconFont::LoadGlyphInfo( int pxsize, uint32_t ch, SGRX_GlyphInfo* info )
{
	m_rendersize = pxsize;
	
	// common data
	info->glyph_kern_id = 0;
	info->bmoffx = 0;
	info->bmoffy = 0;
	
	// image or no image
	NSVGimage* img = m_icons.getcopy( ch );
	m_loaded_img = img;
	if( img == NULL )
	{
		info->bmsizex = 0;
		info->bmsizey = 0;
		info->advx = pxsize;
		info->advy = pxsize;
		return;
	}
	
	if( img->width > img->height )
	{
		m_loaded_width = pxsize;
		m_loaded_height = pxsize * img->height / img->width;
	}
	else
	{
		m_loaded_width = pxsize * img->width / img->height;
		m_loaded_height = pxsize;
	}
	info->advx = info->bmsizex = m_loaded_width;
	info->advy = info->bmsizey = m_loaded_height;
}

void SVGIconFont::GetGlyphBitmap( uint32_t* out, int pitch )
{
	NSVGimage* img = m_loaded_img;
	if( img )
	{
		float scale = img->width > img->height ?
			m_loaded_width / float(img->width) :
			m_loaded_height / float(img->height);
		nsvgRasterize( g_NSVGRasterizer, m_loaded_img, 0, 0, scale,
			(uint8_t*) out, m_loaded_width, m_loaded_height, pitch * 4 );
	}
}

bool SVGIconFont::_LoadGlyph( uint32_t ch, const StringView& path )
{
	String svgdata;
	if( FS_LoadTextFile( path, svgdata ) == false )
	{
		LOG_ERROR << LOG_DATE << "  Could not read SVG file: " << path;
		return false;
	}
	
	// need null-terminated for func.
	svgdata.push_back(0);
	NSVGimage* img = nsvgParse( svgdata.data(), "px", 96 );
	if( img == NULL )
	{
		LOG_ERROR << LOG_DATE << "  Failed to parse SVG file: " << path;
		return false;
	}
	
	m_icons[ ch ] = img;
	return true;
}

SVGIconFont* sgrx_int_CreateSVGIconFont( const StringView& path )
{
	StringView filename = path.until( ":" );
	
	char bfr[ 256 ];
	String confdata;
	if( FS_LoadTextFile( filename, confdata ) == false )
	{
		LOG_ERROR << LOG_DATE << "  Could not read SVG icon font config: " << path;
		return NULL;
	}
	StringView basedir = filename.up_to_last( "/" );
	
	SVGIconFont* sif = new SVGIconFont;
	ConfigReader cfgrd( confdata );
	StringView key, value;
	while( cfgrd.Read( key, value ) )
	{
		sgrx_snprintf( bfr, 256, "%.*s%.*s",
			(int) basedir.size(), basedir.data(),
			(int) value.size(), value.data() );
		if( sif->_LoadGlyph( String_ParseInt( key ), bfr ) == false )
		{
			delete sif;
			return NULL;
		}
	}
	
	return sif;
}



FontRenderer::FontRenderer( int pagecount, int pagesize ) :
	m_cursor( V2(0) ),
	m_currentFont( NULL ),
	m_currentSize( 0 ),
	m_cache_frame( 0 )
{
	m_cache.Resize( pagecount, pagesize, pagesize );
}

FontRenderer::~FontRenderer()
{
}

bool FontRenderer::SetFont( const StringView& name, int pxsize )
{
	FontHandle fh = GR2D_GetFont( name );
	if( fh )
	{
		m_currentFont = fh;
		m_currentSize = pxsize;
	}
	return fh != NULL;
}

void FontRenderer::SetCursor( const Vec2& pos )
{
	m_cursor = pos;
}

int FontRenderer::PutText( BatchRenderer* br, const StringView& text )
{
//	LOG_FUNCTION;
	
	if( !m_currentFont )
		return 0;
	
	int n = 0;
	uint32_t prev_glyph_id = NOT_FOUND;
	
	float ifw = 1.0f / m_cache.m_pageWidth;
	float ifh = 1.0f / m_cache.m_pageHeight;
	
	float adjusted_y = m_cursor.y + m_currentFont->GetYOffset( m_currentSize );
	
	StringView it = text;
	while( it.size() > 0 )
	{
		uint32_t cp = UNICODE_INVCHAR;
		int ret = UTF8Decode( it.data(), it.size(), &cp );
		ret = abs( ret );
		it.skip( ret );
		
		GlyphCache::Node* node = _GetGlyph( cp );
		if( node && br )
		{
			if( prev_glyph_id != NOT_FOUND )
				m_cursor.x += m_currentFont->GetKerning( prev_glyph_id, node->data.glyph_kern_id );
			
			float ftx0 = node->x0 * ifw, ftx1 = node->x1 * ifw;
			float fty0 = node->y0 * ifh, fty1 = node->y1 * ifh;
			
			float fx0 = m_cursor.x + node->data.bmoffx;
			float fy0 = adjusted_y + m_currentSize - node->data.bmoffy;
			float fx1 = fx0 + node->x1 - node->x0;
			float fy1 = fy0 + node->y1 - node->y0;
			
			br->SetPrimitiveType( PT_Triangles );
			if( br->CheckSetTexture( m_cache.GetPageTexture( node->page ) ) )
				m_cache_frame++;
			
			br->Tex( ftx0, fty0 ).Pos( fx0, fy0 );
			br->Tex( ftx1, fty0 ).Pos( fx1, fy0 );
			br->Tex( ftx1, fty1 ).Pos( fx1, fy1 );
			
			br->Tex( ftx1, fty1 ).Pos( fx1, fy1 );
			br->Tex( ftx0, fty1 ).Pos( fx0, fy1 );
			br->Tex( ftx0, fty0 ).Pos( fx0, fy0 );
		}
		if( node )
		{
			m_cursor.x += node->data.advx;
		}
		prev_glyph_id = node->data.glyph_kern_id;
		n++;
	}
	return n;
}

int FontRenderer::SkipText( const StringView& text )
{
	return PutText( NULL, text );
}

float FontRenderer::GetTextWidth( const StringView& text )
{
	float ocx = m_cursor.x;
	m_cursor.x = 0;
	SkipText( text );
	float rv = m_cursor.x;
	m_cursor.x = ocx;
	return rv;
}

float FontRenderer::GetAdvanceX( uint32_t cpprev, uint32_t cpcurr )
{
//	LOG_FUNCTION;
	if( m_currentFont == NULL )
		return 0;
	
	// this glyph might not be allocated, could invalidate ptrs
	GlyphCache::Node* currnode = _GetGlyph( cpcurr );
	if( !currnode )
		return 0;
	// this glyph will be allocated
	GlyphCache::Node* prevnode = cpprev != 0 ? _GetGlyph( cpprev ) : NULL;
	
	float adv = currnode->data.advx;
	if( prevnode )
	{
		adv += m_currentFont->GetKerning(
			prevnode->data.glyph_kern_id,
			currnode->data.glyph_kern_id );
	}
	return adv;
}


FontRenderer::GlyphCache::Node* FontRenderer::_GetGlyph( uint32_t ch )
{
	CacheKey ckey = { m_currentFont, m_currentSize, ch };
	GlyphCache::Node* node = m_cache.Find( m_cache_frame, ckey );
	if( node )
		return node;
	
	// load glyph
	SGRX_GlyphInfo ginfo;
	m_currentFont->LoadGlyphInfo( m_currentSize, ch, &ginfo );
	int width = ginfo.bmsizex;
	int height = ginfo.bmsizey;
	node = m_cache.Alloc( m_cache_frame, ckey, ginfo, width, height );
	if( !node )
	{
		LOG_ERROR << "  FAILED TO ALLOCATE GLYPH!!!";
		return NULL;
	}
	
	if( g_VerboseLogging )
		LOG << "Allocated glyph " << ch << " (" << width << "x" << height << ")";
	
	Array< uint32_t > bitmap;
	bitmap.resize( width * height );
	m_currentFont->GetGlyphBitmap( bitmap.data(), width );
	m_cache.GetPageTexture( node->page ).UploadRGBA8Part( bitmap.data(), 0, width, height, node->x0, node->y0 );
	
	return node;
}


bool GR2D_LoadFont( const StringView& key, const StringView& path )
{
	LOG_FUNCTION;
	
	SGRX_IFont* fif = NULL;
	if( ( fif = sgrx_int_CreateFont( path ) ) == NULL )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load font: " << path;
		return false;
	}
	fif->m_key = key;
	g_LoadedFonts->set( fif->m_key, fif );
	return true;
}

bool GR2D_LoadSVGIconFont( const StringView& key, const StringView& path )
{
	LOG_FUNCTION;
	
	SGRX_IFont* fif = NULL;
	if( ( fif = sgrx_int_CreateSVGIconFont( path ) ) == NULL )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load SVG icon font: " << path;
		return false;
	}
	fif->m_key = key;
	g_LoadedFonts->set( fif->m_key, fif );
	return true;
}

FontHandle GR2D_GetFont( const StringView& key )
{
	return g_LoadedFonts->getcopy( key );
}

