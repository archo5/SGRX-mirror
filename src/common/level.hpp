

#pragma once

#include "gfwcore.hpp"
#include "levelcache.hpp"
#include "gamegui.hpp"


extern GFW_EXPORT CVarBool gcv_cl_gui;
extern GFW_EXPORT CVarBool gcv_cl_debug;
extern GFW_EXPORT CVarBool gcv_g_paused;

GFW_EXPORT void RegisterCommonGameCVars();


struct GameLevel;
typedef uint32_t TimeVal;


extern sgs_ObjInterface g_sgsobj_empty_handle[1];


enum CoreLevelEventIDs
{
	EID_Type_CoreLevel = 9000,
	EID_GOCreate,
	EID_GODestroy,
	EID_GOResourceAdd,
	EID_GOResourceRemove,
	EID_GOResourceUpdate,
	EID_GOBehaviorAdd,
	EID_GOBehaviorRemove,
};

enum LevelEventType
{
	LEV_None = 0,
	LEV_PrePhysicsFixedUpdate,
	LEV_FixedUpdate,
	LEV_Update,
	LEV_Draw2D,
	LEV_DebugDraw,
};


EXP_STRUCT LevelScrObj : SGRX_RefCounted
{
	SGS_OBJECT SGS_NO_DESTRUCT;
	typedef sgsHandle< LevelScrObj > ScrHandle;
	
	GFW_EXPORT LevelScrObj( GameLevel* lev );
	GFW_EXPORT virtual ~LevelScrObj();
	
	template< class T > void _InitScriptInterface( T* ptr );
	GFW_EXPORT void DestroyScriptInterface();
#define ENT_SGS_IMPLEMENT \
	virtual void InitScriptInterface(){ if( m_sgsObject == NULL ) _InitScriptInterface( this ); } \
	virtual sgs_ObjInterface* GetSGSInterface() const { return _sgs_interface; }
	
	ENT_SGS_IMPLEMENT;
	
	sgsVariable GetScriptedObject(){ return ScrHandle( this ).get_variable(); }
	GFW_EXPORT void AddSelfToLevel( StringView name );
	
	virtual void* GetInterfaceImpl( uint32_t iface_id ){ return NULL; }
	template< class T > T* GetInterface(){ return (T*) GetInterfaceImpl( T::e_iface_uid ); }
#define ENT_HAS_INTERFACE( T, reqid, ptr ) if( T::e_iface_uid == reqid ) return (T*) ptr
	
	sgsHandle< GameLevel > _sgs_getLevel();
	SGS_PROPERTY_FUNC( READ _sgs_getLevel ) SGS_ALIAS( sgsHandle< GameLevel > level );
	
	SGS_PROPERTY sgsVariable _data;
	SGS_BACKING_STORE( _data.var );
	
	GameLevel* m_level;
};


EXP_STRUCT Transform
{
	Transform() :
		_localPosition( V3(0) ),
		_localRotation( Quat::Identity ),
		_localScale( V3(1) ),
		_parent( NULL ),
		_worldMatrix( Mat4::Identity ),
		_invWorldMatrix( Mat4::Identity ),
		_outdated( true ),
		_inTransformUpdate( false ),
		_destroying( false ),
		_xfChangeInvoker( NULL )
	{}
	~Transform()
	{
		_destroying = true;
		while( _ch.size() )
			_ch.last()->_SetParent( NULL, true );
		_SetParent( NULL );
	}
	
	Vec3 _localPosition;
	Quat _localRotation;
	Vec3 _localScale;
	Transform* _parent;
	Array< Transform* > _ch;
	mutable Mat4 _worldMatrix;
	mutable Mat4 _invWorldMatrix;
	mutable bool _outdated;
	bool _inTransformUpdate;
	bool _destroying;
	const void* _xfChangeInvoker;
	
	GFW_EXPORT virtual void OnTransformUpdate() = 0;
	void _OnTransformUpdate()
	{
		if( _destroying )
			return;
		if( _inTransformUpdate )
		{
			LOG_WARNING << "OnTransformUpdate recursion - transform modified in callback";
			return;
		}
		_inTransformUpdate = true;
		OnTransformUpdate();
		_inTransformUpdate = false;
	}
	
	FINLINE Mat4 _GetInvParentMtx()
	{
		return _parent ? _parent->_invWorldMatrix : Mat4::Identity;
	}
	FINLINE void _SetParent( Transform* nptf, bool preserveWorldTransform = false )
	{
		if( _parent == nptf )
			return;
		
		// check 1
		Transform* pp = nptf;
		while( pp )
		{
			if( pp == this )
			{
				LOG_WARNING << "SetParent - tried to set self or child as parent";
				return;
			}
			pp = pp->_parent;
		}
		
		if( _parent )
		{
			_parent->_ch.remove_first( this );
			_parent = NULL;
		}
		if( nptf )
		{
			_parent = nptf;
			nptf->_ch.push_back( this );
		}
		if( preserveWorldTransform )
			SetWorldMatrix( _worldMatrix );
		OnEdit();
	}
#define TF_ONREAD if(_outdated) OnRead();
	void OnRead() const
	{
		_worldMatrix = GetLocalMatrix();
		if( _parent )
			_worldMatrix = _worldMatrix * _parent->GetWorldMatrix();
		_invWorldMatrix = _worldMatrix.Inverted();
		_outdated = false;
	}
	void OnEdit()
	{
		_outdated = true;
		_OnTransformUpdate();
		for( size_t i = 0; i < _ch.size(); ++i )
			_ch[ i ]->OnEdit();
	}
	
	FINLINE Mat4 GetLocalMatrix() const
	{
		return Mat4::CreateSRT( _localScale, _localRotation, _localPosition );
	}
	FINLINE void SetLocalMatrix( Mat4 m )
	{
		_localPosition = m.GetTranslation();
		_localRotation = m.GetRotationQuaternion();
		_localScale = m.GetScale();
		OnEdit();
	}
	FINLINE Vec3 GetLocalPosition() const { return _localPosition; }
	FINLINE void SetLocalPosition( Vec3 p ){ _localPosition = p; OnEdit(); }
	FINLINE Quat GetLocalRotation() const { return _localRotation; }
	FINLINE void SetLocalRotation( Quat q ){ _localRotation = q; OnEdit(); }
	FINLINE Vec3 GetLocalRotationXYZ() const { return RAD2DEG( _localRotation.ToXYZ() ); }
	FINLINE void SetLocalRotationXYZ( Vec3 v ){ _localRotation = Quat::CreateFromXYZ( DEG2RAD( v ) ); OnEdit(); }
	FINLINE Vec3 GetLocalScale() const { return _localScale; }
	FINLINE void SetLocalScale( Vec3 s ){ _localScale = s; OnEdit(); }
	
	FINLINE Vec3 GetWorldPosition() const { TF_ONREAD; return _worldMatrix.GetTranslation(); }
	FINLINE Quat GetWorldRotation() const { TF_ONREAD; return _worldMatrix.GetRotationQuaternion(); }
	FINLINE Vec3 GetWorldRotationXYZ() const { TF_ONREAD; return RAD2DEG( _worldMatrix.GetRotationQuaternion().ToXYZ() ); }
	FINLINE Vec3 GetWorldScale() const { TF_ONREAD; return _worldMatrix.GetScale(); }
	void SetWorldPosition( Vec3 v ){ _localPosition += _GetInvParentMtx().TransformNormal( v - GetWorldPosition() ); OnEdit(); }
	FINLINE Mat4 GetWorldMatrix() const { TF_ONREAD; return _worldMatrix; }
	void SetWorldMatrix( Mat4 mtx ){ SetLocalMatrix( mtx * _GetInvParentMtx() ); }
	
	FINLINE Vec3 LocalToWorld( Vec3 p ) const { TF_ONREAD; return _worldMatrix.TransformPos( p ); }
	FINLINE Vec3 WorldToLocal( Vec3 p ) const { TF_ONREAD; return _invWorldMatrix.TransformPos( p ); }
	FINLINE Vec3 LocalToWorldDir( Vec3 d ) const { TF_ONREAD; return _worldMatrix.TransformNormal( d ); }
	FINLINE Vec3 WorldToLocalDir( Vec3 d ) const { TF_ONREAD; return _invWorldMatrix.TransformNormal( d ); }
#undef TF_ONREAD
};


