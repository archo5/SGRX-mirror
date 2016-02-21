

#include "mapedit.hpp"



void EdEntity::BeforeDelete()
{
	if( m_ownerEnt )
	{
		m_ownerEnt->m_subEnts.uerase( m_ownerEnt->m_subEnts.find_first_at( this ) );
	}
	m_ownerEnt = NULL;
	while( m_subEnts.size() )
	{
		g_EdWorld->DeleteObject( m_subEnts[0] );
	}
}

void EdEntity::DebugDraw()
{
	if( m_subEnts.size() )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
		if( m_subEnts.size() >= 2 )
		{
			br.Col( 0.0f, 0.1f, 0.9f ).SetPrimitiveType( PT_LineStrip );
			br.Pos( m_subEnts.last()->Pos() );
			for( size_t i = 0; i < m_subEnts.size(); ++i )
			{
				br.Pos( m_subEnts[ i ]->Pos() );
			}
		}
		br.SetPrimitiveType( PT_Lines );
		for( size_t i = 0; i < m_subEnts.size(); ++i )
		{
			br.Col( 0.0f, 0.4f, 0.9f ).Pos( Pos() );
			br.Col( 0.0f, 0.1f, 0.9f ).Pos( m_subEnts[ i ]->Pos() );
		}
	}
	if( m_ownerEnt && m_ownerEnt->selected == false )
		m_ownerEnt->DebugDraw();
}


EdEntNew::EdEntNew( sgsString type, bool isproto ) : EdEntity( isproto ), m_entityType( type )
{
	StringView typestr( type.c_str(), type.size() );
	sgsVariable eiface = g_Level->GetEntityInterface( typestr );
	m_iconTex = GR_GetTexture( eiface.getprop("ED_Icon").getdef<StringView>( "editor/icons/default.png" ) );
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s properties", StackString<240>(typestr).str );
	m_group.caption = bfr;
	m_group.SetOpen( true );
	Add( &m_group );
	
	{
		SGS_CSCOPE( g_Level->GetSGSC() );
		sgs_PushPtr( g_Level->GetSGSC(), this );
		eiface.thiscall( g_Level->GetSGSC(), "ED_PropInfo", 1 );
	}
	
	Fields2Data();
}

EdEntNew::~EdEntNew()
{
	ClearFields();
}

EdEntNew& EdEntNew::operator = ( const EdEntNew& o )
{
	m_pos = o.m_pos;
	m_entityType = o.m_entityType;
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		m_fields[ i ].property->TakeValue( o.m_fields[ i ].property );
	}
	Fields2Data();
	return *this;
}

EdEntity* EdEntNew::CloneEntity()
{
	EdEntNew* N = new EdEntNew( m_entityType, false );
	*N = *this;
	return N;
}

void EdEntNew::Serialize( SVHBR& arch )
{
	arch << m_pos;
	String data;
	arch << data;
	m_data = g_Level->GetScriptCtx().Unserialize( data );
	Data2Fields();
}

void EdEntNew::Serialize( SVHBW& arch )
{
	arch << m_pos;
	String data;
	data = g_Level->GetScriptCtx().Serialize( m_data );
	arch << data;
}

