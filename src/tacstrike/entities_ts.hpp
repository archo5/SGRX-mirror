

#pragma once
#include <engext.hpp>
#include <script.hpp>

SGS_CPPBC_INHERIT_BEGIN
#include "level.hpp"
#include "systems.hpp"
SGS_CPPBC_INHERIT_END


extern CVarBool g_cv_notarget;


struct TSEventData_CharHit
{
	struct TSCharacter* ch;
	float power;
};

enum TSEventIDs
{
	_TSEV_Base_ = EID_Type_User,
	TSEV_CharHit, // userdata = TSEventData_CharHit*
	TSEV_CharDied, // userdata = TSCharacter*
};


struct TSCamera : Entity
{
	SGS_OBJECT_INHERIT( Entity );
	ENT_SGS_IMPLEMENT;
	
	TSCamera(
		GameLevel* lev,
		const StringView& name,
		const StringView& charname,
		const Vec3& pos,
		const Quat& rot,
		const Vec3& scl,
		const Vec3& dir0,
		const Vec3& dir1
	);
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	
	AnimCharacter m_animChar;
	AnimMixer::Layer m_anLayers[1];
	int m_viewAttachmentID;
	
	bool m_playerVisible;
	YawPitch m_lastSeenPlayerDir;
	YawPitch m_curDir;
	float m_timeout;
	int m_state;
	float m_alertTimeout;
	float m_noticeTimeout;
	
	YawPitch m_dir0;
	YawPitch m_dir1;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME moveTime ) float m_moveTime;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME pauseTime ) float m_pauseTime;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME fov ) float m_fov;
};


enum TSActions
{
	ACT_None = 0,
	
	ACT_Chr_Move, // v3 (x/y-dir, z-speed)
	ACT_Chr_Turn, // v3 (x/y-dir, z-speed-rad/sec)
	ACT_Chr_Crouch, // .x>0.5 => b
	ACT_Chr_AimAt, // .x>0.5 => b, y = speed (special channel)
	ACT_Chr_AimTarget, // v3
	ACT_Chr_Shoot, // .x>0.5 => b
	ACT_Chr_DoAction, // .x>0.5 => b
};


struct TSCharacter : Actor, SGRX_MeshInstUserData
{
	SGS_OBJECT_INHERIT( Actor );
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
	
	TSCharacter( GameLevel* lev );
	~TSCharacter();
	virtual void OnTransformUpdate();
	SGS_METHOD void SetPlayerMode( bool isPlayer );
	SGS_METHOD void InitializeMesh( const StringView& path );
	void ProcessAnims( float deltaTime );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void HandleMovementPhysics( float deltaTime );
	void TurnTo( const Vec2& turnDir, float speedDelta );
	void PushTo( const Vec3& pos, float speedDelta );
	void BeginClosestAction( float maxdist );
	bool BeginAction( Entity* E );
	bool IsInAction();
	bool CanInterruptAction();
	void InterruptAction( bool force );
	
	SGS_METHOD bool IsPlayingAnim() const { return m_animTimeLeft > 0; }
	SGS_METHOD void PlayAnim( StringView name, bool loop );
	SGS_METHOD void StopAnim();
	
	virtual bool IsAlive(){ return m_health > 0; }
	virtual void Reset();
	
	void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	void Hit( float pwr );
	virtual void OnDeath();
	
	SGS_METHOD Vec3 GetQueryPosition_FT(){ return GetPosition_FT() + V3(0,0,0.5f); }
	SGS_METHOD Vec3 GetPosition_FT(){ return m_bodyHandle->GetPosition(); }
	SGS_METHOD Vec3 GetViewDir_FT(){ return V3( cosf( m_turnAngle ), sinf( m_turnAngle ), 0 ); }
	SGS_METHOD Vec3 GetAimDir_FT(){ return m_aimDir.ToVec3(); }
	Mat4 GetBulletOutputMatrix();
	
	SGS_METHOD Vec3 GetQueryPosition(){ return GetWorldPosition() + V3(0,0,0.5f); }
	SGS_METHOD Vec3 GetAimDir(){ return m_interpAimDir; }
	
	PhyRigidBodyHandle m_bodyHandle;
	PhyShapeHandle m_shapeHandle;
	LightHandle m_shadowInst;
	
	AnimCharacter m_animChar;
	AnimPlayer m_anMainPlayer;
	AnimPlayer m_anTopPlayer;
	AnimMixer::Layer m_anLayers[4];
	
	SGS_PROPERTY_FUNC( READ WRITE VARNAME health ) float m_health;
	float m_armor;
	
