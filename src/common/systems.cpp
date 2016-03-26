

#include "systems.hpp"
#include "entities.hpp"
#include "level.hpp"



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

void LevelMapSystem::OnRemoveEntity( Entity* e )
{
	RemoveItem( e );
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

void LevelMapSystem::UpdateItem( Entity* e, const MapItemInfo& data )
{
	m_mapItemData[ e ] = data;
}

void LevelMapSystem::RemoveItem( Entity* e )
{
	m_mapItemData.unset( e );
}

void LevelMapSystem::DrawUIRect( float x0, float y0, float x1, float y1, float linesize )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	float map_aspect = safe_fdiv( x1 - x0, y1 - y0 );
	
	br.Reset();
	
	Mat4 lookat = Mat4::CreateLookAt( V3( viewPos.x, viewPos.y, -0.5f ), V3(0,0,1), V3(0,-1,0) );
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
	
	br.Flush();
	GR2D_UnsetViewport();
	GR2D_UnsetScissorRect();
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
}

void LevelMapSystem::sgsUpdate( EntityScrHandle e, int type, Vec3 pos, Vec3 dir, float szfwd, float szrt )
{
	MapItemInfo mii = { type, pos, dir, szfwd, szrt };
	UpdateItem( e, mii );
}

void LevelMapSystem::sgsRemove( EntityScrHandle e )
{
	RemoveItem( e );
}


MessagingSystem::MessagingSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	m_tx_icon_info = GR_GetTexture( "ui/icon_info.png" );
	m_tx_icon_warning = GR_GetTexture( "ui/icon_warning.png" );
	m_tx_icon_cont = GR_GetTexture( "ui/icon_cont.png" );
	
	_InitScriptInterface( this );
	AddSelfToLevel( "messages" );
	
	sgs_RegIntConst ric[] =
	{
		{ "MT_Continued", MSMessage::Continued },
		{ "MT_Info", MSMessage::Info },
		{ "MT_Warning", MSMessage::Warning },
		{ NULL, 0 },
	};
	sgs_RegIntConsts( m_level->GetSGSC(), ric, -1 );
}

void MessagingSystem::Clear()
{
	m_messages.clear();
}

void MessagingSystem::AddMessage( MSMessage::Type type, const StringView& sv, float tmlength )
{
	m_messages.push_back( MSMessage() );
	MSMessage& msg = m_messages.last();
	msg.type = type;
	msg.text = sv;
	msg.tmlength = tmlength;
	msg.position = 0;
}

void MessagingSystem::Tick( float deltaTime, float blendFactor )
{
	for( size_t i = m_messages.size(); i > 0; )
	{
		i--;
		MSMessage& msg = m_messages[ i ];
		msg.position += deltaTime;
		if( msg.position > msg.tmlength + 2 )
		{
			m_messages.erase( i );
		}
	}
}

void MessagingSystem::DrawUI()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	GR2D_SetFont( "core", 16 );
	
	float y = 0;
	
	for( size_t i = 0; i < m_messages.size(); ++i )
	{
		MSMessage& msg = m_messages[ i ];
		float q = smoothlerp_range( msg.position, 0, 1, 1 + msg.tmlength, 2 + msg.tmlength );
		y += 10 - ( 1 - q ) * 50;
		
		br.Reset();
		br.Col( 0, 0.5f * q );
		br.Quad( 100, y, 450, y + 40 );
		br.Col( 1 );
		switch( msg.type )
		{
		case MSMessage::Info: br.SetTexture( m_tx_icon_info ); break;
		case MSMessage::Warning: br.SetTexture( m_tx_icon_warning ); break;
		case MSMessage::Continued:
		default:
			br.SetTexture( m_tx_icon_cont );
			break;
		}
		br.Quad( 110, y+10, 130, y+30 );
		br.UnsetTexture();
		br.Col( 1, 0.9f * q );
		GR2D_DrawTextLine( 150, y + 20, msg.text, HALIGN_LEFT, VALIGN_CENTER );
		
		y += 40;
	}
}

void MessagingSystem::sgsAddMsg( int type, StringView text, float time )
{
	AddMessage( (MSMessage::Type) type, text, sgs_StackSize( C ) >= 3 ? time : 3.0f );
}


ObjectiveSystem::ObjectiveSystem( GameLevel* lev ) :
	IGameLevelSystem( lev, e_system_uid ),
	SHOW_OBJECTIVES( "show_objectives" ),
	m_alpha(0)
{
	m_tx_icon_open = GR_GetTexture( "ui/obj_open.png" );
	m_tx_icon_done = GR_GetTexture( "ui/obj_done.png" );
	m_tx_icon_failed = GR_GetTexture( "ui/obj_failed.png" );
	
	_InitScriptInterface( this );
	AddSelfToLevel( "objectives" );
	
	sgs_RegIntConst ric[] =
	{
		{ "OS_Hidden", OSObjective::Hidden },
		{ "OS_Open", OSObjective::Open },
		{ "OS_Done", OSObjective::Done },
		{ "OS_Failed", OSObjective::Failed },
		{ "OS_Cancelled", OSObjective::Cancelled },
		{ NULL, 0 },
	};
	sgs_RegIntConsts( m_level->GetSGSC(), ric, -1 );
}

void ObjectiveSystem::Clear()
{
	m_objectives.clear();
}

int ObjectiveSystem::AddObjective(
	const StringView& title,
	OSObjective::State state,
	const StringView& desc,
	bool required,
	Vec3* location )
{
	int out = m_objectives.size();
	m_objectives.push_back( OSObjective() );
	OSObjective& obj = m_objectives.last();
	obj.title = title;
	obj.state = state;
	obj.desc = desc;
	obj.required = required;
	obj.hasLocation = location != NULL;
	obj.location = location ? *location : V3(0);
	return out;
}

