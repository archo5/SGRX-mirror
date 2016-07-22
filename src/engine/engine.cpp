

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

void* Sys_LoadLib( const char* path )
{
	void* ret = SDL_LoadObject( path );
	if( !ret )
	{
		LOG_ERROR << "Failed to load library at " << path;
	}
	return ret;
}

void Sys_UnloadLib( void* lib )
{
	SDL_UnloadObject( lib );
}

void* Sys_GetProc( void* lib, const char* name )
{
	void* ret = SDL_LoadFunction( lib, name );
	if( !ret )
	{
		LOG_ERROR << "Failed to retrieve function address of " << name;
	}
	return ret;
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
GameHandle g_Game = NULL;
static uint32_t g_GameTime = 0;
static ActionMap* g_ActionMap;
static CObjMap* g_CObjs;
static Vec2 g_CursorPos = {0,0};
static Vec2 g_CursorScale = {0,0};
static int g_PCPX = 0;
static int g_PCPY = 0;
static bool g_PCPE = false;
static int g_PCPLFR = 0;
static EventLinksByID g_EventLinksByID;
static EventLinksByHandler g_EventLinksByHandler;
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
struct CVarShowCursor : CVarBool {
	CVarShowCursor() : CVarBool( "cl_showcursor", true ){}
	virtual void OnChange(){ Game_ShowCursor( value ); }
} gcv_cl_showcursor;

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
	REGCOBJ( gcv_cl_showcursor );
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

void Window_EnableDragDrop( bool enable )
{
	SDL_EventState( SDL_DROPFILE, enable ? SDL_ENABLE : SDL_DISABLE );
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

void CVarFloat::ToString( String& out )
{
	char bfr[ 1200 ];
	sgrx_snprintf( bfr, 1200, "%.6g", value );
	out.append( bfr );
}

void CVarFloat::FromString( StringView str )
{
	value = String_ParseFloat( str );
}

void CVarEnum::ToString( String& out )
{
	out = val_list[ value ];
}

void CVarEnum::FromString( StringView str )
{
	StringView* l = val_list;
	value = 0;
	while( l->size() )
	{
		if( *l == str )
		{
			value = l - val_list;
			break;
		}
		l++;
	}
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

GameHandle Game_Get(){ return g_Game; }

GameHandle Game_Change( IGame* ng )
{
	GameHandle old = g_Game;
	g_Game = ng;
	return old;
}

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
		case ACTINPUT_TYPE_BUTTON+SDL_CONTROLLER_BUTTON_Y: return "[gamepad] Y";
		case ACTINPUT_TYPE_BUTTON+SDL_CONTROLLER_BUTTON_A: return "[gamepad] A";
		case ACTINPUT_TYPE_BUTTON+SDL_CONTROLLER_BUTTON_B: return "[gamepad] B";
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
	if( g_RenderSettings.fullscreen == FULLSCREEN_WINDOWED )
	{
		x /= g_CursorScale.x;
		y /= g_CursorScale.y;
	}
	SDL_WarpMouseInWindow( g_Window, x, y );
}

void Game_PostSetCursorPos( int x, int y )
{
	g_PCPX = x;
	g_PCPY = y;
	g_PCPE = true;
}

bool Game_WasPSCP()
{
	return g_PCPLFR == 1;
}

void Game_ShowCursor( bool show )
{
	SDL_ShowCursor( show ? SDL_ENABLE : SDL_DISABLE );
}


void Game_OnEvent( const Event& e )
{
	LOG_FUNCTION;
	
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 &&
		e.key.keysym.sym == SDLK_RETURN &&
		( e.key.keysym.mod == KMOD_LALT ||
			e.key.keysym.mod == KMOD_RALT ) )
	{
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.fullscreen = rs.fullscreen ? FULLSCREEN_NONE : FULLSCREEN_WINDOWED;
		GR_SetVideoMode( rs );
	}
	
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
	
	g_Game->OnEvent( e );
	
	Game_FireEvent( EID_WindowEvent, EventData( (void*) &e ) );
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
	
	g_BatchRenderer->Flush();
	
	g_Renderer->Swap();
	renderer_clear_rts();
}


void Game_End()
{
	g_Running = false;
}



SGRX_RenderDirector::SGRX_RenderDirector() : m_curMode(0){}

SGRX_RenderDirector::~SGRX_RenderDirector(){}

void SGRX_RenderDirector::OnDrawScene( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info )
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
	
	int shadow_pass_id = scene->FindPass( SGRX_FP_Shadow );
	// initial actions
	if( m_curMode != SGRX_RDMode_Unlit )
	{
		ctrl->RenderShadows( scene, shadow_pass_id );
	}
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
		ctrl->RenderTypes( scene, shadow_pass_id, 1, SGRX_TY_Solid );
	}
	
	// draw things
	OnDrawSceneGeom( ctrl, info, rttMAIN, dssMAIN, rttDEPTH );
	
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
		{
			const TextureInfo& clutInfo = scene->clutTexture.GetInfo();
			br.ShaderData[0] = V4(
				safe_fdiv( 0.5f, clutInfo.width ),
				safe_fdiv( 0.5f, clutInfo.height ),
				safe_fdiv( 0.5f, clutInfo.depth ), 0 );
		}
		br.SetTexture( 0, rttMAIN )
		  .SetTexture( 2, rttVBLUR )
		  .SetTexture( 3, rttVBLUR2 )
		  .SetTexture( 4, rttDEPTH )
		  .SetTexture( 5, scene->clutTexture ).SetShader( pppsh_final ).VPQuad( info.viewport ).Flush();
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

