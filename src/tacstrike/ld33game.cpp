

#include "level.hpp"
#include "systems.hpp"
#include "entities_ld33.hpp"


GameLevel* g_GameLevel = NULL;
SoundSystemHandle g_SoundSys;

InputState MOVE_LEFT( "move_left" );
InputState MOVE_RIGHT( "move_right" );
InputState MOVE_UP( "move_up" );
InputState MOVE_DOWN( "move_down" );
InputState INTERACT( "interact" );
InputState THROW_COIN( "throw_coin" );
InputState SPRINT( "sprint" );
InputState SHOW_OBJECTIVES( "show_objectives" );


static void resetcontrols()
{
	MOVE_LEFT.value = 0;
	MOVE_RIGHT.value = 0;
	MOVE_UP.value = 0;
	MOVE_DOWN.value = 0;
	INTERACT.value = 0;
	THROW_COIN.value = 0;
	SPRINT.value = 0;
	SHOW_OBJECTIVES.value = 0;
}


bool g_show_money = false;
bool g_show_suspicion = false;
float g_money = 0;
const float g_money_target = 1000000.0f;
float g_suspicion = 0;
String g_end_reason;
Vec3 g_endcol = V3(1);


void SetupLevel();



ActionInput g_i_move_left = ACTINPUT_MAKE_KEY( SDLK_a );
ActionInput g_i_move_right = ACTINPUT_MAKE_KEY( SDLK_d );
ActionInput g_i_move_up = ACTINPUT_MAKE_KEY( SDLK_w );
ActionInput g_i_move_down = ACTINPUT_MAKE_KEY( SDLK_s );
ActionInput g_i_interact = ACTINPUT_MAKE_MOUSE( SGRX_MB_LEFT );
ActionInput g_i_throw_coin = ACTINPUT_MAKE_KEY( SDLK_q );
ActionInput g_i_sprint = ACTINPUT_MAKE_KEY( SDLK_LSHIFT );
ActionInput g_i_show_objectives = ACTINPUT_MAKE_KEY( SDLK_TAB );

float g_i_mouse_sensitivity = 1.0f;
bool g_i_mouse_invert_x = false;
bool g_i_mouse_invert_y = false;

float g_s_vol_master = 0.8f;
float g_s_vol_sfx = 0.8f;
float g_s_vol_music = 0.8f;



void SaveConfig( bool nd = true )
{
	RenderSettings rs;
	GR_GetVideoMode( rs );
	
	const char* aa_mode_str = "none";
	if( rs.aa_mode == ANTIALIAS_MULTISAMPLE )
	{
		if( rs.aa_quality == 2 ) aa_mode_str = "msaa2x";
		if( rs.aa_quality == 4 ) aa_mode_str = "msaa4x";
		if( rs.aa_quality == 8 ) aa_mode_str = "msaa8x";
	}
	
	if( nd )
	{
		g_s_vol_master = g_SoundSys->GetVolume( "bus:/" );
		g_s_vol_sfx = g_SoundSys->GetVolume( "bus:/sfx" );
		g_s_vol_music = g_SoundSys->GetVolume( "bus:/music" );
	}
	
	InLocalStorage ils( "CrageGames/OfficeTheftGame" );
	
	char bfr[ 65536 ];
	snprintf( bfr, sizeof(bfr),
		"# renderer\n"
		"r_display %d\n"
		"r_width %d\n"
		"r_height %d\n"
		"r_refresh_rate %d\n"
		"r_fullscreen %d\n"
		"r_vsync %s\n"
		"r_antialias %s\n"
		"# sound\n"
		"s_vol_master %g\n"
		"s_vol_sfx %g\n"
		"s_vol_music %g\n"
		"# input\n"
		"i_move_left %u:%u\n"
		"i_move_right %u:%u\n"
		"i_move_up %u:%u\n"
		"i_move_down %u:%u\n"
		"i_interact %u:%u\n"
		"i_throw_coin %u:%u\n"
		"i_sprint %u:%u\n"
		"i_show_objectives %u:%u\n"
		"i_mouse_sensitivity %g\n"
		"i_mouse_invert_x %s\n"
		"i_mouse_invert_y %s\n"
		
		, rs.display
		, rs.width
		, rs.height
		, rs.refresh_rate
		, rs.fullscreen
		, rs.vsync ? "true" : "false"
		, aa_mode_str
		
		, g_s_vol_master
		, g_s_vol_sfx
		, g_s_vol_music
		
		, (unsigned) ACTINPUT_GET_TYPE( g_i_move_left ), (unsigned) ACTINPUT_GET_VALUE( g_i_move_left )
		, (unsigned) ACTINPUT_GET_TYPE( g_i_move_right ), (unsigned) ACTINPUT_GET_VALUE( g_i_move_right )
		, (unsigned) ACTINPUT_GET_TYPE( g_i_move_up ), (unsigned) ACTINPUT_GET_VALUE( g_i_move_up )
		, (unsigned) ACTINPUT_GET_TYPE( g_i_move_down ), (unsigned) ACTINPUT_GET_VALUE( g_i_move_down )
		, (unsigned) ACTINPUT_GET_TYPE( g_i_interact ), (unsigned) ACTINPUT_GET_VALUE( g_i_interact )
		, (unsigned) ACTINPUT_GET_TYPE( g_i_throw_coin ), (unsigned) ACTINPUT_GET_VALUE( g_i_throw_coin )
		, (unsigned) ACTINPUT_GET_TYPE( g_i_sprint ), (unsigned) ACTINPUT_GET_VALUE( g_i_sprint )
		, (unsigned) ACTINPUT_GET_TYPE( g_i_show_objectives ), (unsigned) ACTINPUT_GET_VALUE( g_i_show_objectives )
		, g_i_mouse_sensitivity
		, g_i_mouse_invert_x ? "true" : "false"
		, g_i_mouse_invert_y ? "true" : "false"
	);
	SaveTextFile( "config.cfg", bfr );
}



struct SplashScreen : IScreen
{
	float m_timer;
	TextureHandle m_tx_crage;
	TextureHandle m_tx_back;
	
	SplashScreen() : m_timer(0)
	{
	}
	
	void OnStart()
	{
		m_timer = 0;
		m_tx_crage = GR_GetTexture( "ui/crage_logo.png" );
		m_tx_back = GR_GetTexture( "ui/flare_bg.png" );
	}
	void OnEnd()
	{
		m_tx_crage = NULL;
		m_tx_back = NULL;
	}
	
	bool OnEvent( const Event& e )
	{
		if( m_timer > 0.5 && ( e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN ) )
		{
			m_timer = 5;
		}
		return true;
	}
	bool Draw( float delta )
	{
		m_timer += delta;
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset()
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
}
g_SplashScreen;



struct ScreenMenu
{
	enum CtrlEvent { EV_MBDown, EV_MBUp, EV_MouseMove };
	enum CtrlType
	{
		CT_Null = 0,
		CT_Button,
		CT_SwitchButton,
		CT_Slider,
	};
	enum CtrlStyle
	{
		CS_Menu,
		CS_Options,
	};
	struct Control
	{
		Control() : type(CT_Null), style(CS_Options), x0(0), y0(0), x1(0), y1(0), visible(true), enabled(true), id(0), sl_value(0), sb_option(0){}
		
		CtrlType type;
		CtrlStyle style;
		String caption;
		float x0, y0, x1, y1;
		bool visible;
		bool enabled;
		int id;
		
		String value_text;
		float sl_value;
		uint64_t sb_option;
	};
	
	ScreenMenu() :
		opacity(1), m_HL(-1), m_lastHL(-1), m_selected(-1)
	{
	}
	~ScreenMenu()
	{
	}
	
