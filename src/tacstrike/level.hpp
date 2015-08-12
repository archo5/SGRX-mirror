

#pragma once

#include "entities.hpp"
#include "levelcache.hpp"


typedef StackString<16> StackShortName;

struct GameLevel : SGRX_PostDraw, SGRX_DebugDraw, SGRX_LightTreeSampler
{
	GameLevel();
	virtual ~GameLevel();
	
	bool Load( const StringView& levelname );
	void CreateEntity( const StringView& type, const StringView& sgsparams );
	StackShortName GenerateName();
	void StartLevel();
	void EndLevel();
	
	void ProcessEvents();
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void Draw2D();
	void DebugDraw();
	void PostDraw();
	void Draw();
	
	void SetNextLevel( const StringView& name );
	void MapEntityByName( Entity* e );
	void UnmapEntityByName( Entity* e );
	Entity* FindEntityByName( const StringView& name );
	void CallEntityByName( const StringView& name, const StringView& action );
	
	void LightMesh( MeshInstHandle mih, Vec3 off = V3(0) );
	
	TimeVal GetTickTime(){ return m_currentTickTime * 1000.0; }
	TimeVal GetPhyTime(){ return m_currentPhyTime * 1000.0; }
	
	// UTILITIES
	ScriptContext m_scriptCtx;
	uint32_t m_nameIDGen;
	double m_currentTickTime;
	double m_currentPhyTime;
	
	// SYSTEMS
	HashTable< StringView, Entity* > m_entNameMap;
	HashTable< String, Vec3 > m_markerMap;
	InfoEmissionSystem m_infoEmitters;
	DamageSystem m_damageSystem;
	BulletSystem m_bulletSystem;
	MessagingSystem m_messageSystem;
	ObjectiveSystem m_objectiveSystem;
	FlareSystem m_flareSystem;
	AIDBSystem m_aidbSystem;
	
	// LEVEL DATA
	bool m_paused;
	float m_endFactor;
	double m_levelTime;
	EntityArray m_entities;
	MeshInstArray m_meshInsts;
	PhyBodyArray m_levelBodies;
	LCLightArray m_lights;
	Array< Vec2 > m_lines;
	SGRX_LightTree m_ltSamples;
	SceneHandle m_scene;
	Player* m_player;
	Vec3 m_playerSpawnInfo[2]; // position, direction
	Vec3 m_levelCameraInfo[2]; // position, direction
	
	// HELPER DATA
	Vec3 m_cachedCameraInfo[2];
	bool m_cameraInfoCached;
	String m_nextLevel;
	
	// COMMON DATA
	TextureHandle m_tex_mapline;
	TextureHandle m_tex_mapframe;
};


