

#include <engine.hpp>
#include <enganim.hpp>
#include <edgui.hpp>
#include "edcomui.hpp"

#include "assetcomp.hpp"


#define ASSET_SCRIPT_NAME "assets.txt"


struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
SGRX_AssetScript* g_EdAS;
struct EDGUIPickers* g_UIPickers;

EDGUIRsrcPicker TMPRSRC;


#define SGRX_GET_FLAG( v, fl ) (((v) & (fl)) != 0)
#define SGRX_SET_FLAG( v, fl, b ) v = ((v) & ~(fl)) | ((b)?(fl):0)


struct EDGUISmallEnumPicker : EDGUIRsrcPicker
{
	void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
};

struct EDGUILongEnumPicker : EDGUIRsrcPicker
{
	void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemWidth = 300;
		m_itemHeight = 16;
	}
};

struct EDGUIMtlBlendMode : EDGUISmallEnumPicker
{
	EDGUIMtlBlendMode()
	{
		caption = "Pick the blending mode";
		m_options.push_back( "None" );
		m_options.push_back( "Basic" );
		m_options.push_back( "Additive" );
		m_options.push_back( "Multiply" );
		_Search( m_searchString );
	}
};

struct EDGUIImageFilterSharpenMode : EDGUISmallEnumPicker
{
	EDGUIImageFilterSharpenMode()
	{
		caption = "Pick the sharpening mode";
		m_options.push_back( "0-1" );
		m_options.push_back( "1-1" );
		m_options.push_back( "1-2" );
		_Search( m_searchString );
	}
};

struct EDGUIImageFilterType : EDGUISmallEnumPicker
{
	EDGUIImageFilterType()
	{
		caption = "Pick an image filter";
		m_options.push_back( "Resize" );
		m_options.push_back( "Sharpen" );
		m_options.push_back( "To linear" );
		m_options.push_back( "From linear" );
		m_options.push_back( "Expand range" );
		m_options.push_back( "Brightness/contrast/power" );
		_Search( m_searchString );
	}
	SGRX_AssetImageFilterType GetPickedType() const
	{
		return SGRX_AssetImageFilterType( m_picked + 1 );
	}
};

struct EDGUITextureOutputFormat : EDGUISmallEnumPicker
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
};

struct EDGUICategoryPicker : EDGUISmallEnumPicker
{
	EDGUICategoryPicker()
	{
		m_looseSearch = true;
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
};

struct EDGUITextureAssetPicker : EDGUILongEnumPicker
{
	EDGUITextureAssetPicker()
	{
		m_looseSearch = true;
		Reload();
	}
	void Reload()
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
};

struct EDGUIAssetPathPicker : EDGUILongEnumPicker, IDirEntryHandler
{
	EDGUIAssetPathPicker() : m_depth(0)
	{
		m_looseSearch = true;
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
	int m_depth;
};

struct EDGUIASMeshNamePicker : EDGUILongEnumPicker
{
	EDGUIASMeshNamePicker()
	{
		m_looseSearch = true;
		Reload();
	}
	void Reload()
	{
		m_options.clear();
		if( m_scene )
		{
			m_scene->GetMeshList( m_options );
		}
		_Search( m_searchString );
	}
	ImpScene3DHandle m_scene;
};

struct EDGUIPickers
{
	EDGUIMtlBlendMode blendMode;
	EDGUIImageFilterSharpenMode sharpenMode;
	EDGUIImageFilterType imageFilterType;
	EDGUITextureOutputFormat textureOutputFormat;
	EDGUICategoryPicker category;
	EDGUITextureAssetPicker textureAsset;
	EDGUIAssetPathPicker assetPath;
	EDGUIASMeshNamePicker meshName;
	EDGUIShaderPicker shader;
	
	void SetMesh( SGRX_Scene3D* scene )
	{
		meshName.m_scene = scene;
	}
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


struct EDGUIImgFilterBase : EDGUILayoutRow
{
	EDGUIImgFilterBase( SGRX_ImageFilter* iflt, bool addCMF = false ) :
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
	
	void AddCMFFields()
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
	
	virtual int OnEvent( EDGUIEvent* e )
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
	
	EDGUIPropFloat m_blend;
	EDGUIPropBool m_cclamp;
	EDGUIPropBool m_applyR;
	EDGUIPropBool m_applyG;
	EDGUIPropBool m_applyB;
	EDGUIPropBool m_applyA;
	SGRX_ImgFilterHandle m_hfilter;
};

struct EDGUIImgFilter_Resize : EDGUIImgFilterBase
{
	EDGUIImgFilter_Resize( SGRX_ImageFilter* iflt ) :
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
				if( e->target == &m_srgb ) F->srgb = m_srgb.m_value;
				break;
			}
		}
		return EDGUIImgFilterBase::OnEvent( e );
	}
	
	EDGUIPropInt m_width;
	EDGUIPropInt m_height;
	EDGUIPropBool m_srgb;
};

