

#include "systems.hpp"
#include "resources.hpp"
#include "level.hpp"


CVarBool gcv_cl_show_log( "cl_show_log", false );
CVarBool gcv_dbg_navmesh( "dbg_navmesh", false );
#define DBG_LIST_NONE 0
#define DBG_LIST_INFOTARGETS 1
StringView dbg_list_vnames[] =
{
	"none",
	"infotargets",
	"",
};
CVarEnum gcv_dbg_list( "dbg_list", dbg_list_vnames );


LevelMapSystem::LevelMapSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid ), viewPos(V3(0))
{
	_InitScriptInterface( this );
	AddSelfToLevel( "map" );
	
	sgs_RegIntConst ric[] =
	{
		{ "MI_None", MI_None },
		{ "MI_Mask_Object", MI_Mask_Object },
		{ "MI_Mask_State", MI_Mask_State },
		{ "MI_Object_Player", MI_Object_Player },
		{ "MI_Object_Enemy", MI_Object_Enemy },
		{ "MI_Object_Camera", MI_Object_Camera },
		{ "MI_Object_Objective", MI_Object_Objective },
		{ "MI_State_Normal", MI_State_Normal },
		{ "MI_State_Suspicious", MI_State_Suspicious },
		{ "MI_State_Alerted", MI_State_Alerted },
		{ NULL, 0 },
	};
	sgs_RegIntConsts( m_level->GetSGSC(), ric, -1 );
	
	m_tex_mapline = GR_GetTexture( "ui/mapline.png" );
}

void LevelMapSystem::Clear()
{
	m_mapItemData.clear();
	m_lines.clear();
	m_layers.clear();
	viewPos = V3(0);
}

bool LevelMapSystem::LoadChunk( const StringView& type, ByteView data )
{
	if( type != LC_FILE_MAPL_NAME )
		return false;
	
	LOG_FUNCTION_ARG( "MAPL chunk" );
	
	LC_Chunk_Mapl parser = { &m_lines, &m_layers };
	ByteReader br( data );
	br << parser;
	if( br.error )
	{
		LOG_ERROR << "Failed to parse MAPL (LevelMapSystem) data";
	}
	return true;
}

#if 0
void LevelMapSystem::UpdateItem( Entity* e, const MapItemInfo& data )
{
	m_mapItemData[ e ] = data;
}

void LevelMapSystem::RemoveItem( Entity* e )
{
	m_mapItemData.unset( e );
}
#endif

void LevelMapSystem::DrawUIRect( float x0, float y0, float x1, float y1, float linesize, sgsVariable cb )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	float map_aspect = safe_fdiv( x1 - x0, y1 - y0 );
	
	br.Reset();
	
	Vec2 updir = V2(0,-1);
	if( CameraResource* mainCamera = m_level->GetMainCamera() )
	{
		Vec3 d = mainCamera->GetWorldMatrix().TransformNormal( V3(0,1,1) );
		updir = d.ToVec2().Normalized();
	}
	
	Mat4 lookat = Mat4::CreateLookAt( V3( viewPos.x, viewPos.y, -0.5f ), V3(0,0,1), V3(updir.x,updir.y,0) );
	Mat4 viewproj = lookat * Mat4::CreateScale( 1.0f / ( 8 * map_aspect ), 1.0f / 8, 1 );
	Mat4 inv_vp = viewproj.Inverted();
	GR2D_SetViewMatrix( viewproj );
	
	GR2D_SetScissorRect( x0, y0, x1, y1 );
	GR2D_SetViewport( x0, y0, x1, y1 );
	
	int closest = 0;
	float cdist = FLT_MAX;
	for( int lid = 0; lid < (int) m_layers.size(); ++lid )
	{
		float dist = fabsf( m_layers[ lid ].height - viewPos.z );
		if( dist < cdist )
		{
			cdist = dist;
			closest = lid;
		}
	}
	for( int lid = 0; lid < (int) m_layers.size(); ++lid )
	{
		float alpha = clamp( 1 - fabsf( lid - closest ) / 4.0f, 0, 1 );
		br.Col( 0.2f, 0.4f, 0.8f, alpha );
		for( uint32_t i = m_layers[ lid ].from; i < m_layers[ lid ].to; i += 2 )
		{
			Vec2 l0 = m_lines[ i ];
			Vec2 l1 = m_lines[ i + 1 ];
			
			br.TexLine( l0, l1, 0.1f );
		}
	}
	
	for( size_t i = 0; i < m_mapItemData.size(); ++i )
	{
		MapItemInfo& mii = m_mapItemData.item( i ).value;
		Vec2 viewpos = mii.position.ToVec2();
		
		if( ( mii.type & MI_Mask_Object ) == MI_Object_Player )
		{
			br.Reset().SetTexture( m_tex_mapline )
				.Col( 0.2f, 0.9f, 0.1f ).Box( viewpos.x, viewpos.y, 1, 1 );
		}
		else if( ( mii.type & MI_Mask_Object ) == MI_Object_Objective )
		{
			Vec2 tv = viewproj.TransformPos( V3( viewpos, 0 ) ).ToVec2();
			Vec2 tv_abs = V2( fabsf( tv.x ), fabsf( tv.y ) );
			br.Reset().SetTexture( m_tex_mapline ).Col( 0.9f, 0.2f, 0.1f );
			if( tv_abs.x > 1 || tv_abs.y > 1 )
			{
				// render as line at the edge
				tv /= TMAX( tv_abs.x, tv_abs.y );
				Vec2 tv1 = tv * 0.9f;
				br.TexLine(
					inv_vp.TransformPos( V3( tv, 0 ) ).ToVec2(),
					inv_vp.TransformPos( V3( tv1, 0 ) ).ToVec2(),
					0.5f );
			}
			else
			{
				// render as regular point in map
				br.Box( viewpos.x, viewpos.y, 2, 2 );
			}
		}
		else
		{
			uint32_t viewcol = 0xffffffff;
			uint32_t dotcol = COLOR_RGB( 245, 20, 10 );
			switch( mii.type & MI_Mask_State )
			{
			case MI_State_Normal:
				viewcol = mii.type & MI_Object_Enemy ?
					COLOR_RGB( 230, 230, 230 ) : COLOR_RGB( 180, 230, 180 );
				break;
			case MI_State_Suspicious:
				viewcol = COLOR_RGB( 170, 170, 100 );
				break;
			case MI_State_Alerted:
				viewcol = COLOR_RGB( 170, 100, 100 );
				break;
			}
			viewcol &= 0x7fffffff;
			uint32_t viewcol_a0 = viewcol & 0x00ffffff;
			Vec2 viewdir = mii.direction.ToVec2().Normalized();
			Vec2 viewtan = viewdir.Perp();
			br.Reset().Colu( viewcol_a0 )
				.SetPrimitiveType( PT_Triangles )
				.Pos( viewpos + viewdir * mii.sizeFwd - viewtan * mii.sizeRight )
				.Pos( viewpos + viewdir * mii.sizeFwd + viewtan * mii.sizeRight )
				.Colu( viewcol ).Pos( viewpos );
			
			br.Reset().SetTexture( m_tex_mapline )
				.Colu( dotcol ).Box( viewpos.x, viewpos.y, 1, 1 );
		}
	}
	
#if 0
	ObjectiveSystem* objSys = m_level->GetSystem<ObjectiveSystem>();
	if( objSys )
	{
		for( size_t i = 0; i < objSys->m_objectives.size(); ++i )
		{
			OSObjective& obj = objSys->m_objectives[ i ];
			if( obj.hasLocation == false || obj.state != OSObjective::Open )
				continue;
			
			Vec2 viewpos = obj.location.ToVec2();
			
			uint32_t dotcol = COLOR_RGB( 20, 245, 10 );
			br.Reset().SetTexture( m_tex_mapline )
				.Colu( dotcol ).Box( viewpos.x, viewpos.y, 1, 1 );
		}
	}
#endif
	
	if( cb.not_null() )
	{
		cb.tcall<void>( C, viewproj, inv_vp );
	}
	
	br.Flush();
	GR2D_UnsetViewport();
	GR2D_UnsetScissorRect();
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
}

#if 0
void LevelMapSystem::sgsUpdate( EntityScrHandle e, int type, Vec3 pos, Vec3 dir, float szfwd, float szrt )
{
	MapItemInfo mii = { type, pos, dir, szfwd, szrt };
	UpdateItem( e, mii );
}

void LevelMapSystem::sgsRemove( EntityScrHandle e )
{
	RemoveItem( e );
}
#endif



FlareSystem::FlareSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid ), m_layers(0xffffffff)
{
	m_ps_flare = GR_GetPixelShader( "flare" );
	m_tex_flare = GR_GetTexture( "textures/fx/flare.png" );
	
	_InitScriptInterface( this );
	AddSelfToLevel( "flares" );
}

void FlareSystem::Clear()
{
	m_flares.clear();
}

void FlareSystem::UpdateFlare( void* handle, const FSFlare& flare )
{
	m_flares.set( handle, flare );
}

bool FlareSystem::RemoveFlare( void* handle )
{
	return m_flares.unset( handle );
}

