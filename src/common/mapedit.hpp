

#pragma once
#include "compiler.hpp"
#include "edutils.hpp"
#include "edcomui.hpp"
#include "scritem.hpp"
#include "../tacstrike/level.hpp" // TODO MOVE


// v0: initial
// v1: added surface.lmquality
// v2: added ent[light].flareoffset
// v3: added surface.xfit/yfit, added groups, added block.position
// v4: added subentities
// v5: level graphics container IDs
// v6: patch.layer.lmquality -> patch, added world.sample_density
// v7: added dynamic lights
// v8: added mesh paths
// v9: separated patch/path LM/physics solidity
#define MAP_FILE_VERSION 9

#define MAX_BLOCK_POLYGONS 32

#define MAX_PATCH_WIDTH 16
#define MAX_PATCH_LAYERS 4
#define MAX_MESHPATH_PARTS 4

#define EDGUI_EVENT_SETENTITY EDGUI_EVENT_USER + 1
#define EDGUI_EVENT_DELOBJECT EDGUI_EVENT_USER + 2


#ifdef MAPEDIT_DEFINE_GLOBALS
#  define MAPEDIT_GLOBAL( x ) x = NULL
#else
#  define MAPEDIT_GLOBAL( x ) extern x
#endif
MAPEDIT_GLOBAL( ScriptContext* g_ScriptCtx );
MAPEDIT_GLOBAL( struct EDGUIMainFrame* g_UIFrame );
MAPEDIT_GLOBAL( SceneHandle g_EdScene );
MAPEDIT_GLOBAL( PhyWorldHandle g_EdPhyWorld );
MAPEDIT_GLOBAL( struct EdWorld* g_EdWorld );
MAPEDIT_GLOBAL( struct EdLevelGraphicsCont* g_EdLGCont );
MAPEDIT_GLOBAL( struct EDGUISDTexPicker* g_UISurfTexPicker );
MAPEDIT_GLOBAL( struct EDGUISurfMtlPicker* g_UISurfMtlPicker );
MAPEDIT_GLOBAL( struct EDGUIMeshPicker* g_UIMeshPicker );
MAPEDIT_GLOBAL( struct EDGUICharUsePicker* g_UICharPicker );
MAPEDIT_GLOBAL( struct EDGUIPartSysPicker* g_UIPartSysPicker );
MAPEDIT_GLOBAL( struct EDGUISoundPicker* g_UISoundPicker );
MAPEDIT_GLOBAL( struct EDGUIScrItemPicker* g_UIScrItemPicker );
MAPEDIT_GLOBAL( struct EDGUIScrFnPicker* g_UIScrFnPicker );
MAPEDIT_GLOBAL( struct EDGUILevelOpenPicker* g_UILevelOpenPicker );
MAPEDIT_GLOBAL( struct EDGUILevelSavePicker* g_UILevelSavePicker );
MAPEDIT_GLOBAL( struct EDGUIEntList* g_EdEntList );
MAPEDIT_GLOBAL( BaseGame* g_BaseGame );
MAPEDIT_GLOBAL( GameLevel* g_Level );



struct SMPVertex
{
	Vec3 pos;
	Vec3 nrm;
	Vec4 tng;
	Vec2 tex;
};

struct EDGUISurfMtlPicker : EDGUIMeshPickerCore
{
	EDGUISurfMtlPicker()
	{
		m_mesh = GR_CreateMesh();
		
		float size = 1;
		SMPVertex verts[4] =
		{
			{ { -size, 0, -size }, {0,0,1}, {1,0,0,1}, { 0, 1 } },
			{ { +size, 0, -size }, {0,0,1}, {1,0,0,1}, { 1, 1 } },
			{ { +size, 0, +size }, {0,0,1}, {1,0,0,1}, { 1, 0 } },
			{ { -size, 0, +size }, {0,0,1}, {1,0,0,1}, { 0, 0 } },
		};
		uint16_t idcs[6] = { 0, 2, 1, 3, 2, 0 };
		SGRX_MeshPart part = { 0, 4, 0, 6 };
		VertexDeclHandle vdh = GR_GetVertexDecl( "pf3nf3tf40f2" );
		m_mesh->SetVertexData( verts, sizeof(verts), vdh );
		m_mesh->SetIndexData( idcs, sizeof(idcs), false );
		m_mesh->SetAABBFromVertexData( verts, sizeof(verts), vdh );
		m_mesh->SetPartData( &part, 1 );
		
		m_scene->camera.direction = V3(0.05f,1,-0.05f).Normalized();
		m_scene->camera.position = -m_scene->camera.direction * 2.0f;
		m_scene->camera.znear = 0.1f;
		m_scene->camera.angle = 60;
		m_scene->camera.UpdateMatrices();
		m_customCamera = true;
		
		caption = "Pick a surface material";
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading surface materials";
		m_options.clear();
		Clear();
		m_options.push_back( "<none>" );
		m_meshInsts.push_back( NULL );
		
		// parse material list
		m_materials.clear();
		String material_data;
		if( FS_LoadTextFile( "editor/materials.txt", material_data ) )
		{
			MapMaterial mmdummy;
			MapMaterial* mmtl = &mmdummy;
			ConfigReader cfgread( material_data );
			StringView key, value;
			while( cfgread.Read( key, value ) )
			{
				if( key == "material" )
				{
					mmtl = new MapMaterial;
					mmtl->name = value;
					mmtl->texcount = 0;
					mmtl->blendmode = SGRX_MtlBlend_None;
					mmtl->flags = 0;
					m_materials.set( mmtl->name, mmtl );
					LOG << "[SMtl]: " << value;
				}
				else if( key == "shader" ) mmtl->shader = value;
				else if( key == "blendmode" )
				{
					if( value == "basic" ) mmtl->blendmode = SGRX_MtlBlend_Basic;
					else if( value == "additive" ) mmtl->blendmode = SGRX_MtlBlend_Additive;
					else if( value == "multiply" ) mmtl->blendmode = SGRX_MtlBlend_Multiply;
					else mmtl->blendmode = SGRX_MtlBlend_None;
				}
				else if( key == "unlit" ) mmtl->flags |= SGRX_MtlFlag_Unlit;
				else if( key == "nocull" ) mmtl->flags |= SGRX_MtlFlag_Nocull;
				else if( key == "0" ){ mmtl->texture[0] = value; mmtl->texcount = TMAX( mmtl->texcount, 0+1 ); }
				else if( key == "1" ){ mmtl->texture[1] = value; mmtl->texcount = TMAX( mmtl->texcount, 1+1 ); }
				else if( key == "2" ){ mmtl->texture[2] = value; mmtl->texcount = TMAX( mmtl->texcount, 2+1 ); }
				else if( key == "3" ){ mmtl->texture[3] = value; mmtl->texcount = TMAX( mmtl->texcount, 3+1 ); }
				else if( key == "4" ){ mmtl->texture[4] = value; mmtl->texcount = TMAX( mmtl->texcount, 4+1 ); }
				else if( key == "5" ){ mmtl->texture[5] = value; mmtl->texcount = TMAX( mmtl->texcount, 5+1 ); }
				else if( key == "6" ){ mmtl->texture[6] = value; mmtl->texcount = TMAX( mmtl->texcount, 6+1 ); }
				else if( key == "7" ){ mmtl->texture[7] = value; mmtl->texcount = TMAX( mmtl->texcount, 7+1 ); }
			}
			LOG << "Loading completed";
		}
		else
		{
			LOG_ERROR << "FAILED to open editor/materials.txt";
		}
		
		// load materials
		for( size_t i = 0; i < m_materials.size(); ++i )
		{
			m_options.push_back( m_materials.item( i ).key );
			AddMtl( m_materials.item( i ).value );
		}
		
		_Search( m_searchString );
	}
	void AddMtl( MapMaterial* MM )
	{
		MeshInstHandle mih = m_scene->CreateMeshInstance();
		mih->SetMesh( m_mesh );
		mih->enabled = false;
		SGRX_Material mtl;
		mtl.shader = MM->shader;
		mtl.blendMode = MM->blendmode;
		mtl.flags = MM->flags;
		for( int i = 0; i < MAX_MATERIAL_TEXTURES; ++i )
		{
			if( MM->texture[ i ].size() )
				mtl.textures[ i ] = GR_GetTexture( MM->texture[ i ] );
		}
		mih->materials.assign( &mtl, 1 );
		lmm_prepmeshinst( mih );
		mih->constants[ 14 ] *= V4( V3( 1.5f ), 1 );
		mih->constants[ 15 ] *= V4( V3( 0.5f ), 1 );
		mih->Precache();
		m_meshInsts.push_back( mih );
	}
	
	MeshHandle m_mesh;
	MapMaterialMap m_materials;
};



struct EDGUISnapProps : EDGUILayoutRow
{
	EDGUISnapProps() :
		m_group( true, "Snapping properties" ),
		m_enableSnap( true ),
		m_snapVerts( true ),
		m_snapRange( 0.2f, 2, 0.01f, 1.0f ),
		m_snapGrid( 0.1f, 2, 0.01f, 100.0f ),
		m_projDist( 0.01f, 2, 0, 1 )
	{
		tyname = "snapprops";
		
		m_enableSnap.caption = "Enable snapping";
		m_snapVerts.caption = "Snap to vertices";
		m_snapRange.caption = "Max. distance";
		m_snapGrid.caption = "Grid unit size";
		m_projDist.caption = "Proj. distance";
		
		m_group.Add( &m_enableSnap );
		m_group.Add( &m_snapVerts );
		m_group.Add( &m_snapRange );
		m_group.Add( &m_snapGrid );
		m_group.Add( &m_projDist );
		Add( &m_group );
	}
	
	bool IsSnapEnabled(){ return m_enableSnap.m_value; }
	bool IsSnapVertices(){ return m_snapVerts.m_value; }
	float GetSnapMaxDist(){ return m_snapRange.m_value; }
	float GetSnapGridSize(){ return m_snapGrid.m_value; }
	
	static float Round( float v ){ return round( v ); }
	static Vec2 Round( Vec2 v ){ return V2( Round( v.x ), Round( v.y ) ); }
	static Vec3 Round( Vec3 v ){ return V3( Round( v.x ), Round( v.y ), Round( v.z ) ); }
	template< class T > void Snap( T& pos )
	{
		if( !m_enableSnap.m_value )
			return;
		
		if( m_snapVerts.m_value )
		{
		}
		
		pos /= m_snapGrid.m_value;
		pos = Round( pos );
		pos *= m_snapGrid.m_value;
	}
	
	EDGUIGroup m_group;
	EDGUIPropBool m_enableSnap;
	EDGUIPropBool m_snapVerts;
	EDGUIPropFloat m_snapRange;
	EDGUIPropFloat m_snapGrid;
	EDGUIPropFloat m_projDist;
};



//
// GROUPS
//

struct EDGUIPropRsrc_PickParentGroup : EDGUIPropRsrc
{
	EDGUIPropRsrc_PickParentGroup( int32_t id, struct EDGUIGroupPicker* gp, const StringView& value );
	virtual void OnReload( bool after );
	int32_t m_id;
};

struct EdGroup : EDGUILayoutRow
{
	// REFCOUNTED
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	int32_t m_refcount;
	// REFCOUNTED END
	
	EdGroup( struct EdGroupManager* groupMgr, int32_t id, int32_t pid, const StringView& name );
	virtual int OnEvent( EDGUIEvent* e );
	Mat4 GetMatrix();
	StringView GetPath();
	EdGroup* Clone();
	
	StringView Name(){ return m_ctlName.m_value; }
	
	template< class T > void Serialize( T& arch )
	{
		m_ctlOrigin.Serialize( arch );
		m_ctlPos.Serialize( arch );
		m_ctlAngles.Serialize( arch );
		m_ctlScaleUni.Serialize( arch );
	}
	
	EdGroupManager* m_groupMgr;
	int32_t m_id;
	int32_t m_parent_id;
	bool m_needsMtxUpdate;
	Mat4 m_mtxLocal;
	Mat4 m_mtxCombined;
	bool m_needsPathUpdate;
	String m_path;
	
	EDGUIGroup m_group;
	EDGUIPropString m_ctlName;
	EDGUIPropRsrc_PickParentGroup m_ctlParent;
	EDGUIPropVec3 m_ctlOrigin;
	EDGUIPropVec3 m_ctlPos;
	EDGUIPropVec3 m_ctlAngles;
	EDGUIPropFloat m_ctlScaleUni;
	
