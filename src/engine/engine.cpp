

#include <stdio.h>
#include <time.h>
#ifdef __MINGW32__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif
#include <windows.h>

#define INCLUDE_REAL_SDL
#include "engine_int.hpp"
#include "renderer.hpp"


void SGRX_INT_InitResourceTables();
void SGRX_INT_DestroyResourceTables();
void SGRX_INT_UnpreserveResources();
void SGRX_INT_InitBatchRendering();
void SGRX_INT_DestroyBatchRendering();


uint32_t GetTimeMsec()
{
#ifdef __linux
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC, &ts );
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
	clock_t clk = clock();
	return clk * 1000 / CLOCKS_PER_SEC;
#endif
}

void Thread_Sleep( uint32_t msec )
{
	Sleep( msec );
}


void Sys_FatalError( const StringView& err )
{
	LOG_ERROR << LOG_DATE << "  " << err;
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "SGRX Engine", StackString<4096>(err), NULL );
	exit( EXIT_FAILURE );
}


//
// GLOBALS
//

struct EventLinkEntry
{
	bool operator == ( const EventLinkEntry& o ) const { return eid == o.eid && eh == o.eh; }
	SGRX_EventID eid;
	SGRX_IEventHandler* eh;
};
struct EventLinkArray : Array< EventLinkEntry >, SGRX_RefCounted
{
};
typedef Handle< EventLinkArray > EventLinkArrayHandle;

typedef HashTable< SGRX_EventID, EventLinkArrayHandle > EventLinksByID;
typedef HashTable< SGRX_IEventHandler*, EventLinkArrayHandle > EventLinksByHandler;
typedef HashTable< int, JoystickHandle > JoystickHashTable;
typedef HashTable< StringView, CObj* > CObjMap;

static String g_GameLibName = "game";
static String g_GameDir = ".";
static String g_GameDir2 = "";
static String g_RendererName = "d3d11";

static bool g_Running = true;
static bool g_HasFocus = true;
static bool g_WindowVisible = true;
static SDL_Window* g_Window = NULL;
static void* g_GameLib = NULL;
IGame* g_Game = NULL;
static uint32_t g_GameTime = 0;
static ActionMap* g_ActionMap;
static CObjMap* g_CObjs;
static Vec2 g_CursorPos = {0,0};
static Vec2 g_CursorScale = {0,0};
static EventLinksByID g_EventLinksByID;
static EventLinksByHandler g_EventLinksByHandler;
static Array< IScreen* > g_OverlayScreens;
static Array< FileSysHandle > g_FileSystems;

static RenderSettings g_RenderSettings = { 0, 1280, 720, 60, FULLSCREEN_NONE, true, ANTIALIAS_NONE, 0 };
static const char* g_RendererPrefix = "sgrx-render-";
static void* g_RenderLib = NULL;
static pfnRndInitialize g_RfnInitialize = NULL;
static pfnRndFree g_RfnFree = NULL;
static pfnRndCreateRenderer g_RfnCreateRenderer = NULL;
IRenderer* g_Renderer = NULL;
static JoystickHashTable* g_Joysticks = NULL;

extern BatchRenderer* g_BatchRenderer;


CVarInt gcv_r_display( "r_display" );
CVarInt gcv_r_width( "r_width" );
CVarInt gcv_r_height( "r_height" );
CVarInt gcv_r_refresh_rate( "r_refresh_rate" );
CVarInt gcv_r_fullscreen( "r_fullscreen" );
CVarBool gcv_r_vsync( "r_vsync" );

struct CVarVideoMode : CVar
{
	CVarVideoMode() : CVar( "r_videomode" ){}
	virtual void ToString( String& out )
	{
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "%d %d %d %d %d %d %d %d\n",
			(int) g_RenderSettings.display,
			(int) g_RenderSettings.width,
			(int) g_RenderSettings.height,
			(int) g_RenderSettings.refresh_rate,
			(int) g_RenderSettings.fullscreen,
			(int) g_RenderSettings.vsync,
			(int) g_RenderSettings.aa_mode,
			(int) g_RenderSettings.aa_quality );
	}
	virtual void FromString( StringView str )
	{
		RenderSettings rs;
		{
			rs.display = str.parse_int();
			rs.width = str.parse_int();
			rs.height = str.parse_int();
			rs.refresh_rate = str.parse_int();
			rs.fullscreen = str.parse_int();
			rs.vsync = str.parse_int() != 0;
			rs.aa_mode = str.parse_int();
			rs.aa_quality = str.parse_int();
		}
		GR_SetVideoMode( rs );
	}
}
gcv_r_videomode;

static void registercvars()
{
	gcv_r_display.SetConst( true );
	gcv_r_width.SetConst( true );
	gcv_r_height.SetConst( true );
	gcv_r_refresh_rate.SetConst( true );
	gcv_r_fullscreen.SetConst( true );
	gcv_r_vsync.SetConst( true );
	
	gcv_r_display.value = g_RenderSettings.display;
	gcv_r_width.value = g_RenderSettings.width;
	gcv_r_height.value = g_RenderSettings.height;
	gcv_r_refresh_rate.value = g_RenderSettings.refresh_rate;
	gcv_r_fullscreen.value = g_RenderSettings.fullscreen;
	gcv_r_vsync.value = g_RenderSettings.vsync;
	
	REGCOBJ( gcv_r_display );
	REGCOBJ( gcv_r_width );
	REGCOBJ( gcv_r_height );
	REGCOBJ( gcv_r_refresh_rate );
	REGCOBJ( gcv_r_fullscreen );
	REGCOBJ( gcv_r_vsync );
	REGCOBJ( gcv_r_videomode );
}



void Window_SetTitle( const StringView& text )
{
	SDL_SetWindowTitle( g_Window, StackString<4096>(text) );
}

bool Window_HasClipboardText()
{
	return SDL_HasClipboardText() != 0;
}

bool Window_GetClipboardText( String& out )
{
	char* cbtext = SDL_GetClipboardText();
	if( !cbtext )
		return false;
	out = cbtext;
	return true;
}

bool Window_SetClipboardText( const StringView& text )
{
	return 0 == SDL_SetClipboardText( String_Concat( text, StringView( "\0", 1 ) ).data() );
}



CObj::~CObj()
{
}

void CObj::ToString( String& out )
{
}

void CObj::DoCommand( StringView args )
{
}

void CVar::DoCommand( StringView args )
{
	if( args.size() == 0 )
	{
		String out;
		ToString( out );
		printf( "CVAR [%s]: %s\n", StackString<128>(name).str, StackString<128>(out).str );
	}
	else if( GetConst() == false )
	{
		FromString( args );
		OnChange();
	}
}

void CVar::FromString( StringView str )
{
}

void CVar::OnChange()
{
}

void CVarBool::ToString( String& out )
{
	out.append( value ? "true" : "false" );
}

void CVarBool::FromString( StringView str )
{
	value = String_ParseBool( str );
}

void CVarInt::ToString( String& out )
{
	char bfr[ 32 ];
	sgrx_snprintf( bfr, 32, "%d", (int) value );
	out.append( bfr );
}

void CVarInt::FromString( StringView str )
{
	value = String_ParseInt( str );
}



SGRX_Joystick::SGRX_Joystick( int which ) : m_id( which ), m_gamectrl( NULL )
{
	m_joystick = SDL_JoystickOpen( which );
	if( SDL_IsGameController( which ) )
		m_gamectrl = SDL_GameControllerOpen( which );
}

