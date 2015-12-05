

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
#include "enganim.hpp"
#include "renderer.hpp"
#include "sound.hpp"


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
typedef HashTable< StringView, SGRX_ConvexPointSet* > ConvexPointSetHashTable;
typedef HashTable< StringView, SGRX_ITexture* > TextureHashTable;
typedef HashTable< uint64_t, SGRX_ITexture* > RenderTargetTable;
typedef HashTable< uint64_t, SGRX_IDepthStencilSurface* > DepthStencilSurfTable;
typedef HashTable< StringView, SGRX_IVertexShader* > VertexShaderHashTable;
typedef HashTable< StringView, SGRX_IPixelShader* > PixelShaderHashTable;
typedef HashTable< SGRX_RenderState, SGRX_IRenderState* > RenderStateHashTable;
typedef HashTable< StringView, SGRX_IVertexDecl* > VertexDeclHashTable;
typedef HashTable< SGRX_VtxInputMapKey, SGRX_IVertexInputMapping* > VtxInputMapHashTable;
typedef HashTable< StringView, SGRX_IMesh* > MeshHashTable;
typedef HashTable< StringView, AnimHandle > AnimHashTable;
typedef HashTable< StringView, FontHandle > FontHashTable;
typedef HashTable< int, JoystickHandle > JoystickHashTable;
typedef HashTable< GenericHandle, int > ResourcePreserveHashTable;

static String g_GameLibName = "game";
static String g_GameDir = ".";
static String g_GameDir2 = "";
static String g_RendererName = "d3d11";

static bool g_Running = true;
static bool g_hasFocus = true;
static bool g_windowVisible = true;
static SDL_Window* g_Window = NULL;
static void* g_GameLib = NULL;
static IGame* g_Game = NULL;
static uint32_t g_GameTime = 0;
static ActionMap* g_ActionMap;
static Vec2 g_CursorPos = {0,0};
static Vec2 g_CursorScale = {0,0};
static EventLinksByID g_EventLinksByID;
static EventLinksByHandler g_EventLinksByHandler;
static Array< IScreen* > g_OverlayScreens;
static Array< FileSysHandle > g_FileSystems;

static RenderSettings g_RenderSettings = { 0, 1280, 720, 60, FULLSCREEN_NONE, false, ANTIALIAS_NONE, 0 };
static const char* g_RendererPrefix = "sgrx-render-";
static void* g_RenderLib = NULL;
static pfnRndInitialize g_RfnInitialize = NULL;
static pfnRndFree g_RfnFree = NULL;
static pfnRndCreateRenderer g_RfnCreateRenderer = NULL;
static IRenderer* g_Renderer = NULL;
static BatchRenderer* g_BatchRenderer = NULL;
static FontRenderer* g_FontRenderer = NULL;
static ConvexPointSetHashTable* g_CPSets = NULL;
static TextureHashTable* g_Textures = NULL;
static RenderTargetTable* g_RenderTargets = NULL;
static DepthStencilSurfTable* g_DepthStencilSurfs = NULL;
static VertexShaderHashTable* g_VertexShaders = NULL;
static PixelShaderHashTable* g_PixelShaders = NULL;
static RenderStateHashTable* g_RenderStates = NULL;
static VertexDeclHashTable* g_VertexDecls = NULL;
static VtxInputMapHashTable* g_VtxInputMaps = NULL;
static MeshHashTable* g_Meshes = NULL;
static AnimHashTable* g_Anims = NULL;
static FontHashTable* g_LoadedFonts = NULL;
static JoystickHashTable* g_Joysticks = NULL;
static ResourcePreserveHashTable* g_PreservedResources = NULL;
static SGRX_FontSettings g_CurFontSettings = { "", 0, 0.0f, -1.2f };




struct FakeSoundEventInstance : SGRX_ISoundEventInstance
{
	FakeSoundEventInstance( bool oneshot ) :
		m_paused(false), m_volume(1), m_pitch(1)
	{
		isOneShot = oneshot;
		isReal = false;
	}
	void Start(){ m_paused = false; }
	void Stop( bool immediate = false ){}
	bool GetPaused(){ return m_paused; }
	void SetPaused( bool paused ){ m_paused = paused; }
	float GetVolume(){ return m_volume; }
	void SetVolume( float v ){ m_volume = v; }
	float GetPitch(){ return m_pitch; }
	void SetPitch( float v ){ m_pitch = v; }
	bool SetParameter( const StringView& name, float value ){ return false; }
	void Set3DAttribs( const SGRX_Sound3DAttribs& attribs ){}
	
	bool m_paused;
	float m_volume;
	float m_pitch;
};

SoundEventInstanceHandle SGRX_ISoundSystem::CreateEventInstance( const StringView& name )
{
	SoundEventInstanceHandle seih = CreateEventInstanceRaw( name );
	if( seih != NULL )
		return seih;
	return new FakeSoundEventInstance( true );
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

inline float ctrldeadzone( float x, float deadzone )
{
	x = clamp( x / 32767.0f, -1, 1 );
	float sgn = sign( x );
	float dst = fabsf( x );
	if( dst < deadzone )
		dst = 0;
	return sgn * dst;
}

void Game_OnEvent( const Event& e )
{
	LOG_FUNCTION;
	if( e.type == SDL_WINDOWEVENT )
	{
		switch( e.window.event )
		{
		case SDL_WINDOWEVENT_FOCUS_GAINED: g_hasFocus = true; break;
		case SDL_WINDOWEVENT_FOCUS_LOST: g_hasFocus = false; break;
		case SDL_WINDOWEVENT_MINIMIZED: g_windowVisible = false; break;
		case SDL_WINDOWEVENT_RESTORED: g_windowVisible = true; break;
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
	if( !g_windowVisible || !g_hasFocus )
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


SGRX_ConvexPointSet::~SGRX_ConvexPointSet()
{
	LOG << "Deleted convex point set: " << m_key;
	g_CPSets->unset( m_key );
}

SGRX_ConvexPointSet* SGRX_ConvexPointSet::Create( const StringView& path )
{
	ByteArray data;
	if( FS_LoadBinaryFile( path, data ) == false )
	{
		LOG_ERROR << LOG_DATE << "  Convex point set file not found: " << path;
		return NULL;
	}
	SGRX_ConvexPointSet* cps = new SGRX_ConvexPointSet;
	ByteReader br( &data );
	cps->Serialize( br );
	if( cps->data.points.size() == 0 )
	{
		LOG_ERROR << LOG_DATE << "  Convex point set invalid or empty: " << path;
		delete cps;
	}
	return cps;
}

ConvexPointSetHandle GP_GetConvexPointSet( const StringView& path )
{
	LOG_FUNCTION;
	
	SGRX_ConvexPointSet* cps = g_CPSets->getcopy( path );
	if( cps )
		return cps;
	
	cps = SGRX_ConvexPointSet::Create( path );
	if( cps == NULL )
	{
		// error already printed
		return ConvexPointSetHandle();
	}
	cps->m_key = path;
	g_CPSets->set( cps->m_key, cps );
	return cps;
}


void RenderStats::Reset()
{
	numVisMeshes = 0;
	numVisPLights = 0;
	numVisSLights = 0;
	numDrawCalls = 0;
	numSDrawCalls = 0;
	numMDrawCalls = 0;
	numPDrawCalls = 0;
}


SGRX_ITexture::SGRX_ITexture() : m_rtkey(0)
{
}

SGRX_ITexture::~SGRX_ITexture()
{
	LOG << "Deleted texture: " << m_key;
	g_Textures->unset( m_key );
	if( m_rtkey )
		g_RenderTargets->unset( m_rtkey );
}

const TextureInfo& TextureHandle::GetInfo() const
{
	static TextureInfo dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}

Vec2 TextureHandle::GetInvSize( Vec2 def ) const
{
	const TextureInfo& TI = GetInfo();
	return V2( TI.width ? 1.0f / TI.width : def.x, TI.height ? 1.0f / TI.height : def.y );
}

bool TextureHandle::UploadRGBA8Part( void* data, int mip, int w, int h, int x, int y )
{
	LOG_FUNCTION;
	
	if( !item )
		return false;
	
	const TextureInfo& TI = item->m_info;
	
	if( mip < 0 || mip >= TI.mipcount )
	{
		LOG_ERROR << "Cannot UploadRGBA8Part - mip count out of bounds (" << mip << "/" << TI.mipcount << ")";
		return false;
	}
	
	TextureInfo mti;
	if( !TextureInfo_GetMipInfo( &TI, mip, &mti ) )
	{
		LOG_ERROR << "Cannot UploadRGBA8Part - failed to get mip info (" << mip << ")";
		return false;
	}
	
	if( w < 0 ) w = mti.width;
	if( h < 0 ) h = mti.height;
	
	return item->UploadRGBA8Part( data, mip, x, y, w, h );
}


SGRX_IDepthStencilSurface::SGRX_IDepthStencilSurface() : m_width(0), m_height(0), m_format(0), m_key(0)
{
}

SGRX_IDepthStencilSurface::~SGRX_IDepthStencilSurface()
{
	LOG << "Deleted depth/stencil surface: " << m_width << "x" << m_height << ", format=" << m_format;
	if( m_key )
		g_DepthStencilSurfs->unset( m_key );
}


SGRX_IVertexShader::~SGRX_IVertexShader()
{
	g_VertexShaders->unset( m_key );
}

SGRX_IPixelShader::~SGRX_IPixelShader()
{
	g_PixelShaders->unset( m_key );
}


SGRX_IRenderState::~SGRX_IRenderState()
{
	g_RenderStates->unset( m_info );
}

void SGRX_IRenderState::SetState( const SGRX_RenderState& state )
{
	m_info = state;
}

const SGRX_RenderState& RenderStateHandle::GetInfo()
{
	static SGRX_RenderState dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}


SGRX_IVertexDecl::~SGRX_IVertexDecl()
{
	g_VertexDecls->unset( m_key );
}

const VDeclInfo& VertexDeclHandle::GetInfo()
{
	static VDeclInfo dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}


bool VD_ExtractFloat1( const VDeclInfo& vdinfo, int vcount, const void* verts, float* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
	case VDECLTYPE_FLOAT2:
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(float*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *((uint8_t*)verts+stride*i+ofs)/255.0f;
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat2( const VDeclInfo& vdinfo, int vcount, const void* verts, Vec2* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V2( *(float*)((uint8_t*)verts+stride*i+ofs), 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec2*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V2(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f );
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat3( const VDeclInfo& vdinfo, int vcount, const void* verts, Vec3* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3( *(float*)((uint8_t*)verts+stride*i+ofs), 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4), 0 );
		}
		break;
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec3*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+2)/255.0f );
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat4( const VDeclInfo& vdinfo, int vcount, const void* verts, Vec4* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4( *(float*)((uint8_t*)verts+stride*i+ofs), 0, 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4), 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT3:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4),
				*(float*)((uint8_t*)verts+stride*i+ofs+8), 0 );
		}
		break;
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec4*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+2)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+3)/255.0f );
		}
		break;
	}
	return true;
}

