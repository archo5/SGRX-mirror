

#include "tsui.hpp"
#include "level.hpp"


TSMenuTheme g_TSMenuTheme;
TSSplashScreen g_SplashScreen;
TSQuitGameQuestionScreen g_QuitGameQuestionScreen;
TSPauseMenuScreen g_PauseMenu;


TSMenuTheme::TSMenuTheme()
{
	color_hl = V4( 35, 148, 215, 1 ) * V4(1.0f/255.0f,1);
	color_sel = V4( 25, 128, 185, 1 ) * V4(1.0f/255.0f,1);
}

void TSMenuTheme::DrawControl( const MenuControl& ctrl, const MenuCtrlInfo& info )
{
	if( ctrl.style == MCS_BigTopLink )
	{
		DrawBigTopLinkButton( ctrl, info );
		return;
	}
	if( ctrl.style == MCS_ObjectiveItem )
	{
		DrawObjectiveItemButton( ctrl, info );
		return;
	}
	if( ctrl.style == MCS_ObjectiveIconLink )
	{
		DrawObjectiveIconLink( ctrl, info );
		return;
	}
	MenuTheme::DrawControl( ctrl, info );
}

void TSMenuTheme::DrawBigTopLinkButton( const MenuControl& ctrl, const MenuCtrlInfo& info )
{
	MENUTHEME_PREP;
	Colors col;
	_GetCtrlColors( ctrl, info, true, col );
	
	GR2D_SetFont( "fancy", info.minw / 30 );
	
	br.Reset().Col( col.fgcol.x, col.fgcol.y, col.fgcol.z, col.fgcol.w * info.menu->opacity );
	GR2D_DrawTextLine(
		round((ax0+ax1)/2),
		round((ay0+ay1)/2),
		ctrl.caption, HALIGN_CENTER, VALIGN_CENTER );
}

void TSMenuTheme::DrawObjectiveItemButton( const MenuControl& ctrl, const MenuCtrlInfo& info )
{
	MENUTHEME_PREP;
	Colors col;
	_GetCtrlColors( ctrl, info, true, col );
	if( ctrl.style == MCS_ObjectiveItem )
	{
		col.fgcol = V4( 153/255.f, 1 );
		if( info.selected )
			col.fgcol = V4( 0.97f, 1 );
		else if( info.highlighted )
			col.fgcol = V4( 193/255.f, 1 );
	}
	
	GR2D_SetFont( "mono", info.minw * 20 / 720.f );
	
	br.Reset().Col( col.fgcol.x, col.fgcol.y, col.fgcol.z, col.fgcol.w * info.menu->opacity );
	GR2D_DrawTextLine(
		round(TLERP(ax0, ax1, 0.8f)),
		round((ay0+ay1)/2),
		ctrl.caption, HALIGN_RIGHT, VALIGN_CENTER );
}

