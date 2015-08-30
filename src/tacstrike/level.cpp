

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#define USE_SERIALIZATION
#include "level.hpp"



Entity::Entity() : m_typeName("<unknown>")
{
}

Entity::~Entity()
{
	m_level->UnmapEntityByName( this );
}



#if 0
static int EndLevel( SGS_CTX )
{
	SGSFN( "EndLevel" );
	g_GameLevel->m_endFactor = 0;
	return 0;
}
static int PlayerGetPosition( SGS_CTX )
{
	SGSFN( "PlayerGetPosition" );
	if( g_GameLevel->m_player )
	{
		sgs_PushVar<Vec3>( C, g_GameLevel->m_player->GetPosition() );
		return 1;
	}
	return 0;
}
static int PlayerHasItem( SGS_CTX )
{
	SGSFN( "PlayerHasItem" );
	if( g_GameLevel->m_player )
	{
		StringView name = sgs_GetVar<StringView>()( C, 0 );
		sgs_PushVar<bool>( C, g_GameLevel->m_player->HasItem( name, sgs_GetVar<int>()( C, 1 ) ) );
		return 1;
	}
	return 0;
}
static int EntitySetProperties( SGS_CTX )
{
	SGSFN( "EntitySetProperties" );
	StringView name = sgs_GetVar<StringView>()( C, 0 );
	Entity* E = g_GameLevel->FindEntityByName( name );
	if( !E )
		return sgs_Msg( C, SGS_WARNING, "failed to find entity: %.*s", (int) name.size(), name.data() );
	sgs_StkIdx ssz = sgs_StackSize( C );
	for( sgs_StkIdx i = 1; i + 1 < ssz; i += 2 )
	{
		StringView key = sgs_GetVar<StringView>()( C, i + 0 );
		sgsVariable value = sgs_GetVar<sgsVariable>()( C, i + 1 );
		E->SetProperty( key, value );
	}
	return 0;
}
static int EntityGetProperties( SGS_CTX )
{
	SGSFN( "EntityGetProperties" );
	StringView name = sgs_GetVar<StringView>()( C, 0 );
	Entity* E = g_GameLevel->FindEntityByName( name );
	if( !E )
		return sgs_Msg( C, SGS_WARNING, "failed to find entity: %.*s", (int) name.size(), name.data() );
	sgs_StkIdx ssz = sgs_StackSize( C );
	for( sgs_StkIdx i = 1; i < ssz; ++i )
	{
		StringView key = sgs_GetVar<StringView>()( C, i );
		g_GameLevel->m_scriptCtx.Push( E->GetProperty( key ) );
	}
	return ssz - 1;
}
static int GetNamedPosition( SGS_CTX )
{
	SGSFN( "GetNamedPosition" );
	StringView name = sgs_GetVar<StringView>()( C, 0 );
	Vec3* pos = g_GameLevel->m_markerMap.getptr( name );
	if( pos == NULL )
		return sgs_Msg( C, SGS_WARNING, "failed to find named position: %.*s", (int) name.size(), name.data() );
	g_GameLevel->m_scriptCtx.Push( *pos );
	return 1;
}

static int SetCutsceneFunc( SGS_CTX )
{
	SGSFN( "SetCutsceneFunc" );
	g_GameLevel->m_cutsceneFunc = sgsVariable( C, 0 );
	g_GameLevel->m_cutsceneTime = sgs_GetVar<float>()( C, 1 );
	return 0;
}
static int SetCutsceneSubtitle( SGS_CTX )
{
	SGSFN( "SetCutsceneSubtitle" );
	g_GameLevel->m_cutsceneSubtitle = sgs_GetVar<StringView>()( C, 0 );
	return 0;
}
static int SetCameraPosDir( SGS_CTX )
{
	SGSFN( "SetCameraPosDir" );
	g_GameLevel->m_scene->camera.position = sgs_GetVar<Vec3>()( C, 0 );
	g_GameLevel->m_scene->camera.direction = sgs_GetVar<Vec3>()( C, 1 );
	g_GameLevel->m_scene->camera.UpdateMatrices();
	return 0;
}