	void OnStart()
	{
	}
	void OnEnd()
	{
	}
	int UpdateCtrl( CtrlEvent evtype )
	{
		if( m_selected < 0 )
			return -1;
		
		Control& CTRL = controls[ m_selected ];
		if( CTRL.type == CT_Slider )
		{
			float MG = fabsf(CTRL.y0-CTRL.y1)/4;
			float BR = MG / 4;
			float IMG = MG + BR * 2;
			float mx0 = (CTRL.x0+CTRL.x1) * 0.5f;
			CTRL.sl_value = clamp( TREVLERP<float>( mx0+IMG, CTRL.x1-IMG, m_mouseX ), 0, 1 );
			return m_selected;
		}
		return -1;
	}
	int OnEvent( const Event& e )
	{
		int ret = -1;
		switch( e.type )
		{
		case SDL_MOUSEBUTTONDOWN:
			{
				m_selected = m_HL;
				ret = UpdateCtrl( EV_MBDown );
				if( m_selected >= 0 )
					g_SoundSys->CreateEventInstance( "/ui/buttondown" )->Start();
			}
			break;
		case SDL_MOUSEBUTTONUP:
			ret = UpdateCtrl( EV_MBUp );
			if( m_selected >= 0 && m_HL == m_selected )
				ret = m_selected;
			if( m_selected >= 0 )
				g_SoundSys->CreateEventInstance( "/ui/buttonup" )->Start();
			m_selected = -1;
			break;
		case SDL_MOUSEMOTION:
			m_mouseX = TREVLERP<float>( x0, x1, (float) e.motion.x );
			m_mouseY = TREVLERP<float>( y0, y1, (float) e.motion.y );
			FindHL( m_mouseX, m_mouseY );
			ret = UpdateCtrl( EV_MouseMove );
			break;
		}
		return ret;
	}
	void Draw( float delta )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		
		for( int i = 0; i < (int) controls.size(); ++i )
		{
			Control& CTRL = controls[ i ];
			if( CTRL.visible == false )
				continue;
			
			float ax0 = TLERP( x0, x1, CTRL.x0 );
			float ax1 = TLERP( x0, x1, CTRL.x1 );
			float ay0 = TLERP( y0, y1, CTRL.y0 );
			float ay1 = TLERP( y0, y1, CTRL.y1 );
			
			Vec4 bgcol = V4( 0.1f, 0.1f, 0.1f, 0 );
			Vec4 fgcol = V4( 0.8f, 0.8f, 0.8f, 1 );
			if( CTRL.enabled == false )
			{
				fgcol.w = 0.5f;
			}
			if( CTRL.type == CT_SwitchButton || CTRL.type == CT_Slider )
			{
				if( i == m_selected || ( m_selected < 0 && i == m_HL ) )
					bgcol = V4( 0.6f, 0.1f, 0.05f, 0.5f );
			}
			else
			{
				if( i == m_HL )
				{
					fgcol = V4( 0.9f, 0.2f, 0.1f, 1 );
				}
				if( i == m_selected )
				{
					fgcol = V4( 0.6f, 0.1f, 0.05f, 1 );
				}
			}
			
			if( CTRL.type == CT_SwitchButton || CTRL.type == CT_Slider )
			{
				float ctlalpha = i == m_selected || ( m_selected < 0 && i == m_HL ) ? 1.0f : 0.9f;
				
				GR2D_SetFont( "core", TMAX(0.0f, fabsf(ay0 - ay1) * 0.2f + (y1 - y0) / 50) * 0.8f );
				
				br.Reset().Col( bgcol.x, bgcol.y, bgcol.z, bgcol.w * opacity ).Quad( ax0, ay0, ax1, ay1 );
				br.Reset().Col( fgcol.x, fgcol.y, fgcol.z, fgcol.w * opacity );
				GR2D_DrawTextLine( round((ax0+ax1)/2 - fabsf(ay0-ay1)/4), round((ay0+ay1)/2), CTRL.caption, HALIGN_RIGHT, VALIGN_CENTER );
				if( CTRL.type == CT_Slider )
				{
					float MG = fabsf(ay0-ay1)/4;
					float BR = MG / 4;
					float IMG = MG + BR * 2;
					float mx0 = (ax0+ax1) * 0.5f;
					br.Reset().Col( 0.8f, ctlalpha * opacity ).QuadFrame( mx0+MG, ay0+MG, ax1-MG, ay1-MG, mx0+MG+BR, ay0+MG+BR, ax1-MG-BR, ay1-MG-BR );
					br.Reset().Col( 0.8f, ctlalpha * opacity ).Quad( mx0+IMG, ay0+IMG, TLERP( mx0+IMG, ax1-IMG, CTRL.sl_value ), ay1-IMG );
					if( CTRL.value_text.size() )
					{
						GR2D_SetFont( "core", TMAX(0.0f, fabsf(ay0 - ay1) * 0.2f + (y1 - y0) / 50) * 0.6f );
						
						br.Flush();
						GR2D_SetScissorRect( mx0+IMG, ay0+IMG, TLERP( mx0+IMG, ax1-IMG, CTRL.sl_value ), ay1-IMG );
						br.Reset().Col( 0.0f, ctlalpha * opacity );
						GR2D_DrawTextLine( round(TLERP( ax0, ax1, 0.75f )), round((ay0+ay1)/2), CTRL.value_text, HALIGN_CENTER, VALIGN_CENTER );
						
						br.Flush();
						GR2D_SetScissorRect( TLERP( mx0+IMG, ax1-IMG, CTRL.sl_value ), ay0+IMG, ax1-IMG, ay1-IMG );
						br.Reset().Col( 0.8f, ctlalpha * opacity );
						GR2D_DrawTextLine( round(TLERP( ax0, ax1, 0.75f )), round((ay0+ay1)/2), CTRL.value_text, HALIGN_CENTER, VALIGN_CENTER );
						
						br.Flush();
						GR2D_UnsetScissorRect();
					}
				}
				else if( CTRL.type == CT_SwitchButton )
				{
					float fac = i == m_selected ? 0.6f : ( i == m_HL ? 0.9f : 0.8f );
					float MG = fabsf(ay0-ay1)/8;
					float BR = MG / 2;
					float mx0 = (ax0+ax1) * 0.5f;
					br.Reset().Col( fac, ctlalpha * opacity ).QuadFrame( mx0+MG, ay0+MG, ax1-MG, ay1-MG, mx0+MG+BR, ay0+MG+BR, ax1-MG-BR, ay1-MG-BR );
					GR2D_DrawTextLine( round(TLERP( ax0, ax1, 0.75f )), round((ay0+ay1)/2), CTRL.value_text, HALIGN_CENTER, VALIGN_CENTER );
				}
			}
			else
			{
				GR2D_SetFont( "core", (y1 - y0) / 30 );
				
				br.Reset().Col( fgcol.x, fgcol.y, fgcol.z, fgcol.w * opacity );
				GR2D_DrawTextLine( round((ax0+ax1)/2), round((ay0+ay1)/2), CTRL.caption, HALIGN_CENTER, VALIGN_CENTER );
			}
		}
	}
	void FindHL( float x, float y )
	{
		int oldhl = m_HL;
		m_HL = -1;
		for( int i = 0; i < (int) controls.size(); ++i )
		{
			Control& CTRL = controls[ i ];
			if( CTRL.x0 <= x && CTRL.x1 >= x &&
				CTRL.y0 <= y && CTRL.y1 >= y )
			{
				m_HL = i;
			}
		}
		if( m_HL >= 0 && m_HL != oldhl )
		{
			g_SoundSys->CreateEventInstance( "/ui/hover" )->Start();
		}
		m_lastHL = m_HL;
	}
	void RecalcSize( float w, float h, float aspect = 1.0f )
	{
		if( h == 0 )
			return;
		float screenasp = w / h;
		
		float tw = w;
		float th = h;
		if( aspect > screenasp ) // based on width
		{
			th = tw / aspect;
		}
		else // based on height
		{
			tw = th * aspect;
		}
		float mx = floor( ( w - tw ) / 2 );
		float my = floor( ( h - th ) / 2 );
		x0 = mx;
		y0 = my;
		x1 = x0 + tw;
		y1 = y0 + th;
	}
	void AddButton( const StringView& caption, float x0, float y0, float x1, float y1, int id = 0 )
	{
		controls.push_back( Control() );
		Control& ctrl = controls.last();
		ctrl.type = CT_Button;
		ctrl.style = CS_Menu;
		ctrl.caption = caption;
		ctrl.x0 = x0;
		ctrl.y0 = y0;
		ctrl.x1 = x1;
		ctrl.y1 = y1;
		ctrl.id = id;
	}
	void AddSwitchButton( const StringView& caption, float x0, float y0, float x1, float y1, int id = 0 )
	{
		controls.push_back( Control() );
		Control& ctrl = controls.last();
		ctrl.type = CT_SwitchButton;
		ctrl.style = CS_Menu;
		ctrl.caption = caption;
		ctrl.x0 = x0;
		ctrl.y0 = y0;
		ctrl.x1 = x1;
		ctrl.y1 = y1;
		ctrl.id = id;
	}
	void AddSlider( const StringView& caption, float x0, float y0, float x1, float y1, int id = 0 )
	{
		controls.push_back( Control() );
		Control& ctrl = controls.last();
		ctrl.type = CT_Slider;
		ctrl.style = CS_Menu;
		ctrl.caption = caption;
		ctrl.x0 = x0;
		ctrl.y0 = y0;
		ctrl.x1 = x1;
		ctrl.y1 = y1;
		ctrl.id = id;
	}
	
