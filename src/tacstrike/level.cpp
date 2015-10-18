

#include "level.hpp"


static sgs_ObjInterface g_empty_handle[] = {{ "empty_handle", NULL }};


IGameLevelSystem::~IGameLevelSystem()
{
	_DestroyScriptInterface();
}

void IGameLevelSystem::_DestroyScriptInterface()
{
	if( m_sgsObject )
	{
		m_sgsObject->data = NULL;
		m_sgsObject->iface = g_empty_handle;
		sgs_ObjRelease( C, m_sgsObject );
		C = NULL;
		m_sgsObject = NULL;
	}
}


Entity::Entity( GameLevel* lev ) : m_typeName("<unknown>"), m_level( lev )
{
	// create the scripted self
	{
		SGS_CSCOPE( m_level->GetSGSC() );
		sgs_PushClass( m_level->GetSGSC(), this );
	//	m_level->AddEntry( "infoEmitters", sgsVariable( m_level->GetSGSC(), -1 ) );
		C = m_level->GetSGSC();
		m_sgsObject = sgs_GetObjectStruct( C, -1 );
		sgs_ObjAcquire( C, m_sgsObject );
	}
}

Entity::~Entity()
{
	m_level->UnmapEntityByName( this );
	m_sgsObject->data = NULL;
	m_sgsObject->iface = g_empty_handle;
	sgs_ObjRelease( C, m_sgsObject );
}

sgsVariable Entity::GetScriptedObject()
{
	return Handle( this ).get_variable();
}

int Entity::_sgsent_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return ((Entity*) obj->data)->_sgsGCMark();
}

int Entity::_sgsent_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop )
{
	return ((Entity*) obj->data)->_sgsGetIndex( key, isprop );
}

int Entity::_sgsent_setindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, sgs_Variable* val, int isprop )
{
	return ((Entity*) obj->data)->_sgsSetIndex( key, val, isprop );
}

int Entity::_sgsent_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	return ((Entity*) obj->data)->_sgsDump( depth );
}

sgsHandle< GameLevel > Entity::_sgs_getLevel()
{
	return sgsHandle< GameLevel >( m_level );
}



GameLevel::GameLevel( PhyWorldHandle phyWorld ) :
	m_phyWorld( phyWorld ),
	m_nameIDGen( 0 ),
	m_currentTickTime( 0 ),
	m_currentPhyTime( 0 ),
	m_paused( false ),
	m_levelTime( 0 ),
	m_player( NULL )
{
	LOG_FUNCTION;
	
	// create the scripted self
	{
		SGS_CSCOPE( GetSGSC() );
		sgs_PushClass( GetSGSC(), this );
		m_self = sgsVariable( GetSGSC(), -1 );
		C = GetSGSC();
		m_sgsObject = sgs_GetObjectStruct( C, -1 );
		sgs_ObjAcquire( C, m_sgsObject );
	}
	
	// create marker pos. array
	m_markerPositions = m_scriptCtx.CreateDict();
	m_metadata = m_scriptCtx.CreateDict();
	AddEntry( "positions", m_markerPositions );
	
	m_playerSpawnInfo[0] = V3(0);
	m_levelCameraInfo[0] = V3(0);
	m_playerSpawnInfo[1] = V3(1,0,0);
	m_levelCameraInfo[1] = V3(1,0,0);
	
	m_scene = GR_CreateScene();
	m_scene->clearColor = 0;
	m_scene->camera.position = Vec3::Create( 4, 4, 4 );
	m_scene->camera.direction = Vec3::Create( -1, -1, -1 ).Normalized();
	m_scene->camera.aspect = 1024.0f / 576.0f;
	m_scene->camera.UpdateMatrices();
}

GameLevel::~GameLevel()
{
	ClearLevel();
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->OnLevelDestroy();
	
	m_sgsObject->data = NULL;
	m_sgsObject->iface = g_empty_handle;
	sgs_ObjRelease( C, m_sgsObject );
}


void GameLevel::SetGlobalToSelf()
{
	m_scriptCtx.SetGlobal( "level", m_self );
}

