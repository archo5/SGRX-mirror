

#pragma once
#define USE_HASHTABLE
#include "compiler.hpp"
#include "edutils.hpp"
#include "edcomui.hpp"


// v0: initial
// v1: added surface.lmquality
// v2: added ent[light].flareoffset
// v3: added surface.xfit/yfit, added groups
#define MAP_FILE_VERSION 3

#define MAX_BLOCK_POLYGONS 32

#define MAX_PATCH_WIDTH 16
#define MAX_PATCH_LAYERS 4

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
MAPEDIT_GLOBAL( struct EdWorld* g_EdWorld );
MAPEDIT_GLOBAL( struct EDGUISurfTexPicker* g_UISurfTexPicker );
MAPEDIT_GLOBAL( struct EDGUIMeshPicker* g_UIMeshPicker );
MAPEDIT_GLOBAL( struct EDGUIPartSysPicker* g_UIPartSysPicker );
MAPEDIT_GLOBAL( struct EDGUISoundPicker* g_UISoundPicker );
MAPEDIT_GLOBAL( struct EDGUIScrFnPicker* g_UIScrFnPicker );
MAPEDIT_GLOBAL( struct EDGUILevelOpenPicker* g_UILevelOpenPicker );
MAPEDIT_GLOBAL( struct EDGUILevelSavePicker* g_UILevelSavePicker );
MAPEDIT_GLOBAL( struct EDGUIEntList* g_EdEntList );



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
// OBJECT
//

enum EObjectType
{
	ObjType_Block = 1,
	ObjType_Entity = 2,
	ObjType_Patch = 3,
};

enum ESpecialAction
{
	SA_None = 0,
	SA_Invert,
	SA_Subdivide,
	SA_Unsubdivide,
	SA_EdgeFlip,
	SA_Extend,
	SA_Remove,
	SA_ExtractPart,
	SA_DuplicatePart,
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



//
// BLOCKS
//

#define ED_TEXGEN_COORDS 0
#define ED_TEXGEN_STRETCH 1

struct EdSurface
{
	String texname;
	int texgenmode;
	float xoff, yoff;
	float scale, aspect;
	float angle;
	float lmquality;
	int xfit, yfit;
	
	EdSurface() :
		texgenmode( ED_TEXGEN_COORDS ),
		xoff( 0 ), yoff( 0 ),
		scale( 1 ), aspect( 1 ),
		angle( 0 ), lmquality( 1 ),
		xfit( 0 ), yfit( 0 )
	{}
	
	template< class T > void Serialize( T& arch )
	{
		arch.marker( "SURFACE" );
		arch << texname;
		arch << texgenmode;
		arch << xoff << yoff;
		arch << scale << aspect;
		arch << angle;
		arch( lmquality, arch.version >= 1, 1.0f );
		arch( xfit, arch.version >= 3, 0 );
		arch( yfit, arch.version >= 3, 0 );
		
		if( T::IsReader ) Precache();
	}
	
	TextureHandle cached_texture;
	
	void Precache()
	{
		char bfr[ 128 ];
		snprintf( bfr, sizeof(bfr), "textures/%.*s.png", (int) texname.size(), texname.data() );
		cached_texture = GR_GetTexture( bfr );
	}
};


typedef LevelCache::Vertex LCVertex;

struct EdBlock : EdObject
{
	EdBlock() : EdObject( ObjType_Block ), position(V3(0)), z0(0), z1(1){}
	
	Vec3 position;
	float z0, z1;
	
	Array< Vec3 > poly;
	Array< EdSurface > surfaces;
	Array< bool > subsel;
	
	MeshHandle cached_mesh;
	MeshInstHandle cached_meshinst;
	
	template< class T > void SerializeT( T& arch )
	{
		arch.marker( "BLOCK" );
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
	void GenerateMesh( LevelCache& LC );
	int GenerateSurface( LCVertex* outbuf, int sid );
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
};

struct EdPatchLayerInfo
{
	EdPatchLayerInfo() : xoff(0), yoff(0), scale(1), aspect(1), angle(0), lmquality(1){}
	
	void Precache()
	{
		char bfr[ 128 ];
		snprintf( bfr, sizeof(bfr), "textures/%.*s.png", (int) texname.size(), texname.data() );
		cached_texture = GR_GetTexture( bfr );
	}
	
	template< class T > void Serialize( T& arch )
	{
		arch << texname;
		arch << xoff << yoff;
		arch << scale << aspect;
		arch << angle;
		arch << lmquality;
	}
	
