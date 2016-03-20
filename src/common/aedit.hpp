

#include <engine.hpp>
#include <enganim.hpp>
#include <edgui.hpp>
#include "edcomui.hpp"

#include "assetcomp.hpp"



struct EDGUISmallEnumPicker : EDGUIRsrcPicker
{
	void _OnChangeZoom();
};

struct EDGUILongEnumPicker : EDGUIRsrcPicker
{
	void _OnChangeZoom();
};

struct EDGUIMtlBlendMode : EDGUISmallEnumPicker
{
	EDGUIMtlBlendMode();
};

struct EDGUIImageFilterSharpenMode : EDGUISmallEnumPicker
{
	EDGUIImageFilterSharpenMode();
};

struct EDGUIImageFilterType : EDGUISmallEnumPicker
{
	EDGUIImageFilterType();
	SGRX_AssetImageFilterType GetPickedType() const
	{
		return SGRX_AssetImageFilterType( m_picked + 1 );
	}
};

struct EDGUITextureOutputFormat : EDGUISmallEnumPicker
{
	EDGUITextureOutputFormat();
};

struct EDGUICategoryPicker : EDGUISmallEnumPicker
{
	EDGUICategoryPicker();
	void Reload();
};

struct EDGUITextureAssetPicker : EDGUILongEnumPicker
{
	EDGUITextureAssetPicker();
	void Reload();
};

struct EDGUIMeshAssetPicker : EDGUILongEnumPicker
{
	EDGUIMeshAssetPicker();
	void Reload();
};

struct EDGUIAssetPathPicker : EDGUILongEnumPicker, IDirEntryHandler
{
	EDGUIAssetPathPicker();
	void AddOptionsFromDir( const StringView& path );
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir );
	void Reload();
	
	int m_depth;
};

struct EDGUIASMeshNamePicker : EDGUILongEnumPicker
{
	EDGUIASMeshNamePicker();
	void Reload();
	
	ImpScene3DHandle m_scene;
};

struct EDGUIASAnimNamePicker : EDGUILongEnumPicker
{
	EDGUIASAnimNamePicker();
	void Reload();
	
	Array< ImpScene3DHandle > m_scenes;
};

struct EDGUIPickers
{
	EDGUIMtlBlendMode blendMode;
	EDGUIImageFilterSharpenMode sharpenMode;
	EDGUIImageFilterType imageFilterType;
	EDGUITextureOutputFormat textureOutputFormat;
	EDGUICategoryPicker category;
	EDGUITextureAssetPicker textureAsset;
	EDGUIMeshAssetPicker meshAsset;
	EDGUIAssetPathPicker assetPath;
	EDGUIASMeshNamePicker meshName;
	EDGUIASAnimNamePicker animName;
	EDGUIShaderPicker shader;
	
	void SetMesh( SGRX_Scene3D* scene )
	{
		meshName.m_scene = scene;
	}
	void ClearAnimScenes()
	{
		animName.m_scenes.clear();
	}
	void AddAnimScene( SGRX_Scene3D* scene )
	{
		animName.m_scenes.push_back( scene );
	}
};

struct EDGUICreatePickButton : EDGUIPropRsrc
{
	EDGUICreatePickButton( EDGUIRsrcPicker* rsrcPicker, const StringView& def = StringView() );
	virtual int OnEvent( EDGUIEvent* e );
};


struct EDGUIImgFilterBase : EDGUILayoutRow
{
	EDGUIImgFilterBase( SGRX_ImageFilter* iflt, bool addCMF = false );
	void AddCMFFields();
	virtual int OnEvent( EDGUIEvent* e );
	
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
	EDGUIImgFilter_Resize( SGRX_ImageFilter* iflt );
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIPropInt m_width;
	EDGUIPropInt m_height;
	EDGUIPropBool m_srgb;
};

struct EDGUIImgFilter_Rearrange : EDGUIImgFilterBase
{
	EDGUIImgFilter_Rearrange( SGRX_ImageFilter* iflt );
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIPropInt m_width;
};

struct EDGUIImgFilter_Sharpen : EDGUIImgFilterBase
{
	EDGUIImgFilter_Sharpen( SGRX_ImageFilter* iflt );
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIPropFloat m_factor;
	EDGUIPropRsrc m_mode;
};

struct EDGUIImgFilter_ExpandRange : EDGUIImgFilterBase
{
	EDGUIImgFilter_ExpandRange( SGRX_ImageFilter* iflt );
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIPropVec4 m_vmin;
	EDGUIPropVec4 m_vmax;
};

struct EDGUIImgFilter_BCP : EDGUIImgFilterBase
{
	EDGUIImgFilter_BCP( SGRX_ImageFilter* iflt );
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIPropBool m_apply_bc1;
	EDGUIPropFloat m_brightness;
	EDGUIPropFloat m_contrast;
	EDGUIPropBool m_apply_pow;
	EDGUIPropFloat m_power;
	EDGUIPropBool m_apply_bc2;
	EDGUIPropFloat m_brightness_2;
	EDGUIPropFloat m_contrast_2;
};


struct EDGUIAssetTexture : EDGUILayoutRow
{
	EDGUIAssetTexture();
	~EDGUIAssetTexture();
	void ReloadFilterList();
	void UpdatePreviewTexture();
	void Prepare( size_t tid );
	void EditFilter( SGRX_ImageFilter* IF );
	virtual int OnEvent( EDGUIEvent* e );
	
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
	EDGUIAssetTextureList();
	void Prepare();
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIPropString m_filter;
	EDGUIBtnList m_buttons;
	EDGUIListItemButton m_editButton;
};

