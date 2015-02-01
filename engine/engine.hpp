

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
	SGRX_Log& operator << ( const void* );
	SGRX_Log& operator << ( const char* );
	SGRX_Log& operator << ( const StringView& );
};
#define LOG SGRX_Log()
#define LOG_ERROR SGRX_Log() << "ERROR: "
#define PARTIAL_LOG SGRX_Log::Mod_Partial
#define LOG_DATE SGRX_Log::Spec_Date
#define LOG_SEP( x ) SGRX_Log::Separator( x )


typedef SDL_Event Event;


struct IScreen
{
	virtual ~IScreen(){}
	virtual bool OnEvent( const Event& e ) = 0; // return value - whether event is inhibited
	virtual bool Draw( float delta ) = 0; // return value - whether to remove
};

bool Game_HasOverlayScreen( IScreen* screen );
void Game_AddOverlayScreen( IScreen* screen );
void Game_RemoveOverlayScreen( IScreen* screen );

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


struct EXPORT RenderSettings
{
	int width;
	int height;
	bool fullscreen;
	bool windowed_fullscreen;
	bool vsync;
};

struct EXPORT SGRX_Texture
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ if( --m_refcount <= 0 ) Destroy(); }
	
	struct ITexture* m_texture;
	int32_t m_refcount;
	String m_key;
	
private:
	void Destroy();
};
typedef Handle< SGRX_Texture > TextureHandle;

EXPORT TextureHandle GR_GetTexture( const StringView& path );

void R2D_SetColor( float* v4f );
FINLINE void R2D_SetColor( float r, float g, float b, float a = 1.0f ){ float rgba[4] = { r, g, b, a }; R2D_SetColor( rgba ); }
void R2D_DrawRect( float x0, float y0, float x1, float y1 );
void R2D_DrawText( const char* text, int size );




extern "C" EXPORT int SGRX_EntryPoint( int argc, char** argv, int debug );