#define ENTITY_IS_A( ent, cls ) ((ent)->GetSGSInterface() == cls::_sgs_interface)


#define IEST_InteractiveItem 0x0001
#define IEST_HeatSource      0x0002
#define IEST_Player          0x0004
#define IEST_Target          0x0010
#define IEST_AIAlert         0x0020


struct EditorEntity
{
	sgsVariable sysParams;
	StringView type;
	sgsVariable props;
};

struct IF_GCC(GFW_EXPORT) IEditorSystemCompiler
{
	virtual ~IEditorSystemCompiler();
	virtual bool GenerateChunk( ByteArray& out, sgsVariable sysParams ){ return false; }
	virtual void ProcessEntity( EditorEntity& ent ) = 0;
	
	GFW_EXPORT void WrapChunk( ByteArray& chunk, const char id[4] );
};

EXP_STRUCT IGameLevelSystem : LevelScrObj
{
	SGS_OBJECT_INHERIT( LevelScrObj ) SGS_NO_DESTRUCT;
	
	IGameLevelSystem( GameLevel* lev, uint32_t uid ) :
		LevelScrObj( lev ), m_system_uid( uid )
	{}
	virtual void OnPostLevelLoad(){}
	virtual void OnLevelDestroy(){ delete this; }
	virtual bool LoadChunk( const StringView& type, ByteView data ){ return false; }
	virtual void Clear(){}
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual void PreRender(){}
	virtual void Draw2D(){}
	virtual void DrawUI(){}
	
	virtual void PostDraw(){}
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	virtual IEditorSystemCompiler* EditorGetSystemCompiler(){ return NULL; }
	
	uint32_t m_system_uid;
};


struct GameObject;

struct EditorUIHelper
{
	enum PickerType
	{
		PT_Mesh,
		PT_PartSys,
		PT_Texture,
		PT_Char,
		PT_Sound,
	};
	
	virtual bool ResourcePicker( PickerType ptype,
		const char* caption, const char* label, String& value ) = 0;
};

struct GOResourceInfo
{
	const char* name;
	struct GOResource* (*createFunc)( struct GameObject* );
};

typedef struct GOBehavior* GOBehaviorCreateFunc( struct GameObject* );

#define IMPLEMENT_RESOURCE( cls, id, nm ) \
	static GOResource* _Create( GameObject* go ){ return new cls( go ); } \
	static void Register( GameLevel* lev ){ \
		GOResourceInfo ri = { nm, _Create }; \
		lev->RegisterNativeClass< cls >( _sgs_interface->name ); \
		lev->m_goResourceMap.set( id, ri ); }

#define IMPLEMENT_BEHAVIOR( cls ) \
	static GOBehavior* _Create( GameObject* go ){ return new cls( go ); } \
	static void Register( GameLevel* lev ){ \
		lev->_RegisterNativeBehavior< cls >( _sgs_interface->name ); \
		lev->m_goNativeBhvrMap.set( _sgs_interface->name, _Create ); }