extern SoundSystemHandle g_SoundSys;
static int SetMusic( SGS_CTX )
{
	SGSFN( "SetMusic" );
	
	if( g_GameLevel->m_music )
		g_GameLevel->m_music->Stop();
	g_GameLevel->m_music = NULL;
	
	StringView name = sgs_GetVar<StringView>()( C, 0 );
	if( name )
	{
		g_GameLevel->m_music = g_SoundSys->CreateEventInstance( name );
		g_GameLevel->m_music->Start();
	}
	
	return 0;
}
static int SetMusicVar( SGS_CTX )
{
	SGSFN( "SetMusicVar" );
	StringView name = sgs_GetVar<StringView>()( C, 0 );
	if( g_GameLevel->m_music && name )
	{
		g_GameLevel->m_music->SetParameter( name, sgs_GetVar<float>()( C, 1 ) );
	}
	return 0;
}

static int IES_UpdateEmitter( SGS_CTX )
{
	SGSFN( "IES_UpdateEmitter" );
	Entity* E = (Entity*) sgs_GetVar<void*>()( C, 0 );
	if( E == NULL )
		return sgs_Msg( C, SGS_WARNING, "given pointer is not an entity" );
	InfoEmissionSystem::Data data =
	{
		sgs_GetVar<Vec3>()( C, 1 ),
		sgs_GetVar<float>()( C, 2 ),
		sgs_GetVar<int>()( C, 3 ),
	};
	g_GameLevel->m_infoEmitters.UpdateEmitter( E, data );
	return 0;
}
static int IES_RemoveEmitter( SGS_CTX )
{
	SGSFN( "IES_RemoveEmitter" );
	Entity* E = (Entity*) sgs_GetVar<void*>()( C, 0 );
	if( E == NULL )
		return sgs_Msg( C, SGS_WARNING, "given pointer is not an entity" );
	g_GameLevel->m_infoEmitters.RemoveEmitter( E );
	return 0;
}

static int SendMessage( SGS_CTX )
{
	SGSFN( "SendMessage" );
	int ssz = sgs_StackSize( C );
	g_GameLevel->m_messageSystem.AddMessage( (MSMessage::Type) sgs_GetVar<int>()( C, 0 ), sgs_GetVar<StringView>()( C, 1 ), ssz < 3 ? 3.0f : sgs_GetVar<float>()( C, 2 ) );
	return 0;
}

static int ObjectiveAdd( SGS_CTX )
{
	SGSFN( "ObjectiveAdd" );
	Vec3 loc = sgs_GetVar<Vec3>()( C, 4 );
	sgs_PushVar<int>( C, g_GameLevel->m_objectiveSystem.AddObjective(
		sgs_GetVar<StringView>()( C, 0 ),
		(OSObjective::State) sgs_GetVar<int>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		sgs_GetVar<bool>()( C, 3 ),
		sgs_ItemType( C, 4 ) == SGS_VT_NULL ? NULL : &loc
	) );
	return 1;
}
static int ObjectiveGetTitle( SGS_CTX )
{
	SGSFN( "ObjectiveGetTitle" );
	int which = sgs_GetVar<int>()( C, 0 );
	if( which < 0 || which >= (int) g_GameLevel->m_objectiveSystem.m_objectives.size() )
		return sgs_Msg( C, SGS_WARNING, "index out of bounds: %d", which );
	sgs_PushVar( C, g_GameLevel->m_objectiveSystem.m_objectives[ which ].title );
	return 1;
}
static int ObjectiveSetTitle( SGS_CTX )
{
	SGSFN( "ObjectiveSetTitle" );
	int which = sgs_GetVar<int>()( C, 0 );
	if( which < 0 || which >= (int) g_GameLevel->m_objectiveSystem.m_objectives.size() )
		return sgs_Msg( C, SGS_WARNING, "index out of bounds: %d", which );
	g_GameLevel->m_objectiveSystem.m_objectives[ which ].title = sgs_GetVar<StringView>()( C, 1 );
	return 0;
}
static int ObjectiveGetState( SGS_CTX )
{
	SGSFN( "ObjectiveGetState" );
	int which = sgs_GetVar<int>()( C, 0 );
	if( which < 0 || which >= (int) g_GameLevel->m_objectiveSystem.m_objectives.size() )
		return sgs_Msg( C, SGS_WARNING, "index out of bounds: %d", which );
	sgs_PushVar<int>( C, g_GameLevel->m_objectiveSystem.m_objectives[ which ].state );
	return 1;
}
static int ObjectiveSetState( SGS_CTX )
{
	SGSFN( "ObjectiveSetState" );
	int which = sgs_GetVar<int>()( C, 0 );
	if( which < 0 || which >= (int) g_GameLevel->m_objectiveSystem.m_objectives.size() )
		return sgs_Msg( C, SGS_WARNING, "index out of bounds: %d", which );
	int state = sgs_GetVar<int>()( C, 1 );
	g_GameLevel->m_objectiveSystem.m_objectives[ which ].state = (OSObjective::State) state;
	return 0;
}

