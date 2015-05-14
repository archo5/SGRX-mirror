

#include "mapedit.hpp"


void EdEntity::LoadIcon()
{
	char bfr[ 256 ];
	snprintf( bfr, 256, "editor/icons/%s.png", tyname );
	m_iconTex = GR_GetTexture( bfr );
	if( !m_iconTex )
		m_iconTex = GR_GetTexture( "editor/icons/default.png" );
}


EdEntMesh::EdEntMesh( bool isproto ) :
	EdEntity( isproto ),
	m_ctlAngles( V3(0), 2, V3(0), V3(360) ),
	m_ctlScaleUni( 1, 2, 0.01f, 100.0f ),
	m_ctlScaleSep( V3(1), 2, V3(0.01f), V3(100.0f) ),
	m_ctlMesh( g_UIMeshPicker, "test_table" )
{
	tyname = "mesh";
	LoadIcon();
	
	m_group.caption = "MESH properties";
	m_ctlAngles.caption = "Rotation";
	m_ctlScaleUni.caption = "Scale (uniform)";
	m_ctlScaleSep.caption = "Scale (separate)";
	m_ctlMesh.caption = "Mesh";
	
	m_group.Add( &m_ctlPos );
	m_group.Add( &m_ctlAngles );
	m_group.Add( &m_ctlScaleUni );
	m_group.Add( &m_ctlScaleSep );
	m_group.Add( &m_ctlMesh );
	Add( &m_group );
}

EdEntMesh& EdEntMesh::operator = ( const EdEntMesh& o )
{
	m_ctlPos.SetValue( o.Pos() );
	m_ctlAngles.SetValue( o.RotAngles() );
	m_ctlScaleUni.SetValue( o.ScaleUni() );
	m_ctlScaleSep.SetValue( o.ScaleSep() );
	m_ctlMesh.SetValue( o.Mesh() );
	return *this;
}

EdEntity* EdEntMesh::CloneEntity()
{
	EdEntMesh* N = new EdEntMesh( false );
	*N = *this;
	return N;
}

void EdEntMesh::UpdateCache( LevelCache& LC )
{
	char bfr[ 256 ];
	snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", TMIN( (int) Mesh().size(), 200 ), Mesh().data() );
	LC.AddMeshInst( bfr, Matrix() );
}

int EdEntMesh::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_ctlMesh )
		{
			cached_mesh = NULL;
		}
		if( !m_isproto )
			RegenerateMesh();
		break;
	}
	return EdEntity::OnEvent( e );
}

void EdEntMesh::DebugDraw()
{
	if( cached_mesh )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		br.Col( 0.1f, 0.3f, 0.8f, 0.5f );
		br.AABB( cached_mesh->m_boundsMin, cached_mesh->m_boundsMax, Matrix() );
	}
}

void EdEntMesh::RegenerateMesh()
{
	if( !cached_mesh )
	{
		char bfr[ 128 ];
		snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", (int) Mesh().size(), Mesh().data() );
		cached_mesh = GR_GetMesh( bfr );
	}
	if( !cached_meshinst )
	{
		cached_meshinst = g_EdScene->CreateMeshInstance();
		lmm_prepmeshinst( cached_meshinst );
	}
	cached_meshinst->mesh = cached_mesh;
	cached_meshinst->matrix = Matrix();
}