typedef IGameLevelSystem* GameLevelSystemCreateFunc( GameLevel* );
#define IMPLEMENT_SYSTEM( cls, id ) \
	enum { e_system_uid = id }; \
	static StringView _Name(){ return #cls; } \
	static IGameLevelSystem* _Create( GameLevel* lev ){ return new cls( lev ); }

EXP_STRUCT GOResource : LevelScrObj
{
	SGS_OBJECT_INHERIT( LevelScrObj ) SGS_NO_DESTRUCT;
	ENT_SGS_IMPLEMENT;
	typedef sgsHandle< GOResource > ScrHandle;
	
	GFW_EXPORT GOResource( GameObject* obj );
	GFW_EXPORT virtual SGS_METHOD void OnDestroy();
	GFW_EXPORT virtual SGS_METHOD void PrePhysicsFixedUpdate();
	GFW_EXPORT virtual SGS_METHOD void FixedUpdate();
	GFW_EXPORT virtual SGS_METHOD void Update();
	GFW_EXPORT virtual SGS_METHOD void PreRender();
	GFW_EXPORT virtual SGS_METHOD void OnTransformUpdate();
	
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	GFW_EXPORT virtual void EditUI( EditorUIHelper* uih, sgsVariable iface );
	GFW_EXPORT virtual void EditLoad( sgsVariable src, sgsVariable iface );
	GFW_EXPORT virtual void EditSave( sgsVariable out, sgsVariable iface );
	GFW_EXPORT virtual void EditorDrawWorld();
	GFW_EXPORT virtual Vec3 EditorIconPos();
	
	GFW_EXPORT Mat4 MatrixResourceToObject( Mat4 xf ) const;
	GFW_EXPORT virtual SGS_METHOD Mat4 GetWorldMatrix() const;
	Mat4 GetLocalMatrix() const { return m_localMatrix; }
	void SetLocalMatrix( Mat4 m ){ m_localMatrix = m; OnTransformUpdate(); }
	int GetMatrixMode() const { return m_matrixMode; }
	void SetMatrixMode( int v ){ m_matrixMode = v; OnTransformUpdate(); }
	sgsHandle<GameObject> _get_object(){ return sgsHandle<GameObject>( m_obj ); }
	SGS_PROPERTY_FUNC( READ _get_object ) SGS_ALIAS( sgsHandle<GameObject> object );
	
	SGS_PROPERTY_FUNC( READ VARNAME __type ) uint32_t m_rsrcType;
	SGS_PROPERTY_FUNC( READ SOURCE m_src_guid.ToString() ) SGS_ALIAS( sgsString __guid );
	
	FINLINE Vec3 GetLocalPosition() const { return m_localMatrix.GetTranslation(); }
	FINLINE Quat GetLocalRotation() const { return m_localMatrix.GetRotationQuaternion(); }
	FINLINE Vec3 GetLocalRotationXYZ() const { return RAD2DEG( m_localMatrix.GetRotationQuaternion().ToXYZ() ); }
	FINLINE Vec3 GetLocalScale() const { return m_localMatrix.GetScale(); }
	
	SGS_PROPERTY_FUNC( READ GetLocalPosition ) SGS_ALIAS( Vec3 localPosition );
	SGS_PROPERTY_FUNC( READ GetLocalRotation ) SGS_ALIAS( Quat localRotation );
	SGS_PROPERTY_FUNC( READ GetLocalRotationXYZ ) SGS_ALIAS( Vec3 localRotationXYZ );
	SGS_PROPERTY_FUNC( READ GetLocalScale ) SGS_ALIAS( Vec3 localScale );
	SGS_PROPERTY_FUNC( READ GetLocalMatrix WRITE SetLocalMatrix VARNAME localMatrix ) Mat4 m_localMatrix;
	SGS_PROPERTY_FUNC( READ GetMatrixMode WRITE SetMatrixMode VARNAME matrixMode ) int m_matrixMode;
	
	GameObject* m_obj;
	SGRX_GUID m_src_guid;
};
typedef Handle< GOResource > H_GOResource;

EXP_STRUCT GOBehavior : LevelScrObj
{
	SGS_OBJECT_INHERIT( LevelScrObj ) SGS_NO_DESTRUCT;
	ENT_SGS_IMPLEMENT;
	typedef sgsHandle< GOBehavior > ScrHandle;
	
	// implementation
	GFW_EXPORT static GOBehavior* _Create( GameObject* go );
	GFW_EXPORT static void Register( GameLevel* lev );
	// --
	
	GFW_EXPORT GOBehavior( GameObject* obj );
	
	GFW_EXPORT virtual void Init();
	GFW_EXPORT virtual void OnDestroy();
	GFW_EXPORT virtual void PrePhysicsFixedUpdate();
	GFW_EXPORT virtual void FixedUpdate();
	GFW_EXPORT virtual void Update();
	GFW_EXPORT virtual void PreRender();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT virtual void OnIDUpdate();
	GFW_EXPORT virtual SGS_METHOD_NAMED( SendMessage )
		void sgsSendMessage( sgsString name, sgsVariable arg );
	GFW_EXPORT void SendMessage( StringView name, sgsVariable arg = sgsVariable() );
	
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	GFW_EXPORT virtual void EditUI( EditorUIHelper* uih, sgsVariable iface );
	GFW_EXPORT virtual void EditLoad( sgsVariable src, sgsVariable iface );
	GFW_EXPORT virtual void EditSave( sgsVariable out, sgsVariable iface );
	GFW_EXPORT virtual void EditorDrawWorld();
	
	GameObject* m_obj;
	SGS_PROPERTY_FUNC( READ VARNAME __type ) sgsString m_type;
	SGRX_GUID m_src_guid;
	SGS_PROPERTY_FUNC( READ SOURCE m_src_guid.ToString() ) SGS_ALIAS( sgsString __guid );
	
	sgsHandle<GameObject> _get_object(){ return sgsHandle<GameObject>( m_obj ); }
	SGS_PROPERTY_FUNC( READ _get_object ) SGS_ALIAS( sgsHandle<GameObject> object );
};
typedef Handle< GOBehavior > H_GOBehavior;

EXP_STRUCT GameObject : LevelScrObj, Transform
{
	SGS_OBJECT_INHERIT( LevelScrObj ) SGS_NO_DESTRUCT;
	ENT_SGS_IMPLEMENT;
	typedef sgsHandle< GameObject > ScrHandle;
	
	GFW_EXPORT GameObject( GameLevel* lev );
	GFW_EXPORT ~GameObject();
	
	GFW_EXPORT GOResource* AddResource( uint32_t type );
	GFW_EXPORT GOResource* RequireResource( uint32_t type, bool retifnc = false );
	GFW_EXPORT void RemoveResource( GOResource* rsrc );
	GFW_EXPORT SGS_METHOD void RemoveResource( GOResource::ScrHandle rsrc );
	
	GFW_EXPORT GOBehavior* AddBehavior( sgsString type );
	GFW_EXPORT GOBehavior* RequireBehavior( sgsString type, bool retifnc = false );
	GFW_EXPORT GOBehavior* _CreateBehaviorReal( sgsString type );
	GFW_EXPORT void RemoveBehavior( GOBehavior* bhvr );
	GFW_EXPORT SGS_METHOD void RemoveBehavior( GOBehavior::ScrHandle bhvr );
	
	GFW_EXPORT virtual void OnDestroy();
	GFW_EXPORT virtual void PrePhysicsFixedUpdate();
	GFW_EXPORT virtual void FixedUpdate();
	GFW_EXPORT virtual void Update();
	GFW_EXPORT virtual void PreRender();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT virtual void OnIDUpdate();
	GFW_EXPORT virtual SGS_METHOD_NAMED( SendMessage )
		void sgsSendMessage( sgsString name, sgsVariable arg );
	GFW_EXPORT void SendMessage( StringView name, sgsVariable arg = sgsVariable() );
	
	GFW_EXPORT virtual void DebugDrawWorld();
	GFW_EXPORT virtual void DebugDrawUI();
	
	GFW_EXPORT virtual void EditorDrawWorld();
	
	SGS_PROPERTY_FUNC( READ SOURCE m_src_guid.ToString() ) SGS_ALIAS( sgsString __guid );
	Array< H_GOResource > m_resources;
	Array< H_GOBehavior > m_behaviors;
	SGRX_GUID m_src_guid;
	
	// transforms
	SGS_PROPERTY_FUNC( READ GetWorldPosition WRITE SetWorldPosition ) SGS_ALIAS( Vec3 position );
	SGS_PROPERTY_FUNC( READ GetWorldRotation WRITE SetLocalRotation ) SGS_ALIAS( Quat rotation );
	SGS_PROPERTY_FUNC( READ GetWorldRotationXYZ WRITE SetLocalRotationXYZ ) SGS_ALIAS( Vec3 rotationXYZ );
	SGS_PROPERTY_FUNC( READ GetWorldScale WRITE SetLocalScale ) SGS_ALIAS( Vec3 scale );
	SGS_PROPERTY_FUNC( READ GetWorldMatrix WRITE SetWorldMatrix ) SGS_ALIAS( Mat4 transform );
	
	SGS_PROPERTY_FUNC( READ GetLocalPosition WRITE SetLocalPosition ) SGS_ALIAS( Vec3 localPosition );
	SGS_PROPERTY_FUNC( READ GetLocalRotation WRITE SetLocalRotation ) SGS_ALIAS( Quat localRotation );
	SGS_PROPERTY_FUNC( READ GetLocalRotationXYZ WRITE SetLocalRotationXYZ ) SGS_ALIAS( Vec3 localRotationXYZ );
	SGS_PROPERTY_FUNC( READ GetLocalScale WRITE SetLocalScale ) SGS_ALIAS( Vec3 localScale );
	SGS_PROPERTY_FUNC( READ GetLocalMatrix WRITE SetLocalMatrix ) SGS_ALIAS( Mat4 localTransform );
	
	SGS_METHOD SGS_ALIAS( Vec3 LocalToWorld( Vec3 p ) );
	SGS_METHOD SGS_ALIAS( Vec3 WorldToLocal( Vec3 p ) );
	SGS_METHOD SGS_ALIAS( Vec3 LocalToWorldDir( Vec3 p ) );
	SGS_METHOD SGS_ALIAS( Vec3 WorldToLocalDir( Vec3 p ) );
	
	// parent-child relation
	GameObject* GetParent() const { return (GameObject*) _parent; }
	void SetParent( GameObject* obj, bool preserveWorldTransform = true ){ _SetParent( obj, preserveWorldTransform ); }
	ScrHandle _sgsGetParent(){ return ScrHandle( (GameObject*) _parent ); }
	FINLINE GameObject* GetChild( size_t i ) const
	{
		if( i >= _ch.size() )
			return NULL;
		return (GameObject*) _ch[ i ];
	}
	FINLINE size_t GetChildCount() const { return _ch.size(); }
	SGS_PROPERTY_FUNC( READ _sgsGetParent WRITE _SetParent ) SGS_ALIAS( ScrHandle parent );
	FINLINE SGS_METHOD_NAMED( SetParent ) void sgsSetParent( GameObject::ScrHandle parent, bool preserve ){ _SetParent( parent, preserve ); }
	
	FINLINE SGS_METHOD_NAMED( GetChild ) ScrHandle sgsGetChild( int i )
	{
		if( i < 0 || i >= (int) _ch.size() )
		{
			sgs_Msg( C, SGS_WARNING, "child index (%d) out of bounds [0;%d)", i, (int) _ch.size() );
			return ScrHandle();
		}
		return ScrHandle( (GameObject*) _ch[ i ] );
	}
	SGS_PROPERTY_FUNC( READ SOURCE _ch.size() ) SGS_ALIAS( int childCount );
	
	// resources / behaviors
	SGS_METHOD_NAMED( AddResource ) sgsVariable sgsAddResource( uint32_t type )
	{ GOResource* rsrc = AddResource( type ); return rsrc ? rsrc->GetScriptedObject() : sgsVariable(); }
	SGS_METHOD_NAMED( AddBehavior ) sgsVariable sgsAddBehavior( sgsString type )
	{ GOBehavior* bhvr = AddBehavior( type ); return bhvr ? bhvr->GetScriptedObject() : sgsVariable(); }
	SGS_METHOD_NAMED( RequireResource ) sgsVariable sgsRequireResource( uint32_t type, bool retifnc )
	{ GOResource* rsrc = RequireResource( type, sgs_StackSize( C ) >= 2 ? retifnc : true );
		return rsrc ? rsrc->GetScriptedObject() : sgsVariable(); }
	SGS_METHOD_NAMED( RequireBehavior ) sgsVariable sgsRequireBehavior( sgsString type, bool retifnc )
	{ GOBehavior* bhvr = RequireBehavior( type, sgs_StackSize( C ) >= 2 ? retifnc : true );
		return bhvr ? bhvr->GetScriptedObject() : sgsVariable(); }
	SGS_METHOD_NAMED( GetResourceCount ) int sgsGetResourceCount() const { return m_resources.size(); }
	SGS_METHOD_NAMED( GetBehaviorCount ) int sgsGetBehaviorCount() const { return m_behaviors.size(); }
	SGS_METHOD_NAMED( GetResourceByNum ) GOResource::ScrHandle sgsGetResourceByNum( int i )
	{
		if( i < 0 || i >= int(m_resources.size()) )
			return GOResource::ScrHandle( m_resources[ i ] );
		return GOResource::ScrHandle();
	}
	SGS_METHOD_NAMED( GetBehaviorByNum ) GOBehavior::ScrHandle sgsGetBehaviorByNum( int i )
	{
		if( i < 0 || i >= int(m_behaviors.size()) )
			return GOBehavior::ScrHandle( m_behaviors[ i ] );
		return GOBehavior::ScrHandle();
	}
	
	GOResource* FindFirstResourceOfTypeID( uint32_t type, int skip = 0 )
	{
		for( size_t i = 0; i < m_resources.size(); ++i )
		{
			GOResource* rsrc = m_resources[ i ];
			if( rsrc->m_rsrcType == type )
			{
				if( skip --> 0 )
					continue;
				return rsrc;
			}
		}
		return NULL;
	}
	GOBehavior* FindFirstBehaviorOfTypeName( sgsString type, int skip = 0 )
	{
		for( size_t i = 0; i < m_behaviors.size(); ++i )
		{
			GOBehavior* bhvr = m_behaviors[ i ];
			if( bhvr->m_type == type )
			{
				if( skip --> 0 )
					continue;
				return bhvr;
			}
		}
		return NULL;
	}
	template< class T > T* FindFirstResourceOfType( int skip = 0 )
	{
		for( size_t i = 0; i < m_resources.size(); ++i )
		{
			GOResource* rsrc = m_resources[ i ];
			T* trh = sgsHandle<T>( rsrc->C, rsrc->m_sgsObject );
			if( trh )
			{
				if( skip --> 0 )
					continue;
				return trh;
			}
		}
		return NULL;
	}
	template< class T > T* FindFirstBehaviorOfType()
	{
		for( size_t i = 0; i < m_behaviors.size(); ++i )
		{
			GOBehavior* bhvr = m_behaviors[ i ];
			T* tbh = sgsHandle<T>( bhvr->C, bhvr->m_sgsObject );
			if( tbh )
				return tbh;
		}
		return NULL;
	}
	SGS_METHOD_NAMED( FindFirstResourceOfType ) GOResource::ScrHandle sgsFindFirstResourceOfType( sgsVariable typeOrMetaObj );
	SGS_METHOD_NAMED( FindFirstBehaviorOfType ) GOBehavior::ScrHandle sgsFindFirstBehaviorOfType( sgsVariable typeOrMetaObj );
	SGS_METHOD_NAMED( FindAllResourcesOfType ) sgsVariable sgsFindAllResourcesOfType( sgsVariable typeOrMetaObj );
	SGS_METHOD_NAMED( FindAllBehaviorsOfType ) sgsVariable sgsFindAllBehaviorsOfType( sgsVariable typeOrMetaObj );
	
	// info target
	FINLINE uint32_t GetInfoMask() const { return m_infoMask; }
	GFW_EXPORT void SetInfoMask( uint32_t mask );

	FINLINE Vec3 GetInfoTarget() const { return m_infoTarget; }
	FINLINE void SetInfoTarget( Vec3 tgt ){ m_infoTarget = tgt; }
	FINLINE Vec3 GetWorldInfoTarget() const { return LocalToWorld( m_infoTarget ); }

	SGS_PROPERTY_FUNC( READ GetInfoMask WRITE SetInfoMask VARNAME infoMask ) uint32_t m_infoMask;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME localInfoTarget ) Vec3 m_infoTarget;
	SGS_PROPERTY_FUNC( READ GetWorldInfoTarget ) SGS_ALIAS( Vec3 infoTarget );
	
	// id/name
	FINLINE StringView GetName() const { return StringView( m_name.c_str(), m_name.size() ); }
	FINLINE void SetName( StringView nm );
	FINLINE StringView GetID() const { return StringView( m_id.c_str(), m_id.size() ); }
	FINLINE void SetID( StringView id );
	FINLINE void sgsSetID( sgsString id );
	
	SGS_PROPERTY_FUNC( READ WRITE VARNAME name ) sgsString m_name;
	SGS_PROPERTY_FUNC( READ WRITE sgsSetID VARNAME id ) sgsString m_id;
};






