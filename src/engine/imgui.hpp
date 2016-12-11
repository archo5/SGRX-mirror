

#pragma once
#define __STDC_FORMAT_MACROS 1
#include <float.h>
#include "engine.hpp"
#include "enganim.hpp"
#include "sound.hpp"


#define IMGUI_API ENGINE_EXPORT
#include "../../ext/src/imgui/imgui.h"


FINLINE ImVec2 operator + ( const ImVec2& a, const ImVec2& b ){ return ImVec2( a.x + b.x, a.y + b.y ); }
FINLINE ImVec2 operator - ( const ImVec2& a, const ImVec2& b ){ return ImVec2( a.x - b.x, a.y - b.y ); }
FINLINE ImVec2& operator -= ( ImVec2& a, const ImVec2& b ){ a.x -= b.x; a.y -= b.y; return a; }
FINLINE ImVec2 operator * ( const ImVec2& a, const ImVec2& b ){ return ImVec2( a.x * b.x, a.y * b.y ); }
FINLINE bool operator == ( const ImVec2& a, const ImVec2& b ){ return a.x == b.x && a.y == b.y; }
FINLINE bool operator != ( const ImVec2& a, const ImVec2& b ){ return a.x != b.x || a.y != b.y; }

FINLINE Vec2 V2( const ImVec2& v ){ return V2( v.x, v.y ); }


ENGINE_EXPORT void SGRX_IMGUI_Init();
ENGINE_EXPORT void SGRX_IMGUI_Free();
ENGINE_EXPORT void SGRX_IMGUI_ClearEvents();
ENGINE_EXPORT void SGRX_IMGUI_Event( const Event& e );
ENGINE_EXPORT void SGRX_IMGUI_NewFrame( float dt );
ENGINE_EXPORT void SGRX_IMGUI_Render();


