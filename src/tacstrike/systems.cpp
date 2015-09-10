

#include "systems.hpp"
#include "level.hpp"



InfoEmissionSystem::InfoEmissionSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	_InitScriptInterface( "infoEmitters" );
	
	sgs_RegIntConst ric[] =
	{
		{ "IEST_InteractiveItem", IEST_InteractiveItem },
		{ "IEST_HeatSource", IEST_HeatSource },
		{ "IEST_Player", IEST_Player },
		{ "IEST_MapItem", IEST_MapItem },
		{ "IEST_AIAlert", IEST_AIAlert },
		{ NULL, 0 },
	};
	sgs_RegIntConsts( m_level->GetSGSC(), ric, -1 );
}

void InfoEmissionSystem::Clear()
{
	m_emissionData.clear();
}

void InfoEmissionSystem::UpdateEmitter( Entity* e, const Data& data )
{
	m_emissionData[ e ] = data;
}

void InfoEmissionSystem::RemoveEmitter( Entity* e )
{
	m_emissionData.unset( e );
}

bool InfoEmissionSystem::QuerySphereAny( const Vec3& pos, float rad, uint32_t types )
{
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		float dst = rad + D.radius;
		if( ( D.pos - pos ).LengthSq() < dst * dst )
			return true;
	}
	return false;
}

bool InfoEmissionSystem::QuerySphereAll( IESProcessor* proc, const Vec3& pos, float rad, uint32_t types )
{
	bool ret = false;
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		
		float dst = rad + D.radius;
		if( ( D.pos - pos ).LengthSq() >= dst * dst )
			continue;
		
		ret = true;
		if( proc->Process( m_emissionData.item( i ).key, D ) == false )
			return true;
	}
	return ret;
}

bool InfoEmissionSystem::QueryBB( const Mat4& mtx, uint32_t types )
{
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		Vec3 tp = mtx.TransformPos( D.pos );
		if( tp.x >= -1 && tp.x <= 1 &&
			tp.y >= -1 && tp.y <= 1 &&
			tp.z >= -1 && tp.z <= 1 )
			return true;
	}
	return false;
}

Entity* InfoEmissionSystem::QueryOneRay( const Vec3& from, const Vec3& to, uint32_t types )
{
	Vec3 dir = to - from;
	float maxdist = dir.Length();
	dir /= maxdist;
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		
		float outdst[1];
		if( RaySphereIntersect( from, dir, D.pos, D.radius, outdst ) && outdst[0] < maxdist )
			return m_emissionData.item( i ).key;
	}
	return NULL;
}

void InfoEmissionSystem::sgsUpdate( Entity::Handle e, Vec3 pos, float rad, uint32_t types )
{
	if( e.not_null() == false )
	{
		sgs_Msg( C, SGS_WARNING, "given pointer is not an entity" );
		return;
	}
	Data d = { pos, rad, types };
	UpdateEmitter( e, d );
}

void InfoEmissionSystem::sgsRemove( Entity::Handle e )
{
	if( e.not_null() == false )
	{
		sgs_Msg( C, SGS_WARNING, "given pointer is not an entity" );
		return;
	}
	RemoveEmitter( e );
}


LevelMapSystem::LevelMapSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid ), m_viewPos(V2(0))
{
	m_tex_mapline = GR_GetTexture( "ui/mapline.png" );
	m_tex_mapframe = GR_GetTexture( "ui/mapframe.png" );
}

void LevelMapSystem::Clear()
{
	m_mapItemData.clear();
}