struct EDGUIAssetMesh : EDGUILayoutRow
{
	EDGUIAssetMesh();
	void UpdatePreviewMesh();
	void ReloadPartList();
	void ReloadImpScene();
	void Prepare( size_t mid );
	void PreparePart( size_t pid );
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUILayoutColumn m_topCol;
	EDGUIButton m_btnDuplicate;
	EDGUIButton m_btnDelete;
	
	EDGUIGroup m_group;
	EDGUIPropRsrc m_sourceFile;
	EDGUIPropRsrc m_outputCategory;
	EDGUIPropString m_outputName;
	EDGUIPropBool m_rotateY2Z;
	EDGUIPropBool m_flipUVY;
	EDGUIPropBool m_transform;
	// mesh part
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
	EDGUIPropDefBool m_MPoptTransform;
	// mesh part list
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
	EDGUIAssetMeshList();
	void Prepare();
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIPropString m_filter;
	EDGUIBtnList m_buttons;
	EDGUIListItemButton m_editButton;
};

struct EDGUIAssetAnimBundle : EDGUILayoutRow
{
	EDGUIAssetAnimBundle();
	void UpdatePreviewAnim();
	void Prepare( size_t abid );
	void ReloadAnimSourceList();
	void PrepareAnimSource( size_t sid );
	void ReloadAnimList();
	void PrepareAnim( size_t aid );
	void ReloadImpScenes();
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUILayoutColumn m_topCol;
	EDGUIButton m_btnDuplicate;
	EDGUIButton m_btnDelete;
	
	// anim. bundle
	EDGUIGroup m_group;
	EDGUIPropRsrc m_outputCategory;
	EDGUIPropString m_outputName;
	EDGUIPropString m_bundlePrefix;
	EDGUIPropRsrc m_previewMesh;
	// animation
	EDGUIGroup m_AN_group;
	EDGUILayoutRow m_AN_cont;
	EDGUIPropRsrc m_AN_source;
	EDGUIPropString m_AN_name;
	EDGUIPropString m_AN_startFrame;
	EDGUIPropString m_AN_endFrame;
	// animation list
	EDGUIGroup m_ANL_group;
	EDGUIButton m_ANL_btnAdd;
	EDGUIBtnList m_ANL_buttons;
	EDGUIListItemButton m_ANL_editButton;
	// anim. source
	EDGUIGroup m_AS_group;
	EDGUILayoutRow m_AS_cont;
	EDGUIPropRsrc m_AS_fileName;
	EDGUIPropString m_AS_prefix;
	EDGUIButton m_AS_addAll;
	// anim. source list
	EDGUIGroup m_ASL_group;
	EDGUIButton m_ASL_btnAdd;
	EDGUIBtnList m_ASL_buttons;
	EDGUIListItemButton m_ASL_editButton;
	
	size_t m_abid;
	size_t m_sid;
	size_t m_aid;
};

struct EDGUIAssetAnimBundleList : EDGUILayoutRow
{
	EDGUIAssetAnimBundleList();
	void Prepare();
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIPropString m_filter;
	EDGUIBtnList m_buttons;
	EDGUIListItemButton m_editButton;
};

struct EDGUIAssetCategoryForm : EDGUILayoutRow
{
	EDGUIAssetCategoryForm();
	void _UpdateButtonText();
	void Prepare( const StringView& name );
	virtual int OnEvent( EDGUIEvent* e );
	
	String m_origName;
	EDGUIGroup m_group;
	EDGUIPropString m_name;
	EDGUIPropString m_path;
	EDGUIButton m_btnSave;
};

struct EDGUIAssetCategoryList : EDGUILayoutRow
{
	EDGUIAssetCategoryList();
	void Prepare();
	virtual int OnEvent( EDGUIEvent* e );
	
	EDGUIGroup m_cgroup;
	EDGUIButton m_ctgBtnAdd;
	EDGUIPropString m_filter;
	EDGUIBtnList m_ctgButtons;
	EDGUIListItemButton m_ctgEditButton;
};


struct EDGUIMainFrame : EDGUIFrame, EDGUIRenderView::FrameInterface
{
	EDGUIMainFrame();
	int OnEvent( EDGUIEvent* e );
	bool ViewEvent( EDGUIEvent* e ){ return true; }
	void DebugDraw();
	void AddToParamList( EDGUIItem* item );
	void ClearParamList();
	void EditTexture( size_t id );
	void EditTextureList();
	void EditMesh( size_t id );
	void EditMeshList();
	void EditAnimBundle( size_t id );
	void EditAnimBundleList();
	void EditCategory( const StringView& name );
	void EditCatList();
	void ResetEditorState();
	void ASCR_Open();
	void ASCR_Save();
	void ASCR_Run( bool force );
	
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
	EDGUIButton m_MBForceRun;
	EDGUILabel m_MB_Cat1;
	EDGUIButton m_MBEditScript;
	EDGUIButton m_MBEditTextures;
	EDGUIButton m_MBEditMeshes;
	EDGUIButton m_MBEditAnimBundles;
	
	// data edit views
	EDGUIAssetTexture m_UITexture;
	EDGUIAssetTextureList m_UITextureList;
	EDGUIAssetMesh m_UIMesh;
	EDGUIAssetMeshList m_UIMeshList;
	EDGUIAssetAnimBundle m_UIAnimBundle;
	EDGUIAssetAnimBundleList m_UIAnimBundleList;
	EDGUIAssetCategoryForm m_UICategory;
	EDGUIAssetCategoryList m_UICatList;
	
	// preview data
	TextureHandle m_texPreview;
	MeshInstHandle m_meshPrevInst;
	AnimPlayer m_animPreview;
};


