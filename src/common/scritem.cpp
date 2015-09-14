

#include "scritem.hpp"



static int SIRigidBodyInfo( SGS_CTX )
{
	SGS_PUSHCLASS( C, SGRX_SIRigidBodyInfo, () );
	return 1;
}

static int SIHingeJointInfo( SGS_CTX )
{
	SGS_PUSHCLASS( C, SGRX_SIHingeJointInfo, () );
	return 1;
}

static int SIConeTwistJointInfo( SGS_CTX )
{
	SGS_PUSHCLASS( C, SGRX_SIConeTwistJointInfo, () );
	return 1;
}

static sgs_RegIntConst scritem_intconsts[] =
{
	{ "SCRITEM_ForceType_Velocity", SCRITEM_ForceType_Velocity },
	{ "SCRITEM_ForceType_Impulse", SCRITEM_ForceType_Impulse },
	{ "SCRITEM_ForceType_Acceleration", SCRITEM_ForceType_Acceleration },
	{ "SCRITEM_ForceType_Force", SCRITEM_ForceType_Force },
	{ NULL, 0 },
};

static sgs_RegFuncConst scritem_funcconsts[] =
{
	{ "SIRigidBodyInfo", SIRigidBodyInfo },
	{ "SIHingeJointInfo", SIHingeJointInfo },
	{ "SIConeTwistJointInfo", SIConeTwistJointInfo },
	{ NULL, NULL },
};

void ScrItem_InstallAPI( SGS_CTX )
{
	sgs_RegIntConsts( C, scritem_intconsts, -1 );
	sgs_RegFuncConsts( C, scritem_funcconsts, -1 );
}


#define SCRITEM_OFSCHK( i, ret ) if( (i) < 0 || (i) >= 4 ){ \
	sgs_Msg( C, SGS_WARNING, "wrong offset: %d outside " SCRITEM_RANGE_STR, (int)(i) ); ret; }
#define SCRITEM_MESHCHK( i, ret ) if( m_meshes[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no mesh at offset %d", (int)(i) ); ret; }
#define SCRITEM_PSYSCHK( i, ret ) if( m_partSys[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no part.sys at offset %d", (int)(i) ); ret; }
#define SCRITEM_BODYCHK( i, ret ) if( m_bodies[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no body at offset %d", (int)(i) ); ret; }
#define SCRITEM_JOINTCHK( i, ret ) if( m_joints[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no joint at offset %d", (int)(i) ); ret; }
#define SCRITEM_DSYSCHK( ret ) if( m_dmgDecalSys == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no decal sys" ); ret; }

SGRX_ScriptedItem* SGRX_ScriptedItem::Create(
	SGRX_Scene* scene,
	SGRX_IPhyWorld* phyWorld,
	SGS_CTX,
	sgsVariable func,
	sgsVariable args )
{
	SGRX_ScriptedItem* SI = SGS_PUSHCLASS( C, SGRX_ScriptedItem, () );
	sgs_ObjAcquire( C, SI->m_sgsObject );
	sgsVariable obj( C, sgsVariable::PickAndPop );
	SI->m_variable.C = C;
	sgs_InitDict( C, &SI->m_variable.var, 0 );
	SI->m_scene = scene;
	SI->m_phyWorld = phyWorld;
	SI->m_lightSampler = &GR_GetDummyLightSampler();
	SI->m_transform = Mat4::Identity;
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		SI->m_meshMatrices[ i ] = Mat4::Identity;
		SI->m_partSysMatrices[ i ] = Mat4::Identity;
	}
	
	// call the initialization function
	{
		SGS_SCOPE;
		args.push( C );
		obj.thiscall( func, 1 );
	}
	
	return SI;
}

void SGRX_ScriptedItem::Acquire()
{
	sgs_ObjAcquire( C, m_sgsObject );
}

void SGRX_ScriptedItem::Release()
{
	sgs_ObjRelease( C, m_sgsObject );
}

void SGRX_ScriptedItem::SetLightSampler( SGRX_LightSampler* sampler )
{
	m_lightSampler = sampler;
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		if( m_partSys[ i ] )
			m_partSys[ i ]->m_lightSampler = m_lightSampler;
	}
}

void SGRX_ScriptedItem::SetPSRaycast( SGRX_IPSRaycast* psrc )
{
	m_psRaycast = psrc;
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		if( m_partSys[ i ] )
			m_partSys[ i ]->m_psRaycast = m_psRaycast;
	}
}

