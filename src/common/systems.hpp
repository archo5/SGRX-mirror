

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
};

struct SGRX_MeshInstUserData
{
	SGRX_MeshInstUserData() : dmgDecalSysOverride(NULL),
		ovrDecalSysOverride(NULL), ownerType(0), typeOverride(NULL){}
	virtual ~SGRX_MeshInstUserData(){}
	virtual void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data ){}
	SGRX_DecalSystem* dmgDecalSysOverride;
	SGRX_DecalSystem* ovrDecalSysOverride;
	uint32_t ownerType;
	const char* typeOverride;
};



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

EXP_STRUCT LevelMapSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 9 };
	
	GFW_EXPORT LevelMapSystem( GameLevel* lev );
	GFW_EXPORT void Clear();
	GFW_EXPORT void OnRemoveEntity( Entity* e );
	GFW_EXPORT bool LoadChunk( const StringView& type, ByteView data );
	GFW_EXPORT void UpdateItem( Entity* e, const MapItemInfo& data );
	GFW_EXPORT void RemoveItem( Entity* e );
	
	GFW_EXPORT SGS_METHOD void DrawUIRect( float x0, float y0, float x1, float y1, float linesize );
	
	GFW_EXPORT SGS_METHOD_NAMED( Update ) void sgsUpdate( EntityScrHandle e, int type, Vec3 pos, Vec3 dir, float szfwd, float szrt );
	GFW_EXPORT SGS_METHOD_NAMED( Remove ) void sgsRemove( EntityScrHandle e );
	
	SGS_PROPERTY Vec3 viewPos;

	HashTable< Entity*, MapItemInfo > m_mapItemData;
	Array< Vec2 > m_lines;
	Array< LC_Map_Layer > m_layers;
	
	TextureHandle m_tex_mapline;
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

EXP_STRUCT MessagingSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 2 };
	
	Array< MSMessage > m_messages;
	
	GFW_EXPORT MessagingSystem( GameLevel* lev );
	GFW_EXPORT void Clear();
	GFW_EXPORT void AddMessage( MSMessage::Type type, const StringView& sv, float tmlength = 3 );
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void DrawUI();
	
	GFW_EXPORT SGS_METHOD_NAMED( Add ) void sgsAddMsg( int type, StringView text, float time );
	
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

EXP_STRUCT ObjectiveSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 3 };
	
	GFW_EXPORT ObjectiveSystem( GameLevel* lev );
	GFW_EXPORT void Clear();
	GFW_EXPORT int AddObjective(
		const StringView& title,
		OSObjective::State state,
		const StringView& desc = "",
		bool required = false,
		Vec3* location = NULL );
	GFW_EXPORT OSObjStats GetStats();
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void DrawUI();
	
	GFW_EXPORT bool _CheckRange( int i );
	GFW_EXPORT SGS_METHOD_NAMED( Add ) int sgsAddObj( StringView title, int state, StringView desc, bool req, Vec3 loc );
	GFW_EXPORT SGS_METHOD_NAMED( GetTitle ) StringView sgsGetTitle( int i );
	GFW_EXPORT SGS_METHOD_NAMED( SetTitle ) void sgsSetTitle( int i, StringView title );
	GFW_EXPORT SGS_METHOD_NAMED( GetState ) int sgsGetState( int i );
	GFW_EXPORT SGS_METHOD_NAMED( SetState ) void sgsSetState( int i, int state );
	GFW_EXPORT SGS_METHOD_NAMED( SetLocation ) void sgsSetLocation( int i, Vec3 loc );
	
	InputState SHOW_OBJECTIVES;
	
	Array< OSObjective > m_objectives;
	
	float m_alpha;
	
	TextureHandle m_tx_icon_open;
	TextureHandle m_tx_icon_done;
	TextureHandle m_tx_icon_failed;
};


EXP_STRUCT HelpTextSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 13 };
	
	GFW_EXPORT HelpTextSystem( GameLevel* lev );
	GFW_EXPORT void Clear();
	GFW_EXPORT void SetText( StringView text, float alpha = 1, float fadetime = 0, float fadeto = 0 );
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT SGS_METHOD void DrawText();
	
	SGS_PROPERTY_FUNC( READ WRITE VARNAME text ) String m_text;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME alpha ) float m_alpha;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME fadeTime ) float m_fadeTime;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME fadeTo ) float m_fadeTo;
	GFW_EXPORT SGS_METHOD_NAMED( Clear ) void sgsClear();
	GFW_EXPORT SGS_METHOD_NAMED( SetText ) void sgsSetText( StringView text, float alpha, float fadetime, float fadeto );
	
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

