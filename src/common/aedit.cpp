

#include "aedit.hpp"


#define ASSET_SCRIPT_NAME "assets.txt"
#define ASSET_INFO_NAME "asset_revs.info"
#define OUTPUT_INFO_NAME "output_revs.info"


struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
SGRX_AssetScript* g_EdAS;
EDGUIPickers* g_UIPickers;

EDGUIRsrcPicker TMPRSRC;


#define SGRX_GET_FLAG( v, fl ) (((v) & (fl)) != 0)
#define SGRX_SET_FLAG( v, fl, b ) v = ((v) & ~(fl)) | ((b)?(fl):0)


void FC_EditTexture( size_t id ){ g_UIFrame->EditTexture( id ); }
void FC_EditTextureList(){ g_UIFrame->EditTextureList(); }
void FC_EditMesh( size_t id ){ g_UIFrame->EditMesh( id ); }
void FC_EditMeshList(){ g_UIFrame->EditMeshList(); }
void FC_EditAnimBundle( size_t id ){ g_UIFrame->EditAnimBundle( id ); }
void FC_EditAnimBundleList(){ g_UIFrame->EditAnimBundleList(); }
void FC_EditCategory( const StringView& name ){ g_UIFrame->EditCategory( name ); }
void FC_EditCatList(){ g_UIFrame->EditCatList(); }
void FC_SetTexture( TextureHandle tex )
{
	g_UIFrame->m_texPreview = tex;
	g_UIFrame->m_meshPrevInst->enabled = false;
	g_UIFrame->m_meshPrevInst->skin_matrices.resize( 0 );
	g_UIFrame->m_animPreview.Prepare( NULL );
	g_UIFrame->m_animPreview.Stop();
}
void FC_SetMesh( MeshHandle mesh )
{
	g_UIFrame->m_texPreview = NULL;
	g_UIFrame->m_meshPrevInst->SetMesh( mesh );
	g_UIFrame->m_meshPrevInst->enabled = mesh != NULL;
	g_UIFrame->m_meshPrevInst->skin_matrices.resize( 0 );
	g_UIFrame->m_animPreview.Prepare( NULL );
	g_UIFrame->m_animPreview.Stop();
}
void FC_SetAnim( MeshHandle mesh, AnimHandle anim )
{
	g_UIFrame->m_texPreview = NULL;
	g_UIFrame->m_meshPrevInst->SetMesh( mesh );
	g_UIFrame->m_meshPrevInst->enabled = mesh != NULL;
	g_UIFrame->m_meshPrevInst->skin_matrices.resize( mesh->m_numBones );
	g_UIFrame->m_animPreview.Prepare( mesh );
	g_UIFrame->m_animPreview.Stop();
	g_UIFrame->m_animPreview.Play( anim );
}


void EDGUISmallEnumPicker::_OnChangeZoom()
{
	EDGUIRsrcPicker::_OnChangeZoom();
	m_itemHeight /= 4;
}

void EDGUILongEnumPicker::_OnChangeZoom()
{
	EDGUIRsrcPicker::_OnChangeZoom();
	m_itemWidth = 300;
	m_itemHeight = 16;
}

EDGUIMtlBlendMode::EDGUIMtlBlendMode()
{
	caption = "Pick the blending mode";
	m_options.push_back( "None" );
	m_options.push_back( "Basic" );
	m_options.push_back( "Additive" );
	m_options.push_back( "Multiply" );
	_Search( m_searchString );
}

EDGUIImageFilterSharpenMode::EDGUIImageFilterSharpenMode()
{
	caption = "Pick the sharpening mode";
	m_options.push_back( "0-1" );
	m_options.push_back( "1-1" );
	m_options.push_back( "1-2" );
	_Search( m_searchString );
}

EDGUIImageFilterType::EDGUIImageFilterType()
{
	caption = "Pick an image filter";
	m_options.push_back( "Resize" );
	m_options.push_back( "Rearrange" );
	m_options.push_back( "Sharpen" );
	m_options.push_back( "To linear" );
	m_options.push_back( "From linear" );
	m_options.push_back( "Expand range" );
	m_options.push_back( "Brightness/contrast/power" );
	_Search( m_searchString );
}

EDGUITextureOutputFormat::EDGUITextureOutputFormat()
{
	caption = "Pick an texture output format";
	for( int i = SGRX_TOF_Unknown + 1; i < SGRX_TOF__COUNT; ++i )
	{
		m_options.push_back( SGRX_TextureOutputFormat_ToString(
			(SGRX_TextureOutputFormat) i ) );
	}
	_Search( m_searchString );
}

EDGUICategoryPicker::EDGUICategoryPicker()
{
	m_looseSearch = true;
	Reload();
}
void EDGUICategoryPicker::Reload()
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

EDGUITextureAssetPicker::EDGUITextureAssetPicker()
{
	m_looseSearch = true;
	Reload();
}
void EDGUITextureAssetPicker::Reload()
{
	m_options.clear();
	if( g_EdAS )
	{
		for( size_t i = 0; i < g_EdAS->textureAssets.size(); ++i )
		{
			const SGRX_TextureAsset& TA = g_EdAS->textureAssets[ i ];
			String opt = TA.outputCategory;
			opt.append( "/" );
			opt.append( TA.outputName );
			m_options.push_back( opt );
		}
	}
	_Search( m_searchString );
}

EDGUIMeshAssetPicker::EDGUIMeshAssetPicker()
{
	m_looseSearch = true;
	Reload();
}
void EDGUIMeshAssetPicker::Reload()
{
	m_options.clear();
	if( g_EdAS )
	{
		for( size_t i = 0; i < g_EdAS->meshAssets.size(); ++i )
		{
			const SGRX_MeshAsset& TA = g_EdAS->meshAssets[ i ];
			String opt = TA.outputCategory;
			opt.append( "/" );
			opt.append( TA.outputName );
			m_options.push_back( opt );
		}
	}
	_Search( m_searchString );
}

EDGUIAssetPathPicker::EDGUIAssetPathPicker() : m_depth(0)
{
	m_looseSearch = true;
	Reload();
}
void EDGUIAssetPathPicker::AddOptionsFromDir( const StringView& path )
{
	if( m_depth > 32 )
		return;
	m_depth++;
	FS_IterateDirectory( path, this );
	m_depth--;
}
bool EDGUIAssetPathPicker::HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
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
void EDGUIAssetPathPicker::Reload()
{
	m_options.clear();
	m_depth = 0;
	AddOptionsFromDir( "assets" );
	_Search( m_searchString );
}

EDGUIASMeshNamePicker::EDGUIASMeshNamePicker()
{
	m_looseSearch = true;
	Reload();
}
void EDGUIASMeshNamePicker::Reload()
{
	m_options.clear();
	if( m_scene )
	{
		m_scene->GetMeshList( m_options );
	}
	_Search( m_searchString );
}

EDGUIASAnimNamePicker::EDGUIASAnimNamePicker()
{
	m_looseSearch = true;
	Reload();
}
void EDGUIASAnimNamePicker::Reload()
{
	m_options.clear();
	for( size_t i = 0; i < m_scenes.size(); ++i )
	{
		if( !m_scenes[ i ] )
			continue;
		m_scenes[ i ]->GetAnimList( m_options );
	}
	_Search( m_searchString );
}

EDGUICreatePickButton::EDGUICreatePickButton( EDGUIRsrcPicker* rsrcPicker, const StringView& def ) :
	EDGUIPropRsrc( rsrcPicker, def )
{
}
int EDGUICreatePickButton::OnEvent( EDGUIEvent* e )
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


EDGUIImgFilterBase::EDGUIImgFilterBase( SGRX_ImageFilter* iflt, bool addCMF ) :
	m_blend( 1, 3, 0, 1 ),
	m_cclamp( true ),
	m_applyR( true ),
	m_applyG( true ),
	m_applyB( true ),
	m_applyA( true ),
	m_hfilter( iflt )
{
	if( addCMF )
		AddCMFFields();
}