void FlareSystem::PostDraw()
{
	SGRX_Camera& cam = m_level->GetScene()->camera;
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	float W = GR_GetWidth();
	float H = GR_GetHeight();
	float sz = TMIN( W, H ) * 0.2f;
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset().SetShader( m_ps_flare ).SetTexture( m_tex_flare );
	br.RenderState.blendStates[0].dstBlend = SGRX_RS_Blend_One;
	br.ShaderData.push_back( V4( W, H, 1.0f / W, 1.0f / H ) );
	br.ShaderData.push_back( V4(1) );
	for( size_t i = 0; i < m_flares.size(); ++i )
	{
		FSFlare& FD = m_flares.item( i ).value;
		if( FD.enabled == false || FD.size <= 0 )
			continue;
		br.ShaderData[1] = V4( FD.color * 2, 0.1f / ( ( FD.pos - cam.position ).Length() + 1 ) );
		Vec3 screenpos = cam.WorldToScreen( FD.pos );
		if( Vec3Dot( FD.pos, cam.direction ) < Vec3Dot( cam.position, cam.direction ) )
			continue;
		SceneRaycastCallback_Any srcb;
		m_level->GetScene()->RaycastAll( cam.position, FD.pos, &srcb, m_layers );
		if( srcb.m_hit )
			continue;
	//	LOG << screenpos.z;
		float dx = cos(0.1f)*0.5f*sz * FD.size;
		float dy = sin(0.1f)*0.5f*sz * FD.size;
		br.TurnedBox( screenpos.x * W, screenpos.y * H, dx, dy );
		br.Flush();
	}
	br.RenderState.blendStates[0].dstBlend = SGRX_RS_Blend_InvSrcAlpha;
}

void FlareSystem::sgsUpdate( void* handle, Vec3 pos, Vec3 col, float size, bool enabled )
{
	if( handle == NULL )
	{
		sgs_Msg( C, SGS_WARNING, "cannot use NULL pointer for association" );
		return;
	}
	FSFlare F = { pos, col, size, sgs_StackSize( C ) >= 5 ? enabled : true };
	UpdateFlare( handle, F );
}

void FlareSystem::sgsRemove( void* handle )
{
	if( handle == NULL )
	{
		sgs_Msg( C, SGS_WARNING, "cannot use NULL pointer for association" );
		return;
	}
	RemoveFlare( handle );
}


LevelCoreSystem::LevelCoreSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	lev->m_lightEnv = &m_lightEnv;
}

void LevelCoreSystem::Clear()
{
	m_meshInsts.clear();
	m_lightInsts.clear();
	m_levelBodies.clear();
	m_lights.clear();
	m_lightEnv.SetSamplePositions( NULL, 0 );
}

bool LevelCoreSystem::LoadChunk( const StringView& type, ByteView data )
{
	if( type != LC_FILE_GEOM_NAME )
		return false;
	
	Array< LC_MeshInst > meshInstDefs;
	LC_PhysicsMesh phyMesh;
	Array< LC_SolidBox > solidBoxes;
	LC_Chunk_Geom geom = { &meshInstDefs, &m_lights, &m_lightEnv, &phyMesh, &solidBoxes };
	ByteReader br( data );
	br << geom;
	if( br.error )
	{
		LOG_ERROR << "Failed to load GEOM (LevelCoreSystem) chunk";
		return true;
	}
	
	// SKYBOX
	if( geom.skyTexture )
	{
		LOG_FUNCTION_ARG( "SKYBOX" );
		m_level->GetScene()->skyTexture = GR_GetTexture( geom.skyTexture );
	}
	
	// cLUT
	if( geom.clutTexture )
	{
		LOG_FUNCTION_ARG( "CLUT" );
		m_level->GetScene()->clutTexture = GR_GetTexture( geom.clutTexture );
	}
	
	// LOAD FLARES
	FlareSystem* FS = m_level->GetSystem<FlareSystem>();
	if( FS )
	{
		for( size_t i = 0; i < m_lights.size(); ++i )
		{
			LC_Light& L = m_lights[ i ];
			if( L.type != LM_LIGHT_POINT && L.type != LM_LIGHT_SPOT &&
				L.type != LM_LIGHT_DYN_POINT && L.type != LM_LIGHT_DYN_SPOT )
				continue;
			FSFlare FD = { L.pos + L.flareoffset, L.color, L.flaresize, true };
			FS->UpdateFlare( &m_lights[ i ], FD );
		}
	}
	
	// create physics geometry - mesh
	{
		LOG_FUNCTION_ARG( "PHY_MESH" );
		
		// TODO: temporarily ignore material data
		Array< uint32_t > fixedidcs;
		for( size_t i = 0; i < phyMesh.indices.size(); i += 4 )
		{
			fixedidcs.append( &phyMesh.indices[ i ], 3 );
		}
		
		SGRX_PhyRigidBodyInfo rbinfo;
		rbinfo.shape = m_level->GetPhyWorld()->CreateTriMeshShape(
			phyMesh.positions.data(), phyMesh.positions.size(),
			fixedidcs.data(), fixedidcs.size(), true );
		m_levelBodies.push_back( m_level->GetPhyWorld()->CreateRigidBody( rbinfo ) );
	}
	
	// create physics geometry - boxes
	{
		LOG_FUNCTION_ARG( "PHY_BOXES" );
		for( size_t i = 0; i < solidBoxes.size(); ++i )
		{
			const LC_SolidBox& SB = solidBoxes[ i ];
			SGRX_PhyRigidBodyInfo rbinfo;
			rbinfo.group = 1;
			rbinfo.shape = m_level->GetPhyWorld()->CreateAABBShape( -SB.scale, SB.scale );
			rbinfo.mass = 0;
			rbinfo.inertia = V3(0);
			rbinfo.position = SB.position;
			rbinfo.rotation = SB.rotation;
			m_levelBodies.push_back( m_level->GetPhyWorld()->CreateRigidBody( rbinfo ) );
		}
	}
	
	// load mesh instances
	{
		LOG_FUNCTION_ARG( "MESH_INSTS" );
		
		StringView levelname = m_level->GetLevelName();
		
		for( size_t i = 0; i < meshInstDefs.size(); ++i )
		{
			LC_MeshInst& MID = meshInstDefs[ i ];
			
			LOG_FUNCTION_ARG( MID.m_meshname );
			
			char subbfr[ 512 ];
			MeshInstHandle MI = m_level->GetScene()->CreateMeshInstance();
			StringView src = MID.m_meshname;
			if( src.ch() == '~' )
			{
				sgrx_snprintf( subbfr, sizeof(subbfr), SGRXPATH__LEVELS "/%.*s" SGRX_LEVEL_DIR_SFX "%.*s", TMIN( (int) levelname.size(), 200 ), levelname.data(), TMIN( (int) src.size() - 1, 200 ), src.data() + 1 );
				MI->SetMesh( subbfr );
			}
			else
				MI->SetMesh( src );
			
			if( MID.m_lmap.width && MID.m_lmap.height )
			{
				TextureHandle lmtex = GR_CreateTexture( MID.m_lmap.width, MID.m_lmap.height, TEXFMT_RGBA8,
					TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y, 1, MID.m_lmap.data.data() );
				MI->SetMITexture( 0, lmtex );
				
				TextureHandle nmtex = GR_CreateTexture( MID.m_lmap.width, MID.m_lmap.height, TEXFMT_RGBA8,
					TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y, 1, MID.m_lmap.nmdata.data() );
				MI->SetMITexture( 1, nmtex );
				
				MI->SetLightingMode( SGRX_LM_Static );
				MI->allowStaticDecals = true;
			}
			else
			{
				for( int i = 10; i < 16; ++i )
					MI->constants[ i ] = V4(0.15f);
			}
			
			MI->matrix = MID.m_mtx;
			
			if( MID.m_flags & LM_MESHINST_UNLIT )
			{
				MI->SetLightingMode( SGRX_LM_Unlit );
			}
			else if( MID.m_flags & LM_MESHINST_DYNLIT )
			{
				MI->SetLightingMode( SGRX_LM_Dynamic );
				m_level->LightMesh( MI );
			}
			else if( ( MID.m_flags & (LM_MESHINST_DECAL|LM_MESHINST_TRANSPARENT|LM_MESHINST_VCOL) ) != 0 && MI->GetMesh() )
			{
				if( MID.m_flags & LM_MESHINST_VCOL )
					MI->SetAllMtlFlags( SGRX_MtlFlag_VCol, 0 );
				if( MID.m_flags & LM_MESHINST_DECAL )
					MI->SetAllMtlFlags( SGRX_MtlFlag_Decal, 0 );
				if( MID.m_flags & LM_MESHINST_TRANSPARENT )
				{
					for( size_t i = 0; i < MI->materials.size(); ++i )
						if( MI->materials[ i ].blendMode == SGRX_MtlBlend_None )
							MI->materials[ i ].blendMode = SGRX_MtlBlend_Basic;
					MI->OnUpdate();
				}
				MI->sortidx = MID.m_decalLayer;
			}
			
			m_meshInsts.push_back( MI );
			
			if( MID.m_flags & LM_MESHINST_SOLID )
			{
				LOG_FUNCTION_ARG( "MI_BODY" );
				
				SGRX_PhyRigidBodyInfo rbinfo;
				rbinfo.shape = m_level->GetPhyWorld()->CreateShapeFromMesh( MI->GetMesh() );
				rbinfo.shape->SetScale( MI->matrix.GetScale() );
				rbinfo.position = MI->matrix.GetTranslation();
				rbinfo.rotation = MI->matrix.GetRotationQuaternion();
				m_levelBodies.push_back( m_level->GetPhyWorld()->CreateRigidBody( rbinfo ) );
			}
		}
	}
	
	// load dynamic lights
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		LC_Light& L = m_lights[ i ];
		if( L.type == LM_LIGHT_DYN_POINT || L.type == LM_LIGHT_DYN_SPOT )
		{
			LightHandle lh = m_level->GetScene()->CreateLight();
			if( L.type == LM_LIGHT_DYN_POINT )
				lh->type = LIGHT_POINT;
			else
				lh->type = LIGHT_SPOT;
			lh->position = L.pos;
			lh->direction = L.dir;
			lh->updir = L.up;
			lh->range = L.range;
			lh->power = L.power;
			lh->color = L.color;
		//	lh-> = L.flaresize;
		//	lh-> = L.flareoffset;
		//	lh-> = L.innerangle;
			lh->angle = L.outerangle;
		//	lh-> = L.spotcurve;
			lh->cookieTexture = GR_GetTexture( "textures/cookies/default.png" );
			if( L.num_shadow_samples > 0 && L.num_shadow_samples <= 2048 )
				lh->shadowTexture = GR_CreateRenderTexture( L.num_shadow_samples, L.num_shadow_samples, TEXFMT_RT_DEPTH_F32 );
			lh->UpdateTransform();
			m_lightInsts.push_back( lh );
		}
	}
	
	return true;
}