SGRX_Joystick::~SGRX_Joystick()
{
	if( m_gamectrl )
		SDL_GameControllerClose( m_gamectrl );
	if( m_joystick )
		SDL_JoystickClose( m_joystick );
}



SGRX_IEventHandler::~SGRX_IEventHandler()
{
	Game_UnregisterEventHandler( this );
}

void SGRX_IEventHandler::RegisterHandler( SGRX_EventID eid )
{
	Game_RegisterEventHandler( this, eid );
}

void SGRX_IEventHandler::UnregisterHandler( SGRX_EventID eid )
{
	Game_UnregisterEventHandler( this, eid );
}



//
// GAME SYSTEMS
//


void Game_RegisterEventHandler( SGRX_IEventHandler* eh, SGRX_EventID eid )
{
	EventLinkEntry ele = { eid, eh };
	EventLinkArrayHandle links = g_EventLinksByID.getcopy( eid );
	if( !links )
	{
		links = new EventLinkArray;
		g_EventLinksByID.set( eid, links );
	}
	links->find_or_add( ele );
	links = g_EventLinksByHandler.getcopy( eh );
	if( !links )
	{
		links = new EventLinkArray;
		g_EventLinksByHandler.set( eh, links );
	}
	links->find_or_add( ele );
}

void Game_UnregisterEventHandler( SGRX_IEventHandler* eh, SGRX_EventID eid )
{
	if( eid == 0 )
	{
		EventLinkArrayHandle links = g_EventLinksByHandler.getcopy( eh );
		if( !links )
			return;
		EventLinkArray& ELA = *links;
		for( size_t i = 0; i < ELA.size(); ++i )
		{
			Game_UnregisterEventHandler( eh, ELA[ i ].eid );
		}
	}
	else
	{
		EventLinkEntry ele = { eid, eh };
		EventLinkArrayHandle links = g_EventLinksByID.getcopy( eid );
		if( links )
		{
			links->remove_first( ele );
		}
		links = g_EventLinksByHandler.getcopy( eh );
		if( links )
		{
			links->remove_first( ele );
			if( links->size() == 0 )
				g_EventLinksByHandler.unset( eh );
		}
	}
}

void Game_FireEvent( SGRX_EventID eid, const EventData& edata )
{
	EventLinkArrayHandle links = g_EventLinksByID.getcopy( eid );
	if( !links )
		return;
	EventLinkArray& ELA = *links;
	for( size_t i = 0; i < ELA.size(); ++i )
	{
		ELA[ i ].eh->HandleEvent( eid, edata );
	}
}


void InputState::_SetState( float x )
{
	x = clamp( x, -1, 1 );
	state = fabsf( x ) >= threshold;
	value = state ? x : 0;
}

void InputState::_Advance()
{
	prev_value = value;
	prev_state = state;
}

void Game_RegisterAction( InputState* cmd )
{
	g_ActionMap->Register( cmd );
}

void Game_UnregisterAction( InputState* cmd )
{
	g_ActionMap->Unregister( cmd );
}

void Game_RegisterCObj( CObj& cobj )
{
	g_CObjs->set( cobj.name, &cobj );
}

void Game_UnregisterCObj( CObj& cobj )
{
	g_CObjs->unset( cobj.name );
}

CObj* Game_FindCObj( StringView name )
{
	return g_CObjs->getcopy( name );
}

bool Game_DoCommand( StringView cmd )
{
	cmd.ltrim( SPACE_CHARS );
	StringView name = cmd.until_any( SPACE_CHARS );
	CObj* obj = g_CObjs->getcopy( name );
	if( !obj )
		return false;
	
	cmd.skip( name.size() );
	cmd.ltrim( SPACE_CHARS );
	obj->DoCommand( cmd );
	return true;
}

InputState* Game_FindAction( const StringView& cmd )
{
	return g_ActionMap->FindAction( cmd );
}

void Game_BindKeyToAction( uint32_t key, InputState* cmd )
{
	g_ActionMap->Map( ACTINPUT_MAKE_KEY( key ), cmd );
}

void Game_BindMouseButtonToAction( int btn, InputState* cmd )
{
	g_ActionMap->Map( ACTINPUT_MAKE_MOUSE( btn ), cmd );
}

void Game_BindGamepadButtonToAction( int btn, InputState* cmd )
{
	g_ActionMap->Map( ACTINPUT_MAKE_GPADBTN( btn ), cmd );
}

void Game_BindGamepadAxisToAction( int axis, InputState* cmd )
{
	g_ActionMap->Map( ACTINPUT_MAKE_GPADAXIS( axis ), cmd );
}

ActionInput Game_GetActionBinding( InputState* cmd )
{
	ActionMap::InputCmdMap* icm = &g_ActionMap->m_inputCmdMap;
	for( size_t i = 0; i < icm->size(); ++i )
	{
		if( icm->item(i).value == cmd )
			return icm->item(i).key;
	}
	return 0;
}

int Game_GetActionBindings( InputState* cmd, ActionInput* out, int bufsize )
{
	int num = 0;
	ActionMap::InputCmdMap* icm = &g_ActionMap->m_inputCmdMap;
	for( size_t i = 0; i < icm->size() && num < bufsize; ++i )
	{
		if( icm->item(i).value == cmd )
			out[ num++ ] = icm->item(i).key;
	}
	return num;
}

void Game_BindInputToAction( ActionInput iid, InputState* cmd )
{
	g_ActionMap->Map( iid, cmd );
}

void Game_UnbindInput( ActionInput iid )
{
	g_ActionMap->Unmap( iid );
}

StringView Game_GetInputName( ActionInput iid )
{
	uint32_t kv = ACTINPUT_GET_VALUE( iid );
	switch( ACTINPUT_GET_TYPE( iid ) )
	{
	case ACTINPUT_UNASSIGNED:
		return "<unassigned>";
	case ACTINPUT_KEY:
		{
			const char* kn = SDL_GetKeyName( kv );
			if( *kn )
				return kn;
		}
		return "<Unknown key>";
	case ACTINPUT_MOUSE:
		switch( kv )
		{
		case SGRX_MB_LEFT: return "Left mouse button";
		case SGRX_MB_RIGHT: return "Right mouse button";
		case SGRX_MB_MIDDLE: return "Middle mouse button";
		case SGRX_MB_X1: return "X1 mouse button";
		case SGRX_MB_X2: return "X2 mouse button";
		}
		return "<Unknown mouse btn.>";
	case ACTINPUT_GAMEPAD:
		switch( kv )
		{
		case ACTINPUT_TYPE_AXIS+SDL_CONTROLLER_AXIS_LEFTX: return "[gamepad] left X axis";
		case ACTINPUT_TYPE_AXIS+SDL_CONTROLLER_AXIS_LEFTY: return "[gamepad] left Y axis";
		case ACTINPUT_TYPE_AXIS+SDL_CONTROLLER_AXIS_RIGHTX: return "[gamepad] right X axis";
		case ACTINPUT_TYPE_AXIS+SDL_CONTROLLER_AXIS_RIGHTY: return "[gamepad] right Y axis";
		case ACTINPUT_TYPE_BUTTON+SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return "[gamepad] RB";
		case ACTINPUT_TYPE_BUTTON+SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "[gamepad] LB";
		case ACTINPUT_TYPE_BUTTON+SDL_CONTROLLER_BUTTON_X: return "[gamepad] X";
		}
		return "<Unknown ctrl. input>";
	}
	return "<Unknown input>";
}

