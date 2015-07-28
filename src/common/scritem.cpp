

#include "scritem.hpp"



#define SCRITEM_OFSCHK( i, ret ) if( (i) < 0 || (i) >= 4 ){ \
	sgs_Msg( C, SGS_WARNING, "wrong offset: %d outside " SCRITEM_RANGE_STR, (int)(i) ); ret; }
#define SCRITEM_MESHCHK( i, ret ) if( m_meshes[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no mesh at offset %d", (int)(i) ); ret; }
#define SCRITEM_PSYSCHK( i, ret ) if( m_partSys[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no part.sys at offset %d", (int)(i) ); ret; }
#define SCRITEM_BODYCHK( i, ret ) if( m_bodies[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no body at offset %d", (int)(i) ); ret; }
#define SCRITEM_DSYSCHK( ret ) if( m_decalSys == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no decal sys" ); ret; }

SGRX_ScriptedItem* SGRX_ScriptedItem::Create( SGRX_Scene* scene, SGRX_IPhyWorld* phyWorld, SGS_CTX, sgsVariable func )
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
	obj.thiscall( func );
	
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

void SGRX_ScriptedItem::Tick( float dt )
{
	{
		SGS_SCOPE;
		sgs_PushReal( C, dt );
		Handle( this ).get_variable().thiscall( "update", 1 );
	}
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		if( m_partSys[ i ] )
			m_partSys[ i ]->Tick( dt );
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
	if( m_decalSys )
		m_decalSys->Upload();
}

void SGRX_ScriptedItem::OnEvent( SGRX_MeshInstance* MI, uint32_t evid, float amt )
{
	if( evid == MIEVT_BulletHit )
	{
		SGS_SCOPE;
		sgs_PushReal( C, amt );
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
		Handle( this ).get_variable().thiscall( "onhit", 2 );
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
	if( m_decalSysMI )
		m_decalSysMI->matrix = mtx;
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

void SGRX_ScriptedItem::MISetMatrix( int i, Mat4 mtx )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_MESHCHK( i, return );
	m_meshMatrices[ i ] = mtx;
	m_meshes[ i ]->matrix = mtx * m_transform;
}

void SGRX_ScriptedItem::PSCreate( int i, StringView path )
{
	SCRITEM_OFSCHK( i, return );
	m_partSys[ i ] = new ParticleSystem;
	m_partSys[ i ]->AddToScene( m_scene );
	if( path )
		m_partSys[ i ]->Load( path );
	m_partSys[ i ]->SetTransform( m_partSysMatrices[ i ] * m_transform );
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

void SGRX_ScriptedItem::DSCreate( StringView texDecalPath, StringView texFalloffPath, uint32_t size )
{
	if( sgs_StackSize( C ) < 3 )
		size = 16*1024;
	
	m_decalSys = new SGRX_DecalSystem;
	m_decalSys->Init( GR_GetTexture( texDecalPath ), GR_GetTexture( texFalloffPath ) );
	m_decalSys->SetSize( size );
	m_decalSys->m_ownMatrix = &m_transform;
	
	m_decalSysMI = m_scene->CreateMeshInstance();
	m_decalSysMI->mesh = m_decalSys->m_mesh;
	m_decalSysMI->matrix = m_transform;
	m_decalSysMI->dynamic = 1;
	m_decalSysMI->decal = 1;
	
	dmgDecalSysOverride = m_decalSys;
}

void SGRX_ScriptedItem::DSDestroy()
{
	m_decalSys = NULL;
	m_decalSysMI = NULL;
	dmgDecalSysOverride = NULL;
}

void SGRX_ScriptedItem::DSResize( uint32_t size )
{
	SCRITEM_DSYSCHK( return );
	m_decalSys->SetSize( size );
}

void SGRX_ScriptedItem::DSClear()
{
	SCRITEM_DSYSCHK( return );
	m_decalSys->ClearAllDecals();
}

SGRX_SIRigidBodyInfo::Handle SGRX_ScriptedItem::RBMakeInfo()
{
	return SGRX_SIRigidBodyInfo::Handle( SGS_PUSHCLASS( C, SGRX_SIRigidBodyInfo, () ) );
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
	return m_bodies[ i ]->GetPosition();
}

void SGRX_ScriptedItem::RBSetPosition( int i, Vec3 v )
{
	SCRITEM_OFSCHK( i, return );
	SCRITEM_BODYCHK( i, return );
	m_bodies[ i ]->SetPosition( v );
}

Mat4 SGRX_ScriptedItem::RBGetMatrix( int i )
{
	SCRITEM_OFSCHK( i, return Mat4::Identity );
	SCRITEM_BODYCHK( i, return Mat4::Identity );
	return Mat4::CreateRotationFromQuat( m_bodies[ i ]->GetRotation() )
		* Mat4::CreateTranslation( m_bodies[ i ]->GetPosition() );
}



