

#include "entities_ts.hpp"


CVarBool gcv_notarget( "notarget", false );
CVarBool gcv_dbg_aiproc( "dbg_aiproc", false );
CVarBool gcv_dbg_aifacts( "dbg_aifacts", false );
CVarBool gcv_dbg_aistack( "dbg_aistack", false );
CVarBool gcv_dbg_aipaths( "dbg_aipaths", false );

void register_tsent_cvars()
{
	REGCOBJ( gcv_notarget );
	REGCOBJ( gcv_dbg_aiproc );
	REGCOBJ( gcv_dbg_aifacts );
	REGCOBJ( gcv_dbg_aistack );
	REGCOBJ( gcv_dbg_aipaths );
}



extern Vec2 CURSOR_POS;
extern InputState MOVE_LEFT;
extern InputState MOVE_RIGHT;
extern InputState MOVE_UP;
extern InputState MOVE_DOWN;
extern InputState MOVE_X;
extern InputState MOVE_Y;
extern InputState AIM_X;
extern InputState AIM_Y;
extern InputState WP_SHOOT;
extern InputState WP_LOCK_ON;
extern InputState WP_REMOVE_LOCK_ON;
extern InputState WP_RELOAD;
extern InputState WP_DROP;
extern InputState WP_HOLSTER;
extern InputState CROUCH;
extern InputState JUMP;
extern InputState DO_ACTION;


#define MAGNUM_CAMERA_NOTICE_TIME 2
#define MAGNUM_CAMERA_ALERT_TIME 5
#define MAGNUM_CAMERA_VIEW_DIST 6.0f


#if 0
TSCamera::TSCamera(
	GameLevel* lev,
	const StringView& name,
	const StringView& charname,
	const Vec3& pos,
	const Quat& rot,
	const Vec3& scl,
	const Vec3& dir0,
	const Vec3& dir1
) :
	Entity( lev ),
	m_animChar( lev->GetScene(), lev->GetPhyWorld() ),
	m_playerVisible( false ), m_lastSeenPlayerDir( YP(0) ),
	m_curDir( YP( dir0 ) ), m_timeout( 0 ), m_state( 0 ),
	m_alertTimeout( 0 ), m_noticeTimeout( 0 ),
	m_dir0( YP( dir0 ) ), m_dir1( YP( dir1 ) ),
	m_moveTime( 3.0f ), m_pauseTime( 2.0f ),
	m_fov( 30.0f )
{
	SetID( name );
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "chars/%s.chr", StackString<200>( charname ).str );
	
	m_anLayers[0].anim = &m_animChar.m_layerAnimator;
	m_anLayers[0].tflags = AnimMixer::TF_Absolute_Rot | AnimMixer::TF_Additive;
	m_animChar.m_anMixer.layers = m_anLayers;
	m_animChar.m_anMixer.layerCount = 1;
	m_animChar.Load( bfr );
	StringView atchlist[] = { "view", "origin", "light" };
	m_animChar.SortEnsureAttachments( atchlist, 3 );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->layers = 0x2;
	MI->matrix = Mat4::CreateSRT( scl, rot, pos );
	m_level->LightMesh( MI );
}

void TSCamera::FixedUpdate()
{
	float deltaTime = m_level->GetDeltaTime();
	
	m_playerVisible = false;
#if 0
	if( m_level->m_player )
	{
		Vec3 ppos = V3(0);
#ifdef TSGAME
		ppos = static_cast<TSCharacter*>(m_level->m_player)->GetWorldPosition();
#endif
		Mat4 viewmtx, originmtx, invviewmtx = Mat4::Identity, invoriginmtx = Mat4::Identity;
		m_animChar.GetAttachmentMatrix( 0, viewmtx );
		m_animChar.GetAttachmentMatrix( 1, originmtx );
		viewmtx.InvertTo( invviewmtx );
		originmtx.InvertTo( invoriginmtx );
		
		Vec3 viewpos = invviewmtx.TransformPos( ppos );
		Vec3 viewdir = viewpos.Normalized();
		float viewangle = acosf( clamp( Vec3Dot( viewdir, V3(1,0,0) ), -1, 1 ) );
		if( viewpos.Length() < MAGNUM_CAMERA_VIEW_DIST && viewangle < DEG2RAD( m_fov ) )
		{
			Vec3 viewpos = viewmtx.TransformPos( V3(0) );
			bool isect = m_level->GetPhyWorld()->Raycast( viewpos, ppos, 1, 1 );
			if( isect == false )
			{
				Vec3 origindir = invoriginmtx.TransformPos( ppos ).Normalized();
				m_playerVisible = true;
				m_lastSeenPlayerDir = YP( origindir );
			}
		}
	}
#endif
	
	if( m_alertTimeout > 0 )
	{
		if( m_playerVisible )
			m_alertTimeout = MAGNUM_CAMERA_ALERT_TIME;
		else
			m_alertTimeout -= deltaTime;
		m_curDir.TurnTo( m_lastSeenPlayerDir, YP(deltaTime * 2) );
	}
	else if( m_playerVisible )
	{
		m_curDir.TurnTo( m_lastSeenPlayerDir, YP(deltaTime) );
		m_noticeTimeout += deltaTime;
		if( m_noticeTimeout > MAGNUM_CAMERA_NOTICE_TIME )
		{
			m_noticeTimeout = 0;
			m_alertTimeout = MAGNUM_CAMERA_ALERT_TIME;
		}
	}
	else if( m_noticeTimeout > 0 )
	{
		m_curDir.TurnTo( m_lastSeenPlayerDir, YP(deltaTime) );
		m_noticeTimeout -= deltaTime;
	}
	else
	{
		switch( m_state )
		{
		case 0:
		case 2: // move
		{
			YawPitch tgt = m_state == 2 ? m_dir1 : m_dir0;
			YawPitch dst = YawPitchDist( m_dir0, m_dir1 ).Abs();
			float fdst = TMAX( dst.yaw, dst.pitch ) * safe_fdiv( deltaTime, m_moveTime );
			m_curDir.TurnTo( tgt, YP( fdst ) );
			if( YawPitchAlmostEqual( m_curDir, tgt ) )
			{
				m_state = ( m_state + 1 ) % 4;
				m_timeout = m_pauseTime;
			}
			break;
		}
		case 1:
		case 3: // wait
			m_timeout -= deltaTime;
			if( m_timeout <= 0 )
			{
				m_state = ( m_state + 1 ) % 4;
			}
			break;
		}
	}
	
	float f_turn_h = clamp( normalize_angle2( m_curDir.yaw ), -FLT_PI * 0.4f, FLT_PI * 0.4f ) / FLT_PI;
	float f_turn_v = clamp( normalize_angle2( m_curDir.pitch ), -FLT_PI * 0.25f, FLT_PI * 0.25f ) / FLT_PI;
	for( size_t i = 0; i < m_animChar.layers.size(); ++i )
	{
		AnimCharacter::Layer& L = m_animChar.layers[ i ];
		if( L.name == StringView("turn_h") )
			L.amount = f_turn_h;
		else if( L.name == StringView("turn_v") )
			L.amount = f_turn_v;
	}
	m_animChar.RecalcLayerState();
	m_animChar.FixedTick( deltaTime );
}

void TSCamera::Update()
{
	m_animChar.PreRender( m_level->GetBlendFactor() );
	Mat4 mtx;
	m_animChar.GetAttachmentMatrix( 2, mtx );
	
	Vec3 color = V3(0,1,0);
	if( m_alertTimeout > 0 )
	{
		color = V3(1,0,0);
		if( m_playerVisible == false && fmodf( m_alertTimeout, 0.4f ) < 0.2f )
			color = V3(0);
	}
	else if( m_noticeTimeout > 0 )
		color = V3(1,1,0);
	FSFlare FD = { mtx.TransformPos( V3(0) ), color, 1, true };
	
	m_level->GetSystem<FlareSystem>()->UpdateFlare( this, FD );
	
	Vec3 tgtpos = m_animChar.GetAttachmentPos( m_animChar.FindAttachment( "origin" ) );
	m_obj->SetInfoMask( IEST_Target );
	m_obj->SetInfoTarget( tgtpos );
	
	LevelMapSystem* lms = m_level->GetSystem<LevelMapSystem>();
	if( lms )
	{
		MapItemInfo mii = {0};
		
		Mat4 mtx;
		m_animChar.GetAttachmentMatrix( 0, mtx );
		
		mii.type = MI_Object_Camera;
		if( m_alertTimeout > 0 ) mii.type |= MI_State_Alerted;
		else if( m_noticeTimeout > 0 ) mii.type |= MI_State_Suspicious;
		else mii.type |= MI_State_Normal;
		
		mii.position = mtx.TransformPos( V3(0) );
		mii.direction = mtx.TransformNormal( V3(1,0,0) );
		mii.sizeFwd = 10;
		mii.sizeRight = 6;
		
		lms->UpdateItem( this, mii );
	}
}
#endif



