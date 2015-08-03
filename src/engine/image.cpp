

#define __STDC_FORMAT_MACROS 1
extern "C" {
#include "dds.h"
}
#include <libpng/png.h>
#include <libjpg/jpeglib.h>


#define USE_ARRAY
#include "renderer.hpp"



size_t TextureInfo_GetTextureSideSize( const TextureInfo* TI )
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

void TextureInfo_GetCopyDims( const TextureInfo* TI, size_t* outcopyrowsize, size_t* outcopyrowcount )
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

bool TextureInfo_GetMipInfo( const TextureInfo* TI, int mip, TextureInfo* outinfo )
{
	TextureInfo info = *TI;
	if( mip >= TI->mipcount )
		return false;
	info.width /= powf( 2, mip ); if( info.width < 1 ) info.width = 1;
	info.height /= powf( 2, mip ); if( info.height < 1 ) info.height = 1;
	info.depth /= powf( 2, mip ); if( info.depth < 1 ) info.depth = 1;
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
		for( m = 0; m < (int) info->mipcount; ++m )
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
	LOG_FUNCTION;
	
	// png_structp png_ptr = png_create_read_struct_2( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, C, &_ss_png_alloc, &_ss_png_free );
	png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	
	if( !png_ptr )
	{
		LOG_ERROR << "failed to initialize PNG";
		return false;
	}
	
	png_infop info_ptr = NULL;
	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		png_destroy_read_struct( &png_ptr, info_ptr ? &info_ptr : NULL, NULL );
	//	LOG_ERROR << "failed to read PNG image: " << filename;
		return false;
	}
	
	info_ptr = png_create_info_struct( png_ptr );
	
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
	LOG_FUNCTION;
	
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
		for( x = 0; x < (int) cinfo.output_width; ++x )
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
	LOG_FUNCTION_ARG( filename );
	
	unsigned w, h;
	int err;
	
	static const dds_u32 dds_supfmt[] = { DDS_FMT_R8G8B8A8, DDS_FMT_B8G8R8A8, DDS_FMT_B8G8R8X8, DDS_FMT_DXT1, DDS_FMT_DXT3, DDS_FMT_DXT5, 0 };
	dds_info ddsinfo;
	
	memset( TD, 0, sizeof(*TD) );
	
	// Try to load STX
	if( texdata.size() > 8 + sizeof(TextureInfo) && memcmp( texdata.data(), "STX\0", 4 ) == 0 )
	{
		uint32_t size = 0;
		memcpy( &size, &texdata[ 4 + sizeof(TextureInfo) ], 4 );
		if( texdata.size() != size + 8 + sizeof(TextureInfo) )
		{
			LOG << LOG_DATE << "  Failed to load texture " << filename << " - incomplete data";
			return false;
		}
		memcpy( &TD->info, &texdata[ 4 ], sizeof(TextureInfo) );
		TD->data.assign( &texdata[ 8 + sizeof(TextureInfo) ], size );
		goto success;
	}
	
	// Try to load DDS
	err = dds_load_from_memory( texdata.data(), texdata.size(), &ddsinfo, dds_supfmt );
	if( err == DDS_SUCCESS || err == DDS_ENOTSUP )
	{
		if( err == DDS_ENOTSUP )
		{
			LOG << LOG_DATE << "  Failed to load texture " << filename << " - unsupported DDS image format";
			return false;
		}
		dds_u32 cmf = ddsinfo.flags & DDS_CUBEMAP_FULL;
		if( cmf && cmf != DDS_CUBEMAP_FULL )
		{
			dds_close( &ddsinfo );
			LOG << LOG_DATE << "  Failed to load texture " << filename << " - incomplete cubemap";
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
			LOG << LOG_DATE << "  Failed to load texture " << filename << " - unknown DDS read error";
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
		LOG_FUNCTION_ARG( "GENERATE_MIPMAPS" );
		
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

size_t TextureData_GetMipDataOffset( TextureInfo* texinfo, int side, int mip )
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


