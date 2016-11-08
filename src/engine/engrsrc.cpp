

#include "engine_int.hpp"
#include "enganim.hpp"
#include "engext.hpp"
#include "sound.hpp"


typedef HashTable< StringView, SGRX_ConvexPointSet* > ConvexPointSetHashTable;
typedef HashTable< StringView, SGRX_ITexture* > TextureHashTable;
typedef HashTable< uint64_t, SGRX_ITexture* > RenderTargetTable;
typedef HashTable< uint64_t, SGRX_IDepthStencilSurface* > DepthStencilSurfTable;
typedef HashTable< StringView, SGRX_IVertexShader* > VertexShaderHashTable;
typedef HashTable< StringView, SGRX_IPixelShader* > PixelShaderHashTable;
typedef HashTable< SGRX_RenderState, SGRX_IRenderState* > RenderStateHashTable;
typedef HashTable< StringView, SGRX_IVertexDecl* > VertexDeclHashTable;
typedef HashTable< SGRX_VtxInputMapKey, SGRX_IVertexInputMapping* > VtxInputMapHashTable;
typedef HashTable< StringView, SGRX_IMesh* > MeshHashTable;
typedef HashTable< StringView, AnimHandle > AnimHashTable;
typedef HashTable< StringView, AnimCharHandle > AnimCharHashTable;
typedef HashTable< StringView, FontHandle > FontHashTable;
typedef HashTable< GenericHandle, int > ResourcePreserveHashTable;

static ConvexPointSetHashTable* g_CPSets = NULL;
static TextureHashTable* g_Textures = NULL;
static RenderTargetTable* g_RenderTargets = NULL;
static DepthStencilSurfTable* g_DepthStencilSurfs = NULL;
static VertexShaderHashTable* g_VertexShaders = NULL;
static PixelShaderHashTable* g_PixelShaders = NULL;
static RenderStateHashTable* g_RenderStates = NULL;
static VertexDeclHashTable* g_VertexDecls = NULL;
static VtxInputMapHashTable* g_VtxInputMaps = NULL;
static MeshHashTable* g_Meshes = NULL;
static AnimHashTable* g_Anims = NULL;
static AnimCharHashTable* g_AnimChars = NULL;
static FontHashTable* g_LoadedFonts = NULL;
static ResourcePreserveHashTable* g_PreservedResources = NULL;

extern IGame* g_Game;
extern IRenderer* g_Renderer;
extern bool g_VerboseLogging;
#define VERBOSE g_VerboseLogging



struct FakeSoundEventInstance : SGRX_ISoundEventInstance
{
	FakeSoundEventInstance( bool oneshot ) :
		m_paused(false), m_volume(1), m_pitch(1)
	{
		isOneShot = oneshot;
		isReal = false;
	}
	void Start(){ m_paused = false; }
	void Stop( bool immediate = false ){}
	bool GetPaused(){ return m_paused; }
	void SetPaused( bool paused ){ m_paused = paused; }
	float GetVolume(){ return m_volume; }
	void SetVolume( float v ){ m_volume = v; }
	float GetPitch(){ return m_pitch; }
	void SetPitch( float v ){ m_pitch = v; }
	bool SetParameter( const StringView& name, float value ){ return false; }
	void Set3DAttribs( const SGRX_Sound3DAttribs& attribs ){}
	
	bool m_paused;
	float m_volume;
	float m_pitch;
};

SoundEventInstanceHandle SGRX_ISoundSystem::CreateEventInstance( const StringView& name )
{
	SoundEventInstanceHandle seih = CreateEventInstanceRaw( name );
	if( seih != NULL )
		return seih;
	return new FakeSoundEventInstance( true );
}



SGRX_ConvexPointSet::~SGRX_ConvexPointSet()
{
	if( VERBOSE ) LOG << "Deleted convex point set: " << m_key;
	g_CPSets->unset( m_key );
}

SGRX_ConvexPointSet* SGRX_ConvexPointSet::Create( const StringView& path )
{
	ByteArray data;
	if( FS_LoadBinaryFile( path, data ) == false )
	{
		LOG_ERROR << LOG_DATE << "  Convex point set file not found: " << path;
		return NULL;
	}
	SGRX_ConvexPointSet* cps = new SGRX_ConvexPointSet;
	ByteReader br( data );
	cps->Serialize( br );
	if( cps->data.points.size() == 0 )
	{
		LOG_ERROR << LOG_DATE << "  Convex point set invalid or empty: " << path;
		delete cps;
	}
	return cps;
}

ConvexPointSetHandle GP_GetConvexPointSet( const StringView& path )
{
	LOG_FUNCTION;
	
	SGRX_ConvexPointSet* cps = g_CPSets->getcopy( path );
	if( cps )
		return cps;
	
	cps = SGRX_ConvexPointSet::Create( path );
	if( cps == NULL )
	{
		// error already printed
		return ConvexPointSetHandle();
	}
	cps->m_key = path;
	g_CPSets->set( cps->m_key, cps );
	return cps;
}


void RenderStats::Reset()
{
	numVisMeshes = 0;
	numVisPLights = 0;
	numVisSLights = 0;
	numDrawCalls = 0;
	numSDrawCalls = 0;
	numMDrawCalls = 0;
	numPDrawCalls = 0;
}


SGRX_ITexture::SGRX_ITexture() : m_rtkey(0)
{
}

SGRX_ITexture::~SGRX_ITexture()
{
	if( VERBOSE ) LOG << "Deleted texture: " << m_key;
	g_Textures->unset( m_key );
	if( m_rtkey )
		g_RenderTargets->unset( m_rtkey );
}

const TextureInfo& TextureHandle::GetInfo() const
{
	static TextureInfo dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}

Vec2 TextureHandle::GetInvSize( Vec2 def ) const
{
	const TextureInfo& TI = GetInfo();
	return V2( TI.width ? 1.0f / TI.width : def.x, TI.height ? 1.0f / TI.height : def.y );
}

bool TextureHandle::UploadRGBA8Part( void* data, int mip, int w, int h, int x, int y )
{
	LOG_FUNCTION;
	
	if( !item )
		return false;
	
	const TextureInfo& TI = item->m_info;
	
	if( mip < 0 || mip >= TI.mipcount )
	{
		LOG_ERROR << "Cannot UploadRGBA8Part - mip count out of bounds (" << mip << "/" << TI.mipcount << ")";
		return false;
	}
	
	TextureInfo mti;
	if( !TextureInfo_GetMipInfo( &TI, mip, &mti ) )
	{
		LOG_ERROR << "Cannot UploadRGBA8Part - failed to get mip info (" << mip << ")";
		return false;
	}
	
	if( w < 0 ) w = mti.width;
	if( h < 0 ) h = mti.height;
	
	return item->UploadRGBA8Part( data, mip, x, y, w, h );
}

bool TextureHandle::UploadRGBA8Part3D( void* data, int mip, int w, int h, int d, int x, int y, int z )
{
	LOG_FUNCTION;
	
	if( !item )
		return false;
	
	const TextureInfo& TI = item->m_info;
	
	if( mip < 0 || mip >= TI.mipcount )
	{
		LOG_ERROR << "Cannot UploadRGBA8Part - mip count out of bounds (" << mip << "/" << TI.mipcount << ")";
		return false;
	}
	
	TextureInfo mti;
	if( !TextureInfo_GetMipInfo( &TI, mip, &mti ) )
	{
		LOG_ERROR << "Cannot UploadRGBA8Part - failed to get mip info (" << mip << ")";
		return false;
	}
	
	if( w < 0 ) w = mti.width;
	if( h < 0 ) h = mti.height;
	if( d < 0 ) d = mti.depth;
	
	return item->UploadRGBA8Part3D( data, mip, x, y, z, w, h, d );
}


SGRX_IDepthStencilSurface::SGRX_IDepthStencilSurface() : m_width(0), m_height(0), m_format(0), m_key(0)
{
}

SGRX_IDepthStencilSurface::~SGRX_IDepthStencilSurface()
{
	if( VERBOSE ) LOG << "Deleted depth/stencil surface: " << m_width << "x" << m_height << ", format=" << m_format;
	if( m_key )
		g_DepthStencilSurfs->unset( m_key );
}


SGRX_IVertexShader::~SGRX_IVertexShader()
{
	g_VertexShaders->unset( m_key );
}

SGRX_IPixelShader::~SGRX_IPixelShader()
{
	g_PixelShaders->unset( m_key );
}


SGRX_IRenderState::~SGRX_IRenderState()
{
	g_RenderStates->unset( m_info );
}

void SGRX_IRenderState::SetState( const SGRX_RenderState& state )
{
	m_info = state;
}

const SGRX_RenderState& RenderStateHandle::GetInfo()
{
	static SGRX_RenderState dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}


void VDeclInfo::TransformVertices( Mat4 xf, void* data, size_t vtxCount ) const
{
	SGRX_CAST( uint8_t*, bytes, data );
	for( size_t i = 0; i < vtxCount; ++i )
	{
		for( uint8_t e = 0; e < count; ++e )
		{
			if( usages[ e ] != VDECLUSAGE_POSITION &&
				usages[ e ] != VDECLUSAGE_NORMAL &&
				usages[ e ] != VDECLUSAGE_TANGENT )
				continue;
			if( types[ e ] == VDECLTYPE_BCOL4 )
				continue;
			
			uint8_t type = types[ e ];
			if( usages[ e ] == VDECLUSAGE_TANGENT )
				type = VDECLTYPE_FLOAT3;
			Vec4 value = { 0, 0, 0, 1 };
			switch( type )
			{
			case VDECLTYPE_FLOAT4:
				value.w = ((float*)(bytes + offsets[ e ]))[3];
			case VDECLTYPE_FLOAT3:
				value.z = ((float*)(bytes + offsets[ e ]))[2];
			case VDECLTYPE_FLOAT2:
				value.y = ((float*)(bytes + offsets[ e ]))[1];
			case VDECLTYPE_FLOAT1:
				value.x = ((float*)(bytes + offsets[ e ]))[0];
			}
			
			if( usages[ e ] != VDECLUSAGE_POSITION )
				value.w = 0;
			value = xf.Transform( value );
			if( usages[ e ] == VDECLUSAGE_POSITION && value.w != 0 )
				value /= value.w;
			
			switch( type )
			{
			case VDECLTYPE_FLOAT4:
				((float*)(bytes + offsets[ e ]))[3] = value.w;
			case VDECLTYPE_FLOAT3:
				((float*)(bytes + offsets[ e ]))[2] = value.z;
			case VDECLTYPE_FLOAT2:
				((float*)(bytes + offsets[ e ]))[1] = value.y;
			case VDECLTYPE_FLOAT1:
				((float*)(bytes + offsets[ e ]))[0] = value.x;
			}
		}
		bytes += size;
	}
}

void VDeclInfo::TransformTexcoords( Vec4 mul, Vec4 add, void* data, size_t vtxCount ) const
{
	SGRX_CAST( uint8_t*, bytes, data );
	for( size_t i = 0; i < vtxCount; ++i )
	{
		for( uint8_t e = 0; e < count; ++e )
		{
			if( usages[ e ] != VDECLUSAGE_TEXTURE0 &&
				usages[ e ] != VDECLUSAGE_TEXTURE1 &&
				usages[ e ] != VDECLUSAGE_TEXTURE2 &&
				usages[ e ] != VDECLUSAGE_TEXTURE3 )
				continue;
			
			Vec4 value = { 0, 0, 0, 0 };
			switch( types[ e ] )
			{
			case VDECLTYPE_BCOL4:
				value = Col32ToVec4( *(uint32_t*)(bytes + offsets[ e ]) );
				break;
			case VDECLTYPE_FLOAT4:
				value.w = ((float*)(bytes + offsets[ e ]))[3];
			case VDECLTYPE_FLOAT3:
				value.z = ((float*)(bytes + offsets[ e ]))[2];
			case VDECLTYPE_FLOAT2:
				value.y = ((float*)(bytes + offsets[ e ]))[1];
			case VDECLTYPE_FLOAT1:
				value.x = ((float*)(bytes + offsets[ e ]))[0];
			}
			
			value = value * mul + add;
			
			switch( types[ e ] )
			{
			case VDECLTYPE_BCOL4:
				*(uint32_t*)(bytes + offsets[ e ]) = Vec4ToCol32( value );
				break;
			case VDECLTYPE_FLOAT4:
				((float*)(bytes + offsets[ e ]))[3] = value.w;
			case VDECLTYPE_FLOAT3:
				((float*)(bytes + offsets[ e ]))[2] = value.z;
			case VDECLTYPE_FLOAT2:
				((float*)(bytes + offsets[ e ]))[1] = value.y;
			case VDECLTYPE_FLOAT1:
				((float*)(bytes + offsets[ e ]))[0] = value.x;
			}
		}
		bytes += size;
	}
}


SGRX_IVertexDecl::~SGRX_IVertexDecl()
{
	g_VertexDecls->unset( m_key );
}

const VDeclInfo& VertexDeclHandle::GetInfo()
{
	static VDeclInfo dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}


bool VD_ExtractFloat1( const VDeclInfo& vdinfo, int vcount, const void* verts, float* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
	case VDECLTYPE_FLOAT2:
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(float*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *((uint8_t*)verts+stride*i+ofs)/255.0f;
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat2( const VDeclInfo& vdinfo, int vcount, const void* verts, Vec2* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V2( *(float*)((uint8_t*)verts+stride*i+ofs), 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec2*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V2(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f );
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat3( const VDeclInfo& vdinfo, int vcount, const void* verts, Vec3* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3( *(float*)((uint8_t*)verts+stride*i+ofs), 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4), 0 );
		}
		break;
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec3*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+2)/255.0f );
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat4( const VDeclInfo& vdinfo, int vcount, const void* verts, Vec4* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4( *(float*)((uint8_t*)verts+stride*i+ofs), 0, 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4), 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT3:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4),
				*(float*)((uint8_t*)verts+stride*i+ofs+8), 0 );
		}
		break;
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec4*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+2)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+3)/255.0f );
		}
		break;
	}
	return true;
}