TSCharacter::TSCharacter( GameObject* obj ) :
	GOBehavior( obj ),
	m_animChar( obj->m_level->GetScene(), obj->m_level->GetPhyWorld() ),
	m_health( 100 ), m_armor( 0 ),
	m_damageMultiplier( 1 ), m_acceptsCriticalDamage( true ),
	m_footstepTime(0), m_isCrouching(false), m_isOnGround(false),
	m_jumpTimeout(0), m_canJumpTimeout(0), m_cachedBodyExtOffset(V2(0)),
	m_groundBody(NULL), m_groundLocalPos(V3(0)), m_groundWorldPos(V3(0)),
	m_ivPos( V3(0) ), m_ivAimDir( V3(1,0,0) ),
	m_turnAngle( 0 ),
	m_aimDir( YP(V3(1,0,0)) ), m_aimDist( 1 ),
	m_infoFlags( IEST_HeatSource | IEST_Target ), m_group( 0 ),
	m_pickupTrigger( false ),
	m_skipTransformUpdate( false )
{
	typeOverride = "*human*";
	ownerID = m_obj;
	
	PhyShapeHandle initShape = m_level->GetPhyWorld()->CreateCylinderShape( V3(0.3f,0.3f,0.5f) );
	
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 0;
	rbinfo.restitution = 0;
	rbinfo.shape = initShape;
	rbinfo.mass = 70;
	rbinfo.inertia = V3(0);
	rbinfo.position = V3(0) + V3(0,0,1);
	rbinfo.canSleep = false;
	rbinfo.group = 2;
	rbinfo.mask = 1;//|2;
	rbinfo.ccdSweptSphereRadius = 0.3f;
	rbinfo.ccdMotionThreshold = 0.001f;
	m_bodyHandle = m_level->GetPhyWorld()->CreateRigidBody( rbinfo );
	m_shapeHandle = m_level->GetPhyWorld()->CreateCylinderShape( V3(0.25f) );
	
	m_shadowInst = m_level->GetScene()->CreateLight();
	m_shadowInst->type = LIGHT_PROJ;
	m_shadowInst->direction = V3(0,0,-1);
	m_shadowInst->updir = V3(0,1,0);
	m_shadowInst->angle = 60;
	m_shadowInst->range = 1.5f;
	m_shadowInst->UpdateTransform();
	m_shadowInst->projectionMaterial.textures[0] = GR_GetTexture( "textures/fx/blobshadow.png" );//GR_GetTexture( "textures/unit.png" );
	m_shadowInst->projectionMaterial.textures[1] = GR_GetTexture( "textures/fx/projfalloff2.png" );
	m_shadowInst->enabled = false;//true;
	
#if 0
	m_shootPS.Load( "psys/gunflash.psy" );
	m_shootPS.AddToScene( m_level->GetScene() );
	m_shootPS.OnRenderUpdate();
	m_shootLT = m_level->GetScene()->CreateLight();
	m_shootLT->type = LIGHT_POINT;
	m_shootLT->enabled = false;
	m_shootLT->position = V3(0);
	m_shootLT->color = V3(0.9f,0.7f,0.5f)*1;
	m_shootLT->range = 4;
	m_shootLT->power = 4;
	m_shootLT->UpdateTransform();
	m_shootTimeout = 0;
#endif
	m_timeSinceLastHit = 9999;
}

TSCharacter::~TSCharacter()
{
}

void TSCharacter::OnTransformUpdate()
{
	if( m_skipTransformUpdate )
		return;
	Vec3 pos = m_obj->GetWorldPosition();
	m_bodyHandle->SetPosition( pos );
	m_ivPos.Set( pos );
}

void TSCharacter::InitializeMesh( const StringView& path )
{
	m_animChar.SetAnimChar( path );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->userData = (SGRX_MeshInstUserData*) this;
	MI->layers = 0x2;
	MI->matrix = Mat4::CreateSRT( V3(1), Quat::Identity, m_ivPos.curr );
	m_level->LightMesh( MI, V3(1) );
	
	ProcessAnims( 0 );
}

void TSCharacter::ProcessAnims( float deltaTime )
{
	IController* ctrl = GetObjectController( m_obj );
	
	// turning
	Vec2 rundir = V2( cosf( m_turnAngle ), sinf( m_turnAngle ) );
	Vec3 aimdir = V3( rundir.x, rundir.y, 0 );
	float aimspeed = ctrl->GetInputV2( ACT_Chr_AimAt ).y;
	if( ctrl->GetInputB( ACT_Chr_AimAt ) )
	{
		aimdir = ctrl->GetInputV3( ACT_Chr_AimTarget ) - GetQueryPosition_FT();
	}
	if( IsAlive() )
	{
		m_aimDir.TurnTo( YP( aimdir ), YP( aimspeed * deltaTime ) );
	}
	else
		m_aimDir.TurnTo( YP( V3(rundir,0) ), YP( deltaTime ) );
	m_aimDist = aimdir.Length();
	aimdir = m_aimDir.ToVec3();
	m_ivAimDir.Advance( V3( aimdir.x, aimdir.y, 0 ) );
	
	Vec2 totalShape2Offset = rundir.Normalized() * 0.2f + aimdir.ToVec2().Normalized() * 0.4f;
	m_cachedBodyExtOffset = totalShape2Offset;
	
	static const float hh1 = 0.5f;
	static const float hh2 = 0.4f;
	static const float rr1 = 0.3f;
	static const float rr2 = 0.8f;
	Vec3 points[ 11 ] =
	{
		V3( 0, 0, -hh1 ),
		V3( 0, 0, hh1 ),
		V3( -rr1, 0, 0 ),
		V3( rr1, 0, 0 ),
		V3( 0, -rr1, 0 ),
		V3( 0, rr1, 0 ),
		V3( -rr1, 0, -hh2 ),
		V3( rr1, 0, -hh2 ),
		V3( 0, -rr1, -hh2 ),
		V3( 0, rr1, -hh2 ),
		V3( totalShape2Offset.Normalized() * rr2, 0 ),
	};
	m_bodyHandle->SetShape( m_level->GetPhyWorld()->CreateConvexHullShape(
		points, SGRX_ARRAY_SIZE( points ) ) );
	m_bodyHandle->FlushContacts();
	
	// committing to animator
	if( Vec2Dot( rundir, aimdir.ToVec2() ) < -0.1f )
		TurnTo( aimdir.ToVec2(), 8 * deltaTime );
	
	float f_turn_btm = ( atan2( rundir.y, rundir.x ) - M_PI / 2 ) / ( M_PI * 2 );
	float f_turn_top = ( atan2( aimdir.y, aimdir.x ) - M_PI / 2 ) / ( M_PI * 2 );
	m_animChar.SetFloat( "btm_angle", f_turn_btm );
	m_animChar.SetFloat( "top_angle", f_turn_top );
}

void TSCharacter::FixedUpdate()
{
	float deltaTime = m_level->GetDeltaTime();
	IController* ctrl = GetObjectController( m_obj );
	
	
	Vec2 i_move = ctrl->GetInputV2( ACT_Chr_Move );
	float i_speed = ctrl->GetInputV3( ACT_Chr_Move ).z;
	
#if 0 // ---------- TODO ---- W A L K ----------
	// animate character
	const char* anim_stand = m_isCrouching ? "crouch" : "stand_with_pistol_up";
	const char* anim_walk_fw = m_isCrouching ? "crouch_walk" : "walk";
	const char* anim_walk_bw = m_isCrouching ? "crouch_walk_bw" : "walk_bw";
	const char* anim_run_fw = m_isCrouching ? "crouch_walk" : "run";
	const char* anim_run_bw = m_isCrouching ? "crouch_walk_bw" : "run_bw";
	
	if( i_speed < 0.5f )
	{
		anim_run_fw = anim_walk_fw;
		anim_run_bw = anim_walk_bw;
	}
#endif
	
	Vec2 md = i_move.Normalized();
	float fwdq = i_speed;
	if( Vec2Dot( md, GetAimDir_FT().ToVec2() ) < -0.2f )
	{
		md = -md;
		fwdq *= -1;
	}
	
	m_animChar.SetFloat( "health", m_health );
	m_animChar.SetFloat( "time_since_last_hit", m_timeSinceLastHit );
	
	m_animChar.SetFloat( "run", clamp( i_move.Length(), 0, 1 ) * fwdq );
	m_animChar.SetBool( "crouch", m_isCrouching );
	GOBehavior* wpn = FindWeapon();
	m_animChar.SetFloat( "aim", wpn ? (
		!wpn->GetScriptedObject().getprop("holstered").get<bool>()) : 0 );
	
	
	HandleMovementPhysics( deltaTime );
	
	sgsVariable fn_afterfixedpos = GetScriptedObject()[ "AfterFixedPos" ];
	if( fn_afterfixedpos.not_null() )
	{
		GetScriptedObject().thiscall( C, fn_afterfixedpos );
	}
	
	ProcessAnims( deltaTime );
	
	if( IsAlive() && m_animChar.CheckMarker( "step" ) )
	{
		Vec3 pos = m_bodyHandle->GetPosition();
		Vec3 lvel = m_bodyHandle->GetLinearVelocity();
		SoundEventInstanceHandle fsev = m_level->GetSoundSys()->CreateEventInstance( "/footsteps" );
		SGRX_Sound3DAttribs s3dattr = { pos, lvel, V3(0), V3(0) };
		fsev->Set3DAttribs( s3dattr );
		fsev->Start();
		m_level->GetSystem<AIDBSystem>()->AddSound( m_obj->GetWorldPosition(), 4, 0.2f, AIS_Footstep );
	}
	
	m_animChar.FixedTick( deltaTime );
	// reset flags
	{
		if( m_pickupTrigger )
		{
			m_pickupTrigger = false;
			m_animChar.SetBool( "pickup", false );
		}
	}
	m_timeSinceLastHit += deltaTime;
	
	if( ctrl->GetInputB( ACT_Chr_DoAction ) )
	{
		BeginClosestAction( 2 );
	}
}


