

#include <stdio.h>
#include <time.h>
#ifdef __MINGW32__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif
#include <windows.h>

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#define USE_SERIALIZATION

#define INCLUDE_REAL_SDL
#include "engine_int.hpp"
#include "enganim.hpp"
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

typedef HashTable< StringView, SGRX_ITexture* > TextureHashTable;
typedef HashTable< StringView, SGRX_IVertexShader* > VertexShaderHashTable;
typedef HashTable< StringView, SGRX_IPixelShader* > PixelShaderHashTable;
typedef HashTable< StringView, SGRX_SurfaceShader* > SurfShaderHashTable;
typedef HashTable< StringView, SGRX_Material* > MaterialHashTable;
typedef HashTable< StringView, SGRX_IVertexDecl* > VertexDeclHashTable;
typedef HashTable< StringView, SGRX_IMesh* > MeshHashTable;
typedef HashTable< StringView, AnimHandle > AnimHashTable;
typedef HashTable< StringView, FontHandle > FontHashTable;

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
static Array< IScreen* > g_OverlayScreens;
static Array< FileSysHandle > g_FileSystems;

static RenderSettings g_RenderSettings = { 0, 1024, 576, 60, FULLSCREEN_NONE, true, ANTIALIAS_MULTISAMPLE, 4 };
static const char* g_RendererPrefix = "sgrx-render-";
static void* g_RenderLib = NULL;
static pfnRndInitialize g_RfnInitialize = NULL;
static pfnRndFree g_RfnFree = NULL;
static pfnRndCreateRenderer g_RfnCreateRenderer = NULL;
static IRenderer* g_Renderer = NULL;
static BatchRenderer* g_BatchRenderer = NULL;
static FontRenderer* g_FontRenderer = NULL;
static TextureHashTable* g_Textures = NULL;
static VertexShaderHashTable* g_VertexShaders = NULL;
static PixelShaderHashTable* g_PixelShaders = NULL;
static SurfShaderHashTable* g_SurfShaders = NULL;
static MaterialHashTable* g_Materials = NULL;
static VertexDeclHashTable* g_VertexDecls = NULL;
static MeshHashTable* g_Meshes = NULL;
static AnimHashTable* g_Anims = NULL;
static FontHashTable* g_LoadedFonts = NULL;




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
	g_ActionMap->Map( ACTINPUT_MAKE_KEY( key ), cmd );
}

void Game_BindKeyToAction( uint32_t key, const StringView& cmd )
{
	g_ActionMap->Map( ACTINPUT_MAKE_KEY( key ), cmd );
}

void Game_BindMouseButtonToAction( int btn, Command* cmd )
{
	g_ActionMap->Map( ACTINPUT_MAKE_MOUSE( btn ), cmd );
}

void Game_BindMouseButtonToAction( int btn, const StringView& cmd )
{
	g_ActionMap->Map( ACTINPUT_MAKE_MOUSE( btn ), cmd );
}

ActionInput Game_GetActionBinding( Command* cmd )
{
	ActionMap::InputCmdMap* icm = &g_ActionMap->m_inputCmdMap;
	for( size_t i = 0; i < icm->size(); ++i )
	{
		if( icm->item(i).value == cmd )
			return icm->item(i).key;
	}
	return 0;
}

void Game_BindInputToAction( ActionInput iid, Command* cmd )
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
	case ACTINPUT_JOYSTICK0:
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

void Game_RemoveAllOverlayScreens()
{
	while( g_OverlayScreens.size() )
	{
		Game_RemoveOverlayScreen( g_OverlayScreens.last() );
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
	g_Game->OnEvent( e );
	
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
		int btn = -1;
		if( e.button.button == SDL_BUTTON_LEFT ) btn = SGRX_MB_LEFT;
		if( e.button.button == SDL_BUTTON_RIGHT ) btn = SGRX_MB_RIGHT;
		if( e.button.button == SDL_BUTTON_MIDDLE ) btn = SGRX_MB_MIDDLE;
		if( btn >= 0 )
		{
			Command* cmd = g_ActionMap->Get( ACTINPUT_MAKE_MOUSE( btn ) );
			if( cmd )
				cmd->_SetState( e.button.state );
		}
	}
}

void Game_Process( float dt )
{
	if( !g_windowVisible || !g_hasFocus )
	{
		Thread_Sleep( 40 );
		return;
	}
	
	float f[4] = { 0.2f, 0.4f, 0.6f, 1.0f };
	g_Renderer->Clear( f );
	
	g_Game->OnTick( dt, g_GameTime );
	
	process_overlay_screens( dt );
	
	g_BatchRenderer->Flush();
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
	
	if( !FS_LoadBinaryFile( path, outdata ) )
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

void IGame::GetShaderCacheFilename( const StringView& type, const char* sfx, const StringView& key, String& name )
{
	name = "shadercache_";
	name.append( type.data(), type.size() );
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

bool IGame::GetCompiledShader( const StringView& type, const char* sfx, const StringView& key, ByteArray& outdata )
{
	if( !key )
		return false;
	
	String filename;
	GetShaderCacheFilename( type, sfx, key, filename );
	
	LOG << "Loading precompiled shader: " << filename << " (type=" << type << ", key=" << key << ")";
	return FS_LoadBinaryFile( filename, outdata );
}

bool IGame::SetCompiledShader( const StringView& type, const char* sfx, const StringView& key, const ByteArray& data )
{
	if( !key )
		return false;
	
	String filename;
	GetShaderCacheFilename( type, sfx, key, filename );
	
	LOG << "Saving precompiled shader: " << filename << " (type=" << type << ", key=" << key << ")";
	return FS_SaveBinaryFile( filename, data.data(), data.size() );
}

bool IGame::OnLoadShader( const StringView& type, const StringView& key, String& outdata )
{
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
				while( def.size() )
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
			else
			{
				prepend.append( STRLIT_BUF( "#define " ) );
				prepend.append( cur.data(), cur.size() );
				prepend.append( STRLIT_BUF( "\n" ) );
			}
			it.skip( cur.size() + 1 );
		}
		
		String tpl_data, mtl_data, vs_data;
		if( !OnLoadShaderFile( type, String_Concat( "tpl_", tpl ), tpl_data ) )
			return false;
		if( mtl.size() && !OnLoadShaderFile( type, String_Concat( "mtl_", mtl ), mtl_data ) )
			return false;
		if( vs.size() && !OnLoadShaderFile( type, String_Concat( "vs_", vs ), vs_data ) )
			return false;
		outdata = String_Concat( prepend, String_Replace( String_Replace( tpl_data, "__CODE__", mtl_data ), "__VSCODE__", vs_data ) );
		return true;
	}
	return OnLoadShaderFile( type, key, outdata );
}

bool IGame::OnLoadShaderFile( const StringView& type, const StringView& path, String& outdata )
{
	String filename = "shaders_";
	filename.append( type.data(), type.size() );
	filename.push_back( '/' );
	filename.append( path.data(), path.size() );
	filename.append( STRLIT_BUF( ".shd" ) );
	
	if( !FS_LoadTextFile( filename, outdata ) )
	{
		LOG_WARNING << "Failed to load shader file: " << filename << " (type=" << type << ", path=" << path << ")";
		return false;
	}
	return ParseShaderIncludes( type, path, outdata );
}

bool IGame::ParseShaderIncludes( const StringView& type, const StringView& path, String& outdata )
{
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
		
		// comment pointing to file name
		nstr.append( "// " );
		nstr.append( incpath );
		nstr.append( "\n" );
		
		// contents of new file
		String incfiledata;
		if( !OnLoadShaderFile( type, incpath, incfiledata ) )
			return false;
		nstr.append( incfiledata );
		
		// continue
		it = inc.after( "\"" ).after( "\"" );
	}
	nstr.append( it );
	
	outdata = nstr;
	return true;
}