	EDGUIButton m_addChild;
	EDGUIButton m_deleteDisownParent;
	EDGUIButton m_deleteDisownRoot;
	EDGUIButton m_deleteRecursive;
	EDGUIButton m_recalcOrigin;
	EDGUIButton m_cloneGroup;
	EDGUIButton m_cloneGroupWithSub;
	EDGUIButton m_exportObj;
};
typedef Handle< EdGroup > EdGroupHandle;
typedef HashTable< int32_t, EdGroupHandle > EdGroupHandleMap;

struct EDGUIGroupPicker : EDGUIRsrcPicker
{
	EDGUIGroupPicker( EdGroupManager* groupMgr ) : m_groupMgr( groupMgr )
	{
		caption = "Pick a group";
		Reload();
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	void Reload();
	EdGroupManager* m_groupMgr;
	int32_t m_ignoreID;
};

struct EdGroupManager : EDGUILayoutRow
{
	EdGroupManager();
	virtual int OnEvent( EDGUIEvent* e );
	void DrawGroups();
	void AddRootGroup();
	Mat4 GetMatrix( int32_t id );
	StringView GetPath( int32_t id );
	EdGroup* AddGroup( int32_t parent_id = 0, StringView name = "", int32_t id = -1 );
	EdGroup* _AddGroup( int32_t id, int32_t parent_id, StringView name );
	EdGroup* FindGroupByID( int32_t id );
	EdGroup* FindGroupByPath( StringView path );
	bool GroupHasParent( int32_t id, int32_t parent_id );
	void PrepareEditGroup( EdGroup* grp );
	void PrepareCurrentEditGroup();
	void TransferGroupsToGroup( int32_t from, int32_t to );
	void QueueDestroy( EdGroup* grp );
	void ProcessDestroyQueue();
	void MatrixInvalidate( int32_t id );
	void PathInvalidate( int32_t id );
	
	template< class T > void Serialize( T& arch )
	{
		if( arch.version >= 3 )
		{
			arch.marker( "GRPLST" );
			{
				EdGroup* grp;
				int32_t groupcount = m_groups.size();
				arch << groupcount;
				for( int32_t i = 0; i < groupcount; ++i )
				{
					arch.marker( "GROUP" );
					if( T::IsWriter )
					{
						grp = m_groups.item( i ).value;
						arch << grp->m_id;
						arch << grp->m_parent_id;
						grp->m_ctlName.Serialize( arch );
					}
					else
					{
						int32_t id = 0, pid = 0;
						String name;
						arch << id;
						arch << pid;
						arch << name;
						grp = _AddGroup( id, pid, name );
					}
					arch << *grp;
				}
			}
		}
		else
			AddRootGroup();
	}
	
	Array< EdGroupHandle > m_destroyQueue;
	EdGroupHandleMap m_groups;
	EDGUIGroupPicker m_grpPicker;
	int32_t m_lastGroupID;
	EDGUIButton m_gotoRoot;
	EDGUIPropRsrc m_editedGroup;
};



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
};

struct EdLevelGraphicsCont
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
	struct Mesh
	{
		String meshpath;
		EdLGCRenderInfo info;
		MeshInstHandle meshInst;
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
	struct Light
	{
		EdLGCLightInfo info;
		LightHandle dynLight;
		
		bool IntersectsAABB( const Vec3& bbmin, const Vec3& bbmax, const Mat4& mtx ) const;
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
	typedef HashTable< uint32_t, Mesh > MeshTable;
	typedef HashTable< uint32_t, Surface > SurfaceTable;
	typedef HashTable< uint32_t, Light > LightTable;
	typedef HashTable< uint32_t, LMapHandle > LMapTable;
	typedef HashTable< uint32_t, uint32_t > InvLMIDTable;
	typedef HashTable< uint32_t, PrevMeshData > MovedMeshSet;
	
	EdLevelGraphicsCont();
	~EdLevelGraphicsCont();
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
	bool GetMeshAABB( uint32_t id, Vec3 out[2] );
	
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
	
	uint32_t m_nextSolidID;
	uint32_t m_nextMeshID;
	uint32_t m_nextSurfID;
	uint32_t m_nextLightID;
	
	SolidTable m_solids;
	MeshTable m_meshes;
	SurfaceTable m_surfaces;
	LightTable m_lights;
	LMapTable m_lightmaps;
	SGRX_LightTree m_sampleTree;
	
	bool m_invalidSamples;
	bool m_alrInvalidSamples;
	InvLMIDTable m_invalidLightmaps;
	InvLMIDTable m_alrInvalidLightmaps;
	MovedMeshSet m_movedMeshes;
	MovedMeshSet m_movedSurfs;
	LightTable m_movedLights;
	
	LMRenderer* m_lmRenderer;
};



//
// OBJECT
//

enum EObjectType
{
	ObjType_Block = 1,
	ObjType_Entity = 2,
	ObjType_Patch = 3,
	ObjType_MeshPath = 4,
};

enum ESpecialAction
{
	SA_None = 0,
	// patch actions
	SA_Invert,
	SA_Subdivide,
	SA_Unsubdivide,
	SA_EdgeFlip,
	SA_Extend,
	SA_Remove,
	SA_ExtractPart,
	SA_DuplicatePart,
	SA_SurfsToPatches,
	SA_RotateCCW,
	SA_RotateCW,
	// entity actions
	SA_MoveBack,
	SA_MoveFwd,
};

typedef SerializeVersionHelper<TextReader> SVHTR;
typedef SerializeVersionHelper<TextWriter> SVHTW;
typedef SerializeVersionHelper<ByteReader> SVHBR;
typedef SerializeVersionHelper<ByteWriter> SVHBW;

struct EdObject
{
	EdObject( uint8_t ty ) : m_refcount(0), m_type(ty), selected(false), group(0) {}
	EdObject( const EdObject& o ) : m_refcount(0), m_type(o.m_type), selected(o.selected), group(o.group) {}
	virtual ~EdObject(){}
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	int32_t m_refcount;
	uint8_t m_type;
	bool selected;
	int32_t group;
	
	virtual EdObject* Clone() = 0;
	virtual void Serialize( SVHTR& arch ) = 0;
	virtual void Serialize( SVHTW& arch ) = 0;
	virtual void Serialize( SVHBR& arch ) = 0;
	virtual void Serialize( SVHBW& arch ) = 0;
	virtual void FLoad( sgsVariable data, int version ) = 0;
	virtual sgsVariable FSave( int version ) = 0;
	virtual Vec3 GetPosition() const = 0;
	virtual void SetPosition( const Vec3& p ) = 0;
	virtual bool RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1] ) const = 0;
	virtual void RegenerateMesh() = 0;
	virtual Vec3 FindCenter() const = 0;
	virtual void Export( OBJExporter& objex ){}
	
	// subelement editing
	virtual int GetNumElements() const = 0;
	virtual Vec3 GetElementPoint( int i ) const = 0;
	virtual bool IsElementSelected( int i ) const = 0;
	virtual bool IsElementSpecial( int i ) const { return false; }
	virtual void SelectElement( int i, bool sel ) = 0;
	virtual void ClearSelection() = 0;
	virtual int GetNumVerts() const = 0;
	virtual Vec3 GetLocalVertex( int i ) const = 0;
	virtual void SetLocalVertex( int i, const Vec3& pos ) = 0;
	virtual bool IsVertexSelected( int i ) const { return false; }
	virtual int GetOnlySelectedVertex() const { return -1; }
	virtual void ScaleVertices( const Vec3& scale ) = 0;
	virtual void MoveSelectedVertices( const Vec3& t ) = 0;
	virtual int GetNumPaintVerts() const { return 0; }
	virtual void GetPaintVertex( int v, int layer, Vec3& outpos, Vec4& outcol ){}
	virtual void SetPaintVertex( int v, int layer, const Vec3& pos, Vec4 col ){}
	virtual void SpecialAction( ESpecialAction act ){}
	virtual bool CanDoSpecialAction( ESpecialAction act ){ return false; }
	
	// temp interface to block, TODO refactor into ^^^
	virtual int GetNumSelectedSurfs(){ return 0; }
	virtual int GetOnlySelectedSurface(){ return -1; }
	
	// utility functions
	void Serialize( TextReader& arch ){ SerializeVersionHelper<TextReader> svh( arch, MAP_FILE_VERSION ); Serialize( svh ); }
	void Serialize( TextWriter& arch ){ SerializeVersionHelper<TextWriter> svh( arch, MAP_FILE_VERSION ); Serialize( svh ); }
	void Serialize( ByteReader& arch ){ SerializeVersionHelper<ByteReader> svh( arch, MAP_FILE_VERSION ); Serialize( svh ); }
	void Serialize( ByteWriter& arch ){ SerializeVersionHelper<ByteWriter> svh( arch, MAP_FILE_VERSION ); Serialize( svh ); }
	void UISelectElement( int i, bool mod );
	void ProjectSelectedVertices();
};

typedef Handle< EdObject > EdObjectHandle;

struct EDGUIPaintProps : EDGUILayoutRow
{
	EDGUIPaintProps();
	
	int GetLayerNum(){ return m_ctlLayerNum.m_value; }
	float GetDistanceFactor( const Vec3& vpos, const Vec3& bpos );
	void Paint( Vec3& vpos, const Vec3& nrm, Vec4& vcol, float factor );
	int OnEvent( EDGUIEvent* e );
	void _UpdateColor();
	
	EDGUIGroup m_ctlGroup;
	EDGUIPropInt m_ctlLayerNum;
	EDGUIPropBool m_ctlPaintPos;
	EDGUIPropBool m_ctlPaintColor;
	EDGUIPropBool m_ctlPaintAlpha;
	EDGUIPropFloat m_ctlRadius;
	EDGUIPropFloat m_ctlFalloff;
	EDGUIPropFloat m_ctlSculptSpeed;
	EDGUIPropFloat m_ctlPaintSpeed;
	EDGUIPropVec3 m_ctlColorHSV;
	EDGUIPropFloat m_ctlAlpha;
	
	Vec4 m_tgtColor;
};



template< class T > T FLoadProp( sgsVariable obj, const char* prop, const T& def )
{
	return FLoadVar( obj.getprop( prop ), def );
}
bool FLoadVar( sgsVariable v, bool def )
{
	return v.getdef( def );
}
int FLoadVar( sgsVariable v, int def )
{
	return v.getdef( def );
}
uint32_t FLoadVar( sgsVariable v, uint32_t def )
{
	return v.getdef( def );
}
float FLoadVar( sgsVariable v, float def )
{
	return v.getdef( def );
}
StringView FLoadVar( sgsVariable v, StringView def )
{
	return v.getdef( def );
}
Vec2 FLoadVar( sgsVariable v, Vec2 def )
{
	return V2( FLoadProp( v, "x", def.x ), FLoadProp( v, "y", def.y ) );
}
Vec3 FLoadVar( sgsVariable v, Vec3 def )
{
	return V3( FLoadProp( v, "x", def.x ), FLoadProp( v, "y", def.y ), FLoadProp( v, "z", def.z ) );
}

sgsVariable FNewDict()
{
	return g_Level->GetScriptCtx().CreateDict();
}
sgsVariable FNewArray()
{
	return g_Level->GetScriptCtx().CreateArray();
}
void FArrayAppend( sgsVariable arr, sgsVariable val )
{
	SGS_CSCOPE( g_Level->GetSGSC() );
	g_Level->GetScriptCtx().Push( val );
	sgs_ArrayPush( g_Level->GetSGSC(), arr.var, 1 );
}
template< class T > void FSaveProp( sgsVariable obj, const char* prop, T value );
template< class T > void FSaveProp( sgsVariable obj, const char* prop, Array<T>& values );
sgsVariable FVar( bool val ){ return sgsVariable().set_bool( val ); }
sgsVariable FVar( int val ){ return sgsVariable().set_int( val ); }
sgsVariable FVar( uint32_t val ){ return sgsVariable().set_int( val ); }
sgsVariable FVar( float val ){ return sgsVariable().set_real( val ); }
sgsVariable FVar( StringView val )
{
	return g_Level->GetScriptCtx().CreateString( val );
}
sgsVariable FVar( Vec2 val )
{
	sgsVariable v = g_Level->GetScriptCtx().CreateDict();
	FSaveProp( v, "x", val.x );
	FSaveProp( v, "y", val.y );
	return v;
}
sgsVariable FVar( Vec3 val )
{
	sgsVariable v = g_Level->GetScriptCtx().CreateDict();
	FSaveProp( v, "x", val.x );
	FSaveProp( v, "y", val.y );
	FSaveProp( v, "z", val.z );
	return v;
}
template< class T > void FSaveProp( sgsVariable obj, const char* prop, T value )
{
	obj.setprop( prop, FVar( value ) );
}
template< class T > void FSaveProp( sgsVariable obj, const char* prop, Array<T>& values )
{
	sgsVariable arr = FNewArray();
	for( size_t i = 0; i < values.size(); ++i )
	{
		FArrayAppend( arr, FVar( values[ i ] ) );
	}
}



