

#pragma once
#include "compiler.hpp"
#include "level.hpp"


//
// LEVEL GRAPHICS CONTAINER
//

typedef LevelCache::Vertex LCVertex;

// note:
//  2-step invalidation
// some changes in meshes/surfaces invalidate lights also
// invalidated lights must invalidate all intersecting meshes as well

#define LGC_CHANGE_ALL 0xffffffff

#define LGC_CHANGE_XFORM 0x01
#define LGC_CHANGE_RSPEC 0x02

#define LGC_MESH_CHANGE_PATH 0x04

struct EdLGCRenderInfo
{
	EdLGCRenderInfo() :
		rflags(LM_MESHINST_SOLID|LM_MESHINST_CASTLMS),
		lmdetail(1),
		decalLayer(0){}
	
	// rspec
	uint32_t rflags; // LM_MESHINST_*
	float lmdetail;
	uint8_t decalLayer;
	
	int RIDiff( const EdLGCRenderInfo& other ) const
	{
		if( rflags != other.rflags ) return 2;
		if( lmdetail != other.lmdetail ) return 1;
		if( decalLayer != other.decalLayer ) return -1;
		return 0;
	}
};

struct EdLGCDrawableInfo : EdLGCRenderInfo
{
	EdLGCDrawableInfo() : xform(Mat4::Identity){}
	
	Mat4 xform;
};

struct EdLGCSolidInfo
{
	EdLGCSolidInfo() : planes(NULL), pcount(0){}
	
	Vec4* planes;
	uint32_t pcount;
};

struct EdLGCMeshInfo : EdLGCDrawableInfo
{
	String path;
};

#define LGC_SURF_CHANGE_VIDATA 0x04
#define LGC_SURF_CHANGE_MTLDATA 0x08
#define LGC_SURF_CHANGE_SOLID 0x10
#define LGC_SURF_CHANGE_LMPARENT 0x20

struct EdLGCSurfaceInfo : EdLGCDrawableInfo
{
	EdLGCSurfaceInfo() :
		vdata(NULL), vcount(0),
		idata(NULL), icount(0),
		lmsize(V2(0)){}
	
	// vidata
	LCVertex* vdata;
	uint32_t vcount;
	uint16_t* idata;
	uint32_t icount;
	Vec2 lmsize;
	// mtldata
	String mtlname;
	// solid
	SGRX_GUID solid_guid;
	// lmparent
	SGRX_GUID lmparent_guid;
};

#define LGC_LIGHT_CHANGE_SPEC 0x04

struct EdLGCLightInfo : LC_Light
{
	EdLGCLightInfo()
	{
		type = LM_LIGHT_POINT;
		pos = V3(0);
		dir = V3(0,0,-1);
		up = V3(0,1,0);
		range = 0;
		power = 1;
		light_radius = 0;
		color = V3(1);
		num_shadow_samples = 1;
		flaresize = 0;
		flareoffset = V3(0);
		innerangle = 30;
		outerangle = 45;
		spotcurve = 1;
	}
	
	bool IntersectsAABB( Vec3 bbmin, Vec3 bbmax, Mat4 mtx ) const
	{
		Mat4 invmtx = Mat4::Identity;
		mtx.InvertTo( invmtx );
		Vec3 locpos = invmtx.TransformPos( pos );
		Vec3 inbbpos = Vec3::Min( Vec3::Max( locpos, bbmin ), bbmax );
		Vec3 locdist = inbbpos - locpos;
		Vec3 world_dist = mtx.TransformNormal( locdist );
		return world_dist.LengthSq() < range;
	}
};

struct EdLevelGraphicsCont : SGRX_IEventHandler
{
	struct PrevMeshData
	{
		Vec3 bbmin;
		Vec3 bbmax;
		Mat4 transform;
	};
	struct Solid
	{
		Array< Vec4 > planes;
	};
	struct Surface
	{
		EdLGCRenderInfo info;
		MeshInstHandle meshInst;
		Array< LCVertex > vertices;
		Array< uint16_t > indices;
		Vec2 lmsize;
		String mtlname;
		SGRX_GUID solid_guid;
		SGRX_GUID lmparent_guid;
		
