

#pragma once

#include "entities.hpp"
#include "levelcache.hpp"


typedef StackString<16> StackShortName;

struct GameLevel : SGRX_PostDraw, SGRX_DebugDraw
{
	GameLevel();
	virtual ~GameLevel();
	
	bool Load( const StringView& levelname );
	void ClearLevel();
	void CreateEntity( const StringView& type, const StringView& sgsparams );
	StackShortName GenerateName();
	void StartLevel();
	void EndLevel();
	
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void Draw2D();
	void DebugDraw();
	void PostDraw();
	void Draw();
	
	void MapEntityByName( Entity* e );
	void UnmapEntityByName( Entity* e );
	Entity* FindEntityByName( const StringView& name );
	void CallEntityByName( const StringView& name, const StringView& action );
	
	void LightMesh( MeshInstHandle mih, int32_t* outlastfound = NULL );
	
	// UTILITIES
	ScriptContext m_scriptCtx;
	uint32_t m_nameIDGen;
	
	// SYSTEMS
	HashTable< StringView, Entity* > m_entNameMap;
	InfoEmissionSystem m_infoEmitters;
	DamageSystem m_damageSystem;
	BulletSystem m_bulletSystem;
	MessagingSystem m_messageSystem;
	ObjectiveSystem m_objectiveSystem;
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
	LightTree m_ltSamples;
	SceneHandle m_scene;
	Player* m_player;
	Vec3 m_playerSpawnInfo[2]; // position, direction
	Vec3 m_levelCameraInfo[2]; // position, direction
	
	// HELPER DATA
	Vec3 m_cachedCameraInfo[2];
	bool m_cameraInfoCached;
	
	// COMMON DATA
	PixelShaderHandle m_ps_flare;
	TextureHandle m_tex_flare;
	TextureHandle m_tex_mapline;
	TextureHandle m_tex_mapframe;
};


