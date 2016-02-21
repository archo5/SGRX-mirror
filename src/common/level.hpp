

#pragma once

#include "gfwcore.hpp"
#include "levelcache.hpp"
#include "gamegui.hpp"


extern GFW_EXPORT CVarBool gcv_cl_gui;
extern GFW_EXPORT CVarBool gcv_cl_debug;
extern GFW_EXPORT CVarBool gcv_g_paused;

GFW_EXPORT void RegisterCommonGameCVars();


struct GameLevel;


extern sgs_ObjInterface g_sgsobj_empty_handle[1];


typedef uint32_t TimeVal;


struct IF_GCC(GFW_EXPORT) LevelScrObj : SGRX_RefCounted
{
	SGS_OBJECT SGS_NO_DESTRUCT;
	typedef sgsHandle< LevelScrObj > ScrHandle;
	
	GFW_EXPORT LevelScrObj( GameLevel* lev );
	GFW_EXPORT virtual ~LevelScrObj();
	
	template< class T > void _InitScriptInterface( T* ptr );
	GFW_EXPORT void DestroyScriptInterface();
#define ENT_SGS_IMPLEMENT \
	virtual void InitScriptInterface(){ if( m_sgsObject == NULL ) _InitScriptInterface( this ); }
	
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


struct IF_GCC(GFW_EXPORT) IActorController : LevelScrObj
{
	SGS_OBJECT_INHERIT( LevelScrObj ) SGS_NO_DESTRUCT;
	typedef sgsHandle< IActorController > ScrHandle;
	
	IActorController( GameLevel* lev ) : LevelScrObj( lev ){}
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual SGS_METHOD Vec3 GetInput( uint32_t iid ){ return V3(0); }
	virtual SGS_METHOD void Reset(){}
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
};
typedef Handle< IActorController > ActorCtrlHandle;


struct IF_GCC(GFW_EXPORT) Transform
{
	Transform() :
		_localPosition( V3(0) ),
		_localRotation( Quat::Identity ),
		_localScale( V3(1) ),
		_worldMatrix( Mat4::Identity ),
		_invWorldMatrix( Mat4::Identity ),
		_updateOnEdit( true )
	{}
	
	Vec3 _localPosition;
	Quat _localRotation;
	Vec3 _localScale;
	Mat4 _worldMatrix;
	Mat4 _invWorldMatrix;
	bool _updateOnEdit;
	
	void OnEdit()
	{
		if( _updateOnEdit == false )
			return;
		_worldMatrix = GetLocalMatrix();
		_invWorldMatrix = _worldMatrix.Inverted();
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
	
	FINLINE Vec3 GetWorldPosition() const { return _worldMatrix.GetTranslation(); }
	FINLINE Quat GetWorldRotation() const { return _worldMatrix.GetRotationQuaternion(); }
	FINLINE Vec3 GetWorldRotationXYZ() const { return RAD2DEG( _worldMatrix.GetRotationQuaternion().ToXYZ() ); }
	FINLINE Vec3 GetWorldScale() const { return _worldMatrix.GetScale(); }
	FINLINE Mat4 GetWorldMatrix() const { return _worldMatrix; }
	
	FINLINE Vec3 LocalToWorld( Vec3 p ) const { return _worldMatrix.TransformPos( p ); }
	FINLINE Vec3 WorldToLocal( Vec3 p ) const { return _invWorldMatrix.TransformPos( p ); }
};

struct IF_GCC(GFW_EXPORT) Entity : LevelScrObj, Transform
{
	SGS_OBJECT_INHERIT( LevelScrObj ) SGS_NO_DESTRUCT;
	typedef sgsHandle< Entity > ScrHandle;
	
	GFW_EXPORT Entity( GameLevel* lev );
	GFW_EXPORT ~Entity();
	GFW_EXPORT virtual void FixedTick( float deltaTime );
	GFW_EXPORT virtual void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT virtual void OnTransformUpdate();
	
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	GFW_EXPORT virtual void EditorDrawWorld();
	
