

#include "level.hpp"
#include "systems.hpp"
#include "entities.hpp"


GameLevel* g_GameLevel = NULL;
SoundSystemHandle g_SoundSys;

Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );
Command MOVE_UP( "move_up" );
Command MOVE_DOWN( "move_down" );
Command INTERACT( "interact" );
Command SHOOT( "shoot" );
Command JUMP( "jump" );
Command SLOW_WALK( "slow_walk" );
Command SHOW_OBJECTIVES( "show_objectives" );



ActionInput g_i_move_left = ACTINPUT_MAKE_KEY( SDLK_a );
ActionInput g_i_move_right = ACTINPUT_MAKE_KEY( SDLK_d );
ActionInput g_i_move_up = ACTINPUT_MAKE_KEY( SDLK_w );
ActionInput g_i_move_down = ACTINPUT_MAKE_KEY( SDLK_s );
ActionInput g_i_interact = ACTINPUT_MAKE_KEY( SDLK_e );
ActionInput g_i_shoot = ACTINPUT_MAKE_MOUSE( SGRX_MB_LEFT );
ActionInput g_i_jump = ACTINPUT_MAKE_KEY( SDLK_SPACE );
ActionInput g_i_slow_walk = ACTINPUT_MAKE_KEY( SDLK_LSHIFT );
ActionInput g_i_show_objectives = ACTINPUT_MAKE_KEY( SDLK_TAB );

float g_i_mouse_sensitivity = 1.0f;
bool g_i_mouse_invert_x = false;
bool g_i_mouse_invert_y = false;

float g_s_vol_master = 0.8f;
float g_s_vol_sfx = 0.8f;
float g_s_vol_music = 0.8f;



struct MLD62Player : Entity
{
	PhyRigidBodyHandle m_bodyHandle;
	PhyShapeHandle m_shapeHandle;
	
	Vec2 m_angles;
	float m_jumpTimeout;
	float m_canJumpTimeout;
	float m_footstepTime;
	bool m_isOnGround;
	float m_isCrouching;
	
	// camera fix
	float m_bobPower;
	float m_bobTime;
	YawPitch m_weaponTurn;
	
	IVState< Vec3 > m_ivPos;
	
	Vec2 inCursorMove;
	
	Entity* m_targetII;
	bool m_targetTriggered;
	
	TextureHandle m_tex_interact_icon;
	
	float m_shootTimeout;
	AnimCharacter m_weapon;
	ParticleSystem m_shootPS;
	LightHandle m_shootLT;
	
	MLD62Player( GameLevel* lev, const Vec3& pos, const Vec3& dir );
	Mat4 GetBulletOutputMatrix();
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	Vec3 GetPosition(){ return m_bodyHandle->GetPosition(); }
};

MLD62Player::MLD62Player( GameLevel* lev, const Vec3& pos, const Vec3& dir ) :
	Entity( lev ),
	m_angles( V2( atan2( dir.y, dir.x ), atan2( dir.z, dir.ToVec2().Length() ) ) ),
	m_jumpTimeout(0), m_canJumpTimeout(0), m_footstepTime(0), m_isOnGround(false), m_isCrouching(0),
	m_bobPower(0), m_bobTime(0), m_weaponTurn(YP(0)),
	m_ivPos( pos ), inCursorMove( V2(0) ),
	m_targetII( NULL ), m_targetTriggered( false ),
	m_shootTimeout(0), m_weapon( lev->GetScene(), lev->GetPhyWorld() )
{
	m_tex_interact_icon = GR_GetTexture( "ui/interact_icon.png" );
	
	m_weapon.Load( "chars/weapon.chr" );
	m_weapon.m_cachedMeshInst->layers = 0x2;
	
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 0;
	rbinfo.restitution = 0;
	rbinfo.shape = m_level->GetPhyWorld()->CreateCylinderShape( V3(0.3f,0.3f,0.3f) );
	rbinfo.mass = 70;
	rbinfo.inertia = V3(0);
	rbinfo.position = pos + V3(0,0,1);
	rbinfo.canSleep = false;
	rbinfo.group = 2;
	m_bodyHandle = m_level->GetPhyWorld()->CreateRigidBody( rbinfo );
	m_shapeHandle = m_level->GetPhyWorld()->CreateCylinderShape( V3(0.29f) );
	
	m_shootPS.Load( "psys/gunflash.psy" );
	m_shootPS.AddToScene( m_level->GetScene() );
	m_shootPS.OnRenderUpdate();
	m_shootLT = m_level->GetScene()->CreateLight();
	m_shootLT->type = LIGHT_POINT;
	m_shootLT->enabled = false;
	m_shootLT->position = pos;
	m_shootLT->color = V3(0.9f,0.7f,0.5f)*1;
	m_shootLT->range = 4;
	m_shootLT->power = 4;
	m_shootLT->UpdateTransform();
	m_shootTimeout = 0;
}

