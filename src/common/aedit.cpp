

#include <engine.hpp>
#include <enganim.hpp>
#include <edgui.hpp>
#include "edcomui.hpp"

#include "assetcomp.hpp"


#define ASSET_SCRIPT_NAME "assets.txt"


struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
SGRX_AssetScript* g_EdAS;
struct EDGUIImageFilterType* g_UIImgFilterType;
struct EDGUITextureOutputFormat* g_UITexOutFmt;
struct EDGUICategoryPicker* g_UICategoryPicker;
struct EDGUIAssetPathPicker* g_UIAssetPathPicker;

EDGUIRsrcPicker TMPRSRC;



struct EDGUIImageFilterType : EDGUIRsrcPicker
{
	EDGUIImageFilterType()
	{
		caption = "Pick an image filter";
		m_options.push_back( "Resize" );
		m_options.push_back( "Sharpen" );
		m_options.push_back( "To linear" );
		m_options.push_back( "From linear" );
		_Search( m_searchString );
	}
	SGRX_AssetImageFilterType GetPickedType() const
	{
		return SGRX_AssetImageFilterType( m_picked + 1 );
	}
	void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
};

struct EDGUITextureOutputFormat : EDGUIRsrcPicker
{
	EDGUITextureOutputFormat()
	{
		caption = "Pick an texture output format";
		for( int i = SGRX_TOF_Unknown + 1; i < SGRX_TOF__COUNT; ++i )
		{
			m_options.push_back( SGRX_TextureOutputFormat_ToString(
				(SGRX_TextureOutputFormat) i ) );
		}
		_Search( m_searchString );
	}
	SGRX_TextureOutputFormat GetPickedType() const
	{
		return SGRX_TextureOutputFormat( m_picked + 1 );
	}
	void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
};

struct EDGUICategoryPicker : EDGUIRsrcPicker
{
	EDGUICategoryPicker()
	{
		Reload();
	}
	void Reload()
	{
		m_options.clear();
		if( g_EdAS )
		{
			for( size_t i = 0; i < g_EdAS->categories.size(); ++i )
			{
				m_options.push_back( g_EdAS->categories.item( i ).key );
			}
		}
		_Search( m_searchString );
	}
	void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
};

struct EDGUIAssetPathPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUIAssetPathPicker() : m_depth(0)
	{
		Reload();
	}
	void AddOptionsFromDir( const StringView& path )
	{
		if( m_depth > 32 )
			return;
		m_depth++;
		FS_IterateDirectory( path, this );
		m_depth--;
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		if( name == "." || name == ".." )
			return true;
		String path = loc;
		path.append( "/" );
		path.append( name );
		if( isdir )
		{
			AddOptionsFromDir( path );
		}
		else
		{
			m_options.push_back( path );
		}
		return true;
	}
	void Reload()
	{
		m_options.clear();
		m_depth = 0;
		AddOptionsFromDir( "assets" );
		_Search( m_searchString );
	}
	void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemWidth = 300;
		m_itemHeight = 16;
	}
	int m_depth;
};

struct EDGUICreatePickButton : EDGUIPropRsrc
{
	EDGUICreatePickButton( EDGUIRsrcPicker* rsrcPicker, const StringView& def = StringView() ) :
		EDGUIPropRsrc( rsrcPicker, def )
	{
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
		case EDGUI_EVENT_PROPCHANGE:
			if( e->target == m_rsrcPicker )
			{
				EDGUIEvent se = { e->type, this };
				BubblingEvent( &se );
				return 0;
			}
			break;
		}
		return EDGUIPropRsrc::OnEvent( e );
	}
};