//
// BLOCKS
//

#define ED_TEXGEN_COORDS 0
#define ED_TEXGEN_STRETCH 1

struct EdSurface : SGRX_RefCounted
{
	String texname;
	int texgenmode;
	float xoff, yoff;
	float scale, aspect;
	float angle;
	float lmquality;
	int xfit, yfit;
	
	uint32_t surface_id;
	
	EdSurface() :
		texgenmode( ED_TEXGEN_COORDS ),
		xoff( 0 ), yoff( 0 ),
		scale( 1 ), aspect( 1 ),
		angle( 0 ), lmquality( 1 ),
		xfit( 0 ), yfit( 0 ),
		surface_id( 0 )
	{}
	~EdSurface()
	{
		if( surface_id )
			g_EdLGCont->DeleteSurface( surface_id );
	}
	
	template< class T > void Serialize( T& arch )
	{
		arch.marker( "SURFACE" );
		uint32_t oldsurfid = surface_id;
		arch( surface_id, arch.version >= 5 );
		if( surface_id != oldsurfid )
		{
			if( oldsurfid )
				g_EdLGCont->DeleteSurface( oldsurfid );
			if( surface_id )
				g_EdLGCont->RequestSurface( surface_id );
		}
		arch << texname;
		arch << texgenmode;
		arch << xoff << yoff;
		arch << scale << aspect;
		arch << angle;
		arch( lmquality, arch.version >= 1, 1.0f );
		arch( xfit, arch.version >= 3, 0 );
		arch( yfit, arch.version >= 3, 0 );
	}
	void FLoad( sgsVariable data, int version )
	{
		UNUSED( version );
		uint32_t oldsurfid = surface_id;
		surface_id = FLoadProp( data, "surface_id", 0 );
		if( surface_id != oldsurfid )
		{
			if( oldsurfid )
				g_EdLGCont->DeleteSurface( oldsurfid );
			if( surface_id )
				g_EdLGCont->RequestSurface( surface_id );
		}
		texname = FLoadProp( data, "texture", SV() );
		texgenmode = FLoadProp( data, "texgenmode", ED_TEXGEN_COORDS );
		xoff = FLoadProp( data, "xoff", 0.0f );
		yoff = FLoadProp( data, "yoff", 0.0f );
		scale = FLoadProp( data, "scale", 1.0f );
		aspect = FLoadProp( data, "aspect", 1.0f );
		angle = FLoadProp( data, "angle", 0.0f );
		lmquality = FLoadProp( data, "lmquality", 1.0f );
		xfit = FLoadProp( data, "xfit", 0 );
		yfit = FLoadProp( data, "yfit", 0 );
	}
	sgsVariable FSave( int version )
	{
		UNUSED( version );
		sgsVariable out = FNewDict();
		FSaveProp( out, "surface_id", int(surface_id) );
		FSaveProp( out, "texture", texname );
		FSaveProp( out, "texgenmode", texgenmode );
		FSaveProp( out, "xoff", xoff );
		FSaveProp( out, "yoff", yoff );
		FSaveProp( out, "scale", scale );
		FSaveProp( out, "aspect", aspect );
		FSaveProp( out, "angle", angle );
		FSaveProp( out, "lmquality", lmquality );
		FSaveProp( out, "xfit", xfit );
		FSaveProp( out, "yfit", yfit );
		return out;
	}
};
typedef Handle< EdSurface > EdSurfHandle;


struct EdBlock : EdObject
{
	EdBlock() : EdObject( ObjType_Block ), position(V3(0)), z0(0), z1(1), solid_id(0){}
	~EdBlock()
	{
		if( solid_id )
			g_EdLGCont->DeleteSolid( solid_id );
	}
	
	Vec3 position;
	float z0, z1;
	
	Array< Vec3 > poly;
	Array< EdSurfHandle > surfaces;
	Array< bool > subsel;
	
	uint32_t solid_id;
	
	template< class T > void SerializeT( T& arch )
	{
		arch.marker( "BLOCK" );
		uint32_t oldsolidid = solid_id;
		arch( solid_id, arch.version >= 5 );
		if( solid_id != oldsolidid )
		{
			if( oldsolidid )
				g_EdLGCont->DeleteSolid( oldsolidid );
			if( solid_id )
				g_EdLGCont->RequestSolid( solid_id );
		}
		if( arch.version >= 3 )
		{
			arch << group;
			arch << position;
		}
		else
		{
			group = 0;
			Vec2 pos = { position.x, position.y };
			arch << pos;
			position = V3( pos.x, pos.y, 0 );
		}
		arch << z0 << z1;
		arch << poly;
		arch << surfaces;
		
		if( T::IsReader )
		{
			subsel.resize_using( GetNumVerts() + GetNumSurfs(), false );
			RegenerateMesh();
		}
	}
	void FLoad( sgsVariable data, int version )
	{
		// type already parsed
		uint32_t oldsolidid = solid_id;
		if( solid_id != oldsolidid )
		{
			if( oldsolidid )
				g_EdLGCont->DeleteSolid( oldsolidid );
			if( solid_id )
				g_EdLGCont->RequestSolid( solid_id );
		}
		solid_id = FLoadProp( data, "solid_id", 0 );
		group = FLoadProp( data, "group", 0 );
		position = FLoadProp( data, "position", V3(0) );
		z0 = FLoadProp( data, "z0", 0 );
		z1 = FLoadProp( data, "z1", 0 );
		
		// poly
		{
			ScriptVarIterator it( data.getprop( "poly" ) );
			poly.clear();
			while( it.Advance() )
			{
				poly.push_back( FLoadVar( it.GetValue(), V3(0) ) );
			}
		}
		
		// surfaces
		{
			ScriptVarIterator it( data.getprop( "surfaces" ) );
			surfaces.clear();
			while( it.Advance() )
			{
				EdSurfHandle surf = new EdSurface;
				surf->FLoad( it.GetValue(), version );
				surfaces.push_back( surf );
			}
		}
	}
	sgsVariable FSave( int version )
	{
		sgsVariable out = FNewDict();
		FSaveProp( out, "solid_id", int(solid_id) );
		FSaveProp( out, "group", group );
		FSaveProp( out, "position", position );
		FSaveProp( out, "z0", z0 );
		FSaveProp( out, "z1", z1 );
		FSaveProp( out, "poly", poly );
		sgsVariable out_surfaces = FNewArray();
		for( size_t i = 0; i < surfaces.size(); ++i )
		{
			FArrayAppend( out_surfaces, surfaces[ i ]->FSave( version ) );
		}
		out.setprop( "surfaces", out_surfaces );
		return out;
	}
	
	virtual int GetNumVerts() const { return poly.size() * 2; }
	virtual Vec3 GetLocalVertex( int i ) const;
	virtual void SetLocalVertex( int i, const Vec3& pos );
	virtual Vec3 GetPosition() const { return position; }
	virtual void SetPosition( const Vec3& p ){ position = p; }
	virtual void ScaleVertices( const Vec3& f );
	virtual void MoveSelectedVertices( const Vec3& t );
	int GetNumSurfs() const { return poly.size() + 2; }
	Vec3 GetSurfaceCenter( int i ) const;
	int GetSurfaceNumVerts( int i );
	virtual int GetNumElements() const { return GetNumVerts() + GetNumSurfs(); }
	virtual Vec3 GetElementPoint( int i ) const;
	
	virtual bool IsVertexSelected( int i ) const;
	void SelectVertex( int i, bool sel );
	virtual int GetOnlySelectedVertex() const;
	bool IsSurfaceSelected( int i );
	void SelectSurface( int i, bool sel );
	int GetNumSelectedSurfs();
	int GetOnlySelectedSurface();
	virtual bool IsElementSelected( int i ) const;
	virtual void SelectElement( int i, bool sel );
	virtual void ClearSelection();
	virtual void SpecialAction( ESpecialAction act );
	virtual bool CanDoSpecialAction( ESpecialAction act );
	
	void _GetTexVecs( int surf, Vec3& tgx, Vec3& tgy );
	uint16_t _AddVtx( const Vec3& vpos, float z, const EdSurface& S, const Vec3& tgx, const Vec3& tgy, Array< LCVertex >& vertices, uint16_t voff );
	void _PostFitTexcoords( const EdSurface& S, LCVertex* vertices, size_t vcount );
	
	void GenCenterPos( EDGUISnapProps& SP );
	virtual Vec3 FindCenter() const;
	
	virtual EdObject* Clone();
	virtual void Serialize( SVHTR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHTW& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBW& arch ){ SerializeT( arch ); }
	bool RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1], int* outsurf ) const;
	virtual bool RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1] ) const { return RayIntersect( rpos, dir, outdst, NULL ); }
	virtual void RegenerateMesh();
	
	LevelCache::Vertex _MakeGenVtx( const Vec3& vpos, float z, const EdSurface& S, const Vec3& tgx, const Vec3& tgy );
	int GenerateSurface( LCVertex* outbuf, int sid, bool tri, bool fit = true );
	void Export( OBJExporter& objex );
};

typedef Handle< EdBlock > EdBlockHandle;



#define perp_prod(u,v) ((u).x * (v).y - (u).y * (v).x)
inline int intersect_lines( const Vec2& l1a, const Vec2& l1b, const Vec2& l2a, const Vec2& l2b, Vec2* out )
{
	Vec2 u = l1b - l1a;
	Vec2 v = l2b - l2a;
	Vec2 w = l1a - l2a;
	float D = perp_prod( u, v );
	
	if( fabs( D ) < SMALL_FLOAT )
		return 0;
	
	float sI = perp_prod( v, w ) / D;
	*out = l1a + sI * u;
	return 1;
}


struct EDGUIVertexProps : EDGUILayoutRow
{
	EDGUIVertexProps();
	void Prepare( EdBlock* block, int vid );
	virtual int OnEvent( EDGUIEvent* e );
	
	EdBlock* m_out;
	int m_vid;
	EDGUIGroup m_group;
	EDGUIPropVec3 m_pos;
	EDGUIButton m_insbef;
	EDGUIButton m_insaft;
};


struct EDGUISurfaceProps : EDGUILayoutRow
{
	EDGUISurfaceProps();
	void Prepare( EdBlock* block, int sid );
	void LoadParams( EdSurface& S, const char* name = "Surface" );
	void BounceBack( EdSurface& S );
	virtual int OnEvent( EDGUIEvent* e );
	
	EdBlock* m_out;
	int m_sid;
	EDGUIGroup m_group;
	EDGUIPropRsrc m_tex;
	EDGUIPropVec2 m_off;
	EDGUIPropVec2 m_scaleasp;
	EDGUIPropFloat m_angle;
	EDGUIPropFloat m_lmquality;
	EDGUIPropInt m_xfit;
	EDGUIPropInt m_yfit;
	EDGUIButton m_resetOffScaleAsp;
	EDGUIButton m_applyFit;
	EDGUIButton m_makeBlendPatch;
	EDGUIButton m_convertToPatch;
};

struct EDGUIBlockProps : EDGUILayoutRow
{
	EDGUIBlockProps();
	void Prepare( EdBlock* block );
	virtual int OnEvent( EDGUIEvent* e );
	
	EdBlock* m_out;
	EDGUIGroup m_group;
	EDGUIGroup m_vertGroup;
	EDGUIPropFloat m_z0;
	EDGUIPropFloat m_z1;
	EDGUIPropVec3 m_pos;
	EDGUIPropRsrc m_blkGroup;
	Array< EDGUIPropVec3 > m_vertProps;
	Array< EDGUISurfaceProps > m_surfProps;
};



//
// PATCHES
//

static const char* patch_texpropnames[ MAX_PATCH_LAYERS ] = { "tex0", "tex1", "tex2", "tex3" };
static const char* patch_colpropnames[ MAX_PATCH_LAYERS ] = { "col0", "col1", "col2", "col3" };

struct EdPatchVtx
{
	Vec3 pos;
	Vec2 tex[ MAX_PATCH_LAYERS ];
	uint32_t col[ MAX_PATCH_LAYERS ];
	
