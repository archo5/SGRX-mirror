

#pragma once
#include <enganim.hpp>
#include <engext.hpp>
#include <physics.hpp>
#include <script.hpp>
#include <sound.hpp>



enum MeshInstEvent
{
	MIEVT_BulletHit = 1,
};

struct MI_BulletHit_Data
{
	Vec3 pos;
	Vec3 vel;
};

struct SGRX_MeshInstUserData
{
	SGRX_MeshInstUserData() : dmgDecalSysOverride(NULL), ownerType(0){}
	virtual ~SGRX_MeshInstUserData(){}
	virtual void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data ){}
	SGRX_DecalSystem* dmgDecalSysOverride;
	uint32_t ownerType;
};



struct SGRX_SIRigidBodyInfo : SGRX_PhyRigidBodyInfo
{
	typedef sgsHandle< SGRX_SIRigidBodyInfo > Handle;
	
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


#define SCRITEM_ForceType_Velocity PFT_Velocity
#define SCRITEM_ForceType_Impulse PFT_Impulse
#define SCRITEM_ForceType_Acceleration PFT_Acceleration
#define SCRITEM_ForceType_Force PFT_Force

void ScrItem_InstallAPI( SGS_CTX );



#define SCRITEM_NUM_SLOTS 4
#define SCRITEM_RANGE_STR "[0-3]"

struct SGRX_ScriptedItem : SGRX_MeshInstUserData
{
	typedef sgsHandle< SGRX_ScriptedItem > Handle;
	
	SGS_OBJECT;
	
	static SGRX_ScriptedItem* Create(
		SGRX_Scene* scene,
		SGRX_IPhyWorld* phyWorld,
		SGS_CTX,
		sgsVariable func,
		sgsVariable args );
	void Acquire();
	void Release();
	
	void SetLightSampler( SGRX_LightSampler* sampler );
	void SetPSRaycast( SGRX_IPSRaycast* psrc );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void PreRender();
	
	virtual void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	
	// ---
	SGS_IFUNC( GETINDEX ) int _getindex(
		SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop );
	SGS_IFUNC( SETINDEX ) int _setindex(
		SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, sgs_Variable* val, int isprop );
	
	// - common
	SGS_METHOD void SetMatrix( Mat4 mtx );
	
	// - mesh instance
	SGS_METHOD void MICreate( int i, StringView path );
	SGS_METHOD void MIDestroy( int i );
	SGS_METHOD bool MIExists( int i );
	SGS_METHOD void MISetMesh( int i, StringView path );
	SGS_METHOD void MISetEnabled( int i, bool enabled );
	SGS_METHOD void MISetDynamic( int i, bool dynamic );
	SGS_METHOD void MISetMatrix( int i, Mat4 mtx );
	
	// - particle system
	SGS_METHOD void PSCreate( int i, StringView path );
	SGS_METHOD void PSDestroy( int i );
	SGS_METHOD bool PSExists( int i );
	SGS_METHOD void PSLoad( int i, StringView path );
	SGS_METHOD void PSSetMatrix( int i, Mat4 mtx );
	SGS_METHOD void PSSetMatrixFromMeshAABB( int i, int mi );
	SGS_METHOD void PSPlay( int i );
	SGS_METHOD void PSStop( int i );
	SGS_METHOD void PSTrigger( int i );
	
	// - decal system
	SGS_METHOD void DSCreate( StringView texDecalPath, StringView texFalloffPath, uint32_t size );
	SGS_METHOD void DSDestroy();
	SGS_METHOD void DSResize( uint32_t size );
	SGS_METHOD void DSClear();
	
	// - rigid bodies
	SGS_METHOD void RBCreateFromMesh( int i, int mi, SGRX_SIRigidBodyInfo* spec );
	SGS_METHOD void RBCreateFromConvexPointSet( int i, StringView cpset, SGRX_SIRigidBodyInfo* spec );
	SGS_METHOD void RBDestroy( int i );
	SGS_METHOD bool RBExists( int i );
	SGS_METHOD void RBSetEnabled( int i, bool enabled );
	SGS_METHOD Vec3 RBGetPosition( int i );
	SGS_METHOD void RBSetPosition( int i, Vec3 v );
	SGS_METHOD Quat RBGetRotation( int i );
	SGS_METHOD void RBSetRotation( int i, Quat v );
	SGS_METHOD Mat4 RBGetMatrix( int i );
	SGS_METHOD void RBApplyForce( int i, int type, Vec3 v, /*opt*/ Vec3 p );
	// ---
	
	sgsVariable m_variable;
	
	SceneHandle m_scene;
	PhyWorldHandle m_phyWorld;
	SGRX_LightSampler* m_lightSampler;
	SGRX_IPSRaycast* m_psRaycast;
	
	DecalSysHandle m_decalSys;
	MeshInstHandle m_decalSysMI;
	MeshInstHandle m_meshes[ SCRITEM_NUM_SLOTS ];
	PartSysHandle m_partSys[ SCRITEM_NUM_SLOTS ];
	PhyRigidBodyHandle m_bodies[ SCRITEM_NUM_SLOTS ];
	IVState< Vec3 > m_bodyPos[ SCRITEM_NUM_SLOTS ];
	IVState< Quat > m_bodyRot[ SCRITEM_NUM_SLOTS ];
	Vec3 m_bodyPosLerp[ SCRITEM_NUM_SLOTS ];
	Quat m_bodyRotLerp[ SCRITEM_NUM_SLOTS ];
//	LightHandle m_lights[ SCRITEM_NUM_SLOTS ];
	
	Mat4 m_transform;
	Mat4 m_meshMatrices[ SCRITEM_NUM_SLOTS ];
	Mat4 m_partSysMatrices[ SCRITEM_NUM_SLOTS ];
//	Mat4 m_lightMatrices[ SCRITEM_NUM_SLOTS ];
};



