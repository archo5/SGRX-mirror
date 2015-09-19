

#pragma once
#include <engine.hpp>
#include <edgui.hpp>
#include <script.hpp>
#include <sound.hpp>


// nullifies mesh lighting
inline void lmm_prepmeshinst( MeshInstHandle mih )
{
	mih->SetLightingMode( SGRX_LM_Dynamic );
	for( int i = 10; i < 16; ++i )
		mih->constants[ i ] = V4(0.15f);
}



struct EDGUISDTexPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUISDTexPicker( const StringView& dir = "textures" ) : m_dir( String_Concat( dir, "/" ) )
	{
		caption = "Pick a texture";
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading textures";
		m_options.clear();
		m_textures.clear();
		m_options.push_back( "" );
		m_textures.push_back( TextureHandle() );
		FS_IterateDirectory( StringView(m_dir).until_last("/"), this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[T]: " << name;
		if( !isdir && name.ends_with( ".png" ) )
		{
			m_options.push_back( name.part( 0, name.size() - 4 ) );
			m_textures.push_back( GR_GetTexture( String_Concat( m_dir, name ) ) );
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
	
	String m_dir;
	Array< TextureHandle > m_textures;
};


struct EDGUIMeshPickerCore : EDGUIRsrcPicker
{
	EDGUIMeshPickerCore() : m_scene( GR_CreateScene() )
	{
		m_meshinst = m_scene->CreateMeshInstance();
		lmm_prepmeshinst( m_meshinst );
	}
	void _DrawItem( int i, int x0, int y0, int x1, int y1 )
	{
		SGRX_Viewport vp = { x0 + 10, y0 + 4, x1 - 10, y1 - 16 };
		
		if( m_meshes[ i ] )
		{
			SGRX_IMesh* M = m_meshes[ i ];
			m_meshinst->SetMesh( m_meshes[ i ] );
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

struct EDGUIMeshPicker : EDGUIMeshPickerCore, IDirEntryHandler
{
	EDGUIMeshPicker( bool fullpaths = false ) :
		m_fullpaths( fullpaths )
	{
		caption = "Pick a mesh";
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
			if( m_fullpaths )
				m_options.push_back( String_Concat( "meshes/", name ) );
			else
				m_options.push_back( name.part( 0, name.size() - 4 ) );
			m_meshes.push_back( GR_GetMesh( String_Concat( "meshes/", name ) ) );
		}
		return true;
	}
	
	bool m_fullpaths;
};

inline String ED_GetMeshFromChar( const StringView& charpath )
{
	ByteArray ba;
	if( FS_LoadBinaryFile( charpath, ba ) == false )
		return String();
	
	// basic unserialization to retrieve mesh
	ByteReader br( &ba );
	br.marker( "SGRXCHAR" );
	SerializeVersionHelper<ByteReader> arch( br, 2 );
	String mesh;
	arch( mesh );
	return mesh;
}

struct EDGUICharUsePicker : EDGUIMeshPickerCore, IDirEntryHandler
{
	EDGUICharUsePicker( bool fullpaths = false ) :
		m_fullpaths( fullpaths )
	{
		caption = "Pick a char";
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading chars";
		m_options.clear();
		m_meshes.clear();
		FS_IterateDirectory( "chars", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[Ch]: " << name;
		if( !isdir && name.ends_with( ".chr" ) )
		{
			String fullpath = String_Concat( "chars/", name );
			String mesh = ED_GetMeshFromChar( fullpath );
			if( mesh.size() == 0 )
				return true;
			
			if( m_fullpaths )
				m_options.push_back( fullpath );
			else
				m_options.push_back( name.part( 0, name.size() - 4 ) );
			m_meshes.push_back( GR_GetMesh( mesh ) );
		}
		return true;
	}
	
	bool m_fullpaths;
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


struct EDGUIScrItemPicker : EDGUIRsrcPicker
{
	EDGUIScrItemPicker( ScriptContext* sctx ) : m_scriptCtx( sctx )
	{
		caption = "Pick a scripted item";
		Reload();
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	void Reload()
	{
		LOG << "Reloading scripted items";
		m_options.clear();
		m_options.push_back( "" );
		
	//	m_scriptCtx->PushEnv();
	//	if( m_scriptCtx->ExecFile( "data/scritems.sgs" ) )
		{
			ScriptVarIterator iter = m_scriptCtx->GlobalIterator();
			while( iter.Advance() )
			{
				sgsString sgsstr = iter.GetKey().get_string();
				StringView str( sgsstr.c_str(), sgsstr.size() );
				if( str.starts_with( "SCRITEM_CREATE_" ) )
				{
					StringView name = str.part( sizeof("SCRITEM_CREATE_") - 1 );
					LOG << "[SI]: " << name;
					m_options.push_back( name );
				}
			}
		}
	//	m_scriptCtx->PopEnv();
		
		_Search( m_searchString );
	}
	
	ScriptContext* m_scriptCtx;
};


// returns level name or empty string if level is not saved yet
struct EDGUIScrFnPicker : EDGUIRsrcPicker
{
	EDGUIScrFnPicker( ScriptContext* sctx ) : m_scriptCtx( sctx )
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
		
		if( m_levelName.size() == 0 )
		{
			LOG << "SAVE LEVEL BEFORE OPENING SCRIPTS";
			_Search( m_searchString );
			return;
		}
		
		char bfr[ 256 ];
		sprintf( bfr, "levels/%s.sgs", (const char*) StackString< 220 >( m_levelName ) );
		m_scriptCtx->PushEnv();
		if( m_scriptCtx->ExecFile( bfr ) )
		{
			ScriptVarIterator iter = m_scriptCtx->GlobalIterator();
			while( iter.Advance() )
			{
				sgsString str = iter.GetKey().get_string();
				if( str.size() )
					m_options.push_back( StringView( str.c_str(), str.size() ) );
			}
		}
		m_scriptCtx->PopEnv();
		
		_Search( m_searchString );
	}
	
	ScriptContext* m_scriptCtx;
	String m_levelName;
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
// first person scene renderer
//

struct EDGUIRenderView : EDGUIItem, SGRX_DebugDraw
{
	struct FrameInterface
	{
		virtual bool ViewEvent( EDGUIEvent* e ) = 0;
		virtual void DebugDraw() = 0;
	};
	
	EDGUIRenderView( SceneHandle scene, FrameInterface* fiface ) :
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
		crplaneheight( 0 ),
		m_edScene( scene ),
		m_mainframe( fiface )
	{
		type = 100000000;
		tyname = "renderview";
		backColor = COLOR_RGBA( 10, 10, 10, 255 );
		
		Vec3 dir = m_edScene->camera.direction;
		Vec2 dir2 = V2( dir.x, dir.y ).Normalized();
		hangle = atan2( dir2.y, dir2.x );
		vangle = asin( m_edScene->camera.direction.z );
		m_edScene->camera.znear = 0.1f;
	}
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_LAYOUT:
			EDGUIItem::OnEvent( e );
			m_edScene->camera.aspect = ( x1 - x0 ) / (float) ( y1 - y0 );
			EventToFrame( e );
			return 1;
		case EDGUI_EVENT_PAINT:
			{
				SGRX_Viewport vp = { x0, y0, x1, y1 };
				SGRX_RenderScene rsinfo( V4( GetTimeMsec() / 1000.0f ), m_edScene );
				rsinfo.viewport = &vp;
				rsinfo.debugdraw = this;
				GR_RenderScene( rsinfo );
			}
			EventToFrame( e );
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
			if( EventToFrame( e ) )
			{
				bool down = e->type == EDGUI_EVENT_KEYDOWN && !( e->key.engmod & (KMOD_CTRL|KMOD_ALT) );
				if( e->key.engkey == SDLK_w ) movefwd = down;
				if( e->key.engkey == SDLK_s ) movebwd = down;
				if( e->key.engkey == SDLK_a ) movelft = down;
				if( e->key.engkey == SDLK_d ) movergt = down;
				if( e->key.engkey == SDLK_LSHIFT ) movefast = down;
				if( e->key.engkey == SDLK_q ) moveup = down;
				if( e->key.engkey == SDLK_z ) movedn = down;
			}
			else
			{
				movefwd = false;
				movebwd = false;
				movelft = false;
				movergt = false;
				movefast = false;
				moveup = false;
				movedn = false;
			}
			return 1;
		case EDGUI_EVENT_BTNDOWN:
		case EDGUI_EVENT_BTNUP:
			if( EventToFrame( e ) )
			{
				bool down = e->type == EDGUI_EVENT_BTNDOWN;
				if( e->mouse.button == 1 ) look = down;
				prevcp = Game_GetCursorPos();
			}
			else
				look = false;
			break;
		case EDGUI_EVENT_BTNCLICK:
			EventToFrame( e );
			break;
		case EDGUI_EVENT_MOUSEMOVE:
			{
				cursor_aim = false;
				Vec2 cp = { e->mouse.x, e->mouse.y };
				if( m_edScene->camera.GetCursorRay( safe_fdiv( cp.x - x0, x1 - x0 ), safe_fdiv( cp.y - y0, y1 - y0 ), crpos, crdir ) )
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
		default:
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
		Vec3 up = m_edScene->camera.updir;
		Vec3 rgt = Vec3Cross( dir, up ).Normalized();
		m_edScene->camera.direction = dir;
		m_edScene->camera.position += ( dir * ( movefwd - movebwd ) + rgt * ( movergt - movelft ) + up * ( moveup - movedn ) ) * speed;
		
		m_edScene->camera.UpdateMatrices();
	}
	Vec2 CPToNormalized( const Vec2& cp ) const
	{
		return V2( safe_fdiv( cp.x - x0, x1 - x0 ), safe_fdiv( cp.y - y0, y1 - y0 ) );
	}
	Vec2 GetViewSize() const
	{
		return V2( x1 - x0, y1 - y0 );
	}
	bool EventToFrame( EDGUIEvent* e ){ return m_mainframe->ViewEvent( e ); }
	void DebugDraw(){ m_mainframe->DebugDraw(); }
	
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
	
	SceneHandle m_edScene;
	FrameInterface* m_mainframe;
};


