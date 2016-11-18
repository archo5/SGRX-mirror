

#include "level.hpp"

#include "resources.hpp"


sgs_ObjInterface g_sgsobj_empty_handle[1] = {{ "empty_handle", NULL }};


CVarBool gcv_cl_gui( "cl_gui", true );
CVarBool gcv_cl_debug( "cl_debug", false );
CVarBool gcv_dbg_physics( "dbg_physics", false );
CVarBool gcv_g_paused( "g_paused", false );

void RegisterCommonGameCVars()
{
	REGCOBJ( gcv_cl_gui );
	REGCOBJ( gcv_cl_debug );
	REGCOBJ( gcv_dbg_physics );
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



GOResource::GOResource( GameObject* obj ) :
	LevelScrObj( obj->m_level ),
	m_rsrcType( 0 ),
	m_localMatrix( Mat4::Identity ),
	m_matrixMode( MM_Relative ),
	m_obj( obj )
{
}

void GOResource::OnDestroy()
{
}

void GOResource::PrePhysicsFixedUpdate()
{
}

void GOResource::FixedUpdate()
{
}

void GOResource::Update()
{
}

void GOResource::PreRender()
{
}

void GOResource::OnTransformUpdate()
{
}

void GOResource::EditUI( EditorUIHelper*, sgsVariable iface )
{
	sgsVariable fn = GetScriptedObject().getprop( "EditorGUI" );
	if( fn.not_null() )
	{
		iface.push( C );
		GetScriptedObject().thiscall( C, fn, 1 );
	}
}

void GOResource::EditLoad( sgsVariable src, sgsVariable iface )
{
	EditUI( NULL, iface );
}

void GOResource::EditSave( sgsVariable out, sgsVariable iface )
{
	EditUI( NULL, iface );
}

Vec3 GOResource::EditorIconPos()
{
	return GetWorldMatrix().GetTranslation();
}

Mat4 GOResource::MatrixResourceToObject( Mat4 xf ) const
{
	if( m_matrixMode == MM_Absolute )
		return xf;
	else
		return xf * m_localMatrix.Inverted();
}

Mat4 GOResource::GetWorldMatrix() const
{
	if( m_matrixMode == MM_Absolute )
		return m_localMatrix;
	else
		return m_localMatrix * m_obj->GetWorldMatrix();
}

void GOResource::EditorDrawWorld()
{
	sgsVariable fn = GetScriptedObject().getprop( "EditorDrawWorld" );
	if( fn.not_null() )
		GetScriptedObject().thiscall( C, fn );
}

GOBehavior* GOBehavior::_Create( GameObject* go )
{
	return new GOBehavior( go );
}

void GOBehavior::Register( GameLevel* lev )
{
	lev->_RegisterNativeBehavior< GOBehavior >( _sgs_interface->name );
	lev->m_goNativeBhvrMap.set( _sgs_interface->name, _Create );
}

GOBehavior::GOBehavior( GameObject* obj ) :
	LevelScrObj( obj->m_level ),
	m_obj( obj )
{
}

void GOBehavior::Init()
{
	sgsVariable fn = GetScriptedObject().getprop( "Init" );
	if( fn.not_null() )
		GetScriptedObject().thiscall( C, fn );
}

void GOBehavior::OnDestroy()
{
	sgsVariable fn = GetScriptedObject().getprop( "OnDestroy" );
	if( fn.not_null() )
		GetScriptedObject().thiscall( C, fn );
}

void GOBehavior::PrePhysicsFixedUpdate()
{
	sgsVariable fn_prephysicsfixedupdate = GetScriptedObject().getprop( "PrePhysicsFixedUpdate" );
	if( fn_prephysicsfixedupdate.not_null() )
		GetScriptedObject().thiscall( C, fn_prephysicsfixedupdate );
}

void GOBehavior::FixedUpdate()
{
	sgsVariable fn_fixedupdate = GetScriptedObject().getprop( "FixedUpdate" );
	if( fn_fixedupdate.not_null() )
		GetScriptedObject().thiscall( C, fn_fixedupdate );
}

void GOBehavior::Update()
{
	sgsVariable fn_update = GetScriptedObject().getprop( "Update" );
	if( fn_update.not_null() )
		GetScriptedObject().thiscall( C, fn_update );
}

void GOBehavior::PreRender()
{
	sgsVariable fn_prerender = GetScriptedObject().getprop( "PreRender" );
	if( fn_prerender.not_null() )
		GetScriptedObject().thiscall( C, fn_prerender );
}

void GOBehavior::OnTransformUpdate()
{
	sgsVariable fn = GetScriptedObject().getprop( "OnTransformUpdate" );
	if( fn.not_null() )
		GetScriptedObject().thiscall( C, fn );
}

void GOBehavior::OnIDUpdate()
{
	sgsVariable fn = GetScriptedObject().getprop( "OnIDUpdate" );
	if( fn.not_null() )
		GetScriptedObject().thiscall( C, fn );
}

void GOBehavior::sgsSendMessage( sgsString name, sgsVariable arg )
{
	m_obj->sgsSendMessage( name, arg );
}

void GOBehavior::SendMessage( StringView name, sgsVariable arg )
{
	sgsSendMessage( m_level->GetScriptCtx().CreateString( name ), arg );
}

void GOBehavior::EditUI( EditorUIHelper*, sgsVariable iface )
{
	sgsVariable fn = GetScriptedObject().getprop( "EditorGUI" );
	if( fn.not_null() )
	{
		iface.push( C );
		GetScriptedObject().thiscall( C, fn, 1 );
	}
}

void GOBehavior::EditLoad( sgsVariable src, sgsVariable iface )
{
	EditUI( NULL, iface );
}

void GOBehavior::EditSave( sgsVariable out, sgsVariable iface )
{
	EditUI( NULL, iface );
}

void GOBehavior::EditorDrawWorld()
{
	sgsVariable fn = GetScriptedObject().getprop( "EditorDrawWorld" );
	if( fn.not_null() )
		GetScriptedObject().thiscall( C, fn );
}


GameObject::GameObject( GameLevel* lev ) :
	LevelScrObj( lev ),
	m_infoMask( 0 ),
	m_infoTarget( V3(0) )
{
}

GameObject::~GameObject()
{
	m_level->m_infoEmitSet.Unregister( this );
	m_level->_UnmapGameObjectByID( this );
}

GOResource* GameObject::AddResource( uint32_t type )
{
	GOResource* rsrc = NULL;
	GOResourceInfo* ri = m_level->m_goResourceMap.getptr( type );
	if( ri )
		rsrc = ri->createFunc( this );
	if( rsrc )
	{
		rsrc->m_rsrcType = type;
		if( m_level->nextObjectGUID.NotNull() )
		{
			rsrc->m_src_guid = m_level->nextObjectGUID;
			m_level->nextObjectGUID.SetNull();
		}
		rsrc->InitScriptInterface();
		m_resources.push_back( rsrc );
		Game_FireEvent( EID_GOResourceAdd, rsrc );
	}
	return rsrc;
}

GOResource* GameObject::RequireResource( uint32_t type, bool retifnc )
{
	GOResource* rsrc = FindFirstResourceOfTypeID( type );
	if( rsrc )
	{
		if( m_level->nextObjectGUID.NotNull() )
		{
			rsrc->m_src_guid = m_level->nextObjectGUID;
			m_level->nextObjectGUID.SetNull();
		}
		return retifnc ? rsrc : NULL;
	}
	return AddResource( type );
}

void GameObject::RemoveResource( GOResource* rsrc )
{
	H_GOResource keepref = rsrc;
	if( rsrc && m_resources.remove_first( rsrc ) )
	{
		Game_FireEvent( EID_GOResourceRemove, rsrc );
	}
}

void GameObject::RemoveResource( GOResource::ScrHandle rsrc )
{
	RemoveResource( (GOResource*) rsrc );
}

GOBehavior* GameObject::_CreateBehaviorReal( sgsString type )
{
	GOBehavior* bhvr = NULL;
	StringView typekey( type.c_str(), type.size() );
	GOBehaviorCreateFunc* createFunc = m_level->m_goNativeBhvrMap.getcopy( typekey, NULL );
	if( createFunc )
		bhvr = createFunc( this );
	if( bhvr )
	{
		bhvr->InitScriptInterface();
		m_behaviors.push_back( bhvr );
		return bhvr;
	}
	
	sgsVariable bclass = m_level->m_scriptCtx.GetGlobal( type );
	if( !bclass.not_null() )
	{
		LOG << "BEHAVIOR TYPE NOT FOUND: " << type.c_str();
		sgs_Msg( m_level->GetSGSC(), SGS_ERROR, "failed to find behavior: %s", type.c_str() );
		return NULL;
	}
	
	sgsString native_name = bclass.getprop("__inherit").get_string();
	bhvr = _CreateBehaviorReal( native_name );
	if( !bhvr )
	{
		LOG_ERROR << "FAILED to create scripted behavior '" << type.c_str() << "'"
			<< " - could not find native behavior '" << native_name.c_str() << "'";
		return NULL;
	}
	bhvr->_data = m_level->m_scriptCtx.CreateDict();
	sgsVariable BSO = bhvr->GetScriptedObject();
	sgsVariable bhvr_orig_metaobj = BSO.get_meta_obj();
	sgsVariable bclass_orig_metaobj = bclass.get_meta_obj();
	if( bclass_orig_metaobj.not_null() &&
		bhvr_orig_metaobj.get_object_struct() != bclass_orig_metaobj.get_object_struct() )
	{
		LOG_ERROR << "FAILED to create scripted behavior: redefining metaobject for '"
			<< type.c_str() << "'";
		delete bhvr;
		return NULL;
	}
	bclass.set_meta_obj( bhvr_orig_metaobj );
	BSO.set_meta_obj( bclass );
	BSO.enable_metamethods( true );
	
	return bhvr;
}

GOBehavior* GameObject::AddBehavior( sgsString type )
{
	SGRX_GUID guid = m_level->nextObjectGUID;
	m_level->nextObjectGUID.SetNull();
	
	GOBehavior* bhvr = _CreateBehaviorReal( type );
	if( bhvr )
	{
		bhvr->m_type = type;
		if( guid.NotNull() )
		{
			bhvr->m_src_guid = guid;
			guid.SetNull();
		}
		bhvr->Init();
		Game_FireEvent( EID_GOBehaviorAdd, bhvr );
	}
	return bhvr;
}

GOBehavior* GameObject::RequireBehavior( sgsString type, bool retifnc )
{
	GOBehavior* bhvr = FindFirstBehaviorOfTypeName( type );
	if( bhvr && bhvr->m_type == type )
	{
		if( m_level->nextObjectGUID.NotNull() )
		{
			bhvr->m_src_guid = m_level->nextObjectGUID;
			m_level->nextObjectGUID.SetNull();
		}
		return retifnc ? bhvr : NULL;
	}
	return AddBehavior( type );
}

void GameObject::RemoveBehavior( GOBehavior* bhvr )
{
	H_GOBehavior keepref = bhvr;
	if( bhvr && m_behaviors.remove_first( bhvr ) )
	{
		Game_FireEvent( EID_GOBehaviorRemove, bhvr );
	}
}

void GameObject::RemoveBehavior( GOBehavior::ScrHandle rsrc )
{
	RemoveBehavior( (GOBehavior*) rsrc );
}

void GameObject::OnDestroy()
{
	for( size_t i = 0; i < m_resources.size(); ++i )
		m_resources[ i ]->OnDestroy();
	for( size_t i = 0; i < m_behaviors.size(); ++i )
		m_behaviors[ i ]->OnDestroy();
}

void GameObject::PrePhysicsFixedUpdate()
{
	for( size_t i = 0; i < m_resources.size(); ++i )
		m_resources[ i ]->PrePhysicsFixedUpdate();
	for( size_t i = 0; i < m_behaviors.size(); ++i )
		m_behaviors[ i ]->PrePhysicsFixedUpdate();
}

void GameObject::FixedUpdate()
{
	for( size_t i = 0; i < m_resources.size(); ++i )
		m_resources[ i ]->FixedUpdate();
	for( size_t i = 0; i < m_behaviors.size(); ++i )
		m_behaviors[ i ]->FixedUpdate();
}

void GameObject::Update()
{
	for( size_t i = 0; i < m_resources.size(); ++i )
		m_resources[ i ]->Update();
	for( size_t i = 0; i < m_behaviors.size(); ++i )
		m_behaviors[ i ]->Update();
}

void GameObject::PreRender()
{
	for( size_t i = 0; i < m_resources.size(); ++i )
		m_resources[ i ]->PreRender();
	for( size_t i = 0; i < m_behaviors.size(); ++i )
		m_behaviors[ i ]->PreRender();
}

void GameObject::OnTransformUpdate()
{
	for( size_t i = 0; i < m_resources.size(); ++i )
	{
		if( m_resources[ i ] != _xfChangeInvoker )
			m_resources[ i ]->OnTransformUpdate();
	}
	for( size_t i = 0; i < m_behaviors.size(); ++i )
	{
		if( m_behaviors[ i ] != _xfChangeInvoker )
			m_behaviors[ i ]->OnTransformUpdate();
	}
}

void GameObject::OnIDUpdate()
{
//	for( size_t i = 0; i < m_resources.size(); ++i )
//		m_resources[ i ]->OnIDUpdate();
	for( size_t i = 0; i < m_behaviors.size(); ++i )
		m_behaviors[ i ]->OnIDUpdate();
}

void GameObject::sgsSendMessage( sgsString name, sgsVariable arg )
{
	for( size_t i = 0; i < m_behaviors.size(); ++i )
	{
		sgsVariable so = m_behaviors[ i ]->GetScriptedObject();
		sgsVariable fn = so.getprop( name );
		if( fn.not_null() )
			so.tthiscall<void>( C, fn, arg );
	}
	sgsVariable so = GetScriptedObject();
	sgsVariable fn = so.getprop( name );
	if( fn.not_null() )
		so.tthiscall<void>( C, fn, arg );
}

void GameObject::SendMessage( StringView name, sgsVariable arg )
{
	sgsSendMessage( m_level->GetScriptCtx().CreateString( name ), arg );
}

void GameObject::DebugDrawWorld()
{
	for( size_t i = 0; i < m_resources.size(); ++i )
		m_resources[ i ]->DebugDrawWorld();
	for( size_t i = 0; i < m_behaviors.size(); ++i )
		m_behaviors[ i ]->DebugDrawWorld();
}

void GameObject::DebugDrawUI()
{
	for( size_t i = 0; i < m_resources.size(); ++i )
		m_resources[ i ]->DebugDrawUI();
	for( size_t i = 0; i < m_behaviors.size(); ++i )
		m_behaviors[ i ]->DebugDrawUI();
}

void GameObject::EditorDrawWorld()
{
	for( size_t i = 0; i < m_resources.size(); ++i )
		m_resources[ i ]->EditorDrawWorld();
	for( size_t i = 0; i < m_behaviors.size(); ++i )
		m_behaviors[ i ]->EditorDrawWorld();
}

GOResource::ScrHandle GameObject::sgsFindFirstResourceOfType( sgsVariable typeOrMetaObj )
{
	for( size_t i = 0; i < m_resources.size(); ++i )
	{
		GOResource* rsrc = m_resources[ i ];
		int tid = typeOrMetaObj.type_id();
		if( tid == SGS_VT_INT )
		{
			if( rsrc->m_rsrcType == typeOrMetaObj.get<uint32_t>() )
				return GOResource::ScrHandle( rsrc );
		}
		else if( tid == SGS_VT_OBJECT )
		{
			sgs_VarObj* obj = rsrc->m_sgsObject;
			while( obj->metaobj )
			{
				if( obj->metaobj == typeOrMetaObj.get_object_struct() )
					return GOResource::ScrHandle( rsrc );
				obj = obj->metaobj;
			}
		}
	}
	return GOResource::ScrHandle();
}

GOBehavior::ScrHandle GameObject::sgsFindFirstBehaviorOfType( sgsVariable typeOrMetaObj )
{
	for( size_t i = 0; i < m_behaviors.size(); ++i )
	{
		GOBehavior* bhvr = m_behaviors[ i ];
		int tid = typeOrMetaObj.type_id();
		if( tid == SGS_VT_STRING )
		{
			if( bhvr->m_type == typeOrMetaObj.get_string() )
				return GOBehavior::ScrHandle( bhvr );
		}
		else if( tid == SGS_VT_OBJECT )
		{
			sgs_VarObj* obj = bhvr->m_sgsObject;
			while( obj->metaobj )
			{
				if( obj->metaobj == typeOrMetaObj.get_object_struct() )
					return GOBehavior::ScrHandle( bhvr );
				obj = obj->metaobj;
			}
		}
	}
	return GOBehavior::ScrHandle();
}

sgsVariable GameObject::sgsFindAllResourcesOfType( sgsVariable typeOrMetaObj )
{
	int count = 0;
	for( size_t i = 0; i < m_resources.size(); ++i )
	{
		GOResource* rsrc = m_resources[ i ];
		int tid = typeOrMetaObj.type_id();
		if( tid == SGS_VT_INT )
		{
			if( rsrc->m_rsrcType == typeOrMetaObj.get<uint32_t>() )
			{
				count++;
				m_level->GetScriptCtx().Push( rsrc->GetScriptedObject() );
			}
		}
		else if( tid == SGS_VT_OBJECT )
		{
			if( rsrc->m_sgsObject->metaobj == typeOrMetaObj.get_object_struct() )
			{
				count++;
				m_level->GetScriptCtx().Push( rsrc->GetScriptedObject() );
			}
		}
	}
	return m_level->GetScriptCtx().CreateArray( count );
}

sgsVariable GameObject::sgsFindAllBehaviorsOfType( sgsVariable typeOrMetaObj )
{
	int count = 0;
	for( size_t i = 0; i < m_behaviors.size(); ++i )
	{
		GOBehavior* bhvr = m_behaviors[ i ];
		int tid = typeOrMetaObj.type_id();
		if( tid == SGS_VT_STRING )
		{
			if( bhvr->m_type == typeOrMetaObj.get_string() )
			{
				count++;
				m_level->GetScriptCtx().Push( bhvr->GetScriptedObject() );
			}
		}
		else if( tid == SGS_VT_OBJECT )
		{
			if( bhvr->m_sgsObject->metaobj == typeOrMetaObj.get_object_struct() )
			{
				count++;
				m_level->GetScriptCtx().Push( bhvr->GetScriptedObject() );
			}
		}
	}
	return m_level->GetScriptCtx().CreateArray( count );
}

void GameObject::SetInfoMask( uint32_t mask )
{
	if( mask && !m_infoMask )
		m_level->m_infoEmitSet.Register( this );
	else if( !mask && m_infoMask )
		m_level->m_infoEmitSet.Unregister( this );
	m_infoMask = mask;
}



IEditorSystemCompiler::~IEditorSystemCompiler()
{
}

void IEditorSystemCompiler::WrapChunk( ByteArray& chunk, const char id[4] )
{
	uint32_t size = chunk.size();
	uint8_t prepbytes[4];
	memcpy( prepbytes, id, 4 );
	memcpy( prepbytes + 4, &size, 4 );
	chunk.insert( 0, prepbytes, 8 );
}


struct GameLevelSystemCompiler : IEditorSystemCompiler
{
	bool GenerateChunk( ByteArray& out, sgsVariable sysParams )
	{
		ByteWriter bw( &out );
		bw << markers;
		WrapChunk( out, LC_FILE_MRKR_NAME );
		return true;
	}
	
	void ProcessEntity( EditorEntity& ent )
	{
		// combine position markers into a chunk
		if( ent.type == "Marker" )
		{
			sgsString name = ent.props.getprop("name").get_string();
			Vec3 pos = ent.props.getprop("position").get<Vec3>();
			LC_Marker M = { StringView( name.c_str(), name.size() ), pos };
			markerNameRefs.push_back( name );
			markers.markers.push_back( M );
			return;
		}
	}
	
	Array< sgsString > markerNameRefs;
	LC_Chunk_Mrkr markers;
};


GameLevel::GameLevel( PhyWorldHandle phyWorld ) :
	m_phyWorld( phyWorld ),
	m_nameIDGen( 0 ),
	m_currentTickTime( 0 ),
	m_currentPhyTime( 0 ),
	m_deltaTime( 0 ),
	m_blendFactor( 0 ),
	m_tickDeltaTime( 0 ),
	m_fixedTickDeltaTime( 0 ),
	m_eventType( LEV_None ),
	m_editorMode( false ),
	m_enableLoadingScreen( true ),
	m_paused( false ),
	m_levelTime( 0 )
{
	LOG_FUNCTION;
	
	m_scriptCtx.RegisterEngineCoreAPI();
	m_scriptCtx.RegisterBatchRenderer();
	GFWRegisterCore( m_scriptCtx.C );
	
	m_guiSys = new GameUISystem( &m_scriptCtx );
	
	// handled events
	RegisterHandler( EID_WindowEvent );
	RegisterHandler( EID_GOResourceAdd );
	RegisterHandler( EID_GOResourceRemove );
	
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
		// info flags
		{ "IEST_InteractiveItem", IEST_InteractiveItem },
		{ "IEST_HeatSource", IEST_HeatSource },
		{ "IEST_Player", IEST_Player },
		{ "IEST_Target", IEST_Target },
		{ "IEST_AIAlert", IEST_AIAlert },
		// basic resource types
		{ "GO_RSRC_MESH", GO_RSRC_MESH },
		{ "GO_RSRC_LIGHT", GO_RSRC_LIGHT },
		{ "GO_RSRC_PSYS", GO_RSRC_PSYS },
		{ "GO_RSRC_RBODY", GO_RSRC_RBODY },
		{ "GO_RSRC_JOINT", GO_RSRC_JOINT },
		{ "GO_RSRC_SNDSRC", GO_RSRC_SNDSRC },
		{ "GO_RSRC_REFPLANE", GO_RSRC_REFPLANE },
		{ "GO_RSRC_CAMERA", GO_RSRC_CAMERA },
		{ "GO_RSRC_FLARE", GO_RSRC_FLARE },
		// rigid body resource shape types
		{ "ShapeType_AABB", ShapeType_AABB },
		{ "ShapeType_Box", ShapeType_Box },
		{ "ShapeType_Sphere", ShapeType_Sphere },
		{ "ShapeType_Cylinder", ShapeType_Cylinder },
		{ "ShapeType_Capsule", ShapeType_Capsule },
		{ "ShapeType_Mesh", ShapeType_Mesh },
		//
		{ NULL, 0 },
	};
	sgs_RegIntConsts( C, ric, -1 );
	
	// init backing store
	m_metadata = m_scriptCtx.CreateDict();
	m_persistent = m_scriptCtx.CreateDict();
	
	// create entity type map
	AddEntry( "entity_types", m_scriptCtx.CreateDict() );
	
	// create behavior type map
	AddEntry( "behavior_types", m_scriptCtx.CreateDict() );
	
	// create marker pos. array
	m_markerPositions = m_scriptCtx.CreateDict();
	AddEntry( "positions", m_markerPositions );
	
	// register resource base class
	RegisterNativeClass<GOResource>( "GOResource" );
	
	// register core resources
	MeshResource::Register( this );
	LightResource::Register( this );
	FlareResource::Register( this );
	ParticleSystemResource::Register( this );
	SoundSourceResource::Register( this );
	RigidBodyResource::Register( this );
	ReflectionPlaneResource::Register( this );
	CameraResource::Register( this );
	
	// register core behaviors
	GOBehavior::Register( this );
	BhResourceMoveObject::Register( this );
	BhResourceMoveResource::Register( this );
	BhControllerBase::Register( this );
	
	// create the graphics scene
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
	m_systems.clear();
	
	m_sgsObject->data = NULL;
	m_sgsObject->iface = g_sgsobj_empty_handle;
	sgs_ObjRelease( C, m_sgsObject );
	
	// free all SGS objects before context destruction
	m_self._release();
	m_metadata._release();
	m_persistent._release();
	m_markerPositions._release();
	
	delete m_guiSys;
}