struct EDGUIImgFilter_Resize : EDGUILayoutRow
{
	EDGUIImgFilter_Resize( SGRX_ImageFilter* iflt ) :
		m_width( 256, 1, 4096 ),
		m_height( 256, 1, 4096 ),
		m_hfilter( iflt )
	{
		SGRX_ImageFilter_Resize* F = iflt->upcast<SGRX_ImageFilter_Resize>();
		m_width.SetValue( F->width );
		m_height.SetValue( F->height );
		
		m_width.caption = "Width";
		m_height.caption = "Height";
		
		Add( &m_width );
		Add( &m_height );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_hfilter )
		{
			SGRX_ImageFilter_Resize* F = m_hfilter->upcast<SGRX_ImageFilter_Resize>();
			switch( e->type )
			{
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_width ) F->width = m_width.m_value;
				if( e->target == &m_height ) F->height = m_height.m_value;
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIPropInt m_width;
	EDGUIPropInt m_height;
	SGRX_ImgFilterHandle m_hfilter;
};

struct EDGUIImgFilter_Sharpen : EDGUILayoutRow
{
	EDGUIImgFilter_Sharpen( SGRX_ImageFilter* iflt ) :
		m_factor( 1, 2, 0, 100 ),
		m_hfilter( iflt )
	{
		SGRX_ImageFilter_Sharpen* F = iflt->upcast<SGRX_ImageFilter_Sharpen>();
		m_factor.SetValue( F->factor );
		
		m_factor.caption = "Factor";
		
		Add( &m_factor );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_hfilter )
		{
			SGRX_ImageFilter_Sharpen* F = m_hfilter->upcast<SGRX_ImageFilter_Sharpen>();
			switch( e->type )
			{
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_factor ) F->factor = m_factor.m_value;
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIPropFloat m_factor;
	SGRX_ImgFilterHandle m_hfilter;
};

struct EDGUIImgFilter_PropertyLess : EDGUILayoutRow
{
};


struct EDGUIAssetTexture : EDGUILayoutRow
{
	EDGUIAssetTexture() :
		m_group( true, "Texture" ),
		m_sourceFile( g_UIAssetPathPicker ),
		m_outputCategory( g_UICategoryPicker ),
		m_outputType( g_UITexOutFmt ),
		m_sfgroup( true, "Selected filter" ),
		m_curFilter( NULL ),
		m_flgroup( true, "Filters" ),
		m_filterBtnAdd( g_UIImgFilterType ),
		m_tid( NOT_FOUND )
	{
		m_outputCategory.m_requestReload = true;
		
		m_sourceFile.caption = "Source file";
		m_outputCategory.caption = "Output category";
		m_outputName.caption = "Output name";
		m_outputType.caption = "Output type";
		m_isSRGB.caption = "Is SRGB?";
		
		m_group.Add( &m_sourceFile );
		m_group.Add( &m_outputCategory );
		m_group.Add( &m_outputName );
		m_group.Add( &m_outputType );
		m_group.Add( &m_isSRGB );
		
		m_filterBtnAdd.SetValue( "Pick filter to add" );
		
		m_filterButtons.Add( &m_filterEditButton );
		m_flgroup.Add( &m_filterBtnAdd );
		m_flgroup.Add( &m_filterButtons );
		
		Add( &m_group );
		Add( &m_columnList );
		m_columnList.Add( &m_sfgroup );
		m_columnList.Add( &m_flgroup );
	}
	
	~EDGUIAssetTexture()
	{
		m_sfgroup.Clear();
		if( m_curFilter )
		{
			delete m_curFilter;
			m_curFilter = NULL;
		}
	}
	
	void ReloadFilterList()
	{
		SGRX_TextureAsset& TA = g_EdAS->textureAssets[ m_tid ];
		
		m_filterButtons.m_options.resize( TA.filters.size() );
		for( size_t i = 0; i < TA.filters.size(); ++i )
		{
			m_filterButtons.m_options[ i ] = TA.filters[ i ]->GetName();
		}
		m_filterButtons.UpdateOptions();
	}
	
	void Prepare( size_t tid )
	{
		m_tid = tid;
		SGRX_TextureAsset& TA = g_EdAS->textureAssets[ tid ];
		
		m_sourceFile.SetValue( TA.sourceFile );
		m_outputCategory.SetValue( TA.outputCategory );
		m_outputName.SetValue( TA.outputName );
		m_outputType.SetValue( SGRX_TextureOutputFormat_ToString( TA.outputType ) );
		m_isSRGB.SetValue( TA.isSRGB );
		
		m_sfgroup.Clear();
		if( m_curFilter )
		{
			delete m_curFilter;
			m_curFilter = NULL;
		}
		
		ReloadFilterList();
	}
	
