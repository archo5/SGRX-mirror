

#include "level.hpp"
#include "systems.hpp"
#include "entities.hpp"
#include "entities_ts.hpp"




struct ISR3Drone : Entity, SGRX_MeshInstUserData
{
	SGS_OBJECT_INHERIT( Entity );
	ENT_SGS_IMPLEMENT;
	
	struct ActionState
	{
		ActionState() : timeoutMoveToStart(0), timeoutEnding(0),
			progress(0), target(NULL){}
		
		float timeoutMoveToStart;
		float timeoutEnding;
		float progress;
		Entity* target;
		InteractInfo info;
	};
	
	ISR3Drone( GameLevel* lev, Vec3 pos, Vec3 dir );
	
	void TurnTo( const Vec2& turnDir, float speedDelta );
	Vec3 GetAimDir();
	void BeginClosestAction( float maxdist );
	bool BeginAction( Entity* E );
	bool IsInAction();
	bool CanInterruptAction();
	void InterruptAction( bool force );
	
	virtual void OnEvent( const StringView& type ){}
	void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	void Hit( float pwr );
	
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	Vec3 GetPosition(){ return m_body->GetPosition(); }
	Vec3 GetQueryPosition(){ return GetPosition() + V3(0,0,0.5f); }
	Vec3 GetInterpPos(){ return m_interpPos; }
	Vec3 GetInterpAimDir(){ return m_interpAimDir; }
	
	PhyRigidBodyHandle m_body;
	PhyShapeHandle m_shapeHandle;
	LightHandle m_shadowInst;
	
	AnimCharacter m_animChar;
	SGRX_MeshInstUserData m_meshInstInfo;
	ActionState m_actState;
	
	IVState< Vec3 > m_ivPos;
	IVState< Quat > m_ivRot;
	float m_turnAngle;
	Vec3 m_interpPos;
	Vec3 m_interpAimDir;
	
	HashTable< String, int > m_items;

	ParticleSystem m_shootPS;
	LightHandle m_shootLT;
	float m_shootTimeout;
	
	float m_health;
	float m_hitTimeout;
	
	Vec2 i_move;
	float i_speed;
	bool i_aim_at;
	Vec3 i_aim_target;
};


struct ISR3Player : ISR3Drone
{
	Vec2 inCursorMove;
	Entity* m_targetII;
	bool m_targetTriggered;
	TSAimHelper m_aimHelper;
	
	ISR3Player( GameLevel* lev, Vec3 pos, Vec3 dir );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
};


struct ISR3Enemy : ISR3Drone
{
	SGS_OBJECT_INHERIT( ISR3Drone );
	ENT_SGS_IMPLEMENT;
	
	Vec2 i_turn;
	
	SGS_PROPERTY_FUNC( READ VARNAME state ) sgsVariable m_enemyState;
	TSFactStorage m_factStorage;
	AIDBSystem* m_aidb;
	sgs_VarObj* m_scrObj;
	
	ISR3Enemy( GameLevel* lev, const StringView& name, const Vec3& pos, const Vec3& dir, sgsVariable args );
	~ISR3Enemy();
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	bool GetMapItemInfo( MapItemInfo* out );
	void DebugDrawWorld();
	void DebugDrawUI();
	
	bool HasFact( int typemask ){ return m_factStorage.HasFact( typemask ); }
	bool HasRecentFact( int typemask, TimeVal maxtime ){ return m_factStorage.HasRecentFact( typemask, maxtime ); }
	TSFactStorage::Fact* GetRecentFact( int typemask, TimeVal maxtime ){ return m_factStorage.GetRecentFact( typemask, maxtime ); }
};



struct ISR3EntityCreationSystem : IGameLevelSystem
{
	enum { e_system_uid = 1000 };
	ISR3EntityCreationSystem( GameLevel* lev );
	virtual bool AddEntity( const StringView& type, sgsVariable data );
	virtual void DrawUI();
};