OSObjStats ObjectiveSystem::GetStats()
{
	OSObjStats out = { m_objectives.size(), 0, 0, 0, 0, 0 };
	for( size_t i = 0; i < m_objectives.size(); ++i )
	{
		if( m_objectives[ i ].state == OSObjective::Hidden ) out.numHidden++;
		if( m_objectives[ i ].state == OSObjective::Open ) out.numOpen++;
		if( m_objectives[ i ].state == OSObjective::Done ) out.numDone++;
		if( m_objectives[ i ].state == OSObjective::Failed ) out.numFailed++;
		if( m_objectives[ i ].state == OSObjective::Cancelled ) out.numCancelled++;
	}
	return out;
}

void ObjectiveSystem::Tick( float deltaTime, float blendFactor )
{
	float tgt = SHOW_OBJECTIVES.value ? 1 : 0;
	float diff = tgt - m_alpha;
	m_alpha += sign( diff ) * TMIN( diff > 0 ? deltaTime * 3 : deltaTime, fabsf( diff ) );
}

void ObjectiveSystem::DrawUI()
{
	int W = GR_GetWidth();
	int H = GR_GetHeight();
	BatchRenderer& br = GR2D_GetBatchRenderer();
	GR2D_SetFont( "core", 16 );
	
	br.Reset();
	br.Col( 0, 0.5f * m_alpha );
	br.Quad( 0, 0, W, H );
	
	br.Col( 1, m_alpha );
	GR2D_DrawTextLine( 200, 10, "Objectives:" );
	
	float y = 40;
	
	for( size_t i = 0; i < m_objectives.size(); ++i )
	{
		OSObjective& obj = m_objectives[ i ];
		if( obj.state == OSObjective::Hidden )
			continue;
		
		switch( obj.state )
		{
		default:
		case OSObjective::Open: br.SetTexture( m_tx_icon_open ); break;
		case OSObjective::Done: br.SetTexture( m_tx_icon_done ); break;
		case OSObjective::Failed: br.SetTexture( m_tx_icon_failed ); break;
		}
		if( obj.state != OSObjective::Cancelled )
		{
			br.Quad( 210, y, 240, y + 30 );
		}
		
		GR2D_DrawTextLine( 250, y+15, obj.title, HALIGN_LEFT, VALIGN_CENTER );
		if( obj.state == OSObjective::Cancelled )
		{
			br.Reset();
			br.Col( 1, m_alpha );
			br.Quad( 250, y + 17, 250 + GR2D_GetTextLength( obj.title ), y + 19 );
		}
		y += 30;
	}
}

bool ObjectiveSystem::_CheckRange( int i )
{
	if( i < 0 || i >= int(m_objectives.size()) )
	{
		sgs_Msg( C, SGS_WARNING, "objective ID out of bounds (requested id=%d, got %d)", i, int(m_objectives.size()) );
		return true;
	}
	return false;
}

int ObjectiveSystem::sgsAddObj( StringView title, int state, StringView desc, bool req, Vec3 loc )
{
	return AddObjective( title, (OSObjective::State) state, desc, req, sgs_StackSize( C ) >= 5 ? &loc : NULL );
}

StringView ObjectiveSystem::sgsGetTitle( int i )
{
	if( _CheckRange( i ) )
		return "";
	return m_objectives[ i ].title;
}

void ObjectiveSystem::sgsSetTitle( int i, StringView title )
{
	if( _CheckRange( i ) )
		return;
	m_objectives[ i ].title = title;
}

int ObjectiveSystem::sgsGetState( int i )
{
	if( _CheckRange( i ) )
		return 0;
	return m_objectives[ i ].state;
}

void ObjectiveSystem::sgsSetState( int i, int state )
{
	if( _CheckRange( i ) )
		return;
	m_objectives[ i ].state = (OSObjective::State) state;
}

void ObjectiveSystem::sgsSetLocation( int i, Vec3 loc )
{
	if( _CheckRange( i ) )
		return;
	m_objectives[ i ].hasLocation = sgs_StackSize( C ) > 1;
	m_objectives[ i ].location = loc;
}



static SGRX_HelpTextRenderer m_defaultRenderer;

HelpTextSystem::HelpTextSystem( GameLevel* lev ) :
	IGameLevelSystem( lev, e_system_uid ),
	m_alpha(0), m_fadeTime(0), m_fadeTo(0), renderer(&m_defaultRenderer)
{
	_InitScriptInterface( this );
	AddSelfToLevel( "helptext" );
}

void HelpTextSystem::Clear()
{
	m_text = "";
	m_alpha = 0;
	m_fadeTime = 0;
	m_fadeTo = 0;
}

void HelpTextSystem::SetText( StringView text, float alpha, float fadetime, float fadeto )
{
	m_text = text;
	m_alpha = alpha;
	m_fadeTime = fadetime;
	m_fadeTo = fadeto;
}

void HelpTextSystem::Tick( float deltaTime, float blendFactor )
{
	if( m_fadeTime > 0 && m_level->IsPaused() == false )
	{
		float diff = m_fadeTo - m_alpha;
		m_alpha += TMIN( fabsf( diff ), deltaTime / m_fadeTime ) * sign( diff );
	}
}

void HelpTextSystem::DrawText()
{
	if( m_text.size() && m_alpha > 0 )
	{
		renderer->opacity = m_alpha;
		renderer->RenderText( m_text );
	}
}

void HelpTextSystem::sgsClear()
{
	Clear();
}

void HelpTextSystem::sgsSetText( StringView text, float alpha, float fadetime, float fadeto )
{
	int ssz = sgs_StackSize( C );
	SetText( text, ssz >= 2 ? alpha : 1, ssz >= 3 ? fadetime : 0, ssz >= 4 ? fadeto : 0 );
}



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
	lev->m_lightTree = &m_ltSamples;
}