void EDGUIImgFilterBase::AddCMFFields()
{
	m_blend.caption = "Blend factor";
	m_cclamp.caption = "Clamp colors?";
	m_applyR.caption = "Apply to R?";
	m_applyG.caption = "Apply to G?";
	m_applyB.caption = "Apply to B?";
	m_applyA.caption = "Apply to A?";
	
	m_blend.SetValue( m_hfilter->blend );
	m_cclamp.SetValue( m_hfilter->cclamp );
	m_applyR.SetValue( ( m_hfilter->colors & 0x1 ) != 0 );
	m_applyG.SetValue( ( m_hfilter->colors & 0x2 ) != 0 );
	m_applyB.SetValue( ( m_hfilter->colors & 0x4 ) != 0 );
	m_applyA.SetValue( ( m_hfilter->colors & 0x8 ) != 0 );
	
	Add( &m_blend );
	Add( &m_cclamp );
	Add( &m_applyR );
	Add( &m_applyG );
	Add( &m_applyB );
	Add( &m_applyA );
}

int EDGUIImgFilterBase::OnEvent( EDGUIEvent* e )
{
	if( m_hfilter )
	{
		SGRX_ImageFilter* F = m_hfilter;
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_blend ){ F->blend = m_blend.m_value; }
			if( e->target == &m_cclamp ){ F->cclamp = m_cclamp.m_value; }
			if( e->target == &m_applyR ){ SGRX_SET_FLAG( F->colors, 0x1, m_applyR.m_value ); }
			if( e->target == &m_applyG ){ SGRX_SET_FLAG( F->colors, 0x2, m_applyG.m_value ); }
			if( e->target == &m_applyB ){ SGRX_SET_FLAG( F->colors, 0x4, m_applyB.m_value ); }
			if( e->target == &m_applyA ){ SGRX_SET_FLAG( F->colors, 0x8, m_applyA.m_value ); }
			break;
		}
	}
	return EDGUILayoutRow::OnEvent( e );
}

EDGUIImgFilter_Resize::EDGUIImgFilter_Resize( SGRX_ImageFilter* iflt ) :
	EDGUIImgFilterBase( iflt ),
	m_width( 256, 1, 4096 ),
	m_height( 256, 1, 4096 ),
	m_srgb( false )
{
	SGRX_ImageFilter_Resize* F = iflt->upcast<SGRX_ImageFilter_Resize>();
	m_width.SetValue( F->width );
	m_height.SetValue( F->height );
	m_srgb.SetValue( F->srgb );
	
	m_width.caption = "Width";
	m_height.caption = "Height";
	m_srgb.caption = "SRGB?";
	
	Add( &m_width );
	Add( &m_height );
	Add( &m_srgb );
}

int EDGUIImgFilter_Resize::OnEvent( EDGUIEvent* e )
{
	if( m_hfilter )
	{
		SGRX_ImageFilter_Resize* F = m_hfilter->upcast<SGRX_ImageFilter_Resize>();
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_width ) F->width = m_width.m_value;
			if( e->target == &m_height ) F->height = m_height.m_value;
			if( e->target == &m_srgb ) F->srgb = m_srgb.m_value;
			break;
		}
	}
	return EDGUIImgFilterBase::OnEvent( e );
}

EDGUIImgFilter_Rearrange::EDGUIImgFilter_Rearrange( SGRX_ImageFilter* iflt ) :
	EDGUIImgFilterBase( iflt ),
	m_width( 16, 1, 4096 )
{
	SGRX_ImageFilter_Rearrange* F = iflt->upcast<SGRX_ImageFilter_Rearrange>();
	m_width.SetValue( F->width );
	
	m_width.caption = "Width";
	
	Add( &m_width );
}

int EDGUIImgFilter_Rearrange::OnEvent( EDGUIEvent* e )
{
	if( m_hfilter )
	{
		SGRX_ImageFilter_Rearrange* F = m_hfilter->upcast<SGRX_ImageFilter_Rearrange>();
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_width ) F->width = m_width.m_value;
			break;
		}
	}
	return EDGUIImgFilterBase::OnEvent( e );
}

EDGUIImgFilter_Sharpen::EDGUIImgFilter_Sharpen( SGRX_ImageFilter* iflt ) :
	EDGUIImgFilterBase( iflt ),
	m_factor( 1, 2, 0, 100 ),
	m_mode( &g_UIPickers->sharpenMode )
{
	SGRX_ImageFilter_Sharpen* F = iflt->upcast<SGRX_ImageFilter_Sharpen>();
	m_factor.SetValue( F->factor );
	m_mode.SetValue( SGRX_ImgFltSharpen_ToString( F->mode ) );
	
	m_factor.caption = "Factor";
	m_mode.caption = "Mode";
	
	Add( &m_factor );
	Add( &m_mode );
	
	AddCMFFields();
}

int EDGUIImgFilter_Sharpen::OnEvent( EDGUIEvent* e )
{
	if( m_hfilter )
	{
		SGRX_ImageFilter_Sharpen* F = m_hfilter->upcast<SGRX_ImageFilter_Sharpen>();
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_factor ) F->factor = m_factor.m_value;
			if( e->target == &m_mode ) F->mode = SGRX_ImgFltSharpen_FromString( m_mode.m_value );
			break;
		}
	}
	return EDGUIImgFilterBase::OnEvent( e );
}

EDGUIImgFilter_ExpandRange::EDGUIImgFilter_ExpandRange( SGRX_ImageFilter* iflt ) :
	EDGUIImgFilterBase( iflt ),
	m_vmin( V4(0), 3, V4(0), V4(1) ),
	m_vmax( V4(0,0,1,1), 3, V4(0), V4(1) )
{
	SGRX_ImageFilter_ExpandRange* F = iflt->upcast<SGRX_ImageFilter_ExpandRange>();
	m_vmin.SetValue( F->vmin );
	m_vmax.SetValue( F->vmax );
	
	m_vmin.caption = "Min. color";
	m_vmax.caption = "Max. color";
	
	Add( &m_vmin );
	Add( &m_vmax );
	
	AddCMFFields();
}

int EDGUIImgFilter_ExpandRange::OnEvent( EDGUIEvent* e )
{
	if( m_hfilter )
	{
		SGRX_ImageFilter_ExpandRange* F = m_hfilter->upcast<SGRX_ImageFilter_ExpandRange>();
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_vmin ) F->vmin = m_vmin.m_value;
			if( e->target == &m_vmax ) F->vmax = m_vmax.m_value;
			break;
		}
	}
	return EDGUIImgFilterBase::OnEvent( e );
}

EDGUIImgFilter_BCP::EDGUIImgFilter_BCP( SGRX_ImageFilter* iflt ) :
	EDGUIImgFilterBase( iflt ),
	m_apply_bc1( true ),
	m_brightness( 0, 2, -100, 100 ),
	m_contrast( 1, 2, -100, 100 ),
	m_apply_pow( false ),
	m_power( 1, 2, 0.01f, 100 ),
	m_apply_bc2( false ),
	m_brightness_2( 0, 2, -100, 100 ),
	m_contrast_2( 1, 2, -100, 100 )
{
	SGRX_ImageFilter_BCP* F = iflt->upcast<SGRX_ImageFilter_BCP>();
	
	m_apply_bc1.SetValue( F->apply_bc1 );
	m_brightness.SetValue( F->brightness );
	m_contrast.SetValue( F->contrast );
	m_apply_pow.SetValue( F->apply_pow );
	m_power.SetValue( F->power );
	m_apply_bc2.SetValue( F->apply_bc2 );
	m_brightness_2.SetValue( F->brightness_2 );
	m_contrast_2.SetValue( F->contrast_2 );
	
	m_apply_bc1.caption = "Apply brightness/contrast-1";
	m_brightness.caption = "Brightness [1]";
	m_contrast.caption = "Contrast [1]";
	m_apply_pow.caption = "Apply power";
	m_power.caption = "Power";
	m_apply_bc2.caption = "Apply brightness/contrast-2";
	m_brightness_2.caption = "Brightness [2]";
	m_contrast_2.caption = "Contrast [2]";
	
	Add( &m_apply_bc1 );
	Add( &m_brightness );
	Add( &m_contrast );
	Add( &m_apply_pow );
	Add( &m_power );
	Add( &m_apply_bc2 );
	Add( &m_brightness_2 );
	Add( &m_contrast_2 );
	
	AddCMFFields();
}

