

#define USE_QUAT
#define USE_ARRAY
#include "../ext/src/bullet.hpp"

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
	
	btCollisionShape* m_colShape;
};

struct BulletConvexHullShape : BulletPhyShape
{
	BulletConvexHullShape( const Vec3* data, size_t count ){ m_colShape = new btConvexHullShape( &data->x, count, sizeof(Vec3) ); _Init(); }
};

struct BulletTriMeshShape : BulletPhyShape
{
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


struct BulletPhyRigidBody : SGRX_IPhyRigidBody
{
	BulletPhyRigidBody( struct BulletPhyWorld* world, const SGRX_PhyRigidBodyInfo& rbinfo );
	~BulletPhyRigidBody();
	
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
	
	struct BulletPhyWorld* m_world;
	btRigidBody* m_body;
	PhyShapeHandle m_shape;
};


struct BulletPhyWorld : SGRX_IPhyWorld
{
	BulletPhyWorld();
	~BulletPhyWorld();
	
	virtual void Step( float dt );
	virtual void DebugDraw();
	
	virtual PhyRigidBodyHandle CreateRigidBody( const SGRX_PhyRigidBodyInfo& info );
	
	virtual Vec3 GetGravity();
	virtual void SetGravity( const Vec3& v );
	
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
	: m_world( world )
{
	btRBCI rbci( 0, NULL, NULL );
	RBInfo2RBCI( rbinfo, rbci );
	
	rbci.m_motionState = new btDefaultMotionState( rbci.m_startWorldTransform );
	m_body = new btRigidBody( rbci );
	if( rbinfo.kinematic )
	{
		m_body->setCollisionFlags( m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT );
	}
	if( !rbinfo.canSleep )
	{
		m_body->setActivationState( DISABLE_DEACTIVATION );
	}
	m_body->setUserPointer( this );
	m_shape = rbinfo.shape;
	m_world->m_world->addRigidBody( m_body );
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

PhyRigidBodyHandle BulletPhyWorld::CreateRigidBody( const SGRX_PhyRigidBodyInfo& info )
{
	return new BulletPhyRigidBody( this, info );
}

Vec3 BulletPhyWorld::GetGravity()
{
	return BV2V( m_world->getGravity() );
}

void BulletPhyWorld::SetGravity( const Vec3& v )
{
	m_world->setGravity( V2BV( v ) );
}

PhyShapeHandle PHY_CreateConvexHullShape( const Vec3* data, size_t count )
{
	return new BulletConvexHullShape( data, count );
}

PhyShapeHandle PHY_CreateAABBShape( const Vec3& min, const Vec3& max )
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

PhyShapeHandle PHY_CreateTriMeshShape( const Vec3* verts, size_t vcount, const void* idata, size_t icount, bool index32 )
{
	return new BulletTriMeshShape( verts, vcount, idata, icount, index32 );
}

PhyShapeHandle PHY_CreateShapeFromMesh( SGRX_IMesh* mesh )
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
		for( int p = 0; p < mesh->m_numParts; ++p )
		{
			SGRX_MeshPart& MP = mesh->m_parts[ p ];
			for( size_t i = MP.indexOffset; i < MP.indexOffset + MP.indexCount; ++i )
				indices.push_back( *(uint32_t*) &mesh->m_idata[ i * 4 ] + MP.vertexOffset );
		}
		return PHY_CreateTriMeshShape( vdata.data(), vdata.size(), indices.data(), indices.size(), i32 );
	}
	else
	{
		Array< uint16_t > indices;
		for( int p = 0; p < mesh->m_numParts; ++p )
		{
			SGRX_MeshPart& MP = mesh->m_parts[ p ];
			for( size_t i = MP.indexOffset; i < MP.indexOffset + MP.indexCount; ++i )
				indices.push_back( *(uint16_t*) &mesh->m_idata[ i * 2 ] + MP.vertexOffset );
		}
		return PHY_CreateTriMeshShape( vdata.data(), vdata.size(), indices.data(), indices.size(), i32 );
	}
}

PhyWorldHandle PHY_CreateWorld()
{
	return new BulletPhyWorld;
}