void LevelCoreSystem::Clear()
{
	m_meshInsts.clear();
	m_lightInsts.clear();
	m_levelBodies.clear();
	m_lights.clear();
	m_ltSamples.SetSamples( NULL, 0 );
}

bool LevelCoreSystem::LoadChunk( const StringView& type, ByteView data )
{
	if( type != LC_FILE_GEOM_NAME )
		return false;
	
	Array< LC_MeshInst > meshInstDefs;
	LC_PhysicsMesh phyMesh;
	Array< LC_SolidBox > solidBoxes;
	LC_Chunk_Geom geom = { &meshInstDefs, &m_lights, &m_ltSamples, &phyMesh, &solidBoxes };
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
				sgrx_snprintf( subbfr, sizeof(subbfr), SGRX_LEVELS_DIR "%.*s" SGRX_LEVEL_DIR_SFX "%.*s", TMIN( (int) levelname.size(), 200 ), levelname.data(), TMIN( (int) src.size() - 1, 200 ), src.data() + 1 );
				MI->SetMesh( subbfr );
			}
			else
				MI->SetMesh( src );
			
			if( MID.m_lmap.width && MID.m_lmap.height )
			{
				TextureHandle lmtex = GR_CreateTexture( MID.m_lmap.width, MID.m_lmap.height, TEXFORMAT_RGBA8,
					TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y, 1, MID.m_lmap.data.data() );
				MI->SetMITexture( 0, lmtex );
				
				TextureHandle nmtex = GR_CreateTexture( MID.m_lmap.width, MID.m_lmap.height, TEXFORMAT_RGBA8,
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
			else if( ( MID.m_flags & LM_MESHINST_DECAL ) != 0 && MI->GetMesh() )
			{
				MI->SetAllMtlFlags( SGRX_MtlFlag_Decal, 0 );
				MI->SetAllBlendModes( SGRX_MtlBlend_Basic );
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
				lh->shadowTexture = GR_CreateRenderTexture( L.num_shadow_samples, L.num_shadow_samples, RT_FORMAT_DEPTH );
			lh->UpdateTransform();
			m_lightInsts.push_back( lh );
		}
	}
	
	return true;
}


GFXSystem::GFXSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	lev->GetScene()->director = this;
}

void GFXSystem::OnAddEntity( Entity* ent )
{
	if( ENTITY_IS_A( ent, ReflectionPlaneEntity ) )
	{
		m_reflectPlanes.push_back( ent );
	}
}

