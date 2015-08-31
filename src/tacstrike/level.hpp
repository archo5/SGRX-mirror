

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
	IGameLevelSystem( GameLevel* lev, uint32_t uid ) : m_level( lev ), m_system_uid( uid ){}
	virtual ~IGameLevelSystem(){}
	virtual bool AddEntity( const StringView& type, sgsVariable data ){ return false; }
	virtual bool LoadChunk( const StringView& type, uint8_t* ptr, size_t size ){ return false; }
	virtual void Clear(){}
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual void DrawUI(){}
	
	virtual void PostDraw(){}
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	GameLevel* m_level;
	uint32_t m_system_uid;
};


struct Entity
{
	Entity();
	virtual ~Entity();
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual void OnEvent( const StringView& type ){}
	virtual void SetProperty( const StringView& name, sgsVariable value ){}
	virtual sgsVariable GetProperty( const StringView& name ){ return sgsVariable(); }
//	virtual bool GetInteractionInfo( Vec3 pos, InteractInfo* out ){ return false; }
	virtual bool CanInterruptAction( float progress ){ return false; }
//	virtual bool GetMapItemInfo( MapItemInfo* out ){ return false; }
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
	
	const char* m_typeName;
	String m_name;
	String m_viewName;
	GameLevel* m_level;
};


typedef StackString<16> StackShortName;

struct GameLevel : SGRX_PostDraw, SGRX_DebugDraw, SGRX_LightTreeSampler
{
	SGS_OBJECT;
	
	GameLevel();
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
	SGS_METHOD_NAMED( CallEntity ) void CallEntityByName( StringView name, StringView action );
	
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
	
	// SYSTEMS
	HashTable< StringView, Entity* > m_entNameMap;
	HashTable< String, Vec3 > m_markerMap;
	Array< IGameLevelSystem* > m_systems;
//	InfoEmissionSystem m_infoEmitters;
//	DamageSystem m_damageSystem;
//	BulletSystem m_bulletSystem;
//	MessagingSystem m_messageSystem;
//	ObjectiveSystem m_objectiveSystem;
//	FlareSystem m_flareSystem;
//	CoverSystem m_coverSystem;
//	AIDBSystem m_aidbSystem;
	
	// LEVEL DATA
	sgsVariable m_self;
	bool m_paused;
	float m_endFactor;
	double m_levelTime;
	Array< Entity* > m_entities;
	Entity* m_player;
	Vec3 m_playerSpawnInfo[2]; // position, direction
	Vec3 m_levelCameraInfo[2]; // position, direction
	sgsVariable m_cutsceneFunc;
	float m_cutsceneTime;
	String m_cutsceneSubtitle;
	SoundEventInstanceHandle m_music;
	
	// HELPER DATA
	Vec3 m_cachedCameraInfo[2];
	bool m_cameraInfoCached;
	String m_nextLevel;
};