int EDGUIImgFilter_BCP::OnEvent( EDGUIEvent* e )
{
	if( m_hfilter )
	{
		SGRX_ImageFilter_BCP* F = m_hfilter->upcast<SGRX_ImageFilter_BCP>();
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_apply_bc1 ) F->apply_bc1 = m_apply_bc1.m_value;
			if( e->target == &m_brightness ) F->brightness = m_brightness.m_value;
			if( e->target == &m_contrast ) F->contrast = m_contrast.m_value;
			if( e->target == &m_apply_pow ) F->apply_pow = m_apply_pow.m_value;
			if( e->target == &m_power ) F->power = m_power.m_value;
			if( e->target == &m_apply_bc2 ) F->apply_bc2 = m_apply_bc2.m_value;
			if( e->target == &m_brightness_2 ) F->brightness_2 = m_brightness_2.m_value;
			if( e->target == &m_contrast_2 ) F->contrast_2 = m_contrast_2.m_value;
			break;
		}
	}
	return EDGUIImgFilterBase::OnEvent( e );
}

EDGUIAssetTexture::EDGUIAssetTexture() :
	m_group( true, "Texture" ),
	m_sourceFile( &g_UIPickers->assetPath ),
	m_outputCategory( &g_UIPickers->category ),
	m_outputType( &g_UIPickers->textureOutputFormat ),
	m_sfgroup( true, "Selected filter" ),
	m_curFilter( NULL ),
	m_flgroup( true, "Filters" ),
	m_filterBtnAdd( &g_UIPickers->imageFilterType ),
	m_tid( NOT_FOUND )
{
	m_outputCategory.m_requestReload = true;
	
	m_btnDuplicate.caption = "Duplicate";
	m_btnDelete.caption = "Delete";
	
	m_topCol.Add( &m_btnDuplicate );
	m_topCol.Add( &m_btnDelete );
	
	m_sourceFile.caption = "Source file";
	m_outputCategory.caption = "Output category";
	m_outputName.caption = "Output name";
	m_outputType.caption = "Output type";
	m_isSRGB.caption = "Is SRGB?";
	m_mips.caption = "Generate mipmaps?";
	m_lerp.caption = "Use linear interpolation?";
	m_clampx.caption = "Clamp X";
	m_clampy.caption = "Clamp Y";
	
	m_group.Add( &m_sourceFile );
	m_group.Add( &m_outputCategory );
	m_group.Add( &m_outputName );
	m_group.Add( &m_outputType );
	m_group.Add( &m_isSRGB );
	m_group.Add( &m_mips );
	m_group.Add( &m_lerp );
	m_group.Add( &m_clampx );
	m_group.Add( &m_clampy );
	
	m_filterBtnAdd.SetValue( "Pick filter to add" );
	
	m_filterButtons.Add( &m_filterEditButton );
	m_flgroup.Add( &m_filterBtnAdd );
	m_flgroup.Add( &m_filterButtons );
	
	Add( &m_topCol );
	Add( &m_group );
	Add( &m_columnList );
	m_columnList.Add( &m_sfgroup );
	m_columnList.Add( &m_flgroup );
}

EDGUIAssetTexture::~EDGUIAssetTexture()
{
	m_sfgroup.Clear();
	if( m_curFilter )
	{
		delete m_curFilter;
		m_curFilter = NULL;
	}
}

void EDGUIAssetTexture::ReloadFilterList()
{
	SGRX_TextureAsset& TA = g_EdAS->textureAssets[ m_tid ];
	
	m_filterButtons.m_options.resize( TA.filters.size() );
	for( size_t i = 0; i < TA.filters.size(); ++i )
	{
		m_filterButtons.m_options[ i ] = TA.filters[ i ]->GetName();
	}
	m_filterButtons.UpdateOptions();
	
	UpdatePreviewTexture();
}

void EDGUIAssetTexture::UpdatePreviewTexture()
{
	SGRX_TextureAsset& TA = g_EdAS->textureAssets[ m_tid ];
	TextureHandle tex;
	SGRX_IFP32Handle img = SGRX_ProcessTextureAsset( TA );
	tex = SGRX_FP32ToTexture( img, TA );
	if( tex == NULL )
		tex = GR_GetTexture( "textures/unit.png" );
	FC_SetTexture( tex );
}

void EDGUIAssetTexture::Prepare( size_t tid )
{
	m_tid = tid;
	SGRX_TextureAsset& TA = g_EdAS->textureAssets[ tid ];
	
	m_sourceFile.SetValue( TA.sourceFile );
	m_outputCategory.SetValue( TA.outputCategory );
	m_outputName.SetValue( TA.outputName );
	m_outputType.SetValue( SGRX_TextureOutputFormat_ToString( TA.outputType ) );
	m_isSRGB.SetValue( TA.isSRGB );
	m_mips.SetValue( TA.mips );
	m_lerp.SetValue( TA.lerp );
	m_clampx.SetValue( TA.clampx );
	m_clampy.SetValue( TA.clampy );
	
	m_sfgroup.Clear();
	if( m_curFilter )
	{
		delete m_curFilter;
		m_curFilter = NULL;
	}
	
	ReloadFilterList();
}