struct GameObjectProcessor
{
	virtual bool ProcessGameObject( GameObject* obj ) = 0;
};

struct GameObjectGather : GameObjectProcessor
{
	struct Item
	{
		GameObject* obj;
		float sortkey;
	};
	
	bool ProcessGameObject( GameObject* obj )
	{
		Item item = { obj, 0 };
		items.push_back( item );
		return false;
	}
	
	static int sort_func( const void* A, const void* B )
	{
		SGRX_CAST( Item*, a, A );
		SGRX_CAST( Item*, b, B );
		return a->sortkey == b->sortkey ? 0 : ( a->sortkey < b->sortkey ? -1 : 1 );
	}
	void Sort()
	{
		qsort( items.data(), items.size(), sizeof(Item), sort_func );
	}
	void DistanceSort( Vec3 pos )
	{
		for( size_t i = 0; i < items.size(); ++i )
			items[ i ].sortkey = ( items[ i ].obj->GetWorldPosition() - pos ).LengthSq();
		Sort();
	}
	
	Array< Item > items;
};

struct InfoEmitGameObjectSet
{
	void Clear(){ m_gameObjects.clear(); }
	void Register( GameObject* e ){ m_gameObjects.set( e, NoValue() ); }
	void Unregister( GameObject* e ){ m_gameObjects.unset( e ); }
	