	FINLINE uint32_t GetInfoMask() const { return m_infoMask; }
	GFW_EXPORT void SetInfoMask( uint32_t mask );
	FINLINE StringView GetID() const { return StringView( m_id.c_str(), m_id.size() ); }
	FINLINE void SetID( StringView id );
	FINLINE void sgsSetID( sgsString id );
	FINLINE Vec3 GetInfoTarget() const { return m_infoTarget; }
	FINLINE void SetInfoTarget( Vec3 tgt ){ m_infoTarget = tgt; }
	FINLINE Vec3 GetWorldInfoTarget() const { return LocalToWorld( m_infoTarget ); }
	
	SGS_PROPERTY_FUNC( READ GetWorldPosition WRITE SetLocalPosition
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Vec3 position );
	SGS_PROPERTY_FUNC( READ GetWorldRotation WRITE SetLocalRotation
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Quat rotation );
	SGS_PROPERTY_FUNC( READ GetWorldRotationXYZ WRITE SetLocalRotationXYZ
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Vec3 rotationXYZ );
	SGS_PROPERTY_FUNC( READ GetWorldScale WRITE SetLocalScale
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Vec3 scale );
	SGS_PROPERTY_FUNC( READ GetWorldMatrix WRITE SetLocalMatrix
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Mat4 transform );
	
	SGS_PROPERTY_FUNC( READ GetLocalPosition WRITE SetLocalPosition
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Vec3 localPosition );
	SGS_PROPERTY_FUNC( READ GetLocalRotation WRITE SetLocalRotation
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Quat localRotation );
	SGS_PROPERTY_FUNC( READ GetLocalRotationXYZ WRITE SetLocalRotationXYZ
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Vec3 localRotationXYZ );
	SGS_PROPERTY_FUNC( READ GetLocalScale WRITE SetLocalScale
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Vec3 localScale );
	SGS_PROPERTY_FUNC( READ GetLocalMatrix WRITE SetLocalMatrix
		WRITE_CALLBACK OnTransformUpdate ) SGS_ALIAS( Mat4 localTransform );
	
	SGS_PROPERTY_FUNC( READ GetInfoMask WRITE SetInfoMask VARNAME infoMask ) uint32_t m_infoMask;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME localInfoTarget ) Vec3 m_infoTarget;
	SGS_PROPERTY_FUNC( READ GetWorldInfoTarget ) SGS_ALIAS( Vec3 infoTarget );
	
	SGS_PROPERTY sgsString name;
	SGS_PROPERTY_FUNC( READ WRITE sgsSetID VARNAME id ) sgsString m_id;
};

#define IEST_InteractiveItem 0x0001
#define IEST_HeatSource      0x0002
#define IEST_Player          0x0004
#define IEST_Target          0x0010
#define IEST_AIAlert         0x0020


struct IF_GCC(GFW_EXPORT) IGameLevelSystem : LevelScrObj
{
	SGS_OBJECT_INHERIT( LevelScrObj ) SGS_NO_DESTRUCT;
	
	IGameLevelSystem( GameLevel* lev, uint32_t uid ) :
		LevelScrObj( lev ), m_system_uid( uid )
	{}
	virtual void OnPostLevelLoad(){}
	virtual void OnLevelDestroy(){ delete this; }
	virtual Entity* AddEntity( StringView type ){ return NULL; }
	virtual bool LoadChunk( const StringView& type, ByteView data ){ return false; }
	virtual void Clear(){}
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual void DrawUI(){}
	
	virtual void PostDraw(){}
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	uint32_t m_system_uid;
};


struct IF_GCC(GFW_EXPORT) Actor : Entity
{
	SGS_OBJECT_INHERIT( Entity ) SGS_NO_DESTRUCT;
	ENT_SGS_IMPLEMENT;
	
