

#include "tsui.hpp"
#include "systems.hpp"


extern GameLevel* g_GameLevel;

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
	if( ctrl.style == MCS_ObjectiveItem ||
		ctrl.style == MCS_ObjectiveItemDone )
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
	if( ctrl.style == MCS_ObjectiveItem ||
		ctrl.style == MCS_ObjectiveItemDone )
	{
		col.fgcol = V4( 153/255.f, 1 );
		if( info.selected )
			col.fgcol = V4( 0.97f, 1 );
		else if( info.highlighted )
			col.fgcol = V4( 193/255.f, 1 );
	}
	
	GR2D_SetFont( "mono", info.minw * 20 / 720.f );
	
	br.Reset().Col( col.fgcol.x, col.fgcol.y, col.fgcol.z, col.fgcol.w * info.menu->opacity );
	float rtpos = round(TLERP(ax0, ax1, 0.8f));
	float yht = round((ay0+ay1)/2);
	GR2D_DrawTextLine( rtpos, yht,
		ctrl.caption, HALIGN_RIGHT, VALIGN_CENTER );
	
	if( ctrl.style == MCS_ObjectiveItemDone )
	{
		int width = GR2D_GetTextLength( ctrl.caption );
		float solht = info.minw * 1 / 720.f;
		br.Reset().Col( col.fgcol.x, col.fgcol.y, col.fgcol.z,
			col.fgcol.w * info.menu->opacity );
		br.AARect( rtpos - width, yht + solht * 2, rtpos, yht + solht * 3 );
	}
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
	selobj_id = 0;
	firstobj = 0;
	
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
	
	ReloadObjMenu();
	
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
		Array< OSObjective >& objlist = g_GameLevel->GetSystem<ObjectiveSystem>()->m_objectives;
		int sel = objmenu.OnEvent( e );
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP ) sel = ScrollObjMenu( -1 );
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN ) sel = ScrollObjMenu( 1 );
		if( e.type == SDL_MOUSEWHEEL && e.wheel.y != 0 )
		{
			if( e.wheel.y > 0 )
			{
				firstobj = TMAX( 0, firstobj - 1 );
				ReloadObjMenu();
			}
			else
			{
				firstobj = TMIN( firstobj + 1, (int) objlist.size() - 11 );
				ReloadObjMenu();
			}
		}
		if( sel >= 0 )
		{
			int cid = objmenu.controls[ sel ].id;
			
			if( objmenu.controls[ sel ].group == 0 )
			{
				selobj_id = cid;
				selected_objective = objlist[ selobj_id ];
			}
			if( cid == PMS_OBJ_SCROLLUP )
			{
				firstobj = TMAX( 0, firstobj - 1 );
				ReloadObjMenu();
			}
			if( cid == PMS_OBJ_SCROLLDN )
			{
				firstobj = TMIN( firstobj + 1, (int) objlist.size() - 11 );
				ReloadObjMenu();
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
		OSObjStats stats = g_GameLevel->GetSystem<ObjectiveSystem>()->GetStats();
		
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
		
		// icons
		GR2D_SetFont( "tsicons", objmenu.GetMinw() * 32 / 720.f );
		Array< OSObjective >& objlist = g_GameLevel->GetSystem<ObjectiveSystem>()->m_objectives;
		for( int i = firstobj; i < TMIN( (int) objlist.size(), firstobj + 11 ); ++i )
		{
			OSObjective& OBJ = objlist[ i ];
			
			float q = ( i - firstobj ) * 37;
			br.Reset();
			uint8_t c_a = i == selobj_id ? 255 : 192;
			if( OBJ.required )
			{
				br.Colb( 100, 119, 223, c_a );
				GR2D_DrawTextLine( round(objmenu.IX(438/1280.f)),
					round(objmenu.IY((190+q)/720.f)),
					"\x14", HALIGN_CENTER, VALIGN_CENTER );
			}
			if( OBJ.hasLocation && OBJ.state != OSObjective::Hidden )
			{
				br.Colb( 55, 162, 46, c_a );
				GR2D_DrawTextLine( round(objmenu.IX(480/1280.f)),
					round(objmenu.IY((190+q)/720.f)),
					"\x15", HALIGN_CENTER, VALIGN_CENTER );
			}
		}
		
		br.Reset();
		GR2D_SetFont( "mono", objmenu.GetMinw() * 16 / 720.f );
		GR2D_DrawTextLine( round(objmenu.IX(426/1280.f)), round(objmenu.IY(121/720.f)),
			"Required", HALIGN_LEFT, VALIGN_CENTER );
		GR2D_DrawTextLine( round(objmenu.IX(468/1280.f)), round(objmenu.IY(141/720.f)),
			"Location", HALIGN_LEFT, VALIGN_CENTER );
		
		GR2D_SetFont( "mono", objmenu.GetMinw() * 20 / 720.f );
		// draw objective title, description, state, info
		{
			OSObjective& OBJ = selected_objective;
			
			StringView title = "???", desc = "Objective not discovered yet.";
			if( OBJ.state != OSObjective::Hidden )
			{
				title = OBJ.title;
				desc = OBJ.desc;
			}
			
			br.Reset();
			GR2D_DrawTextLine( round(objmenu.IX(530/1280.f)), round(objmenu.IY(190/720.f)),
				title, HALIGN_LEFT, VALIGN_CENTER );
			if( OBJ.state == OSObjective::Hidden )
				br.Col( 0.7f, 1 );
		//	GR2D_DrawTextLine( round(objmenu.IX(530/1280.f)), round(objmenu.IY(240/720.f)),
		//		desc, HALIGN_LEFT, VALIGN_CENTER );
			GR2D_DrawTextRect( objmenu.IX(530/1280.f), objmenu.IY(230/720.f),
				objmenu.IX(1210/1280.f), objmenu.IY(590/720.f),
				desc, HALIGN_LEFT, VALIGN_TOP );
			
			// state
			br.Col( 1, 0.5f );
			StringView status;
			if( OBJ.state == OSObjective::Open ) status = "IN PROGRESS";
			else if( OBJ.state == OSObjective::Done ) status = "DONE";
			else if( OBJ.state == OSObjective::Failed ) status = "FAILED";
			else if( OBJ.state == OSObjective::Cancelled ) status = "CANCELLED";
			GR2D_DrawTextLine( round(objmenu.IX(1130/1280.f)), round(objmenu.IY(190/720.f)),
				status, HALIGN_RIGHT, VALIGN_CENTER );
			
			// icons
			GR2D_SetFont( "tsicons", objmenu.GetMinw() * 32 / 720.f );
			
			if( OBJ.required )
				br.Colb( 100, 119, 223 );
			else
				br.Col( 0.5f, 0.5f );
			GR2D_DrawTextLine( round(objmenu.IX(1154/1280.f)),
				round(objmenu.IY(190/720.f)),
				"\x16", HALIGN_CENTER, VALIGN_CENTER );
			
			if( OBJ.state != OSObjective::Hidden )
			{
				if( OBJ.hasLocation )
					br.Colb( 55, 162, 46 );
				else
					br.Col( 0.5f, 0.5f );
				
				GR2D_DrawTextLine( round(objmenu.IX(1194/1280.f)),
					round(objmenu.IY(190/720.f)),
					"\x17", HALIGN_CENTER, VALIGN_CENTER );
			}
		}
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

void TSPauseMenuScreen::ReloadObjMenu()
{
	Array< OSObjective >& objlist = g_GameLevel->GetSystem<ObjectiveSystem>()->m_objectives;
	selobj_id = TMIN( (int) objlist.size() - 1,
		TMIN( firstobj + 11 - 1, TMAX( firstobj, selobj_id ) ) );
	
	objmenu.Clear();
	if( objlist.size() > 11 )
	{
		if( firstobj > 0 )
		{
			objmenu.AddButton( "\x12", MCS_ObjectiveIconLink,
				260/1280.f, 130/720.f, 292/1280.f, 162/720.f, PMS_OBJ_SCROLLUP );
		}
		if( firstobj < (int)objlist.size() - 11 )
		{
			objmenu.AddButton( "\x13", MCS_ObjectiveIconLink,
				260/1280.f, 595/720.f, 292/1280.f, 627/720.f, PMS_OBJ_SCROLLDN );
		}
	}
	for( int i = firstobj; i < TMIN( (int) objlist.size(), firstobj + 11 ); ++i )
	{
		OSObjective& OBJ = objlist[ i ];
		float q = ( i - firstobj ) * 37;
		objmenu.AddRadioBtn( OBJ.state == OSObjective::Hidden ? "???" : OBJ.title,
			OBJ.state == OSObjective::Done ? MCS_ObjectiveItemDone : MCS_ObjectiveItem,
			50/1280.f, (170+q)/720.f, 500/1280.f, (207+q)/720.f, 0, i );
	}
	objmenu.SelectInGroup( 0, selobj_id - firstobj );
	objmenu.ReselectHL();
	if( selobj_id >= 0 )
		selected_objective = objlist[ selobj_id ];
	else
	{
		selected_objective.state = OSObjective::Hidden;
		selected_objective.required = false;
	}
}

int TSPauseMenuScreen::ScrollObjMenu( int amount )
{
	int sel = -1;
	Array< OSObjective >& objlist = g_GameLevel->GetSystem<ObjectiveSystem>()->m_objectives;
	while( amount != 0 )
	{
		if( amount < 0 )
		{
			if( selobj_id == firstobj )
			{
				firstobj--;
				selobj_id--;
				if( firstobj < 0 )
				{
					firstobj = objlist.size() - 11;
					selobj_id = objlist.size() - 1;
				}
				ReloadObjMenu();
			}
			else
				sel = objmenu.SelectPrevInGroup( 0 );
		}
		else // amount > 0
		{
			if( selobj_id == firstobj + 11 - 1 )
			{
				firstobj++;
				selobj_id++;
				if( firstobj > (int) objlist.size() - 11 )
				{
					firstobj = 0;
					selobj_id = 0;
				}
				ReloadObjMenu();
			}
			else
				sel = objmenu.SelectNextInGroup( 0 );
		}
		
		amount -= amount > 0 ? 1 : -1;
	}
	return sel;
}