Vec2 Game_GetCursorPos()
{
	return g_CursorPos;
}

Vec2 Game_GetScreenSize()
{
	return V2( GR_GetWidth(), GR_GetHeight() );
}

Vec2 Game_GetCursorPosNormalized()
{
	return Game_GetCursorPos() / Game_GetScreenSize();
}

void Game_SetCursorPos( int x, int y )
{
	SDL_WarpMouseInWindow( g_Window, x, y );
}

void Game_ShowCursor( bool show )
{
	SDL_ShowCursor( show ? SDL_ENABLE : SDL_DISABLE );
}


bool Game_HasOverlayScreens()
{
	return g_OverlayScreens.size() != 0;
}

bool Game_HasOverlayScreen( IScreen* screen )
{
	return g_OverlayScreens.has( screen );
}

void Game_AddOverlayScreen( IScreen* screen )
{
	LOG_FUNCTION;
	g_OverlayScreens.push_back( screen );
	screen->OnStart();
}

void Game_RemoveOverlayScreen( IScreen* screen )
{
	LOG_FUNCTION;
	if( g_OverlayScreens.has( screen ) )
	{
		screen->OnEnd();
		g_OverlayScreens.remove_all( screen );
	}
}

void Game_RemoveAllOverlayScreens()
{
	LOG_FUNCTION;
	while( g_OverlayScreens.size() )
	{
		Game_RemoveOverlayScreen( g_OverlayScreens.last() );
	}
}

static void process_overlay_screens( float dt )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_OverlayScreens.size(); ++i )
	{
		IScreen* scr = g_OverlayScreens[ i ];
		if( scr->Draw( dt ) )
		{
			g_OverlayScreens.erase( i-- );
			if( !g_OverlayScreens.has( scr ) )
				scr->OnEnd();
		}
	}
}


void Game_OnEvent( const Event& e )
{
	LOG_FUNCTION;
	if( e.type == SDL_WINDOWEVENT )
	{
		switch( e.window.event )
		{
		case SDL_WINDOWEVENT_FOCUS_GAINED: g_HasFocus = true; break;
		case SDL_WINDOWEVENT_FOCUS_LOST: g_HasFocus = false; break;
		case SDL_WINDOWEVENT_MINIMIZED: g_WindowVisible = false; break;
		case SDL_WINDOWEVENT_RESTORED: g_WindowVisible = true; break;
		}
	}
	
	if( e.type == SDL_CONTROLLERDEVICEADDED )
	{
		g_Joysticks->set( e.cdevice.which, new SGRX_Joystick( e.cdevice.which ) );
	}
	else if( e.type == SDL_CONTROLLERDEVICEREMOVED )
	{
		g_Joysticks->unset( e.cdevice.which );
	}
	
	if( e.type == SDL_MOUSEMOTION )
	{
		g_CursorPos.x = e.motion.x;
		g_CursorPos.y = e.motion.y;
	}
	
	g_Game->OnEvent( e );
	
	for( size_t i = g_OverlayScreens.size(); i > 0; )
	{
		i--;
		IScreen* screen = g_OverlayScreens[ i ];
		if( screen->OnEvent( e ) )
			return; // event inhibited
	}
	
	if( e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP )
	{
		InputState* cmd = g_ActionMap->Get( ACTINPUT_MAKE_GPADBTN( e.cbutton.button ) );
		if( cmd )
			cmd->_SetState( e.cbutton.state );
	}
	else if( e.type == SDL_CONTROLLERAXISMOTION )
	{
		InputState* cmd = g_ActionMap->Get( ACTINPUT_MAKE_GPADAXIS( e.caxis.axis ) );
		if( cmd )
			cmd->_SetState( e.caxis.value / 32767.0f );
	}
	
	if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP )
	{
		InputState* cmd = g_ActionMap->Get( ACTINPUT_MAKE_KEY( e.key.keysym.sym ) );
		if( cmd )
			cmd->_SetState( e.key.state );
	}
	
	if( e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP )
	{
		int btn = -1;
		if( e.button.button == SDL_BUTTON_LEFT ) btn = SGRX_MB_LEFT;
		if( e.button.button == SDL_BUTTON_RIGHT ) btn = SGRX_MB_RIGHT;
		if( e.button.button == SDL_BUTTON_MIDDLE ) btn = SGRX_MB_MIDDLE;
		if( btn >= 0 )
		{
			InputState* cmd = g_ActionMap->Get( ACTINPUT_MAKE_MOUSE( btn ) );
			if( cmd )
				cmd->_SetState( e.button.state );
		}
	}
}

static void renderer_clear_rts()
{
	SGRX_RTClearInfo info = { SGRX_RT_ClearAll, 0, 0, 1 };
	TextureHandle rts[4] = { NULL, NULL, NULL, NULL };
	g_Renderer->SetRenderTargets( info, NULL, rts );
}

void Game_Process( float dt )
{
	LOG_FUNCTION;
	if( !g_WindowVisible || !g_HasFocus )
	{
		Thread_Sleep( 40 );
		return;
	}
	
	g_Game->OnTick( dt, g_GameTime );
	
	process_overlay_screens( dt );
	
	g_BatchRenderer->Flush();
	
	g_Renderer->Swap();
	renderer_clear_rts();
}


void Game_End()
{
	g_Running = false;
}


void ParseDefaultTextureFlags( const StringView& flags, uint32_t& outusageflags )
{
	if( flags.contains( ":nosrgb" ) ) outusageflags &= ~TEXFLAGS_SRGB;
	if( flags.contains( ":srgb" ) ) outusageflags |= TEXFLAGS_SRGB;
	if( flags.contains( ":wrapx" ) ) outusageflags &= ~TEXFLAGS_CLAMP_X;
	if( flags.contains( ":wrapy" ) ) outusageflags &= ~TEXFLAGS_CLAMP_Y;
	if( flags.contains( ":clampx" ) ) outusageflags |= TEXFLAGS_CLAMP_X;
	if( flags.contains( ":clampy" ) ) outusageflags |= TEXFLAGS_CLAMP_Y;
	if( flags.contains( ":nolerp" ) ) outusageflags &= ~TEXFLAGS_LERP;
	if( flags.contains( ":lerp" ) ) outusageflags |= TEXFLAGS_LERP;
	if( flags.contains( ":nomips" ) ) outusageflags &= ~TEXFLAGS_HASMIPS;
	if( flags.contains( ":mips" ) ) outusageflags |= TEXFLAGS_HASMIPS;
}

