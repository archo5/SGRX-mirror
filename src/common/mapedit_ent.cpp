

#include "mapedit.hpp"



void FieldBool::EditUI()
{
	IMGUIEditBool( caption.c_str(), value );
}

void FieldInt::EditUI()
{
	IMGUIEditInt( caption.c_str(), value, vmin, vmax );
}

void FieldFloat::EditUI()
{
	IMGUIEditFloat( caption.c_str(), value, vmin, vmax, prec );
}

void FieldVec2::EditUI()
{
	IMGUIEditVec2( caption.c_str(), value, vmin, vmax, prec );
}

void FieldVec3::EditUI()
{
	IMGUIEditVec3( caption.c_str(), value, vmin, vmax, prec );
}

void FieldString::EditUI()
{
	switch( type )
	{
	case FT_Mesh:
		g_NUIMeshPicker->Property( "Select mesh", caption.c_str(), value );
		break;
	case FT_Tex:
		g_NUITexturePicker->Property( "Select texture", caption.c_str(), value );
		break;
	case FT_Char:
		g_NUICharPicker->Property( "Select character", caption.c_str(), value );
		break;
	case FT_PartSys:
		g_NUIPartSysPicker->Property( "Select particle system", caption.c_str(), value );
		break;
	case FT_Sound:
		g_NUISoundPicker->Property( "Select sound event", caption.c_str(), value );
		break;
	default:
		IMGUIEditString( caption.c_str(), value, 256 );
		break;
	}
}


sgsVariable ENT_Store()
{
	return g_Level->GetScriptCtx().GetGlobal( "ED_ENTITY_TYPES" );
}

void ENT_EnumEntities( Array< StringView >& out )
{
	ScriptVarIterator it( ENT_Store() );
	while( it.Advance() )
	{
		out.push_back( it.GetKey().get<StringView>() );
	}
}

sgsVariable ENT_GetEntityInterface( StringView name )
{
	return ENT_Store().getprop( FVar( name ) );
}


void EdEntity::DebugDraw()
{
	sgsVariable fn = m_entIface.getprop( "EditorDrawWorld" );
	m_data.thiscall( g_Level->GetSGSC(), fn );
}


EdEntity::EdEntity( sgsString type, bool isproto ) :
	EdObject( ObjType_Entity ),
	m_isproto( isproto ),
	m_pos( V3(0) ),
	m_entityType( type )
{
	SGS_CTX = g_Level->GetSGSC();
	
	StringView typestr( type.c_str(), type.size() );
	m_entIface = ENT_GetEntityInterface( typestr );
	m_iconTex = GR_GetTexture( m_entIface.getprop("ED_Icon").getdef<StringView>(
		SGRXPATH_SRC_EDITOR "/icons/default.png" ) );
	
	{
		SGS_SCOPE;
		sgs_PushPtr( C, this );
		m_entIface.thiscall( C, "ED_PropInfo", 1 );
	}
	
	Fields2Data();
}

EdEntity::~EdEntity()
{
}

EdEntity& EdEntity::operator = ( const EdEntity& o )
{
	m_pos = o.m_pos;
	m_entityType = o.m_entityType;
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		m_fields[ i ] = m_fields[ i ]->Clone();
	}
	Fields2Data();
	return *this;
}

EdObject* EdEntity::Clone()
{
	EdEntity* N = new EdEntity( m_entityType, false );
	*N = *this;
	g_EdWorld->SetEntityID( N );
	N->selected = selected;
	N->group = group;
	return N;
}

void EdEntity::Serialize( SVHBR& arch )
{
	arch << m_pos;
	String data;
	arch << data;
	m_data = g_Level->GetScriptCtx().Unserialize( data );
	Data2Fields();
	Fields2Data();
}

void EdEntity::Serialize( SVHBW& arch )
{
	arch << m_pos;
	String data;
	data = g_Level->GetScriptCtx().Serialize( m_data );
	arch << data;
}

void EdEntity::FLoad( sgsVariable data, int version )
{
	UNUSED( version );
	ASSERT( data.getprop( "entity_type" ) == m_entityType );
	sgsVariable props = data.getprop( "props" );
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		FieldBase* F = m_fields[ i ];
		sgsVariable val = props.getprop( F->key );
		F->SetFromVar( val );
		if( F->type == FT_Vec3 && F->key.equals( "position" ) )
			m_pos = ((FieldVec3*)F)->value;
		if( F->type == FT_String && F->key.equals( "id" ) && ((FieldString*)F)->value.size() == 0 )
			g_EdWorld->SetEntityID( this );
	}
	Fields2Data();
}

