

#include <engine.hpp>
#include <enganim.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#include "levelcache.hpp"


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
extern Command SHOW_OBJECTIVES;


void Game_SetPaused( bool paused );
bool Game_IsPaused();


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

struct Trigger : Entity
{
	String m_func;
	String m_target;
	bool m_once;
	bool m_done;
	bool m_lastState;
	
	bool m_currState;
	
	Trigger( const StringView& fn, const StringView& tgt, bool once, bool laststate = false );
	void Invoke( bool newstate );
	void Update( bool newstate );
};

struct BoxTrigger : Trigger
{
	Mat4 m_matrix;
	
	BoxTrigger( const StringView& fn, const StringView& tgt, bool once, const Vec3& pos, const Quat& rot, const Vec3& scl );
	virtual void FixedTick( float deltaTime );
};

struct ProximityTrigger : Trigger
{
	Vec3 m_position;
	float m_radius;
	
	ProximityTrigger( const StringView& fn, const StringView& tgt, bool once, const Vec3& pos, float rad );
	virtual void FixedTick( float deltaTime );
};

struct SlidingDoor : Trigger
{
	float open_factor; // 0 .. 1
	float open_target; // 0 .. 1
	float open_time; // > 0
	
	Vec3 pos_open;
	Vec3 pos_closed;
	Quat rot_open;
	Quat rot_closed;
	bool target_state;
	
	bool m_isSwitch;
	String m_switchPred;
	
	Vec3 position;
	Quat rotation;
	Vec3 scale;
	
	Vec3 m_bbMin;
	Vec3 m_bbMax;
	MeshInstHandle meshInst;
	PhyRigidBodyHandle bodyHandle;
	SoundEventInstanceHandle soundEvent;
	
	IVState< Vec3 > m_ivPos;
	IVState< Quat > m_ivRot;
	
	void _UpdatePhysics();
	void _UpdateTransforms( float bf );
	SlidingDoor(
		const StringView& name,
		const StringView& mesh,
		const Vec3& pos,
		const Quat& rot,
		const Vec3& scl,
		const Vec3& oopen,
		const Quat& ropen,
		const Vec3& oclos,
		const Quat& rclos,
		float otime,
		bool istate,
		bool isswitch = false,
		const StringView& pred = StringView(),
		const StringView& fn = StringView(),
		const StringView& tgt = StringView(),
		bool once = false
	);
	virtual void FixedTick( float deltaTime );
	virtual void Tick( float deltaTime, float blendFactor );
	virtual void OnEvent( const StringView& type );
};

struct PickupItem : Entity
{
	MeshInstHandle m_meshInst;
	int m_count;
	
	PickupItem( const StringView& id, const StringView& name, int count, const StringView& mesh, const Vec3& pos, const Quat& rot, const Vec3& scl );
	virtual void OnEvent( const StringView& type );
};


struct ParticleFX : Entity
{
	ParticleSystem m_psys;
	String m_soundEventName;
	bool m_soundEventOneShot;
	SoundEventInstanceHandle m_soundEventInst;
	Vec3 m_position;
	
	ParticleFX( const StringView& name, const StringView& psys, const StringView& sndev, const Vec3& pos, const Quat& rot, const Vec3& scl, bool start );
	virtual void Tick( float deltaTime, float blendFactor );
	virtual void OnEvent( const StringView& type );
};


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


// max_clip - clip capacity
// max_bag - ammo bag capacity
// bullet_speed - units per second
// bullet_time - bullet hang time
// damage, damage_rand - total damage on hit = ( damage + frandrange(-1,1) * damage_rand ) * bspeed
// fire_rate - timeout between bullets
// reload_time - time it takes to reload
// spread - bullet spread in degrees
// backfire - spread addition factor for firing time and movement speed
// -- full spread = spread + f(time,speed) * backfire
struct WeaponType
{
	StringView name;
	int max_clip_ammo;
	int max_bag_ammo;
	float bullet_speed;
	float bullet_time;
	float damage;
	float damage_rand;
	float fire_rate;
	float reload_time;
	float spread;
	float backfire;
	StringView sound_shoot;
	StringView sound_reload;
};

#define WEAPON_INACTIVE 0
#define WEAPON_EMPTY -1
#define WEAPON_SHOT 1

struct Weapon
{
	Weapon( BulletSystem* bsys, WeaponType* wt, int ammo );
	
	void SetShooting( bool shoot );
	float GetSpread();
	bool BeginReload();
	void StopReload();
	int Tick( float deltaTime, Vec2 pos, Vec2 dir, float speed );
	
	BulletSystem* m_bulletSystem;
	WeaponType* m_type;
	// TODO VARIABLE FOR WHO USES THIS WEAPON
	float m_fire_timeout;
	float m_reload_timeout;
	int m_ammo_clip;
	int m_ammo_bag;
	bool m_shooting;
	float m_shoot_factor;
	// TODO VARIABLE FOR SHOOT SOUND
	// TODO VARIABLE FOR RELOAD SOUND
	Vec2 m_position;
	Vec2 m_direction;
	float m_owner_speed;
};


