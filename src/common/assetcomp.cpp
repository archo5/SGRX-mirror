

#define ASSIMP_IMPORTER_TYPE Assimp::Importer
#define ASSIMP_SCENE_TYPE const aiScene
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assetcomp.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <libpng/png.h>



SGRX_IFP32Handle SGRX_ResizeImage( SGRX_ImageFP32* image, int width, int height )
{
	if( width < 1 || width > 4096 ||
		height < 1 || height > 4096 )
	{
		printf( "ERROR: resize - size out of bounds" );
		return NULL;
	}
	if( image->GetWidth() == width && image->GetHeight() == height )
		return image;
	
	float fw = width, fh = height;
	float xoff = TMAX( 0.0f, image->GetWidth() / fw - 1.0f ) / ( image->GetWidth() * 2 );
	float yoff = TMAX( 0.0f, image->GetHeight() / fh - 1.0f ) / ( image->GetHeight() * 2 );
	SGRX_IFP32Handle out = new SGRX_ImageFP32( width, height );
	for( int y = 0; y < height; ++y )
	{
		for( int x = 0; x < width; ++x )
		{
			out->Pixel( x, y ) = image->GetLerp( xoff + x / fw, yoff + y / fh );
		}
	}
	return out;
}


static const char* assetimgfiltype_string_table[] =
{
	"resize",
	"sharpen",
	"to_linear",
	"from_linear",
	"expand_range",
};

const char* SGRX_AssetImgFilterType_ToString( SGRX_AssetImageFilterType aift )
{
	int fid = aift;
	if( fid <= 0 || fid >= SGRX_AIF__COUNT )
		return "Unknown";
	return assetimgfiltype_string_table[ fid - 1 ];
}

SGRX_AssetImageFilterType SGRX_AssetImgFilterType_FromString( const StringView& sv )
{
	for( int i = 1; i < SGRX_AIF__COUNT; ++i )
	{
		if( sv == assetimgfiltype_string_table[ i - 1 ] )
			return (SGRX_AssetImageFilterType) i;
	}
	return SGRX_AIF_Unknown;
}

bool SGRX_ImageFilter_Resize::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "WIDTH" )
			width = String_ParseInt( value );
		else if( key == "HEIGHT" )
			height = String_ParseInt( value );
		else if( key == "FILTER_END" )
			return true;
		else
		{
			LOG_ERROR << "Unrecognized AssetScript/ImgFilter(resize) command: " << key << "=" << value;
			return false;
		}
	}
	LOG_ERROR << "Incomplete AssetScript/ImgFilter(resize) data";
	return false;
}

void SGRX_ImageFilter_Resize::Generate( String& out )
{
	char bfr[ 128 ];
	sgrx_snprintf( bfr, 128,
		"  WIDTH %d\n"
		"  HEIGHT %d\n",
		width, height );
	out.append( bfr );
}

SGRX_IFP32Handle SGRX_ImageFilter_Resize::Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs )
{
	return SGRX_ResizeImage( image, width, height );
}

static const char* imgfltsharpen_string_table[] =
{
	"0-1",
	"1-1",
	"1-2",
};

const char* SGRX_ImgFltSharpen_ToString( SGRX_ImgFltSharpen_Mode ifsm )
{
	int fid = ifsm;
	if( fid < 0 || fid >= SGRX_IFS__COUNT )
		return "0-1";
	return imgfltsharpen_string_table[ fid ];
}

SGRX_ImgFltSharpen_Mode SGRX_ImgFltSharpen_FromString( const StringView& sv )
{
	for( int i = 0; i < SGRX_IFS__COUNT; ++i )
	{
		if( sv == imgfltsharpen_string_table[ i ] )
			return (SGRX_ImgFltSharpen_Mode) i;
	}
	return SGRX_IFS_0_1;
}

bool SGRX_ImageFilter_Sharpen::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "FACTOR" )
			factor = String_ParseFloat( value );
		else if( key == "MODE" )
			mode = SGRX_ImgFltSharpen_FromString( value );
		else if( key == "FILTER_END" )
			return true;
		else
		{
			LOG_ERROR << "Unrecognized AssetScript/ImgFilter(resize) command: " << key << "=" << value;
			return false;
		}
	}
	LOG_ERROR << "Incomplete AssetScript/ImgFilter(sharpen) data";
	return false;
}

