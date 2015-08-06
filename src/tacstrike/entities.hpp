

#pragma once

#include <engine.hpp>
#include <enganim.hpp>
#include <pathfinding.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>
#include "systems.hpp"


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
	Vec3 m_pos;
	
	PickupItem( const StringView& id, const StringView& name, int count, const StringView& mesh, const Vec3& pos, const Quat& rot, const Vec3& scl );
	virtual void OnEvent( const StringView& type );
	virtual bool GetInteractionInfo( Vec3 pos, InteractInfo* out );
};

struct Actionable : Entity
{
	MeshInstHandle m_meshInst;
	InteractInfo m_info;
	sgsVariable m_onSuccess;
	
	Actionable( const StringView& name, const StringView& mesh, const Vec3& pos, const Quat& rot, const Vec3& scl, const Vec3& placeoff, const Vec3& placedir );
	virtual void OnEvent( const StringView& type );
	virtual bool GetInteractionInfo( Vec3 pos, InteractInfo* out );
	virtual void SetProperty( const StringView& name, sgsVariable value );
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

#if 0
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
#endif


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

#if 0
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

#endif



#ifdef TSGAME

struct YawPitch
{
	float yaw, pitch;
	
	void Normalize()
	{
		yaw = normalize_angle( yaw );
		pitch = normalize_angle( pitch );
	}
	YawPitch Normalized() const
	{
		YawPitch out = { normalize_angle( yaw ), normalize_angle( pitch ) };
		return out;
	};
	YawPitch Abs() const
	{
		YawPitch out = { fabsf( yaw ), fabsf( pitch ) };
		return out;
	}
	YawPitch Scaled( float f ) const
	{
		YawPitch out = { yaw * f, pitch * f };
		return out;
	}
	
	void TurnTo( const YawPitch& tgt, const YawPitch& speedDelta )
	{
		YawPitch ypend = tgt.Normalized();
		YawPitch ypstart = Normalized();
		if( fabs( ypend.yaw - ypstart.yaw ) > M_PI )
			ypstart.yaw += ypend.yaw > ypstart.yaw ? M_PI * 2 : -M_PI * 2;
		if( fabs( ypend.pitch - ypstart.pitch ) > M_PI )
			ypstart.pitch += ypend.pitch > ypstart.pitch ? M_PI * 2 : -M_PI * 2;
		yaw = ypstart.yaw + sign( ypend.yaw - ypstart.yaw ) * TMIN( fabsf( ypend.yaw - ypstart.yaw ), speedDelta.yaw );
		pitch = ypstart.pitch + sign( ypend.pitch - ypstart.pitch ) * TMIN( fabsf( ypend.pitch - ypstart.pitch ), speedDelta.pitch );
	}
};
FINLINE YawPitch YP( float y, float p ){ YawPitch out = { y, p }; return out; }
FINLINE YawPitch YP( float f ){ return YP( f, f ); }
inline YawPitch YP( const Vec3& v )
{
	return YP( normalize_angle( atan2( v.y, v.x ) ),
		normalize_angle( atan2( v.z, v.ToVec2().Length() ) ) );
}
inline YawPitch YawPitchDist( YawPitch from, YawPitch to )
{
	from = from.Normalized();
	to = to.Normalized();
	if( fabs( to.yaw - from.yaw ) > M_PI )
		from.yaw += to.yaw > from.yaw ? M_PI * 2 : -M_PI * 2;
	if( fabs( to.pitch - from.pitch ) > M_PI )
		from.pitch += to.pitch > from.pitch ? M_PI * 2 : -M_PI * 2;
	return YP( to.yaw - from.yaw, to.pitch - from.pitch );
}
inline bool YawPitchAlmostEqual( const YawPitch& a, const YawPitch& b )
{
	YawPitch dst = YawPitchDist( a, b ).Abs();
	return dst.yaw < SMALL_FLOAT && dst.pitch < SMALL_FLOAT;
}

struct TSCamera : Entity
{
	TSCamera(
		const StringView& name,
		const StringView& charname,
		const Vec3& pos,
		const Quat& rot,
		const Vec3& scl,
		const Vec3& dir0,
		const Vec3& dir1
	);
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void SetProperty( const StringView& name, sgsVariable value );
	bool GetMapItemInfo( MapItemInfo* out );
	
	AnimCharacter m_animChar;
	AnimMixer::Layer m_anLayers[1];
	int m_viewAttachmentID;
	
	bool m_playerVisible;
	YawPitch m_lastSeenPlayerDir;
	YawPitch m_curDir;
	float m_timeout;
	int m_state;
	float m_alertTimeout;
	float m_noticeTimeout;
	
