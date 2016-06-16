

#pragma once
#include <engext.hpp>
#include <script.hpp>

SGS_CPPBC_INHERIT_BEGIN
#include "level.hpp"
#include "systems.hpp"
#include "resources.hpp"
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


#if 0
struct TSCamera : GOBehavior
{
	SGS_OBJECT_INHERIT( GOBehavior );
	ENT_SGS_IMPLEMENT;
	
	TSCamera(
		GameObject* obj,
		const StringView& name,
		const StringView& charname,
		const Vec3& pos,
		const Quat& rot,
		const Vec3& scl,
		const Vec3& dir0,
		const Vec3& dir1
	);
	void FixedUpdate();
	void Update();
	
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
#endif


enum TSActions
{
	ACT_None = 0,
	
	ACT_Chr_Move, // v3 (x/y-dir, z-speed)
	ACT_Chr_Turn, // v3 (x/y-dir, z-speed-rad/sec)
	ACT_Chr_Crouch, // .x>0.5 => b
	ACT_Chr_Jump, // .x>0.5 => b
	ACT_Chr_AimAt, // .x>0.5 => b, y = speed (special channel)
	ACT_Chr_AimTarget, // v3
	ACT_Chr_Shoot, // .x>0.5 => b
	ACT_Chr_DoAction, // .x>0.5 => b
	ACT_Chr_ReloadHolsterDrop, // .x/.y/.z>0.5 => b1/b2/b3
};


struct TSCharacter : GOBehavior, SGRX_MeshInstUserData
{
	SGS_OBJECT_INHERIT( GOBehavior );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_BEHAVIOR( TSCharacter );
	
	struct ActionState
	{
		ActionState() : timeoutMoveToStart(0), timeoutEnding(0),
			progress(0), target(NULL){}
		
		float timeoutMoveToStart;
		float timeoutEnding;
		float progress;
		GameObject* target;
		InteractInfo info;
	};
	
	TSCharacter( GameObject* obj );
	~TSCharacter();
	virtual void OnTransformUpdate();
	SGS_METHOD void SetPlayerMode( bool isPlayer );
	SGS_METHOD void InitializeMesh( const StringView& path );
	void ProcessAnims( float deltaTime );
	void FixedUpdate();
	void Update();
	void _HandleGroundBody( Vec3& pos, SGRX_IPhyRigidBody* body, float dt );
	void HandleMovementPhysics( float deltaTime );
	void TurnTo( const Vec2& turnDir, float speedDelta );
	void PushTo( const Vec3& pos, float speedDelta );
	void BeginClosestAction( float maxdist );
	bool BeginAction( GameObject* obj );
	bool IsInAction();
	bool CanInterruptAction();
	void InterruptAction( bool force );
	
	SGS_METHOD bool IsTouchingPoint( Vec3 p, float hmargin, float vmargin ) const;
	Vec3 GetMoveRefPos() const;
	SGS_PROPERTY_FUNC( READ GetMoveRefPos ) SGS_ALIAS( Vec3 moveRefPos );
	
	SGS_METHOD bool IsPlayingAnim() const { return m_animTimeLeft > 0; }
	SGS_METHOD void PlayAnim( StringView name, bool loop );
	SGS_METHOD void StopAnim();
	// - specific animation triggers
	SGS_METHOD void PlayPickupAnim( Vec3 tgt );
	
	virtual bool IsAlive(){ return m_health > 0; }
	virtual void Reset();
	
	void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	SGS_METHOD void Hit( float pwr );
	virtual void OnDeath();
	
	SGS_METHOD Vec3 GetQueryPosition_FT() const { return m_bodyHandle->GetPosition(); }
	SGS_METHOD Vec3 GetPosition_FT() const { return m_ivPos.curr; }
	SGS_METHOD Vec3 GetViewDir_FT() const { return V3( cosf( m_turnAngle ), sinf( m_turnAngle ), 0 ); }
	SGS_METHOD Vec3 GetAimDir_FT() const { return m_aimDir.ToVec3(); }
	GOBehavior* FindWeapon() const;
	sgsVariable sgsCurWeapon() const { GOBehavior* w = FindWeapon(); return w ? w->GetScriptedObject() : sgsVariable(); }
	SGS_PROPERTY_FUNC( READ sgsCurWeapon ) SGS_ALIAS( sgsVariable curWeapon );
	Mat4 GetBulletOutputMatrix() const;
	
	SGS_METHOD Vec3 GetWorldPosition() const { return m_obj->GetWorldPosition(); }
	SGS_METHOD Vec3 GetQueryPosition() const { return m_obj->GetWorldPosition() + V3(0,0,0.5f); }
	SGS_METHOD Vec3 GetAimDir() const { return m_interpAimDir; }
	
	PhyRigidBodyHandle m_bodyHandle;
	PhyShapeHandle m_shapeHandle;
	LightHandle m_shadowInst;
	
	AnimCharacter m_animChar;
	AnimPlayer m_anMainPlayer;
	
