

#include <engine.hpp>
#include <gamegui.hpp>


InputState MOVE_LEFT( "move_left" );
InputState MOVE_RIGHT( "move_right" );


struct ITest
{
	virtual StringView GetName() const { return "<unnamed test>"; }
	virtual void Do( float dt, float bf ){}
	virtual void FixedTick( float dt ){}
	virtual void OnInitialize(){}
	virtual void OnDestroy(){}
	virtual void OnEvent( const Event& e ){}
};



struct Test_Intro : ITest
{
	virtual StringView GetName() const { return "Intro"; }
	void Do( float, float )
	{
		GR2D_SetWorldMatrix( Mat4::Identity );
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		GR2D_SetFont( "core", 24 );
		GR2D_DrawTextLine( GR_GetWidth()/2, GR_GetHeight()/2,
			"Press F2/F3 to change tests", HALIGN_CENTER, VALIGN_CENTER );
	}
}
g_TestIntro;



struct Test_PixelPerfectRendering : ITest
{
	Test_PixelPerfectRendering() : m_time( 0 ){}
	virtual StringView GetName() const { return "Pixel-perfect rendering"; }
	void Do( float dt, float )
	{
		m_time += dt;
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
	float m_time;
}
g_TestPixelPerfectRendering;



struct Test_GameUI : ITest
{
	virtual StringView GetName() const { return "Game GUI test"; }
	Test_GameUI() : m_guiSys(NULL)
	{
	}
	
	void OnInitialize()
	{
		m_scriptCtx = new ScriptContext;
		m_guiSys = new GameUISystem( m_scriptCtx );
		m_guiSys->Load( "ui/test.sgs" );
	}
	void OnDestroy()
	{
		m_guiSys = NULL;
		delete m_scriptCtx;
	}
	void OnEvent( const Event& e )
	{
		m_guiSys->EngineEvent( e );
	}
	void Do( float dt, float )
	{
		m_guiSys->Draw( dt );
	}
	
	ScriptContext* m_scriptCtx;
	GUISysHandle m_guiSys;
}
g_TestGameUI;





size_t g_CurTest = 0;
ITest* g_Tests[] =
{
	&g_TestIntro,
	&g_TestPixelPerfectRendering,
	&g_TestGameUI,
};
#define TESTCOUNT (sizeof(g_Tests)/sizeof(g_Tests[0]))

static void InitTest()
{
	ITest* T = g_Tests[ g_CurTest ];
	T->OnInitialize();
	char bfr[ 4096 ];
	sgrx_snprintf( bfr, 4096, "Test #%d: %s", (g_CurTest+1), StackString<4096>(T->GetName()).str );
	Window_SetTitle( bfr );
}
static void FreeTest()
{
	ITest* T = g_Tests[ g_CurTest ];
	T->OnDestroy();
}
static void SetTest( size_t i )
{
	if( i < 0 ) i = 0;
	if( i >= TESTCOUNT ) i = TESTCOUNT - 1;
	FreeTest();
	g_CurTest = i;
	InitTest();
}


struct TestSuite : IGame
{
	TestSuite() : m_accum( 0.0f )
	{
	}
	
	bool OnConfigure( int argc, char* argv[] )
	{
#if 0
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.width = 800;
		rs.height = 600;
		//rs.fullscreen = FULLSCREEN_WINDOWED;
		GR_SetVideoMode( rs );
#endif
		return true;
	}
	
	bool OnInitialize()
	{
		Game_FileSystems().insert( 0, new BasicFileSystem( "../data-test" ) );
		
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );

		Game_BindKeyToAction( SDLK_F2, &MOVE_LEFT );
		Game_BindKeyToAction( SDLK_F3, &MOVE_RIGHT );
		
		InitTest();
		return true;
	}
	void OnDestroy()
	{
		FreeTest();
	}
	void OnTick( float dt, uint32_t gametime )
	{
#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)
		m_accum += dt;
		while( m_accum >= 0 )
		{
			g_Tests[ g_CurTest ]->FixedTick( FIXED_TICK_SIZE );
			m_accum -= FIXED_TICK_SIZE;
		}
		
		if( MOVE_LEFT.IsPressed() )
		{
			SetTest( ( g_CurTest + TESTCOUNT - 1 ) % TESTCOUNT );
		}
		if( MOVE_RIGHT.IsPressed() )
		{
			SetTest( ( g_CurTest + 1 ) % TESTCOUNT );
		}
		g_Tests[ g_CurTest ]->Do( dt, ( m_accum + FIXED_TICK_SIZE ) / FIXED_TICK_SIZE );
	}
	void OnEvent( const Event& e )
	{
		g_Tests[ g_CurTest ]->OnEvent( e );
	}
	
	float m_accum;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}