bool VD_ExtractByte4Clamped( const VDeclInfo& vdinfo, int vcount, const void* verts, uint32_t* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4( *(float*)((uint8_t*)verts+stride*i+ofs), 0, 0, 0 ) );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4), 0, 0 ) );
		}
		break;
	case VDECLTYPE_FLOAT3:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4),
				*(float*)((uint8_t*)verts+stride*i+ofs+8), 0 ) );
		}
		break;
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( *(Vec4*)((uint8_t*)verts+stride*i+ofs) );
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+2)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+3)/255.0f ) );
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat3P( const VDeclInfo& vdinfo, int vcount, const void** vertptrs, Vec3* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage );
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3( *(float*)((uint8_t*)vertptrs[i]+ofs), 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*(float*)((uint8_t*)vertptrs[i]+ofs),
				*(float*)((uint8_t*)vertptrs[i]+ofs+4), 0 );
		}
		break;
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec3*)((uint8_t*)vertptrs[i]+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*((uint8_t*)vertptrs[i]+ofs)/255.0f,
				*((uint8_t*)vertptrs[i]+ofs+1)/255.0f,
				*((uint8_t*)vertptrs[i]+ofs+2)/255.0f );
		}
		break;
	}
	return true;
}

void VD_LerpTri( const VDeclInfo& vdinfo, int vcount, void* outbuf, Vec3* factors, const void* v1, const void* v2, const void* v3 )
{
	SGRX_CAST( char*, coutbuf, outbuf );
	SGRX_CAST( const char*, cv1, v1 );
	SGRX_CAST( const char*, cv2, v2 );
	SGRX_CAST( const char*, cv3, v3 );
	for( int i = 0; i < vdinfo.count; ++i )
	{
		int off = vdinfo.offsets[ i ];
		char* ocoutbuf = coutbuf + off;
		const char *ocv1 = cv1 + off, *ocv2 = cv2 + off, *ocv3 = cv3 + off;
		
		switch( vdinfo.types[ i ] )
		{
		case VDECLTYPE_FLOAT1:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(float*)( ocoutbuf + v * vdinfo.size ) = *(float*)ocv1 * f.x + *(float*)ocv2 * f.y + *(float*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_FLOAT2:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(Vec2*)( ocoutbuf + v * vdinfo.size ) = *(Vec2*)ocv1 * f.x + *(Vec2*)ocv2 * f.y + *(Vec2*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_FLOAT3:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(Vec3*)( ocoutbuf + v * vdinfo.size ) = *(Vec3*)ocv1 * f.x + *(Vec3*)ocv2 * f.y + *(Vec3*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_FLOAT4:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(Vec4*)( ocoutbuf + v * vdinfo.size ) = *(Vec4*)ocv1 * f.x + *(Vec4*)ocv2 * f.y + *(Vec4*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_BCOL4:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(BVec4*)( ocoutbuf + v * vdinfo.size ) = *(BVec4*)ocv1 * f.x + *(BVec4*)ocv2 * f.y + *(BVec4*)ocv3 * f.z;
			}
			break;
		}
	}
}



SGRX_IVertexInputMapping::~SGRX_IVertexInputMapping()
{
	g_VtxInputMaps->unset( m_key );
}



SGRX_IMesh::SGRX_IMesh() :
	m_dataFlags( 0 ),
	m_vertexDataSize( 0 ),
	m_indexDataSize( 0 ),
	m_numBones( 0 ),
	m_boundsMin( Vec3::Create( 0 ) ),
	m_boundsMax( Vec3::Create( 0 ) )
{
}

SGRX_IMesh::~SGRX_IMesh()
{
	g_Meshes->unset( m_key );
}

bool SGRX_IMesh::ToMeshData( ByteArray& out )
{
	if( m_meshParts.size() > MAX_MESH_FILE_PARTS )
		return false;
	ByteWriter bw( &out );
	bw.marker( "SS3DMESH" );
	bw.write( uint32_t(m_dataFlags & MDF__PUBFLAGMASK) );
	bw.write( m_boundsMin );
	bw.write( m_boundsMax );
	bw << m_vdata;
	bw << m_idata;
	bw.write( uint8_t(m_vertexDecl->m_key.size()) );
	bw.memory( m_vertexDecl->m_key.data(), m_vertexDecl->m_key.size() );
	bw.write( uint8_t(m_meshParts.size()) );
	for( size_t pid = 0; pid < m_meshParts.size(); ++pid )
	{
		SGRX_MeshPart& MP = m_meshParts[ pid ];
		if( m_dataFlags & MDF_MTLINFO )
		{
			bw.write( uint8_t(MP.mtlFlags) );
			bw.write( uint8_t(MP.mtlBlendMode) );
		}
		bw.write( uint32_t(MP.vertexOffset) );
		bw.write( uint32_t(MP.vertexCount) );
		bw.write( uint32_t(MP.indexOffset) );
		bw.write( uint32_t(MP.indexCount) );
		bw.write( uint8_t(SGRX_MAX_MESH_TEXTURES) ); // materialTextureCount
		bw.write( uint8_t(MP.shader.size()) );
		bw.memory( MP.shader.data(), MP.shader.size() );
		for( int tid = 0; tid < SGRX_MAX_MESH_TEXTURES; ++tid )
		{
			const String& tex = MP.textures[ tid ];
			bw.write( uint8_t(tex.size()) );
			bw.memory( tex.data(), tex.size() );
		}
	}
	if( m_dataFlags & MDF_SKINNED )
	{
		bw.write( uint8_t(m_numBones) );
		for( int bid = 0; bid < m_numBones; ++bid )
		{
			const SGRX_MeshBone& B = m_bones[ bid ];
			bw.write( uint8_t(B.name.size()) );
			bw.memory( B.name.data(), B.name.size() );
			bw.write( uint8_t(B.parent_id) );
			bw.write( B.boneOffset );
		}
	}
	return true;
}

bool SGRX_IMesh::SetPartData( SGRX_MeshPart* parts, int count )
{
	if( count < 0 )
		return false;
	m_meshParts.assign( parts, count );
	return true;
}

bool SGRX_IMesh::SetBoneData( SGRX_MeshBone* bones, int count )
{
	if( count < 0 || count > SGRX_MAX_MESH_BONES )
		return false;
	int i;
	for( i = 0; i < count; ++i )
		m_bones[ i ] = bones[ i ];
	for( ; i < count; ++i )
		m_bones[ i ] = SGRX_MeshBone();
	m_numBones = count;
	return RecalcBoneMatrices();
}

bool SGRX_IMesh::RecalcBoneMatrices()
{
	if( !m_numBones )
	{
		return true;
	}
	
	for( int b = 0; b < m_numBones; ++b )
	{
		if( m_bones[ b ].parent_id < -1 || m_bones[ b ].parent_id >= b )
		{
			LOG_WARNING << "RecalcBoneMatrices: each parent_id must point to a previous bone or no bone (-1) [error in bone "
				<< b << ": " << m_bones[ b ].parent_id << "]";
			return false;
		}
	}
	
	Mat4 skinOffsets[ SGRX_MAX_MESH_BONES ];
	for( int b = 0; b < m_numBones; ++b )
	{
		if( m_bones[ b ].parent_id >= 0 )
			skinOffsets[ b ].Multiply( m_bones[ b ].boneOffset, skinOffsets[ m_bones[ b ].parent_id ] );
		else
			skinOffsets[ b ] = m_bones[ b ].boneOffset;
		m_bones[ b ].skinOffset = skinOffsets[ b ];
	}
	for( int b = 0; b < m_numBones; ++b )
	{
		if( !skinOffsets[ b ].InvertTo( m_bones[ b ].invSkinOffset ) )
		{
			LOG_WARNING << "RecalcBoneMatrices: failed to invert skin offset matrix #" << b;
			m_bones[ b ].invSkinOffset.SetIdentity();
		}
	}
	return true;
}

bool SGRX_IMesh::SetAABBFromVertexData( const void* data, size_t size, VertexDeclHandle vd )
{
	return GetAABBFromVertexData( vd.GetInfo(), (const char*) data, size, m_boundsMin, m_boundsMax );
}

void SGRX_IMesh_RaycastAll_Core_TestTriangle( const Vec3& rpos, const Vec3& rdir, float rlen,
	SceneRaycastCallback* cb, SceneRaycastInfo* srci, VDeclInfo* vdinfo, const void* v1, const void* v2, const void* v3 )
{
	const void* verts[3] = { v1, v3, v2 }; // order swapped for RayPolyIntersect
	Vec3 pos[3] = {0};
	VD_ExtractFloat3P( *vdinfo, 3, verts, pos );
	
	float dist[1];
	if( RayPolyIntersect( rpos, rdir, pos, 3, dist ) && dist[0] >= 0 && dist[0] < rlen )
	{
		srci->factor = dist[0] / rlen;
		srci->normal = Vec3Cross( pos[1] - pos[0], pos[2] - pos[0] ).Normalized();
		if( srci->meshinst )
			srci->normal = srci->meshinst->matrix.TransformNormal( srci->normal );
		
		// TODO u/v
		cb->AddResult( srci );
	}
}

template< class IdxType > void SGRX_IMesh_RaycastAll_Core( SGRX_IMesh* mesh, const Vec3& from, const Vec3& to,
	SceneRaycastCallback* cb, SceneRaycastInfo* srci, size_t fp, size_t ep )
{
	size_t stride = mesh->m_vertexDecl.GetInfo().size;
	SGRX_CAST( IdxType*, indices, mesh->m_idata.data() );
	
	Vec3 dtdir = to - from;
	float rlen = dtdir.Length();
	dtdir /= rlen;
	
	if( ( mesh->m_dataFlags & MDF_TRIANGLESTRIP ) == 0 )
	{
		for( size_t part_id = fp; part_id < ep; ++part_id )
		{
			srci->partID = part_id;
			SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
			if( MP.mtlBlendMode != SGRX_MtlBlend_None &&
				MP.mtlBlendMode != SGRX_MtlBlend_Basic )
				continue;
			
			for( uint32_t tri = MP.indexOffset, triend = MP.indexOffset + MP.indexCount; tri < triend; tri += 3 )
			{
				srci->triID = tri / 3;
				SGRX_IMesh_RaycastAll_Core_TestTriangle( from, dtdir, rlen, cb, srci, &mesh->m_vertexDecl->m_info
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
				);
			}
		}
	}
	else
	{
		for( size_t part_id = fp; part_id < ep; ++part_id )
		{
			srci->partID = part_id;
			SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
			if( MP.mtlBlendMode != SGRX_MtlBlend_None &&
				MP.mtlBlendMode != SGRX_MtlBlend_Basic )
				continue;
			
			for( uint32_t tri = MP.indexOffset + 2, triend = MP.indexOffset + MP.indexCount; tri < triend; ++tri )
			{
				srci->triID = tri - 2;
				uint32_t i1 = tri, i2 = tri + 1 + tri % 2, i3 = tri + 2 - tri % 2;
				SGRX_IMesh_RaycastAll_Core_TestTriangle( from, dtdir, rlen, cb, srci, &mesh->m_vertexDecl->m_info
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ i1 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ i2 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ i3 ] ) * stride ]
				);
			}
		}
	}
}

