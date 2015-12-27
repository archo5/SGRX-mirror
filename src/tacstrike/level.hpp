

#pragma once

#include <engine.hpp>
#include <enganim.hpp>
#include <pathfinding.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#include "levelcache.hpp"
#include "gamegui.hpp"


extern CVarBool gcv_cl_gui;
extern CVarBool gcv_cl_debug;
extern CVarBool gcv_g_paused;

void RegisterCommonGameCVars();


struct GameLevel;


extern sgs_ObjInterface g_sgsobj_empty_handle[1];


typedef uint32_t TimeVal;
typedef uint32_t EntityID;

#define ENTID_NONE ((EntityID)0)


struct SGRX_IActorController : SGRX_RefCounted
{
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual Vec3 GetInput( uint32_t iid ){ return V3(0); }
	virtual void Reset(){}
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
};
typedef Handle< SGRX_IActorController > SGRX_ActorCtrlHandle;


struct IGameLevelSystem
{
	IGameLevelSystem( GameLevel* lev, uint32_t uid ) :
		C( NULL ), m_sgsObject( NULL ),
		m_level( lev ),
		m_system_uid( uid )
	{}
	virtual ~IGameLevelSystem();
	virtual void OnPostLevelLoad(){}
	virtual void OnLevelDestroy(){ delete this; }
	virtual bool AddEntity( const StringView& type, sgsVariable data ){ return false; }
	virtual bool LoadChunk( const StringView& type, uint8_t* ptr, size_t size ){ return false; }
	virtual void Clear(){}
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual void DrawUI(){}
	
	virtual void PostDraw(){}
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	template< class T > void InitScriptInterface( const StringView& name, T* ptr );
	void DestroyScriptInterface();
	SGS_CTX;
	sgs_VarObj* m_sgsObject;
	
	GameLevel* m_level;
	uint32_t m_system_uid;
};


struct Entity
{
	SGS_OBJECT SGS_NO_DESTRUCT;
	typedef sgsHandle< Entity > Handle;
	
	Entity( GameLevel* lev );
	virtual ~Entity();
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual void OnEvent( const StringView& type ){}
	
	virtual void* GetInterfaceImpl( uint32_t iface_id ){ return NULL; }
	template< class T > T* GetInterface(){ return (T*) GetInterfaceImpl( T::e_iface_uid ); }
#define ENT_HAS_INTERFACE( T, reqid, ptr ) if( T::e_iface_uid == reqid ) return (T*) ptr
	
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	sgsVariable GetScriptedObject(){ return Handle( this ).get_variable(); }
	
	template< class T > void _InitScriptInterface( T* ptr );
	void _DestroyScriptInterface_()
	{
		m_sgsObject->data = NULL;
		m_sgsObject->iface = g_sgsobj_empty_handle;
		sgs_ObjRelease( C, m_sgsObject );
	}
#define ENT_SGS_IMPLEMENT \
	virtual void InitScriptInterface(){ if( m_sgsObject == NULL ) _InitScriptInterface( this ); }
	
	ENT_SGS_IMPLEMENT;
	
	const char* m_typeName;
	SGS_PROPERTY_FUNC( READ VARNAME name ) String m_name;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME viewName ) String m_viewName;
	GameLevel* m_level;
	
	StringView _sgs_getTypeName(){ return m_typeName; }
	SGS_PROPERTY_FUNC( READ _sgs_getTypeName ) SGS_ALIAS( StringView typeName );
	
	sgsHandle< GameLevel > _sgs_getLevel();
	SGS_PROPERTY_FUNC( READ _sgs_getLevel ) SGS_ALIAS( sgsHandle< GameLevel > level );
	
	SGS_METHOD_NAMED( CallEvent ) SGS_ALIAS( void OnEvent( StringView type ) );
};


struct SGRX_Actor : Entity
{
	SGS_OBJECT_INHERIT( Entity ) SGS_NO_DESTRUCT;
	ENT_SGS_IMPLEMENT;
	
	SGRX_Actor( GameLevel* lev ) : Entity( lev ){}
	FINLINE Vec3 GetInputV3( uint32_t iid ){ return ctrl ? ctrl->GetInput( iid ) : V3(0); }
	FINLINE Vec2 GetInputV2( uint32_t iid ){ return ctrl ? ctrl->GetInput( iid ).ToVec2() : V2(0); }
	FINLINE float GetInputF( uint32_t iid ){ return ctrl ? ctrl->GetInput( iid ).x : 0; }
	FINLINE bool GetInputB( uint32_t iid ){ return ctrl ? ctrl->GetInput( iid ).x > 0.5f : false; }
	
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
	
	virtual bool IsAlive(){ return true; }
	virtual void Reset(){} // make alive again
	virtual Vec3 GetPosition(){ return V3(0); }
	virtual void SetPosition( Vec3 pos ){} // teleport to this place
	
	SGRX_ActorCtrlHandle ctrl;
};


typedef StackString<16> StackShortName;

