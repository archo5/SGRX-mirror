

#include "gfwcore.hpp"


static int sgsGR_GetTexture( SGS_CTX )
{
	SGSFN( "GR_GetTexture" );
	if( !sgs_LoadArgs( C, "?s" ) )
		return 0;
	TextureHandle h = GR_GetTexture( sgs_GetVar<StringView>()( C, 0 ) );
	if( !h )
		return 0;
	SGS_CREATECLASS( C, NULL, SGSTextureHandle, ( h ) );
	return 1;
}


static sgs_RegIntConst g_gfw_ints[] =
{
	{ "TEXTYPE_2D", TEXTYPE_2D },
	{ "TEXTYPE_CUBE", TEXTYPE_CUBE },
	{ "TEXTYPE_VOLUME", TEXTYPE_VOLUME },
	
	{ "TEXFORMAT_UNKNOWN", TEXFORMAT_UNKNOWN },
	{ "TEXFORMAT_RGBA8", TEXFORMAT_RGBA8 },
	{ "TEXFORMAT_BGRA8", TEXFORMAT_BGRA8 },
	{ "TEXFORMAT_BGRX8", TEXFORMAT_BGRX8 },
	{ "TEXFORMAT_R5G6B5", TEXFORMAT_R5G6B5 },
	{ "TEXFORMAT_DXT1", TEXFORMAT_DXT1 },
	{ "TEXFORMAT_DXT3", TEXFORMAT_DXT3 },
	{ "TEXFORMAT_DXT5", TEXFORMAT_DXT5 },
	
	{ "RT_FORMAT_DEPTH", RT_FORMAT_DEPTH },
	{ "RT_FORMAT_COLOR_HDR16", RT_FORMAT_COLOR_HDR16 },
	{ "RT_FORMAT_COLOR_LDR8", RT_FORMAT_COLOR_LDR8 },
	{ "RT_FORMAT_USE_MSAA", RT_FORMAT_USE_MSAA },
	
	{ "LIGHT_POINT", LIGHT_POINT },
	{ "LIGHT_SPOT", LIGHT_SPOT },
	{ "LIGHT_PROJ", LIGHT_PROJ },
	{ NULL, 0 },
};

static sgs_RegFuncConst g_gfw_funcs[] =
{
	{ "GR_GetTexture", sgsGR_GetTexture },
	{ NULL, NULL },
};

void GFWRegisterCore( SGS_CTX )
{
	sgs_RegFuncConsts( C, g_gfw_funcs, -1 );
	sgs_RegIntConsts( C, g_gfw_ints, -1 );
}

