

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

void EdEntity::LoadIcon()
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "editor/icons/%s.png", tyname );
	m_iconTex = GR_GetTexture( bfr );
	if( !m_iconTex )
		m_iconTex = GR_GetTexture( "editor/icons/default.png" );
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


#if 0
EdEntMesh::EdEntMesh( bool isproto ) :
	EdEntity( isproto ),
	m_ctlAngles( V3(0), 2, V3(0), V3(360) ),
	m_ctlScaleUni( 1, 2, 0.01f, 100.0f ),
	m_ctlScaleSep( V3(1), 2, V3(0.01f), V3(100.0f) ),
	m_ctlMesh( g_UIMeshPicker, "test_table" ),
	m_meshID( 0 )
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

int EdEntMesh::OnEvent( EDGUIEvent* e )
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

void EdEntMesh::DebugDraw()
{
	EdEntity::DebugDraw();
}

void EdEntMesh::RegenerateMesh()
{
	EdLGCMeshInfo M;
	M.xform = Matrix();
	char bfr[ 128 ];
	sgrx_snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", (int) Mesh().size(), Mesh().data() );
	M.path = bfr;
	if( m_meshID )
		g_EdLGCont->UpdateMesh( m_meshID, LGC_CHANGE_ALL, &M );
	else
		m_meshID = g_EdLGCont->CreateMesh( &M );
}


EdEntLight::EdEntLight( bool isproto ) :
	EdEntity( isproto ),
	m_ctlRange( 10, 2, 0, 1000.0f ),
	m_ctlPower( 2, 2, 0.01f, 100.0f ),
	m_ctlColorHSV( V3(0,0,1), 2, V3(0,0,0), V3(1,1,100) ),
	m_ctlLightRadius( 1, 2, 0, 100.0f ),
	m_ctlDynamic( false ),
	m_ctlShSampleCnt( 5, 0, 256 ),
	m_ctlFlareSize( 1, 2, 0, 10 ),
	m_ctlFlareOffset( V3(0), 2, V3(-4), V3(4) ),
	m_ctlIsSpotlight( false ),
	m_ctlSpotRotation( V3(0), 2, V3(0), V3(360) ),
	m_ctlSpotInnerAngle( 0, 2, 0, 180 ),
	m_ctlSpotOuterAngle( 45, 2, 0, 180 ),
	m_ctlSpotCurve( 1, 2, 0.01f, 100.0f ),
	m_lightID( 0 )
{
	tyname = "light";
	LoadIcon();
	
	m_group.caption = "LIGHT properties";
	m_ctlRange.caption = "Range";
	m_ctlPower.caption = "Power";
	m_ctlColorHSV.caption = "Color (HSV)";
	m_ctlLightRadius.caption = "Light radius";
	m_ctlDynamic.caption = "Is dynamic?";
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
	m_group.Add( &m_ctlDynamic );
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
	m_ctlDynamic.SetValue( o.IsDynamic() );
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
	EdEntity::DebugDraw();
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

int EdEntLight::OnEvent( EDGUIEvent* e )
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

void EdEntLight::RegenerateMesh()
{
	EdLGCLightInfo L;
	if( IsDynamic() )
		L.type = IsSpotlight() ? LM_LIGHT_DYN_SPOT : LM_LIGHT_DYN_POINT;
	else
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
	
	if( m_lightID )
		g_EdLGCont->UpdateLight( m_lightID, LGC_CHANGE_ALL, &L );
	else
		m_lightID = g_EdLGCont->CreateLight( &L );
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
#endif


SGSPropInterface::SGSPropInterface()
{
}

void SGSPropInterface::Data2Fields()
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
		case EDGUI_ITEM_PROP_RSRC: ((EDGUIPropRsrc*) F.property)->SetValue( val.get<StringView>() ); break;
	//	case EDGUI_ITEM_PROP_SCRITEM: ((EDGUIPropScrItem*) F.property)->SetProps( val ); break;
		}
	}
}