void SGRX_ImageFilter_Sharpen::Generate( String& out )
{
	char bfr[ 128 ];
	sgrx_snprintf( bfr, 128,
		"  FACTOR %g\n"
		"  MODE %s\n",
		factor,
		SGRX_ImgFltSharpen_ToString( mode ) );
	out.append( bfr );
}

template< int w, int h >
SGRX_IFP32Handle SGRX_ImageConvolutionFilter(
	SGRX_ImageFP32* image, float* kernel )
{
	SGRX_IFP32Handle out = new SGRX_ImageFP32( image->GetWidth(), image->GetHeight() );
	int hw = w / 2, hh = h / 2;
	for( int y = 0; y < image->GetHeight(); ++y )
	{
		for( int x = 0; x < image->GetWidth(); ++x )
		{
			Vec4 col = V4(0);
			for( int ky = 0; ky < h; ++ky )
			{
				for( int kx = 0; kx < w; ++kx )
				{
					col += image->GetClamped( x - hw + kx, y - hh + ky ) * kernel[ kx + ky * w ];
				}
			}
			out->Pixel( x, y ) = col;
		}
	}
	return out;
}

SGRX_IFP32Handle SGRX_ImageFilter_Sharpen::Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs )
{
	if( factor == 0 )
		return image;
	
	float a = 0, b = -1, c = 4;
	switch( mode )
	{
	default:
	case SGRX_IFS_0_1: a = 0; b = -1; c = 4; break;
	case SGRX_IFS_1_1: a = -1; b = -1; c = 8; break;
	case SGRX_IFS_1_2: a = -1; b = -2; c = 12; break;
	}
	a *= factor;
	b *= factor;
	c *= factor;
	c += 1;
	float filter[9] =
	{
		a, b, a,
		b, c, b,
		a, b, a,
	};
	return SGRX_ImageConvolutionFilter<3,3>( image, filter );
}

bool SGRX_ImageFilter_Linear::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "FILTER_END" )
			return true;
		else
		{
			LOG_ERROR << "Unrecognized AssetScript/ImgFilter(resize) command: " << key << "=" << value;
			return false;
		}
	}
	LOG_ERROR << "Incomplete AssetScript/ImgFilter("
		<< ( inverse ? "from_linear" : "to_linear" ) << ") data";
	return false;
}

void SGRX_ImageFilter_Linear::Generate( String& out )
{
}

SGRX_IFP32Handle SGRX_ImageFilter_Linear::Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs )
{
	if( ifs.isSRGB == false )
		return image;
	SGRX_IFP32Handle out = new SGRX_ImageFP32( image->GetWidth(), image->GetHeight() );
	float factor = inverse ? 1.0f / 2.2f : 2.2f;
	for( size_t i = 0; i < image->Size(); ++i )
	{
		Vec4 col =
		{
			powf( (*image)[ i ].x, factor ),
			powf( (*image)[ i ].y, factor ),
			powf( (*image)[ i ].z, factor ),
			powf( (*image)[ i ].w, factor ),
		};
		(*out)[ i ] = col;
	}
	return out;
}


bool SGRX_ImageFilter_ExpandRange::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "MIN" )
			vmin = String_ParseVec4( value );
		else if( key == "MAX" )
			vmax = String_ParseVec4( value );
		else if( key == "FILTER_END" )
			return true;
		else
		{
			LOG_ERROR << "Unrecognized AssetScript/ImgFilter(expand_range) command: " << key << "=" << value;
			return false;
		}
	}
	LOG_ERROR << "Incomplete AssetScript/ImgFilter(expand_range) data";
	return false;
}

void SGRX_ImageFilter_ExpandRange::Generate( String& out )
{
	char bfr[ 2048 ];
	sgrx_snprintf( bfr, 2048,
		"  MIN %g;%g;%g;%g\n"
		"  MAX %g;%g;%g;%g\n",
		vmin.x, vmin.y, vmin.z, vmin.w,
		vmax.x, vmax.y, vmax.z, vmax.w );
	out.append( bfr );
}