	String texname;
	float xoff, yoff;
	float scale, aspect;
	float angle;
	float lmquality;
	
	TextureHandle cached_texture;
	MeshHandle cached_mesh;
	MeshInstHandle cached_meshinst;
};

struct EdPatch : EdObject
{
	EdPatch() : EdObject( ObjType_Patch ), xsize(0), ysize(0), blend(0)
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
	
	template< class T > void SerializeT( T& arch )
	{
		arch.marker( "PATCH" );
		arch << group;
		arch << position;
		arch << xsize << ysize;
		arch << blend;
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
	
	static EdPatch* CreatePatchFromSurface( EdBlock& B, int sid );
	
	Vec3 position;
	EdPatchVtx vertices[ MAX_PATCH_WIDTH * MAX_PATCH_WIDTH ];
	uint16_t edgeflip[ MAX_PATCH_WIDTH ]; // 0 - [\], 1 - [/]
	uint16_t vertsel[ MAX_PATCH_WIDTH ];
	int8_t xsize;
	int8_t ysize;
	uint8_t blend;
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
	EDGUIPropFloat m_lmquality;
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
	EDGUIPatchLayerProps m_layerProps[4];
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
	
	void LoadIcon();
	
	const Vec3& Pos() const { return m_ctlPos.m_value; }
	virtual Vec3 GetPosition() const { return Pos(); }
	virtual void SetPosition( const Vec3& pos ){ m_ctlPos.SetValue( pos ); }
	virtual void ScaleVertices( const Vec3& ){}
	
	virtual int OnEvent( EDGUIEvent* e ){ return EDGUILayoutRow::OnEvent( e ); }
	
	virtual void UpdateCache( LevelCache& LC ){}
	
	virtual EdEntity* CloneEntity() = 0;
	
	virtual EdObject* Clone(){ return CloneEntity(); }
	virtual bool RayIntersect( const Vec3& rpos, const Vec3& rdir, float outdst[1] ) const
	{
		return RaySphereIntersect( rpos, rdir, Pos(), 0.2f, outdst );
	}
	virtual Vec3 FindCenter() const { return Pos(); }
	virtual void RegenerateMesh(){}
	virtual void DebugDraw(){}
	
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
};

typedef Handle< EdEntity > EdEntityHandle;

struct EDGUIEntButton : EDGUIButton
{
	EDGUIEntButton()
	{
		tyname = "entity-button";
	}
	
	EdEntityHandle m_ent_handle;
};



/////////////
////////////
///////////

struct EdEntMesh : EdEntity
{
	EdEntMesh( bool isproto = true );
	
	const String& Mesh() const { return m_ctlMesh.m_value; }
	const Vec3& RotAngles() const { return m_ctlAngles.m_value; }
	float ScaleUni() const { return m_ctlScaleUni.m_value; }
	const Vec3& ScaleSep() const { return m_ctlScaleSep.m_value; }
	Mat4 Matrix() const { return Mat4::CreateSRT( m_ctlScaleSep.m_value * m_ctlScaleUni.m_value, DEG2RAD( m_ctlAngles.m_value ), m_ctlPos.m_value ); }
	
	EdEntMesh& operator = ( const EdEntMesh& o );
	virtual EdEntity* CloneEntity();
	
	template< class T > void SerializeT( T& arch )
	{
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
	
	virtual void UpdateCache( LevelCache& LC );
	virtual int OnEvent( EDGUIEvent* e );
	virtual void DebugDraw();
	virtual void RegenerateMesh();
	
	EDGUIPropVec3 m_ctlAngles;
	EDGUIPropFloat m_ctlScaleUni;
	EDGUIPropVec3 m_ctlScaleSep;
	EDGUIPropRsrc m_ctlMesh;
	
	MeshHandle cached_mesh;
	MeshInstHandle cached_meshinst;
};

struct EdEntLight : EdEntity
{
	EdEntLight( bool isproto = true );
	
	float Range() const { return m_ctlRange.m_value; }
	float Power() const { return m_ctlPower.m_value; }
	const Vec3& ColorHSV() const { return m_ctlColorHSV.m_value; }
	float LightRadius() const { return m_ctlLightRadius.m_value; }
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
		arch << m_ctlPos;
		arch << m_ctlRange;
		arch << m_ctlPower;
		arch << m_ctlColorHSV;
		arch << m_ctlLightRadius;
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
	virtual void UpdateCache( LevelCache& LC );
	