void SGRX_IMesh::RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, SGRX_MeshInstance* cbmi )
{
	if( !m_vdata.size() || !m_idata.size() || !m_meshParts.size() )
		return;
	
	SceneRaycastInfo srci = { 0, V3(0), 0, 0, -1, -1, -1, cbmi };
	if( ( m_dataFlags & MDF_INDEX_32 ) != 0 )
	{
		SGRX_IMesh_RaycastAll_Core< uint32_t >( this, from, to, cb, &srci, 0, m_meshParts.size() );
	}
	else
	{
		SGRX_IMesh_RaycastAll_Core< uint16_t >( this, from, to, cb, &srci, 0, m_meshParts.size() );
	}
}

void SGRX_IMesh::MRC_DebugDraw( SGRX_MeshInstance* mi )
{
	// TODO
}

template< typename T, typename T2 > void sa2_insert( T* arr, T2* arr2, int& count, int& at, const T& val, const T2& val2 )
{
	for( int i = count; i > at; )
	{
		i--;
		arr[ i + 1 ] = arr[ i ];
		arr2[ i + 1 ] = arr2[ i ];
	}
	arr2[ at ] = val2;
	arr[ at++ ] = val;
	count++;
}
template< typename T, typename T2 > void sa2_remove( T* arr, T2* arr2, int& count, int& at )
{
	count--;
	for( int i = at; i < count; ++i )
	{
		arr[ i ] = arr[ i + 1 ];
		arr2[ i ] = arr2[ i + 1 ];
	}
	at--;
}

void SGRX_IMesh_Clip_Core_ClipTriangle( const Mat4& mtx,
	                                    const Mat4& vpmtx,
	                                    ByteArray& outverts,
	                                    SGRX_IVertexDecl* vdecl,
	                                    bool decal,
	                                    float inv_zn2zf,
	                                    uint32_t color,
	                                    const void* v1,
	                                    const void* v2,
	                                    const void* v3 )
{
	const void* verts[3] = { v1, v2, v3 };
	Vec3 pos[3] = {0};
	VD_ExtractFloat3P( vdecl->m_info, 3, verts, pos );
	
	pos[0] = mtx.TransformPos( pos[0] );
	pos[1] = mtx.TransformPos( pos[1] );
	pos[2] = mtx.TransformPos( pos[2] );
	
	Vec4 tpos[3] =
	{
		vpmtx.Transform( V4( pos[0], 1.0f ) ),
		vpmtx.Transform( V4( pos[1], 1.0f ) ),
		vpmtx.Transform( V4( pos[2], 1.0f ) ),
	};
	Vec4 pts[9] =
	{
		tpos[0], tpos[1], tpos[2],
		V4(0), V4(0), V4(0),
		V4(0), V4(0), V4(0)
	};
	Vec3 fcs[9] =
	{
		V3(1,0,0), V3(0,1,0), V3(0,0,1),
		V3(0), V3(0), V3(0),
		V3(0), V3(0), V3(0)
	};
	int pcount = 3;
	
#define IMCCCT_CLIP_Pred( initsd, loopsd )           \
	{                                                \
		Vec4 prevpt = pts[ pcount - 1 ];             \
		Vec3 prevfc = fcs[ pcount - 1 ];             \
		float prevsd = /* = 1 */ initsd /* */;       \
		for( int i = 0; i < pcount; ++i )            \
		{                                            \
			Vec4 currpt = pts[ i ];                  \
			Vec3 currfc = fcs[ i ];                  \
			float currsd = /* = 2 */ loopsd /* */;   \
			if( prevsd * currsd < 0 )                \
			{                                        \
				/* insert intersection point */      \
				float f = safe_fdiv( -prevsd,        \
					( currsd - prevsd ) );           \
				sa2_insert( pts, fcs, pcount, i,     \
					TLERP( prevpt, currpt, f ),      \
					TLERP( prevfc, currfc, f ) );    \
			}                                        \
			if( currsd >= 0 )                        \
			{                                        \
				sa2_remove( pts, fcs, pcount, i );   \
			}                                        \
			prevpt = currpt;                         \
			prevfc = currfc;                         \
			prevsd = currsd;                         \
		}                                            \
	}
	IMCCCT_CLIP_Pred( SMALL_FLOAT - prevpt.w, SMALL_FLOAT - currpt.w ); // clip W <= 0
	IMCCCT_CLIP_Pred( prevpt.x - prevpt.w, currpt.x - currpt.w ); // clip X > W
	IMCCCT_CLIP_Pred( -prevpt.x - prevpt.w, -currpt.x - currpt.w ); // clip X < -W
	IMCCCT_CLIP_Pred( prevpt.y - prevpt.w, currpt.y - currpt.w ); // clip Y > W
	IMCCCT_CLIP_Pred( -prevpt.y - prevpt.w, -currpt.y - currpt.w ); // clip Y < -W
	IMCCCT_CLIP_Pred( prevpt.z - prevpt.w, currpt.z - currpt.w ); // clip Z > W
	IMCCCT_CLIP_Pred( -prevpt.z - prevpt.w, -currpt.z - currpt.w ); // clip Z < -W
	// LOG << "VCOUNT: " << pcount;
	// for(int i = 0; i < pcount;++i) LOG << pts[i] << fcs[i];
	if( pcount < 3 )
		return;
	
	const VDeclInfo* VDI = &vdecl->m_info;
	// convert vertices, fill in missing data
	uint8_t decalvertbuf[ 48 * 3 ];
	if( decal )
	{
		static const VDeclInfo decalvdi =
		{
			{ 0, 12, 24, 36, 0,0,0,0,0,0 },
			{ VDECLTYPE_FLOAT3, VDECLTYPE_FLOAT3, VDECLTYPE_FLOAT3, VDECLTYPE_FLOAT3, 0,0,0,0,0,0 },
			{ VDECLUSAGE_POSITION, VDECLUSAGE_NORMAL, VDECLUSAGE_TEXTURE0, VDECLUSAGE_TANGENT, 0,0,0,0,0,0 },
			4, 48
		};
		VDI = &decalvdi;
		
		Vec3 nrm[3] = {0};
		VD_ExtractFloat3P( vdecl->m_info, 3, verts, nrm, VDECLUSAGE_NORMAL );
		
		nrm[0] = mtx.TransformNormal( nrm[0] );
		nrm[1] = mtx.TransformNormal( nrm[1] );
		nrm[2] = mtx.TransformNormal( nrm[2] );
		
		SGRX_CAST( SGRX_Vertex_Decal*, dvs, decalvertbuf );
		for( int i = 0; i < 3; ++i )
		{
			dvs[i].position = pos[ i ];
			dvs[i].normal = nrm[ i ];
			dvs[i].texcoord = V3(0);
			dvs[i].tangent = 0x007f7f7f;
			dvs[i].color = color;
			dvs[i].padding0 = 0;
		}
		
		v1 = decalvertbuf+0;
		v2 = decalvertbuf+48;
		v3 = decalvertbuf+48*2;
	}
	
	// interpolate vertices
	uint8_t vbuf[ 256 * 9 ];
	memset( vbuf, 0, sizeof(vbuf) );
	int stride = VDI->size;
	VD_LerpTri( *VDI, pcount, vbuf, fcs, v1, v2, v3 );
	if( decal )
	{
		SGRX_CAST( SGRX_Vertex_Decal*, dvs, vbuf );
		for( int i = 0; i < pcount; ++i )
		{
			Vec4 vtp = vpmtx.Transform( V4( *(Vec3*)(vbuf + i * 48), 1.0f ) );
			dvs[ i ].texcoord = V3
			(
				safe_fdiv( vtp.x, vtp.w ) * 0.5f + 0.5f,
				safe_fdiv( vtp.y, vtp.w ) * 0.5f + 0.5f,
				vtp.z * inv_zn2zf
			);
		}
	}
	for( int i = 1; i < pcount - 1; ++i )
	{
		outverts.append( vbuf, stride );
		outverts.append( vbuf + i * stride, stride * 2 );
	}
}

