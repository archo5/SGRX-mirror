

#pragma once

#include <engine.hpp>
#include <enganim.hpp>
#include <pathfinding.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#ifdef GFW_BUILDING
#  define GFW_EXPORT __declspec(dllexport)
#else
#  define GFW_EXPORT __declspec(dllimport)
#endif

#define EXP_STRUCT struct SGS_CPPBC_IGNORE(IF_GCC(GFW_EXPORT))


EXP_STRUCT SGSTextureHandle
{
	SGS_OBJECT;
	
	SGSTextureHandle( TextureHandle _h ) : h(_h){}
	SGS_PROPERTY_FUNC( READ SOURCE h->m_key ) SGS_ALIAS( StringView key );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().type ) SGS_ALIAS( int typeID );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().mipcount ) SGS_ALIAS( int mipmapCount );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().width ) SGS_ALIAS( int width );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().height ) SGS_ALIAS( int height );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().depth ) SGS_ALIAS( int depth );
	SGS_PROPERTY_FUNC( READ SOURCE h.GetInfo().format ) SGS_ALIAS( int formatID );
	SGS_PROPERTY_FUNC( READ SOURCE h->m_isRenderTexture ) SGS_ALIAS( bool isRenderTexture );
	
	TextureHandle h;
};

template<> inline void sgs_PushVar<TextureHandle>( SGS_CTX, const TextureHandle& v )
{
	if( !v )
		sgs_PushNull( C );
	else
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


EXP_STRUCT SGSMeshHandle
{
	SGS_OBJECT;
	
	SGSMeshHandle( MeshHandle _h ) : h(_h){}
	SGS_PROPERTY_FUNC( READ SOURCE h->m_key ) SGS_ALIAS( StringView key );
	SGS_PROPERTY_FUNC( READ SOURCE h->m_dataFlags ) SGS_ALIAS( uint32_t dataFlags );
	SGS_PROPERTY_FUNC( READ SOURCE h->m_meshParts.size() ) SGS_ALIAS( int numParts );
	SGS_PROPERTY_FUNC( READ SOURCE h->m_numBones ) SGS_ALIAS( int numBones );
	SGS_PROPERTY_FUNC( READ SOURCE h->m_boundsMin ) SGS_ALIAS( Vec3 boundsMin );
	SGS_PROPERTY_FUNC( READ SOURCE h->m_boundsMax ) SGS_ALIAS( Vec3 boundsMax );
	SGS_PROPERTY_FUNC( READ SOURCE h->GetBufferVertexCount() ) SGS_ALIAS( uint32_t totalVertexCount );
	SGS_PROPERTY_FUNC( READ SOURCE h->GetBufferIndexCount() ) SGS_ALIAS( uint32_t totalIndexCount );
	
	MeshHandle h;
};

template<> inline void sgs_PushVar<MeshHandle>( SGS_CTX, const MeshHandle& v )
{
	if( !v )
		sgs_PushNull( C );
	else
		SGS_CREATECLASS( C, NULL, SGSMeshHandle, ( v ) );
}
template<> struct sgs_GetVar<MeshHandle>
{
	MeshHandle operator () ( SGS_CTX, sgs_StkIdx item )
	{
		if( sgs_IsObject( C, item, SGSMeshHandle::_sgs_interface ) )
			return ((SGSMeshHandle*)sgs_GetObjectData( C, item ))->h;
		return NULL;
	}
};


GFW_EXPORT void GFWRegisterCore( SGS_CTX );