void EdEntNew::FLoad( sgsVariable data, int version )
{
	UNUSED( version );
	ASSERT( data.getprop( "entity_type" ) == m_entityType );
	sgsVariable props = data.getprop( "props" );
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		Field& F = m_fields[ i ];
		sgsVariable val = props.getprop( F.key );
		switch( F.property->type )
		{
		case EDGUI_ITEM_PROP_BOOL: ((EDGUIPropBool*)F.property)->SetValue( FLoadVar( val, false ) ); break;
		case EDGUI_ITEM_PROP_INT: ((EDGUIPropInt*)F.property)->SetValue( FLoadVar( val, int32_t(0) ) ); break;
		case EDGUI_ITEM_PROP_FLOAT: ((EDGUIPropFloat*)F.property)->SetValue( FLoadVar( val, 0.0f ) ); break;
		case EDGUI_ITEM_PROP_VEC2: ((EDGUIPropVec2*)F.property)->SetValue( FLoadVar( val, V2(0) ) ); break;
		case EDGUI_ITEM_PROP_VEC3:
			((EDGUIPropVec3*)F.property)->SetValue( FLoadVar( val, V3(0) ) );
			if( F.key.equals( "position" ) )
				m_pos = ((EDGUIPropVec3*)F.property)->m_value;
			break;
		case EDGUI_ITEM_PROP_STRING: ((EDGUIPropString*)F.property)->SetValue( FLoadVar( val, SV("") ) ); break;
		case EDGUI_ITEM_PROP_RSRC: ((EDGUIPropRsrc*)F.property)->SetValue( FLoadVar( val, SV("") ) ); break;
		case EDGUI_ITEM_PROP_ENUM_SB: ((EDGUIPropEnumSB*)F.property)->SetValue( FLoadVar( val, 0 ) ); break;
		}
	}
	Fields2Data();
}

sgsVariable EdEntNew::FSave( int version )
{
	UNUSED( version );
	sgsVariable props = FNewDict();
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		Field& F = m_fields[ i ];
		sgsVariable val;
		switch( F.property->type )
		{
		case EDGUI_ITEM_PROP_BOOL: val = FVar( ((EDGUIPropBool*)F.property)->m_value ); break;
		case EDGUI_ITEM_PROP_INT: val = FVar( ((EDGUIPropInt*)F.property)->m_value ); break;
		case EDGUI_ITEM_PROP_FLOAT: val = FVar( ((EDGUIPropFloat*)F.property)->m_value ); break;
		case EDGUI_ITEM_PROP_VEC2: val = FVar( ((EDGUIPropVec2*)F.property)->m_value ); break;
		case EDGUI_ITEM_PROP_VEC3: val = FVar( ((EDGUIPropVec3*)F.property)->m_value ); break;
		case EDGUI_ITEM_PROP_STRING: val = FVar( ((EDGUIPropString*)F.property)->m_value ); break;
		case EDGUI_ITEM_PROP_RSRC: val = FVar( ((EDGUIPropRsrc*)F.property)->m_value ); break;
		case EDGUI_ITEM_PROP_ENUM_SB: val = FVar( ((EDGUIPropEnumSB*)F.property)->m_value ); break;
		}
		props.setprop( F.key, val );
	}
	
	sgsVariable out = FNewDict();
	out.setprop( "entity_type", m_entityType );
	out.setprop( "props", props );
	return out;
}

void EdEntNew::SetPosition( const Vec3& pos )
{
	EdEntity::SetPosition( pos );
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		Field& F = m_fields[ i ];
		if( F.key.equals( "position" ) && F.property->type == EDGUI_ITEM_PROP_VEC3 )
		{
			((EDGUIPropVec3*)F.property)->SetValue( m_pos );
		}
	}
	Fields2Data();
}

void EdEntNew::Data2Fields()
{
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		Field& F = m_fields[ i ];
		sgsVariable val = m_data.getprop( F.key );
		if( val.type_id() == SGS_VT_NULL )
			continue;
		switch( F.property->type )
		{
		case EDGUI_ITEM_PROP_BOOL: ((EDGUIPropBool*) F.property)->SetValue( val.get<bool>() ); break;
		case EDGUI_ITEM_PROP_INT: ((EDGUIPropInt*) F.property)->SetValue( val.get<int>() ); break;
		case EDGUI_ITEM_PROP_FLOAT: ((EDGUIPropFloat*) F.property)->SetValue( val.get<float>() ); break;
		case EDGUI_ITEM_PROP_VEC2: ((EDGUIPropVec2*) F.property)->SetValue( val.get<Vec2>() ); break;
		case EDGUI_ITEM_PROP_VEC3: ((EDGUIPropVec3*) F.property)->SetValue( val.get<Vec3>() ); break;
		case EDGUI_ITEM_PROP_STRING: ((EDGUIPropString*) F.property)->SetValue( val.get<StringView>() ); break;
		case EDGUI_ITEM_PROP_ENUM_SB: ((EDGUIPropEnumSB*) F.property)->SetValue( val.get<int>() ); break;
		case EDGUI_ITEM_PROP_RSRC: ((EDGUIPropRsrc*) F.property)->SetValue( val.get<StringView>() ); break;
		}
	}
}