void GameLevel::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	switch( eid )
	{
	case EID_WindowEvent: {
			SGRX_CAST( Event*, ev, edata.GetUserData() );
			m_guiSys->EngineEvent( *ev );
		} break;
	case EID_GOResourceAdd: {
			SGRX_CAST( GOResource*, R, edata.GetUserData() );
			if( R->m_rsrcType == GO_RSRC_CAMERA )
				m_cameras.push_back( (CameraResource*) R );
		} break;
	case EID_GOResourceRemove: {
			SGRX_CAST( GOResource*, R, edata.GetUserData() );
			if( R->m_rsrcType == GO_RSRC_CAMERA )
				m_cameras.remove_first( (CameraResource*) R );
		} break;
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
	LoadingScreen( bool enable ) : m_running(enable), m_alpha(0), m_alphaTgt(1)
	{
		if( enable )
		{
			m_thread.Start( _Proc, this );
			sgrx_sleep( 500 );
		}
	}
	~LoadingScreen()
	{
		if( m_running )
		{
			m_alphaTgt = 0;
			sgrx_sleep( 500 );
			m_running = false;
		}
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

struct ScriptedLoadingScreen
{
	ScriptedLoadingScreen( bool enable, StringView levelname )
		: m_scriptCtx(NULL), m_running(enable)
	{
		if( enable )
		{
			m_scriptCtx = new ScriptContext;
			m_guiSys = new GameUISystem( m_scriptCtx );
			
			m_scriptCtx->Include( SGRXPATH__LEVELS "/default.loadscr" );
			
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, SGRXPATH__LEVELS "/%s.loadscr", StackPath( levelname ).str );
			m_scriptCtx->Include( bfr );
			
			int fade_in_time = m_scriptCtx->GetGlobal( "fade_in_time" ).get<float>() * 1000;
			m_thread.Start( _Proc, this );
			
			sgrx_sleep( fade_in_time );
		}
	}
	~ScriptedLoadingScreen()
	{
		if( m_running )
		{
			m_scriptMutex.Lock();
			
			m_scriptCtx->GlobalCall( "OnEnd" );
			int fade_out_time = m_scriptCtx->GetGlobal( "fade_out_time" ).get<float>() * 1000;
			
			m_scriptMutex.Unlock();
			
			sgrx_sleep( fade_out_time );
			m_running = false;
			m_thread.Join();
			
			delete m_guiSys;
			delete m_scriptCtx;
		}
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
			float deltaTime = ( newt - prevt );
			t += deltaTime;
			prevt = newt;
			
			m_scriptMutex.Lock();
			
			sgs_ProcessSubthreads( m_scriptCtx->C, deltaTime );
			m_guiSys->Draw( deltaTime );
			
			m_scriptMutex.Unlock();
			
			br.Flush();
			SGRX_Swap();
		}
	}
	
	static void _Proc( void* data )
	{
		((ScriptedLoadingScreen*)data)->Run();
	}
	
	SGRX_Thread m_thread;
	SGRX_Mutex m_scriptMutex;
	ScriptContext* m_scriptCtx;
	GameUISystem* m_guiSys;
	volatile bool m_running;
};



