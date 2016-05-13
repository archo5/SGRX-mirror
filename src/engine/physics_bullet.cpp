

#include <bullet.hpp>

#include "physics.hpp"


typedef btRigidBody::btRigidBodyConstructionInfo btRBCI;


static FINLINE btVector3 V2BV( const Vec3& v )
{
	return btVector3( v.x, v.y, v.z );
}
static FINLINE Vec3 BV2V( const btVector3& v )
{
	return V3( v.x(), v.y(), v.z() );
}
static FINLINE btQuaternion Q2BQ( const Quat& q )
{
	return btQuaternion( q.x, q.y, q.z, q.w );
}
static FINLINE Quat BQ2Q( const btQuaternion& q )
{
	return QUAT( q.x(), q.y(), q.z(), q.w() );
}
static FINLINE btMatrix3x3 Q2BM3( const Quat& q )
{
	return btMatrix3x3( Q2BQ( q ) );
}


struct BulletPhyShape : SGRX_IPhyShape
{
	virtual ~BulletPhyShape(){ delete m_colShape; }
	void _Init(){ m_colShape->setUserPointer( this ); }
	
	virtual Vec3 GetScale() const
	{
		return BV2V( m_colShape->getLocalScaling() );
	}
	virtual void SetScale( const Vec3& scale )
	{
		m_colShape->setLocalScaling( V2BV( scale ) );
	}
	virtual Vec3 CalcInertia( float mass ) const
	{
		btVector3 outi;
		m_colShape->calculateLocalInertia( mass, outi );
		return BV2V( outi );
	}
	
	virtual int GetChildShapeCount(){ return 0; }
	virtual void AddChildShape( SGRX_IPhyShape* shape, Vec3 pos = V3(0), Quat rot = Quat::Identity ){}
	virtual void RemoveChildShapeByType( SGRX_IPhyShape* shape ){}
	virtual void UpdateChildShapeTransform( int i, Vec3 pos, Quat rot = Quat::Identity ){}
	
	btCollisionShape* m_colShape;
};

struct BulletSphereShape : BulletPhyShape
{
	BulletSphereShape( float radius ){ m_colShape = new btSphereShape( radius ); _Init(); }
};

struct BulletCapsuleShape : BulletPhyShape
{
	BulletCapsuleShape( float radius, float height ){ m_colShape = new btCapsuleShapeZ( radius, height ); _Init(); }
};

struct BulletCylinderShape : BulletPhyShape
{
	BulletCylinderShape( const Vec3& extents ){ m_colShape = new btCylinderShapeZ( V2BV( extents ) ); _Init(); }
};

struct BulletBoxShape : BulletPhyShape
{
	BulletBoxShape( const Vec3& extents ){ m_colShape = new btBoxShape( V2BV( extents ) ); _Init(); }
};

struct BulletConvexHullShape : BulletPhyShape
{
	BulletConvexHullShape( const Vec3* data, size_t count ){ m_colShape = new btConvexHullShape( &data->x, count, sizeof(Vec3) ); _Init(); }
};

struct BulletTriMeshShape : BulletPhyShape
{
	CLASS_ALIGNED_ALLOC();

	BulletTriMeshShape( const Vec3* verts, size_t vcount, const void* idata, size_t icount, bool index32 )
	{
		if( index32 )
		{
			uint32_t* idcs = (uint32_t*) idata;
			for( size_t i = 0; i < icount; i += 3 )
			{
				m_meshIface.addTriangle( V2BV( verts[ idcs[ i+0 ] ] ), V2BV( verts[ idcs[ i+1 ] ] ), V2BV( verts[ idcs[ i+2 ] ] ), true );
			}
		}
		else
		{
			uint16_t* idcs = (uint16_t*) idata;
			for( size_t i = 0; i < icount; i += 3 )
			{
				m_meshIface.addTriangle( V2BV( verts[ idcs[ i+0 ] ] ), V2BV( verts[ idcs[ i+1 ] ] ), V2BV( verts[ idcs[ i+2 ] ] ), true );
			}
		}
		m_colShape = new btBvhTriangleMeshShape( &m_meshIface, false, true );
		_Init();
	}
	
	btTriangleMesh m_meshIface;
};

