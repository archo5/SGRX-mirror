

#include <engine.hpp>
#include <enganim.hpp>
#include <imgui.hpp>
#include "assetcomp.hpp"


#define ASSET_SCRIPT_NAME SGRXPATH_SRC "/sys/assets.txt"
#define ASSET_INFO_NAME SGRXPATH_SRC "/sys/asset_revs.info"
#define OUTPUT_INFO_NAME SGRXPATH_SRC "/sys/output_revs.info"


SceneHandle g_EdScene;
SGRX_AssetScript* g_EdAS;
SGRX_Asset* g_CurAsset;


struct AssetRenderView* g_NUIRenderView;
struct IMGUIFilePicker* g_NUIAssetPicker;


enum EditorMode
{
	EditAssets,
	EditCategories,
	MiscProps,
};
int g_mode = EditAssets;



struct AssetRenderView : IMGUIRenderView
{
	AssetRenderView() : IMGUIRenderView( g_EdScene )
	{
		m_meshPrevInst = m_scene->CreateMeshInstance();
		lmm_prepmeshinst( m_meshPrevInst );
	}
	void DebugDraw()
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
	
	// preview data
	TextureHandle m_texPreview;
	MeshInstHandle m_meshPrevInst;
	Array< AnimTrackFrame > m_animFrames;
	AnimPlayer m_animPreview;
};

void FC_SetTexture( TextureHandle tex )
{
	g_NUIRenderView->m_texPreview = tex;
	g_NUIRenderView->m_meshPrevInst->enabled = false;
	g_NUIRenderView->m_meshPrevInst->skin_matrices.resize( 0 );
	g_NUIRenderView->m_animPreview.m_mesh = NULL;
	g_NUIRenderView->m_animPreview.Stop();
}
void FC_SetMesh( MeshHandle mesh )
{
	g_NUIRenderView->m_texPreview = NULL;
	g_NUIRenderView->m_meshPrevInst->SetMesh( mesh );
	g_NUIRenderView->m_meshPrevInst->enabled = mesh != NULL;
	g_NUIRenderView->m_meshPrevInst->skin_matrices.resize( 0 );
	g_NUIRenderView->m_animPreview.m_mesh = NULL;
	g_NUIRenderView->m_animPreview.Stop();
}
void FC_SetAnim( MeshHandle mesh, AnimHandle anim )
{
	g_NUIRenderView->m_texPreview = NULL;
	g_NUIRenderView->m_meshPrevInst->SetMesh( mesh );
	g_NUIRenderView->m_meshPrevInst->enabled = mesh != NULL;
	g_NUIRenderView->m_meshPrevInst->skin_matrices.resize( mesh.GetBoneCount() );
	g_NUIRenderView->m_animFrames.resize( mesh.GetBoneCount() );
	g_NUIRenderView->m_animPreview.m_mesh = mesh;
	g_NUIRenderView->m_animPreview.m_pose = g_NUIRenderView->m_animFrames;
	g_NUIRenderView->m_animPreview.Prepare();
	g_NUIRenderView->m_animPreview.Stop();
	g_NUIRenderView->m_animPreview.Play( anim );
}


//
// ASSET
//

bool PickCategoryName( const char* label, String& name )
{
	int id = 0;
	String namelist = "<None>";
	namelist.push_back( '\0' );
	for( size_t i = 0; i < g_EdAS->categories.size(); ++i )
	{
		if( g_EdAS->categories.item( i ).key == name )
			id = i + 1;
		namelist.append( g_EdAS->categories.item( i ).key );
		namelist.append( " (" );
		namelist.append( g_EdAS->categories.item( i ).value );
		namelist.append( ")" );
		namelist.push_back( '\0' );
	}
	namelist.push_back( '\0' );
	bool ret = ImGui::Combo( label, &id, namelist.data() );
	if( ret )
	{
		if( id > 0 && id <= (int) g_EdAS->categories.size() )
			name = g_EdAS->categories.item( id - 1 ).key;
		else
			name = "";
	}
	return ret;
}