	float m_footstepTime;
	bool m_isCrouching;
	bool m_isOnGround;
	IVState< Vec3 > m_ivPos;
	IVState< Vec3 > m_ivAimDir;
	Vec3 m_interpPos;
	Vec3 m_interpAimDir;
	
	float m_turnAngle;
	YawPitch m_aimDir;
	float m_aimDist;
	
	ActionState m_actState;
	uint32_t m_infoFlags;
	float m_animTimeLeft;
	
	ParticleSystem m_shootPS;
	LightHandle m_shootLT;
	float m_shootTimeout;
	SGS_PROPERTY_FUNC( READ VARNAME timeSinceLastHit ) float m_timeSinceLastHit;
	void SetViewDir( Vec3 v ){ m_turnAngle = v.ToVec2().Normalized().Angle(); }
	SGS_PROPERTY_FUNC( WRITE SetViewDir ) SGS_ALIAS( Vec3 viewDir );
	void SetFootPosition( Vec3 p ){ SetWorldPosition( p + V3(0,0,1.5f) ); }
	SGS_PROPERTY_FUNC( WRITE SetFootPosition ) SGS_ALIAS( Vec3 footPosition );
	bool m_skipTransformUpdate;
	
	SGS_METHOD_NAMED( GetAttachmentPos ) Vec3 sgsGetAttachmentPos( StringView atch, Vec3 off );
};


struct TSScriptedController : IActorController
{
	SGS_OBJECT_INHERIT( IActorController );
	SGS_BACKING_STORE( _data.var );
	SGS_BACKING_STORE( _backing.var );
	
	TSScriptedController( GameLevel* lev );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	Vec3 GetInput( uint32_t iid );
	void Reset();
	
	SGS_PROPERTY sgsVariable _data;
	SGS_PROPERTY sgsVariable _backing;
	SGS_PROPERTY_FUNC( READ VARNAME level ) GameLevel* m_level;
	
	SGS_STATICMETHOD sgsVariable Create( SGS_CTX, GameLevelScrHandle lev );
};


struct TSAimHelper : EntityProcessor
{
	TSAimHelper( GameLevel* lev );
	void Tick( float deltaTime, Vec3 pos, Vec2 cp, bool lock );
	Vec3 GetAimPoint();
	
	Vec3 _CalcRCPos( Vec3 pos );
	
	GameLevel* m_level;
	Vec3 m_pos;
	Vec2 m_cp;
	void* m_aimPtr;
	Vec3 m_aimPoint;
	Vec3 m_rcPoint;
	float m_aimFactor;
	
	// lock target query
	virtual bool ProcessEntity( Entity* E );
	float m_pDist;
	void* m_closestEnt;
	Vec3 m_closestPoint;
};


struct TSPlayerController : IActorController
{
	SGS_OBJECT_INHERIT( IActorController );
	
	TSAimHelper m_aimHelper;
	Vec2 i_move;
	Vec3 i_aim_target;
	Vec3 i_turn;
	
	TSPlayerController( GameLevel* lev );
	void Tick( float deltaTime, float blendFactor );
	virtual Vec3 GetInput( uint32_t iid );
	
	SGS_METHOD void CalcUIAimInfo();
	bool _shouldDrawCP() const { return m_aimHelper.m_aimPtr == NULL && m_aimHelper.m_closestEnt; }
	SGS_PROPERTY_FUNC( READ _shouldDrawCP ) SGS_ALIAS( bool ahShouldDrawClosestPoint );
	SGS_PROPERTY_FUNC( READ SOURCE m_aimHelper.m_closestPoint ) SGS_ALIAS( Vec3 ahClosestPoint );
	SGS_PROPERTY_FUNC( READ SOURCE m_aimHelper.m_aimPoint ) SGS_ALIAS( Vec3 ahAimPoint );
	SGS_PROPERTY_FUNC( READ SOURCE m_aimHelper.m_aimFactor ) SGS_ALIAS( float ahAimFactor );
	SGS_PROPERTY_FUNC( READ SOURCE m_aimHelper.m_pDist ) SGS_ALIAS( float ahCPDistance );
	
	SGS_STATICMETHOD sgsVariable Create( SGS_CTX, GameLevelScrHandle lev );
};


struct TPSPlayerController : IActorController
{
	SGS_OBJECT_INHERIT( IActorController );
	
	GameLevel* m_level;
	YawPitch m_angles;
	SGS_PROPERTY_FUNC( READ GetCameraPos ) SGS_ALIAS( Vec3 cameraPos );
	Vec2 i_move;
	Vec3 i_aim_target;
	Vec3 i_turn;
	
