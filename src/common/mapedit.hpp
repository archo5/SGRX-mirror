

#pragma once
#include "compiler.hpp"
#include "edutils.hpp"
#include "level.hpp"
#include "entities.hpp"
#include "resources.hpp"

#include <imgui.hpp>

#include "mapedit_graphics.hpp"


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


#ifdef MAPEDIT_DEFINE_GLOBALS
#  define MAPEDIT_GLOBAL( x ) x = NULL
#else
#  define MAPEDIT_GLOBAL( x ) extern x
#endif
MAPEDIT_GLOBAL( struct EdMainFrame* g_UIFrame );
MAPEDIT_GLOBAL( SceneHandle g_EdScene );
MAPEDIT_GLOBAL( struct EdWorld* g_EdWorld );
MAPEDIT_GLOBAL( struct EdLevelGraphicsCont* g_EdLGCont );
MAPEDIT_GLOBAL( BaseGame* g_BaseGame );
MAPEDIT_GLOBAL( GameLevel* g_Level );

MAPEDIT_GLOBAL( IMGUIFilePicker* g_NUILevelPicker );
MAPEDIT_GLOBAL( IMGUIMeshPicker* g_NUIMeshPicker );
MAPEDIT_GLOBAL( IMGUIFilePicker* g_NUIPartSysPicker );
MAPEDIT_GLOBAL( IMGUITexturePicker* g_NUITexturePicker );
MAPEDIT_GLOBAL( IMGUICharPicker* g_NUICharPicker );
MAPEDIT_GLOBAL( IMGUISoundPicker* g_NUISoundPicker );
MAPEDIT_GLOBAL( struct IMGUISurfMtlPicker* g_NUISurfMtlPicker );



struct SMPVertex
{
	Vec3 pos;
	Vec3 nrm;
	Vec4 tng;
	Vec2 tex;
};

struct IMGUISurfMtlPicker : IMGUIMeshPickerCore
{
	IMGUISurfMtlPicker()
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
		
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading surface materials";
		Clear();
	//	Entry e = { "", NULL };
	//	m_entries.push_back( e );
		
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
			AddMtl( m_materials.item( i ).key, m_materials.item( i ).value );
		}
		
		_Search( m_searchString );
	}
	void AddMtl( StringView name, MapMaterial* MM )
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
		Entry e = { name, mih };
		m_entries.push_back( e );
	}
	
	MeshHandle m_mesh;
	MapMaterialMap m_materials;
};

struct ImplEditorUIHelper : EditorUIHelper
{
	bool ResourcePicker( PickerType ptype, const char* caption, const char* label, String& value )
	{
		switch( ptype )
		{
		case PT_Mesh: return g_NUIMeshPicker->Property( caption, label, value );
		case PT_PartSys: return g_NUIPartSysPicker->Property( caption, label, value );
		case PT_Texture: return g_NUITexturePicker->Property( caption, label, value );
		case PT_Char: return g_NUICharPicker->Property( caption, label, value );
		case PT_Sound: return g_NUISoundPicker->Property( caption, label, value );
		default: return IMGUIEditString( label, value, 256 );
		}
	}
};



struct EdSnapProps
{
	EdSnapProps()
	{
		enableSnap = true;
		snapVerts = true;
		snapRange = 0.2f;
		snapGrid = 0.1f;
		projDist = 0.01f;
	}
	
	void EditUI()
	{
		IMGUIEditBool( "Enable snapping", enableSnap );
		IMGUIEditBool( "Snap to vertices", snapVerts );
		IMGUIEditFloat( "Max. distance", snapRange, 0.01, 1 );
		IMGUIEditFloat( "Grid unit size", snapGrid, 0.01, 100 );
		IMGUIEditFloat( "Proj. distance", projDist, 0, 1 );
	}
	
	static float Round( float v ){ return round( v ); }
	static Vec2 Round( Vec2 v ){ return V2( Round( v.x ), Round( v.y ) ); }
	static Vec3 Round( Vec3 v ){ return V3( Round( v.x ), Round( v.y ), Round( v.z ) ); }
	template< class T > void Snap( T& pos )
	{
		if( !enableSnap )
			return;
		
		if( snapVerts )
		{
		}
		
		pos /= snapGrid;
		pos = Round( pos );
		pos *= snapGrid;
	}
	
	bool enableSnap;
	bool snapVerts;
	float snapRange;
	float snapGrid;
	float projDist;
};



//
// GROUPS
//

struct EdGroup : SGRX_RefCounted
{
	EdGroup( struct EdGroupManager* groupMgr, int32_t id, int32_t pid, const StringView& name );
	Mat4 GetMatrix();
	StringView GetPath();
	EdGroup* Clone();
	
	StringView Name(){ return m_name; }
	
	template< class T > void Serialize( T& arch )
	{
		arch << m_origin;
		arch << m_position;
		arch << m_angles;
		arch << m_scaleUni;
	}
	
	void EditUI();
	
	EdGroupManager* m_groupMgr;
	int32_t m_id;
	int32_t m_parent_id;
	bool m_needsMtxUpdate;
	Mat4 m_mtxLocal;
	Mat4 m_mtxCombined;
	bool m_needsPathUpdate;
	String m_path;
	
	String m_name;
	String m_parent;
	Vec3 m_origin;
	Vec3 m_position;
	Vec3 m_angles;
	float m_scaleUni;
};
typedef Handle< EdGroup > EdGroupHandle;
typedef HashTable< int32_t, EdGroupHandle > EdGroupHandleMap;

