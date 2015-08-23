

#pragma once


struct TSCamera : Entity
{
	TSCamera(
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
	void SetProperty( const StringView& name, sgsVariable value );
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
	float m_moveTime;
	float m_pauseTime;
	float m_fov;
};


enum TSTaskType
{
	TT_Wait,
	TT_Turn,
	TT_Walk,
};
struct TSTask
{
	TSTaskType type;
	float timeout;
	Vec2 target;
};
typedef Array< TSTask > TSTaskArray;
void TSParseTaskArray( TSTaskArray& out, sgsVariable var );

struct TSCharacter : Entity
{
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
	
	TSCharacter( const Vec3& pos, const Vec3& dir );
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
	
	bool i_crouch;
	Vec2 i_move;
	float i_speed;
	bool i_aim_at;
	Vec3 i_aim_target;
};


#ifndef TSGAME_NO_PLAYER

struct TSPlayer : TSCharacter
{
	Vec2 m_angles;
	Vec2 inCursorMove;
	
	Entity* m_targetII;
	bool m_targetTriggered;
	
	HashTable< String, int > m_items;
	
	TextureHandle m_tex_interact_icon;
	TextureHandle m_tex_cursor;
	ParticleSystem m_shootPS;
	LightHandle m_shootLT;
	float m_shootTimeout;
	float m_crouchIconShowTimeout;
	float m_standIconShowTimeout;
	
	TSPlayer( const Vec3& pos, const Vec3& dir );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	Vec3 FindTargetPosition();
	
	bool AddItem( const StringView& item, int count );
	bool HasItem( const StringView& item, int count = 1 );
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
	void Insert( FactType type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref = 0 );
	bool Update( FactType type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0 );
	void InsertOrUpdate( FactType type, Vec3 pos, float rad,
		TimeVal created, TimeVal expires, uint32_t ref = 0 );
	bool MovingUpdate( FactType type, Vec3 pos, float movespeed,
		TimeVal created, TimeVal expires, uint32_t ref = 0 );
	void MovingInsertOrUpdate( FactType type, Vec3 pos, float movespeed,
		TimeVal created, TimeVal expires, uint32_t ref = 0 );
	
	Array< Fact > facts;
	uint32_t last_mod_id;
	uint32_t m_next_fact_id;
};

struct TSEnemy : TSCharacter
{
	TSTaskArray m_patrolTasks;
	TSTaskArray m_disturbTasks;
	float m_taskTimeout;
	int m_curTaskID;
	bool m_curTaskMode;
	String m_disturbActionName;
	
	Vec2 i_turn;
	float m_turnAngleStart;
	float m_turnAngleEnd;
	
	sgsVariable m_enemyState;
	TSFactStorage m_factStorage;
	
	TSEnemy( const StringView& name, const Vec3& pos, const Vec3& dir, sgsVariable args );
	~TSEnemy();
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void SetProperty( const StringView& name, sgsVariable value );
	void UpdateTask();
	bool GetMapItemInfo( MapItemInfo* out );
	void DebugDrawWorld();
	void DebugDrawUI();
};