EdEntLight::EdEntLight( bool isproto ) :
	EdEntity( isproto ),
	m_ctlRange( 10, 2, 0, 1000.0f ),
	m_ctlPower( 2, 2, 0.01f, 100.0f ),
	m_ctlColorHSV( V3(0,0,1), 2, V3(0,0,0), V3(1,1,100) ),
	m_ctlLightRadius( 1, 2, 0, 100.0f ),
	m_ctlShSampleCnt( 5, 0, 256 ),
	m_ctlFlareSize( 1, 2, 0, 10 ),
	m_ctlFlareOffset( V3(0), 2, V3(-4), V3(4) ),
	m_ctlIsSpotlight( false ),
	m_ctlSpotRotation( V3(0), 2, V3(0), V3(360) ),
	m_ctlSpotInnerAngle( 0, 2, 0, 180 ),
	m_ctlSpotOuterAngle( 45, 2, 0, 180 ),
	m_ctlSpotCurve( 1, 2, 0.01f, 100.0f )
{
	tyname = "light";
	LoadIcon();
	
	m_group.caption = "LIGHT properties";
	m_ctlRange.caption = "Range";
	m_ctlPower.caption = "Power";
	m_ctlColorHSV.caption = "Color (HSV)";
	m_ctlLightRadius.caption = "Light radius";
	m_ctlShSampleCnt.caption = "# shadow samples";
	m_ctlFlareSize.caption = "Flare size";
	m_ctlFlareOffset.caption = "Flare offset";
	m_ctlIsSpotlight.caption = "Is spotlight?";
	m_ctlSpotRotation.caption = "Spotlight rotation";
	m_ctlSpotInnerAngle.caption = "Spotlight inner angle";
	m_ctlSpotOuterAngle.caption = "Spotlight outer angle";
	m_ctlSpotCurve.caption = "Spotlight curve";
	
	m_group.Add( &m_ctlPos );
	m_group.Add( &m_ctlRange );
	m_group.Add( &m_ctlPower );
	m_group.Add( &m_ctlColorHSV );
	m_group.Add( &m_ctlLightRadius );
	m_group.Add( &m_ctlShSampleCnt );
	m_group.Add( &m_ctlFlareSize );
	m_group.Add( &m_ctlFlareOffset );
	m_group.Add( &m_ctlIsSpotlight );
	m_group.Add( &m_ctlSpotRotation );
	m_group.Add( &m_ctlSpotInnerAngle );
	m_group.Add( &m_ctlSpotOuterAngle );
	m_group.Add( &m_ctlSpotCurve );
	Add( &m_group );
}

EdEntLight& EdEntLight::operator = ( const EdEntLight& o )
{
	m_ctlPos.SetValue( o.Pos() );
	m_ctlRange.SetValue( o.Range() );
	m_ctlPower.SetValue( o.Power() );
	m_ctlColorHSV.SetValue( o.ColorHSV() );
	m_ctlLightRadius.SetValue( o.LightRadius() );
	m_ctlShSampleCnt.SetValue( o.ShadowSampleCount() );
	m_ctlFlareSize.SetValue( o.FlareSize() );
	m_ctlFlareOffset.SetValue( o.FlareOffset() );
	m_ctlIsSpotlight.SetValue( o.IsSpotlight() );
	m_ctlSpotRotation.SetValue( o.SpotRotation() );
	m_ctlSpotInnerAngle.SetValue( o.SpotInnerAngle() );
	m_ctlSpotOuterAngle.SetValue( o.SpotOuterAngle() );
	m_ctlSpotCurve.SetValue( o.SpotCurve() );
	return *this;
}

EdEntity* EdEntLight::CloneEntity()
{
	EdEntLight* N = new EdEntLight( false );
	*N = *this;
	return N;
}

void EdEntLight::DebugDraw()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	if( IsSpotlight() )
	{
		br.Col( 0.9f, 0.8f, 0.1f, 0.5f );
		br.ConeOutline( Pos(), SpotDir(), SpotUp(), Range(), SpotOuterAngle(), 32 );
		br.Col( 0.9f, 0.5f, 0.1f, 0.5f );
		br.ConeOutline( Pos(), SpotDir(), SpotUp(), pow( 0.5f, Power() ) * Range(), SpotOuterAngle(), 32 );
	}
	else
	{
		br.Col( 0.9f, 0.8f, 0.1f, 0.5f );
		br.SphereOutline( Pos(), Range(), 64 );
		br.Col( 0.9f, 0.5f, 0.1f, 0.5f );
		br.SphereOutline( Pos(), pow( 0.5f, Power() ) * Range(), 32 );
	}
	br.Col( 0.9f, 0.1f, 0.1f, 0.5f );
	br.Tick( Pos() + FlareOffset(), 0.1f );
}

void EdEntLight::UpdateCache( LevelCache& LC )
{
	LC_Light L;
	L.type = IsSpotlight() ? LM_LIGHT_SPOT : LM_LIGHT_POINT;
	L.pos = Pos();
	L.dir = SpotDir();
	L.up = SpotUp();
	L.range = Range();
	L.power = Power();
	L.color = HSV( ColorHSV() );
	L.light_radius = LightRadius();
	L.num_shadow_samples = ShadowSampleCount();
	L.flaresize = FlareSize();
	L.flareoffset = FlareOffset();
	L.innerangle = SpotInnerAngle();
	L.outerangle = SpotOuterAngle();
	L.spotcurve = SpotCurve();
	LC.AddLight( L );
}


