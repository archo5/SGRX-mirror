

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

SGRX_IFP32Handle SGRX_ImagePower( SGRX_ImageFP32* image, float power )
{
	SGRX_IFP32Handle out = new SGRX_ImageFP32( image->GetWidth(), image->GetHeight() );
	for( size_t i = 0; i < image->Size(); ++i )
	{
		Vec4 col =
		{
			powf( (*image)[ i ].x, power ),
			powf( (*image)[ i ].y, power ),
			powf( (*image)[ i ].z, power ),
			powf( (*image)[ i ].w, power ),
		};
		(*out)[ i ] = col;
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
	"bcp",
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

bool SGRX_ImageFilter::ParseCMFParam( StringView key, StringView value )
{
	if( key == "CMF_COLORS" )
	{
		colors = String_ParseInt( value ) & 0xf;
		return true;
	}
	if( key == "CMF_BLEND" )
	{
		blend = String_ParseFloat( value );
		return true;
	}
	if( key == "CMF_CLAMP" )
	{
		cclamp = String_ParseBool( value );
		return true;
	}
	return false;
}

void SGRX_ImageFilter::GenerateCMFParams( String& out )
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256,
		"  CMF_BLEND %g\n"
		"  CMF_CLAMP %s\n"
		"  CMF_COLORS 0x%x\n",
		blend, cclamp ? "true" : "false", int(colors) );
	out.append( bfr );
}

void SGRX_ImageFilter::CMFBlend( SGRX_ImageFP32* src, SGRX_ImageFP32* dst )
{
	if( src->GetWidth() != dst->GetWidth() || src->GetHeight() != dst->GetHeight() )
		return;
	for( size_t i = 0; i < src->Size(); ++i )
	{
		Vec4 srccol = (*src)[ i ];
		Vec4& dstcol = (*dst)[ i ];
		dstcol = TLERP( srccol, dstcol, blend );
		if( cclamp )
		{
			dstcol.x = clamp( dstcol.x, 0, 1 );
			dstcol.y = clamp( dstcol.y, 0, 1 );
			dstcol.z = clamp( dstcol.z, 0, 1 );
			dstcol.w = clamp( dstcol.w, 0, 1 );
		}
		if( ~colors & 0x1 ) dstcol.x = srccol.x;
		if( ~colors & 0x2 ) dstcol.y = srccol.y;
		if( ~colors & 0x4 ) dstcol.z = srccol.z;
		if( ~colors & 0x8 ) dstcol.w = srccol.w;
	}
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
		else if( key == "SRGB" )
			srgb = String_ParseBool( value );
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
		"  HEIGHT %d\n"
		"  SRGB %s\n",
		width, height, srgb ? "true" : "false" );
	out.append( bfr );
}

SGRX_IFP32Handle SGRX_ImageFilter_Resize::Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs )
{
	SGRX_IFP32Handle out = image;
	if( srgb )
		out = SGRX_ImagePower( out, 2.2f );
	out = SGRX_ResizeImage( out, width, height );
	if( srgb )
		out = SGRX_ImagePower( out, 1.0f / 2.2f );
	return out;
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
		else if( ParseCMFParam( key, value ) )
			continue;
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
	GenerateCMFParams( out );
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
	SGRX_IFP32Handle dst = SGRX_ImageConvolutionFilter<3,3>( image, filter );
	CMFBlend( image, dst );
	return dst;
}

bool SGRX_ImageFilter_Linear::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "FILTER_END" )
			return true;
		else if( ParseCMFParam( key, value ) )
			continue;
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
	GenerateCMFParams( out );
}

SGRX_IFP32Handle SGRX_ImageFilter_Linear::Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs )
{
	if( ifs.isSRGB == false )
		return image;
	SGRX_IFP32Handle out = SGRX_ImagePower( image, inverse ? 1.0f / 2.2f : 2.2f );
	CMFBlend( image, out );
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
		else if( ParseCMFParam( key, value ) )
			continue;
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
	GenerateCMFParams( out );
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
			( (*image)[ i ].x - cmin.x ) / diff.x,
			( (*image)[ i ].y - cmin.y ) / diff.y,
			( (*image)[ i ].z - cmin.z ) / diff.z,
			( (*image)[ i ].w - cmin.w ) / diff.w,
		};
		(*out)[ i ] = col;
	}
	CMFBlend( image, out );
	return out;
}


