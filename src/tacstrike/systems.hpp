

#pragma once
#include <engine.hpp>
#include <enganim.hpp>
#include <engext.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#include "level.hpp"
#include "pathfinding.hpp"
#include "../common/scritem.hpp"


// SYSTEM ID ALLOCATION (increment to allocate)
// last id = 14


//
// INTERFACES
//

enum EInteractionType
{
	IT_None = 0,
	IT_Button,
	IT_Pickup,
	IT_Investigate,
	IT_PickLock,
};
#define IA_NEEDS_LONG_END(t) ((t)==IT_Investigate||(t)==IT_PickLock)

struct InteractInfo
{
	EInteractionType type;
	Vec3 placePos; // placement
	Vec3 placeDir;
	float timeEstimate;
	float timeActual;
};

struct IInteractiveEntity
{
	enum { e_iface_uid = 1 };
	
	virtual bool GetInteractionInfo( Vec3 pos, InteractInfo* out ){ return false; }
	virtual bool CanInterruptAction( float progress ){ return false; }
};


enum EMapItemType
{
	MI_None = 0,
	MI_Mask_Object = 0xff00,
	MI_Mask_State = 0x00ff,
	MI_Object_Player = 0x0100,
	MI_Object_Enemy = 0x0200,
	MI_Object_Camera = 0x0300,
	MI_State_Normal = 0x0001,
	MI_State_Suspicious = 0x0002,
	MI_State_Alerted = 0x0003,
};

struct MapItemInfo
{
	int type; // EMapItemType combo
	Vec3 position;
	Vec3 direction;
	float sizeFwd;
	float sizeRight;
};

struct LevelMapSystem : IGameLevelSystem
{
	enum { e_system_uid = 9 };
	
	LevelMapSystem( GameLevel* lev );
	void Clear();
	bool LoadChunk( const StringView& type, ByteView data );
	void UpdateItem( Entity* e, const MapItemInfo& data );
	void RemoveItem( Entity* e );
	void DrawUI();
	
	Vec2 m_viewPos;

	HashTable< Entity*, MapItemInfo > m_mapItemData;
	Array< Vec2 > m_lines;
	
	TextureHandle m_tex_mapline;
	TextureHandle m_tex_mapframe;
};


struct MSMessage
{
	enum Type
	{
		Continued,
		Info,
		Warning,
	};
	
	Type type;
	String text;
	float tmlength;
	float position;
};

struct MessagingSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 2 };
	
	Array< MSMessage > m_messages;
	
	MessagingSystem( GameLevel* lev );
	void Clear();
	void AddMessage( MSMessage::Type type, const StringView& sv, float tmlength = 3 );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	SGS_METHOD_NAMED( Add ) void sgsAddMsg( int type, StringView text, float time );
	
	TextureHandle m_tx_icon_info;
	TextureHandle m_tx_icon_warning;
	TextureHandle m_tx_icon_cont;
};


struct OSObjective
{
	enum State
	{
		Hidden = 0,
		Open,
		Done,
		Failed,
		Cancelled,
	};
	
	String title;
	String desc;
	State state;
	bool required;
	bool hasLocation;
	Vec3 location;
};

struct OSObjStats
{
	int numTotal;
	int numHidden;
	int numOpen;
	int numDone;
	int numFailed;
	int numCancelled;
};

struct ObjectiveSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 3 };
	
	ObjectiveSystem( GameLevel* lev );
	void Clear();
	int AddObjective(
		const StringView& title,
		OSObjective::State state,
		const StringView& desc = "",
		bool required = false,
		Vec3* location = NULL );
	OSObjStats GetStats();
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	bool _CheckRange( int i );
	SGS_METHOD_NAMED( Add ) int sgsAddObj( StringView title, int state, StringView desc, bool req, Vec3 loc );
	SGS_METHOD_NAMED( GetTitle ) StringView sgsGetTitle( int i );
	SGS_METHOD_NAMED( SetTitle ) void sgsSetTitle( int i, StringView title );
	SGS_METHOD_NAMED( GetState ) int sgsGetState( int i );
	SGS_METHOD_NAMED( SetState ) void sgsSetState( int i, int state );
	SGS_METHOD_NAMED( SetLocation ) void sgsSetLocation( int i, Vec3 loc );
	
	InputState SHOW_OBJECTIVES;
	
	Array< OSObjective > m_objectives;
	
	float m_alpha;
	
	TextureHandle m_tx_icon_open;
	TextureHandle m_tx_icon_done;
	TextureHandle m_tx_icon_failed;
};


