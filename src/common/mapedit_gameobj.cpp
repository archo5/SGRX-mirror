

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
	
	String name( obj->m_name.c_str(), obj->m_name.size() );
	if( IMGUIEditString( "Name", name, 256 ) )
		obj->m_name = g_Level->m_scriptCtx.CreateString( name );
	
	String id( obj->m_id.c_str(), obj->m_id.size() );
	if( IMGUIEditString( "ID", id, 256 ) )
		obj->m_id = g_Level->m_scriptCtx.CreateString( id );
	
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
	
	IMGUI_GROUP_BEGIN( "Behaviors", true )
	{
		static String bhvrname;
		if( ImGui::Button( "Add behavior" ) )
		{
			bhvrname = "";
			ImGui::OpenPopup( "add_behavior" );
		}
		
		if( ImGui::BeginPopup( "add_behavior" ) )
		{
			IMGUIEditString( "Name", bhvrname, 256 );
			sgsString sgsname = g_Level->m_scriptCtx.CreateString( bhvrname );
			bool used = obj->m_behaviors.isset( sgsname );
			bool can = !used && sgsname.size();
			if( can )
			{
				size_t sz = obj->m_behaviors.size();
				
				Array< StringView > bhvrlist;
				g_Level->EnumBehaviors( bhvrlist );
				for( size_t i = 0; i < bhvrlist.size(); ++i )
				{
					if( ImGui::Selectable( StackString<256>(bhvrlist[ i ]) ) )
					{
						GOBehavior* bhvr = obj->AddBehavior( sgsname, g_Level->m_scriptCtx.CreateString( bhvrlist[ i ] ) );
						if( bhvr->m_src_guid.IsNull() )
							bhvr->m_src_guid.SetGenerated();
					}
				}
				
				if( sz != obj->m_behaviors.size() )
					ImGui::TriggerChangeCheck();
			}
			else
			{
				ImGui::SameLine();
				ImGui::Text( used ? "<used>" : "<empty>" );
			}
			ImGui::EndPopup();
		}
		
		for( size_t i = 0; i < obj->m_bhvr_order.size(); ++i )
		{
			ImGui::PushID( i );
			
			ImVec2 cp_before = ImGui::GetCursorPos();
			float width = ImGui::GetContentRegionAvail().x;
			
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "[%s] %s",
				obj->m_bhvr_order[ i ]->m_type.c_str(),
				obj->m_bhvr_order[ i ]->m_name.c_str() );
			IMGUI_GROUP_BEGIN( bfr, true )
			{
				ImplEditorUIHelper ieuih;
				obj->m_bhvr_order[ i ]->EditUI( &ieuih,
					g_Level->GetScriptCtx().GetGlobal( "ED_IMGUI" ) );
			}
			IMGUI_GROUP_END;
			
			ImVec2 cp_after = ImGui::GetCursorPos();
			
			if( i > 0 )
			{
				ImGui::SetCursorPos( cp_before + ImVec2( width - 90, 0 ) );
				if( ImGui::Button( "[up]", ImVec2( 30, 14 ) ) )
				{
					TSWAP( obj->m_bhvr_order[ i - 1 ], obj->m_bhvr_order[ i ] );
					ImGui::TriggerChangeCheck();
				}
			}
			
			if( i < obj->m_bhvr_order.size() - 1 )
			{
				ImGui::SetCursorPos( cp_before + ImVec2( width - 60, 0 ) );
				if( ImGui::Button( "[dn]", ImVec2( 30, 14 ) ) )
				{
					TSWAP( obj->m_bhvr_order[ i ], obj->m_bhvr_order[ i + 1 ] );
					ImGui::TriggerChangeCheck();
				}
			}
			
			ImGui::SetCursorPos( cp_before + ImVec2( width - 30, 0 ) );
			if( ImGui::Button( "[del]", ImVec2( 30, 14 ) ) )
			{
				obj->RemoveBehavior( obj->m_bhvr_order[ i ]->m_name );
				ImGui::TriggerChangeCheck();
			}
			
			ImGui::SetCursorPos( cp_after );
			
			ImGui::PopID();
		}
	}
	IMGUI_GROUP_END;
	
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
				GOResource* rsrc = NULL;
				
				if( ImGui::Selectable( "Mesh resource" ) )
					rsrc = obj->AddResource( sgsname, GO_RSRC_MESH );
				if( ImGui::Selectable( "Light resource" ) )
					rsrc = obj->AddResource( sgsname, GO_RSRC_LIGHT );
				
				if( rsrc )
				{
					if( rsrc->m_src_guid.IsNull() )
						rsrc->m_src_guid.SetGenerated();
					ImGui::TriggerChangeCheck();
				}
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
				ImplEditorUIHelper ieuih;
				obj->m_resources.item( i ).value->EditUI( &ieuih,
					g_Level->GetScriptCtx().GetGlobal( "ED_IMGUI" ) );
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

