

#include "level.hpp"
#include "systems.hpp"
#include "entities.hpp"
#include "entities_ts.hpp"




struct ISR3Drone : Entity, SGRX_MeshInstUserData
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
	
	ISR3Drone( GameLevel* lev, Vec3 pos, Vec3 dir );
	
	void TurnTo( const Vec2& turnDir, float speedDelta );
	Vec3 GetAimDir();
	void BeginClosestAction( float maxdist );
	bool BeginAction( Entity* E );
	bool IsInAction();
	bool CanInterruptAction();
	void InterruptAction( bool force );
	
	void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	void Hit( float pwr );
	
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	Vec3 GetPosition(){ return m_body->GetPosition(); }
	Vec3 GetQueryPosition(){ return GetPosition() + V3(0,0,0.5f); }
	
	PhyRigidBodyHandle m_body;
	PhyShapeHandle m_shapeHandle;
	LightHandle m_shadowInst;
	
	AnimCharacter m_animChar;
	SGRX_MeshInstUserData m_meshInstInfo;
	ActionState m_actState;
	
	IVState< Vec3 > m_ivPos;
	IVState< Quat > m_ivRot;
	float m_turnAngle;
	
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