void EditFilter( size_t i, SGRX_ImageFilter* IF )
{
	static const char* sharpen_modes[] = { "0-1", "1-1", "1-2" };
	
	switch( IF->GetType() )
	{
	case SGRX_AIF_Resize: {
		SGRX_CAST( SGRX_ImageFilter_Resize*, F, IF );
		IMGUI_GROUP( "Resize", true,
		{
			IMGUIEditInt( "Width", F->width, 1, 4096 );
			IMGUIEditInt( "Height", F->height, 1, 4096 );
			IMGUIEditInt( "Depth", F->depth, 1, 4096 );
			IMGUIEditBool( "SRGB", F->srgb );
		});
		} break;
	case SGRX_AIF_Rearrange: {
		SGRX_CAST( SGRX_ImageFilter_Rearrange*, F, IF );
		IMGUI_GROUP( "Rearrange", true,
		{
			IMGUIEditInt( "Width", F->width, 1, 4096 );
		});
		} break;
	case SGRX_AIF_Sharpen: {
		SGRX_CAST( SGRX_ImageFilter_Sharpen*, F, IF );
		IMGUI_GROUP( "Sharpen", true,
		{
			IMGUIEditFloat( "Factor", F->factor, 0, 1 );
			IMGUI_COMBOBOX( "Mode", F->mode, sharpen_modes );
		});
		} break;
	case SGRX_AIF_ToLinear: {
		IMGUI_GROUP( "To linear", true, { ImGui::Text( "- no parameters -" ); } );
		} break;
	case SGRX_AIF_FromLinear: {
		IMGUI_GROUP( "From linear", true, { ImGui::Text( "- no parameters -" ); } );
		} break;
	case SGRX_AIF_ExpandRange: {
		SGRX_CAST( SGRX_ImageFilter_ExpandRange*, F, IF );
		IMGUI_GROUP( "Expand range", true,
		{
			IMGUIEditVec4( "Min. value", F->vmin, -1000, 1000 );
			IMGUIEditVec4( "Max. value", F->vmax, -1000, 1000 );
		});
		} break;
	case SGRX_AIF_BCP: {
		SGRX_CAST( SGRX_ImageFilter_BCP*, F, IF );
		IMGUI_GROUP( "Brightness/contrast/power", true,
		{
			IMGUIEditBool( "Apply brightness/contrast 1", F->apply_bc1 );
			IMGUIEditFloat( "Brightness 1", F->brightness, -10, 10 );
			IMGUIEditFloat( "Contrast 1", F->contrast, 0.01f, 100 );
			IMGUIEditBool( "Apply power", F->apply_pow );
			IMGUIEditFloat( "Power", F->power, 0.01f, 100 );
			IMGUIEditBool( "Apply brightness/contrast 2", F->apply_bc2 );
			IMGUIEditFloat( "Brightness 2", F->brightness_2, -10, 10 );
			IMGUIEditFloat( "Contrast 2", F->contrast_2, 0.01f, 100 );
		});
		} break;
	default: ImGui::Text( "<UNKNOWN FILTER>" ); break;
	}
}

void UpdateTexturePreview( SGRX_TextureAsset& ta )
{
	TextureHandle tex;
	SGRX_IFP32Handle img = SGRX_ProcessTextureAsset( ta );
	tex = SGRX_FP32ToTexture( img, ta );
	if( tex == NULL )
		tex = GR_GetTexture( "textures/unit.png" );
	FC_SetTexture( tex );
}

void EditTextureAsset( SGRX_TextureAsset& ta )
{
	ImGui::BeginChangeCheck();
	
	ImGui::Text( "Texture" );
	ImGui::Separator();
	
	g_NUIAssetPicker->Property( "Select source file", "Source file", ta.sourceFile );
	
	ImGui::BeginChangeMask( 1 );
	PickCategoryName( "Output category", ta.outputCategory );
	IMGUIEditString( "Output name", ta.outputName, 256 );
	ImGui::EndChangeMask();
	
	if( ImGui::Button( SGRX_TextureOutputFormat_ToString( ta.outputType ),
		ImVec2( ImGui::GetContentRegionAvailWidth() * 2.f/3.f, 20 ) ) )
		ImGui::OpenPopup( "output_type" );
	ImGui::SameLine();
	ImGui::Text( "Output type" );
	if( ImGui::BeginPopup( "output_type" ) )
	{
		if( ImGui::Selectable( SGRX_TextureOutputFormat_ToString( SGRX_TOF_PNG_RGBA32 ) ) )
		{
			ta.outputType = SGRX_TOF_PNG_RGBA32;
			ImGui::TriggerChangeCheck();
		}
		if( ImGui::Selectable( SGRX_TextureOutputFormat_ToString( SGRX_TOF_STX_RGBA32 ) ) )
		{
			ta.outputType = SGRX_TOF_STX_RGBA32;
			ImGui::TriggerChangeCheck();
		}
		ImGui::EndPopup();
	}
	
	IMGUIEditBool( "Is SRGB?", ta.isSRGB );
	IMGUIEditBool( "Generate mipmaps", ta.mips );
	IMGUIEditBool( "Use linear interpolation", ta.lerp );
	IMGUIEditBool( "Clamp X", ta.clampx );
	IMGUIEditBool( "Clamp Y", ta.clampy );
	
	ImGui::Separator();
	ImGui::Text( "Filters" );
	ImGui::Separator();
	
	if( ImGui::Button( "Add filter", ImVec2( ImGui::GetContentRegionAvail().x, 24 ) ) )
		ImGui::OpenPopup( "add_filter" );
	if( ImGui::BeginPopup( "add_filter" ) )
	{
		size_t ofs = ta.filters.size();
		ImGui::Text( "Select filter type:" );
		ImGui::Separator();
		if( ImGui::Selectable( "Resize" ) ) ta.filters.push_back( new SGRX_ImageFilter_Resize );
		if( ImGui::Selectable( "Rearrange" ) ) ta.filters.push_back( new SGRX_ImageFilter_Rearrange );
		if( ImGui::Selectable( "Sharpen" ) ) ta.filters.push_back( new SGRX_ImageFilter_Sharpen );
		if( ImGui::Selectable( "To linear" ) ) ta.filters.push_back( new SGRX_ImageFilter_Linear(false) );
		if( ImGui::Selectable( "From linear" ) ) ta.filters.push_back( new SGRX_ImageFilter_Linear(true) );
		if( ImGui::Selectable( "Expand range" ) ) ta.filters.push_back( new SGRX_ImageFilter_ExpandRange );
		if( ImGui::Selectable( "Brightness/contrast/power" ) ) ta.filters.push_back( new SGRX_ImageFilter_BCP );
		ImGui::EndPopup();
		if( ta.filters.size() != ofs )
			ImGui::TriggerChangeCheck();
	}
	IMGUIEditArray( ta.filters, EditFilter, NULL );
	
	int chg = ImGui::EndChangeCheck();
	if( chg & 1 )
		ta.ri.rev_asset++;
	if( chg & 2 )
		UpdateTexturePreview( ta );
}



