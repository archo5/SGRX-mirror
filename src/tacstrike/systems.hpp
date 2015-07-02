

#include <engine.hpp>
#include <enganim.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>



//
// GLOBAL INTERFACE
//
extern struct GameLevel* g_GameLevel;
extern PhyWorldHandle g_PhyWorld;
extern SoundSystemHandle g_SoundSys;

extern Command MOVE_LEFT;
extern Command MOVE_RIGHT;
extern Command MOVE_UP;
extern Command MOVE_DOWN;
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
	virtual bool GetInteractionInfo( Vec3 pos, InteractInfo* out ){ return false; }
	virtual bool CanInterruptAction( float progress ){ return false; }
};

typedef Array< Entity* > EntityArray;



#define IEST_InteractiveItem 0x0001
#define IEST_HeatSource      0x0002
#define IEST_Player          0x0004
#define IEST_Enemy           0x0008

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
	
	HashTable< Entity*, Data > m_emissionData;
	
	void UpdateEmitter( Entity* e, const Data& data );
	void RemoveEmitter( Entity* e );
	bool QuerySphereAny( const Vec3& pos, float rad, uint32_t types );
	bool QuerySphereAll( IESProcessor* proc, const Vec3& pos, float rad, uint32_t types );
	bool QueryBB( const Mat4& mtx, uint32_t types );
	Entity* QueryOneRay( const Vec3& from, const Vec3& to, uint32_t types );
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
	
	String text;
	State state;
};

struct ObjectiveSystem
{
	Array< OSObjective > m_objectives;
	
	float m_alpha;
	
	ObjectiveSystem();
	int AddObjective( const StringView& sv, OSObjective::State state );
	void Tick( float dt );
	void DrawUI();
	
	TextureHandle m_tx_icon_open;
	TextureHandle m_tx_icon_done;
	TextureHandle m_tx_icon_failed;
};


enum GameActorType
{
	GAT_None = 0,
	GAT_Player = 1,
	GAT_Enemy = 2,
};

struct MeshInstInfo
{
	MeshInstInfo() : typeOverride( NULL ), ownerType( GAT_None ){}
	
	const char* typeOverride;
	GameActorType ownerType;
};


struct DamageSystem
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
	
	const char* Init( SceneHandle scene );
	void Free();
	void Tick( float deltaTime );
	void AddBulletDamage( const StringView& type, SGRX_IMesh* m_targetMesh, int partID,
		const Mat4& worldMatrix, const Vec3& pos, const Vec3& dir, const Vec3& nrm, float scale = 1.0f );
	void Clear();
	
	Array< MtlHandle > m_bulletDecalMaterials;
	Array< DecalMapPartInfo > m_bulletDecalInfo;
	DecalSystem m_bulletDecalSys;
	MeshInstHandle m_bulletDecalMesh;
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
		GameActorType ownerType;
		// penetration depth calculations
		Vec3 intersectStart;
		int numSolidRefs;
	};
	typedef Array< Bullet > BulletArray;
	
	BulletSystem( DamageSystem* dmgsys );
	
	void Tick( SGRX_Scene* scene, float deltaTime );
	
	void Add( const Vec3& pos, const Vec3& vel, float timeleft, float dmg, GameActorType ownerType );
	void Clear();
	
	BulletArray m_bullets;
	DamageSystem* m_damageSystem;
	
	Array< SceneRaycastInfo > m_tmpStore;
};


struct AIDBSystem
{
	void Load( ByteArray& data );
	
	SGRX_Pathfinder m_pathfinder;
};