	PhyShapeHandle m_castShape;
	
	TPSPlayerController( GameLevel* lev );
	void Tick( float deltaTime, float blendFactor );
	virtual Vec3 GetInput( uint32_t iid );
	void SafePosPush( Vec3& pos, Vec3 dir );
	SGS_METHOD Vec3 GetCameraPos();
	TSCharacter* GetChar()
	{
		return m_entity && ENTITY_IS_A( m_entity, TSCharacter )
			? (TSCharacter*) m_entity
			: NULL;
	}
	
	SGS_PROPERTY_FUNC( READ SOURCE m_angles.ToVec3() ) SGS_ALIAS( Vec3 direction );
	SGS_STATICMETHOD sgsVariable Create( SGS_CTX, GameLevelScrHandle lev );
};


struct TSEnemyController : IActorController
{
	SGS_OBJECT_INHERIT( IActorController );
	
	GameLevel* m_level;
	bool i_crouch;
	Vec2 i_move;
	float i_speed;
	Vec3 i_turn;
	bool i_aim_at;
	Vec3 i_aim_target;
	bool i_shoot;
	bool i_act;
	
	SGS_PROPERTY_FUNC( READ WRITE VARNAME inPlayerTeam ) bool m_inPlayerTeam;
	SGS_PROPERTY sgsVariable _data;
	SGS_BACKING_STORE( _data.var );
	
	AIFactStorage m_factStorage;
	CSCoverInfo m_coverInfo;
	Array< Vec3 > m_path;
	AIDBSystem* m_aidb;
	CoverSystem* m_coverSys;
	TSCharacter* GetChar()
	{
		return m_entity && ENTITY_IS_A( m_entity, TSCharacter )
			? (TSCharacter*) m_entity
			: NULL;
	}
	
	TSEnemyController( GameLevel* lev );
	~TSEnemyController();
	virtual void FixedTick( float deltaTime );
	virtual void Tick( float deltaTime, float blendFactor );
	virtual Vec3 GetInput( uint32_t iid );
	virtual void Reset();
	void DebugDrawWorld();
	void DebugDrawUI();
	
	// core queries
	SGS_METHOD bool CanSeePoint( Vec3 pt );
	SGS_METHOD bool LookingAtPoint( Vec3 pt );
	
	// fact storage
	SGS_METHOD_NAMED( HasFact ) bool sgsHasFact( uint32_t typemask );
	SGS_METHOD_NAMED( HasRecentFact ) bool sgsHasRecentFact( uint32_t typemask, TimeVal maxtime );
	SGS_METHOD_NAMED( GetRecentFact ) SGS_MULTRET sgsGetRecentFact( uint32_t typemask, TimeVal maxtime );
	SGS_METHOD_NAMED( InsertFact ) void sgsInsertFact( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref );
	SGS_METHOD_NAMED( UpdateFact ) bool sgsUpdateFact( uint32_t type, Vec3 pos,
		float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	SGS_METHOD_NAMED( InsertOrUpdateFact ) void sgsInsertOrUpdateFact( uint32_t type, Vec3 pos,
		float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	
	// cover info
	SGS_METHOD_NAMED( QueryCoverLines ) void sgsQueryCoverLines( Vec3 bbmin,
		Vec3 bbmax, float dist, float height, Vec3 viewer, bool visible );
	SGS_METHOD_NAMED( GetCoverPosition ) sgsMaybe<Vec3> sgsGetCoverPosition(
		Vec3 position, float distpow, float interval /* = 0.1 */ );
	
	// pathfinding
	SGS_METHOD_NAMED( IsWalkable ) bool sgsIsWalkable( Vec3 pos, Vec3 ext );
	SGS_METHOD_NAMED( FindPath ) bool sgsFindPath( const Vec3& to );
	SGS_METHOD_NAMED( HasPath ) bool sgsHasPath();
	SGS_METHOD_NAMED( GetPathPointCount ) int sgsGetPathPointCount();
	SGS_METHOD_NAMED( AdvancePath ) bool sgsAdvancePath( float dist );
	SGS_METHOD_NAMED( GetNextPathPoint ) sgsMaybe<Vec3> sgsGetNextPathPoint();
	SGS_METHOD_NAMED( RemoveNextPathPoint ) bool sgsRemoveNextPathPoint();
	
	SGS_STATICMETHOD sgsVariable Create( SGS_CTX, GameLevelScrHandle lev );
};



struct TSGameSystem : IGameLevelSystem
{
	enum { e_system_uid = 1000 };
	TSGameSystem( GameLevel* lev );
	virtual Entity* AddEntity( StringView type );
};


