

#pragma once
#include <engine.hpp>
#include <enganim.hpp>
#include <engext.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>
#include "pathfinding.hpp"
#include "../common/scritem.hpp"



typedef uint32_t TimeVal;
typedef uint32_t EntityID;

#define ENTID_NONE ((EntityID)0)



//
// GLOBAL INTERFACE
//
extern struct GameLevel* g_GameLevel;
extern PhyWorldHandle g_PhyWorld;
extern SoundSystemHandle g_SoundSys;
extern SGRX_LineSet g_DebugLines;

extern Command MOVE_LEFT;
extern Command MOVE_RIGHT;
extern Command MOVE_UP;
extern Command MOVE_DOWN;
extern Command MOVE_X;
extern Command MOVE_Y;
extern Command AIM_X;
extern Command AIM_Y;
extern Command SHOOT;
extern Command RELOAD;
extern Command SLOW_WALK;
extern Command SPRINT;
extern Command CROUCH;
extern Command SHOW_OBJECTIVES;

void Game_SetPaused( bool paused );
bool Game_IsPaused();



typedef Array< MeshInstHandle > MeshInstArray;
typedef Array< PhyRigidBodyHandle > PhyBodyArray;


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

enum EMapItemType
{
	MI_None = 0,
	MI_Mask_State = 0x00ff,
	MI_Object_Enemy = 0x0100,
	MI_Object_Camera = 0x0200,
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


struct Entity
{
	const char* m_typeName;
	String m_name;
	String m_viewName;
	
	Entity();
	virtual ~Entity();
	virtual void FixedTick( float deltaTime ){}
	virtual void Tick( float deltaTime, float blendFactor ){}
	virtual void OnEvent( const StringView& type ){}
	virtual void SetProperty( const StringView& name, sgsVariable value ){}
	virtual sgsVariable GetProperty( const StringView& name ){ return sgsVariable(); }
	virtual bool GetInteractionInfo( Vec3 pos, InteractInfo* out ){ return false; }
	virtual bool CanInterruptAction( float progress ){ return false; }
	virtual bool GetMapItemInfo( MapItemInfo* out ){ return false; }
	virtual void DebugDrawWorld(){}
	virtual void DebugDrawUI(){}
};

typedef Array< Entity* > EntityArray;



#define IEST_InteractiveItem 0x0001
#define IEST_HeatSource      0x0002
#define IEST_Player          0x0004
#define IEST_MapItem         0x0008

struct InfoEmissionSystem
{
	struct Data
	{
		Vec3 pos;
		float radius;
		uint32_t types;
	};
	struct IESProcessor
	{
		virtual bool Process( Entity*, const Data& ) = 0;
	};
	
	void Clear();
	void UpdateEmitter( Entity* e, const Data& data );
	void RemoveEmitter( Entity* e );
	bool QuerySphereAny( const Vec3& pos, float rad, uint32_t types );
	bool QuerySphereAll( IESProcessor* proc, const Vec3& pos, float rad, uint32_t types );
	bool QueryBB( const Mat4& mtx, uint32_t types );
	Entity* QueryOneRay( const Vec3& from, const Vec3& to, uint32_t types );
	
	HashTable< Entity*, Data > m_emissionData;
};

struct IESItemGather : InfoEmissionSystem::IESProcessor
{
	struct Item
	{
		Entity* E;
		InfoEmissionSystem::Data D;
		float sortkey;
	};
	
	bool Process( Entity* E, const InfoEmissionSystem::Data& D )
	{
		Item item = { E, D, 0 };
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
			items[ i ].sortkey = ( items[ i ].D.pos - pos ).LengthSq();
		Sort();
	}
	
	Array< Item > items;
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

struct MessagingSystem
{
	Array< MSMessage > m_messages;
	
	MessagingSystem();
	void AddMessage( MSMessage::Type type, const StringView& sv, float tmlength = 3 );
	void Tick( float dt );
	void DrawUI();
	
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

struct ObjectiveSystem
{
	Array< OSObjective > m_objectives;
	
	float m_alpha;
	
	ObjectiveSystem();
	int AddObjective(
		const StringView& title,
		OSObjective::State state,
		const StringView& desc = "",
		bool required = false,
		Vec3* location = NULL );
	OSObjStats GetStats();
	void Tick( float dt );
	void DrawUI();
	
	TextureHandle m_tx_icon_open;
	TextureHandle m_tx_icon_done;
	TextureHandle m_tx_icon_failed;
};

struct FSFlare
{
	Vec3 pos;
	Vec3 color;
	float size;
	bool enabled;
};

struct FlareSystem
{
	FlareSystem();
	void Clear();
	void UpdateFlare( void* handle, const FSFlare& flare );
	bool RemoveFlare( void* handle );
	void Draw( SGRX_Camera& cam );
	
	HashTable< void*, FSFlare > m_flares;
	PixelShaderHandle m_ps_flare;
	TextureHandle m_tex_flare;
};


enum GameActorType // = SGRX_MeshInstUserData::ownerType
{
	GAT_None = 0,
	GAT_Player = 1,
	GAT_Enemy = 2,
};


struct DamageSystem : SGRX_ScenePSRaycast
{
	struct Material : SGRX_RefCounted
	{
		String match;
		Array< int > decalIDs;
		ParticleSystem particles;
		String sound;
		
		bool CheckMatch( const StringView& test ) const { return match.size() == 0 || test.contains( match ); }
	};
	typedef Handle< Material > MtlHandle;
	
	const char* Init( SceneHandle scene, SGRX_LightSampler* sampler );
	void Free();
	void Tick( float deltaTime );
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
	MeshInstHandle m_bulletDecalMesh;
	SGRX_DecalSystem m_bloodDecalSys;
	MeshInstHandle m_bloodDecalMesh;
};


struct BulletSystem
{
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
	
	BulletSystem( DamageSystem* dmgsys );
	
	void Tick( SGRX_Scene* scene, float deltaTime );
	
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

struct AIDBSystem
{
	int GetNumSounds(){ return m_sounds.size(); }
	bool CanHearSound( Vec3 pos, int i );
	AISound GetSoundInfo( int i ){ return m_sounds[ i ]; }
	
	void Load( ByteArray& data );
	void AddSound( Vec3 pos, float rad, float timeout, AISoundType type );
	void Tick( float deltaTime );
	
	SGRX_Pathfinder m_pathfinder;
	Array< AISound > m_sounds;
};


