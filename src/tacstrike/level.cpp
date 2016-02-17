

#include "level.hpp"


sgs_ObjInterface g_sgsobj_empty_handle[1] = {{ "empty_handle", NULL }};


CVarBool gcv_cl_gui( "cl_gui", true );
CVarBool gcv_cl_debug( "cl_debug", false );
CVarBool gcv_g_paused( "g_paused", false );

void RegisterCommonGameCVars()
{
	REGCOBJ( gcv_cl_gui );
	REGCOBJ( gcv_cl_debug );
	REGCOBJ( gcv_g_paused );
}


LevelScrObj::LevelScrObj( GameLevel* lev ) :
	m_level( lev )
{
	m_sgsObject = NULL;
	C = NULL;
}

LevelScrObj::~LevelScrObj()
{
	DestroyScriptInterface();
}

void LevelScrObj::DestroyScriptInterface()
{
	if( m_sgsObject )
	{
		m_sgsObject->data = NULL;
		m_sgsObject->iface = g_sgsobj_empty_handle;
		sgs_ObjRelease( C, m_sgsObject );
		C = NULL;
		m_sgsObject = NULL;
	}
}

void LevelScrObj::AddSelfToLevel( StringView name )
{
	m_level->AddEntry( name, GetScriptedObject() );
}

sgsHandle< GameLevel > LevelScrObj::_sgs_getLevel()
{
	return sgsHandle< GameLevel >( m_level );
}


Entity::Entity( GameLevel* lev ) :
	LevelScrObj( lev ),
	m_infoMask( 0 ),
	m_infoTarget( V3(0) ),
	m_typeName("<unknown>")
{
}

Entity::~Entity()
{
	m_level->UnmapEntityByName( this );
	m_level->m_infoEmitSet.Unregister( this );
}

void Entity::FixedTick( float deltaTime )
{
	sgsVariable fn_fixedupdate = GetScriptedObject().getprop( "FixedUpdate" );
	if( fn_fixedupdate.not_null() )
	{
		sgs_PushReal( C, deltaTime );
		GetScriptedObject().thiscall( C, fn_fixedupdate, 1 );
	}
}

void Entity::Tick( float deltaTime, float blendFactor )
{
	sgsVariable fn_update = GetScriptedObject().getprop( "Update" );
	if( fn_update.not_null() )
	{
		sgs_PushReal( C, deltaTime );
		sgs_PushReal( C, blendFactor );
		GetScriptedObject().thiscall( C, fn_update, 2 );
	}
}

void Entity::OnTransformUpdate()
{
	GetScriptedObject().thiscall( C, "OnTransformUpdate" );
}

void Entity::SetInfoMask( uint32_t mask )
{
	if( mask && !m_infoMask )
		m_level->m_infoEmitSet.Register( this );
	else if( !mask && m_infoMask )
		m_level->m_infoEmitSet.Unregister( this );
	m_infoMask = mask;
}