GFXSystem::GFXSystem( GameLevel* lev ) :
	IGameLevelSystem( lev, e_system_uid )
{
	lev->GetScene()->director = this;
	RegisterHandler( EID_GOResourceAdd );
	RegisterHandler( EID_GOResourceRemove );
}

void GFXSystem::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	if( eid == EID_GOResourceAdd )
	{
		SGRX_CAST( GOResource*, R, edata.GetUserData() );
		if( R->m_rsrcType == GO_RSRC_REFPLANE )
			m_reflectPlanes.push_back( (ReflectionPlaneResource*) R );
		else if( R->m_rsrcType == GO_RSRC_FLARE )
			m_flares.push_back( (FlareResource*) R );
	}
	else if( eid == EID_GOResourceRemove )
	{
		SGRX_CAST( GOResource*, R, edata.GetUserData() );
		if( R->m_rsrcType == GO_RSRC_REFPLANE )
			m_reflectPlanes.remove_first( (ReflectionPlaneResource*) R );
		else if( R->m_rsrcType == GO_RSRC_FLARE )
			m_flares.remove_first( (FlareResource*) R );
	}
}

FINLINE float PointPlaneSignedDistance( const Vec3& pos, const Vec4& plane )
{
	return Vec3Dot( plane.ToVec3(), pos ) - plane.w;
}

FINLINE Vec3 Vec3ReflectPos( const Vec3& pos, const Vec4& plane )
{
	return pos - plane.ToVec3() * ( PointPlaneSignedDistance( pos, plane ) * 2 );
}

void GFXSystem::OnDrawScene( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info )
{
	DevelopSystem* ds = m_level->GetSystem<DevelopSystem>();
	if( m_level->m_editorMode ||
		!m_level->m_cameras.size() ||
		( ds && ds->screenshotMode ) )
	{
		OnDrawSceneWithRefl( ctrl, info );
		return;
	}
	
	SGRX_Scene* scene = info.scene;
	SGRX_Camera origCamera = scene->camera;
	SGRX_Viewport* origViewport = info.viewport;
	
	// TODO sort cameras
	for( size_t i = 0; i < m_level->m_cameras.size(); ++i )
	{
		SGRX_CAST( CameraResource*, CR, m_level->m_cameras[ i ] );
		CR->GetCamera( scene->camera );
		OnDrawSceneWithRefl( ctrl, info );
	}
	
	scene->camera = origCamera;
	info.viewport = origViewport;
}

void GFXSystem::OnDrawSceneWithRefl( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info )
{
#define RT_REFL 0xffe0
	
	// shortcuts
	SGRX_Scene* scene = info.scene;
	
	int reflW = info.GetOutputWidth();
	int reflH = info.GetOutputHeight();
	SGRX_Camera origCamera = scene->camera;
	SGRX_Viewport* origViewport = info.viewport;
	
	// initial actions
	if( m_curMode != SGRX_RDMode_Unlit )
	{
		ctrl->RenderShadows( scene, SGRX_PassType_Shadow );
	}
	
	// RENDER REFLECTIONS
	TextureHandle rttREFL = GR_GetRenderTarget( reflW, reflH, TEXFMT_RT_COLOR_HDR16, RT_REFL );
	DepthStencilSurfHandle dssREFL = GR_GetDepthStencilSurface( reflW, reflH, TEXFMT_RT_COLOR_HDR16, RT_REFL );
	
	GR_PreserveResource( rttREFL );
	GR_PreserveResource( dssREFL );
	
	for( size_t i = 0; i < m_reflectPlanes.size(); ++i )
	{
		GOResource* RPE = m_reflectPlanes[ i ];
		
		Mat4 xf = RPE->GetWorldMatrix();
		Vec3 pos = xf.GetTranslation();
		Vec3 dir = xf.TransformNormal( V3(0,0,1) ).Normalized();
		Vec4 plane = V4( dir, Vec3Dot( dir, pos ) );
		
		scene->camera.position = Vec3ReflectPos( scene->camera.position, plane );
		scene->camera.direction = Vec3Reflect( scene->camera.direction, plane.ToVec3() );
		scene->camera.updir = Vec3Reflect( scene->camera.updir, plane.ToVec3() );
		scene->camera.UpdateMatrices();
		info.viewport = NULL;
		
		ctrl->SortRenderItems( scene );
		
		OnDrawSceneGeom( ctrl, info, rttREFL, dssREFL, NULL );
	}
	if( m_reflectPlanes.size() == 0 )
	{
		ctrl->SetRenderTargets( dssREFL, SGRX_RT_ClearAll, 0, 0, 1, rttREFL );
	}
	
	scene->camera = origCamera;
	info.viewport = origViewport;
	
	// RENDER MAIN SCENE
	ctrl->m_overrideTextures[ 10 ] = scene->skyTexture;
	ctrl->m_overrideTextures[ 11 ] = rttREFL; // GR_GetTexture( "textures/unit.png" );
	GR_PreserveResource( ctrl->m_overrideTextures[ 0 ] );
	ctrl->SortRenderItems( scene );
	SGRX_RenderDirector::OnDrawScene( ctrl, info );
}

void GFXSystem::OnDrawSceneGeom( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info,
	SGRX_RTSpec rtt, DepthStencilSurfHandle dss, TextureHandle rttDEPTH )
{
	SGRX_RenderDirector::OnDrawSceneGeom( ctrl, info, rtt, dss, rttDEPTH );
	
	Vec2 vpsz = info.GetOutputSizeF();
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, vpsz.x, vpsz.y ) );
	for( size_t i = 0; i < m_flares.size(); ++i )
	{
		GameObject* obj = m_flares[ i ]->m_obj;
		
		LightResource* lr = obj->FindFirstResourceOfType<LightResource>();
		
		Vec3 flareColor = V3(1);
		if( lr )
		{
			Vec3 fc = lr->GetFinalColor();
			float hue = GetHue( fc );
			float sat = GetHSVSaturation( fc );
			float val = GetValue( fc );
			val = 1.0f - 1.0f / ( val * 2 + 1 );
			flareColor = HSV( V3( hue, sat, val ) );
		}
		m_flares[ i ]->m_flare.Draw(
			m_level->m_editorMode ? 1 : m_level->GetDeltaTime(),
			m_level->GetScene(),
			vpsz,
			m_flares[ i ]->GetWorldMatrix().GetTranslation(),
			flareColor );
	}
}


ScriptedSequenceSystem::ScriptedSequenceSystem( GameLevel* lev ) :
	IGameLevelSystem( lev, e_system_uid ), m_cmdSkip( "skip_cutscene" ), m_time( 0 )
{
	Game_RegisterAction( &m_cmdSkip );
//	Game_BindInputToAction( ACTINPUT_MAKE_KEY( SDLK_SPACE ), &m_cmdSkip );
	
	_InitScriptInterface( this );
	AddSelfToLevel( "scrSeq" );
}

void ScriptedSequenceSystem::Tick( float deltaTime, float blendFactor )
{
	if( m_func.not_null() )
	{
		SGS_SCOPE;
		sgs_PushVar( C, m_time );
		m_func.call( C, 1, 1 );
		if( sgs_GetVar<bool>()( C, -1 ) )
		{
			m_func = sgsVariable();
			m_subtitle = "";
		}
		if( m_cmdSkip.value )
			m_time += deltaTime * 20;
		else
			m_time += deltaTime;
	}
}

void ScriptedSequenceSystem::DrawUI()
{
	int size_x = GR_GetWidth();
	int size_y = GR_GetHeight();
	int sqr = TMIN( size_x, size_y );
	
	if( m_func.not_null() )
	{
		GR2D_SetFont( "core", sqr / 40 );
		GR2D_SetColor( 1, 1 );
		GR2D_DrawTextLine( sqr/20, sqr/20, "Press <Space> to speed up", HALIGN_LEFT, VALIGN_TOP );
	}
	
	if( m_subtitle.size() )
	{
		GR2D_SetFont( "core", sqr / 20 );
		GR2D_SetColor( 0, 1 );
		GR2D_DrawTextLine( size_x / 2 + 1, size_y * 3 / 4 + 1, m_subtitle, HALIGN_CENTER, VALIGN_CENTER );
		GR2D_SetColor( 1, 1 );
		GR2D_DrawTextLine( size_x / 2, size_y * 3 / 4, m_subtitle, HALIGN_CENTER, VALIGN_CENTER );
	}
}

