

#define __STDC_FORMAT_MACROS 1
extern "C" {
#include "dds.h"
}
#include <libpng/png.h>
#include <libjpg/jpeglib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include "renderer.hpp"



size_t TextureInfo::GetBytesPerUnit() const
{
	switch( format )
	{
	/* bytes per pixel */
	case TEXFMT_BGRA8:
	case TEXFMT_BGRX8:
	case TEXFMT_RGBA8: return 4;
	case TEXFMT_R5G6B5: return 2;
	/* bytes per block */
	case TEXFMT_DXT1: return 8;
	case TEXFMT_DXT3:
	case TEXFMT_DXT5:
	case TEXFMT_3DC: return 16;
	}
	return 0;
}

size_t TextureInfo::GetUnitCountX() const
{
	return TEXFORMAT_ISBLOCK4FORMAT( format ) ? divideup( width, 4 ) : width;
}

size_t TextureInfo::GetUnitCountY() const
{
	return TEXFORMAT_ISBLOCK4FORMAT( format ) ? divideup( height, 4 ) : height;
}

size_t TextureInfo::GetUnitCountZ() const
{
	return TEXFORMAT_ISBLOCK4FORMAT( format ) ? divideup( depth, 4 ) : depth;
}

size_t TextureInfo::GetSideSize() const
{
	size_t cx = GetUnitCountX();
	size_t bpu = GetBytesPerUnit();
	switch( type )
	{
	case TEXTYPE_2D: return cx * GetUnitCountY() * bpu;
	case TEXTYPE_CUBE: return cx * cx * bpu;
	case TEXTYPE_VOLUME: return cx * GetUnitCountY() * GetUnitCountZ() * bpu;
	}
	return 0;
}

size_t TextureInfo::GetMipSize() const
{
	return GetSideSize() * ( type == TEXTYPE_CUBE ? 6 : 1 );
}

void TextureInfo::GetCopyDims( size_t* outcopyrowsize,
	size_t* outcopyrowcount, size_t* outcopyslicecount ) const
{
	size_t cx = GetUnitCountX();
	size_t bpu = GetBytesPerUnit();
	switch( type )
	{
	case TEXTYPE_2D:
		*outcopyrowsize = cx * bpu;
		*outcopyrowcount = GetUnitCountY();
		if( outcopyslicecount )
			*outcopyslicecount = 1;
		break;
	case TEXTYPE_CUBE:
		*outcopyrowsize = cx * bpu;
		*outcopyrowcount = cx;
		if( outcopyslicecount )
			*outcopyslicecount = 1;
		break;
	case TEXTYPE_VOLUME:
		*outcopyrowsize = cx * bpu;
		if( outcopyslicecount )
		{
			// if requested explicit slice count, give it and remove it from row count
			*outcopyrowcount = GetUnitCountY();
			*outcopyslicecount = GetUnitCountZ();
		}
		else
			*outcopyrowcount = GetUnitCountY() * GetUnitCountZ();
		break;
	default:
		*outcopyrowsize = 0;
		*outcopyrowcount = 0;
		if( outcopyslicecount )
			*outcopyslicecount = 0;
		break;
	}
}

bool TextureInfo::GetMipInfo( int mip, TextureInfo* outinfo ) const
{
	TextureInfo info = *this;
	if( mip >= mipcount )
		return false;
	info.width /= pow( 2, mip ); if( info.width < 1 ) info.width = 1;
	info.height /= pow( 2, mip ); if( info.height < 1 ) info.height = 1;
	info.depth /= pow( 2, mip ); if( info.depth < 1 ) info.depth = 1;
	info.mipcount -= mip;
	*outinfo = info;
	return true;
}

size_t TextureInfo::GetMipDataOffset( int side, int mip ) const
{
	size_t off = 0;
	int mipit = mip;
	while( mipit --> 0 )
		off += GetMipDataSize( mipit );
	if( side && type == TEXTYPE_CUBE )
	{
		size_t fullsidesize = 0;
		mipit = mipcount;
		while( mipit --> 0 )
			fullsidesize += GetMipDataSize( mipit );
		off += fullsidesize * side;
	}
	return off;
}

size_t TextureInfo::GetMipDataSize( int mip ) const
{
	TextureInfo mipTI;
	if( !GetMipInfo( mip, &mipTI ) )
		return 0;
	return mipTI.GetSideSize();
}

void TextureData::SkipMips( int n )
{
	size_t skip = SkipMipInfo( n );
	data.erase( 0, skip );
}

size_t TextureData::SkipMipInfo( int n )
{
	size_t skip = 0;
	while( n --> 0 && info.mipcount > 1 )
	{
		skip += info.GetMipSize();
		info.width /= 2; if( info.width < 1 ) info.width = 1;
		info.height /= 2; if( info.height < 1 ) info.height = 1;
		info.depth /= 2; if( info.depth < 1 ) info.depth = 1;
		info.mipcount--;
	}
	return skip;
}