#define ENTGROUP_GOODGUYS 1
#define ENTGROUP_BADGUYS 2
#define ENTGROUP_GOODGUYS_DEAD 3
#define ENTGROUP_BADGUYS_DEAD 4

struct Character
{
	bool m_is_player;
	Weapon* m_weapon;
	
	Vec2 m_position;
	Vec2 m_velocity;
	float m_angle;
	
	// TODO VARIABLE FOR PHYSICS REP
	
	Vec2 m_leg_dir;
	float m_leg_time;
	float m_leg_factor;
	
	float m_health;
	bool m_dead;
	
	float m_scale;
	
	TextureHandle m_sprite_base;
	TextureHandle m_sprite_arms;
	TextureHandle m_sprite_shadow;
	TextureHandle m_sprite_dead_char;
	
	TextureHandle m_sprite_flash;
	TextureHandle m_sprite_flash_light;
	float m_flash_strength;
	
	Character( bool isplayer, Vec2 pos, float angle, const StringView& sprite );
	void Die();
	void BulletHit( Vec2 hitvec, Vec2 hitpos, Vec2 hitnrm );
	void MoveChar( Vec2 move, float deltaTime );
	void DrawChar2D();
};


#ifdef BRSD4GAME

struct Player : Entity
{
	PhyRigidBodyHandle m_bodyHandle;
	PhyShapeHandle m_shapeHandle;
	
	Vec2 m_angles;
	float m_jumpTimeout;
	float m_canJumpTimeout;
	float m_footstepTime;
	bool m_isOnGround;
	float m_isCrouching;
	
	IVState< Vec3 > m_ivPos;
	
	Vec2 inCursorMove;
	
	Entity* m_targetII;
	bool m_targetTriggered;
	
	HashTable< String, int > m_items;
	
	TextureHandle m_tex_interact_icon;
	
	Player( const Vec3& pos, const Vec3& dir );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	bool AddItem( const StringView& item, int count );
	bool HasItem( const StringView& item, int count = 1 );
};

#elif defined(LD32GAME)

enum LD32TaskType
{
	TT_Wait,
	TT_Turn,
	TT_Walk,
};
struct LD32Task
{
	LD32TaskType type;
	float timeout;
	Vec2 target;
};
typedef Array< LD32Task > LD32TaskArray;
void LD32ParseTaskArray( LD32TaskArray& out, sgsVariable var );

struct LD32Char : Entity
{
	LD32Char( const Vec3& pos, const Vec3& dir, const Vec4& color );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	
	PhyRigidBodyHandle m_bodyHandle;
	PhyShapeHandle m_shapeHandle;
	MeshInstHandle m_meshInst;
	LightHandle m_shadowInst;
	AnimPlayer m_anMainPlayer;
	AnimInterp m_anEnd;
	
	float m_footstepTime;
	bool m_isCrouching;
	IVState< Vec3 > m_ivPos;
	IVState< Quat > m_ivDir;
	
	Vec2 m_position;
	Vec2 m_moveDir;
	float m_turnAngle;
};

struct Player : LD32Char
{
	Vec2 m_angles;
	Vec2 inCursorMove;
	
	Entity* m_targetII;
	bool m_targetTriggered;
	
	HashTable< String, int > m_items;
	
	TextureHandle m_tex_interact_icon;
	
	Player( const Vec3& pos, const Vec3& dir );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	bool AddItem( const StringView& item, int count );
	bool HasItem( const StringView& item, int count = 1 );
};

struct Enemy : LD32Char
{
	LD32TaskArray m_patrolTasks;
	LD32TaskArray m_disturbTasks;
	float m_taskTimeout;
	int m_curTaskID;
	bool m_curTaskMode;
	String m_disturbActionName;
	
	float m_turnAngleStart;
	float m_turnAngleEnd;
	
	Enemy( const StringView& name, const Vec3& pos, const Vec3& dir );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void UpdateTask();
};

#else

struct Player : Character
{
	TextureHandle m_tex_crosshair;
	float m_sprint_time;
	float m_max_sprint_time;
	bool m_sprint_cooldown;
	float m_dead_time;
	
	HashTable< String, int > m_items;
	
	Player( const Vec3& pos );
	void FixedTick( float deltaTime ){}
	void Tick( float deltaTime, float blendFactor );
	void Draw2D();
	void DrawUI();
	
	bool AddItem( const StringView& item, int count );
	bool HasItem( const StringView& item, int count = 1 );
};

#endif


typedef StackString<16> StackShortName;

typedef Array< LC_Light > LightArray;

typedef Array< Entity* > EntityArray;
typedef Array< MeshInstHandle > MeshInstArray;
typedef Array< PhyRigidBodyHandle > PhyBodyArray;


struct GameLevel : SGRX_PostDraw
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
	
	// LEVEL DATA
	bool m_paused;
	float m_endFactor;
	double m_levelTime;
	EntityArray m_entities;
	MeshInstArray m_meshInsts;
	PhyBodyArray m_levelBodies;
	LightArray m_lights;
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

