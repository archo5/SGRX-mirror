

#include "level.hpp"


extern Command MOVE_LEFT;
extern Command MOVE_RIGHT;
extern Command MOVE_UP;
extern Command MOVE_DOWN;
extern Command INTERACT;
extern Command JUMP;
extern Command CROUCH;
extern Command SHOW_OBJECTIVES;


LD33Player::LD33Player( const Vec3& pos, const Vec3& dir ) :
	m_angles( V2( atan2( dir.y, dir.x ), atan2( dir.z, dir.ToVec2().Length() ) ) ),
	m_jumpTimeout(0), m_canJumpTimeout(0), m_footstepTime(0), m_isOnGround(false), m_isCrouching(0),
	m_ivPos( pos ), inCursorMove( V2(0) ),
	m_targetII( NULL ), m_targetTriggered( false )
{
	m_tex_interact_icon = GR_GetTexture( "ui/interact_icon.png" );
	
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 0;
	rbinfo.restitution = 0;
	rbinfo.shape = g_PhyWorld->CreateCylinderShape( V3(0.3f,0.3f,0.3f) );
	rbinfo.mass = 70;
	rbinfo.inertia = V3(0);
	rbinfo.position = pos + V3(0,0,1);
	rbinfo.canSleep = false;
	rbinfo.group = 2;
	m_bodyHandle = g_PhyWorld->CreateRigidBody( rbinfo );
	m_shapeHandle = g_PhyWorld->CreateCylinderShape( V3(0.29f) );
}

void LD33Player::FixedTick( float deltaTime )
{
	SGRX_PhyRaycastInfo rcinfo;
	SGRX_PhyRaycastInfo rcinfo2;
	
	m_jumpTimeout = TMAX( 0.0f, m_jumpTimeout - deltaTime );
	m_canJumpTimeout = TMAX( 0.0f, m_canJumpTimeout - deltaTime );
	
	Vec3 pos = m_bodyHandle->GetPosition();
	m_ivPos.Advance( pos );
	
	m_isCrouching = CROUCH.value;
	if( g_PhyWorld->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,3), 1, 1, &rcinfo ) &&
		g_PhyWorld->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-3), 1, 1, &rcinfo2 ) &&
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
	if( g_PhyWorld->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-ht), 1, 1, &rcinfo )
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
		lvel.z = 4;
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
	
	float curspeed = Vec2Dot( lvel2, md );
	float revmaxfactor = clamp( maxspeed - curspeed, 0, 1 );
	lvel2 += md * accel * revmaxfactor * deltaTime;
	
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
	g_GameLevel->m_infoEmitters.UpdateEmitter( this, D );
}

void LD33Player::Tick( float deltaTime, float blendFactor )
{
	m_angles += inCursorMove * V2(-0.01f);
	m_angles.y = clamp( m_angles.y, (float) -M_PI/2 + SMALL_FLOAT, (float) M_PI/2 - SMALL_FLOAT );
	
	float ch = cos( m_angles.x ), sh = sin( m_angles.x );
	float cv = cos( m_angles.y ), sv = sin( m_angles.y );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
	Vec3 dir = V3( ch * cv, sh * cv, sv );
	
	SGRX_Sound3DAttribs s3dattr = { pos, m_bodyHandle->GetLinearVelocity(), dir, V3(0,0,1) };
	
	m_footstepTime += deltaTime * m_isOnGround * m_bodyHandle->GetLinearVelocity().Length() * ( 0.6f + m_isCrouching * 0.3f );
	if( m_footstepTime >= 1 )
	{
		SoundEventInstanceHandle fsev = g_SoundSys->CreateEventInstance( "/footsteps" );
		fsev->SetVolume( 1.0f - m_isCrouching * 0.3f );
		fsev->Set3DAttribs( s3dattr );
		fsev->Start();
		m_footstepTime = 0;
	}
	
	g_GameLevel->m_scene->camera.znear = 0.1f;
	g_GameLevel->m_scene->camera.angle = 90;
	g_GameLevel->m_scene->camera.direction = dir;
	g_GameLevel->m_scene->camera.position = pos;
	g_GameLevel->m_scene->camera.UpdateMatrices();
	
	g_SoundSys->Set3DAttribs( s3dattr );
	
	m_targetII = g_GameLevel->m_infoEmitters.QueryOneRay( pos, pos + dir, IEST_InteractiveItem );
	if( m_targetII && INTERACT.value && !m_targetTriggered )
	{
		m_targetTriggered = true;
		m_targetII->OnEvent( "trigger_switch" );
	}
	else if( !INTERACT.value )
	{
		m_targetTriggered = false;
	}
}

void LD33Player::DrawUI()
{
	GR2D_GetBatchRenderer().Reset().Col(1);
	GR2D_SetFont( "fonts/lato-regular.ttf", 12 );
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

bool LD33Player::AddItem( const StringView& item, int count )
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

bool LD33Player::HasItem( const StringView& item, int count )
{
	int* ic = m_items.getptr( item );
	return ic && *ic >= count;
}