GameLevel::GameLevel( PhyWorldHandle phyWorld ) :
	m_phyWorld( phyWorld ),
	m_nameIDGen( 0 ),
	m_currentTickTime( 0 ),
	m_currentPhyTime( 0 ),
	m_deltaTime( 0 ),
	m_editorMode( false ),
	m_paused( false ),
	m_levelTime( 0 ),
	m_player( NULL )
{
	LOG_FUNCTION;
	
	m_scriptCtx.RegisterInputAPI();
	m_scriptCtx.RegisterBatchRenderer();
	
	m_guiSys = new GameUISystem( &m_scriptCtx );
	
	// handled events
	RegisterHandler( EID_WindowEvent );
	
	// create the scripted self
	{
		SGS_CSCOPE( GetSGSC() );
		sgs_CreateClass( GetSGSC(), NULL, this );
		m_self = sgsVariable( GetSGSC(), -1 );
		C = GetSGSC();
		m_sgsObject = sgs_GetObjectStruct( C, -1 );
		sgs_ObjAcquire( C, m_sgsObject );
	}
	
	// register basic constants
	sgs_RegIntConst ric[] =
	{
		{ "IEST_InteractiveItem", IEST_InteractiveItem },
		{ "IEST_HeatSource", IEST_HeatSource },
		{ "IEST_Player", IEST_Player },
		{ "IEST_Target", IEST_Target },
		{ "IEST_AIAlert", IEST_AIAlert },
		{ NULL, 0 },
	};
	sgs_RegIntConsts( C, ric, -1 );
	
	// init backing store
	m_metadata = m_scriptCtx.CreateDict();
	
	// create entity type map
	AddEntry( "entity_types", m_scriptCtx.CreateDict() );
	
	// create marker pos. array
	m_markerPositions = m_scriptCtx.CreateDict();
	AddEntry( "positions", m_markerPositions );
	
	m_scene = GR_CreateScene();
	m_scene->clearColor = 0;
	m_scene->camera.position = Vec3::Create( 4, 4, 4 );
	m_scene->camera.direction = Vec3::Create( -1, -1, -1 ).Normalized();
	m_scene->camera.aspect = 1024.0f / 576.0f;
	m_scene->camera.znear = 0.1f;
	m_scene->camera.UpdateMatrices();
}

GameLevel::~GameLevel()
{
	ClearLevel();
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->OnLevelDestroy();
	
	m_sgsObject->data = NULL;
	m_sgsObject->iface = g_sgsobj_empty_handle;
	sgs_ObjRelease( C, m_sgsObject );
}

void GameLevel::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	switch( eid )
	{
	case EID_WindowEvent:
		{
			SGRX_CAST( Event*, ev, edata.GetUserData() );
			m_guiSys->EngineEvent( *ev );
		}
		break;
	}
}


void GameLevel::SetGlobalToSelf()
{
	m_scriptCtx.SetGlobal( "level", m_self );
}

void GameLevel::AddSystem( IGameLevelSystem* sys )
{
	m_systems.push_back( sys );
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
	
	ByteReader br( ba );
	LC_Level levelData;
	br << levelData;
	
	for( size_t cid = 0; cid < levelData.chunks.size(); ++cid )
	{
		const LC_Chunk& C = levelData.chunks[ cid ];
		StringView type( C.sys_id, 4 );
		
		LOG_FUNCTION_ARG( type );
		
		for( size_t i = 0; i < m_systems.size(); ++i )
		{
			m_systems[ i ]->LoadChunk( type, ByteView( C.ptr, C.size ) );
		}
		
		if( type == LC_FILE_ENTS_NAME )
		{
			LOG_FUNCTION_ARG( "ENTITIES" );
			
			LC_Chunk_Ents ents;
			ByteReader ebr( ByteView( C.ptr, C.size ) );
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
				Entity* E = CreateEntity( SEA[ i ].type );
				if( E )
					ScriptAssignProperties( E->GetScriptedObject(), data );
			}
		}
		else if( type == LC_FILE_MRKR_NAME )
		{
			LOG_FUNCTION_ARG( "MARKERS" );
			
			LC_Chunk_Mrkr mrkr;
			ByteReader mbr( ByteView( C.ptr, C.size ) );
			mbr << mrkr;
			
			for( size_t i = 0; i < mrkr.markers.size(); ++i )
			{
				const LC_Marker& M = mrkr.markers[ i ];
				m_markerPositions.setprop(
					m_scriptCtx.CreateString( M.name ),
					m_scriptCtx.CreateVec3( M.pos ) );
			}
		}
	}
	
	m_currentTickTime = 0;
	m_currentPhyTime = 0;
	m_levelTime = 0;
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->OnPostLevelLoad();
	
	m_scriptCtx.GlobalCall( "onLevelStart" );
	
	return true;
}

void GameLevel::EnumEntities( Array< StringView >& out )
{
	sgsVariable entarr = m_self.getprop( "entity_types" );
	ScriptVarIterator it( entarr );
	while( it.Advance() )
	{
		out.push_back( it.GetKey().get<StringView>() );
	}
}