void ScriptedSequenceSystem::sgsStart( sgsVariable func, float t )
{
	m_func = func;
	m_time = t;
}


MusicSystem::MusicSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	_InitScriptInterface( this );
	AddSelfToLevel( "music" );
}

MusicSystem::~MusicSystem()
{
	if( m_music )
		m_music->Stop();
}

void MusicSystem::sgsSetTrack( StringView path )
{
	if( m_music )
		m_music->Stop();
	if( path )
	{
		m_music = m_level->GetSoundSys()->CreateEventInstance( path );
		m_music->Start();
	}
	else
		m_music = NULL;
}

void MusicSystem::sgsSetVar( StringView name, float val )
{
	if( m_music && name )
	{
		m_music->SetParameter( name, val );
	}
}


DamageSystem::DamageSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	const char* err = Init( lev->GetScene(), lev );
	if( err )
	{
		LOG_ERROR << LOG_DATE << "  Failed to init DMGSYS: " << err;
	}
}

DamageSystem::~DamageSystem()
{
	Free();
}

const char* DamageSystem::Init( SceneHandle scene, SGRX_LightSampler* sampler )
{
	this->scene = scene;
	
	static char errbfr[ 350 ];
	
	String mtlconfig;
	if( FS_LoadTextFile( "damage.dat", mtlconfig ) == false )
		return( "Failed to load damage.dat" );
	
	// defaults
	String decal_base_tex = "textures/fx/impact_decals.png";
	String decal_falloff_tex = "textures/fx/projfalloff2.png";
	MtlHandle cur_mtl;
	
	ConfigReader cfgrd( mtlconfig );
	StringView key, value;
	while( cfgrd.Read( key, value ) )
	{
		if( key == "decal_base_tex" ){ decal_base_tex = value; continue; }
		if( key == "decal_falloff_tex" ){ decal_falloff_tex = value; continue; }
		if( key == "material" )
		{
			cur_mtl = new Material;
			cur_mtl->match = value;
			m_bulletDecalMaterials.push_back( cur_mtl );
			continue;
		}
		if( key.part( 0, 4 ) == "mtl_" )
		{
			// error handling
			if( cur_mtl == NULL )
				return "mtl_ command has no material";
		}
		if( key == "mtl_solid" )
		{
			cur_mtl->isSolid = String_ParseBool( value );
		}
		if( key == "mtl_decal" )
		{
			int id = m_bulletDecalInfo.size();
			bool suc = false;
			Vec4 tex_aabb = String_ParseVec4( value.until( "|" ), &suc );
			if( suc == false )
				return "mtl_decal - cannot parse coord rect";
			
			float decal_size = String_ParseFloat( value.after( "|" ), &suc );
			if( suc == false )
				return "mtl_decal - cannot parse size";
			
			DecalMapPartInfo dmpi = { tex_aabb, V3(decal_size) };
			m_bulletDecalInfo.push_back( dmpi );
			cur_mtl->decalIDs.push_back( id );
			
			continue;
		}
		if( key == "mtl_particles" )
		{
			if( cur_mtl->particles.Load( value ) == false )
			{
				sgrx_snprintf( errbfr, 350, "Failed to load particle system '%.*s' while parsing "
					"damage.dat", TMIN( 250, (int) value.size() ), value.data() );
				return( errbfr );
			}
			cur_mtl->particles.m_lightSampler = sampler;
			cur_mtl->particles.AddToScene( scene );
			continue;
		}
		if( key == "mtl_sound" )
		{
			cur_mtl->sound = value;
			// TODO validate
		}
	}
	
	m_bulletDecalSys.m_lightSampler = sampler;
	m_bulletDecalSys.Init( scene,
		GR_GetTexture( decal_base_tex ),
		GR_GetTexture( decal_falloff_tex ) );
	m_bulletDecalSys.SetSize( 48 * 1024 * 10 ); // random size
	m_bulletDecalSys.m_meshInst->sortidx = 202;
	
	m_bloodDecalSys.m_lightSampler = sampler;
	m_bloodDecalSys.Init( scene,
		GR_GetTexture( "textures/particles/blood.png" ),
		GR_GetTexture( decal_falloff_tex ) );
	m_bloodDecalSys.SetSize( 48 * 1024 * 10 ); // random size
	m_bloodDecalSys.m_meshInst->sortidx = 201;
	
	LOG << LOG_DATE << "  Damage system initialized successfully";
	return NULL;
}

void DamageSystem::Free()
{
	m_bulletDecalSys.Free();
	m_bloodDecalSys.Free();
}

void DamageSystem::Tick( float deltaTime, float blendFactor )
{
	UNUSED( deltaTime );
	m_bulletDecalSys.Upload();
	m_bloodDecalSys.Upload();
	for( size_t i = 0; i < m_bulletDecalMaterials.size(); ++i )
	{
		Material* mtl = m_bulletDecalMaterials[ i ];
		if( m_level->IsPaused() == false )
		{
			mtl->particles.Tick( deltaTime );
		}
	}
}

bool DamageSystem::AddBulletDamage( SGRX_DecalSystem* dmgDecalSysOverride,
	const StringView& type, SGRX_IMesh* targetMesh, int partID,
	const Mat4& worldMatrix, const Vec3& pos, const Vec3& dir, const Vec3& nrm, float scale )
{
	bool solid = true;
	int decalID = -1;
	for( size_t i = 0; i < m_bulletDecalMaterials.size(); ++i )
	{
		Material* mtl = m_bulletDecalMaterials[ i ];
		if( mtl->CheckMatch( type ) )
		{
			// decal
			if( mtl->decalIDs.size() )
				decalID = mtl->decalIDs[ rand() % mtl->decalIDs.size() ];
			
			// particles
			Mat4 tf = Mat4::CreateRotationZ( randf() * M_PI * 2 )
				* Mat4::CreateRotationBetweenVectors( V3(0,0,1), nrm )
				* Mat4::CreateTranslation( pos );
			mtl->particles.SetTransform( tf );
			mtl->particles.Trigger();
			
			// sound
			SoundEventInstanceHandle sev = m_level->GetSoundSys()->CreateEventInstance( mtl->sound );
			SGRX_Sound3DAttribs s3dattr = { pos, V3(0), nrm, Vec3Cross( dir, nrm ).Normalized() };
			sev->Set3DAttribs( s3dattr );
			sev->Start();
			
			solid = mtl->isSolid;
			break;
		}
	}
	
	if( decalID != -1 && targetMesh )
	{
		Vec3 dpdir = -nrm;
		DecalProjectionInfo projInfo =
		{
			pos, dpdir, fabsf( Vec3Dot( dpdir, V3(0,0,1) ) ) > 0.99f ? V3(0,1,0) : V3(0,0,1),
			0, scale, 1, 0.5f, scale, 0.5f, false,
			m_bulletDecalInfo[ decalID ]
		};
		if( dmgDecalSysOverride == NULL )
			dmgDecalSysOverride = &m_bulletDecalSys;
		if( partID < 0 )
			dmgDecalSysOverride->AddDecal( projInfo, targetMesh, worldMatrix );
		else
			dmgDecalSysOverride->AddDecal( projInfo, targetMesh, partID, worldMatrix );
	}
	
	return solid;
}

struct DmgSys_GenBlood : IProcessor
{
	void Process( void* data )
	{
		SGRX_CAST( SGRX_MeshInstance*, MI, data );
		if( MI->GetMesh() == NULL ||
			MI->raycastOverride ||
			MI->IsSkinned() )
			return;
		SGRX_CAST( SGRX_MeshInstUserData*, mii, MI->userData );
		if( mii && mii->ovrDecalSysOverride )
		{
			mii->ovrDecalSysOverride->AddDecal( projInfo, MI->GetMesh(), MI->matrix );
			return;
		}
		if( MI->GetLightingMode() != SGRX_LM_Static )
			return;
		DS->m_bloodDecalSys.AddDecal( projInfo, MI->GetMesh(), MI->matrix );
	}
	
	DamageSystem* DS;
	DecalProjectionInfo projInfo;
};

void DamageSystem::AddBlood( Vec3 pos, Vec3 dir )
{
	int decalID = 0;
	if( decalID != -1 )
	{
		DecalProjectionInfo projInfo =
		{
			pos, dir, fabsf( Vec3Dot( dir, V3(0,0,1) ) ) > 0.99f ? V3(0,1,0) : V3(0,0,1),
			45.0f, 1, 1, 0.5f, 1, 0, true,
			{ V4(0,0,1,1), V3(1,1,2) }
		};
		SGRX_Camera cam;
		m_bloodDecalSys.GenerateCamera( projInfo, &cam );
		DmgSys_GenBlood gb;
		{
			gb.DS = this;
			gb.projInfo = projInfo;
		}
		m_level->GetScene()->GatherMeshes( cam, &gb );
	}
}

void DamageSystem::Clear()
{
	m_bulletDecalSys.ClearAllDecals();
	m_bloodDecalSys.ClearAllDecals();
}


BulletSystem::BulletSystem( GameLevel* lev ) :
	IGameLevelSystem( lev, e_system_uid ),
	m_damageSystem( lev->GetSystem<DamageSystem>() )
{
	_InitScriptInterface( this );
	AddSelfToLevel( "bulletSystem" );
}

