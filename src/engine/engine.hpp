

#pragma once
#ifdef INCLUDE_REAL_SDL
#  include <SDL2/SDL.h>
#  include <SDL2/SDL_syswm.h>
#  define SDLSTRUCT
#elif defined NO_SDL
struct SDL_Event;
#  define SDLSTRUCT struct
#else
#  include "sdl_events_min.h"
#  define SDLSTRUCT struct
#endif
#include "utils.hpp"


#define SGRX_MAX_TEXTURES 16


struct SGRX_Mesh;
struct SGRX_DrawItem;
struct SGRX_MeshInstance;
struct SGRX_Light;
struct SGRX_Scene;


ENGINE_EXPORT uint32_t GetTimeMsec();
ENGINE_EXPORT void Thread_Sleep( uint32_t msec );
ENGINE_EXPORT void Sys_FatalError( const StringView& err );

ENGINE_EXPORT bool Window_HasClipboardText();
ENGINE_EXPORT bool Window_GetClipboardText( String& out );
ENGINE_EXPORT bool Window_SetClipboardText( const StringView& text );

typedef SDL_Event Event;


struct Command
{
	Command( const StringView& sv, float thr = 0.25f ) :
		name(sv),
		threshold(thr),
		value(0), prev_value(0),
		state(false), prev_state(false)
	{}
	virtual ~Command(){}
	virtual void OnChangeState(){}
	
	FINLINE bool IsPressed() const { return state && !prev_state; }
	FINLINE bool IsReleased() const { return !state && prev_state; }
	
	ENGINE_EXPORT void _SetState( float x );
	ENGINE_EXPORT void _Advance();
	
	StringView name;
	float threshold;
	float value, prev_value;
	bool state, prev_state;
};

struct Command_Func : Command
{
	virtual void Function() = 0;
	Command_Func( const StringView& sv, float thr = 0.1f ) : Command( sv, thr ){}
	void OnChangeState(){ if( state && !state ) Function(); }
};

#define SGRX_MB_UNKNOWN 0
#define SGRX_MB_LEFT 1
#define SGRX_MB_MIDDLE 2
#define SGRX_MB_RIGHT 3
#define SGRX_MB_X1 4
#define SGRX_MB_X2 5

typedef uint64_t ActionInput;
#define ACTINPUT_UNASSIGNED 0
#define ACTINPUT_KEY 1
#define ACTINPUT_MOUSE 2
#define ACTINPUT_GAMEPAD 3
#define ACTINPUT_TYPE_BUTTON 0x0000
#define ACTINPUT_TYPE_AXIS 0x2000
#define ACTINPUT_MAKE( type, val ) (((uint64_t)(type)<<32ull)|(val))
#define ACTINPUT_MAKE_KEY( val ) ACTINPUT_MAKE( ACTINPUT_KEY, val )
#define ACTINPUT_MAKE_MOUSE( val ) ACTINPUT_MAKE( ACTINPUT_MOUSE, val )
#define ACTINPUT_MAKE_GPADBTN( val ) ACTINPUT_MAKE( ACTINPUT_GAMEPAD, ACTINPUT_TYPE_BUTTON + val )
#define ACTINPUT_MAKE_GPADAXIS( val ) ACTINPUT_MAKE( ACTINPUT_GAMEPAD, ACTINPUT_TYPE_AXIS + val )
#define ACTINPUT_GET_TYPE( iid ) (((iid)>>32ull)&0xffffffff)
#define ACTINPUT_GET_VALUE( iid ) ((iid)&0xffffffff)

ENGINE_EXPORT void Game_RegisterAction( Command* cmd );
ENGINE_EXPORT void Game_UnregisterAction( Command* cmd );
ENGINE_EXPORT Command* Game_FindAction( const StringView& cmd );
ENGINE_EXPORT void Game_BindKeyToAction( uint32_t key, Command* cmd );
ENGINE_EXPORT void Game_BindKeyToAction( uint32_t key, const StringView& cmd );
ENGINE_EXPORT void Game_BindMouseButtonToAction( int btn, Command* cmd );
ENGINE_EXPORT void Game_BindMouseButtonToAction( int btn, const StringView& cmd );
ENGINE_EXPORT void Game_BindGamepadButtonToAction( int btn, Command* cmd );
ENGINE_EXPORT void Game_BindGamepadButtonToAction( int btn, const StringView& cmd );
ENGINE_EXPORT void Game_BindGamepadAxisToAction( int axis, Command* cmd );
ENGINE_EXPORT void Game_BindGamepadAxisToAction( int axis, const StringView& cmd );
ENGINE_EXPORT ActionInput Game_GetActionBinding( Command* cmd );
ENGINE_EXPORT int Game_GetActionBindings( Command* cmd, ActionInput* out, int bufsize );
ENGINE_EXPORT void Game_BindInputToAction( ActionInput iid, Command* cmd );
ENGINE_EXPORT void Game_UnbindInput( ActionInput iid );
ENGINE_EXPORT StringView Game_GetInputName( ActionInput iid );
ENGINE_EXPORT Vec2 Game_GetCursorPos();
ENGINE_EXPORT Vec2 Game_GetScreenSize();
ENGINE_EXPORT Vec2 Game_GetCursorPosNormalized();
ENGINE_EXPORT void Game_SetCursorPos( int x, int y );
ENGINE_EXPORT void Game_ShowCursor( bool show );


struct ENGINE_EXPORT IScreen
{
	virtual ~IScreen(){}
	virtual void OnStart(){}
	virtual void OnEnd(){}
	virtual bool OnEvent( const Event& e ) = 0; // return value - whether event is inhibited
	virtual bool Draw( float delta ) = 0; // return value - whether to remove
};

ENGINE_EXPORT bool Game_HasOverlayScreens();
ENGINE_EXPORT bool Game_HasOverlayScreen( IScreen* screen );
ENGINE_EXPORT void Game_AddOverlayScreen( IScreen* screen );
ENGINE_EXPORT void Game_RemoveOverlayScreen( IScreen* screen );
ENGINE_EXPORT void Game_RemoveAllOverlayScreens();

ENGINE_EXPORT void Game_End();


#define TEXFLAGS_SRGB    0x01
#define TEXFLAGS_HASMIPS 0x02
#define TEXFLAGS_LERP    0x04
#define TEXFLAGS_CLAMP_X 0x10
#define TEXFLAGS_CLAMP_Y 0x20

struct IF_GCC(ENGINE_EXPORT) IDirEntryHandler
{
	ENGINE_EXPORT virtual bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir ) = 0;
};

struct IF_GCC(ENGINE_EXPORT) IFileSystem
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	ENGINE_EXPORT IFileSystem();
	ENGINE_EXPORT virtual ~IFileSystem();
	ENGINE_EXPORT virtual bool LoadBinaryFile( const StringView& path, ByteArray& out ) = 0;
	ENGINE_EXPORT virtual bool SaveBinaryFile( const StringView& path, const void* data, size_t size ) = 0;
	ENGINE_EXPORT virtual bool LoadTextFile( const StringView& path, String& out ) = 0;
	ENGINE_EXPORT virtual bool SaveTextFile( const StringView& path, const StringView& data ) = 0;
	ENGINE_EXPORT virtual bool FindRealPath( const StringView& path, String& out ) = 0;
	ENGINE_EXPORT virtual bool FileExists( const StringView& path ) = 0;
	ENGINE_EXPORT virtual bool DirCreate( const StringView& path ) = 0;
	ENGINE_EXPORT virtual uint32_t FileModTime( const StringView& path ) = 0;
	ENGINE_EXPORT virtual void IterateDirectory( const StringView& path, IDirEntryHandler* deh ) = 0;
	
	int32_t m_refcount;
};

struct IF_GCC(ENGINE_EXPORT) BasicFileSystem : IFileSystem
{
	ENGINE_EXPORT BasicFileSystem( const StringView& root );
	ENGINE_EXPORT virtual bool LoadBinaryFile( const StringView& path, ByteArray& out );
	ENGINE_EXPORT virtual bool SaveBinaryFile( const StringView& path, const void* data, size_t size );
	ENGINE_EXPORT virtual bool LoadTextFile( const StringView& path, String& out );
	ENGINE_EXPORT virtual bool SaveTextFile( const StringView& path, const StringView& data );
	ENGINE_EXPORT virtual bool FindRealPath( const StringView& path, String& out );
	ENGINE_EXPORT virtual bool FileExists( const StringView& path );
	ENGINE_EXPORT virtual bool DirCreate( const StringView& path );
	ENGINE_EXPORT virtual uint32_t FileModTime( const StringView& path );
	ENGINE_EXPORT virtual void IterateDirectory( const StringView& path, IDirEntryHandler* deh );
	
	String m_fileRoot;
};
typedef Handle< IFileSystem > FileSysHandle;

ENGINE_EXPORT StringView Game_GetDir();
ENGINE_EXPORT Array< FileSysHandle >& Game_FileSystems();

ENGINE_EXPORT bool FS_LoadBinaryFile( const StringView& path, ByteArray& out );
ENGINE_EXPORT bool FS_SaveBinaryFile( const StringView& path, const void* data, size_t size );
ENGINE_EXPORT bool FS_LoadTextFile( const StringView& path, String& out );
ENGINE_EXPORT bool FS_SaveTextFile( const StringView& path, const StringView& data );
ENGINE_EXPORT bool FS_FindRealPath( const StringView& path, String& out );
ENGINE_EXPORT bool FS_FileExists( const StringView& path );
ENGINE_EXPORT bool FS_DirCreate( const StringView& path );
ENGINE_EXPORT uint32_t FS_FileModTime( const StringView& path );
ENGINE_EXPORT void FS_IterateDirectory( const StringView& path, IDirEntryHandler* deh );