void EDGUIAssetTexture::EditFilter( SGRX_ImageFilter* IF )
{
	EDGUILayoutRow* newflt = NULL;
	switch( IF->GetType() )
	{
	case SGRX_AIF_Resize: newflt = new EDGUIImgFilter_Resize( IF ); break;
	case SGRX_AIF_Rearrange: newflt = new EDGUIImgFilter_Rearrange( IF ); break;
	case SGRX_AIF_Sharpen: newflt = new EDGUIImgFilter_Sharpen( IF ); break;
	case SGRX_AIF_ToLinear: newflt = new EDGUIImgFilterBase( IF, true ); break;
	case SGRX_AIF_FromLinear: newflt = new EDGUIImgFilterBase( IF, true ); break;
	case SGRX_AIF_ExpandRange: newflt = new EDGUIImgFilter_ExpandRange( IF ); break;
	case SGRX_AIF_BCP: newflt = new EDGUIImgFilter_BCP( IF ); break;
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

int EDGUIAssetTexture::OnEvent( EDGUIEvent* e )
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
			if( e->target == &m_mips ){ TA.mips = m_mips.m_value; }
			if( e->target == &m_lerp ){ TA.lerp = m_lerp.m_value; }
			if( e->target == &m_clampx ){ TA.clampx = m_clampx.m_value; }
			if( e->target == &m_clampy ){ TA.clampy = m_clampy.m_value; }
			if( e->target == &m_filterBtnAdd )
			{
				SGRX_ImageFilter* IF = NULL;
				switch( g_UIPickers->imageFilterType.GetPickedType() )
				{
				case SGRX_AIF_Resize: IF = new SGRX_ImageFilter_Resize; break;
				case SGRX_AIF_Rearrange: IF = new SGRX_ImageFilter_Rearrange; break;
				case SGRX_AIF_Sharpen: IF = new SGRX_ImageFilter_Sharpen; break;
				case SGRX_AIF_ToLinear: IF = new SGRX_ImageFilter_Linear( false ); break;
				case SGRX_AIF_FromLinear: IF = new SGRX_ImageFilter_Linear( true ); break;
				case SGRX_AIF_ExpandRange: IF = new SGRX_ImageFilter_ExpandRange; break;
				case SGRX_AIF_BCP: IF = new SGRX_ImageFilter_BCP; break;
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
		case EDGUI_EVENT_PROPCHANGE:
			TA.ri.rev_asset++;
			UpdatePreviewTexture();
			break;
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btnDuplicate )
			{
				SGRX_TextureAsset TAcopy;
				TAcopy.Clone( g_EdAS->textureAssets[ m_tid ] );
				m_tid = g_EdAS->textureAssets.size();
				TAcopy.outputName.append( " - Copy" );
				g_EdAS->textureAssets.push_back( TAcopy );
				Prepare( m_tid );
				return 1;
			}
			if( e->target == &m_btnDelete )
			{
				g_EdAS->textureAssets.erase( m_tid );
				m_tid = NOT_FOUND;
				FC_EditTextureList();
				return 1;
			}
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

EDGUIAssetTextureList::EDGUIAssetTextureList() :
	m_group( true, "Texture assets" ),
	m_editButton( false )
{
	m_btnAdd.caption = "Add texture";
	m_filter.caption = "Filter";
	
	m_buttons.Add( &m_editButton );
	m_group.Add( &m_buttons );
	Add( &m_btnAdd );
	Add( &m_filter );
	Add( &m_group );
}

void EDGUIAssetTextureList::Prepare()
{
	m_buttons.m_options.clear();
	m_buttons.m_idTable.clear();
	m_buttons.m_options.reserve( g_EdAS->textureAssets.size() );
	m_buttons.m_idTable.reserve( g_EdAS->textureAssets.size() );
	for( size_t i = 0; i < g_EdAS->textureAssets.size(); ++i )
	{
		String name;
		g_EdAS->textureAssets[ i ].GetFullName( name );
		if( StringView(name).match_loose( m_filter.m_value ) )
		{
			g_EdAS->textureAssets[ i ].GetDesc( name );
			m_buttons.m_options.push_back( name );
			m_buttons.m_idTable.push_back( i );
		}
	}
	m_buttons.UpdateOptions();
}

int EDGUIAssetTextureList::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_filter )
		{
			Prepare();
		}
		break;
	case EDGUI_EVENT_BTNCLICK:
		if( e->target == &m_btnAdd )
		{
			g_EdAS->textureAssets.push_back( SGRX_TextureAsset() );
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

EDGUIAssetMesh::EDGUIAssetMesh() :
	m_group( true, "Mesh" ),
	m_sourceFile( &g_UIPickers->assetPath ),
	m_outputCategory( &g_UIPickers->category ),
	m_MPgroup( true, "Selected part" ),
	m_MPmeshName( &g_UIPickers->meshName ),
	m_MPshader( &g_UIPickers->shader ),
	m_MPtexture0( &g_UIPickers->textureAsset ),
	m_MPtexture1( &g_UIPickers->textureAsset ),
	m_MPtexture2( &g_UIPickers->textureAsset ),
	m_MPtexture3( &g_UIPickers->textureAsset ),
	m_MPtexture4( &g_UIPickers->textureAsset ),
	m_MPtexture5( &g_UIPickers->textureAsset ),
	m_MPtexture6( &g_UIPickers->textureAsset ),
	m_MPtexture7( &g_UIPickers->textureAsset ),
	m_MPmtlUnlit( false ),
	m_MPmtlNocull( false ),
	m_MPmtlDecal( false ),
	m_MPmtlBlendMode( &g_UIPickers->blendMode ),
	m_MPLgroup( true, "Parts" ),
	m_mid( NOT_FOUND ),
	m_pid( NOT_FOUND )
{
	m_outputCategory.m_requestReload = true;
	
	m_btnDuplicate.caption = "Duplicate";
	m_btnDelete.caption = "Delete";
	
	m_topCol.Add( &m_btnDuplicate );
	m_topCol.Add( &m_btnDelete );
	
	m_sourceFile.caption = "Source file";
	m_outputCategory.caption = "Output category";
	m_outputName.caption = "Output name";
	m_rotateY2Z.caption = "Rotate Y->Z";
	m_flipUVY.caption = "Flip UV/Y";
	m_transform.caption = "Transform";
	
	m_group.Add( &m_sourceFile );
	m_group.Add( &m_outputCategory );
	m_group.Add( &m_outputName );
	m_group.Add( &m_rotateY2Z );
	m_group.Add( &m_flipUVY );
	m_group.Add( &m_transform );
	
	m_MPLBtnAdd.caption = "Add part";
	m_MPLButtons.Add( &m_MPLEditButton );
	m_MPLgroup.Add( &m_MPLBtnAdd );
	m_MPLgroup.Add( &m_MPLButtons );
	
	m_MPmeshName.caption = "Mesh name";
	m_MPshader.caption = "Shader";
	m_MPtexture0.caption = "Texture 0";
	m_MPtexture1.caption = "Texture 1";
	m_MPtexture2.caption = "Texture 2";
	m_MPtexture3.caption = "Texture 3";
	m_MPtexture4.caption = "Texture 4";
	m_MPtexture5.caption = "Texture 5";
	m_MPtexture6.caption = "Texture 6";
	m_MPtexture7.caption = "Texture 7";
	m_MPmtlUnlit.caption = "Unlit?";
	m_MPmtlNocull.caption = "Disable culling?";
	m_MPmtlDecal.caption = "Decal?";
	m_MPmtlBlendMode.caption = "Blend mode";
	m_MPoptTransform.caption = "Transform (override)?";
	
	m_MPmeshName.m_requestReload = true;
	
	m_MPart.Add( &m_MPmeshName );
	m_MPart.Add( &m_MPshader );
	m_MPart.Add( &m_MPtexture0 );
	m_MPart.Add( &m_MPtexture1 );
	m_MPart.Add( &m_MPtexture2 );
	m_MPart.Add( &m_MPtexture3 );
	m_MPart.Add( &m_MPtexture4 );
	m_MPart.Add( &m_MPtexture5 );
	m_MPart.Add( &m_MPtexture6 );
	m_MPart.Add( &m_MPtexture7 );
	m_MPart.Add( &m_MPmtlUnlit );
	m_MPart.Add( &m_MPmtlNocull );
	m_MPart.Add( &m_MPmtlDecal );
	m_MPart.Add( &m_MPmtlBlendMode );
	m_MPart.Add( &m_MPoptTransform );
	
	Add( &m_topCol );
	Add( &m_group );
	Add( &m_MPgroup );
	Add( &m_MPLgroup );
}

void EDGUIAssetMesh::UpdatePreviewMesh()
{
	SGRX_MeshAsset& MA = g_EdAS->meshAssets[ m_mid ];
	MeshHandle mesh = SGRX_ProcessMeshAsset( g_EdAS, MA );
	FC_SetMesh( mesh );
}

void EDGUIAssetMesh::ReloadPartList()
{
	SGRX_MeshAsset& MA = g_EdAS->meshAssets[ m_mid ];
	
	m_MPLButtons.m_options.resize( MA.parts.size() );
	for( size_t i = 0; i < MA.parts.size(); ++i )
	{
		MA.parts[ i ]->GetDesc( i, m_MPLButtons.m_options[ i ] );
	}
	m_MPLButtons.UpdateOptions();
	
	UpdatePreviewMesh();
}

void EDGUIAssetMesh::ReloadImpScene()
{
	g_UIPickers->SetMesh( new SGRX_Scene3D( m_sourceFile.m_value ) );
}

void EDGUIAssetMesh::Prepare( size_t mid )
{
	g_UIPickers->textureAsset.Reload();
	
	m_mid = mid;
	SGRX_MeshAsset& MA = g_EdAS->meshAssets[ mid ];
	
	m_sourceFile.SetValue( MA.sourceFile );
	m_outputCategory.SetValue( MA.outputCategory );
	m_outputName.SetValue( MA.outputName );
	m_rotateY2Z.SetValue( MA.rotateY2Z );
	m_flipUVY.SetValue( MA.flipUVY );
	m_transform.SetValue( MA.transform );
	
	ReloadPartList();
	PreparePart( NOT_FOUND );
	
	ReloadImpScene();
}

void EDGUIAssetMesh::PreparePart( size_t pid )
{
	m_pid = pid;
	SGRX_MeshAsset& MA = g_EdAS->meshAssets[ m_mid ];
	SGRX_MeshAssetPart* MP = m_pid != NOT_FOUND ? MA.parts[ m_pid ] : NULL;
	
	m_MPmeshName.SetValue( MP ? MP->meshName : "" );
	m_MPshader.SetValue( MP ? MP->shader : "" );
	m_MPtexture0.SetValue( MP ? MP->textures[ 0 ] : "" );
	m_MPtexture1.SetValue( MP ? MP->textures[ 1 ] : "" );
	m_MPtexture2.SetValue( MP ? MP->textures[ 2 ] : "" );
	m_MPtexture3.SetValue( MP ? MP->textures[ 3 ] : "" );
	m_MPtexture4.SetValue( MP ? MP->textures[ 4 ] : "" );
	m_MPtexture5.SetValue( MP ? MP->textures[ 5 ] : "" );
	m_MPtexture6.SetValue( MP ? MP->textures[ 6 ] : "" );
	m_MPtexture7.SetValue( MP ? MP->textures[ 7 ] : "" );
	m_MPmtlUnlit.SetValue( MP ? SGRX_GET_FLAG( MP->mtlFlags, SGRX_MtlFlag_Unlit ) : false );
	m_MPmtlNocull.SetValue( MP ? SGRX_GET_FLAG( MP->mtlFlags, SGRX_MtlFlag_Nocull ) : false );
	m_MPmtlDecal.SetValue( MP ? SGRX_GET_FLAG( MP->mtlFlags, SGRX_MtlFlag_Decal ) : false );
	m_MPmtlBlendMode.SetValue( SGRX_MtlBlend_ToString( MP ? MP->mtlBlendMode : 0 ) );
	m_MPoptTransform.SetValue( MP ? MP->optTransform : 0 );
	m_MPgroup.Clear();
	if( MP )
		m_MPgroup.Add( &m_MPart );
}

int EDGUIAssetMesh::OnEvent( EDGUIEvent* e )
{
	if( m_mid != NOT_FOUND )
	{
		SGRX_MeshAsset& MA = g_EdAS->meshAssets[ m_mid ];
		SGRX_MeshAssetPart* MP = m_pid != NOT_FOUND ? MA.parts[ m_pid ] : NULL;
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_sourceFile ){ MA.sourceFile = m_sourceFile.m_value; UpdatePreviewMesh(); }
			if( e->target == &m_outputCategory ){ MA.outputCategory = m_outputCategory.m_value; }
			if( e->target == &m_outputName ){ MA.outputName = m_outputName.m_value; }
			if( e->target == &m_rotateY2Z ){ MA.rotateY2Z = m_rotateY2Z.m_value; UpdatePreviewMesh(); }
			if( e->target == &m_flipUVY ){ MA.flipUVY = m_flipUVY.m_value; UpdatePreviewMesh(); }
			if( e->target == &m_transform ){ MA.transform = m_transform.m_value; UpdatePreviewMesh(); }
			if( MP )
			{
				bool ed = false;
				if( e->target == &m_MPmeshName ){ ed = true; MP->meshName = m_MPmeshName.m_value; }
				if( e->target == &m_MPshader ){ ed = true; MP->shader = m_MPshader.m_value; }
				if( e->target == &m_MPtexture0 ){ ed = true; MP->textures[ 0 ] = m_MPtexture0.m_value; }
				if( e->target == &m_MPtexture1 ){ ed = true; MP->textures[ 1 ] = m_MPtexture1.m_value; }
				if( e->target == &m_MPtexture2 ){ ed = true; MP->textures[ 2 ] = m_MPtexture2.m_value; }
				if( e->target == &m_MPtexture3 ){ ed = true; MP->textures[ 3 ] = m_MPtexture3.m_value; }
				if( e->target == &m_MPtexture4 ){ ed = true; MP->textures[ 4 ] = m_MPtexture4.m_value; }
				if( e->target == &m_MPtexture5 ){ ed = true; MP->textures[ 5 ] = m_MPtexture5.m_value; }
				if( e->target == &m_MPtexture6 ){ ed = true; MP->textures[ 6 ] = m_MPtexture6.m_value; }
				if( e->target == &m_MPtexture7 ){ ed = true; MP->textures[ 7 ] = m_MPtexture7.m_value; }
				if( e->target == &m_MPmtlUnlit ){ ed = true;
					SGRX_SET_FLAG( MP->mtlFlags, SGRX_MtlFlag_Unlit, m_MPmtlUnlit.m_value ); }
				if( e->target == &m_MPmtlNocull ){ ed = true;
					SGRX_SET_FLAG( MP->mtlFlags, SGRX_MtlFlag_Nocull, m_MPmtlNocull.m_value ); }
				if( e->target == &m_MPmtlDecal ){ ed = true;
					SGRX_SET_FLAG( MP->mtlFlags, SGRX_MtlFlag_Decal, m_MPmtlDecal.m_value ); }
				if( e->target == &m_MPmtlBlendMode ){ ed = true;
					MP->mtlBlendMode = SGRX_MtlBlend_FromString( m_MPmtlBlendMode.m_value ); }
				if( e->target == &m_MPoptTransform ){ ed = true;
					MP->optTransform = m_MPoptTransform.m_value; }
				if( ed )
				{
					ReloadPartList();
				}
			}
			break;
		case EDGUI_EVENT_PROPCHANGE:
			MA.ri.rev_asset++;
			if( e->target == &m_sourceFile ){ ReloadImpScene(); }
			break;
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btnDuplicate )
			{
				SGRX_MeshAsset MAcopy;
				MAcopy.Clone( g_EdAS->meshAssets[ m_mid ] );
				m_mid = g_EdAS->meshAssets.size();
				MAcopy.outputName.append( " - Copy" );
				g_EdAS->meshAssets.push_back( MAcopy );
				Prepare( m_mid );
				return 1;
			}
			if( e->target == &m_btnDelete )
			{
				g_EdAS->meshAssets.erase( m_mid );
				m_mid = NOT_FOUND;
				FC_EditMeshList();
				return 1;
			}
			if( e->target == &m_MPLBtnAdd )
			{
				MA.parts.push_back( new SGRX_MeshAssetPart );
				ReloadPartList();
				m_frame->UpdateMouse();
			}
			if( e->target == &m_MPLEditButton )
			{
				PreparePart( m_MPLEditButton.id2 );
			}
			if( e->target == &m_MPLEditButton.m_up )
			{
				size_t i = m_MPLEditButton.id2;
				if( i > 0 )
					TSWAP( MA.parts[ i ], MA.parts[ i - 1 ] );
				ReloadPartList();
				m_frame->UpdateMouse();
				return 1;
			}
			if( e->target == &m_MPLEditButton.m_dn )
			{
				size_t i = m_MPLEditButton.id2;
				if( i < MA.parts.size() - 1 )
					TSWAP( MA.parts[ i ], MA.parts[ i + 1 ] );
				ReloadPartList();
				m_frame->UpdateMouse();
				return 1;
			}
			if( e->target == &m_MPLEditButton.m_del )
			{
				MA.parts.erase( m_MPLEditButton.id2 );
				m_pid = NOT_FOUND;
				ReloadPartList();
				m_frame->UpdateMouse();
				return 1;
			}
			break;
		}
	}
	return EDGUILayoutRow::OnEvent( e );
}

