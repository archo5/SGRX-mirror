

#include <engine.hpp>
#include <enganim.hpp>
#include <edgui.hpp>
#include "edcomui.hpp"

#include "assetcomp.hpp"


#define ASSET_SCRIPT_NAME "assets.txt"


struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
SGRX_AssetScript* g_EdAS;

EDGUIRsrcPicker TMPRSRC;


struct EDGUIAssetTexture : EDGUILayoutRow
{
};

struct EDGUIAssetTextureList : EDGUILayoutRow
{
	EDGUIAssetTextureList() :
		m_group( true, "Texture assets" ),
		m_editButton( false )
	{
		m_btnAdd.caption = "Add texture";
		
		Add( &m_btnAdd );
		Add( &m_group );
	}
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIBtnList m_layerButtons;
	EDGUIListItemButton m_editButton;
};

struct EDGUIAssetMesh : EDGUILayoutRow
{
	EDGUIAssetMesh() :
		m_group( true, "Mesh" ),
		m_sourceFile( &TMPRSRC ),
		m_outputCategory( &TMPRSRC ),
		m_mid( NOT_FOUND )
	{
		m_sourceFile.caption = "Source file";
		m_outputCategory.caption = "Output category";
		m_outputName.caption = "Output name";
		
		m_group.Add( &m_sourceFile );
		m_group.Add( &m_outputCategory );
		m_group.Add( &m_outputName );
		
		Add( &m_group );
	}
	