	SGS_PROPERTY_FUNC( READ WRITE VARNAME health ) float m_health;
	float m_armor;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME damageMultiplier ) float m_damageMultiplier;
	
	float m_footstepTime;
	SGS_PROPERTY_FUNC( READ VARNAME isCrouching ) bool m_isCrouching;
	bool m_isOnGround;
	float m_jumpTimeout;
	float m_canJumpTimeout;
	Vec2 m_cachedBodyExtOffset;
	
	// if current body matches this pointer, relative position to body is preserved
	// {
	void* m_groundBody;
	Vec3 m_groundLocalPos;
	Vec3 m_groundWorldPos;
	Vec3 m_groundVelocity;
	// }
	
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
	
	bool m_pickupTrigger;
	
	ParticleSystem m_shootPS;
	LightHandle m_shootLT;
	float m_shootTimeout;
	SGS_PROPERTY_FUNC( READ VARNAME timeSinceLastHit ) float m_timeSinceLastHit;
	void SetViewDir( Vec3 v ){ m_turnAngle = v.ToVec2().Normalized().Angle(); }
	SGS_PROPERTY_FUNC( WRITE SetViewDir ) SGS_ALIAS( Vec3 viewDir );
	void SetFootPosition( Vec3 p ){ m_obj->SetWorldPosition( p + V3(0,0,1.5f) ); }
	SGS_PROPERTY_FUNC( WRITE SetFootPosition ) SGS_ALIAS( Vec3 footPosition );
	bool m_skipTransformUpdate;
	
	SGS_METHOD_NAMED( GetAttachmentPos ) Vec3 sgsGetAttachmentPos( StringView atch, Vec3 off );
	SGS_METHOD_NAMED( GetAttachmentMatrix ) Mat4 sgsGetAttachmentMatrix( StringView atch );
};


struct TSAimHelper : GameObjectProcessor
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
	virtual bool ProcessGameObject( GameObject* obj );
	float m_pDist;
	void* m_closestObj;
	Vec3 m_closestPoint;
};


struct TSPlayerController : BhControllerBase
{
	SGS_OBJECT_INHERIT( BhControllerBase );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_BEHAVIOR( TSPlayerController );
	
	TSAimHelper m_aimHelper;
	Vec2 i_move;
	Vec3 i_aim_target;
	Vec3 i_turn;
	
	TSPlayerController( GameObject* obj );
	virtual void Update();
	virtual Vec3 GetInput( uint32_t iid );
	
	SGS_METHOD void CalcUIAimInfo();
	bool _shouldDrawCP() const { return m_aimHelper.m_aimPtr == NULL && m_aimHelper.m_closestObj; }
	SGS_PROPERTY_FUNC( READ _shouldDrawCP ) SGS_ALIAS( bool ahShouldDrawClosestPoint );
	SGS_PROPERTY_FUNC( READ SOURCE m_aimHelper.m_closestPoint ) SGS_ALIAS( Vec3 ahClosestPoint );
	SGS_PROPERTY_FUNC( READ SOURCE m_aimHelper.m_aimPoint ) SGS_ALIAS( Vec3 ahAimPoint );
	SGS_PROPERTY_FUNC( READ SOURCE m_aimHelper.m_aimFactor ) SGS_ALIAS( float ahAimFactor );
	SGS_PROPERTY_FUNC( READ SOURCE m_aimHelper.m_pDist ) SGS_ALIAS( float ahCPDistance );
};


struct TPSPlayerController : BhControllerBase
{
	SGS_OBJECT_INHERIT( BhControllerBase );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_BEHAVIOR( TPSPlayerController );
	
	YawPitch m_angles;
	Vec2 i_move;
	Vec3 i_aim_target;
	Vec3 i_turn;
	bool i_crouch;
	
	PhyShapeHandle m_castShape;
	SGS_PROPERTY bool lastFrameReset;
	
	TPSPlayerController( GameObject* obj );
	virtual SGS_METHOD void Update();
	virtual Vec3 GetInput( uint32_t iid );
	void SafePosPush( Vec3& pos, Vec3 dir );
	SGS_METHOD Vec3 GetCameraPos( TSCharacter* chr, bool tick );
	SGS_METHOD void UpdateMoveAim( bool tick );
	TSCharacter* GetChar()
	{
		return m_obj->FindBehaviorOfType<TSCharacter>();
	}
	
	void SetDir( Vec3 v ){ m_angles = YP(v); }
	SGS_PROPERTY_FUNC( READ WRITE SetDir SOURCE m_angles.ToVec3() ) SGS_ALIAS( Vec3 direction );
};


struct TSEnemyController : BhControllerBase
{
	SGS_OBJECT_INHERIT( BhControllerBase );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_BEHAVIOR( TSEnemyController );
	
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
		return m_obj->FindBehaviorOfType<TSCharacter>();
	}
	
	TSEnemyController( GameObject* obj );
	~TSEnemyController();
	virtual void FixedUpdate();
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
};



struct TSGameSystem : IGameLevelSystem
{
	enum { e_system_uid = 1000 };
	TSGameSystem( GameLevel* lev );
};


