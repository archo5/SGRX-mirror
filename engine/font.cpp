

#include <ft2build.h>
#include FT_FREETYPE_H

#define USE_ARRAY
#define USE_HASHTABLE
#define FT_INCLUDED
#include "engine_int.hpp"



static FT_Library g_FTLib;

void InitializeFontRendering()
{
	FT_Init_FreeType( &g_FTLib );
}


FontRenderer::FontRenderer( int pagecount, int pagesize ) :
	m_cursor_x( 0.0f ),
	m_cursor_y( 0.0f ),
	m_currentFont( NULL ),
	m_cache_frame( 0 )
{
	m_cache.Resize( pagecount, pagesize, pagesize );
}

FontRenderer::~FontRenderer()
{
	for( size_t i = 0; i < m_fonts.size(); ++i )
	{
		delete m_fonts.item( i ).value;
	}
}

bool FontRenderer::SetFont( const StringView& name, int pxsize )
{
	FontKey fk = { name, pxsize };
	Font* f = _GetOrCreateFont( fk );
	if( f )
	{
		m_currentFont = f;
		return true;
	}
	return false;
}

void FontRenderer::SetCursor( float x, float y )
{
	m_cursor_x = x;
	m_cursor_y = y;
}

int FontRenderer::PutText( BatchRenderer* br, const StringView& text )
{
	if( !m_currentFont )
		return 0;
	
	int n = 0;
	GlyphCache::Node* prev = NULL;
	
	float fw = m_cache.m_pageWidth;
	float fh = m_cache.m_pageHeight;
	
	StringView it = text;
	while( it.size() > 0 )
	{
		uint32_t cp = UNICODE_INVCHAR;
		int ret = UTF8Decode( it.data(), it.size(), &cp );
		ret = abs( ret );
		it.skip( ret );
		
		GlyphCache::Node* node = _GetGlyph( m_currentFont, cp );
		if( node && br )
		{
			float ftx0 = node->x0 / fw, ftx1 = node->x1 / fw;
			float fty0 = node->y0 / fh, fty1 = node->y1 / fh;
			
			float fx0 = m_cursor_x + node->key.bmoffx;
			float fy0 = m_cursor_y + node->key.bmoffy;
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
		prev = node;
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
	float ocx = m_cursor_x;
	m_cursor_x = 0;
	SkipText( text );
	float rv = m_cursor_x;
	m_cursor_x = ocx;
	return rv;
}


FontRenderer::Font* FontRenderer::_GetOrCreateFont( const FontKey& fk )
{
	Font* font = m_fonts.getcopy( fk, NULL );
	if( !font )
	{
		FT_Face face;
		StackString< ENGINE_MAX_PATH > path( fk.name );
		if( FT_New_Face( g_FTLib, path, 0, &face ) )
			return NULL;
		font = new Font;
		font->key = fk;
		font->face = face;
		m_fonts.set( fk, font );
	}
	return font;
}

FontRenderer::GlyphCache::Node* FontRenderer::_GetGlyph( Font* font, uint32_t ch )
{
	CacheKey ckey = { font, ch };
	GlyphCache::Node* node = m_cache.Find( m_cache_frame, ckey );
	if( node )
		return node;
	
	// load glyph
	FT_GlyphSlot glyph;
	FT_Load_Char( font->face, ch, 0 );
	glyph = font->face->glyph;
	FT_Render_Glyph( glyph, FT_RENDER_MODE_NORMAL );
	
	ckey.ft_glyph_id = FT_Get_Char_Index( font->face, ch );
	ckey.bmoffx = glyph->bitmap_left;
	ckey.bmoffy = glyph->bitmap_top;
	ckey.advx = glyph->advance.x >> 6;
	ckey.advy = glyph->advance.y >> 6;
	node = m_cache.Alloc( m_cache_frame, ckey, glyph->bitmap.width, glyph->bitmap.rows );
	
	return node;
}