	void Prepare( size_t mid )
	{
		m_mid = mid;
		SGRX_MeshAsset& MA = g_EdAS->meshAssets[ mid ];
		
		m_sourceFile.SetValue( MA.sourceFile );
		m_outputCategory.SetValue( MA.outputCategory );
		m_outputName.SetValue( MA.outputName );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_mid != NOT_FOUND )
		{
			SGRX_MeshAsset& MA = g_EdAS->meshAssets[ m_mid ];
			switch( e->type )
			{
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_sourceFile ){ MA.sourceFile = m_sourceFile.m_value; }
				if( e->target == &m_outputCategory ){ MA.outputCategory = m_outputCategory.m_value; }
				if( e->target == &m_outputName ){ MA.outputName = m_outputName.m_value; }
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIPropRsrc m_sourceFile;
	EDGUIPropRsrc m_outputCategory;
	EDGUIPropString m_outputName;
	size_t m_mid;
};

void FC_EditMesh( size_t id );
void FC_EditMeshList();

struct EDGUIAssetMeshList : EDGUILayoutRow
{
	EDGUIAssetMeshList() :
		m_group( true, "Mesh assets" ),
		m_editButton( false )
	{
		m_btnAdd.caption = "Add mesh";
		
		m_buttons.Add( &m_editButton );
		m_group.Add( &m_buttons );
		Add( &m_btnAdd );
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_buttons.m_options.resize( g_EdAS->meshAssets.size() );
		for( size_t i = 0; i < g_EdAS->meshAssets.size(); ++i )
		{
			g_EdAS->meshAssets[ i ].GetDesc( m_buttons.m_options[ i ] );
		}
		m_buttons.UpdateOptions();
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btnAdd )
			{
				g_EdAS->meshAssets.push_back( SGRX_MeshAsset() );
				FC_EditMesh( g_EdAS->meshAssets.size() - 1 );
				return 1;
			}
			if( e->target == &m_editButton )
			{
				FC_EditMesh( m_editButton.id2 );
				return 1;
			}
			if( e->target == &m_editButton.m_del )
			{
				g_EdAS->meshAssets.erase( m_editButton.id2 );
				Prepare();
				return 1;
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIBtnList m_buttons;
	EDGUIListItemButton m_editButton;
};


struct EDGUIMainFrame : EDGUIFrame, EDGUIRenderView::FrameInterface
{
	EDGUIMainFrame() :
		m_UIMenuSplit( true, 26, 0 ),
		m_UIParamSplit( false, 0, 0.55f ),
		m_UIRenderView( g_EdScene, this )
	{
		tyname = "mainframe";
		
		Add( &m_UIMenuSplit );
		m_UIMenuSplit.SetFirstPane( &m_UIMenuButtons );
		m_UIMenuSplit.SetSecondPane( &m_UIParamSplit );
		m_UIParamSplit.SetFirstPane( &m_UIRenderView );
		m_UIParamSplit.SetSecondPane( &m_UIParamScroll );
		m_UIParamScroll.Add( &m_UIParamList );
		
		// menu
		m_MB_Cat0.caption = "File:";
		m_MBSave.caption = "Save";
		m_MB_Cat1.caption = "Edit:";
		m_MBEditTextures.caption = "Textures";
		m_MBEditMeshes.caption = "Meshes";
		m_UIMenuButtons.Add( &m_MB_Cat0 );
		m_UIMenuButtons.Add( &m_MBSave );
		m_UIMenuButtons.Add( &m_MB_Cat1 );
		m_UIMenuButtons.Add( &m_MBEditTextures );
		m_UIMenuButtons.Add( &m_MBEditMeshes );
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if(0);
			
			else if( e->target == &m_MBSave ) PS_Save();
			
			else if( e->target == &m_MBEditTextures )
			{
				EditTextureList();
			}
			else if( e->target == &m_MBEditMeshes )
			{
				EditMeshList();
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
	
	void EditTextureList()
	{
	}
	void EditMesh( size_t id )
	{
		ClearParamList();
		m_UIMesh.Prepare( id );
		AddToParamList( &m_UIMesh );
	}
	void EditMeshList()
	{
		ClearParamList();
		m_UIMeshList.Prepare();
		AddToParamList( &m_UIMeshList );
	}
	
	void ResetEditorState()
	{
		ClearParamList();
	}
	void PS_Open()
	{
		LOG << "Trying to open asset script";
		
		if( g_EdAS->Load( ASSET_SCRIPT_NAME ) == false )
		{
			LOG_ERROR << "FAILED TO LOAD ASSET SCRIPT";
			return;
		}
		
		ResetEditorState();
	}
	void PS_Save()
	{
		LOG << "Trying to save asset script";
		
		if( g_EdAS->Save( ASSET_SCRIPT_NAME ) )
		{
			LOG_ERROR << "FAILED TO SAVE ASSET SCRIPT";
			return;
		}
	}
	
	// core layout
	EDGUILayoutSplitPane m_UIMenuSplit;
	EDGUILayoutSplitPane m_UIParamSplit;
	EDGUILayoutColumn m_UIMenuButtons;
	EDGUIVScroll m_UIParamScroll;
	EDGUILayoutRow m_UIParamList;
	EDGUIRenderView m_UIRenderView;

	// menu
	EDGUILabel m_MB_Cat0;
	EDGUIButton m_MBSave;
	EDGUILabel m_MB_Cat1;
	EDGUIButton m_MBEditTextures;
	EDGUIButton m_MBEditMeshes;
	
	// data edit views
	EDGUIAssetMesh m_UIMesh;
	EDGUIAssetMeshList m_UIMeshList;
};

void FC_EditMesh( size_t id ){ g_UIFrame->EditMesh( id ); }
void FC_EditMeshList(){ g_UIFrame->EditMeshList(); }



struct PSEditor : IGame
{
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
	//	g_UITexPicker = new EDGUISDTexPicker( "textures/particles" );
	//	g_UIShaderPicker = new EDGUIShaderPicker;
	//	g_UIMeshPicker = new EDGUIMeshPicker;
	//	g_UIPSOpenPicker = new EDGUIPSOpenPicker;
	//	g_UIPSSavePicker = new EDGUIPSSavePicker;
	//	g_UIPSISFXPicker = new EDGUIPSISFXPicker;
		
		// core layout
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = V3(3);
		g_EdScene->camera.UpdateMatrices();
		g_EdAS = new SGRX_AssetScript;
		g_UIFrame = new EDGUIMainFrame();
		g_UIFrame->Resize( GR_GetWidth(), GR_GetHeight() );
		
		return true;
	}
	void OnDestroy()
	{
	//	delete g_UIPSISFXPicker;
	//	g_UIPSISFXPicker = NULL;
	//	delete g_UIPSSavePicker;
	//	g_UIPSSavePicker = NULL;
	//	delete g_UIPSOpenPicker;
	//	g_UIPSOpenPicker = NULL;
	//	delete g_UIMeshPicker;
	//	g_UIMeshPicker = NULL;
	//	delete g_UIShaderPicker;
	//	g_UIShaderPicker = NULL;
	//	delete g_UITexPicker;
	//	g_UITexPicker = NULL;
		delete g_UIFrame;
		g_UIFrame = NULL;
		delete g_EdAS;
		g_EdAS = NULL;
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
		g_UIFrame->Draw();
	}
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