void EdEntNew::Fields2Data()
{
	sgsVariable data = FNewDict();
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		Field& F = m_fields[ i ];
		switch( F.property->type )
		{
		case EDGUI_ITEM_PROP_BOOL: data.setprop( F.key, FIntVar(((EDGUIPropBool*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_INT: data.setprop( F.key, FIntVar( ((EDGUIPropInt*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_FLOAT: data.setprop( F.key, FIntVar(((EDGUIPropFloat*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_VEC2: data.setprop( F.key, FIntVar( ((EDGUIPropVec2*) F.property)->m_value ) ); break;
		case EDGUI_ITEM_PROP_VEC3: data.setprop( F.key, FIntVar( ((EDGUIPropVec3*) F.property)->m_value ) ); break;
		case EDGUI_ITEM_PROP_STRING: data.setprop( F.key, FIntVar( ((EDGUIPropString*) F.property)->m_value ) ); break;
		case EDGUI_ITEM_PROP_ENUM_SB: data.setprop( F.key, FIntVar( ((EDGUIPropEnumSB*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_RSRC: data.setprop( F.key, FIntVar( ((EDGUIPropRsrc*) F.property)->m_value ) ); break;
		}
	}
	m_data = data;
}

void EdEntNew::AddField( sgsString key, StringView name, EDGUIProperty* prop )
{
	prop->caption = name;
	Field F = { key, prop };
	m_fields.push_back( F );
	m_group.Add( prop );
}

void EdEntNew::ClearFields()
{
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		delete m_fields[ i ].property;
	}
	m_fields.clear();
}


static int EE_AddFieldBool( SGS_CTX )
{
	SGSFN( "EE_AddFieldBool" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	bool def = sgs_StackSize( C ) > 3 ? sgs_GetVar<bool>()( C, 3 ) : false;
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropBool( def ) );
	return 0;
}
static int EE_AddFieldInt( SGS_CTX )
{
	SGSFN( "EE_AddFieldInt" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	int32_t def = sgs_StackSize( C ) > 3 ? sgs_GetVar<int32_t>()( C, 3 ) : 0;
	int32_t min = sgs_StackSize( C ) > 4 ? sgs_GetVar<int32_t>()( C, 4 ) : 0x80000000;
	int32_t max = sgs_StackSize( C ) > 5 ? sgs_GetVar<int32_t>()( C, 5 ) : 0x7fffffff;
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropInt( def, min, max ) );
	return 0;
}
static int EE_AddFieldFloat( SGS_CTX )
{
	SGSFN( "EE_AddFieldFloat" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	float def = sgs_StackSize( C ) > 3 ? sgs_GetVar<float>()( C, 3 ) : 0;
	int prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	float min = sgs_StackSize( C ) > 5 ? sgs_GetVar<float>()( C, 5 ) : -FLT_MAX;
	float max = sgs_StackSize( C ) > 6 ? sgs_GetVar<float>()( C, 6 ) : FLT_MAX;
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropFloat( def, prec, min, max ) );
	return 0;
}
static int EE_AddFieldVec2( SGS_CTX )
{
	SGSFN( "EE_AddFieldVec2" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	Vec2 def = sgs_StackSize( C ) > 3 ? sgs_GetVar<Vec2>()( C, 3 ) : V2(0);
	int prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	Vec2 min = sgs_StackSize( C ) > 5 ? sgs_GetVar<Vec2>()( C, 5 ) : V2(-FLT_MAX);
	Vec2 max = sgs_StackSize( C ) > 6 ? sgs_GetVar<Vec2>()( C, 6 ) : V2(FLT_MAX);
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropVec2( def, prec, min, max ) );
	return 0;
}
static int EE_AddFieldVec3( SGS_CTX )
{
	SGSFN( "EE_AddFieldVec3" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	Vec3 def = sgs_StackSize( C ) > 3 ? sgs_GetVar<Vec3>()( C, 3 ) : V3(0);
	int prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	Vec3 min = sgs_StackSize( C ) > 5 ? sgs_GetVar<Vec3>()( C, 5 ) : V3(-FLT_MAX);
	Vec3 max = sgs_StackSize( C ) > 6 ? sgs_GetVar<Vec3>()( C, 6 ) : V3(FLT_MAX);
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropVec3( def, prec, min, max ) );
	return 0;
}
static int EE_AddFieldString( SGS_CTX )
{
	SGSFN( "EE_AddFieldString" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropString( sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldEnumSB( SGS_CTX )
{
	SGSFN( "EE_AddFieldEnumSB" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	EDGUIPropEnumSB* P = new EDGUIPropEnumSB;
	ScriptVarIterator it( sgs_GetVar<sgsVariable>()( C, 4 ) );
	while( it.Advance() )
	{
		EDGUIPropEnumSB::Entry E = { it.GetValue().get<StringView>(), it.GetKey().get<int32_t>() };
		P->m_enum.push_back( E );
	}
	P->SetValue( sgs_GetVar<int32_t>()( C, 3 ) );
	E->AddField( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<StringView>()( C, 2 ), P );
	return 0;
}
static int EE_AddFieldMesh( SGS_CTX )
{
	SGSFN( "EE_AddFieldMesh" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UIMeshPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldTex( SGS_CTX )
{
	SGSFN( "EE_AddFieldTex" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UISurfTexPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldChar( SGS_CTX )
{
	SGSFN( "EE_AddFieldChar" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UICharPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldPartSys( SGS_CTX )
{
	SGSFN( "EE_AddFieldPartSys" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UIPartSysPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldSound( SGS_CTX )
{
	SGSFN( "EE_AddFieldSound" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UISoundPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
#if 0
static int EE_AddFieldScrFn( SGS_CTX )
{
	SGSFN( "EE_AddFieldScrFn" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UIScrFnPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldScrItem( SGS_CTX )
{
	SGSFN( "EE_AddFieldScrItem" );
	SGRX_CAST( EdEntNew*, E, sgs_GetVar<void*>()( C, 0 ) );
	EDGUIPropVec3* posprop = E->IsScrEnt() ?
		&((EdEntScripted*)E)->m_ctlPos :
		((EDGUIPropScrItem*)E)->m_pctlPos;
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropScrItem( posprop, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddButtonSubent( SGS_CTX )
{
	SGSFN( "EE_AddButtonSubent" );
	SGRX_CAST( EdEntNew*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->AddButtonSubent( sgs_GetVar<StringView>()( C, 1 ) );
	return 0;
}

static int EE_SetChar( SGS_CTX )
{
	SGSFN( "EE_SetChar" );
	SGRX_CAST( EdEntNew*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->SetSpecialMesh(
		ED_GetMeshFromChar( sgs_GetVar<StringView>()( C, 1 ) ),
		sgs_GetVar<Mat4>()( C, 2 ) );
	return 0;
}
static int EE_SetMeshInstanceCount( SGS_CTX )
{
	SGSFN( "EE_SetMeshInstanceCount" );
	SGRX_CAST( EdEntNew*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->SetMeshInstanceCount( sgs_GetVar<int>()( C, 1 ) );
	return 0;
}
static int EE_SetMeshInstanceData( SGS_CTX )
{
	SGSFN( "EE_SetMeshInstanceData" );
	SGRX_CAST( EdEntNew*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	int flags = LM_MESHINST_SOLID | LM_MESHINST_CASTLMS;
	if( sgs_StackSize( C ) > 4 )
		flags = sgs_GetVar<int>()( C, 4 );
	float lmdetail = 1;
	if( sgs_StackSize( C ) > 5 )
		lmdetail = sgs_GetVar<float>()( C, 5 );
	E->SetMeshInstanceData(
		sgs_GetVar<int>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		sgs_GetVar<Mat4>()( C, 3 ),
		flags,
		lmdetail );
	return 0;
}
static int EE_GetMeshAABB( SGS_CTX )
{
	SGSFN( "EE_GetMeshAABB" );
	SGRX_CAST( EdEntNew*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	Vec3 aabb[2];
	E->GetMeshAABB( sgs_GetVar<int>()( C, 1 ), aabb );
	sgs_PushVar( C, aabb[0] );
	sgs_PushVar( C, aabb[1] );
	return 2;
}
static int EE_SetScriptedItem( SGS_CTX )
{
	SGSFN( "EE_SetScriptedItem" );
	SGRX_CAST( EdEntNew*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->SetScriptedItem( sgs_GetVar<StringView>()( C, 1 ), sgsVariable( C, 2 ) );
	return 0;
}

static int EE_SetChangeFunc( SGS_CTX )
{
	SGSFN( "EE_SetChangeFunc" );
	SGRX_CAST( EdEntNew*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->onChange = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}
static int EE_SetDebugDrawFunc( SGS_CTX )
{
	SGSFN( "EE_SetDebugDrawFunc" );
	SGRX_CAST( EdEntNew*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->onDebugDraw = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}
static int EE_SetGatherFunc( SGS_CTX )
{
	SGSFN( "EE_SetGatherFunc" );
	SGRX_CAST( EdEntNew*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->onGather = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}
#endif

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
#if 0
	{ "EE_AddFieldScrFn", EE_AddFieldScrFn },
	{ "EE_AddFieldScrItem", EE_AddFieldScrItem },
	{ "EE_AddButtonSubent", EE_AddButtonSubent },
	{ "EE_SetChar", EE_SetChar },
	{ "EE_SetMeshInstanceCount", EE_SetMeshInstanceCount },
	{ "EE_SetMeshInstanceData", EE_SetMeshInstanceData },
	{ "EE_GetMeshAABB", EE_GetMeshAABB },
	{ "EE_SetScriptedItem", EE_SetScriptedItem },
	{ "EE_SetChangeFunc", EE_SetChangeFunc },
	{ "EE_SetDebugDrawFunc", EE_SetDebugDrawFunc },
	{ "EE_SetGatherFunc", EE_SetGatherFunc },
#endif
	{ "EE_GenMatrix_SRaP", EE_GenMatrix_SRaP },
	SGS_RC_END(),
};


EDGUIEntList::EDGUIEntList() :
	EDGUIGroup( true, "Pick an entity type" )
{
	Array< StringView > ents;
	g_Level->EnumEntities( ents );
	
	ASSERT( ents.size() > 0 && "No entities were found!" );
	
	m_button_count = ents.size();
	m_buttons = new EDGUIEntButton[ m_button_count ];
	
	for( int i = 0; i < m_button_count; ++i )
	{
		m_buttons[ i ].caption = ents[ i ];
		m_buttons[ i ].protoEnt = new EdEntNew( FVar( ents[ i ] ).get_string(), true );
		
		Add( &m_buttons[ i ] );
	}
	
	m_buttons[ 0 ].SetHighlight( true );
	
	g_EdEntList = this;
}

EDGUIEntList::~EDGUIEntList()
{
	g_EdEntList = NULL;
	delete [] m_buttons;
}

int EDGUIEntList::OnEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_BTNCLICK )
	{
		for( int i = 0; i < m_button_count; ++i )
		{
			bool found = e->target == &m_buttons[ i ];
			m_buttons[ i ].SetHighlight( found );
			if( found )
			{
				EDGUIEvent se = { EDGUI_EVENT_SETENTITY, &m_buttons[ i ] };
				BubblingEvent( &se );
			}
		}
	}
	return EDGUIGroup::OnEvent( e );
}