void IGame::OnDrawScene( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info )
{
#define RT_MAIN 0xfff0
#define RT_HBLUR 0xfff1
#define RT_VBLUR 0xfff2
#define RT_HBLUR2 0xfff3
#define RT_VBLUR2 0xfff4
#define RT_HPASS 0xfff5
#define RT_DEPTH 0xfff6
	
	// preserve state
	Mat4 viewMtx = g_BatchRenderer->viewMatrix;
	
	// shortcuts
	SGRX_Scene* scene = info.scene;
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	int W = GR_GetWidth();
	int H = GR_GetHeight();
	int W4 = TMAX( W / 4, 1 ), H4 = TMAX( H / 4, 1 );
	int W16 = TMAX( W4 / 4, 1 ), H16 = TMAX( H4 / 4, 1 );
	
	// load shaders
	PixelShaderHandle pppsh_final = GR_GetPixelShader( "sys_pp_final" );
	PixelShaderHandle pppsh_highpass = GR_GetPixelShader( "sys_pp_highpass" );
	PixelShaderHandle pppsh_blur = GR_GetPixelShader( "sys_pp_blur" );
	
	GR_PreserveResource( pppsh_final );
	GR_PreserveResource( pppsh_highpass );
	GR_PreserveResource( pppsh_blur );
	
	// initial actions
	ctrl->RenderShadows( scene, 0 );
	ctrl->SortRenderItems( scene );
	
	// prepare render targets
	TextureHandle rttMAIN, rttHPASS, rttHBLUR, rttVBLUR, rttHBLUR2, rttVBLUR2, rttDEPTH;
	DepthStencilSurfHandle dssMAIN;
	if( info.enablePostProcessing )
	{
		rttMAIN = GR_GetRenderTarget( W, H, RT_FORMAT_COLOR_HDR16, RT_MAIN );
		rttDEPTH = GR_GetRenderTarget( W, H, RT_FORMAT_DEPTH, RT_DEPTH );
		rttHPASS = GR_GetRenderTarget( W, H, RT_FORMAT_COLOR_HDR16, RT_HPASS );
		rttHBLUR = GR_GetRenderTarget( W4, H, RT_FORMAT_COLOR_HDR16, RT_HBLUR );
		rttVBLUR = GR_GetRenderTarget( W4, H4, RT_FORMAT_COLOR_HDR16, RT_VBLUR );
		rttHBLUR2 = GR_GetRenderTarget( W16, H4, RT_FORMAT_COLOR_HDR16, RT_HBLUR2 );
		rttVBLUR2 = GR_GetRenderTarget( W16, H16, RT_FORMAT_COLOR_HDR16, RT_VBLUR2 );
		dssMAIN = GR_GetDepthStencilSurface( W, H, RT_FORMAT_COLOR_HDR16, RT_MAIN );
		
		GR_PreserveResource( rttMAIN );
		GR_PreserveResource( rttDEPTH );
		GR_PreserveResource( rttHPASS );
		GR_PreserveResource( rttHBLUR );
		GR_PreserveResource( rttVBLUR );
		GR_PreserveResource( rttHBLUR2 );
		GR_PreserveResource( rttVBLUR2 );
		GR_PreserveResource( dssMAIN );
		
		ctrl->SetRenderTargets( dssMAIN, SGRX_RT_ClearAll, 0, 0, 1, rttDEPTH );
		ctrl->RenderTypes( scene, 0, 1, SGRX_TY_Solid );
	}
	
	// draw things
	OnDrawSceneGeom( ctrl, info, rttMAIN, dssMAIN );
	
	// post-process
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, 1, 1 ) );
	if( info.enablePostProcessing )
	{
		br.Reset();
		br.ShaderData.push_back( V4(0) );
		
		float spread = 3.5f;
		ctrl->SetRenderTargets( NULL, SGRX_RT_ClearAll, 0, 0, 1, rttHPASS );
		br.SetTexture( rttMAIN ).SetShader( pppsh_highpass ).Quad( 0, 0, 1, 1 ).Flush();
		
		br.ShaderData[0] = V4( 0, 0, safe_fdiv( spread, W ), 0 );
		ctrl->SetRenderTargets( NULL, SGRX_RT_ClearAll, 0, 0, 1, rttHBLUR );
		br.SetTexture( rttHPASS ).SetShader( pppsh_blur ).Quad( 0, 0, 1, 1 ).Flush();
		
		br.ShaderData[0] = V4( 0, 0, 0, safe_fdiv( spread, H ) );
		ctrl->SetRenderTargets( NULL, SGRX_RT_ClearAll, 0, 0, 1, rttVBLUR );
		br.SetTexture( rttHBLUR ).SetShader( pppsh_blur ).Quad( 0, 0, 1, 1 ).Flush();
		
		br.ShaderData[0] = V4( 0, 0, safe_fdiv( spread, W4 ), 0 );
		ctrl->SetRenderTargets( NULL, SGRX_RT_ClearAll, 0, 0, 1, rttHBLUR2 );
		br.SetTexture( rttVBLUR ).SetShader( pppsh_blur ).Quad( 0, 0, 1, 1 ).Flush();
		
		br.ShaderData[0] = V4( 0, 0, 0, safe_fdiv( spread, H4 ) );
		ctrl->SetRenderTargets( NULL, SGRX_RT_ClearAll, 0, 0, 1, rttVBLUR2 );
		br.SetTexture( rttHBLUR2 ).SetShader( pppsh_blur ).Quad( 0, 0, 1, 1 ).Flush();
		
		ctrl->SetRenderTargets( NULL, 0, 0, 0, 1 );
		br.SetTexture( 0, rttMAIN )
		  .SetTexture( 2, rttVBLUR )
		  .SetTexture( 3, rttVBLUR2 )
		  .SetTexture( 4, rttDEPTH ).SetShader( pppsh_final ).VPQuad( info.viewport ).Flush();
	}
	else
	{
		ctrl->SetRenderTargets( NULL, 0, 0, 0, 1 );
		br.Reset().SetTexture( rttMAIN ).VPQuad( info.viewport ).Flush();
	}
	
	// debug rendering from camera viewpoint and optional depth clipping
	if( info.debugdraw )
	{
		ctrl->SetRenderTargets( dssMAIN, 0, 0, 0, 1 );
		if( info.viewport )
			GR2D_SetViewport( info.viewport->x0, info.viewport->y0, info.viewport->x1, info.viewport->y1 );
		GR2D_SetViewMatrix( scene->camera.mView * scene->camera.mProj );
		br.Flush().Reset();
		info.debugdraw->DebugDraw();
		br.Flush();
		if( info.viewport )
			GR2D_UnsetViewport();
		ctrl->SetRenderTargets( NULL, 0, 0, 0, 1 );
	}
	
	GR2D_SetViewMatrix( viewMtx );
	br.Reset();
}

void IGame::OnDrawSceneGeom( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info,
	TextureHandle rtt, DepthStencilSurfHandle dss )
{
	SGRX_Scene* scene = info.scene;
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	ctrl->SetRenderTargets( dss, SGRX_RT_ClearAll, 0, scene->clearColor, 1, rtt );
	if( info.viewport )
		GR2D_SetViewport( info.viewport->x0, info.viewport->y0, info.viewport->x1, info.viewport->y1 );
	
	ctrl->RenderTypes( scene, 1, 1, SGRX_TY_Solid );
	ctrl->RenderTypes( scene, 3, 4, SGRX_TY_Solid );
	ctrl->RenderTypes( scene, 1, 1, SGRX_TY_Decal );
	ctrl->RenderTypes( scene, 3, 4, SGRX_TY_Decal );
	ctrl->RenderTypes( scene, 1, 1, SGRX_TY_Transparent );
	ctrl->RenderTypes( scene, 3, 4, SGRX_TY_Transparent );
	if( info.postdraw )
	{
		GR2D_SetViewMatrix( scene->camera.mView * scene->camera.mProj );
		br.Flush().Reset();
		info.postdraw->PostDraw();
		br.Flush();
	}
}