void SGSPropInterface::Fields2Data()
{
	sgsVariable data = FNewDict();
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		Field& F = m_fields[ i ];
		switch( F.property->type )
		{
		case EDGUI_ITEM_PROP_BOOL: data.setprop( F.key, FVar(((EDGUIPropBool*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_INT: data.setprop( F.key, FVar( ((EDGUIPropInt*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_FLOAT: data.setprop( F.key, FVar(((EDGUIPropFloat*) F.property)->m_value) ); break;
		case EDGUI_ITEM_PROP_VEC2: data.setprop( F.key, FVar( ((EDGUIPropVec2*) F.property)->m_value ) ); break;
		case EDGUI_ITEM_PROP_VEC3: data.setprop( F.key, FVar( ((EDGUIPropVec3*) F.property)->m_value ) ); break;
		case EDGUI_ITEM_PROP_STRING: data.setprop( F.key, FVar( ((EDGUIPropString*) F.property)->m_value ) ); break;
		case EDGUI_ITEM_PROP_RSRC: data.setprop( F.key, FVar( ((EDGUIPropRsrc*) F.property)->m_value ) ); break;
	//	case EDGUI_ITEM_PROP_SCRITEM: data.setprop( F.key, ((EDGUIPropScrItem*) F.property)->GetProps() ); break;
		}
	}
	m_data = data;
}

void SGSPropInterface::AddField( sgsString key, StringView name, EDGUIProperty* prop )
{
#if 0
	if( prop->type == EDGUI_ITEM_PROP_SCRITEM )
	{
		SGRX_CAST( EDGUIPropScrItem*, psi, prop );
		psi->m_group.caption = name;
		psi->m_group.SetOpen( true );
	}
	else
#endif
	{
		prop->caption = name;
	}
	Field F = { key, prop };
	m_fields.push_back( F );
	GetGroup().Add( prop );
}


#if 0
EDGUIPropScrItem::EDGUIPropScrItem( EDGUIPropVec3* posprop, const StringView& def ) :
	m_group( true, "Scripted item" ),
	m_ctlScrItem( g_UIScrItemPicker, def ),
	m_pctlPos( posprop )
{
	type = EDGUI_ITEM_PROP_SCRITEM;
	tyname = "scritem";
	
	m_ctlScrItem.caption = "Scr.item name";
	
	Add( &m_group );
	OnTypeChange();
}

EDGUIPropScrItem::~EDGUIPropScrItem()
{
	ClearFields();
}

void EDGUIPropScrItem::ClearFields()
{
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		if( m_fields[ i ].property == &m_ctlScrItem ||
			m_fields[ i ].property == m_pctlPos )
			continue;
		delete m_fields[ i ].property;
	}
	m_fields.clear();
}

void EDGUIPropScrItem::OnTypeChange()
{
	ClearFields();
	m_group.Clear();
	
	m_group.Add( &m_ctlScrItem );
	
	Field typef = { sgsString( g_ScriptCtx->C, "__type" ), &m_ctlScrItem };
	m_fields.push_back( typef );
	
	// add fields
	{
		SGS_CSCOPE( g_ScriptCtx->C );
		g_ScriptCtx->Push( (void*) GetPropInterface() );
		char bfr[ 256 ];
		sgrx_snprintf( bfr, 256, "SCRITEM_ARGS_%s", StackString<240>(m_ctlScrItem.m_value).str );
		g_ScriptCtx->GlobalCall( bfr, 1, 0 );
	}
	
	Data2Fields();
	Fields2Data();
	m_data.setprop( g_ScriptCtx->CreateString( "position" ),
		g_ScriptCtx->CreateVec3( m_pctlPos->m_value ) );
}

int EDGUIPropScrItem::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		Fields2Data();
		if( e->target == &m_ctlScrItem )
			OnTypeChange();
		break;
		
	case EDGUI_EVENT_LAYOUT:
		{
			x0 = e->layout.x0;
			y0 = e->layout.y0;
			x1 = e->layout.x1;
			y1 = y0;
			int at = y1;
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				SetSubitemLayout( m_subitems[ i ], x0, at, x1, at );
				at = m_subitems[ i ]->y1;
			}
			y1 = at;
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT, this };
			if( m_parent )
				m_parent->OnEvent( &se );
		}
		return 1;
		
	}
	return EDGUIProperty::OnEvent( e );
}

bool EDGUIPropScrItem::TakeValue( EDGUIProperty* src )
{
	if( src->type != type )
		return false;
	SGRX_CAST( EDGUIPropScrItem*, psi, src );
	m_ctlScrItem.TakeValue( &psi->m_ctlScrItem );
	SetProps( psi->m_data );
	return true;
}

void EDGUIPropScrItem::SetProps( sgsVariable var )
{
	m_data = var;
	OnTypeChange();
}

sgsVariable EDGUIPropScrItem::GetProps()
{
	m_data.setprop( g_ScriptCtx->CreateString( "position" ),
		g_ScriptCtx->CreateVec3( m_pctlPos->m_value ) );
	return m_data;
}


EdEntScripted::EdEntScripted( const char* enttype, bool isproto ) :
	EdEntity( isproto ),
	m_subEntAddBtn( NULL ),
	m_levelCache( NULL ),
	cached_scritem( NULL )
{
	sgrx_snprintf( m_typename, 64, "%s", enttype );
	tyname = m_typename;
	LoadIcon();
	
	char bfr[ 256 ];
	
	sgrx_snprintf( bfr, 256, "%s properties", StackString<240>(enttype).str );
	m_group.caption = bfr;
	m_group.SetOpen( true );
	m_group.Add( &m_ctlPos );
	Add( &m_group );
	
	Field posf = { sgsString( g_ScriptCtx->C, "position" ), &m_ctlPos };
	m_fields.push_back( posf );
	
	{
		SGS_CSCOPE( g_ScriptCtx->C );
		g_ScriptCtx->Push( (void*) GetPropInterface() );
		sgrx_snprintf( bfr, 256, "ED_ENT_%s", StackString<240>(enttype).str );
		g_ScriptCtx->GlobalCall( bfr, 1, 0 );
	}
	
	Fields2Data();
}

EdEntScripted::~EdEntScripted()
{
	ClearFields();
	for( size_t i = 0; i < m_meshIDs.size(); ++i )
	{
		if( m_meshIDs[ i ] )
			g_EdLGCont->DeleteMesh( m_meshIDs[ i ] );
	}
	if( cached_scritem )
		cached_scritem->Release();
	if( m_subEntAddBtn )
		delete m_subEntAddBtn;
}

void EdEntScripted::ClearFields()
{
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		if( m_fields[ i ].property == &m_ctlPos )
			continue;
		delete m_fields[ i ].property;
	}
	m_fields.clear();
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

template< class T > void EdEntScripted_Unserialize( EdEntScripted* ES, T& arch )
{
	arch << ES->m_ctlPos;
	for( size_t i = 0; i < ES->m_meshIDs.size(); ++i )
	{
		g_EdLGCont->DeleteMesh( ES->m_meshIDs[ i ] );
	}
	arch( ES->m_meshIDs, arch.version >= 5 );
	for( size_t i = 0; i < ES->m_meshIDs.size(); ++i )
	{
		g_EdLGCont->RequestMesh( ES->m_meshIDs[ i ] );
	}
	String data;
	arch << data;
	ES->m_data = g_ScriptCtx->Unserialize( data );
	ES->Data2Fields();
	ES->_OnChange();
	ES->Data2Fields();
}

template< class T > void EdEntScripted_Serialize( EdEntScripted* ES, T& arch )
{
	arch << ES->m_ctlPos;
	arch( ES->m_meshIDs, arch.version >= 5 );
	String data;
	data = g_ScriptCtx->Serialize( ES->m_data );
	arch << data;
	ES->Fields2Data();
}

void EdEntScripted::Serialize( SVHTR& arch )
{
	EdEntScripted_Unserialize( this, arch );
}

void EdEntScripted::Serialize( SVHTW& arch )
{
	EdEntScripted_Serialize( this, arch );
}

void EdEntScripted::Serialize( SVHBR& arch )
{
	EdEntScripted_Unserialize( this, arch );
}

void EdEntScripted::Serialize( SVHBW& arch )
{
	EdEntScripted_Serialize( this, arch );
}

void EdEntScripted::AddSelfToSEA( Array< LC_ScriptedEntity >& sea )
{
	String data = g_ScriptCtx->Serialize( m_data );
	sea.push_back( LC_ScriptedEntity() );
	LC_ScriptedEntity& me = sea.last();
	me.type = m_typename;
	me.serialized_params = data;
	for( size_t i = 0; i < m_subEnts.size(); ++i )
	{
		if( m_subEnts[ i ]->IsScriptedEnt() == false )
			continue;
		SGRX_CAST( EdEntScripted*, scrent, m_subEnts[ i ].item );
		scrent->AddSelfToSEA( me.subentities );
	}
}

void EdEntScripted::UpdateCache( LevelCache& LC )
{
	AddSelfToSEA( LC.m_scriptents );
	
	if( onGather.not_null() )
	{
		m_levelCache = &LC;
		SGS_CSCOPE( g_ScriptCtx->C );
		g_ScriptCtx->Push( (void*) GetPropInterface() );
		g_ScriptCtx->Push( m_data );
		g_ScriptCtx->Call( onGather, 2 );
		m_levelCache = NULL;
	}
}

void EdEntScripted::_OnChange()
{
	if( onChange.not_null() )
	{
		SGS_CSCOPE( g_ScriptCtx->C );
		g_ScriptCtx->Push( (void*) GetPropInterface() );
		g_ScriptCtx->Push( m_data );
		g_ScriptCtx->Call( onChange, 2 );
	}
}

void EdEntScripted::RegenerateMesh()
{
	Fields2Data();
	_OnChange();
}

int EdEntScripted::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( m_isproto == false )
			RegenerateMesh();
		break;
	case EDGUI_EVENT_BTNCLICK:
		if( m_isproto == false && e->target == m_subEntAddBtn )
		{
			EdEntity* E = ENT_FindProtoByName( StackString<128>( m_subEntProto ) );
			if( E == NULL )
				break;
			EdEntity* copy = E->CloneEntity();
			copy->m_ctlPos.SetValue( Pos() );
			g_EdWorld->AddObject( copy );
			copy->m_ownerEnt = this;
			m_subEnts.push_back( copy );
			
			// grab it
			copy->selected = true;
			selected = false;
			g_UIFrame->SetEditTransform( &g_UIFrame->m_emEditObjs.m_transform );
			copy->selected = false;
			selected = true;
		}
		break;
	}
	return EdEntity::OnEvent( e );
}

