

#include "level.hpp"
#include "tsui.hpp"
#include "entities.hpp"
#include "entities_ts.hpp"



struct TSFightGameMode : IGameLevelSystem, SGRX_IEventHandler
{
	enum GameState
	{
		GS_Playing,
		GS_Intro,
		GS_Ending,
		GS_TEST,
		GS_TEST2,
	};
	enum { e_system_uid = 1000 };
	TSFightGameMode( GameLevel* lev );
	~TSFightGameMode();
	void OnPostLevelLoad();
	bool AddEntity( const StringView& type, sgsVariable data );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	void HandleEvent( SGRX_EventID eid, const EventData& edata );
	
	Vec3 PickFurthestSpawnPoint( Vec3 from );
	
	GameState m_state;
	float m_timeout;
	int m_points_ply;
	int m_points_enm;
	int m_points_target;
	float m_respawnTimeout_ply;
	float m_respawnTimeout_enm;
	float m_hitAlpha;
	TSCharacter* m_player;
	TSCharacter* m_enemy;
	SGRX_ActorCtrlHandle m_actorCtrl_ply;
	SGRX_ActorCtrlHandle m_actorCtrl_enm;
	Array< Vec3 > m_spawnPoints;
};