SGRX_IFP32Handle SGRX_ImageFilter_ExpandRange::Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs )
{
	Vec4 cmin = V4( HSV( vmin.ToVec3() ), vmin.w );
	Vec4 cmax = V4( HSV( vmax.ToVec3() ), vmax.w );
	Vec4 diff = cmax - cmin;
	if( diff.x == 0 || diff.y == 0 || diff.z == 0 || diff.w == 0 )
		return image;
	SGRX_IFP32Handle out = new SGRX_ImageFP32( image->GetWidth(), image->GetHeight() );
	for( size_t i = 0; i < image->Size(); ++i )
	{
		Vec4 col =
		{
			clamp( ( (*image)[ i ].x - cmin.x ) / diff.x, 0, 1 ),
			clamp( ( (*image)[ i ].y - cmin.y ) / diff.y, 0, 1 ),
			clamp( ( (*image)[ i ].z - cmin.z ) / diff.z, 0, 1 ),
			clamp( ( (*image)[ i ].w - cmin.w ) / diff.w, 0, 1 ),
		};
		(*out)[ i ] = col;
	}
	return out;
}


static const char* texoutfmt_string_table[] =
{
	"PNG/RGBA32",
	"STX/RGBA32",
};

const char* SGRX_TextureOutputFormat_ToString( SGRX_TextureOutputFormat fmt )
{
	int fid = fmt;
	if( fid <= 0 || fid >= SGRX_TOF__COUNT )
		return "Unknown";
	return texoutfmt_string_table[ fid - 1 ];
}

SGRX_TextureOutputFormat SGRX_TextureOutputFormat_FromString( const StringView& sv )
{
	for( int i = 1; i < SGRX_TOF__COUNT; ++i )
	{
		if( sv == texoutfmt_string_table[ i - 1 ] )
			return (SGRX_TextureOutputFormat) i;
	}
	return SGRX_TOF_Unknown;
}

SGRX_TextureAsset::SGRX_TextureAsset() :
	outputType(SGRX_TOF_PNG_RGBA32),
	isSRGB(true), mips(true), lerp(true), clampx(false), clampy(false)
{
}

bool SGRX_TextureAsset::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "SOURCE" )
			sourceFile = value;
		else if( key == "OUTPUT_CATEGORY" )
			outputCategory = value;
		else if( key == "OUTPUT_NAME" )
			outputName = value;
		else if( key == "OUTPUT_TYPE" )
		{
			outputType = SGRX_TextureOutputFormat_FromString( value );
			if( outputType == SGRX_TOF_Unknown )
			{
				LOG_WARNING << "Unknown texture output type: " << value << ", changed to PNG/RGBA32";
				outputType = SGRX_TOF_PNG_RGBA32;
			}
		}
		else if( key == "IS_SRGB" )
			isSRGB = String_ParseBool( value );
		else if( key == "MIPS" )
			mips = String_ParseBool( value );
		else if( key == "LERP" )
			lerp = String_ParseBool( value );
		else if( key == "CLAMPX" )
			clampx = String_ParseBool( value );
		else if( key == "CLAMPY" )
			clampy = String_ParseBool( value );
		else if( key == "FILTER" )
		{
			SGRX_ImgFilterHandle IF;
			if( value == "resize" )
				IF = new SGRX_ImageFilter_Resize;
			else if( value == "sharpen" )
				IF = new SGRX_ImageFilter_Sharpen;
			else if( value == "to_linear" )
				IF = new SGRX_ImageFilter_Linear( false );
			else if( value == "from_linear" )
				IF = new SGRX_ImageFilter_Linear( true );
			else if( value == "expand_range" )
				IF = new SGRX_ImageFilter_ExpandRange;
			else
			{
				LOG_ERROR << "Unrecognized ImgFilter: " << value;
				return false;
			}
			if( IF->Parse( cread ) == false )
				return false;
			filters.push_back( IF );
		}
		else if( key == "TEXTURE_END" )
			return true;
		else
		{
			LOG_ERROR << "Unrecognized AssetScript/Texture command: " << key << "=" << value;
			return false;
		}
	}
	LOG_ERROR << "Incomplete AssetScript/Texture data";
	return false;
}