//
// PHYSICS DATA
//

struct IF_GCC(ENGINE_EXPORT) SGRX_ConvexPointSet : SGRX_RCRsrc
{
	struct Data
	{
		template< class T > void Serialize( T& arch )
		{
			arch.marker( "SGRXCPST" );
			arch << points;
		}
		
		Array< Vec3 > points;
	};
	
	ENGINE_EXPORT ~SGRX_ConvexPointSet();
	ENGINE_EXPORT static SGRX_ConvexPointSet* Create( const StringView& path );
	template< class T > void Serialize( T& arch ){ data.Serialize( arch ); }
	
	Data data;
};
typedef Handle< SGRX_ConvexPointSet > ConvexPointSetHandle;

ENGINE_EXPORT ConvexPointSetHandle GP_GetConvexPointSet( const StringView& path );


//
// RENDERER DATA
//

#define FULLSCREEN_NONE 0
#define FULLSCREEN_NORMAL 1
#define FULLSCREEN_WINDOWED 2

#define ANTIALIAS_NONE 0
#define ANTIALIAS_MULTISAMPLE 1

struct RenderSettings
{
	int display;
	int width;
	int height;
	int refresh_rate;
	int fullscreen;
	bool vsync;
	int aa_mode;
	int aa_quality;
};

struct DisplayMode
{
	int width;
	int height;
	int refresh_rate;
	
	bool operator == ( const DisplayMode& o ) const { return width == o.width && height == o.height && refresh_rate == o.refresh_rate; }
	void Log( SGRX_Log& L ) const
	{
		L << "Display mode (width: " << width << ", height: " << height << ", refresh rate: " << refresh_rate << ")";
	}
	typedef SGRX_Log::Loggable< DisplayMode > LogT;
};

//inline SGRX_Log& SGRX_Log::operator << ( const DisplayMode& dm )
//{
//	*this << "Display mode (width: " << dm.width << ", height: " << dm.height << ", refresh rate: " << dm.refresh_rate << ")";
//	return *this;
//}


ENGINE_EXPORT bool GR_SetVideoMode( const RenderSettings& rs );
ENGINE_EXPORT void GR_GetVideoMode( RenderSettings& rs );

ENGINE_EXPORT int GR_GetDisplayCount();
ENGINE_EXPORT const char* GR_GetDisplayName( int id );
ENGINE_EXPORT bool GR_ListDisplayModes( int display, Array< DisplayMode >& out );

struct ENGINE_EXPORT RenderStats
{
	void Reset();
	
	uint32_t numVisMeshes;
	uint32_t numVisPLights; // point lights
	uint32_t numVisSLights; // spotlights
	uint32_t numDrawCalls;
	uint32_t numSDrawCalls; // shadow
	uint32_t numMDrawCalls; // main
	uint32_t numPDrawCalls; // post-processing
};

#define TEXTYPE_2D     1 /* 1 side, width x height */
#define TEXTYPE_CUBE   2 /* 6 sides, width x width */
#define TEXTYPE_VOLUME 3 /* 1 side, width x height x depth */

#define TEXFORMAT_UNKNOWN 0
#define TEXFORMAT_RGBA8  1
#define TEXFORMAT_BGRA8  2
#define TEXFORMAT_BGRX8  3
#define TEXFORMAT_R5G6B5 5
#define TEXFORMAT_DXT1   11
#define TEXFORMAT_DXT3   13
#define TEXFORMAT_DXT5   15
#define TEXFORMAT_ISBLOCK4FORMAT( x ) ((x)==TEXFORMAT_DXT1||(x)==TEXFORMAT_DXT3||(x)==TEXFORMAT_DXT5)

#define RT_FORMAT_DEPTH       0x4000
#define RT_FORMAT_COLOR_HDR16 0x4001 // 16bit unlimited color range
#define RT_FORMAT_COLOR_LDR8  0x4002 // 8bit [0;1] color range
#define RT_FORMAT_USE_MSAA    0x8000

struct TextureInfo /* 16 bytes */
{
	uint8_t type; /* TEXTYPE */
	uint8_t mipcount;
	uint16_t width;
	uint16_t height;
	uint16_t depth;
	uint16_t format; /* TEXFORMAT / RT_FORMAT */
	uint16_t flags; /* TEXFLAGS */
};

struct IF_GCC(ENGINE_EXPORT) SGRX_ITexture : SGRX_RCRsrc
{
	ENGINE_EXPORT SGRX_ITexture();
	ENGINE_EXPORT virtual ~SGRX_ITexture();
	ENGINE_EXPORT virtual bool UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h ) = 0;
	
	TextureInfo m_info;
	bool m_isRenderTexture;
	uint64_t m_rtkey;
};

struct TextureHandle : Handle< SGRX_ITexture >
{
	TextureHandle() : Handle(){}
	TextureHandle( const TextureHandle& h ) : Handle( h ){}
	TextureHandle( SGRX_ITexture* tex ) : Handle( tex ){}
	
	ENGINE_EXPORT const TextureInfo& GetInfo() const;
	ENGINE_EXPORT Vec2 GetInvSize( Vec2 def = V2(0) ) const;
	ENGINE_EXPORT bool UploadRGBA8Part( void* data, int mip = 0, int w = -1, int h = -1, int x = 0, int y = 0 );
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IDepthStencilSurface : SGRX_RefCounted
{
	ENGINE_EXPORT SGRX_IDepthStencilSurface();
	ENGINE_EXPORT virtual ~SGRX_IDepthStencilSurface();
	
	uint16_t m_width;
	uint16_t m_height;
	uint32_t m_format; /* RT_FORMAT */
	uint64_t m_key;
};

typedef Handle< SGRX_IDepthStencilSurface > DepthStencilSurfHandle;

struct IF_GCC(ENGINE_EXPORT) SGRX_IVertexShader : SGRX_RCRsrc
{
	ENGINE_EXPORT virtual ~SGRX_IVertexShader();
};

struct VertexShaderHandle : Handle< SGRX_IVertexShader >
{
	VertexShaderHandle() : Handle(){}
	VertexShaderHandle( const VertexShaderHandle& h ) : Handle( h ){}
	VertexShaderHandle( SGRX_IVertexShader* shdr ) : Handle( shdr ){}
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IPixelShader : SGRX_RCRsrc
{
	ENGINE_EXPORT virtual ~SGRX_IPixelShader();
};

struct PixelShaderHandle : Handle< SGRX_IPixelShader >
{
	PixelShaderHandle() : Handle(){}
	PixelShaderHandle( const PixelShaderHandle& h ) : Handle( h ){}
	PixelShaderHandle( SGRX_IPixelShader* shdr ) : Handle( shdr ){}
};


// Cull mode: 2 bits
#define SGRX_RS_CullMode_None 0
#define SGRX_RS_CullMode_Back 1
#define SGRX_RS_CullMode_Front 2

// Blend op: 3 bits
#define SGRX_RS_BlendOp_Add 0
#define SGRX_RS_BlendOp_Sub 1
#define SGRX_RS_BlendOp_RevSub 2
#define SGRX_RS_BlendOp_Min 3
#define SGRX_RS_BlendOp_Max 4

// Blend factor: 4 bits
#define SGRX_RS_Blend_Zero 0
#define SGRX_RS_Blend_One 1
#define SGRX_RS_Blend_SrcColor 2
#define SGRX_RS_Blend_InvSrcColor 3
#define SGRX_RS_Blend_DstColor 4
#define SGRX_RS_Blend_InvDstColor 5
#define SGRX_RS_Blend_SrcAlpha 6
#define SGRX_RS_Blend_InvSrcAlpha 7
#define SGRX_RS_Blend_DstAlpha 8
#define SGRX_RS_Blend_InvDstAlpha 9
#define SGRX_RS_Blend_Factor 10
#define SGRX_RS_Blend_InvFactor 11

// Color write enable : 4 bits
#define SGRX_RS_ColorWrite_R 0x1
#define SGRX_RS_ColorWrite_G 0x2
#define SGRX_RS_ColorWrite_B 0x4
#define SGRX_RS_ColorWrite_A 0x8
#define SGRX_RS_ColorWrite_All 0xf

// Depth comparison: 3 bits
#define SGRX_RS_DepthComp_Never 0
#define SGRX_RS_DepthComp_Always 1
#define SGRX_RS_DepthComp_Equal 2
#define SGRX_RS_DepthComp_NotEqual 3
#define SGRX_RS_DepthComp_Less 4
#define SGRX_RS_DepthComp_LessEqual 5
#define SGRX_RS_DepthComp_Greater 6
#define SGRX_RS_DepthComp_GreaterEqual 7

// Stencil operation : 3 bits
#define SGRX_RS_StencilOp_Keep 0
#define SGRX_RS_StencilOp_Zero 1
#define SGRX_RS_StencilOp_Replace 2
#define SGRX_RS_StencilOp_Invert 3
#define SGRX_RS_StencilOp_Incr 4
#define SGRX_RS_StencilOp_Decr 5
#define SGRX_RS_StencilOp_IncrSat 6
#define SGRX_RS_StencilOp_DecrSat 7

#define SGRX_RS_MAX_RENDER_TARGETS 8

struct SGRX_RenderState // 68 bytes (aligned to 80?)
{
	struct BlendState // 27 bits -> 4 bytes
	{
		uint32_t colorWrite : 4;
		uint32_t blendEnable : 1;
		uint32_t blendOp : 3;
		uint32_t srcBlend : 4;
		uint32_t dstBlend : 4;
		uint32_t blendOpAlpha : 3;
		uint32_t srcBlendAlpha : 4;
		uint32_t dstBlendAlpha : 4;
		