void TSMenuTheme::DrawObjectiveIconLink( const MenuControl& ctrl, const MenuCtrlInfo& info )
{
	MENUTHEME_PREP;
	
	Vec4 col = V4( 153/255.f, 1 );
	if( info.selected )
		col = V4( 0.97f, 1 );
	else if( info.highlighted )
		col = V4( 193/255.f, 1 );
	
	br.Reset().Col( col.x, col.y, col.z, col.w * info.menu->opacity );
	GR2D_SetFont( "tsicons", info.minw * 32 / 720.f );
	GR2D_DrawTextLine(
		round((ax0+ax1)/2),
		round((ay0+ay1)/2),
		ctrl.caption, HALIGN_CENTER, VALIGN_CENTER );
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
	GR2D_SetFont( "core", (menu.y1 - menu.y0) / 30 );
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


TSPauseMenuScreen::TSPauseMenuScreen() : notfirst(false), show_objectives(false)
{
	topmenu.theme = &g_TSMenuTheme;
	pausemenu.theme = &g_TSMenuTheme;
	objmenu.theme = &g_TSMenuTheme;
	
	topmenu.AddRadioBtn( "MENU", MCS_BigTopLink, 0.2f, 0.0f, 0.4f, 0.14f, 0 );
	topmenu.AddRadioBtn( "OBJECTIVES", MCS_BigTopLink, 0.4f, 0.0f, 0.6f, 0.14f, 0 );
	topmenu.SelectInGroup( 0, 0 );
	
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
	
	objmenu.Clear();
	objmenu.AddButton( "\x12", MCS_ObjectiveIconLink,
		260/1280.f, 130/720.f, 292/1280.f, 162/720.f, 1 );
	objmenu.AddButton( "\x13", MCS_ObjectiveIconLink,
		260/1280.f, 595/720.f, 292/1280.f, 627/720.f, 1 );
	Array< OSObjective >& objlist = g_GameLevel->m_objectiveSystem.m_objectives;
	for( size_t i = 0; i < objlist.size(); ++i )
	{
		OSObjective& OBJ = objlist[ i ];
		float q = i * 37;
		objmenu.AddRadioBtn( OBJ.state == OSObjective::Hidden ? "???" : OBJ.title,
			MCS_ObjectiveItem, 50/1280.f, (170+q)/720.f, 500/1280.f, (207+q)/720.f, 0 );
	}
	objmenu.SelectInGroup( 0, 0 );
	
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
	
	// TOP MENU
	{
		int sel = topmenu.OnEvent( e );
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_p )
		{
			topmenu.SelectInGroup( 0, 0 );
			sel = 0;
		}
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_o )
		{
			topmenu.SelectInGroup( 0, 1 );
			sel = 1;
		}
		if( sel == 0 ) show_objectives = false;
		else if( sel == 1 ) show_objectives = true;
	}
	
	// PAUSE MENU
	if( show_objectives == false )
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
	
	// OBJECTIVE MENU
	if( show_objectives )
	{
		int sel = objmenu.OnEvent( e );
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP )
			sel = objmenu.SelectPrevInGroup( 0 );
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN )
			sel = objmenu.SelectNextInGroup( 0 );
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
	
	topmenu.RecalcSize( GR_GetWidth(), GR_GetHeight(), 4.0f/3.0f );
	pausemenu.RecalcSize( GR_GetWidth(), GR_GetHeight(), 16.0f/9.0f );
	objmenu.RecalcSize( GR_GetWidth(), GR_GetHeight(), 16.0f/9.0f );
	float ref = pausemenu.GetMinw();
	
	br.Reset().Col( 0.9f, topmenu.opacity * 0.5f );
	GR2D_SetFont( "fancy", ref / 30 );
	GR2D_DrawTextLine( ref / 20, ref / 20, "PAUSED", HALIGN_LEFT, VALIGN_TOP );
	
	topmenu.Draw( delta );
	if( show_objectives == false )
	{
		pausemenu.Draw( delta );
	}
	if( show_objectives )
	{
		OSObjStats stats = g_GameLevel->m_objectiveSystem.GetStats();
		
		br.Reset().Col( 0.976f, objmenu.opacity );
		br.AARect(
			objmenu.IX(435.0f/1280.0f), objmenu.IY(134.0f/720.0f),
			objmenu.IX(439.0f/1280.0f), objmenu.IY(168.0f/720.0f) );
		br.AARect(
			objmenu.IX(477.0f/1280.0f), objmenu.IY(155.0f/720.0f),
			objmenu.IX(481.0f/1280.0f), objmenu.IY(168.0f/720.0f) );
		br.AARect(
			objmenu.IX(510.0f/1280.0f), objmenu.IY(170.0f/720.0f),
			objmenu.IX(514.0f/1280.0f), objmenu.IY(590.0f/720.0f) );
		
		GR2D_SetFont( "mono", objmenu.GetMinw() * 16 / 720.f );
		GR2D_DrawTextLine( round(objmenu.IX(426/1280.f)), round(objmenu.IY(121/720.f)),
			"Required", HALIGN_LEFT, VALIGN_CENTER );
		GR2D_DrawTextLine( round(objmenu.IX(468/1280.f)), round(objmenu.IY(141/720.f)),
			"Location", HALIGN_LEFT, VALIGN_CENTER );
		
		GR2D_SetFont( "mono", objmenu.GetMinw() * 20 / 720.f );
		// draw "in progress"
		{
			char bfr[32];
			sgrx_snprintf( bfr, sizeof(bfr), "(%d/%d)", stats.numOpen, stats.numTotal );
			int szx1 = GR2D_GetTextLength( "IN PROGRESS: " );
			int szx2 = GR2D_GetTextLength( bfr );
			
			br.Colb( 35, 148, 215 );
			GR2D_DrawTextLine(
				round(objmenu.IX(1210/1280.f)) - szx1 - szx2,
				round(objmenu.IY(85/720.f)),
				"IN PROGRESS: ", HALIGN_LEFT, VALIGN_CENTER );
			br.Colb( 35, 148, 215, 191 );
			GR2D_DrawTextLine(
				round(objmenu.IX(1210/1280.f)) - szx2,
				round(objmenu.IY(85/720.f)),
				bfr, HALIGN_LEFT, VALIGN_CENTER );
		}
		// draw "unknown"
		{
			char bfr[32];
			sgrx_snprintf( bfr, sizeof(bfr), "(%d/%d)", stats.numHidden, stats.numTotal );
			int szx1 = GR2D_GetTextLength( "UNKNOWN: " );
			int szx2 = GR2D_GetTextLength( bfr );
			
			br.Colb( 35, 148, 215 );
			GR2D_DrawTextLine(
				round(objmenu.IX(1210/1280.f)) - szx1 - szx2,
				round(objmenu.IY(113/720.f)),
				"UNKNOWN: ", HALIGN_LEFT, VALIGN_CENTER );
			br.Colb( 35, 148, 215, 191 );
			GR2D_DrawTextLine(
				round(objmenu.IX(1210/1280.f)) - szx2,
				round(objmenu.IY(113/720.f)),
				bfr, HALIGN_LEFT, VALIGN_CENTER );
		}
		// draw "completed"
		{
			char bfr[32];
			sgrx_snprintf( bfr, sizeof(bfr), "(%d/%d)", stats.numDone, stats.numTotal );
			int szx1 = GR2D_GetTextLength( "COMPLETED: " );
			int szx2 = GR2D_GetTextLength( bfr );
			
			br.Colb( 35, 148, 215 );
			GR2D_DrawTextLine(
				round(objmenu.IX(1210/1280.f)) - szx1 - szx2,
				round(objmenu.IY(141/720.f)),
				"COMPLETED: ", HALIGN_LEFT, VALIGN_CENTER );
			br.Colb( 35, 148, 215, 191 );
			GR2D_DrawTextLine(
				round(objmenu.IX(1210/1280.f)) - szx2,
				round(objmenu.IY(141/720.f)),
				bfr, HALIGN_LEFT, VALIGN_CENTER );
		}
		
		objmenu.Draw( delta );
	}
	
	return false;
}


