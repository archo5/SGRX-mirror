

#pragma once
#include <engine.hpp>
#include <enganim.hpp>
#include <pathfinding.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#include "level.hpp"
#include "systems.hpp"


EXP_STRUCT Trigger : Entity
{
	SGS_OBJECT_INHERIT( Entity );
	ENT_SGS_IMPLEMENT;
	
	SGS_PROPERTY_FUNC( READ WRITE VARNAME func ) sgsVariable m_func;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME funcOut ) sgsVariable m_funcOut;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME once ) bool m_once;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME done ) bool m_done;
	bool m_lastState;
	
	bool m_currState;
	
	GFW_EXPORT Trigger( GameLevel* lev, bool laststate = false );
	GFW_EXPORT SGS_METHOD void Invoke( bool newstate );
	GFW_EXPORT void Update( bool newstate );
	
	GFW_EXPORT SGS_METHOD_NAMED( SetupTrigger ) void sgsSetupTrigger( bool once, sgsVariable fn, sgsVariable fnout );
};

EXP_STRUCT SlidingDoor : Trigger
{
	SGS_OBJECT_INHERIT( Trigger );
	ENT_SGS_IMPLEMENT;
	
	float open_factor; // 0 .. 1
	float open_target; // 0 .. 1
	float open_time; // > 0
	
	Vec3 pos_open;
	Vec3 pos_closed;
	Quat rot_open;
	Quat rot_closed;
	bool target_state;
	
	SGS_PROPERTY_FUNC( READ VARNAME isSwitch ) bool m_isSwitch;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME switchPred ) sgsVariable m_switchPred;
	
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
	
	GFW_EXPORT void _UpdatePhysics();
	GFW_EXPORT void _UpdateTransforms( float bf );
	GFW_EXPORT SlidingDoor(
		GameLevel* lev,
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
		bool isswitch = false
	);
	GFW_EXPORT virtual void FixedTick( float deltaTime );
	GFW_EXPORT virtual void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT virtual void OnEvent( const StringView& type );
};

EXP_STRUCT PickupItem : Entity, IInteractiveEntity
{
	SGS_OBJECT_INHERIT( Entity );
	ENT_SGS_IMPLEMENT;
	
	MeshInstHandle m_meshInst;
	String m_type;
	int m_count;
	Vec3 m_pos;
	
	GFW_EXPORT PickupItem( GameLevel* lev, const StringView& name, const StringView& type, int count,
		const StringView& mesh, const Vec3& pos, const Quat& rot, const Vec3& scl );
	GFW_EXPORT virtual void OnEvent( const StringView& type );
	GFW_EXPORT virtual bool GetInteractionInfo( Vec3 pos, InteractInfo* out );
	
	virtual void* GetInterfaceImpl( uint32_t iface_id )
	{
		ENT_HAS_INTERFACE( IInteractiveEntity, iface_id, this );
		return NULL;
	}
};

EXP_STRUCT Actionable : Entity, IInteractiveEntity
{
	SGS_OBJECT_INHERIT( Entity );
	ENT_SGS_IMPLEMENT;
	
	MeshInstHandle m_meshInst;
	InteractInfo m_info;
	void sgsSetEnabled(){ SetEnabled( m_enabled ); }
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK sgsSetEnabled VARNAME enabled ) bool m_enabled;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME timeEstimate ) SGS_ALIAS( float m_info.timeEstimate );
	SGS_PROPERTY_FUNC( READ WRITE VARNAME timeActual ) SGS_ALIAS( float m_info.timeActual );
	SGS_PROPERTY_FUNC( READ WRITE VARNAME onSuccess ) sgsVariable m_onSuccess;
	SGS_PROPERTY_FUNC( READ VARNAME position ) SGS_ALIAS( Vec3 m_info.placePos );
	
	GFW_EXPORT Actionable( GameLevel* lev, const StringView& name, const StringView& mesh, const Vec3& pos, const Quat& rot, const Vec3& scl, const Vec3& placeoff, const Vec3& placedir );
	GFW_EXPORT virtual void OnEvent( const StringView& type );
	GFW_EXPORT virtual bool GetInteractionInfo( Vec3 pos, InteractInfo* out );
	GFW_EXPORT void SetEnabled( bool v );
	
	virtual void* GetInterfaceImpl( uint32_t iface_id )
	{
		ENT_HAS_INTERFACE( IInteractiveEntity, iface_id, this );
		return NULL;
	}
};


