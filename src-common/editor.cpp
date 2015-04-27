

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_MAT4
#define USE_ARRAY
#define USE_SERIALIZATION
#include <engine.hpp>
#include <edgui.hpp>
#include <script.hpp>
#include <sound.hpp>

#include "compiler.hpp"



#define MAX_BLOCK_POLYGONS 32



ScriptContext* g_ScriptCtx;
struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
struct EdWorld* g_EdWorld;
struct EDGUISurfTexPicker* g_UISurfTexPicker;
struct EDGUIMeshPicker* g_UIMeshPicker;
struct EDGUIPartSysPicker* g_UIPartSysPicker;
struct EDGUISoundPicker* g_UISoundPicker;
struct EDGUIScrFnPicker* g_UIScrFnPicker;
struct EDGUILevelOpenPicker* g_UILevelOpenPicker;
struct EDGUILevelSavePicker* g_UILevelSavePicker;



// returns level name or empty string if level is not saved yet
StringView GetLevelName();



static void prepmeshinst( MeshInstHandle mih )
{
	mih->dynamic = 1;
	for( int i = 10; i < 16; ++i )
		mih->constants[ i ] = V4(0.5f);
}



struct EDGUISnapProps : EDGUILayoutRow
{
	EDGUISnapProps() :
		m_group( true, "Snapping properties" ),
		m_enableSnap( true ),
		m_snapVerts( true ),
		m_snapRange( 0.2f, 2, 0.01f, 1.0f ),
		m_snapGrid( 0.1f, 2, 0.01f, 100.0f )
	{
		tyname = "snapprops";
		
		m_enableSnap.caption = "Enable snapping";
		m_snapVerts.caption = "Snap to vertices";
		m_snapRange.caption = "Max. distance";
		m_snapGrid.caption = "Grid unit size";
		
		m_group.Add( &m_enableSnap );
		m_group.Add( &m_snapVerts );
		m_group.Add( &m_snapRange );
		m_group.Add( &m_snapGrid );
		Add( &m_group );
	}
	
	bool IsSnapEnabled(){ return m_enableSnap.m_value; }
	bool IsSnapVertices(){ return m_snapVerts.m_value; }
	float GetSnapMaxDist(){ return m_snapRange.m_value; }
	float GetSnapGridSize(){ return m_snapGrid.m_value; }
	
	EDGUIGroup m_group;
	EDGUIPropBool m_enableSnap;
	EDGUIPropBool m_snapVerts;
	EDGUIPropFloat m_snapRange;
	EDGUIPropFloat m_snapGrid;
};

void SP_Snap( EDGUISnapProps& SP, Vec2& pos );


struct EDGUISurfTexPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUISurfTexPicker()
	{
		caption = "Pick a texture";
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading textures";
		m_options.clear();
		m_textures.clear();
		FS_IterateDirectory( "textures", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[T]: " << name;
		if( !isdir && name.ends_with( ".png" ) )
		{
			m_options.push_back( name.part( 0, name.size() - 4 ) );
			m_textures.push_back( GR_GetTexture( String_Concat( "textures/", name ) ) );
		}
		return true;
	}
	void _DrawItem( int i, int x0, int y0, int x1, int y1 )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		
		br.Col( 1 );
		br.SetTexture( m_textures[ i ] );
		br.Quad( x0 + 10, y0 + 4, x1 - 10, y1 - 16 );
		
		br.Col( 0.9f, 1.0f );
		GR2D_DrawTextLine( ( x0 + x1 ) / 2, y1 - 8, m_options[ i ], HALIGN_CENTER, VALIGN_CENTER );
	}
	
	Array< TextureHandle > m_textures;
};


struct EDGUIMeshPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUIMeshPicker() :
		m_scene( GR_CreateScene() )
	{
		caption = "Pick a mesh";
		m_meshinst = m_scene->CreateMeshInstance();
		prepmeshinst( m_meshinst );
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading meshes";
		m_options.clear();
		m_meshes.clear();
		FS_IterateDirectory( "meshes", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[M]: " << name;
		if( !isdir && name.ends_with( ".ssm" ) )
		{
			m_options.push_back( name.part( 0, name.size() - 4 ) );
			m_meshes.push_back( GR_GetMesh( String_Concat( "meshes/", name ) ) );
		}
		return true;
	}
	void _DrawItem( int i, int x0, int y0, int x1, int y1 )
	{
		SGRX_Viewport vp = { x0 + 10, y0 + 4, x1 - 10, y1 - 16 };
		
		if( m_meshes[ i ] )
		{
			SGRX_IMesh* M = m_meshes[ i ];
			m_meshinst->mesh = m_meshes[ i ];
			m_scene->camera.position = M->m_boundsMax + ( M->m_boundsMax - M->m_boundsMin ) * 0.5f + V3(0.1f);
			m_scene->camera.direction = ( M->m_boundsMin - M->m_boundsMax ).Normalized();
			m_scene->camera.znear = 0.1f;
			m_scene->camera.angle = 60;
			m_scene->camera.UpdateMatrices();
			
			SGRX_RenderScene rsinfo( V4( GetTimeMsec() / 1000.0f ), m_scene );
			rsinfo.viewport = &vp;
			GR_RenderScene( rsinfo );
		}
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Col( 0.9f, 1.0f );
		GR2D_DrawTextLine( ( x0 + x1 ) / 2, y1 - 8, m_options[ i ], HALIGN_CENTER, VALIGN_CENTER );
	}
	
	Array< MeshHandle > m_meshes;
	SceneHandle m_scene;
	MeshInstHandle m_meshinst;
};


struct EDGUIPartSysPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUIPartSysPicker()
	{
		caption = "Pick a particle system";
		Reload();
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	void Reload()
	{
		LOG << "Reloading particle systems";
		m_options.clear();
		FS_IterateDirectory( "psys", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[P]: " << name;
		if( !isdir && name.ends_with( ".psy" ) )
		{
			m_options.push_back( name.part( 0, name.size() - 4 ) );
		}
		return true;
	}
};


struct EDGUISoundPicker : EDGUIRsrcPicker
{
	EDGUISoundPicker()
	{
		sys = SND_CreateSystem();
		sys->Load( "sound/master.bank" );
		sys->Load( "sound/master.strings.bank" );
		caption = "Pick a sound event";
		Reload();
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	void Reload()
	{
		LOG << "Enumerating sound events";
		sys->EnumerateSoundEvents( m_options );
		_Search( m_searchString );
	}
	SoundSystemHandle sys;
};


struct EDGUIScrFnPicker : EDGUIRsrcPicker
{
	EDGUIScrFnPicker()
	{
		caption = "Pick a script function to use";
		Reload();
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	void Reload()
	{
		LOG << "Reloading script functions";
		m_options.clear();
		m_options.push_back( "" );
		
		StringView levelname;
		if( !( levelname = GetLevelName() ) )
		{
			LOG << "SAVE LEVEL BEFORE OPENING SCRIPTS";
			_Search( m_searchString );
			return;
		}
		
		char bfr[ 256 ];
		sprintf( bfr, "levels/%s.sgs", (const char*) StackString< 220 >( levelname ) );
		g_ScriptCtx->PushEnv();
		if( g_ScriptCtx->ExecFile( bfr ) )
		{
			ScriptVarIterator iter = g_ScriptCtx->GlobalIterator();
			while( iter.Advance() )
			{
				sgsString str = iter.GetKey().get_string();
				if( str.size() )
					m_options.push_back( StringView( str.c_str(), str.size() ) );
			}
		}
		g_ScriptCtx->PopEnv();
		
		_Search( m_searchString );
	}
};


struct EDGUILevelPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUILevelPicker(){ Reload(); }
	void Reload()
	{
		LOG << "Reloading levels";
		m_options.clear();
		FS_IterateDirectory( "levels", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[L]: " << name;
		if( !isdir && name.ends_with( ".tle" ) )
		{
			m_options.push_back( name.part( 0, name.size() - 4 ) );
		}
		return true;
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( e->type == EDGUI_EVENT_PROPCHANGE && e->target == &m_confirm )
		{
			if( m_confirm.m_value == 1 )
			{
				EDGUIRsrcPicker::_OnPickResource();
			}
			return 1;
		}
		return EDGUIRsrcPicker::OnEvent( e );
	}
	
	EDGUIQuestion m_confirm;
};

struct EDGUILevelOpenPicker : EDGUILevelPicker
{
	EDGUILevelOpenPicker()
	{
		caption = "Pick a level to open";
		m_confirm.caption = "Do you really want to open the level? All unsaved changes will be lost!";
	}
	virtual void _OnPickResource()
	{
		m_confirm.Open( this );
		m_frame->Add( &m_confirm );
	}
};

struct EDGUILevelSavePicker : EDGUILevelPicker
{
	EDGUILevelSavePicker()
	{
		caption = "Pick a level to save or write the name";
		m_confirm.caption = "Do you really want to overwrite the level?";
	}
	virtual void _OnPickResource()
	{
		if( m_options.find_first_at( m_pickedOption ) == NOT_FOUND )
			EDGUIRsrcPicker::_OnPickResource();
		else
		{
			m_confirm.Open( this );
			m_frame->Add( &m_confirm );
		}
	}
	virtual void _OnConfirm()
	{
		_OnPickResource();
	}
};



//
// B
// L
// O
// C
// K
// S
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
	
	EdSurface() : texgenmode( ED_TEXGEN_COORDS ), xoff( 0 ), yoff( 0 ), scale( 1 ), aspect( 1 ), angle( 0 ), lmquality( 1 ){}
	
	template< class T > void Serialize( T& arch )
	{
		arch.marker( "SURFACE" );
		arch << texname;
		arch << texgenmode;
		arch << xoff << yoff;
		arch << scale << aspect;
		arch << angle;
		arch( lmquality, arch.version >= 1, 1.0f );
		
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

#define EdVtx_DECL "pf3nf30f21f2"
struct EdVtx
{
	Vec3 pos;
	Vec3 dummynrm;
	float tx0, ty0;
	float tx1, ty1;
	
	bool operator == ( const EdVtx& o ) const
	{
		return pos == o.pos
			&& tx0 == o.tx0
			&& ty0 == o.ty0
			&& tx1 == o.tx1
			&& ty1 == o.ty1;
	}
};

struct EdBlock
{
	EdBlock() : position(V2(0)), z0(0), z1(1){}
	
	Vec2 position;
	float z0, z1;
	
	Array< Vec3 > poly;
	Array< EdSurface > surfaces;
	
	MeshHandle cached_mesh;
	MeshInstHandle cached_meshinst;
	
	template< class T > void Serialize( T& arch )
	{
		arch.marker( "BLOCK" );
		arch << position;
		arch << z0 << z1;
		arch << poly;
		arch << surfaces;
		
		if( T::IsReader ) RegenerateMesh();
	}
	
	void _GetTexVecs( int surf, Vec3& tgx, Vec3& tgy )
	{
		if( surf < (int) poly.size() )
		{
			size_t v0 = surf, v1 = ( surf + 1 ) % poly.size(); 
			Vec2 edgedir = ( poly[ v1 ] - poly[ v0 ] ).ToVec2().Normalized();
			tgx = Vec3::Create( edgedir.x, edgedir.y, 0 );
			tgy = Vec3::Create( 0, 0, -1 );
		}
		else if( surf == (int) poly.size() )
		{
			tgx = Vec3::Create( -1, 0, 0 );
			tgy = Vec3::Create( 0, 1, 0 );
		}
		else
		{
			tgx = Vec3::Create( -1, 0, 0 );
			tgy = Vec3::Create( 0, -1, 0 );
		}
	}
	uint16_t _AddVtx( const Vec3& vpos, float z, const EdSurface& S, const Vec3& tgx, const Vec3& tgy, Array< EdVtx >& vertices, uint16_t voff )
	{
		EdVtx V = { { vpos.x, vpos.y, z }, {0,0,1}, 0, 0, 0, 0 };
		
		V.tx0 = Vec3Dot( V.pos + V3( position.x, position.y, 0 ), tgx ) / S.scale + S.xoff;
		V.ty0 = Vec3Dot( V.pos + V3( position.x, position.y, 0 ), tgy ) / S.scale * S.aspect + S.yoff;
		
		size_t off = vertices.find_first_at( V, voff );
		if( off != NOT_FOUND )
			return (uint16_t) off - voff;
		vertices.push_back( V );
		return (uint16_t) vertices.size() - 1 - voff;
	}
	
	void GenCenterPos( EDGUISnapProps& SP )
	{
		if( !poly.size() )
			return;
		
		Vec2 cp = {0,0}, oldpos = position;
		for( size_t i = 0; i < poly.size(); ++i )
			cp += poly[i].ToVec2();
		cp /= poly.size();
		cp += position;
		
		SP_Snap( SP, cp );
		
		position = cp;
		for( size_t i = 0; i < poly.size(); ++i )
			poly[i].SetXY( poly[i].ToVec2() - cp - oldpos );
	}
	
	bool RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1], int* outsurf = NULL )
	{
		Vec3 pts[16];
		int pcount = poly.size();
		
		// TOP
		for( size_t i = 0; i < poly.size(); ++i )
		{
			pts[i] = V3( poly[i].x + position.x, poly[i].y + position.y, z1 );
		}
		if( RayPolyIntersect( rpos, dir, pts, pcount, outdst ) )
		{
			if( outsurf )
				*outsurf = poly.size();
			return true;
		}
		
		// BOTTOM
		for( size_t i = 0; i < poly.size(); ++i )
		{
			pts[ poly.size() - i - 1 ] = V3( poly[i].x + position.x, poly[i].y + position.y, z0 );
		}
		if( RayPolyIntersect( rpos, dir, pts, pcount, outdst ) )
		{
			if( outsurf )
				*outsurf = poly.size() + 1;
			return true;
		}
		
		// SIDES
		pcount = 4;
		for( size_t i = 0; i < poly.size(); ++i )
		{
			size_t i1 = ( i + 1 ) % poly.size();
			pts[0] = V3( poly[i].x + position.x, poly[i].y + position.y, z1 );
			pts[1] = V3( poly[i].x + position.x, poly[i].y + position.y, z0 );
			pts[2] = V3( poly[i1].x + position.x, poly[i1].y + position.y, z0 );
			pts[3] = V3( poly[i1].x + position.x, poly[i1].y + position.y, z1 );
			if( RayPolyIntersect( rpos, dir, pts, pcount, outdst ) )
			{
				if( outsurf )
					*outsurf = i;
				return true;
			}
		}
		return false;
	}
	
	void RegenerateMesh()
	{
		if( poly.size() < 3 || poly.size() > MAX_BLOCK_POLYGONS - 2 )
			return;
		
		if( !cached_mesh )
			cached_mesh = GR_CreateMesh();
		if( !cached_meshinst )
		{
			cached_meshinst = g_EdScene->CreateMeshInstance();
			cached_meshinst->mesh = cached_mesh;
			prepmeshinst( cached_meshinst );
		}
		cached_meshinst->matrix = Mat4::CreateTranslation( position.x, position.y, 0 );
		for( size_t i = 0; i < surfaces.size(); ++i )
			surfaces[ i ].Precache();
		
		VertexDeclHandle vd = GR_GetVertexDecl( EdVtx_DECL );
		Array< EdVtx > vertices;
		Array< uint16_t > indices;
		SGRX_MeshPart meshparts[ MAX_BLOCK_POLYGONS ];
		int numparts = 0;
		
		// SIDES
		if( z0 != z1 )
		{
			for( size_t i = 0; i < poly.size(); ++i )
			{
				SGRX_MeshPart mp = { vertices.size(), 0, indices.size(), 0 };
				
				Vec3 tgx, tgy;
				_GetTexVecs( i, tgx, tgy );
				size_t i1 = ( i + 1 ) % poly.size();
				uint16_t v1 = _AddVtx( poly[i], z0, surfaces[i], tgx, tgy, vertices, mp.vertexOffset );
				uint16_t v2 = _AddVtx( poly[i], z1 + poly[i].z, surfaces[i], tgx, tgy, vertices, mp.vertexOffset );
				uint16_t v3 = _AddVtx( poly[i1], z1 + poly[i1].z, surfaces[i], tgx, tgy, vertices, mp.vertexOffset );
				uint16_t v4 = _AddVtx( poly[i1], z0, surfaces[i], tgx, tgy, vertices, mp.vertexOffset );
				indices.push_back( v1 );
				indices.push_back( v2 );
				indices.push_back( v3 );
				indices.push_back( v3 );
				indices.push_back( v4 );
				indices.push_back( v1 );
				
				mp.vertexCount = vertices.size() - mp.vertexOffset;
				mp.indexCount = indices.size() - mp.indexOffset;
				
				MaterialHandle mh = GR_CreateMaterial();
				mh->shader = GR_GetSurfaceShader( "default" );
				mh->textures[ 0 ] = surfaces[ i ].cached_texture;
				mp.material = mh;
				
				meshparts[ numparts++ ] = mp;
			}
		}
		
		// TOP
		{
			SGRX_MeshPart mp = { vertices.size(), 0, indices.size(), 0 };
			
			Vec3 tgx, tgy;
			_GetTexVecs( poly.size(), tgx, tgy );
			for( size_t i = 0; i < poly.size(); ++i )
				_AddVtx( poly[i], z1 + poly[i].z, surfaces[ poly.size() ], tgx, tgy, vertices, mp.vertexOffset );
			for( size_t i = 2; i < poly.size(); ++i )
			{
				indices.push_back( 0 );
				indices.push_back( i );
				indices.push_back( i - 1 );
			}
			
			mp.vertexCount = vertices.size() - mp.vertexOffset;
			mp.indexCount = indices.size() - mp.indexOffset;
			
			MaterialHandle mh = GR_CreateMaterial();
			mh->shader = GR_GetSurfaceShader( "default" );
			mh->textures[ 0 ] = surfaces[ poly.size() ].cached_texture;
			mp.material = mh;
			
			meshparts[ numparts++ ] = mp;
		}
		
		// BOTTOM
		{
			SGRX_MeshPart mp = { vertices.size(), 0, indices.size(), 0 };
			
			Vec3 tgx, tgy;
			_GetTexVecs( poly.size() + 1, tgx, tgy );
			for( size_t i = 0; i < poly.size(); ++i )
				_AddVtx( poly[i], z0, surfaces[ poly.size() + 1 ], tgx, tgy, vertices, mp.vertexOffset );
			for( size_t i = 2; i < poly.size(); ++i )
			{
				indices.push_back( 0 );
				indices.push_back( i - 1 );
				indices.push_back( i );
			}
			
			mp.vertexCount = vertices.size() - mp.vertexOffset;
			mp.indexCount = indices.size() - mp.indexOffset;
			
			MaterialHandle mh = GR_CreateMaterial();
			mh->shader = GR_GetSurfaceShader( "default" );
			mh->textures[ 0 ] = surfaces[ poly.size() + 1 ].cached_texture;
			mp.material = mh;
			
			meshparts[ numparts++ ] = mp;
		}
		
		cached_mesh->SetAABBFromVertexData( vertices.data(), vertices.size_bytes(), vd );
		cached_mesh->SetVertexData( vertices.data(), vertices.size_bytes(), vd, false );
		cached_mesh->SetIndexData( indices.data(), indices.size_bytes(), false );
		cached_mesh->SetPartData( meshparts, numparts );
	}
	
	LevelCache::Vertex _MakeGenVtx( const Vec3& vpos, float z, const EdSurface& S, const Vec3& tgx, const Vec3& tgy )
	{
		LevelCache::Vertex V = { { vpos.x + position.x, vpos.y + position.y, z }, { 0, 0, 1 }, 0xffffffff, 0, 0, 0, 0 };
		
		V.tx0 = Vec3Dot( V.pos, tgx ) / S.scale + S.xoff;
		V.ty0 = Vec3Dot( V.pos, tgy ) / S.scale * S.aspect + S.yoff;
		
		return V;
	}
	void GenerateMesh( LevelCache& LC )
	{
		if( poly.size() < 3 || poly.size() > MAX_BLOCK_POLYGONS - 2 )
			return;
		
		// GENERATE PLANES
		Vec3 toppoly[ MAX_BLOCK_POLYGONS ];
		int topverts = 0;
		Vec4 planes[ MAX_BLOCK_POLYGONS ];
		int numplanes = 0;
		
		planes[ numplanes++ ] = V4( 0, 0, -1, -z0 );
		for( size_t i = 0; i < poly.size(); ++i )
		{
			Vec2 vpos = poly[ i ].ToVec2() + position;
			
			size_t i1 = ( i + 1 ) % poly.size();
			Vec2 dir = ( poly[ i1 ] - poly[ i ] ).ToVec2().Perp().Normalized();
			if( !dir.NearZero() )
			{
				planes[ numplanes++ ] = V4( dir.x, dir.y, 0, Vec2Dot( vpos, dir ) );
			}
			
			toppoly[ topverts++ ] = V3( vpos.x, vpos.y, z1 + poly[i].z );
		}
		if( PolyGetPlane( toppoly, topverts, planes[ numplanes ] ) )
			numplanes++;
		
		// ADD SOLID
		size_t solid = LC.AddSolid( planes, numplanes );
		
		// GENERATE MESH
		if( z0 != z1 )
		{
			for( size_t i = 0; i < poly.size(); ++i )
			{
				Vec3 tgx, tgy;
				_GetTexVecs( i, tgx, tgy );
				size_t i1 = ( i + 1 ) % poly.size();
				LevelCache::Vertex verts[] =
				{
					_MakeGenVtx( poly[i], z0, surfaces[i], tgx, tgy ),
					_MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[i], tgx, tgy ),
					_MakeGenVtx( poly[i1], z1 + poly[i1].z, surfaces[i], tgx, tgy ),
					_MakeGenVtx( poly[i1], z0, surfaces[i], tgx, tgy ),
				};
				LC.AddPoly( verts, 4, surfaces[ i ].texname, surfaces[ i ].lmquality, solid );
			}
		}
		
		// TOP
		{
			LevelCache::Vertex verts[ MAX_BLOCK_POLYGONS - 2 ];
			Vec3 tgx, tgy;
			_GetTexVecs( poly.size(), tgx, tgy );
			for( size_t i = 0; i < poly.size(); ++i )
				verts[ poly.size() - 1 - i ] = _MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[ poly.size() ], tgx, tgy );
			LC.AddPoly( verts, poly.size(), surfaces[ poly.size() ].texname, surfaces[ poly.size() ].lmquality, solid );
		}
		
		// BOTTOM
		{
			LevelCache::Vertex verts[ MAX_BLOCK_POLYGONS - 2 ];
			Vec3 tgx, tgy;
			_GetTexVecs( poly.size() + 1, tgx, tgy );
			for( size_t i = 0; i < poly.size(); ++i )
				verts[ i ] = _MakeGenVtx( poly[i], z0, surfaces[ poly.size() + 1 ], tgx, tgy );
			LC.AddPoly( verts, poly.size(), surfaces[ poly.size() + 1 ].texname, surfaces[ poly.size() + 1 ].lmquality, solid );
		}
	}
};