	YawPitch m_dir0;
	YawPitch m_dir1;
	float m_moveTime;
	float m_pauseTime;
	float m_fov;
};

extern Command DO_ACTION;

#define Player TSPlayer

enum TSTaskType
{
	TT_Wait,
	TT_Turn,
	TT_Walk,
};
struct TSTask
{
	TSTaskType type;
	float timeout;
	Vec2 target;
};
typedef Array< TSTask > TSTaskArray;
void TSParseTaskArray( TSTaskArray& out, sgsVariable var );

struct TSCharacter : Entity
{
	struct ActionState
	{
		ActionState() : timeoutMoveToStart(0), timeoutEnding(0),
			progress(0), target(NULL){}
		
		float timeoutMoveToStart;
		float timeoutEnding;
		float progress;
		Entity* target;
		InteractInfo info;
	};
	
	TSCharacter( const Vec3& pos, const Vec3& dir );
	void InitializeMesh( const StringView& path );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void HandleMovementPhysics( float deltaTime );
	void TurnTo( const Vec2& turnDir, float speedDelta );
	void PushTo( const Vec3& pos, float speedDelta );
	void BeginClosestAction( float maxdist );
	bool BeginAction( Entity* E );
	bool IsInAction();
	bool CanInterruptAction();
	void InterruptAction( bool force );
	
	Vec3 GetQueryPosition();
	Vec3 GetPosition();
	Vec3 GetViewDir();
	Vec3 GetAimDir();
	Mat4 GetBulletOutputMatrix();
	
	Vec3 GetInterpPos();
	Vec3 GetInterpAimDir();
	
	PhyRigidBodyHandle m_bodyHandle;
	PhyShapeHandle m_shapeHandle;
	LightHandle m_shadowInst;
	
	AnimCharacter m_animChar;
	MeshInstInfo m_meshInstInfo;
	AnimPlayer m_anMainPlayer;
	AnimPlayer m_anTopPlayer;
	AnimMixer::Layer m_anLayers[4];
	
	float m_footstepTime;
	bool m_isCrouching;
	bool m_isOnGround;
	IVState< Vec3 > m_ivPos;
	IVState< Vec3 > m_ivAimDir;
	Vec3 m_interpPos;
	Vec3 m_interpAimDir;
	
	Vec3 m_position;
	Vec2 m_moveDir;
	float m_turnAngle;
	
	ActionState m_actState;
	
	bool i_crouch;
	Vec2 i_move;
	bool i_aim_at;
	Vec3 i_aim_target;
};

struct TSPlayer : TSCharacter
{
	Vec2 m_angles;
	Vec2 inCursorMove;
	
	Entity* m_targetII;
	bool m_targetTriggered;
	
	HashTable< String, int > m_items;
	
	TextureHandle m_tex_interact_icon;
	TextureHandle m_tex_cursor;
	ParticleSystem m_shootPS;
	LightHandle m_shootLT;
	float m_shootTimeout;
	float m_crouchIconShowTimeout;
	float m_standIconShowTimeout;
	
	TSPlayer( const Vec3& pos, const Vec3& dir );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	Vec3 FindTargetPosition();
	
	bool AddItem( const StringView& item, int count );
	bool HasItem( const StringView& item, int count = 1 );
};

struct TSFactStorage
{
	enum FactType
	{
		FT_Unknown = 0,
		FT_Sound_Noise,
		FT_Sound_Footstep,
		FT_Sight_ObjectState,
		FT_Sight_Friend,
		FT_Sight_Foe,
		FT_Position_Friend,
		FT_Position_Foe,
	};
	
	struct Fact
	{
		uint32_t id;
		uint32_t ref;
		FactType type;
		Vec3 position;
		TimeVal created;
		TimeVal expires;
	};
	
	TSFactStorage();
	void SortCreatedDesc();
	void Process( TimeVal curTime );
	void Insert( FactType type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref = 0 );
	bool Update( FactType type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0 );
	void InsertOrUpdate( FactType type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0 );
	bool MovingUpdate( FactType type, Vec3 pos, float movespeed,
		TimeVal created, TimeVal expires, uint32_t ref = 0 );
	void MovingInsertOrUpdate( FactType type, Vec3 pos, float movespeed,
		TimeVal created, TimeVal expires, uint32_t ref = 0 );
	
	Array< Fact > facts;
	uint32_t last_mod_id;
	uint32_t m_next_fact_id;
};

struct TSEnemy : TSCharacter
{
	TSTaskArray m_patrolTasks;
	TSTaskArray m_disturbTasks;
	float m_taskTimeout;
	int m_curTaskID;
	bool m_curTaskMode;
	String m_disturbActionName;
	
	float m_turnAngleStart;
	float m_turnAngleEnd;
	
	sgsVariable m_enemyState;
	TSFactStorage m_factStorage;
	
	TSEnemy( const StringView& name, const Vec3& pos, const Vec3& dir );
	~TSEnemy();
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void UpdateTask();
	bool GetMapItemInfo( MapItemInfo* out );
	void DebugDrawWorld();
	void DebugDrawUI();
};

#endif