struct TexturePicker : IMGUIEntryPicker
{
	void Reload()
	{
		m_entries.clear();
		m_entries.push_back( "" );
		for( size_t i = 0; i < g_EdAS->textureAssets.size(); ++i )
		{
			const SGRX_TextureAsset& TA = g_EdAS->textureAssets[ i ];
			String opt = TA.outputCategory;
			opt.append( "/" );
			opt.append( TA.outputName );
			m_entries.push_back( opt );
		}
		_Search( m_searchString );
	}
}
g_TexturePicker;

struct MeshPartPicker : IMGUIEntryPicker
{
	void Reload( StringView path )
	{
		m_entries.clear();
		Array< String > options;
		ImpScene3DHandle scene = new SGRX_Scene3D( path );
		if( scene )
		{
			scene->GetMeshList( options );
		}
		m_entries.resize( options.size() );
		for( size_t i = 0; i < options.size(); ++i )
			m_entries[ i ] = options[ i ];
		_Search( m_searchString );
	}
}
g_MeshAssetPartPicker;

IMGUIShaderPicker g_ShaderPicker;

void EditMeshPart( size_t i, SGRX_MeshAssetPart* mp )
{
	static const char* texLabels[ 8 ] =
	{
		"Texture 0", "Texture 1", "Texture 2", "Texture 3",
		"Texture 4", "Texture 5", "Texture 6", "Texture 7",
	};
	
	IMGUI_GROUP( "Mesh part", true,
	{
		g_MeshAssetPartPicker.Property( "Pick mesh part name", "Mesh name", mp->meshName );
		g_ShaderPicker.Property( "Shader", mp->shader );
		
		for( int i = 0; i < 8; ++i )
		{
			ImGui::PushID( i );
			g_TexturePicker.Property( "Pick a texture asset", texLabels[ i ], mp->textures[ i ] );
			ImGui::PopID();
		}
		
		IMGUI_GROUP( "Material flags", true,
		{
			IMGUIEditIntFlags( "Unlit", mp->mtlFlags, SGRX_MtlFlag_Unlit );
			IMGUIEditIntFlags( "Disable culling", mp->mtlFlags, SGRX_MtlFlag_Nocull );
			IMGUIEditIntFlags( "Decal", mp->mtlFlags, SGRX_MtlFlag_Decal );
			IMGUIEditIntFlags( "Vertex colors", mp->mtlFlags, SGRX_MtlFlag_VCol );
		});
		IMGUIComboBox( "Blend mode", mp->mtlBlendMode, "None\0Basic\0Additive\0Multiply\0" );
		
		ImGui::RadioButton( "Yes", &mp->optTransform, 1 );
		ImGui::SameLine();
		ImGui::RadioButton( "Default", &mp->optTransform, 0 );
		ImGui::SameLine();
		ImGui::RadioButton( "No", &mp->optTransform, -1 );
		ImGui::SameLine( 0, 50 );
		ImGui::Text( "Transform" );
	});
}

void UpdateMeshPreview( SGRX_MeshAsset& ma )
{
	MeshHandle mesh = SGRX_ProcessMeshAsset( g_EdAS, ma );
	FC_SetMesh( mesh );
}

void EditMeshAsset( SGRX_MeshAsset& ma )
{
	ImGui::BeginChangeCheck();
	
	ImGui::Text( "Mesh" );
	ImGui::Separator();
	
	bool src = g_NUIAssetPicker->Property( "Select source file", "Source file", ma.sourceFile );
	if( src )
		g_MeshAssetPartPicker.Reload( ma.sourceFile );
	
	ImGui::BeginChangeMask( 1 );
	PickCategoryName( "Output category", ma.outputCategory );
	IMGUIEditString( "Output name", ma.outputName, 256 );
	ImGui::EndChangeMask();
	
	IMGUIEditBool( "Rotate Y -> Z", ma.rotateY2Z );
	IMGUIEditBool( "Flip UV/Y", ma.flipUVY );
	IMGUIEditBool( "Transform", ma.transform );
	
	if( ImGui::Button( "Add mesh part", ImVec2( ImGui::GetContentRegionAvail().x, 24 ) ) )
	{
		ma.parts.push_back( new SGRX_MeshAssetPart );
		ImGui::TriggerChangeCheck();
	}
	IMGUIEditArray( ma.parts, EditMeshPart, NULL );
	
	int chg = ImGui::EndChangeCheck();
	if( chg & 1 )
		ma.ri.rev_asset++;
	if( chg & 2 )
		UpdateMeshPreview( ma );
}


