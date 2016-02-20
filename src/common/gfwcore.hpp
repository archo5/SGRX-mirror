

#pragma once

#include <engine.hpp>
#include <enganim.hpp>
#include <pathfinding.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>


struct SGSTextureHandle
{
	SGS_OBJECT;
	
	SGSTextureHandle( TextureHandle _h ) : h(_h){}
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().type ) SGS_ALIAS( int typeID );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().mipcount ) SGS_ALIAS( int mipmapCount );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().width ) SGS_ALIAS( int width );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().height ) SGS_ALIAS( int height );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().depth ) SGS_ALIAS( int depth );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().format ) SGS_ALIAS( int formatID );
	SGS_PROPERTY_FUNC( READ SOURCE h->m_isRenderTexture ) SGS_ALIAS( bool isRenderTexture );
	SGS_PROPERTY_FUNC( READ SOURCE h->m_key ) SGS_ALIAS( StringView key );
	
	TextureHandle h;
};

template<> inline void sgs_PushVar<TextureHandle>( SGS_CTX, const TextureHandle& v )
{
	SGS_CREATECLASS( C, NULL, SGSTextureHandle, ( v ) );
}
template<> struct sgs_GetVar<TextureHandle>
{
	TextureHandle operator () ( SGS_CTX, sgs_StkIdx item )
	{
		if( sgs_IsObject( C, item, SGSTextureHandle::_sgs_interface ) )
			return ((SGSTextureHandle*)sgs_GetObjectData( C, item ))->h;
		return NULL;
	}
};

void GFWRegisterCore( SGS_CTX );

