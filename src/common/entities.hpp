

#pragma once
#include <engine.hpp>
#include <enganim.hpp>
#include <pathfinding.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#include "level.hpp"
#include "systems.hpp"


struct SGRX_RigidBodyInfo : SGRX_PhyRigidBodyInfo
{
	typedef sgsHandle< SGRX_RigidBodyInfo > Handle;
	
	SGS_OBJECT;
	
	SGS_PROPERTY SGS_ALIAS( Vec3 position );
	SGS_PROPERTY SGS_ALIAS( Quat rotation );
	SGS_PROPERTY SGS_ALIAS( float friction );
	SGS_PROPERTY SGS_ALIAS( float restitution );
	SGS_PROPERTY SGS_ALIAS( float mass );
	SGS_PROPERTY SGS_ALIAS( Vec3 inertia );
	SGS_PROPERTY SGS_ALIAS( float linearDamping );
	SGS_PROPERTY SGS_ALIAS( float angularDamping );
	SGS_PROPERTY SGS_ALIAS( Vec3 linearFactor );
	SGS_PROPERTY SGS_ALIAS( Vec3 angularFactor );
	SGS_PROPERTY SGS_ALIAS( bool kinematic );
	SGS_PROPERTY SGS_ALIAS( bool canSleep );
	SGS_PROPERTY SGS_ALIAS( bool enabled );
	SGS_PROPERTY SGS_ALIAS( uint16_t group );
	SGS_PROPERTY SGS_ALIAS( uint16_t mask );
};

struct SGRX_HingeJointInfo : SGRX_PhyHingeJointInfo
{
	typedef sgsHandle< SGRX_HingeJointInfo > Handle;
	
	SGS_OBJECT;
	
	SGS_PROPERTY SGS_ALIAS( Vec3 pivotA );
	SGS_PROPERTY SGS_ALIAS( Vec3 pivotB );
	SGS_PROPERTY SGS_ALIAS( Vec3 axisA );
	SGS_PROPERTY SGS_ALIAS( Vec3 axisB );
};

struct SGRX_ConeTwistJointInfo : SGRX_PhyConeTwistJointInfo
{
	typedef sgsHandle< SGRX_ConeTwistJointInfo > Handle;
	
	SGS_OBJECT;
	
	SGS_PROPERTY SGS_ALIAS( Mat4 frameA );
	SGS_PROPERTY SGS_ALIAS( Mat4 frameB );
};


EXP_STRUCT MultiEntity : SGRX_MeshInstUserData
{
	GFW_EXPORT void PreRender();
	
	GFW_EXPORT virtual void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	
	// - decal system
	GFW_EXPORT SGS_METHOD void DSCreate( StringView texDmgDecalPath,
		StringView texOvrDecalPath, StringView texFalloffPath, uint32_t size );
	GFW_EXPORT SGS_METHOD void DSDestroy();
	GFW_EXPORT SGS_METHOD void DSResize( uint32_t size );
	GFW_EXPORT SGS_METHOD void DSClear();
	
	// - rigid bodies
	GFW_EXPORT SGS_METHOD void RBCreateFromConvexPointSet( int i, StringView cpset, SGRX_RigidBodyInfo* spec );
	GFW_EXPORT SGS_METHOD void RBApplyForce( int i, int type, Vec3 v, /*opt*/ Vec3 p );
	
	// - joints
	GFW_EXPORT SGS_METHOD void JTCreateHingeB2W( int i, int bi, SGRX_HingeJointInfo* spec );
	GFW_EXPORT SGS_METHOD void JTCreateHingeB2B( int i, int biA, int biB, SGRX_HingeJointInfo* spec );
	GFW_EXPORT SGS_METHOD void JTCreateConeTwistB2W( int i, int bi, SGRX_ConeTwistJointInfo* spec );
	GFW_EXPORT SGS_METHOD void JTCreateConeTwistB2B( int i, int biA, int biB, SGRX_ConeTwistJointInfo* spec );
	// ---
	
	// removal placeholders
	sgsVariable GetScriptedObject(){ return sgsVariable(); }
	SGS_CTX;
	GameLevel* m_level;
	
	DecalSysHandle m_dmgDecalSys;
	DecalSysHandle m_ovrDecalSys;
	
	MeshInstHandle m_meshes[ 4 ];
	PhyRigidBodyHandle m_bodies[ 4 ];
	PhyJointHandle m_joints[ 4 ];
};




#if 0
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
#endif