void SGRX_RenderDirector::OnDrawSceneGeom( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info,
	TextureHandle rtt, DepthStencilSurfHandle dss, TextureHandle rttDEPTH )
{
	SGRX_Scene* scene = info.scene;
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	ctrl->SetRenderTargets( dss, SGRX_RT_ClearAll, 0, scene->clearColor, 1, rtt );
	if( info.viewport )
		GR2D_SetViewport( info.viewport->x0, info.viewport->y0, info.viewport->x1, info.viewport->y1 );
	
	if( m_curMode == SGRX_RDMode_Unlit )
	{
		int unlit_pass_id = scene->FindPass( SGRX_FP_Base | SGRX_FP_NoPoint | SGRX_FP_NoSpot, ":MOD_UNLIT" );
		
		ctrl->RenderTypes( scene, unlit_pass_id, 1, SGRX_TY_Solid );
		
		OnDrawFog( ctrl, info, rttDEPTH );
		
		ctrl->RenderTypes( scene, unlit_pass_id, 1, SGRX_TY_Decal );
		ctrl->RenderTypes( scene, unlit_pass_id, 1, SGRX_TY_Transparent );
	}
	else
	{
		StringView shdef = m_curMode == SGRX_RDMode_NoDiffCol ? ":MOD_NODIFFCOL" : "";
		int base_pass_id = scene->FindPass( SGRX_FP_Base, shdef );
		int point_pass_id = scene->FindPass( SGRX_FP_Point, shdef );
		int spot_pass_id = scene->FindPass( SGRX_FP_Spot, shdef );
		
		ctrl->RenderTypes( scene, base_pass_id, 1, SGRX_TY_Solid );
		ctrl->RenderTypes( scene, point_pass_id, 4, SGRX_TY_Solid );
		ctrl->RenderTypes( scene, spot_pass_id, 4, SGRX_TY_Solid );
		
		OnDrawFog( ctrl, info, rttDEPTH );
		
		ctrl->RenderTypes( scene, base_pass_id, 1, SGRX_TY_Decal );
		ctrl->RenderTypes( scene, point_pass_id, 4, SGRX_TY_Decal );
		ctrl->RenderTypes( scene, spot_pass_id, 4, SGRX_TY_Decal );
		ctrl->RenderTypes( scene, base_pass_id, 1, SGRX_TY_Transparent );
		ctrl->RenderTypes( scene, point_pass_id, 4, SGRX_TY_Transparent );
		ctrl->RenderTypes( scene, spot_pass_id, 4, SGRX_TY_Transparent );
	}
	
	if( info.postdraw )
	{
		GR2D_SetViewMatrix( scene->camera.mView * scene->camera.mProj );
		br.Flush().Reset();
		info.postdraw->PostDraw();
		br.Flush();
	}
}