template< class IdxType >
void SGRX_IMesh_Clip_Core( SGRX_IMesh* mesh,
	                       const Mat4& mtx,
	                       const Mat4& vpmtx,
	                       bool decal,
	                       float inv_zn2zf,
	                       uint32_t color,
	                       ByteArray& outverts,
	                       size_t fp,
	                       size_t ep )
{
	size_t stride = mesh->m_vertexDecl.GetInfo().size;
	SGRX_CAST( IdxType*, indices, mesh->m_idata.data() );
	if( ( mesh->m_dataFlags & MDF_TRIANGLESTRIP ) == 0 )
	{
		for( size_t part_id = fp; part_id < ep; ++part_id )
		{
			SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
			if( MP.mtlBlendMode != SGRX_MtlBlend_None &&
				MP.mtlBlendMode != SGRX_MtlBlend_Basic )
				continue;
			for( uint32_t tri = MP.indexOffset, triend = MP.indexOffset + MP.indexCount; tri < triend; tri += 3 )
			{
				SGRX_IMesh_Clip_Core_ClipTriangle( mtx, vpmtx, outverts, mesh->m_vertexDecl, decal, inv_zn2zf, color
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
				);
			}
		}
	}
	else
	{
		for( size_t part_id = fp; part_id < ep; ++part_id )
		{
			SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
			if( MP.mtlBlendMode != SGRX_MtlBlend_None &&
				MP.mtlBlendMode != SGRX_MtlBlend_Basic )
				continue;
			for( uint32_t tri = MP.indexOffset + 2, triend = MP.indexOffset + MP.indexCount; tri < triend; ++tri )
			{
				uint32_t i1 = tri, i2 = tri + 1 + tri % 2, i3 = tri + 2 - tri % 2;
				SGRX_IMesh_Clip_Core_ClipTriangle( mtx, vpmtx, outverts, mesh->m_vertexDecl, decal, inv_zn2zf, color
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ i1 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ i2 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ i3 ] ) * stride ]
				);
			}
		}
	}
}

void SGRX_IMesh::Clip( const Mat4& mtx,
	                   const Mat4& vpmtx,
	                   ByteArray& outverts,
	                   bool decal,
	                   float inv_zn2zf,
	                   uint32_t color,
	                   size_t firstPart,
	                   size_t numParts )
{
	if( m_vdata.size() == 0 || m_idata.size() == 0 )
		return;
	
	size_t MPC = m_meshParts.size();
	firstPart = TMIN( firstPart, MPC - 1 );
	size_t oneOverLastPart = TMIN( firstPart + TMIN( numParts, MPC ), MPC );
	if( ( m_dataFlags & MDF_INDEX_32 ) != 0 )
	{
		SGRX_IMesh_Clip_Core< uint32_t >( this, mtx, vpmtx, decal, inv_zn2zf, color, outverts, firstPart, oneOverLastPart );
	}
	else
	{
		SGRX_IMesh_Clip_Core< uint16_t >( this, mtx, vpmtx, decal, inv_zn2zf, color, outverts, firstPart, oneOverLastPart );
	}
}


bool SGRX_IMesh::IsBoneUnder( int bone, int parent )
{
	while( bone != parent && bone != -1 )
	{
		bone = m_bones[ bone ].parent_id;
	}
	return bone == parent;
}


SGRX_Log& operator << ( SGRX_Log& L, const SGRX_Camera& cam )
{
	L << "CAMERA:";
	L << "\n    position = " << cam.position;
	L << "\n    direction = " << cam.direction;
	L << "\n    updir = " << cam.updir;
	L << "\n    angle = " << cam.angle;
	L << "\n    aspect = " << cam.aspect;
	L << "\n    aamix = " << cam.aamix;
	L << "\n    znear = " << cam.znear;
	L << "\n    zfar = " << cam.zfar;
	L << "\n    mView = " << cam.mView;
	L << "\n    mProj = " << cam.mProj;
	L << "\n    mInvView = " << cam.mInvView;
	return L;
}

void SGRX_Camera::UpdateViewMatrix()
{
	mView.LookAt( position, direction, updir );
	mView.InvertTo( mInvView );
}

void SGRX_Camera::UpdateProjMatrix()
{
	mProj.Perspective( angle, aspect, aamix, znear, zfar );
}

void SGRX_Camera::UpdateMatrices()
{
	UpdateViewMatrix();
	UpdateProjMatrix();
}

Vec3 SGRX_Camera::WorldToScreen( const Vec3& pos, bool* infront )
{
	Vec3 P = mView.TransformPos( pos );
	Vec4 psP = mProj.Transform( V4( P, 1 ) );
	P = psP.ToVec3() * ( 1.0f / psP.w );
	P.x = P.x * 0.5f + 0.5f;
	P.y = P.y * -0.5f + 0.5f;
	if( infront )
		*infront = psP.w > 0;
	return P;
}

bool SGRX_Camera::GetCursorRay( float x, float y, Vec3& pos, Vec3& dir ) const
{
	Vec3 tPos = { x * 2 - 1, y * -2 + 1, 0 };
	Vec3 tTgt = { x * 2 - 1, y * -2 + 1, 1 };
	
	Mat4 viewProjMatrix, inv;
	viewProjMatrix.Multiply( mView, mProj );
	if( !viewProjMatrix.InvertTo( inv ) )
		return false;
	
	tPos = inv.TransformPos( tPos );
	tTgt = inv.TransformPos( tTgt );
	Vec3 tDir = ( tTgt - tPos ).Normalized();
	
	pos = tPos;
	dir = tDir;
	return true;
}


SGRX_Light::SGRX_Light( SGRX_Scene* s ) :
	_scene( s ),
	type( LIGHT_POINT ),
	enabled( true ),
	position( Vec3::Create( 0 ) ),
	direction( Vec3::Create( 0, 1, 0 ) ),
	updir( Vec3::Create( 0, 0, 1 ) ),
	color( Vec3::Create( 1 ) ),
	range( 100 ),
	power( 2 ),
	angle( 60 ),
	aspect( 1 ),
	hasShadows( false ),
	layers( 0x1 ),
	matrix( Mat4::Identity ),
	_tf_position( Vec3::Create( 0 ) ),
	_tf_direction( Vec3::Create( 0, 1, 0 ) ),
	_tf_updir( Vec3::Create( 0, 0, 1 ) ),
	_tf_range( 100 ),
	_dibuf_begin( NULL ),
	_dibuf_end( NULL )
{
	projectionMaterial.shader = "proj_default";
	projectionMaterial.flags = SGRX_MtlFlag_Unlit | SGRX_MtlFlag_Decal;
	projectionMaterial.blendMode = SGRX_MtlBlend_Basic;
	UpdateTransform();
}

SGRX_Light::~SGRX_Light()
{
	if( _scene )
	{
		_scene->m_lights.unset( this );
	}
}

void SGRX_Light::UpdateTransform()
{
	_tf_position = matrix.TransformPos( position );
	_tf_direction = matrix.TransformNormal( direction );
	_tf_updir = matrix.TransformNormal( updir );
	_tf_range = matrix.TransformNormal( V3( sqrtf( range * range / 3 ) ) ).Length();
	viewMatrix = Mat4::CreateLookAt( _tf_position, _tf_direction, _tf_updir );
	projMatrix = Mat4::CreatePerspective( angle, aspect, 0.5, _tf_range * 0.001f, _tf_range );
	viewProjMatrix.Multiply( viewMatrix, projMatrix );
}

void SGRX_Light::GenerateCamera( SGRX_Camera& outcam )
{
	outcam.position = _tf_position;
	outcam.direction = _tf_direction;
	outcam.updir = _tf_updir;
	outcam.angle = angle;
	outcam.aspect = aspect;
	outcam.aamix = 0.5f;
	outcam.znear = _tf_range * 0.001f;
	outcam.zfar = _tf_range;
	outcam.UpdateMatrices();
}

void SGRX_Light::SetTransform( const Mat4& mtx )
{
	matrix = mtx;
}

void SGRX_Light::GetVolumePoints( Vec3 pts[8] )
{
	if( type == LIGHT_POINT )
	{
		pts[0] = _tf_position + V3(-_tf_range, -_tf_range, -_tf_range);
		pts[1] = _tf_position + V3(+_tf_range, -_tf_range, -_tf_range);
		pts[2] = _tf_position + V3(-_tf_range, +_tf_range, -_tf_range);
		pts[3] = _tf_position + V3(+_tf_range, +_tf_range, -_tf_range);
		pts[4] = _tf_position + V3(-_tf_range, -_tf_range, +_tf_range);
		pts[5] = _tf_position + V3(+_tf_range, -_tf_range, +_tf_range);
		pts[6] = _tf_position + V3(-_tf_range, +_tf_range, +_tf_range);
		pts[7] = _tf_position + V3(+_tf_range, +_tf_range, +_tf_range);
	}
	else
	{
		Mat4 inv = Mat4::Identity;
		viewProjMatrix.InvertTo( inv );
		Vec3 ipts[8] =
		{
			V3(-1, -1, -1), V3(+1, -1, -1),
			V3(-1, +1, -1), V3(+1, +1, -1),
			V3(-1, -1, +1), V3(+1, -1, +1),
			V3(-1, +1, +1), V3(+1, +1, +1),
		};
		for( int i = 0; i < 8; ++i )
			pts[ i ] = inv.TransformPos( ipts[ i ] );
	}
}


SGRX_CullScene::~SGRX_CullScene()
{
}


SGRX_DrawItem::SGRX_DrawItem() : MI( NULL ), part( 0 ), type( 0 ), _lightbuf_begin( NULL ), _lightbuf_end( NULL )
{
}


SGRX_Material::SGRX_Material() : flags(0), blendMode(SGRX_MtlBlend_None)
{
}


SGRX_MeshInstance::SGRX_MeshInstance( SGRX_Scene* s ) :
	_scene( s ),
	raycastOverride( NULL ),
	userData( NULL ),
	color( Vec4::Create( 1 ) ),
	m_lightingMode( SGRX_LM_Dynamic ),
	layers( 0x1 ),
	enabled( true ),
	allowStaticDecals( false ),
//	dynamic( false ),
	sortidx( 0 ),
	m_invalid( true )
{
	matrix.SetIdentity();
	for( int i = 0; i < MAX_MI_CONSTANTS; ++i )
		constants[ i ] = Vec4::Create( 0 );
}

SGRX_MeshInstance::~SGRX_MeshInstance()
{
	if( _scene )
	{
		_scene->m_meshInstances.unset( this );
	}
}

void SGRX_MeshInstance::SetTransform( const Mat4& mtx )
{
	matrix = mtx;
}