void BulletSystem::Tick( float deltaTime, float blendFactor )
{
	for( size_t i = 0; i < m_bullets.size(); ++i )
	{
		Bullet& B = m_bullets[i];
		
		B.timeleft -= deltaTime;
		if( B.timeleft <= 0 )
		{
			m_bullets.erase( i-- );
			break;
		}
		Vec3 p1 = B.position;
		Vec3 p2 = p1 + B.velocity * deltaTime;
		
		bool remb = _ProcessBullet( p1, p2, B ) >= 0;
		
		if( remb )
			m_bullets.erase( i-- );
		else
			B.position = p2;
	}
}

void BulletSystem::Add( Vec3 pos, Vec3 vel, float timeleft, Vec2 dmg, void* ownerID )
{
	Bullet B = { pos, vel, vel.Normalized(), timeleft, dmg, ownerID };
	m_bullets.push_back( B );
}

float BulletSystem::Zap( Vec3 p1, Vec3 p2, Vec2 dmg, void* ownerID )
{
	Vec3 dir = ( p2 - p1 ).Normalized();
	Bullet B = { p1, dir * dmg.x, dir, 0, dmg, ownerID };
	return _ProcessBullet( p1, p2, B );
}

float BulletSystem::_ProcessBullet( Vec3 p1, Vec3 p2, Bullet& B )
{
	SceneRaycastCallback_Sorting cb( &m_tmpStore );
	m_level->GetScene()->RaycastAll( p1, p2, &cb, 0xffffffff );
	#ifdef TSGAME
//	g_DebugLines.DrawLine( p1, p2, m_tmpStore.size() ? COLOR_RGB(255,0,0) : COLOR_RGB(255,255,0) );
	#endif
	
	// sorted list of raycast hits
	float outdst = -1;
	bool remb = false;
	if( m_tmpStore.size() )
	{
		for( size_t hitid = 0; hitid < m_tmpStore.size(); ++hitid )
		{
			SceneRaycastInfo& HIT = m_tmpStore[ hitid ];
			if( HIT.meshinst->enabled == false )
				continue;
			
			float entryIfL0 = Vec3Dot( B.dir, HIT.normal );
			SGRX_MeshInstUserData* mii = (SGRX_MeshInstUserData*) HIT.meshinst->userData;
			if( mii && mii->ownerID == B.ownerToSkip )
				continue;
			
			StringView decalType = "unknown";
			if( mii && mii->typeOverride )
			{
				decalType = mii->typeOverride;
			}
			else
			{
				SGRX_IMesh* mesh = HIT.meshinst->GetMesh();
				if( HIT.partID >= 0 && HIT.partID < (int) mesh->m_meshParts.size() )
				{
					SGRX_MeshPart& MP = mesh->m_meshParts[ HIT.partID ];
					if( MP.textures[0].size() &&
						( MP.mtlBlendMode == SGRX_MtlBlend_None ||
						MP.mtlBlendMode == SGRX_MtlBlend_Basic ) )
					{
						decalType = MP.textures[0];
					//	printf("%s\n", StackString<256>(decalType).str);
					}
				}
			}
			
			// apply damage to hit point
			outdst = HIT.factor;
			Vec3 hitpoint = TLERP( p1, p2, HIT.factor );
			SGRX_DecalSystem* dmgDecalSys = mii ? mii->dmgDecalSysOverride : NULL;
			bool needDecal = ( HIT.meshinst->allowStaticDecals || dmgDecalSys ) &&
				HIT.meshinst->IsSkinned() == false;
			bool isSolid = m_damageSystem->AddBulletDamage( dmgDecalSys, decalType,
				needDecal ? HIT.meshinst->GetMesh() : NULL,
				-1, HIT.meshinst->matrix, hitpoint, B.dir, HIT.normal );
			
			// blood?
			if( decalType == "*human*" )
			{
				m_damageSystem->AddBlood( hitpoint, B.dir );
			}
			
			// send event
			if( mii )
			{
				MI_BulletHit_Data data = { hitpoint, B.velocity, B.damage.x, B.damage.y };
				mii->MeshInstUser_OnEvent( HIT.meshinst, MIEVT_BulletHit, &data );
			}
			
			if( !isSolid )
				continue;
			
			// handling wall penetration
			B.numSolidRefs += entryIfL0 < 0 ? 1 : -1;
			if( B.numSolidRefs == 1 )
			{
				// entry into solid
				B.intersectStart = hitpoint;
				// todo upgrade
				remb = true;
				break;
			}
			else if( B.numSolidRefs == 0 )
			{
				// genuine exit, calculate penetration
				float dist = ( hitpoint - B.intersectStart ).Length();
				float q = dist * 50;
				float speedScale = q < 1 ? 1 : 1 / q;
				speedScale = ( speedScale - 1 ) * 1.15f + 1;
				if( speedScale < 0 )
				{
					remb = true;
					break;
				}
				B.velocity *= speedScale;
			}
			else if( B.numSolidRefs < 0 )
			{
				// fake exit, abort all
				remb = true;
				break;
			}
		}
	}
	// make sure mesh instance handles are freed
	m_tmpStore.clear();
	return remb ? outdst : -1;
}

void BulletSystem::Clear()
{
	m_bullets.clear();
}



AIFactStorage::AIFactStorage() : m_lastTime(0), last_mod_id(0), m_next_fact_id(1)
{
}

void AIFactStorage::Clear()
{
	facts.clear();
	m_lastTime = 0;
	last_mod_id = 0;
	m_next_fact_id = 1;
}

static bool sort_facts_created_desc( const void* pa, const void* pb, void* )
{
	SGRX_CAST( AIFact*, fa, pa );
	SGRX_CAST( AIFact*, fb, pb );
	return fa->created > fb->created;
}

void AIFactStorage::SortCreatedDesc()
{
	sgrx_combsort( facts.data(), facts.size(), sizeof(facts[0]), sort_facts_created_desc, NULL );
}

static bool sort_facts_expires_desc( const void* pa, const void* pb, void* )
{
	SGRX_CAST( AIFact*, fa, pa );
	SGRX_CAST( AIFact*, fb, pb );
	return fa->expires > fb->expires;
}

void AIFactStorage::Process( TimeVal curTime )
{
	m_lastTime = curTime;
	
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( facts[ i ].expires < curTime )
		{
			facts.uerase( i-- );
			break;
		}
	}
	
	if( facts.size() > 256 )
	{
		sgrx_combsort( facts.data(), facts.size(), sizeof(facts[0]), sort_facts_expires_desc, NULL );
		facts.resize( 256 );
	}
}

bool AIFactStorage::HasFact( uint32_t typemask )
{
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( (1<<facts[ i ].type) & typemask )
			return true;
	}
	return false;
}

bool AIFactStorage::ExpireFacts( uint32_t typemask )
{
	bool any = false;
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( (1<<facts[ i ].type) & typemask )
		{
			any = true;
			facts.uerase( i-- );
		}
	}
	return any;
}

bool AIFactStorage::HasRecentFact( uint32_t typemask, TimeVal maxtime )
{
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( ( (1<<facts[ i ].type) & typemask ) != 0 && facts[ i ].created + maxtime > m_lastTime )
			return true;
	}
	return false;
}

bool AIFactStorage::HasRecentFactAt( uint32_t typemask, TimeVal maxtime, Vec3 pos, float rad )
{
	float rad2 = rad * rad;
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( ( (1<<facts[ i ].type) & typemask ) != 0 &&
			facts[ i ].created + maxtime > m_lastTime &&
			( pos - facts[ i ].position ).LengthSq() < rad2 )
			return true;
	}
	return false;
}

AIFact* AIFactStorage::GetRecentFact( uint32_t typemask, TimeVal maxtime )
{
	AIFact* F = NULL;
//	puts("GetRecentFact");
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( ( (1<<facts[ i ].type) & typemask ) != 0 && facts[ i ].created + maxtime > m_lastTime )
		{
			F = &facts[ i ];
		//	printf("fact %p created at %d within %d\n", F, F->created, maxtime );
			maxtime = m_lastTime - facts[ i ].created;
		}
	}
	return F;
}

void AIFactStorage::Insert( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref )
{
	AIFact F = { m_next_fact_id++, ref, type, pos, created, expires };
//	printf( "INSERT FACT: type %d, pos: %g;%g;%g, created: %d, expires: %d\n",
//		(int)type, pos.x,pos.y,pos.z, (int)created, (int)expires );
	facts.push_back( F );
	last_mod_id = F.id;
}

bool AIFactStorage::Update( uint32_t type, Vec3 pos, float rad,
	TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	float rad2 = rad * rad;
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( facts[ i ].type == type &&
			( facts[ i ].position - pos ).LengthSq() < rad2 )
		{
			facts[ i ].position = pos;
			if( reset )
			{
				facts[ i ].created = created;
				facts[ i ].expires = expires;
			}
			facts[ i ].ref = ref;
			last_mod_id = facts[ i ].id;
			return true;
		}
	}
	
	return false;
}

void AIFactStorage::RemoveExt( uint32_t* types, size_t typecount )
{
	for( size_t i = 0; i < facts.size(); ++i )
	{
		size_t j = 0;
		for( ; j < typecount; ++j )
			if( facts[ i ].type == types[ j ] )
				break;
		if( j == typecount )
			continue;
		
		facts.erase( i-- );
	}
}

void AIFactStorage::InsertOrUpdate( uint32_t type, Vec3 pos, float rad,
	TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( Update( type, pos, rad, created, expires, ref, reset ) == false )
		Insert( type, pos, created, expires, ref );
}