struct CharMoveAwayProc : GameObjectProcessor
{
	CharMoveAwayProc( TSCharacter* c, const Vec3& p ) : self(c), pos(p), force(V3(0)){}
	bool ProcessGameObject( GameObject* obj )
	{
		static const float character_radius = 0.4f;
		
		TSCharacter* chr = obj->FindFirstBehaviorOfType<TSCharacter>();
		if( !chr || !chr->IsAlive() || chr == self )
			return false;
		
		Vec3 ocp = chr->m_ivPos.Get( chr->m_level->GetBlendFactor() );
		Vec3 dir = pos - ocp;
		float dist = dir.Length();
		if( dist > 0.0001f && dist < character_radius * 2 )
		{
			float diff = character_radius * 2 - dist;
			force += dir * ( diff * 0.5f / dist );
		}
		return false;
	}
	Vec3 GetForce() const { return force; }
	TSCharacter* self;
	Vec3 pos;
	Vec3 force;
};

void TSCharacter::Update()
{
	float deltaTime = m_level->GetDeltaTime();
	float blendFactor = m_level->GetBlendFactor();
	IController* ctrl = GetObjectController( m_obj );
	
	Vec3 turn = ctrl->GetInputV3( ACT_Chr_Turn );
	if( turn.z > 0 )
		TurnTo( turn.ToVec2(), turn.z * deltaTime );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
	if( IsAlive() )
	{
		CharMoveAwayProc cmap( this, pos );
		m_level->QuerySphere( &cmap, IEST_Target, GetQueryPosition_FT(), 2.0f );
		pos += cmap.GetForce();
		m_bodyHandle->ApplyCentralForce( PFT_Velocity, cmap.GetForce() * deltaTime );
	}
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->matrix = Mat4::CreateTranslation( pos ); // Mat4::CreateSRT( V3(1), rdir, pos );
	m_shadowInst->position = pos + V3(0,0,1);
	
	m_level->LightMesh( MI, V3(0,0,m_isCrouching ? 0.6f : 1) );
	
	m_animChar.PreRender( blendFactor );
	// update position
	{
		TempSwapper<const void*> ts( m_obj->_xfChangeInvoker, this );
		m_obj->SetWorldPosition( pos );
	}
	m_interpAimDir = m_ivAimDir.Get( blendFactor );
	
	
	if( m_health > 0 )
	{
		Vec3 tgtpos = m_animChar.GetLocalAttachmentPos( m_animChar.FindAttachment( "target" ) );
		m_obj->SetInfoMask( m_infoFlags );
		m_obj->SetInfoTarget( tgtpos );
	}
	
	
	AnimDeformer& AnD = m_animChar.m_anDeformer;
	for( size_t i = 0; i < AnD.forces.size(); ++i )
	{
		AnimDeformer::Force& F = AnD.forces[ i ];
		float t = F.lifetime;
		F.amount = sinf( t * FLT_PI ) * expf( -t * 8 ) * 7 * F.dir.Length();
		if( F.lifetime > 3 )
		{
			AnD.forces.erase( i-- );
		}
	}
	
	
	GOBehavior* wpn = FindWeapon();
	if( wpn )
	{
		// RELOAD
		if( ctrl->GetInputB1( ACT_Chr_ReloadHolsterDrop ) && IsAlive() )
		{
			wpn->SendMessage( "Reload" );
		}
		// [UN]HOLSTER
		if( ctrl->GetInputB2( ACT_Chr_ReloadHolsterDrop ) && IsAlive() )
		{
			wpn->SendMessage( "HolsterToggle" );
		}
		// DROP WEAPON
		if( ctrl->GetInputB3( ACT_Chr_ReloadHolsterDrop ) && IsAlive() )
		{
			wpn->SendMessage( "DropWeapon" );
		}
		// SHOOT
		if( ctrl->GetInputB( ACT_Chr_Shoot ) && IsAlive() )
		{
			Vec3 aimtgt = ctrl->GetInputV3( ACT_Chr_AimTarget );
			if( !ctrl->GetInputB( ACT_Chr_AimAt ) )
			{
				aimtgt = GetQueryPosition() + GetAimDir() * 1000;
			}
			wpn->SendMessage( "SetShootTarget", m_level->GetScriptCtx().CreateVec3( aimtgt ) );
		}
		else
			wpn->SendMessage( "SetShootTarget", sgsVariable() );
	}
	
	
#if 0
	m_shootLT->enabled = false;
	if( m_shootTimeout > 0 )
	{
		m_shootTimeout -= deltaTime;
		m_shootLT->enabled = true;
	}
	if( ctrl->GetInputB( ACT_Chr_Shoot ) && IsAlive() && m_shootTimeout <= 0 )
	{
		Mat4 mtx = GetBulletOutputMatrix();
		Vec3 origin = mtx.TransformPos( V3(0) );
		Vec3 dir = ( ctrl->GetInputV3( ACT_Chr_AimTarget ) - origin ).Normalized();
		dir = ( dir + V3( randf11(), randf11(), randf11() ) * 0.02f ).Normalized();
		m_level->GetSystem<BulletSystem>()->Add( origin, dir * 100, 1, 1, ownerType );
		m_shootPS.SetTransform( mtx );
		m_shootPS.Trigger();
		m_shootLT->position = origin;
		m_shootLT->UpdateTransform();
		m_shootLT->enabled = true;
		m_shootTimeout += 0.1f;
		m_level->GetSystem<AIDBSystem>()->AddSound( m_obj->GetWorldPosition(), 10, 0.2f, AIS_Shot );
		
		m_level->PlaySound( "/mp5_shot", origin, dir );
		
		Mat4 inv = m_animChar.m_cachedMeshInst->matrix.Inverted();
		Vec3 forcePos = inv.TransformPos( origin );
		Vec3 forceDir = inv.TransformNormal( mtx.TransformNormal(V3(0,0,-1)) ).Normalized();
		AnD.AddLocalForce( forcePos, forceDir * 0.2f, 1.0f );
	}
	m_shootLT->color = V3(0.9f,0.7f,0.5f)*0.5f * smoothlerp_oneway( m_shootTimeout, 0, 0.1f );
	
	m_shootPS.Tick( deltaTime );
#endif
}

void TSCharacter::_HandleGroundBody( Vec3& pos, SGRX_IPhyRigidBody* body, float dt )
{
	ASSERT( body );
	Mat4 bodyWorldMatrix = Mat4::CreateSRT( V3(1), body->GetRotation(), body->GetPosition() );
	if( body == m_groundBody )
	{
		Vec3 newWorldPos = bodyWorldMatrix.TransformPos( m_groundLocalPos );
		pos += newWorldPos - m_groundWorldPos;
		m_groundVelocity = newWorldPos - m_groundWorldPos;
		if( dt > 0 )
			m_groundVelocity /= dt;
		m_groundWorldPos = newWorldPos;
	}
	else
	{
		m_groundBody = body;
		Mat4 invBodyWorldMatrix = bodyWorldMatrix.Inverted();
		m_groundWorldPos = pos;
		m_groundLocalPos = invBodyWorldMatrix.TransformPos( pos );
		m_groundVelocity = V3(0);
	}
}

void TSCharacter::HandleMovementPhysics( float deltaTime )
{
	IController* ctrl = GetObjectController( m_obj );
	
	// disabled features
	bool jump = ctrl->GetInputB( ACT_Chr_Jump );
//	float m_jumpTimeout = 1, m_canJumpTimeout = 1;
	
	SGRX_PhyRaycastInfo rcinfo;
	SGRX_PhyRaycastInfo rcinfo2;
	
	m_jumpTimeout = TMAX( 0.0f, m_jumpTimeout - deltaTime );
	m_canJumpTimeout = TMAX( 0.0f, m_canJumpTimeout - deltaTime );
	
	Vec3 pos = m_bodyHandle->GetPosition();
	
	bool prevCrouch = m_isCrouching;
	m_isCrouching = ctrl->GetInputB( ACT_Chr_Crouch );
	if( m_level->GetPhyWorld()->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,3), 1, 1, &rcinfo ) &&
		m_level->GetPhyWorld()->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-3), 1, 1, &rcinfo2 ) &&
		fabsf( rcinfo.point.z - rcinfo2.point.z ) < 1.8f )
	{
		m_isCrouching = true;
	}
	
	if( prevCrouch != m_isCrouching )
	{
		float diff = prevCrouch - m_isCrouching;
		Vec3 chg = V3( 0, 0, 0.7f * diff );
		pos += chg;
		m_bodyHandle->SetPosition( pos );
	}
	
	float cheight = m_isCrouching ? 0.6f : 1.3f;
	float rcxdist = 1.0f;
	Vec3 lvel = m_bodyHandle->GetLinearVelocity();
	float ht = cheight - 0.25f;
