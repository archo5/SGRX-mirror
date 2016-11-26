

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
	
	// texture formats
	{ "TEXFF_BLOCKFMT", TEXFF_BLOCKFMT },
	{ "TEXFF_RTFMT", TEXFF_RTFMT },
	{ "TEXFF_RTCOLFMT", TEXFF_RTCOLFMT },
	{ "TEXFF_RTDEPTHFMT", TEXFF_RTDEPTHFMT },
	{ "TEXFMT_UNKNOWN", TEXFMT_UNKNOWN },
	{ "TEXFMT_RGBA8", TEXFMT_RGBA8 },
	{ "TEXFMT_BGRA8", TEXFMT_BGRA8 },
	{ "TEXFMT_BGRX8", TEXFMT_BGRX8 },
	{ "TEXFMT_R5G6B5", TEXFMT_R5G6B5 },
	{ "TEXFMT_DXT1", TEXFMT_DXT1 },
	{ "TEXFMT_DXT3", TEXFMT_DXT3 },
	{ "TEXFMT_DXT5", TEXFMT_DXT5 },
	{ "TEXFMT_3DC", TEXFMT_3DC },
	{ "TEXFMT_RT_COLOR_HDR16", TEXFMT_RT_COLOR_HDR16 },
	{ "TEXFMT_RT_COLOR_LDR8", TEXFMT_RT_COLOR_LDR8 },
	{ "TEXFMT_RT_DEPTH_F32", TEXFMT_RT_DEPTH_F32 },
	{ "TEXFMT_RT_DEPTH_D24S8", TEXFMT_RT_DEPTH_D24S8 },
	
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
	
	// matrix modes
	{ "MM_Relative", MM_Relative },
	{ "MM_Absolute", MM_Absolute },
	
	// resource move masks
	{ "MoveMask_Position", MoveMask_Position },
	{ "MoveMask_Rotation", MoveMask_Rotation },
	{ "MoveMask_Scale", MoveMask_Scale },
	{ "MoveMask_ALL", MoveMask_ALL },
	
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