static const float inv255 = 1.0f / 255.0f;
static uint32_t _avg_col4( uint32_t a, uint32_t b, uint32_t c, uint32_t d )
{
	float alphapow = 2.2f;
	uint32_t ocr = ( COLOR_EXTRACT_R( a ) + COLOR_EXTRACT_R( b ) + COLOR_EXTRACT_R( c ) + COLOR_EXTRACT_R( d ) ) / 4;
	uint32_t ocg = ( COLOR_EXTRACT_G( a ) + COLOR_EXTRACT_G( b ) + COLOR_EXTRACT_G( c ) + COLOR_EXTRACT_G( d ) ) / 4;
	uint32_t ocb = ( COLOR_EXTRACT_B( a ) + COLOR_EXTRACT_B( b ) + COLOR_EXTRACT_B( c ) + COLOR_EXTRACT_B( d ) ) / 4;
	uint32_t oca = powf( 0.25f * (
		powf( COLOR_EXTRACT_A( a ) * inv255, alphapow ) +
		powf( COLOR_EXTRACT_A( b ) * inv255, alphapow ) +
		powf( COLOR_EXTRACT_A( c ) * inv255, alphapow ) +
		powf( COLOR_EXTRACT_A( d ) * inv255, alphapow )
	), 1.0f / alphapow ) * 255.0f;
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
	case DDS_FMT_R8G8B8A8: return TEXFMT_RGBA8;
	case DDS_FMT_B8G8R8A8: return TEXFMT_BGRA8;
	case DDS_FMT_B8G8R8X8: return TEXFMT_BGRX8;
	case DDS_FMT_DXT1: return TEXFMT_DXT1;
	case DDS_FMT_DXT3: return TEXFMT_DXT3;
	case DDS_FMT_DXT5: return TEXFMT_DXT5;
	default: return TEXFMT_UNKNOWN;
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


#define STX_HEADER_SIZE (8 + sizeof(TextureInfo))

bool TextureData_Load( TextureData* TD, IFileReader* fr, const StringView& filename, uint8_t lod )
{
	LOG_FUNCTION_ARG( filename );
	
	unsigned w, h;
	int err;
	ByteArray texdata;
	
	static const dds_u32 dds_supfmt[] = { DDS_FMT_R8G8B8A8, DDS_FMT_B8G8R8A8, DDS_FMT_B8G8R8X8, DDS_FMT_DXT1, DDS_FMT_DXT3, DDS_FMT_DXT5, 0 };
	dds_info ddsinfo;
	
	// Try to load STX
	fr->Seek( 0 );
	uint8_t file_header[ STX_HEADER_SIZE ] = {0};
	uint32_t hdrmaxsize = fr->TryRead( STX_HEADER_SIZE, file_header );
	if( hdrmaxsize >= STX_HEADER_SIZE &&
		fr->Length() > STX_HEADER_SIZE &&
		memcmp( file_header, "STX\0", 4 ) == 0 )
	{
		uint32_t size = 0;
		memcpy( &size, &file_header[ 4 + sizeof(TextureInfo) ], 4 );
		if( fr->Length() != size + STX_HEADER_SIZE )
		{
			LOG << LOG_DATE << "  Failed to load texture " << filename << " - incomplete data";
			return false;
		}
		memcpy( &TD->info, &file_header[ 4 ], sizeof(TextureInfo) );
		
		size_t skiplen = TD->SkipMipInfo( lod );
		ASSERT( fr->Length() - STX_HEADER_SIZE > skiplen );
		TD->data.resize( fr->Length() - STX_HEADER_SIZE - skiplen );
		fr->Seek( STX_HEADER_SIZE + skiplen );
		if( !fr->Read( TD->data.size(), TD->data.data() ) )
		{
			LOG << LOG_DATE << "  Failed to load texture " << filename << " - read error";
			return false;
		}
		goto success;
	}
	
	fr->ReadAll( texdata );
	
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
		
		TD->SkipMips( lod );
		goto success;
	}
	
	// Try to load PNG
	if( png_decode32( TD->data, &w, &h, texdata, filename ) )
	{
		TD->info.type = TEXTYPE_2D;
		TD->info.width = w;
		TD->info.height = h;
		TD->info.depth = 1;
		TD->info.format = TEXFMT_RGBA8;
	//	TD->info.flags = 0;
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
		TD->info.format = TEXFMT_RGBA8;
	//	TD->info.flags = 0;
		TD->info.mipcount = 1;
		goto success_genmips;
	}
	
	// type not supported
	goto failure;
	
success_genmips:
	if( TD->info.type == TEXTYPE_2D && ( TD->info.format == TEXFMT_RGBA8 || TD->info.format == TEXFMT_BGRA8 ) )
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
		
		TD->SkipMips( lod );
	}
success:
	
	return true;
failure:
	LOG << LOG_DATE << "  Failed to load texture '" << filename << "' - unsupported type";
	return false;
}