static int FS_UpdateFlare( SGS_CTX )
{
	SGSFN( "FS_UpdateFlare" );
	void* P = sgs_GetVar<void*>()( C, 0 );
	if( P == NULL )
		return sgs_Msg( C, SGS_WARNING, "cannot use NULL pointer for association" );
	FSFlare data =
	{
		sgs_GetVar<Vec3>()( C, 1 ),
		sgs_GetVar<Vec3>()( C, 2 ),
		sgs_GetVar<float>()( C, 3 ),
		sgs_GetVar<bool>()( C, 4 ),
	};
	g_GameLevel->m_flareSystem.UpdateFlare( P, data );
	return 0;
}
static int FS_RemoveFlare( SGS_CTX )
{
	SGSFN( "FS_RemoveFlare" );
	void* P = sgs_GetVar<void*>()( C, 0 );
	if( P == NULL )
		return sgs_Msg( C, SGS_WARNING, "cannot use NULL pointer for association" );
	g_GameLevel->m_flareSystem.RemoveFlare( P );
	return 0;
}
#endif

static sgs_RegFuncConst g_gameapi_rfc[] =
{
#if 0
	{ "EndLevel", EndLevel },
	{ "SetLevel", SetLevel },
	{ "CallEntity", CallEntity },
	{ "PlayerGetPosition", PlayerGetPosition },
	{ "PlayerHasItem", PlayerHasItem },
	{ "EntitySetProperties", EntitySetProperties },
	{ "EntityGetProperties", EntityGetProperties },
	{ "GetNamedPosition", GetNamedPosition },
	{ "SetCutsceneFunc", SetCutsceneFunc },
	{ "SetCutsceneSubtitle", SetCutsceneSubtitle },
	{ "SetCameraPosDir", SetCameraPosDir },
	{ "SetMusic", SetMusic },
	{ "SetMusicVar", SetMusicVar },
	// Info emission system
	{ "IES_UpdateEmitter", IES_UpdateEmitter },
	{ "IES_RemoveEmitter", IES_RemoveEmitter },
	// Messaging system
	{ "SendMessage", SendMessage },
	// Objective system
	{ "ObjectiveAdd", ObjectiveAdd },
	{ "ObjectiveGetTitle", ObjectiveGetTitle },
	{ "ObjectiveSetTitle", ObjectiveSetTitle },
	{ "ObjectiveGetState", ObjectiveGetState },
	{ "ObjectiveSetState", ObjectiveSetState },
	// Flare system
	{ "FS_UpdateFlare", FS_UpdateFlare },
	{ "FS_RemoveFlare", FS_RemoveFlare },
#endif
	SGS_RC_END(),
};

static sgs_RegIntConst g_gameapi_ric[] =
{
#if 0
	{ "IEST_InteractiveItem", IEST_InteractiveItem },
	{ "IEST_HeatSource", IEST_HeatSource },
	{ "IEST_Player", IEST_Player },
	{ "IEST_MapItem", IEST_MapItem },
	{ "IEST_AIAlert", IEST_AIAlert },
	// Messaging system
	{ "MT_Continued", MSMessage::Continued },
	{ "MT_Info", MSMessage::Info },
	{ "MT_Warning", MSMessage::Warning },
	// Objective system
	{ "OS_Hidden", OSObjective::Hidden },
	{ "OS_Open", OSObjective::Open },
	{ "OS_Done", OSObjective::Done },
	{ "OS_Failed", OSObjective::Failed },
	{ "OS_Cancelled", OSObjective::Cancelled },
#endif
	{ NULL, 0 },
};