Mat4 MLD62Player::GetBulletOutputMatrix()
{
	Mat4 out = m_weapon.m_cachedMeshInst->matrix;
	for( size_t i = 0; i < m_weapon.attachments.size(); ++i )
	{
		if( m_weapon.attachments[ i ].name == StringView("barrel") )
		{
			m_weapon.GetAttachmentMatrix( i, out );
			break;
		}
	}
	return out;
}

void MLD62Player::FixedTick( float deltaTime )
{
	SGRX_IPhyWorld* PW = m_level->GetPhyWorld();
	SGRX_PhyRaycastInfo rcinfo;
	SGRX_PhyRaycastInfo rcinfo2;
	
	m_jumpTimeout = TMAX( 0.0f, m_jumpTimeout - deltaTime );
	m_canJumpTimeout = TMAX( 0.0f, m_canJumpTimeout - deltaTime );
	
	Vec3 pos = m_bodyHandle->GetPosition();
	m_ivPos.Advance( pos );
	
	bool slowWalk = SLOW_WALK.value > 0.5f;
	m_isCrouching = 0;
	if( PW->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,3), 1, 1, &rcinfo ) &&
		PW->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-3), 1, 1, &rcinfo2 ) &&
		fabsf( rcinfo.point.z - rcinfo2.point.z ) < 1.8f )
	{
		m_isCrouching = 1;
	}
	
	float cheight = m_isCrouching ? 0.5f : 1.5f;
	float rcxdist = 1.0f;
	Vec3 lvel = m_bodyHandle->GetLinearVelocity();
	float ht = cheight - 0.29f;
	if( lvel.z >= -SMALL_FLOAT && !m_jumpTimeout )
		ht += rcxdist;
	
	bool ground = false;
	if( PW->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-ht), 1, 1, &rcinfo )
		&& fabsf( rcinfo.point.z - pos.z ) < cheight + SMALL_FLOAT )
	{
		Vec3 v = m_bodyHandle->GetPosition();
		float tgt = rcinfo.point.z + cheight;
		v.z += TMIN( deltaTime * 5, fabsf( tgt - v.z ) ) * sign( tgt - v.z );
		m_bodyHandle->SetPosition( v );
		lvel.z = 0;
		ground = true;
		m_canJumpTimeout = 0.5f;
	}
	if( !m_jumpTimeout && m_canJumpTimeout && JUMP.value )
	{
		lvel.z = 5;
		m_jumpTimeout = 0.5f;
		
		SoundEventInstanceHandle fsev = g_SoundSys->CreateEventInstance( "/footsteps" );
		SGRX_Sound3DAttribs s3dattr = { pos, lvel, V3(0), V3(0) };
		fsev->Set3DAttribs( s3dattr );
		fsev->Start();
	}
	
	if( !m_isOnGround && ground )
	{
		m_footstepTime = 1.0f;
	}
	m_isOnGround = ground;
	
	Vec2 realdir = { cos( m_angles.x ), sin( m_angles.x ) };
	Vec2 perpdir = realdir.Perp();
	
	Vec2 md = { MOVE_LEFT.value - MOVE_RIGHT.value, MOVE_DOWN.value - MOVE_UP.value };
	md.Normalize();
	md = -realdir * md.y - perpdir * md.x;
	
	Vec2 lvel2 = lvel.ToVec2();
	
	float maxspeed = 5;
	float accel = ( md.NearZero() && !m_isCrouching ) ? 38 : 30;
	if( m_isCrouching ){ accel = 5; maxspeed = 2.5f; }
	if( !ground ){ accel = 10; }
	if( slowWalk ){ accel *= 0.5f; maxspeed *= 0.5f; }
	
	float curspeed = Vec2Dot( lvel2, md );
	float revmaxfactor = clamp( maxspeed - curspeed, 0, 1 );
	lvel2 += md * accel * revmaxfactor * deltaTime;
	
	///// WALLRUN /////
	if( JUMP.value && ground == false )
	{
		if( PW->ConvexCast( m_shapeHandle, pos, pos + V3( md.x, md.y, 0 ), 1, 1, &rcinfo ) )
		{
			m_bodyHandle->ApplyCentralForce( PFT_Acceleration, V3(0,0,40) );
		}
	}
	
	///// FRICTION /////
	curspeed = Vec2Dot( lvel2, md );
	if( ground )
	{
		if( curspeed > maxspeed )
			curspeed = maxspeed;
	}
	lvel2 -= md * curspeed;
	{
		Vec2 ldd = lvel2.Normalized();
		float llen = lvel2.Length();
		llen = TMAX( 0.0f, llen - deltaTime * ( ground ? 20 : ( m_isCrouching ? 0.5f : 3 ) ) );
		lvel2 = ldd * llen;
	}
	lvel2 += md * curspeed;
	
	// if( llen > 5 )
	// 	llen = 5;
	// else if( !md.Length() )
	// 	llen = TMAX( 0.0f, llen - deltaTime * ( ground ? 20 : 3 ) );
	