void IGame::OnMakeRenderState( const SGRX_RenderPass& pass, const SGRX_Material& mtl, SGRX_RenderState& out )
{
	out.cullMode = mtl.flags & SGRX_MtlFlag_Nocull ? SGRX_RS_CullMode_None : SGRX_RS_CullMode_Back;
	
	if( pass.isShadowPass == false )
	{
		bool decal = ( mtl.flags & SGRX_MtlFlag_Decal ) != 0;
		bool ltovr = pass.isBasePass == false && pass.isShadowPass == false;
		out.depthBias = decal ? -1e-5f : 0;
		out.depthWriteEnable = ( ltovr || decal || mtl.blendMode != SGRX_MtlBlend_None ) == false;
		out.blendStates[ 0 ].blendEnable = ltovr || mtl.blendMode != SGRX_MtlBlend_None;
		if( ltovr || mtl.blendMode == SGRX_MtlBlend_Additive )
		{
			out.blendStates[ 0 ].srcBlend = SGRX_RS_Blend_SrcAlpha;
			out.blendStates[ 0 ].dstBlend = SGRX_RS_Blend_One;
		}
		else if( mtl.blendMode == SGRX_MtlBlend_Multiply )
		{
			out.blendStates[ 0 ].srcBlend = SGRX_RS_Blend_Zero;
			out.blendStates[ 0 ].dstBlend = SGRX_RS_Blend_SrcColor;
		}
		else
		{
			out.blendStates[ 0 ].srcBlend = SGRX_RS_Blend_SrcAlpha;
			out.blendStates[ 0 ].dstBlend = SGRX_RS_Blend_InvSrcAlpha;
		}
	}
	else
	{
		out.depthBias = 1e-5f;
		out.slopeDepthBias = 0.5f;
	}
}

void IGame::OnLoadMtlShaders( const SGRX_RenderPass& pass,
	const StringView& defines, const SGRX_Material& mtl,
	SGRX_MeshInstance* MI, VertexShaderHandle& VS, PixelShaderHandle& PS )
{
	if( pass.isBasePass == false && pass.isShadowPass == false &&
		( ( mtl.flags & SGRX_MtlFlag_Unlit ) != 0 || MI->GetLightingMode() == SGRX_LM_Unlit ) )
	{
		PS = NULL;
		VS = NULL;
		return;
	}
	
	String name = "mtl:";
	name.append( mtl.shader );
	name.append( ":" );
	name.append( pass.shader );
	
	if( pass.isShadowPass )
		name.append( ":SHADOW_PASS" );
	else
	{
		char bfr[32];
		// lighting mode
		{
			sgrx_snprintf( bfr, 32, "%d", MI->GetLightingMode() );
			name.append( ":LMODE " );
			name.append( bfr );
		}
		if( pass.isBasePass )
			name.append( ":BASE_PASS" );
		if( pass.numPL )
		{
			sgrx_snprintf( bfr, 32, "%d", pass.numPL );
			name.append( ":NUM_POINTLIGHTS " );
			name.append( bfr );
		}
		if( pass.numSL )
		{
			sgrx_snprintf( bfr, 32, "%d", pass.numSL );
			name.append( ":NUM_SPOTLIGHTS " );
			name.append( bfr );
		}
	}
	
	// misc. parameters
	name.append( ":" );
	name.append( defines ); // scene defines
	
	if( mtl.flags & SGRX_MtlFlag_Decal )
		name.append( ":DECAL" ); // color multiplied by vertex color, even with other lighting models
	
	PS = GR_GetPixelShader( name );
	
	if( MI->IsSkinned() )
		name.append( ":SKIN" );
	
	VS = GR_GetVertexShader( name );
}

bool IGame::OnLoadTexture( const StringView& key, ByteArray& outdata, uint32_t& outusageflags )
{
	LOG_FUNCTION;
	
	if( !key )
		return false;
	
	StringView path = key.until( ":" );
	
	// try .stx (optimized) before original
	if( FS_LoadBinaryFile( String_Concat( path, ".stx" ), outdata ) == false &&
		FS_LoadBinaryFile( path, outdata ) == false )
		return false;
	
	outusageflags = TEXFLAGS_HASMIPS | TEXFLAGS_LERP;
	if( path.contains( "diff." ) )
	{
		// diffuse maps
		outusageflags |= TEXFLAGS_SRGB;
	}
	
	StringView flags = key.from( ":" );
	ParseDefaultTextureFlags( flags, outusageflags );
	
	return true;
}

void IGame::GetShaderCacheFilename( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, String& name )
{
	LOG_FUNCTION;
	
	name = "shadercache_";
	name.append( rinfo.shaderCacheSfx );
	name.append( "/" );
	
	StringView it = key;
	while( it.size() )
	{
		char ch = it.ch();
		it.skip(1);
		
		if( ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) || ( ch >= '0' && ch <= '9' ) || ch == '_' )
			name.push_back( ch );
		else if( name.last() != '$' )
			name.push_back( '$' );
	}
	
	name.append( sfx );
	name.append( ".csh" );
}

bool IGame::GetCompiledShader( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, ByteArray& outdata )
{
	LOG_FUNCTION;
	
	if( !key )
		return false;
	
	String filename;
	GetShaderCacheFilename( rinfo, sfx, key, filename );
	
	LOG << "Loading precompiled shader: " << filename << " (type=" << rinfo.shaderCacheSfx << ", key=" << key << ")";
	return FS_LoadBinaryFile( filename, outdata );
}

bool IGame::SetCompiledShader( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, const ByteArray& data )
{
	LOG_FUNCTION;
	
	if( !key )
		return false;
	
	String filename;
	GetShaderCacheFilename( rinfo, sfx, key, filename );
	
	LOG << "Saving precompiled shader: " << filename << " (type=" << rinfo.shaderCacheSfx << ", key=" << key << ")";
	return FS_SaveBinaryFile( filename, data.data(), data.size() );
}

bool IGame::OnLoadShader( const SGRX_RendererInfo& rinfo, const StringView& key, String& outdata )
{
	LOG_FUNCTION;
	
	if( !key )
		return false;
	
	if( key.part( 0, 4 ) == "mtl:" )
	{
		int i = 0;
		String prepend;
		StringView tpl, mtl, vs, defs, cur, it = key.part( 4 );
		while( it.size() )
		{
			i++;
			cur = it.until( ":" );
			if( i == 1 )
			{
				StringView defs = cur.after( "+" );
				StringView spec = cur.until( "+" );
				mtl = spec.until( "|" );
				vs = spec.after( "|" );
				
				StringView def = defs.until( "+" );
				while( def.size() || defs.ch() == '+' )
				{
					prepend.append( STRLIT_BUF( "#define " ) );
					prepend.append( def.data(), def.size() );
					prepend.append( STRLIT_BUF( "\n" ) );
					defs = defs.after( "+" );
					def = defs.until( "+" );
				}
			}
			else if( i == 2 )
				tpl = cur;
			else if( cur.size() )
			{
				prepend.append( STRLIT_BUF( "#define " ) );
				prepend.append( cur.data(), cur.size() );
				prepend.append( STRLIT_BUF( "\n" ) );
			}
			it.skip( cur.size() + 1 );
		}
		
		String tpl_data, mtl_data, vs_data;
		if( !OnLoadShaderFile( rinfo, tpl, tpl_data ) )
			return false;
		if( mtl.size() && !OnLoadShaderFile( rinfo, String_Concat( "mtl_", mtl ), mtl_data ) )
			return false;
		if( vs.size() && !OnLoadShaderFile( rinfo, String_Concat( "vs_", vs ), vs_data ) )
			return false;
		outdata = String_Concat( prepend, String_Replace( String_Replace( tpl_data, "__CODE__", mtl_data ), "__VSCODE__", vs_data ) );
		return true;
	}
	return OnLoadShaderFile( rinfo, key, outdata );
}