	void EditFilter( SGRX_ImageFilter* IF )
	{
		EDGUILayoutRow* newflt = NULL;
		switch( IF->GetType() )
		{
		case SGRX_AIF_Resize: newflt = new EDGUIImgFilter_Resize( IF ); break;
		case SGRX_AIF_Sharpen: newflt = new EDGUIImgFilter_Sharpen( IF ); break;
		case SGRX_AIF_ToLinear: newflt = new EDGUIImgFilter_PropertyLess(); break;
		case SGRX_AIF_FromLinear: newflt = new EDGUIImgFilter_PropertyLess(); break;
		default: break;
		}
		if( newflt )
		{
			if( m_curFilter )
				delete m_curFilter;
			m_curFilter = newflt;
			m_sfgroup.Clear();
			m_sfgroup.Add( m_curFilter );
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "Selected filter: %s",
				SGRX_AssetImgFilterType_ToString( IF->GetType() ) );
			m_sfgroup.caption = bfr;
			m_sfgroup.SetOpen( true );
		}
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_tid != NOT_FOUND )
		{
			SGRX_TextureAsset& TA = g_EdAS->textureAssets[ m_tid ];
			switch( e->type )
			{
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_sourceFile ){ TA.sourceFile = m_sourceFile.m_value; }
				if( e->target == &m_outputCategory ){ TA.outputCategory = m_outputCategory.m_value; }
				if( e->target == &m_outputName ){ TA.outputName = m_outputName.m_value; }
				if( e->target == &m_outputType )
				{
					TA.outputType = SGRX_TextureOutputFormat_FromString( m_outputType.m_value );
				}
				if( e->target == &m_isSRGB ){ TA.isSRGB = m_isSRGB.m_value; }
				if( e->target == &m_filterBtnAdd )
				{
					SGRX_ImageFilter* IF = NULL;
					switch( g_UIImgFilterType->GetPickedType() )
					{
					case SGRX_AIF_Resize: IF = new SGRX_ImageFilter_Resize; break;
					case SGRX_AIF_Sharpen: IF = new SGRX_ImageFilter_Sharpen; break;
					case SGRX_AIF_ToLinear: IF = new SGRX_ImageFilter_Linear( false ); break;
					case SGRX_AIF_FromLinear: IF = new SGRX_ImageFilter_Linear( true ); break;
					default: break;
					}
					if( IF )
					{
						TA.filters.push_back( IF );
						ReloadFilterList();
						EditFilter( IF );
						return 1;
					}
				}
				break;
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_filterEditButton )
				{
					EditFilter( TA.filters[ m_filterEditButton.id2 ] );
					m_frame->UpdateMouse();
					return 1;
				}
				if( e->target == &m_filterEditButton.m_up )
				{
					size_t i = m_filterEditButton.id2;
					if( i > 0 )
						TSWAP( TA.filters[ i ], TA.filters[ i - 1 ] );
					ReloadFilterList();
					m_frame->UpdateMouse();
					return 1;
				}
				if( e->target == &m_filterEditButton.m_dn )
				{
					size_t i = m_filterEditButton.id2;
					if( i < TA.filters.size() - 1 )
						TSWAP( TA.filters[ i ], TA.filters[ i + 1 ] );
					ReloadFilterList();
					m_frame->UpdateMouse();
					return 1;
				}
				if( e->target == &m_filterEditButton.m_del )
				{
					TA.filters.erase( m_filterEditButton.id2 );
					ReloadFilterList();
					m_frame->UpdateMouse();
					return 1;
				}
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIPropRsrc m_sourceFile;
	EDGUIPropRsrc m_outputCategory;
	EDGUIPropString m_outputName;
	EDGUIPropRsrc m_outputType;
	EDGUIPropBool m_isSRGB;
	EDGUILayoutColumn m_columnList;
	EDGUIGroup m_sfgroup;
	EDGUILayoutRow* m_curFilter;
	EDGUIGroup m_flgroup;
	EDGUICreatePickButton m_filterBtnAdd;
	EDGUIBtnList m_filterButtons;
	EDGUIListItemButton m_filterEditButton;
	size_t m_tid;
};

void FC_EditTexture( size_t id );
void FC_EditTextureList();

struct EDGUIAssetTextureList : EDGUILayoutRow
{
	EDGUIAssetTextureList() :
		m_group( true, "Texture assets" ),
		m_editButton( false )
	{
		m_btnAdd.caption = "Add texture";
		
		m_buttons.Add( &m_editButton );
		m_group.Add( &m_buttons );
		Add( &m_btnAdd );
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_buttons.m_options.resize( g_EdAS->textureAssets.size() );
		for( size_t i = 0; i < g_EdAS->textureAssets.size(); ++i )
		{
			g_EdAS->textureAssets[ i ].GetDesc( m_buttons.m_options[ i ] );
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
				SGRX_TextureAsset texasset;
				texasset.outputType = SGRX_TOF_PNG_RGBA32;
				texasset.isSRGB = true;
				g_EdAS->textureAssets.push_back( texasset );
				FC_EditTexture( g_EdAS->textureAssets.size() - 1 );
				return 1;
			}
			if( e->target == &m_editButton )
			{
				FC_EditTexture( m_editButton.id2 );
				return 1;
			}
			if( e->target == &m_editButton.m_del )
			{
				g_EdAS->textureAssets.erase( m_editButton.id2 );
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

struct EDGUIAssetMesh : EDGUILayoutRow
{
	EDGUIAssetMesh() :
		m_group( true, "Mesh" ),
		m_sourceFile( g_UIAssetPathPicker ),
		m_outputCategory( g_UICategoryPicker ),
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

void FC_EditCategory( const StringView& name );
void FC_EditScript();

struct EDGUIAssetCategoryForm : EDGUILayoutRow
{
	EDGUIAssetCategoryForm() :
		m_group( true, "Edit category" )
	{
		m_name.caption = "Name";
		m_path.caption = "Path";
		
		m_group.Add( &m_name );
		m_group.Add( &m_path );
		m_group.Add( &m_btnSave );
		
		Add( &m_group );
		
		_UpdateButtonText();
	}
	
	void _UpdateButtonText()
	{
		String* path = g_EdAS->categories.getptr( m_name.m_value );
		if( StringView(m_name.m_value) == "" )
		{
			m_btnSave.caption = "-- cannot save unnamed category --";
		}
		else if( m_name.m_value == m_origName )
		{
			m_btnSave.caption = "Save category";
		}
		else if( path )
		{
			m_btnSave.caption = String_Concat( "Overwrite category path: ", *path );
		}
		else if( m_origName.size() == 0 )
		{
			m_btnSave.caption = "Save category";
		}
		else
		{
			m_btnSave.caption = "Save and rename category";
		}
	}
	
	void Prepare( const StringView& name )
	{
		m_origName = name;
		m_name.SetValue( name );
		String* path = g_EdAS->categories.getptr( name );
		m_path.SetValue( path ? *path : "" );
		_UpdateButtonText();
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_name )
			{
				_UpdateButtonText();
			}
			break;
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btnSave && m_name.m_value.size() )
			{
				g_EdAS->categories.unset( m_origName );
				g_EdAS->categories.set( m_name.m_value, m_path.m_value );
				FC_EditScript();
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	String m_origName;
	EDGUIGroup m_group;
	EDGUIPropString m_name;
	EDGUIPropString m_path;
	EDGUIButton m_btnSave;
};

struct EDGUIAssetScript : EDGUILayoutRow
{
	EDGUIAssetScript() :
		m_cgroup( true, "Categories" ),
		m_ctgEditButton( false )
	{
		m_ctgBtnAdd.caption = "Add category";
		
		m_ctgButtons.Add( &m_ctgEditButton );
		
		m_cgroup.Add( &m_ctgBtnAdd );
		m_cgroup.Add( &m_ctgButtons );
		
		Add( &m_cgroup );
	}
	
	void Prepare()
	{
		m_ctgButtons.m_options.resize( g_EdAS->categories.size() );
		for( size_t i = 0; i < g_EdAS->categories.size(); ++i )
		{
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "%s => %s",
				StackString<100>( g_EdAS->categories.item( i ).key ).str,
				StackString<100>( g_EdAS->categories.item( i ).value ).str );
			m_ctgButtons.m_options[ i ] = bfr;
		}
		m_ctgButtons.UpdateOptions();
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_ctgBtnAdd )
			{
				FC_EditCategory( "" );
			}
			if( e->target == &m_ctgEditButton )
			{
				FC_EditCategory( g_EdAS->categories.item( m_ctgEditButton.id2 ).key );
			}
			if( e->target == &m_ctgEditButton.m_del )
			{
				g_EdAS->categories.unset( g_EdAS->categories.item( m_ctgEditButton.id2 ).key );
				Prepare();
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_cgroup;
	EDGUIButton m_ctgBtnAdd;
	EDGUIBtnList m_ctgButtons;
	EDGUIListItemButton m_ctgEditButton;
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
		m_MBRun.caption = "Run";
		m_MB_Cat1.caption = "Edit:";
		m_MBEditScript.caption = "Script";
		m_MBEditTextures.caption = "Textures";
		m_MBEditMeshes.caption = "Meshes";
		m_UIMenuButtons.Add( &m_MB_Cat0 );
		m_UIMenuButtons.Add( &m_MBSave );
		m_UIMenuButtons.Add( &m_MBRun );
		m_UIMenuButtons.Add( &m_MB_Cat1 );
		m_UIMenuButtons.Add( &m_MBEditScript );
		m_UIMenuButtons.Add( &m_MBEditTextures );
		m_UIMenuButtons.Add( &m_MBEditMeshes );
		
		ASCR_Open();
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if(0);
			
			else if( e->target == &m_MBSave ) ASCR_Save();
			else if( e->target == &m_MBRun ) ASCR_Run();
			
			else if( e->target == &m_MBEditScript )
			{
				EditScript();
			}
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
	
	void EditTexture( size_t id )
	{
		ClearParamList();
		m_UITexture.Prepare( id );
		AddToParamList( &m_UITexture );
	}
	void EditTextureList()
	{
		ClearParamList();
		m_UITextureList.Prepare();
		AddToParamList( &m_UITextureList );
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
	void EditCategory( const StringView& name )
	{
		ClearParamList();
		m_UICategory.Prepare( name );
		AddToParamList( &m_UICategory );
	}
	void EditScript()
	{
		ClearParamList();
		m_UIAssetScript.Prepare();
		AddToParamList( &m_UIAssetScript );
	}
	
	void ResetEditorState()
	{
		EditScript();
	}
	void ASCR_Open()
	{
		LOG << "Trying to open asset script";
		
		if( g_EdAS->Load( ASSET_SCRIPT_NAME ) == false )
		{
			LOG_ERROR << "FAILED TO LOAD ASSET SCRIPT";
			return;
		}
		
		ResetEditorState();
	}
	void ASCR_Save()
	{
		LOG << "Trying to save asset script";
		
		if( g_EdAS->Save( ASSET_SCRIPT_NAME ) )
		{
			LOG_ERROR << "FAILED TO SAVE ASSET SCRIPT";
			return;
		}
	}
	void ASCR_Run()
	{
		SGRX_ProcessAssets( *g_EdAS );
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
	EDGUIButton m_MBRun;
	EDGUILabel m_MB_Cat1;
	EDGUIButton m_MBEditScript;
	EDGUIButton m_MBEditTextures;
	EDGUIButton m_MBEditMeshes;
	
	// data edit views
	EDGUIAssetTexture m_UITexture;
	EDGUIAssetTextureList m_UITextureList;
	EDGUIAssetMesh m_UIMesh;
	EDGUIAssetMeshList m_UIMeshList;
	EDGUIAssetCategoryForm m_UICategory;
	EDGUIAssetScript m_UIAssetScript;
};

void FC_EditTexture( size_t id ){ g_UIFrame->EditTexture( id ); }
void FC_EditTextureList(){ g_UIFrame->EditTextureList(); }
void FC_EditMesh( size_t id ){ g_UIFrame->EditMesh( id ); }
void FC_EditMeshList(){ g_UIFrame->EditMeshList(); }
void FC_EditCategory( const StringView& name ){ g_UIFrame->EditCategory( name ); }
void FC_EditScript(){ g_UIFrame->EditScript(); }



struct ASEditor : IGame
{
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
	//	g_UITexPicker = new EDGUISDTexPicker( "textures/particles" );
	//	g_UIShaderPicker = new EDGUIShaderPicker;
		g_UIAssetPathPicker = new EDGUIAssetPathPicker;
		g_UICategoryPicker = new EDGUICategoryPicker;
		g_UITexOutFmt = new EDGUITextureOutputFormat;
		g_UIImgFilterType = new EDGUIImageFilterType;
		
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
		delete g_UIImgFilterType;
		g_UIImgFilterType = NULL;
		delete g_UITexOutFmt;
		g_UITexOutFmt = NULL;
		delete g_UICategoryPicker;
		g_UICategoryPicker = NULL;
		delete g_UIAssetPathPicker;
		g_UIAssetPathPicker = NULL;
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