void EdEntScripted::DebugDraw()
{
	EdEntity::DebugDraw();
	if( !onDebugDraw.not_null() )
		return;
	
	SGS_CSCOPE( g_ScriptCtx->C );
	g_ScriptCtx->Push( (void*) GetPropInterface() );
	g_ScriptCtx->Push( m_data );
	g_ScriptCtx->Call( onDebugDraw, 2 );
}

void EdEntScripted::AddButtonSubent( StringView type )
{
	if( m_subEntAddBtn )
	{
		LOG_WARNING << "CAN ONLY ADD ONE SUBENTITY BUTTON";
		return;
	}
	char buf[ 64 ];
	sgrx_snprintf( buf, 64, "ADD SUBENT.: %.*s", TMIN( 50, (int) type.size() ), type.data() );
	EDGUIButton* btn = new EDGUIButton();
	btn->caption = buf;
	m_subEntAddBtn = btn;
	m_group.Add( btn );
	m_subEntProto = type;
}

void EdEntScripted::SetSpecialMesh( StringView path, const Mat4& mtx )
{
	if( path == "" )
	{
		cached_specmeshinst = NULL;
		return;
	}
	else
	{
		cached_specmeshinst = g_EdScene->CreateMeshInstance();
		cached_specmeshinst->SetMesh( path );
		cached_specmeshinst->matrix = mtx;
		lmm_prepmeshinst( cached_specmeshinst );
	}
}