EDGUIAssetMeshList::EDGUIAssetMeshList() :
	m_group( true, "Mesh assets" ),
	m_editButton( false )
{
	m_btnAdd.caption = "Add mesh";
	m_filter.caption = "Filter";
	
	m_buttons.Add( &m_editButton );
	m_group.Add( &m_buttons );
	Add( &m_btnAdd );
	Add( &m_filter );
	Add( &m_group );
}

void EDGUIAssetMeshList::Prepare()
{
	m_buttons.m_options.clear();
	m_buttons.m_idTable.clear();
	m_buttons.m_options.reserve( g_EdAS->meshAssets.size() );
	m_buttons.m_idTable.reserve( g_EdAS->meshAssets.size() );
	for( size_t i = 0; i < g_EdAS->meshAssets.size(); ++i )
	{
		String name;
		g_EdAS->meshAssets[ i ].GetFullName( name );
		if( StringView(name).match_loose( m_filter.m_value ) )
		{
			g_EdAS->meshAssets[ i ].GetDesc( name );
			m_buttons.m_options.push_back( name );
			m_buttons.m_idTable.push_back( i );
		}
	}
	m_buttons.UpdateOptions();
}

int EDGUIAssetMeshList::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_filter )
		{
			Prepare();
		}
		break;
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