EXP_STRUCT ParticleFX : Entity
{
	ParticleSystem m_psys;
	String m_soundEventName;
	bool m_soundEventOneShot;
	SoundEventInstanceHandle m_soundEventInst;
	Vec3 m_position;
	
	GFW_EXPORT ParticleFX( GameLevel* lev, const StringView& name, const StringView& psys, const StringView& sndev, const Vec3& pos, const Quat& rot, const Vec3& scl, bool start );
	GFW_EXPORT virtual void Tick( float deltaTime, float blendFactor );
	GFW_EXPORT virtual void OnEvent( const StringView& type );
};


EXP_STRUCT MeshEntity : Entity
{
	SGS_OBJECT_INHERIT( Entity );
	ENT_SGS_IMPLEMENT;
	
	GFW_EXPORT MeshEntity( GameLevel* lev );
	GFW_EXPORT ~MeshEntity();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT virtual void EditorDrawWorld();
	GFW_EXPORT void _UpdateLighting();
	GFW_EXPORT void _UpdateBody();
	
	bool IsStatic() const { return m_isStatic; }
	void SetStatic( bool v ){ if( m_isStatic != v ){ m_body = NULL; } m_isStatic = v; _UpdateBody(); }
	bool IsVisible() const { return m_isVisible; }
	void SetVisible( bool v ){ m_isVisible = v; m_meshInst->enabled = v; }
	MeshHandle GetMesh() const { return m_mesh; }
	GFW_EXPORT void SetMesh( MeshHandle mesh );
	bool IsSolid() const { return m_isSolid; }
	void SetSolid( bool v ){ m_isSolid = v; _UpdateBody(); }
	bool GetLightingMode() const { return m_lightingMode; }
	void SetLightingMode( int v ){ m_lightingMode = v;
		m_meshInst->SetLightingMode( (SGRX_LightingMode) v ); _UpdateLighting(); }
	
	SGS_PROPERTY_FUNC( READ IsStatic WRITE SetStatic VARNAME isStatic ) bool m_isStatic;
	SGS_PROPERTY_FUNC( READ IsVisible WRITE SetVisible VARNAME visible ) bool m_isVisible;
	SGS_PROPERTY_FUNC( READ GetMesh WRITE SetMesh VARNAME mesh ) MeshHandle m_mesh;
	SGS_PROPERTY_FUNC( READ IsSolid WRITE SetSolid VARNAME solid ) bool m_isSolid;
	SGS_PROPERTY_FUNC( READ GetLightingMode WRITE SetLightingMode VARNAME lightingMode ) int m_lightingMode;
	// editor-only static mesh parameters
	SGS_PROPERTY_FUNC( READ WRITE VARNAME lmQuality ) float m_lmQuality;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME castLMS ) bool m_castLMS;
	
	MeshInstHandle m_meshInst;
	PhyShapeHandle m_phyShape;
	PhyRigidBodyHandle m_body;
};