	Actor( GameLevel* lev ) : Entity( lev ){}
	FINLINE SGS_METHOD Vec3 GetInputV3( uint32_t iid ){ return ctrl ? ctrl->GetInput( iid ) : V3(0); }
	FINLINE SGS_METHOD Vec2 GetInputV2( uint32_t iid ){ return ctrl ? ctrl->GetInput( iid ).ToVec2() : V2(0); }
	FINLINE SGS_METHOD float GetInputF( uint32_t iid ){ return ctrl ? ctrl->GetInput( iid ).x : 0; }
	FINLINE SGS_METHOD bool GetInputB( uint32_t iid ){ return ctrl ? ctrl->GetInput( iid ).x > 0.5f : false; }
	
	virtual void FixedTick( float deltaTime )
	{
		if( ctrl )
			ctrl->FixedTick( deltaTime );
	}
	virtual void Tick( float deltaTime, float blendFactor )
	{
		if( ctrl )
			ctrl->Tick( deltaTime, blendFactor );
	}
	virtual void DebugDrawWorld()
	{
		if( ctrl )
			ctrl->DebugDrawWorld();
	}
	virtual void DebugDrawUI()
	{
		if( ctrl )
			ctrl->DebugDrawUI();
	}
	
	virtual SGS_METHOD bool IsAlive(){ return true; }
	virtual SGS_METHOD void Reset(){} // make alive again
	virtual Vec3 GetPosition(){ return V3(0); }
	virtual void SetPosition( Vec3 pos ){} // teleport to this place
	
	ActorCtrlHandle ctrl;
	
	IActorController::ScrHandle _getCtrl(){ return IActorController::ScrHandle(ctrl); }
	SGS_PROPERTY_FUNC( READ _getCtrl ) SGS_ALIAS( sgsHandle<IActorController> ctrl );
	SGS_PROPERTY_FUNC( READ GetPosition WRITE SetPosition ) SGS_ALIAS( Vec3 position );
};


struct EntityProcessor
{
	virtual bool ProcessEntity( Entity* e ) = 0;
};

struct EntityGather : EntityProcessor
{
	struct Item
	{
		Entity* E;
		float sortkey;
	};
	
	bool ProcessEntity( Entity* E )
	{
		Item item = { E, 0 };
		items.push_back( item );
		return true;
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
			items[ i ].sortkey = ( items[ i ].E->GetWorldPosition() - pos ).LengthSq();
		Sort();
	}
	
	Array< Item > items;
};

struct InfoEmitEntitySet
{
	void Clear(){ m_entities.clear(); }
	void Register( Entity* e ){ m_entities.set( e, NoValue() ); }
	void Unregister( Entity* e ){ m_entities.unset( e ); }
	
	struct NoTest
	{
		FINLINE bool operator () ( Entity* E ) const { return true; }
	};
	struct SphereTest
	{
		Vec3 position;
		float radius_squared;
		FINLINE bool operator () ( Entity* E ) const
		{
			return ( E->GetWorldInfoTarget() - position ).LengthSq() <= radius_squared;
		}
	};
	struct OBBTest
	{
		Vec3 bbmin;
		Vec3 bbmax;
		Mat4 inverse_matrix;
		FINLINE bool operator () ( Entity* E ) const
		{
			Vec3 tp = inverse_matrix.TransformPos( E->GetWorldInfoTarget() );
			if( tp.x >= bbmin.x && tp.x <= bbmax.x &&
				tp.y >= bbmin.y && tp.y <= bbmax.y &&
				tp.z >= bbmin.z && tp.z <= bbmax.z )
				return true;
			return false;
		}
	};
	template< class T > bool Query( const T& test, uint32_t types, EntityProcessor* proc = NULL )
	{
		bool ret = false;
		for( size_t i = 0; i < m_entities.size(); ++i )
		{
			Entity* E = m_entities.item( i ).key;
			if( !( E->m_infoMask & types ) )
				continue;
			
			if( !test( E ) )
				continue;
			
			if( !proc )
				return true;
			
			if( proc->ProcessEntity( E ) )
				return true;
			ret = true;
		}
		return ret;
	}
	