EdEntLightSample::EdEntLightSample( bool isproto ) :
	EdEntity( isproto )
{
	tyname = "ltsample";
	LoadIcon();
	
	m_group.caption = "LIGHT SAMPLE properties";
	m_group.Add( &m_ctlPos );
	Add( &m_group );
}

EdEntLightSample& EdEntLightSample::operator = ( const EdEntLightSample& o )
{
	m_ctlPos.SetValue( o.Pos() );
	return *this;
}

EdEntity* EdEntLightSample::CloneEntity()
{
	EdEntLightSample* N = new EdEntLightSample( false );
	*N = *this;
	return N;
}

void EdEntLightSample::UpdateCache( LevelCache& LC )
{
	LC.AddSample( Pos() );
}


EdEntScripted::EdEntScripted( const char* enttype, bool isproto ) :
	EdEntity( isproto ),
	m_levelCache( NULL )
{
	strncpy( m_typename, enttype, 63 );
	m_typename[ 63 ] = 0;
	tyname = m_typename;
	LoadIcon();
	
	char bfr[ 256 ];
	
	sprintf( bfr, "%.240s properties", enttype );
	m_group.caption = bfr;
	m_group.SetOpen( true );
	m_group.Add( &m_ctlPos );
	Add( &m_group );
	
	Field posf = { "position", &m_ctlPos };
	m_fields.push_back( posf );
	
	g_ScriptCtx->Push( (void*) this );
	sprintf( bfr, "ED_ENT_%.240s", enttype );
	g_ScriptCtx->GlobalCall( bfr, 1, 0 );
	
	Fields2Data();
}

EdEntScripted::~EdEntScripted()
{
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		if( m_fields[ i ].property == &m_ctlPos )
			continue;
		delete m_fields[ i ].property;
	}
}

EdEntScripted& EdEntScripted::operator = ( const EdEntScripted& o )
{
	ASSERT( strcmp( tyname, o.tyname ) == 0 );
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		m_fields[ i ].property->TakeValue( o.m_fields[ i ].property );
	}
	Fields2Data();
	return *this;
}

EdEntity* EdEntScripted::CloneEntity()
{
	EdEntScripted* N = new EdEntScripted( tyname, false );
	*N = *this;
	return N;
}

void EdEntScripted::Data2Fields()
{
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		Field& F = m_fields[ i ];
		sgsVariable val = m_data.getprop( StackString<256>( F.key ) );
		switch( F.property->type )
		{
		case EDGUI_ITEM_PROP_BOOL: ((EDGUIPropBool*) F.property)->SetValue( val.get<bool>() ); break;
		case EDGUI_ITEM_PROP_INT: ((EDGUIPropInt*) F.property)->SetValue( val.get<int>() ); break;
		case EDGUI_ITEM_PROP_FLOAT: ((EDGUIPropFloat*) F.property)->SetValue( val.get<float>() ); break;
		case EDGUI_ITEM_PROP_VEC2: ((EDGUIPropVec2*) F.property)->SetValue( val.get<Vec2>() ); break;
		case EDGUI_ITEM_PROP_VEC3: ((EDGUIPropVec3*) F.property)->SetValue( val.get<Vec3>() ); break;
		case EDGUI_ITEM_PROP_STRING: ((EDGUIPropString*) F.property)->SetValue( val.get<String>() ); break;
		case EDGUI_ITEM_PROP_RSRC: ((EDGUIPropRsrc*) F.property)->SetValue( val.get<String>() ); break;
		}
	}
}