ENGINE_EXPORT bool IMGUIEditBool( const char* label, bool& v );
ENGINE_EXPORT bool IMGUIEditInt_( const char* label, int& v, int vmin, int vmax );
template< class T > bool IMGUIEditInt( const char* label, T& v, int vmin, int vmax )
{
	int iv = v;
	bool ret = IMGUIEditInt_( label, iv, vmin, vmax );
	v = iv;
	return ret;
}
template< class T > bool IMGUIEditIntFlags( const char* label, T& v, unsigned flag )
{
	unsigned iv = v;
	bool ret = ImGui::CheckboxFlags( label, &iv, flag );
	v = iv;
	return ret;
}
ENGINE_EXPORT bool IMGUIEditMask_( const char* label, uint64_t& v, int count, const char** tips = NULL );
template< class T > bool IMGUIEditMask( const char* label, T& v, int count, const char** tips = NULL )
{
	uint64_t iv = v;
	bool ret = IMGUIEditMask_( label, iv, count, tips );
	v = iv;
	return ret;
}
ENGINE_EXPORT bool IMGUIEditFloat( const char* label, float& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditVec2( const char* label, Vec2& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditVec3( const char* label, Vec3& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditVec4( const char* label, Vec4& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditFloatSlider( const char* label, float& v, float vmin, float vmax, float power = 1 );
ENGINE_EXPORT bool IMGUIEditQuat( const char* label, Quat& v );
ENGINE_EXPORT bool IMGUIEditXFMat4( const char* label, Mat4& v );
ENGINE_EXPORT bool IMGUIEditColorHSVHDR( const char* label, Vec3& v, float maxval = 1.0f );
ENGINE_EXPORT bool IMGUIEditColorRGBLDR( const char* label, Vec3& v );
ENGINE_EXPORT bool IMGUIEditColorRGBA32( const char* label, uint32_t& c );
ENGINE_EXPORT bool IMGUIEditString( const char* label, String& str, int maxsize );
ENGINE_EXPORT bool IMGUIEditString( const char* label, RCString& str, int maxsize );
ENGINE_EXPORT void IMGUIErrorStr( StringView str );
ENGINE_EXPORT void IMGUIError( const char* str, ... );
ENGINE_EXPORT void IMGUIYesNo( bool v );


struct IF_GCC(ENGINE_EXPORT) IMGUIRenderView : SGRX_RefCounted, SGRX_DebugDraw
{
	ENGINE_EXPORT IMGUIRenderView( SGRX_Scene* scene );
	ENGINE_EXPORT bool CanAcceptKeyboardInput();
	ENGINE_EXPORT void Process( float deltaTime, bool editable = true );
	ENGINE_EXPORT static void _StaticDraw( const ImDrawList* parent_list, const ImDrawCmd* cmd );
	ENGINE_EXPORT virtual void OnBeforeDraw();
	ENGINE_EXPORT virtual void DebugDraw();
	
	ENGINE_EXPORT void EditCameraParams();
	
	float hangle;
	float vangle;
	
	bool mouseOn;
	Vec3 crpos, crdir;
	bool cursor_aim;
	Vec2 cursor_hpos;
	float crplaneheight;
	SGRX_Scene* m_scene;
	SGRX_Viewport m_vp;
};


struct IF_GCC(ENGINE_EXPORT) IMGUIPickerCore
{
	enum LayoutType
	{
		Layout_Grid,
		Layout_List,
	};
	
	ENGINE_EXPORT IMGUIPickerCore();
	ENGINE_EXPORT virtual ~IMGUIPickerCore();
	
	ENGINE_EXPORT virtual void OpenPopup( const char* caption );
	ENGINE_EXPORT virtual bool Popup( const char* caption, String& str );
	ENGINE_EXPORT virtual bool Popup( const char* caption, RCString& str );
	ENGINE_EXPORT virtual bool Property( const char* caption, const char* label, String& str );
	ENGINE_EXPORT virtual void Reload();
	ENGINE_EXPORT virtual bool SearchUI( String& str );
	ENGINE_EXPORT void _Search( StringView text );
	
	virtual size_t GetEntryCount() const = 0;
	virtual RCString GetEntryName( size_t i ) const = 0;
	virtual RCString GetEntryKey( size_t i ) const = 0;
	ENGINE_EXPORT virtual bool EntryUI( size_t i, String& str );
	
	LayoutType m_layoutType;
	ImVec2 m_itemSize;
	HashTable< StringView, RCString > m_nameMap;
	// search
	bool m_looseSearch;
	String m_searchString;
	Array< int > m_filtered;
};

struct IF_GCC(ENGINE_EXPORT) IMGUIEntryPicker : IMGUIPickerCore
{
	virtual size_t GetEntryCount() const { return m_entries.size(); }
	virtual RCString GetEntryName( size_t i ) const { return m_entries[ i ]; }
	virtual RCString GetEntryKey( size_t i ) const { return m_entries[ i ]; }
	
	Array< RCString > m_entries;
};

struct IF_GCC(ENGINE_EXPORT) IMGUISoundPicker : IMGUIEntryPicker
{
	ENGINE_EXPORT void Reload();
	SoundSystemHandle sys;
};

struct IF_GCC(ENGINE_EXPORT) IMGUIFilePicker : IMGUIEntryPicker, IDirEntryHandler
{
	ENGINE_EXPORT IMGUIFilePicker( const char* dir, const char* ext, bool confirm = true );
	ENGINE_EXPORT bool Popup( const char* caption, String& str, bool save );
	ENGINE_EXPORT void Reload();
	ENGINE_EXPORT virtual bool SearchUI( String& str );
	
	ENGINE_EXPORT bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir );
	ENGINE_EXPORT virtual bool EntryUI( size_t i, String& str );
	ENGINE_EXPORT virtual bool ConfirmPopup( const char* caption, const char* label, const char* file );
	
	bool m_saveMode;
	bool m_confirm;
	const char* m_directory;
	const char* m_extension;
};

struct IF_GCC(ENGINE_EXPORT) IMGUIPreviewPickerCore : IMGUIPickerCore
{
	ENGINE_EXPORT IMGUIPreviewPickerCore();
	
	ENGINE_EXPORT virtual void _DrawItem( int i, int x0, int y0, int x1, int y1 );
	struct _StaticDrawItemData
	{
		IMGUIPreviewPickerCore* self;
		int i, x0, y0, x1, y1;
	};
	ENGINE_EXPORT static void _StaticDrawItem( const ImDrawList* parent_list, const ImDrawCmd* cmd );
	
	ENGINE_EXPORT virtual bool EntryUI( size_t i, String& str );
};

struct IF_GCC(ENGINE_EXPORT) IMGUIAssetPickerCore : IMGUIPreviewPickerCore
{
	struct BaseEntry : SGRX_RefCounted
	{
		RCString key;
		RCString name;
		RCString path;
	};
	typedef Handle< BaseEntry > BaseEntryHandle;
	
	ENGINE_EXPORT IMGUIAssetPickerCore();
	ENGINE_EXPORT ~IMGUIAssetPickerCore();
	
	ENGINE_EXPORT void Clear();
	ENGINE_EXPORT void Reload();
	ENGINE_EXPORT void ReloadAssets( int type );
	ENGINE_EXPORT void AddEntry( StringView key, StringView name, StringView path = SV() );
	
	ENGINE_EXPORT virtual void ReloadEntries() = 0;
	ENGINE_EXPORT virtual void AppendEntry() = 0;
	ENGINE_EXPORT virtual void InitEntryPreview( BaseEntry* e ) = 0;
	ENGINE_EXPORT virtual void _DrawItem( int i, int x0, int y0, int x1, int y1 ) = 0;
	
	virtual size_t GetEntryCount() const { return m_entries.size(); }
	virtual RCString GetEntryName( size_t i ) const { return m_entries[ i ]->name; }
	virtual RCString GetEntryKey( size_t i ) const { return m_entries[ i ]->key; }
	
	bool m_customCamera;
	Array< BaseEntryHandle > m_entries;
	SceneHandle m_scene;
};

struct IF_GCC(ENGINE_EXPORT) IMGUITexturePicker : IMGUIAssetPickerCore
{
	struct TextureEntry : BaseEntry
	{
		TextureHandle texture;
	};
	
	ENGINE_EXPORT IMGUITexturePicker();
	ENGINE_EXPORT virtual void ReloadEntries();
	ENGINE_EXPORT virtual void AppendEntry();
	ENGINE_EXPORT virtual void InitEntryPreview( BaseEntry* e );
	ENGINE_EXPORT virtual void _DrawItem( int i, int x0, int y0, int x1, int y1 );
};

struct IF_GCC(ENGINE_EXPORT) IMGUIMeshPicker : IMGUIAssetPickerCore
{
	struct MeshEntry : BaseEntry
	{
		MeshInstHandle mesh;
	};
	
	ENGINE_EXPORT IMGUIMeshPicker();
	ENGINE_EXPORT virtual void ReloadEntries();
	ENGINE_EXPORT virtual void AppendEntry();
	ENGINE_EXPORT virtual void InitEntryPreview( BaseEntry* e );
	ENGINE_EXPORT virtual void _DrawItem( int i, int x0, int y0, int x1, int y1 );
};

struct IF_GCC(ENGINE_EXPORT) IMGUICharPicker : IMGUIMeshPicker, IDirEntryHandler
{
	ENGINE_EXPORT IMGUICharPicker();
	ENGINE_EXPORT virtual void ReloadEntries();
	ENGINE_EXPORT bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir );
};

struct IF_GCC(ENGINE_EXPORT) IMGUIShaderPicker : IDirEntryHandler
{
	ENGINE_EXPORT virtual ~IMGUIShaderPicker();
	ENGINE_EXPORT bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir );
	ENGINE_EXPORT bool Property( const char* label, String& str );
	Array< RCString > m_shaderList;
};

struct IF_GCC(ENGINE_EXPORT) IMGUIAnimPicker : IMGUIAssetPickerCore
{
	ENGINE_EXPORT IMGUIAnimPicker();
	ENGINE_EXPORT virtual void ReloadEntries();
	ENGINE_EXPORT virtual void AppendEntry();
	ENGINE_EXPORT virtual void InitEntryPreview( BaseEntry* e );
	ENGINE_EXPORT virtual void _DrawItem( int i, int x0, int y0, int x1, int y1 );
	ENGINE_EXPORT virtual bool EntryUI( size_t i, String& str );
};

struct IF_GCC(ENGINE_EXPORT) IMGUIEnumPicker
{
	ENGINE_EXPORT bool Property( const char* label, int32_t& val );
	HashTable< int32_t, RCString > m_entries;
};


template< class T, class F > void IMGUIEditArray( Array< T >& data, F& editfn, const char* addbtn = "Add" )
{
	if( addbtn && ImGui::Button( addbtn, ImVec2( ImGui::GetContentRegionAvail().x, 24 ) ) )
	{
		data.push_back( T() );
		ImGui::TriggerChangeCheck();
	}
	
	for( size_t i = 0; i < data.size(); ++i )
	{
		ImGui::PushID( i );
		
		ImVec2 cp_before = ImGui::GetCursorPos();
		float width = ImGui::GetContentRegionAvail().x;
		editfn( i, data[ i ] );
		ImVec2 cp_after = ImGui::GetCursorPos();
		
		if( i > 0 )
		{
			ImGui::SetCursorPos( cp_before + ImVec2( width - 90, 0 ) );
			if( ImGui::Button( "[up]", ImVec2( 30, 14 ) ) )
			{
				TSWAP( data[ i - 1 ], data[ i ] );
				ImGui::TriggerChangeCheck();
			}
		}
		
		if( i < data.size() - 1 )
		{
			ImGui::SetCursorPos( cp_before + ImVec2( width - 60, 0 ) );
			if( ImGui::Button( "[dn]", ImVec2( 30, 14 ) ) )
			{
				TSWAP( data[ i ], data[ i + 1 ] );
				ImGui::TriggerChangeCheck();
			}
		}
		
		ImGui::SetCursorPos( cp_before + ImVec2( width - 30, 0 ) );
		if( ImGui::Button( "[del]", ImVec2( 30, 14 ) ) )
		{
			data.erase( i-- );
			ImGui::TriggerChangeCheck();
		}
		ImGui::SetCursorPos( cp_after );
		
		ImGui::PopID();
	}
}

#define IMGUI_MAIN_WINDOW_BEGIN \
	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(0,0) ); \
	ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0 ); \
	ImGui::SetNextWindowPos( ImVec2(0,0) ); \
	ImGui::SetNextWindowSize( ImVec2( GR_GetWidth(), GR_GetHeight() ) ); \
	if( ImGui::Begin( "main", NULL, \
		ImGuiWindowFlags_NoTitleBar | \
		ImGuiWindowFlags_NoResize | \
		ImGuiWindowFlags_NoMove | \
		ImGuiWindowFlags_MenuBar | \
		ImGuiWindowFlags_NoBringToFrontOnFocus ) ) \
	{ \
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(4,4) ); \
		ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 4 );