bool IGame::OnLoadShaderFile( const SGRX_RendererInfo& rinfo, const StringView& path, String& outdata )
{
	LOG_FUNCTION;
	
	String filename = "shaders";
	filename.push_back( '/' );
	filename.append( path.data(), path.size() );
	filename.append( STRLIT_BUF( ".shd" ) );
	
	if( !FS_LoadTextFile( filename, outdata ) )
	{
		LOG_WARNING << "Failed to load shader file: " << filename << " (type=" << rinfo.shaderCacheSfx << ", path=" << path << ")";
		return false;
	}
	
	char bfr[ 400 ];
	sgrx_snprintf( bfr, 400, "#define %s\n#line 1 \"%s\"\n",
		StackString<100>(rinfo.shaderTypeDefine).str, StackPath(path).str );
	outdata.insert( 0, bfr, sgrx_snlen( bfr, 400 ) );
	
	return ParseShaderIncludes( rinfo, path, outdata );
}

bool IGame::ParseShaderIncludes( const SGRX_RendererInfo& rinfo, const StringView& path, String& outdata )
{
	LOG_FUNCTION;
	
	String basepath = path.up_to_last( "/" );
	String nstr;
	StringView it = outdata, inc;
	while( ( inc = it.from( "#include" ) ) != StringView() )
	{
		// generate path
		String incstr = inc.after( "\"" ).until( "\"" );
		String incpath = String_Concat( basepath, incstr );
		
		// append prior data
		nstr.append( it.data(), inc.data() - it.data() );
		
		// contents of new file (includes starting directive)
		String incfiledata;
		if( !OnLoadShaderFile( rinfo, incpath, incfiledata ) )
			return false;
		nstr.append( incfiledata );
		
		// directive for original file
		int linenum = StringView(outdata).part( 0, inc.data() - outdata.data() ).count( "\n" ) + 1;
		char bfr[ 32 ];
		sgrx_snprintf( bfr, 32, "%d", linenum );
		nstr.append( "#line " );
		nstr.append( bfr );
		nstr.append( " \"" );
		nstr.append( path );
		nstr.append( "\"\n" );
		
		// continue
		it = inc.after( "\"" ).after( "\"" );
	}
	nstr.append( it );
	
	outdata = nstr;
	return true;
}

bool IGame::OnLoadMesh( const StringView& key, ByteArray& outdata )
{
	LOG_FUNCTION;
	
	if( !key )
		return false;
	
	StringView path = key.until( ":" );
	
	if( !FS_LoadBinaryFile( path, outdata ) )
		return false;
	
	return true;
}


IFileSystem::IFileSystem() : m_refcount(0)
{
}

IFileSystem::~IFileSystem()
{
}

BasicFileSystem::BasicFileSystem( const StringView& root ) : m_fileRoot(root)
{
	if( m_fileRoot.size() && m_fileRoot.last() != '/' )
		m_fileRoot.push_back( '/' );
}

bool BasicFileSystem::LoadBinaryFile( const StringView& path, ByteArray& out )
{
	LOG_FUNCTION;
	return ::LoadBinaryFile( String_Concat( m_fileRoot, path ), out );
}

bool BasicFileSystem::SaveBinaryFile( const StringView& path, const void* data, size_t size )
{
	LOG_FUNCTION;
	return ::SaveBinaryFile( String_Concat( m_fileRoot, path ), data, size );
}

bool BasicFileSystem::LoadTextFile( const StringView& path, String& out )
{
	LOG_FUNCTION;
	return ::LoadTextFile( String_Concat( m_fileRoot, path ), out );
}

bool BasicFileSystem::SaveTextFile( const StringView& path, const StringView& data )
{
	LOG_FUNCTION;
	return ::SaveTextFile( String_Concat( m_fileRoot, path ), data );
}

bool BasicFileSystem::FindRealPath( const StringView& path, String& out )
{
	LOG_FUNCTION;
	out = m_fileRoot;
	out.append( path );
	return true;
}

bool BasicFileSystem::FileExists( const StringView& path )
{
	LOG_FUNCTION;
	return ::FileExists( String_Concat( m_fileRoot, path ) );
}

bool BasicFileSystem::DirCreate( const StringView& path )
{
	LOG_FUNCTION;
	return ::DirCreate( String_Concat( m_fileRoot, path ) );
}

uint32_t BasicFileSystem::FileModTime( const StringView& path )
{
	LOG_FUNCTION;
	return ::FileModTime( String_Concat( m_fileRoot, path ) );
}

void BasicFileSystem::IterateDirectory( const StringView& path, IDirEntryHandler* deh )
{
	LOG_FUNCTION;
	DirectoryIterator tdi( String_Concat( m_fileRoot, path ) );
	while( tdi.Next() )
	{
		if( !deh->HandleDirEntry( path, tdi.Name(), tdi.IsDirectory() ) )
			return;
	}
}


StringView Game_GetDir()
{
	return g_GameDir;
}

Array< FileSysHandle >& Game_FileSystems()
{
	return g_FileSystems;
}

bool FS_LoadBinaryFile( const StringView& path, ByteArray& out )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->LoadBinaryFile( path, out ) )
			return true;
	return false;
}

bool FS_SaveBinaryFile( const StringView& path, const void* data, size_t size )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->SaveBinaryFile( path, data, size ) )
			return true;
	return false;
}

bool FS_LoadTextFile( const StringView& path, String& out )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->LoadTextFile( path, out ) )
			return true;
	return false;
}

bool FS_SaveTextFile( const StringView& path, const StringView& data )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->SaveTextFile( path, data ) )
			return true;
	return false;
}

bool FS_FindRealPath( const StringView& path, String& out )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->FindRealPath( path, out ) )
			return true;
	return false;
}

bool FS_FileExists( const StringView& path )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->FileExists( path ) )
			return true;
	return false;
}

bool FS_DirCreate( const StringView& path )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->DirCreate( path ) )
			return true;
	return false;
}

uint32_t FS_FileModTime( const StringView& path )
{
	LOG_FUNCTION;
	uint32_t t;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( ( t = g_FileSystems[ i ]->FileModTime( path ) ) != 0 )
			return t;
	return 0;
}

void FS_IterateDirectory( const StringView& path, IDirEntryHandler* deh )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		g_FileSystems[ i ]->IterateDirectory( path, deh );
}


int GR_GetWidth(){ return g_RenderSettings.width; }
int GR_GetHeight(){ return g_RenderSettings.height; }



void GR_RenderScene( SGRX_RenderScene& info )
{
	g_BatchRenderer->Flush();
	g_BatchRenderer->Reset();
	
	info.scene->m_timevals = info.timevals;
	g_Renderer->_RS_PreProcess( info.scene );
	g_Game->OnDrawScene( g_Renderer, info );
}