		void RecalcTangents();
	};
	struct Mesh
	{
		EdLGCMeshInfo info;
		struct MeshResource* ent;
	};
	struct Light
	{
		EdLGCLightInfo info;
		struct LightResource* ent;
	};
	struct LMap : SGRX_RefCounted
	{
		uint16_t width;
		uint16_t height;
		Array< Vec3 > lmdata;
		Array< Vec4 > nmdata;
		TextureHandle texture;
		TextureHandle nmtexture;
		bool invalid;
		bool alr_invalid; // after lightmap rendering - no need to serialize
		
		template< class T > void Serialize( T& arch )
		{
			arch << width;
			arch << height;
			if( T::IsReader )
				lmdata.resize( size_t(width) * size_t(height) );
			if( T::IsWriter )
				ASSERT( lmdata.size() == size_t( width * height ) );
			arch.memory( lmdata.data(), lmdata.size_bytes() );
			if( T::IsReader )
				nmdata.resize( size_t(width) * size_t(height) );
			if( T::IsWriter )
				ASSERT( nmdata.size() == size_t( width * height ) );
			arch.memory( nmdata.data(), nmdata.size_bytes() );
			arch << invalid;
		}
		void ExportRGBA8( uint32_t* outcol, uint32_t* outnrm );
		void ReloadTex();
	};
	typedef Handle< LMap > LMapHandle;
	
	typedef HashTable< SGRX_GUID, Solid > SolidTable;
	typedef HashTable< SGRX_GUID, Surface > SurfaceTable;
	typedef HashTable< SGRX_GUID, LMapHandle > LMapTable;
	typedef HashTable< SGRX_GUID, NoValue > InvLMIDSet;
	typedef HashTable< SGRX_GUID, PrevMeshData > MovedMeshSet;
	
	EdLevelGraphicsCont();
	~EdLevelGraphicsCont();
	void Reset();
	void LoadLightmaps( const StringView& levname );
	void SaveLightmaps( const StringView& levname );
	void DumpLightmapInfo();
	void LightMesh( SGRX_MeshInstance* MI, SGRX_GUID lmguid );
	void RelightAllMeshes();
	void CreateLightmap( SGRX_GUID lmguid );
	void ClearLightmap( SGRX_GUID lmguid );
	void ApplyLightmap( SGRX_GUID lmguid );
	void InvalidateLightmap( SGRX_GUID lmguid );
	void ValidateLightmap( SGRX_GUID lmguid );
	void ApplyInvalidation();
	void InvalidateMesh( SGRX_GUID guid );
	void InvalidateSurface( SGRX_GUID guid );
	void InvalidateLight( SGRX_GUID guid );
	
	void InvalidateSamples();
	void InvalidateAll();
	bool IsInvalidated( SGRX_GUID lmguid );
	bool ILMBeginRender();
	void ILMAbort();
	void ILMCheck();
	void STRegenerate();
	void ExportLightmap( SGRX_GUID lmguid, LC_Lightmap& outlm );
	void UpdateCache( LevelCache& LC );
	
	void RequestSolid( SGRX_GUID guid, EdLGCSolidInfo* info = NULL );
	void DeleteSolid( SGRX_GUID guid );
	void UpdateSolid( SGRX_GUID guid, EdLGCSolidInfo* info );
	
	void RequestSurface( SGRX_GUID guid, EdLGCSurfaceInfo* info = NULL );
	void DeleteSurface( SGRX_GUID guid );
	void UpdateSurface( SGRX_GUID guid, uint32_t changes, EdLGCSurfaceInfo* info );
	
	int GetInvalidItemCount()
	{
		return int(m_invalidLightmaps.size()) + int(m_invalidSamples);
	}
	
	virtual void HandleEvent( SGRX_EventID eid, const EventData& edata );
	
	SolidTable m_solids;
	SurfaceTable m_surfaces;
	HashTable< SGRX_GUID, Mesh > m_meshes;
	HashTable< SGRX_GUID, Light > m_lights;
	LMapTable m_lightmaps;
	SGRX_LightEnv m_lightEnv;
	
	bool m_invalidSamples;
	bool m_alrInvalidSamples;
	InvLMIDSet m_invalidLightmaps;
	InvLMIDSet m_alrInvalidLightmaps;
	MovedMeshSet m_movedMeshes;
	MovedMeshSet m_movedSurfs;
	HashTable< SGRX_GUID, EdLGCLightInfo > m_movedLights;
	
	LMRenderer* m_lmRenderer;
};