bool VD_ExtractByte4Clamped( const VDeclInfo& vdinfo, int vcount, const void* verts, uint32_t* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4( *(float*)((uint8_t*)verts+stride*i+ofs), 0, 0, 0 ) );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4), 0, 0 ) );
		}
		break;
	case VDECLTYPE_FLOAT3:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4),
				*(float*)((uint8_t*)verts+stride*i+ofs+8), 0 ) );
		}
		break;
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( *(Vec4*)((uint8_t*)verts+stride*i+ofs) );
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+2)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+3)/255.0f ) );
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat3P( const VDeclInfo& vdinfo, int vcount, const void** vertptrs, Vec3* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage );
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3( *(float*)((uint8_t*)vertptrs[i]+ofs), 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*(float*)((uint8_t*)vertptrs[i]+ofs),
				*(float*)((uint8_t*)vertptrs[i]+ofs+4), 0 );
		}
		break;
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec3*)((uint8_t*)vertptrs[i]+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*((uint8_t*)vertptrs[i]+ofs)/255.0f,
				*((uint8_t*)vertptrs[i]+ofs+1)/255.0f,
				*((uint8_t*)vertptrs[i]+ofs+2)/255.0f );
		}
		break;
	}
	return true;
}

void VD_LerpTri( const VDeclInfo& vdinfo, int vcount, void* outbuf, Vec3* factors, const void* v1, const void* v2, const void* v3 )
{
	SGRX_CAST( char*, coutbuf, outbuf );
	SGRX_CAST( const char*, cv1, v1 );
	SGRX_CAST( const char*, cv2, v2 );
	SGRX_CAST( const char*, cv3, v3 );
	for( int i = 0; i < vdinfo.count; ++i )
	{
		int off = vdinfo.offsets[ i ];
		char* ocoutbuf = coutbuf + off;
		const char *ocv1 = cv1 + off, *ocv2 = cv2 + off, *ocv3 = cv3 + off;
		
		switch( vdinfo.types[ i ] )
		{
		case VDECLTYPE_FLOAT1:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(float*)( ocoutbuf + v * vdinfo.size ) = *(float*)ocv1 * f.x + *(float*)ocv2 * f.y + *(float*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_FLOAT2:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(Vec2*)( ocoutbuf + v * vdinfo.size ) = *(Vec2*)ocv1 * f.x + *(Vec2*)ocv2 * f.y + *(Vec2*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_FLOAT3:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(Vec3*)( ocoutbuf + v * vdinfo.size ) = *(Vec3*)ocv1 * f.x + *(Vec3*)ocv2 * f.y + *(Vec3*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_FLOAT4:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(Vec4*)( ocoutbuf + v * vdinfo.size ) = *(Vec4*)ocv1 * f.x + *(Vec4*)ocv2 * f.y + *(Vec4*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_BCOL4:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(BVec4*)( ocoutbuf + v * vdinfo.size ) = *(BVec4*)ocv1 * f.x + *(BVec4*)ocv2 * f.y + *(BVec4*)ocv3 * f.z;
			}
			break;
		}
	}
}



SGRX_IVertexInputMapping::~SGRX_IVertexInputMapping()
{
	g_VtxInputMaps->unset( m_key );
}



SGRX_IMesh::SGRX_IMesh() :
	m_dataFlags( 0 ),
	m_vertexDataSize( 0 ),
	m_indexDataSize( 0 ),
	m_numBones( 0 ),
	m_boundsMin( Vec3::Create( 0 ) ),
	m_boundsMax( Vec3::Create( 0 ) )
{
}

SGRX_IMesh::~SGRX_IMesh()
{
	if( m_key.size() )
		g_Meshes->unset( m_key );
}

bool SGRX_IMesh::ToMeshData( ByteArray& out )
{
	if( m_meshParts.size() > MAX_MESH_FILE_PARTS )
		return false;
	ByteWriter bw( &out );
	bw.marker( "SS3DMESH" );
	bw.write( uint32_t(m_dataFlags & MDF__PUBFLAGMASK) );
	bw.write( m_boundsMin );
	bw.write( m_boundsMax );
	bw << m_vdata;
	bw << m_idata;
	bw.write( uint8_t(m_vertexDecl->m_key.size()) );
	bw.memory( m_vertexDecl->m_key.data(), m_vertexDecl->m_key.size() );
	bw.write( uint8_t(m_meshParts.size()) );
	for( size_t pid = 0; pid < m_meshParts.size(); ++pid )
	{
		SGRX_MeshPart& MP = m_meshParts[ pid ];
		if( m_dataFlags & MDF_MTLINFO )
		{
			bw.write( uint8_t(MP.mtlFlags) );
			bw.write( uint8_t(MP.mtlBlendMode) );
		}
		bw.write( uint32_t(MP.vertexOffset) );
		bw.write( uint32_t(MP.vertexCount) );
		bw.write( uint32_t(MP.indexOffset) );
		bw.write( uint32_t(MP.indexCount) );
		bw.write( uint8_t(SGRX_MAX_MESH_TEXTURES) ); // materialTextureCount
		bw.write( uint8_t(MP.shader.size()) );
		bw.memory( MP.shader.data(), MP.shader.size() );
		for( int tid = 0; tid < SGRX_MAX_MESH_TEXTURES; ++tid )
		{
			const String& tex = MP.textures[ tid ];
			bw.write( uint8_t(tex.size()) );
			bw.memory( tex.data(), tex.size() );
		}
	}
	if( m_dataFlags & MDF_SKINNED )
	{
		bw.write( uint8_t(m_numBones) );
		for( int bid = 0; bid < m_numBones; ++bid )
		{
			const SGRX_MeshBone& B = m_bones[ bid ];
			bw.write( uint8_t(B.name.size()) );
			bw.memory( B.name.data(), B.name.size() );
			bw.write( uint8_t(B.parent_id) );
			bw.write( B.boneOffset );
		}
	}
	return true;
}

bool SGRX_IMesh::SetPartData( const SGRX_MeshPart* parts, int count )
{
	if( count < 0 )
		return false;
	m_meshParts.assign( parts, count );
	return true;
}

bool SGRX_IMesh::SetBoneData( SGRX_MeshBone* bones, int count )
{
	if( count < 0 || count > SGRX_MAX_MESH_BONES )
		return false;
	int i;
	for( i = 0; i < count; ++i )
		m_bones[ i ] = bones[ i ];
	for( ; i < count; ++i )
		m_bones[ i ] = SGRX_MeshBone();
	m_numBones = count;
	return RecalcBoneMatrices();
}

bool SGRX_IMesh::RecalcBoneMatrices()
{
	if( !m_numBones )
	{
		return true;
	}
	
	for( int b = 0; b < m_numBones; ++b )
	{
		if( m_bones[ b ].parent_id < -1 || m_bones[ b ].parent_id >= b )
		{
			LOG_WARNING << "RecalcBoneMatrices: each parent_id must point to a previous bone or no bone (-1) [error in bone "
				<< b << ": " << m_bones[ b ].parent_id << "]";
			return false;
		}
	}
	
	Mat4 skinOffsets[ SGRX_MAX_MESH_BONES ];
	for( int b = 0; b < m_numBones; ++b )
	{
		if( m_bones[ b ].parent_id >= 0 )
			skinOffsets[ b ].Multiply( m_bones[ b ].boneOffset, skinOffsets[ m_bones[ b ].parent_id ] );
		else
			skinOffsets[ b ] = m_bones[ b ].boneOffset;
		m_bones[ b ].skinOffset = skinOffsets[ b ];
	}
	for( int b = 0; b < m_numBones; ++b )
	{
		if( !skinOffsets[ b ].InvertTo( m_bones[ b ].invSkinOffset ) )
		{
			LOG_WARNING << "RecalcBoneMatrices: failed to invert skin offset matrix #" << b;
			m_bones[ b ].invSkinOffset = Mat4::Identity;
		}
	}
	return true;
}

bool SGRX_IMesh::SetAABBFromVertexData( const void* data, size_t size, VertexDeclHandle vd )
{
	return GetAABBFromVertexData( vd.GetInfo(), (const char*) data, size, m_boundsMin, m_boundsMax );
}

static void _Tris_Add( Array< Triangle >& tris, VDeclInfo* vdinfo, const void* v1, const void* v2, const void* v3 )
{
	const void* verts[3] = { v1, v2, v3 };
	Vec3 pos[3] = {0};
	VD_ExtractFloat3P( *vdinfo, 3, verts, pos );
	
	Triangle T = { pos[0], pos[1], pos[2] };
	tris.push_back( T );
}

template< class IdxType > void SGRX_IMesh_GenTriTree_Core( SGRX_IMesh* mesh )
{
	Array< Triangle > tris;
	
	SGRX_CAST( IdxType*, indices, mesh->m_idata.data() );
	VDeclInfo* vdinfo = &mesh->m_vertexDecl->m_info;
	size_t stride = vdinfo->size;
	
	for( size_t part_id = 0; part_id < mesh->m_meshParts.size(); ++part_id )
	{
		SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
		tris.clear();
		for( uint32_t tri = MP.indexOffset, triend = MP.indexOffset + MP.indexCount; tri < triend; tri += 3 )
		{
			_Tris_Add( tris, vdinfo
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
			);
		}
		MP.m_triTree.SetTris( tris.data(), tris.size() );
	}
}

void SGRX_IMesh::GenerateTriangleTree()
{
	LOG_FUNCTION;
	
	if( ( m_dataFlags & MDF_INDEX_32 ) != 0 )
	{
		SGRX_IMesh_GenTriTree_Core< uint32_t >( this );
	}
	else
	{
		SGRX_IMesh_GenTriTree_Core< uint16_t >( this );
	}
}

void SGRX_IMesh_RaycastAll_Core_TestTriangle( const Vec3& rpos, const Vec3& rdir, float rlen,
	SceneRaycastCallback* cb, SceneRaycastInfo* srci, VDeclInfo* vdinfo, const void* v1, const void* v2, const void* v3 )
{
	const void* verts[3] = { v1, v3, v2 }; // order swapped for RayPolyIntersect
	Vec3 pos[3] = {0};
	VD_ExtractFloat3P( *vdinfo, 3, verts, pos );
	
	float dist[1];
	if( RayPolyIntersect( rpos, rdir, pos, 3, dist ) && dist[0] >= 0 && dist[0] < rlen )
	{
		srci->factor = dist[0] / rlen;
		srci->normal = Vec3Cross( pos[1] - pos[0], pos[2] - pos[0] ).Normalized();
		if( srci->meshinst )
			srci->normal = srci->meshinst->matrix.TransformNormal( srci->normal );
		
		// TODO u/v
		cb->AddResult( srci );
	}
}

struct MPTRayQuery : BaseRayQuery
{
	MPTRayQuery( SceneRaycastCallback* cb, SceneRaycastInfo* srciptr, Triangle* ta,
		const Vec3& r0, const Vec3& r1 )
	: srcb( cb ), srci( srciptr ), tris( ta ), ray_end( r1 )
	{
		SetRay( r0, r1 );
	}
	bool operator () ( int32_t* ids, int32_t count )
	{
		for( int32_t i = 0; i < count; ++i )
		{
			Triangle& T = tris[ ids[ i ] ];
			float dist = IntersectLineSegmentTriangle( ray_origin, ray_end, T.P1, T.P2, T.P3 );
			if( dist < 1.0f )
			{
				srci->factor = dist;
				srci->normal = T.GetNormal();
				if( srci->meshinst )
					srci->normal = srci->meshinst->matrix.TransformNormal( srci->normal );
				
				// TODO u/v
				srcb->AddResult( srci );
			}
		}
		return true;
	}
	
	SceneRaycastCallback* srcb;
	SceneRaycastInfo* srci;
	Triangle* tris;
	Vec3 ray_end;
};

template< class IdxType > void SGRX_IMesh_RaycastAll_Core( SGRX_IMesh* mesh, const Vec3& from, const Vec3& to,
	SceneRaycastCallback* cb, SceneRaycastInfo* srci, size_t fp, size_t ep )
{
	SGRX_CAST( IdxType*, indices, mesh->m_idata.data() );
	VDeclInfo* vdinfo = &mesh->m_vertexDecl->m_info;
	size_t stride = vdinfo->size;
	
	Vec3 dtdir = to - from;
	float rlen = dtdir.Length();
	dtdir /= rlen;
	
	for( size_t part_id = fp; part_id < ep; ++part_id )
	{
		srci->partID = part_id;
		SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
		if( MP.mtlBlendMode != SGRX_MtlBlend_None &&
			MP.mtlBlendMode != SGRX_MtlBlend_Basic )
			continue;
		
		if( MP.m_triTree.m_bbTree.m_nodes.size() )
		{
			// we have a tree!
			MPTRayQuery query( cb, srci, MP.m_triTree.m_tris.data(), from, to );
			MP.m_triTree.m_bbTree.RayQuery( query );
		}
		else
		{
			// no tree, iterate all triangles
			for( uint32_t tri = MP.indexOffset, triend = MP.indexOffset + MP.indexCount; tri < triend; tri += 3 )
			{
				srci->triID = tri / 3;
				SGRX_IMesh_RaycastAll_Core_TestTriangle( from, dtdir, rlen, cb, srci, vdinfo
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
				);
			}
		}
	}
}

void SGRX_IMesh::RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, SGRX_MeshInstance* cbmi )
{
	if( !m_vdata.size() || !m_idata.size() || !m_meshParts.size() )
		return;
	
	SceneRaycastInfo srci = { 0, V3(0), 0, 0, -1, -1, -1, cbmi };
	if( ( m_dataFlags & MDF_INDEX_32 ) != 0 )
	{
		SGRX_IMesh_RaycastAll_Core< uint32_t >( this, from, to, cb, &srci, 0, m_meshParts.size() );
	}
	else
	{
		SGRX_IMesh_RaycastAll_Core< uint16_t >( this, from, to, cb, &srci, 0, m_meshParts.size() );
	}
}

void SGRX_IMesh::MRC_DebugDraw( SGRX_MeshInstance* mi )
{
	// TODO
}

template< typename T, typename T2 > void sa2_insert( T* arr, T2* arr2, int& count, int& at, const T& val, const T2& val2 )
{
	for( int i = count; i > at; )
	{
		i--;
		arr[ i + 1 ] = arr[ i ];
		arr2[ i + 1 ] = arr2[ i ];
	}
	arr2[ at ] = val2;
	arr[ at++ ] = val;
	count++;
}
template< typename T, typename T2 > void sa2_remove( T* arr, T2* arr2, int& count, int& at )
{
	count--;
	for( int i = at; i < count; ++i )
	{
		arr[ i ] = arr[ i + 1 ];
		arr2[ i ] = arr2[ i + 1 ];
	}
	at--;
}

void SGRX_IMesh_Clip_Core_ClipTriangle( const Mat4& mtx,
	                                    const Mat4& vpmtx,
	                                    ByteArray& outverts,
	                                    SGRX_IVertexDecl* vdecl,
	                                    bool decal,
	                                    float inv_zn2zf,
	                                    uint32_t color,
	                                    const void* v1,
	                                    const void* v2,
	                                    const void* v3 )
{
	const void* verts[3] = { v1, v2, v3 };
	Vec3 pos[3] = {0};
	VD_ExtractFloat3P( vdecl->m_info, 3, verts, pos );
	
	pos[0] = mtx.TransformPos( pos[0] );
	pos[1] = mtx.TransformPos( pos[1] );
	pos[2] = mtx.TransformPos( pos[2] );
	
	Vec4 tpos[3] =
	{
		vpmtx.Transform( V4( pos[0], 1.0f ) ),
		vpmtx.Transform( V4( pos[1], 1.0f ) ),
		vpmtx.Transform( V4( pos[2], 1.0f ) ),
	};
	Vec4 pts[9] =
	{
		tpos[0], tpos[1], tpos[2],
		V4(0), V4(0), V4(0),
		V4(0), V4(0), V4(0)
	};
	Vec3 fcs[9] =
	{
		V3(1,0,0), V3(0,1,0), V3(0,0,1),
		V3(0), V3(0), V3(0),
		V3(0), V3(0), V3(0)
	};
	int pcount = 3;
	
#define IMCCCT_CLIP_Pred( initsd, loopsd )           \
	{                                                \
		Vec4 prevpt = pts[ pcount - 1 ];             \
		Vec3 prevfc = fcs[ pcount - 1 ];             \
		float prevsd = /* = 1 */ initsd /* */;       \
		for( int i = 0; i < pcount; ++i )            \
		{                                            \
			Vec4 currpt = pts[ i ];                  \
			Vec3 currfc = fcs[ i ];                  \
			float currsd = /* = 2 */ loopsd /* */;   \
			if( prevsd * currsd < 0 )                \
			{                                        \
				/* insert intersection point */      \
				float f = safe_fdiv( -prevsd,        \
					( currsd - prevsd ) );           \
				sa2_insert( pts, fcs, pcount, i,     \
					TLERP( prevpt, currpt, f ),      \
					TLERP( prevfc, currfc, f ) );    \
			}                                        \
			if( currsd >= 0 )                        \
			{                                        \
				sa2_remove( pts, fcs, pcount, i );   \
			}                                        \
			prevpt = currpt;                         \
			prevfc = currfc;                         \
			prevsd = currsd;                         \
		}                                            \
	}
	IMCCCT_CLIP_Pred( SMALL_FLOAT - prevpt.w, SMALL_FLOAT - currpt.w ); // clip W <= 0
	IMCCCT_CLIP_Pred( prevpt.x - prevpt.w, currpt.x - currpt.w ); // clip X > W
	IMCCCT_CLIP_Pred( -prevpt.x - prevpt.w, -currpt.x - currpt.w ); // clip X < -W
	IMCCCT_CLIP_Pred( prevpt.y - prevpt.w, currpt.y - currpt.w ); // clip Y > W
	IMCCCT_CLIP_Pred( -prevpt.y - prevpt.w, -currpt.y - currpt.w ); // clip Y < -W
	IMCCCT_CLIP_Pred( prevpt.z - prevpt.w, currpt.z - currpt.w ); // clip Z > W
	IMCCCT_CLIP_Pred( -prevpt.z - prevpt.w, -currpt.z - currpt.w ); // clip Z < -W
	// LOG << "VCOUNT: " << pcount;
	// for(int i = 0; i < pcount;++i) LOG << pts[i] << fcs[i];
	if( pcount < 3 )
		return;
	
	// interpolate vertices
	uint8_t vbuf[ 256 * 9 ];
	if( decal )
	{
		SGRX_CAST( SGRX_Vertex_Decal*, dvs, vbuf );
		
		// fixed function interpolation -- faster than VD_LerpTri
		Vec3 nrm[3] = {0};
		VD_ExtractFloat3P( vdecl->m_info, 3, verts, nrm, VDECLUSAGE_NORMAL );
		
		nrm[0] = mtx.TransformNormal( nrm[0] );
		nrm[1] = mtx.TransformNormal( nrm[1] );
		nrm[2] = mtx.TransformNormal( nrm[2] );
		
		for( int i = 0; i < pcount; ++i )
		{
			const Vec3& f = fcs[ i ];
			dvs[ i ].position = pos[0] * f.x + pos[1] * f.y + pos[2] * f.z;
			dvs[ i ].normal = nrm[0] * f.x + nrm[1] * f.y + nrm[2] * f.z;
			// ignore texcoords, they will be regenerated
			dvs[ i ].tangent = 0x007f7f7f; // TODO FIX
			dvs[ i ].color = color; // always constant for decals
			dvs[ i ].padding0 = 0;
		}
		
		if( inv_zn2zf /* perspective distance correction */ )
		{
			for( int i = 0; i < pcount; ++i )
			{
				Vec4 vtp = vpmtx.Transform( V4( dvs[ i ].position, 1.0f ) );
				if( vtp.w )
				{
					float rcp_vtp_w = 1.0f / vtp.w;
					dvs[ i ].texcoord = V3
					(
						vtp.x * rcp_vtp_w * 0.5f + 0.5f,
						vtp.y * rcp_vtp_w * 0.5f + 0.5f,
						vtp.z * inv_zn2zf
					);
				}
				else
				{
					dvs[ i ].texcoord = V3(0);
				}
			}
		}
		else
		{
			for( int i = 0; i < pcount; ++i )
			{
				Vec4 vtp = vpmtx.Transform( V4( dvs[ i ].position, 1.0f ) );
				if( vtp.w )
				{
					float rcp_vtp_w = 1.0f / vtp.w;
					dvs[ i ].texcoord = V3
					(
						vtp.x * rcp_vtp_w * 0.5f + 0.5f,
						vtp.y * rcp_vtp_w * 0.5f + 0.5f,
						vtp.z * rcp_vtp_w * 0.5f + 0.5f
					);
				}
				else
				{
					dvs[ i ].texcoord = V3(0);
				}
			}
		}
	}
	else
	{
		memset( vbuf, 0, sizeof(vbuf) );
		VD_LerpTri( vdecl->m_info, pcount, vbuf, fcs, v1, v2, v3 );
	}
	const int stride = decal ? sizeof(SGRX_Vertex_Decal) : vdecl->m_info.size;
	for( int i = 1; i < pcount - 1; ++i )
	{
		outverts.append( vbuf, stride );
		outverts.append( vbuf + i * stride, stride * 2 );
	}
}

template< class IdxType >
struct IMesh_ClipQuery
{
	void operator () ( int32_t* tris, int32_t count )
	{
		for( int32_t i = 0; i < count; ++i )
		{
			int32_t tri = tris[ i ];
			if( usedTris[ tri >> 5 ] & ( 1 << ( tri & 31 ) ) )
				continue;
			usedTris[ tri >> 5 ] |= 1 << ( tri & 31 );
			tri *= 3;
			SGRX_IMesh_Clip_Core_ClipTriangle( mtx, vpmtx, outverts, mesh->m_vertexDecl, decal, inv_zn2zf, color
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
			);
		}
	}
	
	Mat4 mtx;
	Mat4 vpmtx;
	ByteArray& outverts;
	bool decal;
	float inv_zn2zf;
	uint32_t color;
	SGRX_IMesh* mesh;
	SGRX_MeshPart& MP;
	size_t stride;
	IdxType* indices;
	
	Array< uint8_t > usedTris;
};

void FrustumAABB( const Mat4& mvp, Vec3& outmin, Vec3& outmax )
{
	Mat4 inv = mvp.Inverted();
	Vec3 pts[8] =
	{
		inv.TransformPos( V3(-1,-1,-1) ),
		inv.TransformPos( V3(+1,-1,-1) ),
		inv.TransformPos( V3(-1,+1,-1) ),
		inv.TransformPos( V3(+1,+1,-1) ),
		inv.TransformPos( V3(-1,-1,+1) ),
		inv.TransformPos( V3(+1,-1,+1) ),
		inv.TransformPos( V3(-1,+1,+1) ),
		inv.TransformPos( V3(+1,+1,+1) ),
	};
	outmin = outmax = pts[0];
	for( int i = 1; i < 8; ++i )
	{
		outmin = Vec3::Min( outmin, pts[ i ] );
		outmax = Vec3::Max( outmax, pts[ i ] );
	}
}

template< class IdxType >
void SGRX_IMesh_Clip_Core( SGRX_IMesh* mesh,
	                       const Mat4& mtx,
	                       const Mat4& vpmtx,
	                       bool decal,
	                       float inv_zn2zf,
	                       uint32_t color,
	                       ByteArray& outverts,
	                       size_t fp,
	                       size_t ep )
{
	size_t stride = mesh->m_vertexDecl.GetInfo().size;
	SGRX_CAST( IdxType*, indices, mesh->m_idata.data() );
	
	for( size_t part_id = fp; part_id < ep; ++part_id )
	{
		SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
		if( MP.mtlBlendMode != SGRX_MtlBlend_None &&
			MP.mtlBlendMode != SGRX_MtlBlend_Basic )
			continue;
		
		if( MP.m_triTree.m_bbTree.m_nodes.size() )
		{
			// we have a tree!
			Vec3 bbmin, bbmax;
			FrustumAABB( mtx * vpmtx, bbmin, bbmax );
			IMesh_ClipQuery<IdxType> query = {
				mtx, vpmtx, outverts, decal, inv_zn2zf, color,
				mesh, MP, stride, indices
			};
			query.usedTris.resize_using( ( ( MP.indexCount / 3 ) >> 5 ) + 1, 0 );
			MP.m_triTree.m_bbTree.Query( bbmin, bbmax, query );
		}
		else
		{
			for( uint32_t tri = MP.indexOffset, triend = MP.indexOffset + MP.indexCount; tri < triend; tri += 3 )
			{
				SGRX_IMesh_Clip_Core_ClipTriangle( mtx, vpmtx, outverts, mesh->m_vertexDecl, decal, inv_zn2zf, color
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
				);
			}
		}
	}
}

void SGRX_IMesh::Clip( const Mat4& mtx,
	                   const Mat4& vpmtx,
	                   ByteArray& outverts,
	                   bool decal,
	                   float inv_zn2zf,
	                   uint32_t color,
	                   size_t firstPart,
	                   size_t numParts )
{
	if( m_vdata.size() == 0 || m_idata.size() == 0 )
		return;
	
	size_t MPC = m_meshParts.size();
	firstPart = TMIN( firstPart, MPC - 1 );
	size_t oneOverLastPart = TMIN( firstPart + TMIN( numParts, MPC ), MPC );
	if( ( m_dataFlags & MDF_INDEX_32 ) != 0 )
	{
		SGRX_IMesh_Clip_Core< uint32_t >( this, mtx, vpmtx, decal, inv_zn2zf, color, outverts, firstPart, oneOverLastPart );
	}
	else
	{
		SGRX_IMesh_Clip_Core< uint16_t >( this, mtx, vpmtx, decal, inv_zn2zf, color, outverts, firstPart, oneOverLastPart );
	}
}


int SGRX_IMesh::FindBone( const StringView& name )
{
	for( int bid = 0; bid < m_numBones; ++bid )
	{
		if( m_bones[ bid ].name == name )
			return bid;
	}
	return -1;
}

bool SGRX_IMesh::IsBoneUnder( int bone, int parent )
{
	while( bone != parent && bone != -1 )
	{
		bone = m_bones[ bone ].parent_id;
	}
	return bone == parent;
}

int SGRX_IMesh::BoneDistance( int bone, int parent )
{
	int count = 1;
	while( bone != parent && bone != -1 )
	{
		bone = m_bones[ bone ].parent_id;
		count++;
	}
	return bone == parent ? count : -1;
}


SGRX_Log& operator << ( SGRX_Log& L, const SGRX_Camera& cam )
{
	L << "CAMERA:";
	L << "\n    position = " << cam.position;
	L << "\n    direction = " << cam.direction;
	L << "\n    updir = " << cam.updir;
	L << "\n    angle = " << cam.angle;
	L << "\n    aspect = " << cam.aspect;
	L << "\n    aamix = " << cam.aamix;
	L << "\n    znear = " << cam.znear;
	L << "\n    zfar = " << cam.zfar;
	L << "\n    mView = " << cam.mView;
	L << "\n    mProj = " << cam.mProj;
	L << "\n    mInvView = " << cam.mInvView;
	return L;
}

void SGRX_Camera::UpdateViewMatrix()
{
	mView.LookAt( position, direction, updir );
	mInvView = mView.Inverted();
}

void SGRX_Camera::UpdateProjMatrix()
{
	mProj.Perspective( angle, aspect, aamix, znear, zfar );
}

void SGRX_Camera::UpdateMatrices()
{
	UpdateViewMatrix();
	UpdateProjMatrix();
}

Vec3 SGRX_Camera::WorldToScreen( const Vec3& pos, bool* infront )
{
	Vec3 P = mView.TransformPos( pos );
	Vec4 psP = mProj.Transform( V4( P, 1 ) );
	P = psP.ToVec3() * ( 1.0f / psP.w );
	P.x = P.x * 0.5f + 0.5f;
	P.y = P.y * -0.5f + 0.5f;
	if( infront )
		*infront = psP.w > 0;
	return P;
}

bool SGRX_Camera::GetCursorRay( float x, float y, Vec3& pos, Vec3& dir ) const
{
	Vec3 tPos = { x * 2 - 1, y * -2 + 1, 0 };
	Vec3 tTgt = { x * 2 - 1, y * -2 + 1, 1 };
	
	Mat4 viewProjMatrix, inv;
	viewProjMatrix.Multiply( mView, mProj );
	if( !viewProjMatrix.InvertTo( inv ) )
		return false;
	
	tPos = inv.TransformPos( tPos );
	tTgt = inv.TransformPos( tTgt );
	Vec3 tDir = ( tTgt - tPos ).Normalized();
	
	pos = tPos;
	dir = tDir;
	return true;
}


SGRX_Light::SGRX_Light( SGRX_Scene* s ) :
	_scene( s ),
	type( LIGHT_POINT ),
	enabled( true ),
	position( Vec3::Create( 0 ) ),
	direction( Vec3::Create( 0, 1, 0 ) ),
	updir( Vec3::Create( 0, 0, 1 ) ),
	color( Vec3::Create( 1 ) ),
	range( 100 ),
	power( 2 ),
	angle( 60 ),
	aspect( 1 ),
	hasShadows( false ),
	layers( 0x1 ),
	matrix( Mat4::Identity ),
	_tf_position( Vec3::Create( 0 ) ),
	_tf_direction( Vec3::Create( 0, 1, 0 ) ),
	_tf_updir( Vec3::Create( 0, 0, 1 ) ),
	_tf_range( 100 ),
	_dibuf_begin( NULL ),
	_dibuf_end( NULL )
{
	projectionMaterial.shader = "proj_default";
	projectionMaterial.flags = SGRX_MtlFlag_Unlit | SGRX_MtlFlag_Decal;
	projectionMaterial.blendMode = SGRX_MtlBlend_Basic;
	UpdateTransform();
}

SGRX_Light::~SGRX_Light()
{
	if( _scene )
	{
		_scene->m_lights.unset( this );
	}
}

void SGRX_Light::UpdateTransform()
{
	_tf_position = matrix.TransformPos( position );
	_tf_direction = matrix.TransformNormal( direction );
	_tf_updir = matrix.TransformNormal( updir );
	_tf_range = matrix.TransformNormal( V3( sqrtf( range * range / 3 ) ) ).Length();
	viewMatrix = Mat4::CreateLookAt( _tf_position, _tf_direction, _tf_updir );
	projMatrix = Mat4::CreatePerspective( angle, aspect, 0.5, _tf_range * 0.001f, _tf_range );
	viewProjMatrix.Multiply( viewMatrix, projMatrix );
}

void SGRX_Light::GenerateCamera( SGRX_Camera& outcam )
{
	outcam.position = _tf_position;
	outcam.direction = _tf_direction;
	outcam.updir = _tf_updir;
	outcam.angle = angle;
	outcam.aspect = aspect;
	outcam.aamix = 0.5f;
	outcam.znear = _tf_range * 0.001f;
	outcam.zfar = _tf_range;
	outcam.UpdateMatrices();
}

void SGRX_Light::SetTransform( const Mat4& mtx )
{
	matrix = mtx;
}

void SGRX_Light::GetVolumePoints( Vec3 pts[8] )
{
	if( type == LIGHT_POINT )
	{
		pts[0] = _tf_position + V3(-_tf_range, -_tf_range, -_tf_range);
		pts[1] = _tf_position + V3(+_tf_range, -_tf_range, -_tf_range);
		pts[2] = _tf_position + V3(-_tf_range, +_tf_range, -_tf_range);
		pts[3] = _tf_position + V3(+_tf_range, +_tf_range, -_tf_range);
		pts[4] = _tf_position + V3(-_tf_range, -_tf_range, +_tf_range);
		pts[5] = _tf_position + V3(+_tf_range, -_tf_range, +_tf_range);
		pts[6] = _tf_position + V3(-_tf_range, +_tf_range, +_tf_range);
		pts[7] = _tf_position + V3(+_tf_range, +_tf_range, +_tf_range);
	}
	else
	{
		Mat4 inv = Mat4::Identity;
		viewProjMatrix.InvertTo( inv );
		Vec3 ipts[8] =
		{
			V3(-1, -1, -1), V3(+1, -1, -1),
			V3(-1, +1, -1), V3(+1, +1, -1),
			V3(-1, -1, +1), V3(+1, -1, +1),
			V3(-1, +1, +1), V3(+1, +1, +1),
		};
		for( int i = 0; i < 8; ++i )
			pts[ i ] = inv.TransformPos( ipts[ i ] );
	}
}


SGRX_CullScene::~SGRX_CullScene()
{
}


SGRX_DrawItem::SGRX_DrawItem() : MI( NULL ), part( 0 ), type( 0 ), _lightbuf_begin( NULL ), _lightbuf_end( NULL )
{
}


SGRX_Material::SGRX_Material() : flags(0), blendMode(SGRX_MtlBlend_None)
{
}


static int LineNumber( StringView text, StringView name )
{
	return 1 + text.part( name.data() - text.data() ).count( "\n" );
}

static bool IsValidPassProp( StringView name )
{
	// pass properties
	if( name == "Enabled" ) return true;
	if( name == "Order" ) return true;
	if( name == "Inherit" ) return true;
	
	// render states
	if( name == "WireFill" ) return true;
	if( name == "CullMode" ) return true;
	if( name == "SeparateBlend" ) return true;
	if( name == "ScissorEnable" ) return true;
	if( name == "MultisampleEnable" ) return true;
	if( name == "DepthEnable" ) return true;
	if( name == "DepthWriteEnable" ) return true;
	if( name == "DepthFunc" ) return true;
	if( name == "StencilEnable" ) return true;
	if( name == "StencilReadMask" ) return true;
	if( name == "StencilWriteMask" ) return true;
	if( name == "StencilFrontFailOp" ) return true;
	if( name == "StencilFrontDepthFailOp" ) return true;
	if( name == "StencilFrontPassOp" ) return true;
	if( name == "StencilFrontFunc" ) return true;
	if( name == "StencilBackFailOp" ) return true;
	if( name == "StencilBackDepthFailOp" ) return true;
	if( name == "StencilBackPassOp" ) return true;
	if( name == "StencilBackFunc" ) return true;
	if( name == "StencilRef" ) return true;
	if( name == "DepthBias" ) return true;
	if( name == "SlopeDepthBias" ) return true;
	if( name == "DepthBiasClamp" ) return true;
	if( name == "BlendFactor" ) return true;
	
	static const StringView rtVars[] = { "ColorWrite", "BlendEnable",
		"BlendOp", "SrcBlend", "DstBlend", "BlendOpAlpha", "SrcBlendAlpha", "DstBlendAlpha" };
	for( size_t i = 0; i < sizeof(rtVars)/sizeof(rtVars[0]); ++i )
	{
		if( name.starts_with( rtVars[i] ) )
		{
			if( name.size() == rtVars[i].size() ) return true;
			if( name.size() == rtVars[i].size() + 3 )
			{
				StringView indexpart = name.part( rtVars[i].size() );
				if( indexpart.size() == 3 && indexpart[0] == '[' && indexpart[2] == ']' )
				{
					for( int j = 0; j < 8; ++j )
						if( indexpart[1] == "01234567"[j] )
							return true;
				}
			}
		}
	}
	
	return false;
}

static bool _XSD_LoadPassProps( SV2SVMap& props, StringView passtext, StringView text )
{
	ConfigReader cr( passtext );
	StringView key, value;
	while( cr.Read( key, value ) )
	{
		if( !IsValidPassProp( key ) )
		{
			LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - "
				"invalid pass property (" << key << ") at line " << LineNumber( text, key );
			return false;
		}
		props[ key ] = value;
	}
	return true;
}

bool SGRX_XShaderDef::_XSD_LoadProps(
	HashTable< StringView, SV2SVMap >& allprops,
	Array< StringView >& defines,
	StringView text )
{
#define MAX_DISABLEVEL 9999
	int curlevel = 0, disablevel = MAX_DISABLEVEL;
	
	ConfigReader cr( text );
	StringView key, value;
	while( cr.Read( key, value ) )
	{
		if( key == "[[shader]]" )
		{
			StringView shdr = cr.it.until( "[[endshader]]" );
			if( shdr.end() == cr.it.end() )
				return false;
			int newlines = text.part( cr.it.data() - text.data() ).count( "\n" );
			cr.it.skip( shdr.size() );
			cr.it = cr.it.after( "[[endshader]]" );
			
			if( curlevel < disablevel )
			{
				for( int i = 0; i < newlines; ++i )
					shader.append( "\n" ); // for accurate line numbers
				shader.append( shdr );
			}
		}
		else if( key == "[[pass]]" )
		{
			StringView passtext = cr.it.until( "[[endpass]]" );
			if( passtext.end() == cr.it.end() )
				return false;
			cr.it.skip( passtext.size() );
			cr.it = cr.it.after( "[[endpass]]" );
			
			if( curlevel < disablevel )
				_XSD_LoadPassProps( allprops[ value ], passtext, text );
		}
		else if( key == "[[define]]" )
		{
			if( curlevel < disablevel )
				defines.push_back( value );
		}
		else if( key == "[[ifdef]]" )
		{
			if( curlevel < disablevel )
			{
				curlevel++;
				if( defines.find_first_at( value ) == NOT_FOUND )
					disablevel = curlevel;
			}
		}
		else if( key == "[[ifndef]]" )
		{
			if( curlevel < disablevel )
			{
				curlevel++;
				if( defines.find_first_at( value ) != NOT_FOUND )
					disablevel = curlevel;
			}
		}
		else if( key == "[[endif]]" )
		{
			if( curlevel < disablevel )
			{
				curlevel--;
				disablevel = MAX_DISABLEVEL;
			}
		}
		else
		{
			LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - "
				"invalid command (" << key << ") at line " << LineNumber( text, key );
			return false;
		}
	}
	return true;
}

#define LoadPassBool( out, name ) \
	if( props.isset( name ) ){ \
		out = String_ParseBool( *props.getptr( name ) ); }
#define LoadPassInt( out, name, vmin, vmax ) \
	if( props.isset( name ) ){ \
		int64_t v = String_ParseInt( *props.getptr( name ) ); \
		if( v < int64_t(vmin) || v > int64_t(vmax) ){ \
			LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - " \
				"value for property " << name << " out of range [" << (vmin) \
				<< ";" << (vmax) << "] at line " << LineNumber( text, *props.getptr( name ) ); \
			return false; } \
		out = v; }
#define LoadPassFloat( out, name ) \
	if( props.isset( name ) ){ \
		out = String_ParseFloat( *props.getptr( name ) ); }
#define LoadPassVec4( out, name ) \
	if( props.isset( name ) ){ \
		out = String_ParseVec4( *props.getptr( name ) ); }
#define LoadPassEnum( out, name, values ) \
	if( props.isset( name ) ){ \
		int v = String_ParseEnum( *props.getptr( name ), values ); \
		if( v == -1 ){ \
			LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - " \
				"value for property " << name << " is invalid at line " \
				<< LineNumber( text, *props.getptr( name ) ); \
			return false; } \
		out = v; }

bool SGRX_XShaderDef::LoadText( StringView text )
{
	shader.clear();
	passes.clear();
	
	HashTable< StringView, SV2SVMap > allprops;
	Array< StringView > defines;
	
	if( _XSD_LoadProps( allprops, defines, text ) == false )
	{
		return false;
	}
	
	for( size_t i = 0; i < allprops.size(); ++i )
	{
		if( String_ParseBool( allprops.item( i ).value.getcopy( "Enabled", "True" ) ) == false )
			continue; // this pass does not exist
		
		SV2SVMap props = allprops.item( i ).value;
		HashTable< StringView, NoValue > inheritChain;
		inheritChain.set( allprops.item( i ).key, NoValue() );
		
		StringView inherit = props.getcopy( "Inherit", "" );
		while( inherit != "" )
		{
			if( inheritChain.isset( inherit ) )
			{
				LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - "
					"inheritance loop detected while trying to include " << inherit
					<< " from " << inheritChain.item( inheritChain.size() - 1 ).key;
				return false;
			}
			SV2SVMap* subprops = allprops.getptr( inherit );
			if( subprops == NULL )
			{
				LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - "
					"could not inherit " << inherit
					<< " from " << inheritChain.item( inheritChain.size() - 1 ).key;
				return false;
			}
			
			inheritChain.set( inherit, NoValue() );
			inherit = subprops->getcopy( "Inherit", "" );
			for( size_t i = 0; i < subprops->size(); ++i )
			{
				if( props.isset( subprops->item( i ).key ) == false )
					props.set( subprops->item( i ).key, subprops->item( i ).value );
			}
		}
		
		Pass pass;
		pass.name = allprops.item( i ).key;
		pass.render_state.Init();
		pass.order = 0;
		
		LoadPassInt( pass.order, "Order", 0, 0x7fff );
		
		SGRX_RenderState& rs = pass.render_state;
		LoadPassBool( rs.wireFill, "WireFill" );
		static const char* eCullModes[] = { "None", "Back", "Front", NULL };
		LoadPassEnum( rs.cullMode, "CullMode", eCullModes );
		LoadPassBool( rs.separateBlend, "SeparateBlend" );
		LoadPassBool( rs.scissorEnable, "ScissorEnable" );
		LoadPassBool( rs.multisampleEnable, "MultisampleEnable" );
		
		LoadPassBool( rs.depthEnable, "DepthEnable" );
		LoadPassBool( rs.depthWriteEnable, "DepthWriteEnable" );
		static const char* eDepthFuncs[] = { "Never", "Always", "Equal",
			"NotEqual", "Less", "LessEqual", "Greater", "GreaterEqual" };
		LoadPassEnum( rs.depthFunc, "DepthFunc", eDepthFuncs );
		LoadPassBool( rs.stencilEnable, "StencilEnable" );
		LoadPassInt( rs.stencilReadMask, "StencilReadMask", 0, 255 );
		LoadPassInt( rs.stencilWriteMask, "StencilWriteMask", 0, 255 );
		static const char* eStencilOps[] = { "Keep", "Zero", "Replace", 
			"Invert", "Incr", "Decr", "IncrSat", "DecrSat" };
		LoadPassEnum( rs.stencilFrontFailOp, "StencilFrontFailOp", eStencilOps );
		LoadPassEnum( rs.stencilFrontDepthFailOp, "StencilFrontDepthFailOp", eStencilOps );
		LoadPassEnum( rs.stencilFrontPassOp, "StencilFrontPassOp", eStencilOps );
		LoadPassEnum( rs.stencilFrontFunc, "StencilFrontFunc", eDepthFuncs );
		LoadPassEnum( rs.stencilBackFailOp, "StencilBackFailOp", eStencilOps );
		LoadPassEnum( rs.stencilBackDepthFailOp, "StencilBackDepthFailOp", eStencilOps );
		LoadPassEnum( rs.stencilBackPassOp, "StencilBackPassOp", eStencilOps );
		LoadPassEnum( rs.stencilBackFunc, "StencilBackFunc", eDepthFuncs );
		LoadPassInt( rs.stencilRef, "StencilRef", 0, 255 );
		
		LoadPassFloat( rs.depthBias, "DepthBias" );
		LoadPassFloat( rs.slopeDepthBias, "SlopeDepthBias" );
		LoadPassFloat( rs.depthBiasClamp, "DepthBiasClamp" );
		LoadPassVec4( rs.blendFactor, "BlendFactor" );
		
		for( int brtid = 0; brtid < 8; ++brtid )
		{
			SGRX_RenderState::BlendState& bs = rs.blendStates[ brtid ];
			
			static const char* eColorWrites[] = { "None",
				"R", "G", "RG", "B", "RB", "GB", "RGB",
				"A", "RA", "GA", "RGA", "BA", "RBA", "GBA", "RGBA" };
			static const char* eBlendOps[] = { "Add", "Sub", "RevSub", "Min", "Max" };
			static const char* eBlendFactors[] = { "Zero", "One", 
				"SrcColor", "InvSrcColor", "DstColor", "InvDstColor", 
				"SrcAlpha", "InvSrcAlpha", "DstAlpha", "InvDstAlpha", 
				"Factor", "InvFactor" };
			
			// generic version sets all
			LoadPassEnum( bs.colorWrite, "ColorWrite", eColorWrites );
			LoadPassBool( bs.blendEnable, "BlendEnable" );
			LoadPassEnum( bs.blendOp, "BlendOp", eBlendOps );
			LoadPassEnum( bs.srcBlend, "SrcBlend", eBlendFactors );
			LoadPassEnum( bs.dstBlend, "DstBlend", eBlendFactors );
			LoadPassEnum( bs.blendOpAlpha, "BlendOpAlpha", eBlendOps );
			LoadPassEnum( bs.srcBlendAlpha, "SrcBlendAlpha", eBlendFactors );
			LoadPassEnum( bs.dstBlendAlpha, "DstBlendAlpha", eBlendFactors );
			
			// specific version overrides one
			char bfr[ 32 ];
			char numchr = '0' + brtid;
			sgrx_snprintf( bfr, 32, "ColorWrite[%c]", numchr );
			LoadPassEnum( bs.colorWrite, bfr, eColorWrites );
			sgrx_snprintf( bfr, 32, "BlendEnable[%c]", numchr );
			LoadPassBool( bs.blendEnable, bfr );
			sgrx_snprintf( bfr, 32, "BlendOp[%c]", numchr );
			LoadPassEnum( bs.blendOp, bfr, eBlendOps );
			sgrx_snprintf( bfr, 32, "SrcBlend[%c]", numchr );
			LoadPassEnum( bs.srcBlend, bfr, eBlendFactors );
			sgrx_snprintf( bfr, 32, "DstBlend[%c]", numchr );
			LoadPassEnum( bs.dstBlend, bfr, eBlendFactors );
			sgrx_snprintf( bfr, 32, "BlendOpAlpha[%c]", numchr );
			LoadPassEnum( bs.blendOpAlpha, bfr, eBlendOps );
			sgrx_snprintf( bfr, 32, "SrcBlendAlpha[%c]", numchr );
			LoadPassEnum( bs.srcBlendAlpha, bfr, eBlendFactors );
			sgrx_snprintf( bfr, 32, "DstBlendAlpha[%c]", numchr );
			LoadPassEnum( bs.dstBlendAlpha, bfr, eBlendFactors );
		}
		
		passes.push_back( pass );
	}
	
	return true;
}

XShdInstHandle GR_CreateXShdInstance( const SGRX_XShaderDef& def )
{
	XShdInstHandle h = new SGRX_XShdInst;
	return h;
}





struct RenderPass
{
	bool isShadowPass;
	bool isBasePass;
	int numPL;
	int numSL;
};

void OnMakeRenderState( const RenderPass& pass, const SGRX_Material& mtl, SGRX_RenderState& out )
{
	out.cullMode = mtl.flags & SGRX_MtlFlag_Nocull ? SGRX_RS_CullMode_None : SGRX_RS_CullMode_Back;
	
	if( pass.isShadowPass == false )
	{
		bool decal = ( mtl.flags & SGRX_MtlFlag_Decal ) != 0;
		bool ltovr = pass.isBasePass == false && pass.isShadowPass == false;
		out.depthBias = decal ? -1e-5f : 0;
		out.depthWriteEnable = ( ltovr || decal || mtl.blendMode != SGRX_MtlBlend_None ) == false;
		out.blendStates[ 0 ].blendEnable = ltovr || mtl.blendMode != SGRX_MtlBlend_None;
		if( ltovr || mtl.blendMode == SGRX_MtlBlend_Additive )
		{
			out.blendStates[ 0 ].srcBlend = SGRX_RS_Blend_SrcAlpha;
			out.blendStates[ 0 ].dstBlend = SGRX_RS_Blend_One;
		}
		else if( mtl.blendMode == SGRX_MtlBlend_Multiply )
		{
			out.blendStates[ 0 ].srcBlend = SGRX_RS_Blend_Zero;
			out.blendStates[ 0 ].dstBlend = SGRX_RS_Blend_SrcColor;
		}
		else
		{
			out.blendStates[ 0 ].srcBlend = SGRX_RS_Blend_SrcAlpha;
			out.blendStates[ 0 ].dstBlend = SGRX_RS_Blend_InvSrcAlpha;
		}
	}
	else
	{
		out.depthBias = 1e-5f;
		out.slopeDepthBias = 0.5f;
		out.cullMode = SGRX_RS_CullMode_None;
	}
}

void OnLoadMtlShaders( const RenderPass& pass,
	const StringView& defines, const SGRX_Material& mtl,
	const SGRX_MeshInstance* MI, VertexShaderHandle& VS, PixelShaderHandle& PS )
{
	if( pass.isBasePass == false && pass.isShadowPass == false &&
		( ( mtl.flags & SGRX_MtlFlag_Unlit ) != 0 || MI->GetLightingMode() == SGRX_LM_Unlit ) )
	{
		PS = NULL;
		VS = NULL;
		return;
	}
	
	String name = "mtl:";
	name.append( mtl.shader );
	name.append( ":" );
	name.append( "sys_lighting" ); // pass.shader );
	
	if( pass.isShadowPass )
		name.append( ":SHADOW_PASS" );
	else
	{
		char bfr[32];
		// lighting mode
		{
			sgrx_snprintf( bfr, 32, "%d", MI->GetLightingMode() );
			name.append( ":LMODE " );
			name.append( bfr );
		}
		if( pass.isBasePass )
			name.append( ":BASE_PASS" );
		if( pass.numPL )
		{
			sgrx_snprintf( bfr, 32, "%d", pass.numPL );
			name.append( ":NUM_POINTLIGHTS " );
			name.append( bfr );
		}
		if( pass.numSL )
		{
			sgrx_snprintf( bfr, 32, "%d", pass.numSL );
			name.append( ":NUM_SPOTLIGHTS " );
			name.append( bfr );
		}
	}
	
	// misc. parameters
	name.append( ":" );
	name.append( defines ); // scene defines
	
	if( mtl.flags & SGRX_MtlFlag_VCol )
		name.append( ":VCOL" ); // color multiplied by vertex color
	if( mtl.flags & SGRX_MtlFlag_Decal )
		name.append( ":DECAL" ); // ???
	
	PS = GR_GetPixelShader( name );
	
	if( MI->IsSkinned() )
		name.append( ":SKIN" );
	
	VS = GR_GetVertexShader( name );
}



XShdInstHandle GR_CreateXShdInstance( const SGRX_MeshInstance* MI, const SGRX_Material& mtl )
{
	static const RenderPass passes[] =
	{
		{ true, false, 0, 0 }, // shadow pass
		{ false, true, 16, 0 }, // base + 16 point lights
		{ false, false, 16, 0 }, // 16 point lights
		{ false, false, 0, 2 }, // 2 spotlights
	};
	
	XShdInstHandle h = new SGRX_XShdInst;
	
	for( int i = 0; i < (int) SGRX_ARRAY_SIZE( passes ); ++i )
	{
		SGRX_XShdInst::Pass pass;
		pass.order = i * 1000 + 1500;
		
		// load render state
		SGRX_RenderState rs;
		rs.Init();
		OnMakeRenderState( passes[ i ], mtl, rs );
		pass.renderState = GR_GetRenderState( rs );
		
		// load shaders
		OnLoadMtlShaders( passes[ i ], "", mtl, MI,
			pass.vertexShader, pass.pixelShader );
		
		pass.vtxInputMap = GR_GetVertexInputMapping(
			pass.vertexShader, MI->GetMesh()->m_vertexDecl );
		
		h->passes.push_back( pass );
	}
	
	return h;
}


SGRX_MeshInstance::SGRX_MeshInstance( SGRX_Scene* s ) :
	_scene( s ),
	raycastOverride( NULL ),
	userData( NULL ),
	m_lightingMode( SGRX_LM_Dynamic ),
	layers( 0x1 ),
	enabled( true ),
	allowStaticDecals( false ),
	sortidx( 0 ),
	m_invalid( true )
{
	matrix = Mat4::Identity;
	for( int i = 0; i < MAX_MI_CONSTANTS; ++i )
		constants[ i ] = Vec4::Create( 0 );
}

SGRX_MeshInstance::~SGRX_MeshInstance()
{
	if( _scene )
	{
		_scene->m_meshInstances.unset( this );
	}
}

void SGRX_MeshInstance::SetTransform( const Mat4& mtx )
{
	matrix = mtx;
}

void SGRX_MeshInstance::_Precache()
{
	LOG_FUNCTION_ARG( "SGRX_MeshInstance" );
	
	size_t dicnt = TMIN( materials.size(), m_mesh->m_meshParts.size() );
	if( m_invalid || m_drawItems.size() != dicnt )
	{
		m_drawItems.resize( dicnt );
		for( size_t i = 0; i < dicnt; ++i )
		{
			const SGRX_Material& mtl = materials[ i ];
			uint8_t type = 0;
			if( mtl.flags & SGRX_MtlFlag_Decal )
				type = SGRX_TY_Decal;
			else if( mtl.blendMode != SGRX_MtlBlend_None )
				type = SGRX_TY_Transparent;
			else
				type = SGRX_TY_Solid;
			m_drawItems[ i ].MI = this;
			m_drawItems[ i ].XSH = GR_CreateXShdInstance( this, materials[ i ] );
			m_drawItems[ i ].part = i;
			m_drawItems[ i ].type = type;
			m_drawItems[ i ]._lightbuf_begin = NULL;
			m_drawItems[ i ]._lightbuf_end = NULL;
		}
		
		m_invalid = false;
	}
}

void SGRX_MeshInstance::SetMesh( StringView path, bool mtls )
{
	LOG_FUNCTION_ARG(mtls?"[path] MTL":"[path] nomtl");
	
	SetMesh( GR_GetMesh( path ), mtls );
}

void SGRX_MeshInstance::SetMesh( MeshHandle mh, bool mtls )
{
	LOG_FUNCTION_ARG(mtls?"MTL":"nomtl");
	
	m_mesh = mh;
	if( mtls )
	{
		if( mh )
		{
			materials.resize( mh->m_meshParts.size() );
			for( size_t i = 0; i < mh->m_meshParts.size(); ++i )
			{
				const SGRX_MeshPart& MP = mh->m_meshParts[ i ];
				SGRX_Material& M = materials[ i ];
				
				M.shader = MP.shader;
				M.blendMode = MP.mtlBlendMode;
				M.flags = MP.mtlFlags;
				for( size_t t = 0; t < SGRX_MAX_MESH_TEXTURES; ++t )
				{
					M.textures[ t ] = MP.textures[ t ].size() ? GR_GetTexture( MP.textures[ t ] ) : NULL;
				}
			}
		}
		else
		{
			materials.resize( 0 );
		}
	}
	OnUpdate();
}


uint32_t SGRX_FindOrAddVertex( ByteArray& vertbuf, size_t searchoffset, size_t& writeoffset, const uint8_t* vertex, size_t vertsize )
{
	const size_t idxoffset = 0;
	for( size_t i = searchoffset; i < writeoffset; i += vertsize )
	{
		if( 0 == memcmp( &vertbuf[ i ], vertex, vertsize ) )
			return ( i - idxoffset ) / vertsize;
	}
	uint32_t out = ( writeoffset - idxoffset ) / vertsize;
	memcpy( &vertbuf[ writeoffset ], vertex, vertsize );
	writeoffset += vertsize;
	return out;
}

void SGRX_DoIndexTriangleMeshVertices( UInt32Array& indices, ByteArray& vertices, size_t offset, size_t stride )
{
#if 1
	while( offset < vertices.size() )
	{
		indices.push_back( offset / stride );
		offset += stride;
	}
	return;
#endif
	// <= 1 tri
	if( vertices.size() <= offset + stride * 3 )
		return;
	
	uint8_t trivertdata[ 256 * 3 ];
	size_t end = ( ( vertices.size() - offset ) / (stride*3) ) * stride * 3 + offset;
	size_t writeoffset = offset;
	size_t readoffset = offset;
	while( readoffset < end )
	{
		// extract a triangle
		memcpy( trivertdata, &vertices[ readoffset ], stride * 3 );
		readoffset += stride * 3;
		
		// insert each vertex/index
		uint32_t idcs[3] =
		{
			SGRX_FindOrAddVertex( vertices, offset, writeoffset, trivertdata, stride ),
			SGRX_FindOrAddVertex( vertices, offset, writeoffset, trivertdata + stride, stride ),
			SGRX_FindOrAddVertex( vertices, offset, writeoffset, trivertdata + stride * 2, stride ),
		};
		indices.append( idcs, 3 );
	}
	// remove unused data
	vertices.resize( writeoffset );
}

SGRX_ProjectionMeshProcessor::SGRX_ProjectionMeshProcessor( ByteArray* verts, UInt32Array* indices, const Mat4& mtx, float zn2zf ) :
	outVertices( verts ), outIndices( indices ), viewProjMatrix( mtx ), invZNearToZFar( safe_fdiv( 1.0f, zn2zf ) )
{
}

void SGRX_ProjectionMeshProcessor::Process( void* data )
{
	LOG_FUNCTION;
	
	SGRX_CAST( SGRX_MeshInstance*, MI, data );
	
	SGRX_IMesh* M = MI->GetMesh();
	if( M )
	{
		size_t vertoff = outVertices->size();
		M->Clip( MI->matrix, viewProjMatrix, *outVertices, true, invZNearToZFar );
		SGRX_DoIndexTriangleMeshVertices( *outIndices, *outVertices, vertoff, sizeof(SGRX_Vertex_Decal) );
	}
}


SceneRaycastCallback_Any::SceneRaycastCallback_Any() : m_hit(false)
{
}

void SceneRaycastCallback_Any::AddResult( SceneRaycastInfo* )
{
	m_hit = true;
}

SceneRaycastCallback_Closest::SceneRaycastCallback_Closest() : m_hit(false)
{
	SceneRaycastInfo srci = { 1.0f + SMALL_FLOAT, V3(0), 0, 0, -1, -1, -1, NULL };
	m_closest = srci;
}

void SceneRaycastCallback_Closest::AddResult( SceneRaycastInfo* info )
{
	m_hit = true;
	if( info->factor < m_closest.factor )
		m_closest = *info;
}

SceneRaycastCallback_Sorting::SceneRaycastCallback_Sorting( Array< SceneRaycastInfo >* sortarea )
	: m_sortarea( sortarea )
{
	sortarea->clear();
}

void SceneRaycastCallback_Sorting::AddResult( SceneRaycastInfo* info )
{
	if( m_sortarea->size() == 0 )
	{
		m_sortarea->push_back( *info );
		return;
	}
	
	int lowerBound = 0;
	int upperBound = m_sortarea->size() - 1;
	int pos = 0;
	for(;;)
	{
		pos = (upperBound + lowerBound) / 2;
		if( m_sortarea->at(pos).factor == info->factor )
		{
			break;
		}
		else if( m_sortarea->at(pos).factor < info->factor )
		{
			lowerBound = pos + 1;
			if( lowerBound > upperBound )
			{
				pos++;
				break;
			}
		}
		else
		{
			upperBound = pos - 1;
			if( lowerBound > upperBound )
				break;
		}
	}
	
	m_sortarea->insert( pos, *info );
}



SGRX_Scene::SGRX_Scene() :
	debugDrawFlags( 0 ),
	director( GR_GetDefaultRenderDirector() ),
	cullScene( NULL ),
	frontCCW( false ),
	fogColor( Vec3::Create( 0.5 ) ),
	fogHeightFactor( 0 ),
	fogDensity( 0.01f ),
	fogHeightDensity( 0 ),
	fogStartHeight( 0.01f ),
	fogMinDist( 0 ),
	clearColor( 0x00111111 ),
	ambientLightColor( Vec3::Create( 0.1f ) ),
	dirLightColor( Vec3::Create( 0.8f ) ),
	dirLightDir( Vec3::Create( -1 ).Normalized() )
{
	camera.position = Vec3::Create( 10, 10, 10 );
	camera.direction = -camera.position.Normalized();
	camera.updir = Vec3::Create( 0, 0, 1 );
	camera.angle = 90;
	camera.aspect = 1;
	camera.aamix = 0.5f;
	camera.znear = 1;
	camera.zfar = 1000;
	camera.UpdateMatrices();
	
	m_projMeshInst = CreateMeshInstance();
	m_projMeshInst->sortidx = 255;
}

SGRX_Scene::~SGRX_Scene()
{
	m_projMeshInst = NULL;
	
	if( VERBOSE ) LOG << "Deleted scene: " << this;
}

MeshInstHandle SGRX_Scene::CreateMeshInstance()
{
	SGRX_MeshInstance* mi = new SGRX_MeshInstance( this );
	m_meshInstances.set( mi, NoValue() );
	return mi;
}

LightHandle SGRX_Scene::CreateLight()
{
	SGRX_Light* lt = new SGRX_Light( this );
	m_lights.set( lt, NoValue() );
	return lt;
}


struct CubemapRenderer
{
	CubemapRenderer()
	{
		GR_GetCubemapVectors( fwd, up );
		m_vertexDecl = GR_GetVertexDecl( "pf3" );
		m_vertexShader = GR_GetVertexShader( "sys_cubemap_render" );
		m_pixelShader = GR_GetPixelShader( "sys_cubemap_render" );
		m_vtxInputMap = GR_GetVertexInputMapping( m_vertexShader, m_vertexDecl );
		
		SGRX_RenderState rsdesc;
		rsdesc.Init();
		rsdesc.depthEnable = false;
		rsdesc.multisampleEnable = false;
		rsdesc.cullMode = SGRX_RS_CullMode_None;
		m_renderStateCopy = GR_GetRenderState( rsdesc );
		rsdesc.blendStates[0].blendEnable = true;
		rsdesc.blendStates[0].srcBlend = SGRX_RS_Blend_One;
		rsdesc.blendStates[0].dstBlend = SGRX_RS_Blend_One;
		m_renderStateAdd = GR_GetRenderState( rsdesc );
	}
	void Transfer( TextureHandle target, int tgtMip, TextureHandle source )
	{
		Mat4 viewMtx = GR2D_GetBatchRenderer().viewMatrix;
		SGRX_RTClearInfo clearInfo = { SGRX_RT_ClearColor, 0, COLOR_RGB(200,50,200), 0 };
		for( int side = 0; side < 6; ++side )
		{
			static_cast<SGRX_IRenderControl*>(g_Renderer)->SetRenderTargets(
				NULL, clearInfo, SGRX_RTSpec( target, side, tgtMip ) );
			
			int width = target.GetInfo().width / int(pow( 2, tgtMip ));
			GR2D_SetViewport( 0, 0, width, width );
			GR2D_SetViewMatrix(
				Mat4::CreateLookAt( V3(0), fwd[ side ], up[ side ] ) *
				Mat4::CreateScale( -1, 1, 1 ) *
				Mat4::CreatePerspective( 90, 1, 0, 0.01f, 100.0f ) );
			_DrawCubemapSide( side, false, source );
		}
		GR2D_SetViewMatrix( viewMtx );
		GR2D_UnsetViewport();
	}
	void _DrawCubemapSide( int side, bool additive, TextureHandle source )
	{
		Vec3 ydir = -up[ side ];
		Vec3 zdir = fwd[ side ];
		Vec3 xdir = Vec3Cross( ydir, zdir ).Normalized();
		
		Vec3 cubemapVerts[4] =
		{
			zdir - xdir - ydir,
			zdir + xdir - ydir,
			zdir - xdir + ydir,
			zdir + xdir + ydir,
		};
		
		SGRX_ImmDrawData immdd =
		{
			cubemapVerts, SGRX_ARRAY_SIZE(cubemapVerts),
			PT_TriangleStrip,
			m_vertexDecl,
			m_vtxInputMap,
			m_vertexShader,
			m_pixelShader,
			additive ? m_renderStateAdd : m_renderStateCopy,
			NULL, 0, // shader data
		};
		immdd.textures[ 0 ] = source;
		g_Renderer->DrawImmediate( immdd );
	}
	void _DrawCubemap( bool additive, TextureHandle source )
	{
		for( int side = 0; side < 6; ++side )
			_DrawCubemapSide( side, additive, source );
	}
	
	Vec3 fwd[6], up[6];
	
	VertexDeclHandle m_vertexDecl;
	VertexShaderHandle m_vertexShader;
	PixelShaderHandle m_pixelShader;
	VtxInputMapHandle m_vtxInputMap;
	RenderStateHandle m_renderStateCopy;
	RenderStateHandle m_renderStateAdd;
};

TextureHandle SGRX_Scene::CreateCubemap( int size, Vec3 pos )
{
	TextureHandle cubemap = GR_CreateCubeRenderTexture( size, TEXFMT_RT_COLOR_HDR16, SGRX_MIPS_ALL );
	
	CubemapRenderer cmr;
	
	// render scene to cubemap
	TextureHandle lod0map = GR_CreateCubeRenderTexture( size, TEXFMT_RT_COLOR_HDR16, 1 );
	SGRX_Camera bkCam = camera;
	frontCCW = !frontCCW;
	for( int i = 0; i < 6; ++i )
	{
		camera.position = pos;
		camera.direction = cmr.fwd[ i ];
		camera.updir = cmr.up[ i ];
		camera.angle = 90;
		camera.aspect = 1;
		camera.aamix = 0;
		camera.znear = 0.01f;
		camera.zfar = 10000.0f;
		camera.UpdateMatrices();
		camera.mProj = Mat4::CreateScale(-1,1,1) * camera.mProj;
		
		// render side into lod0
		SGRX_RenderScene rs( V4(0), this, false, SGRX_RTSpec( lod0map, i ) );
		GR_RenderScene( rs );
	}
	frontCCW = !frontCCW;
	camera = bkCam;
	
	// render lod0 into output cubemap
	cmr.Transfer( cubemap, 0, lod0map );
	
	// convolve the cubemap
	TextureHandle prevLODMap = lod0map;
	int miplevels = GR_CalcMipCount( size );
	for( int mip = 1; mip < miplevels; ++mip )
	{
		int lodSize = size / int(pow( 2, mip ));
		TextureHandle curLODMap = GR_CreateCubeRenderTexture( lodSize, TEXFMT_RT_COLOR_HDR16, 1 );
		TextureHandle curAccMap = GR_CreateCubeRenderTexture( lodSize, TEXFMT_RT_COLOR_HDR16, 1 );
		
		for( int i = 0; i < 6; ++i )
		{
			// render cubemap with different offsets into accumulator
			cmr.Transfer( curAccMap, 0, prevLODMap );
			
			// resolve accumulator into current LOD
			cmr.Transfer( curLODMap, 0, curAccMap );
		}
		
		// render lodN into output cubemap
		cmr.Transfer( cubemap, mip, curLODMap );
		
		prevLODMap = curLODMap;
	}
	
	return cubemap;
}


void SGRX_Scene::OnUpdate()
{
	for( size_t i = 0; i < m_meshInstances.size(); ++i )
		m_meshInstances.item( i ).key->OnUpdate();
}

bool SGRX_Scene::RaycastAny( const Vec3& from, const Vec3& to, uint32_t layers )
{
	SceneRaycastCallback_Any cb;
	RaycastAll( from, to, &cb, layers );
	return cb.m_hit;
}

bool SGRX_Scene::RaycastOne( const Vec3& from, const Vec3& to, SceneRaycastInfo* outinfo, uint32_t layers )
{
	SceneRaycastCallback_Closest cb;
	RaycastAll( from, to, &cb, layers );
	if( cb.m_hit )
		*outinfo = cb.m_closest;
	return cb.m_hit;
}

void SGRX_Scene::RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, uint32_t layers )
{
	// TODO: broadphase
//	double A = sgrx_hqtime();
	Mat4 inv;
	for( size_t i = 0; i < m_meshInstances.size(); ++i )
	{
		SGRX_MeshInstance* mi = m_meshInstances.item( i ).key;
		if( mi->enabled && mi->GetMesh() &&
			( mi->layers & layers ) && mi->matrix.InvertTo( inv ) )
		{
			Vec3 tffrom = inv.TransformPos( from );
			Vec3 tfto = inv.TransformPos( to );
			if( SegmentAABBIntersect( tffrom, tfto, mi->GetMesh()->m_boundsMin, mi->GetMesh()->m_boundsMax ) )
			{
				IMeshRaycast* mrc = mi->raycastOverride ? mi->raycastOverride : mi->GetMesh();
				mrc->RaycastAll( tffrom, tfto, cb, mi );
			}
		}
	}
//	double B = sgrx_hqtime();
//	LOG << "RaycastAll: " << (B-A)*1000 << " ms";
}

void SGRX_Scene::RaycastAllSort( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, uint32_t layers, Array< SceneRaycastInfo >* tmpstore )
{
	Array< SceneRaycastInfo > mystore;
	if( !tmpstore )
		tmpstore = &mystore;
	SceneRaycastCallback_Sorting mycb( tmpstore );
	RaycastAll( from, to, &mycb, layers );
	for( size_t i = 0; i < tmpstore->size(); ++i )
	{
		cb->AddResult( &tmpstore->at(i) );
	}
}

void SGRX_Scene::GatherMeshes( const SGRX_Camera& cam, IProcessor* meshInstProc, uint32_t layers )
{
	// TODO use cullscene
	for( size_t i = 0; i < m_meshInstances.size(); ++i )
	{
		SGRX_MeshInstance* mi = m_meshInstances.item( i ).key;
		if( mi->layers & layers )
			meshInstProc->Process( mi );
	}
}

void SGRX_Scene::GenerateProjectionMesh( const SGRX_Camera& cam, ByteArray& outverts, UInt32Array& outindices, uint32_t layers )
{
	SGRX_ProjectionMeshProcessor pmp( &outverts, &outindices, cam.mView * cam.mProj, cam.zfar - cam.znear );
	GatherMeshes( cam, &pmp, layers );
}

void SGRX_Scene::DebugDraw_MeshRaycast( uint32_t layers )
{
	for( size_t i = 0; i < m_meshInstances.size(); ++i )
	{
		SGRX_MeshInstance* mi = m_meshInstances.item( i ).key;
		if( mi->layers & layers )
		{
			IMeshRaycast* imrc = mi->raycastOverride ? mi->raycastOverride : mi->GetMesh();
			imrc->MRC_DebugDraw( mi );
		}
	}
}


size_t SGRX_SceneTree::FindNodeIDByName( const StringView& name )
{
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		const Node& N = nodes[ i ];
		if( N.name == name )
			return i;
	}
	return NOT_FOUND;
}

size_t SGRX_SceneTree::FindNodeIDByPath( const StringView& path )
{
	StringView it;
	bool rooted = it.ch() == '/';
	it.skip( 1 );
	
	size_t pos = 0;
	while( it )
	{
		StringView curr = it.until( "/" );
		it.skip( curr.size() + 1 );
	}
	UNUSED( rooted ); // TODO
	UNUSED( pos );
	return _NormalizeIndex( 0 );
}

void SGRX_SceneTree::UpdateTransforms()
{
	transforms.resize( nodes.size() );
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		const Node& N = nodes[ i ];
		if( N.parent_id < i )
			transforms[ i ] = N.transform * transforms[ N.parent_id ];
		else
			transforms[ i ] = N.transform;
	}
	for( size_t i = 0; i < items.size(); ++i )
	{
		Item& I = items[ i ];
		I.item->SetTransform( I.node_id < nodes.size() ? transforms[ I.node_id ] : Mat4::Identity );
	}
}


int GR_CalcMipCount( int width, int height, int depth )
{
	int v = TMAX( width, TMAX( height, depth ) );
	int count = 0;
	while( v > 0 )
	{
		v /= 2;
		count++;
	}
	return count;
}

TextureHandle GR_CreateTexture( int width, int height, SGRX_TextureFormat format, uint32_t flags, int mips, const void* data )
{
	LOG_FUNCTION;
	
	TextureInfo ti = { TEXTYPE_2D, mips, width, height, 1, format, flags };
	SGRX_ITexture* tex = g_Renderer->CreateTexture( &ti, data );
	if( !tex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	tex->m_info = ti;
	
	if( VERBOSE )
		LOG << "Created 2D texture: " << width << "x" << height
		<< ", format=" << format << ", mips=" << mips;
	return tex;
}

TextureHandle GR_CreateTexture3D( int width, int height, int depth, SGRX_TextureFormat format, uint32_t flags, int mips, const void* data )
{
	LOG_FUNCTION;
	
	TextureInfo ti = { TEXTYPE_VOLUME, mips, width, height, depth, format, flags };
	SGRX_ITexture* tex = g_Renderer->CreateTexture( &ti, data );
	if( !tex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	tex->m_info = ti;
	
	if( VERBOSE )
		LOG << "Created 3D texture: " << width << "x" << height << "x" << depth
		<< ", format=" << format << ", mips=" << mips;
	return tex;
}

TextureHandle GR_GetTexture( const StringView& path )
{
	TextureHandle tex;
	LOG_FUNCTION_ARG( path );
	
	tex = g_Textures->getcopy( path );
	if( tex )
		return tex;
	
	double t0 = sgrx_hqtime();
	tex = g_Game->OnCreateSysTexture( path );
	if( !tex )
	{
		// it's a regular texture
		uint32_t usageflags = 0;
		uint8_t lod = 0;
		HFileReader fr = g_Game->OnLoadTexture( path, usageflags, lod );
		if( fr == NULL )
		{
			if( VERBOSE || path != "" )
				LOG_ERROR << LOG_DATE << "  Could not find texture: " << path;
			return TextureHandle();
		}
		
		TextureData texdata;
		if( !TextureData_Load( &texdata, fr, path, lod ) )
		{
			// error is already printed
			return TextureHandle();
		}
		texdata.info.flags = usageflags;
		
		tex = g_Renderer->CreateTexture( &texdata.info, texdata.data.data() );
		if( !tex )
		{
			// error is already printed
			return TextureHandle();
		}
		tex->m_info = texdata.info;
	}
	
	tex->m_key.append( path.data(), path.size() );
	g_Textures->set( tex->m_key, tex );
	
	if( VERBOSE )
		LOG << "Loaded texture: " << path << " (time=" << ( sgrx_hqtime() - t0 ) << ")";
	return tex;
}

TextureHandle GR_CreateRenderTexture( int width, int height, SGRX_TextureFormat format, int mips )
{
	LOG_FUNCTION;
	
	if( mips == SGRX_MIPS_ALL )
		mips = GR_CalcMipCount( width, height );
	else if( mips == 0 )
		mips = 1;
	TextureInfo ti = { TEXTYPE_2D, mips, width, height, 1, format,
		TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y };
	SGRX_ITexture* tex = g_Renderer->CreateRenderTexture( &ti );
	if( !tex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	tex->m_info = ti;
	
	if( VERBOSE )
	{
		LOG << "Created renderable texture: " << width << "x"
			<< height << ", format=" << format << ", mips=" << mips;
	}
	return tex;
}

TextureHandle GR_CreateCubeRenderTexture( int width, SGRX_TextureFormat format, int mips )
{
	LOG_FUNCTION;
	
	if( mips == SGRX_MIPS_ALL )
		mips = GR_CalcMipCount( width );
	else if( mips == 0 )
		mips = 1;
	TextureInfo ti = { TEXTYPE_CUBE, mips, width, width, 1, format,
		TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y };
	SGRX_ITexture* tex = g_Renderer->CreateRenderTexture( &ti );
	if( !tex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	tex->m_info = ti;
	
	if( VERBOSE )
	{
		LOG << "Created renderable cube texture: " << width
			<< ", format=" << format << ", mips=" << mips;
	}
	return tex;
}

TextureHandle GR_GetRenderTarget( int width, int height, SGRX_TextureFormat format, int extra )
{
	ASSERT( width && height && format );
	LOG_FUNCTION;
	
	uint64_t key = (uint64_t(width&0xffff)<<48) | (uint64_t(height&0xffff)<<32) | (uint64_t(format&0xffff)<<16) | (uint64_t(extra&0xffff));
	SGRX_ITexture* rtt = g_RenderTargets->getcopy( key );
	if( rtt )
		return rtt;
	
	TextureHandle rtth = GR_CreateRenderTexture( width, height, format );
	rtth->m_rtkey = key;
	g_RenderTargets->set( key, rtth );
	return rtth;
}

DepthStencilSurfHandle GR_CreateDepthStencilSurface( int width, int height, SGRX_TextureFormat format )
{
	ASSERT( width && height && format );
	LOG_FUNCTION;
	
	SGRX_IDepthStencilSurface* dss = g_Renderer->CreateDepthStencilSurface( width, height, format );
	if( dss == NULL )
	{
		// valid outcome
		return NULL;
	}
	dss->m_width = width;
	dss->m_height = height;
	dss->m_format = format;
	
	if( VERBOSE ) LOG << "Created depth/stencil surface: " << width << "x" << height << ", format=" << format;
	return dss;
}

DepthStencilSurfHandle GR_GetDepthStencilSurface( int width, int height, SGRX_TextureFormat format, int extra )
{
	ASSERT( width && height && format );
	LOG_FUNCTION;
	
	uint64_t key = (uint64_t(width&0xffff)<<48) | (uint64_t(height&0xffff)<<32) | (uint64_t(format&0xffff)<<16) | (uint64_t(extra&0xffff));
	SGRX_IDepthStencilSurface* dss = g_DepthStencilSurfs->getcopy( key );
	if( dss )
		return dss;
	
	DepthStencilSurfHandle dssh = GR_CreateDepthStencilSurface( width, height, format );
	if( dssh == NULL )
	{
		// valid outcome
		return NULL;
	}
	dssh->m_key = key;
	g_DepthStencilSurfs->set( key, dssh );
	return dssh;
}


VertexShaderHandle GR_GetVertexShader( const StringView& path )
{
	LOG_FUNCTION_ARG( path );
	
	String code;
	String errors;
	ByteArray comp;
	
	SGRX_IVertexShader* shd = g_VertexShaders->getcopy( path );
	if( shd )
		return shd;
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( g_Game->GetCompiledShader( g_Renderer->GetInfo(), ".vs", path, comp ) )
		{
			goto has_compiled_shader;
		}
	}
	
	if( !g_Game->OnLoadShader( g_Renderer->GetInfo(), path, code ) )
	{
		LOG_ERROR << LOG_DATE << "  Could not find vertex shader: " << path;
		return VertexShaderHandle();
	}
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( !g_Renderer->CompileShader( path, ShaderType_Vertex, code, comp, errors ) )
		{
			LOG_ERROR << LOG_DATE << "  Failed to compile vertex shader: " << path;
			LOG << errors;
			LOG << "---";
			return VertexShaderHandle();
		}
		
		g_Game->SetCompiledShader( g_Renderer->GetInfo(), ".vs", path, comp );
		
has_compiled_shader:
		shd = g_Renderer->CreateVertexShader( path, comp );
	}
	else
	{
		// TODO: I know...
		ByteArray bcode;
		bcode.resize( code.size() );
		memcpy( bcode.data(), code.data(), code.size() );
		shd = g_Renderer->CreateVertexShader( path, bcode );
	}
	
	if( !shd )
	{
		// error already printed in renderer
		return NULL;
	}
	
	shd->m_key = path;
	g_VertexShaders->set( shd->m_key, shd );
	
	if( VERBOSE ) LOG << "Loaded vertex shader: " << path;
	return shd;
}

PixelShaderHandle GR_GetPixelShader( const StringView& path )
{
	LOG_FUNCTION_ARG( path );
	
	String code;
	String errors;
	ByteArray comp;
	
	SGRX_IPixelShader* shd = g_PixelShaders->getcopy( path );
	if( shd )
		return shd;
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( g_Game->GetCompiledShader( g_Renderer->GetInfo(), ".ps", path, comp ) )
		{
			goto has_compiled_shader;
		}
	}
	
	if( !g_Game->OnLoadShader( g_Renderer->GetInfo(), path, code ) )
	{
		LOG_ERROR << LOG_DATE << "  Could not find pixel shader: " << path;
		return PixelShaderHandle();
	}
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( !g_Renderer->CompileShader( path, ShaderType_Pixel, code, comp, errors ) )
		{
			LOG_ERROR << LOG_DATE << "  Failed to compile pixel shader: " << path;
			LOG << errors;
			LOG << "---";
			return PixelShaderHandle();
		}
		
		g_Game->SetCompiledShader( g_Renderer->GetInfo(), ".ps", path, comp );
		
has_compiled_shader:
		shd = g_Renderer->CreatePixelShader( path, comp );
	}
	else
	{
		// TODO: I know...
		ByteArray bcode;
		bcode.resize( code.size() );
		memcpy( bcode.data(), code.data(), code.size() );
		shd = g_Renderer->CreatePixelShader( path, bcode );
	}
	
	if( !shd )
	{
		// error already printed in renderer
		return NULL;
	}
	
	shd->m_key = path;
	g_PixelShaders->set( shd->m_key, shd );
	
	if( VERBOSE ) LOG << "Loaded pixel shader: " << path;
	return shd;
}


RenderStateHandle GR_GetRenderState( const SGRX_RenderState& state )
{
	LOG_FUNCTION;
	
	SGRX_IRenderState* rs = g_RenderStates->getcopy( state );
	if( rs )
		return rs;
	
	rs = g_Renderer->CreateRenderState( state );
	rs->m_info = state;
	g_RenderStates->set( state, rs );
	
	if( VERBOSE ) LOG << "Created render state";
	return rs;
}


VertexDeclHandle GR_GetVertexDecl( const StringView& vdecl )
{
	LOG_FUNCTION_ARG( vdecl );
	
	SGRX_IVertexDecl* VD = g_VertexDecls->getcopy( vdecl );
	if( VD )
		return VD;
	
	VDeclInfo vdinfo = {0};
	const char* err = VDeclInfo_Parse( &vdinfo, StackString< 64 >( vdecl ) );
	if( err )
	{
		LOG_ERROR << LOG_DATE << "  Failed to parse vertex declaration - " << err << " (" << vdecl << ")";
		return NULL;
	}
	
	VD = g_Renderer->CreateVertexDecl( vdinfo );
	if( !VD )
	{
		// error already printed in renderer
		return NULL;
	}
	
	VD->m_key = vdecl;
	g_VertexDecls->set( VD->m_key, VD );
	
	if( VERBOSE ) LOG << "Created vertex declaration: " << vdecl;
	return VD;
}


VtxInputMapHandle GR_GetVertexInputMapping( SGRX_IVertexShader* vs, SGRX_IVertexDecl* vd )
{
	LOG_FUNCTION;
	
	if( vs == NULL || vd == NULL )
		return NULL;
	
	SGRX_VtxInputMapKey key = { vs, vd };
	SGRX_IVertexInputMapping* vim = g_VtxInputMaps->getcopy( key );
	if( vim )
		return vim;
	
	vim = g_Renderer->CreateVertexInputMapping( vs, vd );
	if( vim == NULL )
	{
		// valid outcome
		return vim;
	}
	
	vim->m_key = key;
	g_VtxInputMaps->set( key, vim );
	
	if( VERBOSE ) LOG << "Created vertex input mapping";
	return vim;
}


MeshHandle GR_CreateMesh()
{
	LOG_FUNCTION;
	
	SGRX_IMesh* mesh = g_Renderer->CreateMesh();
	return mesh;
}

MeshHandle GR_GetMesh( const StringView& path, bool dataonly )
{
	LOG_FUNCTION_ARG( path );
	
	MeshHandle mesh = g_Meshes->getcopy( path );
	if( mesh )
		return mesh;
	
	double t0 = sgrx_hqtime();
	mesh = g_Game->OnCreateSysMesh( path );
	if( mesh )
	{
		if( dataonly == false )
		{
			mesh->GenerateTriangleTree();
		}
		
		mesh->m_key = path;
		g_Meshes->set( mesh->m_key, mesh );
		if( VERBOSE ) LOG << "Created sys. mesh: " << path << " (time=" << ( sgrx_hqtime() - t0 ) << ")";
		return mesh;
	}
	
	ByteArray meshdata;
	if( !g_Game->OnLoadMesh( path, meshdata ) )
	{
		LOG_ERROR << LOG_DATE << "  Failed to access mesh data file - " << path;
		return NULL;
	}
	
	MeshFileData mfd;
	const char* err = MeshData_Parse( (char*) meshdata.data(), meshdata.size(), &mfd );
	if( err )
	{
		LOG_ERROR << LOG_DATE << "  Failed to parse mesh file - " << err;
		return NULL;
	}
	
	SGRX_MeshBone bones[ SGRX_MAX_MESH_BONES ];
	for( int i = 0; i < mfd.numBones; ++i )
	{
		MeshFileBoneData* mfdb = &mfd.bones[ i ];
		bones[ i ].name.append( mfdb->boneName, mfdb->boneNameSize );
		bones[ i ].boneOffset = mfdb->boneOffset;
		bones[ i ].parent_id = mfdb->parent_id == 255 ? -1 : mfdb->parent_id;
	}
	
	VertexDeclHandle vdh;
	mesh = g_Renderer->CreateMesh();
	if( !mesh ||
		!( vdh = GR_GetVertexDecl( StringView( mfd.formatData, mfd.formatSize ) ) ) ||
		( dataonly == false && !mesh->SetVertexData( mfd.vertexData, mfd.vertexDataSize, vdh ) ) ||
		( dataonly == false && !mesh->SetIndexData( mfd.indexData, mfd.indexDataSize, ( mfd.dataFlags & MDF_INDEX_32 ) != 0 ) ) ||
		!mesh->SetBoneData( bones, mfd.numBones ) )
	{
		LOG << "...while trying to create mesh: " << path;
		delete mesh;
		return NULL;
	}
	mesh->m_vertexDecl = vdh; // in case dataonly = false
	
	mesh->m_dataFlags = mfd.dataFlags;
	mesh->m_boundsMin = mfd.boundsMin;
	mesh->m_boundsMax = mfd.boundsMax;
	
	mesh->m_meshParts.clear();
	mesh->m_meshParts.resize( mfd.numParts );
	SGRX_MeshPart* parts = mesh->m_meshParts.data();
	for( int i = 0; i < mfd.numParts; ++i )
	{
		SGRX_MeshPart& P = parts[ i ];
		P.vertexOffset = mfd.parts[ i ].vertexOffset;
		P.vertexCount = mfd.parts[ i ].vertexCount;
		P.indexOffset = mfd.parts[ i ].indexOffset;
		P.indexCount = mfd.parts[ i ].indexCount;
		
		StringView mtltext( mfd.parts[ i ].materialStrings[0], mfd.parts[ i ].materialStringSizes[0] );
		
		// LOAD MATERIAL
		//
		P.mtlFlags = mfd.parts[ i ].flags;
		P.mtlBlendMode = mfd.parts[ i ].blendMode;
		if( mfd.parts[ i ].materialStringSizes[0] >= SHADER_NAME_LENGTH )
		{
			LOG_WARNING << "Shader name for part " << i << " is too long";
		}
		else
		{
			P.shader = mtltext;
		}
		for( int tid = 0; tid < mfd.parts[ i ].materialTextureCount; ++tid )
		{
			P.textures[ tid ] = StringView( mfd.parts[ i ].materialStrings[ tid + 1 ], mfd.parts[ i ].materialStringSizes[ tid + 1 ] );
		}
		
		if( mfd.dataFlags & MDF_PARTNAMES )
		{
			P.name.assign( mfd.parts[ i ].nameStr, mfd.parts[ i ].nameStrSize );
			P.nodeTransform = mfd.parts[ i ].nodeTransform;
		}
	}
	
	mesh->m_vdata.append( (const uint8_t*) mfd.vertexData, mfd.vertexDataSize );
	mesh->m_idata.append( (const uint8_t*) mfd.indexData, mfd.indexDataSize );
	
	if( dataonly == false )
	{
		mesh->GenerateTriangleTree();
	}
	
	mesh->m_key = path;
	g_Meshes->set( mesh->m_key, mesh );
	if( VERBOSE ) LOG << "Loaded mesh: " << path << " (time=" << ( sgrx_hqtime() - t0 ) << ")";;
	return mesh;
}



static void _LoadAnimBundle( const StringView& path, const StringView& prefix )
{
	LOG_FUNCTION;
	
	SGRX_AnimBundle bundle;
	if( !GR_ReadAnimBundle( path, bundle ) )
		return;
	
	for( size_t i = 0; i < bundle.anims.size(); ++i )
	{
		SGRX_Animation* anim = bundle.anims[ i ];
		
		anim->m_key.insert( 0, prefix );		
		g_Anims->set( anim->m_key, anim );
	}
	
	if( VERBOSE ) LOG << "Loaded " << bundle.anims.size() << " anims from anim.bundle " << path;
}

AnimHandle GR_GetAnim( const StringView& name )
{
	if( !name )
		return NULL;
	AnimHandle out = g_Anims->getcopy( name );
	if( out )
		return out;
	
	StringView bundle = name.until( ":" );
	if( name.size() == bundle.size() )
	{
		LOG_ERROR << LOG_DATE << "  Failed to request animation: " << name
			<< " - invalid name (expected <bundle>:<anim>)";
		return NULL;
	}
	
	_LoadAnimBundle( bundle, name.part( 0, bundle.size() + 1 ) );
	
	return g_Anims->getcopy( name );
}

AnimCharHandle GR_GetAnimChar( const StringView& name )
{
	LOG_FUNCTION_ARG( name );
	
	if( !name )
		return NULL;
	AnimCharHandle out = g_AnimChars->getcopy( name );
	if( out )
		return out;
	
	double t0 = sgrx_hqtime();
	out = new AnimCharacter;
	if( !out->Load( name ) )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load animated character: " << name;
		return NULL;
	}
	out->m_key = name;
	g_AnimChars->set( out->m_key, out );
	if( VERBOSE )
		LOG << "Loaded animated character " << name
		<< " (time=" << ( sgrx_hqtime() - t0 ) << ")";
	return out;
}


SceneHandle GR_CreateScene()
{
	LOG_FUNCTION;
	
	SGRX_Scene* scene = new SGRX_Scene;
	scene->clutTexture = GR_GetTexture( "sys:lut_default" );
	
	if( VERBOSE ) LOG << "Created scene";
	return scene;
}


bool GR2D_LoadFont( const StringView& key, const StringView& path )
{
	SGRX_IFont* fif = NULL;
	if( ( fif = sgrx_int_CreateFont( path ) ) == NULL )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load font: " << path;
		return false;
	}
	fif->m_key = key;
	g_LoadedFonts->set( fif->m_key, fif );
	return true;
}

bool GR2D_LoadSVGIconFont( const StringView& key, const StringView& path )
{
	SGRX_IFont* fif = NULL;
	if( ( fif = sgrx_int_CreateSVGIconFont( path ) ) == NULL )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load SVG icon font: " << path;
		return false;
	}
	fif->m_key = key;
	g_LoadedFonts->set( fif->m_key, fif );
	return true;
}