void EdEntScripted::SetMeshInstanceCount( int count )
{
	int i = m_meshIDs.size();
	while( i > count )
	{
		g_EdLGCont->DeleteMesh( m_meshIDs[ --i ] );
	}
	m_meshIDs.resize( count );
	while( i < count )
	{
		m_meshIDs[ i++ ] = g_EdLGCont->CreateMesh();
	}
}

void EdEntScripted::SetMeshInstanceData(
	int which, StringView path, const Mat4& mtx, int flags, float lmdetail )
{
	if( which < 0 || which >= (int) m_meshIDs.size() )
		return;
	
	EdLGCMeshInfo M;
	M.xform = mtx;
	M.path = path;
	M.rflags = flags;
	M.lmdetail = lmdetail;
	g_EdLGCont->UpdateMesh( m_meshIDs[ which ], LGC_CHANGE_ALL, &M );
}

void EdEntScripted::GetMeshAABB( int which, Vec3 out[2] )
{
	out[0] = V3(-1);
	out[1] = V3(1);
	if( which < 0 || which >= (int) m_meshIDs.size() )
		return;
	
	g_EdLGCont->GetMeshAABB( m_meshIDs[ which ], out );
}

void EdEntScripted::SetScriptedItem( StringView name, sgsVariable args )
{
	SGRX_ScriptedItem* nsi = NULL;
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "SCRITEM_CREATE_%s", StackString<200>(name).str );
	sgsVariable func = g_ScriptCtx->GetGlobal( bfr );
	if( func.not_null() )
	{
		nsi = SGRX_ScriptedItem::Create(
			g_EdScene, g_EdPhyWorld, g_ScriptCtx->C,
			func, args );
		nsi->SetLightSampler( &GR_GetDummyLightSampler() );
		nsi->PreRender();
	}
	
	if( cached_scritem )
		cached_scritem->Release();
	cached_scritem = nsi;
}
#endif