struct BulletCompoundShape : BulletPhyShape
{
	BulletCompoundShape()
	{
		m_colShape = new btCompoundShape();
		_Init();
	}
	~BulletCompoundShape()
	{
		for( int i = 0, num = CS()->getNumChildShapes(); i < num; ++i )
		{
			((BulletPhyShape*) CS()->getChildShape( i )->getUserPointer())->Release();
		}
	}
	FINLINE btCompoundShape* CS(){ return (btCompoundShape*) m_colShape; }
	
	virtual int GetChildShapeCount()
	{
		return CS()->getNumChildShapes();
	}
	virtual void AddChildShape( SGRX_IPhyShape* shape, Vec3 pos, Quat rot )
	{
		shape->Acquire();
		btTransform tf( Q2BM3( rot ), V2BV( pos ) );
		CS()->addChildShape( tf, ((BulletPhyShape*) shape)->m_colShape );
	}
	virtual void RemoveChildShapeByType( SGRX_IPhyShape* shape )
	{
		int numBefore = CS()->getNumChildShapes();
		CS()->removeChildShape( ((BulletPhyShape*) shape)->m_colShape );
		int removed = numBefore - CS()->getNumChildShapes();
		while( removed --> 0 )
			shape->Release();
	}
	virtual void UpdateChildShapeTransform( int i, Vec3 pos, Quat rot )
	{
		ASSERT( i >= 0 && i < CS()->getNumChildShapes() );
		btTransform tf( Q2BM3( rot ), V2BV( pos ) );
		CS()->updateChildTransform( i, tf );
	}
};


struct BulletPhyRigidBody : SGRX_IPhyRigidBody
{
	BulletPhyRigidBody( struct BulletPhyWorld* world, const SGRX_PhyRigidBodyInfo& rbinfo );
	~BulletPhyRigidBody();
	
	virtual PhyShapeHandle GetShape() const { return m_shape; }
	virtual void SetShape( PhyShapeHandle s )
	{
		m_body->setCollisionShape( ((BulletPhyShape*)s.item)->m_colShape );
		m_shape = s;
	}
	
	virtual Vec3 GetPosition() const { return BV2V( m_body->getCenterOfMassPosition() ); }
	virtual void SetPosition( const Vec3& v )
	{
		m_body->getWorldTransform().setOrigin( V2BV( v ) );
		m_body->setWorldTransform( m_body->getWorldTransform() );
		if( m_body->isKinematicObject() )
			m_body->getMotionState()->setWorldTransform( m_body->getWorldTransform() );
	}
	virtual Quat GetRotation() const { return BQ2Q( m_body->getOrientation() ); }
	virtual void SetRotation( const Quat& q )
	{
		m_body->getWorldTransform().setRotation( Q2BQ( q ) );
		m_body->setWorldTransform( m_body->getWorldTransform() );
		if( m_body->isKinematicObject() )
			m_body->getMotionState()->setWorldTransform( m_body->getWorldTransform() );
	}
	
	virtual Vec3 GetLinearVelocity() const { return BV2V( m_body->getLinearVelocity() ); }
	virtual void SetLinearVelocity( const Vec3& v ){ m_body->setLinearVelocity( V2BV( v ) ); }
	virtual Vec3 GetAngularVelocity() const { return BV2V( m_body->getAngularVelocity() ); }
	virtual void SetAngularVelocity( const Vec3& v ){ m_body->setAngularVelocity( V2BV( v ) ); }
	
	virtual float GetFriction() const { return m_body->getFriction(); }
	virtual void SetFriction( float v ){ m_body->setFriction( v ); }
	virtual float GetRestitution() const { return m_body->getRestitution(); }
	virtual void SetRestitution( float v ){ m_body->setRestitution( v ); }
	virtual float GetMass() const { float inv = m_body->getInvMass(); return inv != 0 ? 1.0f / inv : 0; }
	virtual Vec3 GetInertia() const
	{
		Vec3 inv = BV2V( m_body->getInvInertiaDiagLocal() );
		return V3(
			inv.x != 0 ? 1.0f / inv.x : 0,
			inv.y != 0 ? 1.0f / inv.y : 0,
			inv.z != 0 ? 1.0f / inv.z : 0 );
	}
	virtual void SetMassAndInertia( float mass, const Vec3& inertia )
	{
		bool mc0 = m_body->getInvMass() != 0;
		bool mc1 = mass != 0;
		m_body->setMassProps( mass, V2BV( inertia ) );
		if( mc0 != mc1 )
		{
			uint32_t cf = m_body->getCollisionFlags();
			if( mc1 )
				m_body->setCollisionFlags( cf & ~btCollisionObject::CF_STATIC_OBJECT );
			else
				m_body->setCollisionFlags( cf | btCollisionObject::CF_STATIC_OBJECT );
			SetEnabled( false ); // call removeRigidBody
			SetEnabled( true ); // call addRigidBody with new mass props
		}
	}
	