//	float accel = ground ? 3 : 1;
//	float dirchange = ground ? 1 : 0.1f;
//	float resist = ground ? 20 : 3;
//	
//	if( ground && lvel2.Length() < 2 )
//		accel *= pow( 10, 1 - lvel2.Length() / 2 ); // startup boost
//	
//	ldd = ( ldd + md * dirchange ).Normalized();
//	if( md.Length() )
//		llen += Vec2Dot( ldd, md ) * TMAX( 0.0f, 5 - llen ) * accel * deltaTime;
//	else
//		llen = TMAX( 0.0f, llen - deltaTime * resist );
	// lvel2 = ldd * llen;
	
//	lvel.x = md.x * 4;
//	lvel.y = md.y * 4;
	lvel.x = lvel2.x;
	lvel.y = lvel2.y;
	
	m_bodyHandle->SetLinearVelocity( lvel );
	
	InfoEmissionSystem::Data D = { pos, 0.5f, IEST_HeatSource | IEST_Player };
	m_level->GetSystem<InfoEmissionSystem>()->UpdateEmitter( this, D );
	
	m_weapon.FixedTick( deltaTime );
}

void MLD62Player::Tick( float deltaTime, float blendFactor )
{
	m_angles += inCursorMove * V2(-0.01f);
	m_angles.y = clamp( m_angles.y, (float) -M_PI/2 + SMALL_FLOAT, (float) M_PI/2 - SMALL_FLOAT );
	
	float ch = cos( m_angles.x ), sh = sin( m_angles.x );
	float cv = cos( m_angles.y ), sv = sin( m_angles.y );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
	Vec3 dir = V3( ch * cv, sh * cv, sv );
	
	SGRX_Sound3DAttribs s3dattr = { pos, m_bodyHandle->GetLinearVelocity(), dir, V3(0,0,1) };
	
	m_footstepTime += deltaTime * m_isOnGround * m_bodyHandle->GetLinearVelocity().Length() * ( 0.6f + m_isCrouching * 0.3f );
	if( m_footstepTime >= 1 && SLOW_WALK.value >= 0.5f )
	{
		SoundEventInstanceHandle fsev = g_SoundSys->CreateEventInstance( "/footsteps" );
		fsev->SetVolume( 1.0f - m_isCrouching * 0.3f );
		fsev->Set3DAttribs( s3dattr );
		fsev->Start();
		m_footstepTime = 0;
		m_level->GetSystem<AIDBSystem>()->AddSound( pos, 10, 0.5f, AIS_Footstep );
	}
	
	///// BOBBING /////
	Vec2 md = { MOVE_LEFT.value - MOVE_RIGHT.value, MOVE_DOWN.value - MOVE_UP.value };
	float bobtgt = m_isOnGround && md.Length() > 0.5f ? 1.0f : 0.0f;
	if( m_isCrouching ) bobtgt *= 0.3f;
	if( SLOW_WALK.value ) bobtgt *= 0.5f;
	float bobdiff = bobtgt - m_bobPower;
	m_bobPower += TMIN( fabsf( bobdiff ), deltaTime * 5 ) * sign( bobdiff );
	m_weaponTurn.yaw += inCursorMove.x * 0.004f;
	m_weaponTurn.pitch += inCursorMove.y * 0.004f;
	m_weaponTurn.TurnTo( YP(0), YawPitchDist(m_weaponTurn,YP(0)).Abs().Scaled(deltaTime*10) );
	
	Vec3 campos = pos;
	campos.z += m_bobPower * 0.1f + fabsf( sinf( m_bobTime * 3 ) ) * 0.1f;
	m_bobTime += deltaTime * m_isOnGround * m_bodyHandle->GetLinearVelocity().Length() * ( 0.6f + m_isCrouching * 0.3f );
	
	m_level->GetScene()->camera.znear = 0.1f;
	m_level->GetScene()->camera.angle = 90;
	m_level->GetScene()->camera.direction = dir;
	m_level->GetScene()->camera.position = campos;
	m_level->GetScene()->camera.UpdateMatrices();
	
	g_SoundSys->Set3DAttribs( s3dattr );
	
	m_targetII = m_level->GetSystem<InfoEmissionSystem>()->QueryOneRay( pos, pos + dir, IEST_InteractiveItem );
	if( m_targetII && INTERACT.value && !m_targetTriggered )
	{
		m_targetTriggered = true;
		m_targetII->OnEvent( "trigger_switch" );
	}
	else if( !INTERACT.value )
	{
		m_targetTriggered = false;
	}
	
	float wpnbob = fabs(sinf(m_bobTime*3))*m_bobPower*0.01f;
	Mat4 mtx = Mat4::Identity;
	mtx = mtx * Mat4::CreateRotationX( -M_PI / 2 );
	mtx = mtx * Mat4::CreateTranslation( -0.3f, -0.6f+wpnbob, +0.5f );
	mtx = mtx * Mat4::CreateScale( V3( 0.2f ) );
	mtx = mtx * Mat4::CreateRotationX( -m_weaponTurn.pitch );
	mtx = mtx * Mat4::CreateRotationY( m_weaponTurn.yaw );
	mtx = mtx * m_level->GetScene()->camera.mInvView;
	
	m_weapon.SetTransform( mtx );
	m_weapon.PreRender( blendFactor );
	m_level->LightMesh( m_weapon.m_cachedMeshInst );
	
	
	m_shootLT->enabled = false;
	if( m_shootTimeout > 0 )
	{
		m_shootTimeout -= deltaTime;
		m_shootLT->enabled = true;
	}
	if( SHOOT.value && m_shootTimeout <= 0 )
	{
		Vec3 aimtgt = campos + dir * 100;
		Mat4 mtx = GetBulletOutputMatrix();
		Vec3 origin = mtx.TransformPos( V3(0) );
		Vec3 dir = ( aimtgt - origin ).Normalized();
		dir = ( dir + V3( randf11(), randf11(), randf11() ) * 0.02f ).Normalized();
		m_level->GetSystem<BulletSystem>()->Add( origin, dir * 100, 1, 1, GAT_Player );
		m_shootPS.SetTransform( mtx );
		m_shootPS.Trigger();
		m_shootLT->position = origin;
		m_shootLT->UpdateTransform();
		m_shootLT->enabled = true;
		m_shootTimeout += 0.1f;
		m_level->GetSystem<AIDBSystem>()->AddSound( pos, 10, 0.2f, AIS_Shot );
	}
	m_shootLT->color = V3(0.9f,0.7f,0.5f)*0.5f * smoothlerp_oneway( m_shootTimeout, 0, 0.1f );
	
	
	m_shootPS.Tick( deltaTime );
	m_shootPS.PreRender();
}