#define IMGUI_MAIN_WINDOW_END \
		ImGui::PopStyleVar( 2 ); \
		ImGui::End(); \
	} \
	ImGui::PopStyleVar( 2 );

#define IMGUI_HSPLIT( at, left, right ) \
	if( ImGui::BeginChild( "hs:lft", ImGui::GetContentRegionAvail() * ImVec2( at, 1 ), true ) ){ \
		left; ImGui::EndChild(); \
	} ImGui::SameLine( ImGui::GetContentRegionAvail().x * at ); \
	if( ImGui::BeginChild( "hs:rgt", ImGui::GetContentRegionAvail(), true ) ){ \
		right; ImGui::EndChild(); \
	}
#define IMGUI_HSPLIT3( spl12, spl23, left, mid, right ) \
	if( ImGui::BeginChild( "hs:lft", ImGui::GetContentRegionAvail() * ImVec2( spl12, 1 ), true ) ){ \
		left; ImGui::EndChild(); \
	} ImGui::SameLine( ImGui::GetContentRegionAvail().x * spl12 ); \
	if( ImGui::BeginChild( "hs:mid", ImGui::GetContentRegionAvail() * ImVec2( ( spl23 - spl12 ) / ( 1 - spl12 ), 1 ), true ) ){ \
		mid; ImGui::EndChild(); \
	} ImGui::SameLine( ImGui::GetContentRegionAvail().x * spl23 ); \
	if( ImGui::BeginChild( "hs:rgt", ImGui::GetContentRegionAvail(), true ) ){ \
		right; ImGui::EndChild(); \
	}