static int InitGameAPI( SGS_CTX )
{
	sgs_RegFuncConsts( C, g_gameapi_rfc, -1 );
	sgs_RegIntConsts( C, g_gameapi_ric, -1 );
	return 0;
}



Command SKIP_CUTSCENE( "skip_cutscene" );

GameLevel::GameLevel() :
	m_nameIDGen( 0 ),
	m_currentTickTime( 0 ),
	m_currentPhyTime( 0 ),
//	m_bulletSystem( &m_damageSystem ),
	m_paused( false ),
	m_endFactor( -1 ),
	m_levelTime( 0 ),
	m_player( NULL )
{
	LOG_FUNCTION;
	
	Game_RegisterAction( &SKIP_CUTSCENE );
	Game_BindInputToAction( ACTINPUT_MAKE_KEY( SDLK_SPACE ), &SKIP_CUTSCENE );
	
	m_lightTree = &m_ltSamples;
	
	m_playerSpawnInfo[0] = V3(0);
	m_levelCameraInfo[0] = V3(0);
	m_playerSpawnInfo[1] = V3(1,0,0);
	m_levelCameraInfo[1] = V3(1,0,0);
	
	m_scene = GR_CreateScene();
	m_scene->camera.position = Vec3::Create( 4, 4, 4 );
	m_scene->camera.direction = Vec3::Create( -1, -1, -1 ).Normalized();
	m_scene->camera.aspect = 1024.0f / 576.0f;
	m_scene->camera.UpdateMatrices();
	
//	const char* err = m_damageSystem.Init( m_scene, this );
//	if( err )
//	{
//		LOG_ERROR << LOG_DATE << "  Failed to init DMGSYS: " << err;
//	}
	
	InitGameAPI( m_scriptCtx.C );
	m_scriptCtx.Include( "data/enemy" );
}

GameLevel::~GameLevel()
{
	ClearLevel();
//	m_damageSystem.Free();
}





struct LoadingScreen
{
	LoadingScreen() : m_running(true), m_alpha(0), m_alphaTgt(1)
	{
		m_thread.Start( _Proc, this );
		sgrx_sleep( 500 );
	}
	~LoadingScreen()
	{
		m_alphaTgt = 0;
		sgrx_sleep( 500 );
		m_running = false;
	}
	
	void Run()
	{
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
		float t = 0;
		double prevt = sgrx_hqtime();
		while( m_running )
		{
			sgrx_sleep( 10 );
			double newt = sgrx_hqtime();
			
			float minw = TMIN( GR_GetWidth(), GR_GetHeight() );
			GR2D_SetFont( "fancy", minw / 20 );
			
			GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
			br.Reset().Col( 0, 1 ).Quad( 0, 0, 999999, 999999 );
			
		//	br.Reset().Col( 1, 1 ).SetPrimitiveType(PT_Lines)
		//		.Pos( V2(0,0) ).Pos( 0+cosf(t)*50, 0+sinf(t)*50);
			
			br.Reset().Col( 1.0f, 0.5f * m_alpha );
			int textlen = int( t * 37 ) % 100;
			StringView text = "LOADING...";
			text = text.part( 0, textlen );
			GR2D_DrawTextLine( minw / 20, GR_GetHeight() - minw / 20,
				text, HALIGN_LEFT, VALIGN_BOTTOM );
			
			br.Flush();
			SGRX_Swap();
			
			float delta = ( newt - prevt );
			t += delta;
			prevt = newt;
			
			float alpha_dt = m_alphaTgt - m_alpha;
			m_alpha += sign( alpha_dt ) * TMIN( fabsf( alpha_dt ), delta * 2 );
		}
	}
	
	static void _Proc( void* data )
	{
		((LoadingScreen*)data)->Run();
	}
	