struct MeshAssetPicker : IMGUIEntryPicker
{
	void Reload()
	{
		m_entries.clear();
		for( size_t i = 0; i < g_EdAS->meshAssets.size(); ++i )
		{
			const SGRX_MeshAsset& TA = g_EdAS->meshAssets[ i ];
			String opt = TA.outputCategory;
			opt.append( "/" );
			opt.append( TA.outputName );
			m_entries.push_back( opt );
		}
		_Search( m_searchString );
	}
}
g_MeshAssetPicker;

size_t g_CurAnim = NOT_FOUND;
void UpdateAnimPreview( SGRX_AnimBundleAsset& aba )
{
	AnimHandle anim = SGRX_ProcessSingleAnim( aba, g_CurAnim );
	FC_SetAnim( g_EdAS->GetMesh( aba.previewMesh ), anim );
}

void EditAnimButton( size_t i, SGRX_ABAnimation& anim )
{
	String desc;
	anim.GetDesc( desc );
	if( ImGui::Selectable( StackString<1024>(desc), false, 0,
		ImVec2( ImGui::GetContentRegionAvailWidth() - 98, 16 ) ) )
	{
		g_CurAnim = i;
		UpdateAnimPreview( *g_CurAsset->ToAnimBundle() );
	}
}

void EditAnimSource( size_t i, SGRX_ABAnimSource& src )
{
	String desc;
	src.GetDesc( desc );
	IMGUI_GROUP( StackPath(desc), true,
	{
		g_NUIAssetPicker->Property( "Choose the animation file", "File", src.file );
		
		ImGui::BeginChangeMask( 1 );
		IMGUIEditString( "Prefix", src.prefix, 256 );
		ImGui::EndChangeMask();
	});
}

struct AnimAssetPartPicker : IMGUIEntryPicker
{
	void Reload( SGRX_AnimBundleAsset& aba )
	{
		m_entries.clear();
		for( size_t i = 0; i < aba.sources.size(); ++i )
		{
			SGRX_ABAnimSource& as = aba.sources[ i ];
			if( !as.file.size() )
				continue;
			ImpScene3DHandle scene = new SGRX_Scene3D( as.file, SIOF_Anims );
			if( !scene )
				continue;
			Array< String > anims;
			scene->GetAnimList( anims );
			for( size_t j = 0; j < anims.size(); ++j )
				m_entries.push_back( anims[ j ] );
		}
		_Search( m_searchString );
	}
}
g_AnimAssetPartPicker;

void EditAnimBundleAsset( SGRX_AnimBundleAsset& aba )
{
	ImGui::BeginChangeCheck();
	
	ImGui::Text( "Animation bundle" );
	ImGui::Separator();
	
	ImGui::BeginChangeMask( 1 );
	PickCategoryName( "Output category", aba.outputCategory );
	IMGUIEditString( "Output name", aba.outputName, 256 );
	ImGui::EndChangeMask();
	
	g_MeshAssetPicker.Property( "Pick a mesh for animation preview", "Preview mesh", aba.previewMesh );
	
	if( g_CurAnim < aba.anims.size() )
	{
		IMGUI_GROUP( "Animation", true,
		{
			SGRX_ABAnimation& anim = aba.anims[ g_CurAnim ];
			
			g_AnimAssetPartPicker.Property( "Select animation source", "Source", anim.source );
			
			ImGui::BeginChangeMask( 1 );
			IMGUIEditString( "Name", anim.name, 256 );
			ImGui::EndChangeMask();
			
			bool ovrStart = anim.startFrame != -1;
			if( IMGUIEditBool( "Override start frame", ovrStart ) )
			{
				if( ovrStart )
					anim.startFrame = 0;
				else
					anim.startFrame = -1;
			}
			if( ovrStart )
				IMGUIEditInt( "Start frame", anim.startFrame, 0, 99999 );
			
			bool ovrEnd = anim.endFrame != -1;
			if( IMGUIEditBool( "Override end frame", ovrEnd ) )
			{
				if( ovrEnd )
					anim.endFrame = 100;
				else
					anim.endFrame = -1;
			}
			if( ovrEnd )
				IMGUIEditInt( "End frame", anim.endFrame, 0, 99999 );
		});
	}
	
	IMGUIEditArray( aba.anims, EditAnimButton, "Add animation" );
	
	ImGui::BeginChangeCheck();
	IMGUIEditArray( aba.sources, EditAnimSource, "Add source" );
	if( ImGui::EndChangeCheck() & 2 )
	{
		g_AnimAssetPartPicker.Reload( aba );
	}
	
	int chg = ImGui::EndChangeCheck();
	if( chg & 1 )
		aba.ri.rev_asset++;
	if( chg & 2 )
		UpdateAnimPreview( aba );
}


void EditFileAsset( SGRX_FileAsset& fa )
{
	ImGui::BeginChangeCheck();
	
	ImGui::Text( "File" );
	ImGui::Separator();
	
	if( g_NUIAssetPicker->Property( "Select source file", "Source file", fa.sourceFile ) )
	{
		if( fa.outputName.size() == 0 )
			fa.outputName = SV(fa.sourceFile).after_last( "/" );
	}
	
	PickCategoryName( "Output category", fa.outputCategory );
	IMGUIEditString( "Output name", fa.outputName, 256 );
	
	int chg = ImGui::EndChangeCheck();
	if( chg )
		fa.ri.rev_asset++;
}


String g_CurCategory;
String g_CurCatName;
String g_CurCatPath;
bool g_RenameAssetCats = false;

