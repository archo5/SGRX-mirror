

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

struct EDGUIASAnimNamePicker : EDGUILongEnumPicker
{
	EDGUIASAnimNamePicker();
	void Reload();
	
	Array< ImpScene3DHandle > m_scenes;
};

struct EDGUIPickers
{
	EDGUICategoryPicker category;
	EDGUITextureAssetPicker textureAsset;
	EDGUIMeshAssetPicker meshAsset;
	EDGUIAssetPathPicker assetPath;
	EDGUIASAnimNamePicker animName;
	
	void ClearAnimScenes()
	{
		animName.m_scenes.clear();
	}
	void AddAnimScene( SGRX_Scene3D* scene )
	{
		animName.m_scenes.push_back( scene );
	}
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