Entity* GameLevel::CreateEntity( const StringView& type )
{
	for( size_t i = 0; i < m_systems.size(); ++i )
	{
		Entity* ent = m_systems[ i ]->AddEntity( type );
		if( ent )
		{
			ent->InitScriptInterface();
			m_entities.push_back( ent );
			return ent;
		}
	}
	
	sgsVariable eclass = m_scriptCtx.GetGlobal( type );
	if( !eclass.not_null() )
	{
		LOG << "ENTITY TYPE NOT FOUND: " << type;
		sgs_Msg( GetSGSC(), SGS_ERROR, "failed to find entity: %s", StackPath(type).str );
		return NULL;
	}
	
	StringView native_name = eclass.getprop("__inherit").get<StringView>();
	Entity* ent = CreateEntity( native_name );
	if( !ent )
	{
		LOG_ERROR << "FAILED to create scripted entity '" << type << "'"
			<< " - could not find native entity '" << native_name << "'";
		return NULL;
	}
	ent->_data = m_scriptCtx.CreateDict();
	sgsVariable ESO = ent->GetScriptedObject();
	sgsVariable ent_orig_metaobj = ESO.get_meta_obj();
	sgsVariable eclass_orig_metaobj = eclass.get_meta_obj();
	if( eclass_orig_metaobj.not_null() &&
		ent_orig_metaobj.get_object_struct() != eclass_orig_metaobj.get_object_struct() )
	{
		LOG_ERROR << "FAILED to create scripted entity: redefining metaobject for '" << type << "'";
		delete ent;
		return NULL;
	}
	eclass.set_meta_obj( ent_orig_metaobj );
	ESO.set_meta_obj( eclass );
	ESO.enable_metamethods( true );
	
	sgsVariable fn_init = ESO.getprop("Init");
	if( fn_init.not_null() )
		ESO.thiscall( GetSGSC(), fn_init );
	return ent;
}

void GameLevel::DestroyEntity( Entity* eptr )
{
	delete eptr;
	m_entities.remove_all( eptr );
}

StackShortName GameLevel::GenerateName()
{
	m_nameIDGen++;
	StackShortName tmp("");
	sgrx_snprintf( tmp.str, 15, "_name%u", (unsigned int) m_nameIDGen );
	return tmp;
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
		m_nextLevel = "";
	}
}

void GameLevel::FixedTick( float deltaTime )
{
	if( IsPaused() == false )
	{
		m_currentTickTime += deltaTime;
		
		int ITERS = 10;
		for( int i = 0; i < ITERS; ++i )
			m_phyWorld->Step( deltaTime / ITERS );
		
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->FixedTick( deltaTime );
		
		{
			SGS_SCOPE;
			m_scriptCtx.Push( deltaTime );
			m_scriptCtx.GlobalCall( "onLevelFixedTick", 1 );
		}
	}
}

void GameLevel::Tick( float deltaTime, float blendFactor )
{
	m_deltaTime = deltaTime;
	m_levelTime += deltaTime;
	
	if( IsPaused() == false )
	{
		m_currentPhyTime += deltaTime;
		
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->Tick( deltaTime, blendFactor );
	}
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->Tick( deltaTime, blendFactor );
	
	sgs_ProcessSubthreads( m_scriptCtx.C, deltaTime );
}

void GameLevel::Draw2D()
{
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	if( gcv_cl_gui.value )
	{
		m_guiSys->Draw( m_deltaTime );
		
		for( size_t i = 0; i < m_systems.size(); ++i )
			m_systems[ i ]->DrawUI();
	}
	
	if( gcv_cl_debug.value )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		
		for( size_t i = 0; i < m_systems.size(); ++i )
			m_systems[ i ]->DebugDrawUI();
		
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->DebugDrawUI();
	}
}

void GameLevel::DebugDraw()
{
	if( gcv_cl_debug.value )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		UNUSED( br );
		
		for( size_t i = 0; i < m_systems.size(); ++i )
			m_systems[ i ]->DebugDrawWorld();
		
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->DebugDrawWorld();
		
		m_scriptCtx.GlobalCall( "onLevelDebugDraw" );
		
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
#endif
	}
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