	SGRX_Thread m_thread;
	volatile bool m_running;
	volatile float m_alpha;
	volatile float m_alphaTgt;
};



bool GameLevel::Load( const StringView& levelname )
{
	LOG_FUNCTION_ARG( levelname );
	
	LoadingScreen LS;
	
	ByteArray ba;
	
	// load level file & scripts, clear previous data
	{
		LOG_FUNCTION_ARG( "CORE/CLEAR/SCRIPT" );
		
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "levels/%.*s/cache", TMIN( (int) levelname.size(), 200 ), levelname.data() );
		if( !FS_LoadBinaryFile( bfr, ba ) )
			return false;
		
		ClearLevel();
		
		sgrx_snprintf( bfr, sizeof(bfr), "levels/%.*s", TMIN( (int) levelname.size(), 200 ), levelname.data() );
		m_scriptCtx.Include( bfr );
	}
	
	ByteReader br( &ba );
	
	Array< LC_ScriptedEntity > screntdefs;
	Array< LC_MeshInst > mesh_inst_defs;
	
	br.marker( "COMPILED" );
	SerializeVersionHelper<ByteReader> svh( br, LC_FILE_VERSION );
	
	// load basic data
	{
		LOG_FUNCTION_ARG( "UNSER_CORE" );
		
		svh.marker( "SCRENTS" );
		svh << screntdefs;
		
		svh.marker( "INST" );
		svh << mesh_inst_defs;
		
		svh.marker( "LINES" );
		svh << m_lines;
		
		svh.marker( "LIGHTS" );
		svh << m_lights;
	}
	
	// LOAD FLARES
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		LC_Light& L = m_lights[ i ];
		if( L.type != LM_LIGHT_POINT && L.type != LM_LIGHT_SPOT )
			continue;
		FSFlare FD = { L.pos + L.flareoffset, L.color, L.flaresize, true };
		m_flareSystem.UpdateFlare( &m_lights[ i ], FD );
	}
	
	// LOAD LIGHT SAMPLES
	{
		LOG_FUNCTION_ARG( "SAMPLES" );
		
		svh.marker( "SAMPLES" );
		svh << m_ltSamples;
	}
	
	// create static geometry
	{
		LOG_FUNCTION_ARG( "PHY_MESH" );
		
		svh.marker( "PHYMESH" );
		SGRX_PhyRigidBodyInfo rbinfo;
		LC_PhysicsMesh phy_mesh;
		svh << phy_mesh;
		
		// TODO: temporarily ignore material data
		Array< uint32_t > fixedidcs;
		for( size_t i = 0; i < phy_mesh.indices.size(); i += 4 )
		{
			fixedidcs.append( &phy_mesh.indices[ i ], 3 );
		}
		rbinfo.shape = m_phyWorld->CreateTriMeshShape(
			phy_mesh.positions.data(), phy_mesh.positions.size(),
			fixedidcs.data(), fixedidcs.size(), true );
		m_levelBodies.push_back( m_phyWorld->CreateRigidBody( rbinfo ) );
	}
	
	// initialize AI DB
	{
		LOG_FUNCTION_ARG( "AI_DB" );
		
		ByteArray navmesh;
		
		svh.marker( "NAVMESH" );
		svh << navmesh;
		
		m_aidbSystem.Load( navmesh );
	}
	
	// load mesh instances
	{
		LOG_FUNCTION_ARG( "MESH_INSTS" );
		
		for( size_t i = 0; i < mesh_inst_defs.size(); ++i )
		{
			LC_MeshInst& MID = mesh_inst_defs[ i ];
			
			LOG_FUNCTION_ARG( MID.m_meshname );
			
			char subbfr[ 512 ];
			MeshInstHandle MI = m_scene->CreateMeshInstance();
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
				LightMesh( MI );
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
				rbinfo.shape = m_phyWorld->CreateShapeFromMesh( MI->mesh );
				rbinfo.shape->SetScale( MI->matrix.GetScale() );
				rbinfo.position = MI->matrix.GetTranslation();
				rbinfo.rotation = MI->matrix.GetRotationQuaternion();
				m_levelBodies.push_back( m_phyWorld->CreateRigidBody( rbinfo ) );
			}
		}
	}
	
	// create entities
	{
		LOG_FUNCTION_ARG( "ENTITIES" );
		
		for( size_t i = 0; i < screntdefs.size(); ++i )
		{
			sgsVariable data = m_scriptCtx.Unserialize( screntdefs[ i ].serialized_params );
			if( data.not_null() == false )
			{
				LOG << "BAD PARAMS FOR ENTITY " << screntdefs[ i ].type;
				continue;
			}
			CreateEntity( screntdefs[ i ].type, data );
		}
	}
	
	
	return true;
}

