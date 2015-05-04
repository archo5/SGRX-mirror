

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#include "level.hpp"


GameLevel* g_GameLevel = NULL;
bool g_Paused = false;
PhyWorldHandle g_PhyWorld;
SoundSystemHandle g_SoundSys;

Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );
Command MOVE_UP( "move_up" );
Command MOVE_DOWN( "move_down" );
Command INTERACT( "interact" );
Command CROUCH( "crouch" );
// UNUSED
Command SHOW_OBJECTIVES( "show_objectives" );


static void resetcontrols()
{
	MOVE_LEFT.value = 0;
	MOVE_RIGHT.value = 0;
	MOVE_UP.value = 0;
	MOVE_DOWN.value = 0;
	INTERACT.value = 0;
	CROUCH.value = 0;
}


SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_SHADOWS, RPF_ENABLED, 1, 0, 0, "shadow" },
	{ RPT_SCREEN, RPF_ENABLED, 1, 0, 0, "ps_ss_fog" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_LIGHTOVERLAY | RPF_ENABLED, 100, 0, 4, "ext_s4" },
	{ RPT_PROJECTORS, RPF_ENABLED, 1, 0, 0, "projector" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_LIGHTOVERLAY | RPF_ENABLED, 100, 0, 4, "ext_s4" },
};



void Game_SetPaused( bool paused )
{
	g_Paused = paused;
}

bool Game_IsPaused()
{
	return g_Paused;
}



ActionInput g_i_move_left = ACTINPUT_MAKE_KEY( SDLK_a );
ActionInput g_i_move_right = ACTINPUT_MAKE_KEY( SDLK_d );
ActionInput g_i_move_up = ACTINPUT_MAKE_KEY( SDLK_w );
ActionInput g_i_move_down = ACTINPUT_MAKE_KEY( SDLK_s );
ActionInput g_i_interact = ACTINPUT_MAKE_MOUSE( SGRX_MB_LEFT );
ActionInput g_i_crouch = ACTINPUT_MAKE_KEY( SDLK_c );

float g_i_mouse_sensitivity = 1.0f;
bool g_i_mouse_invert_x = false;
bool g_i_mouse_invert_y = false;

float g_s_vol_master = 0.8f;
float g_s_vol_sfx = 0.8f;
float g_s_vol_music = 0.8f;



LD32Char::LD32Char( const Vec3& pos, const Vec3& dir, const Vec4& color ) :
	m_footstepTime(0), m_isCrouching(0),
	m_ivPos( pos ), m_ivDir( Quat::CreateAxisAngle( V3(0,0,1), atan2( dir.y, dir.x ) ) ),
	m_position( pos.ToVec2() ), m_moveDir( V2(0) ), m_turnAngle( atan2( dir.y, dir.x ) )
{
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 0;
	rbinfo.restitution = 0;
	rbinfo.shape = g_PhyWorld->CreateCapsuleShape( 0.3f, 1.4f );
	rbinfo.mass = 70;
	rbinfo.inertia = V3(0);
	rbinfo.position = pos + V3(0,0,1);
	rbinfo.canSleep = false;
	rbinfo.group = 2;
	rbinfo.linearFactor = V3(1,1,0);
	m_bodyHandle = g_PhyWorld->CreateRigidBody( rbinfo );
	m_shapeHandle = g_PhyWorld->CreateCylinderShape( V3(0.29f) );
	
	m_meshInst = g_GameLevel->m_scene->CreateMeshInstance();
	m_meshInst->dynamic = 1;
	m_meshInst->layers = 0x2;
	m_meshInst->mesh = GR_GetMesh( "meshes/charmodel2.ssm" );
	m_meshInst->matrix = Mat4::CreateSRT( V3(1), m_ivDir.curr, pos );
	g_GameLevel->LightMesh( m_meshInst );
	m_meshInst->constants[ 0 ] = color;
	m_meshInst->skin_matrices.resize( m_meshInst->mesh->m_numBones );
	
	m_shadowInst = g_GameLevel->m_scene->CreateLight();
	m_shadowInst->type = LIGHT_PROJ;
	m_shadowInst->direction = V3(0,0,-1);
	m_shadowInst->updir = V3(0,1,0);
	m_shadowInst->angle = 60;
	m_shadowInst->range = 1.5f;
	m_shadowInst->UpdateTransform();
	m_shadowInst->projectionShader = GR_GetPixelShader( "mtl:proj_default:base_proj" );
	m_shadowInst->projectionTextures[0] = GR_GetTexture( "textures/fx/blobshadow.png" );//GR_GetTexture( "textures/unit.png" );
	m_shadowInst->projectionTextures[1] = GR_GetTexture( "textures/fx/projfalloff2.png" );
	
	m_anEnd.animSource = &m_anMainPlayer;
	m_anEnd.PrepareForMesh( m_meshInst->mesh );
	m_anMainPlayer.Play( GR_GetAnim( "stand_anim" ) );
}

