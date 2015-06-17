

#include <engine.hpp>
#include <enganim.hpp>
#include <edgui.hpp>
#include "edcomui.hpp"


struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
struct EDGUIMeshPicker* g_UIMeshPicker;
struct EDGUICharOpenPicker* g_UICharOpenPicker;
struct EDGUICharSavePicker* g_UICharSavePicker;
struct EDGUIBonePicker* g_UIBonePicker;
struct EDGUIBodyType* g_UIBodyType;
struct EDGUIJointType* g_UIJointType;
struct EDGUITransformType* g_UITransformType;
AnimCharacter* g_AnimChar;


inline Quat EA2Q( Vec3 v ){ return Mat4::CreateRotationXYZ( v ).GetRotationQuaternion(); }
inline Vec3 Q2EA( Quat q ){ return Mat4::CreateRotationFromQuat( q ).GetXYZAngles(); }
inline StringView BodyType2String( uint8_t t )
{
	switch( t )
	{
	case AnimCharacter::BodyType_None: return "None";
	case AnimCharacter::BodyType_Sphere: return "Sphere";
	case AnimCharacter::BodyType_Capsule: return "Capsule";
	case AnimCharacter::BodyType_Box: return "Box";
	default: return "None";
	}
}


struct EDGUICharPicker : EDGUIRsrcPicker
{
	EDGUICharPicker(){ Reload(); }
	void Reload()
	{
		LOG << "Reloading chars";
		m_options.clear();
		DirectoryIterator tdi( "chars" );
		while( tdi.Next() )
		{
			StringView fn = tdi.Name();
			LOG << fn;
			if( !tdi.IsDirectory() && fn.ends_with( ".chr" ) )
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

struct EDGUICharOpenPicker : EDGUICharPicker
{
	EDGUICharOpenPicker()
	{
		caption = "Pick a character to open";
		m_confirm.caption = "Do you really want to open the character? All unsaved changes will be lost!";
	}
	virtual void _OnPickResource()
	{
		m_confirm.Open( this );
		m_frame->Add( &m_confirm );
	}
};

struct EDGUICharSavePicker : EDGUICharPicker
{
	EDGUICharSavePicker()
	{
		caption = "Pick a character to save or write the name";
		m_confirm.caption = "Do you really want to overwrite the character?";
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


struct EDGUIBonePicker : EDGUIRsrcPicker
{
	EDGUIBonePicker()
	{
		caption = "Pick a bone to use";
	}
	void Reload()
	{
		LOG << "Reloading bones";
		m_options.clear();
		m_options.push_back( "" );
		if( g_AnimChar == NULL || !g_AnimChar->m_cachedMesh )
		{
			LOG << "No mesh to load bones from";
			return;
		}
		SGRX_IMesh* M = g_AnimChar->m_cachedMesh;
		for( int i = 0; i < M->m_numBones; ++i )
		{
			m_options.push_back( M->m_bones[ i ].name );
		}
		_Search( m_searchString );
	}
};


struct EDGUIBodyType : EDGUIRsrcPicker
{
	EDGUIBodyType()
	{
		caption = "Pick a body type";
		m_options.push_back( "None" );
		m_options.push_back( "Sphere" );
		m_options.push_back( "Capsule" );
		m_options.push_back( "Box" );
	}
};


struct EDGUIJointType : EDGUIRsrcPicker
{
	EDGUIJointType()
	{
		caption = "Pick a joint type";
		m_options.push_back( "None" );
		m_options.push_back( "Hinge" );
	}
};


struct EDGUITransformType : EDGUIRsrcPicker
{
	EDGUITransformType()
	{
		caption = "Pick a transform type";
		m_options.push_back( "None" );
		m_options.push_back( "Move" );
		m_options.push_back( "Rotate" );
	}
};


struct EDGUIBoneProps : EDGUILayoutRow
{
	EDGUIBoneProps() :
		m_group( true, "Bone properties" ),
		m_group_hbox( true, "Hitbox" ),
		m_group_body( true, "Body" ),
		m_hbox_rotangles( V3(0), 2, V3(0), V3(360) ),
		m_hbox_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_hbox_extents( V3(0.1f), 2, V3(0), V3(100) ),
		m_hbox_multiplier( 1, 2, 0.01f, 100.0f ),
		m_body_rotangles( V3(0), 2, V3(0), V3(360) ),
		m_body_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_body_type( g_UIBodyType, "None" ),
		m_bid( -1 )
	{
		m_hbox_rotangles.caption = "Rotation (angles)";
		m_hbox_offset.caption = "Offset";
		m_hbox_extents.caption = "Extents";
		m_hbox_multiplier.caption = "Multiplier";
		
		m_body_rotangles.caption = "Rotation (angles)";
		m_body_offset.caption = "Offset";
		m_body_type.caption = "Body types";
		m_body_size.caption = "Body size";
		
		m_group_hbox.Add( &m_hbox_rotangles );
		m_group_hbox.Add( &m_hbox_offset );
		m_group_hbox.Add( &m_hbox_extents );
		m_group_hbox.Add( &m_hbox_multiplier );
		
		m_group_body.Add( &m_body_rotangles );
		m_group_body.Add( &m_body_offset );
		m_group_body.Add( &m_body_type );
		m_group_body.Add( &m_body_size );
		
		m_group.Add( &m_group_hbox );
		m_group.Add( &m_group_body );
		Add( &m_group );
	}
	
	void Prepare( int which )
	{
		m_bid = which;
		AnimCharacter::BoneInfo& B = g_AnimChar->bones[ m_bid ];
		
		m_hbox_rotangles.SetValue( Q2EA( B.hitbox.rotation ) );
		m_hbox_offset.SetValue( B.hitbox.position );
		m_hbox_extents.SetValue( B.hitbox.extents );
		m_hbox_multiplier.SetValue( B.hitbox.multiplier );
		m_body_rotangles.SetValue( Q2EA( B.body.rotation ) );
		m_body_offset.SetValue( B.body.position );
		m_body_type.SetValue( BodyType2String( B.body.type ) );
		m_body_size.SetValue( B.body.size );
	}
	
	EDGUIGroup m_group;
	EDGUIGroup m_group_hbox;
	EDGUIGroup m_group_body;
	EDGUIPropVec3 m_hbox_rotangles;
	EDGUIPropVec3 m_hbox_offset;
	EDGUIPropVec3 m_hbox_extents;
	EDGUIPropFloat m_hbox_multiplier;
	EDGUIPropVec3 m_body_rotangles;
	EDGUIPropVec3 m_body_offset;
	EDGUIPropRsrc m_body_type;
	EDGUIPropVec3 m_body_size;
	int m_bid;
};


void FC_EditBone( int which );
struct EDGUIBoneListProps : EDGUILayoutRow
{
	EDGUIBoneListProps() :
		m_group( true, "Bones" )
	{
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_group.Clear();
		m_boneButtons.clear();
		m_boneButtons.resize( g_AnimChar->bones.size() );
		for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
		{
			m_boneButtons[ i ].caption = g_AnimChar->bones[ i ].name;
			m_group.Add( &m_boneButtons[ i ] );
		}
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if( e->target >= (EDGUIItem*) &m_boneButtons[0] &&
				e->target <= (EDGUIItem*) &m_boneButtons.last() )
			{
				FC_EditBone( e->target - (EDGUIItem*) &m_boneButtons[0] );
				return 1;
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	Array< EDGUIButton > m_boneButtons;
};


struct EDGUIAttachmentProps : EDGUILayoutRow
{
	EDGUIAttachmentProps() :
		m_group( true, "Attachment" ),
		m_bone( g_UIBonePicker, "" ),
		m_rotangles( V3(0), 2, V3(0), V3(360) ),
		m_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_aid( -1 )
	{
		m_name.caption = "Name";
		m_bone.caption = "Bone";
		m_rotangles.caption = "Rotation (angles)";
		m_offset.caption = "Offset";
		
		m_group.Add( &m_name );
		m_group.Add( &m_bone );
		m_group.Add( &m_rotangles );
		m_group.Add( &m_offset );
		
		Add( &m_group );
	}
	
	void Prepare( int which )
	{
		m_aid = which;
		AnimCharacter::Attachment& A = g_AnimChar->attachments[ m_aid ];
		
		m_name.SetValue( A.name );
		m_bone.SetValue( A.bone );
		m_rotangles.SetValue( Q2EA( A.rotation ) );
		m_offset.SetValue( A.position );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_aid >= 0 )
		{
			AnimCharacter::Attachment& A = g_AnimChar->attachments[ m_aid ];
			switch( e->type )
			{
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_name ) A.name = m_name.m_value;
				else if( e->target == &m_bone ) A.bone = m_bone.m_value;
				else if( e->target == &m_rotangles ) A.rotation = EA2Q( m_rotangles.m_value );
				else if( e->target == &m_offset ) A.position = m_offset.m_value;
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIPropString m_name;
	EDGUIPropRsrc m_bone;
	EDGUIPropVec3 m_rotangles;
	EDGUIPropVec3 m_offset;
	int m_aid;
};


void FC_EditAttachment( int which );
struct EDGUIAttachmentListProps : EDGUILayoutRow
{
	EDGUIAttachmentListProps() :
		m_group( true, "Attachments" )
	{
		m_btnAdd.caption = "Add attachment";
		
		Add( &m_btnAdd );
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_group.Clear();
		m_atchButtons.clear();
		m_atchButtons.resize( g_AnimChar->attachments.size() );
		for( size_t i = 0; i < g_AnimChar->attachments.size(); ++i )
		{
			m_atchButtons[ i ].caption = g_AnimChar->attachments[ i ].name;
			m_group.Add( &m_atchButtons[ i ] );
		}
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btnAdd )
			{
				g_AnimChar->attachments.push_back( AnimCharacter::Attachment() );
				g_AnimChar->attachments.last().name = "<unnamed>";
				Prepare();
				return 1;
			}
			if( m_atchButtons.size() &&
				e->target >= (EDGUIItem*) &m_atchButtons[0] &&
				e->target <= (EDGUIItem*) &m_atchButtons.last() )
			{
				FC_EditAttachment( e->target - (EDGUIItem*) &m_atchButtons[0] );
				return 1;
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	Array< EDGUIButton > m_atchButtons;
};


struct EDGUILayerTransformProps : EDGUILayoutRow
{
	/*
		bone
		type
		offset / axis
		angle
	*/
};


struct EDGUILayerProps : EDGUILayoutRow
{
	/*
		name
		layer transform list
	*/
};


struct EDGUILayerListProps : EDGUILayoutRow
{
	void Prepare()
	{
	}
	
	// layer list
};


struct EDGUICharProps : EDGUILayoutRow
{
	EDGUICharProps() :
		m_group( true, "Character properties" ),
		m_mesh( g_UIMeshPicker )
	{
		tyname = "charprops";
		
		m_mesh.caption = "Mesh";
		
		m_group.Add( &m_mesh );
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_mesh.SetValue( g_AnimChar->mesh );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_mesh )
			{
				LOG << "Picked MESH: " << m_mesh.m_value;
				g_AnimChar->mesh = m_mesh.m_value;
				g_AnimChar->OnRenderUpdate();
				SGRX_IMesh* M = g_AnimChar->m_cachedMesh;
				if( M )
				{
					for( int i = 0; i < M->m_numBones; ++i )
					{
						StringView name = M->m_bones[ i ].name;
						size_t j = 0;
						for( ; j < g_AnimChar->bones.size(); ++j )
						{
							if( g_AnimChar->bones[ j ].name == name )
								break;
						}
						if( j == g_AnimChar->bones.size() )
						{
							AnimCharacter::BoneInfo B;
							B.name = name;
							g_AnimChar->bones.push_back( B );
						}
					}
				}
				lmm_prepmeshinst( g_AnimChar->m_cachedMeshInst );
				g_UIBonePicker->Reload();
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIPropRsrc m_mesh;
};


struct EDGUIMainFrame : EDGUIFrame, EDGUIRenderView::FrameInterface
{
	EDGUIMainFrame() :
		m_UIMenuSplit( true, 26, 0 ),
		m_UIParamSplit( false, 0, 0.6f ),
		m_UIRenderView( g_EdScene, this )
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
		m_MB_Cat1.caption = "Edit:";
		m_MBEditChar.caption = "Edit character";
		m_MBEditBones.caption = "Edit bones";
		m_MBEditAttachments.caption = "Edit attachments";
		m_MBEditLayers.caption = "Edit layers";
		m_UIMenuButtons.Add( &m_MB_Cat0 );
		m_UIMenuButtons.Add( &m_MBNew );
		m_UIMenuButtons.Add( &m_MBOpen );
		m_UIMenuButtons.Add( &m_MBSave );
		m_UIMenuButtons.Add( &m_MBSaveAs );
		m_UIMenuButtons.Add( &m_MB_Cat1 );
		m_UIMenuButtons.Add( &m_MBEditChar );
		m_UIMenuButtons.Add( &m_MBEditBones );
		m_UIMenuButtons.Add( &m_MBEditAttachments );
		m_UIMenuButtons.Add( &m_MBEditLayers );
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if(0);
			
			else if( e->target == &m_MBNew ) CH_New();
			else if( e->target == &m_MBOpen ) CH_Open();
			else if( e->target == &m_MBSave ) CH_Save();
			else if( e->target == &m_MBSaveAs ) CH_SaveAs();
			
			else if( e->target == &m_MBEditChar )
			{
				ClearParamList();
				m_charProps.Prepare();
				AddToParamList( &m_charProps );
				SetActiveMode( e->target );
			}
			else if( e->target == &m_MBEditBones )
			{
				ClearParamList();
				m_boneListProps.Prepare();
				AddToParamList( &m_boneListProps );
				SetActiveMode( e->target );
			}
			else if( e->target == &m_MBEditAttachments )
			{
				ClearParamList();
				m_atchListProps.Prepare();
				AddToParamList( &m_atchListProps );
				SetActiveMode( e->target );
			}
			else if( e->target == &m_MBEditLayers )
			{
				ClearParamList();
				m_layerListProps.Prepare();
				AddToParamList( &m_layerListProps );
				SetActiveMode( e->target );
			}
			
			return 1;
			
		case EDGUI_EVENT_PROPCHANGE:
			if( e->target == g_UICharOpenPicker )
			{
				CH_Real_Open( g_UICharOpenPicker->GetValue() );
			}
			if( e->target == g_UICharSavePicker )
			{
				CH_Real_Save( g_UICharSavePicker->GetValue() );
			}
			return 1;
		}
		return EDGUIFrame::OnEvent( e );
	}
	
	void SetActiveMode( EDGUIItem* btn )
	{
		m_MBEditChar.SetHighlight( &m_MBEditChar == btn );
		m_MBEditBones.SetHighlight( &m_MBEditBones == btn );
		m_MBEditAttachments.SetHighlight( &m_MBEditAttachments == btn );
		m_MBEditLayers.SetHighlight( &m_MBEditLayers == btn );
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
	
	void EditBone( int which )
	{
		ClearParamList();
		m_boneProps.Prepare( which );
		AddToParamList( &m_boneProps );
	}
	void EditAttachment( int which )
	{
		ClearParamList();
		m_atchProps.Prepare( which );
		AddToParamList( &m_atchProps );
	}
	
	void ResetEditorState()
	{
		g_UIBonePicker->Reload();
		
		ClearParamList();
		m_charProps.Prepare();
		AddToParamList( &m_charProps );
		SetActiveMode( &m_MBEditChar );
	}
	void CH_New()
	{
		m_fileName = "";
		delete g_AnimChar;
		g_AnimChar = new AnimCharacter;
		g_AnimChar->AddToScene( g_EdScene );
		ResetEditorState();
	}
	void CH_Open()
	{
		g_UICharOpenPicker->Reload();
		g_UICharOpenPicker->Open( this, "" );
		m_frame->Add( g_UICharOpenPicker );
	}
	void CH_Save()
	{
		if( m_fileName.size() )
		{
			CH_Real_Save( m_fileName );
		}
		else
		{
			CH_SaveAs();
		}
	}
	void CH_SaveAs()
	{
		g_UICharSavePicker->Reload();
		g_UICharSavePicker->Open( this, "" );
		m_frame->Add( g_UICharSavePicker );
	}
	void CH_Real_Open( const StringView& str )
	{
		LOG << "Trying to open character: " << str;
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "chars/%.*s.chr", TMIN( (int) str.size(), 200 ), str.data() );
		if( !g_AnimChar->Load( str ) )
		{
			LOG_ERROR << "FAILED TO LOAD CHAR FILE: " << bfr;
			return;
		}
		ResetEditorState();
		m_fileName = str;
	}
	void CH_Real_Save( const StringView& str )
	{
		LOG << "Trying to save character: " << str;
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "chars/%.*s.chr", TMIN( (int) str.size(), 200 ), str.data() );
		if( !g_AnimChar->Save( str ) )
		{
			LOG_ERROR << "FAILED TO SAVE CHAR FILE: " << bfr;
			return;
		}
		m_fileName = str;
	}
	
	String m_fileName;
	
	// property blocks
	EDGUIBoneProps m_boneProps;
	EDGUIBoneListProps m_boneListProps;
	EDGUIAttachmentProps m_atchProps;
	EDGUIAttachmentListProps m_atchListProps;
	EDGUILayerTransformProps m_layerXfProps;
	EDGUILayerProps m_layerProps;
	EDGUILayerListProps m_layerListProps;
	EDGUICharProps m_charProps;
	
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
	EDGUILabel m_MB_Cat1;
	EDGUIButton m_MBEditChar;
	EDGUIButton m_MBEditBones;
	EDGUIButton m_MBEditAttachments;
	EDGUIButton m_MBEditLayers;
};

void FC_EditBone( int which ){ g_UIFrame->EditBone( which ); }
void FC_EditAttachment( int which ){ g_UIFrame->EditAttachment( which ); }




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
		
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
		g_UIMeshPicker = new EDGUIMeshPicker( true );
		g_UICharOpenPicker = new EDGUICharOpenPicker;
		g_UICharSavePicker = new EDGUICharSavePicker;
		g_UIBonePicker = new EDGUIBonePicker;
		g_UIBodyType = new EDGUIBodyType;
		g_UIJointType = new EDGUIJointType;
		g_UITransformType = new EDGUITransformType;
		
		// core layout
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = V3(3);
		g_EdScene->camera.UpdateMatrices();
		g_AnimChar = new AnimCharacter;
		g_AnimChar->AddToScene( g_EdScene );
		g_UIFrame = new EDGUIMainFrame();
		g_UIFrame->Resize( GR_GetWidth(), GR_GetHeight() );
		
		// param area
		g_UIFrame->ResetEditorState();
		
		return true;
	}
	void OnDestroy()
	{
		delete g_UITransformType;
		g_UITransformType = NULL;
		delete g_UIJointType;
		g_UIJointType = NULL;
		delete g_UIBodyType;
		g_UIBodyType = NULL;
		delete g_UIBonePicker;
		g_UIBonePicker = NULL;
		delete g_UICharSavePicker;
		g_UICharSavePicker = NULL;
		delete g_UICharOpenPicker;
		g_UICharOpenPicker = NULL;
		delete g_UIMeshPicker;
		g_UIMeshPicker = NULL;
		delete g_UIFrame;
		g_UIFrame = NULL;
		delete g_AnimChar;
		g_AnimChar = NULL;
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
		g_AnimChar->Tick( dt );
		g_AnimChar->PreRender();
		g_UIFrame->Draw();
	}
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