void SGRX_ScriptedItem::FixedTick( float deltaTime )
{
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		if( m_bodies[ i ] )
		{
			m_bodyPos[ i ].Advance( m_bodies[ i ]->GetPosition() );
			m_bodyRot[ i ].Advance( m_bodies[ i ]->GetRotation() );
		}
	}
	// fixed update event
	{
		SGS_SCOPE;
		sgs_PushReal( C, deltaTime );
		Handle( this ).get_variable().thiscall( "fixedupdate", 1 );
	}
}

void SGRX_ScriptedItem::Tick( float deltaTime, float blendFactor )
{
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		if( m_bodies[ i ] )
		{
			m_bodyPosLerp[ i ] = m_bodyPos[ i ].Get( blendFactor );
			m_bodyRotLerp[ i ] = m_bodyRot[ i ].Get( blendFactor );
		}
	}
	// update event
	{
		SGS_SCOPE;
		sgs_PushReal( C, deltaTime );
		sgs_PushReal( C, blendFactor );
		Handle( this ).get_variable().thiscall( "update", 2 );
	}
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		if( m_partSys[ i ] )
			m_partSys[ i ]->Tick( deltaTime );
	}
}

void SGRX_ScriptedItem::PreRender()
{
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		if( m_meshes[ i ] )
			m_lightSampler->LightMesh( m_meshes[ i ] );
		if( m_partSys[ i ] )
			m_partSys[ i ]->PreRender();
	}
	if( m_dmgDecalSys )
		m_dmgDecalSys->Upload();
	if( m_ovrDecalSys )
		m_ovrDecalSys->Upload();
	if( m_dmgDecalSysMI )
		m_lightSampler->LightMesh( m_dmgDecalSysMI );
	if( m_ovrDecalSysMI )
		m_lightSampler->LightMesh( m_ovrDecalSysMI );
}

void SGRX_ScriptedItem::EntityEvent( const StringView& type )
{
	SGS_SCOPE;
	sgs_PushVar( C, type );
	Handle( this ).get_variable().thiscall( "onevent", 1 );
}

void SGRX_ScriptedItem::OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data )
{
	if( evid == MIEVT_BulletHit )
	{
		SGRX_CAST( MI_BulletHit_Data*, bhinfo, data );
		SGS_SCOPE;
		sgs_PushVec3p( C, &bhinfo->pos.x );
		sgs_PushVec3p( C, &bhinfo->vel.x );
		int i = 0;
		for( ; i < SCRITEM_NUM_SLOTS; ++i )
		{
			if( MI == m_meshes[ i ] )
			{
				sgs_PushInt( C, i );
				break;
			}
		}
		if( i == SCRITEM_NUM_SLOTS )
			sgs_PushInt( C, -1 ); // wat
		Handle( this ).get_variable().thiscall( "onhit", 3 );
	}
}

// ---

int SGRX_ScriptedItem::_getindex(
	SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop )
{
	SGRX_CAST( SGRX_ScriptedItem*, SI, obj->data );
	SGSRESULT res = sgs_PushIndexPP( C, &SI->m_variable.var, key, isprop );
	if( res != SGS_ENOTFND )
		return res; // found or serious error
	return _sgs_getindex( C, obj, key, isprop );
}

int SGRX_ScriptedItem::_setindex(
	SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, sgs_Variable* val, int isprop )
{
	SGRX_CAST( SGRX_ScriptedItem*, SI, obj->data );
	if( _sgs_setindex( C, obj, key, val, isprop ) != SGS_SUCCESS )
		sgs_SetIndexPPP( C, &SI->m_variable.var, key, val, isprop );
	return SGS_SUCCESS;
}

void SGRX_ScriptedItem::SetMatrix( Mat4 mtx )
{
	m_transform = mtx;
	if( m_dmgDecalSysMI )
		m_dmgDecalSysMI->matrix = mtx;
	if( m_ovrDecalSysMI )
		m_ovrDecalSysMI->matrix = mtx;
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		if( m_meshes[ i ] )
			m_meshes[ i ]->matrix = m_meshMatrices[ i ] * mtx;
		if( m_partSys[ i ] )
			m_partSys[ i ]->SetTransform( m_partSysMatrices[ i ] * mtx );
	}
}

void SGRX_ScriptedItem::MICreate( int i, StringView path )
{
	SCRITEM_OFSCHK( i, return );
	m_meshes[ i ] = m_scene->CreateMeshInstance();
	m_meshes[ i ]->matrix = m_meshMatrices[ i ] * m_transform;
	m_meshes[ i ]->dynamic = 1;
	m_meshes[ i ]->userData = this;
	if( path )
		m_meshes[ i ]->mesh = GR_GetMesh( path );
}