bool GameLevel::Load( const StringView& levelname )
{
	LOG_FUNCTION_ARG( levelname );
	
	ScriptedLoadingScreen LS( m_enableLoadingScreen, levelname );
	
	ByteArray ba;
	
	// load level file & scripts, clear previous data
	char bfr[ 256 ];
	{
		LOG_FUNCTION_ARG( "CORE" );
		
		sgrx_snprintf( bfr, sizeof(bfr), SGRXPATH__LEVELS "/%.*s" SGRX_LEVEL_COMPILED_SFX, TMIN( (int) levelname.size(), 200 ), levelname.data() );
		if( !FS_LoadBinaryFile( bfr, ba ) )
			return false;
	}
	
	{
		LOG_FUNCTION_ARG( "CLEAR" );
		ClearLevel();
	}
	
	m_levelName = levelname;
	LOG << "Loading level " << m_levelName;
	
	{
		LOG_FUNCTION_ARG( "SCRIPT" );
		
		sgrx_snprintf( bfr, sizeof(bfr), SGRXPATH__LEVELS "/%.*s", TMIN( (int) levelname.size(), 200 ), levelname.data() );
		m_scriptCtx.Include( bfr );
	}
	
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
		
		if( type == LC_FILE_GOBJ_NAME )
		{
			LOG_FUNCTION_ARG( "GAME OBJECTS" );
			
			HashTable< SGRX_GUID, MeshResource* > meshResources;
			HashTable< SGRX_GUID, sgsVariable > idVars;
			
			LC_Chunk_Gobj gobj;
			ByteReader gbr( ByteView( C.ptr, C.size ) );
			gbr << gobj;
			
			// create objects
			for( size_t i = 0; i < gobj.gameObjects.size(); ++i )
			{
				LC_GameObject& GO = gobj.gameObjects[ i ];
				
				nextObjectGUID = GO.guid;
				GameObject* obj = CreateGameObject();
				obj->SetName( GO.name );
				obj->SetID( GO.id );
				obj->SetLocalMatrix( GO.transform );
				obj->SetInfoMask( GO.infoMask );
				obj->SetInfoTarget( GO.localInfoTarget );
				
				if( obj->m_src_guid.NotNull() )
					idVars.set( obj->m_src_guid, obj->GetScriptedObject() );
				
				for( size_t j = 0; j < GO.srlz_resources.size(); ++j )
				{
					sgsVariable data = m_scriptCtx.Unserialize( GO.srlz_resources[ j ] );
				//	puts(sgs_DebugDumpVar(GetSGSC(), data.var));sgs_Pop(GetSGSC(),1);
					uint32_t type = data.getprop( "__type" ).get<uint32_t>();
					SGRX_GUID guid = SGRX_GUID::ParseString( data.getprop( "__guid" ).get_string().c_str() );
					
					nextObjectGUID = guid;
					GOResource* rsrc = obj->AddResource( type );
					if( rsrc )
					{
						if( guid.NotNull() )
							idVars.set( guid, rsrc->GetScriptedObject() );
						if( type == GO_RSRC_MESH && guid.NotNull() )
							meshResources.set( guid, (MeshResource*) rsrc );
						ScriptAssignProperties( rsrc->GetScriptedObject(), data, "__" );
					}
				}
				
				for( size_t j = 0; j < GO.srlz_behaviors.size(); ++j )
				{
					sgsVariable data = m_scriptCtx.Unserialize( GO.srlz_behaviors[ j ] );
				//	puts(sgs_DebugDumpVar(GetSGSC(), data.var));sgs_Pop(GetSGSC(),1);
					sgsString type = data.getprop( "__type" ).get_string();
					SGRX_GUID guid = SGRX_GUID::ParseString( data.getprop( "__guid" ).get_string().c_str() );
					
					nextObjectGUID = guid;
					GOBehavior* bhvr = obj->AddBehavior( type );
					if( bhvr )
					{
						if( guid.NotNull() )
							idVars.set( guid, bhvr->GetScriptedObject() );
						ScriptAssignProperties( bhvr->GetScriptedObject(), data, "__" );
					}
				}
			}
			
			// apply lightmaps
			for( size_t i = 0; i < gobj.lightmaps.size(); ++i )
			{
				LC_GOLightmap& LM = gobj.lightmaps[ i ];
				MeshResource* MR = meshResources.getcopy( LM.rsrc_guid );
				if( !MR )
					continue;
				if( !LM.lmap.width || !LM.lmap.height )
					continue;
				
				TextureHandle lmtex = GR_CreateTexture( LM.lmap.width, LM.lmap.height, TEXFMT_RGBA8,
					TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y, 1, LM.lmap.data.data() );
				MR->m_meshInst->SetMITexture( 0, lmtex );
				
				TextureHandle nmtex = GR_CreateTexture( LM.lmap.width, LM.lmap.height, TEXFMT_RGBA8,
					TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y, 1, LM.lmap.nmdata.data() );
				MR->m_meshInst->SetMITexture( 1, nmtex );
			}
			
			// apply links
			for( size_t i = 0; i < gobj.links.size(); ++i )
			{
				LC_GOLink& LNK = gobj.links[ i ];
				
				sgsVariable obj = idVars.getcopy( LNK.obj_guid );
				sgsVariable dst = idVars.getcopy( LNK.dst_guid );
				sgsVariable prop = GetScriptCtx().CreateString( LNK.prop ).get_variable();
				obj.setprop( prop, dst );
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

void GameLevel::ClearLevel()
{
	m_currentTickTime = 0;
	m_currentPhyTime = 0;
	
	for( size_t i = 0; i < m_gameObjects.size(); ++i )
		delete m_gameObjects[ i ];
	m_gameObjects.clear();
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->Clear();
	
	m_mainCamera._release();
	m_cameras.clear();
}


static int sort_cameras_by_depth( const void* A, const void* B )
{
	SGRX_CAST( CameraResource**, pCA, A );
	SGRX_CAST( CameraResource**, pCB, B );
	if( (*pCA)->depth != (*pCB)->depth )
		return (*pCA)->depth - (*pCB)->depth;
	return (int) ( pCA - pCB );
}

void GameLevel::_PreCallbackFixup()
{
	qsort( m_cameras.data(), m_cameras.size(), sizeof(CameraResource*), sort_cameras_by_depth );
	
	if( GetMainCamera() && GetMainCamera()->enabled == false )
	{
		m_mainCamera = sgsHandle<CameraResource>();
	}
	
	if( !GetMainCamera() )
	{
		for( size_t i = 0; i < m_cameras.size(); ++i )
		{
			if( m_cameras[ i ]->enabled )
			{
				m_mainCamera = sgsHandle<CameraResource>( m_cameras[ i ] );
				break;
			}
		}
	}
	
	if( GetMainCamera() )
		GetMainCamera()->GetCamera( m_scene->camera );
}


void GameLevel::FixedTick( float deltaTime )
{
	m_deltaTime = deltaTime;
	m_blendFactor = 1;
	m_fixedTickDeltaTime = deltaTime;
	
	if( IsPaused() == false )
	{
		m_currentTickTime += deltaTime;
		
		_PreCallbackFixup();
		
		m_eventType = LEV_PrePhysicsFixedUpdate;
		for( size_t i = 0; i < m_gameObjects.size(); ++i )
			m_gameObjects[ i ]->PrePhysicsFixedUpdate();
		
		m_eventType = LEV_FixedUpdate;
		
		int ITERS = 2;
		for( int i = 0; i < ITERS; ++i )
			m_phyWorld->Step( deltaTime / ITERS );
		
		_PreCallbackFixup();
		
		for( size_t i = 0; i < m_gameObjects.size(); ++i )
			m_gameObjects[ i ]->FixedUpdate();
		
		sgsVariable fn_onLevelFixedTick = m_scriptCtx.GetGlobal( "onLevelFixedTick" );
		if( fn_onLevelFixedTick.not_null() )
		{
			m_scriptCtx.Push( deltaTime );
			fn_onLevelFixedTick.call( C, 1 );
		}
	}
	m_eventType = LEV_None;
	
	_RemoveGameObjects( deltaTime );
}

void GameLevel::Tick( float deltaTime, float blendFactor )
{
	m_eventType = LEV_Update;
	m_deltaTime = deltaTime;
	m_blendFactor = blendFactor;
	m_tickDeltaTime = deltaTime;
	
	CameraResource* mainCamera = GetMainCamera();
	if( mainCamera )
	{
		Mat4 mtx = mainCamera->GetWorldMatrix();
		SGRX_Sound3DAttribs attr =
		{
			mtx.TransformPos( V3(0,0,0) ),
			V3(0),
			mtx.TransformNormal( V3(0,0,1) ).Normalized(),
			mtx.TransformNormal( V3(0,1,0) ).Normalized()
		};
		m_soundSys->Set3DAttribs( attr );
	}
	
	if( IsPaused() == false )
	{
		m_levelTime += deltaTime;
		m_currentPhyTime += deltaTime;
		
		_PreCallbackFixup();
		
		for( size_t i = 0; i < m_gameObjects.size(); ++i )
			m_gameObjects[ i ]->Update();
	}
	
	_PreCallbackFixup();
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->Tick( deltaTime, blendFactor );
	for( size_t i = 0; i < m_gameObjects.size(); ++i )
		m_gameObjects[ i ]->PreRender();
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->PreRender();
	
	sgs_ProcessSubthreads( m_scriptCtx.C, deltaTime );
	m_eventType = LEV_None;
	
	_RemoveGameObjects( deltaTime );
}

void GameLevel::Draw2D()
{
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	m_eventType = LEV_Draw2D;
	
	for( size_t i = 0; i < m_systems.size(); ++i )
		m_systems[ i ]->Draw2D();
	
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
		
		for( size_t i = 0; i < m_gameObjects.size(); ++i )
			m_gameObjects[ i ]->DebugDrawUI();
	}
	
	m_eventType = LEV_None;
}

void GameLevel::DebugDraw()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.lines.Flush();
	
	if( gcv_cl_debug.value )
	{
		m_eventType = LEV_DebugDraw;
		
		if( gcv_dbg_physics.value )
			m_phyWorld->DebugDraw();
		
		for( size_t i = 0; i < m_systems.size(); ++i )
			m_systems[ i ]->DebugDrawWorld();
		
		for( size_t i = 0; i < m_gameObjects.size(); ++i )
			m_gameObjects[ i ]->DebugDrawWorld();
		
		sgsVariable onLevelDebugDraw = m_scriptCtx.GetGlobal( "onLevelDebugDraw" );
		if( onLevelDebugDraw.not_null() )
			onLevelDebugDraw.call( GetSGSC() );
		
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
	
	m_eventType = LEV_None;
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



bool GameLevel::Query( GameObjectProcessor* optProc, uint32_t mask )
{
	return m_infoEmitSet.Query( InfoEmitGameObjectSet::NoTest(), mask, optProc );
}

bool GameLevel::QuerySphere( GameObjectProcessor* optProc, uint32_t mask, Vec3 pos, float rad )
{
	InfoEmitGameObjectSet::SphereTest test = { pos, rad * rad };
	return m_infoEmitSet.Query( test, mask, optProc );
}

bool GameLevel::QueryOBB( GameObjectProcessor* optProc, uint32_t mask, Mat4 mtx, Vec3 bbmin, Vec3 bbmax )
{
	InfoEmitGameObjectSet::OBBTest test = { bbmin, bbmax, mtx.Inverted() };
	return m_infoEmitSet.Query( test, mask, optProc );
}

struct EP_sgsFunc : GameObjectProcessor
{
	sgsVariable func;
	EP_sgsFunc( sgsVariable src ) : func( src ){}
	bool ProcessGameObject( GameObject* obj )
	{
		SGS_CSCOPE( func.get_ctx() );
		sgs_PushVar( func.get_ctx(), obj->GetScriptedObject() );
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


void GameLevel::PlaySound( StringView name, Vec3 pos, Vec3 dir )
{
	if( !m_soundSys )
		return;
	
	SoundEventInstanceHandle ev = m_soundSys->CreateEventInstance( name );
	if( ev )
	{
		SGRX_Sound3DAttribs attr = { pos, V3(0), dir, V3(0,0,1) };
		ev->Set3DAttribs( attr );
		ev->Start();
	}
}


void GameLevel::LightMesh( SGRX_MeshInstance* meshinst, Vec3 off )
{
	SGRX_LightSampler::LightMesh( meshinst, off );
}


void GameLevel::GetEditorCompilers( Array< IEditorSystemCompiler* >& out )
{
	for( size_t i = 0; i < m_systems.size(); ++i )
	{
		IEditorSystemCompiler* esc = m_systems[ i ]->EditorGetSystemCompiler();
		if( esc )
			out.push_back( esc );
	}
	out.push_back( new GameLevelSystemCompiler );
}



GameObject* GameLevel::CreateGameObject()
{
	GameObject* go = new GameObject( this );
	go->InitScriptInterface();
	m_gameObjects.push_back( go );
	if( nextObjectGUID.NotNull() )
	{
		go->m_src_guid = nextObjectGUID;
		nextObjectGUID.SetNull();
	}
	Game_FireEvent( EID_GOCreate, go );
	return go;
}

void GameLevel::DestroyGameObject( GameObject* obj, bool ch, float t )
{
	if( m_eventType != LEV_None || t )
	{
		GameObjRmInfo& info = m_gameObjectsToRemove[ obj ];
		info.t = t;
		if( ch )
			info.ch = true;
	}
	else
	{
		_RealDestroyGameObject( obj, ch );
	}
}

void GameLevel::_RealDestroyGameObject( GameObject* obj, bool ch )
{
	if( ch )
	{
		while( obj->_ch.size() )
		{
			_RealDestroyGameObject( (GameObject*) obj->_ch.last(), true );
		}
	}
	
	while( obj->m_behaviors.size() )
		obj->RemoveBehavior( obj->m_behaviors.last() );
	
	while( obj->m_resources.size() )
		obj->RemoveResource( obj->m_resources.last() );
	
	Game_FireEvent( EID_GODestroy, obj );
	obj->OnDestroy();
	delete obj;
	m_gameObjects.remove_first( obj );
}

void GameLevel::_RemoveGameObjects( float dt )
{
	for( size_t i = 0; i < m_gameObjectsToRemove.size(); ++i )
	{
		GameObject* obj = m_gameObjectsToRemove.item( i ).key;
		GameObjRmInfo& info = m_gameObjectsToRemove.item( i ).value;
		info.t -= dt;
		if( info.t <= 0 )
		{
			_RealDestroyGameObject( obj, info.ch );
			m_gameObjectsToRemove.unset( obj );
			i--;
		}
	}
}

sgsVariable GameLevel::sgsCreateGameObject()
{
	return CreateGameObject()->GetScriptedObject();
}

void GameLevel::sgsDestroyGameObject( sgsVariable oh, bool ch )
{
	GameObject* obj = oh.downcast<GameObject>();
	if( sgs_StackSize( C ) < 2+1 )
		ch = true;
	if( obj )
		DestroyGameObject( obj, ch );
}

void GameLevel::_MapGameObjectByID( GameObject* obj )
{
	m_gameObjIDMap[ obj->GetID() ] = obj;
}

void GameLevel::_UnmapGameObjectByID( GameObject* obj )
{
	m_gameObjIDMap.unset( obj->GetID() );
}

GameObject* GameLevel::FindGameObjectByID( const StringView& name )
{
	return m_gameObjIDMap.getcopy( name );
}

GameObject::ScrHandle GameLevel::sgsFindGameObject( StringView name )
{
	return GameObject::ScrHandle( FindGameObjectByID( name ) );
}

void GameLevel::EnumBehaviors( Array< StringView >& out )
{
	sgsVariable bhvrarr = m_self.getprop( "behavior_types" );
	ScriptVarIterator it( bhvrarr );
	while( it.Advance() )
	{
		out.push_back( it.GetKey().get<StringView>() );
	}
}

sgsVariable GameLevel::GetBehaviorInterface( StringView name )
{
	return m_self.getprop( "behavior_types" ).getprop( m_scriptCtx.CreateString( name ) );
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


static int SetOverlayMusic( SGS_CTX )
{
	SGRX_CAST( BaseGame*, g, Game_Get().item );
	g->SetOverlayMusic( sgs_GetVar<StringView>()( C, 0 ) );
	return 0;
}

static sgs_RegFuncConst basegame_api[] =
{
	{ "SetOverlayMusic", SetOverlayMusic },
	{ NULL, NULL },
};

BaseGame::BaseGame() :
	m_maxTickSize( 1.0f/15.0f ),
	m_fixedTickSize( 1.0f/30.0f ),
	m_accum( 0 ),
	m_timeMultiplier( 1 ),
	m_level( NULL ),
	m_editor( NULL ),
	m_needsEditor( false )
{
	m_mapName = "<UNSPECIFIED>";
	ParseConfigFile();
}

int BaseGame::OnArgument( char* arg, int argcleft, char** argvleft )
{
	if( streq( arg, "EDIT" ) )
	{
		m_needsEditor = true;
		return 1;
	}
	if( strpeq( arg, "-map=" ) )
	{
		m_mapName = arg + 5;
	}
	return IGame::OnArgument( arg, argcleft, argvleft );
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
	InitSoundSystem();
	m_level = CreateLevel();
	return true;
}

void BaseGame::OnDestroy()
{
	SAFE_DELETE( m_level );
	SetOverlayMusic( SV() );
	m_soundSys = NULL;
}

void BaseGame::InitSoundSystem()
{
	if( m_soundSys == NULL )
	{
		m_soundSys = CreateSoundSystem();
		
		StringView it = m_soundBanks;
		while( it.size() )
		{
			m_soundSys->Load( it.until( ":" ) );
			it = it.after( ":" );
		}
	}
}

void BaseGame::ParseConfigFile()
{
	LOG_FUNCTION;
	
	String text;
	if( !FS_LoadTextFile( "game.cfg", text ) )
	{
		LOG_ERROR << "Failed to load game.cfg";
		return;
	}
	
	ConfigReader cr( text );
	StringView key, value;
	while( cr.Read( key, value ) )
	{
		if( key == "startuplevel" )
		{
			m_mapName = value;
		}
		else if( key == "soundbank" )
		{
			if( m_soundBanks.size() )
				m_soundBanks.append( ":" );
			m_soundBanks.append( value );
		}
		else if( key == "font" )
		{
			StringView name = value.until( "=" );
			StringView path = value.after( "=" );
			GR2D_LoadFont( name, path );
		}
		else if( key == "svgfont" )
		{
			StringView name = value.until( "=" );
			StringView path = value.after( "=" );
			GR2D_LoadSVGIconFont( name, path );
		}
		else if( key == "action" )
		{
			StringView name = value.until( "," );
			StringView threshold = value.after( "," ).after_all( HSPACE_CHARS );
			Game_AddAction( name, threshold.size() ? String_ParseFloat( threshold ) : 0.25f );
		}
		else if( key == "module" || key == "dir2" || key == "renderer" )
		{
			// already parsed by engine core
		}
		else
		{
			LOG_WARNING << "Unknown key (" << key << " = " << value << ")";
		}
	}
}

GameLevel* BaseGame::CreateLevel()
{
	InitSoundSystem();
	GameLevel* level = new GameLevel( CreatePhyWorld() );
	level->m_soundSys = m_soundSys;
	sgs_RegFuncConsts( level->GetSGSC(), basegame_api, -1 );
	level->SetGlobalToSelf();
	level->GetPhyWorld()->SetGravity( V3( 0, 0, -9.81f ) );
	return level;
}

void BaseGame::OnLevelChange()
{
	String nextLevel = m_level->m_nextLevel;
	String persistentData = m_level->GetScriptCtx().Serialize( m_level->m_persistent );
	
	delete m_level;
	
	m_level = CreateLevel();
	m_level->m_persistent = m_level->GetScriptCtx().Unserialize( persistentData );
	m_level->Load( nextLevel );
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
	
	// events
	if( m_level->m_nextLevel.size() )
	{
		OnLevelChange();
	}
	
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

void BaseGame::SetOverlayMusic( StringView path )
{
	if( !m_soundSys )
	{
		LOG_WARNING << "BaseGame::SetOverlayMusic - no sound system";
		return;
	}
	
	if( m_ovrMusicPath == path )
	{
		return;
	}
	m_ovrMusicPath = path;
	
	if( m_ovrMusic )
	{
		m_ovrMusic->Stop();
	}
	m_ovrMusic = NULL;
	if( path )
	{
		m_ovrMusic = m_soundSys->CreateEventInstance( path );
		m_ovrMusic->Start();
	}
}