bool LevelMapSystem::LoadChunk( const StringView& type, uint8_t* ptr, size_t size )
{
	if( type != LC_FILE_MAPL_NAME )
		return false;
	
	LOG_FUNCTION_ARG( "MAPL chunk" );
	
	LC_Chunk_Mapl parser = { &m_lines };
	ByteReader br( ptr, size );
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

void LevelMapSystem::DrawUI()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	int size_x = GR_GetWidth();
	int size_y = GR_GetHeight();
	int sqr = TMIN( size_x, size_y );
	
	int safe_margin = sqr * 1 / 16;
	int mapsize_x = sqr * 4 / 10;
	int mapsize_y = sqr * 3 / 10;
	int msm = 0; // sqr / 100;
	float map_aspect = mapsize_x / (float) mapsize_y;
	int x1 = size_x - safe_margin;
	int x0 = x1 - mapsize_x;
	int y0 = safe_margin;
	int y1 = y0 + mapsize_y;
	
	br.Reset().Col( 0, 0.5f );
	br.Quad( x0, y0, x1, y1 );
	br.Flush();
	
	br.Reset().SetTexture( NULL ).Col( 0.2f, 0.4f, 0.8f );
	
	Mat4 lookat = Mat4::CreateLookAt( V3( m_viewPos.x, m_viewPos.y, -0.5f ), V3(0,0,1), V3(0,-1,0) );
	GR2D_SetViewMatrix( lookat * Mat4::CreateScale( 1.0f / ( 8 * map_aspect ), 1.0f / 8, 1 ) );
	
	GR2D_SetScissorRect( x0, y0, x1, y1 );
	GR2D_SetViewport( x0, y0, x1, y1 );
	
	for( size_t i = 0; i < m_lines.size(); i += 2 )
	{
		Vec2 l0 = m_lines[ i ];
		Vec2 l1 = m_lines[ i + 1 ];
		
		br.TexLine( l0, l1, 0.1f );
	}
	
//	MapItemDraw ed;
//	m_infoEmitters.QuerySphereAll( &ed, V3( pos.x, pos.y, 1 ), 100, IEST_MapItem );
	for( size_t i = 0; i < m_mapItemData.size(); ++i )
	{
		MapItemInfo& mii = m_mapItemData.item( i ).value;
		Vec2 viewpos = mii.position.ToVec2();
		
		if( ( mii.type & MI_Mask_Object ) == MI_Object_Player )
		{
			br.Reset().SetTexture( m_tex_mapline )
				.Col( 0.2f, 0.9f, 0.1f ).Box( viewpos.x, viewpos.y, 1, 1 );
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
	
	br.Flush();
	GR2D_UnsetViewport();
	GR2D_UnsetScissorRect();
	
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	br.Reset().SetTexture( m_tex_mapframe ).Quad( x0 - msm, y0 - msm, x1 + msm, y1 + msm ).Flush();
}


MessagingSystem::MessagingSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	m_tx_icon_info = GR_GetTexture( "ui/icon_info.png" );
	m_tx_icon_warning = GR_GetTexture( "ui/icon_warning.png" );
	m_tx_icon_cont = GR_GetTexture( "ui/icon_cont.png" );
	
	_InitScriptInterface( "messages" );
	
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
	
	_InitScriptInterface( "objectives" );
	
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


FlareSystem::FlareSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	m_ps_flare = GR_GetPixelShader( "flare" );
	m_tex_flare = GR_GetTexture( "textures/fx/flare.png" );
	
	_InitScriptInterface( "flares" );
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
	br.ShaderData.push_back( V4( W, H, 1.0f / W, 1.0f / H ) );
	br.ShaderData.push_back( V4(1) );
	for( size_t i = 0; i < m_flares.size(); ++i )
	{
		FSFlare& FD = m_flares.item( i ).value;
		if( FD.enabled == false || FD.size <= 0 )
			continue;
		br.ShaderData[1] = V4( FD.color, 0.1f / ( ( FD.pos - cam.position ).Length() + 1 ) );
		Vec3 screenpos = cam.WorldToScreen( FD.pos );
		if( Vec3Dot( FD.pos, cam.direction ) < Vec3Dot( cam.position, cam.direction ) )
			continue;
		SceneRaycastCallback_Any srcb;
		m_level->GetScene()->RaycastAll( cam.position, FD.pos, &srcb );
		if( srcb.m_hit )
			continue;
	//	LOG << screenpos.z;
		float dx = cos(0.1f)*0.5f*sz * FD.size;
		float dy = sin(0.1f)*0.5f*sz * FD.size;
		br.TurnedBox( screenpos.x * W, screenpos.y * H, dx, dy );
		br.Flush();
	}
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
	m_levelBodies.clear();
	m_lights.clear();
	m_ltSamples.SetSamples( NULL, 0 );
}

bool LevelCoreSystem::AddEntity( const StringView& type, sgsVariable data )
{
	///////////////////////////
	if( type == "solidbox" )
	{
		Vec3 scale = data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>();
		SGRX_PhyRigidBodyInfo rbinfo;
		rbinfo.group = 2;
		rbinfo.shape = m_level->GetPhyWorld()->CreateAABBShape( -scale, scale );
		rbinfo.mass = 0;
		rbinfo.inertia = V3(0);
		rbinfo.position = data.getprop("position").get<Vec3>();
		rbinfo.rotation = Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion();
		m_levelBodies.push_back( m_level->GetPhyWorld()->CreateRigidBody( rbinfo ) );
		return true;
	}
	
	return false;
}

bool LevelCoreSystem::LoadChunk( const StringView& type, uint8_t* ptr, size_t size )
{
	if( type != LC_FILE_GEOM_NAME )
		return false;
	
	Array< LC_MeshInst > meshInstDefs;
	LC_PhysicsMesh phyMesh;
	LC_Chunk_Geom geom = { &meshInstDefs, &m_lights, &m_ltSamples, &phyMesh };
	ByteReader br( ptr, size );
	br << geom;
	if( br.error )
	{
		LOG_ERROR << "Failed to load GEOM (LevelCoreSystem) chunk";
		return true;
	}
	
	// LOAD FLARES
	FlareSystem* FS = m_level->GetSystem<FlareSystem>();
	if( FS )
	{
		for( size_t i = 0; i < m_lights.size(); ++i )
		{
			LC_Light& L = m_lights[ i ];
			if( L.type != LM_LIGHT_POINT && L.type != LM_LIGHT_SPOT )
				continue;
			FSFlare FD = { L.pos + L.flareoffset, L.color, L.flaresize, true };
			FS->UpdateFlare( &m_lights[ i ], FD );
		}
	}
	
	// create static geometry
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
				sgrx_snprintf( subbfr, sizeof(subbfr), "levels/%.*s%.*s", TMIN( (int) levelname.size(), 200 ), levelname.data(), TMIN( (int) src.size() - 1, 200 ), src.data() + 1 );
				MI->mesh = GR_GetMesh( subbfr );
			}
			else
				MI->mesh = GR_GetMesh( src );
			
			if( MID.m_lmap.width && MID.m_lmap.height )
			{
				MI->textures[0] = GR_CreateTexture( MID.m_lmap.width, MID.m_lmap.height, TEXFORMAT_RGBA8,
					TEXFLAGS_LERP_X | TEXFLAGS_LERP_Y | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y, 1 );
				MI->textures[0]->UploadRGBA8Part( MID.m_lmap.data.data(),
					0, 0, 0, MID.m_lmap.width, MID.m_lmap.height );
			}
			else
			{
				MI->dynamic = true;
				for( int i = 10; i < 16; ++i )
					MI->constants[ i ] = V4(0.15f);
			}
			
			MI->matrix = MID.m_mtx;
			
			if( MID.m_flags & LM_MESHINST_DYNLIT )
			{
				MI->dynamic = true;
				m_level->LightMesh( MI );
			}
			
			if( MID.m_flags & LM_MESHINST_DECAL )
			{
				MI->decal = true;
				MI->transparent = true;
				MI->sortidx = MID.m_decalLayer;
			}
			
			m_meshInsts.push_back( MI );
			
			if( MID.m_flags & LM_MESHINST_SOLID )
			{
				LOG_FUNCTION_ARG( "MI_BODY" );
				
				SGRX_PhyRigidBodyInfo rbinfo;
				rbinfo.shape = m_level->GetPhyWorld()->CreateShapeFromMesh( MI->mesh );
				rbinfo.shape->SetScale( MI->matrix.GetScale() );
				rbinfo.position = MI->matrix.GetTranslation();
				rbinfo.rotation = MI->matrix.GetRotationQuaternion();
				m_levelBodies.push_back( m_level->GetPhyWorld()->CreateRigidBody( rbinfo ) );
			}
		}
	}
	
	return true;
}