void SGRX_ScriptedItem::MIDestroy( int i )
{
	SCRITEM_OFSCHK( i, return );
	m_meshes[ i ] = NULL;
}

bool SGRX_ScriptedItem::MIExists( int i )
{
	SCRITEM_OFSCHK( i, return false );
	return m_meshes[ i ] != NULL;
}

void SGRX_ScriptedItem::MISetMesh( int i, StringView path )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_MESHCHK( i, return );
	m_meshes[ i ]->mesh = GR_GetMesh( path );
}

void SGRX_ScriptedItem::MISetEnabled( int i, bool enabled )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_MESHCHK( i, return );
	m_meshes[ i ]->enabled = enabled;
}

void SGRX_ScriptedItem::MISetDynamic( int i, bool dynamic )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_MESHCHK( i, return );
	m_meshes[ i ]->dynamic = dynamic;
}

void SGRX_ScriptedItem::MISetMatrix( int i, Mat4 mtx )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_MESHCHK( i, return );
	m_meshMatrices[ i ] = mtx;
	m_meshes[ i ]->matrix = mtx * m_transform;
}

void SGRX_ScriptedItem::MISetShaderConst( int i, int v, Vec4 var )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_MESHCHK( i, return );
	if( v < 0 || v >= MAX_MI_CONSTANTS )
	{
		sgs_Msg( C, SGS_WARNING, "shader constant %d outside range [0;%d)", v, MAX_MI_CONSTANTS );
		return;
	}
	m_meshes[ i ]->constants[ v ] = var;
}

void SGRX_ScriptedItem::PSCreate( int i, StringView path )
{
	SCRITEM_OFSCHK( i, return );
	m_partSys[ i ] = new ParticleSystem;
	m_partSys[ i ]->AddToScene( m_scene );
	if( path )
		m_partSys[ i ]->Load( path );
	m_partSys[ i ]->SetTransform( m_partSysMatrices[ i ] * m_transform );
	m_partSys[ i ]->m_lightSampler = m_lightSampler;
	m_partSys[ i ]->m_psRaycast = m_psRaycast;
}

void SGRX_ScriptedItem::PSDestroy( int i )
{
	SCRITEM_OFSCHK( i, return );
	m_partSys[ i ] = NULL;
}

bool SGRX_ScriptedItem::PSExists( int i )
{
	SCRITEM_OFSCHK( i, return false );
	return m_partSys[ i ] != NULL;
}

void SGRX_ScriptedItem::PSLoad( int i, StringView path )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_PSYSCHK( i, return );
	m_partSys[ i ]->Load( path );
}

void SGRX_ScriptedItem::PSSetMatrix( int i, Mat4 mtx )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_PSYSCHK( i, return );
	m_partSysMatrices[ i ] = mtx;
	m_partSys[ i ]->SetTransform( mtx * m_transform );
}

void SGRX_ScriptedItem::PSSetMatrixFromMeshAABB( int i, int mi )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_PSYSCHK( i, return );
	SCRITEM_OFSCHK( mi, return );
	SCRITEM_MESHCHK( mi, return );
	SGRX_IMesh* M = m_meshes[ mi ]->mesh;
	if( M == NULL )
	{
		sgs_Msg( C, SGS_WARNING, "mesh is not loaded" );
		return;
	}
	Vec3 diff = M->m_boundsMax - M->m_boundsMin;
	Vec3 off = ( M->m_boundsMax + M->m_boundsMin ) * 0.5f;
	Mat4 mtx = Mat4::CreateScale( diff * 0.5f ) * Mat4::CreateTranslation( off );
	m_partSysMatrices[ i ] = mtx;
	m_partSys[ i ]->SetTransform( mtx * m_transform );
}

void SGRX_ScriptedItem::PSPlay( int i )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_PSYSCHK( i, return );
	m_partSys[ i ]->Play();
}

void SGRX_ScriptedItem::PSStop( int i )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_PSYSCHK( i, return );
	m_partSys[ i ]->Stop();
}

void SGRX_ScriptedItem::PSTrigger( int i )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_PSYSCHK( i, return );
	m_partSys[ i ]->Trigger();
}