EDGUIAssetAnimBundle::EDGUIAssetAnimBundle() :
	m_group( true, "Animation bundle" ),
	m_outputCategory( &g_UIPickers->category ),
	m_previewMesh( &g_UIPickers->meshAsset ),
	m_AN_group( true, "Animation" ),
	m_AN_source( &g_UIPickers->animName ),
	m_ANL_group( true, "Animation list" ),
	m_ANL_editButton( false ),
	m_AS_group( true, "Animation source" ),
	m_AS_fileName( &g_UIPickers->assetPath ),
	m_ASL_group( true, "Animation source list" ),
	m_ASL_editButton( false ),
	m_abid( NOT_FOUND ),
	m_sid( NOT_FOUND ),
	m_aid( NOT_FOUND )
{
	m_outputCategory.m_requestReload = true;
	
	m_btnDuplicate.caption = "Duplicate";
	m_btnDelete.caption = "Delete";
	
	m_outputCategory.caption = "Output category";
	m_outputName.caption = "Output name";
	m_bundlePrefix.caption = "Bundle prefix";
	m_previewMesh.caption = "Preview mesh";
	
	m_AN_source.caption = "Source/name";
	m_AN_name.caption = "Name override";
	m_AN_startFrame.caption = "Start frame";
	m_AN_endFrame.caption = "End frame";
	
	m_ANL_btnAdd.caption = "Add animation";
	
	m_AS_fileName.caption = "Source file";
	m_AS_prefix.caption = "Prefix";
	m_AS_addAll.caption = "Add all animations from this source";
	
	m_ASL_btnAdd.caption = "Add animation source";
	
	m_topCol.Add( &m_btnDuplicate );
	m_topCol.Add( &m_btnDelete );
	
	m_group.Add( &m_outputCategory );
	m_group.Add( &m_outputName );
	m_group.Add( &m_bundlePrefix );
	m_group.Add( &m_previewMesh );
	
	m_AN_cont.Add( &m_AN_source );
	m_AN_cont.Add( &m_AN_name );
	m_AN_cont.Add( &m_AN_startFrame );
	m_AN_cont.Add( &m_AN_endFrame );
	
	m_ANL_buttons.Add( &m_ANL_editButton );
	m_ANL_group.Add( &m_ANL_btnAdd );
	m_ANL_group.Add( &m_ANL_buttons );
	
	m_AS_cont.Add( &m_AS_fileName );
	m_AS_cont.Add( &m_AS_prefix );
	m_AS_cont.Add( &m_AS_addAll );
	
	m_ASL_buttons.Add( &m_ASL_editButton );
	m_ASL_group.Add( &m_ASL_btnAdd );
	m_ASL_group.Add( &m_ASL_buttons );
	
	Add( &m_topCol );
	Add( &m_group );
	Add( &m_AN_group );
	Add( &m_ANL_group );
	Add( &m_AS_group );
	Add( &m_ASL_group );
}

void EDGUIAssetAnimBundle::UpdatePreviewAnim()
{
	SGRX_AnimBundleAsset& ABA = g_EdAS->animBundleAssets[ m_abid ];
	AnimHandle anim = SGRX_ProcessSingleAnim( ABA, m_aid );
	FC_SetAnim( g_EdAS->GetMesh( m_previewMesh.m_value ), anim );
}

void EDGUIAssetAnimBundle::Prepare( size_t abid )
{
	g_UIPickers->meshAsset.Reload();
	
	m_abid = abid;
	m_sid = NOT_FOUND;
	m_aid = NOT_FOUND;
	SGRX_AnimBundleAsset& ABA = g_EdAS->animBundleAssets[ abid ];
	
	m_outputCategory.SetValue( ABA.outputCategory );
	m_outputName.SetValue( ABA.outputName );
	m_bundlePrefix.SetValue( ABA.bundlePrefix );
	m_previewMesh.SetValue( ABA.previewMesh );
	
	ReloadAnimSourceList();
	PrepareAnimSource( NOT_FOUND );
	
	ReloadAnimList();
	PrepareAnim( NOT_FOUND );
}

void EDGUIAssetAnimBundle::ReloadAnimSourceList()
{
	SGRX_AnimBundleAsset& ABA = g_EdAS->animBundleAssets[ m_abid ];
	
	m_ASL_buttons.m_options.resize( ABA.sources.size() );
	for( size_t i = 0; i < ABA.sources.size(); ++i )
	{
		ABA.sources[ i ].GetDesc( m_ASL_buttons.m_options[ i ] );
	}
	m_ASL_buttons.UpdateOptions();
	
	ReloadImpScenes();
}

void EDGUIAssetAnimBundle::PrepareAnimSource( size_t sid )
{
	m_sid = sid;
	
	if( m_sid != NOT_FOUND )
	{
		const SGRX_ABAnimSource& AS = g_EdAS->animBundleAssets[ m_abid ].sources[ sid ];
		m_AS_fileName.SetValue( AS.file );
		m_AS_prefix.SetValue( AS.prefix );
		
		m_AS_group.Add( &m_AS_cont );
	}
	else
		m_AS_group.Clear();
}

void EDGUIAssetAnimBundle::ReloadAnimList()
{
	SGRX_AnimBundleAsset& ABA = g_EdAS->animBundleAssets[ m_abid ];
	
	m_ANL_buttons.m_options.resize( ABA.anims.size() );
	for( size_t i = 0; i < ABA.anims.size(); ++i )
	{
		ABA.anims[ i ].GetDesc( m_ANL_buttons.m_options[ i ] );
	}
	m_ANL_buttons.UpdateOptions();
	OnChangeLayout();
}

void EDGUIAssetAnimBundle::PrepareAnim( size_t aid )
{
	m_aid = aid;
	
	if( m_aid != NOT_FOUND )
	{
		const SGRX_ABAnimation& AN = g_EdAS->animBundleAssets[ m_abid ].anims[ aid ];
		m_AN_source.SetValue( AN.source );
		m_AN_name.SetValue( AN.name );
		
		char bfr[ 32 ];
		if( AN.startFrame != -1 )
		{
			sgrx_snprintf( bfr, 32, "%d", AN.startFrame );
			m_AN_startFrame.SetValue( bfr );
		}
		else
			m_AN_startFrame.SetValue( "" );
		if( AN.endFrame != -1 )
		{
			sgrx_snprintf( bfr, 32, "%d", AN.endFrame );
			m_AN_endFrame.SetValue( bfr );
		}
		else
			m_AN_endFrame.SetValue( "" );
		
		m_AN_group.Add( &m_AN_cont );
		
		UpdatePreviewAnim();
	}
	else
		m_AN_group.Clear();
}

void EDGUIAssetAnimBundle::ReloadImpScenes()
{
	SGRX_AnimBundleAsset& ABA = g_EdAS->animBundleAssets[ m_abid ];
	g_UIPickers->ClearAnimScenes();
	for( size_t i = 0; i < ABA.sources.size(); ++i )
	{
		g_UIPickers->AddAnimScene( new SGRX_Scene3D( ABA.sources[ i ].file, SIOF_Anims ) );
	}
	g_UIPickers->animName.Reload();
}