struct EDGUIImgFilter_Sharpen : EDGUIImgFilterBase
{
	EDGUIImgFilter_Sharpen( SGRX_ImageFilter* iflt ) :
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
	
	virtual int OnEvent( EDGUIEvent* e )
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
	
	EDGUIPropFloat m_factor;
	EDGUIPropRsrc m_mode;
};

struct EDGUIImgFilter_ExpandRange : EDGUIImgFilterBase
{
	EDGUIImgFilter_ExpandRange( SGRX_ImageFilter* iflt ) :
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
	
	virtual int OnEvent( EDGUIEvent* e )
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
	
	EDGUIPropVec4 m_vmin;
	EDGUIPropVec4 m_vmax;
};

struct EDGUIImgFilter_BCP : EDGUIImgFilterBase
{
	EDGUIImgFilter_BCP( SGRX_ImageFilter* iflt ) :
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
	
	virtual int OnEvent( EDGUIEvent* e )
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
	
	EDGUIPropBool m_apply_bc1;
	EDGUIPropFloat m_brightness;
	EDGUIPropFloat m_contrast;
	EDGUIPropBool m_apply_pow;
	EDGUIPropFloat m_power;
	EDGUIPropBool m_apply_bc2;
	EDGUIPropFloat m_brightness_2;
	EDGUIPropFloat m_contrast_2;
};


void FC_SetTexture( TextureHandle tex );
void FC_SetMesh( MeshHandle mesh );
void FC_EditTexture( size_t id );
void FC_EditTextureList();
void FC_EditMesh( size_t id );
void FC_EditMeshList();

struct EDGUIAssetTexture : EDGUILayoutRow
{
	EDGUIAssetTexture() :
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
		
		UpdatePreviewTexture();
	}
	
	void UpdatePreviewTexture()
	{
		SGRX_TextureAsset& TA = g_EdAS->textureAssets[ m_tid ];
		TextureHandle tex;
		SGRX_IFP32Handle img = SGRX_ProcessTextureAsset( TA );
		tex = SGRX_FP32ToTexture( img, TA );
		if( tex == NULL )
			tex = GR_GetTexture( "textures/unit.png" );
		FC_SetTexture( tex );
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
	
	void EditFilter( SGRX_ImageFilter* IF )
	{
		EDGUILayoutRow* newflt = NULL;
		switch( IF->GetType() )
		{
		case SGRX_AIF_Resize: newflt = new EDGUIImgFilter_Resize( IF ); break;
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
				UpdatePreviewTexture();
				break;
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_btnDuplicate )
				{
					SGRX_TextureAsset TAcopy = g_EdAS->textureAssets[ m_tid ];
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
	
	EDGUILayoutColumn m_topCol;
	EDGUIButton m_btnDuplicate;
	EDGUIButton m_btnDelete;
	EDGUIGroup m_group;
	EDGUIPropRsrc m_sourceFile;
	EDGUIPropRsrc m_outputCategory;
	EDGUIPropString m_outputName;
	EDGUIPropRsrc m_outputType;
	EDGUIPropBool m_isSRGB;
	EDGUIPropBool m_mips;
	EDGUIPropBool m_lerp;
	EDGUIPropBool m_clampx;
	EDGUIPropBool m_clampy;
	EDGUILayoutColumn m_columnList;
	EDGUIGroup m_sfgroup;
	EDGUILayoutRow* m_curFilter;
	EDGUIGroup m_flgroup;
	EDGUICreatePickButton m_filterBtnAdd;
	EDGUIBtnList m_filterButtons;
	EDGUIListItemButton m_filterEditButton;
	size_t m_tid;
};

struct EDGUIAssetTextureList : EDGUILayoutRow
{
	EDGUIAssetTextureList() :
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
	
	void Prepare()
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
	
	virtual int OnEvent( EDGUIEvent* e )
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
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIPropString m_filter;
	EDGUIBtnList m_buttons;
	EDGUIListItemButton m_editButton;
};

struct EDGUIAssetMesh : EDGUILayoutRow
{
	EDGUIAssetMesh() :
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
		
		m_group.Add( &m_sourceFile );
		m_group.Add( &m_outputCategory );
		m_group.Add( &m_outputName );
		m_group.Add( &m_rotateY2Z );
		m_group.Add( &m_flipUVY );
		
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
		
		Add( &m_topCol );
		Add( &m_group );
		Add( &m_MPgroup );
		Add( &m_MPLgroup );
	}
	
	void UpdatePreviewMesh()
	{
		SGRX_MeshAsset& MA = g_EdAS->meshAssets[ m_mid ];
		MeshHandle mesh = SGRX_ProcessMeshAsset( g_EdAS, MA );
		FC_SetMesh( mesh );
	}
	
	void ReloadPartList()
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
	
	void ReloadImpScene()
	{
		g_UIPickers->SetMesh( new SGRX_Scene3D( m_sourceFile.m_value ) );
	}
	
	void Prepare( size_t mid )
	{
		g_UIPickers->textureAsset.Reload();
		
		m_mid = mid;
		SGRX_MeshAsset& MA = g_EdAS->meshAssets[ mid ];
		
		m_sourceFile.SetValue( MA.sourceFile );
		m_outputCategory.SetValue( MA.outputCategory );
		m_outputName.SetValue( MA.outputName );
		m_rotateY2Z.SetValue( MA.rotateY2Z );
		m_flipUVY.SetValue( MA.flipUVY );
		
		ReloadPartList();
		PreparePart( NOT_FOUND );
		
		ReloadImpScene();
	}
	
	void PreparePart( size_t pid )
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
		m_MPgroup.Clear();
		if( MP )
			m_MPgroup.Add( &m_MPart );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
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
					if( ed )
					{
						ReloadPartList();
					}
				}
				break;
			case EDGUI_EVENT_PROPCHANGE:
				if( e->target == &m_sourceFile ){ ReloadImpScene(); }
				break;
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_btnDuplicate )
				{
					SGRX_MeshAsset MAcopy = g_EdAS->meshAssets[ m_mid ];
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
	
	EDGUILayoutColumn m_topCol;
	EDGUIButton m_btnDuplicate;
	EDGUIButton m_btnDelete;
	EDGUIGroup m_group;
	EDGUIPropRsrc m_sourceFile;
	EDGUIPropRsrc m_outputCategory;
	EDGUIPropString m_outputName;
	EDGUIPropBool m_rotateY2Z;
	EDGUIPropBool m_flipUVY;
	EDGUIGroup m_MPgroup;
	EDGUILayoutRow m_MPart;
	EDGUIPropRsrc m_MPmeshName;
	EDGUIPropRsrc m_MPshader;
	EDGUIPropRsrc m_MPtexture0;
	EDGUIPropRsrc m_MPtexture1;
	EDGUIPropRsrc m_MPtexture2;
	EDGUIPropRsrc m_MPtexture3;
	EDGUIPropRsrc m_MPtexture4;
	EDGUIPropRsrc m_MPtexture5;
	EDGUIPropRsrc m_MPtexture6;
	EDGUIPropRsrc m_MPtexture7;
	EDGUIPropBool m_MPmtlUnlit;
	EDGUIPropBool m_MPmtlNocull;
	EDGUIPropBool m_MPmtlDecal;
	EDGUIPropRsrc m_MPmtlBlendMode;
	EDGUIGroup m_MPLgroup;
	EDGUIButton m_MPLBtnAdd;
	EDGUIBtnList m_MPLButtons;
	EDGUIListItemButton m_MPLEditButton;
	size_t m_mid;
	size_t m_pid;
	ImpScene3DHandle m_scene;
};