void SGRX_MeshInstance::_Precache()
{
	size_t dicnt = TMIN( materials.size(), m_mesh->m_meshParts.size() );
	if( m_invalid || m_drawItems.size() != dicnt )
	{
		m_drawItems.resize( dicnt );
		for( size_t i = 0; i < dicnt; ++i )
		{
			const SGRX_Material& mtl = materials[ i ];
			uint8_t type = 0;
			if( mtl.flags & SGRX_MtlFlag_Decal )
				type = SGRX_TY_Decal;
			else if( mtl.blendMode != SGRX_MtlBlend_None )
				type = SGRX_TY_Transparent;
			else
				type = SGRX_TY_Solid;
			m_drawItems[ i ].MI = this;
			m_drawItems[ i ].part = i;
			m_drawItems[ i ].type = type;
			m_drawItems[ i ]._lightbuf_begin = NULL;
			m_drawItems[ i ]._lightbuf_end = NULL;
		}
		
		m_srsData.resize( dicnt * _scene->m_passes.size() );
		for( size_t diid = 0; diid < dicnt; ++diid )
		{
			for( size_t pid = 0; pid < _scene->m_passes.size(); ++pid )
			{
				SGRX_SRSData& srs = GetSRSData( pid, diid );
				
				// load render state
				SGRX_RenderState rs;
				rs.Init();
				g_Game->OnMakeRenderState( _scene->m_passes[ pid ], materials[ diid ], rs );
				srs.RS = GR_GetRenderState( rs );
				
				// load shaders
				g_Game->OnLoadMtlShaders( _scene->m_passes[ pid ], _scene->m_defines, materials[ diid ], this, srs.VS, srs.PS );
				
				// create/load vertex input mapping
				srs.VIM = GR_GetVertexInputMapping( srs.VS, m_mesh->m_vertexDecl );
			}
		}
		
		m_invalid = false;
	}
}

void SGRX_MeshInstance::SetMesh( StringView path, bool mtls )
{
	SetMesh( GR_GetMesh( path ), mtls );
}

void SGRX_MeshInstance::SetMesh( MeshHandle mh, bool mtls )
{
	m_mesh = mh;
	if( mtls )
	{
		if( mh )
		{
			materials.resize( mh->m_meshParts.size() );
			for( size_t i = 0; i < mh->m_meshParts.size(); ++i )
			{
				const SGRX_MeshPart& MP = mh->m_meshParts[ i ];
				SGRX_Material& M = materials[ i ];
				
				M.shader = MP.shader;
				M.blendMode = MP.mtlBlendMode;
				M.flags = MP.mtlFlags;
				for( size_t t = 0; t < SGRX_MAX_MESH_TEXTURES; ++t )
				{
					M.textures[ t ] = MP.textures[ t ].size() ? GR_GetTexture( MP.textures[ t ] ) : NULL;
				}
			}
		}
		else
		{
			materials.resize( 0 );
		}
	}
	OnUpdate();
}


uint32_t SGRX_FindOrAddVertex( ByteArray& vertbuf, size_t searchoffset, size_t& writeoffset, const uint8_t* vertex, size_t vertsize )
{
	const size_t idxoffset = 0;
	for( size_t i = searchoffset; i < writeoffset; i += vertsize )
	{
		if( 0 == memcmp( &vertbuf[ i ], vertex, vertsize ) )
			return ( i - idxoffset ) / vertsize;
	}
	uint32_t out = ( writeoffset - idxoffset ) / vertsize;
	memcpy( &vertbuf[ writeoffset ], vertex, vertsize );
	writeoffset += vertsize;
	return out;
}

void SGRX_DoIndexTriangleMeshVertices( UInt32Array& indices, ByteArray& vertices, size_t offset, size_t stride )
{
#if 0
	while( offset < vertices.size() )
	{
		indices.push_back( offset / stride );
		offset += stride;
	}
	return;
#endif
	// <= 1 tri
	if( vertices.size() <= offset + stride * 3 )
		return;
	
	uint8_t trivertdata[ 256 * 3 ];
	size_t end = ( ( vertices.size() - offset ) / (stride*3) ) * stride * 3 + offset;
	size_t writeoffset = offset;
	size_t readoffset = offset;
	while( readoffset < end )
	{
		// extract a triangle
		memcpy( trivertdata, &vertices[ readoffset ], stride * 3 );
		readoffset += stride * 3;
		
		// insert each vertex/index
		uint32_t idcs[3] =
		{
			SGRX_FindOrAddVertex( vertices, offset, writeoffset, trivertdata, stride ),
			SGRX_FindOrAddVertex( vertices, offset, writeoffset, trivertdata + stride, stride ),
			SGRX_FindOrAddVertex( vertices, offset, writeoffset, trivertdata + stride * 2, stride ),
		};
		indices.append( idcs, 3 );
	}
	// remove unused data
	vertices.resize( writeoffset );
}

SGRX_ProjectionMeshProcessor::SGRX_ProjectionMeshProcessor( ByteArray* verts, UInt32Array* indices, const Mat4& mtx, float zn2zf ) :
	outVertices( verts ), outIndices( indices ), viewProjMatrix( mtx ), invZNearToZFar( safe_fdiv( 1.0f, zn2zf ) )
{
}

void SGRX_ProjectionMeshProcessor::Process( void* data )
{
	LOG_FUNCTION;
	
	SGRX_CAST( SGRX_MeshInstance*, MI, data );
	
	SGRX_IMesh* M = MI->GetMesh();
	if( M )
	{
		size_t vertoff = outVertices->size();
		M->Clip( MI->matrix, viewProjMatrix, *outVertices, true, invZNearToZFar );
		SGRX_DoIndexTriangleMeshVertices( *outIndices, *outVertices, vertoff, 48 );
	}
}


SceneRaycastCallback_Any::SceneRaycastCallback_Any() : m_hit(false)
{
}

void SceneRaycastCallback_Any::AddResult( SceneRaycastInfo* )
{
	m_hit = true;
}

SceneRaycastCallback_Closest::SceneRaycastCallback_Closest() : m_hit(false)
{
	SceneRaycastInfo srci = { 1.0f + SMALL_FLOAT, V3(0), 0, 0, -1, -1, -1, NULL };
	m_closest = srci;
}

void SceneRaycastCallback_Closest::AddResult( SceneRaycastInfo* info )
{
	m_hit = true;
	if( info->factor < m_closest.factor )
		m_closest = *info;
}

SceneRaycastCallback_Sorting::SceneRaycastCallback_Sorting( Array< SceneRaycastInfo >* sortarea )
	: m_sortarea( sortarea )
{
	sortarea->clear();
}

void SceneRaycastCallback_Sorting::AddResult( SceneRaycastInfo* info )
{
	if( m_sortarea->size() == 0 )
	{
		m_sortarea->push_back( *info );
		return;
	}
	
	int lowerBound = 0;
	int upperBound = m_sortarea->size() - 1;
	int pos = 0;
	for(;;)
	{
		pos = (upperBound + lowerBound) / 2;
		if( m_sortarea->at(pos).factor == info->factor )
		{
			break;
		}
		else if( m_sortarea->at(pos).factor < info->factor )
		{
			lowerBound = pos + 1;
			if( lowerBound > upperBound )
			{
				pos++;
				break;
			}
		}
		else
		{
			upperBound = pos - 1;
			if( lowerBound > upperBound )
				break;
		}
	}
	
	m_sortarea->insert( pos, *info );
}



static SGRX_RenderPass g_DefaultRenderPasses[] =
{
	// shadow pass
	{ true, false, 0, 0, "sys_lighting" },
	{ false, true, 16, 0, "sys_lighting" },
	{ false, false, 16, 0, "sys_lighting" },
	{ false, false, 0, 2, "sys_lighting" },
};

SGRX_Scene::SGRX_Scene() :
	cullScene( NULL ),
	fogColor( Vec3::Create( 0.5 ) ),
	fogHeightFactor( 0 ),
	fogDensity( 0.01f ),
	fogHeightDensity( 0 ),
	fogStartHeight( 0.01f ),
	fogMinDist( 0 ),
	clearColor( 0x00111111 ),
	ambientLightColor( Vec3::Create( 0.1f ) ),
	dirLightColor( Vec3::Create( 0.8f ) ),
	dirLightDir( Vec3::Create( -1 ).Normalized() )
{
	camera.position = Vec3::Create( 10, 10, 10 );
	camera.direction = -camera.position.Normalized();
	camera.updir = Vec3::Create( 0, 0, 1 );
	camera.angle = 90;
	camera.aspect = 1;
	camera.aamix = 0.5f;
	camera.znear = 1;
	camera.zfar = 1000;
	camera.UpdateMatrices();
	
	m_defines = ":MOD_BLENDCOLOR 0";
	SetRenderPasses( g_DefaultRenderPasses, SGRX_ARRAY_SIZE(g_DefaultRenderPasses) );
	
	m_projMeshInst = CreateMeshInstance();
	m_projMeshInst->sortidx = 255;
}

SGRX_Scene::~SGRX_Scene()
{
	m_projMeshInst = NULL;
	
	LOG << "Deleted scene: " << this;
}

void SGRX_Scene::SetRenderPasses( SGRX_RenderPass* passes, int count )
{
	m_passes.assign( passes, count );
	OnUpdate();
}

void SGRX_Scene::SetDefines( StringView defines )
{
	m_defines = defines;
	OnUpdate();
}

MeshInstHandle SGRX_Scene::CreateMeshInstance()
{
	SGRX_MeshInstance* mi = new SGRX_MeshInstance( this );
	m_meshInstances.set( mi, NULL );
	return mi;
}

LightHandle SGRX_Scene::CreateLight()
{
	SGRX_Light* lt = new SGRX_Light( this );
	m_lights.set( lt, NULL );
	return lt;
}

void SGRX_Scene::OnUpdate()
{
	for( size_t i = 0; i < m_meshInstances.size(); ++i )
		m_meshInstances.item( i ).key->OnUpdate();
}

bool SGRX_Scene::RaycastOne( const Vec3& from, const Vec3& to, SceneRaycastInfo* outinfo, uint32_t layers )
{
	SceneRaycastCallback_Closest cb;
	RaycastAll( from, to, &cb, layers );
	if( cb.m_hit )
		*outinfo = cb.m_closest;
	return cb.m_hit;
}