EdEntNew::EdEntNew( sgsString type, bool isproto ) : EdEntity( isproto ), m_entityType( type )
{
	StringView typestr( type.c_str(), type.size() );
	sgsVariable eiface = g_Level->GetEntityInterface( typestr );
	m_iconTex = GR_GetTexture( eiface.getprop("ED_Icon").getdef<StringView>( "editor/icons/default.png" ) );
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s properties", StackString<240>(typestr).str );
	m_group.caption = bfr;
	m_group.SetOpen( true );
//	m_group.Add( &m_ctlPos );
	Add( &m_group );
	
	{
		SGS_CSCOPE( g_Level->GetSGSC() );
		sgs_PushPtr( g_Level->GetSGSC(), (void*) GetPropInterface() );
		eiface.thiscall( g_Level->GetSGSC(), "ED_PropInfo", 1 );
	}
	
	Fields2Data();
}

EdEntNew& EdEntNew::operator = ( const EdEntNew& o )
{
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

void EdEntNew::FLoad( sgsVariable data, int version )
{
	UNUSED( version );
	ASSERT( data.getprop( "entity_type" ) == m_entityType );
	sgsVariable props = data.getprop( "props" );
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		Field& F = m_fields[ i ];
		sgsVariable val = data.getprop( F.key );
		switch( F.property->type )
		{
		case EDGUI_ITEM_PROP_BOOL: ((EDGUIPropBool*)F.property)->SetValue( FLoadVar( val, false ) ); break;
		case EDGUI_ITEM_PROP_INT: ((EDGUIPropInt*)F.property)->SetValue( FLoadVar( val, int32_t(0) ) ); break;
		case EDGUI_ITEM_PROP_FLOAT: ((EDGUIPropFloat*)F.property)->SetValue( FLoadVar( val, 0.0f ) ); break;
		case EDGUI_ITEM_PROP_VEC2: ((EDGUIPropVec2*)F.property)->SetValue( FLoadVar( val, V2(0) ) ); break;
		case EDGUI_ITEM_PROP_VEC3: ((EDGUIPropVec3*)F.property)->SetValue( FLoadVar( val, V3(0) ) ); break;
		case EDGUI_ITEM_PROP_STRING:
		case EDGUI_ITEM_PROP_RSRC: ((EDGUIPropString*)F.property)->SetValue( FLoadVar( val, SV("") ) ); break;
		}
	}
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
		case EDGUI_ITEM_PROP_STRING:
		case EDGUI_ITEM_PROP_RSRC: val = FVar( ((EDGUIPropString*)F.property)->m_value ); break;
		}
		props.setprop( F.key, val );
	}
	
	sgsVariable out = FNewDict();
	out.setprop( "entity_type", m_entityType );
	out.setprop( "props", props );
	return out;
}

