

#pragma once

#include <engine.hpp>
#include <enganim.hpp>
#include <particlesys.hpp>
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

#define GFW_DEFINE_HANDLE_CPPBC_CONV( NTYPE, SGSTYPE ) \
	template<> inline void sgs_PushVar<NTYPE>( SGS_CTX, const NTYPE& v ) \
	{ \
		if( !v ) \
			sgs_PushNull( C ); \
		else \
			SGS_CREATECLASS( C, NULL, SGSTYPE, ( v ) ); \
	} \
	template<> struct sgs_GetVar<NTYPE> \
	{ \
		NTYPE operator () ( SGS_CTX, sgs_StkIdx item ) \
		{ \
			if( sgs_IsObject( C, item, SGSTYPE::_sgs_interface ) ) \
				return ((SGSTYPE*)sgs_GetObjectData( C, item ))->h; \
			return NULL; \
		} \
	};


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
GFW_DEFINE_HANDLE_CPPBC_CONV( TextureHandle, SGSTextureHandle );


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
GFW_DEFINE_HANDLE_CPPBC_CONV( MeshHandle, SGSMeshHandle );


EXP_STRUCT SGSMeshInstHandle
{
	SGS_OBJECT;
	
	SGSMeshInstHandle( MeshInstHandle _h ) : h(_h){}
	SGS_PROPERTY_FUNC( READ WRITE SOURCE h->layers ) SGS_ALIAS( uint32_t layers );
	SGS_PROPERTY_FUNC( READ WRITE SOURCE h->enabled ) SGS_ALIAS( bool enabled );
	SGS_PROPERTY_FUNC( READ WRITE SOURCE h->allowStaticDecals ) SGS_ALIAS( bool allowStaticDecals );
	SGS_PROPERTY_FUNC( READ WRITE SOURCE h->sortidx ) SGS_ALIAS( uint32_t sortidx );
	SGS_PROPERTY_FUNC( READ WRITE SOURCE h->matrix ) SGS_ALIAS( Mat4 transform );
	MeshHandle sgsGetMesh() const { return h->GetMesh(); }
	SGS_PROPERTY_FUNC( READ sgsGetMesh WRITE h->SetMesh ) SGS_ALIAS( MeshHandle mesh );
	int sgsGetLightingMode() const { return h->GetLightingMode(); }
	void sgsSetLightingMode( int m ){ h->SetLightingMode( (SGRX_LightingMode) m ); }
	SGS_PROPERTY_FUNC( READ sgsGetLightingMode WRITE sgsSetLightingMode ) SGS_ALIAS( int lightingMode );
	SGS_PROPERTY_FUNC( READ h->GetMaterialCount WRITE h->SetMaterialCount ) SGS_ALIAS( uint16_t materialCount );
	
	SGS_METHOD void SetMesh( sgsVariable handleOrPath, bool mtls )
	{
		if( sgs_StackSize( C ) < 2 ) mtls = true;
		if( handleOrPath.is_string() )
		{
			sgsString s = handleOrPath.get_string();
			h->SetMesh( StringView( s.c_str(), s.size() ), mtls );
			return;
		}
		h->SetMesh( sgs_GetVar<MeshHandle>()( C, 0 ) );
	}
	SGS_METHOD void SetMITexture( int i, TextureHandle tex )
	{
		if( i < 0 || i >= 4 )
		{
			sgs_Msg( C, SGS_WARNING, "slot %d out of bounds [0;4)", i );
			return;
		}
		h->SetMITexture( i, tex );
	}
	
	MeshInstHandle h;
};
GFW_DEFINE_HANDLE_CPPBC_CONV( MeshInstHandle, SGSMeshInstHandle );


#define ForceType_Velocity PFT_Velocity
#define ForceType_Impulse PFT_Impulse
#define ForceType_Acceleration PFT_Acceleration
#define ForceType_Force PFT_Force


enum MatrixMode
{
	MM_Relative = 0,
	MM_Absolute = 1,
};

#define MoveMask_Position 0x01
#define MoveMask_Rotation 0x02
#define MoveMask_Scale    0x04
#define MoveMask_ALL      0x07


GFW_EXPORT void GFWRegisterCore( SGS_CTX );