	virtual float GetLinearDamping() const { return m_body->getLinearDamping(); }
	virtual void SetLinearDamping( float v ){ m_body->setDamping( v, m_body->getAngularDamping() ); }
	virtual float GetAngularDamping() const { return m_body->getAngularDamping(); }
	virtual void SetAngularDamping( float v ){ m_body->setDamping( m_body->getLinearDamping(), v ); }
	virtual Vec3 GetLinearFactor() const { return BV2V( m_body->getLinearFactor() ); }
	virtual void SetLinearFactor( const Vec3& v ){ m_body->setLinearFactor( V2BV( v ) ); }
	virtual Vec3 GetAngularFactor() const { return BV2V( m_body->getAngularFactor() ); }
	virtual void SetAngularFactor( const Vec3& v ){ m_body->setAngularFactor( V2BV( v ) ); }
	
	virtual bool IsKinematic() const { return m_body->isKinematicObject(); }
	virtual void SetKinematic( bool v )
	{
		SetEnabled( false ); // call removeRigidBody
		int flags = m_body->getCollisionFlags();
		if( v )
		{
			m_body->setMassProps( 0, btVector3(0,0,0) );
			m_body->setCollisionFlags( flags | btCollisionObject::CF_KINEMATIC_OBJECT );
			m_body->setWorldTransform( m_body->getWorldTransform() );
			m_body->getMotionState()->setWorldTransform( m_body->getWorldTransform() );
			m_body->forceActivationState( DISABLE_DEACTIVATION );
		}
		else
			m_body->setCollisionFlags( flags & ~btCollisionObject::CF_KINEMATIC_OBJECT );
		SetEnabled( true ); // call addRigidBody to update broadphase?
	}
	virtual bool CanSleep() const { return m_body->getActivationState() == DISABLE_DEACTIVATION; }
	virtual void SetCanSleep( bool v )
	{
		int state = m_body->getActivationState();
		if( v && state == ISLAND_SLEEPING )
			return; // already sleeping, do not disturb
		m_body->forceActivationState( v ? 0 : DISABLE_DEACTIVATION );
	}
	virtual void WakeUp(){ m_body->activate( true ); }
	virtual bool GetEnabled() const { return m_body->isInWorld(); }
	virtual void SetEnabled( bool enabled );
	virtual uint16_t GetGroup() const { return m_group; }
	virtual uint16_t GetMask() const { return m_mask; }
	virtual void SetGroupAndMask( uint16_t group, uint16_t mask )
	{
		m_group = group;
		m_mask = mask;
		SetEnabled( false ); // call removeRigidBody
		SetEnabled( true ); // call addRigidBody with new group/mask
	}
	
	virtual float GetCCDSweptSphereRadius() const { return m_body->getCcdSweptSphereRadius(); }
	virtual void SetCCDSweptSphereRadius( float v ){ m_body->setCcdSweptSphereRadius( v ); }
	virtual float GetCCDMotionThreshold() const { return m_body->getCcdMotionThreshold(); }
	virtual void SetCCDMotionThreshold( float v ){ m_body->setCcdMotionThreshold( v ); }
	
	virtual void ApplyCentralForce( EPhyForceType type, const Vec3& v );
	virtual void ApplyForce( EPhyForceType type, const Vec3& v, const Vec3& p );
	virtual void FlushContacts();
	
	struct BulletPhyWorld* m_world;
	btRigidBody* m_body;
	PhyShapeHandle m_shape;
	uint16_t m_group;
	uint16_t m_mask;
};


struct BulletPhyJoint : SGRX_IPhyJoint
{
	BulletPhyJoint( struct BulletPhyWorld* w, bool dc ) :
		m_world(w), m_disableCollisions(dc){}
	virtual ~BulletPhyJoint(){}
	
	void _SetEnabled( btTypedConstraint* ct, bool enabled );
	
	struct BulletPhyWorld* m_world;
	bool m_disableCollisions;
	PhyRigidBodyHandle m_bodyA;
	PhyRigidBodyHandle m_bodyB;
};