void EDGO_SerializeSpatial( GameObject* obj, ByteWriter& bw )
{
	Vec3 pos = obj->GetLocalPosition();
	Quat rot = obj->GetLocalRotation();
	Vec3 scl = obj->GetLocalScale();
	bw << pos << rot << scl;
}

void EDGO_SerializeSpatial( GameObject* obj, ByteReader& br )
{
	Vec3 pos, scl;
	Quat rot;
	br << pos << rot << scl;
	obj->SetLocalPosition( pos );
	obj->SetLocalRotation( rot );
	obj->SetLocalScale( scl );
}

GameObject* EDGO_FLoad( sgsVariable data )
{
	GameObject* obj = g_Level->CreateGameObject();
	
	obj->m_name = data.getprop( "name" ).get_string();
	obj->m_id = data.getprop( "id" ).get_string();
	SGRX_GUID guid = SGRX_GUID::ParseString(
		data.getprop( "guid" ).get<StringView>() );
	obj->m_src_guid = guid.NotNull() ? guid : SGRX_GUID::Generate();
	obj->SetLocalPosition( FLoadProp( data, "position", V3(0) ) );
	obj->SetLocalRotation( FLoadProp( data, "rotation", Quat::Identity ) );
	obj->SetLocalScale( FLoadProp( data, "scale", V3(1) ) );
	
	ScriptVarIterator it_rsrc( data.getprop( "resources" ) );
	while( it_rsrc.Advance() )
	{
		sgsVariable data_rsrc = it_rsrc.GetValue();
		sgsString name = data_rsrc.getprop( "__name" ).get_string();
		uint32_t type = data_rsrc.getprop( "__type" ).get<uint32_t>();
		SGRX_GUID guid = SGRX_GUID::ParseString(
			data_rsrc.getprop( "__guid" ).get<StringView>() );
		g_Level->nextObjectGUID = guid.NotNull() ? guid : SGRX_GUID::Generate();
		
		GOResource* rsrc = obj->AddResource( name, type );
		if( rsrc == NULL )
		{
			LOG_ERROR << "FAILED TO LOAD RESOURCE type="
				<< type << ", name=" << name.c_str();
			continue;
		}
		
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", data_rsrc );
		rsrc->EditLoad( data_rsrc,
			g_Level->GetScriptCtx().GetGlobal( "ED_ILOAD" ) );
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", sgsVariable() );
	}
	
	ScriptVarIterator it_bhvr( data.getprop( "behaviors" ) );
	while( it_bhvr.Advance() )
	{
		sgsVariable data_bhvr = it_bhvr.GetValue();
		sgsString name = data_bhvr.getprop( "__name" ).get_string();
		sgsString type = data_bhvr.getprop( "__type" ).get_string();
		SGRX_GUID guid = SGRX_GUID::ParseString(
			data_bhvr.getprop( "__guid" ).get<StringView>() );
		g_Level->nextObjectGUID = guid.NotNull() ? guid : SGRX_GUID::Generate();
		
		GOBehavior* bhvr = obj->AddBehavior( name, type );
		if( bhvr == NULL )
		{
			LOG_ERROR << "FAILED TO LOAD BEHAVIOR type="
				<< type.c_str() << ", name=" << name.c_str();
			continue;
		}
		bhvr->m_src_guid = guid.NotNull() ? guid : SGRX_GUID::Generate();
		
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", data_bhvr );
		bhvr->EditLoad( data_bhvr,
			g_Level->GetScriptCtx().GetGlobal( "ED_ILOAD" ) );
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", sgsVariable() );
	}
	
	return obj;
}