RenderStats& GR_GetRenderStats()
{
	return g_Renderer->m_stats;
}


void GR2D_SetWorldMatrix( const Mat4& mtx )
{
	g_BatchRenderer->Flush();
	g_BatchRenderer->worldMatrix = mtx;
	g_BatchRenderer->_RecalcMatrices();
	g_Renderer->SetMatrix( false, mtx );
}

void GR2D_SetViewMatrix( const Mat4& mtx )
{
	g_BatchRenderer->Flush();
	g_BatchRenderer->viewMatrix = mtx;
	g_BatchRenderer->_RecalcMatrices();
	g_Renderer->SetMatrix( true, mtx );
}

void GR2D_SetViewport( int x0, int y0, int x1, int y1 )
{
	g_BatchRenderer->Flush();
	g_Renderer->SetViewport( x0, y0, x1, y1 );
}

void GR2D_UnsetViewport()
{
	g_BatchRenderer->Flush();
	g_Renderer->SetViewport( 0, 0, GR_GetWidth(), GR_GetHeight() );
}

void GR2D_SetScissorRect( int x0, int y0, int x1, int y1 )
{
	g_BatchRenderer->Flush();
	int rect[4] = { x0, y0, x1, y1 };
	g_Renderer->SetScissorRect( rect );
}

void GR2D_UnsetScissorRect()
{
	g_BatchRenderer->Flush();
	g_Renderer->SetScissorRect( NULL );
}


BatchRenderer& GR2D_GetBatchRenderer()
{
	return *g_BatchRenderer;
}


void SGRX_Swap()
{
	g_Renderer->Swap();
	renderer_clear_rts();
}


//
// INTERNALS
//

static void remap_cursor()
{
	g_CursorScale = V2(1);
	
	SDL_DisplayMode dm;
	if( g_RenderSettings.fullscreen == FULLSCREEN_WINDOWED )
	{
		if( SDL_GetCurrentDisplayMode( g_RenderSettings.display, &dm ) < 0 )
		{
			LOG << "Failed to get current display mode for display " << g_RenderSettings.display;
		}
		else
			g_CursorScale = V2( g_RenderSettings.width, g_RenderSettings.height ) / V2( TMAX( 1, dm.w ), TMAX( 1, dm.h ) );
	}
	LOG << "CURSOR REMAP: " << g_CursorScale;
}

static bool read_config()
{
	LOG_FUNCTION;
	
	String text;
	if( !LoadTextFile( "config.cfg", text ) )
	{
		LOG << "Failed to load config.cfg";
		return false;
	}
	
	ConfigReader cr( text );
	StringView key, value;
	while( cr.Read( key, value ) )
	{
		if( key == "game" )
		{
			if( value.size() )
			{
				g_GameLibName = value;
				LOG << "CONFIG: Game library: " << value;
			}
		}
		else if( key == "dir" )
		{
			if( value.size() )
			{
				g_GameDir = value;
				LOG << "CONFIG: Game directory: " << value;
			}
		}
		else if( key == "dir2" )
		{
			if( value.size() )
			{
				g_GameDir2 = value;
				LOG << "CONFIG: Game directory #2: " << value;
			}
		}
		else if( key == "renderer" )
		{
			if( value.size() )
			{
				g_RendererName = value;
				LOG << "CONFIG: Renderer: " << value;
			}
		}
		else
		{
			LOG_WARNING << "Unknown key (" << key << " = " << value << ")";
		}
	}
	
	return true;
}

