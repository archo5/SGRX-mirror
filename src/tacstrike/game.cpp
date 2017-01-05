

#include "entities_ts.hpp"


Vec2 CURSOR_POS = V2(0);


struct TACStrikeGame : BaseGame
{
	TACStrikeGame()
	{
		RegisterSystem<TSGameSystem>();
		cursor_dt = V2(0);
		RegisterCommonGameCVars();
	}
	
	Vec2 cursor_dt;
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_MOUSEMOTION )
		{
			CURSOR_POS = Game_GetCursorPos();
		}
		else if( e.type == SDL_CONTROLLERAXISMOTION )
		{
#if AAA
			float rad = TMIN( GR_GetWidth(), GR_GetHeight() ) * 0.05f;
			Vec2 off = V2( AIM_X.value, AIM_Y.value );
			if( off.Length() > 0.35f )
			{
				cursor_dt = off.Normalized() *
					powf( TREVLERP<float>( 0.35f, 0.75f, off.Length() ), 1.5f ) * rad;
			}
			else
				cursor_dt = V2(0);
#elif BBB
			float rad = TMAX( GR_GetWidth(), GR_GetHeight() ) * 0.5f;
			Vec2 off = V2( AIM_X.value, AIM_Y.value );
			if( off.Length() > 0.1f )
				CURSOR_POS = Game_GetScreenSize() * 0.5f + off * rad;
#endif
		}
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
	//	CURSOR_POS += V2( AIM_X.value, AIM_Y.value ) * TMIN( GR_GetWidth(), GR_GetHeight() ) * 0.03f;
		CURSOR_POS += cursor_dt;
		CURSOR_POS.x = clamp( CURSOR_POS.x, 0, GR_GetWidth() );
		CURSOR_POS.y = clamp( CURSOR_POS.y, 0, GR_GetHeight() );
		m_level->GetScriptCtx().SetGlobal( "CURSOR_POS",
			m_level->GetScriptCtx().CreateVec2( CURSOR_POS ) );
		m_timeMultiplier = Game_GetActionState( "slowdown_test" ).state ? 0.25f : 1.0f;
		
		BaseGame::OnTick( dt, gametime );
	}
};


extern "C" EXPORT IGame* CreateGame()
{
	return new TACStrikeGame;
}

