

#pragma once
#include "engine.hpp"


struct EXPORT SGRX_IPhyShape
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	SGRX_IPhyShape() : _refcount(0), _type(0){}
	virtual ~SGRX_IPhyShape(){}
	
	virtual Vec3 GetScale() const = 0;
	virtual void SetScale( const Vec3& scale ) = 0;
	
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
		kinematic(false), canSleep(true)
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
};

struct EXPORT SGRX_IPhyRigidBody
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	SGRX_IPhyRigidBody() : _refcount(0){}
	virtual ~SGRX_IPhyRigidBody(){}
	
	virtual Vec3 GetPosition() const = 0;
	virtual void SetPosition( const Vec3& v ) = 0;
	virtual Quat GetRotation() const = 0;
	virtual void SetRotation( const Quat& v ) = 0;
	
	int32_t _refcount;
};
typedef Handle< SGRX_IPhyRigidBody > PhyRigidBodyHandle;


struct EXPORT SGRX_IPhyWorld
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	SGRX_IPhyWorld() : _refcount(0){}
	virtual ~SGRX_IPhyWorld(){}
	
	virtual void Step( float dt ) = 0;
	virtual void DebugDraw() = 0;
	
	virtual PhyRigidBodyHandle CreateRigidBody( const SGRX_PhyRigidBodyInfo& info ) = 0;
	
	virtual Vec3 GetGravity() = 0;
	virtual void SetGravity( const Vec3& v ) = 0;
	
	int32_t _refcount;
};
typedef Handle< SGRX_IPhyWorld > PhyWorldHandle;


EXPORT PhyShapeHandle PHY_CreateConvexHullShape( const Vec3* data, size_t count );
EXPORT PhyShapeHandle PHY_CreateAABBShape( const Vec3& min, const Vec3& max );
EXPORT PhyShapeHandle PHY_CreateTriMeshShape( const Vec3* verts, size_t vcount, const void* idcs, size_t icount, bool index32 = false );
EXPORT PhyShapeHandle PHY_CreateShapeFromMesh( SGRX_IMesh* mesh );
EXPORT PhyWorldHandle PHY_CreateWorld();