#define perp_prod(u,v) ((u).x * (v).y - (u).y * (v).x)
static int intersect_lines( const Vec2& l1a, const Vec2& l1b, const Vec2& l2a, const Vec2& l2b, Vec2* out )
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
	EDGUIVertexProps() :
		m_out( NULL ),
		m_vid( 0 ),
		m_pos( V3(0), 2, V3(-8192), V3(8192) )
	{
		tyname = "surfaceprops";
		m_group.caption = "Vertex properties";
		m_pos.caption = "Offset";
		m_insbef.caption = "Insert before";
		m_insaft.caption = "Insert after";
		
		m_group.Add( &m_pos );
		m_group.Add( &m_insbef );
		m_group.Add( &m_insaft );
		m_group.SetOpen( true );
		Add( &m_group );
	}
	
	void Prepare( EdBlock& B, int vid )
	{
		m_out = &B;
		m_vid = vid;
		
		char bfr[ 32 ];
		snprintf( bfr, sizeof(bfr), "Vertex #%d", vid );
		m_group.caption = bfr;
		m_group.SetOpen( true );
		
		m_pos.SetValue( B.poly[ vid ] );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( m_out && e->target == &m_pos )
			{
				m_out->poly[ m_vid ] = m_pos.m_value;
				m_out->RegenerateMesh();
			}
			break;
		case EDGUI_EVENT_BTNCLICK:
			if( ( e->target == &m_insbef || e->target == &m_insaft ) && m_out->poly.size() < 14 )
			{
				size_t insat = m_vid, sz = m_out->poly.size();
				if( e->target == &m_insaft )
					insat = ( insat + 1 ) % sz;
				size_t befat = ( insat + sz - 1 ) % sz;
				
				Vec2 p0 = m_out->poly[ ( befat + sz - 1 ) % sz ].ToVec2();
				Vec2 p1 = m_out->poly[ befat ].ToVec2();
				Vec2 p2 = m_out->poly[ insat ].ToVec2();
				Vec2 p3 = m_out->poly[ ( insat + 1 ) % sz ].ToVec2();
				Vec2 edge_normal = ( p2 - p1 ).Perp().Normalized();
				Vec2 mid = ( p1 + p2 ) / 2;
				Vec2 mid2 = mid;
				float factor = 0.5f;
				if( intersect_lines( p0, p1, p3, p2, &mid2 ) )
				{
					if( Vec2Dot( mid2, edge_normal ) < Vec2Dot( mid, edge_normal ) )
						factor = -0.1f;
					mid = TLERP( mid, mid2, factor );
				}
				else
					mid += edge_normal;
				Vec3 mid_fin = { mid.x, mid.y, ( m_out->poly[ befat ].z + m_out->poly[ insat ].z ) * 0.5f };
				
				EdSurface Scopy = m_out->surfaces[ befat ];
				m_out->poly.insert( insat, mid_fin );
				if( (int) insat < m_vid )
					m_vid++;
				m_out->surfaces.insert( insat, Scopy );
				
				m_out->RegenerateMesh();
				return 1;
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EdBlock* m_out;
	int m_vid;
	EDGUIGroup m_group;
	EDGUIPropVec3 m_pos;
	EDGUIButton m_insbef;
	EDGUIButton m_insaft;
};


struct EDGUISurfaceProps : EDGUILayoutRow
{
	EDGUISurfaceProps() :
		m_out( NULL ),
		m_sid( 0 ),
		m_tex( g_UISurfTexPicker, "metal0" ),
		m_off( V2(0), 2, V2(0), V2(1) ),
		m_scaleasp( V2(1), 2, V2(0.01f), V2(100) ),
		m_angle( 0, 1, 0, 360 ),
		m_lmquality( 1, 2, 0.01f, 100.0f )
	{
		tyname = "surfaceprops";
		m_group.caption = "Surface properties";
		m_tex.caption = "Texture";
		m_off.caption = "Offset";
		m_scaleasp.caption = "Scale/Aspect";
		m_angle.caption = "Angle";
		m_lmquality.caption = "Lightmap quality";
		
		m_group.Add( &m_tex );
		m_group.Add( &m_off );
		m_group.Add( &m_scaleasp );
		m_group.Add( &m_angle );
		m_group.Add( &m_lmquality );
		m_group.SetOpen( true );
		Add( &m_group );
	}
	
	void Prepare( EdBlock& B, int sid )
	{
		m_out = &B;
		m_sid = sid;
		EdSurface& S = B.surfaces[ sid ];
		
		char bfr[ 32 ];
		snprintf( bfr, sizeof(bfr), "Surface #%d", sid );
		LoadParams( S, bfr );
	}
	
	void LoadParams( EdSurface& S, const char* name = "Surface" )
	{
		m_group.caption = name;
		m_group.SetOpen( true );
		
		m_tex.SetValue( S.texname );
		m_off.SetValue( V2( S.xoff, S.yoff ) );
		m_scaleasp.SetValue( V2( S.scale, S.aspect ) );
		m_angle.SetValue( S.angle );
		m_lmquality.SetValue( S.lmquality );
	}
	
	void BounceBack( EdSurface& S )
	{
		S.texname = m_tex.m_value;
		S.xoff = m_off.m_value.x;
		S.yoff = m_off.m_value.y;
		S.scale = m_scaleasp.m_value.x;
		S.aspect = m_scaleasp.m_value.y;
		S.angle = m_angle.m_value;
		S.lmquality = m_lmquality.m_value;
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( m_out && ( e->target == &m_tex || e->target == &m_off || e->target == &m_scaleasp || e->target == &m_angle || e->target == &m_lmquality ) )
			{
				if( e->target == &m_tex )
				{
					m_out->surfaces[ m_sid ].texname = m_tex.m_value;
				}
				else if( e->target == &m_off )
				{
					m_out->surfaces[ m_sid ].xoff = m_off.m_value.x;
					m_out->surfaces[ m_sid ].yoff = m_off.m_value.y;
				}
				else if( e->target == &m_scaleasp )
				{
					m_out->surfaces[ m_sid ].scale = m_scaleasp.m_value.x;
					m_out->surfaces[ m_sid ].aspect = m_scaleasp.m_value.y;
				}
				else if( e->target == &m_angle )
				{
					m_out->surfaces[ m_sid ].angle = m_angle.m_value;
				}
				else if( e->target == &m_lmquality )
				{
					m_out->surfaces[ m_sid ].lmquality = m_lmquality.m_value;
				}
				m_out->RegenerateMesh();
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EdBlock* m_out;
	int m_sid;
	EDGUIGroup m_group;
	EDGUIPropRsrc m_tex;
	EDGUIPropVec2 m_off;
	EDGUIPropVec2 m_scaleasp;
	EDGUIPropFloat m_angle;
	EDGUIPropFloat m_lmquality;
};

struct EDGUIBlockProps : EDGUILayoutRow
{
	EDGUIBlockProps() :
		m_out( NULL ),
		m_group( true, "Block properties" ),
		m_vertGroup( false, "Vertices" ),
		m_z0( 0, 2, -8192, 8192 ),
		m_z1( 2, 2, -8192, 8192 ),
		m_pos( V2(0), 2, V2(-8192), V2(8192) )
	{
		tyname = "blockprops";
		m_z0.caption = "Bottom height";
		m_z1.caption = "Top height";
		m_pos.caption = "Position";
	}
	
	void Prepare( EdBlock& B )
	{
		m_out = &B;
		
		Clear();
		
		Add( &m_group );
		m_z0.SetValue( B.z0 );
		m_group.Add( &m_z0 );
		m_z1.SetValue( B.z1 );
		m_group.Add( &m_z1 );
		m_pos.SetValue( B.position );
		m_group.Add( &m_pos );
		m_group.Add( &m_vertGroup );
		
		m_vertProps.clear();
		m_vertProps.resize( B.poly.size() );
		for( size_t i = 0; i < B.poly.size(); ++i )
		{
			char bfr[ 4 ];
			snprintf( bfr, sizeof(bfr), "#%d", (int) i );
			m_vertProps[ i ] = EDGUIPropVec3( B.poly[ i ], 2, V3(-8192), V3(8192) );
			m_vertProps[ i ].caption = bfr;
			m_vertProps[ i ].id1 = i;
			m_vertGroup.Add( &m_vertProps[ i ] );
		}
		
		m_surfProps.clear();
		m_surfProps.resize( B.surfaces.size() );
		for( size_t i = 0; i < B.surfaces.size(); ++i )
		{
			m_surfProps[ i ].Prepare( B, i );
			m_group.Add( &m_surfProps[ i ] );
		}
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_z0 || e->target == &m_z1 || e->target == &m_pos )
			{
				if( e->target == &m_z0 )
				{
					m_out->z0 = m_z0.m_value;
				}
				else if( e->target == &m_z1 )
				{
					m_out->z1 = m_z1.m_value;
				}
				else if( e->target == &m_pos )
				{
					m_out->position = m_pos.m_value;
				}
				m_out->RegenerateMesh();
			}
			{
				size_t at = m_vertGroup.m_subitems.find_first_at( e->target );
				if( at != NOT_FOUND )
				{
					m_out->poly[ at ] = m_vertProps[ at ].m_value;
					m_out->RegenerateMesh();
				}
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EdBlock* m_out;
	EDGUIGroup m_group;
	EDGUIGroup m_vertGroup;
	EDGUIPropFloat m_z0;
	EDGUIPropFloat m_z1;
	EDGUIPropVec2 m_pos;
	Array< EDGUIPropVec3 > m_vertProps;
	Array< EDGUISurfaceProps > m_surfProps;
};


//
// E
// N
// T
// I
// T
// I
// E
// S
//

typedef SerializeVersionHelper<TextReader> SVHTR;
typedef SerializeVersionHelper<TextWriter> SVHTW;

struct EdEntity : EDGUILayoutRow
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	int32_t m_refcount;
	
	EdEntity( bool isproto ) :
		m_refcount( 0 ),
		m_isproto( isproto ),
		m_group( true, "Entity properties" ),
		m_ctlPos( V3(0), 2, V3(-8192), V3(8192) )
	{
		tyname = "_entity_overrideme_";
		
		m_ctlPos.caption = "Position";
	}
	
	void LoadIcon()
	{
		char bfr[ 256 ];
		snprintf( bfr, 256, "editor/icons/%s.png", tyname );
		m_iconTex = GR_GetTexture( bfr );
		if( !m_iconTex )
			m_iconTex = GR_GetTexture( "editor/icons/default.png" );
	}
	
	const Vec3& Pos() const { return m_ctlPos.m_value; }
	void SetPosition( const Vec3& pos ){ m_ctlPos.SetValue( pos ); }
	
	virtual int OnEvent( EDGUIEvent* e ){ return EDGUILayoutRow::OnEvent( e ); }
	
	virtual void Serialize( SVHTR& arch ) = 0;
	virtual void Serialize( SVHTW& arch ) = 0;
	virtual void UpdateCache( LevelCache& LC ){}
	
	virtual EdEntity* Clone() = 0;
	virtual bool RayIntersect( const Vec3& rpos, const Vec3& rdir, float outdst[1] )
	{
		return RaySphereIntersect( rpos, rdir, Pos(), 0.2f, outdst );
	}
	virtual void RegenerateMesh(){}
	virtual void DebugDraw(){}
	
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
	EdEntMesh( bool isproto = true ) :
		EdEntity( isproto ),
		m_ctlAngles( V3(0), 2, V3(0), V3(360) ),
		m_ctlScaleUni( 1, 2, 0.01f, 100.0f ),
		m_ctlScaleSep( V3(1), 2, V3(0.01f), V3(100.0f) ),
		m_ctlMesh( g_UIMeshPicker, "test_table" )
	{
		tyname = "mesh";
		LoadIcon();
		
		m_group.caption = "MESH properties";
		m_ctlAngles.caption = "Rotation";
		m_ctlScaleUni.caption = "Scale (uniform)";
		m_ctlScaleSep.caption = "Scale (separate)";
		m_ctlMesh.caption = "Mesh";
		
		m_group.Add( &m_ctlPos );
		m_group.Add( &m_ctlAngles );
		m_group.Add( &m_ctlScaleUni );
		m_group.Add( &m_ctlScaleSep );
		m_group.Add( &m_ctlMesh );
		Add( &m_group );
	}
	
	const String& Mesh() const { return m_ctlMesh.m_value; }
	const Vec3& RotAngles() const { return m_ctlAngles.m_value; }
	float ScaleUni() const { return m_ctlScaleUni.m_value; }
	const Vec3& ScaleSep() const { return m_ctlScaleSep.m_value; }
	Mat4 Matrix() const { return Mat4::CreateSRT( m_ctlScaleSep.m_value * m_ctlScaleUni.m_value, DEG2RAD( m_ctlAngles.m_value ), m_ctlPos.m_value ); }
	
	EdEntMesh& operator = ( const EdEntMesh& o )
	{
		m_ctlPos.SetValue( o.Pos() );
		m_ctlAngles.SetValue( o.RotAngles() );
		m_ctlScaleUni.SetValue( o.ScaleUni() );
		m_ctlScaleSep.SetValue( o.ScaleSep() );
		m_ctlMesh.SetValue( o.Mesh() );
		return *this;
	}
	
	virtual EdEntity* Clone()
	{
		EdEntMesh* N = new EdEntMesh( false );
		*N = *this;
		return N;
	}
	
	template< class T > void SerializeT( T& arch )
	{
		arch << m_ctlAngles;
		arch << m_ctlScaleUni;
		arch << m_ctlScaleSep;
		arch << m_ctlMesh;
	}
	virtual void Serialize( SVHTR& arch ){ SerializeT( arch ); }
	virtual void Serialize( SVHTW& arch ){ SerializeT( arch ); }
	
	virtual void UpdateCache( LevelCache& LC )
	{
		char bfr[ 256 ];
		snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", TMIN( (int) Mesh().size(), 200 ), Mesh().data() );
		LC.AddMeshInst( bfr, Matrix() );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_ctlMesh )
			{
				cached_mesh = NULL;
			}
			if( !m_isproto )
				RegenerateMesh();
			break;
		}
		return EdEntity::OnEvent( e );
	}
	
	virtual void DebugDraw()
	{
		if( cached_mesh )
		{
			BatchRenderer& br = GR2D_GetBatchRenderer();
			br.Reset();
			br.Col( 0.1f, 0.3f, 0.8f, 0.5f );
			br.AABB( cached_mesh->m_boundsMin, cached_mesh->m_boundsMax, Matrix() );
		}
	}
	
	virtual void RegenerateMesh()
	{
		if( !cached_mesh )
		{
			char bfr[ 128 ];
			snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", (int) Mesh().size(), Mesh().data() );
			cached_mesh = GR_GetMesh( bfr );
		}
		if( !cached_meshinst )
		{
			cached_meshinst = g_EdScene->CreateMeshInstance();
			prepmeshinst( cached_meshinst );
		}
		cached_meshinst->mesh = cached_mesh;
		cached_meshinst->matrix = Matrix();
	}
	
	EDGUIPropVec3 m_ctlAngles;
	EDGUIPropFloat m_ctlScaleUni;
	EDGUIPropVec3 m_ctlScaleSep;
	EDGUIPropRsrc m_ctlMesh;
	
	MeshHandle cached_mesh;
	MeshInstHandle cached_meshinst;
};

struct EdEntLight : EdEntity
{
	EdEntLight( bool isproto = true ) :
		EdEntity( isproto ),
		m_ctlRange( 10, 2, 0, 1000.0f ),
		m_ctlPower( 2, 2, 0.01f, 100.0f ),
		m_ctlColorHSV( V3(0,0,1), 2, V3(0,0,0), V3(1,1,100) ),
		m_ctlLightRadius( 1, 2, 0, 100.0f ),
		m_ctlShSampleCnt( 5, 0, 256 ),
		m_ctlFlareSize( 1, 2, 0, 10 ),
		m_ctlFlareOffset( V3(0), 2, V3(-4), V3(4) ),
		m_ctlIsSpotlight( false ),
		m_ctlSpotRotation( V3(0), 2, V3(0), V3(360) ),
		m_ctlSpotInnerAngle( 0, 2, 0, 180 ),
		m_ctlSpotOuterAngle( 45, 2, 0, 180 ),
		m_ctlSpotCurve( 1, 2, 0.01f, 100.0f )
	{
		tyname = "light";
		LoadIcon();
		
		m_group.caption = "LIGHT properties";
		m_ctlRange.caption = "Range";
		m_ctlPower.caption = "Power";
		m_ctlColorHSV.caption = "Color (HSV)";
		m_ctlLightRadius.caption = "Light radius";
		m_ctlShSampleCnt.caption = "# shadow samples";
		m_ctlFlareSize.caption = "Flare size";
		m_ctlFlareOffset.caption = "Flare offset";
		m_ctlIsSpotlight.caption = "Is spotlight?";
		m_ctlSpotRotation.caption = "Spotlight rotation";
		m_ctlSpotInnerAngle.caption = "Spotlight inner angle";
		m_ctlSpotOuterAngle.caption = "Spotlight outer angle";
		m_ctlSpotCurve.caption = "Spotlight curve";
		
		m_group.Add( &m_ctlPos );
		m_group.Add( &m_ctlRange );
		m_group.Add( &m_ctlPower );
		m_group.Add( &m_ctlColorHSV );
		m_group.Add( &m_ctlLightRadius );
		m_group.Add( &m_ctlShSampleCnt );
		m_group.Add( &m_ctlFlareSize );
		m_group.Add( &m_ctlFlareOffset );
		m_group.Add( &m_ctlIsSpotlight );
		m_group.Add( &m_ctlSpotRotation );
		m_group.Add( &m_ctlSpotInnerAngle );
		m_group.Add( &m_ctlSpotOuterAngle );
		m_group.Add( &m_ctlSpotCurve );
		Add( &m_group );
	}
	
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
	
	EdEntLight& operator = ( const EdEntLight& o )
	{
		m_ctlPos.SetValue( o.Pos() );
		m_ctlRange.SetValue( o.Range() );
		m_ctlPower.SetValue( o.Power() );
		m_ctlColorHSV.SetValue( o.ColorHSV() );
		m_ctlLightRadius.SetValue( o.LightRadius() );
		m_ctlShSampleCnt.SetValue( o.ShadowSampleCount() );
		m_ctlFlareSize.SetValue( o.FlareSize() );
		m_ctlFlareOffset.SetValue( o.FlareOffset() );
		m_ctlIsSpotlight.SetValue( o.IsSpotlight() );
		m_ctlSpotRotation.SetValue( o.SpotRotation() );
		m_ctlSpotInnerAngle.SetValue( o.SpotInnerAngle() );
		m_ctlSpotOuterAngle.SetValue( o.SpotOuterAngle() );
		m_ctlSpotCurve.SetValue( o.SpotCurve() );
		return *this;
	}
	
	virtual EdEntity* Clone()
	{
		EdEntLight* N = new EdEntLight( false );
		*N = *this;
		return N;
	}
	
	template< class T > void SerializeT( T& arch )
	{
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
	
	virtual void DebugDraw()
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		if( IsSpotlight() )
		{
			br.Col( 0.9f, 0.8f, 0.1f, 0.5f );
			br.ConeOutline( Pos(), SpotDir(), SpotUp(), Range(), SpotOuterAngle(), 32 );
			br.Col( 0.9f, 0.5f, 0.1f, 0.5f );
			br.ConeOutline( Pos(), SpotDir(), SpotUp(), pow( 0.5f, Power() ) * Range(), SpotOuterAngle(), 32 );
		}
		else
		{
			br.Col( 0.9f, 0.8f, 0.1f, 0.5f );
			br.SphereOutline( Pos(), Range(), 64 );
			br.Col( 0.9f, 0.5f, 0.1f, 0.5f );
			br.SphereOutline( Pos(), pow( 0.5f, Power() ) * Range(), 32 );
		}
		br.Col( 0.9f, 0.1f, 0.1f, 0.5f );
		br.Tick( Pos() + FlareOffset(), 0.1f );
	}
	
	virtual void UpdateCache( LevelCache& LC )
	{
		LC_Light L;
		L.type = IsSpotlight() ? LM_LIGHT_SPOT : LM_LIGHT_POINT;
		L.pos = Pos();
		L.dir = SpotDir();
		L.up = SpotUp();
		L.range = Range();
		L.power = Power();
		L.color = HSV( ColorHSV() );
		L.light_radius = LightRadius();
		L.num_shadow_samples = ShadowSampleCount();
		L.flaresize = FlareSize();
		L.flareoffset = FlareOffset();
		L.innerangle = SpotInnerAngle();
		L.outerangle = SpotOuterAngle();
		L.spotcurve = SpotCurve();
		LC.AddLight( L );
	}
	
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
	EdEntLightSample( bool isproto = true ) :
		EdEntity( isproto )
	{
		tyname = "ltsample";
		LoadIcon();
		
		m_group.caption = "LIGHT SAMPLE properties";
		m_group.Add( &m_ctlPos );
		Add( &m_group );
	}
	
	EdEntLightSample& operator = ( const EdEntLightSample& o )
	{
		m_ctlPos.SetValue( o.Pos() );
		return *this;
	}
	
	virtual EdEntity* Clone()
	{
		EdEntLightSample* N = new EdEntLightSample( false );
		*N = *this;
		return N;
	}
	
	virtual void Serialize( SVHTR& arch ){}
	virtual void Serialize( SVHTW& arch ){}
	
	virtual void UpdateCache( LevelCache& LC )
	{
		LC.AddSample( Pos() );
	}
};

struct EdEntScripted : EdEntity
{
	struct Field
	{
		String key;
		EDGUIProperty* property;
	};
	
	EdEntScripted( const char* enttype, bool isproto = true ) :
		EdEntity( isproto ),
		m_levelCache( NULL )
	{
		strncpy( m_typename, enttype, 63 );
		m_typename[ 63 ] = 0;
		tyname = m_typename;
		LoadIcon();
		
		char bfr[ 256 ];
		
		sprintf( bfr, "%.240s properties", enttype );
		m_group.caption = bfr;
		m_group.SetOpen( true );
		m_group.Add( &m_ctlPos );
		Add( &m_group );
		
		Field posf = { "position", &m_ctlPos };
		m_fields.push_back( posf );
		
		g_ScriptCtx->Push( (void*) this );
		sprintf( bfr, "ED_ENT_%.240s", enttype );
		g_ScriptCtx->GlobalCall( bfr, 1, 0 );
		
		Fields2Data();
	}
	~EdEntScripted()
	{
		for( size_t i = 0; i < m_fields.size(); ++i )
		{
			if( m_fields[ i ].property == &m_ctlPos )
				continue;
			delete m_fields[ i ].property;
		}
	}
	
	EdEntScripted& operator = ( const EdEntScripted& o )
	{
		ASSERT( strcmp( tyname, o.tyname ) == 0 );
		for( size_t i = 0; i < m_fields.size(); ++i )
		{
			m_fields[ i ].property->TakeValue( o.m_fields[ i ].property );
		}
		Fields2Data();
		return *this;
	}
	
	virtual EdEntity* Clone()
	{
		EdEntScripted* N = new EdEntScripted( tyname, false );
		*N = *this;
		return N;
	}
	
	void Data2Fields()
	{
		for( size_t i = 0; i < m_fields.size(); ++i )
		{
			Field& F = m_fields[ i ];
			sgsVariable val = m_data.getprop( StackString<256>( F.key ) );
			switch( F.property->type )
			{
			case EDGUI_ITEM_PROP_BOOL: ((EDGUIPropBool*) F.property)->SetValue( val.get<bool>() ); break;
			case EDGUI_ITEM_PROP_INT: ((EDGUIPropInt*) F.property)->SetValue( val.get<int>() ); break;
			case EDGUI_ITEM_PROP_FLOAT: ((EDGUIPropFloat*) F.property)->SetValue( val.get<float>() ); break;
			case EDGUI_ITEM_PROP_VEC2: ((EDGUIPropVec2*) F.property)->SetValue( val.get<Vec2>() ); break;
			case EDGUI_ITEM_PROP_VEC3: ((EDGUIPropVec3*) F.property)->SetValue( val.get<Vec3>() ); break;
			case EDGUI_ITEM_PROP_STRING: ((EDGUIPropString*) F.property)->SetValue( val.get<String>() ); break;
			case EDGUI_ITEM_PROP_RSRC: ((EDGUIPropRsrc*) F.property)->SetValue( val.get<String>() ); break;
			}
		}
	}
	void Fields2Data()
	{
		sgsVariable data = g_ScriptCtx->CreateDict();
		for( size_t i = 0; i < m_fields.size(); ++i )
		{
			Field& F = m_fields[ i ];
			switch( F.property->type )
			{
			case EDGUI_ITEM_PROP_BOOL: data.setprop( StackString<256>( F.key ), sgsVariable().set(((EDGUIPropBool*) F.property)->m_value) ); break;
			case EDGUI_ITEM_PROP_INT: data.setprop( StackString<256>( F.key ), sgsVariable().set( (sgs_Int) ((EDGUIPropInt*) F.property)->m_value) ); break;
			case EDGUI_ITEM_PROP_FLOAT: data.setprop( StackString<256>( F.key ), sgsVariable().set(((EDGUIPropFloat*) F.property)->m_value) ); break;
			case EDGUI_ITEM_PROP_VEC2: data.setprop( StackString<256>( F.key ), g_ScriptCtx->CreateVec2( ((EDGUIPropVec2*) F.property)->m_value ) ); break;
			case EDGUI_ITEM_PROP_VEC3: data.setprop( StackString<256>( F.key ), g_ScriptCtx->CreateVec3( ((EDGUIPropVec3*) F.property)->m_value ) ); break;
			case EDGUI_ITEM_PROP_STRING: data.setprop( StackString<256>( F.key ), g_ScriptCtx->CreateStringVar( ((EDGUIPropString*) F.property)->m_value ) ); break;
			case EDGUI_ITEM_PROP_RSRC: data.setprop( StackString<256>( F.key ), g_ScriptCtx->CreateStringVar( ((EDGUIPropRsrc*) F.property)->m_value ) ); break;
			}
		}
		m_data = data;
	}
	
	virtual void Serialize( SVHTR& arch )
	{
		String data;
		arch << data;
		m_data = g_ScriptCtx->Unserialize( data );
		Data2Fields();
	}
	virtual void Serialize( SVHTW& arch )
	{
		String data;
		data = g_ScriptCtx->Serialize( m_data );
		arch << data;
		Fields2Data();
	}
	
	virtual void UpdateCache( LevelCache& LC )
	{
		String data;
		data = g_ScriptCtx->Serialize( m_data );
		LC.AddScriptedEntity( m_typename, data );
		
		if( onGather.not_null() )
		{
			m_levelCache = &LC;
			SGS_CSCOPE( g_ScriptCtx->C );
			g_ScriptCtx->Push( (void*) this );
			g_ScriptCtx->Push( m_data );
			g_ScriptCtx->Call( onGather, 2 );
			m_levelCache = NULL;
		}
	}
	
	virtual void RegenerateMesh()
	{
		Fields2Data();
		
		if( onChange.not_null() )
		{
			SGS_CSCOPE( g_ScriptCtx->C );
			g_ScriptCtx->Push( (void*) this );
			g_ScriptCtx->Push( m_data );
			g_ScriptCtx->Call( onChange, 2 );
		}
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( !m_isproto )
				RegenerateMesh();
			break;
		}
		return EdEntity::OnEvent( e );
	}
	virtual void DebugDraw()
	{
		if( !onDebugDraw.not_null() )
			return;
		
		SGS_CSCOPE( g_ScriptCtx->C );
		g_ScriptCtx->Push( (void*) this );
		g_ScriptCtx->Push( m_data );
		g_ScriptCtx->Call( onDebugDraw, 2 );
	}
	
	void AddFieldBool( sgsString key, sgsString name, bool def )
	{
		EDGUIPropBool* prop = new EDGUIPropBool( def );
		prop->caption = StringView( name.c_str(), name.size() );
		Field F = { StringView( key.c_str(), key.size() ), prop };
		m_fields.push_back( F );
		m_group.Add( prop );
	}
	void AddFieldInt( sgsString key, sgsString name, int32_t def = 0, int32_t min = (int32_t) 0x80000000, int32_t max = (int32_t) 0x7fffffff )
	{
		EDGUIPropInt* prop = new EDGUIPropInt( def, min, max );
		prop->caption = StringView( name.c_str(), name.size() );
		Field F = { StringView( key.c_str(), key.size() ), prop };
		m_fields.push_back( F );
		m_group.Add( prop );
	}
	void AddFieldFloat( sgsString key, sgsString name, float def = 0, int prec = 2, float min = -FLT_MAX, float max = FLT_MAX )
	{
		EDGUIPropFloat* prop = new EDGUIPropFloat( def, prec, min, max );
		prop->caption = StringView( name.c_str(), name.size() );
		Field F = { StringView( key.c_str(), key.size() ), prop };
		m_fields.push_back( F );
		m_group.Add( prop );
	}
	void AddFieldVec2( sgsString key, sgsString name, Vec2 def = V2(0), int prec = 2, Vec2 min = V2(-FLT_MAX), Vec2 max = V2(FLT_MAX) )
	{
		EDGUIPropVec2* prop = new EDGUIPropVec2( def, prec, min, max );
		prop->caption = StringView( name.c_str(), name.size() );
		Field F = { StringView( key.c_str(), key.size() ), prop };
		m_fields.push_back( F );
		m_group.Add( prop );
	}
	void AddFieldVec3( sgsString key, sgsString name, Vec3 def = V3(0), int prec = 2, Vec3 min = V3(-FLT_MAX), Vec3 max = V3(FLT_MAX) )
	{
		EDGUIPropVec3* prop = new EDGUIPropVec3( def, prec, min, max );
		prop->caption = StringView( name.c_str(), name.size() );
		Field F = { StringView( key.c_str(), key.size() ), prop };
		m_fields.push_back( F );
		m_group.Add( prop );
	}
	void AddFieldString( sgsString key, sgsString name, sgsString def )
	{
		EDGUIPropString* prop = new EDGUIPropString( StringView( def.c_str(), def.size() ) );
		prop->caption = StringView( name.c_str(), name.size() );
		Field F = { StringView( key.c_str(), key.size() ), prop };
		m_fields.push_back( F );
		m_group.Add( prop );
	}
	void AddFieldRsrc( sgsString key, sgsString name, EDGUIRsrcPicker* rsrcPicker, sgsString def )
	{
		EDGUIPropRsrc* prop = new EDGUIPropRsrc( rsrcPicker, StringView( def.c_str(), def.size() ) );
		prop->caption = StringView( name.c_str(), name.size() );
		Field F = { StringView( key.c_str(), key.size() ), prop };
		m_fields.push_back( F );
		m_group.Add( prop );
	}
	void SetMesh( sgsString name )
	{
		cached_mesh = GR_GetMesh( StringView( name.c_str(), name.size() ) );
		for( size_t i = 0; i < cached_meshinsts.size(); ++i )
			cached_meshinsts[ i ]->mesh = cached_mesh;
	}
	void SetMeshInstanceCount( int count )
	{
		int i = cached_meshinsts.size();
		cached_meshinsts.resize( count );
		while( count > i )
		{
			MeshInstHandle cmi = g_EdScene->CreateMeshInstance();
			prepmeshinst( cmi );
			cmi->mesh = cached_mesh;
			cmi->matrix = Mat4::Identity;
			cached_meshinsts[ i++ ] = cmi;
		}
	}
	void SetMeshInstanceMatrix( int which, const Mat4& mtx )
	{
		if( which < 0 || which >= (int) cached_meshinsts.size() )
			return;
		cached_meshinsts[ which ]->matrix = mtx;
	}
	void GetMeshAABB( Vec3 out[2] )
	{
		if( cached_mesh )
		{
			out[0] = cached_mesh->m_boundsMin;
			out[1] = cached_mesh->m_boundsMax;
		}
		else
		{
			out[0] = V3(-1);
			out[1] = V3(1);
		}
	}
	
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

static int EE_AddFieldBool( SGS_CTX )
{
	SGSFN( "EE_AddFieldBool" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	bool def = sgs_StackSize( C ) > 3 ? sgs_GetVar<bool>()( C, 3 ) : false;
	E->AddFieldBool( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def );
	return 0;
}
static int EE_AddFieldInt( SGS_CTX )
{
	SGSFN( "EE_AddFieldInt" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	int32_t def = sgs_StackSize( C ) > 3 ? sgs_GetVar<int32_t>()( C, 3 ) : 0;
	int32_t min = sgs_StackSize( C ) > 4 ? sgs_GetVar<int32_t>()( C, 4 ) : 0x80000000;
	int32_t max = sgs_StackSize( C ) > 5 ? sgs_GetVar<int32_t>()( C, 5 ) : 0x7fffffff;
	E->AddFieldInt( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def, min, max );
	return 0;
}
static int EE_AddFieldFloat( SGS_CTX )
{
	SGSFN( "EE_AddFieldFloat" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	float def = sgs_StackSize( C ) > 3 ? sgs_GetVar<float>()( C, 3 ) : 0;
	int prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	float min = sgs_StackSize( C ) > 5 ? sgs_GetVar<float>()( C, 5 ) : -FLT_MAX;
	float max = sgs_StackSize( C ) > 6 ? sgs_GetVar<float>()( C, 6 ) : FLT_MAX;
	E->AddFieldFloat( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def, prec, min, max );
	return 0;
}
static int EE_AddFieldVec2( SGS_CTX )
{
	SGSFN( "EE_AddFieldVec2" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	Vec2 def = sgs_StackSize( C ) > 3 ? sgs_GetVar<Vec2>()( C, 3 ) : V2(0);
	int prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	Vec2 min = sgs_StackSize( C ) > 5 ? sgs_GetVar<Vec2>()( C, 5 ) : V2(-FLT_MAX);
	Vec2 max = sgs_StackSize( C ) > 6 ? sgs_GetVar<Vec2>()( C, 6 ) : V2(FLT_MAX);
	E->AddFieldVec2( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def, prec, min, max );
	return 0;
}
static int EE_AddFieldVec3( SGS_CTX )
{
	SGSFN( "EE_AddFieldVec3" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	Vec3 def = sgs_StackSize( C ) > 3 ? sgs_GetVar<Vec3>()( C, 3 ) : V3(0);
	int prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	Vec3 min = sgs_StackSize( C ) > 5 ? sgs_GetVar<Vec3>()( C, 5 ) : V3(-FLT_MAX);
	Vec3 max = sgs_StackSize( C ) > 6 ? sgs_GetVar<Vec3>()( C, 6 ) : V3(FLT_MAX);
	E->AddFieldVec3( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def, prec, min, max );
	return 0;
}
static int EE_AddFieldString( SGS_CTX )
{
	SGSFN( "EE_AddFieldString" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldString( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldMesh( SGS_CTX )
{
	SGSFN( "EE_AddFieldMesh" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UIMeshPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldTex( SGS_CTX )
{
	SGSFN( "EE_AddFieldTex" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UISurfTexPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldPartSys( SGS_CTX )
{
	SGSFN( "EE_AddFieldPartSys" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UIPartSysPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldSound( SGS_CTX )
{
	SGSFN( "EE_AddFieldSound" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UISoundPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldScrFn( SGS_CTX )
{
	SGSFN( "EE_AddFieldScrFn" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UIScrFnPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}

static int EE_SetMesh( SGS_CTX )
{
	SGSFN( "EE_SetMesh" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->SetMesh( sgs_GetVar<sgsString>()( C, 1 ) );
	return 0;
}
static int EE_SetMeshInstanceCount( SGS_CTX )
{
	SGSFN( "EE_SetMeshInstanceCount" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->SetMeshInstanceCount( sgs_GetVar<int>()( C, 1 ) );
	return 0;
}
static int EE_SetMeshInstanceMatrix( SGS_CTX )
{
	SGSFN( "EE_SetMeshInstanceMatrix" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->SetMeshInstanceMatrix( sgs_GetVar<int>()( C, 1 ), sgs_GetVar<Mat4>()( C, 2 ) );
	return 0;
}
static int EE_GetMeshAABB( SGS_CTX )
{
	SGSFN( "EE_GetMeshAABB" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	Vec3 aabb[2];
	E->GetMeshAABB( aabb );
	sgs_PushVar( C, aabb[0] );
	sgs_PushVar( C, aabb[1] );
	return 2;
}

static int EE_SetChangeFunc( SGS_CTX )
{
	SGSFN( "EE_SetChangeFunc" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->onChange = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}
static int EE_SetDebugDrawFunc( SGS_CTX )
{
	SGSFN( "EE_SetDebugDrawFunc" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->onDebugDraw = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}
static int EE_SetGatherFunc( SGS_CTX )
{
	SGSFN( "EE_SetGatherFunc" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->onGather = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}

static int EE_Gather_Mesh( SGS_CTX )
{
	SGSFN( "EE_Gather_Mesh" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	float lmquality = sgs_StackSize( C ) > 3 ? sgs_GetVar<float>()( C, 3 ) : 1.0f;
	bool solid = sgs_StackSize( C ) > 4 ? sgs_GetVar<bool>()( C, 4 ) : true;
	bool dynlit = sgs_StackSize( C ) > 5 ? sgs_GetVar<bool>()( C, 5 ) : false;
	bool castlms = sgs_StackSize( C ) > 6 ? sgs_GetVar<bool>()( C, 6 ) : true;
	if( E->m_levelCache )
		E->m_levelCache->AddMeshInst( sgs_GetVar<String>()( C, 1 ), sgs_GetVar<Mat4>()( C, 2 ), lmquality, solid, dynlit, castlms );
	return 0;
}

static int EE_GenMatrix_SRaP( SGS_CTX )
{
	SGSFN( "EE_GenMatrix_SRaP" );
	Vec3 scale = sgs_GetVar<Vec3>()( C, 0 );
	Vec3 rot_angles = sgs_GetVar<Vec3>()( C, 1 );
	Vec3 position = sgs_GetVar<Vec3>()( C, 2 );
	Mat4 out = Mat4::CreateSRT( scale, DEG2RAD( rot_angles ), position );
	sgs_PushVar( C, out );
	return 1;
}

static sgs_RegFuncConst g_ent_scripted_rfc[] =
{
	{ "EE_AddFieldBool", EE_AddFieldBool },
	{ "EE_AddFieldInt", EE_AddFieldInt },
	{ "EE_AddFieldFloat", EE_AddFieldFloat },
	{ "EE_AddFieldVec2", EE_AddFieldVec2 },
	{ "EE_AddFieldVec3", EE_AddFieldVec3 },
	{ "EE_AddFieldString", EE_AddFieldString },
	{ "EE_AddFieldMesh", EE_AddFieldMesh },
	{ "EE_AddFieldTex", EE_AddFieldTex },
	{ "EE_AddFieldPartSys", EE_AddFieldPartSys },
	{ "EE_AddFieldSound", EE_AddFieldSound },
	{ "EE_AddFieldScrFn", EE_AddFieldScrFn },
	{ "EE_SetMesh", EE_SetMesh },
	{ "EE_SetMeshInstanceCount", EE_SetMeshInstanceCount },
	{ "EE_SetMeshInstanceMatrix", EE_SetMeshInstanceMatrix },
	{ "EE_GetMeshAABB", EE_GetMeshAABB },
	{ "EE_SetChangeFunc", EE_SetChangeFunc },
	{ "EE_SetDebugDrawFunc", EE_SetDebugDrawFunc },
	{ "EE_SetGatherFunc", EE_SetGatherFunc },
	{ "EE_Gather_Mesh", EE_Gather_Mesh },
	{ "EE_GenMatrix_SRaP", EE_GenMatrix_SRaP },
	SGS_RC_END(),
};



/////////////
////////////
///////////

#define EDGUI_EVENT_SETENTITY EDGUI_EVENT_USER + 1

struct EDGUIEntList* g_EdEntList;

struct EDGUIEntList : EDGUIGroup
{
	struct Decl
	{
		const char* name;
		EdEntity* ent;
	};
	
	EDGUIEntList() : EDGUIGroup( true, "Pick an entity type" )
	{
		Array< Decl > ents;
		Decl coreents[] =
		{
			{ "Mesh", new EdEntMesh },
			{ "Light", new EdEntLight },
			{ "Light sample", new EdEntLightSample },
		};
		ents.append( coreents, sizeof(coreents)/sizeof(coreents[0]) );
		
		ScriptVarIterator globals = g_ScriptCtx->GlobalIterator();
		while( globals.Advance() )
		{
			sgsString str = globals.GetKey().get_string();
			StringView key( str.c_str(), str.size() );
			if( key.part( 0, 7 ) == "ED_ENT_" )
			{
				key = key.part( 7 );
				Decl decl = { key.data(), new EdEntScripted( key.data() ) };
				ents.push_back( decl );
			}
		}
		
		m_button_count = ents.size();
		m_buttons = new EDGUIEntButton[ m_button_count ];
		
		for( int i = 0; i < m_button_count; ++i )
		{
			m_buttons[ i ].caption = ents[ i ].name;
			m_buttons[ i ].m_ent_handle = ents[ i ].ent;
			
			Add( &m_buttons[ i ] );
		}
		
		m_buttons[ 0 ].SetHighlight( true );
		
		g_EdEntList = this;
	}
	~EDGUIEntList()
	{
		g_EdEntList = NULL;
		delete [] m_buttons;
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( e->type == EDGUI_EVENT_BTNCLICK )
		{
			for( int i = 0; i < m_button_count; ++i )
			{
				bool found = e->target == &m_buttons[ i ];
				m_buttons[ i ].SetHighlight( found );
				if( found )
				{
					EDGUIEvent se = { EDGUI_EVENT_SETENTITY, &m_buttons[ i ] };
					BubblingEvent( &se );
				}
			}
		}
		return EDGUIGroup::OnEvent( e );
	}
	
	EDGUIEntButton* m_buttons;
	int m_button_count;
};

EdEntity* ENT_FindProtoByName( const char* name )
{
	for( int i = 0; i < g_EdEntList->m_button_count; ++i )
	{
		EdEntity* e = g_EdEntList->m_buttons[ i ].m_ent_handle;
		if( !strcmp( e->tyname, name ) )
			return e;
	}
	return NULL;
}

template< class T > void ENT_Serialize( T& arch, EdEntity* e )
{
	String ty = e->tyname;
	
	arch.marker( "ENTITY" );
	arch << ty;
	arch << e->m_ctlPos;
	arch << *e;
}

template< class T > EdEntity* ENT_Unserialize( T& arch )
{
	String ty;
	Vec3 p;
	
	arch.marker( "ENTITY" );
	arch << ty;
	arch << p;
	
	EdEntity* e = ENT_FindProtoByName( StackString< 128 >( ty ) );
	if( !e )
	{
		LOG_ERROR << "FAILED TO FIND ENTITY: " << ty;
		return NULL;
	}
	e = e->Clone();
	e->m_ctlPos.SetValue( p );
	e->Serialize( arch );
	e->RegenerateMesh();
	
	return e;
}



struct EdWorld : EDGUILayoutRow
{
	EdWorld() :
		m_ctlGroup( true, "Level properties" ),
		m_ctlAmbientColor( V3(0,0,0.1f), 2, V3(0), V3(1,1,100) ),
		m_ctlDirLightDir( V2(0,0), 2, V2(-8192), V2(8192) ),
		m_ctlDirLightColor( V3(0,0,0.0f), 2, V3(0), V3(1,1,100) ),
		m_ctlDirLightDivergence( 10, 2, 0, 180 ),
		m_ctlDirLightNumSamples( 15, 0, 256 ),
		m_ctlLightmapClearColor( V3(0,0,0), 2, V3(0), V3(1,1,100) ),
		m_ctlRADNumBounces( 2, 0, 256 ),
		m_ctlLightmapDetail( 1, 2, 0.01f, 10 ),
		m_ctlLightmapBlurSize( 1, 2, 0, 10 ),
		m_ctlAODistance( 2, 2, 0, 100 ),
		m_ctlAOMultiplier( 1, 2, 0, 2 ),
		m_ctlAOFalloff( 2, 2, 0.01f, 100.0f ),
		m_ctlAOEffect( 0, 2, -1, 1 ),
	//	m_ctlAODivergence( 0, 2, 0, 1 ),
		m_ctlAOColor( V3(0,0,0), 2, V3(0), V3(1,1,100) ),
		m_ctlAONumSamples( 15, 0, 256 )
	{
		tyname = "world";
		m_ctlAmbientColor.caption = "Ambient color";
		m_ctlDirLightDir.caption = "Dir.light direction (dX,dY)";
		m_ctlDirLightColor.caption = "Dir.light color (HSV)";
		m_ctlDirLightDivergence.caption = "Dir.light divergence";
		m_ctlDirLightNumSamples.caption = "Dir.light sample count";
		m_ctlLightmapClearColor.caption = "Lightmap clear color (HSV)";
		m_ctlRADNumBounces.caption = "Radiosity bounce count";
		m_ctlLightmapDetail.caption = "Lightmap detail";
		m_ctlLightmapBlurSize.caption = "Lightmap blur size";
		m_ctlAODistance.caption = "AO distance";
		m_ctlAOMultiplier.caption = "AO multiplier";
		m_ctlAOFalloff.caption = "AO falloff";
		m_ctlAOEffect.caption = "AO effect";
	//	m_ctlAODivergence.caption = "AO divergence";
		m_ctlAOColor.caption = "AO color";
		m_ctlAONumSamples.caption = "AO sample count";
		m_vd = GR_GetVertexDecl( EdVtx_DECL );
		
		Add( &m_ctlGroup );
		m_ctlGroup.Add( &m_ctlAmbientColor );
		m_ctlGroup.Add( &m_ctlDirLightDir );
		m_ctlGroup.Add( &m_ctlDirLightColor );
		m_ctlGroup.Add( &m_ctlDirLightDivergence );
		m_ctlGroup.Add( &m_ctlDirLightNumSamples );
		m_ctlGroup.Add( &m_ctlLightmapClearColor );
		m_ctlGroup.Add( &m_ctlRADNumBounces );
		m_ctlGroup.Add( &m_ctlLightmapDetail );
		m_ctlGroup.Add( &m_ctlLightmapBlurSize );
		m_ctlGroup.Add( &m_ctlAODistance );
		m_ctlGroup.Add( &m_ctlAOMultiplier );
		m_ctlGroup.Add( &m_ctlAOFalloff );
		m_ctlGroup.Add( &m_ctlAOEffect );
	//	m_ctlGroup.Add( &m_ctlAODivergence );
		m_ctlGroup.Add( &m_ctlAOColor );
		m_ctlGroup.Add( &m_ctlAONumSamples );
		
		TestData();
	}
	
	template< class T > void Serialize( T& arch )
	{
		arch.marker( "WORLD" );
		SerializeVersionHelper<T> svh( arch, 2 );
		
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
		
		svh << m_blocks;
		
		if( T::IsWriter )
		{
			int32_t numents = m_entities.size();
			svh << numents;
			for( size_t i = 0; i < m_entities.size(); ++i )
			{
				ENT_Serialize( svh, m_entities[ i ] );
			}
		}
		else
		{
			int32_t numents;
			svh << numents;
			m_entities.clear();
			for( int32_t i = 0; i < numents; ++i )
			{
				EdEntity* e = ENT_Unserialize( svh );
				if( e )
					m_entities.push_back( e );
			}
		}
	}
	
	void Reset()
	{
		m_blocks.clear();
		m_entities.clear();
	}
	
	void TestData()
	{
		EdBlock b1;
		
		b1.position = V2(0);
		b1.z0 = 0;
		b1.z1 = 2;
		
		Vec3 poly[] = { {-2,-2,0}, {2,-2,0}, {2,2,0}, {-2,2,0} };
		b1.poly.assign( poly, 4 );
		
		EdSurface surf;
		surf.texname = "metal0";
		for( int i = 0; i < 6; ++i )
			b1.surfaces.push_back( surf );
		
		m_blocks.push_back( b1 );
		b1.z1 = 1;
		b1.position = V2( 0.1f, 1 );
		m_blocks.push_back( b1 );
		
		RegenerateMeshes();
	}
	void RegenerateMeshes()
	{
		for( size_t i = 0; i < m_blocks.size(); ++i )
			m_blocks[ i ].RegenerateMesh();
	}
	void DrawWires_Blocks( int hlblock, int selblock )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		
		br.SetPrimitiveType( PT_Lines ).UnsetTexture();
		for( size_t i = 0; i < m_blocks.size(); ++i )
		{
			if( (int) i == selblock )
				br.Col( 0.9f, 0.5, 0.1f, 1 );
			else if( (int) i == hlblock )
				br.Col( 0.1f, 0.5, 0.9f, 0.9f );
			else
				br.Col( 0.1f, 0.5, 0.9f, 0.5f );
			
			const EdBlock& B = m_blocks[ i ];
			for( size_t v = 0; v < B.poly.size(); ++v )
			{
				size_t v1 = ( v + 1 ) % B.poly.size();
				
				br.Pos( B.position.x + B.poly[ v ].x,  B.position.y + B.poly[ v ].y, B.z0 );
				br.Pos( B.position.x + B.poly[ v1 ].x, B.position.y + B.poly[ v1 ].y, B.z0 );
				
				br.Pos( B.position.x + B.poly[ v ].x,  B.position.y + B.poly[ v ].y, B.z1 );
				br.Pos( B.position.x + B.poly[ v1 ].x, B.position.y + B.poly[ v1 ].y, B.z1 );
				
				br.Prev( 3 ).Prev( 2 );
			}
		}
		
		br.Flush();
	}
	void DrawPoly_BlockSurf( int block, int surf, bool sel )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		
		br.Flush();
		br.SetPrimitiveType( PT_TriangleStrip ).UnsetTexture();
		
		if( sel )
			br.Col( 0.9f, 0.5, 0.1f, 0.2f );
		else
			br.Col( 0.1f, 0.5, 0.9f, 0.1f );
		
		const EdBlock& B = m_blocks[ block ];
		if( surf == (int) B.poly.size() )
		{
			for( size_t i = 0; i < B.poly.size(); ++i )
			{
				size_t v;
				if( i % 2 == 0 )
					v = i / 2;
				else
					v = B.poly.size() - 1 - i / 2;
				br.Pos( B.poly[ v ].x + B.position.x, B.poly[ v ].y + B.position.y, B.z1 );
			}
	//		br.Prev( B.poly.size() - 1 );
		}
		else if( surf == (int) B.poly.size() + 1 )
		{
			for( size_t i = B.poly.size(); i > 0; )
			{
				--i;
				size_t v;
				if( i % 2 == 0 )
					v = i / 2;
				else
					v = B.poly.size() - 1 - i / 2;
				br.Pos( B.poly[ v ].x + B.position.x, B.poly[ v ].y + B.position.y, B.z0 );
			}
	//		br.Prev( B.poly.size() - 1 );
		}
		else
		{
			size_t v = surf, v1 = ( surf + 1 ) % B.poly.size();
			br.Pos( B.position.x + B.poly[ v ].x,  B.position.y + B.poly[ v ].y, B.z0 );
			br.Pos( B.position.x + B.poly[ v1 ].x, B.position.y + B.poly[ v1 ].y, B.z0 );
			
			br.Pos( B.position.x + B.poly[ v ].x, B.position.y + B.poly[ v ].y, B.z1 );
			br.Pos( B.position.x + B.poly[ v1 ].x,  B.position.y + B.poly[ v1 ].y, B.z1 );
		}
		
		br.Flush();
	}
	void DrawPoly_BlockVertex( int block, int vert, bool sel )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		
		br.SetPrimitiveType( PT_Lines ).UnsetTexture();
		
		if( sel )
			br.Col( 0.9f, 0.5, 0.1f, 0.9f );
		else
			br.Col( 0.1f, 0.5, 0.9f, 0.5f );
		
		const EdBlock& B = m_blocks[ block ];
		Vec3 P = V3( B.poly[ vert ].x + B.position.x, B.poly[ vert ].y + B.position.y, B.z0 );
		
		float s = 0.5f;
		br.Pos( P - V3(s,0,0) ).Pos( P + V3(0,0,s) ).Prev(0).Pos( P + V3(s,0,0) ).Prev(0).Pos( P - V3(0,0,s) ).Prev(0).Prev(6);
		br.Pos( P - V3(0,s,0) ).Pos( P + V3(0,0,s) ).Prev(0).Pos( P + V3(0,s,0) ).Prev(0).Pos( P - V3(0,0,s) ).Prev(0).Prev(6);
		br.Pos( P - V3(s,0,0) ).Pos( P + V3(0,s,0) ).Prev(0).Pos( P + V3(s,0,0) ).Prev(0).Pos( P - V3(0,s,0) ).Prev(0).Prev(6);
	}
	void DrawWires_Entities( int hlmesh, int selmesh )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
		
		br.SetPrimitiveType( PT_Lines ).UnsetTexture();
		for( size_t i = 0; i < m_entities.size(); ++i )
		{
			if( (int) i == selmesh )
				br.Col( 0.9f, 0.5, 0.1f, 0.9f );
			else if( (int) i == hlmesh )
				br.Col( 0.1f, 0.5, 0.9f, 0.7f );
			else
				br.Col( 0.1f, 0.5, 0.9f, 0.25f );
			
			float q = 0.2f;
			Vec3 P = m_entities[ i ]->Pos();
			br.Pos( P - V3(q,0,0) ).Pos( P + V3(0,0,q) ).Prev(0).Pos( P + V3(q,0,0) ).Prev(0).Pos( P - V3(0,0,q) ).Prev(0).Prev(6);
			br.Pos( P - V3(0,q,0) ).Pos( P + V3(0,0,q) ).Prev(0).Pos( P + V3(0,q,0) ).Prev(0).Pos( P - V3(0,0,q) ).Prev(0).Prev(6);
			br.Pos( P - V3(q,0,0) ).Pos( P + V3(0,q,0) ).Prev(0).Pos( P + V3(q,0,0) ).Prev(0).Pos( P - V3(0,q,0) ).Prev(0).Prev(6);
		}
		
		Mat4& iv = g_EdScene->camera.mInvView;
		Vec3 axes[2] = { iv.TransformNormal( V3(1,0,0) ), iv.TransformNormal( V3(0,1,0) ) };
		
		br.Col( 1 );
		for( size_t i = 0; i < m_entities.size(); ++i )
		{
			br.SetTexture( m_entities[ i ]->m_iconTex );
			br.Sprite( m_entities[ i ]->Pos(), axes[0]*0.1f, axes[1]*0.1f );
		}
		
		// debug draw highlighted/selected entities
		if( selmesh >= 0 )
			m_entities[ selmesh ]->DebugDraw();
		if( hlmesh >= 0 && hlmesh != selmesh )
			m_entities[ hlmesh ]->DebugDraw();
		
		br.Flush();
	}
	
	bool RayBlocksIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outblock[1] )
	{
		float ndst[1], mindst = FLT_MAX;
		int curblk = -1;
		if( searchfrom < 0 )
			searchfrom = m_blocks.size();
		for( int i = searchfrom - 1; i >= 0; --i )
		{
			if( m_blocks[ i ].RayIntersect( pos, dir, ndst ) && ndst[0] < mindst )
			{
				curblk = i;
				mindst = ndst[0];
			}
		}
		for( int i = m_blocks.size() - 1; i >= searchfrom; --i )
		{
			if( m_blocks[ i ].RayIntersect( pos, dir, ndst ) && ndst[0] < mindst )
			{
				curblk = i;
				mindst = ndst[0];
			}
		}
		outdst[0] = mindst;
		outblock[0] = curblk;
		return curblk != -1;
	}
	
	bool RayEntitiesIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outent[1] )
	{
		float ndst[1], mindst = FLT_MAX;
		int curent = -1;
		if( searchfrom < 0 )
			searchfrom = m_entities.size();
		for( int i = searchfrom - 1; i >= 0; --i )
		{
			if( m_entities[ i ]->RayIntersect( pos, dir, ndst ) && ndst[0] < mindst )
			{
				curent = i;
				mindst = ndst[0];
			}
		}
		for( int i = m_entities.size() - 1; i >= searchfrom; --i )
		{
			if( m_entities[ i ]->RayIntersect( pos, dir, ndst ) && ndst[0] < mindst )
			{
				curent = i;
				mindst = ndst[0];
			}
		}
		outdst[0] = mindst;
		outent[0] = curent;
		return curent != -1;
	}
	
	EDGUIItem* GetBlockProps( size_t bid )
	{
		m_ctlBlockProps.Prepare( m_blocks[ bid ] );
		return &m_ctlBlockProps;
	}
	EDGUIItem* GetVertProps( size_t bid, size_t vid )
	{
		m_ctlVertProps.Prepare( m_blocks[ bid ], vid );
		return &m_ctlVertProps;
	}
	EDGUIItem* GetSurfProps( size_t bid, size_t sid )
	{
		m_ctlSurfProps.Prepare( m_blocks[ bid ], sid );
		return &m_ctlSurfProps;
	}
	EDGUIItem* GetEntityProps( size_t mid )
	{
		return m_entities[ mid ];
	}
	
	VertexDeclHandle m_vd;
	
	Array< EdBlock > m_blocks;
	Array< EdEntityHandle > m_entities;
	
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
};


void SP_Snap( EDGUISnapProps& SP, Vec2& pos )
{
	if( !SP.m_enableSnap.m_value )
		return;
	
	if( SP.m_snapVerts.m_value )
	{
	}
	
	pos /= SP.m_snapGrid.m_value;
	pos.x = round( pos.x );
	pos.y = round( pos.y );
	pos *= SP.m_snapGrid.m_value;
}


SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
};


struct EDGUIRenderView : EDGUIItem, SGRX_DebugDraw
{
	EDGUIRenderView() :
		movefwd( false ),
		movebwd( false ),
		movelft( false ),
		movergt( false ),
		movefast( false ),
		moveup( false ),
		movedn( false ),
		look( false ),
		hangle( 0 ),
		vangle( 0 ),
		crpos(V3(0)),
		crdir(V3(0)),
		cursor_aim(false),
		cursor_hpos(V2(0)),
		crplaneheight( 0 )
	{
		type = 100000000;
		tyname = "renderview";
		backColor = COLOR_RGBA( 10, 10, 10, 255 );
		
		Vec3 dir = g_EdScene->camera.direction;
		Vec2 dir2 = V2( dir.x, dir.y ).Normalized();
		hangle = atan2( dir2.y, dir2.x );
		vangle = asin( g_EdScene->camera.direction.z );
		g_EdScene->camera.znear = 0.1f;
	}
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_LAYOUT:
			EDGUIItem::OnEvent( e );
			g_EdScene->camera.aspect = ( x1 - x0 ) / (float) ( y1 - y0 );
			return 1;
		case EDGUI_EVENT_PAINT:
			{
				SGRX_Viewport vp = { x0, y0, x1, y1 };
				SGRX_RenderScene rsinfo( V4( GetTimeMsec() / 1000.0f ), g_EdScene );
				rsinfo.viewport = &vp;
				rsinfo.debugdraw = this;
				GR_RenderScene( rsinfo );
			}
			return 1;
		case EDGUI_EVENT_SETFOCUS:
			m_frame->_SetFocus( this );
			return 1;
		case EDGUI_EVENT_LOSEFOCUS:
			movefwd = false;
			movebwd = false;
			movelft = false;
			movergt = false;
			movefast = false;
			moveup = false;
			movedn = false;
			look = false;
			return 1;
		case EDGUI_EVENT_KEYDOWN:
		case EDGUI_EVENT_KEYUP:
			{
				bool down = e->type == EDGUI_EVENT_KEYDOWN && !( e->key.engmod & KMOD_CTRL );
				if( e->key.engkey == SDLK_w ) movefwd = down;
				if( e->key.engkey == SDLK_s ) movebwd = down;
				if( e->key.engkey == SDLK_a ) movelft = down;
				if( e->key.engkey == SDLK_d ) movergt = down;
				if( e->key.engkey == SDLK_LSHIFT ) movefast = down;
				if( e->key.engkey == SDLK_q ) moveup = down;
				if( e->key.engkey == SDLK_z ) movedn = down;
			}
			EventToFrame( e );
			return 1;
		case EDGUI_EVENT_BTNDOWN:
		case EDGUI_EVENT_BTNUP:
			{
				bool down = e->type == EDGUI_EVENT_BTNDOWN;
				if( e->mouse.button == 1 ) look = down;
				prevcp = Game_GetCursorPos();
			}
			EventToFrame( e );
			break;
		case EDGUI_EVENT_BTNCLICK:
			EventToFrame( e );
			break;
		case EDGUI_EVENT_MOUSEMOVE:
			{
				cursor_aim = false;
				Vec2 cp = { e->mouse.x, e->mouse.y };
				if( g_EdScene->camera.GetCursorRay( ( cp.x - x0 ) / ( x1 - x0 ), ( cp.y - y0 ) / ( y1 - y0 ), crpos, crdir ) )
				{
					float dsts[2];
					if( RayPlaneIntersect( crpos, crdir, V4(0,0,1,crplaneheight), dsts ) && dsts[0] > 0 )
					{
						Vec3 isp = crpos + crdir * dsts[0];
						cursor_hpos = V2( isp.x, isp.y );
						cursor_aim = true;
					}
				}
			}
			EventToFrame( e );
			break;
		}
		return EDGUIItem::OnEvent( e );
	}
	void UpdateCamera( float deltaTime )
	{
		float speed = 1;
		if( movefast )
			speed *= 10;
		speed *= deltaTime;
		
		Vec2 cp = Game_GetCursorPos();
		if( look )
		{
			Vec2 diff = ( cp - prevcp );
			hangle -= diff.x * 0.01f;
			vangle -= diff.y * 0.01f;
			vangle = clamp( vangle, (float) -M_PI * 0.49f, (float) M_PI * 0.49f );
		}
		prevcp = cp;
		
		float c_hangle = cos( hangle ), s_hangle = sin( hangle ), c_vangle = cos( vangle ), s_vangle = sin( vangle );
		Vec3 dir = { c_hangle * c_vangle, s_hangle * c_vangle, s_vangle };
		Vec3 up = g_EdScene->camera.up;
		Vec3 rgt = Vec3Cross( dir, up ).Normalized();
		g_EdScene->camera.direction = dir;
		g_EdScene->camera.position += ( dir * ( movefwd - movebwd ) + rgt * ( movergt - movelft ) + up * ( moveup - movedn ) ) * speed;
		
		g_EdScene->camera.UpdateMatrices();
	}
	void EventToFrame( EDGUIEvent* e );
	void DebugDraw();
	
	Vec2 prevcp;
	bool movefwd;
	bool movebwd;
	bool movelft;
	bool movergt;
	bool movefast;
	bool moveup;
	bool movedn;
	bool look;
	float hangle;
	float vangle;
	
	Vec3 crpos, crdir;
	bool cursor_aim;
	Vec2 cursor_hpos;
	float crplaneheight;
};

enum ED_EditMode
{
	ED_DrawBlock,
	ED_EditBlock,
	ED_PaintSurfs,
	ED_AddEntity,
	ED_EditEntity,
	ED_EditLevel,
};

enum ED_BlockDrawMode
{
	BD_Polygon = 1,
	BD_BoxStrip = 2,
};

struct EDGUIMainFrame : EDGUIFrame
{
	EDGUIMainFrame() :
		m_cursorAim( false ),
		m_blockDrawMode( BD_Polygon ),
		m_newBlockPropZ0( 0, 2, -8192, 8192 ),
		m_newBlockPropZ1( 2, 2, -8192, 8192 ),
		m_hlBlock( -1 ),
		m_selBlock( -1 ),
		m_hlSurf( -1 ),
		m_selSurf( -1 ),
		m_hlVert( -1 ),
		m_selVert( -1 ),
		m_dragAdjacent( false ),
		m_paintBlock( -1 ),
		m_paintSurf( -1 ),
		m_isPainting( false ),
		m_hlEnt( -1 ),
		m_selEnt( -1 ),
		m_UIMenuSplit( true, 26, 0 ),
		m_UIParamSplit( false, 0, 0.7f )
	{
		tyname = "mainframe";
		
		Add( &m_UIMenuSplit );
		m_UIMenuSplit.SetFirstPane( &m_UIMenuButtons );
		m_UIMenuSplit.SetSecondPane( &m_UIParamSplit );
		m_UIParamSplit.SetFirstPane( &m_UIRenderView );
		m_UIParamSplit.SetSecondPane( &m_UIParamList );
		
		// menu
		m_MB_Cat0.caption = "File:";
		m_MBNew.caption = "New";
		m_MBOpen.caption = "Open";
		m_MBSave.caption = "Save";
		m_MBSaveAs.caption = "Save As";
		m_MBCompile.caption = "Compile";
		m_MB_Cat1.caption = "Edit:";
		m_MBDrawBlock.caption = "Draw Block";
		m_MBEditBlock.caption = "Edit Block";
		m_MBPaintSurfs.caption = "Paint surfaces";
		m_MBAddEntity.caption = "Add Entity";
		m_MBEditEntity.caption = "Edit Entity";
		m_MBLevelInfo.caption = "Level Info";
		m_newBlockPropZ0.caption = "Bottom Z";
		m_newBlockPropZ1.caption = "Top Z";
		m_UIMenuButtons.Add( &m_MB_Cat0 );
		m_UIMenuButtons.Add( &m_MBNew );
		m_UIMenuButtons.Add( &m_MBOpen );
		m_UIMenuButtons.Add( &m_MBSave );
		m_UIMenuButtons.Add( &m_MBSaveAs );
		m_UIMenuButtons.Add( &m_MBCompile );
		m_UIMenuButtons.Add( &m_MB_Cat1 );
		m_UIMenuButtons.Add( &m_MBDrawBlock );
		m_UIMenuButtons.Add( &m_MBEditBlock );
		m_UIMenuButtons.Add( &m_MBPaintSurfs );
		m_UIMenuButtons.Add( &m_MBAddEntity );
		m_UIMenuButtons.Add( &m_MBEditEntity );
		m_UIMenuButtons.Add( &m_MBLevelInfo );
		
		m_entityProps = m_entGroup.m_buttons[0].m_ent_handle;
		
		m_txMarker = GR_GetTexture( "editor/marker.png" );
		
		SetMode( ED_EditLevel );
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if(0);
			
			else if( e->target == &m_MBNew ) Level_New();
			else if( e->target == &m_MBOpen ) Level_Open();
			else if( e->target == &m_MBSave ) Level_Save();
			else if( e->target == &m_MBSaveAs ) Level_SaveAs();
			else if( e->target == &m_MBCompile ) Level_Compile();
			
			else if( e->target == &m_MBDrawBlock ) SetMode( ED_DrawBlock );
			else if( e->target == &m_MBEditBlock ) SetMode( ED_EditBlock );
			else if( e->target == &m_MBPaintSurfs ) SetMode( ED_PaintSurfs );
			else if( e->target == &m_MBAddEntity ) SetMode( ED_AddEntity );
			else if( e->target == &m_MBEditEntity ) SetMode( ED_EditEntity );
			else if( e->target == &m_MBLevelInfo ) SetMode( ED_EditLevel );
			
			return 1;
			
		case EDGUI_EVENT_SETENTITY:
			SetEntityType( ((EDGUIEntButton*)e->target)->m_ent_handle );
			return 1;
			
		case EDGUI_EVENT_PROPCHANGE:
			if( e->target == g_UILevelOpenPicker )
			{
				Level_Real_Open( g_UILevelOpenPicker->GetValue() );
			}
			if( e->target == g_UILevelSavePicker )
			{
				Level_Real_Save( g_UILevelSavePicker->GetValue() );
			}
			if( e->target == &m_newBlockPropZ0 )
			{
				m_UIRenderView.crplaneheight = m_newBlockPropZ0.m_value;
			}
			if( e->target == &m_entityProps->m_ctlPos )
			{
				m_UIRenderView.crplaneheight = m_entityProps->Pos().z;
			}
			if( m_selEnt >= 0 && e->target == &g_EdWorld->m_entities[ m_selEnt ]->m_ctlPos )
			{
				m_UIRenderView.crplaneheight = g_EdWorld->m_entities[ m_selEnt ]->Pos().z;
			}
			return 1;
		}
		return EDGUIFrame::OnEvent( e );
	}
	void ViewEvent( EDGUIEvent* e )
	{
		if( e->type == EDGUI_EVENT_MOUSEMOVE )
		{
			m_cursorAim = m_UIRenderView.cursor_aim;
			m_cursorWorldPos = m_UIRenderView.cursor_hpos;
			SP_Snap( m_snapProps, m_cursorWorldPos );
		}
		
		if( m_mode == ED_DrawBlock )
		{
			if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 && m_cursorAim && m_drawnVerts.size() < 14 )
			{
				m_drawnVerts.push_back( m_cursorWorldPos );
			}
			if( e->type == EDGUI_EVENT_KEYUP )
			{
				if( e->key.engkey == SDLK_RETURN )
				{
					if( m_blockDrawMode == BD_Polygon )
					{
						if( m_drawnVerts.size() >= 3 && m_drawnVerts.size() <= 14 )
						{
							_AddNewBlock();
							m_drawnVerts.clear();
						}
					}
					else if( m_blockDrawMode == BD_BoxStrip )
					{
						if( m_drawnVerts.size() >= 2 )
						{
							Array< Vec2 > verts;
							TSWAP( verts, m_drawnVerts );
							for( size_t i = 1; i < verts.size(); ++i )
							{
								Vec2 p0 = verts[ i - 1 ];
								Vec2 p1 = verts[ i ];
								m_drawnVerts.push_back( V2( p0.x, p0.y ) );
								m_drawnVerts.push_back( V2( p1.x, p0.y ) );
								m_drawnVerts.push_back( V2( p1.x, p1.y ) );
								m_drawnVerts.push_back( V2( p0.x, p1.y ) );
								_AddNewBlock();
								m_drawnVerts.clear();
							}
						}
					}
				}
				if( e->key.engkey == SDLK_ESCAPE )
				{
					m_drawnVerts.clear();
				}
				if( e->key.engkey == SDLK_BACKSPACE && m_drawnVerts.size() )
				{
					m_drawnVerts.pop_back();
				}
				if( e->key.engkey == SDLK_1 ) m_blockDrawMode = BD_Polygon;
				if( e->key.engkey == SDLK_2 ) m_blockDrawMode = BD_BoxStrip;
			}
		}
		else if( m_mode == ED_EditBlock )
		{
			if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 )
			{
				if( m_selBlock != -1 && m_hlVert != -1 )
				{
					if( m_selVert == m_hlVert )
						m_selVert = -1;
					else
					{
						m_selVert = m_hlVert;
						m_selSurf = -1;
					}
				}
				else if( m_selBlock != -1 && m_hlSurf != -1 )
				{
					if( m_selSurf == m_hlSurf )
						m_selSurf = -1;
					else
					{
						m_selSurf = m_hlSurf;
						m_selVert = -1;
					}
				}
				else
				{
					m_selBlock = m_hlBlock;
					m_selSurf = -1;
					m_selVert = -1;
				}
				_ReloadBlockProps();
				m_frame->_HandleMouseMove( false );
			}
			if( e->type == EDGUI_EVENT_MOUSEMOVE )
			{
				float outdst[1];
				int outblock[1];
				if( m_selBlock >= 0 )
				{
					EdBlock& B = g_EdWorld->m_blocks[ m_selBlock ];
					float mindst = FLT_MAX;
					m_hlVert = -1;
					for( size_t i = 0; i < B.poly.size(); ++i )
					{
						if( RaySphereIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, V3( B.poly[i].x + B.position.x, B.poly[i].y + B.position.y, B.z0 ), 0.2f, outdst ) && outdst[0] < mindst )
						{
							mindst = outdst[0];
							m_hlVert = i;
						}
					}
					m_hlSurf = -1;
					if( B.RayIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, outdst, outblock ) && outdst[0] < mindst )
					{
						m_hlSurf = outblock[0];
						m_hlVert = -1;
					}
				}
				if( m_selBlock < 0 || ( m_hlVert < 0 && m_hlSurf < 0 ) )
				{
					m_hlBlock = -1;
					if( g_EdWorld->RayBlocksIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, m_selBlock, outdst, outblock ) )
						m_hlBlock = outblock[0];
				}
				if( m_grabbed && m_selBlock >= 0 )
				{
					EdBlock& B = g_EdWorld->m_blocks[ m_selBlock ];
					Vec2 tgtpos = m_cursorWorldPos + m_cpdiff;
					int selvert = m_selVert;
					bool itssurf = false;
					if( selvert < 0 && m_selSurf < (int) B.poly.size() )
					{
						selvert = m_selSurf;
						itssurf = true;
					}
					if( selvert >= 0 )
					{
						SP_Snap( m_snapProps, tgtpos );
						B.poly[ selvert ].SetXY( tgtpos );
						if( m_dragAdjacent || itssurf )
						{
							size_t bps = B.poly.size();
							Vec2 edgeNrm0 = ( m_origPos - m_origPos0 ).Perp().Normalized();
							Vec2 edgeNrm1 = ( m_origPos1 - m_origPos ).Perp().Normalized();
							Vec2 diff = tgtpos - m_origPos;
							if( itssurf )
							{
								diff = edgeNrm1 * Vec2Dot( edgeNrm1, diff );
								B.poly[ selvert ].SetXY( m_origPos + edgeNrm1 * Vec2Dot( edgeNrm1, diff ) );
							}
							B.poly[ ( selvert + bps - 1 ) % bps ].SetXY( m_origPos0 + edgeNrm0 * Vec2Dot( edgeNrm0, diff ) );
							B.poly[ ( selvert + 1 ) % bps ].SetXY( m_origPos1 + edgeNrm1 * Vec2Dot( edgeNrm1, diff ) );
						}
					}
					else
						B.position = tgtpos;
					g_EdWorld->m_blocks[ m_selBlock ].RegenerateMesh();
					_ReloadBlockProps();
				}
			}
			// GRAB
			if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_g && !e->key.repeat && m_selBlock >= 0 && m_cursorAim )
			{
				EdBlock& B = g_EdWorld->m_blocks[ m_selBlock ];
				int selvert = m_selVert;
				if( selvert < 0 && m_selSurf < (int) B.poly.size() )
					selvert = m_selSurf;
				if( selvert >= 0 )
				{
					size_t bps = B.poly.size();
					m_origPos = B.poly[ selvert ].ToVec2();
					m_origPos0 = B.poly[ ( selvert + bps - 1 ) % bps ].ToVec2();
					m_origPos1 = B.poly[ ( selvert + 1 ) % bps ].ToVec2();
					m_cpdiff = m_origPos - m_cursorWorldPos;
				}
				else
					m_cpdiff = B.position - m_cursorWorldPos;
				m_dragAdjacent = ( e->key.engmod & KMOD_CTRL ) != 0;
				m_grabbed = true;
			}
			if( e->type == EDGUI_EVENT_KEYUP && ( e->key.engkey == SDLK_g || e->key.engkey == SDLK_d ) && m_grabbed )
			{
				m_grabbed = false;
			}
			// DELETE
			if( e->type == EDGUI_EVENT_KEYUP && e->key.engkey == SDLK_DELETE && m_selBlock >= 0 )
			{
				ClearParamList();
				if( m_selVert >= 0 )
				{
					g_EdWorld->m_ctlVertProps.m_out = NULL; // just in case
					EdBlock& B = g_EdWorld->m_blocks[ m_selBlock ];
					B.poly.erase( m_selVert );
					B.surfaces.erase( m_selVert );
					if( m_hlVert == m_selVert )
						m_hlVert = -1;
					m_hlSurf = -1;
					m_selSurf = -1;
					m_selVert = -1;
					B.RegenerateMesh();
				}
				else
				{
					g_EdWorld->m_ctlBlockProps.m_out = NULL; // just in case
					g_EdWorld->m_blocks.erase( m_selBlock );
					if( m_hlBlock == m_selBlock )
						m_hlBlock = -1;
					m_selBlock = -1;
					m_selVert = -1;
					m_selSurf = -1;
				}
			}
			// DUPLICATE
			if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_d && !e->key.repeat && ( e->key.engmod & KMOD_CTRL ) && m_selBlock >= 0 )
			{
				ClearParamList();
				g_EdWorld->m_ctlBlockProps.m_out = NULL; // just in case
				EdBlock B = g_EdWorld->m_blocks[ m_selBlock ];
				m_selBlock = g_EdWorld->m_blocks.size();
				m_selVert = -1;
				m_selSurf = -1;
				B.cached_mesh = NULL;
				B.cached_meshinst = NULL;
				B.RegenerateMesh();
				g_EdWorld->m_blocks.push_back( B );
				m_cpdiff = B.position - m_cursorWorldPos;
				m_grabbed = true;
				AddToParamList( g_EdWorld->GetBlockProps( m_selBlock ) );
			}
		}
		else if( m_mode == ED_PaintSurfs )
		{
			bool dopaint = false;
			if( e->type == EDGUI_EVENT_BTNDOWN && e->mouse.button == 0 )
			{
				m_isPainting = true;
				dopaint = true;
			}
			if( e->type == EDGUI_EVENT_BTNUP && e->mouse.button == 0 ) m_isPainting = false;
			if( e->type == EDGUI_EVENT_MOUSEMOVE )
			{
				float outdst[1];
				int outblock[1];
				m_paintBlock = -1;
				if( g_EdWorld->RayBlocksIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, m_paintBlock, outdst, outblock ) )
					m_paintBlock = outblock[0];
				m_paintSurf = -1;
				if( m_paintBlock >= 0 && g_EdWorld->m_blocks[ m_paintBlock ].RayIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, outdst, outblock ) )
					m_paintSurf = outblock[0];
				if( m_isPainting )
					dopaint = true;
			}
			if( e->type == EDGUI_EVENT_KEYDOWN && !e->key.repeat && e->key.engkey == SDLK_g && m_paintBlock >= 0 && m_paintSurf >= 0 )
			{
				m_paintSurfProps.LoadParams( g_EdWorld->m_blocks[ m_paintBlock ].surfaces[ m_paintSurf ] );
			}
			if( dopaint && m_paintBlock >= 0 && m_paintSurf >= 0 )
			{
				m_paintSurfProps.BounceBack( g_EdWorld->m_blocks[ m_paintBlock ].surfaces[ m_paintSurf ] );
				g_EdWorld->m_blocks[ m_paintBlock ].RegenerateMesh();
			}
		}
		else if( m_mode == ED_AddEntity )
		{
			if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 && m_cursorAim )
			{
				_AddNewEntity();
			}
		}
		else if( m_mode == ED_EditEntity )
		{
			if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 )
			{
				m_grabbed = false;
				m_selEnt = m_hlEnt;
				_ReloadEntityProps();
			}
			if( e->type == EDGUI_EVENT_MOUSEMOVE )
			{
				float outdst[1];
				int outmesh[1];
				m_hlEnt = -1;
				if( g_EdWorld->RayEntitiesIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, m_selEnt, outdst, outmesh ) )
					m_hlEnt = outmesh[0];
				if( m_grabbed && m_selEnt >= 0 )
				{
					EdEntity* N = g_EdWorld->m_entities[ m_selEnt ];
					N->SetPosition( V3( m_cursorWorldPos.x + m_cpdiff.x, m_cursorWorldPos.y + m_cpdiff.y, N->Pos().z ) );
					N->RegenerateMesh();
					_ReloadEntityProps();
				}
			}
			// GRAB
			if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_g && !e->key.repeat && m_selEnt >= 0 && m_cursorAim )
			{
				Vec3 P = g_EdWorld->m_entities[ m_selEnt ]->Pos();
				m_cpdiff = V2(P.x,P.y) - m_cursorWorldPos;
				m_grabbed = true;
			}
			if( e->type == EDGUI_EVENT_KEYUP && ( e->key.engkey == SDLK_g || e->key.engkey == SDLK_d ) && m_grabbed )
			{
				m_grabbed = false;
			}
			// DELETE
			if( e->type == EDGUI_EVENT_KEYUP && e->key.engkey == SDLK_DELETE && m_selEnt >= 0 )
			{
				ClearParamList();
				g_EdWorld->m_entities.erase( m_selEnt );
				if( m_hlEnt == m_selEnt )
					m_hlEnt = -1;
				m_selEnt = -1;
			}
			// DUPLICATE
			if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_d && !e->key.repeat && ( e->key.engmod & KMOD_CTRL ) && m_selEnt >= 0 )
			{
				EdEntity* N = g_EdWorld->m_entities[ m_selEnt ]->Clone();
				m_selEnt = g_EdWorld->m_entities.size();
				N->RegenerateMesh();
				g_EdWorld->m_entities.push_back( N );
				Vec3 P = N->Pos();
				m_cpdiff = V2(P.x,P.y) - m_cursorWorldPos;
				m_grabbed = true;
				_ReloadEntityProps();
			}
		}
	}
	void _ReloadBlockProps()
	{
		ClearParamList();
		if( m_selBlock >= 0 )
		{
			m_UIRenderView.crplaneheight = g_EdWorld->m_blocks[ m_selBlock ].z0;
			if( m_selVert >= 0 )
			{
				AddToParamList( g_EdWorld->GetVertProps( m_selBlock, m_selVert ) );
			}
			else if( m_selSurf >= 0 )
				AddToParamList( g_EdWorld->GetSurfProps( m_selBlock, m_selSurf ) );
			else
				AddToParamList( g_EdWorld->GetBlockProps( m_selBlock ) );
		}
	}
	void _ReloadEntityProps()
	{
		ClearParamList();
		if( m_selEnt >= 0 )
		{
			m_UIRenderView.crplaneheight = g_EdWorld->m_entities[ m_selEnt ]->Pos().z;
			AddToParamList( g_EdWorld->GetEntityProps( m_selEnt ) );
		}
	}
	void _DrawCursor( bool drawimg, float height )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.UnsetTexture();
		if( m_cursorAim )
		{
			Vec2 pos = m_cursorWorldPos;
			if( m_drawnVerts.size() > 1 )
			{
				br.Col( 0.9f, 0.1f, 0, 0.4f ).SetPrimitiveType( PT_LineStrip );
				br.Pos( m_drawnVerts.last().x, m_drawnVerts.last().y, height );
				br.Pos( pos.x, pos.y, height );
				br.Pos( m_drawnVerts[0].x, m_drawnVerts[0].y, height );
			}
			if( drawimg )
			{
				br.SetTexture( m_txMarker ).Col( 0.9f, 0.1f, 0, 0.9f );
				br.Box( pos.x, pos.y, 1, 1, height );
			}
			br.UnsetTexture().SetPrimitiveType( PT_Lines );
			// up
			br.Col( 0, 0.1f, 0.9f, 0.9f ).Pos( pos.x, pos.y, height );
			br.Col( 0, 0.1f, 0.9f, 0.0f ).Pos( pos.x, pos.y, 4 + height );
			// -X
			br.Col( 0.5f, 0.1f, 0, 0.9f ).Pos( pos.x, pos.y, height );
			br.Col( 0.5f, 0.1f, 0, 0.0f ).Pos( pos.x - 4, pos.y, height );
			// +X
			br.Col( 0.9f, 0.1f, 0, 0.9f ).Pos( pos.x, pos.y, height );
			br.Col( 0.9f, 0.1f, 0, 0.0f ).Pos( pos.x + 4, pos.y, height );
			// -Y
			br.Col( 0.1f, 0.5f, 0, 0.9f ).Pos( pos.x, pos.y, height );
			br.Col( 0.1f, 0.5f, 0, 0.0f ).Pos( pos.x, pos.y - 4, height );
			// +Y
			br.Col( 0.1f, 0.9f, 0, 0.9f ).Pos( pos.x, pos.y, height );
			br.Col( 0.1f, 0.9f, 0, 0.0f ).Pos( pos.x, pos.y + 4, height );
		}
	}
	void DrawCursor( bool drawimg = true )
	{
		_DrawCursor( drawimg, 0 );
		if( m_UIRenderView.crplaneheight )
			_DrawCursor( drawimg, m_UIRenderView.crplaneheight );
	}
	void DebugDraw()
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		if( m_mode == ED_DrawBlock )
		{
			br.UnsetTexture();
			if( m_blockDrawMode == BD_Polygon && m_drawnVerts.size() >= 3 )
			{
				br.Flush();
				br.Col( 0.9f, 0.1f, 0, 0.3f ).SetPrimitiveType( PT_TriangleStrip );
				for( size_t i = 0; i < m_drawnVerts.size(); ++i )
				{
					size_t v;
					if( i % 2 == 0 )
						v = i / 2;
					else
						v = m_drawnVerts.size() - 1 - i / 2;
					br.Pos( m_drawnVerts[v].x, m_drawnVerts[v].y, m_UIRenderView.crplaneheight );
				}
				br.Flush();
			}
			if( m_blockDrawMode == BD_BoxStrip )
			{
				for( size_t i = 1; i < m_drawnVerts.size(); ++i )
				{
					br.Col( 0.9f, 0.1f, 0, 0.3f ).SetPrimitiveType( PT_TriangleStrip );
					Vec2 p0 = m_drawnVerts[ i - 1 ];
					Vec2 p1 = m_drawnVerts[ i ];
					float z = m_UIRenderView.crplaneheight;
					br.Pos( p0.x, p0.y, z );
					br.Pos( p1.x, p0.y, z );
					br.Pos( p0.x, p1.y, z );
					br.Pos( p1.x, p1.y, z );
					br.Flush();
				}
			}
			if( m_drawnVerts.size() >= 2 )
			{
				br.Flush();
				br.Col( 0.9f, 0.1f, 0, 0.7f ).SetPrimitiveType( PT_LineStrip );
				for( size_t i = 0; i < m_drawnVerts.size(); ++i )
					br.Pos( m_drawnVerts[i].x, m_drawnVerts[i].y, m_UIRenderView.crplaneheight );
				br.Pos( m_drawnVerts[0].x, m_drawnVerts[0].y, m_UIRenderView.crplaneheight );
				br.Flush();
			}
			for( size_t i = 0; i < m_drawnVerts.size(); ++i )
			{
				br.Col( 0.9f, 0.1f, 0, 0.8f );
				br.CircleOutline( m_drawnVerts[i].x, m_drawnVerts[i].y, 0.02f, m_UIRenderView.crplaneheight, 16 );
			}
			if( m_blockDrawMode == BD_Polygon && m_cursorAim )
			{
				Vec2 pos = m_cursorWorldPos;
				if( m_drawnVerts.size() > 1 )
				{
					br.Flush();
					br.Col( 0.9f, 0.1f, 0, 0.4f ).SetPrimitiveType( PT_LineStrip );
					br.Pos( m_drawnVerts.last().x, m_drawnVerts.last().y, m_UIRenderView.crplaneheight );
					br.Pos( pos.x, pos.y, m_UIRenderView.crplaneheight );
					br.Pos( m_drawnVerts[0].x, m_drawnVerts[0].y, m_UIRenderView.crplaneheight );
					br.Flush();
				}
			}
			DrawCursor();
		}
		else if( m_mode == ED_EditBlock )
		{
			g_EdWorld->DrawWires_Blocks( m_hlBlock, m_selBlock );
			if( m_selBlock >= 0 )
			{
				if( m_selSurf >= 0 )
					g_EdWorld->DrawPoly_BlockSurf( m_selBlock, m_selSurf, true );
				if( m_hlSurf >= 0 )
					g_EdWorld->DrawPoly_BlockSurf( m_selBlock, m_hlSurf, false );
				if( m_selVert >= 0 )
					g_EdWorld->DrawPoly_BlockVertex( m_selBlock, m_selVert, true );
				if( m_hlVert >= 0 )
					g_EdWorld->DrawPoly_BlockVertex( m_selBlock, m_hlVert, true );
			}
			if( m_grabbed )
				DrawCursor( false );
		}
		else if( m_mode == ED_PaintSurfs )
		{
			if( m_paintBlock >= 0 && m_paintSurf >= 0 )
			{
				g_EdWorld->DrawPoly_BlockSurf( m_paintBlock, m_paintSurf, m_isPainting );
			}
		}
		else if( m_mode == ED_AddEntity )
		{
			g_EdWorld->DrawWires_Entities( -1, -1 );
			DrawCursor( false );
		}
		else if( m_mode == ED_EditEntity )
		{
			g_EdWorld->DrawWires_Entities( m_hlEnt, m_selEnt );
			if( m_grabbed )
				DrawCursor( false );
		}
		br.Flush();
	}
	
	void AddToParamList( EDGUIItem* item )
	{
		m_UIParamList.Add( item );
	}
	void ClearParamList()
	{
		while( m_UIParamList.m_subitems.size() )
			m_UIParamList.Remove( m_UIParamList.m_subitems.last() );
	}
	
	void ResetEditorState()
	{
		ClearParamList();
		m_hlBlock = -1;
		m_selBlock = -1;
		m_hlVert = -1;
		m_selVert = -1;
		m_hlSurf = -1;
		m_selSurf = -1;
		m_hlEnt = -1;
		m_selEnt = -1;
	}
	
	void Level_New()
	{
		m_fileName = "";
		g_EdWorld->Reset();
		ResetEditorState();
	}
	void Level_Open()
	{
		g_UILevelOpenPicker->Reload();
		g_UILevelOpenPicker->Open( this, "" );
		m_frame->Add( g_UILevelOpenPicker );
	}
	void Level_Save()
	{
		if( m_fileName.size() )
		{
			Level_Real_Save( m_fileName );
		}
		else
		{
			Level_SaveAs();
		}
	}
	void Level_SaveAs()
	{
		g_UILevelSavePicker->Reload();
		g_UILevelSavePicker->Open( this, "" );
		m_frame->Add( g_UILevelSavePicker );
	}
	void Level_Compile()
	{
		if( m_fileName.size() )
		{
			Level_Real_Compile();
		}
		else
		{
			Level_SaveAs();
		}
	}
	void Level_Real_Open( const String& str )
	{
		LOG << "Trying to open level: " << str;
		
		char bfr[ 256 ];
		snprintf( bfr, sizeof(bfr), "levels/%.*s.tle", TMIN( (int) str.size(), 200 ), str.data() );
		String data;
		if( !FS_LoadTextFile( bfr, data ) )
		{
			LOG_ERROR << "FAILED TO LOAD LEVEL FILE: " << bfr;
			return;
		}
		
		ResetEditorState();
		
		TextReader tr( &data );
		g_EdWorld->Serialize( tr );
		if( tr.error )
		{
			LOG_ERROR << "FAILED TO READ LEVEL FILE (at " << (int) tr.pos << "): " << bfr;
			return;
		}
		
		m_fileName = str;
	}
	void Level_Real_Save( const String& str )
	{
		LOG << "Trying to save level: " << str;
		String data;
		TextWriter arch( &data );
		
		arch << *g_EdWorld;
		
		char bfr[ 256 ];
		snprintf( bfr, sizeof(bfr), "levels/%.*s.tle", TMIN( (int) str.size(), 200 ), str.data() );
		if( !FS_SaveTextFile( bfr, data ) )
		{
			LOG_ERROR << "FAILED TO SAVE LEVEL FILE: " << bfr;
			return;
		}
		
		m_fileName = str;
	}
	void Level_Real_Compile()
	{
		LOG << "Compiling level";
		LevelCache lcache;
		
		lcache.AmbientColor = HSV( g_EdWorld->m_ctlAmbientColor.m_value );
		lcache.LightmapClearColor = HSV( g_EdWorld->m_ctlLightmapClearColor.m_value );
		lcache.RADNumBounces = g_EdWorld->m_ctlRADNumBounces.m_value;
		lcache.LightmapDetail = g_EdWorld->m_ctlLightmapDetail.m_value;
		lcache.LightmapBlurSize = g_EdWorld->m_ctlLightmapBlurSize.m_value;
		lcache.AODistance = g_EdWorld->m_ctlAODistance.m_value;
		lcache.AOMultiplier = g_EdWorld->m_ctlAOMultiplier.m_value;
		lcache.AOFalloff = g_EdWorld->m_ctlAOFalloff.m_value;
		lcache.AOEffect = g_EdWorld->m_ctlAOEffect.m_value;
	//	lcache.AODivergence = g_EdWorld->m_ctlAODivergence.m_value;
		lcache.AOColor = HSV( g_EdWorld->m_ctlAOColor.m_value );
		lcache.AONumSamples = g_EdWorld->m_ctlAONumSamples.m_value;
		
		// DIRECTIONAL LIGHT
		if( g_EdWorld->m_ctlDirLightColor.m_value.z && g_EdWorld->m_ctlDirLightNumSamples.m_value )
		{
			LC_Light L;
			L.type = LM_LIGHT_DIRECT;
			L.range = 1024;
			Vec2 dir = g_EdWorld->m_ctlDirLightDir.m_value;
			L.dir = -V3( dir.x, dir.y, -1 ).Normalized();
			L.color = HSV( g_EdWorld->m_ctlDirLightColor.m_value );
			L.light_radius = g_EdWorld->m_ctlDirLightDivergence.m_value / 180.0f;
			L.num_shadow_samples = g_EdWorld->m_ctlDirLightNumSamples.m_value;
			lcache.AddLight( L );
		}
		
		for( size_t i = 0; i < g_EdWorld->m_blocks.size(); ++i )
			g_EdWorld->m_blocks[ i ].GenerateMesh( lcache );
		
		for( size_t i = 0; i < g_EdWorld->m_entities.size(); ++i )
			g_EdWorld->m_entities[ i ]->UpdateCache( lcache );
		
		char bfr[ 256 ];
		snprintf( bfr, sizeof(bfr), "levels/%.*s", TMIN( (int) m_fileName.size(), 200 ), m_fileName.data() );
		
		if( !lcache.SaveCache( bfr ) )
			LOG_ERROR << "FAILED TO SAVE CACHE";
		else
			LOG << "Level is compiled";
	}
	
	void SetMode( ED_EditMode newmode )
	{
		m_mode = newmode;
		m_MBDrawBlock.SetHighlight( newmode == ED_DrawBlock );
		m_MBEditBlock.SetHighlight( newmode == ED_EditBlock );
		m_MBAddEntity.SetHighlight( newmode == ED_AddEntity );
		m_MBEditEntity.SetHighlight( newmode == ED_EditEntity );
		m_MBLevelInfo.SetHighlight( newmode == ED_EditLevel );
		ClearParamList();
		if( newmode == ED_DrawBlock )
		{
			m_UIRenderView.crplaneheight = m_newBlockPropZ0.m_value;
			m_drawnVerts.clear();
			AddToParamList( &m_snapProps );
			AddToParamList( &m_newBlockPropZ0 );
			AddToParamList( &m_newBlockPropZ1 );
			AddToParamList( &m_newSurfProps );
		}
		else if( newmode == ED_EditBlock )
		{
			m_hlBlock = -1;
			m_selBlock = -1;
			m_grabbed = false;
		}
		else if( newmode == ED_PaintSurfs )
		{
			m_paintBlock = -1;
			m_paintSurf = -1;
			m_isPainting = false;
			AddToParamList( &m_paintSurfProps );
		}
		else if( newmode == ED_AddEntity )
		{
			m_UIRenderView.crplaneheight = m_entityProps->Pos().z;
			AddToParamList( &m_entGroup );
			AddToParamList( m_entityProps );
		}
		else if( newmode == ED_EditEntity )
		{
			m_hlEnt = -1;
			m_selEnt = -1;
			m_grabbed = false;
		}
		else if( newmode == ED_EditLevel )
		{
			AddToParamList( g_EdWorld );
		}
	}
	void SetEntityType( const EdEntityHandle& eh )
	{
		m_entityProps = eh;
		if( m_mode == ED_AddEntity )
		{
			ClearParamList();
			AddToParamList( &m_entGroup );
			AddToParamList( m_entityProps );
		}
	}
	
	void _AddNewBlock()
	{
		EdBlock B;
		B.position = V2(0,0);
		B.z0 = m_newBlockPropZ0.m_value;
		B.z1 = m_newBlockPropZ1.m_value;
		B.poly.resize( m_drawnVerts.size() );
		for( size_t i = 0; i < m_drawnVerts.size(); ++i )
			B.poly[ i ] = V3( m_drawnVerts[ i ].x, m_drawnVerts[ i ].y, 0 );
		if( PolyArea( m_drawnVerts.data(), m_drawnVerts.size() ) < 0 )
			B.poly.reverse();
		B.GenCenterPos( m_snapProps );
		for( size_t i = 0; i < m_drawnVerts.size() + 2; ++i )
		{
			EdSurface S;
			m_newSurfProps.BounceBack( S );
			B.surfaces.push_back( S );
		}
		B.RegenerateMesh();
		g_EdWorld->m_blocks.push_back( B );
	}
	void _AddNewEntity()
	{
		EdEntity* N = m_entityProps->Clone();
		N->SetPosition( V3( m_cursorWorldPos.x, m_cursorWorldPos.y, N->Pos().z ) );
		N->RegenerateMesh();
		g_EdWorld->m_entities.push_back( N );
	}
	
	String m_fileName;
	
	ED_EditMode m_mode;
	
	// UNIVERSAL
	TextureHandle m_txMarker;
	EDGUISnapProps m_snapProps;
	bool m_cursorAim;
	Vec2 m_cursorWorldPos;
	Vec2 m_cpdiff;
	bool m_grabbed;
	Vec2 m_origPos;
	Vec2 m_origPos0;
	Vec2 m_origPos1;
	// --
	
	// DRAW BLOCK
	ED_BlockDrawMode m_blockDrawMode;
	Array< Vec2 > m_drawnVerts;
	EDGUIPropFloat m_newBlockPropZ0;
	EDGUIPropFloat m_newBlockPropZ1;
	EDGUISurfaceProps m_newSurfProps;
	// --
	
	// EDIT BLOCK
	int m_hlBlock;
	int m_selBlock;
	int m_hlSurf;
	int m_selSurf;
	int m_hlVert;
	int m_selVert;
	bool m_dragAdjacent;
	// --
	
	// PAINT BLOCK
	int m_paintBlock;
	int m_paintSurf;
	bool m_isPainting;
	EDGUISurfaceProps m_paintSurfProps;
	
	// ADD ENTITY
	EdEntityHandle m_entityProps;
	
	// EDIT ENTITY
	int m_hlEnt;
	int m_selEnt;
	// --
	
	// core layout
	EDGUILayoutSplitPane m_UIMenuSplit;
	EDGUILayoutSplitPane m_UIParamSplit;
	EDGUILayoutColumn m_UIMenuButtons;
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
	EDGUIButton m_MBEditBlock;
	EDGUIButton m_MBPaintSurfs;
	EDGUIButton m_MBAddEntity;
	EDGUIButton m_MBEditEntity;
	EDGUIButton m_MBLevelInfo;
	
	// entity list
	EDGUIEntList m_entGroup;
};