void SGRX_TextureAsset::Generate( String& out )
{
	out.append( "TEXTURE\n" );
	out.append( " SOURCE " ); out.append( sourceFile ); out.append( "\n" );
	out.append( " OUTPUT_CATEGORY " ); out.append( outputCategory ); out.append( "\n" );
	out.append( " OUTPUT_NAME " ); out.append( outputName ); out.append( "\n" );
	out.append( " OUTPUT_TYPE " );
	out.append( SGRX_TextureOutputFormat_ToString( outputType ) ); out.append( "\n" );
	out.append( " IS_SRGB " ); out.append( isSRGB ? "true" : "false" ); out.append( "\n" );
	out.append( " MIPS " ); out.append( mips ? "true" : "false" ); out.append( "\n" );
	out.append( " LERP " ); out.append( lerp ? "true" : "false" ); out.append( "\n" );
	out.append( " CLAMPX " ); out.append( clampx ? "true" : "false" ); out.append( "\n" );
	out.append( " CLAMPY " ); out.append( clampy ? "true" : "false" ); out.append( "\n" );
	for( size_t i = 0; i < filters.size(); ++i )
	{
		out.append( " FILTER " );
		out.append( filters[ i ]->GetName() );
		out.append( "\n" );
		filters[ i ]->Generate( out );
		out.append( " FILTER_END\n" );
	}
	out.append( "TEXTURE_END\n" );
}

void SGRX_TextureAsset::GetDesc( String& out )
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s/%s [%s]",
		StackString<100>(outputCategory).str,
		StackString<100>(outputName).str,
		SGRX_TextureOutputFormat_ToString(outputType) );
	out = bfr;
}

bool SGRX_MeshAsset::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "SOURCE" )
			sourceFile = value;
		else if( key == "OUTPUT_CATEGORY" )
			outputCategory = value;
		else if( key == "OUTPUT_NAME" )
			outputName = value;
		else if( key == "MESH_END" )
			return true;
		else
		{
			LOG_ERROR << "Unrecognized AssetScript/Mesh command: " << key << "=" << value;
			return false;
		}
	}
	LOG_ERROR << "Incomplete AssetScript/Mesh data";
	return false;
}

void SGRX_MeshAsset::Generate( String& out )
{
	out.append( "MESH\n" );
	out.append( " SOURCE " ); out.append( sourceFile ); out.append( "\n" );
	out.append( " OUTPUT_CATEGORY " ); out.append( outputCategory ); out.append( "\n" );
	out.append( " OUTPUT_NAME " ); out.append( outputName ); out.append( "\n" );
	out.append( "MESH_END\n" );
}

void SGRX_MeshAsset::GetDesc( String& out )
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s/%s",
		StackString<100>(outputCategory).str,
		StackString<100>(outputName).str );
	out = bfr;
}

bool SGRX_AssetScript::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "CATEGORY" )
		{
			categories.set( value.until("="), value.after("=") );
		}
		else if( key == "TEXTURE" )
		{
			textureAssets.push_back( SGRX_TextureAsset() );
			if( textureAssets.last().Parse( cread ) == false )
				return false;
		}
		else if( key == "MESH" )
		{
			meshAssets.push_back( SGRX_MeshAsset() );
			if( meshAssets.last().Parse( cread ) == false )
				return false;
		}
		else
		{
			LOG_ERROR << "Unrecognized AssetScript command: " << key << "=" << value;
			return false;
		}
	}
	return true;
}

void SGRX_AssetScript::Generate( String& out )
{
	for( size_t i = 0; i < categories.size(); ++i )
	{
		out.append( "CATEGORY " );
		out.append( categories.item( i ).key );
		out.append( "=" );
		out.append( categories.item( i ).value );
		out.append( "\n" );
	}
	
	for( size_t i = 0; i < textureAssets.size(); ++i )
	{
		textureAssets[ i ].Generate( out );
	}
	
	for( size_t i = 0; i < meshAssets.size(); ++i )
	{
		meshAssets[ i ].Generate( out );
	}
}

bool SGRX_AssetScript::Load( const StringView& path )
{
	String data;
	if( FS_LoadTextFile( path, data ) == false )
		return false;
	
	ConfigReader cread( data );
	return Parse( cread );
}

bool SGRX_AssetScript::Save( const StringView& path )
{
	String data;
	Generate( data );
	
	return FS_SaveTextFile( path, data );
}



SGRX_Scene3D::SGRX_Scene3D( const StringView& path ) : m_scene(NULL)
{
	ByteArray data;
	if( FS_LoadBinaryFile( path, data ) == false )
		return;
	
	m_imp = new Assimp::Importer;
	int flags =
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_LimitBoneWeights;
	m_scene = m_imp->ReadFileFromMemory( data.data(), data.size(),
		flags, StackString<32>(path.after_all(".")) );
	if( m_scene == NULL )
	{
		delete m_imp;
		return;
	}
}