struct HelpTextSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 13 };
	
	HelpTextSystem( GameLevel* lev );
	void Clear();
	void SetText( StringView text, float alpha = 1, float fadetime = 0, float fadeto = 0 );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	SGS_PROPERTY_FUNC( READ WRITE VARNAME text ) String m_text;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME alpha ) float m_alpha;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME fadeTime ) float m_fadeTime;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME fadeTo ) float m_fadeTo;
	SGS_METHOD_NAMED( Clear ) void sgsClear();
	SGS_METHOD_NAMED( SetText ) void sgsSetText( StringView text, float alpha, float fadetime, float fadeto );
	
	// renderer access
	SGS_PROPERTY_FUNC( READ WRITE VALIDATE renderer SOURCE renderer->fontSize ) SGS_ALIAS( int fontSize );
	SGS_PROPERTY_FUNC( READ WRITE VALIDATE renderer SOURCE renderer->centerPos ) SGS_ALIAS( Vec2 centerPos );
	SGS_PROPERTY_FUNC( READ WRITE VALIDATE renderer SOURCE renderer->lineHeightFactor ) SGS_ALIAS( float lineHeightFactor );
	
	SGRX_HelpTextRenderer* renderer;
};


struct FSFlare
{
	Vec3 pos;
	Vec3 color;
	float size;
	bool enabled;
};

struct FlareSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 4 };
	
	FlareSystem( GameLevel* lev );
	void Clear();
	void UpdateFlare( void* handle, const FSFlare& flare );
	bool RemoveFlare( void* handle );
	void PostDraw();
	
	SGS_METHOD_NAMED( Update ) void sgsUpdate( void* handle, Vec3 pos, Vec3 col, float size, bool enabled );
	SGS_METHOD_NAMED( Remove ) void sgsRemove( void* handle );
	SGS_PROPERTY_FUNC( READ WRITE VARNAME layers ) uint32_t m_layers;
	
	HashTable< void*, FSFlare > m_flares;
	PixelShaderHandle m_ps_flare;
	TextureHandle m_tex_flare;
};


struct LevelCoreSystem : IGameLevelSystem
{
	enum { e_system_uid = 10 };
	
	LevelCoreSystem( GameLevel* lev );
	virtual void Clear();
	virtual bool LoadChunk( const StringView& type, ByteView data );
	
	Array< MeshInstHandle > m_meshInsts;
	Array< LightHandle > m_lightInsts;
	Array< PhyRigidBodyHandle > m_levelBodies;
	Array< LC_Light > m_lights;
	SGRX_LightTree m_ltSamples;
};


struct ScriptedSequenceSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 11 };
	
	ScriptedSequenceSystem( GameLevel* lev );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	void _StartCutscene(){ m_time = 0; }
	
	InputState m_cmdSkip;
	SGS_METHOD_NAMED( Start ) void sgsStart( sgsVariable func, float t );
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _StartCutscene VARNAME func ) sgsVariable m_func;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME time ) float m_time;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME subtitle ) String m_subtitle;
};


struct MusicSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 12 };
	
	MusicSystem( GameLevel* lev );
	~MusicSystem();
	
	SGS_METHOD_NAMED( SetTrack ) void sgsSetTrack( StringView path );
	SGS_METHOD_NAMED( SetVar ) void sgsSetVar( StringView name, float val );
	
	SoundEventInstanceHandle m_music;
};


enum GameActorType // = SGRX_MeshInstUserData::ownerType
{
	GAT_None = 0,
	GAT_Player = 1,
	GAT_Enemy = 2,
};


struct DamageSystem : IGameLevelSystem, SGRX_ScenePSRaycast
{
	enum { e_system_uid = 5 };
	
	struct Material : SGRX_RefCounted
	{
		String match;
		Array< int > decalIDs;
		ParticleSystem particles;
		String sound;
		
		bool CheckMatch( const StringView& test ) const { return match.size() == 0 || test.contains( match ); }
	};
	typedef Handle< Material > MtlHandle;
	
	DamageSystem( GameLevel* lev );
	~DamageSystem();
	const char* Init( SceneHandle scene, SGRX_LightSampler* sampler );
	void Free();
	void Tick( float deltaTime, float blendFactor );
	void AddBulletDamage( SGRX_DecalSystem* dmgDecalSysOverride,
		const StringView& type, SGRX_IMesh* m_targetMesh, int partID,
		const Mat4& worldMatrix, const Vec3& pos, const Vec3& dir, const Vec3& nrm, float scale = 1.0f );
	void AddBlood( Vec3 pos, Vec3 dir );
	void Clear();
	
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
	enum { e_system_uid = 6 };
	