	template< class T > void Serialize( T& arch )
	{
		arch << pos;
		for( int i = 0; i < MAX_PATCH_LAYERS; ++i )
		{
			arch << tex[ i ];
			arch << col[ i ];
		}
	}
	void FLoad( sgsVariable data, int version )
	{
		UNUSED( version );
		pos = FLoadProp( data, "pos", V3(0) );
		for( int i = 0; i < MAX_PATCH_LAYERS; ++i )
		{
			tex[ i ] = FLoadProp( data, patch_texpropnames[ i ], V2(0) );
			col[ i ] = FLoadProp( data, patch_colpropnames[ i ], uint32_t(0xffffffff) );
		}
	}
	sgsVariable FSave( int version )
	{
		UNUSED( version );
		sgsVariable out = FNewDict();
		FSaveProp( out, "pos", pos );
		for( int i = 0; i < MAX_PATCH_LAYERS; ++i )
		{
			FSaveProp( out, patch_texpropnames[ i ], tex[ i ] );
			FSaveProp( out, patch_colpropnames[ i ], col[ i ] );
		}
		return out;
	}
};

struct EdPatchLayerInfo
{
	EdPatchLayerInfo() :
		xoff(0), yoff(0),
		scale(1), aspect(1),
		angle(0),
		surface_id(0){}
	~EdPatchLayerInfo()
	{
		if( surface_id )
			g_EdLGCont->DeleteSurface( surface_id );
	}
	
	template< class T > void Serialize( T& arch )
	{
		uint32_t oldsurfid = surface_id;
		arch( surface_id, arch.version >= 5 );
		if( surface_id != oldsurfid )
		{
			if( oldsurfid )
				g_EdLGCont->DeleteSurface( oldsurfid );
			if( surface_id )
				g_EdLGCont->RequestSurface( surface_id );
		}
		arch << texname;
		arch << xoff << yoff;
		arch << scale << aspect;
		arch << angle;
		float lmq = 0;
		arch( lmq, arch.version < 6 );
	}
	void FLoad( sgsVariable data, int version )
	{
		UNUSED( version );
		uint32_t oldsurfid = surface_id;
		surface_id = FLoadProp( data, "surface_id", 0 );
		if( surface_id != oldsurfid )
		{
			if( oldsurfid )
				g_EdLGCont->DeleteSurface( oldsurfid );
			if( surface_id )
				g_EdLGCont->RequestSurface( surface_id );
		}
		texname = FLoadProp( data, "texture", SV() );
		xoff = FLoadProp( data, "xoff", 0.0f );
		yoff = FLoadProp( data, "yoff", 0.0f );
		scale = FLoadProp( data, "scale", 1.0f );
		aspect = FLoadProp( data, "aspect", 1.0f );
		angle = FLoadProp( data, "angle", 0.0f );
	}
	sgsVariable FSave( int version )
	{
		UNUSED( version );
		sgsVariable out = FNewDict();
		FSaveProp( out, "texture", texname );
		FSaveProp( out, "xoff", xoff );
		FSaveProp( out, "yoff", yoff );
		FSaveProp( out, "scale", scale );
		FSaveProp( out, "aspect", aspect );
		FSaveProp( out, "angle", angle );
		return out;
	}
	
	String texname;
	float xoff, yoff;
	float scale, aspect;
	float angle;
	
	uint32_t surface_id;
};

#define PATCH_IS_SOLID 0x80

struct EdPatch : EdObject
{
	EdPatch() : EdObject( ObjType_Patch ), xsize(0), ysize(0), blend(0), lmquality(1)
	{
		TMEMSET<uint16_t>( edgeflip, MAX_PATCH_WIDTH, 0 );
		TMEMSET<uint16_t>( vertsel, MAX_PATCH_WIDTH, 0 );
	}
	
	bool InsertXLine( int at );
	bool InsertYLine( int at );
	bool RemoveXLine( int at );
	bool RemoveYLine( int at );
	static void _InterpolateVertex( EdPatchVtx* out, EdPatchVtx* v0, EdPatchVtx* v1, float s );
	
	virtual EdObject* Clone();
	virtual void Serialize( SVHTR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHTW& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBW& arch ){ SerializeT( arch ); }
	virtual Vec3 GetPosition() const { return position; }
	virtual void SetPosition( const Vec3& p ){ position = p; }
	virtual bool RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1] ) const;
	virtual void RegenerateMesh();
	virtual Vec3 FindCenter() const;
	Vec2 GenerateMeshData( Array< LCVertex >& outverts, Array< uint16_t >& outidcs, int layer );
	
	virtual int GetNumElements() const { return GetNumVerts() + GetNumQuads() + GetNumXEdges() + GetNumYEdges(); }
	virtual Vec3 GetElementPoint( int i ) const;
	virtual bool IsElementSelected( int i ) const { return i < xsize * ysize && IsVertexSelected( i ); }
	virtual bool IsElementSpecial( int i ) const { return i >= GetNumVerts(); }
	virtual void SelectElement( int i, bool sel );
	virtual void ClearSelection(){ TMEMSET<uint16_t>( vertsel, MAX_PATCH_WIDTH, 0 ); }
	virtual int GetNumVerts() const { return xsize * ysize; }
	virtual Vec3 GetLocalVertex( int i ) const { return vertices[ ( i % xsize ) + i / xsize * MAX_PATCH_WIDTH ].pos + position; }
	virtual void SetLocalVertex( int i, const Vec3& pos ){ vertices[ ( i % xsize ) + i / xsize * MAX_PATCH_WIDTH ].pos = pos - position; }
	virtual bool IsVertexSelected( int i ) const { return IsVertSel( i % xsize, i / xsize ); }
	virtual int GetOnlySelectedVertex() const;
	virtual void ScaleVertices( const Vec3& f );
	virtual void MoveSelectedVertices( const Vec3& t );
	virtual int GetNumPaintVerts() const { return xsize * ysize; }
	virtual void GetPaintVertex( int v, int layer, Vec3& outpos, Vec4& outcol );
	virtual void SetPaintVertex( int v, int layer, const Vec3& pos, Vec4 col );
	virtual void SpecialAction( ESpecialAction act );
	virtual bool CanDoSpecialAction( ESpecialAction act );
	
	int GetNumQuads() const { return ( xsize - 1 ) * ( ysize - 1 ); }
	Vec3 GetQuadCenter( int i ) const;
	int GetNumXEdges() const { return ( xsize - 1 ) * ysize; }
	Vec3 GetXEdgeCenter( int i ) const;
	int GetNumYEdges() const { return xsize * ( ysize - 1 ); }
	Vec3 GetYEdgeCenter( int i ) const;
	bool IsVertSel( int x, int y ) const { return 0 != ( vertsel[ y ] & ( 1 << x ) ); }
	bool IsXEdgeSel( int x, int y ) const { return IsVertSel( x, y ) && IsVertSel( x + 1, y ); }
	bool IsYEdgeSel( int x, int y ) const { return IsVertSel( x, y ) && IsVertSel( x, y + 1 ); }
	bool IsQuadSel( int x, int y ) const
	{
		return IsVertSel( x, y ) && IsVertSel( x + 1, y )
			&& IsVertSel( x, y + 1 ) && IsVertSel( x + 1, y + 1 );
	}
	bool IsAnyRectSel( bool invert = false, int outdims[2] = NULL ) const;
	bool IsAnySideSel( bool bothpatches = false ) const;
	uint16_t GetXSelMask( int i ) const;
	uint16_t GetYSelMask( int i ) const;
	bool IsXLineSel( int i ) const;
	bool IsYLineSel( int i ) const;
	bool IsAllSel() const;
	
	Vec2 TexGenFit( int layer );
	void TexGenScale( int layer, const Vec2& scale );
	void TexGenFitNat( int layer );
	void TexGenNatural( int layer );
	void TexGenPlanar( int layer );
	void TexGenPostProc( int layer );
	
	template< class T > void SerializeT( T& arch )
	{
		arch.marker( "PATCH" );
		arch << group;
		arch << position;
		arch << xsize << ysize;
		arch << blend;
		arch( m_isLMSolid, arch.version >= 9, true );
		arch( m_isPhySolid, arch.version >= 9, true );
		arch( lmquality, arch.version >= 6, 1.0f );
		for( int y = 0; y < ysize; ++y )
		{
			for( int x = 0; x < xsize; ++x )
				arch << vertices[ x + y * MAX_PATCH_WIDTH ];
		}
		for( int y = 0; y < ysize; ++y )
			arch << edgeflip[ y ];
		for( int l = 0; l < MAX_PATCH_LAYERS; ++l )
			arch << layers[ l ];
	}
	void FLoad( sgsVariable data, int version )
	{
		// type already parsed
		group = FLoadProp( data, "group", 0 );
		position = FLoadProp( data, "position", V3(0) );
		xsize = FLoadProp( data, "xsize", 2 );
		ysize = FLoadProp( data, "ysize", 2 );
		blend = FLoadProp( data, "blend", 0 );
		m_isLMSolid = FLoadProp( data, "isLMSolid", true );
		m_isPhySolid = FLoadProp( data, "isPhySolid", true );
		lmquality = FLoadProp( data, "lmquality", 1.0f );
		
		// vertices
		{
			ScriptVarIterator it( data.getprop( "vertices" ) );
			for( int y = 0; y < ysize; ++y )
			{
				for( int x = 0; x < xsize; ++x )
				{
					it.Advance();
					vertices[ x + y * MAX_PATCH_WIDTH ].FLoad( it.GetValue(), version );
				}
			}
		}
		
		// edgeflip
		{
			ScriptVarIterator it( data.getprop( "edgeflip" ) );
			for( int y = 0; y < ysize; ++y )
			{
				it.Advance();
				edgeflip[ y ] = FLoadVar( it.GetValue(), 0 );
			}
		}
		
		// layers
		{
			ScriptVarIterator it( data.getprop( "layers" ) );
			for( int l = 0; l < MAX_PATCH_LAYERS; ++l )
			{
				it.Advance();
				layers[ l ].FLoad( it.GetValue(), version );
			}
		}
	}
	sgsVariable FSave( int version )
	{
		sgsVariable out = FNewDict();
		
		FSaveProp( out, "group", group );
		FSaveProp( out, "position", position );
		FSaveProp( out, "xsize", xsize );
		FSaveProp( out, "ysize", ysize );
		FSaveProp( out, "blend", blend );
		FSaveProp( out, "isLMSolid", m_isLMSolid );
		FSaveProp( out, "isPhySolid", m_isPhySolid );
		FSaveProp( out, "lmquality", lmquality );
		
		sgsVariable out_vertices = FNewArray();
		for( int y = 0; y < ysize; ++y )
		{
			for( int x = 0; x < xsize; ++x )
			{
				FArrayAppend( out_vertices, vertices[ x + y * MAX_PATCH_WIDTH ].FSave( version ) );
			}
		}
		out.setprop( "vertices", out_vertices );
		
		sgsVariable out_edgeflip = FNewArray();
		for( int y = 0; y < ysize; ++y )
		{
			FArrayAppend( out_edgeflip, FVar( edgeflip[ y ] ) );
		}
		out.setprop( "edgeflip", out_edgeflip );
		
		sgsVariable out_layers = FNewArray();
		for( int l = 0; l < MAX_PATCH_LAYERS; ++l )
		{
			FArrayAppend( out_layers, layers[ l ].FSave( version ) );
		}
		out.setprop( "layers", out_layers );
		
		return out;
	}
	
	static EdPatch* CreatePatchFromSurface( EdBlock& B, int sid );
	
	Vec3 position;
	EdPatchVtx vertices[ MAX_PATCH_WIDTH * MAX_PATCH_WIDTH ];
	uint16_t edgeflip[ MAX_PATCH_WIDTH ]; // 0 - [\], 1 - [/]
	uint16_t vertsel[ MAX_PATCH_WIDTH ];
	int8_t xsize;
	int8_t ysize;
	uint8_t blend;
	bool m_isLMSolid;
	bool m_isPhySolid;
	float lmquality;
	EdPatchLayerInfo layers[ MAX_PATCH_LAYERS ];
};

typedef Handle< EdPatch > EdPatchHandle;


struct EDGUIPatchVertProps : EDGUILayoutRow
{
	EDGUIPatchVertProps();
	void Prepare( EdPatch* patch, int vid );
	virtual int OnEvent( EDGUIEvent* e );
	