void SGRX_Scene::RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, uint32_t layers )
{
	// TODO: broadphase
//	double A = sgrx_hqtime();
	Mat4 inv;
	for( size_t i = 0; i < m_meshInstances.size(); ++i )
	{
		SGRX_MeshInstance* mi = m_meshInstances.item( i ).key;
		if( mi->enabled && mi->GetMesh() &&
			( mi->layers & layers ) && mi->matrix.InvertTo( inv ) )
		{
			Vec3 tffrom = inv.TransformPos( from );
			Vec3 tfto = inv.TransformPos( to );
			if( SegmentAABBIntersect( tffrom, tfto, mi->GetMesh()->m_boundsMin, mi->GetMesh()->m_boundsMax ) )
			{
				IMeshRaycast* mrc = mi->raycastOverride ? mi->raycastOverride : mi->GetMesh();
				mrc->RaycastAll( tffrom, tfto, cb, mi );
			}
		}
	}
//	double B = sgrx_hqtime();
//	LOG << "RaycastAll: " << (B-A)*1000 << " ms";
}

void SGRX_Scene::RaycastAllSort( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, uint32_t layers, Array< SceneRaycastInfo >* tmpstore )
{
	Array< SceneRaycastInfo > mystore;
	if( !tmpstore )
		tmpstore = &mystore;
	SceneRaycastCallback_Sorting mycb( tmpstore );
	RaycastAll( from, to, &mycb, layers );
	for( size_t i = 0; i < tmpstore->size(); ++i )
	{
		cb->AddResult( &tmpstore->at(i) );
	}
}

void SGRX_Scene::GatherMeshes( const SGRX_Camera& cam, IProcessor* meshInstProc, uint32_t layers )
{
	// TODO use cullscene
	for( size_t i = 0; i < m_meshInstances.size(); ++i )
	{
		SGRX_MeshInstance* mi = m_meshInstances.item( i ).key;
		if( mi->layers & layers )
			meshInstProc->Process( mi );
	}
}

void SGRX_Scene::GenerateProjectionMesh( const SGRX_Camera& cam, ByteArray& outverts, UInt32Array& outindices, uint32_t layers )
{
	SGRX_ProjectionMeshProcessor pmp( &outverts, &outindices, cam.mView * cam.mProj, cam.zfar - cam.znear );
	GatherMeshes( cam, &pmp, layers );
}

void SGRX_Scene::DebugDraw_MeshRaycast( uint32_t layers )
{
	for( size_t i = 0; i < m_meshInstances.size(); ++i )
	{
		SGRX_MeshInstance* mi = m_meshInstances.item( i ).key;
		if( mi->layers & layers )
		{
			IMeshRaycast* imrc = mi->raycastOverride ? mi->raycastOverride : mi->GetMesh();
			imrc->MRC_DebugDraw( mi );
		}
	}
}


size_t SGRX_SceneTree::FindNodeIDByName( const StringView& name )
{
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		const Node& N = nodes[ i ];
		if( N.name == name )
			return i;
	}
	return NOT_FOUND;
}

size_t SGRX_SceneTree::FindNodeIDByPath( const StringView& path )
{
	StringView it;
	bool rooted = it.ch() == '/';
	it.skip( 1 );
	
	size_t pos = 0;
	while( it )
	{
		StringView curr = it.until( "/" );
		it.skip( curr.size() + 1 );
	}
	UNUSED( rooted ); // TODO
	UNUSED( pos );
	return _NormalizeIndex( 0 );
}

void SGRX_SceneTree::UpdateTransforms()
{
	transforms.resize( nodes.size() );
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		const Node& N = nodes[ i ];
		if( N.parent_id < i )
			transforms[ i ] = N.transform * transforms[ N.parent_id ];
		else
			transforms[ i ] = N.transform;
	}
	for( size_t i = 0; i < items.size(); ++i )
	{
		Item& I = items[ i ];
		I.item->SetTransform( I.node_id < nodes.size() ? transforms[ I.node_id ] : Mat4::Identity );
	}
}


int GR_GetWidth(){ return g_RenderSettings.width; }
int GR_GetHeight(){ return g_RenderSettings.height; }


TextureHandle GR_CreateTexture( int width, int height, int format, uint32_t flags, int mips )
{
	LOG_FUNCTION;
	
	TextureInfo ti = { TEXTYPE_2D, mips, width, height, 1, format, flags };
	SGRX_ITexture* tex = g_Renderer->CreateTexture( &ti, NULL );
	if( !tex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	tex->m_info = ti;
	
	LOG << "Created 2D texture: " << width << "x" << height << ", format=" << format << ", mips=" << mips;
	return tex;
}

TextureHandle GR_GetTexture( const StringView& path )
{
	LOG_FUNCTION_ARG( path );
	
	SGRX_ITexture* tx = g_Textures->getcopy( path );
	if( tx )
		return tx;
	
	uint32_t usageflags;
	ByteArray imgdata;
	if( !g_Game->OnLoadTexture( path, imgdata, usageflags ) )
	{
		LOG_ERROR << LOG_DATE << "  Could not find texture: " << path;
		return TextureHandle();
	}
	
	TextureData texdata;
	if( !TextureData_Load( &texdata, imgdata, path ) )
	{
		// error is already printed
		return TextureHandle();
	}
	texdata.info.flags = usageflags;
	
	SGRX_ITexture* tex = g_Renderer->CreateTexture( &texdata.info, texdata.data.data() );
	if( !tex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	tex->m_info = texdata.info;
	tex->m_key.append( path.data(), path.size() );
	g_Textures->set( tex->m_key, tex );
	
	LOG << "Loaded texture: " << path;
	return tex;
}

TextureHandle GR_CreateRenderTexture( int width, int height, int format )
{
	LOG_FUNCTION;
	
	TextureInfo ti = { TEXTYPE_2D, 1, width, height, 1, format,
		TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y };
	SGRX_ITexture* tex = g_Renderer->CreateRenderTexture( &ti );
	if( !tex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	tex->m_info = ti;
	
	LOG << "Created renderable texture: " << width << "x" << height << ", format=" << format;
	return tex;
}

TextureHandle GR_GetRenderTarget( int width, int height, int format, int extra )
{
	ASSERT( width && height && format );
	LOG_FUNCTION;
	
	uint64_t key = (uint64_t(width&0xffff)<<48) | (uint64_t(height&0xffff)<<32) | (uint64_t(format&0xffff)<<16) | (uint64_t(extra&0xffff));
	SGRX_ITexture* rtt = g_RenderTargets->getcopy( key );
	if( rtt )
		return rtt;
	
	TextureHandle rtth = GR_CreateRenderTexture( width, height, format );
	rtth->m_rtkey = key;
	g_RenderTargets->set( key, rtth );
	return rtth;
}

DepthStencilSurfHandle GR_CreateDepthStencilSurface( int width, int height, int format )
{
	ASSERT( width && height && format );
	LOG_FUNCTION;
	
	SGRX_IDepthStencilSurface* dss = g_Renderer->CreateDepthStencilSurface( width, height, format );
	if( dss == NULL )
	{
		// valid outcome
		return NULL;
	}
	dss->m_width = width;
	dss->m_height = height;
	dss->m_format = format;
	
	LOG << "Created depth/stencil surface: " << width << "x" << height << ", format=" << format;
	return dss;
}

DepthStencilSurfHandle GR_GetDepthStencilSurface( int width, int height, int format, int extra )
{
	ASSERT( width && height && format );
	LOG_FUNCTION;
	
	uint64_t key = (uint64_t(width&0xffff)<<48) | (uint64_t(height&0xffff)<<32) | (uint64_t(format&0xffff)<<16) | (uint64_t(extra&0xffff));
	SGRX_IDepthStencilSurface* dss = g_DepthStencilSurfs->getcopy( key );
	if( dss )
		return dss;
	
	DepthStencilSurfHandle dssh = GR_CreateDepthStencilSurface( width, height, format );
	if( dssh == NULL )
	{
		// valid outcome
		return NULL;
	}
	dssh->m_key = key;
	g_DepthStencilSurfs->set( key, dssh );
	return dssh;
}


VertexShaderHandle GR_GetVertexShader( const StringView& path )
{
	LOG_FUNCTION_ARG( path );
	
	String code;
	String errors;
	ByteArray comp;
	
	SGRX_IVertexShader* shd = g_VertexShaders->getcopy( path );
	if( shd )
		return shd;
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( g_Game->GetCompiledShader( g_Renderer->GetInfo(), ".vs", path, comp ) )
		{
			goto has_compiled_shader;
		}
	}
	
	if( !g_Game->OnLoadShader( g_Renderer->GetInfo(), path, code ) )
	{
		LOG_ERROR << LOG_DATE << "  Could not find vertex shader: " << path;
		return VertexShaderHandle();
	}
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( !g_Renderer->CompileShader( path, ShaderType_Vertex, code, comp, errors ) )
		{
			LOG_ERROR << LOG_DATE << "  Failed to compile vertex shader: " << path;
			LOG << errors;
			LOG << "---";
			return VertexShaderHandle();
		}
		
		g_Game->SetCompiledShader( g_Renderer->GetInfo(), ".vs", path, comp );
		
has_compiled_shader:
		shd = g_Renderer->CreateVertexShader( path, comp );
	}
	else
	{
		// TODO: I know...
		ByteArray bcode;
		bcode.resize( code.size() );
		memcpy( bcode.data(), code.data(), code.size() );
		shd = g_Renderer->CreateVertexShader( path, bcode );
	}
	
	if( !shd )
	{
		// error already printed in renderer
		return NULL;
	}
	
	shd->m_key = path;
	g_VertexShaders->set( shd->m_key, shd );
	
	LOG << "Loaded vertex shader: " << path;
	return shd;
}

PixelShaderHandle GR_GetPixelShader( const StringView& path )
{
	LOG_FUNCTION_ARG( path );
	
	String code;
	String errors;
	ByteArray comp;
	
	SGRX_IPixelShader* shd = g_PixelShaders->getcopy( path );
	if( shd )
		return shd;
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( g_Game->GetCompiledShader( g_Renderer->GetInfo(), ".ps", path, comp ) )
		{
			goto has_compiled_shader;
		}
	}
	
	if( !g_Game->OnLoadShader( g_Renderer->GetInfo(), path, code ) )
	{
		LOG_ERROR << LOG_DATE << "  Could not find pixel shader: " << path;
		return PixelShaderHandle();
	}
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( !g_Renderer->CompileShader( path, ShaderType_Pixel, code, comp, errors ) )
		{
			LOG_ERROR << LOG_DATE << "  Failed to compile pixel shader: " << path;
			LOG << errors;
			LOG << "---";
			return PixelShaderHandle();
		}
		
		g_Game->SetCompiledShader( g_Renderer->GetInfo(), ".ps", path, comp );
		
has_compiled_shader:
		shd = g_Renderer->CreatePixelShader( path, comp );
	}
	else
	{
		// TODO: I know...
		ByteArray bcode;
		bcode.resize( code.size() );
		memcpy( bcode.data(), code.data(), code.size() );
		shd = g_Renderer->CreatePixelShader( path, bcode );
	}
	
	if( !shd )
	{
		// error already printed in renderer
		return NULL;
	}
	
	shd->m_key = path;
	g_PixelShaders->set( shd->m_key, shd );
	
	LOG << "Loaded pixel shader: " << path;
	return shd;
}


