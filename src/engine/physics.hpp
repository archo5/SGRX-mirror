

#pragma once
#include "engine.hpp"
#include "enganim.hpp"


#ifdef PHYSICS_BUILDING
#  define PHYSICS_EXPORT __declspec(dllexport)
#else
#  define PHYSICS_EXPORT __declspec(dllimport)
#endif


inline Vec3 PHY_QuaternionToEulerXYZ( const Quat& q )
{
	float sqx = q.x * q.x, sqy = q.y * q.y, sqz = q.z * q.z, sqw = q.w * q.w;
	return V3
	(
		atan2f( 2.0 * ( q.y * q.z + q.x * q.w ), -sqx - sqy + sqz + sqw ),
		asinf( -2.0 * ( q.x * q.z - q.y * q.w ) ),
		atan2f( 2.0 * ( q.x * q.y + q.z * q.w ), sqx - sqy - sqz + sqw )
	);
}


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
	
	virtual PhyShapeHandle GetShape() const = 0;
	virtual void SetShape( PhyShapeHandle s ) = 0;
	
	virtual Vec3 GetPosition() const = 0;
	virtual void SetPosition( const Vec3& v ) = 0;
	virtual Quat GetRotation() const = 0;
	virtual void SetRotation( const Quat& v ) = 0;
	
	virtual Vec3 GetLinearVelocity() const = 0;
	virtual void SetLinearVelocity( const Vec3& v ) = 0;
	virtual Vec3 GetAngularVelocity() const = 0;
	virtual void SetAngularVelocity( const Vec3& v ) = 0;
	
	virtual float GetFriction() const = 0;
	virtual void SetFriction( float v ) = 0;
	virtual float GetRestitution() const = 0;
	virtual void SetRestitution( float v ) = 0;
	virtual float GetMass() const = 0;
	virtual Vec3 GetInertia() const = 0;
	virtual void SetMassAndInertia( float mass, const Vec3& inertia ) const = 0;
	
	virtual float GetLinearDamping() const = 0;
	virtual void SetLinearDamping( float v ) = 0;
	virtual float GetAngularDamping() const = 0;
	virtual void SetAngularDamping( float v ) = 0;
	virtual Vec3 GetLinearFactor() const = 0;
	virtual void SetLinearFactor( const Vec3& v ) = 0;
	virtual Vec3 GetAngularFactor() const = 0;
	virtual void SetAngularFactor( const Vec3& v ) = 0;
	
	virtual bool IsKinematic() const = 0;
	virtual void SetKinematic( bool v ) = 0;
	virtual bool CanSleep() const = 0;
	virtual void SetCanSleep( bool v ) = 0;
	virtual void WakeUp() = 0;
	virtual bool GetEnabled() const = 0;
	virtual void SetEnabled( bool enabled ) = 0;
	virtual uint16_t GetGroup() const = 0;
	virtual uint16_t GetMask() const = 0;
	virtual void SetGroupAndMask( uint16_t group, uint16_t mask ) = 0;
	
	virtual void ApplyCentralForce( EPhyForceType type, const Vec3& v ) = 0;
	virtual void ApplyForce( EPhyForceType type, const Vec3& v, const Vec3& p ) = 0;
};
typedef Handle< SGRX_IPhyRigidBody > PhyRigidBodyHandle;


struct SGRX_PhyJointBaseInfo
{
	SGRX_PhyJointBaseInfo() : enabled(true), disableCollisions(true){}
	PhyRigidBodyHandle bodyA;
	PhyRigidBodyHandle bodyB;
	bool enabled;
	bool disableCollisions;
};

struct SGRX_PhyHingeJointInfo : SGRX_PhyJointBaseInfo
{
	SGRX_PhyHingeJointInfo() :
		useFrames(false),
		pivotA(V3(0)), pivotB(V3(0)),
		axisA(V3(0,0,1)), axisB(V3(0,0,1))
	{}
	bool useFrames;
	Vec3 pivotA;
	Vec3 pivotB;
	Vec3 axisA;
	Vec3 axisB;
	Mat4 frameA;
	Mat4 frameB;
};

struct SGRX_PhyConeTwistJointInfo : SGRX_PhyJointBaseInfo
{
	SGRX_PhyConeTwistJointInfo() :
		frameA(Mat4::Identity), frameB(Mat4::Identity),
		coneLimitX(FLT_MAX), coneLimitY(FLT_MAX), twistLimit(FLT_MAX)
	{}
	Mat4 frameA;
	Mat4 frameB;
	float coneLimitX; // limit angles (in radians)
	float coneLimitY;
	float twistLimit;
};

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
	virtual PhyShapeHandle CreateBoxShape( const Vec3& extents ) = 0;
	virtual PhyShapeHandle CreateConvexHullShape( const Vec3* data, size_t count ) = 0;
	virtual PhyShapeHandle CreateAABBShape( const Vec3& min, const Vec3& max ) = 0;
	virtual PhyShapeHandle CreateTriMeshShape( const Vec3* verts, size_t vcount, const void* idcs, size_t icount, bool index32 = false ) = 0;
	virtual PhyShapeHandle CreateShapeFromMesh( SGRX_IMesh* mesh ) = 0;
	
	virtual PhyRigidBodyHandle CreateRigidBody( const SGRX_PhyRigidBodyInfo& info ) = 0;
	virtual PhyJointHandle CreateHingeJoint( const SGRX_PhyHingeJointInfo& info ) = 0;
	virtual PhyJointHandle CreateConeTwistJoint( const SGRX_PhyConeTwistJointInfo& info ) = 0;
	
	virtual Vec3 GetGravity() = 0;
	virtual void SetGravity( const Vec3& v ) = 0;
	
	virtual bool Raycast( const Vec3& from, const Vec3& to, uint16_t group = 0x01, uint16_t mask = 0xffff, SGRX_PhyRaycastInfo* outinfo = NULL ) = 0;
	virtual bool ConvexCast( const PhyShapeHandle& sh, const Vec3& from, const Vec3& to, uint16_t group = 0x01, uint16_t mask = 0xffff, SGRX_PhyRaycastInfo* outinfo = NULL, float depth = 0.0f ) = 0;
};
typedef Handle< SGRX_IPhyWorld > PhyWorldHandle;


PHYSICS_EXPORT PhyWorldHandle PHY_CreateWorld();


