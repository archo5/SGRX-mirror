

#pragma once
#ifdef INCLUDE_REAL_SDL
#  include <SDL2/SDL.h>
#  include <SDL2/SDL_syswm.h>
#elif defined NO_SDL
struct SDL_Event;
#else
#  include "sdl_events_min.h"
#endif
#include "utils.hpp"


struct EXPORT SGRX_Log
{
	struct Separator
	{
		Separator( const char* s ) : sep( s ){}
		const char* sep;
	};
	enum EMod_Partial { Mod_Partial };
	enum ESpec_Date { Spec_Date };
	
	bool end_newline;
	bool need_sep;
	const char* sep;
	
	SGRX_Log();
	~SGRX_Log();
	void prelog();
	
	SGRX_Log& operator << ( const Separator& );
	SGRX_Log& operator << ( EMod_Partial );
	SGRX_Log& operator << ( ESpec_Date );
	SGRX_Log& operator << ( int8_t );
	SGRX_Log& operator << ( uint8_t );
	SGRX_Log& operator << ( int16_t );
	SGRX_Log& operator << ( uint16_t );
	SGRX_Log& operator << ( int32_t );
	SGRX_Log& operator << ( uint32_t );
	SGRX_Log& operator << ( int64_t );
	SGRX_Log& operator << ( uint64_t );
	SGRX_Log& operator << ( float );
	SGRX_Log& operator << ( double );
	SGRX_Log& operator << ( const void* );
	SGRX_Log& operator << ( const char* );
	SGRX_Log& operator << ( const StringView& );
	SGRX_Log& operator << ( const Vec2& );
	SGRX_Log& operator << ( const Vec3& );
	SGRX_Log& operator << ( const Mat4& );
};
#define LOG SGRX_Log()
#define LOG_ERROR SGRX_Log() << "ERROR: "
#define PARTIAL_LOG SGRX_Log::Mod_Partial
#define LOG_DATE SGRX_Log::Spec_Date
#define LOG_SEP( x ) SGRX_Log::Separator( x )


typedef SDL_Event Event;


struct EXPORT Command
{
	Command( const StringView& sv, float thr = 0.1f ) :
		name(sv),
		threshold(thr),
		value(0), prev_value(0),
		state(false), prev_state(false)
	{}
	virtual ~Command(){}
	virtual void OnChangeState(){}
	
	FINLINE bool IsPressed() const { return state && !prev_state; }
	FINLINE bool IsReleased() const { return !state && prev_state; }
	
	void _SetState( float x );
	void _Advance();
	
	StringView name;
	float threshold;
	float value, prev_value;
	bool state, prev_state;
};

struct EXPORT Command_Func : Command
{
	virtual void Function() = 0;
	void OnChangeState(){ if( state && !state ) Function(); }
};

EXPORT void Game_RegisterAction( Command* cmd );
EXPORT void Game_UnregisterAction( Command* cmd );
EXPORT void Game_BindKeyToAction( uint32_t key, Command* cmd );
EXPORT void Game_BindKeyToAction( uint32_t key, const StringView& cmd );
EXPORT Vec2 Game_GetCursorPos();


struct EXPORT IScreen
{
	virtual ~IScreen(){}
	virtual void OnStart(){}
	virtual void OnEnd(){}
	virtual bool OnEvent( const Event& e ) = 0; // return value - whether event is inhibited
	virtual bool Draw( float delta ) = 0; // return value - whether to remove
};

EXPORT bool Game_HasOverlayScreen( IScreen* screen );
EXPORT void Game_AddOverlayScreen( IScreen* screen );
EXPORT void Game_RemoveOverlayScreen( IScreen* screen );

#define TEXFLAGS_SRGB    0x01
#define TEXFLAGS_HASMIPS 0x02
#define TEXFLAGS_LERP_X  0x04
#define TEXFLAGS_LERP_Y  0x08
#define TEXFLAGS_CLAMP_X 0x10
#define TEXFLAGS_CLAMP_Y 0x20

struct EXPORT IGame
{
	virtual void OnConfigure( int argc, char** argv ){}
	virtual void OnInitialize(){}
	virtual void OnDestroy(){}
	virtual void OnTick( float dt, uint32_t gametime ) = 0;
	
	virtual bool OnLoadTexture( const StringView& path, ByteArray& outdata, uint32_t& outusageflags );
};


//
// RENDERER DATA
//