void SetCurAsset( SGRX_Asset* asset )
{
	g_mode = EditAssets;
	g_CurAsset = asset;
	if( asset->assetType == SGRX_AT_Texture )
	{
		UpdateTexturePreview( *asset->ToTexture() );
	}
	if( asset->assetType == SGRX_AT_Mesh )
	{
		UpdateMeshPreview( *asset->ToMesh() );
		g_MeshAssetPartPicker.Reload( asset->ToMesh()->sourceFile );
	}
	if( asset->assetType == SGRX_AT_AnimBundle )
	{
		UpdateAnimPreview( *asset->ToAnimBundle() );
		g_AnimAssetPartPicker.Reload( *asset->ToAnimBundle() );
		g_CurAnim = NOT_FOUND;
	}
}
void SetCurCategory( size_t i )
{
	if( i == NOT_FOUND )
	{
		g_CurCategory = "";
		g_CurCatName = "";
		g_CurCatPath = "";
		return;
	}
	g_CurCategory = g_EdAS->categories.item( i ).key;
	g_CurCatName = g_CurCategory;
	g_CurCatName.resize( sgrx_snlen( g_CurCatName.data(), g_CurCatName.size() ) );
	g_CurCatPath = g_EdAS->categories.item( i ).value;
}


void EditCurAsset()
{
	if( !g_CurAsset )
	{
		ImGui::Text( "Select asset on the left side to edit it" );
		return;
	}
	
	ImGui::Columns( 2 );
	if( ImGui::Button( "Duplicate", ImVec2( ImGui::GetContentRegionAvailWidth(), 16 ) ) )
	{
		switch( g_CurAsset->assetType )
		{
		case SGRX_AT_Texture: {
				SGRX_TextureAsset new_asset;
				new_asset.Clone( *g_CurAsset->ToTexture() );
				g_EdAS->textureAssets.push_back( new_asset );
				g_CurAsset = &g_EdAS->textureAssets.last();
			} break;
		case SGRX_AT_Mesh: {
				SGRX_MeshAsset new_asset;
				new_asset.Clone( *g_CurAsset->ToMesh() );
				g_EdAS->meshAssets.push_back( new_asset );
				g_CurAsset = &g_EdAS->meshAssets.last();
			} break;
		case SGRX_AT_AnimBundle: {
				SGRX_AnimBundleAsset new_asset;
				new_asset.Clone( *g_CurAsset->ToAnimBundle() );
				g_EdAS->animBundleAssets.push_back( new_asset );
				g_CurAsset = &g_EdAS->animBundleAssets.last();
			} break;
		case SGRX_AT_File: {
				SGRX_FileAsset new_asset;
				new_asset.Clone( *g_CurAsset->ToFile() );
				g_EdAS->fileAssets.push_back( new_asset );
				g_CurAsset = &g_EdAS->fileAssets.last();
			} break;
		}
		g_CurAsset->outputName.append( " - Copy" );
	}
	ImGui::NextColumn();
	if( ImGui::Button( "Delete", ImVec2( ImGui::GetContentRegionAvailWidth(), 16 ) ) )
	{
		switch( g_CurAsset->assetType )
		{
		case SGRX_AT_Texture:
			g_EdAS->textureAssets.erase( g_CurAsset->ToTexture() - g_EdAS->textureAssets.data() );
			break;
		case SGRX_AT_Mesh:
			g_EdAS->meshAssets.erase( g_CurAsset->ToMesh() - g_EdAS->meshAssets.data() );
			break;
		case SGRX_AT_AnimBundle:
			g_EdAS->animBundleAssets.erase( g_CurAsset->ToAnimBundle() - g_EdAS->animBundleAssets.data() );
			break;
		case SGRX_AT_File:
			g_EdAS->fileAssets.erase( g_CurAsset->ToFile() - g_EdAS->fileAssets.data() );
			break;
		}
		g_CurAsset = NULL;
		return;
	}
	ImGui::Columns( 1 );
	ImGui::Separator();
	
	switch( g_CurAsset->assetType )
	{
	case SGRX_AT_Texture: EditTextureAsset( *g_CurAsset->ToTexture() ); break;
	case SGRX_AT_Mesh: EditMeshAsset( *g_CurAsset->ToMesh() ); break;
	case SGRX_AT_AnimBundle: EditAnimBundleAsset( *g_CurAsset->ToAnimBundle() ); break;
	case SGRX_AT_File: EditFileAsset( *g_CurAsset->ToFile() ); break;
	}
}


void RenameAssetCat( SGRX_Asset* A )
{
	if( A->outputCategory == g_CurCategory )
	{
		A->outputCategory = g_CurCatName;
		A->ri.rev_asset++;
	}
}