	struct NoTest
	{
		FINLINE bool operator () ( GameObject* E ) const { return true; }
	};
	struct SphereTest
	{
		Vec3 position;
		float radius_squared;
		FINLINE bool operator () ( GameObject* E ) const
		{
			return ( E->GetWorldInfoTarget() - position ).LengthSq() <= radius_squared;
		}
	};
	struct OBBTest
	{
		Vec3 bbmin;
		Vec3 bbmax;
		Mat4 inverse_matrix;
		FINLINE bool operator () ( GameObject* obj ) const
		{
			Vec3 tp = inverse_matrix.TransformPos( obj->GetWorldInfoTarget() );
			if( tp.x >= bbmin.x && tp.x <= bbmax.x &&
				tp.y >= bbmin.y && tp.y <= bbmax.y &&
				tp.z >= bbmin.z && tp.z <= bbmax.z )
				return true;
			return false;
		}
	};
	template< class T > bool Query( const T& test, uint32_t types, GameObjectProcessor* proc = NULL )
	{
		bool ret = false;
		for( size_t i = 0; i < m_gameObjects.size(); ++i )
		{
			GameObject* obj = m_gameObjects.item( i ).key;
			if( !( obj->m_infoMask & types ) )
				continue;
			
			if( !test( obj ) )
				continue;
			
			if( !proc )
				return true;
			
			if( proc->ProcessGameObject( obj ) )
				return true;
			ret = true;
		}
		return ret;
	}
	