bool SGRX_ImageFilter_BCP::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "APPLY_BC1" )
			apply_bc1 = String_ParseBool( value );
		else if( key == "BRIGHTNESS" )
			brightness = String_ParseFloat( value );
		else if( key == "CONTRAST" )
			contrast = String_ParseFloat( value );
		else if( key == "APPLY_POW" )
			apply_pow = String_ParseBool( value );
		else if( key == "POWER" )
			power = String_ParseFloat( value );
		else if( key == "APPLY_BC2" )
			apply_bc2 = String_ParseBool( value );
		else if( key == "BRIGHTNESS_2" )
			brightness_2 = String_ParseFloat( value );
		else if( key == "CONTRAST_2" )
			contrast_2 = String_ParseFloat( value );
		else if( key == "FILTER_END" )
			return true;
		else if( ParseCMFParam( key, value ) )
			continue;
		else
		{
			LOG_ERROR << "Unrecognized AssetScript/ImgFilter(expand_range) command: " << key << "=" << value;
			return false;
		}
	}
	LOG_ERROR << "Incomplete AssetScript/ImgFilter(expand_range) data";
	return false;
}

void SGRX_ImageFilter_BCP::Generate( String& out )
{
	char bfr[ 4096 ];
	sgrx_snprintf( bfr, 4096,
		"  APPLY_BC1 %s\n"
		"  BRIGHTNESS %g\n"
		"  CONTRAST %g\n"
		"  APPLY_POW %s\n"
		"  POWER %g\n"
		"  APPLY_BC2 %s\n"
		"  BRIGHTNESS_2 %g\n"
		"  CONTRAST_2 %g\n",
		apply_bc1 ? "true" : "false", brightness, contrast,
		apply_pow ? "true" : "false", power,
		apply_bc2 ? "true" : "false", brightness_2, contrast_2 );
	out.append( bfr );
	GenerateCMFParams( out );
}

SGRX_IFP32Handle SGRX_ImageFilter_BCP::Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs )
{
	SGRX_IFP32Handle out = new SGRX_ImageFP32( image->GetWidth(), image->GetHeight() );
	for( size_t i = 0; i < image->Size(); ++i )
		(*out)[ i ] = (*image)[ i ];
	if( apply_bc1 )
	{
		for( size_t i = 0; i < image->Size(); ++i )
			(*out)[ i ] = (*out)[ i ] * V4(contrast) + V4(brightness);
	}
	if( apply_pow )
	{
		for( size_t i = 0; i < image->Size(); ++i )
		{
			Vec4 col = (*out)[ i ];
			(*out)[ i ] = V4
			(
				pow( fabsf( col.x ), power ) * sign( col.x ),
				pow( fabsf( col.y ), power ) * sign( col.y ),
				pow( fabsf( col.z ), power ) * sign( col.z ),
				pow( fabsf( col.w ), power ) * sign( col.w )
			);
		}
	}
	if( apply_bc2 )
	{
		for( size_t i = 0; i < image->Size(); ++i )
			(*out)[ i ] = (*out)[ i ] * V4(contrast_2) + V4(brightness_2);
	}
	CMFBlend( image, out );
	return out;
}