		bool operator == ( const BlendState& o ) const
		{
			if( colorWrite != o.colorWrite ) return false;
			if( blendEnable != o.blendEnable ) return false;
			if( blendOp != o.blendOp ) return false;
			if( srcBlend != o.srcBlend ) return false;
			if( dstBlend != o.dstBlend ) return false;
			if( blendOpAlpha != o.blendOpAlpha ) return false;
			if( srcBlendAlpha != o.srcBlendAlpha ) return false;
			if( dstBlendAlpha != o.dstBlendAlpha ) return false;
			return true;
		}
		
		Hash getHash() const
		{
			// must only use first 24 bits
			Hash out = 0;
			out ^= colorWrite << 0;
			out ^= blendEnable << 4;
			out ^= blendOp << 5;
			out ^= srcBlend << 8;
			out ^= dstBlend << 12;
			out ^= blendOpAlpha << 16;
			out ^= srcBlendAlpha << 19;
			out ^= dstBlendAlpha << 9;
			return out;
		}
	};
	
	// rasterizer state : 6 bits
	uint32_t wireFill : 1; // wireframe fill instead of solid?
	uint32_t cullMode : 2; // triangle face culling
	uint32_t separateBlend : 1; // whether to use different blend states for different render targets
	uint32_t scissorEnable : 1;
	uint32_t multisampleEnable : 1;
	
	// depth state : 54 bits
	uint32_t depthEnable : 1;
	uint32_t depthWriteEnable : 1;
	uint32_t depthFunc : 3;
	uint32_t stencilEnable : 1;
	uint32_t stencilReadMask : 8;
	uint32_t stencilWriteMask : 8;
	uint32_t stencilFrontFailOp : 3;
	uint32_t stencilFrontDepthFailOp : 3;
	uint32_t stencilFrontPassOp : 3;
	uint32_t stencilFrontFunc : 3;
	uint32_t stencilBackFailOp : 3;
	uint32_t stencilBackDepthFailOp : 3;
	uint32_t stencilBackPassOp : 3;
	uint32_t stencilBackFunc : 3;
	uint32_t stencilRef : 8;
	
	// all flags : 60 bits -> 8 bytes
	
	// blend states : 4 * 8 = 32 bytes
	BlendState blendStates[ SGRX_RS_MAX_RENDER_TARGETS ];
	
	// depth parameters : 28 bytes
	float depthBias;
	float slopeDepthBias;
	float depthBiasClamp;
	Vec4 blendFactor;
	
	void Init()
	{
		// most data is 0
		memset( this, 0, sizeof(*this) );
		depthEnable = 1;
		depthWriteEnable = 1;
		depthFunc = SGRX_RS_DepthComp_LessEqual;
		multisampleEnable = 1;
		slopeDepthBias = 0;
		depthBiasClamp = 0;
		stencilReadMask = 0xff;
		stencilWriteMask = 0xff;
		blendFactor = V4(1);
		for( int i = 0; i < SGRX_RS_MAX_RENDER_TARGETS; ++i )
		{
			blendStates[ i ].colorWrite = SGRX_RS_ColorWrite_All;
			blendStates[ i ].srcBlend = SGRX_RS_Blend_SrcAlpha;
			blendStates[ i ].dstBlend = SGRX_RS_Blend_InvSrcAlpha;
			blendStates[ i ].srcBlendAlpha = SGRX_RS_Blend_SrcAlpha;
			blendStates[ i ].dstBlendAlpha = SGRX_RS_Blend_InvSrcAlpha;
		}
	}
	
	void InitOpposite()
	{
		// initialize opposite to default state, to make *all* comparisons fail
		Init();
		uint8_t* buf = (uint8_t*) this;
		uint8_t* end = buf + sizeof(*this);
		while( buf < end )
			*buf++ ^= 0xff;
	}
	
	bool operator == ( const SGRX_RenderState& o ) const
	{
		if( wireFill != o.wireFill ) return false;
		if( cullMode != o.cullMode ) return false;
		if( separateBlend != o.separateBlend ) return false;
		if( scissorEnable != o.scissorEnable ) return false;
		if( multisampleEnable != o.multisampleEnable ) return false;
		if( depthEnable != o.depthEnable ) return false;
		if( depthWriteEnable != o.depthWriteEnable ) return false;
		if( depthFunc != o.depthFunc ) return false;
		if( stencilEnable != o.stencilEnable ) return false;
		if( stencilReadMask != o.stencilReadMask ) return false;
		if( stencilWriteMask != o.stencilWriteMask ) return false;
		if( stencilFrontFailOp != o.stencilFrontFailOp ) return false;
		if( stencilFrontDepthFailOp != o.stencilFrontDepthFailOp ) return false;
		if( stencilFrontPassOp != o.stencilFrontPassOp ) return false;
		if( stencilFrontFunc != o.stencilFrontFunc ) return false;
		if( stencilBackFailOp != o.stencilBackFailOp ) return false;
		if( stencilBackDepthFailOp != o.stencilBackDepthFailOp ) return false;
		if( stencilBackPassOp != o.stencilBackPassOp ) return false;
		if( stencilBackFunc != o.stencilBackFunc ) return false;
		if( stencilRef != o.stencilRef ) return false;
		if( depthBias != o.depthBias ) return false;
		if( slopeDepthBias != o.slopeDepthBias ) return false;
		if( depthBiasClamp != o.depthBiasClamp ) return false;
		if( blendFactor != o.blendFactor ) return false;
		
		int count = separateBlend ? SGRX_RS_MAX_RENDER_TARGETS : 1;
		for( int i = 0; i < count; ++i )
		{
			if( !( blendStates[ i ] == o.blendStates[ i ] ) )
				return false;
		}
		return true;
	}
	
	Hash getHash() const
	{
		Hash out = 0x12345678;
		
		out ^= wireFill << 0;
		out ^= cullMode << 1;
		out ^= separateBlend << 3;
		out ^= scissorEnable << 4;
		out ^= multisampleEnable << 5;
		
		out ^= depthEnable << 6;
		out ^= depthWriteEnable << 7;
		out ^= depthFunc << 8;
		out ^= stencilEnable << 11;
		out ^= stencilReadMask << 12;
		out ^= stencilWriteMask << 20;
		out ^= stencilFrontFailOp << 28;
		out ^= stencilFrontDepthFailOp << 0;
		out ^= stencilFrontPassOp << 3;
		out ^= stencilFrontFunc << 6;
		out ^= stencilBackFailOp << 9;
		out ^= stencilBackDepthFailOp << 12;
		out ^= stencilBackPassOp << 15;
		out ^= stencilBackFunc << 18;
		out ^= stencilRef << 21;
		
		out ^= HashVar( depthBias );
		out ^= HashVar( slopeDepthBias );
		out ^= HashVar( depthBiasClamp );
		
		int count = separateBlend ? SGRX_RS_MAX_RENDER_TARGETS : 1;
		for( int i = 0; i < count; ++i )
		{
			out ^= blendStates[ i ].getHash() << i;
		}
		
		return out;
	}
};
inline Hash HashVar( const SGRX_RenderState& state ){ return state.getHash(); }

struct IF_GCC(ENGINE_EXPORT) SGRX_IRenderState : SGRX_RCRsrc
{
	ENGINE_EXPORT virtual ~SGRX_IRenderState();
	ENGINE_EXPORT virtual void SetState( const SGRX_RenderState& state );
	
	SGRX_RenderState m_info;
};

struct RenderStateHandle : Handle< SGRX_IRenderState >
{
	RenderStateHandle() : Handle(){}
	RenderStateHandle( const RenderStateHandle& h ) : Handle( h ){}
	RenderStateHandle( SGRX_IRenderState* vd ) : Handle( vd ){}
	
	ENGINE_EXPORT const SGRX_RenderState& GetInfo();
};


#define VDECL_MAX_ITEMS 10

#define VDECLTYPE_FLOAT1 0
#define VDECLTYPE_FLOAT2 1
#define VDECLTYPE_FLOAT3 2
#define VDECLTYPE_FLOAT4 3
#define VDECLTYPE_BCOL4 4

/* usage type | expected data type */
#define VDECLUSAGE_POSITION 0 /* float3 */
#define VDECLUSAGE_COLOR    1 /* float3/float4/bcol4 */
#define VDECLUSAGE_NORMAL   2 /* float3 */
#define VDECLUSAGE_TANGENT  3 /* float4 */
#define VDECLUSAGE_BLENDWT  4 /* preferably bcol4 */
#define VDECLUSAGE_BLENDIDX 5 /* preferably bcol4 (will be passed as ubyte4, not float4) */
#define VDECLUSAGE_TEXTURE0 6 /* any .. */
#define VDECLUSAGE_TEXTURE1 7
#define VDECLUSAGE_TEXTURE2 8
#define VDECLUSAGE_TEXTURE3 9

#define SHADER_NAME_LENGTH 64
#define MAX_MI_TEXTURES    4
#define MAX_MI_CONSTANTS   16

#define MAX_ANIM_MARKER_NAME_LENGTH 16

struct VDeclInfo
{
	uint8_t offsets[ VDECL_MAX_ITEMS ];
	uint8_t types  [ VDECL_MAX_ITEMS ];
	uint8_t usages [ VDECL_MAX_ITEMS ];
	uint8_t count;
	uint8_t size;
	