ScriptedSequenceSystem::ScriptedSequenceSystem( GameLevel* lev ) :
	IGameLevelSystem( lev, e_system_uid ), m_cmdSkip( "skip_cutscene" ), m_time( 0 )
{
	Game_RegisterAction( &m_cmdSkip );
	Game_BindInputToAction( ACTINPUT_MAKE_KEY( SDLK_SPACE ), &m_cmdSkip );
	
	_InitScriptInterface( "scrSeq" );
}

void ScriptedSequenceSystem::Tick( float deltaTime, float blendFactor )
{
	if( m_func.not_null() )
	{
		SGS_SCOPE;
		sgs_PushVar( C, m_time );
		if( m_func.call( 1, 1 ) )
		{
			if( sgs_GetVar<bool>()( C, -1 ) )
			{
				m_func = sgsVariable();
				m_subtitle = "";
			}
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
	
	if( m_func.not_null() && Game_HasOverlayScreens() == false )
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
	_InitScriptInterface( "music" );
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
		m_music = g_SoundSys->CreateEventInstance( path );
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
	m_bulletDecalSys.Init(
		GR_GetTexture( decal_base_tex ),
		GR_GetTexture( decal_falloff_tex ) );
	m_bulletDecalSys.SetSize( 48 * 1024 * 10 ); // random size
	m_bulletDecalMesh = scene->CreateMeshInstance();
	m_bulletDecalMesh->decal = true;
	m_bulletDecalMesh->sortidx = 202;
	m_bulletDecalMesh->mesh = m_bulletDecalSys.m_mesh;
	m_bulletDecalMesh->textures[0] = GR_GetTexture( "textures/white.png" );
	
	m_bloodDecalSys.m_lightSampler = sampler;
	m_bloodDecalSys.Init(
		GR_GetTexture( "textures/particles/blood.png" ),
		GR_GetTexture( decal_falloff_tex ) );
	m_bloodDecalSys.SetSize( 48 * 1024 * 10 ); // random size
	m_bloodDecalMesh = scene->CreateMeshInstance();
	m_bloodDecalMesh->decal = true;
	m_bloodDecalMesh->sortidx = 201;
	m_bloodDecalMesh->mesh = m_bloodDecalSys.m_mesh;
	m_bloodDecalMesh->textures[0] = GR_GetTexture( "textures/white.png" );
	
	LOG << LOG_DATE << "  Damage system initialized successfully";
	return NULL;
}

void DamageSystem::Free()
{
	m_bulletDecalSys.Free();
	m_bulletDecalMesh = NULL;
	m_bloodDecalSys.Free();
	m_bloodDecalMesh = NULL;
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

void DamageSystem::AddBulletDamage( SGRX_DecalSystem* dmgDecalSysOverride,
	const StringView& type, SGRX_IMesh* targetMesh, int partID,
	const Mat4& worldMatrix, const Vec3& pos, const Vec3& dir, const Vec3& nrm, float scale )
{
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
			// TODO
			
			break;
		}
	}
	
	if( decalID != -1 && targetMesh )
	{
		DecalProjectionInfo projInfo =
		{
			pos, dir, fabsf( Vec3Dot( dir, V3(0,0,1) ) ) > 0.99f ? V3(0,1,0) : V3(0,0,1),
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
}

struct DmgSys_GenBlood : IProcessor
{
	void Process( void* data )
	{
		SGRX_CAST( SGRX_MeshInstance*, MI, data );
		if( MI->mesh == NULL ||
			MI->raycastOverride ||
			MI->skin_matrices.size() ||
			MI->decal )
			return;
		SGRX_CAST( SGRX_MeshInstUserData*, mii, MI->userData );
		if( mii && mii->ovrDecalSysOverride )
		{
			mii->ovrDecalSysOverride->AddDecal( projInfo, MI->mesh, MI->matrix );
			return;
		}
		if( MI->dynamic )
			return;
		DS->m_bloodDecalSys.AddDecal( projInfo, MI->mesh, MI->matrix );
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
		m_bloodDecalMesh->_scene->GatherMeshes( cam, &gb );
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
		
		SceneRaycastCallback_Sorting cb( &m_tmpStore );
		m_level->GetScene()->RaycastAll( p1, p2, &cb, 0xffffffff );
		#ifdef TSGAME
	//	g_DebugLines.DrawLine( p1, p2, m_tmpStore.size() ? COLOR_RGB(255,0,0) : COLOR_RGB(255,255,0) );
		#endif
		
		// sorted list of raycast hits
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
					SGRX_IMesh* mesh = HIT.meshinst->mesh;
					if( HIT.partID >= 0 && HIT.partID < (int) mesh->m_meshParts.size() )
					{
						SGRX_MeshPart& MP = mesh->m_meshParts[ HIT.partID ];
						if( MP.material &&
							MP.material->textures[0] &&
							( MP.material->blendMode == MBM_NONE ||
							MP.material->blendMode == MBM_BASIC ) )
						{
							decalType = MP.material->textures[0]->m_key;
						//	printf("%s\n", StackString<256>(decalType).str);
						}
					}
				}
				
				// apply damage to hit point
				Vec3 hitpoint = TLERP( p1, p2, HIT.factor );
				SGRX_DecalSystem* dmgDecalSys = mii ? mii->dmgDecalSysOverride : NULL;
				bool needDecal = ( HIT.meshinst->dynamic == false || dmgDecalSys ) &&
					HIT.meshinst->skin_matrices.size() == 0;
				m_damageSystem->AddBulletDamage( dmgDecalSys, decalType,
					needDecal ? HIT.meshinst->mesh : NULL,
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
				
				// handling wall penetration
				B.numSolidRefs += entryIfL0 < 0 ? 1 : -1;
				if( B.numSolidRefs == 1 )
				{
					// entry into solid
					B.intersectStart = hitpoint;
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
					B.velocity * speedScale;
				}
				else if( B.numSolidRefs < 0 )
				{
					// fake exit, abort all
					remb = true;
					break;
				}
			}
		}
		
		if( remb )
			m_bullets.erase( i-- );
		else
			B.position = p2;
	}
}

void BulletSystem::Add( const Vec3& pos, const Vec3& vel, float timeleft, float dmg, uint32_t ownerType )
{
	Bullet B = { pos, vel, vel.Normalized(), timeleft, dmg, ownerType };
	m_bullets.push_back( B );
}

void BulletSystem::Clear()
{
	m_bullets.clear();
}



bool AIDBSystem::CanHearSound( Vec3 pos, int i )
{
	AISound& S = m_sounds[ i ];
	return ( pos - S.position ).Length() < S.radius;
}

bool AIDBSystem::LoadChunk( const StringView& type, uint8_t* ptr, size_t size )
{
	if( type != LC_FILE_PFND_NAME )
		return false;
	
	LOG_FUNCTION_ARG( "PFND chunk" );
	
	m_pathfinder.Load( ptr, size );
	return true;
}

void AIDBSystem::AddSound( Vec3 pos, float rad, float timeout, AISoundType type )
{
	AISound S = { pos, rad, timeout, type };
	m_sounds.push_back( S );
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


void CSCoverInfo::InflateSolids( float amt )
{
	for( size_t shid = 0; shid < shapes.size(); ++shid )
	{
		if( shapes[ shid ].inside == false )
			continue;
		
		for( int i = 0; i < shapes[ shid ].numPlanes; ++i )
		{
			Vec4& P = planes[ shapes[ shid ].offset + i ];
			// - shift the plane forward
			P.w += amt;
		}
	}
}

void CSCoverInfo::OffsetShadowSides( float rad )
{
	for( size_t shid = 0; shid < shapes.size(); ++shid )
	{
		if( shapes[ shid ].inside )
			continue;
		
		for( int i = 0; i < shapes[ shid ].numPlanes; ++i )
		{
			Vec4& P = planes[ shapes[ shid ].offset + i ];
			// - offset by side-ness (top/bottom planes shouldn't cut in, sides should)
			P.w -= ( 1 - fabsf( Vec3Dot( P.ToVec3(), V3(0,0,1) ) ) ) * rad;
		}
	}
}

void CSCoverInfo::ClipCoverWithShapes()
{
}

void CSCoverInfo::ClipWithSpheres( Vec4* spheres, int count )
{
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
				coverpts[ pos0 ].nout += nout;
				coverpts[ pos0 ].nup += nup;
			}
			
			if( pos1 == NOT_FOUND )
			{
				pos1 = coverpts.size();
				coverpts.push_back( cp1 );
			}
			else
			{
				coverpts[ pos1 ].nout += nout;
				coverpts[ pos1 ].nup += nup;
			}
			
			coveridcs.push_back( pos0 );
			coveridcs.push_back( pos1 );
		}
	}
	
	for( size_t i = 0; i < coverpts.size(); ++i )
	{
		coverpts[ i ].nout.Normalize();
		coverpts[ i ].nup.Normalize();
	}
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
	
	m_edgeMeshes.push_back( EM );
	m_edgeMeshesByName.set( EM->m_key, EM );
}