sgsVariable EdEntity::FSave( int version )
{
	UNUSED( version );
	sgsVariable props = FNewDict();
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		FieldBase* F = m_fields[ i ];
		props.setprop( F->key, F->ToVar() );
	}
	
	sgsVariable out = FNewDict();
	out.setprop( "entity_type", m_entityType );
	out.setprop( "props", props );
	return out;
}

void EdEntity::SetPosition( const Vec3& pos )
{
	m_pos = pos;
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		FieldBase* F = m_fields[ i ];
		if( F->type == FT_Vec3 && F->key.equals( "position" ) )
		{
			((FieldVec3*)F)->value = m_pos;
		}
	}
	Fields2Data();
}

void EdEntity::EditUI()
{
	ImGui::Text( "Entity '%s' properties", m_entityType.c_str() );
	ImGui::Separator();
	
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		ImGui::BeginChangeCheck();
		
		FieldBase* F = m_fields[ i ];
		F->EditUI();
		
		if( ImGui::EndChangeCheck() )
		{
			if( F->type == FT_Vec3 && F->key.equals( "position" ) )
				m_pos = ((FieldVec3*)F)->value;
			Fields2Data();
		}
	}
}

void EdEntity::Data2Fields()
{
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		FieldBase* F = m_fields[ i ];
		sgsVariable val = m_data.getprop( F->key );
		if( val.type_id() == SGS_VT_NULL )
			continue;
		F->SetFromVar( val );
	}
}

void EdEntity::Fields2Data()
{
	sgsVariable data = FNewDict();
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		FieldBase* F = m_fields[ i ];
		data.setprop( F->key, F->ToVar() );
	}
	data.set_meta_obj( m_entIface );
	m_data = data;
}

void EdEntity::SetID( StringView idstr )
{
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		FieldBase* F = m_fields[ i ];
		if( F->type == FT_String && F->key.equals( "id" ) )
		{
			((FieldString*)F)->value = idstr;
		}
	}
	Fields2Data();
}

void EdEntity::AddField( sgsString key, sgsString name, FieldBase* F )
{
	F->key = key;
	F->caption = name;
	m_fields.push_back( F );
}


