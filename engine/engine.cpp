

#include <stdio.h>
#include <time.h>
#include <intrin.h>
#include <windows.h>

#define __STDC_FORMAT_MACROS 1
extern "C" {
#include "dds.h"
}
#include "../ext/src/libpng/png.h"
#include "../ext/src/libjpg/jpeglib.h"

#define USE_ARRAY

#define INCLUDE_REAL_SDL
#include "engine_int.hpp"
#include "renderer.hpp"


uint32_t GetTimeMsec()
{
#ifdef __linux
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC, &ts );
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
	clock_t clk = clock();
	return clk * 1000 / CLOCKS_PER_SEC;
#endif
}


//
// GLOBALS
//

static bool g_Running = true;
static SDL_Window* g_Window = NULL;
static void* g_GameLib = NULL;
static IGame* g_Game = NULL;
static uint32_t g_GameTime = 0;

static RenderSettings g_RenderSettings = { 1024, 576, false, false, false };
static const char* g_RendererName = "sgrx-render-d3d9";
static void* g_RenderLib = NULL;
static pfnRndInitialize g_RfnInitialize = NULL;
static pfnRndFree g_RfnFree = NULL;
static pfnRndCreateRenderer g_RfnCreateRenderer = NULL;
static IRenderer* g_Renderer = NULL;
static Array< IScreen* > g_OverlayScreens;


//
// LOGGING
//

SGRX_Log::SGRX_Log() : end_newline(true), need_sep(false), sep("") {}
SGRX_Log::~SGRX_Log(){ sep = ""; if( end_newline ) *this << "\n"; }

void SGRX_Log::prelog()
{
	if( need_sep )
		printf( "%s", sep );
	else
		need_sep = true;
}

