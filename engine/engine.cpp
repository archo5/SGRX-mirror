

#include <stdio.h>
#include <time.h>
#include <intrin.h>
#include <windows.h>

#define USE_VEC2
#define USE_VEC3
#define USE_MAT4
#define USE_ARRAY

#define INCLUDE_REAL_SDL
#include "engine_int.hpp"
#include "renderer.hpp"


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


//
// GLOBALS
//

static bool g_Running = true;
static SDL_Window* g_Window = NULL;
static void* g_GameLib = NULL;
static IGame* g_Game = NULL;
static uint32_t g_GameTime = 0;
static ActionMap* g_ActionMap;
static Vec2 g_CursorPos = {0,0};

static RenderSettings g_RenderSettings = { 1024, 576, false, false, false };
static const char* g_RendererName = "sgrx-render-d3d9";
static void* g_RenderLib = NULL;
static pfnRndInitialize g_RfnInitialize = NULL;
static pfnRndFree g_RfnFree = NULL;
static pfnRndCreateRenderer g_RfnCreateRenderer = NULL;
static IRenderer* g_Renderer = NULL;
static BatchRenderer* g_BatchRenderer = NULL;
static FontRenderer* g_FontRenderer = NULL;
static Array< IScreen* > g_OverlayScreens;


//
// LOGGING
//

SGRX_Log::SGRX_Log() : end_newline(true), need_sep(false), sep("") {}
SGRX_Log::~SGRX_Log(){ sep = ""; if( end_newline ) *this << "\n"; }

void SGRX_Log::prelog()
{
	if( need_sep )
		printf( "%s", sep );
	else
		need_sep = true;
}