EXP_STRUCT FlareSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 4 };
	
	GFW_EXPORT FlareSystem( GameLevel* lev );
	GFW_EXPORT void Clear();
	GFW_EXPORT void UpdateFlare( void* handle, const FSFlare& flare );
	GFW_EXPORT bool RemoveFlare( void* handle );
	GFW_EXPORT void PostDraw();
	
	GFW_EXPORT SGS_METHOD_NAMED( Update ) void sgsUpdate( void* handle, Vec3 pos, Vec3 col, float size, bool enabled );
	GFW_EXPORT SGS_METHOD_NAMED( Remove ) void sgsRemove( void* handle );
	SGS_PROPERTY_FUNC( READ WRITE VARNAME layers ) uint32_t m_layers;
	
	HashTable< void*, FSFlare > m_flares;
	PixelShaderHandle m_ps_flare;
	TextureHandle m_tex_flare;
};


EXP_STRUCT LevelCoreSystem : IGameLevelSystem
{
	enum { e_system_uid = 10 };
	
	GFW_EXPORT LevelCoreSystem( GameLevel* lev );
	GFW_EXPORT virtual void Clear();
	GFW_EXPORT virtual bool LoadChunk( const StringView& type, ByteView data );
	
	Array< MeshInstHandle > m_meshInsts;
	Array< LightHandle > m_lightInsts;
	Array< PhyRigidBodyHandle > m_levelBodies;
	Array< LC_Light > m_lights;
	SGRX_LightTree m_ltSamples;
};


EXP_STRUCT GFXSystem : IGameLevelSystem, SGRX_RenderDirector
{
	enum { e_system_uid = 15 };
	
	GFW_EXPORT GFXSystem( GameLevel* lev );
	GFW_EXPORT void OnAddEntity( Entity* ent );
	GFW_EXPORT void OnRemoveEntity( Entity* ent );
	
	GFW_EXPORT virtual void OnDrawScene( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info );
	
	Array< Entity* > m_reflectPlanes;
};


EXP_STRUCT ScriptedSequenceSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 11 };
	
	GFW_EXPORT ScriptedSequenceSystem( GameLevel* lev );
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void DrawUI();
	void _StartCutscene(){ m_time = 0; }
	
	InputState m_cmdSkip;
	GFW_EXPORT SGS_METHOD_NAMED( Start ) void sgsStart( sgsVariable func, float t );
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _StartCutscene VARNAME func ) sgsVariable m_func;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME time ) float m_time;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME subtitle ) String m_subtitle;
};


EXP_STRUCT MusicSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 12 };
	
	GFW_EXPORT MusicSystem( GameLevel* lev );
	GFW_EXPORT ~MusicSystem();
	
	GFW_EXPORT SGS_METHOD_NAMED( SetTrack ) void sgsSetTrack( StringView path );
	GFW_EXPORT SGS_METHOD_NAMED( SetVar ) void sgsSetVar( StringView name, float val );
	
	SoundEventInstanceHandle m_music;
};


enum GameActorType // = SGRX_MeshInstUserData::ownerType
{
	GAT_None = 0,
	GAT_Player = 1,
	GAT_Enemy = 2,
};


EXP_STRUCT DamageSystem : IGameLevelSystem, SGRX_ScenePSRaycast
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
	
	GFW_EXPORT DamageSystem( GameLevel* lev );
	GFW_EXPORT ~DamageSystem();
	GFW_EXPORT const char* Init( SceneHandle scene, SGRX_LightSampler* sampler );
	GFW_EXPORT void Free();
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void AddBulletDamage( SGRX_DecalSystem* dmgDecalSysOverride,
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
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
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
	
	GFW_EXPORT BulletSystem( GameLevel* lev );
	
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	
	GFW_EXPORT SGS_METHOD void Add( Vec3 pos, Vec3 vel, float timeleft, float dmg, uint32_t ownerType );
	GFW_EXPORT SGS_METHOD bool Zap( Vec3 p1, Vec3 p2, float dmg, uint32_t ownerType );
	GFW_EXPORT void Clear();
	
	bool _ProcessBullet( Vec3 p1, Vec3 p2, Bullet& B );
	
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
	GFW_EXPORT bool HasRecentFact( uint32_t typemask, TimeVal maxtime );
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
	
	Array< AIFact > facts;
	TimeVal m_lastTime;
	uint32_t last_mod_id;
	uint32_t m_next_fact_id;
};