void SGRX_RenderDirector::OnDrawFog( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info, TextureHandle rttDEPTH )
{
	SGRX_Scene* scene = info.scene;
	if( !scene->skyTexture || !rttDEPTH )
		return;
	
	SGRX_Camera& CAM = scene->camera;
	BatchRenderer& br = GR2D_GetBatchRenderer();
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, 1, 1 ) );
	
	PixelShaderHandle pppsh_sky = GR_GetPixelShader( "sys_pp_sky" );
	GR_PreserveResource( pppsh_sky );
	
	br.Reset();
	br.ShaderData.resize( 8 );
	memcpy( &br.ShaderData[0], &CAM.mInvView, sizeof(Mat4) );
	br.ShaderData[4] = V4( CAM.position, 1 );
	br.ShaderData[5] = V4( scene->fogColor.Pow( 2.0 ), scene->fogHeightFactor );
	br.ShaderData[6] = V4( scene->fogDensity, scene->fogHeightDensity, scene->fogStartHeight, scene->fogMinDist );
	float fsx = safe_fdiv( 1, CAM.mProj.m[0][0] );
	float fsy = safe_fdiv( 1, CAM.mProj.m[1][1] );
	br.ShaderData[7] = V4( -fsx, fsy, fsx, -fsy );
	
	br.RenderState.depthEnable = 1;
	br.RenderState.depthWriteEnable = 0;
	
	br.SetShader( pppsh_sky );
	br.SetTexture( scene->skyTexture );
	br.Quad( 0, 0, 1, 1, 1 ).Flush().Reset();
}

int SGRX_RenderDirector::GetModeCount()
{
	return 3;
}

void SGRX_RenderDirector::SetMode( int mode )
{
	int modeCount = GetModeCount();
	if( mode >= 0 && mode < modeCount )
		m_curMode = mode;
	else
		ASSERT( !"SGRX_RenderDirector::SetMode - invalid mode ID" );
}



void ParseDefaultTextureFlags( const StringView& flags, uint32_t& outusageflags, uint8_t& outlod )
{
	if( flags.contains( ":nosrgb" ) ) outusageflags &= ~TEXFLAGS_SRGB;
	if( flags.contains( ":srgb" ) ) outusageflags |= TEXFLAGS_SRGB;
	if( flags.contains( ":wrapx" ) ) outusageflags &= ~TEXFLAGS_CLAMP_X;
	if( flags.contains( ":wrapy" ) ) outusageflags &= ~TEXFLAGS_CLAMP_Y;
	if( flags.contains( ":wrapz" ) ) outusageflags &= ~TEXFLAGS_CLAMP_Z;
	if( flags.contains( ":clampx" ) ) outusageflags |= TEXFLAGS_CLAMP_X;
	if( flags.contains( ":clampy" ) ) outusageflags |= TEXFLAGS_CLAMP_Y;
	if( flags.contains( ":clampz" ) ) outusageflags |= TEXFLAGS_CLAMP_Z;
	if( flags.contains( ":nolerp" ) ) outusageflags &= ~TEXFLAGS_LERP;
	if( flags.contains( ":lerp" ) ) outusageflags |= TEXFLAGS_LERP;
	if( flags.contains( ":nomips" ) ) outusageflags &= ~TEXFLAGS_HASMIPS;
	if( flags.contains( ":mips" ) ) outusageflags |= TEXFLAGS_HASMIPS;
	
	size_t pos = flags.find_first_at( ":lod" );
	if( pos != NOT_FOUND )
	{
		outlod = (uint8_t) String_ParseInt( flags.part( pos + 4 ) );
	}
}

bool IGame::OnConfigure( int argc, char** argv )
{
	for( int i = 1; i < argc; )
	{
		int skip = OnArgument( argv[ i ], argc - i - 1, &argv[ i + 1 ] );
		ASSERT( skip >= 0 );
		if( skip <= 0 )
			skip = 1;
		i += skip;
	}
	return true;
}

