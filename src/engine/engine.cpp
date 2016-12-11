

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
bool g_VerboseLogging = false;
static SDL_Window* g_Window = NULL;
static void* g_GameLib = NULL;
GameHandle g_Game = NULL;
static uint32_t g_GameTime = 0;
static ActionMap* g_ActionMap;
static CObjMap* g_CObjs;
static Vec2 g_CursorPos = {0,0};
static Vec2 g_PrevCursorPos = {0,0};
static Vec2 g_CursorScale = {0,0};
static int g_PCPX = 0;
static int g_PCPY = 0;
static bool g_PCPE = false;
static int g_PCPLFR = 0;
static bool g_CPDeltaMode = false;
static EventLinksByID g_EventLinksByID;
static EventLinksByHandler g_EventLinksByHandler;
static Array< FileSysHandle > g_FileSystems;
struct RCStringPair
{
	RCString key;
	RCString value;
};
static HashTable< StringView, RCStringPair > g_FSMapping;

static RenderSettings g_RenderSettings = { 0, -1, 1280, 720, 60, FULLSCREEN_NONE, true, ANTIALIAS_NONE, 0 };
static const char* g_RendererPrefix = "sgrx-render-";
static void* g_RenderLib = NULL;
static pfnRndInitialize g_RfnInitialize = NULL;
static pfnRndFree g_RfnFree = NULL;
static pfnRndCreateRenderer g_RfnCreateRenderer = NULL;
IRenderer* g_Renderer = NULL;
static JoystickHashTable* g_Joysticks = NULL;

// current input method info
#define CURRENT_INPUT_METHOD_SWITCH_TIME 0.2f
struct CurrentInputMethod
{
	bool isController;
	bool nextIsController;
	float timeSinceLastChange;
	void LastPress( bool ctrl )
	{
		if( nextIsController != ctrl )
		{
			nextIsController = ctrl;
			timeSinceLastChange = 0;
		}
	}
	void OnFrame( float dt )
	{
		if( timeSinceLastChange > CURRENT_INPUT_METHOD_SWITCH_TIME )
			isController = nextIsController;
		else
			timeSinceLastChange += dt;
	}
}
g_CurrentInputMethod;


CVarInt gcv_r_display( "r_display" );
CVarInt gcv_r_gpu( "r_gpu" );
CVarInt gcv_r_width( "r_width" );
CVarInt gcv_r_height( "r_height" );
CVarInt gcv_r_refresh_rate( "r_refresh_rate" );
CVarInt gcv_r_fullscreen( "r_fullscreen" );
CVarBool gcv_r_vsync( "r_vsync" );