//	if( lvel.z >= -SMALL_FLOAT && !m_jumpTimeout )
		ht += rcxdist;
	
	m_ivPos.Advance( pos + V3(0,0,-cheight) );
	
	bool ground = false;
	if( m_level->GetPhyWorld()->ConvexCast( m_shapeHandle,
		pos + V3(0,0,0), pos + V3(0,0,-ht), 1, 1, &rcinfo )
		&& ( fabsf( rcinfo.point.z - pos.z ) < cheight || ( lvel.z < 0 && m_isOnGround ) ) )
	{
		Vec3 v = m_bodyHandle->GetPosition();
		float tgt = rcinfo.point.z + cheight;
		v.z += TMIN( deltaTime * 5, fabsf( tgt - v.z ) ) * sign( tgt - v.z );
		_HandleGroundBody( v, rcinfo.body, deltaTime );
		m_bodyHandle->SetPosition( v );
		lvel.z = 0;
		ground = true;
		m_canJumpTimeout = 0.5f;
	}
	else
	{
		m_groundBody = NULL;
	}
	
	m_animChar.SetBool( "jump", false );
	if( !m_jumpTimeout && m_canJumpTimeout && jump )
	{
		lvel.z = 2.5f; // 4;
		lvel += (lvel * V3(1,1,0)).Normalized() * 3;
		lvel += m_groundVelocity;
		m_jumpTimeout = 0.5f;
		
		SoundEventInstanceHandle fsev = m_level->GetSoundSys()->CreateEventInstance( "/footsteps" );
		SGRX_Sound3DAttribs s3dattr = { pos, lvel, V3(0), V3(0) };
		fsev->Set3DAttribs( s3dattr );
		fsev->Start();
		m_level->GetSystem<AIDBSystem>()->AddSound( m_obj->GetWorldPosition(), 4, 0.2f, AIS_Footstep );
		
		m_animChar.SetBool( "jump", true );
	}
	
	if( !m_isOnGround && ground )
	{
		m_footstepTime = 1.0f;
	}
	m_isOnGround = ground;
	
	Vec2 md = ctrl->GetInputV2( ACT_Chr_Move );
	if( md.LengthSq() > 1 )
		md.Normalize();
	
	Vec2 lvel2 = lvel.ToVec2();
	
	float maxspeed = 5 * ctrl->GetInputV3( ACT_Chr_Move ).z;
	float accel = ( md.NearZero() && !m_isCrouching ) ? 38 : 30;
	if( m_isCrouching ){ accel = 5; maxspeed = 2.5f; }
	if( !ground ){ accel = 0; /* 10 */; } // TODO CREATE VARIABLE
	
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
	
	lvel.x = lvel2.x;
	lvel.y = lvel2.y;
	
	m_bodyHandle->SetLinearVelocity( lvel );
}

void TSCharacter::TurnTo( const Vec2& turnDir, float speedDelta )
{
	if( !IsAlive() )
		return;
	float angroot = m_aimDir.yaw + M_PI;
	float angend = normalize_angle( turnDir.Angle() - angroot ) + angroot;
	float angstart = normalize_angle( m_turnAngle - angroot ) + angroot;
	m_turnAngle = angstart + sign( angend - angstart ) * TMIN( fabsf( angend - angstart ), speedDelta );
}

void TSCharacter::PushTo( const Vec3& pos, float speedDelta )
{
	Vec2 diff = pos.ToVec2() - m_obj->GetWorldPosition().ToVec2();
	diff = diff.Normalized() * TMIN( speedDelta, diff.Length() );
	m_bodyHandle->SetPosition( m_obj->GetWorldPosition() + V3( diff.x, diff.y, 0 ) );
}

void TSCharacter::BeginClosestAction( float maxdist )
{
	Vec3 QP = GetQueryPosition_FT();
	GameObjectGather iact_gather;
	m_level->QuerySphere( &iact_gather, IEST_InteractiveItem, QP, 5 );
	if( iact_gather.items.size() )
	{
		iact_gather.DistanceSort( QP );
		if( ( iact_gather.items[ 0 ].obj->GetWorldInfoTarget() - QP ).Length() < maxdist )
			BeginAction( iact_gather.items[ 0 ].obj );
	}
}

void TSCharacter::BeginAction( GameObject* obj )
{
	if( !obj )
		return;
	obj->SendMessage( "OnInteract", m_obj->GetScriptedObject() );
}

bool TSCharacter::IsTouchingPoint( Vec3 p, float hmargin, float vmargin ) const
{
	Vec3 p0 = GetPosition_FT() + V3(0,0,1);
	Vec3 p1 = p0 + V3( m_cachedBodyExtOffset, 0 );
	Vec3 dist = PointLineDistance3( p, p0, p1 );
	// horizontal/vertical signed distances
	float hsd = dist.ToVec2().Length() - 0.3f - hmargin;
	float vsd = fabsf( dist.z ) - 1.0f - vmargin;
	return hsd <= 0 && vsd <= 0;
}

Vec3 TSCharacter::GetMoveRefPos() const
{
	return GetPosition_FT() + V3(0,0,1) + V3( m_cachedBodyExtOffset * 0.5f, 0 );
}

void TSCharacter::PlayPickupAnim( Vec3 tgt )
{
	m_animChar.SetFloat( "pickup_height",
		tgt.z - ( GetWorldPosition().z + 1 ) );
	m_animChar.SetBool( "pickup", true );
	m_pickupTrigger = true;
}

void TSCharacter::SetSkin( StringView name )
{
	m_animChar.SetSkin( name );
}

void TSCharacter::sgsSetACVar( sgsString name, float val )
{
	m_animChar.SetFloat( name.c_str(), val );
}

void TSCharacter::Reset()
{
	m_health = 100;
	m_animChar.DisablePhysics();
//	m_anLayers[3].factor = 0;
	m_bodyHandle->SetEnabled( true );
	m_animChar.m_cachedMeshInst->layers = 0xffffffff;
}

void TSCharacter::MeshInstUser_OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data )
{
	if( evid == MIEVT_BulletHit )
	{
		SGRX_CAST( MI_BulletHit_Data*, bhinfo, data );
		Mat4 inv = Mat4::Identity;
		m_animChar.m_cachedMeshInst->matrix.InvertTo( inv );
		Vec3 pos = inv.TransformPos( bhinfo->pos );
		Vec3 vel = inv.TransformNormal( bhinfo->vel ).Normalized();
		m_animChar.m_anDeformer.AddModelForce( pos, vel * 0.4f, 0.3f );
		Hit( bhinfo->vel.Length() * ( m_acceptsCriticalDamage ?
			bhinfo->critDmg : bhinfo->dmg ) * 0.15f );
	}
}

void TSCharacter::OnDeath()
{
	m_bodyHandle->SetEnabled( false );
//	m_animChar.EnablePhysics();
	m_animChar.m_anDeformer.forces.clear();
//	m_anLayers[3].factor = 1;
	m_obj->SetInfoMask( 0 );
	m_animChar.m_cachedMeshInst->layers = 0;
	
	SendMessage( "OnDeath" );
	// event
	Game_FireEvent( TSEV_CharDied, this );
}

void TSCharacter::Hit( float pwr )
{
	pwr *= m_damageMultiplier;
//	float m_health = 1000000;
	if( m_health > 0 )
	{
		m_timeSinceLastHit = 0;
//		RenewAction();
		m_health -= pwr;
		
		// event
		{
			TSEventData_CharHit data = { this, pwr };
			Game_FireEvent( TSEV_CharHit, &data );
		}
		
		if( m_health <= 0 )
		{
			OnDeath();
		}
	}
}

GOBehavior* TSCharacter::FindWeapon() const
{
	for( size_t i = 0; i < m_obj->GetChildCount(); ++i )
	{
		GameObject* subobj = m_obj->GetChild( i );
		GOBehavior* bhvr = subobj->m_behaviors.getcopy(
			m_level->GetScriptCtx().CreateString( "weapon" ) );
		if( bhvr )
			return bhvr;
	}
	return NULL;
}

Vec3 TSCharacter::sgsGetAttachmentPos( StringView atch, Vec3 off )
{
	return m_animChar.GetAttachmentPos( m_animChar.FindAttachment( atch ), off );
}

Mat4 TSCharacter::sgsGetAttachmentMatrix( StringView atch )
{
	return m_animChar.GetAttachmentMatrix(
		m_animChar.FindAttachment( atch ) );
}



TSAimHelper::TSAimHelper( GameLevel* lev ) :
	m_level(lev), m_pos(V3(0)), m_cp(V2(0)), m_aimPtr(NULL),
	m_aimPoint(V3(0)), m_rcPoint(V3(0)), m_aimFactor(0), m_pDist(0),
	m_closestObj(NULL), m_closestPoint(V3(0))
{
}