	HashTable< GameObject*, NoValue > m_gameObjects;
};



struct CameraResource;


typedef sgsHandle< struct GameLevel > GameLevelScrHandle;

EXP_STRUCT GameLevel :
	SGRX_PostDraw,
	SGRX_DebugDraw,
	SGRX_LightEnvSampler,
	SGRX_IEventHandler
{
	SGS_OBJECT SGS_NO_DESTRUCT;
	
	GFW_EXPORT GameLevel( PhyWorldHandle phyWorld );
	GFW_EXPORT virtual ~GameLevel();
	GFW_EXPORT virtual void HandleEvent( SGRX_EventID eid, const EventData& edata );
	
	// configuration
	GFW_EXPORT void SetGlobalToSelf();
	GFW_EXPORT void AddSystem( IGameLevelSystem* sys );
	template< class T > T* GetSystem() const
	{
		for( size_t i = 0; i < m_systems.size(); ++i )
		{
			if( T::e_system_uid == m_systems[ i ]->m_system_uid )
				return static_cast<T*>( m_systems[ i ] );
		}
		return NULL;
	}
	GFW_EXPORT void AddEntry( const StringView& name, sgsVariable var );
	
	// system/entity interface
	sgsVariable GetScriptedObject()     { return m_self; }
	StringView GetLevelName() const     { return m_levelName; }
	bool IsPaused() const               { return m_paused || gcv_g_paused.value; }
	SGRX_IPhyWorld* GetPhyWorld() const { return m_phyWorld; }
	SGRX_ISoundSystem* GetSoundSys() const { return m_soundSys; }
	SGRX_Scene* GetScene() const        { return m_scene; }
	ScriptContext& GetScriptCtx()       { return m_scriptCtx; }
	sgs_Context* GetSGSC() const        { return m_scriptCtx.C; }
	GameUISystem* GetGUI()              { return m_guiSys; }
	LevelEventType GetEventType() const { return m_eventType; }
	float GetDeltaTime() const          { return m_deltaTime; }
	float GetBlendFactor() const        { return m_blendFactor; }
	float GetTickDeltaTime() const      { return m_tickDeltaTime; }
	float GetFixedTickDeltaTime() const { return m_fixedTickDeltaTime; }
	CameraResource* GetMainCamera()     { return m_mainCamera; }
	bool GetEditorMode() const          { return m_editorMode; }
	
	GFW_EXPORT bool Load( StringView levelname );
	template< class T > void RegisterNativeClass( StringView type )
	{
		sgsVariable iface = sgs_GetClassInterface< T >( GetSGSC() );
		m_scriptCtx.SetGlobal( type, iface );
	}
	GFW_EXPORT void ClearLevel();
	
	GFW_EXPORT void _PreCallbackFixup();
	
	GFW_EXPORT void FixedTick( float deltaTime );
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void Draw2D();
	GFW_EXPORT void Draw();
	
	// interface implementations
	GFW_EXPORT void DebugDraw();
	GFW_EXPORT void PostDraw();
	
	// serialization
	template< class T > void Serialize( T& arch );
	
	GFW_EXPORT bool Query( GameObjectProcessor* optProc, uint32_t mask );
	GFW_EXPORT bool QuerySphere( GameObjectProcessor* optProc, uint32_t mask, Vec3 pos, float rad );
	GFW_EXPORT bool QueryOBB( GameObjectProcessor* optProc, uint32_t mask, Mat4 mtx, Vec3 bbmin = V3(-1), Vec3 bbmax = V3(1) );
	GFW_EXPORT SGS_METHOD_NAMED( Query ) bool sgsQuery( sgsVariable optProc, uint32_t mask );
	GFW_EXPORT SGS_METHOD_NAMED( QuerySphere ) bool sgsQuerySphere( sgsVariable optProc, uint32_t mask, Vec3 pos, float rad );
	GFW_EXPORT SGS_METHOD_NAMED( QueryOBB ) bool sgsQueryOBB( sgsVariable optProc, uint32_t mask, Mat4 mtx, Vec3 bbmin, Vec3 bbmax );
	
	GFW_EXPORT SGS_METHOD void PlaySound( StringView name, Vec3 pos, Vec3 dir );
	
	// ---
	
	GFW_EXPORT void LightMesh( SGRX_MeshInstance* meshinst, Vec3 off = V3(0) );
	
	SGS_METHOD TimeVal GetTickTime(){ return m_currentTickTime * 1000.0; }
	SGS_METHOD TimeVal GetPhyTime(){ return m_currentPhyTime * 1000.0; }
	
	SGS_PROPERTY_FUNC( READ WRITE VARNAME mainCamera ) sgsHandle< CameraResource > m_mainCamera;
	Array< CameraResource* > m_cameras;
	
	// Editor
	void GetEditorCompilers( Array< IEditorSystemCompiler* >& out );
	
	// ENGINE OBJECTS
	SceneHandle m_scene;
	ScriptContext m_scriptCtx;
	PhyWorldHandle m_phyWorld;
	SoundSystemHandle m_soundSys;
	GameUISystem* m_guiSys;
	
	// UTILITIES
	uint32_t m_nameIDGen;
	double m_currentTickTime;
	double m_currentPhyTime;
	SGS_PROPERTY_FUNC( READ VARNAME deltaTime ) float m_deltaTime;
	SGS_PROPERTY_FUNC( READ VARNAME blendFactor ) float m_blendFactor;
	SGS_PROPERTY_FUNC( READ VARNAME tickDeltaTime ) float m_tickDeltaTime;
	SGS_PROPERTY_FUNC( READ VARNAME fixedTickDeltaTime ) float m_fixedTickDeltaTime;
	SGS_PROPERTY_FUNC( READ VARNAME name ) String m_levelName;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME nextLevel ) String m_nextLevel;
	LevelEventType m_eventType;
	bool m_editorMode;
	bool m_enableLoadingScreen;
	
	// SYSTEMS
	Array< IGameLevelSystem* > m_systems;
	
	// LEVEL DATA
	sgsVariable m_self;
	sgsVariable m_metadata;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME persistent ) sgsVariable m_persistent;
	SGS_BACKING_STORE( m_metadata.var );
	sgsVariable m_markerPositions;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME paused ) bool m_paused;
	double m_levelTime;
	
	// GAMEOBJECT PROTO
	struct GameObjRmInfo
	{
		GameObjRmInfo() : ch( false ), t( 0 ){}
		bool ch;
		float t;
	};
	GFW_EXPORT GameObject* CreateGameObject();
	GFW_EXPORT void DestroyGameObject( GameObject* obj, bool ch = true, float t = 0 );
	GFW_EXPORT void _RealDestroyGameObject( GameObject* obj, bool ch );
	GFW_EXPORT void _RemoveGameObjects( float dt );
	GFW_EXPORT SGS_METHOD_NAMED( CreateGameObject ) sgsVariable sgsCreateGameObject();
	GFW_EXPORT SGS_METHOD_NAMED( DestroyGameObject ) void sgsDestroyGameObject( sgsVariable oh, bool ch );
	GFW_EXPORT void _MapGameObjectByID( GameObject* obj );
	GFW_EXPORT void _UnmapGameObjectByID( GameObject* obj );
	GFW_EXPORT GameObject* FindGameObjectByID( const StringView& name );
	GFW_EXPORT SGS_METHOD_NAMED( FindGameObject ) GameObject::ScrHandle sgsFindGameObject( StringView name );
	template< class T > void _RegisterNativeBehavior( StringView type )
	{
		sgsVariable iface = sgs_GetClassInterface< T >( GetSGSC() );
		m_scriptCtx.SetGlobal( type, iface );
		m_self.getprop( "behavior_types" ).setprop( m_scriptCtx.CreateStringVar( type ), iface );
	}
	void EnumBehaviors( Array< StringView >& out );
	sgsVariable GetBehaviorInterface( StringView name );
	Array< GameObject* > m_gameObjects;
	HashTable< GameObject*, GameObjRmInfo > m_gameObjectsToRemove;
	SGRX_GUID nextObjectGUID;
	HashTable< uint32_t, GOResourceInfo > m_goResourceMap;
	HashTable< StringView, GOBehaviorCreateFunc* > m_goNativeBhvrMap;
	HashTable< StringView, GameObject* > m_gameObjIDMap;
	InfoEmitGameObjectSet m_infoEmitSet;
};

