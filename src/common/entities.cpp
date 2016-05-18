

#include "entities.hpp"
#include "resources.hpp"



void MultiEntity::PreRender()
{
	if( m_dmgDecalSys )
		m_dmgDecalSys->Upload();
	if( m_ovrDecalSys )
		m_ovrDecalSys->Upload();
	if( m_dmgDecalSys )
		m_level->LightMesh( m_dmgDecalSys->m_meshInst );
	if( m_ovrDecalSys )
		m_level->LightMesh( m_ovrDecalSys->m_meshInst );
}

void MultiEntity::OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data )
{
	if( evid == MIEVT_BulletHit && GetScriptedObject().getprop( "OnHit" ).not_null() )
	{
		SGRX_CAST( MI_BulletHit_Data*, bhinfo, data );
		SGS_SCOPE;
		sgs_CreateVec3p( C, NULL, &bhinfo->pos.x );
		sgs_CreateVec3p( C, NULL, &bhinfo->vel.x );
		int i = 0;
		for( ; i < 4; ++i )
		{
			if( MI == m_meshes[ i ] )
			{
				sgs_PushInt( C, i );
				break;
			}
		}
		if( i == 4 )
			sgs_PushInt( C, -1 ); // wat
		GetScriptedObject().thiscall( C, "OnHit", 3 );
	}
}

void MultiEntity::DSCreate( StringView texDmgDecalPath,
	StringView texOvrDecalPath, StringView texFalloffPath, uint32_t size )
{
	if( sgs_StackSize( C ) < 4 )
		size = 64*1024;
	
	m_dmgDecalSys = new SGRX_DecalSystem;
	m_dmgDecalSys->Init( m_level->GetScene(), GR_GetTexture( texDmgDecalPath ), GR_GetTexture( texFalloffPath ) );
	m_dmgDecalSys->SetSize( size );
	m_dmgDecalSys->SetDynamic( true );
	
	dmgDecalSysOverride = m_dmgDecalSys;
	
	m_ovrDecalSys = new SGRX_DecalSystem;
	m_ovrDecalSys->Init( m_level->GetScene(), GR_GetTexture( texOvrDecalPath ), GR_GetTexture( texFalloffPath ) );
	m_ovrDecalSys->SetSize( size );
	m_ovrDecalSys->SetDynamic( true );
	
	ovrDecalSysOverride = m_ovrDecalSys;
}

void MultiEntity::DSDestroy()
{
	m_dmgDecalSys = NULL;
	m_ovrDecalSys = NULL;
	dmgDecalSysOverride = NULL;
	ovrDecalSysOverride = NULL;
}

void MultiEntity::DSResize( uint32_t size )
{
	m_dmgDecalSys->SetSize( size );
	m_ovrDecalSys->SetSize( size );
}

void MultiEntity::DSClear()
{
	m_dmgDecalSys->ClearAllDecals();
	m_ovrDecalSys->ClearAllDecals();
}

void MultiEntity::RBCreateFromConvexPointSet( int i, StringView cpset, SGRX_RigidBodyInfo* spec )
{
	ConvexPointSetHandle cpsh = GP_GetConvexPointSet( cpset );
	if( cpsh == NULL )
	{
		sgs_Msg( C, SGS_WARNING, "failed to load convex point set" );
		return;
	}
	SGRX_PhyRigidBodyInfo rbi;
	if( spec )
		rbi = *spec;
	rbi.shape = m_level->GetPhyWorld()->CreateConvexHullShape( cpsh->data.points.data(), cpsh->data.points.size() );
	m_bodies[ i ] = m_level->GetPhyWorld()->CreateRigidBody( rbi );
}

void MultiEntity::RBApplyForce( int i, int type, Vec3 v, /*opt*/ Vec3 p )
{
	if( sgs_StackSize( C ) >= 4 )
		m_bodies[ i ]->ApplyForce( (EPhyForceType) type, v, p );
	else
		m_bodies[ i ]->ApplyCentralForce( (EPhyForceType) type, v );
}

void MultiEntity::JTCreateHingeB2W( int i, int bi, SGRX_HingeJointInfo* spec )
{
	SGRX_PhyHingeJointInfo hjinfo = *spec;
	hjinfo.bodyA = m_bodies[ bi ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateHingeJoint( hjinfo );
}

void MultiEntity::JTCreateHingeB2B( int i, int biA, int biB, SGRX_HingeJointInfo* spec )
{
	SGRX_PhyHingeJointInfo hjinfo = *spec;
	hjinfo.bodyA = m_bodies[ biA ];
	hjinfo.bodyB = m_bodies[ biB ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateHingeJoint( hjinfo );
}

void MultiEntity::JTCreateConeTwistB2W( int i, int bi, SGRX_ConeTwistJointInfo* spec )
{
	SGRX_PhyConeTwistJointInfo ctjinfo = *spec;
	ctjinfo.bodyA = m_bodies[ bi ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateConeTwistJoint( ctjinfo );
}

void MultiEntity::JTCreateConeTwistB2B( int i, int biA, int biB, SGRX_ConeTwistJointInfo* spec )
{
	SGRX_PhyConeTwistJointInfo ctjinfo = *spec;
	ctjinfo.bodyA = m_bodies[ biA ];
	ctjinfo.bodyB = m_bodies[ biB ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateConeTwistJoint( ctjinfo );
}