bool IGame::OnLoadMesh( const StringView& key, ByteArray& outdata )
{
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
	return ::LoadBinaryFile( String_Concat( m_fileRoot, path ), out );
}

bool BasicFileSystem::SaveBinaryFile( const StringView& path, const void* data, size_t size )
{
	return ::SaveBinaryFile( String_Concat( m_fileRoot, path ), data, size );
}

bool BasicFileSystem::LoadTextFile( const StringView& path, String& out )
{
	return ::LoadTextFile( String_Concat( m_fileRoot, path ), out );
}

bool BasicFileSystem::SaveTextFile( const StringView& path, const StringView& data )
{
	return ::SaveTextFile( String_Concat( m_fileRoot, path ), data );
}

bool BasicFileSystem::FileExists( const StringView& path )
{
	return ::FileExists( String_Concat( m_fileRoot, path ) );
}

bool BasicFileSystem::DirCreate( const StringView& path )
{
	return ::DirCreate( String_Concat( m_fileRoot, path ) );
}

void BasicFileSystem::IterateDirectory( const StringView& path, IDirEntryHandler* deh )
{
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
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->LoadBinaryFile( path, out ) )
			return true;
	return false;
}

bool FS_SaveBinaryFile( const StringView& path, const void* data, size_t size )
{
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->SaveBinaryFile( path, data, size ) )
			return true;
	return false;
}

bool FS_LoadTextFile( const StringView& path, String& out )
{
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->LoadTextFile( path, out ) )
			return true;
	return false;
}

bool FS_SaveTextFile( const StringView& path, const StringView& data )
{
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->SaveTextFile( path, data ) )
			return true;
	return false;
}

bool FS_FileExists( const StringView& path )
{
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->FileExists( path ) )
			return true;
	return false;
}

bool FS_DirCreate( const StringView& path )
{
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		if( g_FileSystems[ i ]->DirCreate( path ) )
			return true;
	return false;
}

