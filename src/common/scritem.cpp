

#include "scritem.hpp"



#define SCRITEM_OFSCHK( i ) if( (i) < 0 || (i) >= 4 ){ \
	sgs_Msg( C, SGS_WARNING, "wrong offset: %d outside " SCRITEM_RANGE_STR, (int)(i) ); return; }
#define SCRITEM_MESHCHK( i ) if( m_meshes[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no mesh at offset %d", (int)(i) ); return; }
#define SCRITEM_PSYSCHK( i ) if( m_partSys[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no part.sys at offset %d", (int)(i) ); return; }

SGRX_ScriptedItem* SGRX_ScriptedItem::Create( SGRX_Scene* scene, SGS_CTX, sgsVariable func )
{
	SGRX_ScriptedItem* SI = SGS_PUSHCLASS( C, SGRX_ScriptedItem, () );
	sgs_ObjAcquire( C, SI->m_sgsObject );
	sgsVariable obj( C, sgsVariable::PickAndPop );
	SI->m_variable.C = C;
	sgs_InitDict( C, &SI->m_variable.var, 0 );
	SI->m_scene = scene;
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
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		if( m_meshes[ i ] )
			m_meshes[ i ]->matrix = m_meshMatrices[ i ] * mtx;
		if( m_partSys[ i ] )
			m_partSys[ i ]->SetTransform( m_partSysMatrices[ i ] * mtx );
	}
}

void SGRX_ScriptedItem::CreateMeshInst( int i, StringView path )
{
	SCRITEM_OFSCHK( i );
	m_meshes[ i ] = m_scene->CreateMeshInstance();
	m_meshes[ i ]->matrix = m_meshMatrices[ i ] * m_transform;
	m_meshes[ i ]->dynamic = 1;
	m_meshes[ i ]->userData = this;
	if( path )
		m_meshes[ i ]->mesh = GR_GetMesh( path );
}

void SGRX_ScriptedItem::DestroyMeshInst( int i )
{
	SCRITEM_OFSCHK( i );
	m_meshes[ i ] = NULL;
}

void SGRX_ScriptedItem::SetMesh( int i, StringView path )
{
	SCRITEM_OFSCHK( i );
	SCRITEM_MESHCHK( i );
	m_meshes[ i ]->mesh = GR_GetMesh( path );
}

void SGRX_ScriptedItem::SetMeshInstMatrix( int i, Mat4 mtx )
{
	SCRITEM_OFSCHK( i );
	SCRITEM_MESHCHK( i );
	m_meshMatrices[ i ] = mtx;
	m_meshes[ i ]->matrix = mtx * m_transform;
}

void SGRX_ScriptedItem::CreatePartSys( int i, StringView path )
{
	SCRITEM_OFSCHK( i );
	m_partSys[ i ] = new ParticleSystem;
	m_partSys[ i ]->AddToScene( m_scene );
	if( path )
		m_partSys[ i ]->Load( path );
	m_partSys[ i ]->SetTransform( m_partSysMatrices[ i ] * m_transform );
}

void SGRX_ScriptedItem::DestroyPartSys( int i )
{
	SCRITEM_OFSCHK( i );
	m_partSys[ i ] = NULL;
}

void SGRX_ScriptedItem::LoadPartSys( int i, StringView path )
{
	SCRITEM_OFSCHK( i );
	SCRITEM_PSYSCHK( i );
	m_partSys[ i ]->Load( path );
}

void SGRX_ScriptedItem::SetPartSysMatrix( int i, Mat4 mtx )
{
	SCRITEM_OFSCHK( i );
	SCRITEM_PSYSCHK( i );
	m_partSysMatrices[ i ] = mtx;
	m_partSys[ i ]->SetTransform( mtx * m_transform );
}

void SGRX_ScriptedItem::PartSysPlay( int i )
{
	SCRITEM_OFSCHK( i );
	SCRITEM_PSYSCHK( i );
	m_partSys[ i ]->Play();
}

void SGRX_ScriptedItem::PartSysStop( int i )
{
	SCRITEM_OFSCHK( i );
	SCRITEM_PSYSCHK( i );
	m_partSys[ i ]->Stop();
}

void SGRX_ScriptedItem::PartSysTrigger( int i )
{
	SCRITEM_OFSCHK( i );
	SCRITEM_PSYSCHK( i );
	m_partSys[ i ]->Trigger();
}