struct GameLevel :
	SGRX_PostDraw,
	SGRX_DebugDraw,
	SGRX_LightTreeSampler,
	SGRX_IEventHandler
{
	SGS_OBJECT SGS_NO_DESTRUCT;
	
	GameLevel( PhyWorldHandle phyWorld );
	virtual ~GameLevel();
	virtual void HandleEvent( SGRX_EventID eid, const EventData& edata );
	
	// configuration
	void SetGlobalToSelf();
	void AddSystem( IGameLevelSystem* sys );
	template< class T > T* GetSystem() const
	{
		for( size_t i = 0; i < m_systems.size(); ++i )
		{
			if( T::e_system_uid == m_systems[ i ]->m_system_uid )
				return static_cast<T*>( m_systems[ i ] );
		}
		return NULL;
	}
	void AddEntity( Entity* E );
	void AddEntry( const StringView& name, sgsVariable var );
	
	// system/entity interface
	StringView GetLevelName() const { return m_levelName; }
	void SetPlayer( Entity* E ){ m_player = E; }
	bool IsPaused() const { return m_paused || gcv_g_paused.value; }
	SGRX_IPhyWorld* GetPhyWorld() const { return m_phyWorld; }
	SGRX_Scene* GetScene() const { return m_scene; }
	ScriptContext& GetScriptCtx(){ return m_scriptCtx; }
	sgs_Context* GetSGSC() const { return m_scriptCtx.C; }
	GameUISystem* GetGUI(){ return m_guiSys; }
	float GetDeltaTime() const { return m_deltaTime; }
	
	bool Load( const StringView& levelname );
	void CreateEntity( const StringView& type, sgsVariable data );
	StackShortName GenerateName();
	void ClearLevel();
	
	void ProcessEvents();
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void Draw2D();
	void Draw();
	
	// interface implementations
	void DebugDraw();
	void PostDraw();
	
	SGS_METHOD_NAMED( SetLevel ) void SetNextLevel( StringView name );
	void MapEntityByName( Entity* e );
	void UnmapEntityByName( Entity* e );
	Entity* FindEntityByName( const StringView& name );
	SGS_METHOD_NAMED( FindEntity ) Entity::Handle sgsFindEntity( StringView name );
	SGS_METHOD_NAMED( CallEntity ) void CallEntityByName( StringView name, StringView action );
	SGS_METHOD_NAMED( SetCameraPosDir ) void sgsSetCameraPosDir( Vec3 pos, Vec3 dir );
	SGS_METHOD_NAMED( WorldToScreen ) SGS_MULTRET sgsWorldToScreen( Vec3 pos );
	SGS_METHOD_NAMED( WorldToScreenPx ) SGS_MULTRET sgsWorldToScreenPx( Vec3 pos );
	
	// ---
	SGS_IFUNC( GETINDEX ) int _getindex( SGS_ARGS_GETINDEXFUNC );
	SGS_IFUNC( SETINDEX ) int _setindex( SGS_ARGS_SETINDEXFUNC );
	
	void LightMesh( MeshInstHandle mih, Vec3 off = V3(0) );
	
	TimeVal GetTickTime(){ return m_currentTickTime * 1000.0; }
	TimeVal GetPhyTime(){ return m_currentPhyTime * 1000.0; }
	
	// ENGINE OBJECTS
	SceneHandle m_scene;
	ScriptContext m_scriptCtx;
	PhyWorldHandle m_phyWorld;
	GUISysHandle m_guiSys;
	
	// UTILITIES
	uint32_t m_nameIDGen;
	double m_currentTickTime;
	double m_currentPhyTime;
	float m_deltaTime;
	String m_levelName;
	String m_nextLevel;
	
	// SYSTEMS
	HashTable< StringView, Entity* > m_entNameMap;
	Array< IGameLevelSystem* > m_systems;
	
	// LEVEL DATA
	sgsVariable m_self;
	sgsVariable m_metadata;
	sgsVariable m_markerPositions;
	bool m_paused;
	double m_levelTime;
	Array< Entity* > m_entities;
	Entity* m_player;
	Vec3 m_playerSpawnInfo[2]; // position, direction
	Vec3 m_levelCameraInfo[2]; // position, direction
};



template< class T > void IGameLevelSystem::InitScriptInterface( const StringView& name, T* ptr )
{
	T::_sgs_interface->destruct = NULL;
	SGS_CSCOPE( m_level->GetSGSC() );
	sgs_CreateClass( m_level->GetSGSC(), NULL, ptr );
	m_level->AddEntry( name, sgsVariable( m_level->GetSGSC(), -1 ) );
	C = m_level->GetSGSC();
	m_sgsObject = sgs_GetObjectStruct( C, -1 );
	sgs_ObjAcquire( C, m_sgsObject );
}

template< class T > void Entity::_InitScriptInterface( T* ptr )
{
	T::_sgs_interface->destruct = NULL;
	SGS_CSCOPE( m_level->GetSGSC() );
	sgs_CreateClass( m_level->GetSGSC(), NULL, ptr );
	C = m_level->GetSGSC();
	m_sgsObject = sgs_GetObjectStruct( C, -1 );
	sgs_ObjAcquire( C, m_sgsObject );
}