void SGRX_ScriptedItem::DSCreate( StringView texDmgDecalPath,
	StringView texOvrDecalPath, StringView texFalloffPath, uint32_t size )
{
	if( sgs_StackSize( C ) < 4 )
		size = 64*1024;
	
	m_dmgDecalSys = new SGRX_DecalSystem;
	m_dmgDecalSys->Init( GR_GetTexture( texDmgDecalPath ), GR_GetTexture( texFalloffPath ) );
	m_dmgDecalSys->SetSize( size );
	m_dmgDecalSys->m_ownMatrix = &m_transform;
	
	m_dmgDecalSysMI = m_scene->CreateMeshInstance();
	m_dmgDecalSysMI->mesh = m_dmgDecalSys->m_mesh;
	m_dmgDecalSysMI->matrix = m_transform;
	m_dmgDecalSysMI->dynamic = 1;
	
	dmgDecalSysOverride = m_dmgDecalSys;
	
	m_ovrDecalSys = new SGRX_DecalSystem;
	m_ovrDecalSys->Init( GR_GetTexture( texOvrDecalPath ), GR_GetTexture( texFalloffPath ) );
	m_ovrDecalSys->SetSize( size );
	m_ovrDecalSys->m_ownMatrix = &m_transform;
	
	m_ovrDecalSysMI = m_scene->CreateMeshInstance();
	m_ovrDecalSysMI->mesh = m_ovrDecalSys->m_mesh;
	m_ovrDecalSysMI->matrix = m_transform;
	m_ovrDecalSysMI->dynamic = 1;
	
	ovrDecalSysOverride = m_ovrDecalSys;
}

void SGRX_ScriptedItem::DSDestroy()
{
	m_dmgDecalSys = NULL;
	m_dmgDecalSysMI = NULL;
	m_ovrDecalSys = NULL;
	m_ovrDecalSysMI = NULL;
	dmgDecalSysOverride = NULL;
	ovrDecalSysOverride = NULL;
}

void SGRX_ScriptedItem::DSResize( uint32_t size )
{
	SCRITEM_DSYSCHK( return );
	m_dmgDecalSys->SetSize( size );
	m_ovrDecalSys->SetSize( size );
}

void SGRX_ScriptedItem::DSClear()
{
	SCRITEM_DSYSCHK( return );
	m_dmgDecalSys->ClearAllDecals();
	m_ovrDecalSys->ClearAllDecals();
}

void SGRX_ScriptedItem::RBCreateFromMesh( int i, int mi, SGRX_SIRigidBodyInfo* spec )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_OFSCHK( mi, return );
	SCRITEM_MESHCHK( mi, return );
	SGRX_IMesh* M = m_meshes[ mi ]->mesh;
	if( M == NULL )
	{
		sgs_Msg( C, SGS_WARNING, "mesh is not loaded" );
		return;
	}
	SGRX_PhyRigidBodyInfo rbi;
	if( spec )
		rbi = *spec;
	rbi.shape = m_phyWorld->CreateShapeFromMesh( M );
	m_bodies[ i ] = m_phyWorld->CreateRigidBody( rbi );
	m_bodyPos[ i ] = IVState<Vec3>( m_bodyPosLerp[ i ] = rbi.position );
	m_bodyRot[ i ] = IVState<Quat>( m_bodyRotLerp[ i ] = rbi.rotation );
}

void SGRX_ScriptedItem::RBCreateFromConvexPointSet( int i, StringView cpset, SGRX_SIRigidBodyInfo* spec )
{
	SCRITEM_OFSCHK( i, return );
	ConvexPointSetHandle cpsh = GP_GetConvexPointSet( cpset );
	if( cpsh == NULL )
	{
		sgs_Msg( C, SGS_WARNING, "failed to load convex point set" );
		return;
	}
	SGRX_PhyRigidBodyInfo rbi;
	if( spec )
		rbi = *spec;
	rbi.shape = m_phyWorld->CreateConvexHullShape( cpsh->data.points.data(), cpsh->data.points.size() );
	m_bodies[ i ] = m_phyWorld->CreateRigidBody( rbi );
	m_bodyPos[ i ] = IVState<Vec3>( m_bodyPosLerp[ i ] = rbi.position );
	m_bodyRot[ i ] = IVState<Quat>( m_bodyRotLerp[ i ] = rbi.rotation );
}

void SGRX_ScriptedItem::RBDestroy( int i )
{
	SCRITEM_OFSCHK( i, return );
	m_bodies[ i ] = NULL;
}

bool SGRX_ScriptedItem::RBExists( int i )
{
	SCRITEM_OFSCHK( i, return false );
	return m_bodies[ i ] != NULL;
}

void SGRX_ScriptedItem::RBSetEnabled( int i, bool enabled )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_BODYCHK( i, return );
	m_bodies[ i ]->SetEnabled( enabled );
}

Vec3 SGRX_ScriptedItem::RBGetPosition( int i )
{
	SCRITEM_OFSCHK( i, return V3(0) );
	SCRITEM_BODYCHK( i, return V3(0) );
	return m_bodyPosLerp[ i ]; // m_bodies[ i ]->GetPosition();
}