void GFXSystem::OnRemoveEntity( Entity* ent )
{
	if( ENTITY_IS_A( ent, ReflectionPlaneEntity ) )
	{
		m_reflectPlanes.remove_first( ent );
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
#define RT_REFL 0xffe0
	
	// shortcuts
	SGRX_Scene* scene = info.scene;
	
	int reflW = GR_GetWidth();
	int reflH = GR_GetHeight();
	if( info.viewport )
	{
		reflW = info.viewport->x1 - info.viewport->x0;
		reflH = info.viewport->y1 - info.viewport->y0;
	}
	SGRX_Camera origCamera = scene->camera;
	SGRX_Viewport* origViewport = info.viewport;
	
	// initial actions
	int shadow_pass_id = scene->FindPass( SGRX_FP_Shadow );
	if( m_curMode != SGRX_RDMode_Unlit )
	{
		ctrl->RenderShadows( scene, shadow_pass_id );
	}
	ctrl->SortRenderItems( scene );
	
	// RENDER REFLECTIONS
	TextureHandle rttREFL = GR_GetRenderTarget( reflW, reflH, RT_FORMAT_COLOR_HDR16, RT_REFL );
	DepthStencilSurfHandle dssREFL = GR_GetDepthStencilSurface( reflW, reflH, RT_FORMAT_COLOR_HDR16, RT_REFL );
	
	GR_PreserveResource( rttREFL );
	GR_PreserveResource( dssREFL );
	
	for( size_t i = 0; i < m_reflectPlanes.size(); ++i )
	{
		Entity* RPE = m_reflectPlanes[ i ];
		
		Vec3 pos = RPE->GetWorldPosition();
		Vec3 dir = RPE->LocalToWorldDir( V3(0,0,1) ).Normalized();
		Vec4 plane = V4( dir, Vec3Dot( dir, pos ) );
		
		scene->camera.position = Vec3ReflectPos( scene->camera.position, plane );
		scene->camera.direction = Vec3Reflect( scene->camera.direction, plane.ToVec3() );
		scene->camera.updir = Vec3Reflect( scene->camera.updir, plane.ToVec3() );
		scene->camera.UpdateMatrices();
		info.viewport = NULL;
		
		OnDrawSceneGeom( ctrl, info, rttREFL, dssREFL, NULL );
	}
	
	scene->camera = origCamera;
	info.viewport = origViewport;
	
	// RENDER MAIN SCENE
	ctrl->m_overrideTextures[ 10 ] = scene->skyTexture;
	ctrl->m_overrideTextures[ 11 ] = rttREFL; // GR_GetTexture( "textures/unit.png" );
	GR_PreserveResource( ctrl->m_overrideTextures[ 0 ] );
	SGRX_RenderDirector::OnDrawScene( ctrl, info );
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
	if( FS_LoadTextFile( "data/damage.dat", mtlconfig ) == false )
		return( "Failed to load data/damage.dat" );
	
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
					"data/damage.dat", TMIN( 250, (int) value.size() ), value.data() );
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
		mtl->particles.PreRender();
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

void BulletSystem::Add( Vec3 pos, Vec3 vel, float timeleft, float dmg, uint32_t ownerType )
{
	Bullet B = { pos, vel, vel.Normalized(), timeleft, dmg, ownerType };
	m_bullets.push_back( B );
}

float BulletSystem::Zap( Vec3 p1, Vec3 p2, float dmg, uint32_t ownerType )
{
	Vec3 dir = ( p2 - p1 ).Normalized();
	Bullet B = { p1, dir * dmg, dir, 0, dmg, ownerType };
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
			if( mii && mii->ownerType == B.ownerType )
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
				MI_BulletHit_Data data = { hitpoint, B.velocity };
				mii->OnEvent( HIT.meshinst, MIEVT_BulletHit, &data );
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

bool AIFactStorage::HasRecentFact( uint32_t typemask, TimeVal maxtime )
{
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( ( (1<<facts[ i ].type) & typemask ) != 0 && facts[ i ].created + maxtime > m_lastTime )
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

struct LC_AIDB_Compiler : IEditorSystemCompiler
{
	bool GenerateChunk( ByteArray& out )
	{
		ByteWriter bw( &out );
		bw << data;
		WrapChunk( out, LC_FILE_AIRM_NAME );
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
			data.rooms.push_back( room );
			ent.remove = true;
			return;
		}
	}
	
	LC_Chunk_AIRM data;
};



AIDBSystem::AIDBSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	_InitScriptInterface( this );
	AddSelfToLevel( "aidb" );
	
	sgs_RegIntConst ric[] =
	{
		{ "AIS_Footstep", AIS_Footstep },
		{ "AIS_Shot", AIS_Shot },
		
		{ "FT_Unknown", FT_Unknown },
		{ "FT_Sound_Noise", FT_Sound_Noise },
		{ "FT_Sound_Footstep", FT_Sound_Footstep },
		{ "FT_Sound_Shot", FT_Sound_Shot },
		{ "FT_Sight_ObjectState", FT_Sight_ObjectState },
		{ "FT_Sight_Alarming", FT_Sight_Alarming },
		{ "FT_Sight_Friend", FT_Sight_Friend },
		{ "FT_Sight_Foe", FT_Sight_Foe },
		{ "FT_Position_Friend", FT_Position_Friend },
		{ "FT_Position_Foe", FT_Position_Foe },
		{ NULL, 0 },
	};
	sgs_RegIntConsts( m_level->GetSGSC(), ric, -1 );
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

SGS_MULTRET AIDBSystem::sgsGetRecentFact( sgs_Context* coro, uint32_t typemask, TimeVal maxtime )
{
	AIFact* F = m_globalFacts.GetRecentFact( typemask, maxtime );
	if( F )
	{
		sgs_CreateLiteClassFrom( coro, NULL, F );
		return 1;
	}
	return 0;
}

void AIDBSystem::sgsInsertFact( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref )
{
	m_globalFacts.Insert( type, pos, created, expires, ref );
}

bool AIDBSystem::sgsUpdateFact( sgs_Context* coro, uint32_t type, Vec3 pos,
	float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( sgs_StackSize( coro ) < 7 )
		reset = true;
	return m_globalFacts.Update( type, pos, rad, created, expires, ref, reset );
}

void AIDBSystem::sgsInsertOrUpdateFact( sgs_Context* coro, uint32_t type, Vec3 pos,
	float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( sgs_StackSize( coro ) < 7 )
		reset = true;
	m_globalFacts.InsertOrUpdate( type, pos, rad, created, expires, ref, reset );
}

SGS_MULTRET AIDBSystem::sgsPushRoom( sgs_Context* coro, AIRoom* room )
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
	
	// global exclusion using covers
	CoverSystem* coverSys = m_level->GetSystem<CoverSystem>();
	if( coverSys )
	{
		for( size_t i = 0; i < coverSys->m_edgeMeshes.size(); ++i )
		{
			for( size_t p = 0; p < points.size(); ++p )
			{
				if( coverSys->m_edgeMeshes[ i ]->PointInBox( points[ p ] ) )
					points.erase( p-- );
			}
		}
	}
	
	// copy to sgs array
	for( size_t i = 0; i < points.size(); ++i )
	{
		sgs_PushVar( coro, points[ i ] );
	}
	return sgs_CreateArray( coro, NULL, points.size() );
}

SGS_MULTRET AIDBSystem::sgsGetRoomList( sgs_Context* coro )
{
	for( size_t i = 0; i < m_rooms.size(); ++i )
	{
		sgs_PushVar( coro, m_rooms.item( i ).key );
	}
	return sgs_CreateArray( coro, NULL, m_rooms.size() );
}

sgsString AIDBSystem::sgsGetRoomNameByPos( sgs_Context* coro, Vec3 pos )
{
	AIRoom* room = FindRoomByPos( pos );
	if( room == NULL )
		return sgsString();
	StringView name = room->m_key;
	return sgsString( coro, name.data(), name.size() );
}

SGS_MULTRET AIDBSystem::sgsGetRoomByPos( sgs_Context* coro, Vec3 pos )
{
	AIRoom* room = FindRoomByPos( pos );
	if( room == NULL )
		return 0;
	return sgsPushRoom( coro, room );
}

SGS_MULTRET AIDBSystem::sgsGetRoomPoints( sgs_Context* coro, StringView name )
{
	AIRoom* room = m_rooms.getcopy( name );
	if( room == NULL )
		return 0;
	return sgsPushRoom( coro, room );
}


void CSCoverInfo::Clear()
{
	planes.clear();
	shapes.clear();
	covers.clear();
	factors.clear();
}

bool CSCoverInfo::GetPosition( Vec3 position, float distpow, Vec3& out, float interval )
{
	if( covers.size() == 0 )
		return false;
	
	// generate cover line goodness factors
	factors.resize( covers.size() );
	for( size_t cid = 0; cid < covers.size(); ++cid )
	{
		float dist = PointLineDistance( position, covers[ cid ].p0, covers[ cid ].p1 );
		factors[ cid ] = TLERP( randf(), 1/(1+dist), distpow );
	}
	
	// pick best cover line
	size_t found_cid = _GetBestFactorID();
	CSCoverLine clin = covers[ found_cid ];
	
	// calculate cover point placement on line
	float len = ( clin.p1 - clin.p0 ).Length();
	int count = floor( len / interval ) + 1;
	float hoff = fmodf( len, interval ) / 2;
	float qstart = hoff / len;
	float qdt = interval / len;
	
	// generate cover point goodness factors
	factors.resize( count );
	float q = qstart;
	for( int i = 0; i < count; ++i, q += qdt )
	{
		Vec3 pt = TLERP( clin.p0, clin.p1, q );
		float dist = ( pt - position ).Length();
		factors[ i ] = TLERP( randf(), 1/(1+dist), distpow );
	}
	
	// pick best cover point
	size_t found_ptid = _GetBestFactorID();
	out = TLERP( clin.p0, clin.p1, qstart + qdt * found_ptid );
	
	return true;
}

void CSCoverInfo::ClipWithSpheres( Vec4* spheres, int count )
{
	for( int sid = 0; sid < count; ++sid )
	{
		Vec4 sphere = spheres[ sid ];
		Vec3 sP = sphere.ToVec3();
		float sR = sphere.w;
		
		size_t cover_count = covers.size();
		for( size_t cid = 0; cid < cover_count; ++cid )
		{
			CSCoverLine clin = covers[ cid ];
			
			Vec3 pN = ( clin.p1 - clin.p0 ).Normalized();
			float pDs = Vec3Dot( pN, sP );
			float pD0 = Vec3Dot( pN, clin.p0 );
			float pD1 = Vec3Dot( pN, clin.p1 );
			if( fabsf( pD1 - pD0 ) > SMALL_FLOAT )
			{
				float fs = ( pDs - pD0 ) / ( pD1 - pD0 );
				Vec3 sphere_proj = TLERP( clin.p0, clin.p1, fs );
				float dist = ( sphere_proj - sP ).Length();
				float pushfac = sR * cosf( asinf( clamp( dist / sR, -1, 1 ) ) );
				if( pushfac > SMALL_FLOAT )
				{
					float fs0 = fs - pushfac / fabsf( pD1 - pD0 );
					float fs1 = fs + pushfac / fabsf( pD1 - pD0 );
					// 6 cases
					if( fs0 <= 0 )
					{
						if( fs1 <= 0 ) continue; // start/end before line, skip
						else if( fs1 < 1 )
						{
							// start-before/end-middle, clip beginning
							covers[ cid ].p0 = TLERP( clin.p0, clin.p1, fs1 );
							continue;
						}
						else
						{
							// start-before/end-after, remove the whole line
							covers.uerase( cid-- );
							cover_count--;
						}
					}
					else if( fs0 < 1 )
					{
						if( fs1 < 1 )
						{
							// start/end at middle, clip middle out
							CSCoverLine nl0 = { clin.p0, TLERP( clin.p0, clin.p1, fs0 ) };
							CSCoverLine nl1 = { TLERP( clin.p0, clin.p1, fs1 ), clin.p1 };
							covers[ cid ] = nl0;
							covers.push_back( nl1 );
						}
						else
						{
							// start-middle/end-after, clip end
							covers[ cid ].p1 = TLERP( clin.p0, clin.p1, fs0 );
							continue;
						}
					}
					else continue; // start/end after line, skip
				}
				else continue; // sphere didn't hit the line
			}
			else continue; // line too short to clip
		}
	}
}

size_t CSCoverInfo::_GetBestFactorID()
{
	float q = -1;
	size_t id = NOT_FOUND;
	for( size_t i = 0; i < factors.size(); ++i )
	{
		float nq = factors[ i ];
		if( nq > q )
		{
			q = nq;
			id = i;
		}
	}
	return id;
}

bool _IntersectLinePlane( CSCoverLine& ioline, Vec4 plane )
{
	float sigdst0 = Vec3Dot( plane.ToVec3(), ioline.p0 ) - plane.w;
	float sigdst1 = Vec3Dot( plane.ToVec3(), ioline.p1 ) - plane.w;
	if( sigdst0 <= 0 && sigdst1 <= 0 )
		return true;
	float q = safe_fdiv( -sigdst0, sigdst1 - sigdst0 );
	if( sigdst0 <= 0 )
	{
		ioline.p0 = TLERP( ioline.p0, ioline.p1, q );
	}
	else if( sigdst1 <= 0 )
	{
		ioline.p1 = TLERP( ioline.p0, ioline.p1, q );
	}
	return false;
}

void CSCoverInfo::_CullWithShadowLines( size_t firstcover, Vec4 P )
{
	for( size_t covid = firstcover; covid < covers.size(); ++covid )
	{
		bool rem = _IntersectLinePlane( covers[ covid ], P );
		if( rem )
			covers.uerase( covid-- );
	}
}

void CSCoverInfo::_CullWithSolids()
{
	for( size_t sid = 0; sid < shapes.size(); ++sid )
	{
		const Shape& S = shapes[ sid ];
		
		size_t cover_count = covers.size();
		for( size_t covid = 0; covid < cover_count; ++covid )
		{
			CSCoverLine clin = covers[ covid ];
			CSCoverLine clout[2];
			int cloutnum = 0;
			
			for( int plid = 0; plid < S.numPlanes; ++plid )
			{
				Vec4 plane = planes[ S.offset + plid ];
				float sigdst0 = Vec3Dot( plane.ToVec3(), clin.p0 ) - plane.w;
				float sigdst1 = Vec3Dot( plane.ToVec3(), clin.p1 ) - plane.w;
				if( sigdst0 >= 0 && sigdst1 >= 0 )
				{
					clout[ 0 ] = clin;
					cloutnum = 1;
					break;
				}
				float q = safe_fdiv( -sigdst0, sigdst1 - sigdst0 );
				Vec3 isp = TLERP( clin.p0, clin.p1, q );
				
				// check if intersection point is inside solid
				bool ipin = true;
				for( int i = 0; i < S.numPlanes; ++i )
				{
					Vec4 P = planes[ S.offset + i ];
					if( Vec3Dot( P.ToVec3(), isp ) > P.w + SMALL_FLOAT )
					{
						// point outside, ignore it
						ipin = false;
						break;
					}
				}
				
				if( ipin )
				{
					if( sigdst0 >= 0 ) // p0 in front, p1 behind
					{
						if( ( clin.p0 - isp ).LengthSq() > SMALL_FLOAT )
						{
							ASSERT( cloutnum < 2 );
							clout[ cloutnum ].p0 = clin.p0;
							clout[ cloutnum ].p1 = isp;
							cloutnum++;
							clin.p0 = isp;
						}
					}
					else if( sigdst1 >= 0 ) // p1 in front, p0 behind
					{
						if( ( clin.p1 - isp ).LengthSq() > SMALL_FLOAT )
						{
							ASSERT( cloutnum < 2 );
							clout[ cloutnum ].p1 = clin.p1;
							clout[ cloutnum ].p0 = isp;
							cloutnum++;
							clin.p1 = isp;
						}
					}
				}
				
				// if isp outside solid, it's not the right intersection point
				// if( sigdst0 < 0 && sigdst1 < 0 ) --> ambiguous outcome, must resolve with other planes
			}
			
			// update situation
			if( cloutnum == 0 )
			{
				covers.uerase( covid-- );
				cover_count--;
			}
			else if( cloutnum == 1 )
			{
				covers[ covid ] = clout[ 0 ];
			}
			else // if( cloutnum == 2 )
			{
				covers[ covid ] = clout[ 0 ];
				covers.push_back( clout[ 1 ] );
			}
		}
	}
}

static const float cos135deg = cosf( FLT_PI * 0.75f );
void CoverSystem::CoverPoint::AdjustNormals( Vec3 newout, Vec3 newup )
{
	Vec3 oldrt = Vec3Cross( nout, nup ).Normalized();
	Vec3 newrt = Vec3Cross( newout, newup ).Normalized();
	
	float dot = clamp( Vec3Dot( oldrt, newrt ), -1, 1 );
	if( dot < cos135deg )
	{
		nout += newout;
		nout.Normalize();
	}
	else
	{
		nout = ( nout + newout ).Normalized() / cosf( 0.5f * acosf( dot ) );
	}
	
	nup += newup;
	nup.Normalize();
}

bool CoverSystem::EdgeMesh::InAABB( const Vec3& ibmin, const Vec3& ibmax ) const
{
	return bbmin.x < ibmax.x
		&& bbmin.y < ibmax.y
		&& bbmin.z < ibmax.z
		&& ibmin.x < bbmax.x
		&& ibmin.y < bbmax.y
		&& ibmin.z < bbmax.z;
}

bool CoverSystem::EdgeMesh::PointInBox( Vec3 pt ) const
{
	Vec3 ixp = inv_bbox_xf.TransformPos( pt );
	return ixp.x >= obb_min.x
		&& ixp.y >= obb_min.y
		&& ixp.z >= obb_min.z
		&& ixp.x <= obb_max.x
		&& ixp.y <= obb_max.y
		&& ixp.z <= obb_max.z;
}

void CoverSystem::EdgeMesh::CalcCoverLines()
{
	coverpts.clear();
	coveridcs.clear();
	
	for( size_t i = 0; i < edges.size(); ++i )
	{
		const Edge& E = edges[ i ];
		bool is0 = Vec3Dot( E.n0, V3(0,0,-1) ) > 0.707f;
		bool is1 = Vec3Dot( E.n1, V3(0,0,-1) ) > 0.707f;
		if( ( is0 && is1 == false ) || ( is1 && is0 == false ) )
		{
			Vec3 nout = is0 ? E.n1 : E.n0;
			Vec3 nup = is0 ? -E.n0 : -E.n1;
			CoverPoint cp0 = { E.p0, nout, nup };
			CoverPoint cp1 = { E.p1, nout, nup };
			
			size_t pos0 = coverpts.find_first_at( cp0 );
			size_t pos1 = coverpts.find_first_at( cp1 );
			
			if( pos0 == NOT_FOUND )
			{
				pos0 = coverpts.size();
				coverpts.push_back( cp0 );
			}
			else
			{
				coverpts[ pos0 ].AdjustNormals( nout, nup );
			}
			
			if( pos1 == NOT_FOUND )
			{
				pos1 = coverpts.size();
				coverpts.push_back( cp1 );
			}
			else
			{
				coverpts[ pos1 ].AdjustNormals( nout, nup );
			}
			
			coveridcs.push_back( pos0 );
			coveridcs.push_back( pos1 );
		}
	}
}



#define LC_FILE_COVR_NAME "COVR"
#define LC_FILE_COVR_VERSION 0
struct LC_AICover
{
	StringView name;
	Mat4 transform;
	
	template< class T > void Serialize( T& arch )
	{
		arch.stringView( name );
		arch << transform;
	}
};
struct LC_Chunk_COVR
{
	Array< LC_AICover > covers;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_COVR_VERSION );
		svh << covers;
	}
};