static int init_graphics()
{
	LOG_FUNCTION;
	
	int flags = 0;
	if( g_RenderSettings.fullscreen )
	{
		flags |= g_RenderSettings.fullscreen == FULLSCREEN_WINDOWED ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
	}
	g_Window = SDL_CreateWindow( "SGRX Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_RenderSettings.width, g_RenderSettings.height, flags );
	remap_cursor();
	SDL_StartTextInput();
	
	char renderer_dll[ 257 ] = {0};
	sgrx_snprintf( renderer_dll, 256, "%s%s.dll", g_RendererPrefix, StackString<200>( g_RendererName ).str );
	g_RenderLib = SDL_LoadObject( renderer_dll );
	if( !g_RenderLib )
	{
		LOG_ERROR << "Failed to load renderer: '" << renderer_dll << "'";
		return 101;
	}
	g_RfnInitialize = (pfnRndInitialize) SDL_LoadFunction( g_RenderLib, "Initialize" );
	g_RfnFree = (pfnRndFree) SDL_LoadFunction( g_RenderLib, "Free" );
	g_RfnCreateRenderer = (pfnRndCreateRenderer) SDL_LoadFunction( g_RenderLib, "CreateRenderer" );
	if( !g_RfnInitialize || !g_RfnFree || !g_RfnCreateRenderer )
	{
		LOG_ERROR << "Failed to load functions from renderer (" << renderer_dll << ")";
		return 102;
	}
	StackPath rennamesp( g_RendererName );
	const char* rendername = rennamesp;
	if( !g_RfnInitialize( &rendername ) )
	{
		LOG_ERROR << "Failed to load renderer (" << rendername << ")";
		return 103;
	}
	
	SDL_SysWMinfo sysinfo;
	SDL_VERSION( &sysinfo.version );
	if( SDL_GetWindowWMInfo( g_Window, &sysinfo ) <= 0 )
	{
		LOG_ERROR << "Failed to get window pointer: " << SDL_GetError();
		return 104;
	}
	
	g_Renderer = g_RfnCreateRenderer( g_RenderSettings, sysinfo.info.win.window );
	if( !g_Renderer )
	{
		LOG_ERROR << "Failed to create renderer (" << rendername << ")";
		return 105;
	}
	renderer_clear_rts();
	LOG << LOG_DATE << "  Loaded renderer: " << rendername;
	
	SGRX_INT_InitResourceTables();
	LOG << LOG_DATE << "  Created renderer resource caches";
	SGRX_INT_InitBatchRendering();
	
	g_Joysticks = new JoystickHashTable();
	
	g_Renderer->_RS_ProjectorInit();
	if( !g_Renderer->LoadInternalResources() )
	{
		LOG_ERROR << "Failed to load renderer (" << rendername << ") internal resources";
		return 106;
	}
	LOG << LOG_DATE << "  Loaded internal renderer resources";
	
	return 0;
}

static void free_graphics()
{
	LOG_FUNCTION;
	
	g_Renderer->UnloadInternalResources();
	g_Renderer->_RS_ProjectorFree();
	
	delete g_Joysticks;
	g_Joysticks = NULL;
	
	SGRX_INT_DestroyBatchRendering();
	SGRX_INT_DestroyResourceTables();
	
	g_Renderer->Destroy();
	g_Renderer = NULL;
	
	g_RfnFree();
	
	SDL_UnloadObject( g_RenderLib );
	g_RenderLib = NULL;
	
	SDL_DestroyWindow( g_Window );
	g_Window = NULL;
}


bool GR_SetVideoMode( const RenderSettings& rs )
{
	LOG_FUNCTION;
	
	if( rs.width < 1 || rs.height < 1 )
		return false;
	
	if( g_Window )
	{
		int ret;
		
		// set fullscreen mode
		switch( rs.fullscreen )
		{
			case FULLSCREEN_NONE: ret = SDL_SetWindowFullscreen( g_Window, 0 ); break;
			case FULLSCREEN_NORMAL: ret = SDL_SetWindowFullscreen( g_Window, SDL_WINDOW_FULLSCREEN ); break;
			case FULLSCREEN_WINDOWED: ret = SDL_SetWindowFullscreen( g_Window, SDL_WINDOW_FULLSCREEN_DESKTOP ); break;
			default: ret = -2;
		}
		if( ret < 0 )
		{
			if( ret == -2 )
			{
				LOG << "Failed to set fullscreen mode: unrecognized mode";
			}
			else
			{
				LOG << "Failed to set fullscreen mode: " << SDL_GetError();
			}
			return false;
		}
		
		// set window size
		SDL_SetWindowSize( g_Window, rs.width, rs.height );
		SDL_DisplayMode dm = { SDL_PIXELFORMAT_RGBX8888, rs.width, rs.height, rs.refresh_rate, NULL };
		ret = SDL_SetWindowDisplayMode( g_Window, &dm );
		if( ret < 0 )
		{
			LOG << "Failed to set display mode: " << SDL_GetError();
			return false;
		}
		
		if( g_Renderer )
		{
			g_Renderer->Modify( rs );
			renderer_clear_rts();
		}
	}
	
	g_RenderSettings = rs;
	gcv_r_display.value = g_RenderSettings.display;
	gcv_r_width.value = g_RenderSettings.width;
	gcv_r_height.value = g_RenderSettings.height;
	gcv_r_refresh_rate.value = g_RenderSettings.refresh_rate;
	gcv_r_fullscreen.value = g_RenderSettings.fullscreen;
	gcv_r_vsync.value = g_RenderSettings.vsync;
	
	remap_cursor();
	
	return true;
}

void GR_GetVideoMode( RenderSettings& rs )
{
	rs = g_RenderSettings;
}


int GR_GetDisplayCount()
{
	return SDL_GetNumVideoDisplays();
}

const char* GR_GetDisplayName( int id )
{
	return SDL_GetDisplayName( id );
}

bool GR_ListDisplayModes( int display, Array< DisplayMode >& out )
{
	LOG_FUNCTION;
	
	out.clear();
	
	int numdm = SDL_GetNumDisplayModes( display );
	if( numdm < 0 )
	{
		LOG << "Failed to get display mode count (display=" << display << "): " << SDL_GetError();
		return false;
	}
	
	out.reserve( numdm );
	
	SDL_DisplayMode dmd;
	for( int i = 0; i < numdm; ++i )
	{
		int err = SDL_GetDisplayMode( display, i, &dmd );
		if( err < 0 )
		{
			LOG << "Failed to get display mode (display=" << display << ", mode = " << i << "): " << SDL_GetError();
			return false;
		}
		
		DisplayMode dm = { dmd.w, dmd.h, dmd.refresh_rate };
		if( out.has( dm ) == false )
			out.push_back( dm );
	}
	
	return true;
}


typedef IGame* (*pfnCreateGame) ();

int SGRX_EntryPoint( int argc, char** argv, int debug )
{
	LOG_FUNCTION;
	
#if 1
	LOG << "Engine self-test...";
	int ret = TestSystems();
	if( ret )
	{
		LOG << "Test FAILED with code: " << ret;
		return 0;
	}
	LOG << "Test completed successfully.";
#endif
	
	LOG << LOG_DATE << "  Engine started";
	
	if( !read_config() )
		return 4;
	
	for( int i = 1; i < argc; ++i )
	{
		if( i + 1 < argc )
		{
			if( !strcmp( argv[i], "-game" ) ){ g_GameLibName = argv[++i]; LOG << "ARG: Game library: " << g_GameLibName; }
			else if( !strcmp( argv[i], "-dir" ) ){ g_GameDir = argv[++i]; LOG << "ARG: Game directory: " << g_GameDir; }
			else if( !strcmp( argv[i], "-dir2" ) ){ g_GameDir = argv[++i]; LOG << "ARG: Game directory #2: " << g_GameDir2; }
			else if( !strcmp( argv[i], "-renderer" ) ){ g_RendererName = argv[++i]; LOG << "ARG: Renderer: " << g_RendererName; }
		}
	}
	
	g_FileSystems.push_back( new BasicFileSystem( g_GameDir ) );
	if( g_GameDir2.size() )
		g_FileSystems.push_back( new BasicFileSystem( g_GameDir2 ) );
	
//	if( !CWDSet( g_GameDir ) )
//	{
//		LOG_ERROR << "FAILED TO SET GAME DIRECTORY";
//		return 12;
//	}
	
	/* initialize SDL */
	if( SDL_Init(
		SDL_INIT_TIMER | SDL_INIT_VIDEO |
		SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC |
		SDL_INIT_GAMECONTROLLER |
		SDL_INIT_EVENTS | SDL_INIT_NOPARACHUTE
	) < 0 )
	{
		LOG_ERROR << "Couldn't initialize SDL: " << SDL_GetError();
		return 15;
	}
	SDL_JoystickEventState( SDL_ENABLE );
	
	g_CObjs = new CObjMap;
	g_ActionMap = new ActionMap;
	
	registercvars();
	
	g_GameLibName.append( STRLIT_BUF( ".dll" ) );
	
	g_GameLib = SDL_LoadObject( StackPath( g_GameLibName ) );
	if( !g_GameLib )
	{
		LOG_ERROR << "Failed to load " << g_GameLibName;
		return 26;
	}
	pfnCreateGame cgproc = (pfnCreateGame) SDL_LoadFunction( g_GameLib, "CreateGame" );
	if( !cgproc )
	{
		LOG_ERROR << "Failed to find entry point";
		return 37;
	}
	g_Game = cgproc();
	if( !g_Game )
	{
		LOG_ERROR << "Failed to create the game";
		return 48;
	}
	
	if( g_Game->OnConfigure( argc, argv ) == false )
		return 51;
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	
	if( init_graphics() )
		return 56;
	
	if( g_Game->OnInitialize() == false )
		return 63;
	
	uint32_t prevtime = GetTimeMsec();
	SDL_Event event;
	while( g_Running )
	{
		SGRX_INT_UnpreserveResources();
		g_ActionMap->Advance();
		SDL_JoystickUpdate();
		while( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_QUIT )
			{
				g_Running = false;
				break;
			}
			
			if( event.type == SDL_MOUSEMOTION )
			{
				event.motion.x *= g_CursorScale.x;
				event.motion.y *= g_CursorScale.y;
				event.motion.xrel *= g_CursorScale.x;
				event.motion.yrel *= g_CursorScale.y;
			}
			if( event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP )
			{
				event.button.x *= g_CursorScale.x;
				event.button.y *= g_CursorScale.y;
			}
			Game_OnEvent( event );
		}
		
		uint32_t curtime = GetTimeMsec();
		uint32_t dt = curtime - prevtime;
		prevtime = curtime;
		g_GameTime += dt;
		float fdt = dt / 1000.0f;
		
		Game_Process( fdt );
	}
	
	g_Game->OnDestroy();
	Game_RemoveAllOverlayScreens();
	
	free_graphics();
	
	SDL_UnloadObject( g_GameLib );
	
	delete g_ActionMap;
	delete g_CObjs;
	
	LOG << LOG_DATE << "  Engine finished";
	return 0;
}