EXP_STRUCT AIDBSystem : IGameLevelSystem
{
	SGS_OBJECT_LITE;
	SGS_NO_DESTRUCT;
	
	enum { e_system_uid = 7 };
	
	int GetNumSounds(){ return m_sounds.size(); }
	GFW_EXPORT bool CanHearSound( Vec3 pos, int i );
	AISound GetSoundInfo( int i ){ return m_sounds[ i ]; }
	
	GFW_EXPORT AIDBSystem( GameLevel* lev );
	GFW_EXPORT bool LoadChunk( const StringView& type, ByteView data );
	GFW_EXPORT IEditorSystemCompiler* EditorGetSystemCompiler();
	GFW_EXPORT void AddSound( Vec3 pos, float rad, float timeout, AISoundType type );
	GFW_EXPORT void AddRoomPart( const StringView& name, Mat4 xf, bool negative, float cell_size );
	GFW_EXPORT AIRoom* FindRoomByPos( Vec3 pos );
	GFW_EXPORT void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void FixedTick( float deltaTime );
	GFW_EXPORT void DebugDrawWorld();
	
	SGRX_Pathfinder m_pathfinder;
	Array< AISound > m_sounds;
	HashTable< StringView, AIRoomHandle > m_rooms;
	AIFactStorage m_globalFacts;
	
	GFW_EXPORT SGS_METHOD_NAMED( AddSound ) void sgsAddSound( Vec3 pos, float rad, float timeout, int type );
	
	GFW_EXPORT SGS_METHOD_NAMED( HasFact ) bool sgsHasFact( uint32_t typemask );
	GFW_EXPORT SGS_METHOD_NAMED( HasRecentFact ) bool sgsHasRecentFact( uint32_t typemask, TimeVal maxtime );
	GFW_EXPORT SGS_METHOD_NAMED( GetRecentFact ) SGS_MULTRET sgsGetRecentFact( sgs_Context* coro, uint32_t typemask, TimeVal maxtime );
	GFW_EXPORT SGS_METHOD_NAMED( InsertFact ) void sgsInsertFact( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref );
	GFW_EXPORT SGS_METHOD_NAMED( UpdateFact ) bool sgsUpdateFact( sgs_Context* coro, uint32_t type, Vec3 pos,
		float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	GFW_EXPORT SGS_METHOD_NAMED( InsertOrUpdateFact ) void sgsInsertOrUpdateFact( sgs_Context* coro,
		uint32_t type, Vec3 pos, float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	GFW_EXPORT SGS_MULTRET sgsPushRoom( sgs_Context* coro, AIRoom* room );
	GFW_EXPORT SGS_METHOD_NAMED( GetRoomList ) SGS_MULTRET sgsGetRoomList( sgs_Context* coro );
	GFW_EXPORT SGS_METHOD_NAMED( GetRoomNameByPos ) sgsString sgsGetRoomNameByPos( sgs_Context* coro, Vec3 pos );
	GFW_EXPORT SGS_METHOD_NAMED( GetRoomByPos ) SGS_MULTRET sgsGetRoomByPos( sgs_Context* coro, Vec3 pos );
	GFW_EXPORT SGS_METHOD_NAMED( GetRoomPoints ) SGS_MULTRET sgsGetRoomPoints( sgs_Context* coro, StringView name );
};


struct CSCoverLine
{
	Vec3 p0;
	Vec3 p1;
};

EXP_STRUCT CSCoverInfo
{
	struct Shape
	{
		size_t offset;
		int numPlanes;
	};
	
	GFW_EXPORT void Clear();
	GFW_EXPORT bool GetPosition( Vec3 position, float distpow, Vec3& out, float interval = 0.1f );
	GFW_EXPORT void ClipWithSpheres( Vec4* spheres, int count );
	
	GFW_EXPORT size_t _GetBestFactorID();
	GFW_EXPORT void _CullWithShadowLines( size_t firstcover, Vec4 P );
	GFW_EXPORT void _CullWithSolids();
	
	Array< Vec4 > planes;
	Array< Shape > shapes;
	Array< CSCoverLine > covers;
	Array< float > factors;
};

EXP_STRUCT CoverSystem : IGameLevelSystem
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
	EXP_STRUCT CoverPoint
	{
		Vec3 pos;
		Vec3 nout;
		Vec3 nup;
		
		GFW_EXPORT void AdjustNormals( Vec3 newout, Vec3 newup );
		bool operator == ( const CoverPoint& o ) const { return pos == o.pos; }
	};
	EXP_STRUCT EdgeMesh : SGRX_RCRsrc
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
		
		GFW_EXPORT bool InAABB( const Vec3& ibmin, const Vec3& ibmax ) const;
		GFW_EXPORT bool PointInBox( Vec3 pt ) const;
		GFW_EXPORT void CalcCoverLines();
	};
	typedef Handle< EdgeMesh > EdgeMeshHandle;
	
	CoverSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid ){}
	GFW_EXPORT bool LoadChunk( const StringView& type, ByteView data );
	GFW_EXPORT IEditorSystemCompiler* EditorGetSystemCompiler();
	GFW_EXPORT void Clear();
	GFW_EXPORT void AddAABB( StringView name, Vec3 bbmin, Vec3 bbmax, Mat4 mtx );
	
	GFW_EXPORT void QueryLines( Vec3 bbmin, Vec3 bbmax, float dist,
		float height, Vec3 viewer, bool visible, CSCoverInfo& cinfo );
	
	Array< EdgeMeshHandle > m_edgeMeshes;
	HashTable< StringView, EdgeMeshHandle > m_edgeMeshesByName;
};


EXP_STRUCT DevelopSystem : IGameLevelSystem, SGRX_IEventHandler
{
	enum { e_system_uid = 14 };
	
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