void SGRX_ScriptedItem::RBSetPosition( int i, Vec3 v )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_BODYCHK( i, return );
	m_bodyPos[ i ] = IVState<Vec3>( m_bodyPosLerp[ i ] = v );
	m_bodies[ i ]->SetPosition( v );
}

Quat SGRX_ScriptedItem::RBGetRotation( int i )
{
	SCRITEM_OFSCHK( i, return Quat::Identity );
	SCRITEM_BODYCHK( i, return Quat::Identity );
	return m_bodyRotLerp[ i ]; // m_bodies[ i ]->GetRotation();
}

void SGRX_ScriptedItem::RBSetRotation( int i, Quat v )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_BODYCHK( i, return );
	m_bodyRot[ i ] = IVState<Quat>( m_bodyRotLerp[ i ] = v );
	m_bodies[ i ]->SetRotation( v );
}

Mat4 SGRX_ScriptedItem::RBGetMatrix( int i )
{
	SCRITEM_OFSCHK( i, return Mat4::Identity );
	SCRITEM_BODYCHK( i, return Mat4::Identity );
	return Mat4::CreateRotationFromQuat( m_bodyRotLerp[ i ] ) // m_bodies[ i ]->GetRotation() )
		* Mat4::CreateTranslation( m_bodyPosLerp[ i ] ); // m_bodies[ i ]->GetPosition() );
}

void SGRX_ScriptedItem::RBApplyForce( int i, int type, Vec3 v, /*opt*/ Vec3 p )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_BODYCHK( i, return );
	if( sgs_StackSize( C ) >= 4 )
		m_bodies[ i ]->ApplyForce( (EPhyForceType) type, v, p );
	else
		m_bodies[ i ]->ApplyCentralForce( (EPhyForceType) type, v );
}

void SGRX_ScriptedItem::JTCreateHingeB2W( int i, int bi, SGRX_SIHingeJointInfo* spec )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_OFSCHK( bi, return );
	SCRITEM_BODYCHK( bi, return );
	SGRX_PhyHingeJointInfo hjinfo = *spec;
	hjinfo.bodyA = m_bodies[ bi ];
	m_joints[ i ] = m_phyWorld->CreateHingeJoint( hjinfo );
}

void SGRX_ScriptedItem::JTCreateHingeB2B( int i, int biA, int biB, SGRX_SIHingeJointInfo* spec )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_OFSCHK( biA, return );
	SCRITEM_BODYCHK( biA, return );
	SCRITEM_OFSCHK( biB, return );
	SCRITEM_BODYCHK( biB, return );
	SGRX_PhyHingeJointInfo hjinfo = *spec;
	hjinfo.bodyA = m_bodies[ biA ];
	hjinfo.bodyB = m_bodies[ biB ];
	m_joints[ i ] = m_phyWorld->CreateHingeJoint( hjinfo );
}

void SGRX_ScriptedItem::JTCreateConeTwistB2W( int i, int bi, SGRX_SIConeTwistJointInfo* spec )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_OFSCHK( bi, return );
	SCRITEM_BODYCHK( bi, return );
	SGRX_PhyConeTwistJointInfo ctjinfo = *spec;
	ctjinfo.bodyA = m_bodies[ bi ];
	m_joints[ i ] = m_phyWorld->CreateConeTwistJoint( ctjinfo );
}

void SGRX_ScriptedItem::JTCreateConeTwistB2B( int i, int biA, int biB, SGRX_SIConeTwistJointInfo* spec )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_OFSCHK( biA, return );
	SCRITEM_BODYCHK( biA, return );
	SCRITEM_OFSCHK( biB, return );
	SCRITEM_BODYCHK( biB, return );
	SGRX_PhyConeTwistJointInfo ctjinfo = *spec;
	ctjinfo.bodyA = m_bodies[ biA ];
	ctjinfo.bodyB = m_bodies[ biB ];
	m_joints[ i ] = m_phyWorld->CreateConeTwistJoint( ctjinfo );
}

void SGRX_ScriptedItem::JTDestroy( int i )
{
	SCRITEM_OFSCHK( i, return );
	m_joints[ i ] = NULL;
}

bool SGRX_ScriptedItem::JTExists( int i )
{
	SCRITEM_OFSCHK( i, return false );
	return m_joints[ i ] != NULL;
}

void SGRX_ScriptedItem::JTSetEnabled( int i, bool enabled )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_JOINTCHK( i, return );
	m_joints[ i ]->SetEnabled( enabled );
}