RenderStateHandle GR_GetRenderState( const SGRX_RenderState& state )
{
	LOG_FUNCTION;
	
	SGRX_IRenderState* rs = g_RenderStates->getcopy( state );
	if( rs )
		return rs;
	
	rs = g_Renderer->CreateRenderState( state );
	rs->m_info = state;
	g_RenderStates->set( state, rs );
	
	LOG << "Created render state";
	return rs;
}


VertexDeclHandle GR_GetVertexDecl( const StringView& vdecl )
{
	LOG_FUNCTION_ARG( vdecl );
	
	SGRX_IVertexDecl* VD = g_VertexDecls->getcopy( vdecl );
	if( VD )
		return VD;
	
	VDeclInfo vdinfo = {0};
	const char* err = VDeclInfo_Parse( &vdinfo, StackString< 64 >( vdecl ) );
	if( err )
	{
		LOG_ERROR << LOG_DATE << "  Failed to parse vertex declaration - " << err << " (" << vdecl << ")";
		return NULL;
	}
	
	VD = g_Renderer->CreateVertexDecl( vdinfo );
	if( !VD )
	{
		// error already printed in renderer
		return NULL;
	}
	
	VD->m_key = vdecl;
	g_VertexDecls->set( VD->m_key, VD );
	
	LOG << "Created vertex declaration: " << vdecl;
	return VD;
}


VtxInputMapHandle GR_GetVertexInputMapping( SGRX_IVertexShader* vs, SGRX_IVertexDecl* vd )
{
	LOG_FUNCTION;
	
	if( vs == NULL || vd == NULL )
		return NULL;
	
	SGRX_VtxInputMapKey key = { vs, vd };
	SGRX_IVertexInputMapping* vim = g_VtxInputMaps->getcopy( key );
	if( vim )
		return vim;
	
	vim = g_Renderer->CreateVertexInputMapping( vs, vd );
	if( vim == NULL )
	{
		// valid outcome
		return vim;
	}
	
	vim->m_key = key;
	g_VtxInputMaps->set( key, vim );
	
	LOG << "Created vertex input mapping";
	return vim;
}


MeshHandle GR_CreateMesh()
{
	LOG_FUNCTION;
	
	SGRX_IMesh* mesh = g_Renderer->CreateMesh();
	return mesh;
}

MeshHandle GR_GetMesh( const StringView& path )
{
	LOG_FUNCTION_ARG( path );
	
	SGRX_IMesh* mesh = g_Meshes->getcopy( path );
	if( mesh )
		return mesh;
	
	ByteArray meshdata;
	if( !g_Game->OnLoadMesh( path, meshdata ) )
	{
		LOG_ERROR << LOG_DATE << "  Failed to access mesh data file - " << path;
		return NULL;
	}
	
	MeshFileData mfd;
	const char* err = MeshData_Parse( (char*) meshdata.data(), meshdata.size(), &mfd );
	if( err )
	{
		LOG_ERROR << LOG_DATE << "  Failed to parse mesh file - " << err;
		return NULL;
	}
	
	SGRX_MeshBone bones[ SGRX_MAX_MESH_BONES ];
	for( int i = 0; i < mfd.numBones; ++i )
	{
		MeshFileBoneData* mfdb = &mfd.bones[ i ];
		bones[ i ].name.append( mfdb->boneName, mfdb->boneNameSize );
		bones[ i ].boneOffset = mfdb->boneOffset;
		bones[ i ].parent_id = mfdb->parent_id == 255 ? -1 : mfdb->parent_id;
	}
	
	VertexDeclHandle vdh;
	mesh = g_Renderer->CreateMesh();
	if( !mesh ||
		!( vdh = GR_GetVertexDecl( StringView( mfd.formatData, mfd.formatSize ) ) ) ||
		!mesh->SetVertexData( mfd.vertexData, mfd.vertexDataSize, vdh, ( mfd.dataFlags & MDF_TRIANGLESTRIP ) != 0 ) ||
		!mesh->SetIndexData( mfd.indexData, mfd.indexDataSize, ( mfd.dataFlags & MDF_INDEX_32 ) != 0 ) ||
		!mesh->SetBoneData( bones, mfd.numBones ) )
	{
		LOG << "...while trying to create mesh: " << path;
		delete mesh;
		return NULL;
	}
	
	mesh->m_dataFlags = mfd.dataFlags;
	mesh->m_boundsMin = mfd.boundsMin;
	mesh->m_boundsMax = mfd.boundsMax;
	
	mesh->m_meshParts.clear();
	mesh->m_meshParts.resize( mfd.numParts );
	SGRX_MeshPart* parts = mesh->m_meshParts.data();
	for( int i = 0; i < mfd.numParts; ++i )
	{
		SGRX_MeshPart& P = parts[ i ];
		P.vertexOffset = mfd.parts[ i ].vertexOffset;
		P.vertexCount = mfd.parts[ i ].vertexCount;
		P.indexOffset = mfd.parts[ i ].indexOffset;
		P.indexCount = mfd.parts[ i ].indexCount;
		
		StringView mtltext( mfd.parts[ i ].materialStrings[0], mfd.parts[ i ].materialStringSizes[0] );
		
		// LOAD MATERIAL
		//
		P.mtlFlags = mfd.parts[ i ].flags;
		P.mtlBlendMode = mfd.parts[ i ].blendMode;
		if( mfd.parts[ i ].materialStringSizes[0] >= SHADER_NAME_LENGTH )
		{
			LOG_WARNING << "Shader name for part " << i << " is too long";
		}
		else
		{
			P.shader = mtltext;
		}
		for( int tid = 0; tid < mfd.parts[ i ].materialTextureCount; ++tid )
		{
			P.textures[ tid ] = StringView( mfd.parts[ i ].materialStrings[ tid + 1 ], mfd.parts[ i ].materialStringSizes[ tid + 1 ] );
		}
	}
	
	mesh->m_vdata.append( (const uint8_t*) mfd.vertexData, mfd.vertexDataSize );
	mesh->m_idata.append( (const uint8_t*) mfd.indexData, mfd.indexDataSize );
	mesh->m_key = path;
	g_Meshes->set( mesh->m_key, mesh );
	
	LOG << "Created mesh: " << path;
	return mesh;
}



static SGRX_Animation* _create_animation( AnimFileParser* afp, int anim )
{
	LOG_FUNCTION;
	
	ASSERT( anim >= 0 && anim < (int) afp->animData.size() );
	
	const AnimFileParser::Anim& AN = afp->animData[ anim ];
	
	SGRX_Animation* nanim = new SGRX_Animation;
	
	nanim->name.assign( AN.name, AN.nameSize );
	nanim->frameCount = AN.frameCount;
	nanim->speed = AN.speed;
	nanim->data.resize( AN.trackCount * 10 * AN.frameCount );
	for( int t = 0; t < AN.trackCount; ++t )
	{
		const AnimFileParser::Track& TRK = afp->trackData[ AN.trackDataOff + t ];
		nanim->trackNames.push_back( StringView( TRK.name, TRK.nameSize ) );
		
		float* indata = TRK.dataPtr;
		Vec3* posdata = nanim->GetPosition( t );
		Quat* rotdata = nanim->GetRotation( t );
		Vec3* scldata = nanim->GetScale( t );
		
		for( uint32_t f = 0; f < AN.frameCount; ++f )
		{
			posdata[ f ].x = indata[ f * 10 + 0 ];
			posdata[ f ].y = indata[ f * 10 + 1 ];
			posdata[ f ].z = indata[ f * 10 + 2 ];
			rotdata[ f ].x = indata[ f * 10 + 3 ];
			rotdata[ f ].y = indata[ f * 10 + 4 ];
			rotdata[ f ].z = indata[ f * 10 + 5 ];
			rotdata[ f ].w = indata[ f * 10 + 6 ];
			scldata[ f ].x = indata[ f * 10 + 7 ];
			scldata[ f ].y = indata[ f * 10 + 8 ];
			scldata[ f ].z = indata[ f * 10 + 9 ];
		}
	}
	nanim->markers.resize( AN.markerCount );
	for( int m = 0; m < AN.markerCount; ++m )
	{
		const AnimFileParser::Marker& MRK = afp->markerData[ AN.markerDataOff + m ];
		SGRX_Animation::Marker& MOUT = nanim->markers[ m ];
		memcpy( MOUT.name, MRK.name, MAX_ANIM_MARKER_NAME_LENGTH );
		MOUT.frame = MRK.frame;
	}
	
	return nanim;
}

int GR_LoadAnims( const StringView& path, const StringView& prefix )
{
	LOG_FUNCTION;
	
	ByteArray ba;
	if( !FS_LoadBinaryFile( path, ba ) )
	{
		LOG << "Failed to load animation file: " << path;
		return 0;
	}
	AnimFileParser afp( ba );
	if( afp.error )
	{
		LOG << "Failed to parse animation file (" << path << ") - " << afp.error;
		return 0;
	}
	
	int numanims = 0;
	for( int i = 0; i < (int) afp.animData.size(); ++i )
	{
		SGRX_Animation* anim = _create_animation( &afp, i );
		
		if( prefix )
		{
			anim->name.insert( 0, prefix.data(), prefix.size() );
		}
		
		numanims++;
		g_Anims->set( anim->name, anim );
	}
	
	LOG << "Loaded " << numanims << " animations from " << path << " with prefix " << prefix;
	
	return numanims;
}

AnimHandle GR_GetAnim( const StringView& name )
{
	return g_Anims->getcopy( name );
}



void GR_PreserveResourcePtr( SGRX_RefCounted* rsrc )
{
	g_PreservedResources->set( rsrc, 2 );
}