void EditCurCategory()
{
	if( ImGui::Selectable( "Back to category list" ) )
	{
		SetCurCategory( NOT_FOUND );
		return;
	}
	ImGui::Separator();
	
	ImGui::Text( "Edit category: %s", StackPath(g_CurCategory).str );
	ImGui::Separator();
	
	IMGUIEditString( "Name", g_CurCatName, 256 );
	IMGUIEditString( "Path", g_CurCatPath, 256 );
	IMGUIEditBool( "Also edit asset categories on rename", g_RenameAssetCats );
	
	const char* act = g_CurCatName != g_CurCategory
		? ( g_EdAS->categories.isset( g_CurCatName ) ? "Move/Overwrite" : "Move/rename" )
		: "Edit";
	if( g_CurCatName.size() && ImGui::Button( act ) )
	{
		if( g_CurCatName != g_CurCategory )
		{
			g_EdAS->categories.unset( g_CurCategory );
		}
		g_EdAS->categories.set( g_CurCatName, g_CurCatPath );
		
		if( g_RenameAssetCats && g_CurCatName != g_CurCategory )
		{
			for( size_t i = 0; i < g_EdAS->textureAssets.size(); ++i )
				RenameAssetCat( &g_EdAS->textureAssets[ i ] );
			for( size_t i = 0; i < g_EdAS->meshAssets.size(); ++i )
				RenameAssetCat( &g_EdAS->meshAssets[ i ] );
			for( size_t i = 0; i < g_EdAS->animBundleAssets.size(); ++i )
				RenameAssetCat( &g_EdAS->animBundleAssets[ i ] );
		}
		
		SetCurCategory( NOT_FOUND );
	}
}

void EditCategoryList()
{
	if( g_CurCategory.size() )
	{
		EditCurCategory();
		return;
	}
	
	ImGui::Text( "Categories" );
	ImGui::SameLine();
	if( ImGui::Button( "Add" ) )
	{
		g_EdAS->categories.set( StringView("\0",1), "" );
		SetCurCategory( g_EdAS->categories.size() - 1 );
	}
	ImGui::Separator();
	
	for( size_t i = 0; i < g_EdAS->categories.size(); ++i )
	{
		char bfr[ 256 ];
		sgrx_snprintf( bfr, 256, "%s => %s",
			StackString<100>( g_EdAS->categories.item( i ).key ).str,
			StackString<100>( g_EdAS->categories.item( i ).value ).str );
		if( ImGui::Selectable( bfr ) )
			SetCurCategory( i );
	}
}


//
// ASSET LIST
//

enum EGroupBy
{
	GB_None,
	GB_Type,
	GB_Category,
	
	GB__MAX,
};

enum ESortBy
{
	SB_Name_ASC,
	SB_Name_DESC,
	
	SB__MAX,
};

bool g_ShowTextures = true;
bool g_ShowMeshes = true;
bool g_ShowAnimBundles = true;
bool g_ShowFiles = true;
int g_GroupBy = GB_Category;
int g_SortBy = SB_Name_ASC;
String g_Filter;

#define IMPL_ASSET_SORT_FN( name, test1, test2 ) \
	int name( const void* a, const void* b ) \
	{ \
		SGRX_CAST( SGRX_Asset**, pa, a ); \
		SGRX_CAST( SGRX_Asset**, pb, b ); \
		int t1 = test1; if( t1 ) return t1; \
		int t2 = test2; if( t2 ) return t2; \
		return pa - pb; \
	}
#define ISORT( sa, sb, p ) (int((sa)->p) - int((sb)->p))
#define SCOMP( sa, sb, p ) (SV((sa)->p).compare_to( (sb)->p ))
IMPL_ASSET_SORT_FN( assetsort_none_namea, 0, SCOMP( *pa, *pb, outputName ) );
IMPL_ASSET_SORT_FN( assetsort_none_named, 0, -SCOMP( *pa, *pb, outputName ) );
IMPL_ASSET_SORT_FN( assetsort_type_namea, ISORT( *pa, *pb, assetType ), SCOMP( *pa, *pb, outputName ) );
IMPL_ASSET_SORT_FN( assetsort_type_named, ISORT( *pa, *pb, assetType ), -SCOMP( *pa, *pb, outputName ) );
IMPL_ASSET_SORT_FN( assetsort_cat_namea, SCOMP( *pa, *pb, outputCategory ), SCOMP( *pa, *pb, outputName ) );
IMPL_ASSET_SORT_FN( assetsort_cat_named, SCOMP( *pa, *pb, outputCategory ), -SCOMP( *pa, *pb, outputName ) );
int (*g_AssetCmpFuncs[ GB__MAX ][ SB__MAX ])( const void*, const void* ) =
{
	{ assetsort_none_namea, assetsort_none_named },
	{ assetsort_type_namea, assetsort_type_named },
	{ assetsort_cat_namea, assetsort_cat_named },
};