void TSAimHelper::Tick( float deltaTime, GameObject* ownerObj, Vec3 pos, Vec2 cp, bool lock )
{
	m_ownerObj = ownerObj;
	m_pos = pos;
	if( m_aimFactor == 0 || m_aimPtr == NULL )
		m_cp = cp;
	m_rcPoint = _CalcRCPos( pos );
	
	if( lock )
	{
		lock = DoQuery();
		if( m_aimPtr == NULL || m_aimPtr == m_closestObj )
		{
			m_aimPoint = m_closestPoint;
			if( m_aimFactor < SMALL_FLOAT )
			{
				m_aimPtr = m_closestObj;
			}
		}
		else
		{
			m_aimPtr = NULL;
		}
	}
	else m_aimPtr = NULL;
	
	float tgt = m_aimPtr ? 1 : 0;
	float diff = tgt - m_aimFactor;
	m_aimFactor += TMIN( fabsf( diff ), deltaTime * 5 ) * sign( diff );
	
	if( m_aimFactor > 0 && diff >= 0 )
	{
		Vec3 tgtpos = TLERP( m_rcPoint, m_aimPoint, m_aimFactor );
		Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
		Vec2 target_pos = m_level->GetScene()->camera.WorldToScreen( tgtpos ).ToVec2();
		Vec2 tgt = V2( clamp( target_pos.x, 0, 1 ), clamp( target_pos.y, 0, 1 ) ) * screen_size;
		Game_SetCursorPos( tgt.x, tgt.y );
	}
}

bool TSAimHelper::DoQuery()
{
	m_pDist = 0.5f;
	m_closestObj = NULL;
	return m_level->QuerySphere( this, IEST_Target, m_pos, 8.0f );
}

Vec3 TSAimHelper::GetAimPoint()
{
	return TLERP( m_rcPoint, m_aimPoint, m_aimFactor );
}

Vec3 TSAimHelper::_CalcRCPos( Vec3 pos )
{
	Vec3 crpos, crdir;
	m_level->GetScene()->camera.GetCursorRay( m_cp.x, m_cp.y, crpos, crdir );
	Vec3 crtgt = crpos + crdir * 100;
	
	SGRX_PhyRaycastInfo rcinfo;
	if( m_level->GetPhyWorld()->Raycast( crpos, crtgt, 0x1, 0x1, &rcinfo ) )
	{
		bool atwall = 0.707f > fabsf(Vec3Dot( rcinfo.normal, V3(0,0,1) )); // > ~45deg to up vector
		bool frontface = Vec3Dot( rcinfo.normal, crdir ) < 0; 
		if( atwall && frontface )
			return rcinfo.point;
		
		// must adjust target height above ground
		if( frontface )
			return rcinfo.point + V3(0,0,1.1f);
	}
	
	// backup same level plane test if aiming into nothing
	float dsts[2];
	if( RayPlaneIntersect( crpos, crdir, V4(0,0,1.1f,pos.z), dsts ) )
	{
		return crpos + crdir * dsts[0];
	}
	
	return V3(0);
}

bool TSAimHelper::ProcessGameObject( GameObject* obj )
{
	if( obj == m_ownerObj )
		return true;
	
	Vec3 tgtPos = obj->GetWorldInfoTarget();
	SceneRaycastCallback_Any srcb;
	m_level->GetScene()->RaycastAll( m_pos, tgtPos, &srcb, 0x1 );
	if( srcb.m_hit )
		return true;
	
	if( obj == m_aimPtr )
	{
		m_closestObj = m_aimPtr;
		m_closestPoint = tgtPos;
		return false;
	}
	Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
	float bsz = TMIN( screen_size.x, screen_size.y );
	Vec2 curpos = m_cp * screen_size;
	Vec2 scrpos = m_level->GetScene()->camera.WorldToScreen( tgtPos ).ToVec2() * screen_size;
	float npdist = ( curpos - scrpos ).Length();
	if( npdist < m_pDist * bsz )
	{
		m_closestObj = obj;
		m_closestPoint = tgtPos;
		m_pDist = npdist / bsz;
	}
	return true;
}



void ApplyDeadzone( Vec2& v, float lmin = 0.1f, float lmax = 0.9f )
{
	float len = TCLAMP( v.Length(), lmin, lmax );
	v = v.Normalized() * TREVLERP<float>( lmin, lmax, len );
}

TSAimHelperV2::TSAimHelperV2( GameLevel* lev ) :
	m_level(lev),
	m_ownerObj(NULL), m_aimTarget(NULL), m_aimFactor(0),
	m_relocking(false), m_prevJoyAxis(V2(0)), m_cachedAimPoint(V3(0)),
	m_scalableScreenPos(V2(0)), m_ctDist(0)
{
}

void TSAimHelperV2::Tick( Vec2 joyaxis, GameObject* owner )
{
	float deltaTime = m_level->GetDeltaTime();
	
	m_ownerObj = owner;
	if( m_aimTarget && !( m_aimTarget->GetInfoMask() & IEST_Target ) )
		m_aimTarget = NULL;
	if( m_aimTarget )
		m_cachedAimPoint = m_aimTarget->GetWorldInfoTarget();
	m_aimFactor += deltaTime * ( m_aimTarget ? 10 : -10 );
	m_aimFactor = TCLAMP( m_aimFactor, 0.0f, 1.0f );
	
	ApplyDeadzone( joyaxis );
	m_relocking = joyaxis.Length() > m_prevJoyAxis.Length() + 0.001f;
	if( m_relocking )
	{
		// reconfiguring lock-on
		m_scalableScreenPos = joyaxis;
		DoQuery();
	}
	m_prevJoyAxis = joyaxis;
}

void TSAimHelperV2::RemoveLockOn()
{
	m_aimTarget = NULL;
}

bool TSAimHelperV2::DoQuery()
{
	m_ctDist = FLT_MAX;
	m_aimTarget = NULL;
	return m_level->QuerySphere( this, IEST_Target, m_ownerObj->GetWorldInfoTarget(), 8.0f );
}

bool TSAimHelperV2::IsAiming()
{
	return !!m_aimTarget;
}

Vec3 TSAimHelperV2::GetAimPoint()
{
	return m_cachedAimPoint;
}

bool TSAimHelperV2::ProcessGameObject( GameObject* obj )
{
	if( obj == m_ownerObj )
		return false;
	
	Vec3 tgtPos = obj->GetWorldInfoTarget();
	
	Vec2 curpos = m_scalableScreenPos;
	Vec2 scrpos = m_level->GetScene()->camera.WorldToScreen( tgtPos ).ToVec2() * 2 - V2(1);
	float npdist = ( curpos - scrpos ).Length();
	if( npdist < m_ctDist && Vec2Dot( curpos, scrpos ) > 0 )
	{
		m_aimTarget = obj;
		m_ctDist = npdist;
	}
	return false;
}



TSPlayerController::TSPlayerController( GameObject* obj ) :
	BhControllerBase( obj ),
	m_aimHelper( obj->m_level ),
	i_move( V2(0) ), i_aim_target( V3(0) ),
	i_turn( V3(0) ), i_crouch( false ),
	m_prevPos( V3(0) ),
	m_shootTimeout( 0.0f ),
	m_moveFactor( 0.0f ),
	m_imprecisionFactor( 1.0f ),
	m_criticalHitThreshold( 0.2f )
{
}

void TSPlayerController::Update()
{
	float dt = m_level->GetDeltaTime();
	BhControllerBase::Update();
	
	TSCharacter* P = m_obj->FindFirstBehaviorOfType<TSCharacter>();
	Vec3 pos = V3(0);
	if( P )
	{
		pos = P->GetQueryPosition();
	//	Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
	//	m_aimHelper.Tick( dt, m_obj, pos, CURSOR_POS / screen_size, WP_LOCK_ON.value > 0.5f );
		m_aimHelper.Tick( V2( AIM_X.value, AIM_Y.value ), m_obj );
		
		// movement factor
		Vec3 relpos1 = pos - m_aimHelper.GetAimPoint();
		Vec3 relpos2 = m_prevPos - i_aim_target;
		m_moveFactor += ( relpos1 - relpos2 ).Length() * ( 0.02f / dt );
		m_moveFactor -= dt * 2;
		m_moveFactor = TCLAMP( m_moveFactor, 0.0f, 1.0f );
		m_prevPos = pos;
		
		// final imprecision factor state mgmt
		float& src = m_imprecisionFactor;
		if( m_shootTimeout < 0.1f ) // shots fired recently, increase imprecision
		{
			src = TMIN( 2.0f, src + dt * 2 );
		}
		else if( src > 1 ) // recovering from shots
		{
			src = TMAX( 1.0f, src - dt );
		}
		else if( m_moveFactor == 0 && m_aimHelper.IsAiming() ) // character not moving, aim carefully for 2 secs
		{
			src = TMAX( 0.0f, src - dt / ( i_crouch ? 1 : 2 ) );
		}
		else if( src < 1 ) // character moving, revert careful aim
		{
			src = TMIN( 1.0f, src + dt * m_moveFactor );
		}
		
		// shoot timeout for recoil imprecision
		// processed after check to avoid factoring dt into calc
		m_shootTimeout += dt;
	}
	
	LevelMapSystem* map = m_level->GetSystem<LevelMapSystem>();
	if( map )
		map->viewPos = pos;
	
	if( WP_REMOVE_LOCK_ON.value )
		m_aimHelper.RemoveLockOn();
	
	Vec2 walkaxis = V2( MOVE_X.value, MOVE_Y.value );
	ApplyDeadzone( walkaxis );
	i_move = V2
	(
		-walkaxis.x + MOVE_LEFT.value - MOVE_RIGHT.value,
		walkaxis.y + MOVE_DOWN.value - MOVE_UP.value
	);
	
	CameraResource* maincam = m_level->GetMainCamera();
	if( maincam )
	{
		Mat4 wm = maincam->GetWorldMatrix();
		Vec2 fwdir = wm.TransformNormal( V3( 0, -1, -1 ) ).ToVec2().Normalized();
		Vec2 rtdir = wm.TransformNormal( V3( -1, 0, 0 ) ).ToVec2().Normalized();
		i_move = rtdir * i_move.x + fwdir * i_move.y;
	}
	
	i_aim_target = m_aimHelper.GetAimPoint();
	i_turn = V3(0);
	if( i_move.Length() > 0.1f )
	{
		Vec2 md = i_move;
		if( m_aimHelper.IsAiming() && Vec2Dot( ( i_aim_target - pos ).ToVec2(), md ) < 0 )
			md = -md;
		i_turn = V3( md.x, md.y, 8 );
	}
	
	if( CROUCH.IsPressed() )
	{
		i_crouch = !i_crouch;
	}
}