void EdEntNew::ClearFields()
{
	for( size_t i = 0; i < m_fields.size(); ++i )
	{
		if( m_fields[ i ].property == &m_ctlPos )
			continue;
		delete m_fields[ i ].property;
	}
	m_fields.clear();
}


static int EE_AddFieldBool( SGS_CTX )
{
	SGSFN( "EE_AddFieldBool" );
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropString( sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldMesh( SGS_CTX )
{
	SGSFN( "EE_AddFieldMesh" );
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UIMeshPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldTex( SGS_CTX )
{
	SGSFN( "EE_AddFieldTex" );
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UISurfTexPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldChar( SGS_CTX )
{
	SGSFN( "EE_AddFieldChar" );
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UICharPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldPartSys( SGS_CTX )
{
	SGSFN( "EE_AddFieldPartSys" );
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UIPartSysPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldSound( SGS_CTX )
{
	SGSFN( "EE_AddFieldSound" );
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
	E->AddField(
		sgs_GetVar<sgsString>()( C, 1 ),
		sgs_GetVar<StringView>()( C, 2 ),
		new EDGUIPropRsrc( g_UIScrFnPicker, sgs_GetVar<StringView>()( C, 3 ) ) );
	return 0;
}
static int EE_AddFieldScrItem( SGS_CTX )
{
	SGSFN( "EE_AddFieldScrItem" );
	SGRX_CAST( SGSPropInterface*, E, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->AddButtonSubent( sgs_GetVar<StringView>()( C, 1 ) );
	return 0;
}

static int EE_SetChar( SGS_CTX )
{
	SGSFN( "EE_SetChar" );
	SGRX_CAST( SGSPropInterface*, PI, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->SetMeshInstanceCount( sgs_GetVar<int>()( C, 1 ) );
	return 0;
}
static int EE_SetMeshInstanceData( SGS_CTX )
{
	SGSFN( "EE_SetMeshInstanceData" );
	SGRX_CAST( SGSPropInterface*, PI, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, PI, sgs_GetVar<void*>()( C, 0 ) );
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
	SGRX_CAST( SGSPropInterface*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->SetScriptedItem( sgs_GetVar<StringView>()( C, 1 ), sgsVariable( C, 2 ) );
	return 0;
}
#endif

static int EE_SetChangeFunc( SGS_CTX )
{
	SGSFN( "EE_SetChangeFunc" );
	SGRX_CAST( SGSPropInterface*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->onChange = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}
static int EE_SetDebugDrawFunc( SGS_CTX )
{
	SGSFN( "EE_SetDebugDrawFunc" );
	SGRX_CAST( SGSPropInterface*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->onDebugDraw = sgs_GetVar<sgsVariable>()( C, 1 );
	return 0;
}
static int EE_SetGatherFunc( SGS_CTX )
{
	SGSFN( "EE_SetGatherFunc" );
	SGRX_CAST( SGSPropInterface*, PI, sgs_GetVar<void*>()( C, 0 ) );
	if( PI->IsScrEnt() == false )
		return sgs_Msg( C, SGS_WARNING, "not scripted ent" );
	SGRX_CAST( EdEntScripted*, E, PI );
	E->onGather = sgs_GetVar<sgsVariable>()( C, 1 );
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
#endif
	{ "EE_SetChangeFunc", EE_SetChangeFunc },
	{ "EE_SetDebugDrawFunc", EE_SetDebugDrawFunc },
	{ "EE_SetGatherFunc", EE_SetGatherFunc },
	{ "EE_GenMatrix_SRaP", EE_GenMatrix_SRaP },
	SGS_RC_END(),
};


EDGUIEntList::EDGUIEntList() :
	EDGUIGroup( true, "Pick an entity type" )
{
#if 0
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
		if( key.starts_with( "ED_ENT_" ) )
		{
			key = key.part( sizeof("ED_ENT_") - 1 );
			Decl decl = { key.data(), new EdEntScripted( key.data() ) };
			ents.push_back( decl );
		}
	}
#endif
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

#if 0
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
#endif