EXP_STRUCT LightEntity : Entity
{
	SGS_OBJECT_INHERIT( Entity );
	ENT_SGS_IMPLEMENT;
	
	GFW_EXPORT LightEntity( GameLevel* lev );
	GFW_EXPORT ~LightEntity();
	virtual void OnTransformUpdate()
	{
		if( m_light )
		{
			m_light->SetTransform( GetWorldMatrix() );
			m_light->UpdateTransform();
		}
	}
	
	GFW_EXPORT void _UpdateLight();
	GFW_EXPORT void _UpdateShadows();
	GFW_EXPORT void _UpdateFlare();
	
#define RETNIFNOLIGHT if( !m_light ) return;
	bool IsStatic() const { return m_isStatic; }
	void SetStatic( bool v ){ m_isStatic = v; _UpdateLight(); }
	int GetType() const { return m_type; }
	void SetType( int v ){ m_type = v; RETNIFNOLIGHT; m_light->type = v; _UpdateShadows(); }
	bool IsEnabled() const { return m_isEnabled; }
	void SetEnabled( bool v ){ m_isEnabled = v; RETNIFNOLIGHT; m_light->enabled = v; }
	Vec3 GetColor() const { return m_color; }
	void SetColor( Vec3 v ){ m_color = v; RETNIFNOLIGHT; m_light->color = v * m_intensity; }
	float GetIntensity() const { return m_intensity; }
	void SetIntensity( float v ){ m_intensity = v; RETNIFNOLIGHT; m_light->color = m_color * v; }
	float GetRange() const { return m_range; }
	void SetRange( float v ){ m_range = v; RETNIFNOLIGHT; m_light->range = v; m_light->UpdateTransform(); }
	float GetPower() const { return m_power; }
	void SetPower( float v ){ m_power = v; RETNIFNOLIGHT; m_light->power = v; }
	float GetAngle() const { return m_angle; }
	void SetAngle( float v ){ m_angle = v; RETNIFNOLIGHT; m_light->angle = v; m_light->UpdateTransform(); }
	float GetAspect() const { return m_aspect; }
	void SetAspect( float v ){ m_aspect = v; RETNIFNOLIGHT; m_light->aspect = v; m_light->UpdateTransform(); }
	bool HasShadows() const { return m_hasShadows; }
	void SetShadows( bool v ){ m_hasShadows = v; RETNIFNOLIGHT; m_light->hasShadows = v; _UpdateShadows(); }
	float GetFlareSize() const { return m_flareSize; }
	void SetFlareSize( float v ){ m_flareSize = v; _UpdateFlare(); }
	Vec3 GetFlareOffset() const { return m_flareOffset; }
	void SetFlareOffset( Vec3 v ){ m_flareOffset = v; _UpdateFlare(); }
	TextureHandle GetCookieTexture() const { return m_cookieTexture; }
	void SetCookieTexture( TextureHandle h ){ m_cookieTexture = h; RETNIFNOLIGHT; m_light->cookieTexture = h; }
	
	SGS_PROPERTY_FUNC( READ IsStatic WRITE SetStatic VARNAME isStatic ) bool m_isStatic;
	SGS_PROPERTY_FUNC( READ GetType WRITE SetType VARNAME type ) int m_type;
	SGS_PROPERTY_FUNC( READ IsEnabled WRITE SetEnabled VARNAME enabled ) bool m_isEnabled;
	SGS_PROPERTY_FUNC( READ GetColor WRITE SetColor VARNAME color ) Vec3 m_color;
	SGS_PROPERTY_FUNC( READ GetIntensity WRITE SetIntensity VARNAME intensity ) float m_intensity;
	SGS_PROPERTY_FUNC( READ GetRange WRITE SetRange VARNAME range ) float m_range;
	SGS_PROPERTY_FUNC( READ GetPower WRITE SetPower VARNAME power ) float m_power;
	SGS_PROPERTY_FUNC( READ GetAngle WRITE SetAngle VARNAME angle ) float m_angle;
	SGS_PROPERTY_FUNC( READ GetAspect WRITE SetAspect VARNAME aspect ) float m_aspect;
	SGS_PROPERTY_FUNC( READ HasShadows WRITE SetShadows VARNAME hasShadows ) bool m_hasShadows;
	SGS_PROPERTY_FUNC( READ GetCookieTexture WRITE SetCookieTexture VARNAME cookieTexture ) TextureHandle m_cookieTexture;
	SGS_PROPERTY_FUNC( READ GetFlareSize WRITE SetFlareSize VARNAME flareSize ) float m_flareSize;
	SGS_PROPERTY_FUNC( READ GetFlareOffset WRITE SetFlareOffset VARNAME flareOffset ) Vec3 m_flareOffset;
	// editor-only static light parameters
	SGS_PROPERTY_FUNC( READ WRITE VARNAME innerAngle ) float m_innerAngle;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME spotCurve ) float m_spotCurve;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME lightRadius ) float lightRadius;
	
	LightHandle m_light;
};



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


struct StockEntityCreationSystem : IGameLevelSystem
{
	enum { e_system_uid = 999 };
	GFW_EXPORT StockEntityCreationSystem( GameLevel* lev );
	GFW_EXPORT virtual Entity* AddEntity( StringView type );
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