struct BulletPhyHingeJoint : BulletPhyJoint
{
	BulletPhyHingeJoint( struct BulletPhyWorld* world, const SGRX_PhyHingeJointInfo& hjinfo );
	virtual ~BulletPhyHingeJoint();
	
	virtual void SetEnabled( bool enabled ){ _SetEnabled( m_joint, enabled ); }
	
	btHingeConstraint* m_joint;
};


struct BulletPhyConeTwistJoint : BulletPhyJoint
{
	BulletPhyConeTwistJoint( struct BulletPhyWorld* world, const SGRX_PhyConeTwistJointInfo& ctjinfo );
	virtual ~BulletPhyConeTwistJoint();
	
	virtual void SetEnabled( bool enabled ){ _SetEnabled( m_joint, enabled ); }
	
	btConeTwistConstraint* m_joint;
};


struct BulletPhyWorld : SGRX_IPhyWorld
{
	BulletPhyWorld();
	~BulletPhyWorld();
	
	virtual void Step( float dt );
	virtual void DebugDraw();
	
	virtual PhyShapeHandle CreateSphereShape( float radius );
	virtual PhyShapeHandle CreateCapsuleShape( float radius, float height );
	virtual PhyShapeHandle CreateCylinderShape( const Vec3& extents );
	virtual PhyShapeHandle CreateBoxShape( const Vec3& extents );
	virtual PhyShapeHandle CreateConvexHullShape( const Vec3* data, size_t count );
	virtual PhyShapeHandle CreateAABBShape( const Vec3& min, const Vec3& max );
	virtual PhyShapeHandle CreateTriMeshShape( const Vec3* verts, size_t vcount, const void* idcs, size_t icount, bool index32 = false );
	virtual PhyShapeHandle CreateShapeFromMesh( SGRX_IMesh* mesh );
	virtual PhyShapeHandle CreateCompoundShape();
	
	virtual PhyRigidBodyHandle CreateRigidBody( const SGRX_PhyRigidBodyInfo& info );
	virtual PhyJointHandle CreateHingeJoint( const SGRX_PhyHingeJointInfo& info );
	virtual PhyJointHandle CreateConeTwistJoint( const SGRX_PhyConeTwistJointInfo& info );
	
	virtual Vec3 GetGravity();
	virtual void SetGravity( const Vec3& v );
	
	virtual bool Raycast( const Vec3& from, const Vec3& to, uint16_t group, uint16_t mask, SGRX_PhyRaycastInfo* outinfo );
	virtual bool ConvexCast( const PhyShapeHandle& sh, const Vec3& from, const Vec3& to, uint16_t group, uint16_t mask, SGRX_PhyRaycastInfo* outinfo, float depth );
	
	btBroadphaseInterface*
		m_broadphase;
	btDefaultCollisionConfiguration*
		m_collisionConfig;
	btCollisionDispatcher*
		m_collisionDispatcher;
	btSequentialImpulseConstraintSolver*
		m_constraintSolver;
	btDiscreteDynamicsWorld*
		m_world;
};


static FINLINE void RBInfo2RBCI( const SGRX_PhyRigidBodyInfo& rbinfo, btRBCI& rbci )
{
	BulletPhyShape* ps = (BulletPhyShape*) (SGRX_IPhyShape*) rbinfo.shape;
	rbci.m_collisionShape = ps->m_colShape;
	rbci.m_startWorldTransform = btTransform( Q2BM3( rbinfo.rotation ), V2BV( rbinfo.position ) );
	rbci.m_friction = rbinfo.friction;
	rbci.m_restitution = rbinfo.restitution;
	rbci.m_mass = rbinfo.mass;
	rbci.m_localInertia = V2BV( rbinfo.inertia );
	rbci.m_linearDamping = rbinfo.linearDamping;
	rbci.m_angularDamping = rbinfo.angularDamping;
}