	EdPatch* m_out;
	int m_vid;
	EDGUIGroup m_group;
	EDGUIPropVec3 m_pos;
	EDGUIPropVec2 m_tex[ MAX_PATCH_LAYERS ];
	EDGUIPropVec4 m_col[ MAX_PATCH_LAYERS ];
};

struct EDGUIPatchLayerProps : EDGUILayoutRow
{
	EDGUIPatchLayerProps();
	void Prepare( EdPatch* patch, int lid );
	void LoadParams( EdPatchLayerInfo& L, const char* name = "Layer" );
	void BounceBack( EdPatchLayerInfo& L );
	virtual int OnEvent( EDGUIEvent* e );
	
	EdPatch* m_out;
	int m_lid;
	EDGUIGroup m_group;
	EDGUIPropRsrc m_tex;
	EDGUIPropVec2 m_off;
	EDGUIPropVec2 m_scaleasp;
	EDGUIPropFloat m_angle;
	EDGUILayoutColumn m_texGenCol;
	EDGUILabel m_texGenLbl;
	EDGUIButton m_genFit;
	EDGUIButton m_genFitNat;
	EDGUIButton m_genNatural;
	EDGUIButton m_genPlanar;
};

struct EDGUIPatchProps : EDGUILayoutRow
{
	EDGUIPatchProps();
	void Prepare( EdPatch* patch );
	virtual int OnEvent( EDGUIEvent* e );
	
	EdPatch* m_out;
	EDGUIGroup m_group;
	EDGUIPropVec3 m_pos;
	EDGUIPropRsrc m_blkGroup;
	EDGUIPropBool m_isDecal;
	EDGUIPropBool m_isLMSolid;
	EDGUIPropBool m_isPhySolid;
	EDGUIPropInt m_layerStart;
	EDGUIPropFloat m_lmquality;
	EDGUIPatchLayerProps m_layerProps[4];
};


//
// MESHPATH
//

struct EdMeshPathPoint
{
	EdMeshPathPoint() : pos(V3(0)), smooth(false), sel(false){}
	
	template< class T > void Serialize( T& arch )
	{
		arch << pos << smooth;
	}
	void FLoad( sgsVariable data, int version )
	{
		UNUSED( version );
		pos = FLoadProp( data, "pos", V3(0) );
		smooth = FLoadProp( data, "smooth", false );
	}
	sgsVariable FSave( int version )
	{
		UNUSED( version );
		sgsVariable out = FNewDict();
		FSaveProp( out, "pos", pos );
		FSaveProp( out, "smooth", smooth );
		return out;
	}
	
	Vec3 pos;
	bool smooth;
	bool sel;
};

struct EdMeshPathPart
{
	EdMeshPathPart() :
		xoff(0), yoff(0),
		scale(1), aspect(1),
		angle(0),
		surface_id(0){}
	~EdMeshPathPart()
	{
		if( surface_id )
			g_EdLGCont->DeleteSurface( surface_id );
	}
	
	template< class T > void Serialize( T& arch )
	{
		uint32_t oldsurfid = surface_id;
		arch( surface_id, arch.version >= 5 );
		if( surface_id != oldsurfid )
		{
			if( oldsurfid )
				g_EdLGCont->DeleteSurface( oldsurfid );
			if( surface_id )
				g_EdLGCont->RequestSurface( surface_id );
		}
		arch << texname;
		arch << xoff << yoff;
		arch << scale << aspect;
		arch << angle;
		float lmq = 0;
		arch( lmq, arch.version < 6 );
	}
	void FLoad( sgsVariable data, int version )
	{
		UNUSED( version );
		uint32_t oldsurfid = surface_id;
		surface_id = FLoadProp( data, "surface_id", 0 );
		if( surface_id != oldsurfid )
		{
			if( oldsurfid )
				g_EdLGCont->DeleteSurface( oldsurfid );
			if( surface_id )
				g_EdLGCont->RequestSurface( surface_id );
		}
		texname = FLoadProp( data, "texture", SV() );
		xoff = FLoadProp( data, "xoff", 0.0f );
		yoff = FLoadProp( data, "yoff", 0.0f );
		scale = FLoadProp( data, "scale", 1.0f );
		aspect = FLoadProp( data, "aspect", 1.0f );
		angle = FLoadProp( data, "angle", 0.0f );
	}
	sgsVariable FSave( int version )
	{
		UNUSED( version );
		sgsVariable out = FNewDict();
		FSaveProp( out, "surface_id", surface_id );
		FSaveProp( out, "texture", texname );
		FSaveProp( out, "xoff", xoff );
		FSaveProp( out, "yoff", yoff );
		FSaveProp( out, "scale", scale );
		FSaveProp( out, "aspect", aspect );
		FSaveProp( out, "angle", angle );
		return out;
	}
	
	String texname;
	float xoff, yoff;
	float scale, aspect;
	float angle;
	
	uint32_t surface_id;
};

enum EMPATH_TurnMode
{
	MPATH_TurnMode_H = 0, // horizontal
	MPATH_TurnMode_Fwd, // forward
	MPATH_TurnMode_HFwdSkew, // horizontal+forward (skew)
};

struct EdMeshPath : EdObject
{
	EdMeshPath() : EdObject( ObjType_MeshPath ), m_position( V3(0) ), m_lmquality( 1 ),
		m_isLMSolid( true ), m_isPhySolid( false ), m_doSmoothing( false ), m_isDynamic( false ),
		m_intervalScaleOffset(V2(1,0)), m_pipeModeOvershoot(0),
		m_rotAngles( V3(0) ), m_scaleUni( 1 ), m_scaleSep( V3(1) ), m_turnMode(0)
	{
	}
	
	virtual EdObject* Clone();
	virtual void Serialize( SVHTR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHTW& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBW& arch ){ SerializeT( arch ); }
	virtual Vec3 GetPosition() const { return m_position; }
	virtual void SetPosition( const Vec3& p ){ m_position = p; }
	virtual bool RayIntersect( const Vec3& rpos, const Vec3& rdir, float outdst[1] ) const;
	float FindPointOnPath( float at, size_t& pi0, size_t& pi1 );
	Vec4 GetPlane( size_t pt );
	virtual void RegenerateMesh();
	virtual Vec3 FindCenter() const;
	size_t PlaceItem( struct LCVDataEdit& edit, float at, float off );
	Vec2 GenerateMeshData( Array< LCVertex >& outverts, Array< uint16_t >& outidcs, int layer );
	
	virtual int GetNumElements() const { return m_points.size(); }
	virtual Vec3 GetElementPoint( int i ) const { return m_points[ i ].pos + m_position; }
	virtual bool IsElementSelected( int i ) const { return m_points[ i ].sel; }
	virtual bool IsElementSpecial( int i ) const { return false; }
	virtual void SelectElement( int i, bool sel ){ m_points[ i ].sel = true; }
	virtual void ClearSelection(){ for( size_t i = 0; i < m_points.size(); ++i ) m_points[ i ].sel = false; }
	virtual int GetNumVerts() const { return m_points.size(); }
	virtual Vec3 GetLocalVertex( int i ) const { return m_points[ i ].pos + m_position; }
	virtual void SetLocalVertex( int i, const Vec3& pos ){ m_points[ i ].pos = pos - m_position; }
	virtual bool IsVertexSelected( int i ) const { return m_points[ i ].sel; }
	virtual int GetOnlySelectedVertex() const;
	virtual void ScaleVertices( const Vec3& f );
	virtual void MoveSelectedVertices( const Vec3& t );
	virtual void SpecialAction( ESpecialAction act );
	virtual bool CanDoSpecialAction( ESpecialAction act );
	
	template< class T > void SerializeT( T& arch )
	{
		arch.marker( "MESHPATH" );
		arch << group;
		arch << m_position;
		arch << m_meshName;
		arch << m_lmquality;
		arch( m_isLMSolid, arch.version >= 9, true );
		arch << m_isPhySolid;
		arch << m_doSmoothing;
		arch << m_isDynamic;
		arch << m_intervalScaleOffset;
		arch << m_pipeModeOvershoot;
		arch << m_rotAngles;
		arch << m_scaleUni;
		arch << m_scaleSep;
		arch << m_turnMode;
		arch << m_points;
		for( int mp = 0; mp < MAX_MESHPATH_PARTS; ++mp )
			arch << m_parts[ mp ];
	}
	void FLoad( sgsVariable data, int version )
	{
		// type already parsed
		group = FLoadProp( data, "group", 0 );
		m_position = FLoadProp( data, "position", V3(0) );
		m_meshName = FLoadProp( data, "meshName", SV() );
		m_lmquality = FLoadProp( data, "lmquality", 1.0f );
		m_isLMSolid = FLoadProp( data, "isLMSolid", true );
		m_isPhySolid = FLoadProp( data, "isPhySolid", false );
		m_doSmoothing = FLoadProp( data, "doSmoothing", false );
		m_isDynamic = FLoadProp( data, "isDynamic", false );
		m_intervalScaleOffset = FLoadProp( data, "intervalScaleOffset", V2(1,0) );
		m_pipeModeOvershoot = FLoadProp( data, "pipeModeOvershoot", 0 );
		m_rotAngles = FLoadProp( data, "rotAngles", V3(0) );
		m_scaleUni = FLoadProp( data, "scaleUni", 1.0f );
		m_scaleSep = FLoadProp( data, "scaleSep", V3(1) );
		m_turnMode = FLoadProp( data, "turnMode", 0 );
		
		// points
		{
			ScriptVarIterator it( data.getprop( "points" ) );
			m_points.clear();
			while( it.Advance() )
			{
				EdMeshPathPoint mpp;
				mpp.FLoad( it.GetValue(), version );
				m_points.push_back( mpp );
			}
		}
		
		// parts
		{
			ScriptVarIterator it( data.getprop( "parts" ) );
			for( int mp = 0; mp < MAX_MESHPATH_PARTS; ++mp )
			{
				it.Advance();
				m_parts[ mp ].FLoad( it.GetValue(), version );
			}
		}
	}
	sgsVariable FSave( int version )
	{
		sgsVariable out = FNewDict();
		
		FSaveProp( out, "group", group );
		FSaveProp( out, "position", m_position );
		FSaveProp( out, "meshName", m_meshName );
		FSaveProp( out, "lmquality", m_lmquality );
		FSaveProp( out, "isLMSolid", m_isLMSolid );
		FSaveProp( out, "isPhySolid", m_isPhySolid );
		FSaveProp( out, "doSmoothing", m_doSmoothing );
		FSaveProp( out, "isDynamic", m_isDynamic );
		FSaveProp( out, "intervalScaleOffset", m_intervalScaleOffset );
		FSaveProp( out, "pipeModeOvershoot", m_pipeModeOvershoot );
		FSaveProp( out, "rotAngles", m_rotAngles );
		FSaveProp( out, "scaleUni", m_scaleUni );
		FSaveProp( out, "scaleSep", m_scaleSep );
		FSaveProp( out, "turnMode", m_turnMode );
		
		sgsVariable out_points = FNewArray();
		for( size_t i = 0; i < m_points.size(); ++i )
		{
			FArrayAppend( out_points, m_points[ i ].FSave( version ) );
		}
		out.setprop( "points", out_points );
		
		sgsVariable out_parts = FNewArray();
		for( int i = 0; i < MAX_MESHPATH_PARTS; ++i )
		{
			FArrayAppend( out_parts, m_parts[ i ].FSave( version ) );
		}
		out.setprop( "parts", out_parts );
		
		return out;
	}
	
	Vec3 m_position;
	String m_meshName;
	float m_lmquality;
	bool m_isLMSolid;
	bool m_isPhySolid;
	bool m_doSmoothing;
	bool m_isDynamic;
	Vec2 m_intervalScaleOffset;
	int m_pipeModeOvershoot;
	Vec3 m_rotAngles;
	float m_scaleUni;
	Vec3 m_scaleSep;
	int m_turnMode;
	Array< EdMeshPathPoint > m_points;
	EdMeshPathPart m_parts[ MAX_MESHPATH_PARTS ];
	
	MeshHandle m_cachedMesh;
};

typedef Handle< EdMeshPath > EdMeshPathHandle;


struct EDGUIMeshPathPointProps : EDGUILayoutRow
{
	EDGUIMeshPathPointProps();
	void Prepare( EdMeshPath* mpath, int pid );
	virtual int OnEvent( EDGUIEvent* e );
	
	EdMeshPath* m_out;
	int m_pid;
	EDGUIGroup m_group;
	EDGUIPropVec3 m_pos;
	EDGUIPropBool m_smooth;
};

