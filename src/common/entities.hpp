

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


#define ForceType_Velocity PFT_Velocity
#define ForceType_Impulse PFT_Impulse
#define ForceType_Acceleration PFT_Acceleration
#define ForceType_Force PFT_Force


#define MULTIENT_NUM_SLOTS 4
#define MULTIENT_RANGE_STR "[0-3]"

EXP_STRUCT MultiEntity : Entity, SGRX_MeshInstUserData
{
	SGS_OBJECT_INHERIT( Entity ) SGS_NO_DESTRUCT;
	ENT_SGS_IMPLEMENT;
	
	GFW_EXPORT MultiEntity( GameLevel* lev );
	GFW_EXPORT ~MultiEntity();
	GFW_EXPORT virtual void FixedTick( float deltaTime );
	GFW_EXPORT virtual void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT void PreRender();
	
	GFW_EXPORT virtual void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	GFW_EXPORT virtual void OnTransformUpdate();
	
	// - mesh instance
	GFW_EXPORT SGS_METHOD void MICreate( int i, StringView path );
	GFW_EXPORT SGS_METHOD void MIDestroy( int i );
	GFW_EXPORT SGS_METHOD bool MIExists( int i );
	GFW_EXPORT SGS_METHOD void MISetMesh( int i, StringView path );
	GFW_EXPORT SGS_METHOD void MISetEnabled( int i, bool enabled );
	GFW_EXPORT SGS_METHOD void MISetMatrix( int i, Mat4 mtx );
	GFW_EXPORT SGS_METHOD void MISetShaderConst( int i, int v, Vec4 var );
	GFW_EXPORT SGS_METHOD void MISetLayers( int i, uint32_t layers );
	
	GFW_EXPORT MeshHandle sgsGetMI0Mesh();
	GFW_EXPORT void sgsSetMI0Mesh( MeshHandle m );
	SGS_PROPERTY_FUNC( READ sgsGetMI0Mesh WRITE sgsSetMI0Mesh ) SGS_ALIAS( MeshHandle mi0mesh );
	SGS_PROPERTY Vec3 mi0sampleOffset;
	
	// - particle system
	GFW_EXPORT SGS_METHOD void PSCreate( int i, StringView path );
	GFW_EXPORT SGS_METHOD void PSDestroy( int i );
	GFW_EXPORT SGS_METHOD bool PSExists( int i );
	GFW_EXPORT SGS_METHOD void PSLoad( int i, StringView path );
	GFW_EXPORT SGS_METHOD void PSSetMatrix( int i, Mat4 mtx );
	GFW_EXPORT SGS_METHOD void PSSetMatrixFromMeshAABB( int i, int mi );
	GFW_EXPORT SGS_METHOD void PSPlay( int i );
	GFW_EXPORT SGS_METHOD void PSStop( int i );
	GFW_EXPORT SGS_METHOD void PSTrigger( int i );
	
	// - decal system
	GFW_EXPORT SGS_METHOD void DSCreate( StringView texDmgDecalPath,
		StringView texOvrDecalPath, StringView texFalloffPath, uint32_t size );
	GFW_EXPORT SGS_METHOD void DSDestroy();
	GFW_EXPORT SGS_METHOD void DSResize( uint32_t size );
	GFW_EXPORT SGS_METHOD void DSClear();
	
	// - rigid bodies
	GFW_EXPORT SGS_METHOD void RBCreateFromMesh( int i, int mi, SGRX_RigidBodyInfo* spec );
	GFW_EXPORT SGS_METHOD void RBCreateFromConvexPointSet( int i, StringView cpset, SGRX_RigidBodyInfo* spec );
	GFW_EXPORT SGS_METHOD void RBDestroy( int i );
	GFW_EXPORT SGS_METHOD bool RBExists( int i );
	GFW_EXPORT SGS_METHOD void RBSetEnabled( int i, bool enabled );
	GFW_EXPORT SGS_METHOD Vec3 RBGetPosition( int i );
	GFW_EXPORT SGS_METHOD void RBSetPosition( int i, Vec3 v );
	GFW_EXPORT SGS_METHOD Quat RBGetRotation( int i );
	GFW_EXPORT SGS_METHOD void RBSetRotation( int i, Quat v );
	GFW_EXPORT SGS_METHOD Mat4 RBGetMatrix( int i );
	GFW_EXPORT SGS_METHOD void RBApplyForce( int i, int type, Vec3 v, /*opt*/ Vec3 p );
	
	// - joints
	GFW_EXPORT SGS_METHOD void JTCreateHingeB2W( int i, int bi, SGRX_HingeJointInfo* spec );
	GFW_EXPORT SGS_METHOD void JTCreateHingeB2B( int i, int biA, int biB, SGRX_HingeJointInfo* spec );
	GFW_EXPORT SGS_METHOD void JTCreateConeTwistB2W( int i, int bi, SGRX_ConeTwistJointInfo* spec );
	GFW_EXPORT SGS_METHOD void JTCreateConeTwistB2B( int i, int biA, int biB, SGRX_ConeTwistJointInfo* spec );
	GFW_EXPORT SGS_METHOD void JTDestroy( int i );
	GFW_EXPORT SGS_METHOD bool JTExists( int i );
	GFW_EXPORT SGS_METHOD void JTSetEnabled( int i, bool enabled );
	// ---
	
	DecalSysHandle m_dmgDecalSys;
	DecalSysHandle m_ovrDecalSys;
	
	MeshInstHandle m_meshes[ MULTIENT_NUM_SLOTS ];
	PartSysHandle m_partSys[ MULTIENT_NUM_SLOTS ];
	PhyRigidBodyHandle m_bodies[ MULTIENT_NUM_SLOTS ];
	PhyJointHandle m_joints[ MULTIENT_NUM_SLOTS ];
	IVState< Vec3 > m_bodyPos[ MULTIENT_NUM_SLOTS ];
	IVState< Quat > m_bodyRot[ MULTIENT_NUM_SLOTS ];
	Vec3 m_bodyPosLerp[ MULTIENT_NUM_SLOTS ];
	Quat m_bodyRotLerp[ MULTIENT_NUM_SLOTS ];
//	LightHandle m_lights[ MULTIENT_NUM_SLOTS ];
	
	Mat4 m_meshMatrices[ MULTIENT_NUM_SLOTS ];
	Mat4 m_partSysMatrices[ MULTIENT_NUM_SLOTS ];
//	Mat4 m_lightMatrices[ MULTIENT_NUM_SLOTS ];
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


