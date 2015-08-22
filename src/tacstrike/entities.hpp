

#pragma once
#define USE_HASHTABLE
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


struct ScriptedItem : Entity
{
	SGRX_ScriptedItem* m_scrItem;
	
	ScriptedItem( const StringView& name, sgsVariable args );
	~ScriptedItem();
	virtual void FixedTick( float deltaTime );
	virtual void Tick( float deltaTime, float blendFactor );
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



extern Command DO_ACTION;


#if defined(LD33GAME)
#define TSGAME_NO_PLAYER
#include "entities_ts.hpp"
#include "entities_ld33.hpp"
#define Player LD33Player
#elif defined(BRSD4GAME)
#include "entities_brsd4.hpp"
#define Player BRSD4Player
#elif defined(TSGAME)
#include "entities_ts.hpp"
#define Player TSPlayer
#endif


