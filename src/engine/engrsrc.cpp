

#include "engine_int.hpp"
#include "enganim.hpp"
#include "engext.hpp"
#include "sound.hpp"


typedef HashTable< StringView, AnimHandle > AnimHashTable;
typedef HashTable< StringView, AnimCharHandle > AnimCharHashTable;


static ConvexPointSetHashTable* g_CPSets = NULL;
TextureHashTable* g_Textures = NULL;
RenderTargetTable* g_RenderTargets = NULL;
DepthStencilSurfTable* g_DepthStencilSurfs = NULL;
VertexShaderHashTable* g_VertexShaders = NULL;
PixelShaderHashTable* g_PixelShaders = NULL;
RenderStateHashTable* g_RenderStates = NULL;
VertexDeclHashTable* g_VertexDecls = NULL;
VtxInputMapHashTable* g_VtxInputMaps = NULL;
MeshHashTable* g_Meshes = NULL;
static AnimHashTable* g_Anims = NULL;
static AnimCharHashTable* g_AnimChars = NULL;
FontHashTable* g_LoadedFonts = NULL;
static ResourcePreserveHashTable* g_PreservedResources = NULL;

extern IGame* g_Game;
extern IRenderer* g_Renderer;



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