SGRX_Log& SGRX_Log::operator << ( EMod_Partial ){ end_newline = false; return *this; }
SGRX_Log& SGRX_Log::operator << ( ESpec_Date )
{
	time_t ttv;
	time( &ttv );
	struct tm T = *localtime( &ttv );
	char pbuf[ 256 ] = {0};
	strftime( pbuf, 255, "%Y-%m-%d %H:%M:%S", &T );
	printf( pbuf );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( const Separator& s ){ sep = s.sep; return *this; }
SGRX_Log& SGRX_Log::operator << ( int8_t v ){ prelog(); printf( "%d", (int) v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( uint8_t v ){ prelog(); printf( "%d", (int) v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( int16_t v ){ prelog(); printf( "%d", (int) v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( uint16_t v ){ prelog(); printf( "%d", (int) v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( int32_t v ){ prelog(); printf( "%" PRId32, v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( uint32_t v ){ prelog(); printf( "%" PRIu32, v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( int64_t v ){ prelog(); printf( "%" PRId64, v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( uint64_t v ){ prelog(); printf( "%" PRIu64, v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( float v ){ return *this << (double) v; }
SGRX_Log& SGRX_Log::operator << ( double v ){ prelog(); printf( "%f", v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( const void* v ){ prelog(); printf( "[%p]", v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( const char* v ){ prelog(); printf( "%s", v ); return *this; }
SGRX_Log& SGRX_Log::operator << ( const StringView& sv ){ prelog(); printf( "[%d]\"%.*s\"", (int) sv.size(), (int) sv.size(), sv.data() ); return *this; }
SGRX_Log& SGRX_Log::operator << ( const Vec2& v )
{
	prelog();
	printf( "Vec2( %f ; %f )", v.x, v.y );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( const Vec3& v )
{
	prelog();
	printf( "Vec3( %f ; %f ; %f )", v.x, v.y, v.z );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( const Mat4& v )
{
	prelog();
	printf( "Mat4(\n" );
	printf( "\t%f\t%f\t%f\t%f\n",  v.m[0][0], v.m[0][1], v.m[0][2], v.m[0][3] );
	printf( "\t%f\t%f\t%f\t%f\n",  v.m[1][0], v.m[1][1], v.m[1][2], v.m[1][3] );
	printf( "\t%f\t%f\t%f\t%f\n",  v.m[2][0], v.m[2][1], v.m[2][2], v.m[2][3] );
	printf( "\t%f\t%f\t%f\t%f\n)", v.m[3][0], v.m[3][1], v.m[3][2], v.m[3][3] );
	return *this;
}


//
// GAME SYSTEMS
//

void Command::_SetState( float x )
{
	value = x;
	state = x >= threshold;
}

void Command::_Advance()
{
	prev_value = value;
	prev_state = state;
}

void Game_RegisterAction( Command* cmd )
{
	g_ActionMap->Register( cmd );
}

void Game_UnregisterAction( Command* cmd )
{
	g_ActionMap->Unregister( cmd );
}

void Game_BindKeyToAction( uint32_t key, Command* cmd )
{
	g_ActionMap->Map( ACTINPUT_MAKE( ACTINPUT_KEY, key ), cmd );
}

void Game_BindKeyToAction( uint32_t key, const StringView& cmd )
{
	g_ActionMap->Map( ACTINPUT_MAKE( ACTINPUT_KEY, key ), cmd );
}

Vec2 Game_GetCursorPos()
{
	return g_CursorPos;
}


bool Game_HasOverlayScreen( IScreen* screen )
{
	return g_OverlayScreens.has( screen );
}

void Game_AddOverlayScreen( IScreen* screen )
{
	g_OverlayScreens.push_back( screen );
	screen->OnStart();
}

void Game_RemoveOverlayScreen( IScreen* screen )
{
	if( g_OverlayScreens.has( screen ) )
	{
		screen->OnEnd();
		g_OverlayScreens.remove_all( screen );
	}
}

static void process_overlay_screens( float dt )
{
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
	if( e.type == SDL_MOUSEMOTION )
	{
		g_CursorPos.x = e.motion.x;
		g_CursorPos.y = e.motion.y;
	}
	
	for( size_t i = g_OverlayScreens.size(); i > 0; )
	{
		i--;
		IScreen* screen = g_OverlayScreens[ i ];
		if( screen->OnEvent( e ) )
			return; // event inhibited
	}
	
	if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP )
	{
		Command* cmd = g_ActionMap->Get( ACTINPUT_MAKE_KEY( e.key.keysym.sym ) );
		if( cmd )
			cmd->_SetState( e.key.state );
	}
	
	if( e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP )
	{
		Command* cmd = g_ActionMap->Get( ACTINPUT_MAKE_MOUSE( e.button.button ) );
		if( cmd )
			cmd->_SetState( e.button.state );
	}
}

TextureHandle metal;
void Game_Process( float dt )
{
	float f[4] = { 0.2f, 0.4f, 0.6f, 1.0f };
	g_Renderer->Clear( f );
	
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	BatchRenderer& br = *g_BatchRenderer;
	br.SetTexture( metal );
	br.Col( 1 );
	br.SetPrimitiveType( PT_Triangles );
	br.Tex( 0, 0 ).Pos( 0, 0 )
		.Tex( 1, 1 ).Pos( -50, -50 )
		.Tex( 1, 0 ).Pos( -50, 0 );
	br.Tex( 0, 0 ).Pos( 0, 0 )
		.Tex( 1, 1 ).Pos( 200, 200 )
		.Tex( 1, 0 ).Pos( 200, 0 );
	
	GR2D_SetFont( "fonts/lato-regular.ttf", 11 );
	GR2D_SetTextCursor( 0, 0 );
	for( int i = 0; i < 249; ++i )
	{
		GR2D_DrawTextLine( i % 3 == 0 ? " SomeTextHere" : " TextHere" );
		if( GR2D_GetTextCursor().x > GR_GetWidth() )
		{
			GR2D_SetTextCursor( ( i % 5 ) * 5, GR2D_GetTextCursor().y + 11 );
		}
	}
	br.Flush();
	
	g_Game->OnTick( dt, g_GameTime );
	
	process_overlay_screens( dt );
	
	br.Flush();
}


void ParseDefaultTextureFlags( const StringView& flags, uint32_t& outusageflags )
{
	if( flags.contains( ":nosrgb" ) ) outusageflags &= ~TEXFLAGS_SRGB;
	if( flags.contains( ":srgb" ) ) outusageflags |= TEXFLAGS_SRGB;
	if( flags.contains( ":wrapx" ) ) outusageflags &= ~TEXFLAGS_CLAMP_X;
	if( flags.contains( ":wrapy" ) ) outusageflags &= ~TEXFLAGS_CLAMP_Y;
	if( flags.contains( ":clampx" ) ) outusageflags |= TEXFLAGS_CLAMP_X;
	if( flags.contains( ":clampy" ) ) outusageflags |= TEXFLAGS_CLAMP_Y;
	if( flags.contains( ":nolerp" ) ) outusageflags &= ~(TEXFLAGS_LERP_X | TEXFLAGS_LERP_Y);
	if( flags.contains( ":nolerpx" ) ) outusageflags &= ~TEXFLAGS_LERP_X;
	if( flags.contains( ":nolerpy" ) ) outusageflags &= ~TEXFLAGS_LERP_Y;
	if( flags.contains( ":lerp" ) ) outusageflags |= (TEXFLAGS_LERP_X | TEXFLAGS_LERP_Y);
	if( flags.contains( ":lerpx" ) ) outusageflags |= TEXFLAGS_LERP_X;
	if( flags.contains( ":lerpy" ) ) outusageflags |= TEXFLAGS_LERP_Y;
	if( flags.contains( ":nomips" ) ) outusageflags &= ~TEXFLAGS_HASMIPS;
	if( flags.contains( ":mips" ) ) outusageflags |= TEXFLAGS_HASMIPS;
}

bool IGame::OnLoadTexture( const StringView& key, ByteArray& outdata, uint32_t& outusageflags )
{
	if( !key )
		return false;
	
	StringView path = key.until( ":" );
	
	StackString< ENGINE_MAX_PATH > pathNT( path );
	if( !LoadBinaryFile( pathNT, outdata ) )
		return false;
	
	outusageflags = TEXFLAGS_HASMIPS | TEXFLAGS_LERP_X | TEXFLAGS_LERP_Y;
	if( path.contains( "diff." ) )
	{
		// diffuse maps
		outusageflags |= TEXFLAGS_SRGB;
	}
	
	StringView flags = key.from( ":" );
	ParseDefaultTextureFlags( flags, outusageflags );
	
	return true;
}


void SGRX_Texture::Destroy()
{
	m_texture->Destroy();
	delete this;
}

const TextureInfo& SGRX_Texture::GetInfo()
{
	return m_texture->m_info;
}

bool SGRX_Texture::UploadRGBA8Part( void* data, int mip, int w, int h, int x, int y )
{
	const TextureInfo& TI = m_texture->m_info;
	
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
	
	return m_texture->UploadRGBA8Part( data, mip, x, y, w, h );
}

const TextureInfo& TextureHandle::GetInfo()
{
	static TextureInfo dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->GetInfo();
}

bool TextureHandle::UploadRGBA8Part( void* data, int mip, int w, int h, int x, int y )
{
	if( !item )
		return false;
	return item->UploadRGBA8Part( data, mip, w, h, x, y );
}


int GR_GetWidth(){ return g_RenderSettings.width; }
int GR_GetHeight(){ return g_RenderSettings.height; }


TextureHandle GR_CreateTexture( int width, int height, int format, int mips )
{
	TextureInfo ti = { 0, TEXTYPE_2D, width, height, 1, format, mips };
	ITexture* itex = g_Renderer->CreateTexture( &ti, NULL );
	if( !itex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	SGRX_Texture* tex = new SGRX_Texture;
	tex->m_refcount = 0;
	tex->m_texture = itex;
	
	LOG << "Created 2D texture: " << width << "x" << height << ", format=" << format << ", mips=" << mips;
	return tex;
}

TextureHandle GR_GetTexture( const StringView& path )
{
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
	
	ITexture* itex = g_Renderer->CreateTexture( &texdata.info, texdata.data.data() );
	if( !itex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	SGRX_Texture* tex = new SGRX_Texture;
	tex->m_refcount = 0;
	tex->m_texture = itex;
	tex->m_key.append( path.data(), path.size() );
	
	LOG << "Loaded texture: " << path;
	return tex;
}


void GR2D_SetWorldMatrix( const Mat4& mtx )
{
	g_Renderer->SetWorldMatrix( mtx );
}

void GR2D_SetViewMatrix( const Mat4& mtx )
{
	g_Renderer->SetViewMatrix( mtx );
}

bool GR2D_SetFont( const StringView& name, int pxsize )
{
	return g_FontRenderer->SetFont( name, pxsize );
}

void GR2D_SetTextCursor( float x, float y )
{
	g_FontRenderer->SetCursor( x, y );
}

Vec2 GR2D_GetTextCursor()
{
	return Vec2::Create( g_FontRenderer->m_cursor_x, g_FontRenderer->m_cursor_y );
}

int GR2D_DrawTextLine( const StringView& text )
{
	return g_FontRenderer->PutText( g_BatchRenderer, text );
}

int GR2D_DrawTextLine( float x, float y, const StringView& text )
{
	g_FontRenderer->SetCursor( x, y );
	return g_FontRenderer->PutText( g_BatchRenderer, text );
}

BatchRenderer& GR2D_GetBatchRenderer()
{
	return *g_BatchRenderer;
}


//
// RENDERING
//

BatchRenderer::BatchRenderer( struct IRenderer* r ) : m_renderer( r ), m_texture( NULL ), m_primType( PT_None )
{
	m_swapRB = r->GetInfo().swapRB;
	m_proto.x = 0;
	m_proto.y = 0;
	m_proto.z = 0;
	m_proto.u = 0;
	m_proto.v = 0;
	m_proto.color = 0xffffffff;
}

BatchRenderer& BatchRenderer::AddVertices( Vertex* verts, int count )
{
	m_verts.reserve( m_verts.size() + count );
	for( int i = 0; i < count; ++i )
		AddVertex( verts[ i ] );
	return *this;
}

BatchRenderer& BatchRenderer::AddVertex( const Vertex& vert )
{
	m_verts.push_back( vert );
	return *this;
}

BatchRenderer& BatchRenderer::Colb( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	uint32_t col;
	if( m_swapRB )
		col = COLOR_RGBA( b, g, r, a );
	else
		col = COLOR_RGBA( r, g, b, a );
	m_proto.color = col;
	return *this;
}

BatchRenderer& BatchRenderer::SetPrimitiveType( EPrimitiveType pt )
{
	if( pt != m_primType )
	{
		Flush();
		m_primType = pt;
	}
	return *this;
}

BatchRenderer& BatchRenderer::Prev( int i )
{
	if( i < 0 || i >= m_verts.size() )
		AddVertex( m_proto );
	else
		AddVertex( m_verts[ m_verts.size() - 1 - i ] );
	return *this;
}

BatchRenderer& BatchRenderer::Quad( float x0, float y0, float x1, float y1 )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x0, y0 );
	Tex( 1, 0 ); Pos( x1, y0 );
	Tex( 1, 1 ); Pos( x1, y1 );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x0, y1 );
	Prev( 4 );
	return *this;
}

bool BatchRenderer::CheckSetTexture( const TextureHandle& tex )
{
	if( tex != m_texture )
	{
		Flush();
		m_texture = tex;
		return true;
	}
	return false;
}

BatchRenderer& BatchRenderer::SetTexture( const TextureHandle& tex )
{
	CheckSetTexture( tex );
	return *this;
}

BatchRenderer& BatchRenderer::Flush()
{
	if( m_verts.size() )
	{
		m_renderer->DrawBatchVertices( m_verts.data(), m_verts.size(), m_primType, m_texture ? m_texture->m_texture : NULL );
		m_verts.clear();
	}
	return *this;
}



//
// INTERNALS
//

static int init_graphics()
{
	g_Window = SDL_CreateWindow( "SGRX Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_RenderSettings.width, g_RenderSettings.height, 0 );
	
	char renderer_dll[ 65 ] = {0};
	snprintf( renderer_dll, 64, "%s.dll", g_RendererName );
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
	const char* rendername = g_RendererName;
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
	LOG << LOG_DATE << "  Loaded renderer: " << rendername;
	
	g_BatchRenderer = new BatchRenderer( g_Renderer );
	LOG << LOG_DATE << "  Created batch renderer";
	
	InitializeFontRendering();
	g_FontRenderer = new FontRenderer();
	LOG << LOG_DATE << "  Created font renderer";
	
	return 0;
}

static void free_graphics()
{
	delete g_FontRenderer;
	g_FontRenderer = NULL;
	
	delete g_BatchRenderer;
	g_BatchRenderer = NULL;
	
	g_Renderer->Destroy();
	g_Renderer = NULL;
	
	g_RfnFree();
	
	SDL_UnloadObject( g_RenderLib );
	g_RenderLib = NULL;
	
	SDL_DestroyWindow( g_Window );
	g_Window = NULL;
}


typedef IGame* (*pfnCreateGame) ();

int SGRX_EntryPoint( int argc, char** argv, int debug )
{
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
	
	/* initialize SDL */
	if( SDL_Init(
		SDL_INIT_TIMER | SDL_INIT_VIDEO |
		SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC |
		SDL_INIT_GAMECONTROLLER |
		SDL_INIT_EVENTS | SDL_INIT_NOPARACHUTE
	) < 0 )
	{
		LOG_ERROR << "Couldn't initialize SDL: " << SDL_GetError();
		return 5;
	}
	
	g_ActionMap = new ActionMap;
	
	g_GameLib = SDL_LoadObject( "game.dll" );
	if( !g_GameLib )
	{
		LOG_ERROR << "Failed to load game.dll";
		return 6;
	}
	pfnCreateGame cgproc = (pfnCreateGame) SDL_LoadFunction( g_GameLib, "CreateGame" );
	if( !cgproc )
	{
		LOG_ERROR << "Failed to find entry point";
		return 7;
	}
	g_Game = cgproc();
	if( !g_Game )
	{
		LOG_ERROR << "Failed to create the game";
		return 8;
	}
	
	g_Game->OnConfigure( argc, argv );
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	
	if( init_graphics() )
		return 16;
	
	g_Game->OnInitialize();
	metal = GR_GetTexture( "metal0.png" );
	
	uint32_t prevtime = GetTimeMsec();
	SDL_Event event;
	while( g_Running )
	{
		while( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_QUIT )
			{
				g_Running = false;
				break;
			}
			
			Game_OnEvent( event );
		}
		
		uint32_t curtime = GetTimeMsec();
		uint32_t dt = curtime - prevtime;
		prevtime = curtime;
		g_GameTime += dt;
		float fdt = dt / 1000.0f;
		
		Game_Process( fdt );
		
		g_Renderer->Swap();
	}
	metal = NULL;
	
	g_Game->OnDestroy();
	
	free_graphics();
	
	SDL_UnloadObject( g_GameLib );
	
	delete g_ActionMap;
	
	LOG << LOG_DATE << "  Engine finished";
	return 0;
}