void GameLevel::CreateEntity( const StringView& type, sgsVariable data )
{
	for( size_t i = 0; i < m_systems.size(); ++i )
	{
		if( m_systems[ i ]->AddEntity( type, data ) )
			return;
	}
	
	///////////////////////////
	if( type == "player_start" )
	{
		m_playerSpawnInfo[0] = data.getprop("position").get<Vec3>();
		m_playerSpawnInfo[1] = data.getprop("viewdir").get<Vec3>().Normalized();
		return;
	}
	
	///////////////////////////
	if( type == "solidbox" )
	{
		Vec3 scale = data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>();
		SGRX_PhyRigidBodyInfo rbinfo;
		rbinfo.group = 2;
		rbinfo.shape = m_phyWorld->CreateAABBShape( -scale, scale );
		rbinfo.mass = 0;
		rbinfo.inertia = V3(0);
		rbinfo.position = data.getprop("position").get<Vec3>();
		rbinfo.rotation = Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion();
		m_levelBodies.push_back( m_phyWorld->CreateRigidBody( rbinfo ) );
		return;
	}
	
#if defined(LD33GAME) || defined(TSGAME)
	///////////////////////////
	if( type == "enemy_start" )
	{
		TSEnemy* E = new TSEnemy
		(
			data.getprop("name").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			data.getprop("viewdir").get<Vec3>(),
			data
		);
		m_entities.push_back( E );
		return;
	}
#endif
	
	///////////////////////////
	if( type == "camera_start" )
	{
		m_levelCameraInfo[0] = data.getprop("position").get<Vec3>();
		m_levelCameraInfo[1] = data.getprop("viewdir").get<Vec3>().Normalized();
		return;
	}
	
	///////////////////////////
	if( type == "marker" )
	{
		m_markerMap.set( data.getprop("name").get<StringView>(), data.getprop("position").get<Vec3>() );
		return;
	}
	
	///////////////////////////
#ifdef TSGAME
	if( type == "camera" )
	{
		TSCamera* CAM = new TSCamera
		(
			data.getprop("name").get<StringView>(),
			data.getprop("char").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("dir0").get<Vec3>(),
			data.getprop("dir1").get<Vec3>()
		);
		m_entities.push_back( CAM );
		return;
	}
#endif
	
	///////////////////////////
	if( type == "mesharray" ||
		type == "m3sh" )
	{
		return;
	}
	
	LOG << "ENTITY TYPE NOT FOUND: " << type;
}

StackShortName GameLevel::GenerateName()
{
	m_nameIDGen++;
	StackShortName tmp("");
	sgrx_snprintf( tmp.str, 15, "_name%u", (unsigned int) m_nameIDGen );
	return tmp;
}

void GameLevel::StartLevel()
{
	m_currentTickTime = 0;
	m_currentPhyTime = 0;
	m_endFactor = -1;
	m_cutsceneFunc = sgsVariable();
	m_cutsceneSubtitle = "";
	if( m_music )
		m_music->Stop();
	m_music = NULL;
	m_cameraInfoCached = false;
	m_levelTime = 0;
	if( !m_player )
	{
		sgsVariable data = m_scriptCtx.CreateDict();
		data.setprop( "position", m_scriptCtx.CreateVec3( m_playerSpawnInfo[0] ) );
		data.setprop( "viewdir", m_scriptCtx.CreateVec3( m_playerSpawnInfo[1] ) );
		CreateEntity( "player", data );
		ASSERT( m_player && "player must be created by one of the systems" );
	}
	m_scriptCtx.GlobalCall( "onLevelStart" );
}