struct IMGUIGroupPicker : IMGUIEntryPicker
{
	void Reload();
	EdGroupManager* m_groupMgr;
	int32_t m_ignoreID;
};

struct EdGroupManager
{
	EdGroupManager();
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
	void TransferGroupsToGroup( int32_t from, int32_t to );
	void QueueDestroy( EdGroup* grp );
	void ProcessDestroyQueue();
	void MatrixInvalidate( int32_t id );
	void PathInvalidate( int32_t id );
	void EditUI();
	void GroupProperty( const char* label, int& group );
	
	template< class T > void Serialize( T& arch );
	
	Array< EdGroupHandle > m_destroyQueue;
	EdGroupHandleMap m_groups;
	IMGUIGroupPicker m_grpPicker;
	int32_t m_lastGroupID;
	String m_editedGroup;
};



//
// OBJECT
//

enum EObjectType
{
	// not a type
	ObjType_NONE = 0,
	// EdObject-based
	ObjType_Block = 1,
	ObjType_Entity = 2,
	ObjType_Patch = 3,
	ObjType_MeshPath = 4,
	// non-EdObject-based
	ObjType_GameObject = 5,
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
typedef SerializeVersionHelper<ByteReader> SVHBR;
typedef SerializeVersionHelper<ByteWriter> SVHBW;

struct EdObject : SGRX_RefCounted
{
	EdObject( uint8_t ty ) : m_type(ty), selected(false), group(0) {}
	EdObject( const EdObject& o ) : m_type(o.m_type), selected(o.selected), group(o.group) {}
	virtual ~EdObject(){}
	uint8_t m_type;
	bool selected;
	int32_t group;
	
	virtual EdObject* Clone() = 0;
	virtual void Serialize( SVHTR& arch ) = 0;
	virtual void Serialize( SVHBR& arch ) = 0;
	virtual void Serialize( SVHBW& arch ) = 0;
	virtual void FLoad( sgsVariable data, int version ) = 0;
	virtual sgsVariable FSave( int version ) = 0;
	virtual Vec3 GetPosition() const = 0;
	virtual void SetPosition( const Vec3& p ) = 0;
	virtual bool RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1] ) const = 0;
	virtual void RegenerateMesh() = 0;
	virtual void EditUI(){}
	virtual void VertEditUI( int v ){}
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
	void Serialize( ByteReader& arch ){ SerializeVersionHelper<ByteReader> svh( arch, MAP_FILE_VERSION ); Serialize( svh ); }
	void Serialize( ByteWriter& arch ){ SerializeVersionHelper<ByteWriter> svh( arch, MAP_FILE_VERSION ); Serialize( svh ); }
	void UISelectElement( int i, bool mod );
	void ProjectSelectedVertices();
};

typedef Handle< EdObject > EdObjectHandle;

struct EdPaintProps
{
	EdPaintProps();
	
	void EditUI();
	float GetDistanceFactor( const Vec3& vpos, const Vec3& bpos );
	void Paint( Vec3& vpos, const Vec3& nrm, Vec4& vcol, float factor );
	void _UpdateColor();
	
	int layerNum;
	bool paintPos;
	bool paintColor;
	bool paintAlpha;
	float radius;
	float falloff;
	float sculptSpeed;
	float paintSpeed;
	Vec3 colorHSV;
	float alpha;
	
	Vec4 m_tgtColor;
};



template< class T > T FLoadProp( sgsVariable obj, const char* prop, const T& def )
{
	return FLoadVar( obj.getprop( prop ), def );
}
inline bool FLoadVar( sgsVariable v, bool def )
{
	return v.getdef( def );
}
inline int FLoadVar( sgsVariable v, int def )
{
	return v.getdef( def );
}
inline uint32_t FLoadVar( sgsVariable v, uint32_t def )
{
	return v.getdef( def );
}
inline float FLoadVar( sgsVariable v, float def )
{
	return v.getdef( def );
}
inline StringView FLoadVar( sgsVariable v, StringView def )
{
	return v.getdef( def );
}
inline Vec2 FLoadVar( sgsVariable v, Vec2 def )
{
	return V2( FLoadProp( v, "x", def.x ), FLoadProp( v, "y", def.y ) );
}
inline Vec3 FLoadVar( sgsVariable v, Vec3 def )
{
	return V3( FLoadProp( v, "x", def.x ), FLoadProp( v, "y", def.y ), FLoadProp( v, "z", def.z ) );
}
inline Quat FLoadVar( sgsVariable v, Quat def )
{
	return QUAT( FLoadProp( v, "x", def.x ), FLoadProp( v, "y", def.y ),
		FLoadProp( v, "z", def.z ), FLoadProp( v, "w", def.w ) );
}
inline SGRX_GUID FLoadVar( sgsVariable v, SGRX_GUID def )
{
	SGRX_GUID guid = SGRX_GUID::ParseString( FLoadVar( v, SV() ) );
	return guid.NotNull() ? guid : def;
}