void EditAssetList()
{
	static const char* type_letters[] = { "T", "M", "A", "F" };
	static const char* type_names[] = { "Textures", "Meshes", "Anim. bundles", "Files" };
	static const char* gb_options[] = { "None", "Type", "Category" };
	static const char* sb_options[] = { "Name [ASC]", "Name [DESC]" };
	
	IMGUIEditBool( "Show textures", g_ShowTextures );
	ImGui::SameLine();
	IMGUIEditBool( "Show meshes", g_ShowMeshes );
	IMGUIEditBool( "Show anim. bundles", g_ShowAnimBundles );
	IMGUIEditBool( "Show files", g_ShowFiles );
	IMGUI_COMBOBOX( "Group by", g_GroupBy, gb_options );
	IMGUI_COMBOBOX( "Sort by", g_SortBy, sb_options );
	IMGUIEditString( "Search", g_Filter, 256 );
	ImGui::Separator();
	
	Array< SGRX_Asset* > assets;
	if( g_ShowTextures )
	{
		for( size_t i = 0; i < g_EdAS->textureAssets.size(); ++i )
		{
			if( !g_Filter.size() || SV(g_EdAS->textureAssets[ i ].outputName).match_loose( g_Filter ) )
				assets.push_back( &g_EdAS->textureAssets[ i ] );
		}
	}
	if( g_ShowMeshes )
	{
		for( size_t i = 0; i < g_EdAS->meshAssets.size(); ++i )
		{
			if( !g_Filter.size() || SV(g_EdAS->meshAssets[ i ].outputName).match_loose( g_Filter ) )
				assets.push_back( &g_EdAS->meshAssets[ i ] );
		}
	}
	if( g_ShowAnimBundles )
	{
		for( size_t i = 0; i < g_EdAS->animBundleAssets.size(); ++i )
		{
			if( !g_Filter.size() || SV(g_EdAS->animBundleAssets[ i ].outputName).match_loose( g_Filter ) )
				assets.push_back( &g_EdAS->animBundleAssets[ i ] );
		}
	}
	if( g_ShowFiles )
	{
		for( size_t i = 0; i < g_EdAS->fileAssets.size(); ++i )
		{
			if( !g_Filter.size() || SV(g_EdAS->fileAssets[ i ].outputName).match_loose( g_Filter ) )
				assets.push_back( &g_EdAS->fileAssets[ i ] );
		}
	}
	qsort( assets.data(), assets.size(), sizeof(SGRX_Asset*), g_AssetCmpFuncs[ g_GroupBy ][ g_SortBy ] );
	
	if( ImGui::BeginChild( "Assets", ImGui::GetContentRegionAvail() ) )
	{
		StringView lastgroup;
		bool lastopen = false;
		
		char bfr[ 256 ];
		for( size_t i = 0; i < assets.size(); ++i )
		{
			SGRX_Asset* A = assets[ i ];
			
			StringView curgroup;
			if( g_GroupBy == GB_Type )
			{
				curgroup = type_names[ A->assetType ];
			}
			else if( g_GroupBy == GB_Category )
			{
				curgroup = A->outputCategory;
			}
			if( curgroup != lastgroup )
			{
				if( lastopen )
					ImGui::TreePop();
			//	ImGui::SetNextWindowCollapsed( false, ImGuiSetCond_Appearing );
				lastopen = ImGui::TreeNode( StackPath(curgroup) );
				lastgroup = curgroup;
			}
			
			if( lastopen || g_GroupBy == GB_None )
			{
				sgrx_snprintf( bfr, 256, "[%s] %.*s [%.*s]", type_letters[ A->assetType ],
					(int) A->outputName.size(), A->outputName.data(),
					(int) A->outputCategory.size(), A->outputCategory.data() );
				if( ImGui::MenuItem( bfr ) )
				{
					SetCurAsset( A );
				}
			}
		}
		
		if( lastopen )
			ImGui::TreePop();
		
		ImGui::EndChild();
	}
}


void ASCR_Open()
{
	LOG << "Trying to open asset script";
	
	if( g_EdAS->Load( ASSET_SCRIPT_NAME ) == false )
	{
		LOG_ERROR << "FAILED TO LOAD ASSET SCRIPT";
		return;
	}
	g_EdAS->LoadAssetInfo( ASSET_INFO_NAME );
}

void ASCR_Save()
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