SGRX_Scene3D::~SGRX_Scene3D()
{
	delete m_imp;
	m_imp = NULL;
	m_scene = NULL;
}

void SGRX_Scene3D::GetModelList( Array< String >& out )
{
	for( unsigned i = 0; i < m_scene->mNumMeshes; ++i )
	{
		out.push_back( m_scene->mMeshes[ i ]->mName.C_Str() );
	}
}



SGRX_IFP32Handle SGRX_LoadImage( const StringView& path )
{
	ByteArray data;
	if( FS_LoadBinaryFile( path, data ) == false )
	{
		LOG_ERROR << "Failed to load image file: " << path;
		return NULL;
	}
	
	int w = 0, h = 0, nc = 0;
	stbi_ldr_to_hdr_gamma( 1.0f );
	float* coldata = stbi_loadf_from_memory(
		data.data(), data.size(), &w, &h, &nc, 4 );
	if( coldata == NULL )
	{
		LOG_ERROR << "Failed to parse image file: " << path;
		return NULL;
	}
	
	SGRX_IFP32Handle ih = new SGRX_ImageFP32( w, h );
	memcpy( ih->GetData(), coldata, sizeof(Vec4) * w * h );
	
	stbi_image_free( coldata );
	return ih;
}

void SGRX_ImageF32ToRGBA8( SGRX_ImageFP32* image, ByteArray& outdata )
{
	size_t off = outdata.size(), pxcount = image->GetWidth() * image->GetHeight();
	outdata.resize( off + image->GetWidth() * image->GetHeight() * 4 );
	uint8_t* data = &outdata[ off ];
	for( size_t i = 0; i < pxcount; ++i )
	{
		const Vec4& color = (*image)[ i ];
		data[ 0 ] = clamp( color.x, 0, 1 ) * 255;
		data[ 1 ] = clamp( color.y, 0, 1 ) * 255;
		data[ 2 ] = clamp( color.z, 0, 1 ) * 255;
		data[ 3 ] = clamp( color.w, 0, 1 ) * 255;
		data += 4;
	}
}

static void sgrx_png_write_data( png_structp png_ptr, png_bytep data, png_size_t length )
{
	ByteArray* p = (ByteArray*) png_get_io_ptr( png_ptr );
	p->append( data, length );
}

int dumpimg( ByteArray& out, uint8_t* buffer, int width, int height )
{
	int code = 0;
	png_structp png_ptr;
	png_infop info_ptr;
	
	// Initialize write structure
	png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if( png_ptr == NULL )
	{
		printf( "Could not allocate write struct\n" );
		return 1;
	}
	
	// Initialize info structure
	info_ptr = png_create_info_struct( png_ptr );
	if( info_ptr == NULL )
	{
		printf( "Could not allocate info struct\n" );
		return 1;
	}
	// Setup Exception handling
	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		printf( "Error during png creation\n" );
		code = 1;
		goto fail;
	}
	
	png_set_write_fn( png_ptr, &out, sgrx_png_write_data, NULL );
	
	// Write header (8 bit colour depth)
	png_set_IHDR( png_ptr, info_ptr, width, height,
		8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
	
	png_write_info( png_ptr, info_ptr );
	for( int y = 0; y < height; ++y )
	{
		png_write_row( png_ptr, buffer );
		buffer += width * 4;
	}
	png_write_end( png_ptr, NULL );
	
fail:
	if( info_ptr != NULL ) png_free_data( png_ptr, info_ptr, PNG_FREE_ALL, -1 );
	if( png_ptr != NULL ) png_destroy_write_struct( &png_ptr, (png_infopp) NULL );
	return code;
}