struct LC_CoverSys_Compiler : IEditorSystemCompiler
{
	bool GenerateChunk( ByteArray& out )
	{
		ByteWriter bw( &out );
		bw << data;
		WrapChunk( out, LC_FILE_COVR_NAME );
		return true;
	}
	void ProcessEntity( EditorEntity& ent )
	{
		if( ent.type == "AICover" )
		{
			LC_AICover cover =
			{
				ent.props.getprop("name").get<StringView>(),
				Mat4::CreateSRT(
					ent.props.getprop("scale").get<Vec3>(),
					DEG2RAD( ent.props.getprop("rotationXYZ").get<Vec3>() ),
					ent.props.getprop("position").get<Vec3>() ),
			};
			data.covers.push_back( cover );
			ent.remove = true;
			return;
		}
	}
	
	LC_Chunk_COVR data;
};



bool CoverSystem::LoadChunk( const StringView& type, ByteView data )
{
	if( type == LC_FILE_COVR_NAME )
	{
		LC_Chunk_COVR chunk;
		ByteReader br( data );
		br << chunk;
		for( size_t i = 0; i < chunk.covers.size(); ++i )
		{
			AddAABB( chunk.covers[ i ].name, V3(-1), V3(1), chunk.covers[ i ].transform );
		}
		return true;
	}
	return false;
}