template< class T > T* AddSystemToLevel( GameLevel* lev )
{
	T* sys = new T( lev );
	lev->AddSystem( sys );
	return sys;
}

#define SGRX_LEVEL_SAVE_FILE_VERSION 1
template< class T > void GameLevel::Serialize( T& arch )
{
	arch.marker( "SGRX_LEVEL_SAVE_FILE" );
	SerializeVersionHelper<T> svh( arch, SGRX_LEVEL_SAVE_FILE_VERSION );
	
	// name
	svh << m_levelName;
	
	// misc. basic variables
	svh << m_nameIDGen;
	svh << m_currentTickTime;
	svh << m_currentPhyTime;
	svh << m_deltaTime;
	svh << m_nextLevel;
//	svh << m_editorMode; -- editor mode not serialized
	svh << m_paused;
	svh << m_levelTime;
	
//	svh << m_entIDMap; -- will be restored when loading entities
//	svh << m_infoEmitSet; -- will be restored when loading entities
	// m_systems -- TODO
	
	// m_self -- TODO
	// m_metadata -- TODO
	// m_markerPositions -- TODO
//	svh << m_entities; -- will be restored when loading entities
	
	// m_scene -- TODO [camera]
	// m_scriptCtx -- TODO
	// m_phyWorld -- TODO
	// m_soundSys -- TODO
	// m_guiSys -- TODO
}