void FS_IterateDirectory( const StringView& path, IDirEntryHandler* deh )
{
	for( size_t i = 0; i < g_FileSystems.size(); ++i )
		g_FileSystems[ i ]->IterateDirectory( path, deh );
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


SGRX_ITexture::~SGRX_ITexture()
{
	LOG << "Deleted texture: " << m_key;
	g_Textures->unset( m_key );
}

const TextureInfo& TextureHandle::GetInfo() const
{
	static TextureInfo dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}

bool TextureHandle::UploadRGBA8Part( void* data, int mip, int w, int h, int x, int y )
{
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


SGRX_IVertexShader::~SGRX_IVertexShader()
{
	g_VertexShaders->unset( m_key );
}

SGRX_IPixelShader::~SGRX_IPixelShader()
{
	g_PixelShaders->unset( m_key );
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


void VD_ExtractVec3( const VDeclInfo& vdinfo, int vcount, const void** vertptrs, Vec3* outpos, int usage = VDECLUSAGE_POSITION )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage );
	if( ty == -1 || ofs == -1 )
		return;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1: for( int i = 0; i < vcount; ++i ) outpos[ i ] = V3( *(float*)((uint8_t*)vertptrs[i]+ofs), 0, 0 ); break;
	case VDECLTYPE_FLOAT2: for( int i = 0; i < vcount; ++i ) outpos[ i ] = V3( *(float*)((uint8_t*)vertptrs[i]+ofs), *(float*)((uint8_t*)vertptrs[i]+ofs+4), 0 ); break;
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4: for( int i = 0; i < vcount; ++i ) outpos[ i ] = *(Vec3*)((uint8_t*)vertptrs[i]+ofs); break;
	case VDECLTYPE_BCOL4: for( int i = 0; i < vcount; ++i ) outpos[ i ] = V3( *((uint8_t*)vertptrs[i]+ofs)/255.0f, *((uint8_t*)vertptrs[i]+ofs+1)/255.0f, *((uint8_t*)vertptrs[i]+ofs+2)/255.0f ); break;
	}
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


SGRX_SurfaceShader::~SGRX_SurfaceShader()
{
	g_SurfShaders->unset( m_key );
}

void SGRX_SurfaceShader::ReloadShaders()
{
	// prevent deallocation
	Array< VertexShaderHandle > newshaders_vb;
	Array< VertexShaderHandle > newshaders_vs;
	Array< PixelShaderHandle > newshaders_px;
	newshaders_vb.resize( g_Renderer->m_renderPasses.size() );
	newshaders_vs.resize( g_Renderer->m_renderPasses.size() );
	newshaders_px.resize( g_Renderer->m_renderPasses.size() );
	
	for( size_t pass_id = 0; pass_id < g_Renderer->m_renderPasses.size(); ++pass_id )
	{
		const SGRX_RenderPass& PASS = g_Renderer->m_renderPasses[ pass_id ];
		if( PASS.type != RPT_SHADOWS && PASS.type != RPT_OBJECT )
			continue;
		
		char bfr[ 1000 ] = {0};
		sgrx_snprintf( bfr, sizeof(bfr), "mtl:%.*s:%.*s", (int) m_key.size(), m_key.data(), (int) PASS.shader_name.size(), PASS.shader_name.data() );
		if( PASS.flags & RPF_OBJ_DYNAMIC )
			strcat( bfr, ":DYNAMIC" );
		if( PASS.flags & RPF_OBJ_STATIC )
			strcat( bfr, ":STATIC" );
		if( PASS.flags & RPF_DECALS )
			strcat( bfr, ":DECALS" );
		newshaders_px[ pass_id ] = GR_GetPixelShader( bfr );
		newshaders_vb[ pass_id ] = GR_GetVertexShader( bfr );
		strcat( bfr, ":SKIN" );
		newshaders_vs[ pass_id ] = GR_GetVertexShader( bfr );
	}
	
	m_basicVertexShaders = newshaders_vb;
	m_skinVertexShaders = newshaders_vs;
	m_pixelShaders = newshaders_px;
}


SGRX_Material::SGRX_Material() :
	transparent(0), unlit(0), additive(0)
{
}

SGRX_Material::~SGRX_Material()
{
	g_Materials->unset( m_key );
}


SGRX_IMesh::SGRX_IMesh() :
	m_dataFlags( 0 ),
	m_vertexCount( 0 ),
	m_vertexDataSize( 0 ),
	m_indexCount( 0 ),
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

bool SGRX_IMesh::SetPartData( SGRX_MeshPart* parts, int count )
{
	if( count < 0 )
		return false;
	m_meshParts.assign( parts, count );
	return true;
}

bool SGRX_IMesh::SetBoneData( SGRX_MeshBone* bones, int count )
{
	if( count < 0 || count > MAX_MESH_BONES )
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
	
	Mat4 skinOffsets[ MAX_MESH_BONES ];
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
	VD_ExtractVec3( *vdinfo, 3, verts, pos );
	
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

void SGRX_IMesh_Clip_Core_ClipTriangle( const Mat4& mtx, const Mat4& vpmtx, ByteArray& outverts, SGRX_IVertexDecl* vdecl, bool decal, float inv_zn2zf, const void* v1, const void* v2, const void* v3 )
{
	const void* verts[3] = { v1, v2, v3 };
	Vec3 pos[3] = {0};
	VD_ExtractVec3( vdecl->m_info, 3, verts, pos );
	
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
		VD_ExtractVec3( vdecl->m_info, 3, verts, nrm, VDECLUSAGE_NORMAL );
		
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
			dvs[i].color = 0xffffffff;
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

template< class IdxType > void SGRX_IMesh_Clip_Core( SGRX_IMesh* mesh, const Mat4& mtx, const Mat4& vpmtx, bool decal, float inv_zn2zf, ByteArray& outverts, size_t fp, size_t ep )
{
	size_t stride = mesh->m_vertexDecl.GetInfo().size;
	SGRX_CAST( IdxType*, indices, mesh->m_idata.data() );
	if( ( mesh->m_dataFlags & MDF_TRIANGLESTRIP ) == 0 )
	{
		for( size_t part_id = fp; part_id < ep; ++part_id )
		{
			SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
			for( uint32_t tri = MP.indexOffset, triend = MP.indexOffset + MP.indexCount; tri < triend; tri += 3 )
			{
				SGRX_IMesh_Clip_Core_ClipTriangle( mtx, vpmtx, outverts, mesh->m_vertexDecl, decal, inv_zn2zf
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
			for( uint32_t tri = MP.indexOffset + 2, triend = MP.indexOffset + MP.indexCount; tri < triend; ++tri )
			{
				uint32_t i1 = tri, i2 = tri + 1 + tri % 2, i3 = tri + 2 - tri % 2;
				SGRX_IMesh_Clip_Core_ClipTriangle( mtx, vpmtx, outverts, mesh->m_vertexDecl, decal, inv_zn2zf
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ i1 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ i2 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ i3 ] ) * stride ]
				);
			}
		}
	}
}

void SGRX_IMesh::Clip( const Mat4& mtx, const Mat4& vpmtx, ByteArray& outverts, bool decal, float inv_zn2zf, size_t firstPart, size_t numParts )
{
	if( m_vdata.size() == 0 || m_idata.size() == 0 )
		return;
	
	size_t MPC = m_meshParts.size();
	firstPart = TMIN( firstPart, MPC - 1 );
	size_t oneOverLastPart = TMIN( firstPart + TMIN( numParts, MPC ), MPC );
	if( ( m_dataFlags & MDF_INDEX_32 ) != 0 )
	{
		SGRX_IMesh_Clip_Core< uint32_t >( this, mtx, vpmtx, decal, inv_zn2zf, outverts, firstPart, oneOverLastPart );
	}
	else
	{
		SGRX_IMesh_Clip_Core< uint16_t >( this, mtx, vpmtx, decal, inv_zn2zf, outverts, firstPart, oneOverLastPart );
	}
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

bool SGRX_Camera::GetCursorRay( float x, float y, Vec3& pos, Vec3& dir )
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
	_mibuf_begin( NULL ),
	_mibuf_end( NULL )
{
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


SGRX_CullScene::~SGRX_CullScene()
{
}


SGRX_MeshInstance::SGRX_MeshInstance( SGRX_Scene* s ) :
	_scene( s ),
	raycastOverride( NULL ),
	color( Vec4::Create( 1 ) ),
	layers( 0x1 ),
	enabled( true ),
	cpuskin( false ),
	dynamic( false ),
	decal( false ),
	transparent( false ),
	unlit( false ),
//	additive( false ),
	_lightbuf_begin( NULL ),
	_lightbuf_end( NULL )
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
	SGRX_CAST( SGRX_MeshInstance*, MI, data );
	
	SGRX_IMesh* M = MI->mesh;
	if( MI->transparent == false && M )
	{
		size_t vertoff = outVertices->size();
		M->Clip( MI->matrix, viewProjMatrix, *outVertices, true, invZNearToZFar );
		SGRX_DoIndexTriangleMeshVertices( *outIndices, *outVertices, vertoff, 48 );
	}
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



SGRX_Scene::SGRX_Scene() :
	cullScene( NULL ),
	fogColor( Vec3::Create( 0.5 ) ),
	fogHeightFactor( 0 ),
	fogDensity( 0.01f ),
	fogHeightDensity( 0 ),
	fogStartHeight( 0.01f ),
	fogMinDist( 0 ),
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
}

SGRX_Scene::~SGRX_Scene()
{
	LOG << "Deleted scene: " << this;
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
		if( mi->mesh && ( mi->layers & layers ) && mi->matrix.InvertTo( inv ) )
		{
			Vec3 tffrom = inv.TransformPos( from );
			Vec3 tfto = inv.TransformPos( to );
			if( SegmentAABBIntersect( tffrom, tfto, mi->mesh->m_boundsMin, mi->mesh->m_boundsMax ) )
			{
				IMeshRaycast* mrc = mi->raycastOverride ? mi->raycastOverride : mi->mesh;
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


static float _compare_tris( const Vec3& a0, const Vec3& a1, const Vec3& a2, const Vec3& b0, const Vec3& b1, const Vec3& b2 )
{
	float la0 = ( a1 - a0 ).Length();
	float la1 = ( a2 - a1 ).Length();
	float la2 = ( a0 - a2 ).Length();
	float lb0 = ( b1 - b0 ).Length();
	float lb1 = ( b2 - b1 ).Length();
	float lb2 = ( b0 - b2 ).Length();
	// bigger q = less useful triangle
	float qa = TMAX( TMAX( la0, la1 ), la2 ) - TMIN( TMIN( la0, la1 ), la2 );
	float qb = TMAX( TMAX( lb0, lb1 ), lb2 ) - TMIN( TMIN( lb0, lb1 ), lb2 );
	return qa - qb; // if > 0 (qa>qb), second triangle is better
}

static int uint32_sort_desc( const void* a, const void* b )
{
	uint32_t ua = * (const uint32_t*) a;
	uint32_t ub = * (const uint32_t*) b;
	if( ua < ub ) return 1;
	if( ub < ua ) return -1;
	return 0;
}

void LightTree::InsertSamples( const Sample* samples, size_t count )
{
	Array< int32_t > tris;
	
#ifdef VERSION_0
	// inserted samples
	for( size_t i = 0; i < count; ++i )
	{
		const Sample& S = samples[i];
		
		size_t v2 = m_samples.size();
		for( size_t v0 = 0; v0 < v2; ++v0 )
			if( S.pos == m_samples[ v0 ].pos )
				goto bad_sample;
		m_samples.push_back( S );
		
		// v2 not size because we don't want to include the new sample
		for( size_t v0 = 0; v0 < v2; ++v0 )
		{
			for( size_t v1 = v0 + 1; v1 < v2; ++v1 )
			{
				Vec3 tp0 = m_samples[ v0 ].pos, tp1 = m_samples[ v1 ].pos, tp2 = S.pos;
				if( Vec3Cross( tp1 - tp0, tp2 - tp0 ).Length() < 0.1f )
					continue; // triangle area too small to do anything numerically stable with it, must have a better arrangement
				
				// check against all inserted triangles
				tris.clear();
				for( size_t t = 0; t < m_tris.size(); t += 3 )
				{
					Vec3 itp0 = m_samples[ m_tris[t] ].pos,
					     itp1 = m_samples[ m_tris[t+1] ].pos,
					     itp2 = m_samples[ m_tris[t+2] ].pos;
					
				//	int num_common_verts = 0;
				//	if( v0 == m_tris[t+0] ) num_common_verts++;
				//	if( v0 == m_tris[t+1] ) num_common_verts++;
				//	if( v0 == m_tris[t+2] ) num_common_verts++;
				//	if( v1 == m_tris[t+0] ) num_common_verts++;
				//	if( v1 == m_tris[t+1] ) num_common_verts++;
				//	if( v1 == m_tris[t+2] ) num_common_verts++;
				//	if( v2 == m_tris[t+0] ) num_common_verts++;
				//	if( v2 == m_tris[t+1] ) num_common_verts++;
				//	if( v2 == m_tris[t+2] ) num_common_verts++;
					
					if( !TriangleIntersect( tp0, tp1, tp2, itp0, itp1, itp2 ) )
						continue;
					
					float comparetris = _compare_tris( tp0, tp1, tp2, itp0, itp1, itp2 );
					if( comparetris >= 0 )
					{
						// second triangle is better, discard this
						goto bad_tri;
					}
					else
					{
						// triangles to be removed if this one is good
						tris.push_back( t );
					}
				}
				
				if( tris.size() )
				{
					goto bad_tri;
					qsort( tris.data(), tris.size(), sizeof(int32_t), uint32_sort_desc );
					for( size_t t = 0; t < tris.size(); ++t )
					{
						m_tris.erase( tris[ t ], 3 );
					}
				}
				
				// avoiding goto warning, compiler does not see that nothing uses the variable after label
				{
					int32_t nidcs[] = { v0, v1, v2 };
					m_tris.append( nidcs, 3 );
				}
				
				// done with the triangle
			bad_tri:;
			}
		}
bad_sample:;
	}
#else
	Array< uint32_t > samples_todo;
	for( size_t i = 0; i < count; ++i )
	{
		samples_todo.push_back( m_samples.size() );
		m_samples.push_back( samples[ i ] );
	}
	// inserted samples
	while( samples_todo.size() )
	{
		size_t v2 = samples_todo.last();
		const Sample& S = m_samples[ v2 ];
		samples_todo.pop_back();
		
		for( size_t v0 = 0; v0 < m_samples.size(); ++v0 )
		{
			for( size_t v1 = v0 + 1; v1 < m_samples.size(); ++v1 )
			{
				Vec3 tp0 = m_samples[ v0 ].pos, tp1 = m_samples[ v1 ].pos, tp2 = S.pos;
				if( Vec3Cross( tp1 - tp0, tp2 - tp0 ).Length() < 0.1f )
					continue; // triangle area too small to do anything numerically stable with it, must have a better arrangement
				
				// check against all inserted triangles
				tris.clear();
				for( size_t t = 0; t < m_tris.size(); t += 3 )
				{
					Vec3 itp0 = m_samples[ m_tris[t] ].pos,
					     itp1 = m_samples[ m_tris[t+1] ].pos,
					     itp2 = m_samples[ m_tris[t+2] ].pos;
					
				//	int num_common_verts = 0;
				//	if( v0 == m_tris[t+0] ) num_common_verts++;
				//	if( v0 == m_tris[t+1] ) num_common_verts++;
				//	if( v0 == m_tris[t+2] ) num_common_verts++;
				//	if( v1 == m_tris[t+0] ) num_common_verts++;
				//	if( v1 == m_tris[t+1] ) num_common_verts++;
				//	if( v1 == m_tris[t+2] ) num_common_verts++;
				//	if( v2 == m_tris[t+0] ) num_common_verts++;
				//	if( v2 == m_tris[t+1] ) num_common_verts++;
				//	if( v2 == m_tris[t+2] ) num_common_verts++;
					
					if( !TriangleIntersect( tp0, tp1, tp2, itp0, itp1, itp2 ) )
						continue;
					
					float comparetris = _compare_tris( tp0, tp1, tp2, itp0, itp1, itp2 );
					if( comparetris >= 0 )
					{
						// second triangle is better, discard this
						goto bad_tri;
					}
					else
					{
						// triangles to be removed if this one is good
						tris.push_back( t );
					}
				}
				
				if( tris.size() )
				{
					qsort( tris.data(), tris.size(), sizeof(int32_t), uint32_sort_desc );
					for( size_t t = 0; t < tris.size(); ++t )
					{
						size_t tri = tris[ t ];
						samples_todo.push_back( m_tris[tri] );
						samples_todo.push_back( m_tris[tri+1] );
						samples_todo.push_back( m_tris[tri+2] );
						m_tris.erase( tri, 3 );
					}
				}
				
				// avoiding goto warning, compiler does not see that nothing uses the variable after label
				{
					int32_t nidcs[] = { (int32_t) v0, (int32_t) v1, (int32_t) v2 };
					m_tris.append( nidcs, 3 );
				}
				
				// done with the triangle
			bad_tri:;
			}
		}
	}
#endif
	
	// calculate adjancency
	m_triadj.clear();
	m_adjdata.clear();
	
	int32_t nulltriadj[] = { 0, 0 };
	for( size_t t0 = 0; t0 < m_tris.size(); t0 += 3 )
		m_triadj.append( nulltriadj, 2 );
	
	int32_t tricount = m_tris.size() / 3;
	for( int32_t t0 = 0; t0 < tricount; ++t0 )
	{
		for( int32_t t1 = t0 + 1; t1 < tricount; ++t1 )
		{
			int num_common_verts = 0;
			int32_t t0at = t0 * 3;
			int32_t t1at = t1 * 3;
			////////////
		//	for( int i = 0; i < 3; ++i )
		//		for( int j = 0; j < 3; ++j )
		//			if( m_tris[ t0at + i ] == m_tris[ t1at + j ] )
		//				num_common_verts++;
			;///////////
			// UNROLL //
			if( m_tris[ t0at + 0 ] == m_tris[ t1at + 0 ] ) num_common_verts++;
			if( m_tris[ t0at + 0 ] == m_tris[ t1at + 1 ] ) num_common_verts++;
			if( m_tris[ t0at + 0 ] == m_tris[ t1at + 2 ] ) num_common_verts++;
			if( m_tris[ t0at + 1 ] == m_tris[ t1at + 0 ] ) num_common_verts++;
			if( m_tris[ t0at + 1 ] == m_tris[ t1at + 1 ] ) num_common_verts++;
			if( m_tris[ t0at + 1 ] == m_tris[ t1at + 2 ] ) num_common_verts++;
			if( m_tris[ t0at + 2 ] == m_tris[ t1at + 0 ] ) num_common_verts++;
			if( m_tris[ t0at + 2 ] == m_tris[ t1at + 1 ] ) num_common_verts++;
			if( m_tris[ t0at + 2 ] == m_tris[ t1at + 2 ] ) num_common_verts++;
			////////////
			
		//	LOG << num_common_verts;
			if( num_common_verts >= 3 )
			{
				LOG << "ERROR LIGHTTREE cv=" <<num_common_verts<<"|"<<m_tris[t0at+0]<<","<<m_tris[t0at+1]<<","<<m_tris[t0at+2]<<"|"<<m_tris[t1at+0]<<","<<m_tris[t1at+1]<<","<<m_tris[t1at+2];
			}
			if( num_common_verts < 1 )
				continue;
			
			//// insert adjancency data for triangle 0 ////
			m_triadj[ t0 * 2 + 1 ]++; // update own size
			m_adjdata.insert( m_triadj[ t0 * 2 ], t1 );
			for( size_t a = t0 * 2 + 2; a < m_triadj.size(); a += 2 )
				m_triadj[ a ]++; // update following offsets
			
			//// insert adjancency data for triangle 1 ////
			m_triadj[ t1 * 2 + 1 ]++; // update own size
			m_adjdata.insert( m_triadj[ t1 * 2 ], t0 );
			for( size_t a = t1 * 2 + 2; a < m_triadj.size(); a += 2 )
				m_triadj[ a ]++; // update following offsets
		}
	}
	
//	LOG << m_tris;
//	LOG << m_triadj;
//	LOG << m_adjdata;
}

static inline Vec3 _make_plane( const Vec3& p1, const Vec3& p2, const Vec3& p3 )
{
	return Vec3Cross( p2 - p1, p3 - p1 ).Normalized();
	// return V4( cp.x, cp.y, cp.z, Vec3Dot( cp, p1 ) );
}

static inline float _clamped_dist( const Vec3& plane, const Vec3& midpt, const Vec3& p0, const Vec3& p1 )
{
	float qm = Vec3Dot( plane, midpt );
	float q0 = Vec3Dot( plane, p0 );
	float q1 = Vec3Dot( plane, p1 );
//	LOG << "qm = " << qm << ", q0 = " << q0 << ", q1 = " << q1;
	if( ( q0 <= q1 && qm < q0 ) || ( q0 >= q1 && qm > q0 ) ) return 0;
	if( ( q1 <= q0 && qm < q1 ) || ( q1 >= q0 && qm > q1 ) ) return 1;
	if( q0 == q1 ) return 0;
	return ( qm - q0 ) / ( q1 - q0 );
}

static inline void _interpolate_s2( LightTree::Sample& out, const LightTree::Sample& p1, const LightTree::Sample& p2 )
{
	Vec3 dir = ( p2.pos - p1.pos ).Normalized();
	float q = _clamped_dist( dir, out.pos, p1.pos, p2.pos );
	float iq = 1 - q;
	for( int i = 0; i < 6; ++i )
	{
		out.color[ i ] = p1.color[ i ] * iq + p2.color[ i ] * q;
	}
}

static inline void _interpolate_s3( LightTree::Sample& out, const LightTree::Sample& p1, const LightTree::Sample& p2, const LightTree::Sample& p3 )
{
//	LOG << p1.pos << p2.pos << p3.pos << "|" << out.pos;
	Vec3 normal = _make_plane( p1.pos, p2.pos, p3.pos );
	Vec3 en1 = Vec3Cross( normal, p3.pos - p2.pos ).Normalized();
	Vec3 en2 = Vec3Cross( normal, p1.pos - p3.pos ).Normalized();
	Vec3 en3 = Vec3Cross( normal, p2.pos - p1.pos ).Normalized();
//	LOG << en1 << en2 << en3;
	float q1 = _clamped_dist( en1, out.pos, p3.pos, p1.pos );
	float q2 = _clamped_dist( en2, out.pos, p1.pos, p2.pos );
	float q3 = _clamped_dist( en3, out.pos, p2.pos, p3.pos );
//	LOG << q1 << "|" << q2 << "|" << q3;
	for( int i = 0; i < 6; ++i )
	{
		out.color[ i ] = p1.color[ i ] * q1 + p2.color[ i ] * q2 + p3.color[ i ] * q3;
	}
}

static inline void _interpolate_s4( LightTree::Sample& out, const LightTree::Sample& p1, const LightTree::Sample& p2, const LightTree::Sample& p3, const LightTree::Sample& p4 )
{
//	LOG << p1.pos << p2.pos << p3.pos << p4.pos;
	Vec3 plane1 = _make_plane( p2.pos, p3.pos, p4.pos );
	Vec3 plane2 = _make_plane( p1.pos, p3.pos, p4.pos );
	Vec3 plane3 = _make_plane( p1.pos, p2.pos, p4.pos );
	Vec3 plane4 = _make_plane( p1.pos, p2.pos, p3.pos );
	float q1 = _clamped_dist( plane1, out.pos, p2.pos, p1.pos );
	float q2 = _clamped_dist( plane2, out.pos, p3.pos, p2.pos );
	float q3 = _clamped_dist( plane3, out.pos, p4.pos, p3.pos );
	float q4 = _clamped_dist( plane4, out.pos, p1.pos, p4.pos );
//	LOG << q1 << "|" << q2 << "|" << q3 << "|" << q4;
	for( int i = 0; i < 6; ++i )
	{
		out.color[ i ] = p1.color[ i ] * q1 + p2.color[ i ] * q2 + p3.color[ i ] * q3 + p4.color[ i ] * q4;
	}
}

void LightTree::Interpolate( Sample& S, int32_t* outlastfound )
{
	if( m_samples.size() > 4 )
	{
		int32_t prevtri = -1;
		int32_t tri = 0;
		float min_tri_dist = PointTriangleDistance( S.pos, m_samples[m_tris[tri*3+0]].pos, m_samples[m_tris[tri*3+1]].pos, m_samples[m_tris[tri*3+2]].pos );
		bool found = true;
		
		// find closest triangle
		while( min_tri_dist > 0 && found )
		{
			// check adjacent triangles
			int32_t adjstart = m_triadj[ tri * 2 ];
			int32_t adjend = adjstart + m_triadj[ tri * 2 + 1 ];
		//	LOG << "A" << adjend - adjstart;
			found = false;
			for( int32_t a = adjstart; a < adjend; ++a )
			{
				int32_t adjtri = m_adjdata[ a ];
				float new_tri_dist = PointTriangleDistance( S.pos, m_samples[m_tris[adjtri*3+0]].pos, m_samples[m_tris[adjtri*3+1]].pos, m_samples[m_tris[adjtri*3+2]].pos );
		//		LOG << "ND" << new_tri_dist << " VS " << min_tri_dist;
				if( new_tri_dist < min_tri_dist )
				{
					tri = adjtri;
					min_tri_dist = new_tri_dist;
					found = true;
				}
			}
		}
		
		if( outlastfound )
			*outlastfound = tri;
		
		// point on triangle, interpolate
	//	if( min_tri_dist < SMALL_FLOAT )
		{
			_interpolate_s3( S, m_samples[m_tris[tri*3+0]], m_samples[m_tris[tri*3+1]], m_samples[m_tris[tri*3+2]] );
			return;
		}
		
		// TODO TODO TODO
		
		// find next closest triangle for last point
		if( prevtri == -1 )
		{
		}
		
#if 0
		// try to reduce invalid sample sets
		if( numsamples == 4 )
		{
			Vec3 dir = _make_plane( samples[0].pos, samples[1].pos, samples[2].pos );
			float q0 = Vec3Dot( dir, samples[0].pos );
			float q1 = Vec3Dot( dir, samples[1].pos );
			float q2 = Vec3Dot( dir, samples[2].pos );
			float q3 = Vec3Dot( dir, samples[3].pos );
			if( q0 == q1 && q1 == q2 && q2 == q3 )
				numsamples--; // all points are on the same plane
		}
		if( numsamples == 3 )
		{
			Vec3 dir = ( samples[1].pos - samples[0].pos ).Normalized();
			Vec3 dir2 = ( samples[2].pos - samples[1].pos ).Normalized();
			if( fabsf( Vec3Dot( dir, dir2 ) ) > 1 - SMALL_FLOAT )
				numsamples--; // all points are on the same line
		}
		
		// interpolate samples
		if( numsamples == 1 ){ S = samples[0]; return; }
		if( numsamples == 2 ){ _interpolate_s2( S, samples[0], samples[1] ); return; }
		if( numsamples == 3 ){ _interpolate_s3( S, samples[0], samples[1], samples[2] ); return; }
		if( numsamples == 4 ){ _interpolate_s4( S, samples[0], samples[1], samples[2], samples[3] ); return; }
#endif
	}
	
	// interpolation-less output
	if( m_samples.size() == 1 ){ S = m_samples[0]; return; }
	if( m_samples.size() == 2 ){ _interpolate_s2( S, m_samples[0], m_samples[1] ); return; }
	if( m_samples.size() == 3 ){ _interpolate_s3( S, m_samples[0], m_samples[1], m_samples[2] ); return; }
	if( m_samples.size() == 4 ){ _interpolate_s4( S, m_samples[0], m_samples[1], m_samples[2], m_samples[3] ); return; }
	for( int i = 0; i < 6; ++i )
		S.color[ i ] = V3(0);
}


int GR_GetWidth(){ return g_RenderSettings.width; }
int GR_GetHeight(){ return g_RenderSettings.height; }


TextureHandle GR_CreateTexture( int width, int height, int format, int mips )
{
	TextureInfo ti = { 0, TEXTYPE_2D, width, height, 1, format, mips };
	SGRX_ITexture* tex = g_Renderer->CreateTexture( &ti, NULL );
	if( !tex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	tex->m_refcount = 0;
	tex->m_info = ti;
	
	LOG << "Created 2D texture: " << width << "x" << height << ", format=" << format << ", mips=" << mips;
	return tex;
}

TextureHandle GR_GetTexture( const StringView& path )
{
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
	tex->m_refcount = 0;
	tex->m_key.append( path.data(), path.size() );
	g_Textures->set( tex->m_key, tex );
	
	LOG << "Loaded texture: " << path;
	return tex;
}

TextureHandle GR_CreateRenderTexture( int width, int height, int format )
{
	TextureInfo ti = { 0, TEXTYPE_2D, width, height, 1, format, 1 };
	SGRX_ITexture* tex = g_Renderer->CreateRenderTexture( &ti );
	if( !tex )
	{
		// error is already printed
		return TextureHandle();
	}
	
	tex->m_refcount = 0;
	tex->m_info = ti;
	
	LOG << "Created renderable texture: " << width << "x" << height << ", format=" << format;
	return tex;
}


VertexShaderHandle GR_GetVertexShader( const StringView& path )
{
	String code;
	String errors;
	ByteArray comp;
	
	SGRX_IVertexShader* shd = g_VertexShaders->getcopy( path );
	if( shd )
		return shd;
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( g_Game->GetCompiledShader( g_Renderer->GetInfo().shaderTarget, ".vs", path, comp ) )
		{
			goto has_compiled_shader;
		}
	}
	
	if( !g_Game->OnLoadShader( g_Renderer->GetInfo().shaderTarget, path, code ) )
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
		
		g_Game->SetCompiledShader( g_Renderer->GetInfo().shaderTarget, ".vs", path, comp );
		
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
	shd->m_refcount = 0;
	g_VertexShaders->set( shd->m_key, shd );
	
	LOG << "Loaded vertex shader: " << path;
	return shd;
}

PixelShaderHandle GR_GetPixelShader( const StringView& path )
{
	String code;
	String errors;
	ByteArray comp;
	
	SGRX_IPixelShader* shd = g_PixelShaders->getcopy( path );
	if( shd )
		return shd;
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( g_Game->GetCompiledShader( g_Renderer->GetInfo().shaderTarget, ".ps", path, comp ) )
		{
			goto has_compiled_shader;
		}
	}
	
	if( !g_Game->OnLoadShader( g_Renderer->GetInfo().shaderTarget, path, code ) )
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
		
		g_Game->SetCompiledShader( g_Renderer->GetInfo().shaderTarget, ".ps", path, comp );
		
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
	shd->m_refcount = 0;
	g_PixelShaders->set( shd->m_key, shd );
	
	LOG << "Loaded pixel shader: " << path;
	return shd;
}

SurfaceShaderHandle GR_GetSurfaceShader( const StringView& name )
{
	SGRX_SurfaceShader* ssh = g_SurfShaders->getcopy( name );
	if( ssh )
		return ssh;
	
	ssh = new SGRX_SurfaceShader;
	ssh->m_refcount = 0;
	ssh->m_key = name;
	ssh->ReloadShaders();
	g_SurfShaders->set( ssh->m_key, ssh );
	
	LOG << "Created surface shader: " << name;
	return ssh;
}


MaterialHandle GR_CreateMaterial()
{
	SGRX_Material* mtl = new SGRX_Material;
	return mtl;
}


VertexDeclHandle GR_GetVertexDecl( const StringView& vdecl )
{
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
	VD->m_refcount = 0;
	g_VertexDecls->set( VD->m_key, VD );
	
	LOG << "Created vertex declaration: " << vdecl;
	return VD;
}


MeshHandle GR_CreateMesh()
{
	SGRX_IMesh* mesh = g_Renderer->CreateMesh();
	return mesh;
}

MeshHandle GR_GetMesh( const StringView& path )
{
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
	
	SGRX_MeshBone bones[ MAX_MESH_BONES ];
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
		// error already printed
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
		parts[ i ].vertexOffset = mfd.parts[ i ].vertexOffset;
		parts[ i ].vertexCount = mfd.parts[ i ].vertexCount;
		parts[ i ].indexOffset = mfd.parts[ i ].indexOffset;
		parts[ i ].indexCount = mfd.parts[ i ].indexCount;
		
		StringView mtltext( mfd.parts[ i ].materialStrings[0], mfd.parts[ i ].materialStringSizes[0] );
		
		// LOAD MATERIAL
		//
		MaterialHandle mh = GR_CreateMaterial();
		if( mfd.parts[ i ].materialStringSizes[0] >= SHADER_NAME_LENGTH )
		{
			LOG_WARNING << "Shader name for part " << i << " is too long";
		}
		else
		{
			mh->shader = GR_GetSurfaceShader( mtltext );
		}
		for( int tid = 0; tid < mfd.parts[ i ].materialTextureCount; ++tid )
		{
			mh->textures[ tid ] = GR_GetTexture( StringView( mfd.parts[ i ].materialStrings[ tid + 1 ], mfd.parts[ i ].materialStringSizes[ tid + 1 ] ) );
		}
		parts[ i ].material = mh;
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
	assert( anim >= 0 && anim < (int) afp->animData.size() );
	
	const AnimFileParser::Anim& AN = afp->animData[ anim ];
	
	SGRX_Animation* nanim = new SGRX_Animation;
	nanim->_refcount = 0;
	
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
	
	return nanim;
}

int GR_LoadAnims( const StringView& path, const StringView& prefix )
{
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



SceneHandle GR_CreateScene()
{
	SGRX_Scene* scene = new SGRX_Scene;
	
	LOG << "Created scene";
	return scene;
}

bool GR_SetRenderPasses( SGRX_RenderPass* passes, int count )
{
	if( g_Renderer->SetRenderPasses( passes, count ) )
	{
		for( size_t i = 0; i < g_SurfShaders->size(); ++i )
			g_SurfShaders->item( i ).value->ReloadShaders();
		return true;
	}
	return false;
}

void GR_RenderScene( SGRX_RenderScene& info )
{
	g_BatchRenderer->Flush();
	g_BatchRenderer->Reset();
	g_Renderer->RenderScene( &info );
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

void GR2D_SetScissorRect( int x0, int y0, int x1, int y1 )
{
	g_BatchRenderer->Flush();
	int rect[4] = { x0, y0, x1, y1 };
	g_Renderer->SetScissorRect( true, rect );
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

void GR2D_UnsetScissorRect()
{
	g_BatchRenderer->Flush();
	g_Renderer->SetScissorRect( false, NULL );
}


void GR2D_SetColor( float r, float g, float b, float a )
{
	g_BatchRenderer->Col( r, g, b, a );
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
	SGRX_IFont* fnt = g_FontRenderer->m_currentFont;
	settings->font = fnt ? fnt->m_key : "";
	settings->size = g_FontRenderer->m_currentSize;
	settings->letterspacing = 0;
}

void GR2D_SetFontSettings( SGRX_FontSettings* settings )
{
	GR2D_SetFont( settings->font, settings->size );
}

bool GR2D_SetFont( const StringView& name, int pxsize )
{
	return g_FontRenderer->SetFont( name, pxsize );
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

BatchRenderer& GR2D_GetBatchRenderer()
{
	return *g_BatchRenderer;
}


//
// RENDERING
//

BatchRenderer::BatchRenderer( struct IRenderer* r ) : m_renderer( r ), m_diff( false )
{
	m_swapRB = r->GetInfo().swapRB;
	m_proto.x = 0;
	m_proto.y = 0;
	m_proto.z = 0;
	m_proto.u = 0;
	m_proto.v = 0;
	m_proto.color = 0xffffffff;
	worldMatrix = Mat4::Identity;
	viewMatrix = Mat4::Identity;
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
	if( m_diff )
	{
		Flush();
		m_currState = m_nextState;
		m_diff = false;
	}
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

BatchRenderer& BatchRenderer::Prev( int i )
{
	if( i < 0 || i >= (int) m_verts.size() )
		AddVertex( m_proto );
	else
	{
		Vertex v = m_verts[ m_verts.size() - 1 - i ];
		AddVertex( v );
	}
	return *this;
}

BatchRenderer& BatchRenderer::RawQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float z )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x0, y0, z );
	Tex( 1, 0 ); Pos( x1, y1, z );
	Tex( 1, 1 ); Pos( x2, y2, z );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x3, y3, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::Quad( float x0, float y0, float x1, float y1, float z )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x0, y0, z );
	Tex( 1, 0 ); Pos( x1, y0, z );
	Tex( 1, 1 ); Pos( x1, y1, z );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x0, y1, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::QuadFrame( float x0, float y0, float x1, float y1, float ix0, float iy0, float ix1, float iy1, float z )
{
	RawQuad( x0, y0, x1, y0, ix1, iy0, ix0, iy0, z );
	RawQuad( x1, y0, x1, y1, ix1, iy1, ix1, iy0, z );
	RawQuad( x1, y1, x0, y1, ix0, iy1, ix1, iy1, z );
	RawQuad( x0, y1, x0, y0, ix0, iy0, ix0, iy1, z );
	return *this;
}

BatchRenderer& BatchRenderer::TurnedBox( float x, float y, float dx, float dy, float z )
{
	float tx = -dy;
	float ty = dx;
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x - dx - tx, y - dy - ty, z );
	Tex( 1, 0 ); Pos( x - dx + tx, y - dy + ty, z );
	Tex( 1, 1 ); Pos( x + dx + tx, y + dy + ty, z );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x + dx - tx, y + dy - ty, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::Poly( const void* data, int count, float z, int stride )
{
	SGRX_CAST( const uint8_t*, bp, data );
	SetPrimitiveType( PT_TriangleStrip );
	for( int i = 0; i < count; ++i )
	{
		int v;
		if( i % 2 == 0 )
			v = i / 2;
		else
			v = count - 1 - i / 2;
		Pos( *(float*)(bp+v*stride), *(float*)(bp+v*stride+4), stride >= 12 ? *(float*)(bp+i*stride+8) : z );
	}
	return *this;
}

BatchRenderer& BatchRenderer::PolyOutline( const void* data, int count, float z, int stride )
{
	SGRX_CAST( const uint8_t*, bp, data );
	SetPrimitiveType( PT_LineStrip );
	for( int i = 0; i < count; ++i )
		Pos( *(float*)(bp+i*stride), *(float*)(bp+i*stride+4), stride >= 12 ? *(float*)(bp+i*stride+8) : z );
	Prev( count - 1 );
	return *this;
}

BatchRenderer& BatchRenderer::Sprite( const Vec3& pos, const Vec3& dx, const Vec3& dy )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( pos + dx + dy );
	Tex( 1, 0 ); Pos( pos - dx + dy );
	Tex( 1, 1 ); Pos( pos - dx - dy );
	Prev( 0 );
	Tex( 0, 1 ); Pos( pos + dx - dy );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::Sprite( const Vec3& pos, float sx, float sy )
{
	Vec3 dx = invMatrix.TransformNormal( V3( 1, 0, 0 ) ).Normalized() * sx;
	Vec3 dy = invMatrix.TransformNormal( V3( 0, 1, 0 ) ).Normalized() * sy;
	return Sprite( pos, dx, dy );
}

BatchRenderer& BatchRenderer::TexLine( const Vec2& p0, const Vec2& p1, float rad )
{
	SetPrimitiveType( PT_Triangles );
	Vec2 D = ( p1 - p0 ).Normalized();
	Vec2 T = D.Perp();
	
	Tex( 0   , 0 ); Pos( p0 + ( -D +T ) * rad );
	Tex( 0.5f, 0 ); Pos( p0 + (    +T ) * rad );
	Tex( 0.5f, 1 ); Pos( p0 + (    -T ) * rad );
	Prev( 0 ); // #4 requesting #3
	Tex( 0   , 1 ); Pos( p0 + ( -D -T ) * rad );
	Prev( 4 ); // #6 requesting #1
	
	Prev( 4 ); // #7 requesting #2
	Tex( 0.5f, 0 ); Pos( p1 + (    +T ) * rad );
	Tex( 0.5f, 1 ); Pos( p1 + (    -T ) * rad );
	Prev( 0 ); // #10 requesting #9
	Prev( 7 ); // #11 requesting #3
	Prev( 9 ); // #12 requesting #2
	
	Prev( 4 ); // #13 requesting #8
	Tex( 1   , 0 ); Pos( p1 + ( +D +T ) * rad );
	Tex( 1   , 1 ); Pos( p1 + ( +D -T ) * rad );
	Prev( 0 ); // #16 requesting #15
	Prev( 7 ); // #17 requesting #9
	Prev( 9 ); // #18 requesting #8
	
	return *this;
}

BatchRenderer& BatchRenderer::CircleFill( float x, float y, float r, float z, int verts )
{
	if( verts < 0 )
		verts = r * M_PI * 2;
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_Triangles );
		float ad = M_PI * 2.0f / verts;
		float a = ad, ps = 0, pc = 1;
		for( int i = 0; i < verts; ++i )
		{
			float cs = sin( a ), cc = cos( a );
			Pos( x + ps * r, y + pc * r, z );
			Pos( x + cs * r, y + cc * r, z );
			Pos( x, y, z );
			pc = cc;
			ps = cs;
			a += ad;
		}
	}
	return *this;
}

BatchRenderer& BatchRenderer::CircleOutline( float x, float y, float r, float z, int verts )
{
	if( verts < 0 )
		verts = r * M_PI * 2;
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_LineStrip );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			Pos( x + sin( a ) * r, y + cos( a ) * r, z );
			a += ad;
		}
		Prev( verts - 1 );
	}
	return *this;
}

BatchRenderer& BatchRenderer::CircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts )
{
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			if( i > 1 )
				Prev( 0 );
			Pos( pos + sin( a ) * dx + cos( a ) * dy );
			a += ad;
		}
		Prev( 0 );
		Prev( verts * 2 - 2 );
	}
	return *this;
}

