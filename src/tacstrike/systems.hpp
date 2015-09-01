

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
// last id = 10


//
// GLOBAL INTERFACE
//

extern SoundSystemHandle g_SoundSys;
extern SGRX_LineSet g_DebugLines;


template< class T > T* AddSystemToLevel( GameLevel* lev )
{
	T* sys = new T( lev );
	lev->AddSystem( sys );
	return sys;
}


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



#define IEST_InteractiveItem 0x0001
#define IEST_HeatSource      0x0002
#define IEST_Player          0x0004
#define IEST_MapItem         0x0008
#define IEST_AIAlert         0x0020

struct InfoEmissionSystem : IGameLevelSystem
{
	enum { e_system_uid = 1 };
	
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
	
	InfoEmissionSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid ){}
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
	bool LoadChunk( const StringView& type, uint8_t* ptr, size_t size );
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
	SGS_OBJECT;
	
	enum { e_system_uid = 2 };
	
	Array< MSMessage > m_messages;
	
	MessagingSystem( GameLevel* lev );
	~MessagingSystem();
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
	SGS_OBJECT;
	
	enum { e_system_uid = 3 };
	
	ObjectiveSystem( GameLevel* lev );
	~ObjectiveSystem();
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
	
	Command SHOW_OBJECTIVES;
	
	Array< OSObjective > m_objectives;
	
	float m_alpha;
	
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

struct FlareSystem : IGameLevelSystem
{
	enum { e_system_uid = 4 };
	
	FlareSystem( GameLevel* lev );
	void Clear();
	void UpdateFlare( void* handle, const FSFlare& flare );
	bool RemoveFlare( void* handle );
	void PostDraw();
	
	HashTable< void*, FSFlare > m_flares;
	PixelShaderHandle m_ps_flare;
	TextureHandle m_tex_flare;
};


struct LevelCoreSystem : IGameLevelSystem
{
	enum { e_system_uid = 10 };
	
	LevelCoreSystem( GameLevel* lev );
	virtual void Clear();
	virtual bool AddEntity( const StringView& type, sgsVariable data );
	virtual bool LoadChunk( const StringView& type, uint8_t* ptr, size_t size );
	
	Array< MeshInstHandle > m_meshInsts;
	Array< PhyRigidBodyHandle > m_levelBodies;
	Array< LC_Light > m_lights;
	SGRX_LightTree m_ltSamples;
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
	MeshInstHandle m_bulletDecalMesh;
	SGRX_DecalSystem m_bloodDecalSys;
	MeshInstHandle m_bloodDecalMesh;
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

struct AIDBSystem : IGameLevelSystem
{
	enum { e_system_uid = 7 };
	
	int GetNumSounds(){ return m_sounds.size(); }
	bool CanHearSound( Vec3 pos, int i );
	AISound GetSoundInfo( int i ){ return m_sounds[ i ]; }
	
	AIDBSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid ){}
	bool LoadChunk( const StringView& type, uint8_t* ptr, size_t size );
	void AddSound( Vec3 pos, float rad, float timeout, AISoundType type );
	void Tick( float deltaTime, float blendFactor );
	
	SGRX_Pathfinder m_pathfinder;
	Array< AISound > m_sounds;
};


struct CSCoverInfo
{
	Array< Vec4 > shadowPlanes;
	Array< int > shadowPlaneCounts;
};

struct CoverSystem : IGameLevelSystem
{
	enum { e_system_uid = 8 };
	
	struct Edge
	{
#if 1
		int pl0;
		int pl1;
#else
		Vec3 p0; // endpoint 0
		Vec3 p1; // endpoint 1
		Vec3 n0; // adjacent plane 0 normal
		Vec3 n1; // adjacent plane 1 normal
#endif
	};
	struct EdgeMesh : SGRX_RCRsrc
	{
		Array< Edge > edges;
		Array< Vec4 > planes;
		Vec3 pos;
		bool enabled;
	};
	typedef Handle< EdgeMesh > EdgeMeshHandle;
	
	CoverSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid ){}
	void Clear();
	void AddAABB( StringView name, Vec3 bbmin, Vec3 bbmax, Mat4 mtx );
	void Query( Vec3 viewer, float viewdist, CSCoverInfo& shape );
	
	Array< EdgeMeshHandle > m_edgeMeshes;
	HashTable< StringView, EdgeMeshHandle > m_edgeMeshesByName;
};