static const char* texoutfmt_string_table[] =
{
	"PNG/RGBA32",
	"STX/RGBA32",
};
static const char* texoutfmt_ext_string_table[] =
{
	"png",
	"stx",
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

const char* SGRX_TextureOutputFormat_Ext( SGRX_TextureOutputFormat fmt )
{
	int fid = fmt;
	if( fid <= 0 || fid >= SGRX_TOF__COUNT )
		return "";
	return texoutfmt_ext_string_table[ fid - 1 ];
}

SGRX_TextureAsset::SGRX_TextureAsset() :
	outputType(SGRX_TOF_PNG_RGBA32),
	isSRGB(true), mips(true), lerp(true), clampx(false), clampy(false)
{
}

void SGRX_TextureAsset::Clone( const SGRX_TextureAsset& other )
{
	*this = other;
	for( size_t i = 0; i < filters.size(); ++i )
	{
		filters[ i ] = filters[ i ]->Clone();
	}
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
			else if( value == "bcp" )
				IF = new SGRX_ImageFilter_BCP;
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

void SGRX_TextureAsset::GetFullName( String& out )
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s/%s",
		StackString<100>(outputCategory).str,
		StackString<100>(outputName).str );
	out = bfr;
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


bool SGRX_MeshAssetPart::Parse( ConfigReader& cread )
{
	StringView key, value;
	while( cread.Read( key, value ) )
	{
		if( key == "MESH_NAME" )
			meshName = value;
		else if( key == "SHADER" )
			shader = value;
		else if( key == "TEXTURE0" ) textures[ 0 ] = value;
		else if( key == "TEXTURE1" ) textures[ 1 ] = value;
		else if( key == "TEXTURE2" ) textures[ 2 ] = value;
		else if( key == "TEXTURE3" ) textures[ 3 ] = value;
		else if( key == "TEXTURE4" ) textures[ 4 ] = value;
		else if( key == "TEXTURE5" ) textures[ 5 ] = value;
		else if( key == "TEXTURE6" ) textures[ 6 ] = value;
		else if( key == "TEXTURE7" ) textures[ 7 ] = value;
		else if( key == "MTL_FLAGS" )
			mtlFlags = String_ParseInt( value );
		else if( key == "MTL_BLENDMODE" )
			mtlBlendMode = String_ParseInt( value );
		else if( key == "PART_END" )
			return true;
		else
		{
			LOG_ERROR << "Unrecognized AssetScript/Mesh/Part command: " << key << "=" << value;
			return false;
		}
	}
	LOG_ERROR << "Incomplete AssetScript/Mesh/Part data";
	return false;
}

void SGRX_MeshAssetPart::Generate( String& out )
{
	out.append( " PART\n" );
	out.append( "  MESH_NAME " ); out.append( meshName ); out.append( "\n" );
	out.append( "  SHADER " ); out.append( shader ); out.append( "\n" );
	out.append( "  TEXTURE0 " ); out.append( textures[ 0 ] ); out.append( "\n" );
	out.append( "  TEXTURE1 " ); out.append( textures[ 1 ] ); out.append( "\n" );
	out.append( "  TEXTURE2 " ); out.append( textures[ 2 ] ); out.append( "\n" );
	out.append( "  TEXTURE3 " ); out.append( textures[ 3 ] ); out.append( "\n" );
	out.append( "  TEXTURE4 " ); out.append( textures[ 4 ] ); out.append( "\n" );
	out.append( "  TEXTURE5 " ); out.append( textures[ 5 ] ); out.append( "\n" );
	out.append( "  TEXTURE6 " ); out.append( textures[ 6 ] ); out.append( "\n" );
	out.append( "  TEXTURE7 " ); out.append( textures[ 7 ] ); out.append( "\n" );
	char bfr[ 128 ];
	sgrx_snprintf( bfr, 128,
		"  MTL_FLAGS %d\n"
		"  MTL_BLENDMODE %d\n",
		int(mtlFlags),
		int(mtlBlendMode) );
	out.append( bfr );
	out.append( " PART_END\n" );
}

void SGRX_MeshAssetPart::GetDesc( int i, String& out )
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%d: %s", i, StackString<200>(meshName).str );
	out = bfr;
}

void SGRX_MeshAsset::Clone( const SGRX_MeshAsset& other )
{
	*this = other;
	for( size_t i = 0; i < parts.size(); ++i )
	{
		parts[ i ] = new SGRX_MeshAssetPart( *parts[ i ] );
	}
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
		else if( key == "ROTATE_Y2Z" )
			rotateY2Z = String_ParseBool( value );
		else if( key == "FLIP_UVY" )
			flipUVY = String_ParseBool( value );
		else if( key == "PART" )
		{
			SGRX_MeshAPHandle mph = new SGRX_MeshAssetPart;
			if( mph->Parse( cread ) == false )
				return false;
			parts.push_back( mph );
		}
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
	out.append( " ROTATE_Y2Z " ); out.append( rotateY2Z ? "true" : "false" ); out.append( "\n" );
	out.append( " FLIP_UVY " ); out.append( flipUVY ? "true" : "false" ); out.append( "\n" );
	for( size_t i = 0; i < parts.size(); ++i )
		parts[ i ]->Generate( out );
	out.append( "MESH_END\n" );
}

void SGRX_MeshAsset::GetFullName( String& out )
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s/%s",
		StackString<100>(outputCategory).str,
		StackString<100>(outputName).str );
	out = bfr;
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

enum EAssetType
{
	AT_Unknown = 0,
	AT_Texture,
	AT_Mesh,
};

struct AssetInfoItem
{
	uint8_t asset_type;
	String category;
	String name;
	uint32_t rev_asset;
	
	template< class T > void Serialize( T& arch )
	{
		arch << asset_type;
		arch << category;
		arch << name;
		arch << rev_asset;
	}
};

struct AssetOutputItem
{
	uint8_t asset_type;
	String category;
	String name;
	uint32_t ts_source;
	uint32_t ts_output;
	uint32_t rev_output;
	
	template< class T > void Serialize( T& arch )
	{
		arch << asset_type;
		arch << category;
		arch << name;
		arch << ts_source;
		arch << ts_output;
		arch << rev_output;
	}
};

