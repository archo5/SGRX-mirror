

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
	void Tick( float deltaTime ){}
	void Draw2D(){}
	
	void Clear(){}
	
	int todo;
};


struct BulletSystem
{
	struct Bullet
	{
		Vec2 position;
		Vec2 velocity;
		Vec2 dir;
		float timeleft;
		float damage;
	};
	typedef Array< Bullet > BulletArray;
	
	BulletSystem( DamageSystem* dmgsys );
	
	void Tick( float deltaTime );
	void Draw2D();
	
	void Add( const Vec2& pos, const Vec2& vel, float timeleft, float dmg );
	void Clear();
	
	BulletArray m_bullets;
	TextureHandle m_tx_bullet;
	DamageSystem* m_damageSystem;
};