bool SGRX_SaveImage( const StringView& path, SGRX_ImageFP32* image, const SGRX_TextureAsset& TA )
{
	String fullpath = path;
	ByteArray filedata;
	ByteArray imagedata;
	
	switch( TA.outputType )
	{
	case SGRX_TOF_PNG_RGBA32:
		fullpath.append( ".png" );
		SGRX_ImageF32ToRGBA8( image, imagedata );
		if( dumpimg( filedata, imagedata.data(), image->GetWidth(), image->GetHeight() ) )
		{
			printf( "ERROR: failed to encode PNG\n" );
			return false;
		}
		break;
	case SGRX_TOF_STX_RGBA32:
		fullpath.append( ".stx" );
		SGRX_ImageF32ToRGBA8( image, imagedata );
		filedata.append( "STX\0", 4 );
		{
			TextureInfo info = { TEXTYPE_2D, 1,
				image->GetWidth(), image->GetHeight(),
				1, TEXFORMAT_RGBA8, 0 };
			filedata.append( &info, sizeof(info) );
			uint32_t datasize = imagedata.size();
			filedata.append( &datasize, sizeof(datasize) );
			filedata.append( imagedata );
		}
		break;
	default:
		printf( "ERROR: invalid output format: %d\n", TA.outputType );
		break;
	}
	
	if( FS_SaveBinaryFile( fullpath, filedata.data(), filedata.size() ) == false )
	{
		printf( "ERROR: failed to save the texture file: %s\n", StackString<256>(fullpath).str );
		return false;
	}
	return true;
}

SGRX_IFP32Handle SGRX_ProcessTextureAsset( const SGRX_TextureAsset& TA )
{
	printf( "| %s => [%s] %s\n",
		StackString<256>(TA.sourceFile).str,
		StackString<256>(TA.outputCategory).str,
		StackString<256>(TA.outputName).str );
	SGRX_IFP32Handle image = SGRX_LoadImage( TA.sourceFile );
	if( image == NULL )
		return NULL;
	
	SGRX_ImageFilterState ifs =
	{
		TA.isSRGB,
	};
	for( size_t fid = 0; fid < TA.filters.size(); ++fid )
	{
		SGRX_ImageFilter* IF = TA.filters[ fid ];
		printf( "|-- filter: %s... ", IF->GetName() );
		image = IF->Process( image, ifs );
		printf( "%s\n", image ? "OK" : "ERROR" );
		if( image == NULL )
			break;
	}
	
	return image;
}

TextureHandle SGRX_FP32ToTexture( SGRX_ImageFP32* image, const SGRX_TextureAsset& TA )
{
	if( !image )
		return NULL;
	uint32_t flags = 0;
	
//	if( TA.isSRGB )
//		flags |= TEXFLAGS_SRGB;
	if( TA.lerp )
		flags |= TEXFLAGS_LERP;
	if( TA.clampx )
		flags |= TEXFLAGS_CLAMP_X;
	if( TA.clampy )
		flags |= TEXFLAGS_CLAMP_Y;
	
	Array< SGRX_IFP32Handle > mips;
	mips.push_back( image );
	if( TA.mips )
	{
		flags |= TEXFLAGS_HASMIPS;
		while( mips.last()->GetWidth() != 1 || mips.last()->GetHeight() != 1 )
		{
			int w1 = TMAX( mips.last()->GetWidth() / 2, 1 );
			int h1 = TMAX( mips.last()->GetHeight() / 2, 1 );
			mips.push_back( SGRX_ResizeImage( mips.last(), w1, h1 ) );
		}
	}
	
	TextureHandle tex = GR_CreateTexture( image->GetWidth(), image->GetHeight(),
		TEXFORMAT_RGBA8, flags, mips.size() );
	ByteArray imagedata;
	for( size_t i = 0; i < mips.size(); ++i )
	{
		imagedata.clear();
		SGRX_ImageF32ToRGBA8( mips[ i ], imagedata );
		tex.UploadRGBA8Part( imagedata.data(), i );
	}
	return tex;
}

void SGRX_ProcessAssets( const SGRX_AssetScript& script )
{
	puts( "processing assets...");
	
	puts( "- textures...");
	for( size_t tid = 0; tid < script.textureAssets.size(); ++tid )
	{
		const SGRX_TextureAsset& TA = script.textureAssets[ tid ];
		SGRX_IFP32Handle image = SGRX_ProcessTextureAsset( TA );
		if( image == NULL )
			continue;
		
		StringView catPath = script.categories.getcopy( TA.outputCategory );
		char bfr[ 520 ];
		sgrx_snprintf( bfr, 520, "%s/%s",
			StackString<256>(catPath).str,
			StackString<256>(TA.outputName).str );
		if( SGRX_SaveImage( bfr, image, TA ) )
		{
			printf( "|----------- saved!\n" );
		}
	}
	
	puts( "- meshes...");
}