BatchRenderer& BatchRenderer::HalfCircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts )
{
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI / ( verts - 1 );
		for( int i = 0; i < verts; ++i )
		{
			if( i > 1 )
				Prev( 0 );
			Pos( pos + sin( a ) * dy + cos( a ) * dx );
			a += ad;
		}
	}
	return *this;
}

BatchRenderer& BatchRenderer::SphereOutline( const Vec3& pos, float radius, int verts )
{
	CircleOutline( pos, V3(radius,0,0), V3(0,radius,0), verts );
	CircleOutline( pos, V3(0,radius,0), V3(0,0,radius), verts );
	CircleOutline( pos, V3(0,0,radius), V3(radius,0,0), verts );
	return *this;
}

BatchRenderer& BatchRenderer::CapsuleOutline( const Vec3& pos, float radius, const Vec3& nrm, float ht, int verts )
{
	Vec3 N = nrm.Normalized();
	if( N.LengthSq() < SMALL_FLOAT )
		return SphereOutline( pos, radius, verts );
	
	Vec3 refdir = V3( 0, 0, nrm.z >= 0 ? 1 : -1 );
	Vec3 rotaxis = Vec3Cross( N, refdir );
	float rotangle = acosf( clamp( Vec3Dot( N, refdir ), -1, 1 ) );
	
	Mat4 rot = Mat4::CreateRotationAxisAngle( rotaxis, rotangle );
	Vec3 T = rot.TransformNormal( V3(1,0,0) );
	Vec3 B = rot.TransformNormal( V3(0,1,0) );
	Vec3 Tr = T * radius, Br = B * radius, Nr = N * radius;
	
	Vec3 p0 = pos - N * ht, p1 = pos + N * ht;
	
	HalfCircleOutline( p1, Tr, Nr, verts / 2 );
	HalfCircleOutline( p1, Br, Nr, verts / 2 );
	HalfCircleOutline( p0, Tr, -Nr, verts / 2 );
	HalfCircleOutline( p0, Br, -Nr, verts / 2 );
	CircleOutline( p0, Tr, Br, verts );
	if( ht )
	{
		CircleOutline( p1, Tr, Br, verts );
		SetPrimitiveType( PT_Lines );
		Pos( p0 - Tr ); Pos( p1 - Tr );
		Pos( p0 + Tr ); Pos( p1 + Tr );
		Pos( p0 - Br ); Pos( p1 - Br );
		Pos( p0 + Br ); Pos( p1 + Br );
	}
	
	return *this;
}