bool AIFactStorage::CustomUpdate( AIFactDistance& distfn,
	TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	int which = -1;
	float mindist = FLT_MAX;
	for( size_t i = 0; i < facts.size(); ++i )
	{
		float dist = distfn.GetDistance( facts[ i ] );
		if( dist < mindist )
		{
			which = i;
			mindist = dist;
		}
	}
	
	if( which != -1 )
	{
		int i = which;
		facts[ i ].position = distfn.GetPosition();
		if( reset )
		{
			facts[ i ].created = created;
			facts[ i ].expires = expires;
		}
		facts[ i ].ref = ref;
		last_mod_id = facts[ i ].id;
		return true;
	}
	
	return false;
}

void AIFactStorage::CustomInsertOrUpdate( AIFactDistance& distfn, uint32_t type,
	TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( CustomUpdate( distfn, created, expires, ref, reset ) == false )
		Insert( type, distfn.GetPosition(), created, expires, ref );
}


AICharInfo& AIFactStorage::GetCharInfo( GameObject* obj )
{
	GameObject::ScrHandle h( obj );
	AICharInfo* ptr = charInfo.getptr( h );
	if( !ptr )
	{
		ptr = &charInfo[ h ];
		ptr->suspicion = 0;
		ptr->suspicionIncreased = false;
	}
	return *ptr;
}

void AIFactStorage::DecreaseSuspicion( float amt )
{
	for( size_t i = 0; i < charInfo.size(); ++i )
	{
		AICharInfo& ci = charInfo.item( i ).value;
		if( !ci.suspicionIncreased && !ci.IsSuspicious() )
			ci.suspicion = clamp( ci.suspicion - amt, 0, 1 );
		ci.suspicionIncreased = false;
	}
}


bool AIRoom::IsInside( Vec3 pos )
{
	bool in = false;
	for( size_t i = 0; i < parts.size(); ++i )
	{
		if( parts[ i ].negative )
			continue;
		
		Vec3 ixp = parts[ i ].inv_bbox_xf.TransformPos( pos ).Abs();
		if( ixp.x <= 1 && ixp.y <= 1 && ixp.z <= 1 )
		{
			in = true;
			break;
		}
	}
	if( in == false )
		return false; // not in any positive boxes
	
	for( size_t i = 0; i < parts.size(); ++i )
	{
		if( parts[ i ].negative == false )
			continue;
		
		Vec3 ixp = parts[ i ].inv_bbox_xf.TransformPos( pos ).Abs();
		if( ixp.x <= 1 && ixp.y <= 1 && ixp.z <= 1 )
			return false; // in a negative box
	}
	
	return true;
}



#define LC_FILE_AIRM_NAME "AIRM"
#define LC_FILE_AIRM_VERSION 0
struct LC_AIRoom
{
	StringView name;
	Mat4 transform;
	bool negative;
	float cell_size;
	
	template< class T > void Serialize( T& arch )
	{
		arch.stringView( name );
		arch << transform;
		arch << negative;
		arch << cell_size;
	}
};
struct LC_Chunk_AIRM
{
	Array< LC_AIRoom > rooms;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_AIRM_VERSION );
		svh << rooms;
	}
};

#define LC_FILE_AIZN_NAME "AIZN"
#define LC_FILE_AIZN_VERSION 0
struct LC_Chunk_AIZN
{
	uint32_t* defRestrict;
	float* defSuspicion;
	Array< AIZoneInfo >* zones;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_AIZN_VERSION );
		svh << *defRestrict;
		svh << *defSuspicion;
		svh << *zones;
	}
	static int _PrioPred( const void* a, const void* b )
	{
		SGRX_CAST( AIZoneInfo*, za, a );
		SGRX_CAST( AIZoneInfo*, zb, b );
		return za->priority != zb->priority ? ( za->priority < zb->priority ? 1 : -1 ) : 0;
	}
	void SortZones()
	{
		qsort( zones->data(), zones->size(), sizeof(AIZoneInfo), _PrioPred );
	}
};

struct LC_AIDB_Compiler : IEditorSystemCompiler
{
	LC_AIDB_Compiler()
	{
		aizn.defRestrict = &aizn_defRestrict;
		aizn.defSuspicion = &aizn_defSuspicion;
		aizn.zones = &aizn_zones;
	}
	bool GenerateChunk( ByteArray& out, sgsVariable sysParams )
	{
		// room chunk
		{
			ByteArray ch_airm;
			ByteWriter bw( &ch_airm );
			bw << airm;
			WrapChunk( ch_airm, LC_FILE_AIRM_NAME );
			out.append( ch_airm );
		}
		
		// zone chunk
		{
			aizn.SortZones();
			aizn_defRestrict = sysParams.getprop("AIDBSystem")
				.getprop("defaultRestrictions").get<uint32_t>();
			aizn_defSuspicion = sysParams.getprop("AIDBSystem")
				.getprop("defaultSuspicionFactor").get<float>();
			ByteArray ch_aizn;
			ByteWriter bw( &ch_aizn );
			bw << aizn;
			WrapChunk( ch_aizn, LC_FILE_AIZN_NAME );
			out.append( ch_aizn );
		}
		
		return true;
	}
	void ProcessEntity( EditorEntity& ent )
	{
		if( ent.type == "AIRoom" )
		{
			LC_AIRoom room =
			{
				ent.props.getprop("name").get<StringView>(),
				Mat4::CreateSRT(
					ent.props.getprop("scale").get<Vec3>(),
					DEG2RAD( ent.props.getprop("rotationXYZ").get<Vec3>() ),
					ent.props.getprop("position").get<Vec3>() ),
				ent.props.getprop("negative").get<bool>(),
				ent.props.getprop("cellSize").get<float>(),
			};
			airm.rooms.push_back( room );
			return;
		}
		if( ent.type == "AIZone" )
		{
			AIZoneInfo zi =
			{
				ent.props.getprop("restrictedGroups").get<uint32_t>(),
				ent.props.getprop("suspicionFactor").get<float>(),
				ent.props.getprop("priority").get<int32_t>(),
				Mat4::CreateSRT(
					ent.props.getprop("scale").get<Vec3>(),
					DEG2RAD( ent.props.getprop("rotationXYZ").get<Vec3>() ),
					ent.props.getprop("position").get<Vec3>()
				).Inverted()
			};
			aizn_zones.push_back( zi );
			return;
		}
	}
	
	LC_Chunk_AIRM airm;
	
	LC_Chunk_AIZN aizn;
	uint32_t aizn_defRestrict;
	float aizn_defSuspicion;
	Array< AIZoneInfo > aizn_zones;
};



AIDBSystem::AIDBSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	_InitScriptInterface( this );
	AddSelfToLevel( "aidb" );
	
	sgs_RegIntConst ric[] =
	{
		{ "AIS_Footstep", AIS_Footstep },
		{ "AIS_Shot", AIS_Shot },
		{ "AIS_AccessDenied", AIS_AccessDenied },
		
		{ "FT_Unknown", FT_Unknown },
		{ "FT_Sound_Noise", FT_Sound_Noise },
		{ "FT_Sound_Footstep", FT_Sound_Footstep },
		{ "FT_Sound_Shot", FT_Sound_Shot },
		{ "FT_Sound_AccessDenied", FT_Sound_AccessDenied },
		{ "FT_Sight_ObjectState", FT_Sight_ObjectState },
		{ "FT_Sight_Alarming", FT_Sight_Alarming },
		{ "FT_Sight_Friend", FT_Sight_Friend },
		{ "FT_Sight_Foe", FT_Sight_Foe },
		{ "FT_Position_Friend", FT_Position_Friend },
		{ "FT_Position_Foe", FT_Position_Foe },
		{ NULL, 0 },
	};
	sgs_RegIntConsts( m_level->GetSGSC(), ric, -1 );
	
	REGCOBJ( gcv_dbg_navmesh );
	
	Clear();
}

void AIDBSystem::Clear()
{
	m_sounds.clear();
	m_rooms.clear();
	m_zones.clear();
	m_globalFacts.Clear();
	m_defaultRestrictedGroups = 1;
	m_defaultSuspicionFactor = 1000;
}

bool AIDBSystem::CanHearSound( Vec3 pos, int i )
{
	AISound& S = m_sounds[ i ];
	return ( pos - S.position ).Length() < S.radius;
}

bool AIDBSystem::LoadChunk( const StringView& type, ByteView data )
{
	if( type == LC_FILE_PFND_NAME )
	{
		LOG_FUNCTION_ARG( "PFND chunk" );
		m_pathfinder.Load( data );
		return true;
	}
	if( type == LC_FILE_AIRM_NAME )
	{
		LOG_FUNCTION_ARG( "AIRM chunk" );
		LC_Chunk_AIRM airm;
		ByteReader br( data );
		br << airm;
		for( size_t i = 0; i < airm.rooms.size(); ++i )
		{
			AddRoomPart(
				airm.rooms[ i ].name,
				airm.rooms[ i ].transform,
				airm.rooms[ i ].negative,
				airm.rooms[ i ].cell_size
			);
		}
		return true;
	}
	if( type == LC_FILE_AIZN_NAME )
	{
		LOG_FUNCTION_ARG( "AIZN chunk" );
		LC_Chunk_AIZN aizn;
		aizn.defRestrict = &m_defaultRestrictedGroups;
		aizn.defSuspicion = &m_defaultSuspicionFactor;
		aizn.zones = &m_zones;
		ByteReader br( data );
		br << aizn;
		return true;
	}
	if( type == LC_FILE_COV2_NAME )
	{
		LOG_FUNCTION_ARG( "COV2 chunk" );
		LC_Chunk_COV2 cov2;
		cov2.covers = &m_coverParts;
		ByteReader br( data );
		br << cov2;
		_PrepareCoverRanges();
		return true;
	}
	return false;
}