SGRX_Log& SGRX_Log::operator << ( EMod_Partial ){ end_newline = false; return *this; }
SGRX_Log& SGRX_Log::operator << ( ESpec_Date )
{
	time_t ttv;
	time( &ttv );
	struct tm T = *localtime( &ttv );
	char pbuf[ 256 ] = {0};
	strftime( pbuf, 255, "%Y-%m-%d %H:%M:%S", &T );
	printf( pbuf );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( const Separator& s ){ sep = s.sep; return *this; }
SGRX_Log& SGRX_Log::operator << ( int8_t v ){ prelog(); printf( "%d", (int) v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( uint8_t v ){ prelog(); printf( "%d", (int) v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( int16_t v ){ prelog(); printf( "%d", (int) v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( uint16_t v ){ prelog(); printf( "%d", (int) v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( int32_t v ){ prelog(); printf( "%" PRId32, v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( uint32_t v ){ prelog(); printf( "%" PRIu32, v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( int64_t v ){ prelog(); printf( "%" PRId64, v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( uint64_t v ){ prelog(); printf( "%" PRIu64, v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( const void* v ){ prelog(); printf( "[%p]", v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( const char* v ){ prelog(); printf( "%s", v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( const StringView& sv ){ prelog(); printf( "[%d]\"%.*s\"", (int) sv.size(), (int) sv.size(), sv.data() ); return *this; }


//
// GAME SYSTEMS
//

bool Game_HasOverlayScreen( IScreen* screen )
{
	return g_OverlayScreens.has( screen );
}

void Game_AddOverlayScreen( IScreen* screen )
{
	g_OverlayScreens.push_back( screen );
}

void Game_RemoveOverlayScreen( IScreen* screen )
{
	g_OverlayScreens.remove_all( screen );
}

static void process_overlay_screens( float dt )
{
	for( size_t i = 0; i < g_OverlayScreens.size(); ++i )
	{
		IScreen* scr = g_OverlayScreens[ i ];
		if( scr->Draw( dt ) )
			g_OverlayScreens.erase( i-- );
	}
}

void Game_OnEvent( const Event& e )
{
	for( size_t i = g_OverlayScreens.size(); i > 0; )
	{
		i--;
		IScreen* screen = g_OverlayScreens[ i ];
		if( screen->OnEvent( e ) )
			return; // event inhibited
	}
}

TextureHandle metal;
void Game_Process( float dt )
{
	float f[4] = { 0.2f, 0.4f, 0.6f, 1.0f };
	g_Renderer->Clear( f );
	
	BatchRenderer br( g_Renderer );
	br.SetTexture( metal );
	br.SetPrimitiveType( PT_Triangles );
	br.Tex( 0, 0 ).Pos( 0, 0 )
		.Tex( 1, 1 ).Pos( -50, -50 )
		.Tex( 1, 0 ).Pos( -50, 0 );
	br.Tex( 0, 0 ).Pos( 0, 0 )
		.Tex( 1, 1 ).Pos( 200, 200 )
		.Tex( 1, 0 ).Pos( 200, 0 );
	
	g_Game->OnTick( dt, g_GameTime );
	
	process_overlay_screens( dt );
}


void ParseDefaultTextureFlags( const StringView& flags, uint32_t& outusageflags )
{
	if( flags.contains( ":nosrgb" ) ) outusageflags &= ~TEXFLAGS_SRGB;
	if( flags.contains( ":srgb" ) ) outusageflags |= TEXFLAGS_SRGB;
	if( flags.contains( ":wrapx" ) ) outusageflags &= ~TEXFLAGS_CLAMP_X;
	if( flags.contains( ":wrapy" ) ) outusageflags &= ~TEXFLAGS_CLAMP_Y;
	if( flags.contains( ":clampx" ) ) outusageflags |= TEXFLAGS_CLAMP_X;
	if( flags.contains( ":clampy" ) ) outusageflags |= TEXFLAGS_CLAMP_Y;
	if( flags.contains( ":nolerp" ) ) outusageflags &= ~(TEXFLAGS_LERP_X | TEXFLAGS_LERP_Y);
	if( flags.contains( ":nolerpx" ) ) outusageflags &= ~TEXFLAGS_LERP_X;
	if( flags.contains( ":nolerpy" ) ) outusageflags &= ~TEXFLAGS_LERP_Y;
	if( flags.contains( ":lerp" ) ) outusageflags |= (TEXFLAGS_LERP_X | TEXFLAGS_LERP_Y);
	if( flags.contains( ":lerpx" ) ) outusageflags |= TEXFLAGS_LERP_X;
	if( flags.contains( ":lerpy" ) ) outusageflags |= TEXFLAGS_LERP_Y;
	if( flags.contains( ":nomips" ) ) outusageflags &= ~TEXFLAGS_HASMIPS;
	if( flags.contains( ":mips" ) ) outusageflags |= TEXFLAGS_HASMIPS;
}

bool IGame::OnLoadTexture( const StringView& key, ByteArray& outdata, uint32_t& outusageflags )
{
	if( !key )
		return false;
	
	StringView path = key.until( ":" );
	
	StackString< ENGINE_MAX_PATH > pathNT( path );
	if( !LoadBinaryFile( pathNT, outdata ) )
		return false;
	
	outusageflags = TEXFLAGS_HASMIPS | TEXFLAGS_LERP_X | TEXFLAGS_LERP_Y;
	if( path.contains( "diff." ) )
	{
		// diffuse maps
		outusageflags |= TEXFLAGS_SRGB;
	}
	
	StringView flags = key.from( ":" );
	ParseDefaultTextureFlags( flags, outusageflags );
	
	return true;
}


void SGRX_Texture::Destroy()
{
	m_texture->Destroy();
}

TextureHandle GR_CreateTexture( int width, int height, int format, int mips )
{
	TextureInfo ti = { 0, TEXTYPE_2D, width, height, 1, format, mips };
	ITexture* itex = g_Renderer->CreateTexture( &ti, NULL );
	if( !itex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	SGRX_Texture* tex = new SGRX_Texture;
	tex->m_texture = itex;
	
	LOG << "Created 2D texture: " << width << "x" << height << ", format=" << format << ", mips=" << mips;
	return tex;
}

TextureHandle GR_GetTexture( const StringView& path )
{
	uint32_t usageflags;
	ByteArray imgdata;
	if( !g_Game->OnLoadTexture( path, imgdata, usageflags ) )
	{
		LOG_ERROR << LOG_DATE << "  Could not find texture: " << path;
		return TextureHandle();
	}
	
	TextureData texdata;
	if( !TextureData_Load( &texdata, imgdata, path ) )
	{
		// error is already printed
		return TextureHandle();
	}
	
	ITexture* itex = g_Renderer->CreateTexture( &texdata.info, texdata.data.data() );
	if( !itex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	SGRX_Texture* tex = new SGRX_Texture;
	tex->m_texture = itex;
	tex->m_key.append( path.data(), path.size() );
	
	LOG << "Loaded texture: " << path;
	return tex;
}


//
// RENDERING
//

size_t TextureInfo_GetTextureSideSize( TextureInfo* TI )
{
	size_t width = TI->width, height = TI->height, depth = TI->depth;
	int bpu = 0;
	switch( TI->format )
	{
	/* bytes per pixel */
	case TEXFORMAT_BGRA8:
	case TEXFORMAT_BGRX8:
	case TEXFORMAT_RGBA8: bpu = 4; break;
	case TEXFORMAT_R5G6B5: bpu = 2; break;
	/* bytes per block */
	case TEXFORMAT_DXT1: bpu = 8; break;
	case TEXFORMAT_DXT3:
	case TEXFORMAT_DXT5: bpu = 16; break;
	}
	if( TEXFORMAT_ISBLOCK4FORMAT( TI->format ) )
	{
		width = divideup( width, 4 );
		height = divideup( height, 4 );
		depth = divideup( depth, 4 );
	}
	switch( TI->type )
	{
	case TEXTYPE_2D: return width * height * bpu;
	case TEXTYPE_CUBE: return width * width * bpu;
	case TEXTYPE_VOLUME: return width * height * depth * bpu;
	}
	return 0;
}

void TextureInfo_GetCopyDims( TextureInfo* TI, size_t* outcopyrowsize, size_t* outcopyrowcount )
{
	size_t width = TI->width, height = TI->height, depth = TI->depth;
	int bpu = 0;
	switch( TI->format )
	{
	/* bytes per pixel */
	case TEXFORMAT_BGRA8:
	case TEXFORMAT_BGRX8:
	case TEXFORMAT_RGBA8: bpu = 4; break;
	case TEXFORMAT_R5G6B5: bpu = 2; break;
	/* bytes per block */
	case TEXFORMAT_DXT1: bpu = 8; break;
	case TEXFORMAT_DXT3:
	case TEXFORMAT_DXT5: bpu = 16; break;
	}
	if( TEXFORMAT_ISBLOCK4FORMAT( TI->format ) )
	{
		width = divideup( width, 4 );
		height = divideup( height, 4 );
		depth = divideup( depth, 4 );
	}
	switch( TI->type )
	{
	case TEXTYPE_2D: *outcopyrowsize = width * bpu; *outcopyrowcount = height; break;
	case TEXTYPE_CUBE: *outcopyrowsize = width * bpu; *outcopyrowcount = width; break;
	case TEXTYPE_VOLUME: *outcopyrowsize = width * bpu; *outcopyrowcount = height * depth; break;
	default: *outcopyrowsize = 0; *outcopyrowcount = 0; break;
	}
}

bool TextureInfo_GetMipInfo( TextureInfo* TI, int mip, TextureInfo* outinfo )
{
	TextureInfo info = *TI;
	if( mip >= TI->mipcount )
		return false;
	info.width /= pow( 2, mip ); if( info.width < 1 ) info.width = 1;
	info.height /= pow( 2, mip ); if( info.height < 1 ) info.height = 1;
	info.depth /= pow( 2, mip ); if( info.depth < 1 ) info.depth = 1;
	info.mipcount -= mip;
	*outinfo = info;
	return true;
}


static uint32_t _avg_col4( uint32_t a, uint32_t b, uint32_t c, uint32_t d )
{
	uint32_t ocr = ( COLOR_EXTRACT_R( a ) + COLOR_EXTRACT_R( b ) + COLOR_EXTRACT_R( c ) + COLOR_EXTRACT_R( d ) ) / 4;
	uint32_t ocg = ( COLOR_EXTRACT_G( a ) + COLOR_EXTRACT_G( b ) + COLOR_EXTRACT_G( c ) + COLOR_EXTRACT_G( d ) ) / 4;
	uint32_t ocb = ( COLOR_EXTRACT_B( a ) + COLOR_EXTRACT_B( b ) + COLOR_EXTRACT_B( c ) + COLOR_EXTRACT_B( d ) ) / 4;
	uint32_t oca = ( COLOR_EXTRACT_A( a ) + COLOR_EXTRACT_A( b ) + COLOR_EXTRACT_A( c ) + COLOR_EXTRACT_A( d ) ) / 4;
	return COLOR_RGBA( ocr, ocg, ocb, oca );
}

static void _img_ds2x( uint32_t* dst, unsigned dstW, unsigned dstH, uint32_t* src, unsigned srcW, unsigned srcH )
{
	unsigned x, y, sx0, sy0, sx1, sy1;
	uint32_t c00, c10, c01, c11;
	for( y = 0; y < dstH; ++y )
	{
		for( x = 0; x < dstW; ++x )
		{
			sx0 = ( x * 2 ) % srcW;
			sy0 = ( y * 2 ) % srcH;
			sx1 = ( x * 2 + 1 ) % srcW;
			sy1 = ( y * 2 + 1 ) % srcH;
			
			c00 = src[ sx0 + sy0 * srcW ];
			c10 = src[ sx1 + sy0 * srcW ];
			c01 = src[ sx0 + sy1 * srcW ];
			c11 = src[ sx1 + sy1 * srcW ];
			
			dst[ x + y * dstW ] = _avg_col4( c00, c10, c01, c11 );
		}
	}
}

static int ddsfmt_to_enginefmt( dds_u32 fmt )
{
	switch( fmt )
	{
	case DDS_FMT_R8G8B8A8: return TEXFORMAT_RGBA8;
	case DDS_FMT_B8G8R8A8: return TEXFORMAT_BGRA8;
	case DDS_FMT_B8G8R8X8: return TEXFORMAT_BGRX8;
	case DDS_FMT_DXT1: return TEXFORMAT_DXT1;
	case DDS_FMT_DXT3: return TEXFORMAT_DXT3;
	case DDS_FMT_DXT5: return TEXFORMAT_DXT5;
	default: return TEXFORMAT_UNKNOWN;
	}
}


/* =============== DDS =============== */
static bool dds_read_all( dds_info* info, ByteArray& out )
{
	int s, m, nsz = info->flags & DDS_CUBEMAP ? 6 : 1;
	static const dds_u32 sideflags[6] = { DDS_CUBEMAP_PX, DDS_CUBEMAP_NX, DDS_CUBEMAP_PY, DDS_CUBEMAP_NY, DDS_CUBEMAP_PZ, DDS_CUBEMAP_NZ };
	
	out.resize( info->image.size );
	for( s = 0; s < nsz; ++s )
	{
		if( nsz == 6 && !( info->flags & sideflags[ s ] ) )
			continue;
		for( m = 0; m < info->mipcount; ++m )
		{
			if( dds_seek( info, s, m ) != DDS_SUCCESS ||
				!dds_read( info, out.data() + info->sideoffsets[ s ] + info->mipoffsets[ m ] ) )
				return false;
//			printf( "written s=%d m=%d at %d\n", s, m, info->sideoffsets[ s ] + info->mipoffsets[ m ] );
		}
	}
	
	return true;
}

/* =============== PNG =============== */
struct png_read_data
{
	uint8_t *data, *at;
	size_t size;
};

static void _png_memread( png_structp png_ptr, png_bytep data, png_size_t size )
{
	png_read_data* pd = (png_read_data*) png_get_io_ptr( png_ptr );
	ASSERT( pd->at + size <= pd->data + pd->size );
	memcpy( data, pd->at, size );
	pd->at += size;
}

static bool png_decode32( ByteArray& out, unsigned* outw, unsigned* outh, /* const */ ByteArray& texdata, const StringView& filename )
{
	// png_structp png_ptr = png_create_read_struct_2( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, C, &_ss_png_alloc, &_ss_png_free );
	png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	
	if( !png_ptr )
	{
		LOG_ERROR << "failed to initialize PNG";
		return false;
	}
	
	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		png_destroy_read_struct( &png_ptr, NULL, NULL );
	//	LOG_ERROR << "failed to read PNG image: " << filename;
		return false;
	}
	
	png_infop info_ptr = png_create_info_struct( png_ptr );
	
	if( !info_ptr )
	{
		png_destroy_read_struct( &png_ptr, NULL, NULL );
		LOG_ERROR << "failed to set up PNG reading";
		return false;
	}
	
	// Load..
	png_read_data prd = { texdata.data(), texdata.data(), texdata.size() };
	png_set_read_fn( png_ptr, &prd, (png_rw_ptr) &_png_memread );
	png_set_user_limits( png_ptr, 4096, 4096 );
	
	png_read_info( png_ptr, info_ptr );
	png_set_strip_16( png_ptr );
	png_set_packing( png_ptr );
	png_set_gray_to_rgb( png_ptr );
	// png_set_bgr( png_ptr );
	png_set_add_alpha( png_ptr, 0xffffffff, PNG_FILLER_AFTER );
	
	// send info..
	uint32_t width = png_get_image_width( png_ptr, info_ptr );
	uint32_t height = png_get_image_height( png_ptr, info_ptr );
	
	out.resize( width * height * 4 );
	uint8_t* imgdata = out.data();
	
	uint16_t offsets[ 4096 ] = {0};
	int pass, number_passes = png_set_interlace_handling(png_ptr);
	for( pass = 0; pass < number_passes; ++pass )
	{
		uint32_t y;
		for( y = 0; y < height; ++y )
		{
			png_bytep rowp = (png_bytep) imgdata + y * width * 4;
			png_bytep crp = rowp + offsets[ y ];
			png_read_rows(png_ptr, &crp, NULL, 1);
			offsets[ y ] = crp - rowp;
		}
	}
	
	png_read_end( png_ptr, info_ptr );
	
	png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
	*outw = width;
	*outh = height;
	return true;
}

/* =============== JPG =============== */
typedef struct _jpg_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
}
jpg_error_mgr;

static void _jpg_error_exit( j_common_ptr cinfo )
{
	jpg_error_mgr* myerr = (jpg_error_mgr*) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

static bool jpg_decode32( ByteArray& out, unsigned* outw, unsigned* outh, /* const */ ByteArray& texdata, const StringView& filename )
{
	struct jpeg_decompress_struct cinfo;
	jpg_error_mgr jerr;
	
	JSAMPARRAY buffer;
	int x, row_stride;
	
	cinfo.err = jpeg_std_error( &jerr.pub );
	jerr.pub.error_exit = _jpg_error_exit;
	if( setjmp( jerr.setjmp_buffer ) )
	{
		jpeg_destroy_decompress( &cinfo );
	//	LOG_ERROR << "failed to read JPEG image: " << filename;
		return false;
	}
	
	jpeg_create_decompress( &cinfo );
	jpeg_mem_src( &cinfo, texdata.data(), texdata.size() );
	jpeg_read_header( &cinfo, 1 );
	jpeg_start_decompress( &cinfo );
	
	out.resize( cinfo.output_width * cinfo.output_height * 4 );
	uint8_t* imgdata = out.data();
	
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)( (j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1 );
	
	while( cinfo.output_scanline < cinfo.output_height )
	{
		jpeg_read_scanlines( &cinfo, buffer, 1 );
		for( x = 0; x < cinfo.output_width; ++x )
		{
			imgdata[ x * 4   ] = buffer[0][ x * 3   ];
			imgdata[ x * 4+1 ] = buffer[0][ x * 3+1 ];
			imgdata[ x * 4+2 ] = buffer[0][ x * 3+2 ];
			imgdata[ x * 4+3 ] = 0xff;
		}
		imgdata += cinfo.output_width * 4;
	}
	
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );
	
	*outw = cinfo.output_width;
	*outh = cinfo.output_height;
	return true;
}

/* =============== --- =============== */


bool TextureData_Load( TextureData* TD, ByteArray& texdata, const StringView& filename )
{
	unsigned w, h;
	int err;
	
	static const dds_u32 dds_supfmt[] = { DDS_FMT_R8G8B8A8, DDS_FMT_B8G8R8A8, DDS_FMT_B8G8R8X8, DDS_FMT_DXT1, DDS_FMT_DXT3, DDS_FMT_DXT5, 0 };
	dds_info ddsinfo;
	
	memset( TD, 0, sizeof(*TD) );
	
	// Try to load DDS
	err = dds_load_from_memory( texdata.data(), texdata.size(), &ddsinfo, dds_supfmt );
	if( err == DDS_SUCCESS )
	{
		dds_u32 cmf = ddsinfo.flags & DDS_CUBEMAP_FULL;
		if( cmf && cmf != DDS_CUBEMAP_FULL )
		{
			dds_close( &ddsinfo );
			LOG << LOG_DATE << "  Failed to load texture '" << filename << "' - incomplete cubemap";
			return false;
		}
		TD->info.type = ddsinfo.flags & DDS_CUBEMAP ? TEXTYPE_CUBE : ( ddsinfo.flags & DDS_VOLUME ? TEXTYPE_VOLUME : TEXTYPE_2D );
		TD->info.width = ddsinfo.image.width;
		TD->info.height = ddsinfo.image.height;
		TD->info.depth = ddsinfo.image.depth;
		TD->info.format = ddsfmt_to_enginefmt( ddsinfo.image.format );
		TD->info.mipcount = ddsinfo.mipcount;
		if( !dds_read_all( &ddsinfo, TD->data ) )
		{
			LOG << LOG_DATE << "  Failed to load texture '" << filename << "' - unknown DDS read error";
			return false;
		}
		dds_close( &ddsinfo );
		goto success;
	}
	
	// Try to load PNG
	if( png_decode32( TD->data, &w, &h, texdata, filename ) )
	{
		TD->info.type = TEXTYPE_2D;
		TD->info.width = w;
		TD->info.height = h;
		TD->info.depth = 1;
		TD->info.format = TEXFORMAT_RGBA8;
		TD->info.flags = 0;
		TD->info.mipcount = 1;
		goto success_genmips;
	}
	
	// Try to load JPG
	if( jpg_decode32( TD->data, &w, &h, texdata, filename ) )
	{
		TD->info.type = TEXTYPE_2D;
		TD->info.width = w;
		TD->info.height = h;
		TD->info.depth = 1;
		TD->info.format = TEXFORMAT_RGBA8;
		TD->info.flags = 0;
		TD->info.mipcount = 1;
		goto success_genmips;
	}
	
	// type not supported
	goto failure;
	
success_genmips:
	if( TD->info.type == TEXTYPE_2D && ( TD->info.format == TEXFORMAT_RGBA8 || TD->info.format == TEXFORMAT_BGRA8 ) )
	{
		size_t addspace = 0;
		unsigned char* cur;
		
		// calculate additional space required
		while( w > 1 || h > 1 )
		{
			addspace += w * h * 4;
			w /= 2; if( w < 1 ) w = 1;
			h /= 2; if( h < 1 ) h = 1;
			TD->info.mipcount++;
		}
		addspace += 4; /* 1x1 */
		w = TD->info.width;
		h = TD->info.height;
		
		// reallocate
		TD->data.resize( addspace );
		
		// do cascaded ds2x
		cur = TD->data.data();
		while( w > 1 || h > 1 )
		{
			unsigned char* dst = cur + w * h * 4;
			unsigned pw = w, ph = h;
			w /= 2; if( w < 1 ) w = 1;
			h /= 2; if( h < 1 ) h = 1;
			_img_ds2x( (uint32_t*) dst, w, h, (uint32_t*) cur, pw, ph );
			cur = dst;
		}
	}
success:
	
	return true;
failure:
	LOG << LOG_DATE << "  Failed to load texture '" << filename << "' - unsupported type";
	return false;
}

size_t TextureData_GetMipDataOffset( TextureInfo* texinfo, void* data, int side, int mip )
{
	size_t off = 0;
	int mipit = mip;
	while( mipit --> 0 )
		off += TextureData_GetMipDataSize( texinfo, mipit );
	if( side && texinfo->type == TEXTYPE_CUBE )
	{
		size_t fullsidesize = 0;
		mipit = texinfo->mipcount;
		while( mipit --> 0 )
			fullsidesize += TextureData_GetMipDataSize( texinfo, mipit );
		off += fullsidesize * side;
	}
	return off;
}

size_t TextureData_GetMipDataSize( TextureInfo* texinfo, int mip )
{
	TextureInfo mipTI;
	if( !TextureInfo_GetMipInfo( texinfo, mip, &mipTI ) )
		return 0;
	return TextureInfo_GetTextureSideSize( &mipTI );
}

BatchRenderer::BatchRenderer( struct IRenderer* r ) : m_renderer( r ), m_texture( NULL ), m_primType( PT_None )
{
	m_swapRB = r->GetInfo().swapRB;
	m_proto.x = 0;
	m_proto.y = 0;
	m_proto.z = 0;
	m_proto.u = 0;
	m_proto.v = 0;
	m_proto.color = 0xffffffff;
}

BatchRenderer& BatchRenderer::AddVertices( Vertex* verts, int count )
{
	m_verts.reserve( m_verts.size() + count );
	for( int i = 0; i < count; ++i )
		AddVertex( verts[ i ] );
	return *this;
}

BatchRenderer& BatchRenderer::AddVertex( const Vertex& vert )
{
	m_verts.push_back( vert );
	return *this;
}

BatchRenderer& BatchRenderer::Colb( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	uint32_t col;
	if( m_swapRB )
		col = COLOR_RGBA( b, g, r, a );
	else
		col = COLOR_RGBA( r, g, b, a );
	m_proto.color = col;
	return *this;
}

BatchRenderer& BatchRenderer::SetPrimitiveType( EPrimitiveType pt )
{
	if( pt != m_primType )
	{
		Flush();
		m_primType = pt;
	}
	return *this;
}

BatchRenderer& BatchRenderer::SetTexture( SGRX_Texture* tex )
{
	if( tex != m_texture )
	{
		Flush();
		m_texture = tex;
	}
	return *this;
}

BatchRenderer& BatchRenderer::Flush()
{
	if( m_verts.size() )
	{
		m_renderer->DrawBatchVertices( m_verts.data(), m_verts.size(), m_primType, m_texture ? m_texture->m_texture : NULL );
		m_verts.clear();
	}
	return *this;
}



//
// INTERNALS
//

static int init_graphics()
{
	g_Window = SDL_CreateWindow( "SGRX Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_RenderSettings.width, g_RenderSettings.height, 0 );
	
	char renderer_dll[ 65 ] = {0};
	snprintf( renderer_dll, 64, "%s.dll", g_RendererName );
	g_RenderLib = SDL_LoadObject( renderer_dll );
	if( !g_RenderLib )
	{
		LOG_ERROR << "Failed to load renderer: '" << renderer_dll << "'";
		return 101;
	}
	g_RfnInitialize = (pfnRndInitialize) SDL_LoadFunction( g_RenderLib, "Initialize" );
	g_RfnFree = (pfnRndFree) SDL_LoadFunction( g_RenderLib, "Free" );
	g_RfnCreateRenderer = (pfnRndCreateRenderer) SDL_LoadFunction( g_RenderLib, "CreateRenderer" );
	if( !g_RfnInitialize || !g_RfnFree || !g_RfnCreateRenderer )
	{
		LOG_ERROR << "Failed to load functions from renderer (" << renderer_dll << ")";
		return 102;
	}
	const char* rendername = g_RendererName;
	if( !g_RfnInitialize( &rendername ) )
	{
		LOG_ERROR << "Failed to load renderer (" << rendername << ")";
		return 103;
	}
	
	SDL_SysWMinfo sysinfo;
	SDL_VERSION( &sysinfo.version );
	if( SDL_GetWindowWMInfo( g_Window, &sysinfo ) <= 0 )
	{
		LOG_ERROR << "Failed to get window pointer: " << SDL_GetError();
		return 104;
	}
	
	g_Renderer = g_RfnCreateRenderer( g_RenderSettings, sysinfo.info.win.window );
	if( !g_Renderer )
	{
		LOG_ERROR << "Failed to create renderer (" << rendername << ")";
		return 105;
	}
	
	LOG << LOG_DATE << "  Loaded renderer: " << rendername;
	
	return 0;
}

static void free_graphics()
{
	g_Renderer->Destroy();
	g_Renderer = NULL;
	
	g_RfnFree();
	
	SDL_UnloadObject( g_RenderLib );
	g_RenderLib = NULL;
	
	SDL_DestroyWindow( g_Window );
	g_Window = NULL;
}


typedef IGame* (*pfnCreateGame) ();

int SGRX_EntryPoint( int argc, char** argv, int debug )
{
	LOG << LOG_DATE << "  Engine started";
	
	/* initialize SDL */
	if( SDL_Init(
		SDL_INIT_TIMER | SDL_INIT_VIDEO |
		SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC |
		SDL_INIT_GAMECONTROLLER |
		SDL_INIT_EVENTS | SDL_INIT_NOPARACHUTE
	) < 0 )
	{
		LOG_ERROR << "Couldn't initialize SDL: " << SDL_GetError();
		return 5;
	}
	
	g_GameLib = SDL_LoadObject( "game.dll" );
	if( !g_GameLib )
	{
		LOG_ERROR << "Failed to load game.dll";
		return 6;
	}
	pfnCreateGame cgproc = (pfnCreateGame) SDL_LoadFunction( g_GameLib, "CreateGame" );
	if( !cgproc )
	{
		LOG_ERROR << "Failed to find entry point";
		return 7;
	}
	g_Game = cgproc();
	if( !g_Game )
	{
		LOG_ERROR << "Failed to create the game";
		return 8;
	}
	
	g_Game->OnConfigure( argc, argv );
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	
	if( init_graphics() )
		return 16;
	
	g_Game->OnInitialize();
	metal = GR_GetTexture( "metal0.png" );
	
	uint32_t prevtime = GetTimeMsec();
	SDL_Event event;
	while( g_Running )
	{
		while( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_QUIT )
			{
				g_Running = false;
				break;
			}
			// TODO process
		}
		
		uint32_t curtime = GetTimeMsec();
		uint32_t dt = curtime - prevtime;
		prevtime = curtime;
		g_GameTime += dt;
		float fdt = dt / 1000.0f;
		
		Game_Process( fdt );
		
		g_Renderer->Swap();
	}
	metal = NULL;
	
	g_Game->OnDestroy();
	
	free_graphics();
	
	SDL_UnloadObject( g_GameLib );
	
	LOG << LOG_DATE << "  Engine finished";
	return 0;
}