void LD32Char::FixedTick( float deltaTime )
{
	Vec3 pos = m_bodyHandle->GetPosition() - V3(0,0,1);
	m_position = pos.ToVec2();
	
	bool ground = true;
	Vec3 lvel = m_bodyHandle->GetLinearVelocity();
	Vec2 lvel2 = lvel.ToVec2();
	
	float maxspeed = 2;
	float accel = ( m_moveDir.NearZero() && !m_isCrouching ) ? 38 : 30;
	if( m_isCrouching ){ accel = 5; maxspeed = 1.25f; }
	if( !ground ){ accel = 10; }
	
	float curspeed = Vec2Dot( lvel2, m_moveDir );
	float revmaxfactor = clamp( maxspeed - curspeed, 0, 1 );
	lvel2 += m_moveDir * accel * revmaxfactor * deltaTime;
	
	///// FRICTION /////
	curspeed = Vec2Dot( lvel2, m_moveDir );
	if( ground )
	{
		if( curspeed > maxspeed )
			curspeed = maxspeed;
	}
	lvel2 -= m_moveDir * curspeed;
	{
		Vec2 ldd = lvel2.Normalized();
		float llen = lvel2.Length();
		llen = TMAX( 0.0f, llen - deltaTime * ( ground ? 20 : ( m_isCrouching ? 0.5f : 3 ) ) );
		lvel2 = ldd * llen;
	}
	lvel2 += m_moveDir * curspeed;
	
	lvel.x = lvel2.x;
	lvel.y = lvel2.y;
	
	m_bodyHandle->SetLinearVelocity( lvel );
	
	m_ivPos.Advance( pos );
	m_ivDir.Advance( Quat::CreateAxisAngle( V3(0,0,1), m_turnAngle ) );
	m_anEnd.Advance( deltaTime );
}

void LD32Char::Tick( float deltaTime, float blendFactor )
{
	Vec3 pos = m_ivPos.Get( blendFactor );
	Quat rdir = m_ivDir.Get( blendFactor ).Normalized();
	
	m_meshInst->matrix = Mat4::CreateSRT( V3(1), rdir, pos );
	m_shadowInst->position = pos + V3(0,0,1);
	
	g_GameLevel->LightMesh( m_meshInst );
	
	m_anEnd.Interpolate( blendFactor );
	GR_ApplyAnimator( &m_anEnd, m_meshInst );
}


Player::Player( const Vec3& pos, const Vec3& dir ) :
	LD32Char( pos, dir, V4( 0.5f, 0.7f, 0.9f, 1 ) ),
	m_angles( V2( atan2( dir.y, dir.x ), atan2( dir.z, dir.ToVec2().Length() ) ) ), inCursorMove( V2(0) ),
	m_targetII( NULL ), m_targetTriggered( false )
{
}

void Player::FixedTick( float deltaTime )
{
	Vec2 realdir = { cos( m_angles.x ), sin( m_angles.x ) };
	Vec2 perpdir = realdir.Perp();
	
	Vec2 md = { MOVE_LEFT.value - MOVE_RIGHT.value, MOVE_DOWN.value - MOVE_UP.value };
	md.Normalize();
	md = -realdir * md.y - perpdir * md.x;
	
	m_moveDir = md * 1.1f;
	
	bool moving = m_moveDir.Length() > 0.1f;
	const char* animname =
		m_isCrouching
		? ( moving ? "crouch_walk" : "crouch" )
		: ( moving ? "sneak" : "stand_anim" )
	;
	m_anMainPlayer.Play( GR_GetAnim( animname ) );
	
	if( md.Length() > 0.1f )
	{
		float angend = normalize_angle( m_moveDir.Angle() );
		float angstart = normalize_angle( m_turnAngle );
		if( fabs( angend - angstart ) > M_PI )
			angstart += angend > angstart ? M_PI * 2 : -M_PI * 2;
		m_turnAngle = angstart + sign( angend - angstart ) * TMIN( fabsf( angend - angstart ), deltaTime * 8 );
	}
	
	LD32Char::FixedTick( deltaTime );
}