sgsVariable EDGO_FSave( GameObject* obj, bool guids )
{
	sgsVariable data = FNewDict();
	
	data.setprop( "name", obj->m_name.get_variable() );
	data.setprop( "id", obj->m_id.get_variable() );
	if( guids )
	{
		data.setprop( "guid", g_Level->GetScriptCtx().CreateString(
			obj->m_src_guid.ToString() ) );
	}
	FSaveProp( data, "position", obj->GetLocalPosition() );
	FSaveProp( data, "rotation", obj->GetLocalRotation() );
	FSaveProp( data, "scale", obj->GetLocalScale() );
	
	sgsVariable out_rsrc = FNewArray();
	for( size_t i = 0; i < obj->m_resources.size(); ++i )
	{
		GOResource* rsrc = obj->m_resources.item( i ).value;
		sgsVariable data_rsrc = FNewDict();
		
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", data_rsrc );
		rsrc->EditSave( data_rsrc,
			g_Level->GetScriptCtx().GetGlobal( "ED_ISAVE" ) );
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", sgsVariable() );
		
		data_rsrc.setprop( "__name", rsrc->m_name.get_variable() );
		data_rsrc.setprop( "__type", sgsVariable().set_int( rsrc->m_type ) );
		if( guids )
		{
			data_rsrc.setprop( "__guid",
				g_Level->GetScriptCtx().CreateString( rsrc->m_src_guid.ToString() ) );
		}
		FArrayAppend( out_rsrc, data_rsrc );
	}
	data.setprop( "resources", out_rsrc );
	
	sgsVariable out_bhvr = FNewArray();
	for( size_t i = 0; i < obj->m_bhvr_order.size(); ++i )
	{
		GOBehavior* bhvr = obj->m_bhvr_order[ i ];
		sgsVariable data_bhvr = FNewDict();
		
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", data_bhvr );
		bhvr->EditSave( data_bhvr,
			g_Level->GetScriptCtx().GetGlobal( "ED_ISAVE" ) );
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", sgsVariable() );
		
		data_bhvr.setprop( "__name", bhvr->m_name.get_variable() );
		data_bhvr.setprop( "__type", bhvr->m_type.get_variable() );
		if( guids )
		{
			data_bhvr.setprop( "__guid",
				g_Level->GetScriptCtx().CreateString( bhvr->m_src_guid.ToString() ) );
		}
		FArrayAppend( out_bhvr, data_bhvr );
	}
	data.setprop( "behaviors", out_bhvr );
	
	return data;
}

GameObject* EDGO_Clone( GameObject* obj )
{
	sgsVariable data = EDGO_FSave( obj, false );
	return EDGO_FLoad( data );
}


