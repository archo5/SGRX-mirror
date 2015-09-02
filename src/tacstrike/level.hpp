

#pragma once

#include <engine.hpp>
#include <enganim.hpp>
#include <pathfinding.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#include "levelcache.hpp"


struct GameLevel;


typedef uint32_t TimeVal;
typedef uint32_t EntityID;

#define ENTID_NONE ((EntityID)0)


struct IGameLevelSystem
{
	IGameLevelSystem( GameLevel* lev, uint32_t uid ) :
		C( NULL ), m_sgsObject( NULL ),
		m_level( lev ),
		m_system_uid( uid )
	{}
	virtual ~IGameLevelSystem();
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
	
#define _InitScriptInterface( name ) _InitScriptInterface_( name, this )
	template< class T > void _InitScriptInterface_( const StringView& name, T* ptr );
	void _DestroyScriptInterface();
	SGS_CTX;
	sgs_VarObj* m_sgsObject;
	
	GameLevel* m_level;
	uint32_t m_system_uid;
};


struct Entity
{
	SGS_OBJECT;
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
	
	sgsVariable GetScriptedObject();
	
#define ENT_SGS_IMPLEMENT \
	virtual int _sgsDestruct(){ return _sgs_destruct( C, m_sgsObject ); } \
	virtual int _sgsGCMark(){ return _sgs_gcmark( C, m_sgsObject ); } \
	virtual int _sgsGetIndex( sgs_Variable* key, int isprop ){ return _sgs_getindex( C, m_sgsObject, key, isprop ); } \
	virtual int _sgsSetIndex( sgs_Variable* key, sgs_Variable* val, int isprop ){ return _sgs_setindex( C, m_sgsObject, key, val, isprop ); } \
	virtual int _sgsDump( int depth ){ return _sgs_dump( C, m_sgsObject, depth ); }
// --------------------------
	
	ENT_SGS_IMPLEMENT;
	SGS_IFUNC( DESTRUCT ) int _sgsent_destruct( SGS_CTX, sgs_VarObj* obj );
	SGS_IFUNC( GCMARK ) int _sgsent_gcmark( SGS_CTX, sgs_VarObj* obj );
	SGS_IFUNC( GETINDEX ) int _sgsent_getindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop );
	SGS_IFUNC( SETINDEX ) int _sgsent_setindex( SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, sgs_Variable* val, int isprop );
	SGS_IFUNC( DUMP ) int _sgsent_dump( SGS_CTX, sgs_VarObj* obj, int depth );
	
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


typedef StackString<16> StackShortName;

struct GameLevel : SGRX_PostDraw, SGRX_DebugDraw, SGRX_LightTreeSampler
{
	SGS_OBJECT;
	
	GameLevel( PhyWorldHandle phyWorld );
	virtual ~GameLevel();
	
	// configuration
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
	
	// system interface
	StringView GetLevelName() const { return m_levelName; }
	void SetPlayer( Entity* E ){ m_player = E; }
	bool IsPaused() const { return m_paused; }
	
	// entity interface
	SGRX_IPhyWorld* GetPhyWorld() const { return m_phyWorld; }
	SGRX_Scene* GetScene() const { return m_scene; }
	ScriptContext& GetScriptCtx(){ return m_scriptCtx; }
	sgs_Context* GetSGSC() const { return m_scriptCtx.C; }
	
	bool Load( const StringView& levelname );
	void CreateEntity( const StringView& type, sgsVariable data );
	StackShortName GenerateName();
	void StartLevel();
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
	
	void LightMesh( MeshInstHandle mih, Vec3 off = V3(0) );
	
	TimeVal GetTickTime(){ return m_currentTickTime * 1000.0; }
	TimeVal GetPhyTime(){ return m_currentPhyTime * 1000.0; }
	
	// ENGINE OBJECTS
	SceneHandle m_scene;
	ScriptContext m_scriptCtx;
	PhyWorldHandle m_phyWorld;
	
	// UTILITIES
	uint32_t m_nameIDGen;
	double m_currentTickTime;
	double m_currentPhyTime;
	String m_levelName;
	String m_nextLevel;
	
	// SYSTEMS
	HashTable< StringView, Entity* > m_entNameMap;
	Array< IGameLevelSystem* > m_systems;
	
	// LEVEL DATA
	sgsVariable m_self;
	sgsVariable m_markerPositions;
	bool m_paused;
	double m_levelTime;
	Array< Entity* > m_entities;
	Entity* m_player;
	Vec3 m_playerSpawnInfo[2]; // position, direction
	Vec3 m_levelCameraInfo[2]; // position, direction
	sgsVariable m_cutsceneFunc;
	float m_cutsceneTime;
	String m_cutsceneSubtitle;
	SoundEventInstanceHandle m_music;
};



template< class T > void IGameLevelSystem::_InitScriptInterface_( const StringView& name, T* ptr )
{
	SGS_CSCOPE( m_level->GetSGSC() );
	sgs_PushClass( m_level->GetSGSC(), ptr );
	m_level->AddEntry( name, sgsVariable( m_level->GetSGSC(), -1 ) );
	C = m_level->GetSGSC();
	m_sgsObject = sgs_GetObjectStruct( C, -1 );
	sgs_ObjAcquire( C, m_sgsObject );
}