Vec3 TSPlayerController::GetInput( uint32_t iid )
{
	switch( iid )
	{
	case ACT_Chr_Move: return V3( i_move.x, i_move.y, 1 );
	case ACT_Chr_Turn: return i_turn;
	case ACT_Chr_Crouch: return V3(i_crouch);
	case ACT_Chr_Jump: return V3(JUMP.value);
	case ACT_Chr_AimAt: return V3( m_aimHelper.IsAiming() /* aim? */, 32 /* speed */, 0 );
	case ACT_Chr_AimTarget: return i_aim_target;
	case ACT_Chr_Shoot: return V3(WP_SHOOT.value);
	case ACT_Chr_DoAction: return V3(DO_ACTION.value);
	case ACT_Chr_ReloadHolsterDrop: return V3(WP_RELOAD.value, WP_HOLSTER.value, WP_DROP.value);
	}
	return V3(0);
}

void TSPlayerController::CalcUIAimInfo()
{
//	m_aimHelper.DoQuery();
}



TPSPlayerController::TPSPlayerController( GameObject* obj ) :
	BhControllerBase( obj ),
	m_angles( YP(0) ),
	i_move( V2(0) ), i_aim_target( V3(0) ), i_turn( V3(0) ), i_crouch(false),
	lastFrameReset( false )
{
	m_castShape = m_level->GetPhyWorld()->CreateSphereShape( 0.2f );
}

void TPSPlayerController::Update()
{
	float deltaTime = m_level->GetDeltaTime();
	Vec2 joystick_aim = V2( AIM_X.value, AIM_Y.value );
	joystick_aim = -joystick_aim.Normalized() *
		TCLAMP( TREVLERP<float>( 0.2f, 1.0f, joystick_aim.Length() ), 0.0f, 1.0f );
	m_angles.yaw += joystick_aim.x * 10 * deltaTime;
	m_angles.pitch += joystick_aim.y * 10 * deltaTime;
	
	int hx = GR_GetWidth() / 2;
	int hy = GR_GetHeight() / 2;
	Vec2 mouse_aim = V2(0);
	if( GetChar() && GetChar()->IsAlive() )
	{
		if( Game_WasPSCP() )
		{
			mouse_aim = Game_GetCursorPos() - V2( hx, hy );
		}
		Game_PostSetCursorPos( hx, hy );
	}
	float mouse_speed = 0.007f; // 0.01f;
	m_angles.yaw -= mouse_aim.x * mouse_speed;
	m_angles.pitch -= mouse_aim.y * mouse_speed;
	
	m_angles.pitch = TCLAMP( m_angles.pitch, -FLT_PI/3.0f, FLT_PI/3.0f );
	
	if( CROUCH.IsPressed() )
	{
		i_crouch = !i_crouch;
	}
}

Vec3 TPSPlayerController::GetInput( uint32_t iid )
{
	switch( iid )
	{
	case ACT_Chr_Move: return V3( i_move.x, i_move.y, 1 );
	case ACT_Chr_Turn: return i_turn;
	case ACT_Chr_Crouch: return V3(i_crouch);
	case ACT_Chr_Jump: return V3(JUMP.value);
	case ACT_Chr_AimAt: return V3( 1 /* yes */, 32 /* speed */, 0 );
	case ACT_Chr_AimTarget: return i_aim_target;
	case ACT_Chr_Shoot: return V3(WP_SHOOT.value);
	case ACT_Chr_DoAction: return V3(DO_ACTION.value);
	case ACT_Chr_ReloadHolsterDrop: return V3(WP_RELOAD.value, WP_HOLSTER.value, WP_DROP.value);
	}
	return V3(0);
}

void TPSPlayerController::SafePosPush( Vec3& pos, Vec3 dir )
{
	SGRX_PhyRaycastInfo rcinfo;
	if( m_level->GetPhyWorld()->ConvexCast( m_castShape,
			pos, pos + dir, 0x0001, 0x0001, &rcinfo ) )
	{
		pos += dir * rcinfo.factor;
	}
	else
	{
		pos += dir;
	}
}

Vec3 TPSPlayerController::GetCameraPos( TSCharacter* chr, bool tick )
{
	Vec3 campos = tick ? chr->GetWorldPosition() : chr->GetPosition_FT();
	campos += V3(0,0,1);
	SafePosPush( campos, V3(0,0,1) );
	SafePosPush( campos, -m_angles.ToVec3() );
	return campos;
}

struct SceneRaycastCallback_NotChar : SceneRaycastCallback_Closest
{
	virtual void AddResult( SceneRaycastInfo* info )
	{
		if( info->meshinst != chr->m_animChar.m_cachedMeshInst )
			SceneRaycastCallback_Closest::AddResult( info );
	}
	TSCharacter* chr;
};

void TPSPlayerController::UpdateMoveAim( bool tick )
{
	Vec2 move = V2
	(
		MOVE_X.value + MOVE_RIGHT.value - MOVE_LEFT.value,
		MOVE_Y.value + MOVE_DOWN.value - MOVE_UP.value
	);
	SGRX_Scene* scene = m_level->GetScene();
	SGRX_Camera& CAM = scene->camera;
	
	TSCharacter* chr = GetChar();
	if( !chr )
		return;
	
	// raycast for direction
	i_aim_target = CAM.position + CAM.direction.Normalized() * 1000;
	{
		Vec3 pos, dir;
		if( scene->camera.GetCursorRay( 0.5f, 0.5f, pos, dir ) )
		{
			Vec3 start = pos;
			Vec3 end = pos + dir * scene->camera.zfar;
			
			// center plane
			Vec3 ppos = tick ? chr->GetWorldPosition() : chr->GetPosition_FT();
			Vec2 planedir = ( ppos - scene->camera.position ).ToVec2().Normalized();
			Vec3 planedir3 = V3( planedir.x, planedir.y, 0 );
			Vec4 plane = V4( planedir3, Vec3Dot( planedir3, ppos ) + 0.1f ); // safety offset
			
			// offset ray up to center plane
			float dsts[2];
			if( RayPlaneIntersect( start, dir, plane, dsts ) )
			{
				start += dir * dsts[0];
			}
			
			SceneRaycastCallback_NotChar ncrc;
			{
				ncrc.chr = chr;
			}
			scene->RaycastAll( start, end, &ncrc, 0xffffffff );
			if( ncrc.m_hit )
			{
				i_aim_target = TLERP( start, end, ncrc.m_closest.factor );
			}
		}
	}
	
	Vec2 fwd = CAM.direction.ToVec2().Normalized();
	Vec2 rgt = CAM.GetRight().ToVec2().Normalized();
	i_move = fwd * -move.y + rgt * move.x;
	i_turn = V3(0);
	if( i_move.Length() > 0.1f )
	{
		Vec2 md = i_move;
		if( Vec2Dot( ( i_aim_target - chr->GetWorldPosition() ).ToVec2(), md ) < -0.1f )
			md = -md;
		i_turn = V3( md.x, md.y, 8 );
	}
}



TSEnemyController::TSEnemyController( GameObject* obj ) :
	BhControllerBase( obj ),
	i_crouch( false ), i_move( V2(0) ), i_speed( 1 ), i_turn( V3(0) ),
	i_aim_at( false ), i_aim_target( V3(0) ), i_shoot( false ), i_act( false ),
	m_aidb( m_level->GetSystem<AIDBSystem>() )
{
	InitScriptInterface();
	_data = m_level->GetScriptCtx().CreateDict();
	GetScriptedObject().thiscall( C, "Init" );
}

TSEnemyController::~TSEnemyController()
{
}

struct TSEC_FindChar : AIFactDistance
{
	float GetDistance( const AIFact& fact )
	{
		if( fact.type != type )
			return FLT_MAX;
		
		float dist = PointLineDistance( pos,
			fact.position - V3(0,0,0.5f), fact.position + V3(0,0,0.5f) ) - 0.5f;
		float maxdist = ( curTime - fact.created ) * 0.001f * speed;
		if( dist > maxdist )
			return FLT_MAX;
		return dist;
	}
	Vec3 GetPosition(){ return pos; }
	
	uint32_t type;
	Vec3 pos;
	float speed;
	uint32_t curTime;
};