void ASCR_Run( bool force )
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
		Window_EnableDragDrop( true );
		
		// core layout
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = V3(3);
		g_EdScene->camera.znear = 0.1f;
		g_EdScene->camera.UpdateMatrices();
		g_EdAS = new SGRX_AssetScript;
		
		SGRX_IMGUI_Init();
		
		g_NUIRenderView = new AssetRenderView;
		g_NUIAssetPicker = new IMGUIFilePicker( SGRXPATH_ASSETS, "" );
		g_NUIAssetPicker->m_layoutType = IMGUIPickerCore::Layout_List;
		
		ASCR_Open();
		
		return true;
	}
	void OnDestroy()
	{
		delete g_NUIAssetPicker;
		delete g_NUIRenderView;
		
		delete g_EdAS;
		g_EdAS = NULL;
		g_EdScene = NULL;
		
		SGRX_IMGUI_Free();
	}
	static bool IsImageFile( StringView path )
	{
		StringView ext = path.after_last( "." );
		return ext.equal_lower( "png" )
			|| ext.equal_lower( "jpg" )
			|| ext.equal_lower( "jpeg" )
			|| ext.equal_lower( "dds" )
			|| ext.equal_lower( "tga" )
			|| ext.equal_lower( "bmp" );
	}
	static bool IsMeshFile( StringView path )
	{
		StringView ext = path.after_last( "." );
		return ext.equal_lower( "dae" )
			|| ext.equal_lower( "fbx" )
			|| ext.equal_lower( "obj" )
			|| ext.equal_lower( "ssm" );
	}
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_DROPFILE )
		{
			String rootpath;
			if( FS_FindRealPath( SGRXPATH_ASSETS, rootpath ) )
			{
				rootpath = RealPath( rootpath );
				LOG << "Root: " << rootpath;
				
				String path = RealPath( e.drop.file );
				LOG << "Dropped a file: " << path;
				if( IsImageFile( path ) && PathIsUnder( path, rootpath ) )
				{
					String subpath = String_Concat( SGRXPATH_ASSETS "/", GetRelativePath( path, rootpath ) );
					String category = "";
					if( g_CurAsset )
						category = g_CurAsset->outputCategory;
					String name = SV( subpath ).after_last( "/" ).until_last( "." );
					LOG << "Dropped image";
					LOG << "file:" << subpath;
					LOG << "name:" << name;
					LOG << "category:" << category;
					
					SGRX_TextureAsset ta;
					ta.sourceFile = subpath;
					ta.outputCategory = category;
					ta.outputName = name;
					ta.outputType = SGRX_TOF_STX_RGBA32;
					g_EdAS->textureAssets.push_back( ta );
					SetCurAsset( &g_EdAS->textureAssets.last() );
				}
				if( IsMeshFile( path ) && PathIsUnder( path, rootpath ) )
				{
					String subpath = String_Concat( SGRXPATH_ASSETS "/", GetRelativePath( path, rootpath ) );
					String category = "";
					if( g_CurAsset )
						category = g_CurAsset->outputCategory;
					String name = SV( subpath ).after_last( "/" ).until_last( "." );
					LOG << "Dropped mesh";
					LOG << "file:" << subpath;
					LOG << "name:" << name;
					LOG << "category:" << category;
					
					SGRX_MeshAsset ma;
					ma.sourceFile = subpath;
					ma.outputCategory = category;
					ma.outputName = name;
					g_EdAS->meshAssets.push_back( ma );
					SetCurAsset( &g_EdAS->meshAssets.last() );
				}
			}
		}
		SGRX_IMGUI_Event( e );
	}
	void OnTick( float dt, uint32_t gametime )
	{
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		AnimInfo info;
		GR_ApplyAnimator( &g_NUIRenderView->m_animPreview, g_NUIRenderView->m_meshPrevInst );
		g_NUIRenderView->m_animPreview.Advance( dt, &info );
		
		SGRX_IMGUI_NewFrame( dt );
		
		IMGUI_MAIN_WINDOW_BEGIN
		{
			if( ImGui::BeginMenuBar() )
			{
				if( ImGui::Button( "Save" ) )
				{
					ASCR_Save();
				}
				ImGui::SameLine();
				if( ImGui::Button( "Run" ) )
				{
					ASCR_Run( false );
				}
				ImGui::SameLine();
				if( ImGui::BeginMenu( "Force run" ) )
				{
					if( ImGui::MenuItem( "Force run" ) )
					{
						ASCR_Run( true );
					}
					ImGui::EndMenu();
				}
				
				ImGui::SameLine( 0, 50 );
				ImGui::Text( "Edit mode:" );
				ImGui::SameLine();
				ImGui::RadioButton( "Assets", &g_mode, EditAssets );
				ImGui::SameLine();
				ImGui::RadioButton( "Categories", &g_mode, EditCategories );
				ImGui::SameLine();
				ImGui::RadioButton( "Misc. settings", &g_mode, MiscProps );
				
				ImGui::SameLine( 0, 50 );
				if( ImGui::Button( "Create texture" ) )
				{
					SGRX_TextureAsset ta;
					if( g_CurAsset )
						ta.outputCategory = g_CurAsset->outputCategory;
					ta.outputType = SGRX_TOF_STX_RGBA32;
					g_EdAS->textureAssets.push_back( ta );
					SetCurAsset( &g_EdAS->textureAssets.last() );
				}
				ImGui::SameLine();
				if( ImGui::Button( "Create mesh" ) )
				{
					SGRX_MeshAsset ma;
					if( g_CurAsset )
						ma.outputCategory = g_CurAsset->outputCategory;
					g_EdAS->meshAssets.push_back( ma );
					SetCurAsset( &g_EdAS->meshAssets.last() );
				}
				ImGui::SameLine();
				if( ImGui::Button( "Create anim. bundle" ) )
				{
					SGRX_AnimBundleAsset aba;
					if( g_CurAsset )
						aba.outputCategory = g_CurAsset->outputCategory;
					g_EdAS->animBundleAssets.push_back( aba );
					SetCurAsset( &g_EdAS->animBundleAssets.last() );
				}
				ImGui::SameLine();
				if( ImGui::Button( "Create file" ) )
				{
					SGRX_FileAsset fa;
					if( g_CurAsset )
						fa.outputCategory = g_CurAsset->outputCategory;
					g_EdAS->fileAssets.push_back( fa );
					SetCurAsset( &g_EdAS->fileAssets.last() );
				}
				
				ImGui::EndMenuBar();
			}
			
			IMGUI_HSPLIT3( 0.2f, 0.7f,
			{
				EditAssetList();
			},
			{
				g_NUIRenderView->Process( dt );
			},
			{
				if( g_mode == EditAssets )
				{
					EditCurAsset();
				}
				else if( g_mode == EditCategories )
				{
					EditCategoryList();
				}
				else if( g_mode == MiscProps )
				{
					g_NUIRenderView->EditCameraParams();
				}
			});
		}
		IMGUI_MAIN_WINDOW_END;
		
		SGRX_IMGUI_Render();
		SGRX_IMGUI_ClearEvents();
	}
};


extern "C" EXPORT IGame* CreateGame()
{
	return new ASEditor;
}