void CoverSystem::QueryLines( Vec3 bbmin, Vec3 bbmax, float dist, float height, CSCoverInfo& cinfo )
{
	for( size_t emid = 0; emid < m_edgeMeshes.size(); ++emid )
	{
		EdgeMesh* EM = m_edgeMeshes[ emid ];
		if( EM->enabled == false )
			continue;
		if( EM->InAABB( bbmin, bbmax ) == false )
			continue;
		
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
	}
}

void CoverSystem::QuerySolids( Vec3 bbmin, Vec3 bbmax, Vec3 viewer, float viewdist, CSCoverInfo& cinfo )
{
	for( size_t emid = 0; emid < m_edgeMeshes.size(); ++emid )
	{
		EdgeMesh* EM = m_edgeMeshes[ emid ];
		if( EM->enabled == false )
			continue;
		if( EM->InAABB( bbmin, bbmax ) == false )
			continue;
		if( ( viewer - EM->pos ).Length() > viewdist )
			continue;
		
		// shadow clip
		CSCoverInfo::Shape sh = { cinfo.planes.size(), 0, false };
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
				float pD = Vec3Dot( pN, viewer );
				
				// - push plane
				cinfo.planes.push_back( V4( pN, pD ) );
				cinfo.shapes.last().numPlanes++;
			}
		}
		
		// side edge clip
		sh.offset = cinfo.planes.size();
		sh.inside = true;
		cinfo.shapes.push_back( sh );
		for( size_t i = 0; i < EM->planes.size(); ++i )
		{
			cinfo.planes.push_back( EM->planes[ i ] );
			cinfo.shapes.last().numPlanes++;
		}
	}
}


