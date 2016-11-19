

#pragma once
#include <engine.hpp>
#include <enganim.hpp>
#include <engext.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#include "level.hpp"
#include "pathfinding.hpp"


// SYSTEM ID ALLOCATION (increment to allocate)
// last id = 15


//
// INTERFACES
//


enum MeshInstEvent
{
	MIEVT_BulletHit = 1,
};

struct MI_BulletHit_Data
{
	Vec3 pos;
	Vec3 vel;
	float dmg;
	float critDmg;
};

struct SGRX_MeshInstUserData
{
	SGRX_MeshInstUserData() : dmgDecalSysOverride(NULL),
		ovrDecalSysOverride(NULL), ownerID(NULL), typeOverride(NULL){}
	virtual ~SGRX_MeshInstUserData(){}
	virtual void MeshInstUser_OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data ){}
	SGRX_DecalSystem* dmgDecalSysOverride;
	SGRX_DecalSystem* ovrDecalSysOverride;
	void* ownerID;
	const char* typeOverride;
};


EXP_STRUCT LevelMapSystem : IGameLevelSystem
{
	SGS_OBJECT;
	SGS_NO_DESTRUCT;
	IMPLEMENT_SYSTEM( LevelMapSystem, 9 );
	
	GFW_EXPORT LevelMapSystem( GameLevel* lev );
	GFW_EXPORT void Clear();
	GFW_EXPORT bool LoadChunk( const StringView& type, ByteView data );
	
	GFW_EXPORT SGS_METHOD void DrawUIRect( float x0, float y0, float x1, float y1, float linesize, sgsVariable cb );
	
	SGS_PROPERTY Vec3 viewPos;
	
	Array< Vec2 > m_lines;
	Array< LC_Map_Layer > m_layers;
	
	TextureHandle m_tex_mapline;
};


EXP_STRUCT LevelCoreSystem : IGameLevelSystem
{
	IMPLEMENT_SYSTEM( LevelCoreSystem, 10 );
	
	GFW_EXPORT LevelCoreSystem( GameLevel* lev );
	GFW_EXPORT virtual void Clear();
	GFW_EXPORT virtual bool LoadChunk( const StringView& type, ByteView data );
	
	Array< MeshInstHandle > m_meshInsts;
	Array< LightHandle > m_lightInsts;
	Array< PhyRigidBodyHandle > m_levelBodies;
	Array< LC_Light > m_lights;
	SGRX_LightEnv m_lightEnv;
};


EXP_STRUCT GFXSystem : IGameLevelSystem, SGRX_RenderDirector, SGRX_IEventHandler
{
	IMPLEMENT_SYSTEM( GFXSystem, 15 );
	
	GFW_EXPORT GFXSystem( GameLevel* lev );
	GFW_EXPORT void HandleEvent( SGRX_EventID eid, const EventData& edata );
	
	GFW_EXPORT virtual void OnDrawScene( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info );
	GFW_EXPORT virtual void OnDrawSceneWithRefl( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info );
	GFW_EXPORT virtual void OnDrawSceneGeom( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info,
		SGRX_RTSpec rtt, DepthStencilSurfHandle dss, TextureHandle rttDEPTH );
	
	Array< struct ReflectionPlaneResource* > m_reflectPlanes;
	Array< struct FlareResource* > m_flares;
};


EXP_STRUCT ScriptedSequenceSystem : IGameLevelSystem
{
	SGS_OBJECT;
	SGS_NO_DESTRUCT;
	IMPLEMENT_SYSTEM( ScriptedSequenceSystem, 11 );
	
	GFW_EXPORT ScriptedSequenceSystem( GameLevel* lev );
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void DrawUI();
	void _StartCutscene(){ m_time = 0; }
	
	GFW_EXPORT SGS_METHOD_NAMED( Start ) void sgsStart( sgsVariable func, float t );
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _StartCutscene VARNAME func ) sgsVariable m_func;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME time ) float m_time;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME subtitle ) String m_subtitle;
};


