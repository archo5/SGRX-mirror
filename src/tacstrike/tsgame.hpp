

#include "level.hpp"
#include "tsui.hpp"
#include "entities.hpp"
#include "entities_ts.hpp"



struct TSFightGameMode : IGameLevelSystem, IScreen
{
	enum GameState
	{
		GS_Playing,
		GS_Intro,
	};
	enum { e_system_uid = 1000 };
	TSFightGameMode( GameLevel* lev );
	void Tick( float deltaTime, float blendFactor );
	
	GameState m_state;
	float m_timeout;
};

