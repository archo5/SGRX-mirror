

#pragma once
#include "engine.hpp"
#include "enganim.hpp"


#ifdef PHYSICS_BUILDING
#  define PHYSICS_EXPORT __declspec(dllexport)
#else
#  define PHYSICS_EXPORT __declspec(dllimport)
#endif


struct PHYSICS_EXPORT SGRX_IPhyShape
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	SGRX_IPhyShape() : _refcount(0), _type(0){}
	virtual ~SGRX_IPhyShape(){}
	
	virtual Vec3 GetScale() const = 0;
	virtual void SetScale( const Vec3& scale ) = 0;
	
	virtual Vec3 CalcInertia( float mass ) const = 0;
	
	int32_t _refcount;
	
protected:
	int _type;
};
typedef Handle< SGRX_IPhyShape > PhyShapeHandle;


struct SGRX_PhyRigidBodyInfo
{
	SGRX_PhyRigidBodyInfo() :
		position(V3(0)), rotation(Quat::Identity), friction(0.5f), restitution(0.1f),
		mass(0), inertia(V3(1.0f)), linearDamping(0.01f), angularDamping(0.01f),
		kinematic(false), canSleep(true), enabled(true)
	{}
	PhyShapeHandle shape;
	Vec3 position;
	Quat rotation;
	float friction;
	float restitution;
	float mass;
	Vec3 inertia;
	float linearDamping;
	float angularDamping;
	bool kinematic;
	bool canSleep;
	bool enabled;
};

struct PHYSICS_EXPORT SGRX_IPhyRigidBody
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	SGRX_IPhyRigidBody() : _refcount(0){}
	virtual ~SGRX_IPhyRigidBody(){}
	
	virtual void SetEnabled( bool enabled ) = 0;
	virtual Vec3 GetPosition() const = 0;
	virtual void SetPosition( const Vec3& v ) = 0;
	virtual Quat GetRotation() const = 0;
	virtual void SetRotation( const Quat& v ) = 0;
	virtual Vec3 GetLinearVelocity() const = 0;
	virtual void SetLinearVelocity( const Vec3& v ) = 0;
	virtual Vec3 GetAngularVelocity() const = 0;
	virtual void SetAngularVelocity( const Vec3& v ) = 0;
	
	int32_t _refcount;
};
typedef Handle< SGRX_IPhyRigidBody > PhyRigidBodyHandle;


struct PHYSICS_EXPORT SGRX_IPhyJoint
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	SGRX_IPhyJoint() : _refcount(0){}
	virtual ~SGRX_IPhyJoint(){}
	
	virtual void SetEnabled( bool enabled ) = 0;
	
	int32_t _refcount;
};
typedef Handle< SGRX_IPhyJoint > PhyJointHandle;


struct PHYSICS_EXPORT SGRX_IPhyWorld
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	SGRX_IPhyWorld() : _refcount(0){}
	virtual ~SGRX_IPhyWorld(){}
	
	virtual void Step( float dt ) = 0;
	virtual void DebugDraw() = 0;
	
	virtual PhyShapeHandle CreateSphereShape( float radius ) = 0;
	virtual PhyShapeHandle CreateCapsuleShape( float radius, float height ) = 0;
	virtual PhyShapeHandle CreateCylinderShape( const Vec3& extents ) = 0;
	virtual PhyShapeHandle CreateConvexHullShape( const Vec3* data, size_t count ) = 0;
	virtual PhyShapeHandle CreateAABBShape( const Vec3& min, const Vec3& max ) = 0;
	virtual PhyShapeHandle CreateTriMeshShape( const Vec3* verts, size_t vcount, const void* idcs, size_t icount, bool index32 = false ) = 0;
	virtual PhyShapeHandle CreateShapeFromMesh( SGRX_IMesh* mesh ) = 0;
	
	virtual PhyRigidBodyHandle CreateRigidBody( const SGRX_PhyRigidBodyInfo& info ) = 0;
	
	virtual Vec3 GetGravity() = 0;
	virtual void SetGravity( const Vec3& v ) = 0;
	
	int32_t _refcount;
};
typedef Handle< SGRX_IPhyWorld > PhyWorldHandle;


PHYSICS_EXPORT PhyWorldHandle PHY_CreateWorld();


struct ENGINE_EXPORT AnimRagdoll : Animator
{
	struct Body
	{
		Vec3 relPos;
		Quat relRot;
		PhyRigidBodyHandle bodyHandle;
		Vec3 prevPos;
		Vec3 currPos;
		Quat prevRot;
		Quat currRot;
	};
	
	AnimRagdoll();
	void Initialize( PhyWorldHandle world, MeshHandle mesh, struct SkeletonInfo* skinfo );
	virtual void Prepare( String* new_names, int count );
	virtual void Advance( float deltaTime );
	
	void SetBoneTransforms( int bone_id, const Vec3& prev_pos, const Vec3& curr_pos, const Quat& prev_rot, const Quat& curr_rot );
	void AdvanceTransforms( Animator* anim );
	void EnablePhysics( const Mat4& worldMatrix );
	void DisablePhysics();
	
	bool m_enabled;
	float m_lastTickSize;
	MeshHandle m_mesh;
	Array< Body > m_bones;
	Array< PhyJointHandle > m_joints;
};

