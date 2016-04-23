

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


ENGINE_EXPORT bool IMGUIEditFloat( const char* label, float& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditVec3( const char* label, Vec3& v, float vmin, float vmax, int prec = 2 );
ENGINE_EXPORT bool IMGUIEditQuat( const char* label, Quat& v );
ENGINE_EXPORT bool IMGUIEditString( const char* label, String& str, int maxsize );


struct IF_GCC(ENGINE_EXPORT) IMGUIMeshPickerCore
{
	struct Entry
	{
		RCString path;
		MeshInstHandle mesh;
	};
	
	ENGINE_EXPORT bool Use( const char* label, String& str );
	ENGINE_EXPORT void _Search( StringView text );
	
	ENGINE_EXPORT IMGUIMeshPickerCore();
	ENGINE_EXPORT virtual ~IMGUIMeshPickerCore();
	ENGINE_EXPORT void Clear();
	ENGINE_EXPORT void AddMesh( StringView path, StringView rsrcpath = SV() );
	
	ENGINE_EXPORT void _DrawItem( int i, int x0, int y0, int x1, int y1 );
	struct _StaticDrawItemData
	{
		IMGUIMeshPickerCore* self;
		int i, x0, y0, x1, y1;
	};
	ENGINE_EXPORT static void _StaticDrawItem( const ImDrawList* parent_list, const ImDrawCmd* cmd );
	
	const char* m_caption;
	bool m_customCamera;
	bool m_looseSearch;
	Array< Entry > m_entries;
	Array< int > m_filtered;
	String m_searchString;
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
		editfn( data[ i ] );
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