CVarInt gcv_r_scenedbgdraw( "r_scenedbgdraw" );

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
		sgrx_snprintf( bfr, sizeof(bfr), "%d %d %d %d %d %d %d %d %d\n",
			(int) g_RenderSettings.display,
			(int) g_RenderSettings.gpu,
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
			rs.gpu = str.parse_int();
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
	gcv_r_gpu.SetConst( true );
	gcv_r_width.SetConst( true );
	gcv_r_height.SetConst( true );
	gcv_r_refresh_rate.SetConst( true );
	gcv_r_fullscreen.SetConst( true );
	gcv_r_vsync.SetConst( true );
	
	gcv_r_display.value = g_RenderSettings.display;
	gcv_r_gpu.value = g_RenderSettings.gpu;
	gcv_r_width.value = g_RenderSettings.width;
	gcv_r_height.value = g_RenderSettings.height;
	gcv_r_refresh_rate.value = g_RenderSettings.refresh_rate;
	gcv_r_fullscreen.value = g_RenderSettings.fullscreen;
	gcv_r_vsync.value = g_RenderSettings.vsync;
	
	REGCOBJ( gcv_r_display );
	REGCOBJ( gcv_r_gpu );
	REGCOBJ( gcv_r_width );
	REGCOBJ( gcv_r_height );
	REGCOBJ( gcv_r_refresh_rate );
	REGCOBJ( gcv_r_fullscreen );
	REGCOBJ( gcv_r_vsync );
	REGCOBJ( gcv_r_videomode );
	
	REGCOBJ( gcv_r_scenedbgdraw );
	
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

void Window_EnableResizing( bool enable )
{
	SDL_SetWindowResizable( g_Window, enable ? SDL_TRUE : SDL_FALSE );
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
//	LOG << "\n\nREGISTER EVENT HANDLER\n- ptr=" << eh << "\n- id=" << eid << "\n";
	
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
//	LOG << "\n\nUNREGISTER EVENT HANDLER\n- ptr=" << eh << "\n- id=" << eid << "\n";
	
	if( eid == 0 )
	{
		EventLinkArrayHandle links = g_EventLinksByHandler.getcopy( eh );
		if( !links )
			return;
		EventLinkArray& ELA = *links;
		while( ELA.size() )
		{
			Game_UnregisterEventHandler( eh, ELA.last().eid );
		}
	}
	else
	{
		EventLinkEntry ele = { eid, eh };
		
		EventLinkArrayHandle links = g_EventLinksByID.getcopy( eid );
		if( links )
		{
			links->remove_first( ele );
			// keep id-mapped arrays because they're likely to be reused
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

void Game_ListActions( Array< StringView >& out )
{
	out.clear();
	out.reserve( g_ActionMap->m_nameCmdMap.size() );
	for( size_t i = 0; i < g_ActionMap->m_nameCmdMap.size(); ++i )
		out.push_back( g_ActionMap->m_nameCmdMap.item( i ).key );
}

void Game_AddAction( StringView name, float threshold )
{
	g_ActionMap->Register( new InputAction( name, threshold ) );
}

bool Game_ActionExists( StringView name )
{
	return g_ActionMap->FindAction( name ) != NULL;
}

InputData Game_GetActionState( StringView name )
{
	InputAction* act = g_ActionMap->FindAction( name );
	if( act )
		return act->data;
	return InputData();
}

bool Game_GetActionState( StringView name, InputData& out )
{
	InputAction* act = g_ActionMap->FindAction( name );
	if( act )
	{
		out = act->data;
		return true;
	}
	return false;
}

void Game_BindKeyToAction( uint32_t key, StringView name )
{
	g_ActionMap->Map( ACTINPUT_MAKE_KEY( key ), name );
}

void Game_BindMouseButtonToAction( int btn, StringView name )
{
	g_ActionMap->Map( ACTINPUT_MAKE_MOUSE( btn ), name );
}

void Game_BindGamepadButtonToAction( int btn, StringView name )
{
	g_ActionMap->Map( ACTINPUT_MAKE_GPADBTN( btn ), name );
}

void Game_BindGamepadAxisToAction( int axis, StringView name )
{
	g_ActionMap->Map( ACTINPUT_MAKE_GPADAXIS( axis ), name );
}

ActionInput Game_GetActionBinding( StringView name )
{
	ActionMap::InputCmdMap* icm = &g_ActionMap->m_inputCmdMap;
	for( size_t i = 0; i < icm->size(); ++i )
	{
		if( icm->item(i).value->name == name )
			return icm->item(i).key;
	}
	return 0;
}

int Game_GetActionBindings( StringView name, ActionInput* out, int bufsize )
{
	int num = 0;
	ActionMap::InputCmdMap* icm = &g_ActionMap->m_inputCmdMap;
	for( size_t i = 0; i < icm->size() && num < bufsize; ++i )
	{
		if( icm->item(i).value->name == name )
			out[ num++ ] = icm->item(i).key;
	}
	return num;
}

void Game_BindInputToAction( ActionInput iid, StringView name )
{
	g_ActionMap->Map( iid, name );
}

void Game_UnbindInput( ActionInput iid )
{
	g_ActionMap->Unmap( iid );
}

ActionInput Game_GetInputFromNameID( StringView name )
{
	if( name == "btn:Left" ) return ACTINPUT_MAKE_MOUSE( SGRX_MB_LEFT );
	if( name == "btn:Right" ) return ACTINPUT_MAKE_MOUSE( SGRX_MB_RIGHT );
	if( name == "btn:Middle" ) return ACTINPUT_MAKE_MOUSE( SGRX_MB_MIDDLE );
	if( name == "btn:X1" ) return ACTINPUT_MAKE_MOUSE( SGRX_MB_X1 );
	if( name == "btn:X2" ) return ACTINPUT_MAKE_MOUSE( SGRX_MB_X2 );
	
	if( name == "ctrl:LeftX" ) return ACTINPUT_MAKE_GPADAXIS( SDL_CONTROLLER_AXIS_LEFTX );
	if( name == "ctrl:LeftY" ) return ACTINPUT_MAKE_GPADAXIS( SDL_CONTROLLER_AXIS_LEFTY );
	if( name == "ctrl:RightX" ) return ACTINPUT_MAKE_GPADAXIS( SDL_CONTROLLER_AXIS_RIGHTX );
	if( name == "ctrl:RightY" ) return ACTINPUT_MAKE_GPADAXIS( SDL_CONTROLLER_AXIS_RIGHTY );
	if( name == "ctrl:LefttStick" ) return ACTINPUT_MAKE_GPADAXIS( SDL_CONTROLLER_BUTTON_LEFTSTICK );
	if( name == "ctrl:RightStick" ) return ACTINPUT_MAKE_GPADAXIS( SDL_CONTROLLER_BUTTON_RIGHTSTICK );
	if( name == "ctrl:Start" ) return ACTINPUT_MAKE_GPADAXIS( SDL_CONTROLLER_BUTTON_START );
	if( name == "ctrl:A" ) return ACTINPUT_MAKE_GPADBTN( SDL_CONTROLLER_BUTTON_A );
	if( name == "ctrl:B" ) return ACTINPUT_MAKE_GPADBTN( SDL_CONTROLLER_BUTTON_B );
	if( name == "ctrl:X" ) return ACTINPUT_MAKE_GPADBTN( SDL_CONTROLLER_BUTTON_X );
	if( name == "ctrl:Y" ) return ACTINPUT_MAKE_GPADBTN( SDL_CONTROLLER_BUTTON_Y );
	if( name == "ctrl:LB" ) return ACTINPUT_MAKE_GPADBTN( SDL_CONTROLLER_BUTTON_LEFTSHOULDER );
	if( name == "ctrl:RB" ) return ACTINPUT_MAKE_GPADBTN( SDL_CONTROLLER_BUTTON_RIGHTSHOULDER );
	
	if( name.starts_with( "key:" ) )
	{
		SDL_Keycode key = SDL_GetKeyFromName( StackString<64>( name.part( 4 ) ) );
		if( key != SDLK_UNKNOWN )
			return ACTINPUT_MAKE_KEY( key );
	}
	
	return ACTINPUT_UNASSIGNED;
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

Vec2 Game_GetPrevCursorPos()
{
	return g_PrevCursorPos;
}

SGRX_CurrentInputMethod Input_GetCurrentMethod()
{
	return g_CurrentInputMethod.isController
		? SGRX_CIM_Controller
		: SGRX_CIM_KeyboardMouse;
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

void Game_SetCursorDeltaMode( bool dmode )
{
	g_CPDeltaMode = dmode;
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
		case SDL_WINDOWEVENT_RESIZED: {
			RenderSettings rs;
			GR_GetVideoMode( rs );
			rs.width = e.window.data1;
			rs.height = e.window.data2;
			GR_SetVideoMode( rs );
			} break;
		}
	}
	
	if( e.type == SDL_CONTROLLERDEVICEADDED )
	{
		g_CurrentInputMethod.LastPress( true );
		g_Joysticks->set( e.cdevice.which, new SGRX_Joystick( e.cdevice.which ) );
	}
	else if( e.type == SDL_CONTROLLERDEVICEREMOVED )
	{
		if( g_Joysticks->size() == 0 )
			g_CurrentInputMethod.LastPress( false );
		g_Joysticks->unset( e.cdevice.which );
	}
	
	if( e.type == SDL_MOUSEMOTION )
	{
		g_CurrentInputMethod.LastPress( false );
		g_CursorPos.x = e.motion.x;
		g_CursorPos.y = e.motion.y;
	}
	
	if( e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP )
	{
		g_CurrentInputMethod.LastPress( true );
		InputAction* iact = g_ActionMap->Get( ACTINPUT_MAKE_GPADBTN( e.cbutton.button ) );
		if( iact )
			iact->_SetState( e.cbutton.state );
	}
	else if( e.type == SDL_CONTROLLERAXISMOTION )
	{
		g_CurrentInputMethod.LastPress( true );
		InputAction* iact = g_ActionMap->Get( ACTINPUT_MAKE_GPADAXIS( e.caxis.axis ) );
		if( iact )
			iact->_SetState( e.caxis.value / 32767.0f );
	}
	
	if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP )
	{
		g_CurrentInputMethod.LastPress( false );
		InputAction* iact = g_ActionMap->Get( ACTINPUT_MAKE_KEY( e.key.keysym.sym ) );
		if( iact )
			iact->_SetState( e.key.state );
	}
	
	if( e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP )
	{
		g_CurrentInputMethod.LastPress( false );
		int btn = -1;
		if( e.button.button == SDL_BUTTON_LEFT ) btn = SGRX_MB_LEFT;
		if( e.button.button == SDL_BUTTON_RIGHT ) btn = SGRX_MB_RIGHT;
		if( e.button.button == SDL_BUTTON_MIDDLE ) btn = SGRX_MB_MIDDLE;
		if( btn >= 0 )
		{
			InputAction* iact = g_ActionMap->Get( ACTINPUT_MAKE_MOUSE( btn ) );
			if( iact )
				iact->_SetState( e.button.state );
		}
	}
	
	g_Game->OnEvent( e );
	
	Game_FireEvent( EID_WindowEvent, EventData( (void*) &e ) );
}

static void renderer_clear_rts()
{
	SGRX_RTClearInfo info = { SGRX_RT_ClearAll, 0, 0, 1 };
	SGRX_RTSpec rts[4] = { SGRX_RTSpec(), SGRX_RTSpec(), SGRX_RTSpec(), SGRX_RTSpec() };
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
	
	g_CurrentInputMethod.OnFrame( dt );
	g_Game->OnTick( dt, g_GameTime );
	
	GR2D_GetBatchRenderer().Flush();
	
	{
		LOG_FUNCTION_ARG( "PRESENT_MAYBE_VSYNC" );
		g_Renderer->Swap();
	}
	renderer_clear_rts();
}


void Game_End()
{
	g_Running = false;
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
	if( streq( arg, "-gpu" ) && argcleft )
	{
		g_RenderSettings.gpu = String_ParseInt( argvleft[0] );
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
	if( strpeq( arg, "-ccmd:" ) )
	{
		Game_DoCommand( SV(arg).part(6) );
		return 1;
	}
	if( strpeq( arg, "-cvar:" ) )
	{
		StringView cv = SV(arg).part(6);
		CObj* obj = Game_FindCObj( cv.until("=") );
		if( obj )
			obj->DoCommand( cv.after("=") );
		return 1;
	}
	return 0;
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

BasicFileSystem::BasicFileSystem( const StringView& root ) : m_fileRoot(root.str())
{
	if( m_fileRoot.size() && m_fileRoot.last() != '/' )
		m_fileRoot.push_back( '/' );
	m_fileRootCache = m_fileRoot;
	m_fileRootCache.append( "cache/" );
	m_fileRootSrc = m_fileRoot;
	m_fileRootSrc.append( "src/" );
	
	// load asset mappings
	String text;
	if( LoadTextFile( "assets/mapping.txt", text ) )
	{
		ConfigReader cr( text );
		StringView key, val;
		while( cr.Read( key, val ) )
		{
			val = val.after( ":" );
			String path = "assets/";
			path.append( key );
			RCStringPair rcsp = { val, path };
			g_FSMapping.set( rcsp.key, rcsp );
		}
	}
	if( LoadTextFile( "assets/mapping.legacy.txt", text ) )
	{
		ConfigReader cr( text );
		StringView key, val;
		while( cr.Read( key, val ) )
		{
			String path = "assets/";
			path.append( key );
			RCStringPair rcsp = { val, path };
			g_FSMapping.set( rcsp.key, rcsp );
		}
	}
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

void FS_ResolvePath( StringView& path, char resbfr[50] )
{
	RCStringPair* rcsp = g_FSMapping.getptr( path );
	if( rcsp )
		path = rcsp->value;
	else if( path.size() == 17 && path[0] == '\0' )
	{
		SGRX_GUID guid;
		memcpy( guid.bytes, &path[1], 16 );
		strcpy( resbfr, "assets/" );
		guid.ToCharArray( resbfr + 7 );
		path = resbfr;
	}
	else if( path.size() == GUID_STRING_LENGTH )
	{
		SGRX_GUID guid = SGRX_GUID::ParseString( path );
		if( guid.NotNull() )
		{
			strcpy( resbfr, "assets/" );
			guid.ToCharArray( resbfr + 7 );
			path = resbfr;
		}
	}
}

HFileReader FS_OpenBinaryFile( StringView path )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
	{
		HFileReader fr = g_FileSystems[ i ]->OpenBinaryFile( path );
		if( fr )
			return fr;
	}
	return NULL;
}

bool FS_LoadBinaryFile( StringView path, ByteArray& out )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->LoadBinaryFile( path, out ) )
			return true;
	return false;
}

bool FS_SaveBinaryFile( StringView path, const void* data, size_t size )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->SaveBinaryFile( path, data, size ) )
			return true;
	return false;
}

bool FS_LoadTextFile( StringView path, String& out )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->LoadTextFile( path, out ) )
			return true;
	return false;
}