BatchRenderer& BatchRenderer::ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, float angle, int verts )
{
	if( verts >= 3 )
	{
		float hra = DEG2RAD( angle ) / 2.0f;
		float rc = radius * cosf( hra );
		float rs = radius * sinf( hra );
		Vec3 dx = Vec3Cross(dir,up).Normalized() * rs, dy = up.Normalized() * rs;
		
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			Pos( pos );
			Pos( pos + dir * rc + sin( a ) * dx + cos( a ) * dy );
			a += ad;
		}
		CircleOutline( pos + dir * rc, dx, dy, verts );
	}
	return *this;
}

BatchRenderer& BatchRenderer::AABB( const Vec3& bbmin, const Vec3& bbmax, const Mat4& transform )
{
	Vec3 pp[8] =
	{
		transform.TransformPos( V3(bbmin.x,bbmin.y,bbmin.z) ), transform.TransformPos( V3(bbmax.x,bbmin.y,bbmin.z) ),
		transform.TransformPos( V3(bbmin.x,bbmax.y,bbmin.z) ), transform.TransformPos( V3(bbmax.x,bbmax.y,bbmin.z) ),
		transform.TransformPos( V3(bbmin.x,bbmin.y,bbmax.z) ), transform.TransformPos( V3(bbmax.x,bbmin.y,bbmax.z) ),
		transform.TransformPos( V3(bbmin.x,bbmax.y,bbmax.z) ), transform.TransformPos( V3(bbmax.x,bbmax.y,bbmax.z) ),
	};
	SetPrimitiveType( PT_Lines );
	// X
	Pos( pp[0] ); Pos( pp[1] ); Pos( pp[2] ); Pos( pp[3] ); Pos( pp[4] ); Pos( pp[5] ); Pos( pp[6] ); Pos( pp[7] );
	// Y
	Pos( pp[0] ); Pos( pp[2] ); Pos( pp[1] ); Pos( pp[3] ); Pos( pp[4] ); Pos( pp[6] ); Pos( pp[5] ); Pos( pp[7] );
	// Z
	Pos( pp[0] ); Pos( pp[4] ); Pos( pp[1] ); Pos( pp[5] ); Pos( pp[2] ); Pos( pp[6] ); Pos( pp[3] ); Pos( pp[7] );
	return *this;
}

