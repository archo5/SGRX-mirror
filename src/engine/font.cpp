

#include <ft2build.h>
#include FT_FREETYPE_H

#define NANOSVG_IMPLEMENTATION
#include <nanosvg/nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg/nanosvgrast.h>

#define USE_ARRAY
#define USE_HASHTABLE
#define FREETYPE_INCLUDED
#define NANOSVG_INCLUDED
#include "engine_int.hpp"



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
	return fh;
}

void FontRenderer::SetCursor( const Vec2& pos )
{
	m_cursor = pos;
}

int FontRenderer::PutText( BatchRenderer* br, const StringView& text )
{
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
				m_cursor.x += m_currentFont->GetKerning( prev_glyph_id, node->key.info.glyph_kern_id );
			
			float ftx0 = node->x0 * ifw, ftx1 = node->x1 * ifw;
			float fty0 = node->y0 * ifh, fty1 = node->y1 * ifh;
			
			float fx0 = m_cursor.x + node->key.info.bmoffx;
			float fy0 = adjusted_y + m_currentSize - node->key.info.bmoffy;
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
			m_cursor.x += node->key.info.advx;
		}
		prev_glyph_id = node->key.info.glyph_kern_id;
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
	if( m_currentFont == NULL )
		return 0;
	
	GlyphCache::Node* prevnode = cpprev != 0 ? _GetGlyph( cpprev ) : NULL;
	GlyphCache::Node* currnode = _GetGlyph( cpcurr );
	if( !currnode )
		return 0;
	
	float adv = currnode->key.info.advx;
	if( prevnode )
	{
		adv += m_currentFont->GetKerning(
			prevnode->key.info.glyph_kern_id,
			currnode->key.info.glyph_kern_id );
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
	m_currentFont->LoadGlyphInfo( m_currentSize, ch, &ckey.info );
	int width = ckey.info.bmsizex;
	int height = ckey.info.bmsizey;
	node = m_cache.Alloc( m_cache_frame, ckey, width, height );
	if( !node )
	{
		LOG_ERROR << "  FAILED TO ALLOCATE GLYPH!!!";
		return NULL;
	}
	
	LOG << "Allocated glyph " << ch << " (" << width << "x" << height << ")";
	
	Array< uint32_t > bitmap;
	bitmap.resize( width * height );
	m_currentFont->GetGlyphBitmap( bitmap.data(), width );
	m_cache.GetPageTexture( node->page ).UploadRGBA8Part( bitmap.data(), 0, width, height, node->x0, node->y0 );
	
	return node;
}