bool FS_SaveTextFile( StringView path, StringView data )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->SaveTextFile( path, data ) )
			return true;
	return false;
}

bool FS_FindRealPath( StringView path, String& out )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->FindRealPath( path, out ) )
			return true;
	return false;
}

bool FS_FileExists( StringView path )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->FileExists( path ) )
			return true;
	return false;
}

bool FS_DirCreate( StringView path )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->DirCreate( path ) )
			return true;
	return false;
}

uint32_t FS_FileModTime( StringView path )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	uint32_t t;
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( ( t = g_FileSystems[ i ]->FileModTime( path ) ) != 0 )
			return t;
	return 0;
}

void FS_IterateDirectory( StringView path, IDirEntryHandler* deh )
{
	LOG_FUNCTION;
	FS_RESOLVE_PATH( path );
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		g_FileSystems[ i ]->IterateDirectory( path, deh );
}


int GR_GetWidth(){ return g_RenderSettings.width; }
int GR_GetHeight(){ return g_RenderSettings.height; }


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
	if( VERBOSE ) LOG << "CURSOR REMAP: " << g_CursorScale;
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
		if( key == "dir" )
		{
			if( value.size() )
			{
				g_GameDir = value;
				if( VERBOSE ) LOG << "CONFIG: Game directory: " << value;
			}
		}