static int IMGUI_EditBool( SGS_CTX )
{
	SGSFN( "ED_IMGUI.EditBool" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	bool value = obj.getprop( prop ).get<bool>();
	
	if( IMGUIEditBool( label.c_str(), value ) )
		obj.setprop( prop, sgsVariable().set_bool( value ) );
	
	return 0;
}

static int IMGUI_EditInt( SGS_CTX )
{
	SGSFN( "ED_IMGUI.EditInt" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	int value = obj.getprop( prop ).get<int>();
	int vmin = sgs_StackSize( C ) > 3 ? sgs_GetVar<int>()( C, 3 ) : -FLT_MAX;
	int vmax = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : FLT_MAX;
	
	if( IMGUIEditInt( label.c_str(), value, vmin, vmax ) )
		obj.setprop( prop, sgsVariable().set_int( value ) );
	
	return 0;
}

static int IMGUI_EditFloat( SGS_CTX )
{
	SGSFN( "ED_IMGUI.EditFloat" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	float value = obj.getprop( prop ).get<float>();
	float vmin = sgs_StackSize( C ) > 3 ? sgs_GetVar<float>()( C, 3 ) : -FLT_MAX;
	float vmax = sgs_StackSize( C ) > 4 ? sgs_GetVar<float>()( C, 4 ) : FLT_MAX;
	int prec = sgs_StackSize( C ) > 5 ? sgs_GetVar<int>()( C, 5 ) : 2;
	
	if( IMGUIEditFloat( label.c_str(), value, vmin, vmax, prec ) )
		obj.setprop( prop, sgsVariable().set_real( value ) );
	
	return 0;
}

static int IMGUI_EditColorRGBLDR( SGS_CTX )
{
	SGSFN( "ED_IMGUI.EditColorRGBLDR" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	Vec3 value = obj.getprop( prop ).get<Vec3>();
	
	if( IMGUIEditColorRGBLDR( label.c_str(), value ) )
		obj.setprop( prop, g_Level->GetScriptCtx().CreateVec3( value ) );
	
	return 0;
}

static int IMGUI_EditXFMat4( SGS_CTX )
{
	SGSFN( "ED_IMGUI.EditXFMat4" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	Mat4 value = obj.getprop( prop ).get<Mat4>();
	
	if( IMGUIEditXFMat4( label.c_str(), value ) )
		obj.setprop( prop, g_Level->GetScriptCtx().CreateMat4( value ) );
	
	return 0;
}

static int IMGUI_PickMesh( SGS_CTX )
{
	SGSFN( "ED_IMGUI.PickMesh" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	sgsString caption( C, 3 );
	String value = obj.getprop( prop ).get<StringView>();
	
	if( g_NUIMeshPicker->Property( caption.c_str(), label.c_str(), value ) )
		obj.setprop( prop, g_Level->GetScriptCtx().CreateString( value ).get_variable() );
	
	return 0;
}

static int IMGUI_PickTexture( SGS_CTX )
{
	SGSFN( "ED_IMGUI.PickTexture" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	sgsString caption( C, 3 );
	String value = obj.getprop( prop ).get<StringView>();
	
	if( g_NUITexturePicker->Property( caption.c_str(), label.c_str(), value ) )
		obj.setprop( prop, g_Level->GetScriptCtx().CreateString( value ).get_variable() );
	
	return 0;
}

static int IMGUI_ComboNT( SGS_CTX )
{
	SGSFN( "ED_IMGUI.ComboNT" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	sgsString zssl( C, 3 );
	int start = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 0;
	int value = obj.getprop( prop ).get<int>();
	if( zssl.c_str()[ zssl.size() - 1 ] != '\0' )
		return sgs_Msg( C, SGS_WARNING, "expected extra NUL character at end of string" );
	
	if( IMGUIComboBox( label.c_str(), value, zssl.c_str(), start ) )
		obj.setprop( prop, sgsVariable().set_int( value ) );
	
	return 0;
}

static int IMGUI_Button( SGS_CTX )
{
	SGSFN( "ED_IMGUI.Button" );
	sgsString label( C, 0 );
	return sgs_PushBool( C, ImGui::Button( label.c_str() ) );
}

sgs_RegFuncConst g_imgui_rfc[] =
{
	{ "EditBool", IMGUI_EditBool },
	{ "EditInt", IMGUI_EditInt },
	{ "EditFloat", IMGUI_EditFloat },
	{ "EditColorRGBLDR", IMGUI_EditColorRGBLDR },
	{ "EditXFMat4", IMGUI_EditXFMat4 },
	{ "PickMesh", IMGUI_PickMesh },
	{ "PickTexture", IMGUI_PickTexture },
	{ "ComboNT", IMGUI_ComboNT },
	{ "Button", IMGUI_Button },
	SGS_RC_END(),
};


