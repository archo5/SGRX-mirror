

#pragma once
#include <engext.hpp>

#include "level.hpp"
#include "systems.hpp"


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
	bool GetMapItemInfo( MapItemInfo* out );
	
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
	ACT_Chr_Crouch, // .x>0.5 => b
	ACT_Chr_AimAt, // .x>0.5 => b
	ACT_Chr_AimTarget, // v3
};


struct TSCharacter : SGRX_Actor
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
	
	Vec3 GetQueryPosition();
	Vec3 GetPosition();
	Vec3 GetViewDir();
	Vec3 GetAimDir();
	Mat4 GetBulletOutputMatrix();
	
	Vec3 GetInterpPos();
	Vec3 GetInterpAimDir();
	
	PhyRigidBodyHandle m_bodyHandle;
	PhyShapeHandle m_shapeHandle;
	LightHandle m_shadowInst;
	
	AnimCharacter m_animChar;
	SGRX_MeshInstUserData m_meshInstInfo;
	AnimPlayer m_anMainPlayer;
	AnimPlayer m_anTopPlayer;
	AnimMixer::Layer m_anLayers[4];
	
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
	
	ActionState m_actState;
	
	ParticleSystem m_shootPS;
	LightHandle m_shootLT;
	float m_shootTimeout;
	
//	bool i_crouch;
//	Vec2 i_move;
//	float i_speed;
//	bool i_aim_at;
//	Vec3 i_aim_target;
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
	
	TSPlayerController( GameLevel* lev ) : m_aimHelper( lev ){}
	virtual Vec3 GetInput( uint32_t iid );
};

struct TSPlayer : TSCharacter
{
	TSAimHelper m_aimHelper;
	
	TSPlayer( GameLevel* lev, const Vec3& pos, const Vec3& dir );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
};

#endif


struct TSFactStorage
{
	enum FactType
	{
		FT_Unknown = 0,
		FT_Sound_Noise,
		FT_Sound_Footstep,
		FT_Sound_Shot,
		FT_Sight_ObjectState,
		FT_Sight_Alarming,
		FT_Sight_Friend,
		FT_Sight_Foe,
		FT_Position_Friend,
		FT_Position_Foe,
	};
	
	struct Fact
	{
		uint32_t id;
		uint32_t ref;
		FactType type;
		Vec3 position;
		TimeVal created;
		TimeVal expires;
	};
	
	TSFactStorage();
	void SortCreatedDesc();
	void Process( TimeVal curTime );
	bool HasFact( int typemask );
	bool HasRecentFact( int typemask, TimeVal maxtime );
	Fact* GetRecentFact( int typemask, TimeVal maxtime );
	void Insert( FactType type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref = 0 );
	bool Update( FactType type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	void InsertOrUpdate( FactType type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	bool MovingUpdate( FactType type, Vec3 pos, float movespeed,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	void MovingInsertOrUpdate( FactType type, Vec3 pos, float movespeed,
		TimeVal created, TimeVal expires, uint32_t ref = 0, bool reset = true );
	
	Array< Fact > facts;
	TimeVal m_lastTime;
	uint32_t last_mod_id;
	uint32_t m_next_fact_id;
};

struct TSEnemy : TSCharacter
{
	SGS_OBJECT_INHERIT( TSCharacter );
	ENT_SGS_IMPLEMENT;
	
	Vec2 i_turn;
	
	SGS_PROPERTY_FUNC( READ VARNAME state ) sgsVariable m_enemyState;
	TSFactStorage m_factStorage;
	AIDBSystem* m_aidb;
	
	TSEnemy( GameLevel* lev, const StringView& name, const Vec3& pos, const Vec3& dir, sgsVariable args );
	~TSEnemy();
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	bool GetMapItemInfo( MapItemInfo* out );
	void DebugDrawWorld();
	void DebugDrawUI();
	
	bool HasFact( int typemask ){ return m_factStorage.HasFact( typemask ); }
	bool HasRecentFact( int typemask, TimeVal maxtime ){ return m_factStorage.HasRecentFact( typemask, maxtime ); }
	TSFactStorage::Fact* GetRecentFact( int typemask, TimeVal maxtime ){ return m_factStorage.GetRecentFact( typemask, maxtime ); }
};



struct TSGameSystem : IGameLevelSystem
{
	enum { e_system_uid = 1000 };
	TSGameSystem( GameLevel* lev );
	virtual bool AddEntity( const StringView& type, sgsVariable data );
	virtual void Tick( float deltaTime, float blendFactor );
	virtual void DrawUI();
	
#ifndef TSGAME_NO_PLAYER
	TSPlayerController m_playerCtrl;
	float m_crouchIconShowTimeout;
	float m_standIconShowTimeout;
	float m_prevCrouchValue;
#endif
};


