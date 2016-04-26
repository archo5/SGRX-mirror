

#pragma once
#define __STDC_FORMAT_MACROS 1
#include <float.h>
#include "engine.hpp"


#define IMGUI_API ENGINE_EXPORT
#include "../../ext/src/imgui/imgui.h"


FINLINE ImVec2 operator + ( const ImVec2& a, const ImVec2& b ){ return ImVec2( a.x + b.x, a.y + b.y ); }
FINLINE ImVec2 operator - ( const ImVec2& a, const ImVec2& b ){ return ImVec2( a.x - b.x, a.y - b.y ); }
FINLINE ImVec2 operator * ( const ImVec2& a, const ImVec2& b ){ return ImVec2( a.x * b.x, a.y * b.y ); }


ENGINE_EXPORT void SGRX_IMGUI_Init();
ENGINE_EXPORT void SGRX_IMGUI_Free();
ENGINE_EXPORT void SGRX_IMGUI_ClearEvents();
ENGINE_EXPORT void SGRX_IMGUI_Event( const Event& e );
ENGINE_EXPORT void SGRX_IMGUI_NewFrame();
ENGINE_EXPORT void SGRX_IMGUI_Render();


ENGINE_EXPORT bool IMGUIEditBool( const char* label, bool& v );
ENGINE_EXPORT bool IMGUIEditInt_( const char* label, int& v, int vmin, int vmax );
template< class T > IMGUIEditInt( const char* label, T& v, int vmin, int vmax )
{
	int iv = v;
	bool ret = IMGUIEditInt_( label, iv, vmin, vmax );
	v = iv;
	return ret;
}
template< class T > IMGUIEditIntFlags( const char* label, T& v, unsigned flag )
{
	unsigned iv = v;
	bool ret = ImGui::CheckboxFlags( label, &iv, flag );
	v = iv;
	return ret;
}
ENGINE_EXPORT bool IMGUIEditFloat( const char* label, float& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditVec3( const char* label, Vec3& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditVec4( const char* label, Vec4& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditQuat( const char* label, Quat& v );
ENGINE_EXPORT bool IMGUIEditString( const char* label, String& str, int maxsize );
ENGINE_EXPORT void IMGUIErrorStr( StringView str );
ENGINE_EXPORT void IMGUIError( const char* str, ... );
ENGINE_EXPORT void IMGUIYesNo( bool v );


struct IF_GCC(ENGINE_EXPORT) IMGUIRenderView : SGRX_RefCounted, SGRX_DebugDraw
{
	ENGINE_EXPORT IMGUIRenderView( SGRX_Scene* scene );
	ENGINE_EXPORT bool CanAcceptKeyboardInput();
	ENGINE_EXPORT void Process( float deltaTime );
	ENGINE_EXPORT static void _StaticDraw( const ImDrawList* parent_list, const ImDrawCmd* cmd );
	ENGINE_EXPORT virtual void DebugDraw();
	
	ENGINE_EXPORT void EditCameraParams();
	
	float hangle;
	float vangle;
	
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
	ENGINE_EXPORT virtual bool Property( const char* caption, const char* label, String& str );
	ENGINE_EXPORT virtual void Reload();
	ENGINE_EXPORT virtual bool SearchUI( String& str );
	ENGINE_EXPORT void _Search( StringView text );
	
	virtual size_t GetEntryCount() const = 0;
	virtual RCString GetEntryPath( size_t i ) const = 0;
	ENGINE_EXPORT virtual bool EntryUI( size_t i, String& str );
	
	LayoutType m_layoutType;
	ImVec2 m_itemSize;
	// search
	bool m_looseSearch;
	String m_searchString;
	Array< int > m_filtered;
};

struct IF_GCC(ENGINE_EXPORT) IMGUIEntryPicker : IMGUIPickerCore
{
	virtual size_t GetEntryCount() const { return m_entries.size(); }
	virtual RCString GetEntryPath( size_t i ) const { return m_entries[ i ]; }
	
	Array< RCString > m_entries;
};

struct IF_GCC(ENGINE_EXPORT) IMGUIFilePicker : IMGUIEntryPicker, IDirEntryHandler
{
	ENGINE_EXPORT IMGUIFilePicker( const char* dir, const char* ext );
	ENGINE_EXPORT bool Popup( const char* caption, String& str, bool save );
	ENGINE_EXPORT void Reload();
	ENGINE_EXPORT virtual bool SearchUI( String& str );
	
	ENGINE_EXPORT bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir );
	ENGINE_EXPORT virtual bool EntryUI( size_t i, String& str );
	ENGINE_EXPORT virtual bool ConfirmPopup( const char* caption, const char* label, const char* file );
	
	bool m_saveMode;
	const char* m_directory;
	const char* m_extension;
};

struct IF_GCC(ENGINE_EXPORT) IMGUIMeshPickerCore : IMGUIPickerCore
{
	struct Entry
	{
		RCString path;
		MeshInstHandle mesh;
	};
	
	ENGINE_EXPORT IMGUIMeshPickerCore();
	ENGINE_EXPORT ~IMGUIMeshPickerCore();
	
	ENGINE_EXPORT void Clear();
	ENGINE_EXPORT void AddMesh( StringView path, StringView rsrcpath = SV() );
	
	ENGINE_EXPORT void _DrawItem( int i, int x0, int y0, int x1, int y1 );
	struct _StaticDrawItemData
	{
		IMGUIMeshPickerCore* self;
		int i, x0, y0, x1, y1;
	};
	ENGINE_EXPORT static void _StaticDrawItem( const ImDrawList* parent_list, const ImDrawCmd* cmd );
	
	virtual size_t GetEntryCount() const { return m_entries.size(); }
	virtual RCString GetEntryPath( size_t i ) const { return m_entries[ i ].path; }
	ENGINE_EXPORT virtual bool EntryUI( size_t i, String& str );
	
	bool m_customCamera;
	Array< Entry > m_entries;
	SceneHandle m_scene;
};

struct IF_GCC(ENGINE_EXPORT) IMGUIMeshPicker : IMGUIMeshPickerCore, IDirEntryHandler
{
	ENGINE_EXPORT IMGUIMeshPicker();
	ENGINE_EXPORT void Reload();
	ENGINE_EXPORT bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir );
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
		ImGuiWindowFlags_MenuBar ) ) \
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

template< class T > bool IMGUIListbox( const char* name, T& val, const char** list, int lsize )
{
	int curr = val;
	bool ret = ImGui::ListBox( name, &curr, list, lsize );
	val = curr;
	return ret;
}
#define IMGUI_LISTBOX( name, val, list ) IMGUIListbox( name, val, list, SGRX_ARRAY_SIZE( list ) )

template< class T > bool IMGUIComboBox( const char* name, T& val, const char* zssl )
{
	int curr = val;
	bool ret = ImGui::Combo( name, &curr, zssl );
	val = (T) curr;
	return ret;
}
template< class T > bool IMGUIComboBox( const char* name, T& val, const char** list, int lsize )
{
	int curr = val;
	bool ret = ImGui::Combo( name, &curr, list, lsize );
	val = (T) curr;
	return ret;
}
#define IMGUI_COMBOBOX( name, val, list ) IMGUIComboBox( name, val, list, SGRX_ARRAY_SIZE( list ) )




inline void lmm_prepmeshinst( MeshInstHandle mih )
{
	mih->SetLightingMode( SGRX_LM_Dynamic );
	for( int i = 10; i < 16; ++i )
		mih->constants[ i ] = V4(0.15f);
}