struct EPEnemyViewProc : GameObjectProcessor
{
	EPEnemyViewProc() : sawEnemy(false){}
	bool ProcessGameObject( GameObject* obj )
	{
		// exclude self
		if( obj == enemy->m_obj )
			return false;
		
		AIFactStorage& FS = enemy->m_factStorage;
		AIDBSystem* aidb = enemy->m_aidb;
		TSCharacter* mychr = enemy->GetChar();
		ASSERT( mychr );
		
		Vec3 enemypos = obj->GetWorldInfoTarget();
		
		if( obj->GetInfoMask() & IEST_AIAlert )
		{
			if( !enemy->CanSeePoint( enemypos ) )
				return false;
			FS.InsertOrUpdate( FT_Sight_Alarming,
				enemypos, 0, curtime, curtime + 5*1000, 0 );
		}
		else if( gcv_notarget.value == false )
		{
			TSCharacter* chr = obj->FindFirstBehaviorOfType<TSCharacter>();
			if( !chr )
				return false;
			if( !enemy->CanSeePoint( enemypos ) &&
				!enemy->CanSeePoint( chr->sgsGetAttachmentPos( "head", V3(0) ) ) )
				return false;
			
			AIZoneInfo zi = aidb->GetZoneInfoByPos( chr->GetQueryPosition_FT() );
			AICharInfo& ci = FS.GetCharInfo( obj );
			if( zi.restrictedGroups & chr->m_group )
			{
				ci.IncreaseSuspicion( obj->m_level->GetDeltaTime() * zi.suspicionFactor );
			}
			
			bool foe = !( chr->m_group & mychr->m_group ) && ci.IsSuspicious();
			sawEnemy = true;
			
			TSEC_FindChar fchr;
			{
				fchr.type = foe ? FT_Sight_Foe : FT_Sight_Friend,
				fchr.pos = enemypos;
				fchr.speed = 10;
				fchr.curTime = curtime;
			}
			
			// fact of seeing
			FS.CustomInsertOrUpdate( fchr, fchr.type,
				curtime, curtime + 5*1000 );
			
			// fact of position
			fchr.type = foe ? FT_Position_Foe : FT_Position_Friend;
			FS.CustomInsertOrUpdate( fchr, fchr.type,
				curtime, curtime + 30*1000, FS.last_mod_id );
		}
		
		return false;
	}
	
	TimeVal curtime;
	TSEnemyController* enemy;
	bool sawEnemy;
};

struct TSEC_FindSoundSource : AIFactDistance
{
	float GetDistance( const AIFact& fact )
	{
		if( fact.type != FT_Position_Friend && fact.type != FT_Position_Foe )
			return FLT_MAX;
		
		float dist = PointLineDistance( pos,
			fact.position - V3(0,0,1), fact.position + V3(0,0,1) ) - 0.5f;
		float maxdist = ( curTime - fact.created ) * 0.001f * speed;
		if( dist > maxdist )
			return FLT_MAX;
		return dist;
	}
	Vec3 GetPosition(){ return pos; }
	
	Vec3 pos;
	float speed;
	uint32_t curTime;
};

void TSEnemyController::FixedUpdate()
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return;
	
	if( !chr->IsAlive() )
	{
		return;
	}
	
	TimeVal curTime = m_level->GetPhyTime();
	
	// process facts
	m_factStorage.Process( curTime );
	// - self
	TSEC_FindChar fchr;
	{
		fchr.type = FT_Position_Friend;
		fchr.pos = chr->GetQueryPosition_FT();
		fchr.speed = 10;
		fchr.curTime = curTime;
	}
	m_factStorage.CustomInsertOrUpdate( fchr, FT_Position_Friend,
		curTime, curTime + 1*1000, 0 );
	// - sounds
	for( int i = 0; i < m_aidb->GetNumSounds(); ++i )
	{
		if( m_aidb->CanHearSound( chr->GetQueryPosition_FT(), i ) == false )
			continue;
		AISound S = m_aidb->GetSoundInfo( i );
		
		if( S.type == AIS_Footstep || S.type == AIS_Shot )
		{
			AIFactType sndtype = FT_Sound_Footstep;
			if( S.type == AIS_Shot )
				sndtype = FT_Sound_Shot;
			
			int lastid = m_factStorage.last_mod_id;
			TSEC_FindSoundSource fss;
			{
				fss.pos = S.position;
				fss.speed = 10;
				fss.curTime = curTime;
			}
			if( !m_factStorage.CustomUpdate( fss, curTime, curTime + 30*1000, lastid ) &&
				sndtype == FT_Sound_Footstep )
			{
				m_factStorage.InsertOrUpdate( sndtype,
					S.position, SMALL_FLOAT, curTime, curTime + 1*1000, 0, false );
			}
			else if( sndtype != FT_Sound_Footstep )
			{
				m_factStorage.InsertOrUpdate( sndtype,
					S.position, SMALL_FLOAT, curTime, curTime + 1*1000, 0, false );
			}
		}
		else
		{
			m_factStorage.InsertOrUpdate( FT_Sound_Noise,
				S.position, 1, curTime, curTime + 1*1000 );
		}
	}
	// - vision
	EPEnemyViewProc evp;
	evp.curtime = curTime;
	evp.enemy = this;
	uint32_t qmask = IEST_Target | IEST_AIAlert;
	m_level->QuerySphere( &evp, qmask, chr->GetQueryPosition_FT(), 15.0f );
	
	// update suspicion
	m_factStorage.DecreaseSuspicion( m_level->GetDeltaTime() );
	
	// tick ESO
	m_level->m_scriptCtx.Push( m_level->GetDeltaTime() );
	GetScriptedObject().thiscall( C, "tick", 1 );
	
	i_crouch = _data[ "i_crouch" ].get<bool>();
	i_move = _data[ "i_move" ].get<Vec2>();
	i_speed = _data[ "i_speed" ].get<float>();
	i_turn = _data[ "i_turn" ].get<Vec3>();
	sgsVariable aim_target = _data[ "i_aim_target" ];
	i_aim_at = aim_target.not_null();
	i_aim_target = aim_target.get<Vec3>();
	i_shoot = _data[ "i_shoot" ].get<bool>();
	i_act = _data[ "i_act" ].get<bool>();
}

Vec3 TSEnemyController::GetInput( uint32_t iid )
{
	switch( iid )
	{
	case ACT_Chr_Move: return V3( i_move.x, i_move.y, i_speed );
	case ACT_Chr_Turn: return i_turn;
	case ACT_Chr_Crouch: return V3( i_crouch );
	case ACT_Chr_AimAt: return V3( i_aim_at /* y/n */, 8 /* speed */, 0 );
	case ACT_Chr_AimTarget: return i_aim_target;
	case ACT_Chr_Shoot: return V3( i_shoot );
	case ACT_Chr_DoAction: return V3( i_act );
	}
	return V3(0);
}

void TSEnemyController::Reset()
{
	m_factStorage.Clear();
	GetScriptedObject().thiscall( C, "Reset" );
}

void TSEnemyController::DebugDrawWorld()
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return;
	
	if( gcv_dbg_aifacts.value )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset().Col( 0.9f, 0.2f, 0.1f );
		Vec3 pos = chr->GetWorldPosition();
		
		m_factStorage.SortCreatedDesc();
		
		size_t count = TMIN( size_t(10), m_factStorage.facts.size() );
		for( size_t i = 0; i < count; ++i )
		{
			AIFact& F = m_factStorage.facts[ i ];
			br.SetPrimitiveType( PT_Lines );
			br.Pos( pos ).Pos( F.position );
			br.Tick( F.position, 0.1f );
		}
	}
	
	if( gcv_dbg_aipaths.value )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset().Col( 0.9f, 0.2f, 0.1f );
		br.SetPrimitiveType( PT_Lines );
		for( size_t i = 0; i < m_path.size(); ++i )
		{
			float q;
			if( i > 0 )
			{
				q = float(i-1) / ( m_path.size() - 1 );
				br.Col( 1 - q, 1, 0 );
				br.Pos( m_path[ i - 1 ] );
				
				q = float(i) / ( m_path.size() - 1 );
				br.Col( 1 - q, 1, 0 );
				br.Pos( m_path[ i ] );
			}
			br.Tick( m_path[ i ], 0.05f );
		}
	}
	
	if( gcv_dbg_aiproc.value )
	{
		GetScriptedObject().thiscall( C, "debug_draw_world" );
	}
}