#if 0
		else if( key == "game" )
		{
			if( value.size() )
			{
				g_GameLibName = value;
				if( VERBOSE ) LOG << "CONFIG: Game library: " << value;
			}
		}
		else if( key == "dir2" )
		{
			if( value.size() )
			{
				g_GameDir2 = value;
				if( VERBOSE ) LOG << "CONFIG: Game directory #2: " << value;
			}
		}
		else if( key == "renderer" )
		{
			if( value.size() )
			{
				g_RendererName = value;
				if( VERBOSE ) LOG << "CONFIG: Renderer: " << value;
			}
		}
#endif
		else
		{
			LOG_WARNING << "Unknown key (" << key << " = " << value << ")";
		}
	}
	
	return true;
}

static bool read_game_config()
{
	LOG_FUNCTION;
	
	String text;
	if( !FS_LoadTextFile( "game.cfg", text ) )
	{
		LOG << "Failed to load game.cfg";
		return false;
	}
	
	ConfigReader cr( text );
	StringView key, value;
	while( cr.Read( key, value ) )
	{
		if( key == "module" )
		{
			if( value.size() )
			{
				g_GameLibName = value;
				if( VERBOSE ) LOG << "CONFIG: Game library: " << value;
			}
		}
		else if( key == "dir2" )
		{
			if( value.size() )
			{
				g_GameDir2 = value;
				if( VERBOSE ) LOG << "CONFIG: Game directory #2: " << value;
			}
		}
		else if( key == "renderer" )
		{
			if( value.size() )
			{
				g_RendererName = value;
				if( VERBOSE ) LOG << "CONFIG: Renderer: " << value;
			}
		}
	//	else
	//	{
	//		LOG_WARNING << "Unknown key (" << key << " = " << value << ")";
	//	}
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
	unsigned cwp = SDL_WINDOWPOS_CENTERED_DISPLAY( g_RenderSettings.display );
	g_Window = SDL_CreateWindow( "SGRX Engine", cwp, cwp, g_RenderSettings.width, g_RenderSettings.height, flags );
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
	if( VERBOSE ) LOG << LOG_DATE << "  Loaded renderer: " << rendername;
	
	SGRX_INT_InitResourceTables();
	if( VERBOSE ) LOG << LOG_DATE << "  Created renderer resource caches";
	SGRX_INT_InitBatchRendering();
	
	g_Joysticks = new JoystickHashTable();
	
	g_Renderer->_RS_ProjectorInit();
	if( !g_Renderer->LoadInternalResources() )
	{
		LOG_ERROR << "Failed to load renderer (" << rendername << ") internal resources";
		return 106;
	}
	if( VERBOSE ) LOG << LOG_DATE << "  Loaded internal renderer resources";
	
	return 0;
}

static void free_graphics()
{
	LOG_FUNCTION;
	
	if( VERBOSE ) LOG << LOG_DATE << "  Freeing internal graphics resources";
	
	g_Renderer->UnloadInternalResources();
	g_Renderer->_RS_ProjectorFree();
	
	SGRX_INT_DestroyBatchRendering();
	SGRX_INT_DestroyResourceTables();
	
	if( VERBOSE ) LOG << LOG_DATE << "  Destroying renderer";
	
	g_Renderer->Destroy();
	g_Renderer = NULL;
	
	if( VERBOSE ) LOG << LOG_DATE << "  Unloading renderer library";
	
	g_RfnFree();
	
	SDL_UnloadObject( g_RenderLib );
	g_RenderLib = NULL;
	
	if( VERBOSE ) LOG << LOG_DATE << "  Destroying main window";
	
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
				LOG_WARNING << "Failed to set fullscreen mode: unrecognized mode";
			}
			else
			{
				LOG_WARNING << "Failed to set fullscreen mode: " << SDL_GetError();
			}
			return false;
		}
		
		// set window size
		SDL_SetWindowSize( g_Window, rs.width, rs.height );
		SDL_DisplayMode dm = { SDL_PIXELFORMAT_RGBX8888, rs.width, rs.height, rs.refresh_rate, NULL };
		ret = SDL_SetWindowDisplayMode( g_Window, &dm );
		if( ret < 0 )
		{
			LOG_WARNING << "Failed to set display mode: " << SDL_GetError();
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
	gcv_r_gpu.value = g_RenderSettings.gpu;
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
		LOG_WARNING << "Failed to get display mode count (display=" << display << "): " << SDL_GetError();
		return false;
	}
	
	out.reserve( numdm );
	
	SDL_DisplayMode dmd;
	for( int i = 0; i < numdm; ++i )
	{
		int err = SDL_GetDisplayMode( display, i, &dmd );
		if( err < 0 )
		{
			LOG_WARNING << "Failed to get display mode (display=" << display << ", mode = " << i << "): " << SDL_GetError();
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
	g_VerboseLogging = debug >= 2;
	if( debug )
		los.Register();
	
#if 1
	if( VERBOSE ) LOG << "Engine self-test...";
	int ret = TestSystems();
	if( ret )
	{
		LOG << "Test FAILED with code: " << ret;
		return 1;
	}
	if( VERBOSE ) LOG << "Test completed successfully.";
#endif
	
	if( VERBOSE ) LOG << LOG_DATE << "  Engine started";
	
	if( !read_config() )
		return 4;
	
	for( int i = 1; i < argc; ++i )
	{
		if( strpeq( argv[i], "-game=" ) )
		{
			g_GameDir = argv[i] + STRLIT_LEN("-game=");
			if( VERBOSE ) LOG << "ARG: Game directory: " << g_GameDir;
		}
#if 0
		else if( strpeq( argv[i], "-lib=" ) )
		{
			g_GameLibName = argv[i] + STRLIT_LEN("-lib=");
			if( VERBOSE ) LOG << "ARG: Game library: " << g_GameLibName;
		}
		else if( strpeq( argv[i], "-dir2=" ) )
		{
			g_GameDir = argv[i] + STRLIT_LEN("-dir2=");
			if( VERBOSE ) LOG << "ARG: Game directory #2: " << g_GameDir2;
		}
#endif
	}
	
	g_FileSystems.push_back( new BasicFileSystem( g_GameDir ) );
	
	if( !read_game_config() )
		return 5;
	
	if( g_GameDir2.size() )
		g_FileSystems.push_back( new BasicFileSystem( g_GameDir2 ) );
	
//	if( !CWDSet( g_GameDir ) )
//	{
//		LOG_ERROR << "FAILED TO SET GAME DIRECTORY";
//		return 12;
//	}
	
	/* initialize SDL */
	if( SDL_Init(
		SDL_INIT_VIDEO |
		SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC |
		SDL_INIT_GAMECONTROLLER
	) < 0 )
	{
		LOG_ERROR << "Couldn't initialize SDL: " << SDL_GetError();
		return 15;
	}
	SDL_JoystickEventState( SDL_ENABLE );
	
	g_CObjs = new CObjMap;
	g_ActionMap = new ActionMap;
	
	registercvars();
	
	LOG_FUNCTION_CHANGE_ARG( "LOAD GAME LIB" );
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
	
	LOG_FUNCTION_CHANGE_ARG( "CFG GAME LIB" );
	if( g_Game->OnConfigure( argc, argv ) == false )
		return 51;
	
	LOG_FUNCTION_CHANGE_ARG( "" );
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	
	if( init_graphics() )
		return 56;
	
	LOG_FUNCTION_CHANGE_ARG( "INIT GAME" );
	if( g_Game->OnInitialize() == false )
		return 63;
	
	LOG_FUNCTION_CHANGE_ARG( "" );
	uint32_t prevtime = GetTimeMsec();
	SDL_Event event;
	while( g_Running )
	{
		SGRX_INT_UnpreserveResources();
		g_PrevCursorPos = g_CPDeltaMode ?
			V2( GR_GetWidth() / 2, GR_GetHeight() / 2 ) :
			g_CursorPos;
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
		if( g_CPDeltaMode )
			Game_PostSetCursorPos( GR_GetWidth() / 2, GR_GetHeight() / 2 );
		
		uint32_t curtime = GetTimeMsec();
		uint32_t dt = curtime - prevtime;
		// do not allow delta time longer than 100 ms
		if( dt > 100 )
			dt = prevtime - curtime < 100 ? 0 : 100;
		prevtime = curtime;
		g_GameTime += dt;
		float fdt = dt / 1000.0f;
		
		Game_Process( fdt );
	}
	
	LOG_FUNCTION_CHANGE_ARG( "DESTROY GAME" );
	g_Game->OnDestroy();
	g_Game = NULL;
	
	LOG_FUNCTION_CHANGE_ARG( "" );
	delete g_Joysticks;
	g_Joysticks = NULL;
	
	free_graphics();
	
	if( VERBOSE ) LOG << LOG_DATE << "  Unloading game library";
	
	SDL_UnloadObject( g_GameLib );
	
	if( VERBOSE ) LOG << LOG_DATE << "  Freeing some internal engine data";
	
	delete g_ActionMap;
	delete g_CObjs;
	
	size_t byid = 0;
	for( size_t i = 0; i < g_EventLinksByID.size(); ++i )
		byid += g_EventLinksByID.item( i ).value->size();
	if( byid != 0 || g_EventLinksByHandler.size() != 0 )
	{
		LOG_WARNING << "\n!!! DETECTED UNRELEASED EVENT HANDLERS !!!";
		LOG << "- # by id: " << byid;
		LOG << "- # by handler: " << g_EventLinksByHandler.size();
		LOG << "calling all handlers...\n";
		for( size_t i = 0; i < g_EventLinksByHandler.size(); ++i )
		{
			LOG << "- " << ( i + 1 ) << "/" << g_EventLinksByHandler.size() << "...";
			g_EventLinksByHandler.item( i ).key->HandleEvent( EID__Unfreed, EventData() );
		}
	}
	
	if( VERBOSE ) LOG << LOG_DATE << "  Engine finished";
	return 0;
}