int IGame::OnArgument( char* arg, int argcleft, char** argvleft )
{
	if( streq( arg, "D" ) ) return 1; // already processed
	if( streq( arg, "-renderer" ) && argcleft )
	{
		g_RendererName = argvleft[0];
		return 2;
	}
	if( streq( arg, "-display" ) && argcleft )
	{
		g_RenderSettings.display = String_ParseInt( argvleft[0] );
		return 2;
	}
	if( streq( arg, "-width" ) && argcleft )
	{
		g_RenderSettings.width = String_ParseInt( argvleft[0] );
		return 2;
	}
	if( streq( arg, "-height" ) && argcleft )
	{
		g_RenderSettings.height = String_ParseInt( argvleft[0] );
		return 2;
	}
	if( streq( arg, "-rate" ) && argcleft )
	{
		g_RenderSettings.refresh_rate = String_ParseInt( argvleft[0] );
		return 2;
	}
	if( streq( arg, "-windowed" ) )
	{
		g_RenderSettings.fullscreen = argcleft ? !String_ParseInt( argvleft[0] ) : false;
		return argcleft ? 2 : 1;
	}
	if( streq( arg, "-fullscreen" ) )
	{
		g_RenderSettings.fullscreen = argcleft ? String_ParseInt( argvleft[0] ) : true;
		return argcleft ? 2 : 1;
	}
	if( streq( arg, "-vsync" ) && argcleft )
	{
		g_RenderSettings.vsync = String_ParseBool( argvleft[0] );
		return 2;
	}
	return 0;
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
		out.cullMode = SGRX_RS_CullMode_None;
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
	
	if( mtl.flags & SGRX_MtlFlag_VCol )
		name.append( ":VCOL" ); // color multiplied by vertex color
	if( mtl.flags & SGRX_MtlFlag_Decal )
		name.append( ":DECAL" ); // ???
	
	PS = GR_GetPixelShader( name );
	
	if( MI->IsSkinned() )
		name.append( ":SKIN" );
	
	VS = GR_GetVertexShader( name );
}

TextureHandle IGame::OnCreateSysTexture( const StringView& key )
{
	if( key == "sys:lut_default" )
	{
		uint32_t data[8] =
		{
			COLOR_RGB(0,0,0), COLOR_RGB(255,0,0), COLOR_RGB(0,255,0), COLOR_RGB(255,255,0),
			COLOR_RGB(0,0,255), COLOR_RGB(255,0,255), COLOR_RGB(0,255,255), COLOR_RGB(255,255,255),
		};
		return GR_CreateTexture3D( 2, 2, 2, TEXFORMAT_RGBA8,
			TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y | TEXFLAGS_CLAMP_Z, 1, data );
	}
	
	return NULL;
}

HFileReader IGame::OnLoadTexture( const StringView& key, uint32_t& outusageflags, uint8_t& outlod )
{
	LOG_FUNCTION;
	
	if( !key )
		return NULL;
	
	StringView path = key.until( ":", 1 );
	
	// try .stx (optimized) before original
	HFileReader out = FS_OpenBinaryFile( String_Concat( path, ".stx" ) );
	if( !out )
		out = FS_OpenBinaryFile( path );
	if( !out )
		return NULL;
	
	outusageflags = TEXFLAGS_HASMIPS | TEXFLAGS_LERP;
	if( path.contains( "diff." ) )
	{
		// diffuse maps
		outusageflags |= TEXFLAGS_SRGB;
	}
	
	StringView flags = key.from( ":", 1 );
	ParseDefaultTextureFlags( flags, outusageflags, outlod );
	
	return out;
}

void IGame::GetShaderCacheFilename( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, String& name )
{
	LOG_FUNCTION;
	
	name = SGRXPATH_CACHE_SHADERS;
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
	
	String filename = SGRXPATH_SRC_SHADERS;
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
	
	StringView path = key.until( ":", 1 );
	
	if( !FS_LoadBinaryFile( path, outdata ) )
		return false;
	
	return true;
}