void MLD62Player::DrawUI()
{
	GR2D_GetBatchRenderer().Reset().Col(1);
	GR2D_SetFont( "core", 12 );
	char buf[ 150 ];
	sprintf( buf, "speed: %g", m_bodyHandle->GetLinearVelocity().ToVec2().Length() );
//	GR2D_DrawTextLine( 16, 16, buf );
	
	if( m_targetII )
	{
		float bsz = TMIN( GR_GetWidth(), GR_GetHeight() );
		float x = GR_GetWidth() / 2.0f;
		float y = GR_GetHeight() / 2.0f;
		GR2D_GetBatchRenderer().Reset().Col(1).SetTexture( m_tex_interact_icon ).QuadWH( x, y, bsz / 10, bsz / 10 );
	}
}



struct MLD62_BossEye : Entity
{
	MLD62_BossEye( GameLevel* lev, StringView name, Vec3 pos, Vec3 dir );
	
	void Tick( float deltaTime, float blendFactor );
	
	Vec3 m_position;
	YawPitch m_direction;
	MeshInstHandle m_coreMesh;
	MeshInstHandle m_laserMesh;
	PhyRigidBodyHandle m_body;
};

MLD62_BossEye::MLD62_BossEye( GameLevel* lev, StringView name, Vec3 pos, Vec3 dir ) :
	Entity( lev ),
	m_position( pos ), m_direction( YP( dir ) )
{
	m_coreMesh = lev->GetScene()->CreateMeshInstance();
	m_coreMesh->SetMesh( "meshes/robosaw.ssm" );
	m_laserMesh = lev->GetScene()->CreateMeshInstance();
	m_laserMesh->SetMesh( "meshes/laser.ssm" );
	
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 0;
	rbinfo.restitution = 0;
	rbinfo.shape = lev->GetPhyWorld()->CreateSphereShape( 0.5f );
	rbinfo.mass = 0;
	rbinfo.inertia = V3(0);
	rbinfo.position = pos;
	rbinfo.canSleep = false;
	rbinfo.group = 2;
	m_body = lev->GetPhyWorld()->CreateRigidBody( rbinfo );
}