	int GetOffset( int usage ) const
	{
		for( int i = 0; i < count; ++i )
			if( usages[ i ] == usage )
				return offsets[ i ];
		return -1;
	}
	int GetType( int usage ) const
	{
		for( int i = 0; i < count; ++i )
			if( usages[ i ] == usage )
				return types[ i ];
		return -1;
	}
};

#define SGRX_VDECL_DECAL "pf3nf30f3tb4cb4xb4"
struct SGRX_Vertex_Decal
{
	Vec3 position;
	Vec3 normal;
	Vec3 texcoord;
	uint32_t tangent;
	uint32_t color;
	uint32_t padding0;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IVertexDecl : SGRX_RCRsrc
{
	ENGINE_EXPORT virtual ~SGRX_IVertexDecl();
	
	VDeclInfo m_info;
};

struct VertexDeclHandle : Handle< SGRX_IVertexDecl >
{
	VertexDeclHandle() : Handle(){}
	VertexDeclHandle( const VertexDeclHandle& h ) : Handle( h ){}
	VertexDeclHandle( SGRX_IVertexDecl* vd ) : Handle( vd ){}
	
	ENGINE_EXPORT const VDeclInfo& GetInfo();
};



#define SGRX_MtlFlag_Unlit    0x01
#define SGRX_MtlFlag_Nocull   0x02
#define SGRX_MtlFlag_Decal    0x04 // depth offset, changed drawing order
#define SGRX_MtlFlag_Disable  0x08
#define SGRX_MtlBlend_None     0
#define SGRX_MtlBlend_Basic    1 // alpha blending
#define SGRX_MtlBlend_Additive 2
#define SGRX_MtlBlend_Multiply 3

struct SGRX_Material
{
	ENGINE_EXPORT SGRX_Material();
	
	String shader;
	TextureHandle textures[ SGRX_MAX_TEXTURES ];
	
	uint8_t flags;
	uint8_t blendMode;
};


struct IF_GCC(ENGINE_EXPORT) IMeshRaycast
{
	virtual ~IMeshRaycast(){}
	virtual void RaycastAll( const Vec3& from, const Vec3& to, struct SceneRaycastCallback* cb, SGRX_MeshInstance* cbmi = NULL ) = 0;
	virtual void MRC_DebugDraw( SGRX_MeshInstance* mi ) = 0;
};

#define INDEX_16 0
#define INDEX_32 1

/* mesh data flags */
#define MDF_INDEX_32      0x01
#define MDF_TRIANGLESTRIP 0x02
#define MDF_DYNAMIC       0x04 /* dynamic buffer updating */
#define MDF_SKINNED       0x80 /* mesh has bone data (name, offset, parent id) */
#define MDF_MTLINFO      0x100 /* mesh has material info (flags, blend mode) */

#define MDF__PUBFLAGMASK (0x01|0x02|0x80|0x100)
#define MDF__PUBFLAGBASE  0

#define SGRX_MAX_MESH_TEXTURES 8
#define SGRX_MAX_MESH_BONES 64

struct SGRX_MeshPart
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	
	String shader;
	String textures[ SGRX_MAX_MESH_TEXTURES ];
	uint8_t mtlFlags;
	uint8_t mtlBlendMode;
	
	FINLINE bool CanDraw( int i ) const
	{
		return indexCount >= 3;
	}
};

struct SGRX_MeshBone
{
	SGRX_MeshBone() : parent_id(-1){}
	
	String name;
	Mat4 skinOffset;
	Mat4 boneOffset;
	Mat4 invSkinOffset;
	int parent_id;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IMesh : SGRX_RCRsrc, IMeshRaycast
{
	ENGINE_EXPORT SGRX_IMesh();
	ENGINE_EXPORT virtual ~SGRX_IMesh();
	
	bool ToMeshData( ByteArray& out );
	
	virtual bool InitVertexBuffer( size_t size, VertexDeclHandle vd ) = 0;
	virtual bool InitIndexBuffer( size_t size, bool i32 ) = 0;
	virtual bool UpdateVertexData( const void* data, size_t size, bool tristrip ) = 0;
	virtual bool UpdateIndexData( const void* data, size_t size ) = 0;
	ENGINE_EXPORT virtual bool SetPartData( SGRX_MeshPart* parts, int count );
	
	ENGINE_EXPORT bool SetBoneData( SGRX_MeshBone* bones, int count );
	ENGINE_EXPORT bool RecalcBoneMatrices();
	ENGINE_EXPORT bool SetAABBFromVertexData( const void* data, size_t size, VertexDeclHandle vd );
	
	bool SetVertexData( const void* data, size_t size, VertexDeclHandle vd, bool tristrip )
	{
		LOG_FUNCTION;
		return InitVertexBuffer( size, vd ) && UpdateVertexData( data, size, tristrip );
	}
	bool SetIndexData( const void* data, size_t size, bool i32 )
	{
		LOG_FUNCTION;
		return InitIndexBuffer( size, i32 ) && UpdateIndexData( data, size );
	}
	
	FINLINE bool IsValid() const
	{
		return m_vertexDecl && m_meshParts.size();
	}
	
	ENGINE_EXPORT virtual void RaycastAll( const Vec3& from, const Vec3& to, struct SceneRaycastCallback* cb, struct SGRX_MeshInstance* cbmi = NULL );
	ENGINE_EXPORT virtual void MRC_DebugDraw( SGRX_MeshInstance* mi );
	
	ENGINE_EXPORT void Clip(
		const Mat4& mtx,
		const Mat4& vpmtx,
		ByteArray& outverts,
		bool decal = false,
		float inv_zn2zf = 0,
		uint32_t color = 0xffffffff,
		size_t firstPart = 0,
		size_t numParts = (size_t)-1
	);
	
	ENGINE_EXPORT bool IsBoneUnder( int bone, int parent );
	
	/* rendering info */
	uint32_t m_dataFlags;
	VertexDeclHandle m_vertexDecl;
	uint32_t m_vertexCount;
	uint32_t m_vertexDataSize;
	uint32_t m_indexCount;
	uint32_t m_indexDataSize;
	Array< SGRX_MeshPart > m_meshParts;
	SGRX_MeshBone m_bones[ SGRX_MAX_MESH_BONES ];
	int m_numBones;
	
	/* collision detection */
	Vec3 m_boundsMin;
	Vec3 m_boundsMax;
	
	/* vertex/index data */
	ByteArray m_vdata;
	ByteArray m_idata;
};

struct MeshHandle : Handle< SGRX_IMesh >
{
	MeshHandle() : Handle(){}
	MeshHandle( const MeshHandle& h ) : Handle( h ){}
	MeshHandle( struct SGRX_IMesh* mesh ) : Handle( mesh ){}
};

struct SGRX_DrawItemLight
{
	SGRX_DrawItem* DI;
	SGRX_Light* L;
};

#define LIGHT_POINT  1
#define LIGHT_SPOT   2
#define LIGHT_PROJ   3

struct SGRX_Light : SGRX_RCXFItem
{
	ENGINE_EXPORT SGRX_Light( SGRX_Scene* s );
	ENGINE_EXPORT virtual ~SGRX_Light();
	ENGINE_EXPORT void UpdateTransform();
	ENGINE_EXPORT void GenerateCamera( struct SGRX_Camera& outcam );
	ENGINE_EXPORT virtual void SetTransform( const Mat4& mtx );
	ENGINE_EXPORT void GetVolumePoints( Vec3 pts[8] );
	
	SGRX_Scene* _scene;
	
	// inputs
	int type;
	bool enabled;
	Vec3 position;
	Vec3 direction;
	Vec3 updir;
	Vec3 color;
	float range;
	float power;
	float angle;
	float aspect;
	TextureHandle cookieTexture;
	TextureHandle shadowTexture;
	bool hasShadows;
	uint32_t layers;
	SGRX_Material projectionMaterial;
	Mat4 matrix;
	
	// cache
	Vec3 _tf_position;
	Vec3 _tf_direction;
	Vec3 _tf_updir;
	float _tf_range;
	Mat4 viewMatrix;
	Mat4 projMatrix;
	Mat4 viewProjMatrix;
	