BatchRenderer& BatchRenderer::Tick( const Vec3& pos, float radius, const Mat4& transform )
{
	SetPrimitiveType( PT_Lines );
	Pos( transform.TransformPos( pos + V3(-radius,0,0) ) ); Pos( transform.TransformPos( pos + V3(radius,0,0) ) );
	Pos( transform.TransformPos( pos + V3(0,-radius,0) ) ); Pos( transform.TransformPos( pos + V3(0,radius,0) ) );
	Pos( transform.TransformPos( pos + V3(0,0,-radius) ) ); Pos( transform.TransformPos( pos + V3(0,0,radius) ) );
	return *this;
}

bool BatchRenderer::CheckSetTexture( const TextureHandle& tex )
{
	if( tex != m_nextState.texture )
	{
		m_nextState.texture = tex;
		_UpdateDiff();
		return true;
	}
	_UpdateDiff();
	return false;
}

BatchRenderer& BatchRenderer::SetTexture( const TextureHandle& tex )
{
	CheckSetTexture( tex );
	return *this;
}

BatchRenderer& BatchRenderer::SetShader( const PixelShaderHandle& shd )
{
	m_nextState.shader = shd;
	_UpdateDiff();
	return *this;
}

inline bool _is_noncont_primtype( EPrimitiveType pt )
{
	return pt == PT_LineStrip || pt == PT_TriangleStrip;
}

