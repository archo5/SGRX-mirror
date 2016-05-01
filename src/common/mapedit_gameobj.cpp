

#include "mapedit.hpp"

#include "resources.hpp"


bool EDGO_RayIntersect( GameObject* obj, Vec3 rpos, Vec3 rdir, float outdst[1] )
{
	return RaySphereIntersect( rpos, rdir, obj->GetWorldPosition(), 0.2f, outdst );
}

void EDGO_EditUI( GameObject* obj )
{
	ImGui::Text( "Game object properties" );
	ImGui::Separator();
	
	Vec3 pos = obj->GetLocalPosition();
	if( IMGUIEditVec3( "Position", pos, -8192, 8192 ) && pos != obj->GetLocalPosition() )
		obj->SetLocalPosition( pos );
	
	Quat rot = obj->GetLocalRotation();
	if( IMGUIEditQuat( "Rotation", rot ) && rot != obj->GetLocalRotation() )
		obj->SetLocalRotation( rot );
	
	Vec3 scl = obj->GetLocalScale();
	if( IMGUIEditVec3( "Scale", scl, -8192, 8192 ) && scl != obj->GetLocalScale() )
		obj->SetLocalScale( scl );
	
	ImGui::Separator();
	
	IMGUI_GROUP_BEGIN( "Resources", true )
	{
		static String rsrcname;
		if( ImGui::Button( "Add resource" ) )
		{
			rsrcname = "";
			ImGui::OpenPopup( "add_resource" );
		}
		
		if( ImGui::BeginPopup( "add_resource" ) )
		{
			IMGUIEditString( "Name", rsrcname, 256 );
			sgsString sgsname = g_Level->m_scriptCtx.CreateString( rsrcname );
			bool used = obj->m_resources.isset( sgsname );
			bool can = !used && sgsname.size();
			if( can )
			{
				size_t sz = obj->m_resources.size();
				
				if( ImGui::Selectable( "Mesh resource" ) )
					obj->AddResource( sgsname, GO_RSRC_MESH );
				
				if( sz != obj->m_resources.size() )
					ImGui::TriggerChangeCheck();
			}
			else
			{
				ImGui::SameLine();
				ImGui::Text( used ? "<used>" : "<empty>" );
			}
			ImGui::EndPopup();
		}
		
		for( size_t i = 0; i < obj->m_resources.size(); ++i )
		{
			ImGui::PushID( i );
			
			ImVec2 cp_before = ImGui::GetCursorPos();
			float width = ImGui::GetContentRegionAvail().x;
			
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "[%s] %s",
				obj->m_resources.item( i ).value->GetSGSInterface()->name,
				obj->m_resources.item( i ).key.c_str() );
			IMGUI_GROUP_BEGIN( bfr, true )
			{
				obj->m_resources.item( i ).value->EditUI( &g_UIFrame->m_emGameObjects );
			}
			IMGUI_GROUP_END;
			
			ImVec2 cp_after = ImGui::GetCursorPos();
			ImGui::SetCursorPos( cp_before + ImVec2( width - 30, 0 ) );
			if( ImGui::Button( "[del]", ImVec2( 30, 14 ) ) )
			{
				obj->RemoveResource( obj->m_resources.item( i ).key );
				ImGui::TriggerChangeCheck();
			}
			ImGui::SetCursorPos( cp_after );
			
			ImGui::PopID();
		}
	}
	IMGUI_GROUP_END;
}


