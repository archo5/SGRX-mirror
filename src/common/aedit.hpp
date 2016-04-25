

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


struct EDGUIMeshPartModel : EDGUIItemModel
{
	EDGUIMeshPartModel() : meshAsset( NULL ){}
	int GetItemCount(){ return meshAsset ? meshAsset->parts.size() : 0; }
	void GetItemName( int i, String& out ){ if( meshAsset ) meshAsset->parts[ i ]->GetDesc( i, out ); }
	SGRX_MeshAsset* meshAsset;
};

struct EDGUIAssetMesh : EDGUILayoutRow
{
	EDGUIAssetMesh();
	~EDGUIAssetMesh();
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
	EDGUIMeshPartModel m_meshPartModel;
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

struct EDGUIABAnimModel : EDGUIItemModel
{
	EDGUIABAnimModel() : abAsset( NULL ){}
	int GetItemCount(){ return abAsset ? abAsset->anims.size() : 0; }
	void GetItemName( int i, String& out ){ if( abAsset ) abAsset->anims[ i ].GetDesc( out ); }
	SGRX_AnimBundleAsset* abAsset;
};
struct EDGUIABSourceModel : EDGUIItemModel
{
	EDGUIABSourceModel() : abAsset( NULL ){}
	int GetItemCount(){ return abAsset ? abAsset->sources.size() : 0; }
	void GetItemName( int i, String& out ){ if( abAsset ) abAsset->sources[ i ].GetDesc( out ); }
	SGRX_AnimBundleAsset* abAsset;
};

struct EDGUIAssetAnimBundle : EDGUILayoutRow
{
	EDGUIAssetAnimBundle();
	~EDGUIAssetAnimBundle();
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
	
	EDGUIABAnimModel m_animModel;
	EDGUIABSourceModel m_sourceModel;
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


struct EDGUIMainFrame : EDGUIFrame
{
	EDGUIMainFrame();
	void AddToParamList( EDGUIItem* item );
	void ClearParamList();
	void EditMesh( size_t id );
	void EditMeshList();
	void EditAnimBundle( size_t id );
	void EditAnimBundleList();
	
	// core layout
	EDGUILayoutSplitPane m_UIMenuSplit;
	EDGUILayoutSplitPane m_UIParamSplit;
	EDGUIVScroll m_UIParamScroll;
	EDGUILayoutRow m_UIParamList;
	
	// data edit views
	EDGUIAssetMesh m_UIMesh;
	EDGUIAssetMeshList m_UIMeshList;
	EDGUIAssetAnimBundle m_UIAnimBundle;
	EDGUIAssetAnimBundleList m_UIAnimBundleList;
};


