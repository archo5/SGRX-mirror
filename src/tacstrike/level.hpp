

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


struct LevelScrObj : SGRX_RefCounted
{
	SGS_OBJECT SGS_NO_DESTRUCT;
	typedef sgsHandle< LevelScrObj > ScrHandle;
	
	LevelScrObj( GameLevel* lev );
	virtual ~LevelScrObj();
	
	template< class T > void _InitScriptInterface( T* ptr );
	void DestroyScriptInterface();
#define ENT_SGS_IMPLEMENT \
	virtual void InitScriptInterface(){ if( m_sgsObject == NULL ) _InitScriptInterface( this ); }
	
	ENT_SGS_IMPLEMENT;
	
	sgsVariable GetScriptedObject(){ return ScrHandle( this ).get_variable(); }
	void AddSelfToLevel( StringView name );
	
	virtual void* GetInterfaceImpl( uint32_t iface_id ){ return NULL; }
	template< class T > T* GetInterface(){ return (T*) GetInterfaceImpl( T::e_iface_uid ); }
#define ENT_HAS_INTERFACE( T, reqid, ptr ) if( T::e_iface_uid == reqid ) return (T*) ptr
	
	sgsHandle< GameLevel > _sgs_getLevel();
	SGS_PROPERTY_FUNC( READ _sgs_getLevel ) SGS_ALIAS( sgsHandle< GameLevel > level );
	
	GameLevel* m_level;
};


struct IActorController : LevelScrObj
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


struct Entity : LevelScrObj
{
	SGS_OBJECT_INHERIT( LevelScrObj ) SGS_NO_DESTRUCT;
	typedef sgsHandle< Entity > ScrHandle;
	
	Entity( GameLevel* lev );
	~Entity();
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual void OnEvent( const StringView& type ){}
	
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	SGS_PROPERTY_FUNC( READ VARNAME typeName ) StringView m_typeName;
	SGS_PROPERTY_FUNC( READ VARNAME name ) String m_name;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME viewName ) String m_viewName;
	SGS_METHOD_NAMED( CallEvent ) SGS_ALIAS( void OnEvent( StringView type ) );
};


struct IGameLevelSystem : LevelScrObj
{
	SGS_OBJECT_INHERIT( LevelScrObj ) SGS_NO_DESTRUCT;
	
	IGameLevelSystem( GameLevel* lev, uint32_t uid ) :
		LevelScrObj( lev ), m_system_uid( uid )
	{}
	virtual void OnPostLevelLoad(){}
	virtual void OnLevelDestroy(){ delete this; }
	virtual bool AddEntity( const StringView& type, sgsVariable data, sgsVariable& outvar ){ return false; }
	virtual bool LoadChunk( const StringView& type, uint8_t* ptr, size_t size ){ return false; }
	virtual void Clear(){}
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual void DrawUI(){}
	
	virtual void PostDraw(){}
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	uint32_t m_system_uid;
};


struct Actor : Entity
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
	sgsVariable CreateEntity( const StringView& type, sgsVariable data );
	void DestroyEntity( Entity* eptr );
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
	SGS_METHOD_NAMED( CreateEntity ) sgsVariable sgsCreateEntity( StringView type, sgsVariable data );
	SGS_METHOD_NAMED( DestroyEntity ) void sgsDestroyEntity( sgsVariable eh );
	SGS_METHOD_NAMED( FindEntity ) Entity::ScrHandle sgsFindEntity( StringView name );
	SGS_METHOD_NAMED( CallEntity ) void CallEntityByName( StringView name, StringView action );
	SGS_METHOD_NAMED( SetCameraPosDir ) void sgsSetCameraPosDir( Vec3 pos, Vec3 dir );
	SGS_METHOD_NAMED( WorldToScreen ) SGS_MULTRET sgsWorldToScreen( Vec3 pos );
	SGS_METHOD_NAMED( WorldToScreenPx ) SGS_MULTRET sgsWorldToScreenPx( Vec3 pos );
	SGS_METHOD_NAMED( GetCursorWorldPoint ) SGS_MULTRET sgsGetCursorWorldPoint();
	
	// ---
	SGS_IFUNC( GETINDEX ) int _getindex( SGS_ARGS_GETINDEXFUNC );
	SGS_IFUNC( SETINDEX ) int _setindex( SGS_ARGS_SETINDEXFUNC );
	
	void LightMesh( SGRX_MeshInstance* meshinst, Vec3 off = V3(0) );
	
	SGS_METHOD TimeVal GetTickTime(){ return m_currentTickTime * 1000.0; }
	SGS_METHOD TimeVal GetPhyTime(){ return m_currentPhyTime * 1000.0; }
	
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



template< class T > void LevelScrObj::_InitScriptInterface( T* ptr )
{
	T::_sgs_interface->destruct = NULL;
	SGS_CSCOPE( m_level->GetSGSC() );
	sgs_CreateClass( m_level->GetSGSC(), NULL, ptr );
	C = m_level->GetSGSC();
	m_sgsObject = sgs_GetObjectStruct( C, -1 );
	sgs_ObjAcquire( C, m_sgsObject );
}