	EDGUIPropFloat m_ctlRange;
	EDGUIPropFloat m_ctlPower;
	EDGUIPropVec3 m_ctlColorHSV;
	EDGUIPropFloat m_ctlLightRadius;
	EDGUIPropInt m_ctlShSampleCnt;
	EDGUIPropFloat m_ctlFlareSize;
	EDGUIPropVec3 m_ctlFlareOffset;
	EDGUIPropBool m_ctlIsSpotlight;
	EDGUIPropVec3 m_ctlSpotRotation;
	EDGUIPropFloat m_ctlSpotInnerAngle;
	EDGUIPropFloat m_ctlSpotOuterAngle;
	EDGUIPropFloat m_ctlSpotCurve;
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
	
	virtual void UpdateCache( LevelCache& LC );
};

struct EdEntScripted : EdEntity
{
	struct Field
	{
		String key;
		EDGUIProperty* property;
	};
	
	EdEntScripted( const char* enttype, bool isproto = true );
	~EdEntScripted();
	
	EdEntScripted& operator = ( const EdEntScripted& o );
	virtual EdEntity* CloneEntity();
	
	void Data2Fields();
	void Fields2Data();
	
	virtual void Serialize( SVHTR& arch );
	virtual void Serialize( SVHTW& arch );
	virtual void Serialize( SVHBR& arch );
	virtual void Serialize( SVHBW& arch );
	
	virtual void UpdateCache( LevelCache& LC );
	
	virtual void RegenerateMesh();
	
	virtual int OnEvent( EDGUIEvent* e );
	virtual void DebugDraw();
	
	void AddFieldBool( sgsString key, sgsString name, bool def );
	void AddFieldInt( sgsString key, sgsString name, int32_t def = 0, int32_t min = (int32_t) 0x80000000, int32_t max = (int32_t) 0x7fffffff );
	void AddFieldFloat( sgsString key, sgsString name, float def = 0, int prec = 2, float min = -FLT_MAX, float max = FLT_MAX );
	void AddFieldVec2( sgsString key, sgsString name, Vec2 def = V2(0), int prec = 2, Vec2 min = V2(-FLT_MAX), Vec2 max = V2(FLT_MAX) );
	void AddFieldVec3( sgsString key, sgsString name, Vec3 def = V3(0), int prec = 2, Vec3 min = V3(-FLT_MAX), Vec3 max = V3(FLT_MAX) );
	void AddFieldString( sgsString key, sgsString name, sgsString def );
	void AddFieldRsrc( sgsString key, sgsString name, EDGUIRsrcPicker* rsrcPicker, sgsString def );
	void SetMesh( sgsString name );
	void SetMeshInstanceCount( int count );
	void SetMeshInstanceMatrix( int which, const Mat4& mtx );
	void GetMeshAABB( Vec3 out[2] );
	
	char m_typename[ 64 ];
	sgsVariable m_data;
	sgsVariable onChange;
	sgsVariable onDebugDraw;
	sgsVariable onGather;
	Array< Field > m_fields;
	
	LevelCache* m_levelCache;
	
	MeshHandle cached_mesh;
	Array< MeshInstHandle > cached_meshinsts;
};



/////////////
////////////
///////////

extern sgs_RegFuncConst g_ent_scripted_rfc[];

struct EDGUIEntList : EDGUIGroup
{
	struct Decl
	{
		const char* name;
		EdEntity* ent;
	};
	
	EDGUIEntList();
	~EDGUIEntList();
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIEntButton* m_buttons;
	int m_button_count;
};

EdEntity* ENT_FindProtoByName( const char* name );

template< class T > void ENT_Serialize( T& arch, EdEntity* e )
{
	String ty = e->tyname;
	
	arch.marker( "ENTITY" );
	arch << ty;
	arch << *e;
}

template< class T > EdEntity* ENT_Unserialize( T& arch )
{
	String ty;
	
	arch.marker( "ENTITY" );
	arch << ty;
	
	EdEntity* e = ENT_FindProtoByName( StackString< 128 >( ty ) );
	if( !e )
	{
		LOG_ERROR << "FAILED TO FIND ENTITY: " << ty;
		return NULL;
	}
	e = e->CloneEntity();
	e->Serialize( arch );
	e->RegenerateMesh();
	
	return e;
}


//
// WORLD
//

struct EdWorld : EDGUILayoutRow
{
	EdWorld();
	
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
		svh << m_ctlRADNumBounces;
		svh << m_ctlLightmapDetail;
		svh << m_ctlLightmapBlurSize;
		svh << m_ctlAODistance;
		svh << m_ctlAOMultiplier;
		svh << m_ctlAOFalloff;
		svh << m_ctlAOEffect;
	//	svh << m_ctlAODivergence;
		svh << m_ctlAOColor;
		svh << m_ctlAONumSamples;
		