void GameLevel::ClearLevel()
{
	m_currentTickTime = 0;
	m_currentPhyTime = 0;
	
	for( size_t i = 0; i < m_entities.size(); ++i )
		delete m_entities[ i ];
	m_entities.clear();
	m_player = NULL;
	
	m_cutsceneFunc = sgsVariable();
	m_cutsceneSubtitle = "";
	if( m_music )
		m_music->Stop( true );
	m_music = NULL;
	m_endFactor = -1;
	m_cameraInfoCached = false;
	
	m_ltSamples.SetSamples( NULL, 0 );
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->Clear();
//	m_infoEmitters.Clear();
//	m_messageSystem.Clear();
//	m_objectiveSystem.Clear();
//	m_damageSystem.Clear();
//	m_bulletSystem.Clear();
//	m_flareSystem.Clear();
//	m_coverSystem.Clear();
	
	m_lights.clear();
	m_meshInsts.clear();
	m_levelBodies.clear();
}

void GameLevel::ProcessEvents()
{
	if( m_nextLevel.size() != 0 )
	{
		Load( m_nextLevel );
		StartLevel();
		m_nextLevel = "";
	}
}

void GameLevel::FixedTick( float deltaTime )
{
	if( !m_paused )
	{
		m_currentTickTime += deltaTime;
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->FixedTick( deltaTime );
	}
}

void GameLevel::Tick( float deltaTime, float blendFactor )
{
	if( m_endFactor >= 0 )
	{
		m_endFactor = clamp( m_endFactor + deltaTime / 3, 0, 1 );
	}
	
	m_levelTime += deltaTime;
	
	if( !m_player )
	{
		m_scene->camera.position = m_levelCameraInfo[0];
		m_scene->camera.direction = m_levelCameraInfo[1];
		m_scene->camera.updir = V3(0,0,1);
		m_scene->camera.aspect = GR_GetWidth() / (float) GR_GetHeight();
		m_scene->camera.UpdateMatrices();
	}
	
	if( !m_paused )
	{
		m_currentPhyTime += deltaTime;
		
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->Tick( deltaTime, blendFactor );
	//	m_damageSystem.Tick( deltaTime );
	//	m_bulletSystem.Tick( m_scene, deltaTime );
	//	m_aidbSystem.Tick( deltaTime );
	}
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->Tick( deltaTime, blendFactor );
	
//	m_messageSystem.Tick( deltaTime );
//	m_objectiveSystem.Tick( deltaTime );
	
	if( m_cutsceneFunc.not_null() )
	{
		SGS_CSCOPE( m_scriptCtx.C );
		m_scriptCtx.Push( m_cutsceneTime );
		if( m_cutsceneFunc.call( 1, 1 ) )
		{
			if( sgs_GetVar<bool>()( m_scriptCtx.C, -1 ) )
			{
				m_cutsceneFunc = sgsVariable();
				m_cutsceneSubtitle = "";
			}
		}
		if( SKIP_CUTSCENE.value )
			m_cutsceneTime += deltaTime * 20;
		else
			m_cutsceneTime += deltaTime;
	}
	
	if( m_endFactor > 0 )
	{
		if( !m_cameraInfoCached )
		{
			m_cachedCameraInfo[0] = m_scene->camera.position;
			m_cachedCameraInfo[1] = m_scene->camera.direction;
			m_cameraInfoCached = true;
		}
		Vec3 p0 = m_cachedCameraInfo[0];
		Vec3 p1 = m_levelCameraInfo[0];
		Vec3 d0 = m_cachedCameraInfo[1];
		Vec3 d1 = m_levelCameraInfo[1];
		float q = smoothstep( m_endFactor );
		m_scene->camera.position = TLERP( p0, p1, q );
		m_scene->camera.direction = TLERP( d0, d1, q ).Normalized();
		m_scene->camera.UpdateMatrices();
	}
}