FontHandle GR2D_GetFont( const StringView& key )
{
	return g_LoadedFonts->getcopy( key );
}


void SGRX_INT_InitResourceTables()
{
	g_CPSets = new ConvexPointSetHashTable();
	g_Textures = new TextureHashTable();
	g_RenderTargets = new RenderTargetTable();
	g_DepthStencilSurfs = new DepthStencilSurfTable();
	g_VertexShaders = new VertexShaderHashTable();
	g_PixelShaders = new PixelShaderHashTable();
	g_RenderStates = new RenderStateHashTable();
	g_VertexDecls = new VertexDeclHashTable();
	g_VtxInputMaps = new VtxInputMapHashTable();
	g_Meshes = new MeshHashTable();
	g_Anims = new AnimHashTable();
	g_AnimChars = new AnimCharHashTable();
	g_LoadedFonts = new FontHashTable();
	g_PreservedResources = new ResourcePreserveHashTable();
	
	g_LoadedFonts->set( "system", sgrx_int_GetSystemFont( false ) );
	g_LoadedFonts->set( "system_outlined", sgrx_int_GetSystemFont( true ) );
}

void SGRX_INT_DestroyResourceTables()
{
	delete g_LoadedFonts;
	g_LoadedFonts = NULL;
	
	delete g_PreservedResources;
	g_PreservedResources = NULL;
	
	delete g_Anims;
	g_Anims = NULL;
	
	delete g_AnimChars;
	g_AnimChars = NULL;
	
	delete g_Meshes;
	g_Meshes = NULL;
	
	delete g_VtxInputMaps;
	g_VtxInputMaps = NULL;
	
	delete g_VertexDecls;
	g_VertexDecls = NULL;
	
	delete g_RenderStates;
	g_RenderStates = NULL;
	
	delete g_PixelShaders;
	g_PixelShaders = NULL;
	
	delete g_VertexShaders;
	g_VertexShaders = NULL;
	
	delete g_DepthStencilSurfs;
	g_DepthStencilSurfs = NULL;
	
	delete g_RenderTargets;
	g_RenderTargets = NULL;
	
	delete g_Textures;
	g_Textures = NULL;
	
	delete g_CPSets;
	g_CPSets = NULL;
}

void GR_PreserveResourcePtr( SGRX_RefCounted* rsrc )
{
	g_PreservedResources->set( rsrc, 2 );
}

void SGRX_INT_UnpreserveResources()
{
	for( size_t i = 0; i < g_PreservedResources->size(); ++i )
	{
		if( --g_PreservedResources->item( i ).value <= 0 )
			g_PreservedResources->unset( g_PreservedResources->item( i ).key );
	}
}