struct EDGUIMeshPathPartProps : EDGUILayoutRow
{
	EDGUIMeshPathPartProps();
	void Prepare( EdMeshPath* mpath, int pid );
	void LoadParams( EdMeshPathPart& MP, const char* name = "Part" );
	void BounceBack( EdMeshPathPart& MP );
	virtual int OnEvent( EDGUIEvent* e );
	
	EdMeshPath* m_out;
	int m_pid;
	EDGUIGroup m_group;
	EDGUIPropRsrc m_tex;
	EDGUIPropVec2 m_off;
	EDGUIPropVec2 m_scaleasp;
	EDGUIPropFloat m_angle;
};

struct EDGUIMeshPathProps : EDGUILayoutRow
{
	EDGUIMeshPathProps();
	void Prepare( EdMeshPath* mpath );
	virtual int OnEvent( EDGUIEvent* e );
	
	EdMeshPath* m_out;
	EDGUIGroup m_group;
	EDGUIPropRsrc m_meshName;
	EDGUIPropVec3 m_pos;
	EDGUIPropRsrc m_blkGroup;
	EDGUIPropBool m_isLMSolid;
	EDGUIPropBool m_isPhySolid;
	EDGUIPropBool m_doSmoothing;
	EDGUIPropBool m_isDynamic;
	EDGUIPropFloat m_lmquality;
	EDGUIPropVec2 m_intervalScaleOffset;
	EDGUIPropInt m_pipeModeOvershoot;
	EDGUIPropVec3 m_rotAngles;
	EDGUIPropFloat m_scaleUni;
	EDGUIPropVec3 m_scaleSep;
	EDGUIPropInt m_turnMode;
	EDGUIMeshPathPartProps m_partProps[ MAX_MESHPATH_PARTS ];
};



//
// ENTITIES
//

struct EdEntity : EDGUILayoutRow, EdObject
{
	EdEntity( bool isproto ) :
		EdObject( ObjType_Entity ),
		m_isproto( isproto ),
		m_group( true, "Entity properties" ),
		m_ctlPos( V3(0), 2, V3(-8192), V3(8192) )
	{
		tyname = "_entity_overrideme_";
		
		m_ctlPos.caption = "Position";
	}
	
	void BeforeDelete();
	void LoadIcon();
	
	const Vec3& Pos() const { return m_ctlPos.m_value; }
	virtual Vec3 GetPosition() const { return Pos(); }
	virtual void SetPosition( const Vec3& pos ){ m_ctlPos.SetValue( pos ); }
	virtual void ScaleVertices( const Vec3& ){}
	
	virtual int OnEvent( EDGUIEvent* e ){ return EDGUILayoutRow::OnEvent( e ); }
	
	virtual bool IsScriptedEnt(){ return false; }
	virtual void UpdateCache( LevelCache& LC ){}
	
	virtual EdEntity* CloneEntity() = 0;
	
	virtual EdObject* Clone()
	{
		EdObject* obj = CloneEntity();
		obj->selected = selected;
		obj->group = group;
		return obj;
	}
	virtual bool RayIntersect( const Vec3& rpos, const Vec3& rdir, float outdst[1] ) const
	{
		return RaySphereIntersect( rpos, rdir, Pos(), 0.2f, outdst );
	}
	virtual Vec3 FindCenter() const { return Pos(); }
	virtual void RegenerateMesh(){}
	virtual void DebugDraw();
	
	virtual int GetNumElements() const { return 0; }
	virtual Vec3 GetElementPoint( int i ) const { return GetLocalVertex( i ); }
	virtual bool IsElementSelected( int i ) const { return false; }
	virtual void SelectElement( int i, bool sel ){}
	virtual void ClearSelection(){}
	virtual int GetNumVerts() const { return 1; }
	virtual Vec3 GetLocalVertex( int ) const { return Pos(); }
	virtual void SetLocalVertex( int i, const Vec3& pos ){}
	virtual void MoveSelectedVertices( const Vec3& t ){ /* NO VERTICES BY DEFAULT */ }
	
	bool m_isproto;
	EDGUIGroup m_group;
	EDGUIPropVec3 m_ctlPos;
	TextureHandle m_iconTex;
	
	Handle< EdEntity > m_ownerEnt;
	Array< Handle< EdEntity > > m_subEnts;
};

typedef Handle< EdEntity > EdEntityHandle;



/////////////
////////////
///////////

struct EdEntMesh : EdEntity
{
	EdEntMesh( bool isproto = true );
	~EdEntMesh(){ if( m_meshID ) g_EdLGCont->DeleteMesh( m_meshID ); }
	
	const String& Mesh() const { return m_ctlMesh.m_value; }
	const Vec3& RotAngles() const { return m_ctlAngles.m_value; }
	float ScaleUni() const { return m_ctlScaleUni.m_value; }
	const Vec3& ScaleSep() const { return m_ctlScaleSep.m_value; }
	Mat4 Matrix() const { return Mat4::CreateSRT( m_ctlScaleSep.m_value * m_ctlScaleUni.m_value, DEG2RAD( m_ctlAngles.m_value ), m_ctlPos.m_value ); }
	
	EdEntMesh& operator = ( const EdEntMesh& o );
	virtual EdEntity* CloneEntity();
	
	template< class T > void SerializeT( T& arch )
	{
		uint32_t oldmid = m_meshID;
		arch( m_meshID, arch.version >= 5 );
		if( oldmid != m_meshID )
		{
			if( oldmid )
				g_EdLGCont->DeleteMesh( oldmid );
			if( m_meshID )
				g_EdLGCont->RequestMesh( m_meshID );
		}
		arch << m_ctlPos;
		arch << m_ctlAngles;
		arch << m_ctlScaleUni;
		arch << m_ctlScaleSep;
		arch << m_ctlMesh;
	}
	virtual void Serialize( SVHTR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHTW& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBW& arch ){ SerializeT( arch ); }
	
	virtual void UpdateCache( LevelCache& LC ){}
	virtual int OnEvent( EDGUIEvent* e );
	virtual void DebugDraw();
	virtual void RegenerateMesh();
	
	EDGUIPropVec3 m_ctlAngles;
	EDGUIPropFloat m_ctlScaleUni;
	EDGUIPropVec3 m_ctlScaleSep;
	EDGUIPropRsrc m_ctlMesh;
	
	uint32_t m_meshID;
};

struct EdEntLight : EdEntity
{
	EdEntLight( bool isproto = true );
	~EdEntLight(){ if( m_lightID ) g_EdLGCont->DeleteLight( m_lightID ); }
	
	float Range() const { return m_ctlRange.m_value; }
	float Power() const { return m_ctlPower.m_value; }
	const Vec3& ColorHSV() const { return m_ctlColorHSV.m_value; }
	float LightRadius() const { return m_ctlLightRadius.m_value; }
	bool IsDynamic() const { return m_ctlDynamic.m_value; }
	int ShadowSampleCount() const { return m_ctlShSampleCnt.m_value; }
	float FlareSize() const { return m_ctlFlareSize.m_value; }
	Vec3 FlareOffset() const { return m_ctlFlareOffset.m_value; }
	bool IsSpotlight() const { return m_ctlIsSpotlight.m_value; }
	const Vec3& SpotRotation() const { return m_ctlSpotRotation.m_value; }
	float SpotInnerAngle() const { return m_ctlSpotInnerAngle.m_value; }
	float SpotOuterAngle() const { return m_ctlSpotOuterAngle.m_value; }
	float SpotCurve() const { return m_ctlSpotCurve.m_value; }
	Mat4 SpotMatrix() const { return Mat4::CreateRotationXYZ( DEG2RAD( SpotRotation() ) ); }
	Vec3 SpotDir() const { return SpotMatrix().TransformNormal( V3(0,0,-1) ).Normalized(); }
	Vec3 SpotUp() const { return SpotMatrix().TransformNormal( V3(0,-1,0) ).Normalized(); }
	
	EdEntLight& operator = ( const EdEntLight& o );
	virtual EdEntity* CloneEntity();
	
	template< class T > void SerializeT( T& arch )
	{
		uint32_t oldltid = m_lightID;
		arch( m_lightID, arch.version >= 5 );
		if( m_lightID != oldltid )
		{
			if( oldltid )
				g_EdLGCont->DeleteLight( oldltid );
			if( m_lightID )
				g_EdLGCont->RequestLight( m_lightID );
		}
		arch << m_ctlPos;
		arch << m_ctlRange;
		arch << m_ctlPower;
		arch << m_ctlColorHSV;
		arch << m_ctlLightRadius;
		if( arch.version >= 7 )
			m_ctlDynamic.Serialize( arch );
		else if( T::IsReader )
			m_ctlDynamic.SetValue( false );
		arch << m_ctlShSampleCnt;
		arch << m_ctlFlareSize;
		if( arch.version >= 2 )
			m_ctlFlareOffset.Serialize( arch );
		arch << m_ctlIsSpotlight;
		arch << m_ctlSpotRotation;
		arch << m_ctlSpotInnerAngle;
		arch << m_ctlSpotOuterAngle;
		arch << m_ctlSpotCurve;
	}
	virtual void Serialize( SVHTR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHTW& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBW& arch ){ SerializeT( arch ); }
	
	virtual void DebugDraw();
	virtual void UpdateCache( LevelCache& LC ){}
	virtual int OnEvent( EDGUIEvent* e );
	virtual void RegenerateMesh();
	
	EDGUIPropFloat m_ctlRange;
	EDGUIPropFloat m_ctlPower;
	EDGUIPropVec3 m_ctlColorHSV;
	EDGUIPropFloat m_ctlLightRadius;
	EDGUIPropBool m_ctlDynamic;
	EDGUIPropInt m_ctlShSampleCnt;
	EDGUIPropFloat m_ctlFlareSize;
	EDGUIPropVec3 m_ctlFlareOffset;
	EDGUIPropBool m_ctlIsSpotlight;
	EDGUIPropVec3 m_ctlSpotRotation;
	EDGUIPropFloat m_ctlSpotInnerAngle;
	EDGUIPropFloat m_ctlSpotOuterAngle;
	EDGUIPropFloat m_ctlSpotCurve;
	
	uint32_t m_lightID;
};

struct EdEntLightSample : EdEntity
{
	EdEntLightSample( bool isproto = true );
	EdEntLightSample& operator = ( const EdEntLightSample& o );
	virtual EdEntity* CloneEntity();
	
	template< class T > void SerializeT( T& arch )
	{
		arch << m_ctlPos;
	}
	virtual void Serialize( SVHTR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHTW& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHBW& arch ){ SerializeT( arch ); }
	
	virtual void UpdateCache( LevelCache& LC ){}
};


#define EDGUI_ITEM_PROP_SCRITEM 1001

struct SGSPropInterface
{
	struct Field
	{
		sgsString key;
		EDGUIProperty* property;
	};
	
	SGSPropInterface();
	
	virtual void ClearFields() = 0;
	void Data2Fields();
	void Fields2Data();
	SGSPropInterface* GetPropInterface(){ return this; } // instead of casting away multiple inheritance..
	
	void AddField( sgsString key, StringView name, EDGUIProperty* prop );
	
	virtual EDGUIGroup& GetGroup() = 0;
	virtual bool IsScrEnt(){ return false; }
	
	sgsVariable m_data;
	Array< Field > m_fields;
};

struct EDGUIPropScrItem : EDGUIProperty, SGSPropInterface
{
	EDGUIPropScrItem( EDGUIPropVec3* posprop, const StringView& def = "" );
	~EDGUIPropScrItem();
	virtual void ClearFields();
	virtual int OnEvent( EDGUIEvent* e );
	virtual bool TakeValue( EDGUIProperty* src );
	
	void SetProps( sgsVariable var );
	sgsVariable GetProps();
	
	void OnTypeChange();
	
	// SGSPropInterface
	EDGUIGroup& GetGroup(){ return m_group; }
	
	EDGUIGroup m_group;
	EDGUIPropRsrc m_ctlScrItem;
	EDGUIPropVec3* m_pctlPos;
};

struct EdEntScripted : EdEntity, SGSPropInterface
{
	EdEntScripted( const char* enttype, bool isproto = true );
	~EdEntScripted();
	void ClearFields();
	
	EdEntScripted& operator = ( const EdEntScripted& o );
	virtual EdEntity* CloneEntity();
	
	virtual void Serialize( SVHTR& arch );
	virtual void Serialize( SVHTW& arch );
	virtual void Serialize( SVHBR& arch );
	virtual void Serialize( SVHBW& arch );
	