	HashTable< Entity*, NoValue > m_entities;
};

typedef StackString<16> StackShortName;

struct IF_GCC(GFW_EXPORT) GameLevel :
	SGRX_PostDraw,
	SGRX_DebugDraw,
	SGRX_LightTreeSampler,
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
	sgsVariable GetScriptedObject(){ return m_self; }
	StringView GetLevelName() const { return m_levelName; }
	void SetPlayer( Entity* E ){ m_player = E; }
	bool IsPaused() const { return m_paused || gcv_g_paused.value; }
	SGRX_IPhyWorld* GetPhyWorld() const { return m_phyWorld; }
	SGRX_ISoundSystem* GetSoundSys() const { return m_soundSys; }
	SGRX_Scene* GetScene() const { return m_scene; }
	ScriptContext& GetScriptCtx(){ return m_scriptCtx; }
	sgs_Context* GetSGSC() const { return m_scriptCtx.C; }
	GameUISystem* GetGUI(){ return m_guiSys; }
	float GetDeltaTime() const { return m_deltaTime; }
	bool GetEditorMode() const { return m_editorMode; }
	
	GFW_EXPORT bool Load( const StringView& levelname );
	template< class T > void RegisterNativeEntity( StringView type )
	{
		sgsVariable iface = sgs_GetClassInterface< T >( GetSGSC() );
		m_scriptCtx.SetGlobal( type, iface );
		m_self.getprop( "entity_types" ).setprop( m_scriptCtx.CreateStringVar( type ), iface );
	}
	GFW_EXPORT sgsVariable GetEntityInterface( StringView name );
	GFW_EXPORT void EnumEntities( Array< StringView >& out );
	GFW_EXPORT Entity* CreateEntity( const StringView& type );
	GFW_EXPORT void DestroyEntity( Entity* eptr );
	GFW_EXPORT StackShortName GenerateName();
	GFW_EXPORT void ClearLevel();
	
	GFW_EXPORT void ProcessEvents();
	GFW_EXPORT void FixedTick( float deltaTime );
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void Draw2D();
	GFW_EXPORT void Draw();
	
	// interface implementations
	GFW_EXPORT void DebugDraw();
	GFW_EXPORT void PostDraw();
	
	GFW_EXPORT SGS_METHOD_NAMED( SetLevel ) void SetNextLevel( StringView name );
	GFW_EXPORT void _MapEntityByID( Entity* e );
	GFW_EXPORT void _UnmapEntityByID( Entity* e );
	GFW_EXPORT Entity* FindEntityByID( const StringView& name );
	GFW_EXPORT SGS_METHOD_NAMED( CreateEntity ) sgsVariable sgsCreateEntity( StringView type );
	GFW_EXPORT SGS_METHOD_NAMED( DestroyEntity ) void sgsDestroyEntity( sgsVariable eh );
	GFW_EXPORT SGS_METHOD_NAMED( FindEntity ) Entity::ScrHandle sgsFindEntity( StringView name );
	GFW_EXPORT SGS_METHOD_NAMED( SetCameraPosDir ) void sgsSetCameraPosDir( Vec3 pos, Vec3 dir );
	GFW_EXPORT SGS_METHOD_NAMED( WorldToScreen ) SGS_MULTRET sgsWorldToScreen( Vec3 pos );
	GFW_EXPORT SGS_METHOD_NAMED( WorldToScreenPx ) SGS_MULTRET sgsWorldToScreenPx( Vec3 pos );
	GFW_EXPORT SGS_METHOD_NAMED( GetCursorWorldPoint ) SGS_MULTRET sgsGetCursorWorldPoint( uint32_t layers /* = 0xffffffff */ );
	