void EDGUIRenderView::EventToFrame( EDGUIEvent* e )
{
	g_UIFrame->ViewEvent( e );
}

void EDGUIRenderView::DebugDraw()
{
	g_UIFrame->DebugDraw();
}

StringView GetLevelName()
{
	if( !g_UIFrame )
		return StringView();
	return g_UIFrame->m_fileName;
}


struct TACStrikeEditor : IGame
{
	void OnInitialize()
	{
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
		GR2D_SetFont( "fonts/lato-regular.ttf", 12 );
		
		g_ScriptCtx = new ScriptContext;
		g_ScriptCtx->RegisterBatchRenderer();
		sgs_RegFuncConsts( g_ScriptCtx->C, g_ent_scripted_rfc, -1 );
		
		LOG << "\n\nLoading scripted entities:\n\n";
		g_ScriptCtx->ExecFile( "editor/entities.sgs" );
		LOG << "\nLoading completed\n\n";
		
		g_UISurfTexPicker = new EDGUISurfTexPicker;
		g_UIMeshPicker = new EDGUIMeshPicker;
		g_UIPartSysPicker = new EDGUIPartSysPicker;
		g_UISoundPicker = new EDGUISoundPicker;
		g_UIScrFnPicker = new EDGUIScrFnPicker;
		g_UILevelOpenPicker = new EDGUILevelOpenPicker;
		g_UILevelSavePicker = new EDGUILevelSavePicker;
		
		// core layout
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = Vec3::Create(3,3,3);
		g_EdScene->camera.UpdateMatrices();
		g_EdWorld = new EdWorld;
		g_UIFrame = new EDGUIMainFrame();
		g_UIFrame->Resize( GR_GetWidth(), GR_GetHeight() );
		
		// param area
		g_UIFrame->AddToParamList( g_EdWorld );
	}
	void OnDestroy()
	{
		delete g_UILevelSavePicker;
		g_UILevelSavePicker = NULL;
		delete g_UILevelOpenPicker;
		g_UILevelOpenPicker = NULL;
		delete g_UIScrFnPicker;
		g_UIScrFnPicker = NULL;
		delete g_UIPartSysPicker;
		g_UIPartSysPicker = NULL;
		delete g_UISoundPicker;
		g_UISoundPicker = NULL;
		delete g_UIMeshPicker;
		g_UIMeshPicker = NULL;
		delete g_UISurfTexPicker;
		g_UISurfTexPicker = NULL;
		delete g_UIFrame;
		g_UIFrame = NULL;
		delete g_EdWorld;
		g_EdWorld = NULL;
		g_EdScene = NULL;
		delete g_ScriptCtx;
		g_ScriptCtx = NULL;
	}
	void OnEvent( const Event& e )
	{
		g_UIFrame->EngineEvent( &e );
	}
	void OnTick( float dt, uint32_t gametime )
	{
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		g_UIFrame->m_UIRenderView.UpdateCamera( dt );
		g_UIFrame->Draw();
	}
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