sgsVariable GameLevel::sgsCreateEntity( StringView type )
{
	Entity* E = CreateEntity( type );
	return E ? E->GetScriptedObject() : sgsVariable();
}

void GameLevel::sgsDestroyEntity( sgsVariable eh )
{
	Entity* E = eh.downcast<Entity>();
	if( E )
		DestroyEntity( E );
}

Entity::ScrHandle GameLevel::sgsFindEntity( StringView name )
{
	return Entity::ScrHandle( FindEntityByName( name ) );
}

void GameLevel::sgsSetCameraPosDir( Vec3 pos, Vec3 dir )
{
	m_scene->camera.position = pos;
	m_scene->camera.direction = dir;
	m_scene->camera.UpdateMatrices();
}

SGS_MULTRET GameLevel::sgsWorldToScreen( Vec3 pos )
{
	bool infront = false;
	sgs_PushVar( C, m_scene->camera.WorldToScreen( pos, &infront ) );
	sgs_PushVar( C, infront );
	return 2;
}

SGS_MULTRET GameLevel::sgsWorldToScreenPx( Vec3 pos )
{
	bool infront = false;
	sgs_PushVar( C, m_scene->camera.WorldToScreen( pos, &infront )
		* V3( GR_GetWidth(), GR_GetHeight(), 1 ) );
	sgs_PushVar( C, infront );
	return 2;
}

SGS_MULTRET GameLevel::sgsGetCursorWorldPoint( uint32_t layers )
{
	Vec2 cpn = Game_GetCursorPosNormalized();
	Vec3 pos, dir, end;
	if( !m_scene->camera.GetCursorRay( cpn.x, cpn.y, pos, dir ) )
		return 0;
	SceneRaycastInfo hitinfo;
	end = pos + dir * m_scene->camera.zfar;
	if( !m_scene->RaycastOne( pos, end, &hitinfo, sgs_StackSize( C ) >= 1 ? layers : 0xffffffff ) )
		return 0;
	sgs_PushVar( C, TLERP( pos, end, hitinfo.factor ) );
	return 1;
}



bool GameLevel::Query( EntityProcessor* optProc, uint32_t mask )
{
	return m_infoEmitSet.Query( InfoEmitEntitySet::NoTest(), mask, optProc );
}

bool GameLevel::QuerySphere( EntityProcessor* optProc, uint32_t mask, Vec3 pos, float rad )
{
	InfoEmitEntitySet::SphereTest test = { pos, rad * rad };
	return m_infoEmitSet.Query( test, mask, optProc );
}

bool GameLevel::QueryOBB( EntityProcessor* optProc, uint32_t mask, Mat4 mtx, Vec3 bbmin, Vec3 bbmax )
{
	InfoEmitEntitySet::OBBTest test = { bbmin, bbmax, mtx.Inverted() };
	return m_infoEmitSet.Query( test, mask, optProc );
}

struct EP_sgsFunc : EntityProcessor
{
	sgsVariable func;
	EP_sgsFunc( sgsVariable src ) : func( src ){}
	bool ProcessEntity( Entity* E )
	{
		SGS_CSCOPE( func.get_ctx() );
		sgs_PushVar( func.get_ctx(), E->GetScriptedObject() );
		func.call( func.get_ctx(), 1, 1 );
		return sgs_GetBool( func.get_ctx(), -1 );
	}
};

bool GameLevel::sgsQuery( sgsVariable optProc, uint32_t mask )
{
	if( optProc.not_null() )
	{
		EP_sgsFunc ep( optProc );
		return Query( &ep, mask );
	}
	return Query( NULL, mask );
}

bool GameLevel::sgsQuerySphere( sgsVariable optProc, uint32_t mask, Vec3 pos, float rad )
{
	if( optProc.not_null() )
	{
		EP_sgsFunc ep( optProc );
		return QuerySphere( &ep, mask, pos, rad );
	}
	return QuerySphere( NULL, mask, pos, rad );
}