	GFW_EXPORT bool Query( EntityProcessor* optProc, uint32_t mask );
	GFW_EXPORT bool QuerySphere( EntityProcessor* optProc, uint32_t mask, Vec3 pos, float rad );
	GFW_EXPORT bool QueryOBB( EntityProcessor* optProc, uint32_t mask, Mat4 mtx, Vec3 bbmin = V3(-1), Vec3 bbmax = V3(1) );
	GFW_EXPORT SGS_METHOD_NAMED( Query ) bool sgsQuery( sgsVariable optProc, uint32_t mask );
	GFW_EXPORT SGS_METHOD_NAMED( QuerySphere ) bool sgsQuerySphere( sgsVariable optProc, uint32_t mask, Vec3 pos, float rad );
	GFW_EXPORT SGS_METHOD_NAMED( QueryOBB ) bool sgsQueryOBB( sgsVariable optProc, uint32_t mask, Mat4 mtx, Vec3 bbmin, Vec3 bbmax );
	
	// ---
	
	GFW_EXPORT void LightMesh( SGRX_MeshInstance* meshinst, Vec3 off = V3(0) );
	
	SGS_METHOD TimeVal GetTickTime(){ return m_currentTickTime * 1000.0; }
	SGS_METHOD TimeVal GetPhyTime(){ return m_currentPhyTime * 1000.0; }
	
	// ENGINE OBJECTS
	SceneHandle m_scene;
	ScriptContext m_scriptCtx;
	PhyWorldHandle m_phyWorld;
	SoundSystemHandle m_soundSys;
	GUISysHandle m_guiSys;
	
	// UTILITIES
	uint32_t m_nameIDGen;
	double m_currentTickTime;
	double m_currentPhyTime;
	float m_deltaTime;
	String m_levelName;
	String m_nextLevel;
	bool m_editorMode;
	
	// SYSTEMS
	HashTable< StringView, Entity* > m_entIDMap;
	InfoEmitEntitySet m_infoEmitSet;
	Array< IGameLevelSystem* > m_systems;
	
	// LEVEL DATA
	sgsVariable m_self;
	sgsVariable m_metadata;
	SGS_BACKING_STORE( m_metadata.var );
	sgsVariable m_markerPositions;
	bool m_paused;
	double m_levelTime;
	Array< Entity* > m_entities;
	Entity* m_player;
};

template< class T > T* AddSystemToLevel( GameLevel* lev )
{
	T* sys = new T( lev );
	lev->AddSystem( sys );
	return sys;
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

FINLINE void Entity::SetID( StringView id )
{
	sgsSetID( m_level->GetScriptCtx().CreateString( id ) );
}

FINLINE void Entity::sgsSetID( sgsString id )
{
	if( m_id.same_as( id ) )
		return;
	if( m_id.size() )
		m_level->_UnmapEntityByID( this );
	m_id = id;
	if( m_id.size() )
		m_level->_MapEntityByID( this );
}


struct BaseEditor
{
	GFW_EXPORT BaseEditor( struct BaseGame* game );
	GFW_EXPORT ~BaseEditor();
	
	void* m_lib;
	GameHandle m_editorGame;
	GameHandle m_origGame;
};

struct IF_GCC(GFW_EXPORT) BaseGame : IGame
{
	GFW_EXPORT BaseGame();
	GFW_EXPORT virtual int OnArgument( char* arg, int argcleft, char** argvleft );
	GFW_EXPORT virtual bool OnConfigure( int argc, char** argv );
	GFW_EXPORT virtual bool OnInitialize();
	GFW_EXPORT virtual void OnDestroy();
	GFW_EXPORT virtual PhyWorldHandle CreatePhyWorld() = 0;
	GFW_EXPORT virtual GameLevel* CreateLevel();
	GFW_EXPORT virtual void Game_FixedTick( float dt );
	GFW_EXPORT virtual void Game_Tick( float dt, float bf );
	GFW_EXPORT virtual void Game_Render();
	GFW_EXPORT virtual void OnTick( float dt, uint32_t gametime );
	
	float m_maxTickSize;
	float m_fixedTickSize;
	float m_accum;
	float m_timeMultiplier;
	SoundSystemHandle m_soundSys;
	GameLevel* m_level;
	BaseEditor* m_editor;
	bool m_needsEditor;
};


