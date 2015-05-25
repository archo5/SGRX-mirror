

#include <engine.hpp>
#include <enganim.hpp>
#include <edgui.hpp>


struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
struct EDGUIMeshPicker* g_UIMeshPicker;
struct EDGUICSOpenPicker* g_UICSOpenPicker;
struct EDGUICSSavePicker* g_UICSSavePicker;



struct EDGUIMeshPicker : EDGUIRsrcPicker
{
	EDGUIMeshPicker() :
		m_scene( GR_CreateScene() )
	{
		m_meshinst = m_scene->CreateMeshInstance();
		m_meshinst->textures[0] = GR_GetTexture( "textures/white.png" );
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading meshes";
		m_options.clear();
		m_meshes.clear();
		DirectoryIterator tdi( "meshes" );
		while( tdi.Next() )
		{
			StringView fn = tdi.Name();
			LOG << fn;
			if( !tdi.IsDirectory() )
			{
				if( fn.ends_with( ".ssm" ) )
				{
					m_options.push_back( fn.part( 0, fn.size() - 4 ) );
					m_meshes.push_back( GR_GetMesh( String_Concat( "meshes/", fn ) ) );
				}
			}
		}
		_Search( m_searchString );
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

struct EDGUICSPicker : EDGUIRsrcPicker
{
	EDGUICSPicker(){ Reload(); }
	void Reload()
	{
		LOG << "Reloading cutscenes";
		m_options.clear();
		DirectoryIterator tdi( "cutscenes" );
		while( tdi.Next() )
		{
			StringView fn = tdi.Name();
			LOG << fn;
			if( !tdi.IsDirectory() && fn.ends_with( ".csc" ) )
			{
				m_options.push_back( fn.part( 0, fn.size() - 4 ) );
			}
		}
		_Search( m_searchString );
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

struct EDGUICSOpenPicker : EDGUICSPicker
{
	EDGUICSOpenPicker()
	{
		caption = "Pick a cutscene to open";
		m_confirm.caption = "Do you really want to open the cutscene? All unsaved changes will be lost!";
	}
	virtual void _OnPickResource()
	{
		m_confirm.Open( this );
		m_frame->Add( &m_confirm );
	}
};

struct EDGUICSSavePicker : EDGUICSPicker
{
	EDGUICSSavePicker()
	{
		caption = "Pick a cutscene to save or write the name";
		m_confirm.caption = "Do you really want to overwrite the cutscene?";
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
				SGRX_RenderScene rsinfo( V4( GetTimeMsec() / 1000.0f ),g_EdScene );
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
			speed *= 5;
		speed *= deltaTime;
		
		Vec2 cp = Game_GetCursorPos();
		if( look )
		{
			Vec2 diff = ( cp - prevcp );
			hangle -= diff.x * 0.01f;
			vangle -= diff.y * 0.01f;
			vangle = clamp( vangle, -M_PI * 0.49f, M_PI * 0.49f );
		}
		prevcp = cp;
		
		float c_hangle = cos( hangle ), s_hangle = sin( hangle ), c_vangle = cos( vangle ), s_vangle = sin( vangle );
		Vec3 dir = { c_hangle * c_vangle, s_hangle * c_vangle, s_vangle };
		Vec3 up = g_EdScene->camera.updir;
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


struct EDGUIMainFrame : EDGUIFrame
{
	EDGUIMainFrame() :
		m_UIMenuSplit( true, 26, 0 ),
		m_UIGraphSplit( true, 0, 0.7f ),
		m_UIParamSplit( false, 0, 0.7f )
	{
		tyname = "mainframe";
		
	//	g_UIGraphEd = &m_UIGraph;
		
		Add( &m_UIMenuSplit );
		m_UIMenuSplit.SetFirstPane( &m_UIMenuButtons );
		m_UIMenuSplit.SetSecondPane( &m_UIGraphSplit );
		m_UIGraphSplit.SetFirstPane( &m_UIParamSplit );
	//	m_UIGraphSplit.SetSecondPane( &m_UIGraph );
		m_UIParamSplit.SetFirstPane( &m_UIRenderView );
		m_UIParamSplit.SetSecondPane( &m_UIParamList );
		
		// menu
		m_MB_Cat0.caption = "File:";
		m_MBNew.caption = "New";
		m_MBOpen.caption = "Open";
		m_MBSave.caption = "Save";
		m_MBSaveAs.caption = "Save As";
		m_MB_Cat1.caption = "Playback:";
		m_MBPlay.caption = "Play";
		m_MBStop.caption = "Stop";
		m_MBTrigger.caption = "Trigger";
		m_MB_Cat2.caption = "Edit:";
		m_MBEditSystem.caption = "System";
		m_UIMenuButtons.Add( &m_MB_Cat0 );
		m_UIMenuButtons.Add( &m_MBNew );
		m_UIMenuButtons.Add( &m_MBOpen );
		m_UIMenuButtons.Add( &m_MBSave );
		m_UIMenuButtons.Add( &m_MBSaveAs );
		m_UIMenuButtons.Add( &m_MB_Cat1 );
		m_UIMenuButtons.Add( &m_MBPlay );
		m_UIMenuButtons.Add( &m_MBStop );
		m_UIMenuButtons.Add( &m_MBTrigger );
		m_UIMenuButtons.Add( &m_MB_Cat2 );
		m_UIMenuButtons.Add( &m_MBEditSystem );
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if(0);
			
			else if( e->target == &m_MBNew ) CS_New();
			else if( e->target == &m_MBOpen ) CS_Open();
			else if( e->target == &m_MBSave ) CS_Save();
			else if( e->target == &m_MBSaveAs ) CS_SaveAs();
			
		//	else if( e->target == &m_MBPlay ) g_EdPS->Play();
		//	else if( e->target == &m_MBStop ) g_EdPS->Stop();
		//	else if( e->target == &m_MBTrigger ) g_EdPS->Trigger();
			
		//	else if( e->target == &m_MBEditSystem )
		//	{
		//		ClearParamList();
		//		AddToParamList( g_EdPS->GetSystem() );
		//	}
			
			return 1;
			
		case EDGUI_EVENT_PROPCHANGE:
			if( e->target == g_UICSOpenPicker )
			{
				CS_Real_Open( g_UICSOpenPicker->GetValue() );
			}
			if( e->target == g_UICSSavePicker )
			{
				CS_Real_Save( g_UICSSavePicker->GetValue() );
			}
			return 1;
		}
		return EDGUIFrame::OnEvent( e );
	}
	
	void ViewEvent( EDGUIEvent* e )
	{
	}
	
	void DebugDraw()
	{
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
	}
	void CS_New()
	{
		m_fileName = "";
	//	g_EdPS->Reset();
		ResetEditorState();
	}
	void CS_Open()
	{
		g_UICSOpenPicker->Reload();
		g_UICSOpenPicker->Open( this, "" );
		m_frame->Add( g_UICSOpenPicker );
	}
	void CS_Save()
	{
		if( m_fileName.size() )
		{
			CS_Real_Save( m_fileName );
		}
		else
		{
			CS_SaveAs();
		}
	}
	void CS_SaveAs()
	{
		g_UICSSavePicker->Reload();
		g_UICSSavePicker->Open( this, "" );
		m_frame->Add( g_UICSSavePicker );
	}
	void CS_Real_Open( const String& str )
	{
		LOG << "Trying to open cutscene: " << str;
		
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "cutscenes/%.*s.csc", TMIN( (int) str.size(), 200 ), str.data() );
		ByteArray data;
		if( !FS_LoadBinaryFile( bfr, data ) )
		{
			LOG_ERROR << "FAILED TO LOAD CSC FILE: " << bfr;
			return;
		}
		
		ResetEditorState();
		
		ByteReader br( &data );
	//	g_EdPS->Serialize( br );
		if( br.error )
		{
			LOG_ERROR << "FAILED TO READ CSC FILE (at " << (int) br.pos << "): " << bfr;
			return;
		}
		
		m_fileName = str;
	//	g_EdPS->Play();
	}
	void CS_Real_Save( const String& str )
	{
		LOG << "Trying to save cutscene: " << str;
		ByteArray data;
		ByteWriter bw( &data );
		
	//	bw << *g_EdPS;
		
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "cutscenes/%.*s.csc", TMIN( (int) str.size(), 200 ), str.data() );
		if( !SaveBinaryFile( bfr, data.data(), data.size() ) )
		{
			LOG_ERROR << "FAILED TO SAVE CSC FILE: " << bfr;
			return;
		}
		
		m_fileName = str;
	}
	
	String m_fileName;
	
	// core layout
	EDGUILayoutSplitPane m_UIMenuSplit;
	EDGUILayoutSplitPane m_UIGraphSplit;
	EDGUILayoutSplitPane m_UIParamSplit;
	EDGUILayoutColumn m_UIMenuButtons;
	EDGUILayoutRow m_UIParamList;
	EDGUIRenderView m_UIRenderView;
//	EdDualGraph m_UIGraph;

	// menu
	EDGUILabel m_MB_Cat0;
	EDGUIButton m_MBNew;
	EDGUIButton m_MBOpen;
	EDGUIButton m_MBSave;
	EDGUIButton m_MBSaveAs;
	EDGUILabel m_MB_Cat1;
	EDGUIButton m_MBPlay;
	EDGUIButton m_MBStop;
	EDGUIButton m_MBTrigger;
	EDGUILabel m_MB_Cat2;
	EDGUIButton m_MBEditSystem;
};

void EDGUIRenderView::EventToFrame( EDGUIEvent* e )
{
	g_UIFrame->ViewEvent( e );
}

void EDGUIRenderView::DebugDraw()
{
	g_UIFrame->DebugDraw();
}




SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
};

struct CSEditor : IGame
{
	bool OnInitialize()
	{
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
		GR2D_SetFont( "fonts/lato-regular.ttf", 12 );
		
		g_UIMeshPicker = new EDGUIMeshPicker;
		g_UICSOpenPicker = new EDGUICSOpenPicker;
		g_UICSSavePicker = new EDGUICSSavePicker;
		
		// core layout
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = V3(3);
		g_EdScene->camera.UpdateMatrices();
		g_UIFrame = new EDGUIMainFrame();
		g_UIFrame->Resize( GR_GetWidth(), GR_GetHeight() );
		
		// param area
	//	g_UIFrame->AddToParamList( g_EdPS->GetSystem() );
		
		return true;
	}
	void OnDestroy()
	{
		delete g_UICSSavePicker;
		g_UICSSavePicker = NULL;
		delete g_UICSOpenPicker;
		g_UICSOpenPicker = NULL;
		delete g_UIMeshPicker;
		g_UIMeshPicker = NULL;
		delete g_UIFrame;
		g_UIFrame = NULL;
		g_EdScene = NULL;
	}
	void OnEvent( const Event& e )
	{
		g_UIFrame->EngineEvent( &e );
	}
	void OnTick( float dt, uint32_t gametime )
	{
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		g_UIFrame->m_UIRenderView.UpdateCamera( dt );
	//	g_EdPS->Tick( dt );
		g_UIFrame->Draw();
	}
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