IEditorSystemCompiler* AIDBSystem::EditorGetSystemCompiler()
{
	return new LC_AIDB_Compiler;
}

void AIDBSystem::AddSound( Vec3 pos, float rad, float timeout, AISoundType type )
{
	AISound S = { pos, rad, timeout, type };
	m_sounds.push_back( S );
}

void AIDBSystem::AddRoomPart( const StringView& name, Mat4 xf, bool negative, float cell_size )
{
	AIRoom* rh = m_rooms.getcopy( name );
	AIRoomPart part = { xf, xf.Inverted(), xf.GetScale(), negative, cell_size };
	if( rh )
	{
		rh->parts.push_back( part );
	}
	else
	{
		rh = new AIRoom;
		rh->m_key = name;
		rh->parts.push_back( part );
		m_rooms.set( rh->m_key, rh );
	}
}


AIRoom* AIDBSystem::FindRoomByPos( Vec3 pos )
{
	for( size_t i = 0; i < m_rooms.size(); ++i )
	{
		if( m_rooms.item( i ).value->IsInside( pos ) )
			return m_rooms.item( i ).value;
	}
	return NULL;
}

AIZoneInfo AIDBSystem::GetZoneInfoByPos( Vec3 pos )
{
	for( size_t i = 0; i < m_zones.size(); ++i )
	{
		Vec3 p = m_zones[ i ].invBBXF.TransformPos( pos );
		if( p.x >= -1 && p.x <= 1 &&
			p.y >= -1 && p.y <= 1 &&
			p.z >= -1 && p.z <= 1 )
			return m_zones[ i ];
	}
	AIZoneInfo zi = {0};
	zi.restrictedGroups = m_defaultRestrictedGroups;
	zi.suspicionFactor = m_defaultSuspicionFactor;
	return zi;
}


static int cover_parts_sort_by_polyref( const void* pa, const void* pb )
{
	SGRX_CAST( LC_CoverPart*, a, pa );
	SGRX_CAST( LC_CoverPart*, b, pb );
	if( a->polyRef != b->polyRef )
		return a->polyRef < b->polyRef ? -1 : 1;
	return 0;
}

void AIDBSystem::_PrepareCoverRanges()
{
	// sort covers for range compatibility
	qsort( m_coverParts.data(), m_coverParts.size(),
		sizeof(LC_CoverPart), cover_parts_sort_by_polyref );
	
	// generate ranges
	m_tileCoverRanges.clear();
	uint32_t lastref = 0;
	size_t start = 0;
	for( size_t i = 0; i < m_coverParts.size(); ++i )
	{
		uint32_t newref = m_coverParts[ i ].polyRef;
		if( newref != lastref )
		{
			if( lastref )
			{
				AICoverRange r = { start, i };
				m_tileCoverRanges.set( lastref, r );
			}
			lastref = newref;
			start = i;
		}
	}
	if( m_coverParts.size() )
	{
		AICoverRange r = { start, m_coverParts.size() };
		m_tileCoverRanges.set( lastref, r );
	}
	LOG << "[aidb] Prepared " << m_tileCoverRanges.size()
		<< " cover ranges for " << m_coverParts.size() << " covers";
}

LC_CoverPart* AIDBSystem::FindCover( Vec3 pos, Vec3 target, uint32_t mask, uint32_t req, bool inv )
{
	// some of these parameters can be made configurable
#define COVER_POLY_VISIT_LIMIT 64
#define MAX_COVER_DISTANCE 8.0f
	
	m_fcVisitedSet.clear();
	m_fcPolyQueue.clear();
	size_t numSearched = 0;
	uint32_t firstPoly = m_pathfinder.FindPoly( pos );
	bool checkdir = pos != target;
	if( firstPoly )
		m_fcPolyQueue.push_back( firstPoly );
	while( numSearched < m_fcPolyQueue.size() )
	{
		uint32_t poly = m_fcPolyQueue[ numSearched++ ];
		m_fcVisitedSet.set( poly, NoValue() );
		if( ( m_pathfinder.GetPolyPos( poly ) - pos ).LengthSq() >
			MAX_COVER_DISTANCE * MAX_COVER_DISTANCE )
			continue;
		
		// process covers
		AICoverRange* covers = m_tileCoverRanges.getptr( poly );
		if( covers )
		{
			for( uint32_t cid = covers->from; cid < covers->to; ++cid )
			{
				LC_CoverPart* cp = &m_coverParts[ cid ];
				// cover does not fit requirements
				if( ( ( cp->flags & mask ) != req ) ^ inv )
					continue;
				// cover does not work
				Vec2 cdir = V2(
					target.x - ( cp->p0.x + cp->p1.x ) * 0.5f,
					target.y - ( cp->p0.y + cp->p1.y ) * 0.5f
				).Normalized();
				if( checkdir &&
					Vec2Dot( cdir, cp->n.ToVec2().Normalized() ) < cosf(DEG2RAD(45)) )
					continue;
				
				return cp;
			}
		}
		
		// search limit
		if( numSearched >= COVER_POLY_VISIT_LIMIT )
			break;
		
		// add sub-covers to queue
		uint32_t neighbors[ 8 ];
		int count = m_pathfinder.GetPolyNeighbors( poly, neighbors, 8 );
		for( int i = 0; i < count; ++i )
		{
			uint32_t np = neighbors[ i ];
			if( !m_fcVisitedSet.isset( np ) )
				m_fcPolyQueue.push_back( np );
		}
	}
	
	return NULL;
}


void AIDBSystem::Tick( float deltaTime, float blendFactor )
{
	for( size_t i = 0; i < m_sounds.size(); ++i )
	{
		AISound& S = m_sounds[ i ];
		S.timeout -= deltaTime;
		if( S.timeout <= 0 )
		{
			m_sounds.uerase( i-- );
		}
	}
}

void AIDBSystem::FixedTick( float deltaTime )
{
	m_globalFacts.Process( m_level->GetPhyTime() );
}

void AIDBSystem::DebugDrawWorld()
{
	if( gcv_dbg_navmesh.value )
		m_pathfinder.DebugDraw();
}

void AIDBSystem::sgsAddSound( Vec3 pos, float rad, float timeout, int type )
{
	AddSound( pos, rad, timeout, (AISoundType) type );
}

bool AIDBSystem::sgsHasFact( uint32_t typemask )
{
	return m_globalFacts.HasFact( typemask );
}

bool AIDBSystem::sgsHasRecentFact( uint32_t typemask, TimeVal maxtime )
{
	return m_globalFacts.HasRecentFact( typemask, maxtime );
}

SGS_MULTRET AIDBSystem::sgsGetRecentFact( uint32_t typemask, TimeVal maxtime )
{
	AIFact* F = m_globalFacts.GetRecentFact( typemask, maxtime );
	if( F )
	{
		sgs_CreateLiteClassFrom( C, NULL, F );
		return 1;
	}
	return 0;
}

void AIDBSystem::sgsInsertFact( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref )
{
	m_globalFacts.Insert( type, pos, created, expires, ref );
}

bool AIDBSystem::sgsUpdateFact( uint32_t type, Vec3 pos,
	float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( sgs_StackSize( C ) < 7 )
		reset = true;
	return m_globalFacts.Update( type, pos, rad, created, expires, ref, reset );
}

void AIDBSystem::sgsInsertOrUpdateFact( uint32_t type, Vec3 pos,
	float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( sgs_StackSize( C ) < 7 )
		reset = true;
	m_globalFacts.InsertOrUpdate( type, pos, rad, created, expires, ref, reset );
}

SGS_MULTRET AIDBSystem::sgsPushRoom( AIRoom* room )
{
	Array< Vec3 > points;
	
	// first add points
	for( size_t i = 0; i < room->parts.size(); ++i )
	{
		const AIRoomPart& RP = room->parts[ i ];
		if( RP.negative || RP.cell_size < 0.01f )
			continue;
		
		int xcount = floorf( RP.scale.x / RP.cell_size );
		int ycount = floorf( RP.scale.y / RP.cell_size );
		float xncs = RP.cell_size / RP.scale.x;
		float yncs = RP.cell_size / RP.scale.y;
		for( int y = 0; y < ycount; ++y )
		{
			for( int x = 0; x < xcount; ++x )
			{
				Vec3 pos = V3( ( x * 2 - (xcount-1) ) * xncs, ( y * 2 - (ycount-1) ) * yncs, 0 );
				points.push_back( RP.bbox_xf.TransformPos( pos ) );
			}
		}
	}
	
	// then exclude them from
	for( size_t i = 0; i < room->parts.size(); ++i )
	{
		const AIRoomPart& RP = room->parts[ i ];
		if( RP.negative == false )
			continue;
		
		for( size_t p = 0; p < points.size(); ++p )
		{
			Vec3 ixp = RP.inv_bbox_xf.TransformPos( points[ p ] ).Abs();
			if( ixp.x <= 1 && ixp.y <= 1 && ixp.z <= 1 )
				points.erase( p-- );
		}
	}
	
	// copy to sgs array
	for( size_t i = 0; i < points.size(); ++i )
	{
		sgs_PushVar( C, points[ i ] );
	}
	return sgs_CreateArray( C, NULL, points.size() );
}