BatchRenderer& BatchRenderer::SetPrimitiveType( EPrimitiveType pt )
{
	if( _is_noncont_primtype( pt ) || _is_noncont_primtype( m_nextState.primType ) )
		Flush();
	m_nextState.primType = pt;
	_UpdateDiff();
	return *this;
}

BatchRenderer& BatchRenderer::QuadsToTris()
{
	if( m_nextState.primType == PT_Triangles && m_verts.size() % 4 == 0 )
	{
		size_t oldsize = m_verts.size();
		size_t newsize = oldsize / 4 * 6;
		m_verts.resize( newsize );
		for( size_t i = oldsize, j = newsize; i > 0; )
		{
			i -= 4;
			j -= 6;
			Vertex vts[4] = { m_verts[ i ], m_verts[ i + 1 ], m_verts[ i + 2 ], m_verts[ i + 3 ] };
			m_verts[ j+0 ] = vts[0];
			m_verts[ j+1 ] = vts[1];
			m_verts[ j+2 ] = vts[2];
			m_verts[ j+3 ] = vts[2];
			m_verts[ j+4 ] = vts[4];
			m_verts[ j+5 ] = vts[0];
		}
	}
	else
		LOG_ERROR << __FUNCTION__ << " - incorrect vertex count: " << m_verts.size();
	return *this;
}