	float x0, y0, x1, y1;
	float opacity;
	Array< Control > controls;
	
	int m_HL;
	int m_lastHL;
	int m_selected;
	
	float m_mouseX;
	float m_mouseY;
};


struct QuestionScreen : IScreen
{
	QuestionScreen() : animFactor(0)
	{
		question = "What's it gonna be?";
		menu.AddButton( "No", 0.5f, 0.6f, 0.8f, 0.65f );
		menu.AddButton( "Yes", 0.2f, 0.6f, 0.5f, 0.65f );
	}
	void OnStart()
	{
		animFactor = 0.001f;
		animTarget = 1;
		menu.OnStart();
	}
	void OnEnd()
	{
		animFactor = 0;
		menu.OnEnd();
	}
	bool OnEvent( const Event& e )
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
	bool Draw( float delta )
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
	
	virtual bool Action( int mode )
	{
		LOG << "ACTION: " << mode;
		return true;
	}
	
	float animFactor;
	float animTarget;
	String question;
	ScreenMenu menu;
};

struct QuitGameQuestionScreen : QuestionScreen
{
	QuitGameQuestionScreen()
	{
		question = "Do you really want to quit the game?";
	}
	virtual bool Action( int mode )
	{
		if( mode == 1 )
			Game_End();
		return true;
	}
}
g_QuitGameQuestionScreen;


struct GraphicsOptionsMenuScreen : IScreen
{
	static int sort_displaymode_res( const void* a, const void* b )
	{
		DisplayMode* dm1 = (DisplayMode*) a;
		DisplayMode* dm2 = (DisplayMode*) b;
		if( dm1->width != dm2->width ) return dm1->width < dm2->width ? -1 : 1;
		if( dm1->height != dm2->height ) return dm1->height < dm2->height ? -1 : 1;
		return 0;
	}
	static int sort_refrate( const void* a, const void* b )
	{
		int rr1 = *(int*) a;
		int rr2 = *(int*) b;
		if( rr1 != rr2 ) return rr1 < rr2 ? -1 : 1;
		return 0;
	}
	
	int Slid2Mon( float s )
	{
		return TMAX( 0, TMIN( num_displays - 1, (int) round( s * ( num_displays - 1 ) ) ) );
	}
	StringView Slid2MonName( float s )
	{
		int d = Slid2Mon( s );
		static char bfr[ 512 ];
		sprintf( bfr, "%d/%d: %s", d + 1, num_displays, GR_GetDisplayName( d ) );
		return bfr;
	}
	StringView Opt2ModeName( int opt )
	{
		if( opt == FULLSCREEN_NONE ) return "Windowed";
		else if( opt == FULLSCREEN_NORMAL ) return "Fullscreen";
		else if( opt == FULLSCREEN_WINDOWED ) return "Windowed fullscreen";
		return "<unknown>";
	}
	int Slid2Reso( float s )
	{
		return TMAX( 0, TMIN( (int) resolutions.size() - 1, (int) round( s * ( resolutions.size() - 1 ) ) ) );
	}
	StringView Slid2ResoName( float s )
	{
		int which = Slid2Reso( s );
		static char bfr[ 32 ];
		sprintf( bfr, "%d x %d", resolutions[ which ].width, resolutions[ which ].height );
		return bfr;
	}
	int Slid2RefRate( float s )
	{
		return TMAX( 0, TMIN( (int) refrates.size() - 1, (int) round( s * ( refrates.size() - 1 ) ) ) );
	}
	StringView Slid2RefRateName( float s )
	{
		int which = Slid2RefRate( s );
		static char bfr[ 32 ];
		sprintf( bfr, "%d Hz", refrates[ which ] );
		return bfr;
	}
	StringView Opt2VSyncName( int opt ){ return opt ? "Enabled" : "Disabled"; }
	StringView Opt2AAModeName( int opt )
	{
		if( opt == 0 ) return "Disabled";
		else if( opt == 1 ) return "MSAA 2X";
		else if( opt == 2 ) return "MSAA 4X";
		else if( opt == 3 ) return "MSAA 8X";
		return "<unknown>";
	}
	void OnDisplayChange()
	{
		GR_ListDisplayModes( rndset.display, dispmodes );
		DisplayMode curr = { origset.width, origset.height, origset.refresh_rate };
		if( dispmodes.has( curr ) == false )
			dispmodes.push_back( curr );
		
		resolutions.clear();
		for( size_t i = 0; i < dispmodes.size(); ++i )
		{
			DisplayMode dm = { dispmodes[i].width, dispmodes[i].height, 0 };
			if( resolutions.has( dm ) == false )
				resolutions.push_back( dm );
		}
		qsort( resolutions.data(), resolutions.size(), sizeof(DisplayMode), sort_displaymode_res );
		
		for( size_t i = 0; i < resolutions.size(); ++i )
		{
			if( resolutions[ i ].width == rndset.width && resolutions[ i ].height == rndset.height )
			{
				menu.controls[2].sl_value = safe_fdiv( i, (float) ( resolutions.size() - 1 ) );
			}
		}
	}
	void OnResolutionChange()
	{
		rndset.width = resolutions[ Slid2Reso( menu.controls[2].sl_value ) ].width;
		rndset.height = resolutions[ Slid2Reso( menu.controls[2].sl_value ) ].height;
		menu.controls[2].value_text = Slid2ResoName( menu.controls[2].sl_value );
		
		refrates.clear();
		for( size_t i = 0; i < dispmodes.size(); ++i )
		{
			if( dispmodes[ i ].width == rndset.width && dispmodes[ i ].height == rndset.height )
			{
				refrates.push_back( dispmodes[ i ].refresh_rate );
			}
		}
		qsort( refrates.data(), refrates.size(), sizeof(int), sort_refrate );
		
		bool found = false;
		// look for same exact
		for( size_t i = 0; i < refrates.size(); ++i )
		{
			if( refrates[ i ] == wanted_refresh_rate )
			{
				menu.controls[3].sl_value = safe_fdiv( i, (float) ( refrates.size() - 1 ) );
				found = true;
				break;
			}
		}
		// look for 60+
		if( !found )
		{
			for( size_t i = 0; i < refrates.size(); ++i )
			{
				if( refrates[ i ] >= 60 )
				{
					menu.controls[3].sl_value = safe_fdiv( i, (float) ( refrates.size() - 1 ) );
					found = true;
					break;
				}
			}
		}
		// look under 60, backwards
		if( !found )
		{
			for( size_t i = 0; i < refrates.size(); ++i )
			{
				if( refrates[ i ] < 60 )
				{
					menu.controls[3].sl_value = safe_fdiv( i, (float) ( refrates.size() - 1 ) );
					found = true;
					break;
				}
			}
		}
		// WAT
		if( !found )
		{
			LOG << "No refresh rates? WTF!";
		}
		
		OnRefreshRateChange();
	}
	void OnRefreshRateChange()
	{
		rndset.refresh_rate = refrates[ Slid2RefRate( menu.controls[3].sl_value ) ];
		menu.controls[3].value_text = Slid2RefRateName( menu.controls[3].sl_value );
	}
	