IEditorSystemCompiler* CoverSystem::EditorGetSystemCompiler()
{
	return new LC_CoverSys_Compiler;
}

void CoverSystem::Clear()
{
	m_edgeMeshes.clear();
	m_edgeMeshesByName.clear();
}

void CoverSystem::AddAABB( StringView name, Vec3 bbmin, Vec3 bbmax, Mat4 mtx )
{
	EdgeMesh* EM = new EdgeMesh;
	EM->m_key = name;
	EM->pos = mtx.TransformPos( V3(0) );
	EM->inv_bbox_xf = Mat4::Identity;
	mtx.InvertTo( EM->inv_bbox_xf );
	EM->obb_min = bbmin;
	EM->obb_max = bbmax;
	EM->enabled = true;
	
	Mat4 ntx;
	mtx.GenNormalMatrix( ntx );
	
	Edge edges[12] =
	{
		// X
		{ V3(bbmin.x,bbmin.y,bbmin.z), V3(bbmax.x,bbmin.y,bbmin.z), V3(0,-1,0), V3(0,0,-1) },
		{ V3(bbmin.x,bbmax.y,bbmin.z), V3(bbmax.x,bbmax.y,bbmin.z), V3(0,1,0), V3(0,0,-1) },
		{ V3(bbmin.x,bbmin.y,bbmax.z), V3(bbmax.x,bbmin.y,bbmax.z), V3(0,-1,0), V3(0,0,1) },
		{ V3(bbmin.x,bbmax.y,bbmax.z), V3(bbmax.x,bbmax.y,bbmax.z), V3(0,1,0), V3(0,0,1) },
		// Y
		{ V3(bbmin.x,bbmin.y,bbmin.z), V3(bbmin.x,bbmax.y,bbmin.z), V3(-1,0,0), V3(0,0,-1) },
		{ V3(bbmax.x,bbmin.y,bbmin.z), V3(bbmax.x,bbmax.y,bbmin.z), V3(1,0,0), V3(0,0,-1) },
		{ V3(bbmin.x,bbmin.y,bbmax.z), V3(bbmin.x,bbmax.y,bbmax.z), V3(-1,0,0), V3(0,0,1) },
		{ V3(bbmax.x,bbmin.y,bbmax.z), V3(bbmax.x,bbmax.y,bbmax.z), V3(1,0,0), V3(0,0,1) },
		// Z
		{ V3(bbmin.x,bbmin.y,bbmin.z), V3(bbmin.x,bbmin.y,bbmax.z), V3(-1,0,0), V3(0,-1,0) },
		{ V3(bbmax.x,bbmin.y,bbmin.z), V3(bbmax.x,bbmin.y,bbmax.z), V3(1,0,0), V3(0,-1,0) },
		{ V3(bbmin.x,bbmax.y,bbmin.z), V3(bbmin.x,bbmax.y,bbmax.z), V3(-1,0,0), V3(0,1,0) },
		{ V3(bbmax.x,bbmax.y,bbmin.z), V3(bbmax.x,bbmax.y,bbmax.z), V3(1,0,0), V3(0,1,0) },
	};
	
	for( int i = 0; i < 12; ++i )
	{
		edges[ i ].p0 = mtx.TransformPos( edges[ i ].p0 );
		edges[ i ].p1 = mtx.TransformPos( edges[ i ].p1 );
		edges[ i ].n0 = ntx.TransformNormal( edges[ i ].n0 ).Normalized();
		edges[ i ].n1 = ntx.TransformNormal( edges[ i ].n1 ).Normalized();
		edges[ i ].d0 = Vec3Dot( edges[ i ].p0, edges[ i ].n0 );
		edges[ i ].d1 = Vec3Dot( edges[ i ].p1, edges[ i ].n1 );
	}
	
	Vec4 planes[6] =
	{
		V4( edges[4].n0, Vec3Dot( edges[4].n0, edges[4].p0 ) ), // X-
		V4( edges[5].n0, Vec3Dot( edges[5].n0, edges[5].p0 ) ), // X+
		V4( edges[0].n0, Vec3Dot( edges[0].n0, edges[0].p0 ) ), // Y-
		V4( edges[1].n0, Vec3Dot( edges[1].n0, edges[1].p0 ) ), // Y+
		V4( edges[0].n1, Vec3Dot( edges[0].n1, edges[0].p0 ) ), // Z-
		V4( edges[2].n1, Vec3Dot( edges[2].n1, edges[2].p0 ) ), // Z+
	};
	
	EM->edges.assign( edges, 12 );
	EM->planes.assign( planes, 6 );
	EM->CalcCoverLines();
	
	EM->bbmin = V3(FLT_MAX);
	EM->bbmax = V3(-FLT_MAX);
	for( int i = 0; i < 12; ++i )
	{
		EM->bbmin = Vec3::Min( Vec3::Min( edges[ i ].p0, edges[ i ].p1 ), EM->bbmin );
		EM->bbmax = Vec3::Max( Vec3::Max( edges[ i ].p0, edges[ i ].p1 ), EM->bbmax );
	}
	
	m_edgeMeshes.push_back( EM );
	m_edgeMeshesByName.set( EM->m_key, EM );
}