bool SGRX_AssetScript::LoadAssetInfo( const StringView& path )
{
	String data;
	if( FS_LoadTextFile( path, data ) == false )
		return false;
	
	TextReader tr( &data );
	tr.marker( "AS_ASSET_INFO" );
	Array< AssetInfoItem > items;
	tr << items;
	for( size_t i = 0; i < items.size(); ++i )
	{
		const AssetInfoItem& item = items[ i ];
		switch( item.asset_type )
		{
		case AT_Texture:
			for( size_t j = 0; j < textureAssets.size(); ++j )
			{
				SGRX_TextureAsset& TA = textureAssets[ j ];
				if( TA.outputCategory != item.category ||
					TA.outputName != item.name )
					continue;
				TA.ri.rev_asset = item.rev_asset;
			}
			break;
		case AT_Mesh:
			for( size_t j = 0; j < meshAssets.size(); ++j )
			{
				SGRX_MeshAsset& MA = meshAssets[ j ];
				if( MA.outputCategory != item.category ||
					MA.outputName != item.name )
					continue;
				MA.ri.rev_asset = item.rev_asset;
			}
			break;
		}
	}
	
	return tr.error == false;
}

bool SGRX_AssetScript::SaveAssetInfo( const StringView& path )
{
	String data;
	TextWriter tw( &data );
	tw.marker( "AS_ASSET_INFO" );
	Array< AssetInfoItem > items;
	for( size_t j = 0; j < textureAssets.size(); ++j )
	{
		SGRX_TextureAsset& TA = textureAssets[ j ];
		AssetInfoItem item = { AT_Texture, TA.outputCategory, TA.outputName, TA.ri.rev_asset };
		items.push_back( item );
	}
	for( size_t j = 0; j < meshAssets.size(); ++j )
	{
		SGRX_MeshAsset& MA = meshAssets[ j ];
		AssetInfoItem item = { AT_Mesh, MA.outputCategory, MA.outputName, MA.ri.rev_asset };
		items.push_back( item );
	}
	tw << items;
	
	return FS_SaveTextFile( path, data );
}

bool SGRX_AssetScript::LoadOutputInfo( const StringView& path )
{
	String data;
	if( FS_LoadTextFile( path, data ) == false )
		return false;
	
	TextReader tr( &data );
	tr.marker( "AS_OUTPUT_INFO" );
	Array< AssetOutputItem > items;
	tr << items;
	for( size_t i = 0; i < items.size(); ++i )
	{
		const AssetOutputItem& item = items[ i ];
		switch( item.asset_type )
		{
		case AT_Texture:
			for( size_t j = 0; j < textureAssets.size(); ++j )
			{
				SGRX_TextureAsset& TA = textureAssets[ j ];
				if( TA.outputCategory != item.category ||
					TA.outputName != item.name )
					continue;
				TA.ri.ts_source = item.ts_source;
				TA.ri.ts_output = item.ts_output;
				TA.ri.rev_output = item.rev_output;
			}
			break;
		case AT_Mesh:
			for( size_t j = 0; j < meshAssets.size(); ++j )
			{
				SGRX_MeshAsset& MA = meshAssets[ j ];
				if( MA.outputCategory != item.category ||
					MA.outputName != item.name )
					continue;
				MA.ri.ts_source = item.ts_source;
				MA.ri.ts_output = item.ts_output;
				MA.ri.rev_output = item.rev_output;
			}
			break;
		}
	}
	
	return tr.error == false;
}

bool SGRX_AssetScript::SaveOutputInfo( const StringView& path )
{
	String data;
	TextWriter tw( &data );
	tw.marker( "AS_OUTPUT_INFO" );
	Array< AssetOutputItem > items;
	for( size_t j = 0; j < textureAssets.size(); ++j )
	{
		SGRX_TextureAsset& TA = textureAssets[ j ];
		AssetOutputItem item = { AT_Texture, TA.outputCategory, TA.outputName,
			TA.ri.ts_source, TA.ri.ts_output, TA.ri.rev_output };
		items.push_back( item );
	}
	for( size_t j = 0; j < meshAssets.size(); ++j )
	{
		SGRX_MeshAsset& MA = meshAssets[ j ];
		AssetOutputItem item = { AT_Mesh, MA.outputCategory, MA.outputName,
			MA.ri.ts_source, MA.ri.ts_output, MA.ri.rev_output };
		items.push_back( item );
	}
	tw << items;
	
	return FS_SaveTextFile( path, data );
}