int EDGUIAssetAnimBundle::OnEvent( EDGUIEvent* e )
{
	if( m_abid != NOT_FOUND )
	{
		SGRX_AnimBundleAsset& ABA = g_EdAS->animBundleAssets[ m_abid ];
		SGRX_ABAnimSource* AS = m_sid != NOT_FOUND ? &ABA.sources[ m_sid ] : NULL;
		SGRX_ABAnimation* AN = m_aid != NOT_FOUND ? &ABA.anims[ m_aid ] : NULL;
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_outputCategory ){ ABA.outputCategory = m_outputCategory.m_value; }
			if( e->target == &m_outputName ){ ABA.outputName = m_outputName.m_value; }
			if( e->target == &m_bundlePrefix ){ ABA.bundlePrefix = m_bundlePrefix.m_value; }
			if( e->target == &m_previewMesh )
			{
				ABA.previewMesh = m_previewMesh.m_value;
				UpdatePreviewAnim();
			}
			if( AN )
			{
				bool edited = false;
				if( e->target == &m_AN_source ){ AN->source = m_AN_source.m_value; edited = true; }
				if( e->target == &m_AN_name ){ AN->name = m_AN_name.m_value; edited = true; }
				if( e->target == &m_AN_startFrame ){ edited = true;
					AN->startFrame = m_AN_startFrame.m_value.size() ? String_ParseInt( m_AN_startFrame.m_value ) : -1; }
				if( e->target == &m_AN_endFrame ){ edited = true;
					AN->endFrame = m_AN_endFrame.m_value.size() ? String_ParseInt( m_AN_endFrame.m_value ) : -1; }
				if( edited )
					ReloadAnimList();
			}
			if( AS )
			{
				bool edited = false;
				if( e->target == &m_AS_fileName ){ AS->file = m_AS_fileName.m_value; edited = true; }
				if( e->target == &m_AS_prefix ){ AS->prefix = m_AS_prefix.m_value; edited = true; }
				if( edited )
					ReloadAnimSourceList();
			}
			break;
		case EDGUI_EVENT_PROPCHANGE:
			if( e->target != &m_previewMesh )
			{
				ABA.ri.rev_asset++;
			}
			if( e->target == &m_AN_source || e->target == &m_AN_startFrame ||
				e->target == &m_AN_endFrame )
			{
				UpdatePreviewAnim();
			}
			break;
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btnDuplicate )
			{
				SGRX_AnimBundleAsset ABAcopy;
				ABAcopy.Clone( g_EdAS->animBundleAssets[ m_abid ] );
				m_abid = g_EdAS->animBundleAssets.size();
				ABAcopy.outputName.append( " - Copy" );
				g_EdAS->animBundleAssets.push_back( ABAcopy );
				Prepare( m_abid );
				return 1;
			}
			if( e->target == &m_btnDelete )
			{
				g_EdAS->animBundleAssets.erase( m_abid );
				m_abid = NOT_FOUND;
				FC_EditAnimBundleList();
				return 1;
			}
			if( e->target == &m_ANL_btnAdd )
			{
				ABA.anims.push_back( SGRX_ABAnimation() );
				ReloadAnimList();
				m_frame->UpdateMouse();
			}
			if( e->target == &m_ANL_editButton )
			{
				PrepareAnim( m_ANL_editButton.id2 );
			}
			if( e->target == &m_ANL_editButton.m_del )
			{
				ABA.anims.erase( m_ANL_editButton.id2 );
				m_aid = NOT_FOUND;
				ReloadAnimList();
				m_frame->UpdateMouse();
				m_AN_group.Clear();
				return 1;
			}
			if( e->target == &m_AS_addAll && AS )
			{
				ImpScene3DHandle sh = new SGRX_Scene3D( AS->file, SIOF_Anims );
				Array< String > anims;
				sh->GetAnimList( anims );
				for( size_t i = 0; i < anims.size(); ++i )
				{
					ABA.anims.push_back( SGRX_ABAnimation() );
					ABA.anims.last().source = anims[ i ];
				}
				ABA.ri.rev_asset++;
				ReloadAnimList();
				m_frame->UpdateMouse();
			}
			if( e->target == &m_ASL_btnAdd )
			{
				ABA.sources.push_back( SGRX_ABAnimSource() );
				ReloadAnimSourceList();
				m_frame->UpdateMouse();
			}
			if( e->target == &m_ASL_editButton )
			{
				PrepareAnimSource( m_ASL_editButton.id2 );
			}
			if( e->target == &m_ASL_editButton.m_del )
			{
				ABA.sources.erase( m_ASL_editButton.id2 );
				m_sid = NOT_FOUND;
				ReloadAnimSourceList();
				m_frame->UpdateMouse();
				m_AS_group.Clear();
				return 1;
			}
			break;
		}
	}
	return EDGUILayoutRow::OnEvent( e );
}

EDGUIAssetAnimBundleList::EDGUIAssetAnimBundleList() :
	m_group( true, "Anim. bundle assets" ),
	m_editButton( false )
{
	m_btnAdd.caption = "Add anim. bundle";
	m_filter.caption = "Filter";
	
	m_buttons.Add( &m_editButton );
	m_group.Add( &m_buttons );
	Add( &m_btnAdd );
	Add( &m_filter );
	Add( &m_group );
}

void EDGUIAssetAnimBundleList::Prepare()
{
	m_buttons.m_options.clear();
	m_buttons.m_idTable.clear();
	m_buttons.m_options.reserve( g_EdAS->animBundleAssets.size() );
	m_buttons.m_idTable.reserve( g_EdAS->animBundleAssets.size() );
	for( size_t i = 0; i < g_EdAS->animBundleAssets.size(); ++i )
	{
		String name;
		g_EdAS->animBundleAssets[ i ].GetFullName( name );
		if( StringView(name).match_loose( m_filter.m_value ) )
		{
			g_EdAS->animBundleAssets[ i ].GetDesc( name );
			m_buttons.m_options.push_back( name );
			m_buttons.m_idTable.push_back( i );
		}
	}
	m_buttons.UpdateOptions();
}

int EDGUIAssetAnimBundleList::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_filter )
		{
			Prepare();
		}
		break;
	case EDGUI_EVENT_BTNCLICK:
		if( e->target == &m_btnAdd )
		{
			g_EdAS->animBundleAssets.push_back( SGRX_AnimBundleAsset() );
			FC_EditAnimBundle( g_EdAS->animBundleAssets.size() - 1 );
			return 1;
		}
		if( e->target == &m_editButton )
		{
			FC_EditAnimBundle( m_editButton.id2 );
			return 1;
		}
		if( e->target == &m_editButton.m_del )
		{
			g_EdAS->animBundleAssets.erase( m_editButton.id2 );
			Prepare();
			return 1;
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}

EDGUIAssetCategoryForm::EDGUIAssetCategoryForm() :
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

void EDGUIAssetCategoryForm::_UpdateButtonText()
{
	RCString path = g_EdAS->categories.getcopy( m_name.m_value );
	if( StringView(m_name.m_value) == "" )
	{
		m_btnSave.caption = "-- cannot save unnamed category --";
	}
	else if( m_name.m_value == m_origName )
	{
		m_btnSave.caption = "Save category";
	}
	else if( path.size() )
	{
		m_btnSave.caption = String_Concat( "Overwrite category path: ", path );
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

void EDGUIAssetCategoryForm::Prepare( const StringView& name )
{
	m_origName = name;
	m_name.SetValue( name );
	RCString path = g_EdAS->categories.getcopy( name );
	m_path.SetValue( path );
	_UpdateButtonText();
}

int EDGUIAssetCategoryForm::OnEvent( EDGUIEvent* e )
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
			FC_EditCatList();
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}

EDGUIAssetCategoryList::EDGUIAssetCategoryList() :
	m_cgroup( true, "Categories" ),
	m_ctgEditButton( false )
{
	m_ctgBtnAdd.caption = "Add category";
	m_filter.caption = "Filter";
	
	m_ctgButtons.Add( &m_ctgEditButton );
	
	m_cgroup.Add( &m_ctgButtons );
	
	Add( &m_ctgBtnAdd );
	Add( &m_filter );
	Add( &m_cgroup );
}

void EDGUIAssetCategoryList::Prepare()
{
	m_ctgButtons.m_options.clear();
	m_ctgButtons.m_idTable.clear();
	m_ctgButtons.m_options.reserve( g_EdAS->categories.size() );
	m_ctgButtons.m_idTable.reserve( g_EdAS->categories.size() );
	for( size_t i = 0; i < g_EdAS->categories.size(); ++i )
	{
		char bfr[ 256 ];
		sgrx_snprintf( bfr, 256, "%s => %s",
			StackString<100>( g_EdAS->categories.item( i ).key ).str,
			StackString<100>( g_EdAS->categories.item( i ).value ).str );
		if( StringView(bfr).match_loose( m_filter.m_value ) )
		{
			m_ctgButtons.m_options.push_back( bfr );
			m_ctgButtons.m_idTable.push_back( i );
		}
	}
	m_ctgButtons.UpdateOptions();
}

int EDGUIAssetCategoryList::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_filter ) Prepare();
		break;
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


EDGUIMainFrame::EDGUIMainFrame() :
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
	m_MBForceRun.caption = "Force Run";
	m_MB_Cat1.caption = "Edit:";
	m_MBEditScript.caption = "Categories";
	m_MBEditTextures.caption = "Textures";
	m_MBEditMeshes.caption = "Meshes";
	m_MBEditAnimBundles.caption = "Anim. bundles";
	m_UIMenuButtons.Add( &m_MB_Cat0 );
	m_UIMenuButtons.Add( &m_MBSave );
	m_UIMenuButtons.Add( &m_MBRun );
