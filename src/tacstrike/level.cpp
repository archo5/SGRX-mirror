

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#define USE_SERIALIZATION
#include "level.hpp"


static int SendMessage( SGS_CTX )
{
	SGSFN( "SendMessage" );
	int ssz = sgs_StackSize( C );
	g_GameLevel->m_messageSystem.AddMessage( (MSMessage::Type) sgs_GetVar<int>()( C, 0 ), sgs_GetVar<String>()( C, 1 ), ssz < 3 ? 3.0f : sgs_GetVar<float>()( C, 2 ) );
	return 0;
}
static int EndLevel( SGS_CTX )
{
	SGSFN( "EndLevel" );
	g_GameLevel->m_endFactor = 0;
	return 0;
}
static int CallEntity( SGS_CTX )
{
	SGSFN( "CallEntity" );
	g_GameLevel->CallEntityByName( sgs_GetVar<String>()( C, 0 ), sgs_GetVar<String>()( C, 1 ) );
	return 0;
}
static int PlayerHasItem( SGS_CTX )
{
	SGSFN( "PlayerHasItem" );
	sgs_PushVar<bool>( C, g_GameLevel->m_player && g_GameLevel->m_player->HasItem( sgs_GetVar<String>()( C, 0 ), sgs_GetVar<int>()( C, 1 ) ) );
	return 1;
}
static int ObjectiveAdd( SGS_CTX )
{
	SGSFN( "ObjectiveAdd" );
	sgs_PushVar<int>( C, g_GameLevel->m_objectiveSystem.AddObjective( sgs_GetVar<String>()( C, 0 ), (OSObjective::State) sgs_GetVar<int>()( C, 1 ) ) );
	return 1;
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
static int ObjectiveGetText( SGS_CTX )
{
	SGSFN( "ObjectiveGetText" );
	int which = sgs_GetVar<int>()( C, 0 );
	if( which < 0 || which >= (int) g_GameLevel->m_objectiveSystem.m_objectives.size() )
		return sgs_Msg( C, SGS_WARNING, "index out of bounds: %d", which );
	sgs_PushVar( C, g_GameLevel->m_objectiveSystem.m_objectives[ which ].text );
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
#ifdef LD32GAME
static int EnemyDefine( SGS_CTX )
{
	SGSFN( "EnemyDefine" );
	String name = sgs_GetVar<String>()( C, 0 );
	Enemy* enemy = (Enemy*) g_GameLevel->FindEntityByName( name );
	if( !enemy )
		return sgs_Msg( C, SGS_WARNING, "failed to find enemy entity: %.*s", (int) name.size(), name.data() );
	if( strcmp( enemy->m_typeName, "enemy" ) != 0 )
		return sgs_Msg( C, SGS_WARNING, "found entity is not 'enemy': %.*s", (int) name.size(), name.data() );
	LD32ParseTaskArray( enemy->m_patrolTasks, sgs_GetVar<sgsVariable>()( C, 1 ) );
	if( sgs_StackSize( C ) > 2 )
	{
		enemy->m_disturbActionName = sgs_GetVar<String>()( C, 2 );
		LD32ParseTaskArray( enemy->m_disturbTasks, sgs_GetVar<sgsVariable>()( C, 3 ) );
	}
	enemy->UpdateTask();
	return 0;
}
#endif

static sgs_RegFuncConst g_gameapi_rfc[] =
{
	{ "SendMessage", SendMessage },
	{ "EndLevel", EndLevel },
	{ "CallEntity", CallEntity },
	{ "PlayerHasItem", PlayerHasItem },
	{ "ObjectiveAdd", ObjectiveAdd },
	{ "ObjectiveGetText", ObjectiveGetText },
	{ "ObjectiveGetState", ObjectiveGetState },
	{ "ObjectiveSetState", ObjectiveSetState },
#ifdef LD32GAME
	{ "EnemyDefine", EnemyDefine },
#endif
	SGS_RC_END(),
};

static sgs_RegIntConst g_gameapi_ric[] =
{
	{ "MT_Continued", MSMessage::Continued },
	{ "MT_Info", MSMessage::Info },
	{ "MT_Warning", MSMessage::Warning },
	{ "OS_Hidden", OSObjective::Hidden },
	{ "OS_Open", OSObjective::Open },
	{ "OS_Done", OSObjective::Done },
	{ "OS_Failed", OSObjective::Failed },
	{ "OS_Cancelled", OSObjective::Cancelled },
#ifdef LD32GAME
	{ "TT_Wait", TT_Wait },
	{ "TT_Turn", TT_Turn },
	{ "TT_Walk", TT_Walk },
#endif
	{ NULL, 0 },
};

static int InitGameAPI( SGS_CTX )
{
	sgs_RegFuncConsts( C, g_gameapi_rfc, -1 );
	sgs_RegIntConsts( C, g_gameapi_ric, -1 );
	return 0;
}


GameLevel::GameLevel() :
	m_nameIDGen( 0 ),
	m_bulletSystem( &m_damageSystem ),
	m_paused( false ),
	m_endFactor( -1 ),
	m_levelTime( 0 ),
	m_player( NULL )
{
	m_playerSpawnInfo[0] = V3(0);
	m_levelCameraInfo[0] = V3(0);
	m_playerSpawnInfo[1] = V3(1,0,0);
	m_levelCameraInfo[1] = V3(1,0,0);
	
	m_scene = GR_CreateScene();
	m_scene->camera.position = Vec3::Create( 4, 4, 4 );
	m_scene->camera.direction = Vec3::Create( -1, -1, -1 ).Normalized();
	m_scene->camera.aspect = 1024.0f / 576.0f;
	m_scene->camera.UpdateMatrices();
	
	const char* err = m_damageSystem.Init( m_scene );
	if( err )
	{
		LOG_ERROR << LOG_DATE << "  Failed to init DMGSYS: " << err;
	}
	
	m_ps_flare = GR_GetPixelShader( "flare" );
	m_tex_flare = GR_GetTexture( "textures/fx/flare.png" );
	m_tex_mapline = GR_GetTexture( "ui/mapline.png" );
	m_tex_mapframe = GR_GetTexture( "ui/mapframe.png" );
	
	InitGameAPI( m_scriptCtx.C );
}

GameLevel::~GameLevel()
{
	ClearLevel();
	m_damageSystem.Free();
}


bool GameLevel::Load( const StringView& levelname )
{
	char bfr[ 256 ];
	snprintf( bfr, sizeof(bfr), "levels/%.*s/cache", TMIN( (int) levelname.size(), 200 ), levelname.data() );
	ByteArray ba;
	if( !FS_LoadBinaryFile( bfr, ba ) )
		return false;
	
	ClearLevel();
	
	m_scriptCtx.Include( "data/enemy" );
	
	snprintf( bfr, sizeof(bfr), "levels/%.*s", TMIN( (int) levelname.size(), 200 ), levelname.data() );
	m_scriptCtx.Include( bfr );
	
	ByteReader br( &ba );
	
	br.marker( "COMPILED" );
	SerializeVersionHelper<ByteReader> svh( br, LC_FILE_VERSION );
	
	svh.marker( "SCRENTS" );
	Array< LC_ScriptedEntity > screntdefs;
	svh << screntdefs;
	
	svh.marker( "INST" );
	Array< LC_MeshInst > mesh_inst_defs;
	svh << mesh_inst_defs;
	
	svh.marker( "LINES" );
	svh << m_lines;
	
	svh.marker( "LIGHTS" );
	svh << m_lights;
	
	svh.marker( "SAMPLES" );
	Array< LightTree::Sample > lt_samples;
	svh << lt_samples;
	
	// LOAD LIGHT SAMPLES
//	for( size_t i = 0; i < lt_samples.size(); ++i )
//	{
//		LOG << lt_samples[i].pos << "|" << lt_samples[i].color[0] << lt_samples[i].color[1] << lt_samples[i].color[2] << lt_samples[i].color[3] << lt_samples[i].color[4] << lt_samples[i].color[5];
//	}
//	m_ltSamples.InsertSample( lt_samples[ 10 ] );
	m_ltSamples.InsertSamples( lt_samples.data(), lt_samples.size() );
	
	svh.marker( "PHYMESH" );
	LC_PhysicsMesh phy_mesh;
	svh( phy_mesh, svh.version >= 5 );
	
	ByteArray navmesh;
	if( svh.version >= 6 )
	{
		svh.marker( "NAVMESH" );
		svh( navmesh );
	}
	m_aidbSystem.Load( navmesh );
	
	// CREATE STATIC GEOMETRY
	SGRX_PhyRigidBodyInfo rbinfo;
	
	// TODO: temporarily ignore material data
	Array< uint32_t > fixedidcs;
	for( size_t i = 0; i < phy_mesh.indices.size(); i += 4 )
	{
		fixedidcs.append( &phy_mesh.indices[ i ], 3 );
	}
	rbinfo.shape = g_PhyWorld->CreateTriMeshShape(
		phy_mesh.positions.data(), phy_mesh.positions.size(),
		fixedidcs.data(), fixedidcs.size(), true );
	m_levelBodies.push_back( g_PhyWorld->CreateRigidBody( rbinfo ) );
	
	for( size_t i = 0; i < mesh_inst_defs.size(); ++i )
	{
		char subbfr[ 512 ];
		MeshInstHandle MI = m_scene->CreateMeshInstance();
		StringView src = mesh_inst_defs[ i ].m_meshname;
		if( src.ch() == '~' )
		{
			snprintf( subbfr, sizeof(subbfr), "levels/%.*s%.*s", TMIN( (int) levelname.size(), 200 ), levelname.data(), TMIN( (int) src.size() - 1, 200 ), src.data() + 1 );
			MI->mesh = GR_GetMesh( subbfr );
		}
		else
			MI->mesh = GR_GetMesh( src );
		
		snprintf( subbfr, sizeof(subbfr), "levels/%.*s/%d.png", TMIN( (int) levelname.size(), 200 ), levelname.data(), (int) i );
		MI->textures[0] = GR_GetTexture( subbfr );
		
		MI->matrix = mesh_inst_defs[ i ].m_mtx;
		
		if( mesh_inst_defs[ i ].m_flags & LM_MESHINST_DYNLIT )
		{
			MI->dynamic = true;
			LightMesh( MI );
		}
		
		if( mesh_inst_defs[ i ].m_flags & LM_MESHINST_DECAL )
		{
			MI->decal = true;
			MI->transparent = true;
		}
		
		m_meshInsts.push_back( MI );
		
		if( mesh_inst_defs[ i ].m_flags & LM_MESHINST_SOLID )
		{
			rbinfo.shape = g_PhyWorld->CreateShapeFromMesh( MI->mesh );
			rbinfo.shape->SetScale( MI->matrix.GetScale() );
			rbinfo.position = MI->matrix.GetTranslation();
			rbinfo.rotation = MI->matrix.GetRotationQuaternion();
			m_levelBodies.push_back( g_PhyWorld->CreateRigidBody( rbinfo ) );
		}
	}
	
	
	// CREATE ENTITIES
	for( size_t i = 0; i < screntdefs.size(); ++i )
	{
		CreateEntity( screntdefs[ i ].type, screntdefs[ i ].serialized_params );
	}
	
	
	return true;
}

void GameLevel::ClearLevel()
{
	EndLevel();
	m_ltSamples.Clear();
	m_damageSystem.Clear();
	m_bulletSystem.Clear();
	m_lights.clear();
	m_meshInsts.clear();
	m_levelBodies.clear();
}

void GameLevel::CreateEntity( const StringView& type, const StringView& sgsparams )
{
	sgsVariable data = m_scriptCtx.Unserialize( sgsparams );
	if( data.not_null() == false )
	{
		LOG << "BAD PARAMS FOR ENTITY " << type;
		return;
	}
	
	///////////////////////////
	if( type == "player_start" )
	{
		m_playerSpawnInfo[0] = data.getprop("position").get<Vec3>();
		m_playerSpawnInfo[1] = data.getprop("viewdir").get<Vec3>().Normalized();
		return;
	}
	
#if defined(LD32GAME) || defined(TSGAME)
	///////////////////////////
	if( type == "enemy_start" )
	{
#ifdef LD32GAME
		Enemy* E = new Enemy
#endif
#ifdef TSGAME
		TSEnemy* E = new TSEnemy
#endif
		(
			data.getprop("name").get<String>(),
			data.getprop("position").get<Vec3>(),
			data.getprop("viewdir").get<Vec3>()
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
	if( type == "solidbox" )
	{
		Vec3 scale = data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>();
		SGRX_PhyRigidBodyInfo rbinfo;
		rbinfo.shape = g_PhyWorld->CreateAABBShape( -scale, scale );
		rbinfo.mass = 0;
		rbinfo.inertia = V3(0);
		rbinfo.position = data.getprop("position").get<Vec3>();
		rbinfo.rotation = Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion();
		m_levelBodies.push_back( g_PhyWorld->CreateRigidBody( rbinfo ) );
		return;
	}
	
	///////////////////////////
	if( type == "trigger" )
	{
		BoxTrigger* BT = new BoxTrigger
		(
			data.getprop("func").get<String>(),
			data.getprop("target").get<String>(),
			data.getprop("once").get<bool>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>()
		);
		m_entities.push_back( BT );
		return;
	}
	
	///////////////////////////
	if( type == "trigger_prox" )
	{
		ProximityTrigger* PT = new ProximityTrigger
		(
			data.getprop("func").get<String>(),
			data.getprop("target").get<String>(),
			data.getprop("once").get<bool>(),
			data.getprop("position").get<Vec3>(),
			data.getprop("distance").get<float>()
		);
		m_entities.push_back( PT );
		return;
	}
	
	///////////////////////////
	if( type == "door_slide" )
	{
		SlidingDoor* SD = new SlidingDoor
		(
			data.getprop("name").get<String>(),
			data.getprop("mesh").get<String>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("open_offset").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("open_rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			V3(0),
			Quat::Identity,
			data.getprop("open_time").get<float>(),
			false,
			data.getprop("is_switch").get<bool>(),
			data.getprop("pred").get<String>(),
			data.getprop("func").get<String>(),
			data.getprop("target").get<String>(),
			data.getprop("once").get<bool>()
		);
		m_entities.push_back( SD );
		return;
	}
	
	///////////////////////////
	if( type == "door_slide_prox" )
	{
		StackShortName name = GenerateName();
		
		SlidingDoor* SD = new SlidingDoor
		(
			name.str,
			data.getprop("mesh").get<String>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("open_offset").get<Vec3>(),
			Quat::Identity,
			V3(0),
			Quat::Identity,
			data.getprop("open_time").get<float>(),
			false
		);
		m_entities.push_back( SD );
		
		ProximityTrigger* PT = new ProximityTrigger
		(
			"", name.str, false,
			SD->meshInst->matrix.TransformPos( data.getprop("scan_offset").get<Vec3>() ),
			data.getprop("scan_distance").get<float>()
		);
		m_entities.push_back( PT );
		return;
	}
	
	///////////////////////////
	if( type == "pickup" )
	{
		PickupItem* PI = new PickupItem
		(
			data.getprop("id").get<String>(),
			data.getprop("name").get<String>(),
			data.getprop("count").get<int>(),
			data.getprop("mesh").get<String>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>()
		);
		m_entities.push_back( PI );
		return;
	}
	
	///////////////////////////
	if( type == "particle_fx" )
	{
		ParticleFX* PF = new ParticleFX
		(
			data.getprop("name").get<String>(),
			data.getprop("partsys").get<String>(),
			data.getprop("soundevent").get<String>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("start").get<bool>()
		);
		m_entities.push_back( PF );
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
	snprintf( tmp.str, 15, "_name%u", (unsigned int) m_nameIDGen );
	return tmp;
}

void GameLevel::StartLevel()
{
	m_endFactor = -1;
	m_cameraInfoCached = false;
	m_levelTime = 0;
	if( !m_player )
	{
		Player* P = new Player
		(
			m_playerSpawnInfo[0]
			,m_playerSpawnInfo[1]
		);
		m_player = P;
	}
	m_scriptCtx.GlobalCall( "onLevelStart" );
}

void GameLevel::EndLevel()
{
	for( size_t i = 0; i < m_entities.size(); ++i )
		delete m_entities[ i ];
	m_entities.clear();
	
	m_endFactor = -1;
	m_cameraInfoCached = false;
	if( m_player )
	{
		delete m_player;
		m_player = NULL;
	}
}

void GameLevel::FixedTick( float deltaTime )
{
	if( !m_paused )
	{
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->FixedTick( deltaTime );
		if( m_player )
			m_player->FixedTick( deltaTime );
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
		if( m_player )
			m_player->Tick( deltaTime, blendFactor );
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->Tick( deltaTime, blendFactor );
		m_damageSystem.Tick( deltaTime );
		m_bulletSystem.Tick( m_scene, deltaTime );
	}
	
	m_messageSystem.Tick( deltaTime );
	m_objectiveSystem.Tick( deltaTime );
	
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
	
	m_messageSystem.DrawUI();
	m_objectiveSystem.DrawUI();
//	if( m_player )
//		m_player->Draw2D();
	
	// LIGHTS
//	BatchRenderer& br = GR2D_GetBatchRenderer();
//	for( size_t i = 0; i < m_lights.size(); ++i )
//	{
//		Light& L = m_lights[ i ];
//		br.SetTexture( L.tex_flare ).Col( L.color.x, L.color.y, L.color.z, 0.2 );
//		br.Box( L.pos.x, L.pos.y, L.radius, L.radius );
//	}
	
	//
	// UI
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	
	int size_x = GR_GetWidth();
	int size_y = GR_GetHeight();
//	float aspect = size_x / (float) size_y;
	int sqr = TMIN( size_x, size_y );
//	int margin_x = ( size_x - sqr ) / 2;
//	int margin_y = ( size_y - sqr ) / 2;
	int safe_margin = sqr * 1 / 16;
	
#ifndef BRSD4GAME
	// MAP
	{
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
#ifdef LD32GAME
		Vec2 pos = m_player->m_position;
#elif defined(TSGAME)
		Vec2 pos = m_player->m_bodyHandle->GetPosition().ToVec2();
#else
		Vec2 pos = m_scene->camera.position.ToVec2();
#endif
		Mat4 lookat = Mat4::CreateLookAt( V3( pos.x, pos.y, -0.5f ), V3(0,0,1), V3(0,-1,0) );
		GR2D_SetViewMatrix( lookat * Mat4::CreateScale( 1.0f / ( 8 * map_aspect ), 1.0f / 8, 1 ) );
		
		GR2D_SetScissorRect( x0, y0, x1, y1 );
		GR2D_SetViewport( x0, y0, x1, y1 );
		
		for( size_t i = 0; i < m_lines.size(); i += 2 )
		{
			Vec2 l0 = m_lines[ i ];
			Vec2 l1 = m_lines[ i + 1 ];
			
			br.TexLine( l0, l1, 0.1f );
		}
		
		br.Reset().SetTexture( m_tex_mapline ).Col( 0.2f, 0.9f, 0.1f ).Box( pos.x, pos.y, 1, 1 );
		
		br.Flush();
		GR2D_UnsetViewport();
		GR2D_UnsetScissorRect();
		
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		
		br.Reset().SetTexture( m_tex_mapframe ).Quad( x0 - msm, y0 - msm, x1 + msm, y1 + msm ).Flush();
	}
#endif
	
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	if( m_player )
		m_player->DrawUI();
}

void GameLevel::DebugDraw()
{
	m_aidbSystem.m_pathfinder.DebugDraw();
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset().Col( 1, 0, 0 );
	
	Array< Vec3 > path;
	if( m_aidbSystem.m_pathfinder.FindPath( V3(-2.4f,10,1), V3(2.6f,-1.4f,1), path ) && path.size() >= 2 )
	{
		br.SetPrimitiveType( PT_LineStrip );
		for( size_t i = 0; i < path.size(); ++i )
			br.Pos( path[ i ] );
	}
}

void GameLevel::PostDraw()
{
//	return;
	
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	float W = GR_GetWidth();
	float H = GR_GetHeight();
	float sz = TMIN( W, H ) * 0.2f;
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset().SetShader( m_ps_flare ).SetTexture( m_tex_flare );
	br.ShaderData.push_back( V4( W, H, 1.0f / W, 1.0f / H ) );
	br.ShaderData.push_back( V4(1) );
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		LC_Light& L = m_lights[ i ];
		if( L.type != LM_LIGHT_POINT && L.type != LM_LIGHT_SPOT )
			continue;
		Vec3 flarepos = L.pos + L.flareoffset;
		float flaresize = L.flaresize;
		if( flaresize <= 0 )
			continue;
		br.ShaderData[1] = V4( L.color, 0.1f / ( ( flarepos - m_scene->camera.position ).Length() + 1 ) );
		Vec3 screenpos = m_scene->camera.WorldToScreen( flarepos );
		if( Vec3Dot( flarepos, m_scene->camera.direction ) < Vec3Dot( m_scene->camera.position, m_scene->camera.direction ) )
			continue;
		if( g_PhyWorld->Raycast( flarepos, m_scene->camera.position, 1, 1 ) )
			continue;
	//	LOG << screenpos.z;
		float dx = cos(0.1f)*0.5f*sz * flaresize;
		float dy = sin(0.1f)*0.5f*sz * flaresize;
		br.TurnedBox( screenpos.x * W, screenpos.y * H, dx, dy );
		br.Flush();
	}
}

void GameLevel::Draw()
{
	SGRX_RenderScene rsinfo( V4( m_levelTime ), m_scene );
	rsinfo.debugdraw = this;
	rsinfo.postdraw = this;
	GR_RenderScene( rsinfo );
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

void GameLevel::CallEntityByName( const StringView& name, const StringView& action )
{
	Entity* e = m_entNameMap.getcopy( name );
	if( e )
		e->OnEvent( action );
}


void GameLevel::LightMesh( MeshInstHandle mih, int32_t* outlastfound )
{
	LightTree::Sample SMP = { mih->matrix.TransformPos( V3(0) ) };
	m_ltSamples.Interpolate( SMP, outlastfound );
	mih->constants[10] = V4( SMP.color[0], 1 );
	mih->constants[11] = V4( SMP.color[1], 1 );
	mih->constants[12] = V4( SMP.color[2], 1 );
	mih->constants[13] = V4( SMP.color[3], 1 );
	mih->constants[14] = V4( SMP.color[4], 1 );
	mih->constants[15] = V4( SMP.color[5], 1 );
}

