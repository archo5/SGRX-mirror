

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

static String g_GameLibName = "game";
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
	
	if( !LoadBinaryFile( path, outdata ) )
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

void IGame::GetShaderCacheFilename( const StringView& type, const StringView& key, String& name )
{
	name = "shaders_";
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
	
	name.append( ".csh" );
}

bool IGame::GetCompiledShader( const StringView& type, const StringView& key, ByteArray& outdata )
{
	if( !key )
		return false;
	
	String filename;
	GetShaderCacheFilename( type, key, filename );
	
	LOG << "Loading precompiled shader: " << filename << " (type=" << type << ", key=" << key << ")";
	return LoadBinaryFile( filename, outdata );
}

bool IGame::SetCompiledShader( const StringView& type, const StringView& key, const ByteArray& data )
{
	if( !key )
		return false;
	
	String filename;
	GetShaderCacheFilename( type, key, filename );
	
	LOG << "Saving precompiled shader: " << filename << " (type=" << type << ", key=" << key << ")";
	return SaveBinaryFile( filename, data.data(), data.size() );
}

bool IGame::OnLoadShader( const StringView& type, const StringView& key, String& outdata )
{
	if( !key )
		return false;
	
	if( key.part( 0, 4 ) == "mtl:" )
	{
		int i = 0;
		String prepend;
		StringView tpl, mtl, cur, it = key.part( 4 );
		while( it.size() )
		{
			i++;
			cur = it.until( ":" );
			if( i == 1 )
				mtl = cur;
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
		
		String tpl_data, mtl_data;
		if( !OnLoadShaderFile( type, String_Concat( "tpl_ps_", tpl ), tpl_data ) )
			return false;
		if( mtl.size() && !OnLoadShaderFile( type, String_Concat( "mtl_", mtl ), mtl_data ) )
			return false;
		outdata = String_Concat( prepend, String_Replace( tpl_data, "__CODE__", mtl_data ) );
		return true;
	}
	if( key.part( 0, 3 ) == "vs:" )
	{
		int i = 0;
		String prepend;
		StringView tpl, vs, cur, it = key.part( 3 );
		while( it.size() )
		{
			i++;
			cur = it.until( ":" );
			if( i == 1 )
				vs = cur;
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
		
		String tpl_data, vs_data;
		if( !OnLoadShaderFile( type, String_Concat( "tpl_vs_", tpl ), tpl_data ) )
			return false;
		if( vs.size() && !OnLoadShaderFile( type, String_Concat( "vs_", vs ), vs_data ) )
			return false;
		outdata = String_Concat( prepend, String_Replace( tpl_data, "__VSCODE__", vs_data ) );
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
	
	if( !LoadTextFile( filename, outdata ) )
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
	
	if( !LoadBinaryFile( path, outdata ) )
		return false;
	
	return true;
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
	g_VertexDecls->unset( m_text );
}

const VDeclInfo& VertexDeclHandle::GetInfo()
{
	static VDeclInfo dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}


SGRX_SurfaceShader::~SGRX_SurfaceShader()
{
	g_SurfShaders->unset( m_key );
}

void SGRX_SurfaceShader::ReloadShaders()
{
	// prevent deallocation
	Array< PixelShaderHandle > newshaders;
	newshaders.resize( g_Renderer->m_renderPasses.size() );
	
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
		newshaders[ pass_id ] = GR_GetPixelShader( bfr );
	}
	
	m_shaders = newshaders;
}


SGRX_Material::SGRX_Material() :
	transparent(0), unlit(0), additive(0),
	m_refcount(0)
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
	m_numParts( 0 ),
	m_numBones( 0 ),
	m_boundsMin( Vec3::Create( 0 ) ),
	m_boundsMax( Vec3::Create( 0 ) ),
	m_refcount( 0 )
{
}

SGRX_IMesh::~SGRX_IMesh()
{
	g_Meshes->unset( m_key );
}

bool SGRX_IMesh::SetPartData( SGRX_MeshPart* parts, int count )
{
	if( count < 0 || count > MAX_MESH_PARTS )
		return false;
	int i;
	for( i = 0; i < count; ++i )
		m_parts[ i ] = parts[ i ];
	for( ; i < count; ++i )
		m_parts[ i ] = SGRX_MeshPart();
	m_numParts = count;
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


SGRX_Log& operator << ( SGRX_Log& L, const SGRX_Camera& cam )
{
	L << "CAMERA:";
	L << "\n    position = " << cam.position;
	L << "\n    direction = " << cam.direction;
	L << "\n    up = " << cam.up;
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
	mView.LookAt( position, direction, up );
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

Vec3 SGRX_Camera::WorldToScreen( const Vec3& pos )
{
	Vec3 P = mView.TransformPos( pos );
	P = mProj.TransformPos( P );
	P.x = P.x * 0.5f + 0.5f;
	P.y = P.y * -0.5f + 0.5f;
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
	_mibuf_begin( NULL ),
	_mibuf_end( NULL ),
	_refcount( 0 )
{
	RecalcMatrices();
}

SGRX_Light::~SGRX_Light()
{
	if( _scene )
	{
		_scene->m_lights.unset( this );
	}
}

void SGRX_Light::RecalcMatrices()
{
	viewMatrix = Mat4::CreateLookAt( position, direction, updir );
	projMatrix = Mat4::CreatePerspective( angle, aspect, 0.5, range * 0.001f, range );
	viewProjMatrix.Multiply( viewMatrix, projMatrix );
}


SGRX_CullScene::~SGRX_CullScene()
{
}


SGRX_MeshInstance::SGRX_MeshInstance( SGRX_Scene* s ) :
	_scene( s ),
	color( Vec4::Create( 1 ) ),
	enabled( true ),
	cpuskin( false ),
	dynamic( false ),
	transparent( false ),
	unlit( false ),
//	additive( false ),
	_lightbuf_begin( NULL ),
	_lightbuf_end( NULL ),
	_refcount( 0 )
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
	dirLightDir( Vec3::Create( -1 ).Normalized() ),
	m_refcount( 0 )
{
	camera.position = Vec3::Create( 10, 10, 10 );
	camera.direction = -camera.position.Normalized();
	camera.up = Vec3::Create( 0, 0, 1 );
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
		if( g_Game->GetCompiledShader( g_Renderer->GetInfo().shaderTarget, path, comp ) )
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
		
		g_Game->SetCompiledShader( g_Renderer->GetInfo().shaderTarget, path, comp );
		
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
		if( g_Game->GetCompiledShader( g_Renderer->GetInfo().shaderTarget, path, comp ) )
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
		
		g_Game->SetCompiledShader( g_Renderer->GetInfo().shaderTarget, path, comp );
		
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
	
	VD->m_text = vdecl;
	VD->m_refcount = 0;
	g_VertexDecls->set( VD->m_text, VD );
	
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
	
	SGRX_MeshPart parts[ MAX_MESH_PARTS ] = {0};
	for( int i = 0; i < mfd.numParts; ++i )
	{
		parts[ i ].vertexOffset = mfd.parts[ i ].vertexOffset;
		parts[ i ].vertexCount = mfd.parts[ i ].vertexCount;
		parts[ i ].indexOffset = mfd.parts[ i ].indexOffset;
		parts[ i ].indexCount = mfd.parts[ i ].indexCount;
		
		StringView mtltext( mfd.parts[ i ].materialStrings[0], mfd.parts[ i ].materialStringSizes[0] );
		StringView mtlname = mtltext.until( "|" );
		StringView vsname = mtltext.after( "|" );
		
		// LOAD MATERIAL
		//
		MaterialHandle mh = GR_CreateMaterial();
		if( mfd.parts[ i ].materialStringSizes[0] >= SHADER_NAME_LENGTH )
		{
			LOG_WARNING << "Shader name for part " << i << " is too long";
		}
		else
		{
			mh->shader = GR_GetSurfaceShader( mtlname );
		}
		for( int tid = 0; tid < mfd.parts[ i ].materialTextureCount; ++tid )
		{
			mh->textures[ tid ] = GR_GetTexture( StringView( mfd.parts[ i ].materialStrings[ tid + 1 ], mfd.parts[ i ].materialStringSizes[ tid + 1 ] ) );
		}
		parts[ i ].material = mh;
		
		// LOAD VERTEX SHADER
		//
		char vsbfr[ 256 ] = {0};
		sgrx_snprintf( vsbfr, 255, "vs:%.*s:base%s", TMIN( 200, (int) vsname.size() ), vsname.data(), mfd.numBones ? ":SKIN" : "" );
		parts[ i ].vertexShader = GR_GetVertexShader( vsbfr );
	}
	if( !mesh->SetPartData( parts, mfd.numParts ) )
	{
		LOG_WARNING << "Failed to set part data";
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
	if( !LoadBinaryFile( path, ba ) )
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
	g_Renderer->SetMatrix( false, mtx );
}

void GR2D_SetViewMatrix( const Mat4& mtx )
{
	g_Renderer->SetMatrix( true, mtx );
}

void GR2D_SetScissorRect( int x0, int y0, int x1, int y1 )
{
	int rect[4] = { x0, y0, x1, y1 };
	g_Renderer->SetScissorRect( true, rect );
}

void GR2D_SetViewport( int x0, int y0, int x1, int y1 )
{
	g_Renderer->SetViewport( x0, y0, x1, y1 );
}

void GR2D_UnsetViewport()
{
	g_Renderer->SetViewport( 0, 0, GR_GetWidth(), GR_GetHeight() );
}

void GR2D_UnsetScissorRect()
{
	g_Renderer->SetScissorRect( false, NULL );
}

bool GR2D_SetFont( const StringView& name, int pxsize )
{
	return g_FontRenderer->SetFont( name, pxsize );
}

void GR2D_SetColor( float r, float g, float b, float a )
{
	g_BatchRenderer->Col( r, g, b, a );
}

void GR2D_SetTextCursor( float x, float y )
{
	g_FontRenderer->SetCursor( x, y );
}

Vec2 GR2D_GetTextCursor()
{
	return Vec2::Create( g_FontRenderer->m_cursor_x, g_FontRenderer->m_cursor_y );
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
	g_FontRenderer->SetCursor( x, y );
	return g_FontRenderer->PutText( g_BatchRenderer, text );
}

int GR2D_DrawTextLine( float x, float y, const StringView& text, int halign, int valign )
{
	if( !g_FontRenderer->m_currentFont )
		return 0;
	float length = 0;
	if( halign != 0 )
		length = g_FontRenderer->GetTextWidth( text );
	return GR2D_DrawTextLine( x - round( halign * 0.5f * length ), round( y - valign * 0.5f * g_FontRenderer->m_currentFont->key.size ), text );
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
		AddVertex( m_verts[ m_verts.size() - 1 - i ] );
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

BatchRenderer& BatchRenderer::SphereOutline( const Vec3& pos, float radius, int verts )
{
	CircleOutline( pos, V3(radius,0,0), V3(0,radius,0), verts );
	CircleOutline( pos, V3(0,radius,0), V3(0,0,radius), verts );
	CircleOutline( pos, V3(0,0,radius), V3(radius,0,0), verts );
	return *this;
}

BatchRenderer& BatchRenderer::ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, float angle, int verts )
{
	if( verts >= 3 )
	{
		float q = radius * tan( DEG2RAD( angle ) / 2 );
		Vec3 dx = Vec3Cross(dir,up).Normalized() * q, dy = up.Normalized() * q;
		
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			Pos( pos );
			Pos( pos + dir * radius + sin( a ) * dx + cos( a ) * dy );
			a += ad;
		}
		CircleOutline( pos + dir * radius, dx, dy, verts );
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
				LOG << "Game library: " << value;
			}
		}
		else if( key == "dir" )
		{
			if( value.size() )
			{
				if( !CWDSet( value ) )
					LOG_ERROR << "FAILED TO SET GAME DIRECTORY";
				LOG << "Game directory: " << value;
			}
		}
		else if( key == "renderer" )
		{
			if( value.size() )
			{
				g_RendererName = value;
				LOG << "Renderer: " << value;
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
	sgrx_snprintf( renderer_dll, 256, "%s%.*s.dll", g_RendererPrefix, TMIN( (int) g_RendererName.size(), 200 ), g_RendererName.data() );
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
	LOG << LOG_DATE << "  Created renderer resource caches";
	
	g_BatchRenderer = new BatchRenderer( g_Renderer );
	LOG << LOG_DATE << "  Created batch renderer";
	
	InitializeFontRendering();
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
	
	delete g_BatchRenderer;
	g_BatchRenderer = NULL;
	
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
	
	g_GameLibName.append( STRLIT_BUF( ".dll" ) );
	
	g_GameLib = SDL_LoadObject( StackPath( g_GameLibName ) );
	if( !g_GameLib )
	{
		LOG_ERROR << "Failed to load " << g_GameLibName;
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