void GameLevel::AddSystem( IGameLevelSystem* sys )
{
	m_systems.push_back( sys );
}

void GameLevel::AddEntity( Entity* E )
{
	E->m_sgsObject->iface = E->_sgsGetIface();
	m_entities.push_back( E );
}

void GameLevel::AddEntry( const StringView& name, sgsVariable var )
{
	m_self.setprop( m_scriptCtx.CreateString( name ), var );
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
	
	m_levelName = levelname;
	
	ByteReader br( &ba );
	LC_Level levelData;
	br << levelData;
	
	for( size_t cid = 0; cid < levelData.chunks.size(); ++cid )
	{
		const LC_Chunk& C = levelData.chunks[ cid ];
		StringView type( C.sys_id, 4 );
		
		LOG_FUNCTION_ARG( type );
		
		for( size_t i = 0; i < m_systems.size(); ++i )
		{
			m_systems[ i ]->LoadChunk( type, C.ptr, C.size );
		}
		
		if( type == LC_FILE_ENTS_NAME )
		{
			LOG_FUNCTION_ARG( "ENTITIES" );
			
			LC_Chunk_Ents ents;
			ByteReader ebr( C.ptr, C.size );
			ebr << ents;
			
			Array< LC_ScriptedEntity >& SEA = ents.entities; // scripted entity array
			for( size_t i = 0; i < SEA.size(); ++i )
			{
				sgsVariable data = m_scriptCtx.Unserialize( SEA[ i ].serialized_params );
				if( data.not_null() == false )
				{
					LOG << "BAD PARAMS FOR ENTITY " << SEA[ i ].type;
					continue;
				}
				CreateEntity( SEA[ i ].type, data );
			}
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
	if( type == "camera_start" )
	{
		m_levelCameraInfo[0] = data.getprop("position").get<Vec3>();
		m_levelCameraInfo[1] = data.getprop("viewdir").get<Vec3>().Normalized();
		return;
	}
	
	///////////////////////////
	if( type == "marker" )
	{
		m_markerPositions.setprop( data.getprop("name"), data.getprop("position") );
		return;
	}
	
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
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->Clear();
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
		
		int ITERS = 10;
		for( int i = 0; i < ITERS; ++i )
			m_phyWorld->Step( deltaTime / ITERS );
		
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->FixedTick( deltaTime );
	}
}

void GameLevel::Tick( float deltaTime, float blendFactor )
{
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
	}
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->Tick( deltaTime, blendFactor );
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
	return;
	
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

Entity::Handle GameLevel::sgsFindEntity( StringView name )
{
	return Entity::Handle( FindEntityByName( name ) );
}

void GameLevel::CallEntityByName( StringView name, StringView action )
{
	Entity* e = m_entNameMap.getcopy( name );
	if( e )
		e->OnEvent( action );
}

void GameLevel::sgsSetCameraPosDir( Vec3 pos, Vec3 dir )
{
	m_scene->camera.position = pos;
	m_scene->camera.direction = dir;
	m_scene->camera.UpdateMatrices();
}


// ---

int GameLevel::_getindex(
	SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop )
{
	SGRX_CAST( GameLevel*, LVL, obj->data );
	SGSRESULT res = sgs_PushIndexPP( C, &LVL->m_metadata.var, key, isprop );
	if( res != SGS_ENOTFND )
		return res; // found or serious error
	return _sgs_getindex( C, obj, key, isprop );
}

int GameLevel::_setindex(
	SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, sgs_Variable* val, int isprop )
{
	SGRX_CAST( GameLevel*, LVL, obj->data );
	if( _sgs_setindex( C, obj, key, val, isprop ) != SGS_SUCCESS )
		sgs_SetIndexPPP( C, &LVL->m_metadata.var, key, val, isprop );
	return SGS_SUCCESS;
}


void GameLevel::LightMesh( MeshInstHandle mih, Vec3 off )
{
	SGRX_LightSampler::LightMesh( mih, off );
}

