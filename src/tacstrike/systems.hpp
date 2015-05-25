

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
};

typedef Array< Entity* > EntityArray;



#define IEST_InteractiveItem 0x0001
#define IEST_HeatSource      0x0002
#define IEST_Player          0x0004

struct InfoEmissionSystem
{
	struct Data
	{
		Vec3 pos;
		float radius;
		uint32_t types;
	};
	
	HashTable< Entity*, Data > m_emissionData;
	
	void UpdateEmitter( Entity* e, const Data& data );
	void RemoveEmitter( Entity* e );
	bool QueryAny( const Vec3& pos, float rad, uint32_t types );
	bool QueryBB( const Mat4& mtx, uint32_t types );
	Entity* QueryOneRay( const Vec3& from, const Vec3& to, uint32_t types );
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


struct DamageSystem
{
	struct Material : SGRX_RefCounted
	{
		String match;
		Array< int > decalIDs;
		ParticleSystem particles;
		String sound;
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
		// penetration depth calculations
		Vec3 intersectStart;
		int numSolidRefs;
	};
	typedef Array< Bullet > BulletArray;
	
	BulletSystem( DamageSystem* dmgsys );
	
	void Tick( SGRX_Scene* scene, float deltaTime );
	
	void Add( const Vec3& pos, const Vec3& vel, float timeleft, float dmg );
	void Clear();
	
	BulletArray m_bullets;
	DamageSystem* m_damageSystem;
	
	Array< SceneRaycastInfo > m_tmpStore;
};