void MLD62_BossEye::Tick( float deltaTime, float blendFactor )
{
	FlareSystem* FS = m_level->GetSystem<FlareSystem>();
	if( m_level->m_player )
	{
		SGRX_CAST( MLD62Player*, P, m_level->m_player );
		m_direction.TurnTo( YP( ( P->GetPosition() - m_position ).Normalized() ), YP( deltaTime ) );
	}
	
	m_coreMesh->matrix =
		Mat4::CreateRotationY( -m_direction.pitch ) *
		Mat4::CreateRotationZ( m_direction.yaw ) *
		Mat4::CreateTranslation( m_position );
	Mat4 laserMtx =
		Mat4::CreateTranslation( V3(0,0,0.75f) ) *
		Mat4::CreateRotationY( M_PI/2 ) *
		m_coreMesh->matrix;
	
	float dist = 100;
	SceneRaycastInfo rcinfo;
	if( m_level->GetScene()->RaycastOne(
		laserMtx.TransformPos( V3(0,0,0) ),
		laserMtx.TransformPos( V3(0,0,dist) ), &rcinfo, 0x1 ) )
	{
		dist *= rcinfo.factor;
	}
	
	FSFlare laserFlare = { laserMtx.TransformPos( V3(0,0,0) ), V3(2.0f,0.05f,0.01f), 1.0f, true };
	FS->UpdateFlare( this, laserFlare );
	laserFlare.pos = laserMtx.TransformPos( V3(0,0,dist) );
	FS->UpdateFlare( ((char*)this)+1, laserFlare );
	
	m_laserMesh->matrix = Mat4::CreateScale( 0.02f, 0.02f, dist ) * laserMtx;
}



