

#include "mapedit.hpp"

#include "resources.hpp"


bool EDGO_RayIntersect( GameObject* obj, Vec3 rpos, Vec3 rdir, float outdst[1] )
{
	return RaySphereIntersect( rpos, rdir, obj->GetWorldPosition(), 0.2f, outdst );
}

void EDGO_EditUI( GameObject* obj )
{
	{
		ImVec2 cp_before = ImGui::GetCursorPos();
		float width = ImGui::GetContentRegionAvail().x;
		
		ImGui::Text( "Game object properties" );
		ImGui::Separator();
		
		ImVec2 cp_after = ImGui::GetCursorPos();
		
		ImGui::SetCursorPos( cp_before + ImVec2( width - 30, 0 ) );
		if( ImGui::Button( "[copy]", ImVec2( 30, 14 ) ) )
		{
			sgsVariable vardata = EDGO_FSave( obj, false );
			String data = g_Level->GetScriptCtx().ToSGSON( vardata );
			Window_SetClipboardText( String_Concat( "GAMEOBJ:", data ) );
		}
		
		ImGui::SetCursorPos( cp_after );
	}
	
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
	
	const char* tips[] = {
		"Interactive item", "Heat source", "Player", "<reserved>",
		"Target", "AI Alert",
		NULL,
	};
	IMGUIEditMask( "Info mask", obj->m_infoMask, 32, tips );
	
	ImGui::Separator();
	
	IMGUI_GROUP_BEGIN( "Behaviors", true )
	{
		static String bhvrtypename;
		if( ImGui::Button( "Add behavior" ) )
		{
			bhvrtypename = "";
			ImGui::OpenPopup( "add_behavior" );
		}
		
	//	ImGui::SetNextWindowContentSize( ImVec2(330,600) );
		if( ImGui::BeginPopup( "add_behavior" ) )
		{
			IMGUIEditString( "Filter type", bhvrtypename, 256 );
			
			size_t sz = obj->m_behaviors.size();
			
			Array< StringView > bhvrlist;
			g_Level->EnumBehaviors( bhvrlist );
			for( size_t i = 0; i < bhvrlist.size(); ++i )
			{
				if( bhvrlist[ i ].match_loose( bhvrtypename ) &&
					ImGui::Selectable( StackString<256>(bhvrlist[ i ]) ) )
				{
					GOBehavior* bhvr = obj->AddBehavior(
						g_Level->m_scriptCtx.CreateString( bhvrlist[ i ] ) );
					if( bhvr->m_src_guid.IsNull() )
						bhvr->m_src_guid.SetGenerated();
				}
			}
			
			if( sz != obj->m_behaviors.size() )
				ImGui::TriggerChangeCheck();
			
			ImGui::EndPopup();
		}
		
		for( size_t i = 0; i < obj->m_behaviors.size(); ++i )
		{
			ImGui::PushID( i );
			
			ImVec2 cp_before = ImGui::GetCursorPos();
			float width = ImGui::GetContentRegionAvail().x;
			
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "%s",
				obj->m_behaviors[ i ]->m_type.c_str() );
			IMGUI_GROUP_BEGIN( bfr, true )
			{
				ImplEditorUIHelper ieuih;
				obj->m_behaviors[ i ]->EditUI( &ieuih,
					g_Level->GetScriptCtx().GetGlobal( "ED_IMGUI" ) );
			}
			IMGUI_GROUP_END;
			
			ImVec2 cp_after = ImGui::GetCursorPos();
			
			if( i > 0 )
			{
				ImGui::SetCursorPos( cp_before + ImVec2( width - 90, 0 ) );
				if( ImGui::Button( "[up]", ImVec2( 30, 14 ) ) )
				{
					TSWAP( obj->m_behaviors[ i - 1 ], obj->m_behaviors[ i ] );
					ImGui::TriggerChangeCheck();
				}
			}
			
			if( i < obj->m_behaviors.size() - 1 )
			{
				ImGui::SetCursorPos( cp_before + ImVec2( width - 60, 0 ) );
				if( ImGui::Button( "[dn]", ImVec2( 30, 14 ) ) )
				{
					TSWAP( obj->m_behaviors[ i ], obj->m_behaviors[ i + 1 ] );
					ImGui::TriggerChangeCheck();
				}
			}
			
			ImGui::SetCursorPos( cp_before + ImVec2( width - 30, 0 ) );
			if( ImGui::Button( "[del]", ImVec2( 30, 14 ) ) )
			{
				obj->RemoveBehavior( obj->m_behaviors[ i ] );
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
		static String rsrctypename;
		if( ImGui::Button( "Add resource" ) )
		{
			rsrctypename = "";
			ImGui::OpenPopup( "add_resource" );
		}
		
		ImGui::SetNextWindowContentSize( ImVec2(330,600) );
		if( ImGui::BeginPopup( "add_resource" ) )
		{
			IMGUIEditString( "Filter type", rsrctypename, 256 );
			
			GOResource* rsrc = NULL;
			
			for( size_t i = 0; i < g_Level->m_goResourceMap.size(); ++i )
			{
				if( SV(g_Level->m_goResourceMap.item( i ).value.name).match_loose( rsrctypename ) &&
					ImGui::Selectable( g_Level->m_goResourceMap.item( i ).value.name ) )
					rsrc = obj->AddResource( g_Level->m_goResourceMap.item( i ).key );
			}
			
			if( rsrc )
			{
				if( rsrc->m_src_guid.IsNull() )
					rsrc->m_src_guid.SetGenerated();
				ImGui::TriggerChangeCheck();
			}
			
			ImGui::EndPopup();
		}
		
		for( size_t i = 0; i < obj->m_resources.size(); ++i )
		{
			ImGui::PushID( i );
			
			ImVec2 cp_before = ImGui::GetCursorPos();
			float width = ImGui::GetContentRegionAvail().x;
			
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "%s",
				obj->m_resources[ i ]->GetSGSInterface()->name );
			IMGUI_GROUP_BEGIN( bfr, true )
			{
				ImplEditorUIHelper ieuih;
				obj->m_resources[ i ]->EditUI( &ieuih,
					g_Level->GetScriptCtx().GetGlobal( "ED_IMGUI" ) );
			}
			IMGUI_GROUP_END;
			
			ImVec2 cp_after = ImGui::GetCursorPos();
			ImGui::SetCursorPos( cp_before + ImVec2( width - 30, 0 ) );
			if( ImGui::Button( "[del]", ImVec2( 30, 14 ) ) )
			{
				obj->RemoveResource( obj->m_resources[ i ] );
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

template< class T > void _EDGO_RegObj( T* obj )
{
	if( obj->m_src_guid.IsNull() )
		return;
	
	ScriptContext& SC = g_Level->GetScriptCtx();
	SC.GetGlobal( "ED_ILOAD" ).getprop( "objmap" ).setprop(
		SC.CreateString( obj->m_src_guid.ToString() ).get_variable(),
		obj->GetScriptedObject() );
}

GameObject* EDGO_FLoad( sgsVariable data )
{
	GameObject* obj = g_Level->CreateGameObject();
	
	obj->m_name = data.getprop( "name" ).get_string();
	obj->m_id = data.getprop( "id" ).get_string();
	SGRX_GUID guid = SGRX_GUID::ParseString(
		data.getprop( "guid" ).get<StringView>() );
	obj->m_src_guid = guid.NotNull() ? guid : SGRX_GUID::Generate();
	
	SGRX_GUID parent_guid = SGRX_GUID::ParseString(
		data.getprop( "parent" ).get<StringView>() );
	if( parent_guid.NotNull() )
	{
		ScriptContext& SC = g_Level->GetScriptCtx();
		SC.GetGlobal( "ED_ILOAD" ).tthiscall<void>( SC.C, "AddLink",
			obj->GetScriptedObject(), "parent",
			SC.CreateString( parent_guid.ToString() ) );
	}
	
	obj->SetLocalPosition( FLoadProp( data, "position", V3(0) ) );
	obj->SetLocalRotation( FLoadProp( data, "rotation", Quat::Identity ) );
	obj->SetLocalScale( FLoadProp( data, "scale", V3(1) ) );
	obj->SetInfoMask( FLoadProp( data, "infoMask", 0 ) );
	obj->SetInfoTarget( FLoadProp( data, "localInfoTarget", V3(0) ) );
	
	_EDGO_RegObj( obj );
	
	ScriptVarIterator it_rsrc( data.getprop( "resources" ) );
	while( it_rsrc.Advance() )
	{
		sgsVariable data_rsrc = it_rsrc.GetValue();
		uint32_t type = data_rsrc.getprop( "__type" ).get<uint32_t>();
		SGRX_GUID guid = SGRX_GUID::ParseString(
			data_rsrc.getprop( "__guid" ).get<StringView>() );
		g_Level->nextObjectGUID = guid.NotNull() ? guid : SGRX_GUID::Generate();
		
		GOResource* rsrc = obj->AddResource( type );
		if( rsrc == NULL )
		{
			LOG_ERROR << "FAILED TO LOAD RESOURCE type=" << type;
			continue;
		}
		
		_EDGO_RegObj( rsrc );
		
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", data_rsrc );
		rsrc->EditLoad( data_rsrc,
			g_Level->GetScriptCtx().GetGlobal( "ED_ILOAD" ) );
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", sgsVariable() );
	}
	
	ScriptVarIterator it_bhvr( data.getprop( "behaviors" ) );
	while( it_bhvr.Advance() )
	{
		sgsVariable data_bhvr = it_bhvr.GetValue();
		sgsString type = data_bhvr.getprop( "__type" ).get_string();
		SGRX_GUID guid = SGRX_GUID::ParseString(
			data_bhvr.getprop( "__guid" ).get<StringView>() );
		g_Level->nextObjectGUID = guid.NotNull() ? guid : SGRX_GUID::Generate();
	//	LOG << "BHVR" << guid << g_Level->nextObjectGUID;
		
		// TODO FIX MULTIPLE BEHAVIOR SUPPORT
		GOBehavior* bhvr = obj->RequireBehavior( type, true );
	//	if(bhvr)
	//		LOG << "---FINL" << bhvr->m_src_guid;
	//	else
	//		LOG << "----NO BHVR????" << name.c_str() << "|" << type.c_str();
		if( bhvr == NULL )
		{
			LOG_ERROR << "FAILED TO LOAD BEHAVIOR type=" << type.c_str();
			continue;
		}
		
		_EDGO_RegObj( bhvr );
		
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
		
		SGRX_GUID parent_guid = obj->GetParent() ? obj->GetParent()->m_src_guid : SGRX_GUID::Null;
		data.setprop( "parent", g_Level->GetScriptCtx().CreateString( parent_guid.ToString() ) );
	}
	FSaveProp( data, "position", obj->GetLocalPosition() );
	FSaveProp( data, "rotation", obj->GetLocalRotation() );
	FSaveProp( data, "scale", obj->GetLocalScale() );
	FSaveProp( data, "infoMask", obj->GetInfoMask() );
	FSaveProp( data, "localInfoTarget", obj->GetInfoTarget() );
	
	sgsVariable out_rsrc = FNewArray();
	for( size_t i = 0; i < obj->m_resources.size(); ++i )
	{
		GOResource* rsrc = obj->m_resources[ i ];
		sgsVariable data_rsrc = FNewDict();
		
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", data_rsrc );
		rsrc->EditSave( data_rsrc,
			g_Level->GetScriptCtx().GetGlobal( "ED_ISAVE" ) );
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", sgsVariable() );
		
		data_rsrc.setprop( "__type", sgsVariable().set_int( rsrc->m_rsrcType ) );
		if( guids )
		{
			data_rsrc.setprop( "__guid",
				g_Level->GetScriptCtx().CreateString( rsrc->m_src_guid.ToString() ) );
		}
		FArrayAppend( out_rsrc, data_rsrc );
	}
	data.setprop( "resources", out_rsrc );
	
	sgsVariable out_bhvr = FNewArray();
	for( size_t i = 0; i < obj->m_behaviors.size(); ++i )
	{
		GOBehavior* bhvr = obj->m_behaviors[ i ];
		sgsVariable data_bhvr = FNewDict();
		
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", data_bhvr );
		bhvr->EditSave( data_bhvr,
			g_Level->GetScriptCtx().GetGlobal( "ED_ISAVE" ) );
		g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", sgsVariable() );
		
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

sgsVariable EDGO_RSRC_LCSave( GOResource* rsrc )
{
	sgsVariable data = FNewDict();
	g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", data );
	rsrc->EditSave( data, g_Level->GetScriptCtx().GetGlobal( "ED_ILCSV" ) );
	g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", sgsVariable() );
	return data;
}

sgsVariable EDGO_BHVR_LCSave( GOBehavior* bhvr )
{
	sgsVariable data = FNewDict();
	g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", data );
	bhvr->EditSave( data, g_Level->GetScriptCtx().GetGlobal( "ED_ILCSV" ) );
	g_Level->GetScriptCtx().SetGlobal( "ED_SDATA", sgsVariable() );
	return data;
}

void EDGO_LCSave( GameObject* obj, LC_GameObject* out )
{
	out->name = StringView( obj->m_name.c_str(), obj->m_name.size() );
	out->id = StringView( obj->m_id.c_str(), obj->m_id.size() );
	out->transform = obj->GetLocalMatrix();
	out->infoMask = obj->GetInfoMask();
	out->localInfoTarget = obj->GetInfoTarget();
	out->guid = obj->m_src_guid;
	out->parent_guid = obj->GetParent() ?
		obj->GetParent()->m_src_guid : SGRX_GUID::Null;
	
	out->srlz_resources.reserve( obj->m_resources.size() );
	for( size_t i = 0; i < obj->m_resources.size(); ++i )
	{
		GOResource* rsrc = obj->m_resources[ i ];
		sgsVariable data_rsrc = EDGO_RSRC_LCSave( rsrc );
		
		data_rsrc.setprop( "__type", sgsVariable().set_int( rsrc->m_rsrcType ) );
		data_rsrc.setprop( "__guid",
			g_Level->GetScriptCtx().CreateString( rsrc->m_src_guid.ToString() ) );
		
		out->srlz_resources.push_back(
			g_Level->GetScriptCtx().Serialize( data_rsrc ) );
	}
	
	out->srlz_behaviors.reserve( obj->m_behaviors.size() );
	for( size_t i = 0; i < obj->m_behaviors.size(); ++i )
	{
		GOBehavior* bhvr = obj->m_behaviors[ i ];
		sgsVariable data_bhvr = EDGO_BHVR_LCSave( bhvr );
		
		data_bhvr.setprop( "__type", bhvr->m_type.get_variable() );
		data_bhvr.setprop( "__guid",
			g_Level->GetScriptCtx().CreateString( bhvr->m_src_guid.ToString() ) );
		
		out->srlz_behaviors.push_back(
			g_Level->GetScriptCtx().Serialize( data_bhvr ) );
	}
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

static int IMGUI_EditVec3( SGS_CTX )
{
	SGSFN( "ED_IMGUI.EditVec3" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	Vec3 value = obj.getprop( prop ).get<Vec3>();
	float vmin = sgs_StackSize( C ) > 3 ? sgs_GetVar<float>()( C, 3 ) : -FLT_MAX;
	float vmax = sgs_StackSize( C ) > 4 ? sgs_GetVar<float>()( C, 4 ) : FLT_MAX;
	int prec = sgs_StackSize( C ) > 5 ? sgs_GetVar<int>()( C, 5 ) : 2;
	
	if( IMGUIEditVec3( label.c_str(), value, vmin, vmax, prec ) )
		obj.setprop( prop, g_Level->GetScriptCtx().CreateVec3( value ) );
	
	return 0;
}

static int IMGUI_EditFlag( SGS_CTX )
{
	SGSFN( "ED_IMGUI.EditFlag" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	int value = obj.getprop( prop ).get<int>();
	int cflag = sgs_GetVar<int>()( C, 3 );
	
	if( IMGUIEditIntFlags( label.c_str(), value, cflag ) )
		obj.setprop( prop, sgsVariable().set_int( value ) );
	
	return 0;
}

static int IMGUI_EditMask( SGS_CTX )
{
	SGSFN( "ED_IMGUI.EditMask" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	int value = obj.getprop( prop ).get<int>();
	int count = sgs_StackSize( C ) > 3 ? sgs_GetVar<int>()( C, 3 ) : 32;
	
	if( IMGUIEditMask( label.c_str(), value, count ) )
		obj.setprop( prop, sgsVariable().set_int( value ) );
	
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

static int IMGUI_EditString( SGS_CTX )
{
	SGSFN( "ED_IMGUI.EditString" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	int maxlen = sgs_StackSize( C ) > 3 ? sgs_GetVar<int>()( C, 3 ) : 256;
	String value = obj.getprop( prop ).get<String>();
	
	if( IMGUIEditString( label.c_str(), value, maxlen ) )
		obj.setprop( prop, g_Level->GetScriptCtx().CreateString( value ).get_variable() );
	
	return 0;
}

static int IMGUI_PickMesh( SGS_CTX )
{
	SGSFN( "ED_IMGUI.PickMesh" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	sgsString caption( C, 3 );
	String value = obj.getprop( prop ).get<String>();
	
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
	String value = obj.getprop( prop ).get<String>();
	
	if( g_NUITexturePicker->Property( caption.c_str(), label.c_str(), value ) )
		obj.setprop( prop, g_Level->GetScriptCtx().CreateString( value ).get_variable() );
	
	return 0;
}

static int IMGUI_PickPartSys( SGS_CTX )
{
	SGSFN( "ED_IMGUI.PickPartSys" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	sgsString caption( C, 3 );
	String value = obj.getprop( prop ).get<String>();
	
	if( g_NUIPartSysPicker->Property( caption.c_str(), label.c_str(), value ) )
		obj.setprop( prop, g_Level->GetScriptCtx().CreateString( value ).get_variable() );
	
	return 0;
}

static int IMGUI_PickSound( SGS_CTX )
{
	SGSFN( "ED_IMGUI.PickSound" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	sgsString caption( C, 3 );
	String value = obj.getprop( prop ).get<String>();
	
	if( g_NUISoundPicker->Property( caption.c_str(), label.c_str(), value ) )
		obj.setprop( prop, g_Level->GetScriptCtx().CreateString( value ).get_variable() );
	
	return 0;
}

static int IMGUI_PickLocalRsrc( SGS_CTX )
{
	SGSFN( "ED_IMGUI.PickLocalRsrc" );
	sgsString label( C, 0 );
	sgsVariable obj( C, 1 );
	sgsString prop( C, 2 );
	GameObject* go = GameObject::ScrHandle( C, 3 );
	GOResource* rsrc = obj.getprop( prop ).get_handle<GOResource>();
	
	if( !go )
		return sgs_Msg( C, SGS_WARNING, "expected game object as argument 4" );
	
	ImGui::PushID( label.c_str() );
	char bfr[ 256 ];
	if( rsrc )
	{
		sgrx_snprintf( bfr, 256, "%s [%d]", rsrc->GetSGSInterface()->name,
			int(go->m_resources.find_first_at( rsrc )) );
	}
	if( ImGui::Button( rsrc ? bfr : "<none>",
			ImVec2( ImGui::GetContentRegionAvailWidth() * 2.f/3.f, 20 ) ) )
		ImGui::OpenPopup( "pick_local_rsrc" );
	ImGui::SameLine();
	ImGui::Text( "%s", label.c_str() );
	
	if( ImGui::BeginPopup( "pick_local_rsrc" ) )
	{
		if( ImGui::Selectable( "<none>" ) )
			obj.setprop( prop, sgsVariable() );
		for( size_t i = 0; i < go->m_resources.size(); ++i )
		{
			sgrx_snprintf( bfr, 256, "%s [%d]",
				go->m_resources[ i ]->GetSGSInterface()->name, int(i) );
			if( ImGui::Selectable( bfr ) )
			{
				obj.setprop( prop, go->m_resources[ i ]->GetScriptedObject() );
			}
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();
	
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
	{ "EditVec3", IMGUI_EditVec3 },
	{ "EditFlag", IMGUI_EditFlag },
	{ "EditMask", IMGUI_EditMask },
	{ "EditColorRGBLDR", IMGUI_EditColorRGBLDR },
	{ "EditXFMat4", IMGUI_EditXFMat4 },
	{ "EditString", IMGUI_EditString },
	{ "PickMesh", IMGUI_PickMesh },
	{ "PickTexture", IMGUI_PickTexture },
	{ "PickPartSys", IMGUI_PickPartSys },
	{ "PickSound", IMGUI_PickSound },
	{ "PickLocalRsrc", IMGUI_PickLocalRsrc },
	{ "ComboNT", IMGUI_ComboNT },
	{ "Button", IMGUI_Button },
	SGS_RC_END(),
};