	/* frame cache */
	SGRX_DrawItemLight* _dibuf_begin;
	SGRX_DrawItemLight* _dibuf_end;
};

struct LightHandle : Handle< SGRX_Light >
{
	LightHandle() : Handle(){}
	LightHandle( const LightHandle& h ) : Handle( h ){}
	LightHandle( SGRX_Light* lt ) : Handle( lt ){}
};



struct SGRX_CullSceneFrustum
{
	Vec3 position;
	Vec3 direction;
	Vec3 updir;
	float hangle;
	float vangle;
	float znear;
	float zfar;
};

struct SGRX_CullSceneCamera
{
	SGRX_CullSceneFrustum frustum;
	Mat4 viewProjMatrix;
	Mat4 invViewProjMatrix;
};

struct SGRX_CullScenePointLight
{
	Vec3 position;
	float radius;
};

struct SGRX_CullSceneMesh
{
	Mat4 transform;
	Vec3 min, max;
};

struct SGRX_CullScene
{
	ENGINE_EXPORT virtual ~SGRX_CullScene();
	virtual void Camera_Prepare( SGRX_CullSceneCamera* camera ){}
	virtual bool Camera_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield ){ return false; }
	virtual bool Camera_PointLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullScenePointLight* lights, uint32_t* outbitfield ){ return false; }
	virtual bool Camera_SpotLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneFrustum* frusta, Mat4* inv_matrices, uint32_t* outbitfield ){ return false; }
	virtual void SpotLight_Prepare( SGRX_CullSceneCamera* camera ){ Camera_Prepare( camera ); }
	virtual bool SpotLight_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield ){ return Camera_MeshList( count, camera, meshes, outbitfield ); }
};

struct SGRX_DefaultCullScene : SGRX_CullScene
{
	ENGINE_EXPORT virtual bool Camera_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield );
	ENGINE_EXPORT virtual bool Camera_PointLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullScenePointLight* lights, uint32_t* outbitfield );
	ENGINE_EXPORT virtual bool Camera_SpotLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneFrustum* frusta, Mat4* inv_matrices, uint32_t* outbitfield );
	
	Array< Vec3 > m_aabbCache;
};


struct SGRX_DrawItem
{
	ENGINE_EXPORT SGRX_DrawItem();
	
	SGRX_MeshInstance* MI;
	uint16_t part;
	uint8_t type;
	
	/* frame cache */
	SGRX_DrawItemLight* _lightbuf_begin;
	SGRX_DrawItemLight* _lightbuf_end;
};

enum SGRX_LightingMode
{
	SGRX_LM_Unlit = 0, // no lighting at all
	SGRX_LM_Static = 1, // lightmaps
	SGRX_LM_Dynamic = 2, // constants
	SGRX_LM_Decal = 3, // lighting from vertex data
};


struct SGRX_VtxInputMapKey
{
	VertexShaderHandle vsh;
	VertexDeclHandle vdh;
	
	FINLINE bool operator == ( const SGRX_VtxInputMapKey& o ) const { return vsh == o.vsh && vdh == o.vdh; }
};
inline Hash HashVar( const SGRX_VtxInputMapKey& v )
{
	return HashVar( v.vsh.item ) ^ HashVar( v.vdh.item );
}

struct IF_GCC(ENGINE_EXPORT) SGRX_IVertexInputMapping : SGRX_RCRsrc
{
	ENGINE_EXPORT virtual ~SGRX_IVertexInputMapping();
	
	SGRX_VtxInputMapKey m_key;
};
typedef Handle< SGRX_IVertexInputMapping > VtxInputMapHandle;


struct SGRX_SRSData
{
	VertexShaderHandle VS;
	PixelShaderHandle PS;
	RenderStateHandle RS;
	VtxInputMapHandle VIM;
};

struct SGRX_MeshInstance : SGRX_RCXFItem
{
	ENGINE_EXPORT SGRX_MeshInstance( SGRX_Scene* s );
	ENGINE_EXPORT virtual ~SGRX_MeshInstance();
	ENGINE_EXPORT virtual void SetTransform( const Mat4& mtx );
	ENGINE_EXPORT void _Precache();
	
	FINLINE void Precache(){ if( m_mesh && m_mesh->m_vertexDecl ) _Precache(); }
	FINLINE bool CanDraw() const
	{
		return enabled && m_mesh && m_mesh->IsValid();
	}
	FINLINE bool IsSkinned() const { return skin_matrices.size() != 0; }
	
	FINLINE void OnUpdate(){ m_invalid = true; }
	FINLINE SGRX_IMesh* GetMesh() const { return m_mesh; }
	ENGINE_EXPORT void SetMesh( StringView path, bool mtls = true );
	ENGINE_EXPORT void SetMesh( MeshHandle mh, bool mtls = true );
	FINLINE void SetAllMtlFlags( uint8_t add, uint8_t rem = 0xff )
	{
		for( size_t i = 0; i < materials.size(); ++i )
		{
			uint8_t& flags = materials[ i ].flags;
			flags = ( flags & ~rem ) | add;
		}
		OnUpdate();
	}
	FINLINE void SetAllBlendModes( uint8_t bm )
	{
		for( size_t i = 0; i < materials.size(); ++i )
			materials[ i ].blendMode = bm;
		OnUpdate();
	}
	FINLINE void SetMITexture( int i, TextureHandle tex )
	{
		ASSERT( i >= 0 && i < 4 );
		for( size_t mid = 0; mid < materials.size(); ++mid )
			materials[ mid ].textures[ 8 + i ] = tex;
		OnUpdate();
	}
	FINLINE void SetLightingMode( SGRX_LightingMode lm )
	{
		m_lightingMode = lm;
		OnUpdate();
	}
	FINLINE SGRX_LightingMode GetLightingMode() const { return m_lightingMode; }
	FINLINE SGRX_SRSData& GetSRSData( uint8_t pass, size_t part )
	{
		return m_srsData[ m_drawItems.size() * pass + part ];
	}
	FINLINE uint16_t GetMaterialCount() const { return materials.size(); }
	FINLINE void SetMaterialCount( uint16_t n ){ materials.resize( n ); OnUpdate(); }
	FINLINE SGRX_Material& GetMaterial( uint16_t i ){ return materials[ i ]; }
	
	SGRX_Scene* _scene;
	
	MeshHandle m_mesh;
	IMeshRaycast* raycastOverride;
	void* userData;
	Mat4 matrix;
	Vec4 color;
	SGRX_LightingMode m_lightingMode;
	uint32_t layers;
	uint32_t enabled : 1;
	uint32_t allowStaticDecals : 1;
//	uint32_t dynamic : 1;
	uint8_t sortidx;
	
	Vec4 constants[ MAX_MI_CONSTANTS ];
	
	Array< Mat4 > skin_matrices;
	Array< SGRX_Material > materials;
	
	bool m_invalid;
	Array< SGRX_DrawItem > m_drawItems; // =*pass
	Array< SGRX_SRSData > m_srsData; // =*part*pass
};

struct MeshInstHandle : Handle< SGRX_MeshInstance >
{
	MeshInstHandle() : Handle(){}
	MeshInstHandle( const MeshInstHandle& h ) : Handle( h ){}
	MeshInstHandle( SGRX_MeshInstance* mi ) : Handle( mi ){}
};

struct SGRX_Camera
{
	ENGINE_EXPORT void Log( SGRX_Log& elog );
	ENGINE_EXPORT void UpdateViewMatrix();
	ENGINE_EXPORT void UpdateProjMatrix();
	ENGINE_EXPORT void UpdateMatrices();
	
	ENGINE_EXPORT Vec3 WorldToScreen( const Vec3& pos, bool* infront = NULL );
	ENGINE_EXPORT bool GetCursorRay( float x, float y, Vec3& pos, Vec3& dir ) const;
	
	Vec3 position;
	Vec3 direction;
	Vec3 updir;
	float angle;
	float aspect;
	float aamix;
	float znear;
	float zfar;
	
	Mat4 mView;
	Mat4 mProj;
	Mat4 mInvView;
};

ENGINE_EXPORT SGRX_Log& operator << ( SGRX_Log& log, const SGRX_Camera& cam );

struct SGRX_Viewport
{
	int x0, y0, x1, y1;
};

ENGINE_EXPORT uint32_t SGRX_FindOrAddVertex( ByteArray& vertbuf, size_t searchoffset, size_t& writeoffset, const uint8_t* vertex, size_t vertsize );
ENGINE_EXPORT void SGRX_DoIndexTriangleMeshVertices( UInt32Array& indices, ByteArray& vertices, size_t offset, size_t stride );
struct IF_GCC(ENGINE_EXPORT) SGRX_ProjectionMeshProcessor : IProcessor
{
	ByteArray* outVertices;
	UInt32Array* outIndices;
	Mat4 viewProjMatrix;
	float invZNearToZFar;
	
	SGRX_ProjectionMeshProcessor( ByteArray* verts, UInt32Array* indices, const Mat4& mtx, float zn2zf );
	ENGINE_EXPORT virtual void Process( void* data );
};

struct SceneRaycastInfo
{
	float factor;
	Vec3 normal;
	float u, v;
	int partID, triID, boneID;
	MeshInstHandle meshinst;
};
struct IF_GCC(ENGINE_EXPORT) SceneRaycastCallback
{
	virtual void AddResult( SceneRaycastInfo* info ) = 0;
};
struct IF_GCC(ENGINE_EXPORT) SceneRaycastCallback_Any : SceneRaycastCallback
{
	ENGINE_EXPORT SceneRaycastCallback_Any();
	ENGINE_EXPORT virtual void AddResult( SceneRaycastInfo* info );
	
	bool m_hit;
};
struct IF_GCC(ENGINE_EXPORT) SceneRaycastCallback_Closest : SceneRaycastCallback
{
	ENGINE_EXPORT SceneRaycastCallback_Closest();
	ENGINE_EXPORT virtual void AddResult( SceneRaycastInfo* info );
	
	bool m_hit;
	SceneRaycastInfo m_closest;
};
struct IF_GCC(ENGINE_EXPORT) SceneRaycastCallback_Sorting : SceneRaycastCallback
{
	ENGINE_EXPORT SceneRaycastCallback_Sorting( Array< SceneRaycastInfo >* sortarea );
	ENGINE_EXPORT virtual void AddResult( SceneRaycastInfo* info );
	