inline sgsVariable FNewDict()
{
	return g_Level->GetScriptCtx().CreateDict();
}
inline sgsVariable FNewArray()
{
	return g_Level->GetScriptCtx().CreateArray();
}
inline void FArrayAppend( sgsVariable arr, sgsVariable val )
{
	SGS_CSCOPE( g_Level->GetSGSC() );
	g_Level->GetScriptCtx().Push( val );
	sgs_ArrayPush( g_Level->GetSGSC(), arr.var, 1 );
}
template< class T > void FSaveProp( sgsVariable obj, const char* prop, T value );
template< class T > void FSaveProp( sgsVariable obj, const char* prop, Array<T>& values );
inline sgsVariable FVar( bool val ){ return sgsVariable().set_bool( val ); }
inline sgsVariable FVar( int val ){ return sgsVariable().set_int( val ); }
inline sgsVariable FVar( uint32_t val ){ return sgsVariable().set_int( val ); }
inline sgsVariable FVar( float val ){ return sgsVariable().set_real( val ); }
inline sgsVariable FVar( StringView val )
{
	return g_Level->GetScriptCtx().CreateString( val );
}
inline sgsVariable FVar( Vec2 val )
{
	sgsVariable v = g_Level->GetScriptCtx().CreateDict();
	FSaveProp( v, "x", val.x );
	FSaveProp( v, "y", val.y );
	return v;
}
inline sgsVariable FVar( Vec3 val )
{
	sgsVariable v = g_Level->GetScriptCtx().CreateDict();
	FSaveProp( v, "x", val.x );
	FSaveProp( v, "y", val.y );
	FSaveProp( v, "z", val.z );
	return v;
}
inline sgsVariable FVar( Quat val )
{
	sgsVariable v = g_Level->GetScriptCtx().CreateDict();
	FSaveProp( v, "x", val.x );
	FSaveProp( v, "y", val.y );
	FSaveProp( v, "z", val.z );
	FSaveProp( v, "w", val.w );
	return v;
}
inline sgsVariable FVar( SGRX_GUID val )
{
	char bfr[ 36 ];
	val.ToCharArray( bfr, false, false );
	return FVar( StringView( bfr, 36 ) );
}
inline sgsVariable FIntVar( bool val ){ return sgsVariable().set_bool( val ); }
inline sgsVariable FIntVar( int val ){ return sgsVariable().set_int( val ); }
inline sgsVariable FIntVar( uint32_t val ){ return sgsVariable().set_int( val ); }
inline sgsVariable FIntVar( float val ){ return sgsVariable().set_real( val ); }
inline sgsVariable FIntVar( StringView val )
{
	return g_Level->GetScriptCtx().CreateString( val );
}
inline sgsVariable FIntVar( Vec2 val )
{
	return g_Level->GetScriptCtx().CreateVec2( val );
}
inline sgsVariable FIntVar( Vec3 val )
{
	return g_Level->GetScriptCtx().CreateVec3( val );
}
inline sgsVariable FIntVar( MeshHandle mh )
{
	sgs_PushVar( g_Level->GetSGSC(), mh );
	
	return sgsVariable( g_Level->GetSGSC(), sgsVariable::PickAndPop );
}
inline sgsVariable FIntVar( TextureHandle th )
{
	sgs_PushVar( g_Level->GetSGSC(), th );
	return sgsVariable( g_Level->GetSGSC(), sgsVariable::PickAndPop );
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
	obj.setprop( prop, arr );
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
	
	SGRX_GUID surface_guid;
	
	EdSurface() :
		texgenmode( ED_TEXGEN_COORDS ),
		xoff( 0 ), yoff( 0 ),
		scale( 1 ), aspect( 1 ),
		angle( 0 ), lmquality( 1 ),
		xfit( 0 ), yfit( 0 )
	{}
	EdSurface( Handle<EdSurface>::InitBeforeUnserialize ) :
		texgenmode( ED_TEXGEN_COORDS ),
		xoff( 0 ), yoff( 0 ),
		scale( 1 ), aspect( 1 ),
		angle( 0 ), lmquality( 1 ),
		xfit( 0 ), yfit( 0 )
	{}
	~EdSurface()
	{
		if( surface_guid.NotNull() )
			g_EdLGCont->DeleteSurface( surface_guid );
	}
	EdSurface& operator = ( const EdSurface& o )
	{
		texname = o.texname;
		texgenmode = o.texgenmode;
		xoff = o.xoff;
		yoff = o.yoff;
		scale = o.scale;
		aspect = o.aspect;
		angle = o.angle;
		lmquality = o.lmquality;
		xfit = o.xfit;
		yfit = o.yfit;
		return *this;
	}
	
	bool EditUI( struct EdBlock* B = NULL, int sid = -1 );
	template< class T > void Serialize( T& arch );
	void FLoad( sgsVariable data, int version )
	{
		UNUSED( version );
		SGRX_GUID oldsurfguid = surface_guid;
		surface_guid = FLoadProp( data, "surface_guid", SGRX_GUID::Null );
		if( surface_guid != oldsurfguid )
		{
			if( oldsurfguid.NotNull() )
				g_EdLGCont->DeleteSurface( oldsurfguid );
			if( surface_guid.NotNull() )
				g_EdLGCont->RequestSurface( surface_guid );
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
		FSaveProp( out, "surface_guid", surface_guid );
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
	EdBlock() : EdObject( ObjType_Block ), position(V3(0)), z0(0), z1(1){}
	~EdBlock()
	{
		if( solid_guid.NotNull() )
			g_EdLGCont->DeleteSolid( solid_guid );
	}
	
	Vec3 position;
	float z0, z1;
	
	Array< Vec3 > poly;
	Array< EdSurfHandle > surfaces;
	Array< bool > subsel;
	
	SGRX_GUID solid_guid;
	
	template< class T > void SerializeT( T& arch );
	void FLoad( sgsVariable data, int version )
	{
		// type already parsed
		SGRX_GUID oldsolidguid = solid_guid;
		solid_guid = FLoadProp( data, "solid_guid", SGRX_GUID::Null );
		if( solid_guid != oldsolidguid )
		{
			if( oldsolidguid.NotNull() )
				g_EdLGCont->DeleteSolid( oldsolidguid );
			if( solid_guid.NotNull() )
				g_EdLGCont->RequestSolid( solid_guid );
		}
		group = FLoadProp( data, "group", 0 );
		position = FLoadProp( data, "position", V3(0) );
		z0 = FLoadProp( data, "z0", 0.0f );
		z1 = FLoadProp( data, "z1", 0.0f );
		
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
		
		subsel.resize_using( GetNumVerts() + GetNumSurfs(), false );
		RegenerateMesh();
	}
	sgsVariable FSave( int version )
	{
		sgsVariable out = FNewDict();
		FSaveProp( out, "solid_guid", solid_guid );
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
	void EditUI();
	void VertEditUI( int vid );
	
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
	
	void GenCenterPos( EdSnapProps& SP );
	virtual Vec3 FindCenter() const;
	
	virtual EdObject* Clone();
	virtual void Serialize( SVHTR& arch ){ SerializeT( arch ); }
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
	
	template< class T > void Serialize( T& arch );
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
	void EditUI()
	{
		IMGUIEditVec3( "Position", pos, -8192, 8192 );
		IMGUIEditVec2( "Texcoord 0", tex[0], -8192, 8192 );
		IMGUIEditVec2( "Texcoord 1", tex[1], -8192, 8192 );
		IMGUIEditVec2( "Texcoord 2", tex[2], -8192, 8192 );
		IMGUIEditVec2( "Texcoord 3", tex[3], -8192, 8192 );
		IMGUIEditColorRGBA32( "Color 0", col[0] );
		IMGUIEditColorRGBA32( "Color 1", col[1] );
		IMGUIEditColorRGBA32( "Color 2", col[2] );
		IMGUIEditColorRGBA32( "Color 3", col[3] );
	}
};

struct EdPatchLayerInfo
{
	EdPatchLayerInfo() :
		xoff(0), yoff(0),
		scale(1), aspect(1),
		angle(0){}
	~EdPatchLayerInfo()
	{
		if( surface_guid.NotNull() )
			g_EdLGCont->DeleteSurface( surface_guid );
	}
	
	template< class T > void Serialize( T& arch );
	void FLoad( sgsVariable data, int version )
	{
		UNUSED( version );
		SGRX_GUID oldsurfguid = surface_guid;
		surface_guid = FLoadProp( data, "surface_guid", SGRX_GUID::Null );
		if( surface_guid != oldsurfguid )
		{
			if( oldsurfguid.NotNull() )
				g_EdLGCont->DeleteSurface( oldsurfguid );
			if( surface_guid.NotNull() )
				g_EdLGCont->RequestSurface( surface_guid );
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
		FSaveProp( out, "surface_guid", surface_guid );
		FSaveProp( out, "texture", texname );
		FSaveProp( out, "xoff", xoff );
		FSaveProp( out, "yoff", yoff );
		FSaveProp( out, "scale", scale );
		FSaveProp( out, "aspect", aspect );
		FSaveProp( out, "angle", angle );
		return out;
	}
	void EditUI( struct EdPatch* P, int lid );
	
	String texname;
	float xoff, yoff;
	float scale, aspect;
	float angle;
	
	SGRX_GUID surface_guid;
};

enum EPatchRenderMode
{
	PRM_Solid = 0,
	PRM_Transparent,
	PRM_Decal,
};

struct EdPatch : EdObject
{
	EdPatch() : EdObject( ObjType_Patch ), xsize(0), ysize(0),
		blend(0), renderMode(PRM_Solid), m_isLMSolid(false),
		m_isPhySolid(false), lmquality(1)
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
	
	template< class T > void SerializeT( T& arch );
	void FLoad( sgsVariable data, int version )
	{
		// type already parsed
		group = FLoadProp( data, "group", 0 );
		position = FLoadProp( data, "position", V3(0) );
		xsize = FLoadProp( data, "xsize", 2 );
		ysize = FLoadProp( data, "ysize", 2 );
		blend = FLoadProp( data, "blend", 0 ) & ~0x80;
		renderMode = FLoadProp( data, "renderMode", int(PRM_Solid) );
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
		FSaveProp( out, "renderMode", renderMode );
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
	
	void EditUI();
	void VertEditUI( int vid );
	
	static EdPatch* CreatePatchFromSurface( EdBlock& B, int sid );
	
	Vec3 position;
	EdPatchVtx vertices[ MAX_PATCH_WIDTH * MAX_PATCH_WIDTH ];
	uint16_t edgeflip[ MAX_PATCH_WIDTH ]; // 0 - [\], 1 - [/]
	uint16_t vertsel[ MAX_PATCH_WIDTH ];
	int8_t xsize;
	int8_t ysize;
	uint8_t blend;
	uint8_t renderMode;
	bool m_isLMSolid;
	bool m_isPhySolid;
	float lmquality;
	EdPatchLayerInfo layers[ MAX_PATCH_LAYERS ];
};

typedef Handle< EdPatch > EdPatchHandle;


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
	void EditUI()
	{
		IMGUIEditVec3( "Position", pos, -8192, 8192 );
		IMGUIEditBool( "Smooth", smooth );
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
		angle(0){}
	~EdMeshPathPart()
	{
		if( surface_guid.NotNull() )
			g_EdLGCont->DeleteSurface( surface_guid );
	}
	
	template< class T > void Serialize( T& arch );
	void FLoad( sgsVariable data, int version )
	{
		UNUSED( version );
		SGRX_GUID oldsurfguid = surface_guid;
		surface_guid = FLoadProp( data, "surface_guid", SGRX_GUID::Null );
		if( surface_guid != oldsurfguid )
		{
			if( oldsurfguid.NotNull() )
				g_EdLGCont->DeleteSurface( oldsurfguid );
			if( surface_guid.NotNull() )
				g_EdLGCont->RequestSurface( surface_guid );
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
		FSaveProp( out, "surface_guid", surface_guid );
		FSaveProp( out, "texture", texname );
		FSaveProp( out, "xoff", xoff );
		FSaveProp( out, "yoff", yoff );
		FSaveProp( out, "scale", scale );
		FSaveProp( out, "aspect", aspect );
		FSaveProp( out, "angle", angle );
		return out;
	}
	void EditUI();
	
	String texname;
	float xoff, yoff;
	float scale, aspect;
	float angle;
	
	SGRX_GUID surface_guid;
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
		m_isLMSolid( true ), m_isPhySolid( false ), m_skipCut( false ), m_doSmoothing( false ), m_isDynamic( false ),
		m_intervalScaleOffset(V2(1,0)), m_pipeModeOvershoot(0),
		m_rotAngles( V3(0) ), m_scaleUni( 1 ), m_scaleSep( V3(1) ), m_turnMode(0)
	{
	}
	
	virtual EdObject* Clone();
	virtual void Serialize( SVHTR& arch ){ SerializeT( arch ); }
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
	
	template< class T > void SerializeT( T& arch );
	void FLoad( sgsVariable data, int version )
	{
		// type already parsed
		group = FLoadProp( data, "group", 0 );
		m_position = FLoadProp( data, "position", V3(0) );
		m_meshName = FLoadProp( data, "meshName", SV() );
		m_lmquality = FLoadProp( data, "lmquality", 1.0f );
		m_isLMSolid = FLoadProp( data, "isLMSolid", true );
		m_isPhySolid = FLoadProp( data, "isPhySolid", false );
		m_skipCut = FLoadProp( data, "skipCut", false );
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
		FSaveProp( out, "skipCut", m_skipCut );
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
	
	void EditUI();
	void VertEditUI( int vid );
	
	Vec3 m_position;
	String m_meshName;
	float m_lmquality;
	bool m_isLMSolid;
	bool m_isPhySolid;
	bool m_skipCut;
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



//
// FIELDS
//

enum FieldType
{
	FT_Bool,
	FT_Int,
	FT_Float,
	FT_Vec2,
	FT_Vec3,
	FT_String,
	FT_EnumSB,
	FT_Mesh,
	FT_Tex,
	FT_Char,
	FT_PartSys,
	FT_Sound,
};
struct FieldBase : SGRX_RefCounted
{
	FieldBase( FieldType t ) : type( t ){}
	virtual FieldBase* Clone() = 0;
	virtual void EditUI() = 0;
	virtual void SetFromVar( sgsVariable var ) = 0;
	virtual sgsVariable ToVar() = 0;
	virtual void SetFromIntVar( sgsVariable var ) = 0;
	virtual sgsVariable ToIntVar() = 0;
	sgsString key;
	sgsString caption;
	FieldType type;
};
typedef Handle< FieldBase > HField;
#define EFIELD_TYPE_( nm, ty, def, xtra ) struct Field##nm : FieldBase \
{ typedef ty T; T value; \
	xtra \
	Field##nm( FieldType t = FT_##nm ) : FieldBase( t ){} \
	FieldBase* Clone(){ return new Field##nm( *this ); } \
	virtual void EditUI(); \
	void SetFromVar( sgsVariable var ){ value = FLoadVar( var, def ); } \
	sgsVariable ToVar(){ return FVar( value ); } \
	void SetFromIntVar( sgsVariable var ){ value = var.get<ty>(); } \
	sgsVariable ToIntVar(){ return FIntVar( value ); } \
}
#define EFIELD_TYPE( nm, ty, def ) EFIELD_TYPE_( nm, ty, def, ; )
EFIELD_TYPE( Bool, bool, false );
EFIELD_TYPE_( Int, int32_t, int32_t(0), T vmin; T vmax; );
EFIELD_TYPE_( Float, float, 0.0f, float vmin; float vmax; int prec; );
EFIELD_TYPE_( Vec2, Vec2, V2(0), float vmin; float vmax; int prec; );
EFIELD_TYPE_( Vec3, Vec3, V3(0), float vmin; float vmax; int prec; );
EFIELD_TYPE( String, String, String() );
struct FieldEnumSB : FieldInt
{
	FieldEnumSB() : FieldInt( FT_EnumSB ){}
	virtual void EditUI(){ m_picker.Property( caption.c_str(), value ); }
	IMGUIEnumPicker m_picker;
};



//
// ENTITIES
//

struct EdEntity : EdObject
{
	EdEntity( sgsString type, bool isproto );
	~EdEntity();
	
	const Vec3& Pos() const { return m_pos; }
	virtual Vec3 GetPosition() const { return Pos(); }
	virtual void SetPosition( const Vec3& pos );
	virtual void ScaleVertices( const Vec3& ){}
	
	virtual void EditUI();
	
	virtual EdObject* Clone();
	EdEntity& operator = ( const EdEntity& o );
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
	
	virtual void Serialize( SVHTR& arch ){}
	virtual void Serialize( SVHBR& arch );
	virtual void Serialize( SVHBW& arch );
	void FLoad( sgsVariable data, int version );
	sgsVariable FSave( int version );
	
	void SetID( StringView idstr );
	void Data2Fields();
	void Fields2Data();
	void AddField( sgsString key, sgsString name, FieldBase* F );
	
	bool m_isproto;
	Vec3 m_pos;
	TextureHandle m_iconTex;
	
	sgsString m_entityType;
	sgsVariable m_data;
	Array< HField > m_fields;
	
	sgsVariable m_entIface;
};

typedef Handle< EdEntity > EdEntityHandle;

extern sgs_RegIntConst g_ent_scripted_ric[];
extern sgs_RegFuncConst g_ent_scripted_rfc[];

struct EdEntList
{
	struct Entity
	{
		sgsString name;
		EdEntityHandle eh;
	};
	
	EdEntList();
	void EditUI();
	EdEntityHandle CurEntity(){ return which < entities.size() ? entities[ which ].eh : NULL; }
	
	Array< Entity > entities;
	size_t which;
};



//
// GAME OBJECT
//

bool EDGO_RayIntersect( GameObject* obj, Vec3 rpos, Vec3 rdir, float outdst[1] );
void EDGO_EditUI( GameObject* obj );
void EDGO_SerializeSpatial( GameObject* obj, ByteReader& br );
void EDGO_SerializeSpatial( GameObject* obj, ByteWriter& bw );
GameObject* EDGO_FLoad( sgsVariable data );
sgsVariable EDGO_FSave( GameObject* obj, bool guids = true );
GameObject* EDGO_Clone( GameObject* obj );
void EDGO_LCSave( GameObject* obj, LC_GameObject* out );

extern sgs_RegFuncConst g_imgui_rfc[];



/////////////
////////////
///////////


//
// WORLD
//

struct EdWorldBasicInfo
{
	EdWorldBasicInfo()
	{
		prefabMode = false;
	}
	
	void FLoad( sgsVariable data );
	sgsVariable FSave();
	void EditUI();
	
	bool prefabMode;
//	Array< Vec3 > points;
};

struct EdWorldLightingInfo
{
	EdWorldLightingInfo()
	{
		ambientColor = V3(0,0,0.1f);
		dirLightDir = V2(0);
		dirLightColor = V3(0);
		dirLightDvg = 10;
		dirLightNumSamples = 15;
		lightmapClearColor = V3(0);
	//	radNumBounces = 2;
		lightmapDetail = 2;
		lightmapBlurSize = 1;
		aoDist = 2;
		aoMult = 1;
		aoFalloff = 2;
		aoEffect = 0;
	//	aoDivergence = 0;
		aoColor = V3(0);
		aoNumSamples = 15;
		sampleDensity = 1.0f;
	}
	
	void FLoad( sgsVariable data );
	sgsVariable FSave();
	void EditUI();
	
	Vec3 ambientColor;
	Vec2 dirLightDir;
	Vec3 dirLightColor;
	float dirLightDvg;
	int32_t dirLightNumSamples;
	Vec3 lightmapClearColor;
//	int32_t radNumBounces;
	float lightmapDetail;
	float lightmapBlurSize;
	float aoDist;
	float aoMult;
	float aoFalloff;
	float aoEffect;
//	float aoDivergence;
	Vec3 aoColor;
	int32_t aoNumSamples;
	float sampleDensity;
	String skyboxTexture;
	String clutTexture;
};

enum SelectionMask
{
	SelMask_Blocks = 0x1,
	SelMask_Patches = 0x2,
	SelMask_Entities = 0x4,
	SelMask_MeshPaths = 0x8,
	SelMask_GameObjects = 0x10,
	SelMask_ALL = 0x1f
};

struct EdObjIdx
{
	uint8_t type; // ObjType_*
	union
	{
		EdObject* edobj;
		GameObject* gameobj;
		void* ptr;
	};
	
	EdObjIdx() : type( ObjType_NONE ), ptr( NULL ){}
	EdObjIdx( EdObject* o ) : type( o->m_type ), edobj( o ){}
	EdObjIdx( GameObject* o ) : type( ObjType_GameObject ), gameobj( o ){}
	EdObject* GetEdObject() const { return type != ObjType_NONE && type != ObjType_GameObject ? edobj : NULL; }
	GameObject* GetGameObject() const { return type == ObjType_GameObject ? gameobj : NULL; }
	bool Valid() const { return type != ObjType_NONE; }
	bool operator == ( const EdObjIdx& o ) const { return type == o.type && ptr == o.ptr; }
};
inline Hash HashVar( const EdObjIdx& idx )
{
	return HashVar( idx.type ) | HashVar( idx.ptr );
}
typedef Array< EdObjIdx > EdObjIdxArray;

struct EdWorld
{
	EdWorld();
	~EdWorld();
	
	template< class T > void Serialize( T& arch );
	void FLoad( sgsVariable obj );
	sgsVariable FSave();
	
	void Reset();
	void TestData();
	void ReloadSkybox();
	void ReloadCLUT();
	
	void GetAllObjects( EdObjIdxArray& out );
	void RegenerateMeshes();
	void DrawWires_Objects( const EdObjIdx& hl, bool tonedown = false );
	void DrawWires_Blocks( const EdObjIdx& hl );
	void DrawPoly_BlockSurf( const EdObjIdx& block, int surf, bool sel );
	void DrawPoly_BlockVertex( const EdObjIdx& block, int vert, bool sel );
	void DrawWires_Patches( const EdObjIdx& hl, bool tonedown = false );
	void DrawWires_Entities( const EdObjIdx& hl );
	void DrawWires_MeshPaths( const EdObjIdx& hl );
	void DrawWires_GameObjects( const EdObjIdx& hl );
	
	bool RayObjectsIntersect( const Vec3& pos, const Vec3& dir, EdObjIdx searchfrom,
		float outdst[1], EdObjIdx outobj[1], EdObjIdx* skip = NULL, int mask = SelMask_ALL );
	
//	EdEntity* CreateScriptedEntity( const StringView& name, sgsVariable params );
	void AddObject( EdObject* obj, bool regen = true );
	void DeleteObject( EdObjIdx idx, bool update = true );
	
	void DeleteSelectedObjects();
	// returns if there were any selected blocks
	bool DuplicateSelectedObjectsAndMoveSelection();
	int GetNumSelectedObjects();
	EdObjIdx GetOnlySelectedObject();
	bool GetSelectedObjectAABB( Vec3 outaabb[2] );
	
#define SELOBJ_ONLY -1
#define SELOBJ_TOGGLE -2
#define SELOBJ_ENABLE 1
#define SELOBJ_DISABLE 0
	void SelectObject( EdObjIdx idx, int mod );
	
	Vec3 FindCenterOfGroup( int32_t grp );
	void FixTransformsOfGroup( int32_t grp );
	void CopyObjectsToGroup( int32_t grpfrom, int32_t grpto );
	void TransferObjectsToGroup( int32_t grpfrom, int32_t grpto );
	void DeleteObjectsInGroup( int32_t grp );
	void ExportGroupAsOBJ( int32_t grp, const StringView& name );
	
	LC_Light GetDirLightInfo();
	
	void EditUI();
	
	void VertEditUI( EdObjIdx idx, size_t vid )
	{
		EdObject* obj = idx.GetEdObject();
		if( obj )
		{
			ImGui::BeginChangeCheck();
			obj->VertEditUI( vid );
			if( ImGui::EndChangeCheck() )
				obj->RegenerateMesh();
		}
	}
	void SurfEditUI( EdObjIdx idx, size_t sid )
	{
		EdObject* obj = idx.GetEdObject();
		if( obj )
		{
			ImGui::BeginChangeCheck();
			if( obj->m_type == ObjType_Block )
			{
				SGRX_CAST( EdBlock*, B, obj );
				B->surfaces[ sid ]->EditUI( B, sid );
			}
			if( ImGui::EndChangeCheck() )
				obj->RegenerateMesh();
		}
	}
	
	Mat4 GetGroupMatrix( int grp ){ return m_groupMgr.GetMatrix( grp ); }
	
	void SetEntityID( EdEntity* e );
	
	VertexDeclHandle m_vd;
	
	int32_t m_nextID;
	Array< EdObjectHandle > m_edobjs;
	HashTable< EdObjIdx, NoValue > m_selection;
	EdGroupManager m_groupMgr;
	
	EdWorldBasicInfo m_info;
	EdWorldLightingInfo m_lighting;
};

inline void World_AddObject( EdObject* obj ){ g_EdWorld->AddObject( obj ); }

struct EdMultiObjectProps
{
	EdMultiObjectProps();
	void Prepare( bool selsurf = false );
	void OnSetMtl( StringView name );
	void EditUI();
	
	String m_mtl;
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
	virtual int ViewUI(){ return 0; }
	virtual void Draw(){}
	
	virtual void SaveState(){}
	virtual void RestoreState(){}
	virtual void ApplyTransform(){}
	virtual void RecalcTransform(){}
	
	Vec2 m_startCursorPos;
};

struct EdBasicEditTransform : EdEditTransform
{
	virtual int ViewUI();
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
		EdObjIdx idx;
		int offset;
	};
	
	EdBlockEditTransform();
	virtual bool OnEnter();
	virtual int ViewUI();
	virtual void SaveState();
	virtual void RestoreState();
	Vec3 GetMovementVector( const Vec2& a, const Vec2& b );
	
	Array< SavedObject > m_objStateMap;
	ByteArray m_objectStateData;
	ConstraintMode m_cmode;
	Vec3 m_origin;
	bool m_subpointCenter;
	
	// extending
	bool m_extend;
	Vec3 m_xtdAABB[2];
	Vec3 m_xtdMask;
};

struct EdBlockMoveTransform : EdBlockEditTransform
{
	virtual int ViewUI();
	virtual void Draw();
	virtual void ApplyTransform();
	virtual void RecalcTransform();
	
	Vec3 m_transform;
};

struct EdVertexMoveTransform : EdBlockMoveTransform
{
	EdVertexMoveTransform();
	virtual int ViewUI();
	virtual void ApplyTransform();
	
	bool m_project;
};


struct EdEditMode
{
	virtual void OnEnter(){}
	virtual void OnExit(){}
	virtual void OnTransformEnd(){}
	virtual void ViewUI(){}
	virtual void EditUI(){}
	virtual void Draw(){}
};

struct EdDrawBlockEditMode : EdEditMode
{
	enum ED_BlockDrawMode
	{
		BD_Polygon = 1,
		BD_BoxStrip = 2,
		BD_MeshPath = 3,
		BD_Entity = 4,
		BD_GameObject = 5,
	};

	EdDrawBlockEditMode();
	void OnEnter();
	void ViewUI();
	void EditUI();
	void Draw();
	void _AddNewBlock();
	void _AddNewEntity();
	
	int m_blockDrawMode;
	Array< Vec2 > m_drawnVerts;
	float m_newZ0;
	float m_newZ1;
	EdSurface m_newSurf;
	EdEntList m_entGroup;
};

#define NUM_AABB_ACTIVE_POINTS 26
struct EdEditBlockEditMode : EdEditMode
{
	EdEditBlockEditMode();
	void OnEnter();
	void OnTransformEnd();
	void ViewUI();
	void EditUI();
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
	
	unsigned m_selMask;
	EdObjIdx m_hlObj;
	EdObjIdx m_curObj;
	
	Vec3 m_selAABB[2];
	int m_numSel;
	int m_hlBBEl;
	
	EdBlockMoveTransform m_transform;
	EdMultiObjectProps m_moprops;
};

struct EdEditVertexEditMode : EdEditMode
{
	struct ActivePoint
	{
		EdObjIdx block;
		int point;
	};
	
	void OnEnter();
	bool _CanDo( ESpecialAction act );
	void _Do( ESpecialAction act );
	void ViewUI();
	void EditUI();
	void Draw();
	
	ActivePoint GetClosestActivePoint();
	
	bool m_canExtendSurfs;
	ActivePoint m_hlAP;
	Array< EdObjIdx > m_selObjList;
	EdVertexMoveTransform m_transform;
	EdMultiObjectProps m_moprops;
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
	void ViewUI();
	void EditUI();
	void Draw();
	void _TakeSnapshot();
	void _DoPaint();
	
	bool m_isPainting;
	Array< EdObjIdx > m_selObjList;
	Array< PaintVertex > m_originalVerts;
	EdPaintProps m_paintProps;
};

struct EdPaintSurfsEditMode : EdEditMode
{
	EdPaintSurfsEditMode();
	void OnEnter();
	void ViewUI();
	void EditUI();
	void Draw();
	
	EdObjIdx m_paintBlock;
	int m_paintSurf;
	bool m_isPainting;
	EdSurface m_paintSurfTemplate;
};

struct EdEditGroupEditMode : EdEditMode
{
	void EditUI();
	void Draw();
};


struct MapEditorRenderView : IMGUIRenderView
{
	MapEditorRenderView() : IMGUIRenderView( g_EdScene ){}
	void DebugDraw();
};


//
// MAIN FRAME
//
struct EdMainFrame
{
	EdMainFrame();
	bool ViewUI();
	void EditUI();
	void _DrawCursor( bool drawimg, float height );
	void DrawCursor( bool drawimg = true );
	void DebugDraw();
	void OnDeleteObjects();
	
	Vec3 GetCursorRayPos();
	Vec3 GetCursorRayDir();
	Vec3 GetCursorPos();
	Vec2 GetCursorPlanePos();
	float GetCursorPlaneHeight();
	void SetCursorPlaneHeight( float z );
	bool IsCursorAiming();
	void Snap( Vec2& v );
	void Snap( Vec3& v );
	Vec2 Snapped( const Vec2& v );
	Vec3 Snapped( const Vec3& v );
	
	void Level_New();
	bool Level_Real_Open( const StringView& str );
	bool Level_Real_Save( const StringView& str );
	void Level_Real_Compile();
	void Level_Real_Compile_Default();
	void Level_Real_Compile_Prefabs();
	void SetEditMode( EdEditMode* em );
	void SetEditTransform( EdEditTransform* et );
	
	String m_fileName;
	
	// EDIT MODES
	EdEditTransform* m_editTF;
	EdEditMode* m_editMode;
	EdDrawBlockEditMode m_emDrawBlock;
	EdEditBlockEditMode m_emEditObjs;
	EdEditVertexEditMode m_emEditVertex;
	EdPaintVertsEditMode m_emPaintVerts;
	EdPaintSurfsEditMode m_emPaintSurfs;
	EdEditGroupEditMode m_emEditGroup;
	
	// extra edit data
	TextureHandle m_txMarker;
	EdSnapProps m_snapProps;
	MapEditorRenderView m_NUIRenderView;
};


enum EditorMode
{
	CreateObjs,
	EditObjects,
	PaintSurfs,
	EditGroups,
	LevelInfo,
	MiscProps,
};

struct MapEditor : IGame
{
	bool OnInitialize();
	void OnDestroy();
	void OnEvent( const Event& e );
	void OnTick( float dt, uint32_t gametime );
	void SetBaseGame( BaseGame* game );
};


#include "mapedit_serialize.inl"