void Player::Tick( float deltaTime, float blendFactor )
{
	if( CROUCH.IsPressed() )
	{
		m_isCrouching = !m_isCrouching;
	}
	
	LD32Char::Tick( deltaTime, blendFactor );
	
	m_angles += inCursorMove * V2(-0.01f);
	m_angles.y = clamp( m_angles.y, (float) -M_PI/2 + SMALL_FLOAT, (float) M_PI/2 - SMALL_FLOAT );
	
	float ch = cos( m_angles.x ), sh = sin( m_angles.x );
	float cv = cos( m_angles.y ), sv = sin( m_angles.y );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
	Vec3 dir = V3( ch * cv, sh * cv, sv );
	
	g_GameLevel->m_scene->camera.znear = 0.1f;
	g_GameLevel->m_scene->camera.angle = 90;
	g_GameLevel->m_scene->camera.direction = dir;
	g_GameLevel->m_scene->camera.position = pos - dir * 2 + V3(0,0,1.5f);
	g_GameLevel->m_scene->camera.UpdateMatrices();
}

void Player::DrawUI()
{
	if( m_targetII )
	{
		float bsz = TMIN( GR_GetWidth(), GR_GetHeight() );
		float x = GR_GetWidth() / 2.0f;
		float y = GR_GetHeight() / 2.0f;
		GR2D_GetBatchRenderer().Reset().Col(1).SetTexture( m_tex_interact_icon ).QuadWH( x, y, bsz / 10, bsz / 10 );
	}
}

bool Player::AddItem( const StringView& item, int count )
{
	String key = item;
	int* ic = m_items.getptr( key );
	if( count < 0 )
	{
		if( !ic || *ic < count )
			return false;
		*ic += count;
	}
	else
	{
		if( !ic )
			m_items.set( key, count );
		else
			*ic += count;
	}
	return true;
}

bool Player::HasItem( const StringView& item, int count )
{
	int* ic = m_items.getptr( item );
	return ic && *ic >= count;
}


Enemy::Enemy( const StringView& name, const Vec3& pos, const Vec3& dir ) :
	LD32Char( pos, dir, V4( 0.8f, 0.1f, 0.05f, 1 ) ),
	m_taskTimeout( 0 ), m_curTaskID( 0 ), m_curTaskMode( false ), m_turnAngleStart(0), m_turnAngleEnd(0)
{
	m_typeName = "enemy";
	m_name = name;
	
	UpdateTask();
	
	g_GameLevel->MapEntityByName( this );
}

void Enemy::FixedTick( float deltaTime )
{
	LD32Char::FixedTick( deltaTime );
	
	LD32TaskArray* ta = m_curTaskMode ? &m_disturbTasks : &m_patrolTasks;
	if( ta->size() )
	{
		m_taskTimeout -= deltaTime;
		
		LD32Task& T = (*ta)[ m_curTaskID ];
		switch( T.type )
		{
		case TT_Wait:
			m_moveDir = V2(0);
			m_anMainPlayer.Play( GR_GetAnim( "stand_anim" ) );
			break;
		case TT_Turn:
			m_moveDir = V2(0);
			m_turnAngle = TLERP( m_turnAngleStart, m_turnAngleEnd, 1 - m_taskTimeout / T.timeout );
			m_anMainPlayer.Play( GR_GetAnim( "turn" ) );
			break;
		case TT_Walk:
			m_moveDir = ( T.target - m_position ).Normalized();
			m_anMainPlayer.Play( GR_GetAnim( "march" ) );
			break;
		}
	//	LOG << "TASK " << T.type << "|" << T.timeout << "|" << T.target;
		
		if( m_taskTimeout <= 0 || ( T.target - m_position ).Length() < 0.5f )
		{
			m_curTaskID++;
			if( m_curTaskID >= (int) ta->size() )
			{
				m_curTaskID = 0;
				m_curTaskMode = false;
			}
			UpdateTask();
		}
	}
}

void Enemy::Tick( float deltaTime, float blendFactor )
{
	LD32Char::Tick( deltaTime, blendFactor );
}

void Enemy::UpdateTask()
{
	LD32TaskArray* ta = m_curTaskMode ? &m_disturbTasks : &m_patrolTasks;
	if( ta->size() )
	{
		LD32Task& T = (*ta)[ m_curTaskID ];
		m_taskTimeout = T.timeout;
		if( T.type == TT_Turn )
		{
			Vec2 td = ( T.target - m_position ).Normalized();
			m_turnAngleEnd = normalize_angle( atan2( td.y, td.x ) );
			m_turnAngleStart = normalize_angle( m_turnAngle );
			if( fabs( m_turnAngleEnd - m_turnAngleStart ) > M_PI )
				m_turnAngleStart += m_turnAngleEnd > m_turnAngleStart ? M_PI * 2 : -M_PI * 2;
		}
	}
}