	struct Bullet
	{
		Vec3 position;
		Vec3 velocity;
		Vec3 dir;
		float timeleft;
		float damage;
		uint32_t ownerType; // GameActorType
		// penetration depth calculations
		Vec3 intersectStart;
		int numSolidRefs;
	};
	typedef Array< Bullet > BulletArray;
	
	BulletSystem( GameLevel* lev );
	
	void Tick( float deltaTime, float blendFactor );
	
	void Add( const Vec3& pos, const Vec3& vel, float timeleft, float dmg, uint32_t ownerType );
	void Clear();
	
	BulletArray m_bullets;
	DamageSystem* m_damageSystem;
	
	Array< SceneRaycastInfo > m_tmpStore;
};


enum AISoundType
{
	AIS_Unknown = 0,
	AIS_Footstep,
	AIS_Shot,
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

struct AIRoom : SGRX_RCRsrc
{
	bool IsInside( Vec3 pos );
	
	Array< AIRoomPart > parts;
};
typedef Handle< AIRoom > AIRoomHandle;

enum AIFactType // some basic fact types
{
	FT_Unknown = 0,
	FT_Sound_Noise,
	FT_Sound_Footstep,
	FT_Sound_Shot,
	FT_Sight_ObjectState,
	FT_Sight_Alarming,
	FT_Sight_Friend,
	FT_Sight_Foe,
	FT_Position_Friend,
	FT_Position_Foe,
};

struct AIFact
{
	SGS_OBJECT_LITE;
	