BulletPhyRigidBody::BulletPhyRigidBody( struct BulletPhyWorld* world, const SGRX_PhyRigidBodyInfo& rbinfo )
	: m_world( world ), m_group( rbinfo.group ), m_mask( rbinfo.mask )
{
	btRBCI rbci( 0, NULL, NULL );
	RBInfo2RBCI( rbinfo, rbci );
	
	rbci.m_motionState = new btDefaultMotionState( rbci.m_startWorldTransform );
	m_body = new btRigidBody( rbci );
	if( rbinfo.kinematic )
	{
		m_body->setCollisionFlags( m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT );
		m_body->setActivationState( DISABLE_DEACTIVATION );
	}
	if( !rbinfo.canSleep )
	{
		m_body->setActivationState( DISABLE_DEACTIVATION );
	}
	m_body->setLinearFactor( V2BV( rbinfo.linearFactor ) );
	m_body->setAngularFactor( V2BV( rbinfo.angularFactor ) );
	m_body->setUserPointer( this );
	m_shape = rbinfo.shape;
	if( rbinfo.enabled )
	{
		m_world->m_world->addRigidBody( m_body, m_group, m_mask );
	}
}

BulletPhyRigidBody::~BulletPhyRigidBody()
{
	if( m_world )
	{
		m_world->m_world->removeRigidBody( m_body );
	}
	btMotionState* mstate = m_body->getMotionState();
	delete m_body;
	delete mstate;
}

void BulletPhyRigidBody::SetEnabled( bool enabled )
{
	if( enabled && !m_body->isInWorld() )
		m_world->m_world->addRigidBody( m_body, m_group, m_mask );
	else if( !enabled && m_body->isInWorld() )
		m_world->m_world->removeRigidBody( m_body );
}

void BulletPhyRigidBody::ApplyCentralForce( EPhyForceType type, const Vec3& v )
{
	float mass = m_body->getInvMass();
	if( mass == 0 )
		return;
	mass = 1.0f / mass;
	switch( type )
	{
	case PFT_Velocity:
		m_body->applyCentralImpulse( V2BV( v ) * mass );
		break;
	case PFT_Impulse:
		m_body->applyCentralImpulse( V2BV( v ) );
		break;
	case PFT_Acceleration:
		m_body->applyCentralForce( V2BV( v ) * mass );
		break;
	case PFT_Force:
		m_body->applyCentralForce( V2BV( v ) );
		break;
	}
}

void BulletPhyRigidBody::ApplyForce( EPhyForceType type, const Vec3& v, const Vec3& p )
{
	float mass = m_body->getInvMass();
	if( mass == 0 )
		return;
	mass = 1.0f / mass;
	btVector3 relPos = m_body->getCenterOfMassTransform().invXform( V2BV( p ) );
	switch( type )
	{
	case PFT_Velocity:
		m_body->applyImpulse( V2BV( v ) * mass, relPos );
		break;
	case PFT_Impulse:
		m_body->applyImpulse( V2BV( v ), relPos );
		break;
	case PFT_Acceleration:
		m_body->applyForce( V2BV( v ) * mass, relPos );
		break;
	case PFT_Force:
		m_body->applyForce( V2BV( v ), relPos );
		break;
	}
}

void BulletPhyRigidBody::FlushContacts()
{
	m_world->m_broadphase->getOverlappingPairCache()->cleanProxyFromPairs(
		m_body->getBroadphaseProxy(), m_world->m_collisionDispatcher );
}


void BulletPhyJoint::_SetEnabled( btTypedConstraint* ct, bool enabled )
{
	if( enabled )
		m_world->m_world->addConstraint( ct, m_disableCollisions );
	else
		m_world->m_world->removeConstraint( ct );
}


// generate joint frame matrix
inline Mat4 M2JFM( const Mat4& m )
{
	// there's probably a quicker way but..
//	Vec3 tr = m.GetTranslation();
	return m;
		//* Mat4::CreateTranslation( -tr )
		//* Mat4::Basis( V3(0,0,1), V3(-1,0,0), V3(0,1,0), true )
		//* Mat4::CreateTranslation( tr );
}


BulletPhyHingeJoint::BulletPhyHingeJoint( struct BulletPhyWorld* world,
	const SGRX_PhyHingeJointInfo& hjinfo )
	: BulletPhyJoint( world, hjinfo.disableCollisions )
{
	m_bodyA = hjinfo.bodyA;
	m_bodyB = hjinfo.bodyB;
	SGRX_CAST( BulletPhyRigidBody*, rbA, m_bodyA.item );
	SGRX_CAST( BulletPhyRigidBody*, rbB, m_bodyB.item );
	if( hjinfo.useFrames )
	{
		btTransform frameA;
		btTransform frameB;
		frameA.setFromOpenGLMatrix( hjinfo.frameA.a );
		if( rbB )
		{
			frameB.setFromOpenGLMatrix( hjinfo.frameB.a );
			m_joint = new btHingeConstraint( *rbA->m_body, *rbB->m_body, frameA, frameB );
		}
		else
		{
			m_joint = new btHingeConstraint( *rbA->m_body, frameA );
		}
	}
	else
	{
		if( rbB )
		{
			m_joint = new btHingeConstraint( *rbA->m_body, *rbB->m_body, V2BV( hjinfo.pivotA ),
				V2BV( hjinfo.pivotB ), V2BV( hjinfo.axisA ), V2BV( hjinfo.axisB ) );
		}
		else
		{
			m_joint = new btHingeConstraint( *rbA->m_body, V2BV( hjinfo.pivotA ), V2BV( hjinfo.axisA ) );
		}
	}
	if( hjinfo.enabled )
	{
		_SetEnabled( m_joint, true );
	}
}

