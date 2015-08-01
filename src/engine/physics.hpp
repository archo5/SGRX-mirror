

#pragma once
#include "engine.hpp"
#include "enganim.hpp"


#ifdef PHYSICS_BUILDING
#  define PHYSICS_EXPORT __declspec(dllexport)
#else
#  define PHYSICS_EXPORT __declspec(dllimport)
#endif


struct IF_GCC(PHYSICS_EXPORT) SGRX_IPhyShape : SGRX_RefCounted
{
	SGRX_IPhyShape() : _type(0){}
	virtual ~SGRX_IPhyShape(){}
	
	virtual Vec3 GetScale() const = 0;
	virtual void SetScale( const Vec3& scale ) = 0;
	
	virtual Vec3 CalcInertia( float mass ) const = 0;
	
protected:
	int _type;
};
typedef Handle< SGRX_IPhyShape > PhyShapeHandle;


struct SGRX_PhyRigidBodyInfo
{
	SGRX_PhyRigidBodyInfo() :
		position(V3(0)), rotation(Quat::Identity), friction(0.5f), restitution(0.1f), mass(0), inertia(V3(1.0f)),
		linearDamping(0.01f), angularDamping(0.01f), linearFactor(V3(1)), angularFactor(V3(1)),
		kinematic(false), canSleep(true), enabled(true), group(1), mask(0xffff)
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
	Vec3 linearFactor;
	Vec3 angularFactor;
	bool kinematic;
	bool canSleep;
	bool enabled;
	uint16_t group;
	uint16_t mask;
};

enum EPhyForceType
{
	PFT_Velocity,
	PFT_Impulse, // velocity += impulse * invMass
	PFT_Acceleration,
	PFT_Force, // acceleration += force * invMass
};

struct IF_GCC(PHYSICS_EXPORT) SGRX_IPhyRigidBody : SGRX_RefCounted
{
	SGRX_IPhyRigidBody(){}
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
	virtual Vec3 GetLinearFactor() const = 0;
	virtual void SetLinearFactor( const Vec3& v ) = 0;
	virtual Vec3 GetAngularFactor() const = 0;
	virtual void SetAngularFactor( const Vec3& v ) = 0;
	virtual void ApplyCentralForce( EPhyForceType type, const Vec3& v ) = 0;
	virtual void ApplyForce( EPhyForceType type, const Vec3& v, const Vec3& p ) = 0;
};
typedef Handle< SGRX_IPhyRigidBody > PhyRigidBodyHandle;


struct IF_GCC(PHYSICS_EXPORT) SGRX_IPhyJoint : SGRX_RefCounted
{
	SGRX_IPhyJoint(){}
	virtual ~SGRX_IPhyJoint(){}
	
	virtual void SetEnabled( bool enabled ) = 0;
};
typedef Handle< SGRX_IPhyJoint > PhyJointHandle;


struct SGRX_PhyRaycastInfo
{
	float factor;
	Vec3 point;
	Vec3 normal;
	PhyRigidBodyHandle body;
};

struct IF_GCC(PHYSICS_EXPORT) SGRX_IPhyWorld : SGRX_RefCounted
{
	SGRX_IPhyWorld(){}
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
	
	virtual bool Raycast( const Vec3& from, const Vec3& to, uint16_t group = 0x01, uint16_t mask = 0xffff, SGRX_PhyRaycastInfo* outinfo = NULL ) = 0;
	virtual bool ConvexCast( const PhyShapeHandle& sh, const Vec3& from, const Vec3& to, uint16_t group = 0x01, uint16_t mask = 0xffff, SGRX_PhyRaycastInfo* outinfo = NULL, float depth = 0.0f ) = 0;
};
typedef Handle< SGRX_IPhyWorld > PhyWorldHandle;


PHYSICS_EXPORT PhyWorldHandle PHY_CreateWorld();