		if( T::IsWriter )
		{
			int32_t numblocks = m_blocks.size();
			svh << numblocks;
			for( size_t i = 0; i < m_blocks.size(); ++i )
			{
				svh << *m_blocks[ i ].item;
			}
			
			int32_t numents = m_entities.size();
			svh << numents;
			for( size_t i = 0; i < m_entities.size(); ++i )
			{
				ENT_Serialize( svh, m_entities[ i ] );
			}
			
			int32_t numpatches = m_patches.size();
			svh << numpatches;
			for( size_t i = 0; i < m_patches.size(); ++i )
			{
				svh << *m_patches[ i ].item;
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
				EdEntity* e = ENT_Unserialize( svh );
				if( e )
					AddObject( e );
			}
			
			int32_t numpatches;
			svh << numpatches;
			for( int32_t i = 0; i < numpatches; ++i )
			{
				EdPatch* patch = new EdPatch;
				svh << *patch;
				AddObject( patch );
			}
		}
	}
	
	void Reset();
	void TestData();
	void RegenerateMeshes();
	void DrawWires_Objects( EdObject* hl, bool tonedown = false );
	void DrawWires_Blocks( EdObject* hl );
	void DrawPoly_BlockSurf( int block, int surf, bool sel );
	void DrawPoly_BlockVertex( int block, int vert, bool sel );
	void DrawWires_Patches( EdObject* hl, bool tonedown = false );
	void DrawWires_Entities( EdObject* hl );
	bool RayObjectsIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outobj[1], EdObject** skip = NULL );
	bool RayBlocksIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outblock[1], EdObject** skip = NULL );
	bool RayEntitiesIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outent[1], EdObject** skip = NULL );
	bool RayPatchesIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outent[1], EdObject** skip = NULL );
	
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
	Array< EdObjectHandle > m_objects;
	EdGroupManager m_groupMgr;
	
	EDGUIGroup m_ctlGroup;
	EDGUIPropVec3 m_ctlAmbientColor;
	EDGUIPropVec2 m_ctlDirLightDir;
	EDGUIPropVec3 m_ctlDirLightColor;
	EDGUIPropFloat m_ctlDirLightDivergence;
	EDGUIPropInt m_ctlDirLightNumSamples;
	EDGUIPropVec3 m_ctlLightmapClearColor;
	EDGUIPropInt m_ctlRADNumBounces;
	EDGUIPropFloat m_ctlLightmapDetail;
	EDGUIPropFloat m_ctlLightmapBlurSize;
	EDGUIPropFloat m_ctlAODistance;
	EDGUIPropFloat m_ctlAOMultiplier;
	EDGUIPropFloat m_ctlAOFalloff;
	EDGUIPropFloat m_ctlAOEffect;
//	EDGUIPropFloat m_ctlAODivergence;
	EDGUIPropVec3 m_ctlAOColor;
	EDGUIPropInt m_ctlAONumSamples;
	
	EDGUIBlockProps m_ctlBlockProps;
	EDGUIVertexProps m_ctlVertProps;
	EDGUISurfaceProps m_ctlSurfProps;
	EDGUIPatchProps m_ctlPatchProps;
	EDGUIPatchVertProps m_ctlPatchVertProps;
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

struct EdBlockVertexMoveTransform : EdBlockMoveTransform
{
	EdBlockVertexMoveTransform();
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
	void _ReloadBlockProps();
	static Vec3 GetActivePointFactor( int i );
	Vec3 GetActivePoint( int i );
	bool IsActivePointSelectable( int i );
	int GetClosestActivePoint();
	static const char* GetActivePointExtName( int i );
	
	int m_hlObj;
	int m_curObj;
	
	Vec3 m_selAABB[2];
	int m_numSel;
	int m_hlBBEl;
	
	EdBlockMoveTransform m_transform;
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
	EdBlockVertexMoveTransform m_transform;
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
	void SetEntityType( const EdEntityHandle& eh );
	void _AddNewEntity();
	
	EdEntityHandle m_entityProps;
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
};



