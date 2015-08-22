

#pragma once


struct LD33Player : Entity
{
	PhyRigidBodyHandle m_bodyHandle;
	PhyShapeHandle m_shapeHandle;
	
	Vec2 m_angles;
	float m_jumpTimeout;
	float m_canJumpTimeout;
	float m_footstepTime;
	bool m_isOnGround;
	float m_isCrouching;
	
	IVState< Vec3 > m_ivPos;
	
	Vec2 inCursorMove;
	
	Entity* m_targetII;
	bool m_targetTriggered;
	
	HashTable< String, int > m_items;
	
	TextureHandle m_tex_interact_icon;
	
	LD33Player( const Vec3& pos, const Vec3& dir );
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	bool AddItem( const StringView& item, int count );
	bool HasItem( const StringView& item, int count = 1 );
	
	Vec3 GetPosition() const { return m_bodyHandle->GetPosition(); }
};