	Array< SceneRaycastInfo >* m_sortarea;
};

struct SGRX_RenderPass
{
	bool isShadowPass;
	bool isBasePass;
	uint8_t numPL;
	uint8_t numSL;
	StringView shader;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_Scene : SGRX_RefCounted
{
	ENGINE_EXPORT SGRX_Scene();
	ENGINE_EXPORT virtual ~SGRX_Scene();
	ENGINE_EXPORT void SetRenderPasses( SGRX_RenderPass* passes, int count );
	ENGINE_EXPORT void SetDefines( StringView defines );
	ENGINE_EXPORT MeshInstHandle CreateMeshInstance();
	ENGINE_EXPORT LightHandle CreateLight();
	
	ENGINE_EXPORT void OnUpdate();
	
	// finds closest match and returns only that
	ENGINE_EXPORT bool RaycastOne( const Vec3& from, const Vec3& to, SceneRaycastInfo* outinfo = NULL, uint32_t layers = 0xffffffff );
	// finds all matches and returns them in random order
	ENGINE_EXPORT void RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, uint32_t layers = 0xffffffff );
	// - sorted
	ENGINE_EXPORT void RaycastAllSort( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb,
		uint32_t layers = 0xffffffff, Array< SceneRaycastInfo >* tmpstore = NULL );
	
	ENGINE_EXPORT void GatherMeshes( const SGRX_Camera& cam, IProcessor* meshInstProc, uint32_t layers = 0xffffffff );
	ENGINE_EXPORT void GenerateProjectionMesh( const SGRX_Camera& cam, ByteArray& outverts, UInt32Array& outindices, uint32_t layers = 0xffffffff );
	
	ENGINE_EXPORT void DebugDraw_MeshRaycast( uint32_t layers = 0xffffffff );
	
	HashTable< SGRX_MeshInstance*, MeshInstHandle > m_meshInstances;
	HashTable< SGRX_Light*, LightHandle > m_lights;
	Array< SGRX_RenderPass > m_passes;
	StringView m_defines;
	Vec4 m_timevals; // temporary?
	MeshInstHandle m_projMeshInst;
	
	SGRX_CullScene* cullScene;
	SGRX_Camera camera;
	
	Vec3 fogColor;
	float fogHeightFactor;
	float fogDensity;
	float fogHeightDensity;
	float fogStartHeight;
	float fogMinDist;
	
	uint32_t clearColor;
	Vec3 ambientLightColor;
	Vec3 dirLightColor;
	Vec3 dirLightDir;
	
	TextureHandle skyTexture;
};

struct SceneHandle : Handle< SGRX_Scene >
{
	SceneHandle() : Handle(){}
	SceneHandle( const SceneHandle& h ) : Handle( h ){}
	SceneHandle( struct SGRX_Scene* sc ) : Handle( sc ){}
};

struct SGRX_SceneTree
{
	struct Node
	{
		size_t parent_id;
		String name;
		Mat4 transform;
	};
	struct Item
	{
		size_t node_id;
		XFItemHandle item;
	};
	
	size_t _NormalizeIndex( size_t id ){ return id < nodes.size() ? id : NOT_FOUND; }
	Node* FindNodeByName( const StringView& name ){ size_t id = FindNodeIDByName( name ); return id < nodes.size() ? &nodes[ id ] : NULL; }
	Node* FindNodeByPath( const StringView& path ){ size_t id = FindNodeIDByPath( path ); return id < nodes.size() ? &nodes[ id ] : NULL; }
	ENGINE_EXPORT size_t FindNodeIDByName( const StringView& name );
	ENGINE_EXPORT size_t FindNodeIDByPath( const StringView& path );
	ENGINE_EXPORT void UpdateTransforms();
	
	Array< Node > nodes;
	Array< Mat4 > transforms;
	Array< Item > items;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_LightTree
{
	struct Colors
	{
		Vec3 color[6]; // X,Y,Z / +,-
		void Clear(){ for( int i = 0; i < 6; ++i ) color[ i ] = V3(0); }
		template< class T > void Serialize( T& arch ){ for( int i = 0; i < 6; ++i ) arch << color[i]; }
	};
	struct Sample : Colors
	{
		Vec3 pos;
		template< class T > void Serialize( T& arch ){ arch << pos; Colors::Serialize( arch ); }
	};
	struct Node // size = 8(3+3+2) * 4(float/int32)
	{
		Vec3 bbmin;
		Vec3 bbmax;
		int32_t ch; // ch0 = ch, ch1 = ch + 1
		int32_t sdo; // sample data offset
		template< class T > void Serialize( T& arch ){ arch << bbmin << bbmax << ch << sdo; }
	};
	
	template< class T > void Serialize( T& arch )
	{
		arch << m_pos;
		arch << m_colors;
		arch << m_nodes;
		arch << m_sampidx;
	}
	
	ENGINE_EXPORT void SetSamples( Sample* samples, size_t count );
	ENGINE_EXPORT void SetSamplesUncolored( Vec3* samples, size_t count, const Vec3& col = V3(0.15f) );
	ENGINE_EXPORT void GetColors( Vec3 pos, Colors* out );
	ENGINE_EXPORT void _RegenBVH();
	
	// samples
	Array< Vec3 > m_pos;
	Array< Colors > m_colors;
	// BVH
	Array< Node > m_nodes;
	Array< int32_t > m_sampidx; // format: <count> [ <tri> x count ], ...
};

struct IF_GCC(ENGINE_EXPORT) SGRX_LightSampler
{
	ENGINE_EXPORT virtual void SampleLight( const Vec3& pos, Vec3& outcolor );
	ENGINE_EXPORT virtual void SampleLight( const Vec3& pos, Vec3 outcolors[6] ) = 0;
	ENGINE_EXPORT virtual void SampleLight( const Vec3& pos, const Vec3& dir, Vec3& outcolor );
	ENGINE_EXPORT void LightMeshAt( SGRX_MeshInstance* MI, Vec3 pos, int constoff = 10 );
	ENGINE_EXPORT void LightMesh( SGRX_MeshInstance* MI, Vec3 off = V3(0), int constoff = 10 );
	Vec3 SampleLight( const Vec3& pos ){ Vec3 out; SampleLight( pos, out ); return out; }
};

struct IF_GCC(ENGINE_EXPORT) SGRX_DummyLightSampler : SGRX_LightSampler
{
	ENGINE_EXPORT SGRX_DummyLightSampler();
	ENGINE_EXPORT virtual void SampleLight( const Vec3& pos, Vec3& outcolor );
	ENGINE_EXPORT virtual void SampleLight( const Vec3& pos, Vec3 outcolors[6] );
	ENGINE_EXPORT virtual void SampleLight( const Vec3& pos, const Vec3& dir, Vec3& outcolor );
	float defval;
};
ENGINE_EXPORT SGRX_DummyLightSampler& GR_GetDummyLightSampler();

struct IF_GCC(ENGINE_EXPORT) SGRX_LightTreeSampler : SGRX_LightSampler
{
	ENGINE_EXPORT virtual void SampleLight( const Vec3& pos, Vec3 outcolors[6] );
	SGRX_LightTree* m_lightTree;
};

enum EPrimitiveType
{
	PT_None,
	PT_Points,
	PT_Lines,
	PT_LineStrip,
	PT_Triangles,
	PT_TriangleStrip,
};


struct SGRX_ImmDrawData
{
	void* vertices;
	uint32_t vertexCount;
	EPrimitiveType primType;
	SGRX_IVertexDecl* vertexDecl;
	SGRX_IVertexInputMapping* vertexInputMapping;
	SGRX_IVertexShader* vertexShader;
	SGRX_IPixelShader* pixelShader;
	SGRX_IRenderState* renderState;
	Vec4* shdata;
	size_t shvcount;
	SGRX_ITexture* textures[ SGRX_MAX_TEXTURES ];
};


struct BatchRenderer
{
	struct Vertex
	{
		float x, y, z;
		uint32_t color;
		float u, v;
	};
	struct State
	{
		State() : primType(PT_None){}
		FINLINE bool IsDiff( const State& o ) const
		{
			if( pshader != o.pshader ) return true;
			if( vshader != o.vshader ) return true;
			if( primType != o.primType ) return true;
			for( int i = 0; i < SGRX_MAX_TEXTURES; ++i )
				if( textures[ i ] != o.textures[ i ] ) return true;
			return false;
		}
		
		TextureHandle textures[ SGRX_MAX_TEXTURES ];
		VertexShaderHandle vshader;
		PixelShaderHandle pshader;
		EPrimitiveType primType;
	};
	
	ENGINE_EXPORT BatchRenderer( struct IRenderer* r );
	~BatchRenderer(){ if( m_renderer ) Flush(); }
	
