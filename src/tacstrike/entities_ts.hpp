

#pragma once
#include <engext.hpp>

#include "level.hpp"
#include "systems.hpp"


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


struct TSCharacter : SGRX_Actor, SGRX_MeshInstUserData
{
	SGS_OBJECT_INHERIT( SGRX_Actor );
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
	
	TSCharacter( GameLevel* lev, const Vec3& pos, const Vec3& dir );
	void InitializeMesh( const StringView& path );
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
	
	virtual bool IsAlive(){ return m_health > 0; }
	virtual void Reset();
	Vec3 GetPosition();
	void SetPosition( Vec3 pos );
	
	virtual void OnEvent( const StringView& type ){}
	void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	void Hit( float pwr );
	virtual void OnDeath();
	
	Vec3 GetQueryPosition();
	Vec3 GetViewDir();
	Vec3 GetAimDir();
	Mat4 GetBulletOutputMatrix();
	
	Vec3 GetInterpPos();
	Vec3 GetInterpAimDir();
	
	PhyRigidBodyHandle m_bodyHandle;
	PhyShapeHandle m_shapeHandle;
	LightHandle m_shadowInst;
	
	AnimCharacter m_animChar;
	AnimPlayer m_anMainPlayer;
	AnimPlayer m_anTopPlayer;
	AnimMixer::Layer m_anLayers[4];
	
	float m_health;
	float m_armor;
	
	float m_footstepTime;
	bool m_isCrouching;
	bool m_isOnGround;
	IVState< Vec3 > m_ivPos;
	IVState< Vec3 > m_ivAimDir;
	Vec3 m_interpPos;
	Vec3 m_interpAimDir;
	
	Vec3 m_position;
	Vec2 m_moveDir;
	float m_turnAngle;
	YawPitch m_aimDir;
	float m_aimDist;
	
	ActionState m_actState;
	uint32_t m_infoFlags;
	
	ParticleSystem m_shootPS;
	LightHandle m_shootLT;
	float m_shootTimeout;
	SGS_PROPERTY_FUNC( READ VARNAME timeSinceLastHit ) float m_timeSinceLastHit;
	
	SGS_PROPERTY_FUNC( READ GetPosition ) SGS_ALIAS( Vec3 position );
	SGS_METHOD_NAMED( GetAttachmentPos ) Vec3 sgsGetAttachmentPos( StringView atch, Vec3 off );
};


struct TSAimHelper : InfoEmissionSystem::IESProcessor
{
	TSAimHelper( GameLevel* lev );
	void Tick( float deltaTime, Vec3 pos, Vec2 cp, bool lock );
	void DrawUI();
	Vec3 GetAimPoint();
	
	Vec3 _CalcRCPos( Vec3 pos );
	
	GameLevel* m_level;
	TextureHandle m_tex_cursor;
	Vec3 m_pos;
	Vec2 m_cp;
	void* m_aimPtr;
	Vec3 m_aimPoint;
	Vec3 m_rcPoint;
	float m_aimFactor;
	
	// lock target query
	virtual bool Process( Entity* E, const InfoEmissionSystem::Data& D );
	float m_pDist;
	void* m_closestEnt;
	Vec3 m_closestPoint;
};


#ifndef TSGAME_NO_PLAYER

struct TSPlayerController : SGRX_IActorController
{
	TSAimHelper m_aimHelper;
	Vec2 i_move;
	Vec3 i_aim_target;
	Vec3 i_turn;
	
	TSPlayerController( GameLevel* lev );
	void Tick( float deltaTime, float blendFactor );
	virtual Vec3 GetInput( uint32_t iid );
};

#endif


struct TSEnemyController : SGRX_IActorController
{
	SGS_OBJECT;
	
	bool i_crouch;
	Vec2 i_move;
	float i_speed;
	Vec3 i_turn;
	bool i_aim_at;
	Vec3 i_aim_target;
	bool i_shoot;
	bool i_act;
	
	SGS_PROPERTY_FUNC( READ VARNAME state ) sgsVariable m_enemyState;
	GameLevel* m_level;
	AIFactStorage m_factStorage;
	CSCoverInfo m_coverInfo;
	Array< Vec3 > m_path;
	AIDBSystem* m_aidb;
	CoverSystem* m_coverSys;
	TSCharacter* m_char;
	
	TSEnemyController( GameLevel* lev, TSCharacter* chr, sgsVariable args );
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
	SGS_METHOD_NAMED( GetRecentFact ) SGS_MULTRET sgsGetRecentFact( sgs_Context* coro, uint32_t typemask, TimeVal maxtime );
	SGS_METHOD_NAMED( InsertFact ) void sgsInsertFact( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref );
	SGS_METHOD_NAMED( UpdateFact ) bool sgsUpdateFact( sgs_Context* coro, uint32_t type, Vec3 pos,
		float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	SGS_METHOD_NAMED( InsertOrUpdateFact ) void sgsInsertOrUpdateFact( sgs_Context* coro, uint32_t type, Vec3 pos,
		float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset );
	
	// cover info
	SGS_METHOD_NAMED( QueryCoverLines ) void sgsQueryCoverLines( Vec3 bbmin,
		Vec3 bbmax, float dist, float height, Vec3 viewer, bool visible );
	SGS_METHOD_NAMED( GetCoverPosition ) sgsMaybe<Vec3> sgsGetCoverPosition(
		sgs_Context* coro, Vec3 position, float distpow, float interval /* = 0.1 */ );
	
	// pathfinding
	SGS_METHOD_NAMED( IsWalkable ) bool sgsIsWalkable( Vec3 pos, Vec3 ext );
	SGS_METHOD_NAMED( FindPath ) bool sgsFindPath( const Vec3& to );
	SGS_METHOD_NAMED( GetNextPathPoint ) sgsMaybe<Vec3> sgsGetNextPathPoint();
	SGS_METHOD_NAMED( RemoveNextPathPoint ) bool sgsRemoveNextPathPoint();
};



struct TSGameSystem : IGameLevelSystem
{
	enum { e_system_uid = 1000 };
	TSGameSystem( GameLevel* lev );
	virtual bool AddEntity( const StringView& type, sgsVariable data, sgsVariable& outvar );
	virtual void Tick( float deltaTime, float blendFactor );
	virtual void DrawUI();
	
#ifndef TSGAME_NO_PLAYER
	TSPlayerController m_playerCtrl;
	float m_crouchIconShowTimeout;
	float m_standIconShowTimeout;
	float m_prevCrouchValue;
#endif
};