BatchRenderer& BatchRenderer::Flush()
{
	if( m_verts.size() )
	{
		m_renderer->DrawBatchVertices( m_verts.data(), m_verts.size(), m_currState.primType, m_currState.texture, m_currState.shader, ShaderData.data(), ShaderData.size() );
		m_verts.clear();
	}
	return *this;
}

BatchRenderer& BatchRenderer::Reset()
{
	ShaderData.clear();
	CheckSetTexture( NULL );
	SetShader( NULL );
	SetPrimitiveType( PT_None );
	m_proto.color = 0xffffffff;
	m_proto.u = 0;
	m_proto.v = 0;
	return *this;
}

void BatchRenderer::_UpdateDiff()
{
	m_diff = m_currState.texture != m_nextState.texture
		|| m_currState.shader != m_nextState.shader
		|| m_currState.primType != m_nextState.primType
	;
}

void BatchRenderer::_RecalcMatrices()
{
	invMatrix = Mat4::Identity;
	( worldMatrix * viewMatrix ).InvertTo( invMatrix );
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
	LOG << LOG_DATE << "  Loaded renderer: " << rendername;
	
	g_Textures = new TextureHashTable();
	g_VertexShaders = new VertexShaderHashTable();
	g_PixelShaders = new PixelShaderHashTable();
	g_SurfShaders = new SurfShaderHashTable();
	g_Materials = new MaterialHashTable();
	g_VertexDecls = new VertexDeclHashTable();
	g_Meshes = new MeshHashTable();
	g_Anims = new AnimHashTable();
	g_LoadedFonts = new FontHashTable();
	LOG << LOG_DATE << "  Created renderer resource caches";
	
	g_BatchRenderer = new BatchRenderer( g_Renderer );
	LOG << LOG_DATE << "  Created batch renderer";
	
	sgrx_int_InitializeFontRendering();
	g_FontRenderer = new FontRenderer();
	LOG << LOG_DATE << "  Created font renderer";
	
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
	g_Renderer->UnloadInternalResources();
	
	delete g_FontRenderer;
	g_FontRenderer = NULL;
	sgrx_int_FreeFontRendering();
	
	delete g_BatchRenderer;
	g_BatchRenderer = NULL;
	
	delete g_LoadedFonts;
	g_LoadedFonts = NULL;
	
	delete g_Anims;
	g_Anims = NULL;
	
	delete g_Meshes;
	g_Meshes = NULL;
	
	delete g_VertexDecls;
	g_VertexDecls = NULL;
	
	delete g_Materials;
	g_Materials = NULL;
	
	delete g_SurfShaders;
	g_SurfShaders = NULL;
	
	delete g_PixelShaders;
	g_PixelShaders = NULL;
	
	delete g_VertexShaders;
	g_VertexShaders = NULL;
	
	delete g_Textures;
	g_Textures = NULL;
	
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
		g_ActionMap->Advance();
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
		
		g_Renderer->Swap();
	}
	
	g_Game->OnDestroy();
	Game_RemoveAllOverlayScreens();
	
	free_graphics();
	
	SDL_UnloadObject( g_GameLib );
	
	delete g_ActionMap;
	
	LOG << LOG_DATE << "  Engine finished";
	return 0;
}