BulletPhyHingeJoint::~BulletPhyHingeJoint()
{
	m_world->m_world->removeConstraint( m_joint );
	delete m_joint;
}


BulletPhyConeTwistJoint::BulletPhyConeTwistJoint( struct BulletPhyWorld* world,
	const SGRX_PhyConeTwistJointInfo& ctjinfo )
	: BulletPhyJoint( world, ctjinfo.disableCollisions )
{
	m_bodyA = ctjinfo.bodyA;
	m_bodyB = ctjinfo.bodyB;
	SGRX_CAST( BulletPhyRigidBody*, rbA, m_bodyA.item );
	SGRX_CAST( BulletPhyRigidBody*, rbB, m_bodyB.item );
	btTransform frameA;
	btTransform frameB;
	frameA.setFromOpenGLMatrix( M2JFM( ctjinfo.frameA ).a );
	if( rbB )
	{
		frameB.setFromOpenGLMatrix( M2JFM( ctjinfo.frameB ).a );
		m_joint = new btConeTwistConstraint( *rbA->m_body, *rbB->m_body, frameA, frameB );
	}
	else
	{
		m_joint = new btConeTwistConstraint( *rbA->m_body, frameA );
	}
	m_joint->setLimit( ctjinfo.coneLimitX, ctjinfo.coneLimitY, ctjinfo.twistLimit );
	if( ctjinfo.enabled )
	{
		_SetEnabled( m_joint, true );
	}
}

BulletPhyConeTwistJoint::~BulletPhyConeTwistJoint()
{
	m_world->m_world->removeConstraint( m_joint );
	delete m_joint;
}


BulletPhyWorld::BulletPhyWorld()
{
	m_broadphase = new btDbvtBroadphase();
	m_collisionConfig = new btDefaultCollisionConfiguration();
	m_collisionDispatcher = new btCollisionDispatcher( m_collisionConfig );
	m_constraintSolver = new btSequentialImpulseConstraintSolver;
	m_world = new btDiscreteDynamicsWorld
	(
		m_collisionDispatcher,
		m_broadphase,
		m_constraintSolver,
		m_collisionConfig
	);
	m_world->setWorldUserInfo( this );
	m_world->setGravity( btVector3(0,0,0) );
	btContactSolverInfo& info = m_world->getSolverInfo();
	info.m_splitImpulse = true;
	info.m_splitImpulsePenetrationThreshold = 0;//-0.001f;
	info.m_erp2 = 0.2f;
	LOG << LOG_DATE << "  Created Bullet physics world";
}

BulletPhyWorld::~BulletPhyWorld()
{
	btCollisionObjectArray& coa = m_world->getCollisionObjectArray();
	for( int i = 0; i < coa.size(); ++i )
	{
		BulletPhyRigidBody* co = (BulletPhyRigidBody*) coa[i]->getUserPointer();
		co->m_world = NULL;
	}
	delete m_world;
	delete m_constraintSolver;
	delete m_collisionDispatcher;
	delete m_collisionConfig;
	delete m_broadphase;
}

void BulletPhyWorld::Step( float dt )
{
	m_world->stepSimulation( dt, 0 );
}

struct BulletDebugDrawer : btIDebugDraw
{
	int m_debugMode;
	BatchRenderer& br;
	