bool GameLevel::sgsQueryOBB( sgsVariable optProc, uint32_t mask, Mat4 mtx, Vec3 bbmin, Vec3 bbmax )
{
	if( sgs_StackSize( C ) < 4 ) bbmin = V3(-1);
	if( sgs_StackSize( C ) < 5 ) bbmax = V3(1);
	if( optProc.not_null() )
	{
		EP_sgsFunc ep( optProc );
		return QueryOBB( &ep, mask, mtx, bbmin, bbmax );
	}
	return QueryOBB( NULL, mask, mtx, bbmin, bbmax );
}



void GameLevel::LightMesh( SGRX_MeshInstance* meshinst, Vec3 off )
{
	SGRX_LightSampler::LightMesh( meshinst, off );
}


BaseEditor::BaseEditor( BaseGame* game ) : m_editorGame( NULL ), m_origGame( NULL )
{
	m_lib = Sys_LoadLib( "editor.dll" );
	if( !m_lib )
		return;
	IGame* (*pfnCreateGame)() = (IGame*(*)()) Sys_GetProc( m_lib, "CreateGame" );
	if( !pfnCreateGame )
		return;
	void (*pfnSetBaseGame)(BaseGame*) = (void(*)(BaseGame*)) Sys_GetProc( m_lib, "SetBaseGame" );
	m_editorGame = (*pfnCreateGame)();
	(*pfnSetBaseGame)( game );
	m_origGame = Game_Change( m_editorGame );
}

BaseEditor::~BaseEditor()
{
	Game_Change( m_origGame );
	if( m_lib )
	{
		Sys_UnloadLib( m_lib );
		m_lib = NULL;
	}
}


BaseGame::BaseGame() :
	m_maxTickSize( 1.0f/15.0f ),
	m_fixedTickSize( 1.0f/30.0f ),
	m_accum( 0 ),
	m_timeMultiplier( 1 ),
	m_level( NULL ),
	m_editor( NULL ),
	m_needsEditor( false )
{
}

int BaseGame::OnArgument( char* arg, int argcleft, char** argvleft )
{
	if( streq( arg, "EDIT" ) )
	{
		m_needsEditor = true;
		return 1;
	}
	return 0;
}

bool BaseGame::OnConfigure( int argc, char** argv )
{
	IGame::OnConfigure( argc, argv );
	if( m_needsEditor )
	{
		m_editor = new BaseEditor( this );
		return m_editor->m_editorGame->OnConfigure( argc, argv );
	}
	return true;
}

bool BaseGame::OnInitialize()
{
	m_soundSys = SND_CreateSystem();
	m_level = CreateLevel();
	return true;
}

void BaseGame::OnDestroy()
{
	SAFE_DELETE( m_level );
	m_soundSys = NULL;
}

GameLevel* BaseGame::CreateLevel()
{
	GameLevel* level = new GameLevel( PHY_CreateWorld() );
	level->m_soundSys = m_soundSys;
	level->SetGlobalToSelf();
	level->GetPhyWorld()->SetGravity( V3( 0, 0, -9.81f ) );
	return level;
}

void BaseGame::Game_FixedTick( float dt )
{
	m_level->FixedTick( dt );
}

void BaseGame::Game_Tick( float dt, float bf )
{
	m_level->Tick( dt, bf );
}

void BaseGame::Game_Render()
{
	m_level->Draw();
	m_level->Draw2D();
}

void BaseGame::OnTick( float dt, uint32_t gametime )
{
	m_soundSys->Update();
	m_level->ProcessEvents();
	
	if( dt > m_maxTickSize )
		dt = m_maxTickSize;
	dt *= m_timeMultiplier;
	
	m_accum += dt;
	while( m_accum >= 0 )
	{
		Game_FixedTick( m_fixedTickSize );
		m_accum -= m_fixedTickSize;
	}
	
	Game_Tick( dt, ( m_accum + m_fixedTickSize ) / m_fixedTickSize );
	
	Game_Render();
}