void EdEntScripted::Fields2Data()
{
	sgsVariable data = g_ScriptCtx->CreateDict();
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		Field& F = m_fields[ i ];
		switch( F.property->type )
		{
		case EDGUI_ITEM_PROP_BOOL: data.setprop( StackString<256>( F.key ), sgsVariable().set(((EDGUIPropBool*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_INT: data.setprop( StackString<256>( F.key ), sgsVariable().set( (sgs_Int) ((EDGUIPropInt*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_FLOAT: data.setprop( StackString<256>( F.key ), sgsVariable().set(((EDGUIPropFloat*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_VEC2: data.setprop( StackString<256>( F.key ), g_ScriptCtx->CreateVec2( ((EDGUIPropVec2*) F.property)->m_value ) ); break;
		case EDGUI_ITEM_PROP_VEC3: data.setprop( StackString<256>( F.key ), g_ScriptCtx->CreateVec3( ((EDGUIPropVec3*) F.property)->m_value ) ); break;
		case EDGUI_ITEM_PROP_STRING: data.setprop( StackString<256>( F.key ), g_ScriptCtx->CreateStringVar( ((EDGUIPropString*) F.property)->m_value ) ); break;
		case EDGUI_ITEM_PROP_RSRC: data.setprop( StackString<256>( F.key ), g_ScriptCtx->CreateStringVar( ((EDGUIPropRsrc*) F.property)->m_value ) ); break;
		}
	}
	m_data = data;
}

void EdEntScripted::Serialize( SVHTR& arch )
{
	String data;
	arch << data;
	m_data = g_ScriptCtx->Unserialize( data );
	Data2Fields();
}

void EdEntScripted::Serialize( SVHTW& arch )
{
	String data;
	data = g_ScriptCtx->Serialize( m_data );
	arch << data;
	Fields2Data();
}

void EdEntScripted::Serialize( SVHBR& arch )
{
	String data;
	arch << data;
	m_data = g_ScriptCtx->Unserialize( data );
	Data2Fields();
}

void EdEntScripted::Serialize( SVHBW& arch )
{
	String data;
	data = g_ScriptCtx->Serialize( m_data );
	arch << data;
	Fields2Data();
}

void EdEntScripted::UpdateCache( LevelCache& LC )
{
	String data;
	data = g_ScriptCtx->Serialize( m_data );
	LC.AddScriptedEntity( m_typename, data );
	
	if( onGather.not_null() )
	{
		m_levelCache = &LC;
		SGS_CSCOPE( g_ScriptCtx->C );
		g_ScriptCtx->Push( (void*) this );
		g_ScriptCtx->Push( m_data );
		g_ScriptCtx->Call( onGather, 2 );
		m_levelCache = NULL;
	}
}

void EdEntScripted::RegenerateMesh()
{
	Fields2Data();
	
	if( onChange.not_null() )
	{
		SGS_CSCOPE( g_ScriptCtx->C );
		g_ScriptCtx->Push( (void*) this );
		g_ScriptCtx->Push( m_data );
		g_ScriptCtx->Call( onChange, 2 );
	}
}

int EdEntScripted::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( !m_isproto )
			RegenerateMesh();
		break;
	}
	return EdEntity::OnEvent( e );
}

void EdEntScripted::DebugDraw()
{
	if( !onDebugDraw.not_null() )
		return;
	
	SGS_CSCOPE( g_ScriptCtx->C );
	g_ScriptCtx->Push( (void*) this );
	g_ScriptCtx->Push( m_data );
	g_ScriptCtx->Call( onDebugDraw, 2 );
}

void EdEntScripted::AddFieldBool( sgsString key, sgsString name, bool def )
{
	EDGUIPropBool* prop = new EDGUIPropBool( def );
	prop->caption = StringView( name.c_str(), name.size() );
	Field F = { StringView( key.c_str(), key.size() ), prop };
	m_fields.push_back( F );
	m_group.Add( prop );
}

void EdEntScripted::AddFieldInt( sgsString key, sgsString name, int32_t def, int32_t min, int32_t max )
{
	EDGUIPropInt* prop = new EDGUIPropInt( def, min, max );
	prop->caption = StringView( name.c_str(), name.size() );
	Field F = { StringView( key.c_str(), key.size() ), prop };
	m_fields.push_back( F );
	m_group.Add( prop );
}

void EdEntScripted::AddFieldFloat( sgsString key, sgsString name, float def, int prec, float min, float max )
{
	EDGUIPropFloat* prop = new EDGUIPropFloat( def, prec, min, max );
	prop->caption = StringView( name.c_str(), name.size() );
	Field F = { StringView( key.c_str(), key.size() ), prop };
	m_fields.push_back( F );
	m_group.Add( prop );
}

void EdEntScripted::AddFieldVec2( sgsString key, sgsString name, Vec2 def, int prec, Vec2 min, Vec2 max )
{
	EDGUIPropVec2* prop = new EDGUIPropVec2( def, prec, min, max );
	prop->caption = StringView( name.c_str(), name.size() );
	Field F = { StringView( key.c_str(), key.size() ), prop };
	m_fields.push_back( F );
	m_group.Add( prop );
}

void EdEntScripted::AddFieldVec3( sgsString key, sgsString name, Vec3 def, int prec, Vec3 min, Vec3 max )
{
	EDGUIPropVec3* prop = new EDGUIPropVec3( def, prec, min, max );
	prop->caption = StringView( name.c_str(), name.size() );
	Field F = { StringView( key.c_str(), key.size() ), prop };
	m_fields.push_back( F );
	m_group.Add( prop );
}

void EdEntScripted::AddFieldString( sgsString key, sgsString name, sgsString def )
{
	EDGUIPropString* prop = new EDGUIPropString( StringView( def.c_str(), def.size() ) );
	prop->caption = StringView( name.c_str(), name.size() );
	Field F = { StringView( key.c_str(), key.size() ), prop };
	m_fields.push_back( F );
	m_group.Add( prop );
}

void EdEntScripted::AddFieldRsrc( sgsString key, sgsString name, EDGUIRsrcPicker* rsrcPicker, sgsString def )
{
	EDGUIPropRsrc* prop = new EDGUIPropRsrc( rsrcPicker, StringView( def.c_str(), def.size() ) );
	prop->caption = StringView( name.c_str(), name.size() );
	Field F = { StringView( key.c_str(), key.size() ), prop };
	m_fields.push_back( F );
	m_group.Add( prop );
}

void EdEntScripted::SetMesh( sgsString name )
{
	cached_mesh = GR_GetMesh( StringView( name.c_str(), name.size() ) );
	for( size_t i = 0; i < cached_meshinsts.size(); ++i )
		cached_meshinsts[ i ]->mesh = cached_mesh;
}

void EdEntScripted::SetMeshInstanceCount( int count )
{
	int i = cached_meshinsts.size();
	cached_meshinsts.resize( count );
	while( count > i )
	{
		MeshInstHandle cmi = g_EdScene->CreateMeshInstance();
		lmm_prepmeshinst( cmi );
		cmi->mesh = cached_mesh;
		cmi->matrix = Mat4::Identity;
		cached_meshinsts[ i++ ] = cmi;
	}
}

void EdEntScripted::SetMeshInstanceMatrix( int which, const Mat4& mtx )
{
	if( which < 0 || which >= (int) cached_meshinsts.size() )
		return;
	cached_meshinsts[ which ]->matrix = mtx;
}

void EdEntScripted::GetMeshAABB( Vec3 out[2] )
{
	if( cached_mesh )
	{
		out[0] = cached_mesh->m_boundsMin;
		out[1] = cached_mesh->m_boundsMax;
	}
	else
	{
		out[0] = V3(-1);
		out[1] = V3(1);
	}
}

static int EE_AddFieldBool( SGS_CTX )
{
	SGSFN( "EE_AddFieldBool" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	bool def = sgs_StackSize( C ) > 3 ? sgs_GetVar<bool>()( C, 3 ) : false;
	E->AddFieldBool( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def );
	return 0;
}
static int EE_AddFieldInt( SGS_CTX )
{
	SGSFN( "EE_AddFieldInt" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	int32_t def = sgs_StackSize( C ) > 3 ? sgs_GetVar<int32_t>()( C, 3 ) : 0;
	int32_t min = sgs_StackSize( C ) > 4 ? sgs_GetVar<int32_t>()( C, 4 ) : 0x80000000;
	int32_t max = sgs_StackSize( C ) > 5 ? sgs_GetVar<int32_t>()( C, 5 ) : 0x7fffffff;
	E->AddFieldInt( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def, min, max );
	return 0;
}
static int EE_AddFieldFloat( SGS_CTX )
{
	SGSFN( "EE_AddFieldFloat" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	float def = sgs_StackSize( C ) > 3 ? sgs_GetVar<float>()( C, 3 ) : 0;
	int prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	float min = sgs_StackSize( C ) > 5 ? sgs_GetVar<float>()( C, 5 ) : -FLT_MAX;
	float max = sgs_StackSize( C ) > 6 ? sgs_GetVar<float>()( C, 6 ) : FLT_MAX;
	E->AddFieldFloat( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def, prec, min, max );
	return 0;
}
static int EE_AddFieldVec2( SGS_CTX )
{
	SGSFN( "EE_AddFieldVec2" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	Vec2 def = sgs_StackSize( C ) > 3 ? sgs_GetVar<Vec2>()( C, 3 ) : V2(0);
	int prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	Vec2 min = sgs_StackSize( C ) > 5 ? sgs_GetVar<Vec2>()( C, 5 ) : V2(-FLT_MAX);
	Vec2 max = sgs_StackSize( C ) > 6 ? sgs_GetVar<Vec2>()( C, 6 ) : V2(FLT_MAX);
	E->AddFieldVec2( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def, prec, min, max );
	return 0;
}
static int EE_AddFieldVec3( SGS_CTX )
{
	SGSFN( "EE_AddFieldVec3" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	Vec3 def = sgs_StackSize( C ) > 3 ? sgs_GetVar<Vec3>()( C, 3 ) : V3(0);
	int prec = sgs_StackSize( C ) > 4 ? sgs_GetVar<int>()( C, 4 ) : 2;
	Vec3 min = sgs_StackSize( C ) > 5 ? sgs_GetVar<Vec3>()( C, 5 ) : V3(-FLT_MAX);
	Vec3 max = sgs_StackSize( C ) > 6 ? sgs_GetVar<Vec3>()( C, 6 ) : V3(FLT_MAX);
	E->AddFieldVec3( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), def, prec, min, max );
	return 0;
}
static int EE_AddFieldString( SGS_CTX )
{
	SGSFN( "EE_AddFieldString" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldString( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldMesh( SGS_CTX )
{
	SGSFN( "EE_AddFieldMesh" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UIMeshPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldTex( SGS_CTX )
{
	SGSFN( "EE_AddFieldTex" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UISurfTexPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldPartSys( SGS_CTX )
{
	SGSFN( "EE_AddFieldPartSys" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UIPartSysPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldSound( SGS_CTX )
{
	SGSFN( "EE_AddFieldSound" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UISoundPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}
static int EE_AddFieldScrFn( SGS_CTX )
{
	SGSFN( "EE_AddFieldScrFn" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->AddFieldRsrc( sgs_GetVar<sgsString>()( C, 1 ), sgs_GetVar<sgsString>()( C, 2 ), g_UIScrFnPicker, sgs_GetVar<sgsString>()( C, 3 ) );
	return 0;
}

static int EE_SetMesh( SGS_CTX )
{
	SGSFN( "EE_SetMesh" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->SetMesh( sgs_GetVar<sgsString>()( C, 1 ) );
	return 0;
}
static int EE_SetMeshInstanceCount( SGS_CTX )
{
	SGSFN( "EE_SetMeshInstanceCount" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->SetMeshInstanceCount( sgs_GetVar<int>()( C, 1 ) );
	return 0;
}
static int EE_SetMeshInstanceMatrix( SGS_CTX )
{
	SGSFN( "EE_SetMeshInstanceMatrix" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->SetMeshInstanceMatrix( sgs_GetVar<int>()( C, 1 ), sgs_GetVar<Mat4>()( C, 2 ) );
	return 0;
}
static int EE_GetMeshAABB( SGS_CTX )
{
	SGSFN( "EE_GetMeshAABB" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	Vec3 aabb[2];
	E->GetMeshAABB( aabb );
	sgs_PushVar( C, aabb[0] );
	sgs_PushVar( C, aabb[1] );
	return 2;
}

static int EE_SetChangeFunc( SGS_CTX )
{
	SGSFN( "EE_SetChangeFunc" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->onChange = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}
static int EE_SetDebugDrawFunc( SGS_CTX )
{
	SGSFN( "EE_SetDebugDrawFunc" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->onDebugDraw = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}
static int EE_SetGatherFunc( SGS_CTX )
{
	SGSFN( "EE_SetGatherFunc" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	E->onGather = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}

static int EE_Gather_Mesh( SGS_CTX )
{
	SGSFN( "EE_Gather_Mesh" );
	EdEntScripted* E = (EdEntScripted*) sgs_GetVar<void*>()( C, 0 );
	float lmquality = sgs_StackSize( C ) > 3 ? sgs_GetVar<float>()( C, 3 ) : 1.0f;
	bool solid = sgs_StackSize( C ) > 4 ? sgs_GetVar<bool>()( C, 4 ) : true;
	bool dynlit = sgs_StackSize( C ) > 5 ? sgs_GetVar<bool>()( C, 5 ) : false;
	bool castlms = sgs_StackSize( C ) > 6 ? sgs_GetVar<bool>()( C, 6 ) : true;
	if( E->m_levelCache )
		E->m_levelCache->AddMeshInst( sgs_GetVar<String>()( C, 1 ), sgs_GetVar<Mat4>()( C, 2 ), lmquality, solid, dynlit, castlms );
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

sgs_RegFuncConst g_ent_scripted_rfc[] =
{
	{ "EE_AddFieldBool", EE_AddFieldBool },
	{ "EE_AddFieldInt", EE_AddFieldInt },
	{ "EE_AddFieldFloat", EE_AddFieldFloat },
	{ "EE_AddFieldVec2", EE_AddFieldVec2 },
	{ "EE_AddFieldVec3", EE_AddFieldVec3 },
	{ "EE_AddFieldString", EE_AddFieldString },
	{ "EE_AddFieldMesh", EE_AddFieldMesh },
	{ "EE_AddFieldTex", EE_AddFieldTex },
	{ "EE_AddFieldPartSys", EE_AddFieldPartSys },
	{ "EE_AddFieldSound", EE_AddFieldSound },
	{ "EE_AddFieldScrFn", EE_AddFieldScrFn },
	{ "EE_SetMesh", EE_SetMesh },
	{ "EE_SetMeshInstanceCount", EE_SetMeshInstanceCount },
	{ "EE_SetMeshInstanceMatrix", EE_SetMeshInstanceMatrix },
	{ "EE_GetMeshAABB", EE_GetMeshAABB },
	{ "EE_SetChangeFunc", EE_SetChangeFunc },
	{ "EE_SetDebugDrawFunc", EE_SetDebugDrawFunc },
	{ "EE_SetGatherFunc", EE_SetGatherFunc },
	{ "EE_Gather_Mesh", EE_Gather_Mesh },
	{ "EE_GenMatrix_SRaP", EE_GenMatrix_SRaP },
	SGS_RC_END(),
};


EDGUIEntList::EDGUIEntList() :
	EDGUIGroup( true, "Pick an entity type" )
{
	Array< Decl > ents;
	Decl coreents[] =
	{
		{ "Mesh", new EdEntMesh },
		{ "Light", new EdEntLight },
		{ "Light sample", new EdEntLightSample },
	};
	ents.append( coreents, sizeof(coreents)/sizeof(coreents[0]) );
	
	ScriptVarIterator globals = g_ScriptCtx->GlobalIterator();
	while( globals.Advance() )
	{
		sgsString str = globals.GetKey().get_string();
		StringView key( str.c_str(), str.size() );
		if( key.part( 0, 7 ) == "ED_ENT_" )
		{
			key = key.part( 7 );
			Decl decl = { key.data(), new EdEntScripted( key.data() ) };
			ents.push_back( decl );
		}
	}
	
	m_button_count = ents.size();
	m_buttons = new EDGUIEntButton[ m_button_count ];
	
	for( int i = 0; i < m_button_count; ++i )
	{
		m_buttons[ i ].caption = ents[ i ].name;
		m_buttons[ i ].m_ent_handle = ents[ i ].ent;
		
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

EdEntity* ENT_FindProtoByName( const char* name )
{
	for( int i = 0; i < g_EdEntList->m_button_count; ++i )
	{
		EdEntity* e = g_EdEntList->m_buttons[ i ].m_ent_handle;
		if( !strcmp( e->tyname, name ) )
			return e;
	}
	return NULL;
}



