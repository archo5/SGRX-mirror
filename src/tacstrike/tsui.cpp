

#include "tsui.hpp"


TSMenuTheme g_TSMenuTheme;
TSSplashScreen g_SplashScreen;
TSQuitGameQuestionScreen g_QuitGameQuestionScreen;
TSPauseMenuScreen g_PauseMenu;


void TSMenuTheme::DrawControl( const MenuControl& ctrl, const MenuCtrlInfo& info )
{
	if( ctrl.style == MCS_BigTopLink )
	{
		return;
	}
	MenuTheme::DrawControl( ctrl, info );
}


TSSplashScreen::TSSplashScreen() : m_timer(0)
{
}

void TSSplashScreen::OnStart()
{
	m_timer = 0;
	m_tx_crage = GR_GetTexture( "ui/crage_logo.png" );
	m_tx_back = GR_GetTexture( "ui/flare_bg.png" );
}

void TSSplashScreen::OnEnd()
{
	m_tx_crage = NULL;
	m_tx_back = NULL;
}

bool TSSplashScreen::OnEvent( const Event& e )
{
	if( m_timer > 0.5 && ( e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN ) )
	{
		m_timer = 5;
	}
	return true;
}

bool TSSplashScreen::Draw( float delta )
{
	m_timer += delta;
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.UnsetTexture()
		.Col( 0, 1 )
		.Quad( 0, 0, GR_GetWidth(), GR_GetHeight() );
	
	float vis_crage = smoothlerp_range( m_timer, 0, 1, 3, 5 );
	if( vis_crage )
	{
		// background
		float maxw = TMAX( GR_GetWidth(), GR_GetHeight() ) + 50;
		br.SetTexture( m_tx_back ).Col( 1, vis_crage ).QuadWH( 0, -m_timer * 10, maxw, maxw );
		
		// logo
		const TextureInfo& texinfo = m_tx_crage.GetInfo();
		float scale = GR_GetWidth() / 1024.0f;
		br.SetTexture( m_tx_crage ).Box( GR_GetWidth() / 2.0f, GR_GetHeight() / 2.1f, texinfo.width * scale, texinfo.height * scale );
	}
	
	return m_timer > 5;
}


TSQuestionScreen::TSQuestionScreen() : animFactor(0)
{
	question = "What's it gonna be?";
	menu.AddButton( "No", MCS_Link, 0.5f, 0.6f, 0.8f, 0.65f );
	menu.AddButton( "Yes", MCS_Link, 0.2f, 0.6f, 0.5f, 0.65f );
}

void TSQuestionScreen::OnStart()
{
	animFactor = 0.001f;
	animTarget = 1;
	menu.OnStart();
}

void TSQuestionScreen::OnEnd()
{
	animFactor = 0;
	menu.OnEnd();
}

bool TSQuestionScreen::OnEvent( const Event& e )
{
	if( animTarget != 1 )
		return false;
	
	if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
	{
		if( Action( -1 ) )
			animTarget = 0;
		return true;
	}
	
	int sel = menu.OnEvent( e );
	if( sel >= 0 )
	{
		if( Action( sel ) )
			animTarget = 0;
	}
	return true;
}

bool TSQuestionScreen::Draw( float delta )
{
	float diff = animTarget - animFactor;
	animFactor += sign( diff ) * TMIN( fabsf( diff ), delta * 2 );
	
	menu.RecalcSize( GR_GetWidth(), GR_GetHeight() );
	menu.opacity = smoothstep( animFactor );
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	br.Col( 0, 0.5f * menu.opacity );
	br.Quad( 0, menu.y0, GR_GetWidth(), GR_GetHeight() );
	
	br.Col( 0.8f, menu.opacity );
	GR2D_SetFont( "fonts/lato-regular.ttf", (menu.y1 - menu.y0) / 30 );
	GR2D_DrawTextLine( TLERP(menu.x0,menu.x1,0.5f), TLERP(menu.y0,menu.y1,0.3f), question, HALIGN_CENTER, VALIGN_CENTER );
	
	menu.Draw( delta );
	return animFactor <= 0;
}

bool TSQuestionScreen::Action( int mode )
{
	LOG << "ACTION: " << mode;
	return true;
}


TSQuitGameQuestionScreen::TSQuitGameQuestionScreen()
{
	question = "Do you really want to quit the game?";
}

bool TSQuitGameQuestionScreen::Action( int mode )
{
	if( mode == 1 )
		Game_End();
	return true;
}


TSPauseMenuScreen::TSPauseMenuScreen() : notfirst(false)
{
	topmenu.theme = &g_TSMenuTheme;
	pausemenu.theme = &g_TSMenuTheme;
	objmenu.theme = &g_TSMenuTheme;
	
	float bm = 0.05f;
	int bc = 3;
	float bsz = ( 1.0f - ( bc + 1 ) * bm ) / bc;
	float x = bm;
	pausemenu.AddButton( "Resume", MCS_Link, x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
	pausemenu.AddButton( "Options", MCS_Link, x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
	pausemenu.AddButton( "Exit", MCS_Link, x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
}

void TSPauseMenuScreen::OnStart()
{
	topmenu.OnStart();
	pausemenu.OnStart();
	objmenu.OnStart();
	
	Game_ShowCursor( true );
	notfirst = false;
//	g_GameLevel->m_paused = true;
//	resetcontrols();
}

void TSPauseMenuScreen::OnEnd()
{
	topmenu.OnEnd();
	pausemenu.OnEnd();
	objmenu.OnEnd();
	
	Game_ShowCursor( false );
//	g_GameLevel->m_paused = false;
}

void TSPauseMenuScreen::Unpause()
{
	Game_RemoveOverlayScreen( this );
}

bool TSPauseMenuScreen::OnEvent( const Event& e )
{
	if( notfirst && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
	{
		Unpause();
		return true;
	}
	
	// PAUSE MENU
	{
		int sel = pausemenu.OnEvent( e );
		if( sel >= 0 )
		{
			if( sel == 0 )
			{
				// resume playing
				Unpause();
				return true;
			}
			else if( sel == 1 )
			{
			//	Game_AddOverlayScreen( &g_OptionsMenu );
				pausemenu.m_HL = -1;
			}
			else if( sel == 2 )
			{
				Game_AddOverlayScreen( &g_QuitGameQuestionScreen );
				pausemenu.m_HL = -1;
			}
		}
	}
	
	notfirst = true;
	return true;
}

bool TSPauseMenuScreen::Draw( float delta )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	br.Col( 0, 0.5f );
	br.Quad( 0, 0, GR_GetWidth(), GR_GetHeight() );
	
	topmenu.RecalcSize( GR_GetWidth(), GR_GetHeight(), 16.0f/9.0f );
	pausemenu.RecalcSize( GR_GetWidth(), GR_GetHeight(), 16.0f/9.0f );
	objmenu.RecalcSize( GR_GetWidth(), GR_GetHeight(), 16.0f/9.0f );
	float ref = topmenu.GetMinw();
	
	br.Reset();
	br.Col( 0.9f, topmenu.opacity * 0.5f );
	GR2D_SetFont( "fancy", ref / 30 );
	GR2D_DrawTextLine( ref / 20, ref / 20, "PAUSED", HALIGN_LEFT, VALIGN_TOP );
	
	topmenu.Draw( delta );
	pausemenu.Draw( delta );
	objmenu.Draw( delta );
	
	return false;
}