	SGS_PROPERTY uint32_t id;
	SGS_PROPERTY uint32_t ref;
	SGS_PROPERTY uint32_t type; // AIFactType
	SGS_PROPERTY Vec3 position;
	SGS_PROPERTY TimeVal created;
	SGS_PROPERTY TimeVal expires;
};

struct AIFactDistance
{
	virtual float GetDistance( const AIFact& fact ) = 0;
	virtual Vec3 GetPosition() = 0;
};

struct AIFactStorage
{
	AIFactStorage();
	void Clear();
	void SortCreatedDesc();
	void Process( TimeVal curTime );
	bool HasFact( uint32_t typemask );
	bool HasRecentFact( uint32_t typemask, TimeVal maxtime );
	AIFact* GetRecentFact( uint32_t typemask, TimeVal maxtime );
	void Insert( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref = 0 );
	void RemoveExt( uint32_t* types, size_t typecount );
	void Remove( uint32_t type ){ RemoveExt( &type, 1 ); }
	bool Update( uint32_t type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	void InsertOrUpdate( uint32_t type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	bool CustomUpdate( AIFactDistance& distfn,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	void CustomInsertOrUpdate( AIFactDistance& distfn, uint32_t type,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	
	Array< AIFact > facts;
	TimeVal m_lastTime;
	uint32_t last_mod_id;
	uint32_t m_next_fact_id;
};

struct AIDBSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 7 };
	
	int GetNumSounds(){ return m_sounds.size(); }
	bool CanHearSound( Vec3 pos, int i );
	AISound GetSoundInfo( int i ){ return m_sounds[ i ]; }
	
	AIDBSystem( GameLevel* lev );
	bool LoadChunk( const StringView& type, ByteView data );
	void AddSound( Vec3 pos, float rad, float timeout, AISoundType type );
	void AddRoomPart( const StringView& name, Mat4 xf, bool negative, float cell_size );
	AIRoom* FindRoomByPos( Vec3 pos );
	void Tick( float deltaTime, float blendFactor );
	void FixedTick( float deltaTime );
	void DebugDrawWorld();
	
	SGRX_Pathfinder m_pathfinder;
	Array< AISound > m_sounds;
	HashTable< StringView, AIRoomHandle > m_rooms;
	AIFactStorage m_globalFacts;
	
	SGS_METHOD_NAMED( AddSound ) void sgsAddSound( Vec3 pos, float rad, float timeout, int type );
	
	SGS_METHOD_NAMED( HasFact ) bool sgsHasFact( uint32_t typemask );
	SGS_METHOD_NAMED( HasRecentFact ) bool sgsHasRecentFact( uint32_t typemask, TimeVal maxtime );
	SGS_METHOD_NAMED( GetRecentFact ) SGS_MULTRET sgsGetRecentFact( sgs_Context* coro, uint32_t typemask, TimeVal maxtime );
	SGS_METHOD_NAMED( InsertFact ) void sgsInsertFact( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref );
	SGS_METHOD_NAMED( UpdateFact ) bool sgsUpdateFact( sgs_Context* coro, uint32_t type, Vec3 pos,
		float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	SGS_METHOD_NAMED( InsertOrUpdateFact ) void sgsInsertOrUpdateFact( sgs_Context* coro,
		uint32_t type, Vec3 pos, float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	SGS_MULTRET sgsPushRoom( sgs_Context* coro, AIRoom* room );
	SGS_METHOD_NAMED( GetRoomList ) SGS_MULTRET sgsGetRoomList( sgs_Context* coro );
	SGS_METHOD_NAMED( GetRoomNameByPos ) sgsString sgsGetRoomNameByPos( sgs_Context* coro, Vec3 pos );
	SGS_METHOD_NAMED( GetRoomByPos ) SGS_MULTRET sgsGetRoomByPos( sgs_Context* coro, Vec3 pos );
	SGS_METHOD_NAMED( GetRoomPoints ) SGS_MULTRET sgsGetRoomPoints( sgs_Context* coro, StringView name );
};


struct CSCoverLine
{
	Vec3 p0;
	Vec3 p1;
};

struct CSCoverInfo
{
	struct Shape
	{
		size_t offset;
		int numPlanes;
	};
	
	void Clear();
	bool GetPosition( Vec3 position, float distpow, Vec3& out, float interval = 0.1f );
	void ClipWithSpheres( Vec4* spheres, int count );
	
	size_t _GetBestFactorID();
	void _CullWithShadowLines( size_t firstcover, Vec4 P );
	void _CullWithSolids();
	
	Array< Vec4 > planes;
	Array< Shape > shapes;
	Array< CSCoverLine > covers;
	Array< float > factors;
};

struct CoverSystem : IGameLevelSystem
{
	enum { e_system_uid = 8 };
	
	struct Edge
	{
#if 0
		int pl0;
		int pl1;
#else
		Vec3 p0; // endpoint 0
		Vec3 p1; // endpoint 1
		Vec3 n0; // adjacent plane 0 normal
		Vec3 n1; // adjacent plane 1 normal
		float d0; // adjacent plane 0 distance
		float d1; // adjacent plane 1 distance
		
		bool cover; // is cover edge (bottom/side)
		Vec3 nout; // outwards extension direction if p0/p1 match
		Vec3 nup; // upwards extension direction if p0/p1 match
#endif
	};
	struct CoverPoint
	{
		Vec3 pos;
		Vec3 nout;
		Vec3 nup;
		
		void AdjustNormals( Vec3 newout, Vec3 newup );
		bool operator == ( const CoverPoint& o ) const { return pos == o.pos; }
	};
	struct EdgeMesh : SGRX_RCRsrc
	{
		// silhouette info
		Array< Edge > edges;
		// solid info
		Array< Vec4 > planes;
		// cover data
		Array< CoverPoint > coverpts;
		Array< uint16_t > coveridcs;
		
		Vec3 pos;
		Vec3 bbmin;
		Vec3 bbmax;
		bool enabled;
		
		Mat4 inv_bbox_xf;
		Vec3 obb_min;
		Vec3 obb_max;
		
		bool InAABB( const Vec3& ibmin, const Vec3& ibmax ) const;
		bool PointInBox( Vec3 pt ) const;
		void CalcCoverLines();
	};
	typedef Handle< EdgeMesh > EdgeMeshHandle;
	
	CoverSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid ){}
	void Clear();
	void AddAABB( StringView name, Vec3 bbmin, Vec3 bbmax, Mat4 mtx );
	
	void QueryLines( Vec3 bbmin, Vec3 bbmax, float dist,
		float height, Vec3 viewer, bool visible, CSCoverInfo& cinfo );
	
	Array< EdgeMeshHandle > m_edgeMeshes;
	HashTable< StringView, EdgeMeshHandle > m_edgeMeshesByName;
};


struct DevelopSystem : IGameLevelSystem, SGRX_IEventHandler
{
	enum { e_system_uid = 14 };
	
	DevelopSystem( GameLevel* lev );
	void HandleEvent( SGRX_EventID eid, const EventData& edata );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
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