EXP_STRUCT MusicSystem : IGameLevelSystem
{
	SGS_OBJECT;
	SGS_NO_DESTRUCT;
	IMPLEMENT_SYSTEM( MusicSystem, 12 );
	
	GFW_EXPORT MusicSystem( GameLevel* lev );
	GFW_EXPORT ~MusicSystem();
	
	GFW_EXPORT SGS_METHOD_NAMED( SetTrack ) void sgsSetTrack( StringView path );
	GFW_EXPORT SGS_METHOD_NAMED( SetVar ) void sgsSetVar( StringView name, float val );
	
	SoundEventInstanceHandle m_music;
};


EXP_STRUCT DamageSystem : IGameLevelSystem, SGRX_ScenePSRaycast
{
	IMPLEMENT_SYSTEM( DamageSystem, 5 );
	
	struct Material : SGRX_RefCounted
	{
		Material() : isSolid(true){}
		
		String match;
		Array< int > decalIDs;
		ParticleSystem particles;
		String sound;
		bool isSolid;
		
		bool CheckMatch( const StringView& test ) const { return match.size() == 0 || test.contains( match ); }
	};
	typedef Handle< Material > MtlHandle;
	
	GFW_EXPORT DamageSystem( GameLevel* lev );
	GFW_EXPORT ~DamageSystem();
	GFW_EXPORT const char* Init( SceneHandle scene, SGRX_LightSampler* sampler );
	GFW_EXPORT void Free();
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT bool AddBulletDamage( SGRX_DecalSystem* dmgDecalSysOverride,
		const StringView& type, SGRX_IMesh* m_targetMesh, int partID,
		const Mat4& worldMatrix, const Vec3& pos, const Vec3& dir, const Vec3& nrm, float scale = 1.0f );
	GFW_EXPORT void AddBlood( Vec3 pos, Vec3 dir );
	GFW_EXPORT void Clear();
	
	void DoFX( const Vec3& pos, const Vec3& nrm, uint32_t fx )
	{
		if( fx == 1 ){ AddBlood( pos + nrm, -nrm ); }
	}
	
	Array< MtlHandle > m_bulletDecalMaterials;
	Array< DecalMapPartInfo > m_bulletDecalInfo;
	SGRX_DecalSystem m_bulletDecalSys;
	SGRX_DecalSystem m_bloodDecalSys;
};


struct BulletSystem : IGameLevelSystem
{
	SGS_OBJECT;
	SGS_NO_DESTRUCT;
	IMPLEMENT_SYSTEM( BulletSystem, 6 );
	
	struct Bullet
	{
		Vec3 position;
		Vec3 velocity;
		Vec3 dir;
		float timeleft;
		Vec2 damage;
		void* ownerToSkip;
		// penetration depth calculations
		Vec3 intersectStart;
		int numSolidRefs;
	};
	typedef Array< Bullet > BulletArray;
	
	GFW_EXPORT BulletSystem( GameLevel* lev );
	
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	
	GFW_EXPORT SGS_METHOD void Add( Vec3 pos, Vec3 vel, float timeleft, Vec2 dmg, void* ownerID );
	GFW_EXPORT SGS_METHOD float Zap( Vec3 p1, Vec3 p2, Vec2 dmg, void* ownerID );
	GFW_EXPORT void Clear();
	
	float _ProcessBullet( Vec3 p1, Vec3 p2, Bullet& B );
	
	BulletArray m_bullets;
	DamageSystem* m_damageSystem;
	
	Array< SceneRaycastInfo > m_tmpStore;
};


enum AISoundType
{
	AIS_Unknown = 0,
	AIS_Footstep,
	AIS_Shot,
	AIS_AccessDenied,
};

struct AISound
{
	Vec3 position;
	float radius;
	float timeout;
	AISoundType type;
};

struct AIRoomPart
{
	Mat4 bbox_xf;
	Mat4 inv_bbox_xf;
	Vec3 scale;
	bool negative;
	float cell_size;
};

EXP_STRUCT AIRoom : SGRX_RCRsrc
{
	GFW_EXPORT bool IsInside( Vec3 pos );
	
	Array< AIRoomPart > parts;
};
typedef Handle< AIRoom > AIRoomHandle;

enum AIFactType // some basic fact types
{
	FT_Unknown = 0,
	FT_Sound_Noise,
	FT_Sound_Footstep,
	FT_Sound_Shot,
	FT_Sound_AccessDenied,
	FT_Sight_ObjectState,
	FT_Sight_Alarming,
	FT_Sight_Friend,
	FT_Sight_Foe,
	FT_Position_Friend,
	FT_Position_Foe,
};