SGS_MULTRET AIDBSystem::sgsGetRoomList()
{
	for( size_t i = 0; i < m_rooms.size(); ++i )
	{
		sgs_PushVar( C, m_rooms.item( i ).key );
	}
	return sgs_CreateArray( C, NULL, m_rooms.size() );
}

sgsString AIDBSystem::sgsGetRoomNameByPos( Vec3 pos )
{
	AIRoom* room = FindRoomByPos( pos );
	if( room == NULL )
		return sgsString();
	StringView name = room->m_key;
	return sgsString( C, name.data(), name.size() );
}

SGS_MULTRET AIDBSystem::sgsGetRoomByPos( Vec3 pos )
{
	AIRoom* room = FindRoomByPos( pos );
	if( room == NULL )
		return 0;
	return sgsPushRoom( room );
}

SGS_MULTRET AIDBSystem::sgsGetRoomPoints( StringView name )
{
	AIRoom* room = m_rooms.getcopy( name );
	if( room == NULL )
		return 0;
	return sgsPushRoom( room );
}

SGS_MULTRET AIDBSystem::sgsGetZoneInfoByPos( Vec3 pos )
{
	AIZoneInfo zi = GetZoneInfoByPos( pos );
	sgs_CreateLiteClassFrom( C, NULL, &zi );
	return 1;
}


DevelopSystem::DevelopSystem( GameLevel* lev ) :
	IGameLevelSystem( lev, e_system_uid ),
	screenshotMode(false), moveMult(false), moveFwd(false), moveBwd(false),
	moveLft(false), moveRgt(false), moveUp(false), moveDn(false),
	rotView(false), rotLft(false), rotRgt(false),
	cameraPos(V3(0)), cameraDir(YP(0,0)), cameraRoll(0),
	consoleMode(false), justEnabledConsole(false)
{
	RegisterHandler( EID_WindowEvent );
	REGCOBJ( gcv_cl_show_log );
	REGCOBJ( gcv_dbg_list );
}

void DevelopSystem::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	if( eid == EID_WindowEvent )
	{
		SGRX_CAST( Event*, ev, edata.GetUserData() );
		if( m_level->GetEditorMode() == false &&
			ev->type == SDL_KEYDOWN &&
			ev->key.repeat == 0 &&
			ev->key.keysym.sym == SDLK_F2 )
		{
			screenshotMode = !screenshotMode;
			if( screenshotMode )
			{
				SGRX_Camera& CAM = m_level->GetScene()->camera;
				cameraPos = CAM.position;
				cameraDir = YP( CAM.direction );
				cameraRoll = 0;
			}
			gcv_cl_gui.value = !screenshotMode;
			gcv_g_paused.value = screenshotMode;
			Game_ShowCursor( screenshotMode );
		}
		if( ev->type == SDL_KEYDOWN && ev->key.repeat == 0 && ev->key.keysym.sym == SDLK_BACKQUOTE )
		{
			consoleMode = !consoleMode;
			gcv_g_paused.value = consoleMode;
			justEnabledConsole = consoleMode;
		}
		if( ev->type == SDL_KEYDOWN || ev->type == SDL_KEYUP )
		{
			bool dn = ev->type == SDL_KEYDOWN;
			uint32_t k = ev->key.keysym.sym;
			if( k == SDLK_LSHIFT ) moveMult = dn;
			if( k == SDLK_w ) moveFwd = dn;
			if( k == SDLK_s ) moveBwd = dn;
			if( k == SDLK_a ) moveLft = dn;
			if( k == SDLK_d ) moveRgt = dn;
			if( k == SDLK_z ) moveUp = dn;
			if( k == SDLK_x ) moveDn = dn;
			if( k == SDLK_q ) rotLft = dn;
			if( k == SDLK_e ) rotRgt = dn;
			
			if( consoleMode && dn )
			{
				if( k == SDLK_BACKSPACE )
				{
					if( ev->key.keysym.mod & KMOD_CTRL )
					{
						size_t pos = StringView(inputText).find_last_at( " ", inputText.size() - 1, 0 );
						inputText.resize( pos );
					}
					else if( inputText.size() )
						inputText.pop_back();
				}
				else if( k == SDLK_RETURN || k == SDLK_KP_ENTER )
				{
					LOG << ">" << StackString<1024>(inputText).str;
					if( Game_DoCommand( inputText ) == false )
						LOG << "ERROR: could not find command";
					inputText.clear();
				}
			}
		}
		if( ev->type == SDL_TEXTINPUT )
		{
			if( consoleMode && !justEnabledConsole )
				inputText.append( ev->text.text );
		}
		if( ev->type == SDL_MOUSEBUTTONDOWN || ev->type == SDL_MOUSEBUTTONUP )
		{
			bool dn = ev->type == SDL_MOUSEBUTTONDOWN;
			uint32_t b = ev->button.button;
			if( b == SGRX_MB_RIGHT ) rotView = dn;
		}
	}
}

void DevelopSystem::Tick( float deltaTime, float blendFactor )
{
	if( screenshotMode )
	{
		float speed = deltaTime;
		if( moveMult )
			speed *= 10;
		
		Vec3 up = V3(0,0,1);
		Vec3 dir = cameraDir.ToVec3();
		Vec3 right = Vec3Cross( dir, up ).Normalized();
		Mat4 rollMat = Mat4::CreateRotationAxisAngle( dir, cameraRoll );
		right = rollMat.TransformNormal( right ).Normalized();
		
		cameraPos += dir * ( moveFwd - moveBwd ) * speed;
		cameraPos += right * ( moveRgt - moveLft ) * speed;
		cameraPos += V3(0,0,1) * ( moveUp - moveDn ) * speed;
		cameraRoll += ( rotRgt - rotLft ) * 0.5f * speed;
		if( rotView )
		{
			Vec2 cpos = Game_GetCursorPos();
			Game_SetCursorPos( GR_GetWidth() / 2, GR_GetHeight() / 2 );
			Vec2 opos = V2( GR_GetWidth() / 2, GR_GetHeight() / 2 );
			Vec2 curmove = ( cpos - opos ) * -0.01f;
			cameraDir.yaw += curmove.x;
			cameraDir.pitch = clamp( cameraDir.pitch + curmove.y,
				-M_PI*0.5f + SMALL_FLOAT,
				M_PI*0.5f - SMALL_FLOAT );
		}
	}
	
	if( justEnabledConsole )
		justEnabledConsole = false;
}

void DevelopSystem::PreRender()
{
	if( screenshotMode )
	{
		Vec3 up = V3(0,0,1);
		Vec3 dir = cameraDir.ToVec3();
		Mat4 rollMat = Mat4::CreateRotationAxisAngle( dir, cameraRoll );
		
		SGRX_Camera& CAM = m_level->GetScene()->camera;
		CAM.position = cameraPos;
		CAM.direction = dir;
		CAM.updir = rollMat.TransformNormal( up ).Normalized();
		CAM.UpdateMatrices();
	}
}

void DevelopSystem::DrawUI()
{
	if( consoleMode )
	{
		int w = GR_GetWidth();
		int y_cline = 12;
		int y_bsize = 1;
		int y_end = GR_GetHeight() / 3;
		int y_logend = y_end - y_cline - y_bsize * 2;
		
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
		br.Col( 0.2f, 0.3f, 0.4f, 0.5f );
		br.QuadWH( 0, 0, w, y_logend );
		br.Col( 0.4f, 0.05f, 0.01f, 0.5f );
		br.QuadWH( 0, y_end - y_cline - y_bsize, w, y_cline );
		br.Col( 0.5f, 0.6f, 0.7f, 1.0f );
		br.QuadWH( 0, y_logend, w, y_bsize );
		br.QuadWH( 0, y_end - y_bsize, w, y_bsize );
		
		br.Col( 0.9f, 0.9f, 0.9f, 1.0f );
		GR2D_SetFont( "core", y_cline );
		GR2D_DrawTextLine( 10, y_end - y_bsize - y_cline / 2, ">", HALIGN_RIGHT, VALIGN_CENTER );
		GR2D_DrawTextLine( 10, y_end - y_bsize - y_cline / 2, inputText, HALIGN_LEFT, VALIGN_CENTER );
	}
	
	if( gcv_cl_show_log.value )
	{
		GR2D_SetFont( "system_outlined", 7 );
		GR2D_DrawTextLine( 0, 0, "Log: TODO" );
	}
	
	switch( gcv_dbg_list.value )
	{
	case DBG_LIST_NONE: break;
	case DBG_LIST_INFOTARGETS: {
		GR2D_SetFont( "system_outlined", 7 );
		GR2D_DrawTextLine( 0, 0, "Info targets:" );
		const HashTable< GameObject*, NoValue >& objSet = m_level->m_infoEmitSet.m_gameObjects;
		for( size_t i = 0; i < objSet.size(); ++i )
		{
			GameObject* o = objSet.item( i ).key;
			Vec3 p = o->GetWorldPosition();
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "%04d | obj=%p flags=%08X pos=[%.4g;%.4g;%.4g]",
				int(i), o, o->m_infoMask, p.x, p.y, p.z );
			GR2D_DrawTextLine( 0, 11 + i * 9, bfr );
		}
		} break;
	}
}