static void _unpreserve_resources()
{
	for( size_t i = 0; i < g_PreservedResources->size(); ++i )
	{
		if( --g_PreservedResources->item( i ).value <= 0 )
			g_PreservedResources->unset( g_PreservedResources->item( i ).key );
	}
}



SceneHandle GR_CreateScene()
{
	LOG_FUNCTION;
	
	SGRX_Scene* scene = new SGRX_Scene;
	
	LOG << "Created scene";
	return scene;
}

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

bool GR2D_LoadFont( const StringView& key, const StringView& path )
{
	SGRX_IFont* fif = NULL;
	if( ( fif = sgrx_int_CreateFont( path ) ) == NULL )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load font: " << path;
		return false;
	}
	fif->m_key = key;
	g_LoadedFonts->set( fif->m_key, fif );
	return true;
}

bool GR2D_LoadSVGIconFont( const StringView& key, const StringView& path )
{
	SGRX_IFont* fif = NULL;
	if( ( fif = sgrx_int_CreateSVGIconFont( path ) ) == NULL )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load SVG icon font: " << path;
		return false;
	}
	fif->m_key = key;
	g_LoadedFonts->set( fif->m_key, fif );
	return true;
}

FontHandle GR2D_GetFont( const StringView& key )
{
	return g_LoadedFonts->getcopy( key );
}

void GR2D_GetFontSettings( SGRX_FontSettings* settings )
{
	*settings = g_CurFontSettings;
//	SGRX_IFont* fnt = g_FontRenderer->m_currentFont;
//	settings->font = fnt ? fnt->m_key : "";
//	settings->size = g_FontRenderer->m_currentSize;
//	settings->letterspacing = g_CurFontSettings.letterspacing;
//	settings->lineheight = g_CurFontSettings.lineheight;
}

void GR2D_SetFontSettings( SGRX_FontSettings* settings )
{
	g_CurFontSettings = *settings;
	GR2D_SetFont( settings->font, settings->size );
}

bool GR2D_SetFont( const StringView& name, int pxsize )
{
	g_CurFontSettings.font = name;
	g_CurFontSettings.size = pxsize;
	return g_FontRenderer->SetFont( name, pxsize );
}

void GR2D_SetLetterSpacing( float lsp )
{
	g_CurFontSettings.letterspacing = lsp;
}

void GR2D_SetLineHeight( float lht )
{
	g_CurFontSettings.lineheight = lht;
}

void GR2D_SetTextCursor( const Vec2& pos )
{
	g_FontRenderer->SetCursor( pos );
}

Vec2 GR2D_GetTextCursor()
{
	return g_FontRenderer->m_cursor;
}

int GR2D_GetTextLength( const StringView& text )
{
	if( !g_FontRenderer )
		return 0;
	return g_FontRenderer->GetTextWidth( text );
}

int GR2D_DrawTextLine( const StringView& text )
{
	return g_FontRenderer->PutText( g_BatchRenderer, text );
}

int GR2D_DrawTextLine( float x, float y, const StringView& text )
{
	g_FontRenderer->SetCursor( V2( x, y ) );
	return g_FontRenderer->PutText( g_BatchRenderer, text );
}

int GR2D_DrawTextLine( const StringView& text, int halign, int valign )
{
	Vec2 pos = GR2D_GetTextCursor();
	int ret = GR2D_DrawTextLine( pos.x, pos.y, text, halign, valign );
	GR2D_SetTextCursor( GR2D_GetTextCursor().x, pos.y );
	return ret;
}

int GR2D_DrawTextLine( float x, float y, const StringView& text, int halign, int valign )
{
	if( !g_FontRenderer->m_currentFont )
		return 0;
	float length = 0;
	if( halign != 0 )
		length = g_FontRenderer->GetTextWidth( text );
	return GR2D_DrawTextLine( x - round( halign * 0.5f * length ), round( y - valign * 0.5f * g_FontRenderer->m_currentSize ), text );
}

struct TextLine
{
	int start;
	int end;
	int pxwidth;
};

void _GR2D_CalcTextLayout( Array< TextLine >& lines, const StringView& text, int width, int height )
{
	int lineheight = ceilf( g_CurFontSettings.CalcLineHeight() );
	
	int line_start = 0;
	int end_of_last_word = 0;
	int cur_line_width = 0;
	int cur_word_width = 0;
	int num_words = 1;
	
	uint32_t prev_chr_val = 0;
	
	UTF8Iterator IT( text );
	if( IT.Advance() == false )
		return;
	for(;;)
	{
		int chr_pos = IT.offset;
		uint32_t chr_val = IT.codepoint;
		
		int char_width = g_FontRenderer->GetAdvanceX( prev_chr_val, chr_val );
		
		if( chr_val == '\n' )
		{
			cur_line_width += cur_word_width;
			if( prev_chr_val == ' ' )
				num_words--;
			
			TextLine LN = { line_start, chr_pos, cur_line_width };
			lines.push_back( LN );
			
			cur_line_width = 0;
			cur_word_width = 0;
			num_words = 1;
			if( ( (int)lines.size() + 1 ) * lineheight > height )
				break;
			
			// goto next line after all subsequent spaces
			prev_chr_val = 0;
			bool lastadv;
			while( ( lastadv = IT.Advance() ) && IT.codepoint == ' ' );
			if( !lastadv )
				break;
			line_start = IT.offset;
			continue;
		}
		if( chr_val == ' ' )
		{
			cur_line_width += cur_word_width;
			if( prev_chr_val != 0 && prev_chr_val != ' ' )
				num_words++;
			end_of_last_word = chr_pos;
			cur_word_width = 0;
		}
		
		if( cur_line_width + cur_word_width + char_width < width )
		{
			// still within line
			cur_word_width += char_width;
			prev_chr_val = chr_val;
			if( IT.Advance() == false )
				break;
		}
		else
		{
			// over the limit
			if( cur_line_width )
			{
				// if not first word, commit line and restart the word
				cur_word_width = 0;
				num_words--;
				chr_pos = end_of_last_word;
				
				IT.SetOffset( end_of_last_word );
			}
			cur_line_width += cur_word_width;
			
			TextLine LN = { line_start, chr_pos, cur_line_width };
			lines.push_back( LN );
			
			cur_line_width = 0;
			cur_word_width = 0;
			num_words = 1;
			if( ( (int)lines.size() + 1 ) * lineheight > height )
				break;
			
			// goto next line after all subsequent spaces
			prev_chr_val = 0;
			bool lastadv;
			while( ( lastadv = IT.Advance() ) && IT.codepoint == ' ' );
			if( !lastadv )
				break;
			line_start = IT.offset;
		}
	}
	
	cur_line_width += cur_word_width;
	if( cur_line_width )
	{
		TextLine LN = { line_start, text.size(), cur_line_width };
		lines.push_back( LN );
	}
	
	return;
}

static Array< TextLine > lines;
void GR2D_DrawTextRect( int x0, int y0, int x1, int y1,
	const StringView& text, int halign, int valign )
{
	// sizing
	int width = x1 - x0;
	int height = y1 - y0;
	
	int lineheight = ceilf( g_CurFontSettings.CalcLineHeight() );
	if( height < lineheight )
		return;
	
	lines.clear();
	_GR2D_CalcTextLayout( lines, text, width, height );
	
	int vspace = height - lines.size() * lineheight;
	int y = y0;
	if( valign == VALIGN_CENTER ) y += vspace / 2;
	else if( valign == VALIGN_BOTTOM ) y += vspace;
	
	for( size_t i = 0; i < lines.size(); ++i )
	{
		TextLine& LN = lines[ i ];
		int hspace = width - LN.pxwidth;
		int x = x0;
		if( halign == HALIGN_CENTER ) x += hspace / 2;
		else if( halign & HALIGN_RIGHT ) x += hspace;
		
		StringView textpart = text.part( LN.start, LN.end - LN.start );
		
		GR2D_DrawTextLine( x, y, textpart );
		y += lineheight;
	}
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
	
	g_CPSets = new ConvexPointSetHashTable();
	g_Textures = new TextureHashTable();
	g_RenderTargets = new RenderTargetTable();
	g_DepthStencilSurfs = new DepthStencilSurfTable();
	g_VertexShaders = new VertexShaderHashTable();
	g_PixelShaders = new PixelShaderHashTable();
	g_RenderStates = new RenderStateHashTable();
	g_VertexDecls = new VertexDeclHashTable();
	g_VtxInputMaps = new VtxInputMapHashTable();
	g_Meshes = new MeshHashTable();
	g_Anims = new AnimHashTable();
	g_LoadedFonts = new FontHashTable();
	g_Joysticks = new JoystickHashTable();
	g_PreservedResources = new ResourcePreserveHashTable();
	LOG << LOG_DATE << "  Created renderer resource caches";
	
	g_BatchRenderer = new BatchRenderer( g_Renderer );
	LOG << LOG_DATE << "  Created batch renderer";
	
	sgrx_int_InitializeFontRendering();
	g_FontRenderer = new FontRenderer();
	LOG << LOG_DATE << "  Created font renderer";
	
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
	
	delete g_FontRenderer;
	g_FontRenderer = NULL;
	sgrx_int_FreeFontRendering();
	
	delete g_BatchRenderer;
	g_BatchRenderer = NULL;
	
	delete g_PreservedResources;
	g_PreservedResources = NULL;
	
	delete g_Joysticks;
	g_Joysticks = NULL;
	
	delete g_LoadedFonts;
	g_LoadedFonts = NULL;
	
	delete g_Anims;
	g_Anims = NULL;
	
	delete g_Meshes;
	g_Meshes = NULL;
	
	delete g_VtxInputMaps;
	g_VtxInputMaps = NULL;
	
	delete g_VertexDecls;
	g_VertexDecls = NULL;
	
	delete g_RenderStates;
	g_RenderStates = NULL;
	
	delete g_PixelShaders;
	g_PixelShaders = NULL;
	
	delete g_VertexShaders;
	g_VertexShaders = NULL;
	
	delete g_DepthStencilSurfs;
	g_DepthStencilSurfs = NULL;
	
	delete g_RenderTargets;
	g_RenderTargets = NULL;
	
	delete g_Textures;
	g_Textures = NULL;
	
	delete g_CPSets;
	g_CPSets = NULL;
	
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
	
	g_ActionMap = new ActionMap;
	
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
		_unpreserve_resources();
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
	
	LOG << LOG_DATE << "  Engine finished";
	return 0;
}

