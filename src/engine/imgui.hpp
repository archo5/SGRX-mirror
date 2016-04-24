

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
ENGINE_EXPORT bool IMGUIEditFloat( const char* label, float& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditVec3( const char* label, Vec3& v, float vmin, float vmax, int prec = 2 );
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
	
	ImVec2 m_itemSize;
	// search
	bool m_looseSearch;
	String m_searchString;
	Array< int > m_filtered;
};

struct IF_GCC(ENGINE_EXPORT) IMGUIFilePicker : IMGUIPickerCore, IDirEntryHandler
{
	ENGINE_EXPORT IMGUIFilePicker( const char* dir, const char* ext );
	ENGINE_EXPORT bool Popup( const char* caption, String& str, bool save );
	ENGINE_EXPORT void Reload();
	ENGINE_EXPORT virtual bool SearchUI( String& str );
	
	ENGINE_EXPORT bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir );
	virtual size_t GetEntryCount() const { return m_entries.size(); }
	virtual RCString GetEntryPath( size_t i ) const { return m_entries[ i ]; }
	ENGINE_EXPORT virtual bool EntryUI( size_t i, String& str );
	ENGINE_EXPORT virtual bool ConfirmPopup( const char* caption, const char* label, const char* file );
	
	bool m_saveMode;
	const char* m_directory;
	const char* m_extension;
	Array< RCString > m_entries;
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
	if( ImGui::Button( addbtn, ImVec2( ImGui::GetContentRegionAvail().x, 24 ) ) )
	{
		data.push_back( T() );
	}
	
	for( size_t i = 0; i < data.size(); ++i )
	{
		ImVec2 cp_before = ImGui::GetCursorPos();
		float width = ImGui::GetContentRegionAvail().x;
		editfn( i, data[ i ] );
		ImVec2 cp_after = ImGui::GetCursorPos();
		
		if( i > 0 )
		{
			ImGui::SetCursorPos( cp_before + ImVec2( width - 90, 0 ) );
			if( ImGui::Button( "[up]", ImVec2( 30, 14 ) ) )
				TSWAP( data[ i - 1 ], data[ i ] );
		}
		
		if( i < data.size() - 1 )
		{
			ImGui::SetCursorPos( cp_before + ImVec2( width - 60, 0 ) );
			if( ImGui::Button( "[dn]", ImVec2( 30, 14 ) ) )
				TSWAP( data[ i ], data[ i + 1 ] );
		}
		
		ImGui::SetCursorPos( cp_before + ImVec2( width - 30, 0 ) );
		if( ImGui::Button( "[del]", ImVec2( 30, 14 ) ) )
			data.erase( i-- );
		ImGui::SetCursorPos( cp_after );
	}
}

#define IMGUI_HSPLIT( at, left, right ) \
	if( ImGui::BeginChild( "hs:lft", ImGui::GetContentRegionAvail() * ImVec2( at, 1 ), true ) ){ \
		left; ImGui::EndChild(); \
	} ImGui::SameLine( ImGui::GetContentRegionAvail().x * at ); \
	if( ImGui::BeginChild( "hs:rgt", ImGui::GetContentRegionAvail(), true ) ){ \
		right; ImGui::EndChild(); \
	}

#define IMGUI_GROUP( name, opened, cont ) ImGui::SetNextTreeNodeOpened( opened, ImGuiSetCond_Appearing ); \
	if( ImGui::TreeNode( name ) ){ cont; ImGui::TreePop(); }

template< class T > bool IMGUIListbox( const char* name, T& val, const char** list, int lsize )
{
	int curr = val;
	bool ret = ImGui::ListBox( name, &curr, list, lsize );
	if( ret )
		val = curr;
	return ret;
}
#define IMGUI_LISTBOX( name, val, list ) IMGUIListbox( name, val, list, SGRX_ARRAY_SIZE( list ) )

template< class T > bool IMGUIComboBox( const char* name, T& val, const char** list, int lsize )
{
	int curr = val;
	bool ret = ImGui::Combo( name, &curr, list, lsize );
	if( ret )
		val = curr;
	return ret;
}
#define IMGUI_COMBOBOX( name, val, list ) IMGUIComboBox( name, val, list, SGRX_ARRAY_SIZE( list ) )