SGRX_Scene3D::SGRX_Scene3D( const StringView& path ) : m_imp(NULL), m_scene(NULL)
{
	ByteArray data;
	if( FS_LoadBinaryFile( path, data ) == false )
	{
		printf( "Could not load 3D scene file: %s\n", StackString<1024>(path).str );
		return;
	}
	
	m_imp = new Assimp::Importer;
	int flags =
		aiProcess_CalcTangentSpace |
	//	aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_LimitBoneWeights |
		aiProcess_FlipWindingOrder;
	m_scene = m_imp->ReadFileFromMemory( data.data(), data.size(),
		flags, StackString<32>(path.after_all(".")) );
	if( m_scene == NULL )
	{
		delete m_imp;
		m_imp = NULL;
		return;
	}
}

SGRX_Scene3D::~SGRX_Scene3D()
{
	if( m_imp )
	{
		delete m_imp;
		m_imp = NULL;
	}
	m_scene = NULL;
}

static void aiNode_GetMeshList( StringView path, const aiScene* S, aiNode* N, Array< String >& out )
{
	String subpath = path;
	subpath.append( "/" );
	subpath.append( N->mName.C_Str() );
	
	String meshpath = subpath;
	for( unsigned i = 0; i < N->mNumMeshes; ++i )
	{
		unsigned mid = N->mMeshes[ i ];
		aiMesh* M = S->mMeshes[ mid ];
		meshpath = subpath;
		meshpath.append( "|" );
		meshpath.append( M->mName.C_Str() );
		char bfr[ 32 ];
		sgrx_snprintf( bfr, 32, "[%d]", mid );
		meshpath.append( bfr );
		out.push_back( meshpath );
	}
	
	for( unsigned i = 0; i < N->mNumChildren; ++i )
	{
		aiNode_GetMeshList( subpath, S, N->mChildren[ i ], out );
	}
}