#define IMGUI_GROUP( name, opened, cont ) ImGui::SetNextTreeNodeOpened( opened, ImGuiSetCond_Appearing ); \
	if( ImGui::TreeNode( name ) ){ cont; ImGui::TreePop(); }
#define IMGUI_GROUP_BEGIN( name, opened ) ImGui::SetNextTreeNodeOpened( opened, ImGuiSetCond_Appearing ); \
	if( ImGui::TreeNode( name ) ){
#define IMGUI_GROUP_END ImGui::TreePop(); }
#define IMGUI_GROUPCTL_BEGIN( label ) ImGui::PushID( label ); ImGui::BeginGroup(); ImGui::Separator(); ImGui::Text( "%s", label );
#define IMGUI_GROUPCTL_END ImGui::Separator(); ImGui::EndGroup(); ImGui::PopID();

template< class T > bool IMGUIListbox( const char* name, T& val, const char** list, int lsize, int start = 0 )
{
	int curr = int(val) - start;
	bool ret = ImGui::ListBox( name, &curr, list, lsize );
	val = (T) ( curr + start );
	return ret;
}
#define IMGUI_LISTBOX( name, val, list ) IMGUIListbox( name, val, list, SGRX_ARRAY_SIZE( list ) )

template< class T > bool IMGUIComboBox( const char* name, T& val, const char* zssl, int start = 0 )
{
	int curr = int(val) - start;
	bool ret = ImGui::Combo( name, &curr, zssl );
	val = (T) ( curr + start );
	return ret;
}
template< class T > bool IMGUIComboBox( const char* name, T& val, const char** list, int lsize, int start = 0 )
{
	int curr = int(val) - start;
	bool ret = ImGui::Combo( name, &curr, list, lsize );
	val = (T) ( curr + start );
	return ret;
}
#define IMGUI_COMBOBOX( name, val, list ) IMGUIComboBox( name, val, list, SGRX_ARRAY_SIZE( list ) )




inline void lmm_prepmeshinst( MeshInstHandle mih )
{
	mih->SetLightingMode( SGRX_LM_Dynamic );
	for( int i = 10; i < 16; ++i )
		mih->constants[ i ] = V4(0.15f);
}