	void AddSelfToSEA( Array< LC_ScriptedEntity >& sea );
	virtual bool IsScriptedEnt(){ return true; }
	virtual void UpdateCache( LevelCache& LC );
	
	void _OnChange();
	virtual void RegenerateMesh();
	
	virtual int OnEvent( EDGUIEvent* e );
	virtual void DebugDraw();
	
	void AddButtonSubent( StringView type );
	void SetSpecialMesh( StringView path, const Mat4& mtx );
	void SetMeshInstanceCount( int count );
	void SetMeshInstanceData( int which, StringView path, const Mat4& mtx, int flags, float lmdetail );
	void GetMeshAABB( int which, Vec3 out[2] );
	void SetScriptedItem( StringView name, sgsVariable args );
	
	// SGSPropInterface
	EDGUIGroup& GetGroup(){ return m_group; }
	bool IsScrEnt(){ return true; }
	
	char m_typename[ 64 ];
	
	sgsVariable onChange;
	sgsVariable onDebugDraw;
	sgsVariable onGather;
	
	String m_subEntProto;
	EDGUIButton* m_subEntAddBtn;
	
	LevelCache* m_levelCache;
	
	Array< uint32_t > m_meshIDs;
	
	MeshInstHandle cached_specmeshinst;
	SGRX_ScriptedItem* cached_scritem;
};


struct EdEntNew : EdEntity, SGSPropInterface
{
	virtual void Serialize( SVHTR& arch ){}
	virtual void Serialize( SVHTW& arch ){}
	virtual void Serialize( SVHBR& arch ){}
	virtual void Serialize( SVHBW& arch ){}
	
	EdEntNew& operator = ( const EdEntNew& o );
	virtual EdEntity* CloneEntity();
	
	// SGSPropInterface
	void ClearFields();
	EDGUIGroup& GetGroup(){ return m_group; }
};



/////////////
////////////
///////////

extern sgs_RegIntConst g_ent_scripted_ric[];
extern sgs_RegFuncConst g_ent_scripted_rfc[];

struct EDGUIEntList : EDGUIGroup
{
#if 0
	struct Decl
	{
		const char* name;
		EdEntity* ent;
	};
#endif
	EDGUIEntList();
	~EDGUIEntList();
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIButton* m_buttons;
	int m_button_count;
};

#if 0
EdEntity* ENT_FindProtoByName( const char* name );

inline void World_AddObject( EdObject* obj );

template< class T > void ENT_SerializeSubentities( EdEntity* E, T& arch )
{
	int32_t size = E ? E->m_subEnts.size() : 0;
	arch( size, arch.version >= 4, 0 );
	if( T::IsReader )
	{
		if( E )
			E->m_subEnts.resize( size );
		for( int32_t i = 0; i < size; ++i )
		{
			EdEntity* ent = ENT_Unserialize( arch );
			if( E )
			{
				ent->m_ownerEnt = E;
				E->m_subEnts[ i ] = ent;
			}
			World_AddObject( ent );
		}
	}
	else
	{
		ASSERT( E && "need entity for serialization" );
		for( size_t i = 0; i < E->m_subEnts.size(); ++i )
			ENT_Serialize( arch, E->m_subEnts[ i ] );
	}
}

template< class T > void ENT_Serialize( T& arch, EdEntity* e )
{
	String ty = e->tyname;
	
	arch.marker( "ENTITY" );
	arch << ty;
	e->Serialize( arch );
	ENT_SerializeSubentities( e, arch );
}

template< class T > EdEntity* ENT_Unserialize( T& arch, bool fixMissing = false )
{
	String ty;
	
	arch.marker( "ENTITY" );
	arch << ty;
	
	EdEntity* e = ENT_FindProtoByName( StackString< 128 >( ty ) );
	if( !e )
	{
		if( fixMissing && ty != SV("Mesh") && ty != SV("Light") && ty != SV("Light sample") )
		{
			LOG_WARNING << "Failed to find entity, will try to fix: " << ty;
			SGS_CSCOPE( g_ScriptCtx->C );
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "ED_FIXENT_%s", StackString< 128 >( ty ).str );
			sgsVariable func = g_ScriptCtx->GetGlobal( bfr );
			if( func.not_null() )
			{
				Vec3 entpos;
				arch << entpos;
				Array< uint32_t > meshids;
				arch( meshids, arch.version >= 5 );
				String data;
				arch << data;
				g_ScriptCtx->Push( g_ScriptCtx->Unserialize( data ) );
				g_ScriptCtx->Call( func, 1 );
				ENT_SerializeSubentities( NULL, arch );
			}
			else
			{
				LOG_ERROR << "Could not find fixing function: " << bfr;
			}
		}
		else
		{
			LOG_ERROR << "FAILED TO FIND ENTITY: " << ty;
		}
		return NULL;
	}
	e = e->CloneEntity();
	e->Serialize( arch );
	ENT_SerializeSubentities( e, arch );
	e->RegenerateMesh();
	
	return e;
}
#endif


//
// WORLD
//

enum SelectionMask
{
	SelMask_Blocks = 0x1,
	SelMask_Patches = 0x2,
	SelMask_Entities = 0x4,
	SelMask_MeshPaths = 0x8,
	SelMask_ALL = 0xf
};

struct EdWorld : EDGUILayoutRow
{
	EdWorld();
	~EdWorld();
	
	template< class T > void Serialize( T& arch )
	{
		if( T::IsReader )
			Reset();
		
		arch.marker( "WORLD" );
		SerializeVersionHelper<T> svh( arch, MAP_FILE_VERSION );
		
		svh << m_ctlAmbientColor;
		svh << m_ctlDirLightDir;
		svh << m_ctlDirLightColor;
		svh << m_ctlDirLightDivergence;
		svh << m_ctlDirLightNumSamples;
		svh << m_ctlLightmapClearColor;
		int radNumBounces = 0;
		svh( radNumBounces, svh.version < 6 );
	//	svh << m_ctlRADNumBounces;
		svh << m_ctlLightmapDetail;
		svh << m_ctlLightmapBlurSize;
		svh << m_ctlAODistance;
		svh << m_ctlAOMultiplier;
		svh << m_ctlAOFalloff;
		svh << m_ctlAOEffect;
	//	svh << m_ctlAODivergence;
		svh << m_ctlAOColor;
		svh << m_ctlAONumSamples;
		
		if( svh.version >= 6 )
			svh << m_ctlSampleDensity;
		else if( T::IsReader )
			m_ctlSampleDensity.SetValue( 1.0f );
		
		if( T::IsWriter )
		{
			int32_t numblocks = m_blocks.size();
			svh << numblocks;
			for( size_t i = 0; i < m_blocks.size(); ++i )
			{
				svh << *m_blocks[ i ].item;
			}
			
			int32_t numents = 0;
			for( size_t i = 0; i < m_entities.size(); ++i )
				if( m_entities[ i ]->m_ownerEnt == NULL )
					numents++;
			svh << numents;
			for( size_t i = 0; i < m_entities.size(); ++i )
			{
#if 0
				if( m_entities[ i ]->m_ownerEnt == NULL )
					ENT_Serialize( svh, m_entities[ i ] );
#endif
			}
			
			int32_t numpatches = m_patches.size();
			svh << numpatches;
			for( size_t i = 0; i < m_patches.size(); ++i )
			{
				svh << *m_patches[ i ].item;
			}
			
			if( svh.version >= 8 )
			{
				int32_t numpaths = m_mpaths.size();
				svh << numpaths;
				for( size_t i = 0; i < m_mpaths.size(); ++i )
				{
					svh << *m_mpaths[ i ].item;
				}
			}
		}
		else
		{
			int32_t numblocks;
			svh << numblocks;
			for( int32_t i = 0; i < numblocks; ++i )
			{
				EdBlock* block = new EdBlock;
				svh << *block;
				AddObject( block );
			}
			
			int32_t numents;
			svh << numents;
			for( int32_t i = 0; i < numents; ++i )
			{
#if 0
				EdEntity* e = ENT_Unserialize( svh, true );
				if( e )
					AddObject( e );
#endif
			}
			
			int32_t numpatches;
			svh << numpatches;
			for( int32_t i = 0; i < numpatches; ++i )
			{
				EdPatch* patch = new EdPatch;
				svh << *patch;
				AddObject( patch );
			}
			
			if( svh.version >= 8 )
			{
				int32_t numpaths;
				svh << numpaths;
				for( int32_t i = 0; i < numpaths; ++i )
				{
					EdMeshPath* mpath = new EdMeshPath;
					svh << *mpath;
					AddObject( mpath );
				}
			}
		}
	}
	void FLoad( sgsVariable obj );
	sgsVariable FSave();
	
	void Reset();
	void TestData();
	void RegenerateMeshes();
	void DrawWires_Objects( EdObject* hl, bool tonedown = false );
	void DrawWires_Blocks( EdObject* hl );
	void DrawPoly_BlockSurf( int block, int surf, bool sel );
	void DrawPoly_BlockVertex( int block, int vert, bool sel );
	void DrawWires_Patches( EdObject* hl, bool tonedown = false );
	void DrawWires_Entities( EdObject* hl );
	void DrawWires_MeshPaths( EdObject* hl );
	bool RayObjectsIntersect( const Vec3& pos, const Vec3& dir, int searchfrom,
		float outdst[1], int outobj[1], EdObject** skip = NULL, int mask = SelMask_ALL );
	bool RayBlocksIntersect( const Vec3& pos, const Vec3& dir, int searchfrom,
		float outdst[1], int outblock[1], EdObject** skip = NULL, int mask = SelMask_ALL );
	bool RayEntitiesIntersect( const Vec3& pos, const Vec3& dir, int searchfrom,
		float outdst[1], int outent[1], EdObject** skip = NULL, int mask = SelMask_ALL );
	bool RayPatchesIntersect( const Vec3& pos, const Vec3& dir, int searchfrom,
		float outdst[1], int outent[1], EdObject** skip = NULL, int mask = SelMask_ALL );
	
//	EdEntity* CreateScriptedEntity( const StringView& name, sgsVariable params );
	void AddObject( EdObject* obj );
	void DeleteObject( EdObject* obj );
	
	void DeleteSelectedObjects();
	// returns if there were any selected blocks
	bool DuplicateSelectedObjectsAndMoveSelection();
	int GetNumSelectedObjects();
	int GetOnlySelectedObject();
	bool GetSelectedObjectAABB( Vec3 outaabb[2] );
	void SelectObject( int oid, bool mod );
	
	Vec3 FindCenterOfGroup( int32_t grp );
	void FixTransformsOfGroup( int32_t grp );
	void CopyObjectsToGroup( int32_t grpfrom, int32_t grpto );
	void TransferObjectsToGroup( int32_t grpfrom, int32_t grpto );
	void DeleteObjectsInGroup( int32_t grp );
	void ExportGroupAsOBJ( int32_t grp, const StringView& name );
	
	LC_Light GetDirLightInfo();
	
	EDGUIItem* GetObjProps( size_t oid )
	{
		EdObject* obj = m_objects[ oid ];
		if( obj->m_type == ObjType_Block )
		{
			m_ctlBlockProps.Prepare( (EdBlock*) obj );
			return &m_ctlBlockProps;
		}
		if( obj->m_type == ObjType_Entity )
		{
			return (EdEntity*) obj;
		}
		if( obj->m_type == ObjType_Patch )
		{
			m_ctlPatchProps.Prepare( (EdPatch*) obj );
			return &m_ctlPatchProps;
		}
		if( obj->m_type == ObjType_MeshPath )
		{
			m_ctlMeshPathProps.Prepare( (EdMeshPath*) obj );
			return &m_ctlMeshPathProps;
		}
		return NULL;
	}
	EDGUIItem* GetVertProps( size_t oid, size_t vid )
	{
		EdObject* obj = m_objects[ oid ];
		if( obj->m_type == ObjType_Block )
		{
			m_ctlVertProps.Prepare( (EdBlock*) obj, vid );
			return &m_ctlVertProps;
		}
		if( obj->m_type == ObjType_Patch )
		{
			m_ctlPatchVertProps.Prepare( (EdPatch*) obj, vid );
			return &m_ctlPatchVertProps;
		}
		if( obj->m_type == ObjType_MeshPath )
		{
			m_ctlMeshPathPointProps.Prepare( (EdMeshPath*) obj, vid );
			return &m_ctlMeshPathPointProps;
		}
		return NULL;
	}
	EDGUIItem* GetSurfProps( size_t oid, size_t sid )
	{
		EdObject* obj = m_objects[ oid ];
		if( obj->m_type == ObjType_Block )
		{
			m_ctlSurfProps.Prepare( (EdBlock*) obj, sid );
			return &m_ctlSurfProps;
		}
		return NULL;
	}
	