EXP_STRUCT AIFact
{
	SGS_OBJECT_LITE;
	
	SGS_PROPERTY uint32_t id;
	SGS_PROPERTY uint32_t ref;
	SGS_PROPERTY uint32_t type; // AIFactType
	SGS_PROPERTY Vec3 position;
	SGS_PROPERTY TimeVal created;
	SGS_PROPERTY TimeVal expires;
};

EXP_STRUCT AICharInfo
{
	float suspicion;
	bool suspicionIncreased;
	
	bool IsSuspicious(){ return suspicion >= 1; }
	void IncreaseSuspicion( float amt )
	{
		suspicion += amt;
		suspicion = clamp( suspicion, 0, 1 );
		suspicionIncreased = true;
	}
};

EXP_STRUCT AIZoneInfo
{
	SGS_OBJECT_LITE;
	
	uint32_t restrictedGroups;
	float suspicionFactor;
	int32_t priority;
	Mat4 invBBXF;
	
	template< class T > void Serialize( T& arch )
	{
		arch << restrictedGroups;
		arch << suspicionFactor;
		arch << priority;
		arch << invBBXF;
	}
};

EXP_STRUCT AICoverRange
{
	uint32_t from;
	uint32_t to;
};

EXP_STRUCT AIFactDistance
{
	GFW_EXPORT virtual float GetDistance( const AIFact& fact ) = 0;
	GFW_EXPORT virtual Vec3 GetPosition() = 0;
};