	ENGINE_EXPORT BatchRenderer& AddVertices( Vertex* verts, int count );
	ENGINE_EXPORT BatchRenderer& AddVertex( const Vertex& vert );
	FINLINE BatchRenderer& Col( float x ){ return Col( x, x, x, x ); }
	FINLINE BatchRenderer& Col( float x, float a ){ return Col( x, x, x, a ); }
	FINLINE BatchRenderer& Col( float r, float g, float b ){ return Col( r, g, b, 1.0f ); }
	FINLINE BatchRenderer& Col( const Vec3& col, float a = 1.0f ){ return Col( col.x, col.y, col.z, a ); }
	FINLINE BatchRenderer& Col( float r, float g, float b, float a ){ return Colb( COLOR_F2B( r ), COLOR_F2B( g ), COLOR_F2B( b ), COLOR_F2B( a ) ); }
	ENGINE_EXPORT BatchRenderer& Colb( uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff );
	FINLINE BatchRenderer& Colu( uint32_t c ){ return Colb( COLOR_EXTRACT_R( c ), COLOR_EXTRACT_G( c ), COLOR_EXTRACT_B( c ), COLOR_EXTRACT_A( c ) ); }
	FINLINE BatchRenderer& Tex( float x, float y ){ m_proto.u = x; m_proto.v = y; return *this; }
	FINLINE BatchRenderer& Pos( float x, float y, float z = 0.0f ){ m_proto.x = x; m_proto.y = y; m_proto.z = z; AddVertex( m_proto ); return *this; }
	FINLINE BatchRenderer& Pos( const Vec2& pos, float z = 0.0f ){ return Pos( pos.x, pos.y, z ); }
	FINLINE BatchRenderer& Pos( const Vec3& pos ){ return Pos( pos.x, pos.y, pos.z ); }
	
	ENGINE_EXPORT BatchRenderer& Prev( int i );
	ENGINE_EXPORT BatchRenderer& RawQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float z = 0 );
	ENGINE_EXPORT BatchRenderer& Quad( float x0, float y0, float x1, float y1, float z = 0 );
	ENGINE_EXPORT BatchRenderer& QuadFrame( float x0, float y0, float x1, float y1, float ix0, float iy0, float ix1, float iy1, float z = 0 );
	ENGINE_EXPORT BatchRenderer& VPQuad( SGRX_Viewport* vp, float z = 0 );
	FINLINE BatchRenderer& QuadWH( float x, float y, float w, float h, float z = 0 ){ return Quad( x, y, x + w, y + h, z ); }
	FINLINE BatchRenderer& Box( float x, float y, float w, float h, float z = 0 ){ w *= 0.5f; h *= 0.5f; return Quad( x - w, y - h, x + w, y + h, z ); }
	ENGINE_EXPORT BatchRenderer& TurnedBox( float x, float y, float dx, float dy, float z = 0 );
	ENGINE_EXPORT BatchRenderer& Button( Vec4 rect, Vec4 bdr, Vec4 texbdr, float z = 0 );
	ENGINE_EXPORT BatchRenderer& Poly( const void* data, int count, float z = 0, int stride = sizeof(Vec2) );
	ENGINE_EXPORT BatchRenderer& PolyOutline( const void* data, int count, float z = 0, int stride = sizeof(Vec2) );
	ENGINE_EXPORT BatchRenderer& Sprite( const Vec3& pos, const Vec3& dx, const Vec3& dy );
	ENGINE_EXPORT BatchRenderer& Sprite( const Vec3& pos, float sx, float sy );
	ENGINE_EXPORT BatchRenderer& TexLine( const Vec2& p0, const Vec2& p1, float rad );
	ENGINE_EXPORT BatchRenderer& CircleFill( float x, float y, float r, float z = 0, int verts = -1 );
	ENGINE_EXPORT BatchRenderer& CircleOutline( float x, float y, float r, float z = 0, int verts = -1 );
	ENGINE_EXPORT BatchRenderer& CircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts );
	ENGINE_EXPORT BatchRenderer& HalfCircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts );
	ENGINE_EXPORT BatchRenderer& SphereOutline( const Vec3& pos, float radius, int verts );
	ENGINE_EXPORT BatchRenderer& CapsuleOutline( const Vec3& pos, float radius, const Vec3& nrm, float ht, int verts );
	ENGINE_EXPORT BatchRenderer& ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, float angle, int verts );
	ENGINE_EXPORT BatchRenderer& ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, Vec2 angles, int verts );
	ENGINE_EXPORT BatchRenderer& AABB( const Vec3& bbmin, const Vec3& bbmax, const Mat4& transform = Mat4::Identity );
	ENGINE_EXPORT BatchRenderer& Tick( const Vec3& pos, float radius, const Mat4& transform = Mat4::Identity );
	
	ENGINE_EXPORT BatchRenderer& AAPoly( const Vec2* polydata, size_t polysize, float z = 0 );
	ENGINE_EXPORT BatchRenderer& AARect( float x0, float y0, float x1, float y1, float z = 0 );
	ENGINE_EXPORT BatchRenderer& AACircle( float x, float y, float r, float z = 0, int verts = -1 );
	ENGINE_EXPORT BatchRenderer& AAStroke( const Vec2* linedata, size_t linesize, float width, bool closed, float z = 0 );
	ENGINE_EXPORT BatchRenderer& AACircleOutline( float x, float y, float r, float width = 1, float z = 0, int verts = -1 );
	
	ENGINE_EXPORT bool CheckSetTexture( int i, const TextureHandle& tex );
	FINLINE bool CheckSetTexture( const TextureHandle& tex ){ return CheckSetTexture( 0, tex ); }
	FINLINE BatchRenderer& SetTexture( int i, const TextureHandle& tex ){ CheckSetTexture( i, tex ); return *this; }
	FINLINE BatchRenderer& SetTexture( const TextureHandle& tex ){ return SetTexture( 0, tex ); }
	FINLINE BatchRenderer& UnsetTexture( int i ){ return SetTexture( i, NULL ); }
	FINLINE BatchRenderer& UnsetTexture(){ return SetTexture( 0, NULL ); }
	ENGINE_EXPORT BatchRenderer& SetVertexShader( const VertexShaderHandle& shd );
	ENGINE_EXPORT BatchRenderer& SetPixelShader( const PixelShaderHandle& shd );
	FINLINE BatchRenderer& SetShader( const PixelShaderHandle& shd ){ return SetPixelShader( shd ); }
	ENGINE_EXPORT BatchRenderer& SetPrimitiveType( EPrimitiveType pt );
	ENGINE_EXPORT BatchRenderer& QuadsToTris();
	ENGINE_EXPORT BatchRenderer& Flush();
	ENGINE_EXPORT BatchRenderer& Reset();
	ENGINE_EXPORT void _UpdateDiff();
	ENGINE_EXPORT void _RecalcMatrices();
	
	FINLINE void ResetState()
	{
		RenderState.Init();
		RenderState.depthEnable = false;
		RenderState.cullMode = SGRX_RS_CullMode_None;
		RenderState.blendStates[0].blendEnable = true;
	}
	
	SGRX_RenderState RenderState;
	Array< Vec4 > ShaderData;
	
	IRenderer* m_renderer;
	VertexDeclHandle m_vertexDecl;
	VertexShaderHandle m_defVShader;
	PixelShaderHandle m_defPShader;
	TextureHandle m_whiteTex;
	State m_currState;
	State m_nextState;
	bool m_diff;
	Vertex m_proto;
	bool m_swapRB;
	Array< Vertex > m_verts;
	Array< Vec2 > m_polyCache;
	Mat4 worldMatrix;
	Mat4 viewMatrix;
	Mat4 invMatrix;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_LineSet
{
	struct Point
	{
		Vec3 pos;
		uint32_t color;
	};
	
	ENGINE_EXPORT void DrawLine( const Vec3& p1, const Vec3& p2, uint32_t col = COLOR_RGB(255,0,0) );
	ENGINE_EXPORT void Flush();
	
	Array< Point > m_lines;
};

struct ENGINE_EXPORT SGRX_PostDraw
{
	virtual void PostDraw() = 0;
};

struct ENGINE_EXPORT SGRX_DebugDraw
{
	virtual void DebugDraw() = 0;
};

struct SGRX_RenderScene
{
	SGRX_RenderScene( const Vec4& tv, const SceneHandle& sh, bool enablePP = true ) :
		timevals( tv ),
		scene( sh ),
		enablePostProcessing( enablePP ),
		viewport( NULL ),
		postdraw( NULL ),
		debugdraw( NULL )
	{}
	
	Vec4 timevals;
	SceneHandle scene;
	bool enablePostProcessing;
	SGRX_Viewport* viewport;
	SGRX_PostDraw* postdraw;
	SGRX_DebugDraw* debugdraw;
};

struct SGRX_FontSettings
{
	String font;
	int size;
	float letterspacing;
	float lineheight; // +px, -size factor
	
	float CalcLineHeight() const
	{
		return lineheight >= 0 ? lineheight : -lineheight * size;
	}
};

