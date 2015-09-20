

#include <engine.hpp>


Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );


#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct TestSuite : IGame
{
	TestSuite() : m_accum( 0.0f ), m_time( 0 )
	{
	}
	
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 24 );
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );

		Game_BindKeyToAction( SDLK_a, &MOVE_LEFT );
		Game_BindKeyToAction( SDLK_d, &MOVE_RIGHT );
		return true;
	}
	void OnDestroy()
	{
	}
	void OnEvent( const Event& e )
	{
	}
	void Game_Tick( float dt, float bf )
	{
	}
	void Game_FixedTick( float dt )
	{
	}
	void DrawSomeText()
	{
		TextureHandle cliptex = GR_GetTexture( "textures/clip.png:nolerp" );
		GR_PreserveResource( cliptex );
		int size = 24 + 12 * sinf(m_time);
		GR2D_SetFont( "core", size );
		
		GR2D_GetBatchRenderer().Reset().SetTexture( cliptex ).Quad( 0, 0, 16, 16 );
		GR2D_DrawTextLine( round(20 * (sinf(m_time)*0.5f+0.5f)),
			round(20 * (cosf(m_time)*0.5f+0.5f)), "abcdefghijklmnopqrstuvwxyz" );
		GR2D_DrawTextLine( round(20 * (sinf(m_time)*0.5f+0.5f)),
			round(20 * (cosf(m_time)*0.5f+0.5f)) + size, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	}
	void Game_Render()
	{
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, 1, 1 ) );
		TextureHandle cliptex = GR_GetTexture( "textures/clip.png:nolerp" );
		GR2D_GetBatchRenderer().Reset().SetTexture( cliptex ).Quad( 0, 0, 1, 1 );
		
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		DrawSomeText();
		GR2D_SetWorldMatrix( Mat4::CreateScale( 2, 2, 1 ) * Mat4::CreateTranslation( 0, 50, 0 ) );
		DrawSomeText();
		GR2D_SetWorldMatrix( Mat4::CreateScale( 4, 4, 1 ) * Mat4::CreateTranslation( 0, 100, 0 ) );
		DrawSomeText();
		GR2D_SetWorldMatrix( Mat4::Identity );
	}
	void OnTick( float dt, uint32_t gametime )
	{
		m_time += dt;
		m_accum += dt;
		while( m_accum >= 0 )
		{
			Game_FixedTick( FIXED_TICK_SIZE );
			m_accum -= FIXED_TICK_SIZE;
		}
		
		Game_Tick( dt, ( m_accum + FIXED_TICK_SIZE ) / FIXED_TICK_SIZE );
		
		Game_Render();
	}
	
	float m_accum;
	float m_time;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}