template< class T > void LevelScrObj::_InitScriptInterface( T* ptr )
{
	T::_sgs_interface->destruct = NULL;
	SGS_CSCOPE( m_level->GetSGSC() );
	sgs_CreateClass( m_level->GetSGSC(), NULL, ptr );
	C = m_level->GetSGSC();
	m_sgsObject = sgs_GetObjectStruct( C, -1 );
	sgs_ObjAcquire( C, m_sgsObject );
}

FINLINE void GameObject::SetName( StringView nm )
{
	m_name = m_level->GetScriptCtx().CreateString( nm );
}

FINLINE void GameObject::SetID( StringView id )
{
	sgsSetID( m_level->GetScriptCtx().CreateString( id ) );
}

FINLINE void GameObject::sgsSetID( sgsString id )
{
	if( m_id.same_as( id ) )
		return;
	sgsString prev = m_id;
	if( m_id.size() )
		m_level->_UnmapGameObjectByID( this );
	m_id = id;
	if( m_id.size() )
		m_level->_MapGameObjectByID( this );
	OnIDUpdate();
}


enum EditorType
{
	ET_NoEditor = 0,
	ET_MapEditor,
	ET_AssetEditor,
	ET_PSysEditor,
};

struct BaseEditor
{
	GFW_EXPORT BaseEditor( struct BaseGame* game, int type );
	GFW_EXPORT ~BaseEditor();
	
	void* m_lib;
	GameHandle m_editorGame;
	GameHandle m_origGame;
};

EXP_STRUCT BaseGame : IGame
{
	GFW_EXPORT BaseGame();
	GFW_EXPORT virtual int OnArgument( char* arg, int argcleft, char** argvleft );
	GFW_EXPORT virtual bool OnConfigure( int argc, char** argv );
	GFW_EXPORT virtual bool OnInitialize();
	GFW_EXPORT virtual void OnDestroy();
	GFW_EXPORT void InitSoundSystem();
	GFW_EXPORT void ParseConfigFile( bool init );
	template< class T > void RegisterSystem(){ RegisterSystem( T::_Name(), T::_Create ); }
	GFW_EXPORT void RegisterSystem( StringView name, GameLevelSystemCreateFunc func );
	GFW_EXPORT virtual SoundSystemHandle CreateSoundSystem() = 0;
	GFW_EXPORT virtual PhyWorldHandle CreatePhyWorld() = 0;
	GFW_EXPORT virtual GameLevel* CreateLevel();
	GFW_EXPORT virtual void OnLevelChange();
	GFW_EXPORT virtual void Game_FixedTick( float dt );
	GFW_EXPORT virtual void Game_Tick( float dt, float bf );
	GFW_EXPORT virtual void Game_Render();
	GFW_EXPORT virtual void OnTick( float dt, uint32_t gametime );
	
	GFW_EXPORT virtual void SetOverlayMusic( StringView path );
	
	float m_maxTickSize;
	float m_fixedTickSize;
	float m_accum;
	float m_timeMultiplier;
	SoundSystemHandle m_soundSys;
	SoundEventInstanceHandle m_ovrMusic;
	String m_ovrMusicPath;
	GameLevel* m_level;
	BaseEditor* m_editor;
	int m_needsEditor;
	String m_mapName;
	HashTable< RCString, GameLevelSystemCreateFunc* > m_systemCreateFuncs;
	String m_levelSystems;
};