void LD32ParseTaskArray( LD32TaskArray& out, sgsVariable var )
{
	ScriptVarIterator it( var );
	while( it.Advance() )
	{
		LD32Task ntask = { TT_Wait, 100.0f, V2(0) };
		
		sgsVariable item = it.GetValue();
		{
			sgsVariable p_type = item.getprop("type");
			if( p_type.not_null() )
				ntask.type = (LD32TaskType) p_type.get<int>();
		}
		{
			sgsVariable p_tgt = item.getprop("target");
			if( p_tgt.not_null() )
				ntask.target = p_tgt.get<Vec2>();
		}
		{
			sgsVariable p_time = item.getprop("timeout");
			if( p_time.not_null() )
				ntask.timeout = p_time.get<float>();
		}
		out.push_back( ntask );
	}
}


#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct LD32Game : IGame
{
	LD32Game() : m_accum( 0.0f ), m_lastFrameReset( false )
	{
	}
	
	void OnConfigure( int argc, char** argv )
	{
		RenderSettings rs = { 0, 1024, 576, 60, FULLSCREEN_NONE, true, ANTIALIAS_NONE, 4 };
		
		InLocalStorage ils( "CrageGames/LD32Game" );
		
		String configdata;
		if( LoadTextFile( "config.cfg", configdata ) )
		{
			StringView key, value;
			ConfigReader cr( configdata );
			while( cr.Read( key, value ) )
			{
				if(0);
				// GRAPHICS
				else if( key == "r_display" ) rs.display = TMAX( 0, TMIN( GR_GetDisplayCount() - 1, (int) String_ParseInt( value ) ) );
				else if( key == "r_width" ) rs.width = TMAX( 1, TMIN( 16384, (int) String_ParseInt( value ) ) );
				else if( key == "r_height" ) rs.height = TMAX( 1, TMIN( 16384, (int) String_ParseInt( value ) ) );
				else if( key == "r_refresh_rate" ) rs.refresh_rate = TMAX( 1, TMIN( 1200, (int) String_ParseInt( value ) ) );
				else if( key == "r_fullscreen" ) rs.fullscreen = TMAX( 0, TMIN( 2, (int) String_ParseInt( value ) ) );
				else if( key == "r_vsync" ) rs.vsync = String_ParseBool( value );
				else if( key == "r_antialias" )
				{
					if( value == "none" );
					else if( value == "msaa2x" )
					{
						rs.aa_mode = ANTIALIAS_MULTISAMPLE;
						rs.aa_quality = 2;
					}
					else if( value == "msaa4x" )
					{
						rs.aa_mode = ANTIALIAS_MULTISAMPLE;
						rs.aa_quality = 4;
					}
					else if( value == "msaa8x" )
					{
						rs.aa_mode = ANTIALIAS_MULTISAMPLE;
						rs.aa_quality = 8;
					}
				}
				// SOUND
				else if( key == "s_vol_master" ) g_s_vol_master = clamp( String_ParseFloat( value ), 0, 1 );
				else if( key == "s_vol_music" ) g_s_vol_music = clamp( String_ParseFloat( value ), 0, 1 );
				else if( key == "s_vol_sfx" ) g_s_vol_sfx = clamp( String_ParseFloat( value ), 0, 1 );
				// INPUT
				else if( key == "i_move_left"       ) g_i_move_left       = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_move_right"      ) g_i_move_right      = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_move_up"         ) g_i_move_up         = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_move_down"       ) g_i_move_down       = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_interact"        ) g_i_interact        = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_crouch"          ) g_i_crouch          = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_mouse_sensitivity" ) g_i_mouse_sensitivity = String_ParseFloat( value );
				else if( key == "i_mouse_invert_x" ) g_i_mouse_invert_x = String_ParseBool( value );
				else if( key == "i_mouse_invert_y" ) g_i_mouse_invert_y = String_ParseBool( value );
			}
		}
		
		// TODO override with command line params
		
		GR_SetVideoMode( rs );
	}
	
	void OnInitialize()
	{
	//	g_SoundSys = SND_CreateSystem();
		
		g_PhyWorld = PHY_CreateWorld();
		g_PhyWorld->SetGravity( V3( 0 ) );//, 0, -9.81f ) );
		
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &INTERACT );
		Game_RegisterAction( &CROUCH );
		
		Game_BindInputToAction( g_i_move_left, &MOVE_LEFT );
		Game_BindInputToAction( g_i_move_right, &MOVE_RIGHT );
		Game_BindInputToAction( g_i_move_up, &MOVE_UP );
		Game_BindInputToAction( g_i_move_down, &MOVE_DOWN );
		Game_BindInputToAction( g_i_interact, &INTERACT );
		Game_BindInputToAction( g_i_crouch, &CROUCH );
		
		GR_LoadAnims( "meshes/charmodel2.ssm.anm" );
		
	//	g_SoundSys->Load( "sound/master.bank" );
	//	g_SoundSys->Load( "sound/master.strings.bank" );
	//	
	//	g_SoundSys->SetVolume( "bus:/", g_s_vol_master );
	//	g_SoundSys->SetVolume( "bus:/music", g_s_vol_music );
	//	g_SoundSys->SetVolume( "bus:/sfx", g_s_vol_sfx );
	//	LOG << g_SoundSys->GetVolume( "bus:/music" );
	//	
	//	m_music = g_SoundSys->CreateEventInstance( "/lev1_music" );
	//	m_music->Start();
		
		g_GameLevel = new GameLevel();
		g_GameLevel->m_scene->skyTexture = GR_GetTexture( "textures/env/back.dds" );
		
	//	Game_AddOverlayScreen( &g_MainMenu );
	//	Game_AddOverlayScreen( &g_SplashScreen );
		
		g_GameLevel->Load( "test1" );
		g_GameLevel->Tick( 0, 0 );
		g_GameLevel->StartLevel();
	}
	void OnDestroy()
	{
		Game_RemoveAllOverlayScreens();
		
		delete g_GameLevel;
		g_GameLevel = NULL;
		
	//	m_music->Stop();
	//	m_music = NULL;
		
		g_PhyWorld = NULL;
	//	g_SoundSys = NULL;
	}
	
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST )
		{
			m_lastFrameReset = false;
			Game_ShowCursor( true );
		}
		if( e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED )
		{
			if( Game_HasOverlayScreens() == false )
				Game_ShowCursor( false );
		}
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
		{
			if( Game_HasOverlayScreens() == false )
	;//			Game_AddOverlayScreen( &g_PauseMenu );
		}
	}
	
	void Game_FixedTick( float dt )
	{
		int ITERS = 10;
		if( !g_GameLevel->m_paused )
		{
			for( int i = 0; i < ITERS; ++i )
				g_PhyWorld->Step( dt / ITERS );
		}
		g_GameLevel->FixedTick( dt );
	}
	void Game_Tick( float dt, float bf )
	{
		g_GameLevel->Tick( dt, bf );
		if( g_GameLevel->m_endFactor >= 0 && !Game_HasOverlayScreens() )
		{
	//		Game_AddOverlayScreen( &g_EndMenu );
			Game_ShowCursor( true );
		}
	}
	void Game_Render()
	{
		g_GameLevel->Draw();
		g_GameLevel->Draw2D();
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
#if 0
			TODO
		g_SoundSys->Update();
#endif
		
		if( g_GameLevel->m_player )
		{
			g_GameLevel->m_player->inCursorMove = V2(0);
			if( Game_HasOverlayScreens() == false )
			{
				Vec2 cpos = Game_GetCursorPos();
				Game_SetCursorPos( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 opos = V2( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 curmove = cpos - opos;
				if( m_lastFrameReset )
					g_GameLevel->m_player->inCursorMove = curmove * V2( g_i_mouse_invert_x ? -1 : 1, g_i_mouse_invert_y ? -1 : 1 ) * g_i_mouse_sensitivity;
				m_lastFrameReset = true;
			}
			else
				m_lastFrameReset = false;
		}
		
		if( dt > MAX_TICK_SIZE )
			dt = MAX_TICK_SIZE;
		
		m_accum += dt;
		while( m_accum >= 0 )
		{
			Game_FixedTick( FIXED_TICK_SIZE );
			m_accum -= FIXED_TICK_SIZE;
		}
		
		Game_Tick( dt, ( m_accum + FIXED_TICK_SIZE ) / FIXED_TICK_SIZE );
		
		Game_Render();
	}
	
	float m_accum;
	bool m_lastFrameReset;
	
	SoundEventInstanceHandle m_music;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