struct MLD62EntityCreationSystem : IGameLevelSystem
{
	enum { e_system_uid = 1000 };
	MLD62EntityCreationSystem( GameLevel* lev );
	virtual bool AddEntity( const StringView& type, sgsVariable data );
	virtual void DrawUI();
};


MLD62EntityCreationSystem::MLD62EntityCreationSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
}

bool MLD62EntityCreationSystem::AddEntity( const StringView& type, sgsVariable data )
{
#ifndef TSGAME_NO_PLAYER
	///////////////////////////
	if( type == "player" )
	{
		MLD62Player* P = new MLD62Player
		(
			m_level,
			data.getprop("position").get<Vec3>(),
			data.getprop("viewdir").get<Vec3>()
		);
		m_level->AddEntity( P );
		m_level->SetPlayer( P );
		return true;
	}
#endif
	
	///////////////////////////
	if( type == "enemy_start" )
	{
		StringView type = data.getprop("type").get<StringView>();
		if( type == "eye" )
		{
			MLD62_BossEye* E = new MLD62_BossEye
			(
				m_level,
				data.getprop("name").get<StringView>(),
				data.getprop("position").get<Vec3>(),
				data.getprop("viewdir").get<Vec3>()
			);
			m_level->AddEntity( E );
			return true;
		}
	}
	return false;
}

void MLD62EntityCreationSystem::DrawUI()
{
#if 0
	SGRX_CAST( MLD62Player*, P, m_level->m_player );
	if( P )
		P->DrawUI();
#endif
}