	BulletDebugDrawer() : m_debugMode( DBG_DrawWireframe | DBG_DrawAabb ), br( GR2D_GetBatchRenderer() ){}
	virtual ~BulletDebugDrawer(){}
	
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
	{
		br.SetPrimitiveType( PT_Lines );
		br.Col( fromColor.x(), fromColor.y(), fromColor.z() );
		br.Pos( BV2V( from ) );
		br.Col( toColor.x(), toColor.y(), toColor.z() );
		br.Pos( BV2V( to ) );
	}
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color){ drawLine( from, to, color, color ); }
	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
	{
		float q = 0.1f;
		br.SetPrimitiveType( PT_Lines );
		br.Col(1);
		br.Pos( PointOnB.x() - q, PointOnB.y(), PointOnB.z() ).Pos( PointOnB.x() + q, PointOnB.y(), PointOnB.z() );
		br.Pos( PointOnB.x(), PointOnB.y() - q, PointOnB.z() ).Pos( PointOnB.x(), PointOnB.y() + q, PointOnB.z() );
		br.Pos( PointOnB.x(), PointOnB.y(), PointOnB.z() - q ).Pos( PointOnB.x(), PointOnB.y(), PointOnB.z() + q );
	}
	virtual void reportErrorWarning(const char* warningString)
	{
		LOG << "BULLET: " << warningString;
	}
	virtual void draw3dText(const btVector3& location, const char* textString)
	{
	}
	
	virtual void setDebugMode( int debugMode ){ m_debugMode = debugMode; }
	virtual int getDebugMode() const { return m_debugMode; }
};

void BulletPhyWorld::DebugDraw()
{
	BulletDebugDrawer bdd;
	m_world->setDebugDrawer( &bdd );
	m_world->debugDrawWorld();
	m_world->setDebugDrawer( NULL );
}

PhyShapeHandle BulletPhyWorld::CreateSphereShape( float radius )
{
	return new BulletSphereShape( radius );
}

PhyShapeHandle BulletPhyWorld::CreateCapsuleShape( float radius, float height )
{
	return new BulletCapsuleShape( radius, height );
}

PhyShapeHandle BulletPhyWorld::CreateCylinderShape( const Vec3& extents )
{
	return new BulletCylinderShape( extents );
}

PhyShapeHandle BulletPhyWorld::CreateBoxShape( const Vec3& extents )
{
	return new BulletBoxShape( extents );
}

PhyShapeHandle BulletPhyWorld::CreateConvexHullShape( const Vec3* data, size_t count )
{
	return new BulletConvexHullShape( data, count );
}

PhyShapeHandle BulletPhyWorld::CreateAABBShape( const Vec3& min, const Vec3& max )
{
	Vec3 pts[8] =
	{
		V3( min.x, min.y, min.z ), V3( max.x, min.y, min.z ),
		V3( min.x, max.y, min.z ), V3( max.x, max.y, min.z ),
		V3( min.x, min.y, max.z ), V3( max.x, min.y, max.z ),
		V3( min.x, max.y, max.z ), V3( max.x, max.y, max.z ),
	};
	return new BulletConvexHullShape( pts, 8 );
}

PhyShapeHandle BulletPhyWorld::CreateTriMeshShape( const Vec3* verts, size_t vcount, const void* idata, size_t icount, bool index32 )
{
	return new BulletTriMeshShape( verts, vcount, idata, icount, index32 );
}

PhyShapeHandle BulletPhyWorld::CreateShapeFromMesh( SGRX_IMesh* mesh )
{
	int off;
	if( !mesh ||
		!mesh->m_vertexDecl ||
		mesh->m_vertexDecl->m_info.GetType( VDECLUSAGE_POSITION ) != VDECLTYPE_FLOAT3 ||
		( off = mesh->m_vertexDecl->m_info.GetOffset( VDECLUSAGE_POSITION ) ) < 0 )
		return NULL;
	size_t vtxsize = mesh->m_vertexDecl->m_info.size;
	size_t vcount = mesh->m_vdata.size() / vtxsize;
	Array< Vec3 > vdata;
	vdata.resize( vcount );
	for( size_t i = 0; i < vcount; ++i )
	{
		vdata[ i ] = *(Vec3*) &mesh->m_vdata[ i * vtxsize + off ];
	}
	
	bool i32 = !!( mesh->m_dataFlags & MDF_INDEX_32 );
	if( i32 )
	{
		Array< uint32_t > indices;
		for( size_t p = 0; p < mesh->m_meshParts.size(); ++p )
		{
			SGRX_MeshPart& MP = mesh->m_meshParts[ p ];
			for( size_t i = MP.indexOffset; i < MP.indexOffset + MP.indexCount; ++i )
				indices.push_back( *(uint32_t*) &mesh->m_idata[ i * 4 ] + MP.vertexOffset );
		}
		return CreateTriMeshShape( vdata.data(), vdata.size(), indices.data(), indices.size(), i32 );
	}
	else
	{
		Array< uint16_t > indices;
		for( size_t p = 0; p < mesh->m_meshParts.size(); ++p )
		{
			SGRX_MeshPart& MP = mesh->m_meshParts[ p ];
			for( size_t i = MP.indexOffset; i < MP.indexOffset + MP.indexCount; ++i )
				indices.push_back( *(uint16_t*) &mesh->m_idata[ i * 2 ] + MP.vertexOffset );
		}
		return CreateTriMeshShape( vdata.data(), vdata.size(), indices.data(), indices.size(), i32 );
	}
}