EXP_STRUCT AIFactStorage
{
	GFW_EXPORT AIFactStorage();
	GFW_EXPORT void Clear();
	GFW_EXPORT void SortCreatedDesc();
	GFW_EXPORT void Process( TimeVal curTime );
	GFW_EXPORT bool HasFact( uint32_t typemask );
	GFW_EXPORT bool ExpireFacts( uint32_t typemask );
	GFW_EXPORT bool HasRecentFact( uint32_t typemask, TimeVal maxtime );
	GFW_EXPORT bool HasRecentFactAt( uint32_t typemask, TimeVal maxtime, Vec3 pos, float rad );
	GFW_EXPORT AIFact* GetRecentFact( uint32_t typemask, TimeVal maxtime );
	GFW_EXPORT void Insert( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref = 0 );
	GFW_EXPORT void RemoveExt( uint32_t* types, size_t typecount );
	void Remove( uint32_t type ){ RemoveExt( &type, 1 ); }
	GFW_EXPORT bool Update( uint32_t type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	GFW_EXPORT void InsertOrUpdate( uint32_t type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	GFW_EXPORT bool CustomUpdate( AIFactDistance& distfn,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	GFW_EXPORT void CustomInsertOrUpdate( AIFactDistance& distfn, uint32_t type,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	
	GFW_EXPORT AICharInfo& GetCharInfo( GameObject* obj );
	GFW_EXPORT void DecreaseSuspicion( float amt );
	
	Array< AIFact > facts;
	HashTable< GameObject::ScrHandle, AICharInfo > charInfo;
	
	TimeVal m_lastTime;
	uint32_t last_mod_id;
	uint32_t m_next_fact_id;
};

EXP_STRUCT AIDBSystem : IGameLevelSystem
{
	SGS_OBJECT;
	SGS_NO_DESTRUCT;
	IMPLEMENT_SYSTEM( AIDBSystem, 7 );
	
	int GetNumSounds(){ return m_sounds.size(); }
	GFW_EXPORT bool CanHearSound( Vec3 pos, int i );
	AISound GetSoundInfo( int i ){ return m_sounds[ i ]; }
	
	GFW_EXPORT AIDBSystem( GameLevel* lev );
	GFW_EXPORT void Clear();
	GFW_EXPORT bool LoadChunk( const StringView& type, ByteView data );
	GFW_EXPORT IEditorSystemCompiler* EditorGetSystemCompiler();
	
	GFW_EXPORT void AddSound( Vec3 pos, float rad, float timeout, AISoundType type );
	GFW_EXPORT void AddRoomPart( const StringView& name, Mat4 xf, bool negative, float cell_size );
	
	GFW_EXPORT AIRoom* FindRoomByPos( Vec3 pos );
	GFW_EXPORT AIZoneInfo GetZoneInfoByPos( Vec3 pos );
	
	GFW_EXPORT void _PrepareCoverRanges();
	GFW_EXPORT LC_CoverPart* FindCover( Vec3 pos, Vec3 target, uint32_t mask = 0, uint32_t req = 0, bool inv = false );
	
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void FixedTick( float deltaTime );
	GFW_EXPORT void DebugDrawWorld();
	
	// navmesh / cover data
	SGRX_Pathfinder m_pathfinder;
	Array< LC_CoverPart > m_coverParts;
	HashTable< uint32_t, AICoverRange > m_tileCoverRanges;
	// - internal
	Array< uint32_t> m_fcPolyQueue;
	HashTable< uint32_t, NoValue > m_fcVisitedSet;
	// misc. spatial markers
	HashTable< StringView, AIRoomHandle > m_rooms;
	Array< AIZoneInfo > m_zones;
	uint32_t m_defaultRestrictedGroups;
	float m_defaultSuspicionFactor;
	// world facts
	Array< AISound > m_sounds;
	AIFactStorage m_globalFacts;
	
	GFW_EXPORT SGS_METHOD_NAMED( AddSound ) void sgsAddSound( Vec3 pos, float rad, float timeout, int type );
	
	GFW_EXPORT SGS_METHOD_NAMED( HasFact ) bool sgsHasFact( uint32_t typemask );
	GFW_EXPORT SGS_METHOD_NAMED( HasRecentFact ) bool sgsHasRecentFact( uint32_t typemask, TimeVal maxtime );
	GFW_EXPORT SGS_METHOD_NAMED( GetRecentFact ) SGS_MULTRET sgsGetRecentFact( uint32_t typemask, TimeVal maxtime );
	GFW_EXPORT SGS_METHOD_NAMED( InsertFact ) void sgsInsertFact( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref );
	GFW_EXPORT SGS_METHOD_NAMED( UpdateFact ) bool sgsUpdateFact( uint32_t type, Vec3 pos,
		float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	GFW_EXPORT SGS_METHOD_NAMED( InsertOrUpdateFact ) void sgsInsertOrUpdateFact(
		uint32_t type, Vec3 pos, float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	GFW_EXPORT SGS_MULTRET sgsPushRoom( AIRoom* room );
	GFW_EXPORT SGS_METHOD_NAMED( GetRoomList ) SGS_MULTRET sgsGetRoomList();
	GFW_EXPORT SGS_METHOD_NAMED( GetRoomNameByPos ) sgsString sgsGetRoomNameByPos( Vec3 pos );
	GFW_EXPORT SGS_METHOD_NAMED( GetRoomByPos ) SGS_MULTRET sgsGetRoomByPos( Vec3 pos );
	GFW_EXPORT SGS_METHOD_NAMED( GetRoomPoints ) SGS_MULTRET sgsGetRoomPoints( StringView name );
	GFW_EXPORT SGS_METHOD_NAMED( GetZoneInfoByPos ) SGS_MULTRET sgsGetZoneInfoByPos( Vec3 pos );
};


EXP_STRUCT DevelopSystem : IGameLevelSystem, SGRX_IEventHandler
{
	IMPLEMENT_SYSTEM( DevelopSystem, 14 );
	
	GFW_EXPORT DevelopSystem( GameLevel* lev );
	GFW_EXPORT void HandleEvent( SGRX_EventID eid, const EventData& edata );
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void PreRender();
	GFW_EXPORT void DrawUI();
	
	bool screenshotMode : 1;
	bool moveMult : 1;
	bool moveFwd : 1;
	bool moveBwd : 1;
	bool moveLft : 1;
	bool moveRgt : 1;
	bool moveUp : 1;
	bool moveDn : 1;
	bool rotView : 1;
	bool rotLft : 1;
	bool rotRgt : 1;
	Vec3 cameraPos;
	YawPitch cameraDir;
	float cameraRoll;
	
	bool consoleMode : 1;
	bool justEnabledConsole : 1;
	String inputText;
};