MeshHandle IGame::OnCreateSysMesh( const StringView& key )
{
	if( key == "sys:cube" )
	{
		static const SGRX_Vertex_Decal verts[ 4 * 6 ] =
		{
			// +X
			{ V3(+1,-1,+1), V3(1,0,0), V3(0,0,0), COLOR_RGB(127,255,127), 0xffffffff, 0 },
			{ V3(+1,+1,+1), V3(1,0,0), V3(1,0,0), COLOR_RGB(127,255,127), 0xffffffff, 0 },
			{ V3(+1,+1,-1), V3(1,0,0), V3(1,1,0), COLOR_RGB(127,255,127), 0xffffffff, 0 },
			{ V3(+1,-1,-1), V3(1,0,0), V3(0,1,0), COLOR_RGB(127,255,127), 0xffffffff, 0 },
			// -X
			{ V3(-1,+1,+1), V3(1,0,0), V3(0,0,0), COLOR_RGB(127,000,127), 0xffffffff, 0 },
			{ V3(-1,-1,+1), V3(1,0,0), V3(1,0,0), COLOR_RGB(127,000,127), 0xffffffff, 0 },
			{ V3(-1,-1,-1), V3(1,0,0), V3(1,1,0), COLOR_RGB(127,000,127), 0xffffffff, 0 },
			{ V3(-1,+1,-1), V3(1,0,0), V3(0,1,0), COLOR_RGB(127,000,127), 0xffffffff, 0 },
			// +Y
			{ V3(+1,+1,+1), V3(0,1,0), V3(0,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,+1,+1), V3(0,1,0), V3(1,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,+1,-1), V3(0,1,0), V3(1,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(+1,+1,-1), V3(0,1,0), V3(0,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			// -Y
			{ V3(-1,-1,+1), V3(0,1,0), V3(0,0,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(+1,-1,+1), V3(0,1,0), V3(1,0,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(+1,-1,-1), V3(0,1,0), V3(1,1,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(-1,-1,-1), V3(0,1,0), V3(0,1,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			// +Z
			{ V3(+1,-1,+1), V3(0,0,1), V3(0,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,-1,+1), V3(0,0,1), V3(1,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,+1,+1), V3(0,0,1), V3(1,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(+1,+1,+1), V3(0,0,1), V3(0,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			// -Z
			{ V3(-1,-1,-1), V3(0,0,1), V3(0,0,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(+1,-1,-1), V3(0,0,1), V3(1,0,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(+1,+1,-1), V3(0,0,1), V3(1,1,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(-1,+1,-1), V3(0,0,1), V3(0,1,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
		};
		static const uint16_t indices[ 6 * 6 ] =
		{
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4,
			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20,
		};
		static const SGRX_MeshPart part =
		{
			0, 4 * 6, 0, 6 * 6,
			"default",
			{ "textures/unit.png", "", "", "", "", "", "", "" },
			0, SGRX_MtlBlend_None,
			"", Mat4::Identity
		};
		
		MeshHandle mesh = GR_CreateMesh();
		mesh->SetVertexData( verts, sizeof(verts), GR_GetVertexDecl( SGRX_VDECL_DECAL ) );
		mesh->SetIndexData( indices, sizeof(indices), false );
		mesh->SetPartData( &part, 1 );
		mesh->m_boundsMin = V3(-1);
		mesh->m_boundsMax = V3(1);
		mesh->m_vdata.append( verts, sizeof(verts) );
		mesh->m_idata.append( indices, sizeof(indices) );
		return mesh;
	}
	return NULL;
}


bool IFileReader::Read( uint32_t num, uint8_t* out )
{
	return TryRead( num, out ) == num;
}

bool IFileReader::ReadAll( ByteArray& out )
{
	out.resize( Length() );
	Seek( 0 );
	return Read( out.size(), out.data() );
}

struct BasicFileReader : IFileReader
{
	BasicFileReader( FILE* f ) : fp( f )
	{
		fseek( f, 0, SEEK_END );
		len = Tell();
		fseek( f, 0, SEEK_SET );
	}
	~BasicFileReader(){ Close(); }
	void Close()
	{
		if( fp )
		{
			fclose( fp );
			fp = NULL;
		}
	}
	uint64_t Length(){ return len; }
	bool Seek( uint64_t pos )
	{
		return fseek( fp, pos, SEEK_SET ) != -1;
	}
	uint64_t Tell()
	{
		return ftell( fp );
	}
	uint32_t TryRead( uint32_t num, uint8_t* out )
	{
		return fread( out, 1, num, fp );
	}
	
	FILE* fp;
	uint64_t len;
};


#define BFS__COMMON( fret ) \
	if( ppath.size() + root.size() > 1024 ) \
		return fret \
	memcpy( bfr, root.data(), root.size() ); \
	memcpy( bfr + root.size(), ppath.data(), ppath.size() ); \
	bfr[ root.size() + ppath.size() ] = 0; \
	cpath = StringView( bfr, root.size() + ppath.size() );
#define BFS_PROLOG( fret ) \
	LOG_FUNCTION; \
	char bfr[ 1025 ]; \
	StringView root = path.ch() == ':' ? m_fileRoot : m_fileRootCache; \
	StringView cpath, ppath = path.ch() == ':' ? path.part( 1 ) : path; \
	BFS__COMMON( fret )
#define BFS_CONT( fret ) \
	if( path.ch() == ':' ) \
		return fret \
	root = m_fileRootSrc; \
	BFS__COMMON( fret )

BasicFileSystem::BasicFileSystem( const StringView& root ) : m_fileRoot(root)
{
	if( m_fileRoot.size() && m_fileRoot.last() != '/' )
		m_fileRoot.push_back( '/' );
	m_fileRootCache = m_fileRoot;
	m_fileRootCache.append( "cache/" );
	m_fileRootSrc = m_fileRoot;
	m_fileRootSrc.append( "src/" );
}

HFileReader BasicFileSystem::OpenBinaryFile( const StringView& path )
{
	BFS_PROLOG( NULL; );
	FILE* f = fopen( bfr, "rb" );
	if( f )
		return new BasicFileReader( f );
	BFS_CONT( NULL; );
	f = fopen( bfr, "rb" );
	if( f )
		return new BasicFileReader( f );
	return NULL;
}

bool BasicFileSystem::LoadBinaryFile( const StringView& path, ByteArray& out )
{
	BFS_PROLOG( false; );
	if( ::LoadBinaryFile( cpath, out ) )
		return true;
	BFS_CONT( false; );
	return ::LoadBinaryFile( cpath, out );
}

bool BasicFileSystem::SaveBinaryFile( const StringView& path, const void* data, size_t size )
{
	BFS_PROLOG( false; );
	if( ::SaveBinaryFile( cpath, data, size ) )
		return true;
	BFS_CONT( false; );
	return ::SaveBinaryFile( cpath, data, size );
}

bool BasicFileSystem::LoadTextFile( const StringView& path, String& out )
{
	BFS_PROLOG( false; );
	if( ::LoadTextFile( cpath, out ) )
		return true;
	BFS_CONT( false; );
	return ::LoadTextFile( cpath, out );
}

bool BasicFileSystem::SaveTextFile( const StringView& path, const StringView& data )
{
	BFS_PROLOG( false; );
	if( ::SaveTextFile( cpath, data ) )
		return true;
	BFS_CONT( false; );
	return ::SaveTextFile( cpath, data );
}

bool BasicFileSystem::FindRealPath( const StringView& path, String& out )
{
	BFS_PROLOG( false; );
	if( ::FSItemExists( cpath ) )
	{
		out = cpath;
		return true;
	}
	BFS_CONT( false; );
	if( ::FSItemExists( cpath ) )
	{
		out = cpath;
		return true;
	}
	return false;
}

bool BasicFileSystem::FileExists( const StringView& path )
{
	BFS_PROLOG( false; );
	if( ::FileExists( cpath ) )
		return true;
	BFS_CONT( false; );
	return ::FileExists( cpath );
}

bool BasicFileSystem::DirCreate( const StringView& path )
{
	BFS_PROLOG( false; );
	if( ::DirCreate( cpath ) )
		return true;
	BFS_CONT( false; );
	return ::DirCreate( cpath );
}

uint32_t BasicFileSystem::FileModTime( const StringView& path )
{
	BFS_PROLOG( 0; );
	uint32_t t = ::FileModTime( cpath );
	if( t )
		return t;
	BFS_CONT( 0; );
	return ::FileModTime( cpath );
}

void BasicFileSystem::IterateDirectory( const StringView& path, IDirEntryHandler* deh )
{
	BFS_PROLOG( ; );
	{
		DirectoryIterator tdi( cpath );
		while( tdi.Next() )
		{
			if( !deh->HandleDirEntry( path, tdi.Name(), tdi.IsDirectory() ) )
				return;
		}
	}
	BFS_CONT( ; );
	{
		DirectoryIterator tdi( cpath );
		while( tdi.Next() )
		{
			if( !deh->HandleDirEntry( path, tdi.Name(), tdi.IsDirectory() ) )
				return;
		}
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

HFileReader FS_OpenBinaryFile( const StringView& path )
{
	LOG_FUNCTION;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
	{
		HFileReader fr = g_FileSystems[ i ]->OpenBinaryFile( path );
		if( fr )
			return fr;
	}
	return NULL;
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


static SGRX_RenderDirector g_DefaultRenderDirector;

SGRX_RenderDirector* GR_GetDefaultRenderDirector()
{
	return &g_DefaultRenderDirector;
}

SGRX_RenderScene::SGRX_RenderScene(
	const Vec4& tv,
	const SceneHandle& sh,
	bool enablePP
) :
	timevals( tv ),
	scene( sh ),
	enablePostProcessing( enablePP ),
	viewport( NULL ),
	postdraw( NULL ),
	debugdraw( NULL )
{}

void GR_RenderScene( SGRX_RenderScene& info )
{
	g_BatchRenderer->Flush();
	g_BatchRenderer->Reset();
	
	info.scene->m_timevals = info.timevals;
	g_Renderer->_RS_PreProcess( info.scene );
	info.scene->director->OnDrawScene( g_Renderer, info );
	
	for( size_t i = 0; i < SGRX_MAX_TEXTURES; ++i )
		g_Renderer->m_overrideTextures[ i ] = NULL;
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
	
	LOG << LOG_DATE << "  Freeing internal graphics resources";
	
	g_Renderer->UnloadInternalResources();
	g_Renderer->_RS_ProjectorFree();
	
	SGRX_INT_DestroyBatchRendering();
	SGRX_INT_DestroyResourceTables();
	
	LOG << LOG_DATE << "  Destroying renderer";
	
	g_Renderer->Destroy();
	g_Renderer = NULL;
	
	LOG << LOG_DATE << "  Unloading renderer library";
	
	g_RfnFree();
	
	SDL_UnloadObject( g_RenderLib );
	g_RenderLib = NULL;
	
	LOG << LOG_DATE << "  Destroying main window";
	
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
	
	SGRX_LogOutputStdout los;
	if( debug )
		los.Register();
	
#if 1
	LOG << "Engine self-test...";
	int ret = TestSystems();
	if( ret )
	{
		LOG << "Test FAILED with code: " << ret;
		return 1;
	}
	LOG << "Test completed successfully.";
#endif
	
	LOG << LOG_DATE << "  Engine started";
	
	if( !read_config() )
		return 4;
	
	for( int i = 1; i < argc; ++i )
	{
		if( strpeq( argv[i], "-game=" ) )
		{
			g_GameLibName = argv[i] + STRLIT_LEN("-game=");
			LOG << "ARG: Game library: " << g_GameLibName;
		}
		else if( strpeq( argv[i], "-dir=" ) )
		{
			g_GameDir = argv[i] + STRLIT_LEN("-dir=");
			LOG << "ARG: Game directory: " << g_GameDir;
		}
		else if( strpeq( argv[i], "-dir2=" ) )
		{
			g_GameDir = argv[i] + STRLIT_LEN("-dir2=");
			LOG << "ARG: Game directory #2: " << g_GameDir2;
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
			if( event.type == SDL_DROPFILE )
			{
				SDL_free( event.drop.file );
			}
		}
		if( g_PCPLFR == 2 )
			g_PCPLFR = 1;
		if( g_PCPE )
		{
			g_PCPE = false;
			if( g_PCPLFR == 0 )
				g_PCPLFR = 2;
			Game_SetCursorPos( g_PCPX, g_PCPY );
		}
		else if( g_PCPLFR > 0 )
			g_PCPLFR--;
		
		uint32_t curtime = GetTimeMsec();
		uint32_t dt = curtime - prevtime;
		prevtime = curtime;
		g_GameTime += dt;
		float fdt = dt / 1000.0f;
		
		Game_Process( fdt );
	}
	
	g_Game->OnDestroy();
	g_Game = NULL;
	
	delete g_Joysticks;
	g_Joysticks = NULL;
	
	free_graphics();
	
	LOG << LOG_DATE << "  Unloading game library";
	
	SDL_UnloadObject( g_GameLib );
	
	LOG << LOG_DATE << "  Freeing some internal engine data";
	
	delete g_ActionMap;
	delete g_CObjs;
	
	LOG << LOG_DATE << "  Engine finished";
	return 0;
}

