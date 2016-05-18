

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


static int sgsGR_GetMesh( SGS_CTX )
{
	SGSFN( "GR_GetMesh" );
	if( !sgs_LoadArgs( C, "?s" ) )
		return 0;
	MeshHandle h = GR_GetMesh( sgs_GetVar<StringView>()( C, 0 ) );
	if( !h )
		return 0;
	SGS_CREATECLASS( C, NULL, SGSMeshHandle, ( h ) );
	return 1;
}


static sgs_RegIntConst g_gfw_ints[] =
{
	// texture types
	{ "TEXTYPE_2D", TEXTYPE_2D },
	{ "TEXTYPE_CUBE", TEXTYPE_CUBE },
	{ "TEXTYPE_VOLUME", TEXTYPE_VOLUME },
	
	// basic texture formats
	{ "TEXFORMAT_UNKNOWN", TEXFORMAT_UNKNOWN },
	{ "TEXFORMAT_RGBA8", TEXFORMAT_RGBA8 },
	{ "TEXFORMAT_BGRA8", TEXFORMAT_BGRA8 },
	{ "TEXFORMAT_BGRX8", TEXFORMAT_BGRX8 },
	{ "TEXFORMAT_R5G6B5", TEXFORMAT_R5G6B5 },
	{ "TEXFORMAT_DXT1", TEXFORMAT_DXT1 },
	{ "TEXFORMAT_DXT3", TEXFORMAT_DXT3 },
	{ "TEXFORMAT_DXT5", TEXFORMAT_DXT5 },
	
	// RT texture formats
	{ "RT_FORMAT_DEPTH", RT_FORMAT_DEPTH },
	{ "RT_FORMAT_COLOR_HDR16", RT_FORMAT_COLOR_HDR16 },
	{ "RT_FORMAT_COLOR_LDR8", RT_FORMAT_COLOR_LDR8 },
	{ "RT_FORMAT_USE_MSAA", RT_FORMAT_USE_MSAA },
	
	{ "MDF_INDEX_32", MDF_INDEX_32 },
	{ "MDF_DYNAMIC", MDF_DYNAMIC },
	{ "MDF_SKINNED", MDF_SKINNED },
	{ "MDF_MTLINFO", MDF_MTLINFO },
	{ "MDF_PARTNAMES", MDF_PARTNAMES },
	
	// light types
	{ "LIGHT_POINT", LIGHT_POINT },
	{ "LIGHT_SPOT", LIGHT_SPOT },
	{ "LIGHT_PROJ", LIGHT_PROJ },
	
	// lighting modes
	{ "LM_Unlit", SGRX_LM_Unlit },
	{ "LM_Static", SGRX_LM_Static },
	{ "LM_Dynamic", SGRX_LM_Dynamic },
	{ "LM_Decal", SGRX_LM_Decal },
	
	// physics force types
	{ "ForceType_Velocity", ForceType_Velocity },
	{ "ForceType_Impulse", ForceType_Impulse },
	{ "ForceType_Acceleration", ForceType_Acceleration },
	{ "ForceType_Force", ForceType_Force },
	
	{ NULL, 0 },
};

static sgs_RegFuncConst g_gfw_funcs[] =
{
	{ "GR_GetTexture", sgsGR_GetTexture },
	{ "GR_GetMesh", sgsGR_GetMesh },
	{ NULL, NULL },
};

void GFWRegisterCore( SGS_CTX )
{
	sgs_RegFuncConsts( C, g_gfw_funcs, -1 );
	sgs_RegIntConsts( C, g_gfw_ints, -1 );
}