void CoverSystem::QueryLines( Vec3 bbmin, Vec3 bbmax, float dist,
	float height, Vec3 viewer, bool visible, CSCoverInfo& cinfo )
{
	cinfo.Clear();
	
	for( size_t emid = 0; emid < m_edgeMeshes.size(); ++emid )
	{
		EdgeMesh* EM = m_edgeMeshes[ emid ];
		if( EM->enabled == false )
			continue;
		if( EM->InAABB( bbmin, bbmax ) == false )
			continue;
		
		// produce lines
		size_t cover_off = cinfo.covers.size();
		for( size_t i = 0; i < EM->coveridcs.size(); i += 2 )
		{
			const CoverPoint& cp0 = EM->coverpts[ EM->coveridcs[ i + 0 ] ];
			const CoverPoint& cp1 = EM->coverpts[ EM->coveridcs[ i + 1 ] ];
			CSCoverLine outcl =
			{
				cp0.pos + cp0.nout * dist + cp0.nup * height,
				cp1.pos + cp1.nout * dist + cp1.nup * height,
			};
			cinfo.covers.push_back( outcl );
		}
		
		// shadow clip
		if( visible == false )
		{
			// cut away shadow planes
			for( size_t i = 0; i < EM->edges.size(); ++i )
			{
				Edge E = EM->edges[ i ];
				bool is0 = Vec3Dot( E.n0, viewer ) > E.d0;
				bool is1 = Vec3Dot( E.n1, viewer ) > E.d1;
				
				if( ( is0 && is1 == false ) || ( is0 == false && is1 ) )
				{
					// silhouette edge
					// - generate plane
					Vec3 pN = Vec3Cross( E.p0 - viewer, E.p1 - viewer ).Normalized();
					if( Vec3Dot( pN, E.n0 ) < 0 || Vec3Dot( pN, E.n1 ) < 0 )
						pN = -pN;
					float pD = Vec3Dot( pN, viewer );
					
					// - push plane
					Vec4 P = V4( pN, pD );
					P.w -= ( 1 - fabsf( Vec3Dot( P.ToVec3(), V3(0,0,1) ) ) ) * dist;
					cinfo._CullWithShadowLines( cover_off, -P );
				}
			}
			for( size_t i = 0; i < EM->planes.size(); ++i )
			{
				Vec4 P = EM->planes[ i ];
				if( Vec3Dot( P.ToVec3(), viewer ) > P.w )
				{
					P.w -= ( 1 - fabsf( Vec3Dot( P.ToVec3(), V3(0,0,1) ) ) ) * dist;
					cinfo._CullWithShadowLines( cover_off, -P );
				}
			}
		}
		else
		{
			// generate solid from shadow planes
			CSCoverInfo::Shape sh = { cinfo.planes.size(), 0 };
			cinfo.shapes.push_back( sh );
			for( size_t i = 0; i < EM->edges.size(); ++i )
			{
				Edge E = EM->edges[ i ];
				bool is0 = Vec3Dot( E.n0, viewer ) > E.d0;
				bool is1 = Vec3Dot( E.n1, viewer ) > E.d1;
				
				if( ( is0 && is1 == false ) || ( is0 == false && is1 ) )
				{
					// silhouette edge
					// - generate plane
					Vec3 pN = Vec3Cross( E.p0 - viewer, E.p1 - viewer ).Normalized();
					if( Vec3Dot( pN, E.n0 ) < 0 || Vec3Dot( pN, E.n1 ) < 0 )
						pN = -pN;
					float pD = Vec3Dot( pN, viewer );
					
					// - push plane
					Vec4 P = V4( pN, pD );
					P.w += ( 1 - fabsf( Vec3Dot( P.ToVec3(), V3(0,0,1) ) ) ) * dist;
					
					cinfo.planes.push_back( P );
					cinfo.shapes.last().numPlanes++;
				}
			}
			for( size_t i = 0; i < EM->planes.size(); ++i )
			{
				Vec4 P = EM->planes[ i ];
				if( Vec3Dot( P.ToVec3(), viewer ) > P.w )
				{
				//	P.w -= ( 1 - fabsf( Vec3Dot( P.ToVec3(), V3(0,0,1) ) ) ) * dist;
					
					cinfo.planes.push_back( P );
					cinfo.shapes.last().numPlanes++;
				}
			}
		}
		
		// generate solids
		{
			CSCoverInfo::Shape sh = { cinfo.planes.size(), 0 };
			cinfo.shapes.push_back( sh );
			for( size_t i = 0; i < EM->planes.size(); ++i )
			{
				Vec4 P = EM->planes[ i ];
				P.w += dist - SMALL_FLOAT;
				cinfo.planes.push_back( P );
				cinfo.shapes.last().numPlanes++;
			}
		}
	}
	
	cinfo._CullWithSolids();
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
}

void DevelopSystem::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	if( eid == EID_WindowEvent )
	{
		SGRX_CAST( Event*, ev, edata.GetUserData() );
		if( m_level->GetEditorMode() == false &&
			ev->type == SDL_KEYDOWN && ev->key.repeat == 0 && ev->key.keysym.sym == SDLK_F2 )
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
}