void GameLevel::Draw2D()
{
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->DrawUI();
	
	//
	// UI
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	int size_x = GR_GetWidth();
	int size_y = GR_GetHeight();
	int sqr = TMIN( size_x, size_y );
	
	if( m_cutsceneFunc.not_null() && Game_HasOverlayScreens() == false )
	{
		GR2D_SetFont( "core", sqr / 40 );
		GR2D_SetColor( 1, 1 );
		GR2D_DrawTextLine( sqr/20, sqr/20, "Press <Space> to speed up", HALIGN_LEFT, VALIGN_TOP );
	}
	
	if( m_cutsceneSubtitle.size() )
	{
		GR2D_SetFont( "core", sqr / 20 );
		GR2D_SetColor( 0, 1 );
		GR2D_DrawTextLine( size_x / 2 + 1, size_y * 3 / 4 + 1, m_cutsceneSubtitle, HALIGN_CENTER, VALIGN_CENTER );
		GR2D_SetColor( 1, 1 );
		GR2D_DrawTextLine( size_x / 2, size_y * 3 / 4, m_cutsceneSubtitle, HALIGN_CENTER, VALIGN_CENTER );
	}
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->DebugDrawUI();
	
	for( size_t i = 0; i < m_entities.size(); ++i )
		m_entities[ i ]->DebugDrawUI();
}

void GameLevel::DebugDraw()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	UNUSED( br );
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->DebugDrawWorld();
	
	for( size_t i = 0; i < m_entities.size(); ++i )
		m_entities[ i ]->DebugDrawWorld();
	
#if DRAW_SAMPLES
	br.Reset().Col( 0, 1, 0 );
	for( size_t i = 0; i < m_ltSamples.m_pos.size(); ++i )
	{
#if 1
		br.Col( m_ltSamples.m_colors[ i ].color[4] );
	//	SGRX_LightTree::Colors COL;
	//	m_ltSamples.GetColors( m_ltSamples.m_pos[ i ], &COL );
	//	br.Col( COL.color[4] );
#endif
		br.Tick( m_ltSamples.m_pos[ i ], 0.1f );
	}
#endif
	
#if DRAW_PATHFINDER
	m_aidbSystem.m_pathfinder.DebugDraw();
	
#if TEST_PATHFINDER
	br.Reset().Col( 1, 0, 0 );
	
	Array< Vec3 > path;
	if( m_aidbSystem.m_pathfinder.FindPath( V3(-2.4f,10,1), V3(2.6f,-1.4f,1), path ) && path.size() >= 2 )
	{
		br.SetPrimitiveType( PT_LineStrip );
		for( size_t i = 0; i < path.size(); ++i )
			br.Pos( path[ i ] );
	}
#endif
#endif
}

void GameLevel::PostDraw()
{
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->PostDraw();
}

void GameLevel::Draw()
{
	SGRX_RenderScene rsinfo( V4( m_levelTime ), m_scene );
	rsinfo.debugdraw = this;
	rsinfo.postdraw = this;
	GR_RenderScene( rsinfo );
}

void GameLevel::SetNextLevel( StringView name )
{
	m_nextLevel = name;
}

void GameLevel::MapEntityByName( Entity* e )
{
	m_entNameMap[ e->m_name ] = e;
}

void GameLevel::UnmapEntityByName( Entity* e )
{
	m_entNameMap.unset( e->m_name );
}

Entity* GameLevel::FindEntityByName( const StringView& name )
{
	return m_entNameMap.getcopy( name );
}

void GameLevel::CallEntityByName( StringView name, StringView action )
{
	Entity* e = m_entNameMap.getcopy( name );
	if( e )
		e->OnEvent( action );
}


void GameLevel::LightMesh( MeshInstHandle mih, Vec3 off )
{
	SGRX_LightTree::Colors COL;
	m_ltSamples.GetColors( mih->matrix.TransformPos( off ), &COL );
	mih->constants[10] = V4( COL.color[0] * 0.5f, 1 );
	mih->constants[11] = V4( COL.color[1] * 0.5f, 1 );
	mih->constants[12] = V4( COL.color[2] * 0.5f, 1 );
	mih->constants[13] = V4( COL.color[3] * 0.5f, 1 );
	mih->constants[14] = V4( COL.color[4] * 0.5f, 1 );
	mih->constants[15] = V4( COL.color[5] * 0.5f, 1 );
}