//	m_UIMenuButtons.Add( &m_MBForceRun );
	m_UIMenuButtons.Add( &m_MB_Cat1 );
	m_UIMenuButtons.Add( &m_MBEditScript );
	m_UIMenuButtons.Add( &m_MBEditTextures );
	m_UIMenuButtons.Add( &m_MBEditMeshes );
	m_UIMenuButtons.Add( &m_MBEditAnimBundles );
	
	m_meshPrevInst = g_EdScene->CreateMeshInstance();
	lmm_prepmeshinst( m_meshPrevInst );
	
	ASCR_Open();
}

int EDGUIMainFrame::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_BTNCLICK:
		if(0);
		
		else if( e->target == &m_MBSave ) ASCR_Save();
		else if( e->target == &m_MBRun ) ASCR_Run( false );
		else if( e->target == &m_MBForceRun ) ASCR_Run( true );
		
		else if( e->target == &m_MBEditScript )
		{
			EditCatList();
		}
		else if( e->target == &m_MBEditTextures )
		{
			EditTextureList();
		}
		else if( e->target == &m_MBEditMeshes )
		{
			EditMeshList();
		}
		else if( e->target == &m_MBEditAnimBundles )
		{
			EditAnimBundleList();
		}
		
		return 1;
	}
	return EDGUIFrame::OnEvent( e );
}

void EDGUIMainFrame::DebugDraw()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	if( m_texPreview )
	{
		const TextureInfo& info = m_texPreview.GetInfo();
		float aspect = safe_fdiv( info.width, info.height );
		float w = 2 * TMAX( 1.0f, aspect );
		float h = 2 / TMIN( 1.0f, aspect );
		br.Reset();
		br.SetTexture( m_texPreview );
		br.Box( 0, 0, w, h );
	}
	
	SGRX_IMesh* mesh = m_meshPrevInst->GetMesh();
	if( mesh && m_meshPrevInst->enabled )
	{
		br.Reset();
		if( m_meshPrevInst->skin_matrices.size() )
		{
			for( size_t i = 0; i < m_meshPrevInst->skin_matrices.size(); ++i )
			{
				float sxt = ( 1 - float(i) / m_meshPrevInst->skin_matrices.size() );
				br.Axis( mesh->m_bones[ i ].skinOffset * m_meshPrevInst->skin_matrices[ i ], 0.1f + sxt * 0.1f );
			}
		}
		else
		{
			for( int i = 0; i < mesh->m_numBones; ++i )
			{
				float sxt = ( 1 - float(i) / mesh->m_numBones );
				br.Axis( mesh->m_bones[ i ].skinOffset, 0.1f + sxt * 0.1f );
			}
		}
	}
}

void EDGUIMainFrame::AddToParamList( EDGUIItem* item )
{
	m_UIParamList.Add( item );
}
void EDGUIMainFrame::ClearParamList()
{
	while( m_UIParamList.m_subitems.size() )
		m_UIParamList.Remove( m_UIParamList.m_subitems.last() );
}

void EDGUIMainFrame::EditTexture( size_t id )
{
	ClearParamList();
	m_UITexture.Prepare( id );
	AddToParamList( &m_UITexture );
}
void EDGUIMainFrame::EditTextureList()
{
	ClearParamList();
	m_UITextureList.Prepare();
	AddToParamList( &m_UITextureList );
}
void EDGUIMainFrame::EditMesh( size_t id )
{
	ClearParamList();
	m_UIMesh.Prepare( id );
	AddToParamList( &m_UIMesh );
}
void EDGUIMainFrame::EditMeshList()
{
	ClearParamList();
	m_UIMeshList.Prepare();
	AddToParamList( &m_UIMeshList );
}
void EDGUIMainFrame::EditAnimBundle( size_t id )
{
	ClearParamList();
	m_UIAnimBundle.Prepare( id );
	AddToParamList( &m_UIAnimBundle );
}
void EDGUIMainFrame::EditAnimBundleList()
{
	ClearParamList();
	m_UIAnimBundleList.Prepare();
	AddToParamList( &m_UIAnimBundleList );
}
void EDGUIMainFrame::EditCategory( const StringView& name )
{
	ClearParamList();
	m_UICategory.Prepare( name );
	AddToParamList( &m_UICategory );
}
void EDGUIMainFrame::EditCatList()
{
	ClearParamList();
	m_UICatList.Prepare();
	AddToParamList( &m_UICatList );
}

void EDGUIMainFrame::ResetEditorState()
{
	EditCatList();
}
void EDGUIMainFrame::ASCR_Open()
{
	LOG << "Trying to open asset script";
	
	if( g_EdAS->Load( ASSET_SCRIPT_NAME ) == false )
	{
		LOG_ERROR << "FAILED TO LOAD ASSET SCRIPT";
		return;
	}
	g_EdAS->LoadAssetInfo( ASSET_INFO_NAME );
	
	ResetEditorState();
}
void EDGUIMainFrame::ASCR_Save()
{
	LOG << "Trying to save asset script";
	
	if( g_EdAS->Save( ASSET_SCRIPT_NAME ) == false )
	{
		LOG_ERROR << "FAILED TO SAVE ASSET SCRIPT";
		return;
	}
	if( g_EdAS->SaveAssetInfo( ASSET_INFO_NAME ) == false )
	{
		LOG_ERROR << "FAILED TO SAVE ASSET INFO";
		return;
	}
}
void EDGUIMainFrame::ASCR_Run( bool force )
{
	g_EdAS->LoadOutputInfo( OUTPUT_INFO_NAME );
	SGRX_ProcessAssets( *g_EdAS, force );
	g_EdAS->SaveOutputInfo( OUTPUT_INFO_NAME );
}



struct ASEditor : IGame
{
	bool OnConfigure( int argc, char** argv )
	{
		int op = 0;
		const char* cat = NULL;
		for( int i = 1; i < argc; ++i )
		{
			if( !strcmp( argv[ i ], "--dtex" ) ) op = 1;
			if( !strcmp( argv[ i ], "--gmtl" ) ) op = 2;
			if( !strncmp( argv[ i ], "--cat=", 6 ) )
				cat = argv[ i ] + 6;
			else if( !strcmp( argv[ i ], "-c" ) && i + 1 < argc )
				cat = argv[ ++i ];
		}
		switch( op )
		{
		case 1: // dump textures
			{
				SGRX_AssetScript as;
				as.Load( ASSET_SCRIPT_NAME );
				for( size_t i = 0; i < as.textureAssets.size(); ++i )
				{
					SGRX_TextureAsset& TA = as.textureAssets[ i ];
					if( cat )
					{
						if( TA.outputCategory != StringView(cat) )
							continue;
					}
					printf( "%s/%s.%s\n",
						StackString<256>(as.GetCategoryPath(TA.outputCategory)).str,
						StackString<256>(TA.outputName).str,
						SGRX_TextureOutputFormat_Ext( TA.outputType ) );
				}
			}
			return false;
		case 2: // generate materials
			{
				SGRX_AssetScript as;
				as.Load( ASSET_SCRIPT_NAME );
				for( size_t i = 0; i < as.textureAssets.size(); ++i )
				{
					SGRX_TextureAsset& TA = as.textureAssets[ i ];
					if( cat )
					{
						if( TA.outputCategory != StringView(cat) )
							continue;
					}
					printf( "material %s\nshader default\n0 %s/%s.%s\n",
						StackString<256>(TA.outputName).str,
						StackString<256>(as.GetCategoryPath(TA.outputCategory)).str,
						StackString<256>(TA.outputName).str,
						SGRX_TextureOutputFormat_Ext( TA.outputType ) );
				}
			}
			return false;
		}
		return true;
	}
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
		g_UIPickers = new EDGUIPickers;
		
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
		delete g_UIPickers;
		g_UIPickers = NULL;
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
		AnimInfo info;
		GR_ApplyAnimator( &g_UIFrame->m_animPreview, g_UIFrame->m_meshPrevInst );
		g_UIFrame->m_animPreview.Advance( dt, &info );
		g_UIFrame->m_UIRenderView.UpdateCamera( dt );
		g_UIFrame->Draw();
	}
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