void SGRX_Scene3D::GetMeshList( Array< String >& out )
{
	if( m_scene == NULL )
		return;
	aiNode_GetMeshList( "", m_scene, m_scene->mRootNode, out );
//	for( unsigned i = 0; i < m_scene->mNumMeshes; ++i )
//	{
//		out.push_back( m_scene->mMeshes[ i ]->mName.C_Str() );
//	}
	
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
	ByteArray filedata;
	ByteArray imagedata;
	
	switch( TA.outputType )
	{
	case SGRX_TOF_PNG_RGBA32:
		SGRX_ImageF32ToRGBA8( image, imagedata );
		if( dumpimg( filedata, imagedata.data(), image->GetWidth(), image->GetHeight() ) )
		{
			printf( "ERROR: failed to encode PNG\n" );
			return false;
		}
		break;
	case SGRX_TOF_STX_RGBA32:
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
	
	if( FS_SaveBinaryFile( path, filedata.data(), filedata.size() ) == false )
	{
		printf( "ERROR: failed to save the texture file: %s\n", StackString<256>(path).str );
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

struct MeshAssetInputs
{
	bool pos;
	bool nrm;
	bool tng;
	bool col;
	bool tx0;
	bool tx1;
	bool tx2;
	bool tx3;
	// additional flags
	bool y2z;
	bool flip_uvy;
};

static size_t _InsertVertex( ByteArray& out,
	const MeshAssetInputs& fmt, aiMesh* mesh, unsigned idx, size_t from )
{
	uint8_t bfr[ 256 ];
	uint8_t* p = bfr;
	
	if( fmt.pos )
	{
		aiVector3D v = mesh->mVertices ? mesh->mVertices[ idx ] : aiVector3D(0);
		Vec3 ov = { v.x, v.y, v.z };
		if( fmt.y2z )
			ov = V3( ov.x, -ov.z, ov.y );
		memcpy( p, &ov, sizeof(ov) );
		p += sizeof(ov);
	}
	
	if( fmt.nrm )
	{
		aiVector3D v = mesh->mNormals ? mesh->mNormals[ idx ] : aiVector3D(0);
		Vec3 ov = { v.x, v.y, v.z };
		if( fmt.y2z )
			ov = V3( ov.x, -ov.z, ov.y );
		memcpy( p, &ov, sizeof(ov) );
		p += sizeof(ov);
	}
	
	if( fmt.tng )
	{
		Vec4 ov = { 0, 0, 1, 1 };
		if( mesh->mNormals && mesh->mTangents && mesh->mBitangents )
		{
			aiVector3D n = mesh->mNormals[ idx ];
			aiVector3D t = mesh->mTangents[ idx ];
			aiVector3D b = mesh->mBitangents[ idx ];
			Vec3 vn = { n.x, n.y, n.z };
			Vec3 vt = { t.x, t.y, t.z };
			Vec3 vb = { b.x, b.y, b.z };
			Vec3 tt = vt;
			if( fmt.flip_uvy )
				vb = -vb;
			if( fmt.y2z )
				tt = V3( tt.x, -tt.z, tt.y );
			ov = V4( tt, sign( Vec3Dot( Vec3Cross( vn, vt ), vb ) ) );
		}
		memcpy( p, &ov, sizeof(ov) );
		p += sizeof(ov);
	}
	
	if( fmt.col )
	{
		aiColor4D v = mesh->mColors[0] ? mesh->mColors[0][ idx ] : aiColor4D(1);
		Vec4 oc = { v.r, v.g, v.b, v.a };
		uint32_t ov = Vec4ToCol32( oc );
		memcpy( p, &ov, sizeof(ov) );
		p += sizeof(ov);
	}
	
	if( fmt.tx0 )
	{
		aiVector3D v = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][ idx ] : aiVector3D(0);
		Vec2 ov = { v.x, v.y };
		if( fmt.flip_uvy )
			ov.y = 1 - ov.y;
		memcpy( p, &ov, sizeof(ov) );
		p += sizeof(ov);
	}
	
	if( fmt.tx1 )
	{
		aiVector3D v = mesh->mTextureCoords[1] ? mesh->mTextureCoords[1][ idx ] : aiVector3D(0);
		Vec2 ov = { v.x, v.y };
		if( fmt.flip_uvy )
			ov.y = 1 - ov.y;
		memcpy( p, &ov, sizeof(ov) );
		p += sizeof(ov);
	}
	
	if( fmt.tx2 )
	{
		aiVector3D v = mesh->mTextureCoords[2] ? mesh->mTextureCoords[2][ idx ] : aiVector3D(0);
		Vec2 ov = { v.x, v.y };
		if( fmt.flip_uvy )
			ov.y = 1 - ov.y;
		memcpy( p, &ov, sizeof(ov) );
		p += sizeof(ov);
	}
	
	if( fmt.tx3 )
	{
		aiVector3D v = mesh->mTextureCoords[3] ? mesh->mTextureCoords[3][ idx ] : aiVector3D(0);
		Vec2 ov = { v.x, v.y };
		if( fmt.flip_uvy )
			ov.y = 1 - ov.y;
		memcpy( p, &ov, sizeof(ov) );
		p += sizeof(ov);
	}
	
	return out.find_or_add_bytes( bfr, p - bfr, from );
}

static String SGRX_TexIDToPath( const SGRX_AssetScript* AS, const StringView& texid )
{
	StringView cat = texid.until( "/" );
	StringView name = texid.after( "/" );
	const String* catpath = AS->categories.getptr( cat );
	if( catpath == NULL )
		return "";
	
	const SGRX_TextureAsset* TA = NULL;
	for( size_t i = 0; i < AS->textureAssets.size(); ++i )
	{
		if( AS->textureAssets[ i ].outputCategory == cat &&
			AS->textureAssets[ i ].outputName == name )
		{
			TA = &AS->textureAssets[ i ];
			break;
		}
	}
	if( TA == NULL )
		return "";
	
	String out = *catpath;
	out.append( "/" );
	out.append( name );
	out.append( "." );
	out.append( SGRX_TextureOutputFormat_Ext( TA->outputType ) );
	return out;
}

MeshHandle SGRX_ProcessMeshAsset( const SGRX_AssetScript* AS, const SGRX_MeshAsset& MA )
{
	if( MA.parts.size() < 1 || MA.parts.size() > 16 )
	{
		puts( "Mesh part count not in range [1;16]" );
		return NULL;
	}
	
	printf( "| %s => [%s] %s\n",
		StackString<256>(MA.sourceFile).str,
		StackString<256>(MA.outputCategory).str,
		StackString<256>(MA.outputName).str );
	
	ImpScene3DHandle scene = new SGRX_Scene3D( MA.sourceFile );
	if( scene->m_scene == NULL )
	{
		printf( "Failed to load %s\n", StackString<256>(MA.sourceFile).str );
		return NULL;
	}
	const aiScene* S = scene->m_scene;
	
	// enumerate mesh data
	aiMesh* part_ptrs[ 16 ] = {0};
	MeshAssetInputs fmt = {0};
	fmt.y2z = MA.rotateY2Z;
	fmt.flip_uvy = MA.flipUVY;
	int numparts = MA.parts.size();
	for( int i = 0; i < numparts; ++i )
	{
		SGRX_MeshAssetPart* MP = MA.parts[ i ];
		unsigned meshID = String_ParseInt( StringView(MP->meshName).after_last("[").until("]") );
		printf("MESHID %d | %s | %s | %s\n",meshID,
			StackString<256>(StringView(MP->meshName)).str,
			StackString<256>(StringView(MP->meshName).after_last("[")).str,
			StackString<256>(StringView(MP->meshName).after_last("[").until("]")).str);
		if( meshID >= S->mNumMeshes )
		{
			printf( "Mesh ID %d out of bounds (count=%d)\n", meshID, S->mNumMeshes );
			return NULL;
		}
		aiMesh* mesh = S->mMeshes[ meshID ];
		ASSERT( mesh && "Mesh with a valid ID must have a valid aiMesh*" );
		ASSERT( mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE );
		part_ptrs[ i ] = mesh;
		// update format
		if( mesh->mVertices ) fmt.pos = true;
		if( mesh->mNormals ) fmt.nrm = true;
		if( mesh->mNormals && mesh->mTangents && mesh->mBitangents ) fmt.tng = true;
		if( mesh->mColors[0] ) fmt.col = true;
		if( mesh->mTextureCoords[0] ) fmt.tx0 = true;
		if( mesh->mTextureCoords[1] ) fmt.tx1 = true;
		if( mesh->mTextureCoords[2] ) fmt.tx2 = true;
		if( mesh->mTextureCoords[3] ) fmt.tx3 = true;
	}
	
	// generate vdecl
	char vfmt_bfr[ 64 ];
	size_t vsize = 0;
	size_t isize = 2;
	{
		char* p = vfmt_bfr;
		if( fmt.pos ){ p[0] = 'p'; p[1] = 'f'; p[2] = '3'; p += 3; vsize += 12; }
		if( fmt.nrm ){ p[0] = 'n'; p[1] = 'f'; p[2] = '3'; p += 3; vsize += 12; }
		if( fmt.tng ){ p[0] = 't'; p[1] = 'f'; p[2] = '4'; p += 3; vsize += 16; }
		if( fmt.col ){ p[0] = 'c'; p[1] = 'b'; p[2] = '4'; p += 3; vsize += 4; }
		if( fmt.tx0 ){ p[0] = '0'; p[1] = 'f'; p[2] = '2'; p += 3; vsize += 8; }
		if( fmt.tx1 ){ p[0] = '1'; p[1] = 'f'; p[2] = '2'; p += 3; vsize += 8; }
		if( fmt.tx2 ){ p[0] = '2'; p[1] = 'f'; p[2] = '2'; p += 3; vsize += 8; }
		if( fmt.tx3 ){ p[0] = '3'; p[1] = 'f'; p[2] = '2'; p += 3; vsize += 8; }
		*p = 0;
	}
	VertexDeclHandle vdh = GR_GetVertexDecl( vfmt_bfr );
	if( vdh == NULL )
	{
		puts( "Failed to create vertex decl." );
		return NULL;
	}
	
	// generate vertex/index/part data
	ByteArray vdata;
	ByteArray idata;
	SGRX_MeshPart mparts[ 16 ];
	for( int i = 0; i < numparts; ++i )
	{
		SGRX_MeshAssetPart* MP = MA.parts[ i ];
		aiMesh* mesh = part_ptrs[ i ];
		size_t voff = vdata.size() / vsize;
		size_t ioff = idata.size() / isize;
		for( unsigned fid = 0; fid < mesh->mNumFaces; ++fid )
		{
			aiFace& F = mesh->mFaces[ fid ];
			ASSERT( F.mNumIndices == 3 );
			
			unsigned idx0 = F.mIndices[0];
			unsigned idx1 = F.mIndices[1];
			unsigned idx2 = F.mIndices[2];
			
			size_t t0 = _InsertVertex( vdata, fmt, mesh, idx0, voff );
			size_t t1 = _InsertVertex( vdata, fmt, mesh, idx1, voff );
			size_t t2 = _InsertVertex( vdata, fmt, mesh, idx2, voff );
			
			uint16_t idcs[3] = { t0 - voff, t1 - voff, t2 - voff };
			idata.append( &idcs, sizeof(idcs) );
		}
		
		SGRX_MeshPart& outMP = mparts[ i ];
		outMP.vertexOffset = voff;
		outMP.vertexCount = vdata.size() / vsize - voff;
		outMP.indexOffset = ioff;
		outMP.indexCount = idata.size() / isize - ioff;
		outMP.shader = MP->shader;
		for( int t = 0; t < 8; ++t )
			outMP.textures[ t ] = SGRX_TexIDToPath( AS, MP->textures[ t ] );
		outMP.mtlFlags = MP->mtlFlags;
		outMP.mtlBlendMode = MP->mtlBlendMode;
		
		printf( "| part %d: mesh=%p src-vc=%d src-fc=%d dst-vc=%d dst-ic=%d shader=%s\n",
			i, mesh, mesh->mNumVertices, mesh->mNumFaces,
			int(outMP.vertexCount), int(outMP.indexCount),
			StackString<256>(outMP.shader).str );
	}
	
	MeshHandle dstMesh = GR_CreateMesh();
	dstMesh->SetVertexData( vdata.data(), vdata.size(), vdh, false );
	dstMesh->SetAABBFromVertexData( vdata.data(), vdata.size(), vdh );
	dstMesh->SetIndexData( idata.data(), idata.size(), false );
	dstMesh->SetPartData( mparts, numparts );
	dstMesh->m_vdata = vdata;
	dstMesh->m_idata = idata;
	dstMesh->m_dataFlags = MDF_MTLINFO;
	
	return dstMesh;
}

void SGRX_ProcessAssets( SGRX_AssetScript& script, bool force )
{
	puts( "processing assets...");
	
	puts( "- category folders...");
	for( size_t cid = 0; cid < script.categories.size(); ++cid )
	{
		StringView dir = script.categories.item( cid ).value;
		size_t slashpos = dir.find_first_at( "/" );
		while( slashpos != NOT_FOUND )
		{
			FS_DirCreate( dir.part( 0, slashpos ) );
			slashpos = dir.find_first_at( "/", slashpos + 1 );
		}
		FS_DirCreate( dir );
	}
	
	puts( "- textures...");
	for( size_t tid = 0; tid < script.textureAssets.size(); ++tid )
	{
		SGRX_TextureAsset& TA = script.textureAssets[ tid ];
		StringView catPath = script.categories.getcopy( TA.outputCategory );
		char bfr[ 520 ];
		sgrx_snprintf( bfr, 520, "%s/%s.%s",
			StackString<256>(catPath).str,
			StackString<256>(TA.outputName).str,
			SGRX_TextureOutputFormat_Ext( TA.outputType ) );
		if( force == false &&
			TA.ri.rev_output == TA.ri.rev_asset &&
			TA.ri.ts_source != 0 &&
			TA.ri.ts_output != 0 &&
			TA.ri.ts_source == FS_FileModTime( TA.sourceFile ) &&
			TA.ri.ts_output == FS_FileModTime( bfr ) )
			continue;
		
		SGRX_IFP32Handle image = SGRX_ProcessTextureAsset( TA );
		if( image == NULL )
			continue;
		
		if( SGRX_SaveImage( bfr, image, TA ) )
		{
			TA.ri.rev_output = TA.ri.rev_asset;
			TA.ri.ts_source = FS_FileModTime( TA.sourceFile );
			TA.ri.ts_output = FS_FileModTime( bfr );
			printf( "|----------- saved!\n" );
		}
	}
	
	puts( "- meshes...");
	for( size_t tid = 0; tid < script.meshAssets.size(); ++tid )
	{
		SGRX_MeshAsset& MA = script.meshAssets[ tid ];
		StringView catPath = script.categories.getcopy( MA.outputCategory );
		char bfr[ 520 ];
		sgrx_snprintf( bfr, 520, "%s/%s.ssm",
			StackString<256>(catPath).str,
			StackString<256>(MA.outputName).str );
		if( force == false &&
			MA.ri.rev_output == MA.ri.rev_asset &&
			MA.ri.ts_source != 0 &&
			MA.ri.ts_output != 0 &&
			MA.ri.ts_source == FS_FileModTime( MA.sourceFile ) &&
			MA.ri.ts_output == FS_FileModTime( bfr ) )
			continue;
		
		MeshHandle mesh = SGRX_ProcessMeshAsset( &script, MA );
		if( mesh == NULL )
			continue;
		
		ByteArray data;
		if( mesh->ToMeshData( data ) == false )
		{
			puts( "ERROR: failed to serialize mesh" );
			continue;
		}
		
		if( FS_SaveBinaryFile( bfr, data.data(), data.size() ) )
		{
			MA.ri.rev_output = MA.ri.rev_asset;
			MA.ri.ts_source = FS_FileModTime( MA.sourceFile );
			MA.ri.ts_output = FS_FileModTime( bfr );
			printf( "|----------- saved!\n" );
		}
		else
		{
			printf( "ERROR: failed to save file to %s\n", bfr );
		}
	}
}