struct EXPORT RenderSettings
{
	int width;
	int height;
	bool fullscreen;
	bool windowed_fullscreen;
	bool vsync;
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

struct TextureInfo
{
	uint32_t flags; /* TEXFLAGS */
	int type; /* TEXTYPE */
	int width;
	int height;
	int depth;
	int format; /* TEXFORMAT */
	int mipcount;
};

struct EXPORT SGRX_Texture
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) Destroy(); }
	
	const TextureInfo& GetInfo();
	bool UploadRGBA8Part( void* data, int mip, int w, int h, int x, int y );
	
	struct ITexture* m_texture;
	int32_t m_refcount;
	String m_key;
	
private:
	void Destroy();
};

struct EXPORT TextureHandle : Handle< SGRX_Texture >
{
	TextureHandle() : Handle(){}
	TextureHandle( const TextureHandle& h ) : Handle( h ){}
	TextureHandle( SGRX_Texture* tex ) : Handle( tex ){}
	
	const TextureInfo& GetInfo();
	bool UploadRGBA8Part( void* data, int mip = 0, int w = -1, int h = -1, int x = 0, int y = 0 );
};

enum EPrimitiveType
{
	PT_None,
	PT_Points,
	PT_Lines,
	PT_LineStrip,
	PT_Triangles,
	PT_TriangleStrip,
	PT_TriangleFan,
};

struct EXPORT BatchRenderer
{
	struct Vertex
	{
		float x, y, z;
		uint32_t color;
		float u, v;
	};
	
	BatchRenderer( struct IRenderer* r );
	~BatchRenderer(){ if( m_renderer ) Flush(); }
	
	BatchRenderer& AddVertices( Vertex* verts, int count );
	BatchRenderer& AddVertex( const Vertex& vert );
	FINLINE BatchRenderer& Col( float x ){ return Col( x, x, x, x ); }
	FINLINE BatchRenderer& Col( float x, float a ){ return Col( x, x, x, a ); }
	FINLINE BatchRenderer& Col( float r, float g, float b ){ return Col( r, g, b, 1.0f ); }
	FINLINE BatchRenderer& Col( float r, float g, float b, float a ){ return Colb( r * 255, g * 255, b * 255, a * 255 ); }
	BatchRenderer& Colb( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	FINLINE BatchRenderer& Tex( float x, float y ){ m_proto.u = x; m_proto.v = y; return *this; }
	FINLINE BatchRenderer& Pos( float x, float y, float z = 0.0f ){ m_proto.x = x; m_proto.y = y; m_proto.z = z; AddVertex( m_proto ); return *this; }
	
	BatchRenderer& Prev( int i );
	BatchRenderer& Quad( float x0, float y0, float x1, float y1 );
	FINLINE BatchRenderer& QuadWH( float x, float y, float w, float h ){ return Quad( x, y, x + w, y + h ); }
	FINLINE BatchRenderer& Box( float x, float y, float w, float h ){ w *= 0.5f; h *= 0.5f; return Quad( x - w, y - h, x + w, y + h ); }
	
	BatchRenderer& SetPrimitiveType( EPrimitiveType pt );
	bool CheckSetTexture( const TextureHandle& tex );
	BatchRenderer& SetTexture( const TextureHandle& tex );
	BatchRenderer& UnsetTexture(){ return SetTexture( NULL ); }
	BatchRenderer& Flush();
	
	IRenderer* m_renderer;
	TextureHandle m_texture;
	EPrimitiveType m_primType;
	Vertex m_proto;
	bool m_swapRB;
	Array< Vertex > m_verts;
};


EXPORT int GR_GetWidth();
EXPORT int GR_GetHeight();

EXPORT TextureHandle GR_CreateTexture( int width, int height, int format, int mips = 1 );
EXPORT TextureHandle GR_GetTexture( const StringView& path );

EXPORT void GR2D_SetWorldMatrix( const Mat4& mtx );
EXPORT void GR2D_SetViewMatrix( const Mat4& mtx );
EXPORT bool GR2D_SetFont( const StringView& name, int pxsize );
EXPORT void GR2D_SetTextCursor( float x, float y );
EXPORT Vec2 GR2D_GetTextCursor();
EXPORT int GR2D_DrawTextLine( const StringView& text );
EXPORT int GR2D_DrawTextLine( float x, float y, const StringView& text );

EXPORT BatchRenderer& GR2D_GetBatchRenderer();


extern "C" EXPORT int SGRX_EntryPoint( int argc, char** argv, int debug );