#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct SciFiBossFightGame : IGame
{
	SciFiBossFightGame() : m_accum( 0.0f ), m_lastFrameReset( false )
	{
	}
	
	bool OnConfigure( int argc, char** argv )
	{
		return true;
	}
	
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		
		g_SoundSys = SND_CreateSystem();
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &INTERACT );
		Game_RegisterAction( &SHOOT );
		Game_RegisterAction( &JUMP );
		Game_RegisterAction( &SLOW_WALK );
		Game_RegisterAction( &SHOW_OBJECTIVES );
		
		Game_BindInputToAction( g_i_move_left, &MOVE_LEFT );
		Game_BindInputToAction( g_i_move_right, &MOVE_RIGHT );
		Game_BindInputToAction( g_i_move_up, &MOVE_UP );
		Game_BindInputToAction( g_i_move_down, &MOVE_DOWN );
		Game_BindInputToAction( g_i_interact, &INTERACT );
		Game_BindInputToAction( g_i_shoot, &SHOOT );
		Game_BindInputToAction( g_i_jump, &JUMP );
		Game_BindInputToAction( g_i_slow_walk, &SLOW_WALK );
		Game_BindInputToAction( g_i_show_objectives, &SHOW_OBJECTIVES );
		
		g_SoundSys->Load( "sound/master.bank" );
		g_SoundSys->Load( "sound/master.strings.bank" );
		
		g_SoundSys->SetVolume( "bus:/", g_s_vol_master );
		g_SoundSys->SetVolume( "bus:/music", g_s_vol_music );
		g_SoundSys->SetVolume( "bus:/sfx", g_s_vol_sfx );
		
		g_GameLevel = new GameLevel( PHY_CreateWorld() );
		g_GameLevel->SetGlobalToSelf();
		g_GameLevel->GetPhyWorld()->SetGravity( V3( 0, 0, -9.81f ) );
		AddSystemToLevel<InfoEmissionSystem>( g_GameLevel );
		AddSystemToLevel<MessagingSystem>( g_GameLevel );
		AddSystemToLevel<ObjectiveSystem>( g_GameLevel );
		AddSystemToLevel<HelpTextSystem>( g_GameLevel );
		AddSystemToLevel<FlareSystem>( g_GameLevel );
		AddSystemToLevel<LevelCoreSystem>( g_GameLevel );
		AddSystemToLevel<ScriptedSequenceSystem>( g_GameLevel );
		AddSystemToLevel<MusicSystem>( g_GameLevel );
		AddSystemToLevel<DamageSystem>( g_GameLevel );
		AddSystemToLevel<BulletSystem>( g_GameLevel );
		AddSystemToLevel<AIDBSystem>( g_GameLevel );
		AddSystemToLevel<StockEntityCreationSystem>( g_GameLevel );
		AddSystemToLevel<MLD62EntityCreationSystem>( g_GameLevel );
		
		HelpTextSystem* HTS = g_GameLevel->GetSystem<HelpTextSystem>();
		HTS->renderer = &htr;
		htr.lineHeightFactor = 1.4f;
		htr.buttonTex = GR_GetTexture( "ui/key.png" );
		htr.centerPos = V2( GR_GetWidth() / 2, GR_GetHeight() * 3 / 4 );
		htr.fontSize = GR_GetHeight() / 20;
		htr.SetNamedFont( "", "core" );
		
		GR2D_SetFont( "core", TMIN(GR_GetWidth(),GR_GetHeight())/20 );
		g_GameLevel->Load( "test" );
		g_GameLevel->Tick( 0, 0 );
		
		g_GameLevel->StartLevel();
		Game_ShowCursor( false );
		
		return true;
	}
	void OnDestroy()
	{
		delete g_GameLevel;
		g_GameLevel = NULL;
		
		htr.buttonTex = NULL;
		
		g_SoundSys = NULL;
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
				;//Game_AddOverlayScreen( &g_PauseMenu );
		}
	}
	void Game_FixedTick( float dt )
	{
		g_GameLevel->FixedTick( dt );
	}
	void Game_Tick( float dt, float bf )
	{
		g_GameLevel->Tick( dt, bf );
	}
	void Game_Render()
	{
		htr.centerPos = V2( GR_GetWidth() / 2, GR_GetHeight() * 3 / 4 );
		htr.fontSize = GR_GetHeight() / 20;
		htr.buttonBorder = GR_GetHeight() / 80;
		
		g_GameLevel->Draw();
		g_GameLevel->Draw2D();
		
		// TEST //
	//	htr.RenderText( "While running along a wall,\npress #a(jump) to do a wallrun" );
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
		g_SoundSys->Update();
		
		if( g_GameLevel->m_player )
		{
			static_cast<MLD62Player*>(g_GameLevel->m_player)->inCursorMove = V2(0);
			if( Game_HasOverlayScreens() == false )
			{
				Vec2 cpos = Game_GetCursorPos();
				Game_SetCursorPos( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 opos = V2( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 curmove = cpos - opos;
				if( m_lastFrameReset )
					static_cast<MLD62Player*>(g_GameLevel->m_player)->inCursorMove = curmove * V2( g_i_mouse_invert_x ? -1 : 1, g_i_mouse_invert_y ? -1 : 1 ) * g_i_mouse_sensitivity;
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
	
	void OnDrawSceneGeom( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info,
		TextureHandle rtt, DepthStencilSurfHandle dss )
	{
		IGame::OnDrawSceneGeom( ctrl, info, rtt, dss );
		
		if( g_GameLevel->m_player )
		{
			BatchRenderer& br = GR2D_GetBatchRenderer();
			SGRX_Scene* scene = info.scene;
			int W = GR_GetWidth();
			int H = GR_GetHeight();
			float avgw = ( W + H ) * 0.5f;
			int minw = TMIN( W, H );
			
			DepthStencilSurfHandle dssFPV;
			TextureHandle rttGUI;
			dssFPV = GR_GetDepthStencilSurface( W, H, RT_FORMAT_COLOR_HDR16, 0xff00 );
			rttGUI = GR_GetRenderTarget( W, H, RT_FORMAT_COLOR_LDR8, 0xff01 );
			GR_PreserveResource( dssFPV );
			GR_PreserveResource( rttGUI );
			
			// GUI
			ctrl->SetRenderTargets( NULL, SGRX_RT_ClearColor, 0, 0, 1, rttGUI );
			br.Reset();
			GR2D_SetFont( "core", minw / 24 );
			GR2D_DrawTextLine( minw / 10, minw / 10, "HEALTH: ||||||||||||||||||||" );
			TextureHandle crosshairTex = GR_GetTexture( "ui/crosshair.png" );
			GR_PreserveResource( crosshairTex );
			br.Reset().Col(0.5f,0.01f,0,1).SetTexture( crosshairTex ).Box( W/2, H/2, minw/18, minw/18 );
			br.Reset().SetTexture( crosshairTex ).Box( W/2, H/2, minw/20, minw/20 );
			br.Flush();
			
			// WEAPON
			ctrl->SetRenderTargets( dssFPV, SGRX_RT_ClearDepth | SGRX_RT_ClearStencil, 0, 0, 1, rtt );
			if( info.viewport )
				GR2D_SetViewport( info.viewport->x0, info.viewport->y0, info.viewport->x1, info.viewport->y1 );
			
			SGRX_Camera cambk = scene->camera;
			scene->camera.zfar = 5;
			scene->camera.znear = 0.01f;
			scene->camera.UpdateMatrices();
			
			SGRX_CAST( MLD62Player*, player, g_GameLevel->m_player );
			SGRX_MeshInstance* minsts[] = { player->m_weapon.m_cachedMeshInst };
			size_t micount = 1;
			
			ctrl->RenderMeshes( scene, 1, 1, SGRX_TY_Solid, minsts, micount );
			ctrl->RenderMeshes( scene, 3, 4, SGRX_TY_Solid, minsts, micount );
			ctrl->RenderMeshes( scene, 1, 1, SGRX_TY_Decal, minsts, micount );
			ctrl->RenderMeshes( scene, 3, 4, SGRX_TY_Decal, minsts, micount );
			ctrl->RenderMeshes( scene, 1, 1, SGRX_TY_Transparent, minsts, micount );
			ctrl->RenderMeshes( scene, 3, 4, SGRX_TY_Transparent, minsts, micount );
			
			// GUI overlay
			Mat4 backup = br.viewMatrix;
			Mat4 guimtx = Mat4::Identity;
			float turnQ = 0.1f;
			guimtx = guimtx * Mat4::CreateRotationX( -player->m_weaponTurn.pitch*turnQ );
			guimtx = guimtx * Mat4::CreateRotationY( player->m_weaponTurn.yaw*turnQ );
			guimtx = guimtx * scene->camera.mProj;
			GR2D_SetViewMatrix( guimtx );
			br.Reset().SetTexture( rttGUI ).Quad( W/avgw, H/avgw, -W/avgw, -H/avgw, 1 ).Flush();
			GR2D_SetViewMatrix( backup );
			
			scene->camera = cambk;
		}
	}
	
	SGRX_HelpTextRenderer htr;
	float m_accum;
	bool m_lastFrameReset;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