	VertexDeclHandle m_vd;
	
	Array< EdBlockHandle > m_blocks;
	Array< EdEntityHandle > m_entities;
	Array< EdPatchHandle > m_patches;
	Array< EdMeshPathHandle > m_mpaths;
	Array< EdObjectHandle > m_objects;
	EdGroupManager m_groupMgr;
	
	EDGUIGroup m_ctlGroup;
	EDGUIPropVec3 m_ctlAmbientColor;
	EDGUIPropVec2 m_ctlDirLightDir;
	EDGUIPropVec3 m_ctlDirLightColor;
	EDGUIPropFloat m_ctlDirLightDivergence;
	EDGUIPropInt m_ctlDirLightNumSamples;
	EDGUIPropVec3 m_ctlLightmapClearColor;
//	EDGUIPropInt m_ctlRADNumBounces;
	EDGUIPropFloat m_ctlLightmapDetail;
	EDGUIPropFloat m_ctlLightmapBlurSize;
	EDGUIPropFloat m_ctlAODistance;
	EDGUIPropFloat m_ctlAOMultiplier;
	EDGUIPropFloat m_ctlAOFalloff;
	EDGUIPropFloat m_ctlAOEffect;
//	EDGUIPropFloat m_ctlAODivergence;
	EDGUIPropVec3 m_ctlAOColor;
	EDGUIPropInt m_ctlAONumSamples;
	EDGUIPropFloat m_ctlSampleDensity;
	
	EDGUIBlockProps m_ctlBlockProps;
	EDGUIVertexProps m_ctlVertProps;
	EDGUISurfaceProps m_ctlSurfProps;
	EDGUIPatchProps m_ctlPatchProps;
	EDGUIPatchVertProps m_ctlPatchVertProps;
	EDGUIMeshPathProps m_ctlMeshPathProps;
	EDGUIMeshPathPointProps m_ctlMeshPathPointProps;
};

inline void World_AddObject( EdObject* obj ){ g_EdWorld->AddObject( obj ); }

struct EDGUIMultiObjectProps : EDGUILayoutRow
{
	EDGUIMultiObjectProps();
	void Prepare( bool selsurf = false );
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIGroup m_group;
	EDGUIPropRsrc m_mtl;
	bool m_selsurf;
};


// projection space without Z
Vec2 ED_GetCursorPos();
Vec2 ED_GetScreenPos( const Vec3& p );
Vec2 ED_GetScreenDir( const Vec3& d );
Vec2 ED_MovePointOnLine( const Vec2& p, const Vec2& lo, const Vec2& ld );


struct EdEditTransform
{
	virtual bool OnEnter()
	{
		m_startCursorPos = ED_GetCursorPos();
		SaveState();
		RecalcTransform();
		ApplyTransform();
		return true;
	}
	virtual void OnExit(){}
	virtual int OnViewEvent( EDGUIEvent* e ){ return 0; }
	virtual void Draw(){}
	
	virtual void SaveState(){}
	virtual void RestoreState(){}
	virtual void ApplyTransform(){}
	virtual void RecalcTransform(){}
	
	Vec2 m_startCursorPos;
};

struct EdBasicEditTransform : EdEditTransform
{
	virtual int OnViewEvent( EDGUIEvent* e );
};

struct EdBlockEditTransform : EdBasicEditTransform
{
	enum ConstraintMode
	{
		Camera,
		XAxis,
		XPlane,
		YAxis,
		YPlane,
		ZAxis,
		ZPlane,
	};
	struct SavedObject
	{
		int id;
		int offset;
	};
	
	virtual bool OnEnter();
	virtual int OnViewEvent( EDGUIEvent* e );
	virtual void SaveState();
	virtual void RestoreState();
	Vec3 GetMovementVector( const Vec2& a, const Vec2& b );
	
	Array< SavedObject > m_objStateMap;
	ByteArray m_objectStateData;
	ConstraintMode m_cmode;
	Vec3 m_origin;
	
	// extending
	bool m_extend;
	Vec3 m_xtdAABB[2];
	Vec3 m_xtdMask;
};

struct EdBlockMoveTransform : EdBlockEditTransform
{
	virtual int OnViewEvent( EDGUIEvent* e );
	virtual void Draw();
	virtual void ApplyTransform();
	virtual void RecalcTransform();
	
	Vec3 m_transform;
};

struct EdVertexMoveTransform : EdBlockMoveTransform
{
	EdVertexMoveTransform();
	virtual int OnViewEvent( EDGUIEvent* e );
	virtual void ApplyTransform();
	
	bool m_project;
};


struct EdEditMode
{
	virtual void OnEnter(){}
	virtual void OnExit(){}
	virtual void OnTransformEnd(){}
	virtual void OnDeleteObject( int oid ){ OnEnter(); }
	virtual int OnUIEvent( EDGUIEvent* e ){ return 0; }
	virtual void OnViewEvent( EDGUIEvent* e );
	virtual void Draw(){}
};

struct EdDrawBlockEditMode : EdEditMode
{
	enum ED_BlockDrawMode
	{
		BD_Polygon = 1,
		BD_BoxStrip = 2,
		BD_MeshPath = 3,
	};

	EdDrawBlockEditMode();
	void OnEnter();
	int OnUIEvent( EDGUIEvent* e );
	void OnViewEvent( EDGUIEvent* e );
	void Draw();
	void _AddNewBlock();
	
	ED_BlockDrawMode m_blockDrawMode;
	Array< Vec2 > m_drawnVerts;
	EDGUIPropFloat m_newBlockPropZ0;
	EDGUIPropFloat m_newBlockPropZ1;
	EDGUISurfaceProps m_newSurfProps;
};

#define NUM_AABB_ACTIVE_POINTS 26
struct EdEditBlockEditMode : EdEditMode
{
	EdEditBlockEditMode();
	void OnEnter();
	void OnTransformEnd();
	void OnViewEvent( EDGUIEvent* e );
	void Draw();
	bool _CanDo( ESpecialAction act );
	void _Do( ESpecialAction act );
	void _MouseMove();
	void _ReloadBlockProps();
	static Vec3 GetActivePointFactor( int i );
	Vec3 GetActivePoint( int i );
	bool IsActivePointSelectable( int i );
	int GetClosestActivePoint();
	static const char* GetActivePointExtName( int i );
	
	int m_selMask;
	int m_hlObj;
	int m_curObj;
	
	Vec3 m_selAABB[2];
	int m_numSel;
	int m_hlBBEl;
	
	EdBlockMoveTransform m_transform;
	EDGUIMultiObjectProps m_moprops;
};

struct EdEditVertexEditMode : EdEditMode
{
	struct ActivePoint
	{
		int block;
		int point;
	};
	
	void OnEnter();
	bool _CanDo( ESpecialAction act );
	void _Do( ESpecialAction act );
	void OnViewEvent( EDGUIEvent* e );
	void Draw();
	void _ReloadVertSurfProps();
	
	int GetNumObjectActivePoints( int b );
	Vec3 GetActivePoint( int b, int i );
	ActivePoint GetClosestActivePoint();
	
	bool m_canExtendSurfs;
	ActivePoint m_hlAP;
	Array< int > m_selObjList;
	EdVertexMoveTransform m_transform;
	EDGUIMultiObjectProps m_moprops;
};

struct EdPaintVertsEditMode : EdEditMode
{
	struct PaintVertex
	{
		Vec3 pos;
		Vec4 col;
		float factor;
	};
	
	EdPaintVertsEditMode();
	void OnEnter();
	void OnViewEvent( EDGUIEvent* e );
	int OnUIEvent( EDGUIEvent* e );
	void Draw();
	void _TakeSnapshot();
	void _DoPaint();
	
	int GetNumObjectActivePoints( int b );
	Vec3 GetActivePoint( int b, int i );
	
	bool m_isPainting;
	Array< int > m_selObjList;
	Array< PaintVertex > m_originalVerts;
	EDGUIPaintProps m_ctlPaintProps;
};

struct EdPaintSurfsEditMode : EdEditMode
{
	EdPaintSurfsEditMode();
	void OnEnter();
	void OnViewEvent( EDGUIEvent* e );
	void Draw();
	
	int m_paintBlock;
	int m_paintSurf;
	bool m_isPainting;
	EDGUISurfaceProps m_paintSurfProps;
};

struct EdAddEntityEditMode : EdEditMode
{
	EdAddEntityEditMode();
	void OnEnter();
	int OnUIEvent( EDGUIEvent* e );
	void OnViewEvent( EDGUIEvent* e );
	void Draw();
	void SetEntityType( const StringView& eh );
	void _AddNewEntity();
	
#if 0
	EdEntityHandle m_entityProps;
#endif
	String m_entType;
	EDGUIEntList m_entGroup;
};

struct EdEditGroupEditMode : EdEditMode
{
	void OnEnter();
	void Draw();
};

struct EdEditLevelEditMode : EdEditMode
{
	void OnEnter();
};


//
// MAIN FRAME
//
struct EDGUIMainFrame : EDGUIFrame, EDGUIRenderView::FrameInterface
{
	EDGUIMainFrame();
	void PostInit();
	int OnEvent( EDGUIEvent* e );
	bool ViewEvent( EDGUIEvent* e );
	void _DrawCursor( bool drawimg, float height );
	void DrawCursor( bool drawimg = true );
	void DebugDraw();
	
	void AddToParamList( EDGUIItem* item );
	void ClearParamList();
	void RefreshMouse();
	Vec3 GetCursorRayPos();
	Vec3 GetCursorRayDir();
	Vec2 GetCursorPlanePos();
	float GetCursorPlaneHeight();
	void SetCursorPlaneHeight( float z );
	bool IsCursorAiming();
	void Snap( Vec2& v );
	void Snap( Vec3& v );
	Vec2 Snapped( const Vec2& v );
	Vec3 Snapped( const Vec3& v );
	
	void ResetEditorState();
	void Level_New();
	void Level_Open();
	void Level_Save();
	void Level_SaveAs();
	void Level_Compile();
	void Level_Real_Open( const String& str );
	void Level_Real_Save( const String& str );
	void Level_Real_Compile();
	void SetEditMode( EdEditMode* em );
	void SetEditTransform( EdEditTransform* et );
	void SetModeHighlight( EDGUIButton* mybtn );
	
	String m_fileName;
	
	// EDIT MODES
	EdEditTransform* m_editTF;
	EdEditMode* m_editMode;
	EdDrawBlockEditMode m_emDrawBlock;
	EdEditBlockEditMode m_emEditObjs;
	EdEditVertexEditMode m_emEditVertex;
	EdPaintVertsEditMode m_emPaintVerts;
	EdPaintSurfsEditMode m_emPaintSurfs;
	EdAddEntityEditMode m_emAddEntity;
	EdEditGroupEditMode m_emEditGroup;
	EdEditLevelEditMode m_emEditLevel;
	
	// extra edit data
	TextureHandle m_txMarker;
	EDGUISnapProps m_snapProps;
	int m_keyMod;
	
	// core layout
	EDGUILayoutSplitPane m_UIMenuSplit;
	EDGUILayoutSplitPane m_UIMenuLRSplit;
	EDGUILayoutSplitPane m_UIParamSplit;
	EDGUILayoutColumn m_UIMenuButtonsLft;
	EDGUILayoutColumn m_UIMenuButtonsRgt;
	EDGUIVScroll m_UIParamScroll;
	EDGUILayoutRow m_UIParamList;
	EDGUIRenderView m_UIRenderView;

	// menu
	EDGUILabel m_MB_Cat0;
	EDGUIButton m_MBNew;
	EDGUIButton m_MBOpen;
	EDGUIButton m_MBSave;
	EDGUIButton m_MBSaveAs;
	EDGUIButton m_MBCompile;
	EDGUILabel m_MB_Cat1;
	EDGUIButton m_MBDrawBlock;
	EDGUIButton m_MBEditObjects;
	EDGUIButton m_MBPaintSurfs;
	EDGUIButton m_MBAddEntity;
	EDGUIButton m_MBEditGroups;
	EDGUIButton m_MBLevelInfo;
	
	// extra stuff
	EDGUIButton m_btnDumpLMInfo;
};


struct MapEditor : IGame
{
	bool OnInitialize();
	void OnDestroy();
	void OnEvent( const Event& e );
	void OnTick( float dt, uint32_t gametime );
	void SetBaseGame( BaseGame* game );
};