void TSEnemyController::DebugDrawUI()
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return;
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	Vec3 pos = chr->GetWorldPosition();
	bool infront;
	Vec3 screenpos = m_level->GetScene()->camera.WorldToScreen( pos, &infront );
	if( !infront )
		return;
	int x = screenpos.x * GR_GetWidth();
	int y = screenpos.y * GR_GetHeight();
	
	GR2D_SetFont( "system_outlined", 7 );
	int lht = 9;
	
	if( gcv_dbg_aifacts.value )
	{
		char bfr[ 256 ];
		
		size_t count = TMIN( size_t(10), m_factStorage.facts.size() );
		sgrx_snprintf( bfr, 256, "count: %d, mod id: %d, next: %d",
			int(m_factStorage.facts.size()), int(m_factStorage.last_mod_id),
			int(m_factStorage.m_next_fact_id) );
		
		int len = GR2D_GetTextLength( bfr );
		br.Reset().Col( 0.0f, 0.5f ).Quad( x, y, x + len, y + lht - 1 );
		br.Col( 1.0f );
		GR2D_DrawTextLine( x, y, bfr );
		
		y += lht;
		
		for( size_t i = 0; i < count; ++i )
		{
			AIFact& F = m_factStorage.facts[ i ];
			const char* type = "type?";
			switch( F.type )
			{
			case FT_Unknown: type = "unknown"; break;
			case FT_Sound_Noise: type = "snd-noise"; break;
			case FT_Sound_Footstep: type = "snd-step"; break;
			case FT_Sound_Shot: type = "snd-shot"; break;
			case FT_Sight_ObjectState: type = "sight-state"; break;
			case FT_Sight_Alarming: type = "sight-alarm"; break;
			case FT_Sight_Friend: type = "sight-friend"; break;
			case FT_Sight_Foe: type = "sight-foe"; break;
			case FT_Position_Friend: type = "pos-friend"; break;
			case FT_Position_Foe: type = "pos-foe"; break;
			}
			
			sgrx_snprintf( bfr, 256, "Fact #%d (ref=%d) <%s> @ %.4g;%.4g;%.4g cr: %d, exp: %d",
				int(F.id), int(F.ref), type, F.position.x, F.position.y, F.position.z,
				int(F.created), int(F.expires) );
			
			int len = GR2D_GetTextLength( bfr );
			br.Reset().Col( 0.0f, 0.5f ).Quad( x, y, x + len, y + lht - 1 );
			br.Col( 1.0f );
			GR2D_DrawTextLine( x, y, bfr );
			
			y += lht;
		}
	}
	if( gcv_dbg_aistack.value )
	{
		char bfr[ 512 ];
		
		sgs_Context* T = _data.getprop("coro_ai_main").var.data.T;
		sgs_StackFrame* sf = sgs_GetFramePtr( T, NULL, 0 );
		while( sf )
		{
			int line;
			const char *name, *file;
			sgs_StackFrameInfo( T, sf, &name, &file, &line );
			sgrx_snprintf( bfr, 512, "%.240s at %.240s:%d", name, file, line );
			sf = sgs_GetFramePtr( T, sf, 0 );
			
			int len = GR2D_GetTextLength( bfr );
			br.Reset().Col( 0.0f, 0.5f ).Quad( x, y, x + len, y + lht - 1 );
			br.Col( 1.0f );
			GR2D_DrawTextLine( x, y, bfr );
			
			y += lht;
		}
	}
}

bool TSEnemyController::CanSeePoint( Vec3 pt )
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return false;
	
	Vec3 vieworigin = chr->GetQueryPosition_FT();
	Vec3 viewdir = chr->GetAimDir_FT();
	Vec3 view2pos = pt - vieworigin;
	
	// check immediate proximity
	if( view2pos.ToVec2().Length() < 0.5f && fabsf( view2pos.z ) < 1 )
		return true;
	
	float coneAngle = clamp( 70.0f - powf( (view2pos*V3(1,1,2)).Length(), 0.9f ) * 5.2f, 0, 90 );
	float vpdot = Vec3Dot( viewdir.Normalized(), view2pos.Normalized() );
	if( vpdot <= cosf(DEG2RAD(coneAngle)) )
		return false; // outside view cone
	
	if( m_level->GetPhyWorld()->Raycast( vieworigin, pt, 1, 1 ) )
		return false; // behind wall
	
	return true;
}

bool TSEnemyController::LookingAtPoint( Vec3 pt )
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return false;
	
	Vec3 vieworigin = chr->GetQueryPosition_FT();
	Vec3 viewdir = chr->GetAimDir_FT();
	Vec3 view2pos = pt - vieworigin;
	
	// increase vertical FOV
	viewdir.z *= 0.5f;
	view2pos.z *= 0.5f;
	
	float vpdot = Vec3Dot( viewdir.Normalized(), view2pos.Normalized() );
	return vpdot >= cosf(DEG2RAD(40.0f));
}

bool TSEnemyController::sgsHasFact( uint32_t typemask )
{
	return m_factStorage.HasFact( typemask );
}

bool TSEnemyController::sgsExpireFacts( uint32_t typemask )
{
	return m_factStorage.ExpireFacts( typemask );
}

bool TSEnemyController::sgsHasRecentFact( uint32_t typemask, TimeVal maxtime )
{
	return m_factStorage.HasRecentFact( typemask, maxtime );
}

bool TSEnemyController::sgsHasRecentFactAt( uint32_t typemask, TimeVal maxtime, Vec3 pos, float rad )
{
	return m_factStorage.HasRecentFactAt( typemask, maxtime, pos, rad );
}

SGS_MULTRET TSEnemyController::sgsGetRecentFact( uint32_t typemask, TimeVal maxtime )
{
	AIFact* F = m_factStorage.GetRecentFact( typemask, maxtime );
	if( F )
	{
		sgs_CreateLiteClassFrom( C, NULL, F );
		return 1;
	}
	return 0;
}

void TSEnemyController::sgsInsertFact( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref )
{
	m_factStorage.Insert( type, pos, created, expires, ref );
}

bool TSEnemyController::sgsUpdateFact( uint32_t type, Vec3 pos,
	float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( sgs_StackSize( C ) < 7 )
		reset = true;
	return m_factStorage.Update( type, pos, rad, created, expires, ref, reset );
}

void TSEnemyController::sgsInsertOrUpdateFact( uint32_t type, Vec3 pos,
	float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( sgs_StackSize( C ) < 7 )
		reset = true;
	m_factStorage.InsertOrUpdate( type, pos, rad, created, expires, ref, reset );
}

SGS_MULTRET TSEnemyController::sgsGetCover(
	Vec3 position, Vec3 viewer, uint32_t mask /* = 0 */, uint32_t req /* = 0 */, bool inv /* = false */ )
{
	LC_CoverPart* cover = m_aidb->FindCover( position, viewer, mask, req, inv );
	if( cover == NULL )
		return 0;
	sgs_PushVar( C, ( cover->p0 + cover->p1 ) * 0.5f );
	sgs_PushVar( C, cover->flags );
	sgs_PushVar( C, cover->n );
	return 3;
}

bool TSEnemyController::sgsIsWalkable( Vec3 pos, Vec3 ext )
{
	return m_aidb->m_pathfinder.FindPoly( pos, NULL, ext ) != 0;
}

bool TSEnemyController::sgsFindPath( const Vec3& to )
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return false;
	
	m_aidb->m_pathfinder.FindPath( chr->GetQueryPosition_FT() - V3(0,0,0.5f), to, m_path );
	return m_path.size() > 0;
}

bool TSEnemyController::sgsHasPath()
{
	return m_path.size() != 0;
}

int TSEnemyController::sgsGetPathPointCount()
{
	return m_path.size();
}

bool TSEnemyController::sgsAdvancePath( float dist )
{
	while( dist > 0 && m_path.size() >= 2 )
	{
		float len = ( m_path[0] - m_path[1] ).Length();
		if( dist > len )
		{
			dist -= len;
			m_path.erase(0);
		}
		else
		{
			m_path[0] = TLERP( m_path[0], m_path[1], dist / len );
			break;
		}
	}
	if( m_path.size() == 1 )
		m_path.clear();
	return m_path.size() != 0;
}

sgsMaybe<Vec3> TSEnemyController::sgsGetNextPathPoint()
{
	if( m_path.size() )
		return m_path[ 0 ];
	return sgsMaybeNot;
}

bool TSEnemyController::sgsRemoveNextPathPoint()
{
	if( m_path.size() )
	{
		m_path.erase( 0 );
		return true;
	}
	return false;
}


TSGameSystem::TSGameSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	register_tsent_cvars(); // TODO global init?
	
	static const sgs_RegIntConst g_ts_ints[] =
	{
		{ "ACT_Chr_Move", ACT_Chr_Move },
		{ "ACT_Chr_Turn", ACT_Chr_Turn },
		{ "ACT_Chr_Crouch", ACT_Chr_Crouch },
		{ "ACT_Chr_Jump", ACT_Chr_Jump },
		{ "ACT_Chr_AimAt", ACT_Chr_AimAt },
		{ "ACT_Chr_AimTarget", ACT_Chr_AimTarget },
		{ "ACT_Chr_Shoot", ACT_Chr_Shoot },
		{ "ACT_Chr_DoAction", ACT_Chr_DoAction },
		{ "ACT_Chr_ReloadHolsterDrop", ACT_Chr_ReloadHolsterDrop },
		
		{ "COVER_LOW", COVER_LOW },
		{ "COVER_OPENLEFT", COVER_OPENLEFT },
		{ "COVER_OPENRIGHT", COVER_OPENRIGHT },
		{ "COVER_OPENTYPE", COVER_OPENTYPE },
		
		{ NULL, 0 },
	};
	sgs_RegIntConsts( lev->GetSGSC(), g_ts_ints, -1 );
	
	TSCharacter::Register( lev );
	TSPlayerController::Register( lev );
	TPSPlayerController::Register( lev );
	TSEnemyController::Register( lev );
	
	m_level->GetScriptCtx().Include( "levels/ai_core" );
}