	GraphicsOptionsMenuScreen() : num_displays(0), animFactor(0)
	{
		float y = 0.1f;
		menu.AddSwitchButton( "Mode", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		menu.AddSlider( "Display", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		menu.AddSlider( "Resolution", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		menu.AddSlider( "Refresh rate", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		menu.AddSwitchButton( "Vertical sync.", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		menu.AddSwitchButton( "Antialiasing", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		y = 0.9f;
		menu.AddButton( "Apply", 0.2f, y, 0.5f, y + 0.05f, 101 );
		menu.AddButton( "Back", 0.5f, y, 0.8f, y + 0.05f, 102 );
	}
	void OnStart()
	{
		GR_GetVideoMode( rndset );
		origset = rndset;
		num_displays = GR_GetDisplayCount();
		
		menu.controls[0].sb_option = rndset.fullscreen;
		menu.controls[0].value_text = Opt2ModeName( menu.controls[0].sb_option );
		
		menu.controls[1].sl_value = clamp( safe_fdiv( rndset.display, float( num_displays - 1 ) ), 0, 1 );
		menu.controls[1].value_text = Slid2MonName( menu.controls[1].sl_value );
		
		OnDisplayChange();
		
		OnResolutionChange();
		
		wanted_refresh_rate = rndset.refresh_rate;
		OnRefreshRateChange();
		
		menu.controls[4].sb_option = rndset.vsync ? 1 : 0;
		menu.controls[4].value_text = Opt2VSyncName( menu.controls[4].sb_option );
		
		menu.controls[5].sb_option = 0;
		if( rndset.aa_mode == ANTIALIAS_MULTISAMPLE )
		{
			switch( rndset.aa_quality )
			{
			case 2: menu.controls[5].sb_option = 1; break;
			case 4: menu.controls[5].sb_option = 2; break;
			case 8: menu.controls[5].sb_option = 3; break;
			}
		}
		menu.controls[5].value_text = Opt2AAModeName( menu.controls[5].sb_option );
		
		animFactor = 0.001f;
		animTarget = 1;
		menu.OnStart();
	}
	void OnEnd()
	{
		animFactor = 0;
		menu.OnEnd();
	}
	bool OnEvent( const Event& e )
	{
		if( animTarget != 1 )
			return false;
		
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
		{
			animTarget = 0;
		}
		
		int sel = menu.OnEvent( e );
		if( sel >= 0 )
		{
			if( sel == 0 )
			{
				menu.controls[sel].sb_option = ( menu.controls[sel].sb_option + 1 ) % 3;
				menu.controls[sel].value_text = Opt2ModeName( menu.controls[sel].sb_option );
				
				rndset.fullscreen = menu.controls[sel].sb_option;
			}
			else if( sel == 1 )
			{
				menu.controls[sel].value_text = Slid2MonName( menu.controls[sel].sl_value );
				rndset.display = TMAX( 0, TMIN( num_displays - 1, (int) round( menu.controls[sel].sl_value * ( num_displays - 1 ) ) ) );
				OnDisplayChange();
			}
			else if( sel == 2 )
			{
				OnResolutionChange();
			}
			else if( sel == 3 )
			{
				OnRefreshRateChange();
				wanted_refresh_rate = rndset.refresh_rate;
			}
			else if( sel == 4 )
			{
				menu.controls[sel].sb_option = ( menu.controls[sel].sb_option + 1 ) % 2;
				menu.controls[sel].value_text = Opt2VSyncName( menu.controls[sel].sb_option );
				
				rndset.vsync = menu.controls[sel].sb_option != 0;
			}
			else if( sel == 5 )
			{
				menu.controls[sel].sb_option = ( menu.controls[sel].sb_option + 1 ) % 4;
				menu.controls[sel].value_text = Opt2AAModeName( menu.controls[sel].sb_option );
				
				rndset.aa_mode = menu.controls[sel].sb_option > 0 ? ANTIALIAS_MULTISAMPLE : ANTIALIAS_NONE;
				static const int aaqmap[4] = { 0, 2, 4, 8 };
				rndset.aa_quality = aaqmap[ menu.controls[sel].sb_option ];
			}
			else
			{
				LOG << "GRAPHICS: " << rndset.width << "|" << rndset.height << "|" << rndset.refresh_rate <<
					"|" << rndset.fullscreen << "|" << rndset.vsync << "|" << rndset.aa_mode << "|" << rndset.aa_quality;
				if( menu.controls[sel].id == 101 )
				{
					GR_SetVideoMode( rndset );
					SaveConfig();
				}
				else
				{
				}
				animTarget = 0;
			}
		}
		return true;
	}
	bool Draw( float delta )
	{
		float diff = animTarget - animFactor;
		animFactor += sign( diff ) * TMIN( fabsf( diff ), delta * 2 );
		
		menu.RecalcSize( GR_GetWidth(), GR_GetHeight() );
		menu.opacity = smoothstep( animFactor );
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		br.Col( 0, 0.5f * menu.opacity );
		br.Quad( 0, menu.y0, GR_GetWidth(), GR_GetHeight() );
		
		menu.Draw( delta );
		return animFactor <= 0;
	}
	
	Array< DisplayMode > dispmodes;
	Array< DisplayMode > resolutions;
	Array< int > refrates;
	int num_displays;
	int wanted_refresh_rate;
	RenderSettings origset;
	RenderSettings rndset;
	
	float animFactor;
	float animTarget;
	ScreenMenu menu;
}
g_GraphicsOptionsMenu;


struct SoundOptionsMenuScreen : IScreen
{
	SoundOptionsMenuScreen() : animFactor(0)
	{
		float y = 0.2f;
		menu.AddSlider( "Master volume", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		menu.AddSlider( "Effect volume", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		menu.AddSlider( "Music volume", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		y = 0.6f;
		menu.AddButton( "Apply", 0.2f, y, 0.5f, y + 0.05f, 101 );
		menu.AddButton( "Back", 0.5f, y, 0.8f, y + 0.05f, 102 );
	}
	void OnStart()
	{
		vols[0] = g_SoundSys->GetVolume( "bus:/" );
		vols[1] = g_SoundSys->GetVolume( "bus:/sfx" );
		vols[2] = g_SoundSys->GetVolume( "bus:/music" );
		
		for( int i = 0; i < 3; ++i )
		{
			menu.controls[i].sl_value = vols[i];
		}
		
		animFactor = 0.001f;
		animTarget = 1;
		menu.OnStart();
	}
	void OnEnd()
	{
		animFactor = 0;
		menu.OnEnd();
	}
	bool OnEvent( const Event& e )
	{
		if( animTarget != 1 )
			return false;
		
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
		{
			animTarget = 0;
		}
		
		int sel = menu.OnEvent( e );
		if( sel >= 0 )
		{
			if( sel == 0 ) g_SoundSys->SetVolume( "bus:/", menu.controls[0].sl_value );
			else if( sel == 1 ) g_SoundSys->SetVolume( "bus:/sfx", menu.controls[1].sl_value );
			else if( sel == 2 ) g_SoundSys->SetVolume( "bus:/music", menu.controls[2].sl_value );
			else
			{
				if( menu.controls[sel].id == 101 )
				{
					SaveConfig();
				}
				else
				{
					g_SoundSys->SetVolume( "bus:/", vols[0] );
					g_SoundSys->SetVolume( "bus:/sfx", vols[1] );
					g_SoundSys->SetVolume( "bus:/music", vols[2] );
				}
				animTarget = 0;
			}
		}
		return true;
	}
	bool Draw( float delta )
	{
		float diff = animTarget - animFactor;
		animFactor += sign( diff ) * TMIN( fabsf( diff ), delta * 2 );
		
		menu.RecalcSize( GR_GetWidth(), GR_GetHeight() );
		menu.opacity = smoothstep( animFactor );
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		br.Col( 0, 0.5f * menu.opacity );
		br.Quad( 0, menu.y0, GR_GetWidth(), GR_GetHeight() );
		
		menu.Draw( delta );
		return animFactor <= 0;
	}
	
	float vols[3];
	float animFactor;
	float animTarget;
	ScreenMenu menu;
}
g_SoundOptionsMenu;


InputState* g_ctrls[] =
{
	&MOVE_UP,
	&MOVE_DOWN,
	&MOVE_LEFT,
	&MOVE_RIGHT,
	&INTERACT,
	&THROW_COIN,
	&SPRINT,
	&SHOW_OBJECTIVES,
};
ActionInput* g_ctrl_ai[] =
{
	&g_i_move_up,
	&g_i_move_down,
	&g_i_move_left,
	&g_i_move_right,
	&g_i_interact,
	&g_i_throw_coin,
	&g_i_sprint,
	&g_i_show_objectives,
};
#define g_num_ctrls int(sizeof(g_ctrls)/sizeof(g_ctrls[0]))
struct ControlOptionsMenuScreen : IScreen
{
	static float Sens2Slid( float s ){ return ( logf( s ) / logf( 10 ) + 1 ) / 2; }
	static float Slid2Sens( float s ){ return pow( 10.0f, s * 2 - 1 ); }
	static StringView Slid2SensText( float s )
	{
		s = Slid2Sens( s );
		static char bfr[ 128 ];
		sprintf( bfr, "%.2g", s );
		return bfr;
	}
	ControlOptionsMenuScreen() : animFactor(0)
	{
		float y = 0.2f;
		menu.AddSwitchButton( "Move forward", 0.0f, y, 0.5f, y + 0.05f ); y += 0.05f;
		menu.AddSwitchButton( "Move backward", 0.0f, y, 0.5f, y + 0.05f ); y += 0.05f;
		menu.AddSwitchButton( "Move left", 0.0f, y, 0.5f, y + 0.05f ); y += 0.05f;
		menu.AddSwitchButton( "Move right", 0.0f, y, 0.5f, y + 0.05f ); y += 0.05f;
		y = 0.2f;
		menu.AddSwitchButton( "Interact", 0.5f, y, 1.0f, y + 0.05f ); y += 0.05f;
		menu.AddSwitchButton( "Throw a coin", 0.5f, y, 1.0f, y + 0.05f ); y += 0.05f;
		menu.AddSwitchButton( "Move faster", 0.5f, y, 1.0f, y + 0.05f ); y += 0.05f;
		menu.AddSwitchButton( "Show objectives", 0.5f, y, 1.0f, y + 0.05f ); y += 0.05f;
		y = 0.4f;
		menu.AddSlider( "Mouse sensitivity", 0.0f, y, 1.0f, y + 0.1f ); y += 0.1f;
		y = 0.5f;
		menu.AddSwitchButton( "Invert X", 0.0f, y, 0.5f, y + 0.05f );
		menu.AddSwitchButton( "Invert Y", 0.5f, y, 1.0f, y + 0.05f );
		y = 0.6f;
		menu.AddButton( "Apply", 0.2f, y, 0.5f, y + 0.05f, 101 );
		menu.AddButton( "Back", 0.5f, y, 0.8f, y + 0.05f, 102 );
	}
	void OnStart()
	{
		for( int i = 0; i < g_num_ctrls; ++i )
		{
			menu.controls[i].sb_option = Game_GetActionBinding( g_ctrls[i] );
			menu.controls[i].value_text = Game_GetInputName( menu.controls[i].sb_option );
		}
		menu.controls[8].sl_value = Sens2Slid( g_i_mouse_sensitivity );
		menu.controls[8].value_text = Slid2SensText( menu.controls[8].sl_value );
		menu.controls[9].sb_option = g_i_mouse_invert_x ? 1 : 0;
		menu.controls[9].value_text = menu.controls[9].sb_option ? "Yes" : "No";
		menu.controls[10].sb_option = g_i_mouse_invert_y ? 1 : 0;
		menu.controls[10].value_text = menu.controls[10].sb_option ? "Yes" : "No";
		
		keyAnimFactor = 0;
		keyAnimTarget = 0;
		keyToEdit = -1;
		animFactor = 0.001f;
		animTarget = 1;
		menu.OnStart();
	}
	void OnEnd()
	{
		animFactor = 0;
		menu.OnEnd();
	}
	void SetMapping( uint64_t iid )
	{
		for( int i = 0; i < g_num_ctrls; ++i )
		{
			if( i == keyToEdit )
				continue;
			if( menu.controls[ i ].sb_option == iid )
			{
				menu.controls[ i ].sb_option = ACTINPUT_UNASSIGNED;
				menu.controls[ i ].value_text = Game_GetInputName( ACTINPUT_UNASSIGNED );
			}
		}
		menu.controls[ keyToEdit ].sb_option = iid;
		menu.controls[ keyToEdit ].value_text = Game_GetInputName( iid );
	}
	bool OnEvent( const Event& e )
	{
		if( keyAnimTarget )
		{
			if( e.type == SDL_KEYDOWN )
			{
				if( e.key.keysym.sym != SDLK_ESCAPE )
				{
					SetMapping( ACTINPUT_MAKE_KEY( e.key.keysym.sym ) );
					menu.controls[ keyToEdit ].sb_option = ACTINPUT_MAKE_KEY( e.key.keysym.sym );
					menu.controls[ keyToEdit ].value_text = Game_GetInputName( menu.controls[ keyToEdit ].sb_option );
				}
				keyAnimTarget = 0;
			}
			if( e.type == SDL_MOUSEBUTTONDOWN )
			{
				SetMapping( ACTINPUT_MAKE_MOUSE( e.button.button ) );
				menu.controls[ keyToEdit ].sb_option = ACTINPUT_MAKE_MOUSE( e.button.button );
				menu.controls[ keyToEdit ].value_text = Game_GetInputName( menu.controls[ keyToEdit ].sb_option );
				keyAnimTarget = 0;
			}
			return true;
		}
		
		if( animTarget != 1 )
			return false;
		
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
		{
			animTarget = 0;
		}
		
		int sel = menu.OnEvent( e );
		if( sel >= 0 )
		{
			if( sel >= 0 && sel <= 7 )
			{
				menu.m_HL = -1;
				keyToEdit = sel;
				keyAnimTarget = 1;
			}
			else if( sel == 8 )
			{
				menu.controls[8].value_text = Slid2SensText( menu.controls[8].sl_value );
			}
			else if( sel == 9 || sel == 10 )
			{
				menu.controls[sel].sb_option = ( menu.controls[sel].sb_option + 1 ) % 2;
				menu.controls[sel].value_text = menu.controls[sel].sb_option ? "Yes" : "No";
			}
			else
			{
				if( menu.controls[sel].id == 101 )
				{
					g_i_mouse_sensitivity = Slid2Sens( menu.controls[8].sl_value );
					g_i_mouse_invert_x = menu.controls[9].sb_option != 0;
					g_i_mouse_invert_y = menu.controls[10].sb_option != 0;
					for( int i = 0; i < g_num_ctrls; ++i )
					{
						Game_UnbindInput( *g_ctrl_ai[ i ] );
					}
					for( int i = 0; i < g_num_ctrls; ++i )
					{
						*g_ctrl_ai[ i ] = menu.controls[ i ].sb_option;
						Game_BindInputToAction( menu.controls[ i ].sb_option, g_ctrls[ i ] );
					}
					SaveConfig();
				}
				else
				{
				}
				animTarget = 0;
			}
		}
		return true;
	}
	bool Draw( float delta )
	{
		// key setting animation
		{
			float diff = keyAnimTarget - keyAnimFactor;
			keyAnimFactor += sign( diff ) * TMIN( fabsf( diff ), delta * 4 );
		}
		// global animation
		{
			float diff = animTarget - animFactor;
			animFactor += sign( diff ) * TMIN( fabsf( diff ), delta * 2 );
		}
		
		menu.RecalcSize( GR_GetWidth(), GR_GetHeight() );
		menu.opacity = smoothstep( animFactor );
		
		if( menu.y0 < GR_GetHeight() )
		{
			BatchRenderer& br = GR2D_GetBatchRenderer();
			br.Reset();
			br.Col( 0, 0.5f * menu.opacity );
			br.Quad( 0, menu.y0, GR_GetWidth(), GR_GetHeight() );
		}
		
		menu.Draw( delta );
		
		if( keyAnimFactor )
		{
			float ix0 = TLERP( menu.x0, menu.x1, menu.controls[ keyToEdit ].x0 );
			float ix1 = TLERP( menu.x0, menu.x1, menu.controls[ keyToEdit ].x1 );
			float iy0 = TLERP( menu.y0, menu.y1, menu.controls[ keyToEdit ].y0 );
			float iy1 = TLERP( menu.y0, menu.y1, menu.controls[ keyToEdit ].y1 );
			float ox0 = 0;
			float ox1 = GR_GetWidth();
			float oy0 = 0;
			float oy1 = GR_GetHeight();
		//	float maxext = TMAX( TMAX( fabsf( ox0 - ix0 ), fabsf( ox1 - ix1 ) ), TMAX( fabsf( oy0 - iy0 ), fabsf( oy1 - iy1 ) ) );
		//	ox0 = ix0 - maxext;
		//	ox1 = ix1 + maxext;
		//	oy0 = iy0 - maxext;
		//	oy1 = iy1 + maxext;
		//	ox0 = TLERP( ix0, ox0, keyAnimFactor );
		//	ox1 = TLERP( ix1, ox1, keyAnimFactor );
		//	oy0 = TLERP( iy0, oy0, keyAnimFactor );
		//	oy1 = TLERP( iy1, oy1, keyAnimFactor );
			BatchRenderer& br = GR2D_GetBatchRenderer();
			br.Reset();
			br.Col( 0, 0.5f * menu.opacity * keyAnimFactor );
			br.QuadFrame( ox0, oy0, ox1, oy1, ix0, iy0, ix1, iy1 );
			
			GR2D_SetFont( "core", (menu.y1 - menu.y0) / 30 );
			
			br.Reset().Col( 0.9f, menu.opacity * keyAnimFactor );
			GR2D_DrawTextLine( round(TLERP(menu.x0,menu.x1,0.5f)), round(TLERP(menu.y0,menu.y1,0.1f)), "Press key/button to assign or <Escape> to cancel", HALIGN_CENTER, VALIGN_CENTER );
		}
		
		return animFactor <= 0;
	}
	
	float animFactor;
	float animTarget;
	float keyAnimFactor;
	float keyAnimTarget;
	int keyToEdit;
	ScreenMenu menu;
}
g_ControlOptionsMenu;


struct OptionsMenuScreen : IScreen
{
	OptionsMenuScreen() : animFactor(0)
	{
		float bm = 0.05f;
		int bc = 4;
		float bsz = ( 1.0f - ( bc + 1 ) * bm ) / bc;
		float x = bm;
		menu.AddButton( "Graphics", x, 0.7f, x + bsz, 0.8f ); x += bsz + bm;
		menu.AddButton( "Sound", x, 0.7f, x + bsz, 0.8f ); x += bsz + bm;
		menu.AddButton( "Controls", x, 0.7f, x + bsz, 0.8f ); x += bsz + bm;
		menu.AddButton( "Back", x, 0.7f, x + bsz, 0.8f ); x += bsz + bm;
	}
	void OnStart()
	{
		animFactor = 0.001f;
		animTarget = 1;
		menu.OnStart();
	}
	void OnEnd()
	{
		animFactor = 0;
		menu.OnEnd();
	}
	bool OnEvent( const Event& e )
	{
		if( animTarget != 1 )
			return false;
		
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
		{
			animTarget = 0;
		}
		
		int sel = menu.OnEvent( e );
		if( sel >= 0 )
		{
			if( sel == 0 ){ Game_AddOverlayScreen( &g_GraphicsOptionsMenu ); menu.m_HL = -1; }
			else if( sel == 1 ){ Game_AddOverlayScreen( &g_SoundOptionsMenu ); menu.m_HL = -1; }
			else if( sel == 2 ){ Game_AddOverlayScreen( &g_ControlOptionsMenu ); menu.m_HL = -1; }
			else if( sel == 3 ){ animTarget = 0; }
		}
		return true;
	}
	bool Draw( float delta )
	{
		float diff = animTarget - animFactor;
		animFactor += sign( diff ) * TMIN( fabsf( diff ), delta * 2 );
		
		menu.RecalcSize( GR_GetWidth(), GR_GetHeight() );
		menu.y0 += GR_GetHeight() * smoothstep( 1 - animFactor ) * 0.3f;
		menu.y1 += GR_GetHeight() * smoothstep( 1 - animFactor ) * 0.3f;
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		br.Col( 0, 0.5f );
		br.Quad( 0, TLERP( menu.y0, menu.y1, 0.7f ), GR_GetWidth(), GR_GetHeight() );
		
		menu.Draw( delta );
		return animFactor <= 0;
	}
	
	float animFactor;
	float animTarget;
	ScreenMenu menu;
}
g_OptionsMenu;


struct PauseMenuScreen : IScreen
{
	PauseMenuScreen() : notfirst(false)
	{
		float bm = 0.05f;
		int bc = 3;
		float bsz = ( 1.0f - ( bc + 1 ) * bm ) / bc;
		float x = bm;
		menu.AddButton( "Resume", x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
		menu.AddButton( "Options", x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
		menu.AddButton( "Exit", x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
	}
	void OnStart()
	{
		menu.OnStart();
		Game_ShowCursor( true );
		notfirst = false;
		g_GameLevel->m_paused = true;
		resetcontrols();
	}
	void OnEnd()
	{
		menu.OnEnd();
		Game_ShowCursor( false );
		g_GameLevel->m_paused = false;
	}
	void Unpause()
	{
		Game_RemoveOverlayScreen( this );
		SHOW_OBJECTIVES.value = 0;
	}
	bool OnEvent( const Event& e )
	{
		if( notfirst && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
		{
			Unpause();
			return true;
		}
		
		int sel = menu.OnEvent( e );
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
				Game_AddOverlayScreen( &g_OptionsMenu );
				menu.m_HL = -1;
			}
			else if( sel == 2 )
			{
				Game_AddOverlayScreen( &g_QuitGameQuestionScreen );
				menu.m_HL = -1;
			}
		}
		
		notfirst = true;
		return true;
	}
	bool Draw( float delta )
	{
		SHOW_OBJECTIVES.value = 1;
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		br.Col( 0, 0.5f );
		br.Quad( 0, TLERP( menu.y0, menu.y1, 0.8f ), GR_GetWidth(), GR_GetHeight() );
		
		menu.RecalcSize( GR_GetWidth(), GR_GetHeight() );
		
		br.Reset();
		br.Col( 0.8f, menu.opacity );
		GR2D_SetFont( "core", (menu.y1 - menu.y0) / 30 );
		GR2D_DrawTextLine( TLERP(menu.x0,menu.x1,0.5f), TLERP(menu.y0,menu.y1,0.3f), "- PAUSED -", HALIGN_CENTER, VALIGN_CENTER );
		
		menu.Draw( delta );
		return false;
	}
	
	bool notfirst;
	ScreenMenu menu;
}
g_PauseMenu;


struct MainMenuScreen : IScreen
{
	MainMenuScreen()
	{
		float bm = 0.05f;
		int bc = 3;
		float bsz = ( 1.0f - ( bc + 1 ) * bm ) / bc;
		float x = bm;
		menu.AddButton( "New game", x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
		menu.AddButton( "Options", x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
		menu.AddButton( "Exit", x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
	}
	void OnStart()
	{
		m_tx_logo = GR_GetTexture( "ui/logo.png" );
		menu.OnStart();
	}
	void OnEnd()
	{
		m_tx_logo = NULL;
		menu.OnEnd();
	}
	bool OnEvent( const Event& e )
	{
		int sel = menu.OnEvent( e );
		if( sel >= 0 )
		{
			if( sel == 0 )
			{
				// start the level
			//	g_GameLevel->StartLevel();
				Game_RemoveOverlayScreen( this );
				Game_ShowCursor( false );
				return true;
			}
			else if( sel == 1 )
			{
				Game_AddOverlayScreen( &g_OptionsMenu );
				menu.m_HL = -1;
			}
			else if( sel == 2 )
			{
				Game_AddOverlayScreen( &g_QuitGameQuestionScreen );
				menu.m_HL = -1;
			}
		}
		return true;
	}
	bool Draw( float delta )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		br.Col( 0, 0.5f );
		br.Quad( 0, TLERP( menu.y0, menu.y1, 0.8f ), GR_GetWidth(), GR_GetHeight() );
		
		br.Reset();
		br.SetTexture( m_tx_logo );
		br.Quad( TLERP( menu.x0, menu.x1, 0.0f ), TLERP( menu.y0, menu.y1, 0.2f ), TLERP( menu.x0, menu.x1, 1.0f ), TLERP( menu.y0, menu.y1, 0.7f ) );
		
		menu.RecalcSize( GR_GetWidth(), GR_GetHeight() );
		
		menu.Draw( delta );
		return false;
	}
	
	TextureHandle m_tx_logo;
	ScreenMenu menu;
}
g_MainMenu;


struct EndMenuScreen : IScreen
{
	EndMenuScreen() : animFactor(0)
	{
		float bm = 0.05f;
		int bc = 2;
		float bsz = ( 1.0f - ( bc + 1 ) * bm ) / bc;
		float x = bm;
		menu.AddButton( "Restart the level", x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
		menu.AddButton( "Exit", x, 0.8f, x + bsz, 0.9f ); x += bsz + bm;
	}
	void OnStart()
	{
		animFactor = 0.001f;
		animTarget = 1;
		menu.OnStart();
		g_GameLevel->m_paused = true;
	}
	void OnEnd()
	{
		animFactor = 0;
		menu.OnEnd();
		g_GameLevel->m_paused = false;
	}
	bool OnEvent( const Event& e )
	{
		int sel = menu.OnEvent( e );
		if( sel >= 0 )
		{
			if( sel == 0 )
			{
				resetcontrols();
				Game_RemoveOverlayScreen( this );
				GR2D_SetFont( "core", TMIN(GR_GetWidth(),GR_GetHeight())/20 );
				g_GameLevel->Load( "office" );
			//	g_GameLevel->StartLevel();
				Game_ShowCursor( false );
				return true;
			}
			else if( sel == 1 ){ Game_End(); }
		}
		return true;
	}
	bool Draw( float delta )
	{
		float diff = animTarget - animFactor;
		animFactor += sign( diff ) * TMIN( fabsf( diff ), delta * 2 );
		
		menu.RecalcSize( GR_GetWidth(), GR_GetHeight() );
		menu.y0 += GR_GetHeight() * smoothstep( 1 - animFactor ) * 0.3f;
		menu.y1 += GR_GetHeight() * smoothstep( 1 - animFactor ) * 0.3f;
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		br.Col( 0, 0.5f );
		br.Quad( 0, TLERP( menu.y0, menu.y1, 0.7f ), GR_GetWidth(), GR_GetHeight() );
		
		br.Flush();
		GR2D_SetScissorRect( 0, TLERP( menu.y0, menu.y1, 0.4f ), GR_GetWidth(), TLERP( menu.y0, menu.y1, 0.5f ) );
		
		br.Reset();
		br.Col( 0, 0.5f );
		br.Quad( 0, TLERP( menu.y0, menu.y1, 0.4f ), GR_GetWidth(), TLERP( menu.y0, menu.y1, 0.5f ) );
		br.Reset();
		br.Col( g_endcol.x, g_endcol.y, g_endcol.z, 1 );
		GR2D_SetFont( "core", (menu.y1 - menu.y0) / 30 );
		GR2D_DrawTextLine( TLERP( menu.x0, menu.x1, 0.5f ), TLERP( menu.y0, menu.y1, 0.45f ), g_end_reason, HALIGN_CENTER, VALIGN_CENTER );
		
		br.Flush();
		GR2D_UnsetScissorRect();
		
		menu.Draw( delta );
		return animFactor <= 0;
	}
	
	float animFactor;
	float animTarget;
	ScreenMenu menu;
}
g_EndMenu;





static int SetSuspicion( SGS_CTX )
{
	SGSFN( "SetSuspicion" );
	g_suspicion = sgs_GetVar<float>()( C, 0 );
	return 0;
}

static int SetShowMoney( SGS_CTX )
{
	SGSFN( "SetShowMoney" );
	g_show_money = sgs_GetVar<bool>()( C, 0 );
	g_money = sgs_GetVar<float>()( C, 1 );
	return 0;
}

static int SetShowSuspicion( SGS_CTX )
{
	SGSFN( "SetShowSuspicion" );
	g_show_suspicion = sgs_GetVar<bool>()( C, 0 );
	return 0;
}

static int EndGame( SGS_CTX )
{
	SGSFN( "EndGame" );
	g_end_reason = sgs_GetVar<StringView>()( C, 0 );
	g_endcol = sgs_GetVar<bool>()( C, 1 ) ? V3(0.2f, 0.7f, 0.1f) : V3(0.7f, 0.1f, 0.0f);
	if( !Game_HasOverlayScreens() )
	{
		Game_AddOverlayScreen( &g_EndMenu );
		Game_ShowCursor( true );
	}
	return 0;
}

static sgs_RegFuncConst gamerfc[] =
{
	{ "SetSuspicion", SetSuspicion },
	{ "SetShowMoney", SetShowMoney },
	{ "SetShowSuspicion", SetShowSuspicion },
	{ "EndGame", EndGame },
	{ NULL, NULL },
};

void SetupLevel()
{
	sgs_RegFuncConsts( g_GameLevel->m_scriptCtx.C, gamerfc, -1 );
}




#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct OfficeTheftGame : IGame
{
	OfficeTheftGame() : m_accum( 0.0f ), m_lastFrameReset( false )
	{
	}
	
	bool OnConfigure( int argc, char** argv )
	{
		RenderSettings rs = { 0, 1024, 576, 60, FULLSCREEN_NONE, true, ANTIALIAS_NONE, 4 };
		
		InLocalStorage ils( "CrageGames/OfficeTheftGame" );
		
		String configdata;
		if( LoadTextFile( "config.cfg", configdata ) )
		{
			StringView key, value;
			ConfigReader cr( configdata );
			while( cr.Read( key, value ) )
			{
				if(0);
				// GRAPHICS
				else if( key == "r_display" ) rs.display = TMAX( 0, TMIN( GR_GetDisplayCount() - 1, (int) String_ParseInt( value ) ) );
				else if( key == "r_width" ) rs.width = TMAX( 1, TMIN( 16384, (int) String_ParseInt( value ) ) );
				else if( key == "r_height" ) rs.height = TMAX( 1, TMIN( 16384, (int) String_ParseInt( value ) ) );
				else if( key == "r_refresh_rate" ) rs.refresh_rate = TMAX( 1, TMIN( 1200, (int) String_ParseInt( value ) ) );
				else if( key == "r_fullscreen" ) rs.fullscreen = TMAX( 0, TMIN( 2, (int) String_ParseInt( value ) ) );
				else if( key == "r_vsync" ) rs.vsync = String_ParseBool( value );
				else if( key == "r_antialias" )
				{
					if( value == "none" );
					else if( value == "msaa2x" )
					{
						rs.aa_mode = ANTIALIAS_MULTISAMPLE;
						rs.aa_quality = 2;
					}
					else if( value == "msaa4x" )
					{
						rs.aa_mode = ANTIALIAS_MULTISAMPLE;
						rs.aa_quality = 4;
					}
					else if( value == "msaa8x" )
					{
						rs.aa_mode = ANTIALIAS_MULTISAMPLE;
						rs.aa_quality = 8;
					}
				}
				// SOUND
				else if( key == "s_vol_master" ) g_s_vol_master = clamp( String_ParseFloat( value ), 0, 1 );
				else if( key == "s_vol_music" ) g_s_vol_music = clamp( String_ParseFloat( value ), 0, 1 );
				else if( key == "s_vol_sfx" ) g_s_vol_sfx = clamp( String_ParseFloat( value ), 0, 1 );
				// INPUT
				else if( key == "i_move_left"       ) g_i_move_left       = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_move_right"      ) g_i_move_right      = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_move_up"         ) g_i_move_up         = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_move_down"       ) g_i_move_down       = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_interact"        ) g_i_interact        = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_throw_coin"      ) g_i_throw_coin      = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_sprint"          ) g_i_sprint          = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_show_objectives" ) g_i_show_objectives = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_mouse_sensitivity" ) g_i_mouse_sensitivity = String_ParseFloat( value );
				else if( key == "i_mouse_invert_x" ) g_i_mouse_invert_x = String_ParseBool( value );
				else if( key == "i_mouse_invert_y" ) g_i_mouse_invert_y = String_ParseBool( value );
			}
		}
		
		// TODO override with command line params
		
		GR_SetVideoMode( rs );
		
		return true;
	}
	
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		
		g_SoundSys = SND_CreateSystem();
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &INTERACT );
		Game_RegisterAction( &THROW_COIN );
		Game_RegisterAction( &SPRINT );
		Game_RegisterAction( &SHOW_OBJECTIVES );
		
		Game_BindInputToAction( g_i_move_left, &MOVE_LEFT );
		Game_BindInputToAction( g_i_move_right, &MOVE_RIGHT );
		Game_BindInputToAction( g_i_move_up, &MOVE_UP );
		Game_BindInputToAction( g_i_move_down, &MOVE_DOWN );
		Game_BindInputToAction( g_i_interact, &INTERACT );
		Game_BindInputToAction( g_i_throw_coin, &THROW_COIN );
		Game_BindInputToAction( g_i_sprint, &SPRINT );
		Game_BindInputToAction( g_i_show_objectives, &SHOW_OBJECTIVES );
		
	//	GR_LoadAnims( "meshes/tstest.ssm.anm" );
		GR_LoadAnims( "meshes/female1.ssm.anm" );
		
		g_SoundSys->Load( "sound/master.bank" );
		g_SoundSys->Load( "sound/master.strings.bank" );
		
		g_SoundSys->SetVolume( "bus:/", g_s_vol_master );
		g_SoundSys->SetVolume( "bus:/music", g_s_vol_music );
		g_SoundSys->SetVolume( "bus:/sfx", g_s_vol_sfx );
		
	//	m_music = g_SoundSys->CreateEventInstance( "/lev1_music" );
	//	m_music->Start();
		
		g_GameLevel = new GameLevel( PHY_CreateWorld() );
		g_GameLevel->GetPhyWorld()->SetGravity( V3( 0, 0, -9.81f ) );
		AddSystemToLevel<InfoEmissionSystem>( g_GameLevel );
		AddSystemToLevel<MessagingSystem>( g_GameLevel );
		AddSystemToLevel<ObjectiveSystem>( g_GameLevel );
		AddSystemToLevel<FlareSystem>( g_GameLevel );
		AddSystemToLevel<LevelCoreSystem>( g_GameLevel );
		AddSystemToLevel<ScriptedSequenceSystem>( g_GameLevel );
		AddSystemToLevel<MusicSystem>( g_GameLevel );
		AddSystemToLevel<AIDBSystem>( g_GameLevel );
		SetupLevel();
		
		GR2D_SetFont( "core", TMIN(GR_GetWidth(),GR_GetHeight())/20 );
		g_GameLevel->Load( "office" );
		g_GameLevel->Tick( 0, 0 );
		
		Game_AddOverlayScreen( &g_MainMenu );
		Game_AddOverlayScreen( &g_SplashScreen );
		
		return true;
	}
	void OnDestroy()
	{
		Game_RemoveAllOverlayScreens();
		
		delete g_GameLevel;
		g_GameLevel = NULL;
		
	//	m_music->Stop();
	//	m_music = NULL;
		
		g_SoundSys = NULL;
	}
	
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST )
		{
			m_lastFrameReset = false;
			Game_ShowCursor( true );
		}
		if( e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED )
		{
			if( Game_HasOverlayScreens() == false )
				Game_ShowCursor( false );
		}
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
		{
			if( Game_HasOverlayScreens() == false )
				Game_AddOverlayScreen( &g_PauseMenu );
		}
	}
	
	void Game_FixedTick( float dt )
	{
		g_GameLevel->FixedTick( dt );
	}
	void Game_Tick( float dt, float bf )
	{
		g_GameLevel->Tick( dt, bf );
	}
	void Game_Render()
	{
		g_GameLevel->Draw();
		g_GameLevel->Draw2D();
		
		// MONEY
		int w = GR_GetWidth(), h = GR_GetHeight();
		float minw = TMIN( w, h );
		char bfr[1024];
		
		if( g_show_money )
		{
			sgrx_snprintf( bfr, 1024, "Money stolen: $%.2f", g_money );
			GR2D_SetFont( "core", minw / 20 );
			GR2D_SetColor( 0, 1 );
			GR2D_DrawTextLine( round(minw/10)+1, h - round(minw/10)+1, bfr, HALIGN_LEFT, VALIGN_BOTTOM );
			GR2D_SetColor( 1, 1 );
			GR2D_DrawTextLine( round(minw/10), h - round(minw/10), bfr, HALIGN_LEFT, VALIGN_BOTTOM );
		}
		
		if( g_show_suspicion )
		{
			float susp = clamp( g_suspicion, 0, 1 );
			BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
			br.Col( 0, 0.5f );
			br.AARect( w - round(minw/2)+1, h - round(minw/7)+1,
				w - round(minw/10)+1, h - round(minw/10)+1 );
			if( susp )
			{
				br.Col( 1, 0.8f );
				br.AARect( w - round(minw/2), h - round(minw/7),
					TLERP( w - round(minw/2), w - round(minw/10), susp ), h - round(minw/10) );
			}
			GR2D_SetColor( 0.5f, 1 );
			GR2D_DrawTextLine( w - round(minw/10), h - round(minw/10), "SUSPICION", HALIGN_RIGHT, VALIGN_BOTTOM );
		}
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
		g_SoundSys->Update();
		
		if( g_GameLevel->m_player )
		{
			static_cast<LD33Player*>(g_GameLevel->m_player)->inCursorMove = V2(0);
			if( Game_HasOverlayScreens() == false )
			{
				Vec2 cpos = Game_GetCursorPos();
				Game_SetCursorPos( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 opos = V2( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 curmove = cpos - opos;
				if( m_lastFrameReset )
					static_cast<LD33Player*>(g_GameLevel->m_player)->inCursorMove = curmove * V2( g_i_mouse_invert_x ? -1 : 1, g_i_mouse_invert_y ? -1 : 1 ) * g_i_mouse_sensitivity;
				m_lastFrameReset = true;
			}
			else
				m_lastFrameReset = false;
		}
		
		if( dt > MAX_TICK_SIZE )
			dt = MAX_TICK_SIZE;
		
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
	bool m_lastFrameReset;
	
	SoundEventInstanceHandle m_music;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

