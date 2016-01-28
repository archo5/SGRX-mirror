

#include <engine.hpp>
#include <enganim.hpp>
#include <edgui.hpp>
#include "edcomui.hpp"


struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
struct EDGUIMeshPicker* g_UIMeshPicker;
struct EDGUICSOpenPicker* g_UICSOpenPicker;
struct EDGUICSSavePicker* g_UICSSavePicker;



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



struct EDGUIMainFrame : EDGUIFrame, EDGUIRenderView::FrameInterface
{
	EDGUIMainFrame() :
		m_UIMenuSplit( true, 26, 0 ),
		m_UIGraphSplit( true, 0, 0.7f ),
		m_UIParamSplit( false, 0, 0.7f ),
		m_UIRenderView( g_EdScene, this )
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
	
	bool ViewEvent( EDGUIEvent* e )
	{
		return true;
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
		
		ByteReader br( data );
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




struct CSEditor : IGame
{
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
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