static int EE_AddFieldBool( SGS_CTX )
{
	SGSFN( "EE_AddFieldBool" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldBool* F = new FieldBool;
	F->value = sgs_StackSize( C ) > 3 ? sgs_GetVar<bool>()( C, 3 ) : false;
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldInt( SGS_CTX )
{
	SGSFN( "EE_AddFieldInt" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldInt* F = new FieldInt;
	F->value = sgs_StackSize( C ) > 3 ? sgs_GetVar<int32_t>()( C, 3 ) : 0;
	F->vmin = sgs_StackSize( C ) > 4 ? sgs_GetVar<int32_t>()( C, 4 ) : 0x80000000;
	F->vmax = sgs_StackSize( C ) > 5 ? sgs_GetVar<int32_t>()( C, 5 ) : 0x7fffffff;
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldFloat( SGS_CTX )
{
	SGSFN( "EE_AddFieldFloat" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldFloat* F = new FieldFloat;
	F->value = sgs_StackSize( C ) > 3 ? sgs_GetVar<float>()( C, 3 ) : 0;
	F->prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	F->vmin = sgs_StackSize( C ) > 5 ? sgs_GetVar<float>()( C, 5 ) : -FLT_MAX;
	F->vmax = sgs_StackSize( C ) > 6 ? sgs_GetVar<float>()( C, 6 ) : FLT_MAX;
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldVec2( SGS_CTX )
{
	SGSFN( "EE_AddFieldVec2" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldVec2* F = new FieldVec2;
	F->value = sgs_StackSize( C ) > 3 ? sgs_GetVar<Vec2>()( C, 3 ) : V2(0);
	F->prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	F->vmin = sgs_StackSize( C ) > 5 ? sgs_GetVar<float>()( C, 5 ) : -FLT_MAX;
	F->vmax = sgs_StackSize( C ) > 6 ? sgs_GetVar<float>()( C, 6 ) : FLT_MAX;
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldVec3( SGS_CTX )
{
	SGSFN( "EE_AddFieldVec3" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldVec3* F = new FieldVec3;
	F->value = sgs_StackSize( C ) > 3 ? sgs_GetVar<Vec3>()( C, 3 ) : V3(0);
	F->prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	F->vmin = sgs_StackSize( C ) > 5 ? sgs_GetVar<float>()( C, 5 ) : -FLT_MAX;
	F->vmax = sgs_StackSize( C ) > 6 ? sgs_GetVar<float>()( C, 6 ) : FLT_MAX;
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldString( SGS_CTX )
{
	SGSFN( "EE_AddFieldString" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldString* F = new FieldString;
	F->value = sgs_GetVar<StringView>()( C, 3 );
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldEnumSB( SGS_CTX )
{
	SGSFN( "EE_AddFieldEnumSB" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldEnumSB* F = new FieldEnumSB;
	ScriptVarIterator it( sgs_GetVar<sgsVariable>()( C, 4 ) );
	while( it.Advance() )
	{
		F->m_picker.m_entries.set( it.GetKey().get<int32_t>(), it.GetValue().get<StringView>() );
	}
	F->value = sgs_GetVar<int32_t>()( C, 3 );
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldMesh( SGS_CTX )
{
	SGSFN( "EE_AddFieldMesh" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldString* F = new FieldString( FT_Mesh );
	F->value = sgs_GetVar<StringView>()( C, 3 );
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldTex( SGS_CTX )
{
	SGSFN( "EE_AddFieldTex" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldString* F = new FieldString( FT_Tex );
	F->value = sgs_GetVar<StringView>()( C, 3 );
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldChar( SGS_CTX )
{
	SGSFN( "EE_AddFieldChar" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldString* F = new FieldString( FT_Char );
	F->value = sgs_GetVar<StringView>()( C, 3 );
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldPartSys( SGS_CTX )
{
	SGSFN( "EE_AddFieldPartSys" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldString* F = new FieldString( FT_PartSys );
	F->value = sgs_GetVar<StringView>()( C, 3 );
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}
static int EE_AddFieldSound( SGS_CTX )
{
	SGSFN( "EE_AddFieldSound" );
	SGRX_CAST( EdEntity*, E, sgs_GetVar<void*>()( C, 0 ) );
	FieldString* F = new FieldString( FT_Sound );
	F->value = sgs_GetVar<StringView>()( C, 3 );
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), F );
	return 0;
}

static int EE_GenMatrix_SRaP( SGS_CTX )
{
	SGSFN( "EE_GenMatrix_SRaP" );
	Vec3 scale = sgs_GetVar<Vec3>()( C, 0 );
	Vec3 rot_angles = sgs_GetVar<Vec3>()( C, 1 );
	Vec3 position = sgs_GetVar<Vec3>()( C, 2 );
	Mat4 out = Mat4::CreateSRT( scale, DEG2RAD( rot_angles ), position );
	sgs_PushVar( C, out );
	return 1;
}

sgs_RegIntConst g_ent_scripted_ric[] =
{
	{ "LM_MESHINST_SOLID", LM_MESHINST_SOLID },
	{ "LM_MESHINST_DYNLIT", LM_MESHINST_DYNLIT },
	{ "LM_MESHINST_CASTLMS", LM_MESHINST_CASTLMS },
	{ "LM_MESHINST_EDITOR_ONLY", LM_MESHINST_EDITOR_ONLY },
};

sgs_RegFuncConst g_ent_scripted_rfc[] =
{
	{ "EE_AddFieldBool", EE_AddFieldBool },
	{ "EE_AddFieldInt", EE_AddFieldInt },
	{ "EE_AddFieldFloat", EE_AddFieldFloat },
	{ "EE_AddFieldVec2", EE_AddFieldVec2 },
	{ "EE_AddFieldVec3", EE_AddFieldVec3 },
	{ "EE_AddFieldString", EE_AddFieldString },
	{ "EE_AddFieldEnumSB", EE_AddFieldEnumSB },
	{ "EE_AddFieldMesh", EE_AddFieldMesh },
	{ "EE_AddFieldTex", EE_AddFieldTex },
	{ "EE_AddFieldChar", EE_AddFieldChar },
	{ "EE_AddFieldPartSys", EE_AddFieldPartSys },
	{ "EE_AddFieldSound", EE_AddFieldSound },
	
	{ "EE_GenMatrix_SRaP", EE_GenMatrix_SRaP },
	SGS_RC_END(),
};


static int _EEL_SortByName( const void* a, const void* b )
{
	return ((const StringView*)a)->compare_to( *(const StringView*)b );
}

EdEntList::EdEntList() : which( 0 )
{
	Array< StringView > ents;
	ENT_EnumEntities( ents );
	
	qsort( ents.data(), ents.size(), sizeof(ents[0]), _EEL_SortByName );
	
	ASSERT( ents.size() > 0 && "No entities were found!" );
	
	for( size_t i = 0; i < ents.size(); ++i )
	{
		sgsString name = FVar( ents[ i ] ).get_string();
		Entity e = { name, new EdEntity( name, true ) };
		entities.push_back( e );
	}
}

void EdEntList::EditUI()
{
	IMGUI_GROUP_BEGIN( "Entities", true )
	for( size_t i = 0; i < entities.size(); ++i )
	{
		if( ImGui::RadioButton( entities[ i ].name.c_str(), which == i ) )
			which = i;
	}
	IMGUI_GROUP_END;
}