PhyShapeHandle BulletPhyWorld::CreateCompoundShape()
{
	return new BulletCompoundShape();
}

PhyRigidBodyHandle BulletPhyWorld::CreateRigidBody( const SGRX_PhyRigidBodyInfo& info )
{
	return new BulletPhyRigidBody( this, info );
}

PhyJointHandle BulletPhyWorld::CreateHingeJoint( const SGRX_PhyHingeJointInfo& info )
{
	return new BulletPhyHingeJoint( this, info );
}

PhyJointHandle BulletPhyWorld::CreateConeTwistJoint( const SGRX_PhyConeTwistJointInfo& info )
{
	return new BulletPhyConeTwistJoint( this, info );
}

Vec3 BulletPhyWorld::GetGravity()
{
	return BV2V( m_world->getGravity() );
}

void BulletPhyWorld::SetGravity( const Vec3& v )
{
	m_world->setGravity( V2BV( v ) );
}

bool BulletPhyWorld::Raycast( const Vec3& from, const Vec3& to, uint16_t group, uint16_t mask, SGRX_PhyRaycastInfo* outinfo )
{
	btCollisionWorld::ClosestRayResultCallback crrc( V2BV( from ), V2BV( to ) );
	crrc.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
	crrc.m_collisionFilterGroup = group;
	crrc.m_collisionFilterMask = mask;
	m_world->rayTest( crrc.m_rayFromWorld, crrc.m_rayToWorld, crrc );
	if( crrc.hasHit() && outinfo )
	{
		outinfo->factor = crrc.m_closestHitFraction;
		outinfo->normal = BV2V( crrc.m_hitNormalWorld );
		outinfo->point = BV2V( crrc.m_hitPointWorld );
		if( crrc.m_collisionObject->getInternalType() == btCollisionObject::CO_RIGID_BODY && crrc.m_collisionObject->getUserPointer() )
			outinfo->body = (SGRX_IPhyRigidBody*) crrc.m_collisionObject->getUserPointer();
	}
	return crrc.hasHit();
}

bool BulletPhyWorld::ConvexCast( const PhyShapeHandle& sh, const Vec3& from, const Vec3& to, uint16_t group, uint16_t mask, SGRX_PhyRaycastInfo* outinfo, float depth )
{
	btCollisionShape* csh = ((BulletPhyShape*) sh.item)->m_colShape;
	ASSERT( csh->isConvex() );
	btCollisionWorld::ClosestConvexResultCallback ccrc( V2BV( from ), V2BV( to ) );
	ccrc.m_collisionFilterGroup = group;
	ccrc.m_collisionFilterMask = mask;
	m_world->convexSweepTest( (btConvexShape*) csh,
		btTransform( btQuaternion::getIdentity(), ccrc.m_convexFromWorld ),
		btTransform( btQuaternion::getIdentity(), ccrc.m_convexToWorld ), ccrc, depth );
	if( ccrc.hasHit() && outinfo )
	{
		outinfo->factor = ccrc.m_closestHitFraction;
		outinfo->normal = BV2V( ccrc.m_hitNormalWorld );
		outinfo->point = BV2V( ccrc.m_hitPointWorld );
		if( ccrc.m_hitCollisionObject->getInternalType() == btCollisionObject::CO_RIGID_BODY && ccrc.m_hitCollisionObject->getUserPointer() )
			outinfo->body = (SGRX_IPhyRigidBody*) ccrc.m_hitCollisionObject->getUserPointer();
	}
	return ccrc.hasHit();
}


PhyWorldHandle PHY_CreateWorld()
{
	return new BulletPhyWorld;
}