struct EDGUIAssetMeshList : EDGUILayoutRow
{
	EDGUIAssetMeshList() :
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
	
	void Prepare()
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
	
	virtual int OnEvent( EDGUIEvent* e )
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
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIPropString m_filter;
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
		
		m_meshPrevInst = g_EdScene->CreateMeshInstance();
		lmm_prepmeshinst( m_meshPrevInst );
		
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
		if( m_texPreview )
		{
			const TextureInfo& info = m_texPreview.GetInfo();
			float aspect = safe_fdiv( info.width, info.height );
			float w = 2 * TMAX( 1.0f, aspect );
			float h = 2 / TMIN( 1.0f, aspect );
			BatchRenderer& br = GR2D_GetBatchRenderer();
			br.Reset();
			br.SetTexture( m_texPreview );
			br.Box( 0, 0, w, h );
		}
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
		
		if( g_EdAS->Save( ASSET_SCRIPT_NAME ) == false )
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
	
	// preview data
	TextureHandle m_texPreview;
	MeshInstHandle m_meshPrevInst;
};

void FC_EditTexture( size_t id ){ g_UIFrame->EditTexture( id ); }
void FC_EditTextureList(){ g_UIFrame->EditTextureList(); }
void FC_EditMesh( size_t id ){ g_UIFrame->EditMesh( id ); }
void FC_EditMeshList(){ g_UIFrame->EditMeshList(); }
void FC_EditCategory( const StringView& name ){ g_UIFrame->EditCategory( name ); }
void FC_EditScript(){ g_UIFrame->EditScript(); }
void FC_SetTexture( TextureHandle tex )
{
	g_UIFrame->m_texPreview = tex;
	g_UIFrame->m_meshPrevInst->enabled = false;
}
void FC_SetMesh( MeshHandle mesh )
{
	g_UIFrame->m_texPreview = NULL;
	g_UIFrame->m_meshPrevInst->SetMesh( mesh );
	g_UIFrame->m_meshPrevInst->enabled = mesh != NULL;
}



struct ASEditor : IGame
{
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
	//	g_UITexPicker = new EDGUISDTexPicker( "textures/particles" );
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