struct SGRX_GlyphInfo
{
	uint32_t glyph_kern_id;
	int16_t bmsizex;
	int16_t bmsizey;
	int16_t bmoffx;
	int16_t bmoffy;
	int16_t advx;
	int16_t advy;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IFont : SGRX_RCRsrc
{
	virtual void LoadGlyphInfo( int pxsize, uint32_t ch, SGRX_GlyphInfo* info ) = 0;
	virtual void GetGlyphBitmap( uint32_t* out, int pitch ) = 0;
	virtual int GetKerning( uint32_t ch1, uint32_t ch2 ){ return 0; }
	virtual int GetYOffset( int pxsize ){ return 0; }
};
typedef Handle< SGRX_IFont > FontHandle;



struct SGRX_RendererInfo
{
	bool compileShaders;
	StringView shaderCacheSfx;
	StringView shaderTypeDefine;
};


#define SGRX_RT_ClearColor 0x1
#define SGRX_RT_ClearDepth 0x2
#define SGRX_RT_ClearStencil 0x4
#define SGRX_RT_ClearAll 0x7

#define SGRX_RT_NONE uint16_t(-1)

#define SGRX_TY_Solid 0x1
#define SGRX_TY_Decal 0x2
#define SGRX_TY_Transparent 0x4

struct SGRX_RTClearInfo
{
	uint8_t flags;
	uint8_t clearStencil;
	uint32_t clearColor;
	float clearDepth;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IRenderControl
{
	ENGINE_EXPORT virtual ~SGRX_IRenderControl();
	ENGINE_EXPORT virtual void SetRenderTargets( SGRX_IDepthStencilSurface* dss, const SGRX_RTClearInfo& info, TextureHandle rts[4] ) = 0;
	ENGINE_EXPORT virtual void SortRenderItems( SGRX_Scene* scene ) = 0;
	ENGINE_EXPORT virtual void RenderShadows( SGRX_Scene* scene, uint8_t pass_id ) = 0;
	ENGINE_EXPORT virtual void RenderMeshes( SGRX_Scene* scene, uint8_t pass_id, int maxrepeat, uint8_t types, SGRX_MeshInstance** milist, size_t micount ) = 0;
	ENGINE_EXPORT virtual void RenderTypes( SGRX_Scene* scene, uint8_t pass_id, int maxrepeat, uint8_t types ) = 0;
	
	// shortcuts
	FINLINE void SetRenderTargets( SGRX_IDepthStencilSurface* dss, const SGRX_RTClearInfo& info,
		TextureHandle rt0 = NULL, TextureHandle rt1 = NULL, TextureHandle rt2 = NULL, TextureHandle rt3 = NULL )
	{
		TextureHandle rts[4] = { rt0, rt1, rt2, rt3 };
		SetRenderTargets( dss, info, rts );
	}
	FINLINE void SetRenderTargets( SGRX_IDepthStencilSurface* dss, uint8_t flags, uint8_t clearStencil, uint32_t clearColor, float clearDepth,
		TextureHandle rt0 = NULL, TextureHandle rt1 = NULL, TextureHandle rt2 = NULL, TextureHandle rt3 = NULL )
	{
		SGRX_RTClearInfo info = { flags, clearStencil, clearColor, clearDepth };
		TextureHandle rts[4] = { rt0, rt1, rt2, rt3 };
		SetRenderTargets( dss, info, rts );
	}
};

struct IF_GCC(ENGINE_EXPORT) IGame
{
	virtual bool OnConfigure( int argc, char** argv ){ return true; }
	virtual bool OnInitialize(){ return true; }
	virtual void OnDestroy(){}
	virtual void OnEvent( const Event& e ){}
	virtual void OnTick( float dt, uint32_t gametime ) = 0;
	
	ENGINE_EXPORT virtual void OnDrawScene( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info );
	ENGINE_EXPORT virtual void OnDrawSceneGeom( SGRX_IRenderControl* ctrl, SGRX_RenderScene& info,
		TextureHandle rtt, DepthStencilSurfHandle dss );
	
	ENGINE_EXPORT virtual void OnMakeRenderState( const SGRX_RenderPass& pass, const SGRX_Material& mtl, SGRX_RenderState& out );
	ENGINE_EXPORT virtual void OnLoadMtlShaders( const SGRX_RenderPass& pass, const StringView& defines, const SGRX_Material& mtl,
		SGRX_MeshInstance* MI, VertexShaderHandle& VS, PixelShaderHandle& PS );
	ENGINE_EXPORT virtual bool OnLoadTexture( const StringView& key, ByteArray& outdata, uint32_t& outusageflags );
	ENGINE_EXPORT virtual void GetShaderCacheFilename( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, String& name );
	ENGINE_EXPORT virtual bool GetCompiledShader( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, ByteArray& outdata );
	ENGINE_EXPORT virtual bool SetCompiledShader( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, const ByteArray& data );
	ENGINE_EXPORT virtual bool OnLoadShader( const SGRX_RendererInfo& rinfo, const StringView& key, String& outdata );
	ENGINE_EXPORT virtual bool OnLoadShaderFile( const SGRX_RendererInfo& rinfo, const StringView& path, String& outdata );
	ENGINE_EXPORT virtual bool ParseShaderIncludes( const SGRX_RendererInfo& rinfo, const StringView& path, String& outdata );
	ENGINE_EXPORT virtual bool OnLoadMesh( const StringView& key, ByteArray& outdata );
};



ENGINE_EXPORT int GR_GetWidth();
ENGINE_EXPORT int GR_GetHeight();

ENGINE_EXPORT TextureHandle GR_CreateTexture( int width, int height, int format, uint32_t flags, int mips );
ENGINE_EXPORT TextureHandle GR_GetTexture( const StringView& path );
ENGINE_EXPORT TextureHandle GR_CreateRenderTexture( int width, int height, int format );
ENGINE_EXPORT TextureHandle GR_GetRenderTarget( int width, int height, int format, int extra );
ENGINE_EXPORT DepthStencilSurfHandle GR_CreateDepthStencilSurface( int width, int height, int format );
ENGINE_EXPORT DepthStencilSurfHandle GR_GetDepthStencilSurface( int width, int height, int format, int extra = 0 );
ENGINE_EXPORT VertexShaderHandle GR_GetVertexShader( const StringView& path );
ENGINE_EXPORT PixelShaderHandle GR_GetPixelShader( const StringView& path );
ENGINE_EXPORT RenderStateHandle GR_GetRenderState( const SGRX_RenderState& state );
ENGINE_EXPORT VertexDeclHandle GR_GetVertexDecl( const StringView& vdecl );
ENGINE_EXPORT VtxInputMapHandle GR_GetVertexInputMapping( SGRX_IVertexShader* vs, SGRX_IVertexDecl* vd );
ENGINE_EXPORT MeshHandle GR_CreateMesh();
ENGINE_EXPORT MeshHandle GR_GetMesh( const StringView& path );
ENGINE_EXPORT void GR_PreserveResourcePtr( SGRX_RefCounted* rsrc );
template< class T > void GR_PreserveResource( T& handle ){ GR_PreserveResourcePtr( handle.item ); }

ENGINE_EXPORT SceneHandle GR_CreateScene();
ENGINE_EXPORT void GR_RenderScene( SGRX_RenderScene& info );
ENGINE_EXPORT RenderStats& GR_GetRenderStats();

ENGINE_EXPORT void GR2D_SetWorldMatrix( const Mat4& mtx );
ENGINE_EXPORT void GR2D_SetViewMatrix( const Mat4& mtx );
ENGINE_EXPORT void GR2D_SetViewport( int x0, int y0, int x1, int y1 );
ENGINE_EXPORT void GR2D_UnsetViewport();
ENGINE_EXPORT void GR2D_SetScissorRect( int x0, int y0, int x1, int y1 );
ENGINE_EXPORT void GR2D_UnsetScissorRect();

ENGINE_EXPORT void GR2D_SetColor( float r, float g, float b, float a = 1.0f );
inline void GR2D_SetColor( float x, float a ){ GR2D_SetColor( x, x, x, a ); }
inline void GR2D_SetColor( float x ){ GR2D_SetColor( x, x, x, x ); }

ENGINE_EXPORT bool GR2D_LoadFont( const StringView& key, const StringView& path );
ENGINE_EXPORT bool GR2D_LoadSVGIconFont( const StringView& key, const StringView& path );
ENGINE_EXPORT FontHandle GR2D_GetFont( const StringView& key );
ENGINE_EXPORT void GR2D_GetFontSettings( SGRX_FontSettings* settings );
ENGINE_EXPORT void GR2D_SetFontSettings( SGRX_FontSettings* settings );
ENGINE_EXPORT bool GR2D_SetFont( const StringView& name, int pxsize );
ENGINE_EXPORT void GR2D_SetLetterSpacing( float lsp );
ENGINE_EXPORT void GR2D_SetLineHeight( float lht );
ENGINE_EXPORT void GR2D_SetTextCursor( const Vec2& pos );
inline void GR2D_SetTextCursor( float x, float y ){ GR2D_SetTextCursor( V2( x, y ) ); }
ENGINE_EXPORT Vec2 GR2D_GetTextCursor();
ENGINE_EXPORT int GR2D_GetTextLength( const StringView& text );
ENGINE_EXPORT int GR2D_DrawTextLine( const StringView& text );
ENGINE_EXPORT int GR2D_DrawTextLine( float x, float y, const StringView& text );
#define HALIGN_LEFT 0
#define HALIGN_CENTER 1
#define HALIGN_RIGHT 2
#define VALIGN_TOP 0
#define VALIGN_CENTER 1
#define VALIGN_BOTTOM 2
ENGINE_EXPORT int GR2D_DrawTextLine( const StringView& text, int halign, int valign );
ENGINE_EXPORT int GR2D_DrawTextLine( float x, float y, const StringView& text, int halign, int valign );
ENGINE_EXPORT void GR2D_DrawTextRect( int x0, int y0, int x1, int y1,
	const StringView& text, int halign, int valign );

ENGINE_EXPORT BatchRenderer& GR2D_GetBatchRenderer();

ENGINE_EXPORT void SGRX_Swap();

extern "C" ENGINE_EXPORT int SGRX_EntryPoint( int argc, char** argv, int debug );

