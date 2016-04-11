

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
		lmsize(V2(0)),
		solid_id(0),
		lmparent_id(0){}
	
	// vidata
	LCVertex* vdata;
	uint32_t vcount;
	uint16_t* idata;
	uint32_t icount;
	Vec2 lmsize;
	// mtldata
	String mtlname;
	// solid
	uint32_t solid_id;
	// lmparent
	uint32_t lmparent_id;
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

struct EdLevelGraphicsCont : IGameLevelSystem, SGRX_IEventHandler
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
		uint32_t solid_id;
		uint32_t lmparent_id;
		
		void RecalcTangents();
	};
	struct Mesh
	{
		EdLGCMeshInfo info;
		struct MeshEntity* ent;
	};
	struct Light
	{
		EdLGCLightInfo info;
		struct LightEntity* ent;
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
			arch.memory( lmdata.data(), lmdata.size_bytes() );
			if( T::IsReader )
				nmdata.resize( size_t(width) * size_t(height) );
			arch.memory( nmdata.data(), nmdata.size_bytes() );
			arch << invalid;
		}
		void ExportRGBA8( uint32_t* outcol, uint32_t* outnrm );
		void ReloadTex();
	};
	typedef Handle< LMap > LMapHandle;
	
	typedef HashTable< uint32_t, Solid > SolidTable;
	typedef HashTable< uint32_t, Surface > SurfaceTable;
	typedef HashTable< uint32_t, LMapHandle > LMapTable;
	typedef HashTable< uint32_t, uint32_t > InvLMIDTable;
	typedef HashTable< uint32_t, PrevMeshData > MovedMeshSet;
	typedef HashTable< Entity*, uint32_t > EntityLMIDTable;
	
	EdLevelGraphicsCont( GameLevel* lev );
	~EdLevelGraphicsCont();
	void OnDestroy();
	void Reset();
	void LoadLightmaps( const StringView& levname );
	void SaveLightmaps( const StringView& levname );
	void DumpLightmapInfo();
	void LightMesh( SGRX_MeshInstance* MI, uint32_t lmid );
	void RelightAllMeshes();
	void CreateLightmap( uint32_t lmid );
	void ClearLightmap( uint32_t lmid );
	void ApplyLightmap( uint32_t lmid );
	void InvalidateLightmap( uint32_t lmid );
	void ValidateLightmap( uint32_t lmid );
	void ApplyInvalidation();
	void InvalidateMesh( uint32_t id );
	void InvalidateSurface( uint32_t id );
	void InvalidateLight( uint32_t id );
	
	void InvalidateSamples();
	void InvalidateAll();
	bool IsInvalidated( uint32_t lmid );
	bool ILMBeginRender();
	void ILMAbort();
	void ILMCheck();
	void STRegenerate();
	void ExportLightmap( uint32_t lmid, LC_Lightmap& outlm );
	void UpdateCache( LevelCache& LC );
	
	uint32_t CreateSolid( EdLGCSolidInfo* info = NULL );
	void RequestSolid( uint32_t id, EdLGCSolidInfo* info = NULL );
	void DeleteSolid( uint32_t id );
	void UpdateSolid( uint32_t id, EdLGCSolidInfo* info );
	
	uint32_t CreateMesh( EdLGCMeshInfo* info = NULL );
	void RequestMesh( uint32_t id, EdLGCMeshInfo* info = NULL );
	void DeleteMesh( uint32_t id );
	void UpdateMesh( uint32_t id, uint32_t changes, EdLGCMeshInfo* info );
	
	uint32_t CreateSurface( EdLGCSurfaceInfo* info = NULL );
	void RequestSurface( uint32_t id, EdLGCSurfaceInfo* info = NULL );
	void DeleteSurface( uint32_t id );
	void UpdateSurface( uint32_t id, uint32_t changes, EdLGCSurfaceInfo* info );
	
	uint32_t CreateLight( EdLGCLightInfo* info = NULL );
	void RequestLight( uint32_t id, EdLGCLightInfo* info = NULL );
	void DeleteLight( uint32_t id );
	void UpdateLight( uint32_t id, uint32_t changes, EdLGCLightInfo* info );
	
	int GetInvalidItemCount()
	{
		return int(m_invalidLightmaps.size()) + int(m_invalidSamples);
	}
	
	virtual void OnAddEntity( Entity* ent );
	virtual void OnRemoveEntity( Entity* ent );
	virtual void HandleEvent( SGRX_EventID eid, const EventData& edata );
	
	uint32_t m_nextSolidID;
	uint32_t m_nextSurfID;
	uint32_t m_nextMeshEntID;
	uint32_t m_nextLightEntID;
	
	SolidTable m_solids;
	SurfaceTable m_surfaces;
	HashTable< uint32_t, Mesh > m_meshes;
	HashTable< uint32_t, Light > m_lights;
	LMapTable m_lightmaps;
	SGRX_LightTree m_sampleTree;
	
	bool m_invalidSamples;
	bool m_alrInvalidSamples;
	InvLMIDTable m_invalidLightmaps;
	InvLMIDTable m_alrInvalidLightmaps;
	MovedMeshSet m_movedMeshes;
	MovedMeshSet m_movedSurfs;
	HashTable< uint32_t, EdLGCLightInfo > m_movedLights;
	
	LMRenderer* m_lmRenderer;
};


