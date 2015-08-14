

#define MAPEDIT_DEFINE_GLOBALS
#include "mapedit.hpp"



EDGUIPropRsrc_PickParentGroup::EDGUIPropRsrc_PickParentGroup( int32_t id, EDGUIGroupPicker* gp, const StringView& value ) :
	EDGUIPropRsrc( gp, value ),
	m_id( id )
{
}

void EDGUIPropRsrc_PickParentGroup::OnReload( bool after )
{
	SGRX_CAST( EDGUIGroupPicker*, gp, m_rsrcPicker );
	gp->m_ignoreID = after ? -1 : m_id;
}

EdGroup::EdGroup( struct EdGroupManager* groupMgr, int32_t id, int32_t pid, const StringView& name ) :
	m_refcount( 0 ),
	m_groupMgr( groupMgr ),
	m_id( id ),
	m_parent_id( pid ),
	m_needsMtxUpdate( true ),
	m_mtxLocal( Mat4::Identity ),
	m_mtxCombined( Mat4::Identity ),
	m_needsPathUpdate( true ),
	m_group( true, "Group" ),
	m_ctlName( name ),
	m_ctlParent( id, &groupMgr->m_grpPicker, id ? groupMgr->GetPath( pid ) : StringView() ),
	m_ctlOrigin( V3(0), 2, V3(-8192), V3(8192) ),
	m_ctlPos( V3(0), 2, V3(-8192), V3(8192) ),
	m_ctlAngles( V3(0), 2, V3(0), V3(360) ),
	m_ctlScaleUni( 1, 2, 0.01f, 100.0f )
{
	m_ctlName.caption = "Name";
	m_ctlParent.caption = "Parent";
	m_ctlOrigin.caption = "Origin";
	m_ctlPos.caption = "Position";
	m_ctlAngles.caption = "Rotation";
	m_ctlScaleUni.caption = "Scale (uniform)";
	m_addChild.caption = "Add child";
	m_deleteDisownParent.caption = "Delete group and subobjects -> parent";
	m_deleteDisownRoot.caption = "Delete group and subobjects -> root";
	m_deleteRecursive.caption = "Delete group and destroy subobjects";
	m_recalcOrigin.caption = "Recalculate origin";
	m_cloneGroup.caption = "Clone group only";
	m_cloneGroupWithSub.caption = "Clone group with subobjects";
	m_exportObj.caption = "Export group as .obj";
	
	m_group.Add( &m_ctlName );
	if( m_id )
	{
		m_group.Add( &m_ctlParent );
	}
	m_group.Add( &m_ctlOrigin );
	m_group.Add( &m_ctlPos );
	m_group.Add( &m_ctlAngles );
	m_group.Add( &m_ctlScaleUni );
	m_group.Add( &m_addChild );
	if( m_id )
	{
		m_group.Add( &m_deleteDisownParent );
		m_group.Add( &m_deleteDisownRoot );
		m_group.Add( &m_deleteRecursive );
	}
	m_group.Add( &m_recalcOrigin );
	if( m_id )
	{
		m_group.Add( &m_cloneGroup );
		m_group.Add( &m_cloneGroupWithSub );
	}
	m_group.Add( &m_exportObj );
	
	Add( &m_group );
}

int EdGroup::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_BTNCLICK:
		if( e->target == &m_addChild )
		{
			m_groupMgr->PrepareEditGroup( m_groupMgr->AddGroup( m_id ) );
		}
		if( e->target == &m_deleteDisownParent )
		{
			g_EdWorld->TransferObjectsToGroup( m_id, m_parent_id );
			m_groupMgr->TransferGroupsToGroup( m_id, m_parent_id );
			m_groupMgr->QueueDestroy( this );
		}
		if( e->target == &m_deleteDisownRoot )
		{
			g_EdWorld->TransferObjectsToGroup( m_id, 0 );
			m_groupMgr->TransferGroupsToGroup( m_id, 0 );
			m_groupMgr->QueueDestroy( this );
		}
		if( e->target == &m_deleteRecursive )
		{
			m_groupMgr->QueueDestroy( this );
		}
		if( e->target == &m_recalcOrigin )
		{
			m_ctlOrigin.SetValue( g_EdWorld->FindCenterOfGroup( m_id ) );
		}
		if( e->target == &m_cloneGroup )
		{
			EdGroup* grp = Clone();
			m_groupMgr->PrepareEditGroup( grp );
		}
		if( e->target == &m_cloneGroupWithSub )
		{
			EdGroup* grp = Clone();
			g_EdWorld->CopyObjectsToGroup( m_id, grp->m_id );
			m_groupMgr->PrepareEditGroup( grp );
		}
		if( e->target == &m_exportObj )
		{
			g_EdWorld->ExportGroupAsOBJ( m_id, "group.obj" );
		}
		break;
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_ctlOrigin ||
			e->target == &m_ctlPos ||
			e->target == &m_ctlAngles ||
			e->target == &m_ctlScaleUni ||
			e->target == &m_ctlParent )
		{
			m_groupMgr->MatrixInvalidate( m_id );
		}
		break;
	case EDGUI_EVENT_PROPCHANGE:
		if( e->target == &m_ctlParent )
		{
			m_parent_id = m_groupMgr->FindGroupByPath( m_ctlParent.m_value )->m_id;
		}
		if( e->target == &m_ctlName || e->target == &m_ctlParent )
		{
			m_groupMgr->PathInvalidate( m_id );
			m_groupMgr->m_editedGroup.SetValue( GetPath() );
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}

Mat4 EdGroup::GetMatrix()
{
	if( m_needsMtxUpdate )
	{
		m_mtxCombined = m_mtxLocal = Mat4::CreateTranslation( -m_ctlOrigin.m_value ) *
			Mat4::CreateSRT( V3( m_ctlScaleUni.m_value ), DEG2RAD( m_ctlAngles.m_value ), m_ctlPos.m_value + m_ctlOrigin.m_value );
		if( m_id )
		{
			m_mtxCombined = m_mtxCombined * m_groupMgr->GetMatrix( m_parent_id );
		}
	}
	return m_mtxCombined;
}

StringView EdGroup::GetPath()
{
	if( m_needsPathUpdate )
	{
		m_path.clear();
		if( m_id )
		{
			m_path = m_groupMgr->GetPath( m_parent_id );
			m_path.append( "/" );
		}
		m_path.append( Name() );
	}
	return m_path;
}

EdGroup* EdGroup::Clone()
{
	EdGroup* grp = m_groupMgr->AddGroup( m_parent_id );
	grp->m_ctlOrigin.SetValue( m_ctlOrigin.m_value );
	grp->m_ctlPos.SetValue( m_ctlPos.m_value );
	grp->m_ctlAngles.SetValue( m_ctlAngles.m_value );
	grp->m_ctlScaleUni.SetValue( m_ctlScaleUni.m_value );
	return grp;
}


void EDGUIGroupPicker::Reload()
{
	m_options.clear();
	for( size_t i = 0; i < m_groupMgr->m_groups.size(); ++i )
	{
		EdGroup* grp = m_groupMgr->m_groups.item( i ).value;
		if( grp->m_id != m_ignoreID && m_groupMgr->GroupHasParent( grp->m_id, m_ignoreID ) == false )
			m_options.push_back( grp->GetPath() );
	}
	_Search( m_searchString );
}


EdGroupManager::EdGroupManager() :
	m_grpPicker( this ),
	m_lastGroupID(-1),
	m_editedGroup( &m_grpPicker )
{
	m_gotoRoot.caption = "Go to root";
	m_editedGroup.caption = "Edited group";
}

int EdGroupManager::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_BTNCLICK:
		if( e->target == &m_gotoRoot )
		{
			PrepareEditGroup( FindGroupByID( 0 ) );
		}
		break;
	case EDGUI_EVENT_PROPCHANGE:
		if( e->target == &m_editedGroup )
		{
			PrepareCurrentEditGroup();
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}

void EdGroupManager::DrawGroups()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	EdGroup* grp = FindGroupByPath( m_editedGroup.m_value );
	while( grp )
	{
		Mat4 parent_mtx = grp->m_id ? GetMatrix( grp->m_parent_id ) : Mat4::Identity;
		
		br.Col( 0.8f, 0.05f, 0.01f );
		br.Tick( grp->m_ctlPos.m_value + grp->m_ctlOrigin.m_value, 0.1f, parent_mtx );
		br.Col( 0.05f, 0.8f, 0.01f );
		br.Tick( grp->m_ctlPos.m_value, 0.1f, parent_mtx );
		
		if( grp->m_id == 0 )
			break;
		grp = FindGroupByID( grp->m_parent_id );
	}
}

void EdGroupManager::AddRootGroup()
{
	_AddGroup( 0, 0, "root" );
}

Mat4 EdGroupManager::GetMatrix( int32_t id )
{
	EdGroup* grp = m_groups.getcopy( id );
	ASSERT( grp );
	return grp->GetMatrix();
}

StringView EdGroupManager::GetPath( int32_t id )
{
	EdGroup* grp = m_groups.getcopy( id );
	ASSERT( grp );
	return grp->GetPath();
}

EdGroup* EdGroupManager::AddGroup( int32_t parent_id, StringView name, int32_t id )
{
	char bfr[ 32 ];
	if( id < 0 )
		id = m_lastGroupID + 1;
	if( !name )
	{
		sgrx_snprintf( bfr, 32, "group%d", id );
		name = bfr;
	}
	return _AddGroup( id, parent_id, name );
}

EdGroup* EdGroupManager::_AddGroup( int32_t id, int32_t parent_id, StringView name )
{
	EdGroup* grp = new EdGroup( this, id, parent_id, name );
	m_groups[ id ] = grp;
	if( m_lastGroupID < id )
		m_lastGroupID = id;
	return grp;
}

EdGroup* EdGroupManager::FindGroupByID( int32_t id )
{
	return m_groups.getcopy( id );
}

EdGroup* EdGroupManager::FindGroupByPath( StringView path )
{
	for( size_t i = 0; i < m_groups.size(); ++i )
	{
		EdGroup* grp = m_groups.item( i ).value;
		if( grp->GetPath() == path )
			return grp;
	}
	return NULL;
}

bool EdGroupManager::GroupHasParent( int32_t id, int32_t parent_id )
{
	while( id != 0 )
	{
		id = FindGroupByID( id )->m_parent_id;
		if( id == parent_id )
			return true;
	}
	return false;
}

void EdGroupManager::PrepareEditGroup( EdGroup* grp )
{
	Clear();
	Add( &m_gotoRoot );
	Add( &m_editedGroup );
	if( grp )
	{
		m_editedGroup.SetValue( grp->GetPath() );
		Add( grp );
	}
	else
		m_editedGroup.SetValue( "" );
}

void EdGroupManager::PrepareCurrentEditGroup()
{
	PrepareEditGroup( FindGroupByPath( m_editedGroup.m_value ) );
}

void EdGroupManager::TransferGroupsToGroup( int32_t from, int32_t to )
{
	for( size_t i = 0; i < m_groups.size(); ++i )
	{
		EdGroup* grp = m_groups.item( i ).value;
		if( grp->m_parent_id == from )
		{
			grp->m_parent_id = to;
			PathInvalidate( grp->m_id );
		}
	}
}

void EdGroupManager::QueueDestroy( EdGroup* grp )
{
	m_destroyQueue.push_back( grp );
	m_groups.unset( grp->m_id );
}

void EdGroupManager::ProcessDestroyQueue()
{
	for( size_t i = 0; i < m_destroyQueue.size(); ++i )
	{
		g_EdWorld->DeleteObjectsInGroup( m_destroyQueue[ i ]->m_id );
		for( size_t j = 0; j < m_groups.size(); ++j )
		{
			EdGroup* grp = m_groups.item( j ).value;
			if( grp->m_parent_id == m_destroyQueue[ i ]->m_id )
				m_destroyQueue.push_back( grp );
		}
	}
	m_destroyQueue.clear();
}

void EdGroupManager::MatrixInvalidate( int32_t id )
{
	FindGroupByID( id )->m_needsMtxUpdate = true;
	g_EdWorld->FixTransformsOfGroup( id );
	for( size_t i = 0; i < m_groups.size(); ++i )
	{
		EdGroup* grp = m_groups.item( i ).value;
		if( grp->m_id && grp->m_parent_id == id )
			MatrixInvalidate( grp->m_id ); // TODO: might need to fix algorithm for performance?
	}
}

void EdGroupManager::PathInvalidate( int32_t id )
{
	FindGroupByID( id )->m_needsPathUpdate = true;
	for( size_t i = 0; i < m_groups.size(); ++i )
	{
		EdGroup* grp = m_groups.item( i ).value;
		if( grp->m_id && grp->m_parent_id == id )
			PathInvalidate( grp->m_id ); // TODO: might need to fix algorithm for performance?
	}
}



EdLevelGraphicsCont::EdLevelGraphicsCont()
	: m_nextMeshID(1), m_nextSurfID(1), m_nextLightID(1)
{
}

void EdLevelGraphicsCont::Reset()
{
	m_nextMeshID = 1;
	m_nextSurfID = 1;
	m_nextLightID = 1;
	m_meshes.clear();
	m_surfaces.clear();
	m_lights.clear();
}

void EdLevelGraphicsCont::LightMesh( SGRX_MeshInstance* MI )
{
	// dummy static lighting
	MI->textures[0] = GR_GetTexture( "textures/deflm.png" );
	// dummy dynamic lighting
	for( int i = 10; i < 16; ++i )
		MI->constants[ i ] = V4(0.15f);
}

uint32_t EdLevelGraphicsCont::CreateMesh( EdLGCMeshInfo* info )
{
	uint32_t id = m_nextMeshID++;
	ASSERT( id );
	RequestMesh( id, info );
	return id;
}

void EdLevelGraphicsCont::RequestMesh( uint32_t id, EdLGCMeshInfo* info )
{
	ASSERT( m_meshes.isset( id ) == false );
	Mesh M;
	M.meshInst = g_EdScene->CreateMeshInstance();
	m_meshes.set( id, M );
	UpdateMesh( id, LGC_CHANGE_ALL, info );
}

void EdLevelGraphicsCont::DeleteMesh( uint32_t id )
{
	ASSERT( m_meshes.isset( id ) );
	m_meshes.unset( id );
	if( id == m_nextMeshID - 1 )
		m_nextMeshID--;
}

static EdLGCMeshInfo g_defMeshInfo;
void EdLevelGraphicsCont::UpdateMesh( uint32_t id, uint32_t changes, EdLGCMeshInfo* info )
{
	if( info == NULL )
		info = &g_defMeshInfo;
	ASSERT( m_meshes.isset( id ) );
	Mesh& M = m_meshes[ id ];
	if( changes & LGC_MESH_CHANGE_PATH )
	{
		M.meshInst->mesh = info->path.size() ? GR_GetMesh( info->path ) : NULL;
	}
	if( changes & LGC_CHANGE_XFORM )
	{
		M.meshInst->matrix = info->xform;
	}
	if( changes & LGC_CHANGE_RSPEC )
	{
		M.info = *info;
		M.meshInst->dynamic = ( info->rflags & LM_MESHINST_DYNLIT ) != 0;
		M.meshInst->decal = ( info->rflags & LM_MESHINST_DECAL ) != 0;
		LightMesh( M.meshInst );
	}
}

bool EdLevelGraphicsCont::GetMeshAABB( uint32_t id, Vec3 out[2] )
{
	ASSERT( m_meshes.isset( id ) );
	Mesh& M = m_meshes[ id ];
	if( M.meshInst->mesh == NULL )
		return false;
	
	out[0] = M.meshInst->mesh->m_boundsMin;
	out[1] = M.meshInst->mesh->m_boundsMax;
	return true;
}

uint32_t EdLevelGraphicsCont::CreateSurface( EdLGCSurfaceInfo* info )
{
	uint32_t id = m_nextSurfID++;
	ASSERT( id );
	RequestSurface( id, info );
	return id;
}

void EdLevelGraphicsCont::RequestSurface( uint32_t id, EdLGCSurfaceInfo* info )
{
	ASSERT( m_surfaces.isset( id ) == false );
	Surface S;
	S.meshInst = g_EdScene->CreateMeshInstance();
	S.meshInst->mesh = GR_CreateMesh();
	S.material = GR_CreateMaterial();
	S.material->shader = GR_GetSurfaceShader( "default" );
	m_surfaces.set( id, S );
	UpdateSurface( id, LGC_CHANGE_ALL, info );
}

void EdLevelGraphicsCont::DeleteSurface( uint32_t id )
{
	ASSERT( m_surfaces.isset( id ) );
	m_surfaces.unset( id );
	if( id == m_nextSurfID - 1 )
		m_nextSurfID--;
}

static EdLGCSurfaceInfo g_defSurfInfo;
void EdLevelGraphicsCont::UpdateSurface( uint32_t id, uint32_t changes, EdLGCSurfaceInfo* info )
{
	if( info == NULL )
		info = &g_defSurfInfo;
	ASSERT( m_surfaces.isset( id ) );
	Surface& S = m_surfaces[ id ];
	if( changes & LGC_SURF_CHANGE_VIDATA )
	{
		S.vertices.assign( info->vdata, info->vcount );
		S.indices.assign( info->idata, info->icount );
		S.lmsize = info->lmsize;
		
		VertexDeclHandle vd = GR_GetVertexDecl( LCVertex_DECL );
		S.meshInst->mesh->SetVertexData( S.vertices.data(), S.vertices.size_bytes(), vd, false );
		S.meshInst->mesh->SetIndexData( S.indices.data(), S.indices.size_bytes(), false );
		S.meshInst->mesh->SetAABBFromVertexData( S.vertices.data(), S.vertices.size_bytes(), vd );
		SGRX_MeshPart mp = { 0, S.vertices.size(), 0, S.indices.size(), S.material };
		S.meshInst->mesh->SetPartData( &mp, 1 );
	}
	if( changes & LGC_SURF_CHANGE_MTLDATA )
	{
		S.mtlname = info->mtlname;
		if( S.mtlname.size() )
		{
			char bfr[ 256 ];
			sgrx_snprintf( bfr, sizeof(bfr), "textures/%s.png", StackString<200>(S.mtlname).str );
			S.material->textures[0] = GR_GetTexture( bfr );
		}
	}
	if( changes & LGC_CHANGE_XFORM )
	{
		S.meshInst->matrix = info->xform;
	}
	if( changes & LGC_CHANGE_RSPEC )
	{
		S.info = *info;
		S.meshInst->dynamic = ( info->rflags & LM_MESHINST_DYNLIT ) != 0;
		S.meshInst->decal = ( info->rflags & LM_MESHINST_DECAL ) != 0;
		S.material->blendMode = S.meshInst->decal ? MBM_BASIC : MBM_NONE;
		LightMesh( S.meshInst );
	}
	S.meshInst->enabled = S.mtlname.size() != 0 && S.vertices.size() != 0 && S.indices.size() != 0;
}

uint32_t EdLevelGraphicsCont::CreateLight( EdLGCLightInfo* info )
{
	uint32_t id = m_nextLightID++;
	ASSERT( id );
	RequestLight( id, info );
	return id;
}

void EdLevelGraphicsCont::RequestLight( uint32_t id, EdLGCLightInfo* info )
{
	ASSERT( m_lights.isset( id ) == false );
	Light L;
	m_lights.set( id, L );
	UpdateLight( id, LGC_CHANGE_ALL, info );
}

void EdLevelGraphicsCont::DeleteLight( uint32_t id )
{
	ASSERT( m_lights.isset( id ) );
	m_lights.unset( id );
	if( id == m_nextLightID - 1 )
		m_nextLightID--;
}

static EdLGCLightInfo g_defLightInfo;
void EdLevelGraphicsCont::UpdateLight( uint32_t id, uint32_t changes, EdLGCLightInfo* info )
{
	if( info == NULL )
		info = &g_defLightInfo;
	ASSERT( m_lights.isset( id ) );
	Light& L = m_lights[ id ];
}



void EdObject::UISelectElement( int i, bool mod )
{
	if( i >= 0 && i < GetNumElements() )
	{
		if( mod )
			SelectElement( i, !IsElementSelected( i ) );
		else
		{
			ClearSelection();
			SelectElement( i, true );
		}
		
	}
	else if( mod == false )
		ClearSelection();
}

void EdObject::ProjectSelectedVertices()
{
	Vec3 origin = g_EdScene->camera.position;
	int numverts = GetNumVerts();
	EdObject* skiplist[2] = { this, NULL };
	for( int i = 0; i < numverts; ++i )
	{
		if( IsVertexSelected( i ) == false )
			continue;
		
		Vec3 pos = GetLocalVertex( i );
		
		Vec3 dir = ( pos - origin ).Normalized();
		float dst = FLT_MAX;
		float ndst = 0;
		if( g_EdWorld->RayBlocksIntersect( origin, dir, -1, &ndst, NULL, skiplist ) && ndst < dst ) dst = ndst;
		if( g_EdWorld->RayPatchesIntersect( origin, dir, -1, &ndst, NULL, skiplist ) && ndst < dst ) dst = ndst;
		
		if( dst < FLT_MAX )
		{
			dst -= g_UIFrame->m_snapProps.m_projDist.m_value;
			SetLocalVertex( i, origin + dir * dst );
		}
	}
}


EDGUIPaintProps::EDGUIPaintProps() :
	m_ctlGroup( true, "Painting properties" ),
	m_ctlLayerNum( 0, 0, 3 ),
	m_ctlPaintPos( true ),
	m_ctlPaintColor( true ),
	m_ctlPaintAlpha( true ),
	m_ctlRadius( 1.0f, 2, 0.0f, 64.0f ),
	m_ctlFalloff( 1.0f, 2, 0.01f, 100.0f ),
	m_ctlSculptSpeed( 1.0f, 2, 0.01f, 100.0f ),
	m_ctlPaintSpeed( 1.0f, 2, 0.01f, 100.0f ),
	m_ctlColorHSV( V3(0,0,1), 2, V3(0), V3(1,1,1) ),
	m_ctlAlpha( 1.0f, 2, 0.0f, 1.0f )
{
	m_ctlLayerNum.caption = "Color layer [0-3]";
	m_ctlPaintPos.caption = "Sculpt?";
	m_ctlPaintColor.caption = "Paint color?";
	m_ctlPaintAlpha.caption = "Paint alpha?";
	m_ctlRadius.caption = "Brush radius";
	m_ctlFalloff.caption = "Brush falloff";
	m_ctlSculptSpeed.caption = "Sculpt speed";
	m_ctlPaintSpeed.caption = "Paint speed";
	m_ctlColorHSV.caption = "Color (HSV)";
	m_ctlAlpha.caption = "Alpha";
	
	m_ctlGroup.Add( &m_ctlLayerNum );
	m_ctlGroup.Add( &m_ctlPaintPos );
	m_ctlGroup.Add( &m_ctlPaintColor );
	m_ctlGroup.Add( &m_ctlPaintAlpha );
	m_ctlGroup.Add( &m_ctlRadius );
	m_ctlGroup.Add( &m_ctlFalloff );
	m_ctlGroup.Add( &m_ctlSculptSpeed );
	m_ctlGroup.Add( &m_ctlPaintSpeed );
	m_ctlGroup.Add( &m_ctlColorHSV );
	m_ctlGroup.Add( &m_ctlAlpha );
	Add( &m_ctlGroup );
	
	_UpdateColor();
}

float EDGUIPaintProps::GetDistanceFactor( const Vec3& vpos, const Vec3& bpos )
{
	float dist = ( vpos - bpos ).Length();
	return powf( 1 - clamp( dist / m_ctlRadius.m_value, 0, 1 ), m_ctlFalloff.m_value );
}

void EDGUIPaintProps::Paint( Vec3& vpos, const Vec3& nrm, Vec4& vcol, float factor )
{
	if( m_ctlPaintPos.m_value )
	{
		vpos += nrm * factor * m_ctlSculptSpeed.m_value * ( ( g_UIFrame->m_keyMod & KMOD_ALT ) ? -1 : 1 );
	}
	if( m_ctlPaintColor.m_value || m_ctlPaintAlpha.m_value )
	{
		Vec4 cf = V4( V3( m_ctlPaintColor.m_value ), m_ctlPaintAlpha.m_value ) * clamp( m_ctlPaintSpeed.m_value * factor, 0, 1 );
		vcol = TLERP( vcol, ( g_UIFrame->m_keyMod & KMOD_ALT ) ? V4(1) - m_tgtColor : m_tgtColor, cf );
	}
}

int EDGUIPaintProps::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_ctlColorHSV || e->target == &m_ctlAlpha )
			_UpdateColor();
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}

void EDGUIPaintProps::_UpdateColor()
{
	m_tgtColor = V4( HSV( m_ctlColorHSV.m_value ), m_ctlAlpha.m_value );
}



EdWorld::EdWorld() :
	m_ctlGroup( true, "Level properties" ),
	m_ctlAmbientColor( V3(0,0,0.1f), 2, V3(0), V3(1,1,100) ),
	m_ctlDirLightDir( V2(0,0), 2, V2(-8192), V2(8192) ),
	m_ctlDirLightColor( V3(0,0,0.0f), 2, V3(0), V3(1,1,100) ),
	m_ctlDirLightDivergence( 10, 2, 0, 180 ),
	m_ctlDirLightNumSamples( 15, 0, 256 ),
	m_ctlLightmapClearColor( V3(0,0,0), 2, V3(0), V3(1,1,100) ),
	m_ctlRADNumBounces( 2, 0, 256 ),
	m_ctlLightmapDetail( 1, 2, 0.01f, 16 ),
	m_ctlLightmapBlurSize( 1, 2, 0, 10 ),
	m_ctlAODistance( 2, 2, 0, 100 ),
	m_ctlAOMultiplier( 1, 2, 0, 2 ),
	m_ctlAOFalloff( 2, 2, 0.01f, 100.0f ),
	m_ctlAOEffect( 0, 2, -1, 1 ),
//	m_ctlAODivergence( 0, 2, 0, 1 ),
	m_ctlAOColor( V3(0,0,0), 2, V3(0), V3(1,1,100) ),
	m_ctlAONumSamples( 15, 0, 256 )
{
	tyname = "world";
	m_ctlAmbientColor.caption = "Ambient color";
	m_ctlDirLightDir.caption = "Dir.light direction (dX,dY)";
	m_ctlDirLightColor.caption = "Dir.light color (HSV)";
	m_ctlDirLightDivergence.caption = "Dir.light divergence";
	m_ctlDirLightNumSamples.caption = "Dir.light sample count";
	m_ctlLightmapClearColor.caption = "Lightmap clear color (HSV)";
	m_ctlRADNumBounces.caption = "Radiosity bounce count";
	m_ctlLightmapDetail.caption = "Lightmap detail";
	m_ctlLightmapBlurSize.caption = "Lightmap blur size";
	m_ctlAODistance.caption = "AO distance";
	m_ctlAOMultiplier.caption = "AO multiplier";
	m_ctlAOFalloff.caption = "AO falloff";
	m_ctlAOEffect.caption = "AO effect";
//	m_ctlAODivergence.caption = "AO divergence";
	m_ctlAOColor.caption = "AO color";
	m_ctlAONumSamples.caption = "AO sample count";
	m_vd = GR_GetVertexDecl( LCVertex_DECL );
	
	Add( &m_ctlGroup );
	m_ctlGroup.Add( &m_ctlAmbientColor );
	m_ctlGroup.Add( &m_ctlDirLightDir );
	m_ctlGroup.Add( &m_ctlDirLightColor );
	m_ctlGroup.Add( &m_ctlDirLightDivergence );
	m_ctlGroup.Add( &m_ctlDirLightNumSamples );
	m_ctlGroup.Add( &m_ctlLightmapClearColor );
	m_ctlGroup.Add( &m_ctlRADNumBounces );
	m_ctlGroup.Add( &m_ctlLightmapDetail );
	m_ctlGroup.Add( &m_ctlLightmapBlurSize );
	m_ctlGroup.Add( &m_ctlAODistance );
	m_ctlGroup.Add( &m_ctlAOMultiplier );
	m_ctlGroup.Add( &m_ctlAOFalloff );
	m_ctlGroup.Add( &m_ctlAOEffect );
//	m_ctlGroup.Add( &m_ctlAODivergence );
	m_ctlGroup.Add( &m_ctlAOColor );
	m_ctlGroup.Add( &m_ctlAONumSamples );
	
	TestData();
}

EdWorld::~EdWorld()
{
	Reset();
}

void EdWorld::Reset()
{
	while( m_objects.size() )
		DeleteObject( m_objects.last() );
	m_blocks.clear();
	m_entities.clear();
	m_patches.clear();
	m_objects.clear();
	g_EdLGCont->Reset();
}

void EdWorld::TestData()
{
	m_groupMgr.AddRootGroup();
	
	EdBlock b1;
	
	b1.position = V3(0);
	b1.z0 = 0;
	b1.z1 = 2;
	
	Vec3 poly[] = { {-2,-2,0}, {2,-2,0}, {2,2,0}, {-2,2,0} };
	b1.poly.assign( poly, 4 );
	
	EdSurface surf;
	surf.texname = "metal0";
	for( int i = 0; i < 6; ++i )
		b1.surfaces.push_back( surf );
	b1.subsel.resize( b1.GetNumElements() );
	b1.ClearSelection();
	
	AddObject( b1.Clone() );
	b1.z1 = 1;
	b1.position = V3( 0.1f, 1, 0.5f );
	AddObject( b1.Clone() );
	
	RegenerateMeshes();
}

void EdWorld::RegenerateMeshes()
{
	for( size_t i = 0; i < m_objects.size(); ++i )
		m_objects[ i ]->RegenerateMesh();
}

void EdWorld::DrawWires_Objects( EdObject* hl, bool tonedown )
{
	DrawWires_Blocks( hl );
	DrawWires_Patches( hl, tonedown );
	DrawWires_Entities( hl );
}

void EdWorld::DrawWires_Blocks( EdObject* hl )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	br.SetPrimitiveType( PT_Lines ).UnsetTexture();
	for( size_t i = 0; i < m_blocks.size(); ++i )
	{
		const EdBlock& B = *m_blocks[ i ];
		GR2D_SetWorldMatrix( m_groupMgr.GetMatrix( B.group ) );
		
		if( B.selected )
		{
			if( &B == hl )
				br.Col( 0.9f, 0.2f, 0.1f, 1 );
			else
				br.Col( 0.9f, 0.5f, 0.1f, 1 );
		}
		else if( &B == hl )
			br.Col( 0.1f, 0.8f, 0.9f, 0.9f );
		else
			br.Col( 0.1f, 0.5f, 0.9f, 0.5f );
		
		for( size_t v = 0; v < B.poly.size(); ++v )
		{
			size_t v1 = ( v + 1 ) % B.poly.size();
			
			br.Pos( B.position.x + B.poly[ v ].x,  B.position.y + B.poly[ v ].y, B.z0 + B.position.z );
			br.Pos( B.position.x + B.poly[ v1 ].x, B.position.y + B.poly[ v1 ].y, B.z0 + B.position.z );
			
			br.Pos( B.position.x + B.poly[ v ].x,  B.position.y + B.poly[ v ].y, B.poly[ v ].z + B.z1 + B.position.z );
			br.Pos( B.position.x + B.poly[ v1 ].x, B.position.y + B.poly[ v1 ].y, B.poly[ v1 ].z + B.z1 + B.position.z );
			
			br.Prev( 3 ).Prev( 2 );
		}
	}
	
	br.Flush();
	GR2D_SetWorldMatrix( Mat4::Identity );
}

void EdWorld::DrawPoly_BlockSurf( int block, int surf, bool sel )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	br.SetPrimitiveType( PT_TriangleStrip ).UnsetTexture();
	
	if( sel )
		br.Col( 0.9f, 0.5, 0.1f, 0.2f );
	else
		br.Col( 0.1f, 0.5, 0.9f, 0.1f );
	
	const EdBlock& B = *m_blocks[ block ];
	GR2D_SetWorldMatrix( m_groupMgr.GetMatrix( B.group ) );
	if( surf == (int) B.poly.size() )
	{
		for( size_t i = 0; i < B.poly.size(); ++i )
		{
			size_t v;
			if( i % 2 == 0 )
				v = i / 2;
			else
				v = B.poly.size() - 1 - i / 2;
			br.Pos( B.poly[ v ].x + B.position.x, B.poly[ v ].y + B.position.y, B.poly[ v ].z + B.z1 + B.position.z );
		}
//		br.Prev( B.poly.size() - 1 );
	}
	else if( surf == (int) B.poly.size() + 1 )
	{
		for( size_t i = B.poly.size(); i > 0; )
		{
			--i;
			size_t v;
			if( i % 2 == 0 )
				v = i / 2;
			else
				v = B.poly.size() - 1 - i / 2;
			br.Pos( B.poly[ v ].x + B.position.x, B.poly[ v ].y + B.position.y, B.z0 + B.position.z );
		}
//		br.Prev( B.poly.size() - 1 );
	}
	else
	{
		size_t v = surf, v1 = ( surf + 1 ) % B.poly.size();
		br.Pos( B.position.x + B.poly[ v ].x,  B.position.y + B.poly[ v ].y, B.z0 + B.position.z );
		br.Pos( B.position.x + B.poly[ v1 ].x, B.position.y + B.poly[ v1 ].y, B.z0 + B.position.z );
		
		br.Pos( B.position.x + B.poly[ v ].x, B.position.y + B.poly[ v ].y, B.z1 + B.position.z );
		br.Pos( B.position.x + B.poly[ v1 ].x,  B.position.y + B.poly[ v1 ].y, B.z1 + B.position.z );
	}
	
	br.Flush();
	GR2D_SetWorldMatrix( Mat4::Identity );
}

void EdWorld::DrawPoly_BlockVertex( int block, int vert, bool sel )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	br.SetPrimitiveType( PT_Lines ).UnsetTexture();
	
	if( sel )
		br.Col( 0.9f, 0.5, 0.1f, 0.9f );
	else
		br.Col( 0.1f, 0.5, 0.9f, 0.5f );
	
	const EdBlock& B = *m_blocks[ block ];
	GR2D_SetWorldMatrix( m_groupMgr.GetMatrix( B.group ) );
	Vec3 P = V3( B.poly[ vert ].x + B.position.x, B.poly[ vert ].y + B.position.y, B.z0 + B.position.z );
	
	float s = 0.5f;
	br.Pos( P - V3(s,0,0) ).Pos( P + V3(0,0,s) ).Prev(0).Pos( P + V3(s,0,0) ).Prev(0).Pos( P - V3(0,0,s) ).Prev(0).Prev(6);
	br.Pos( P - V3(0,s,0) ).Pos( P + V3(0,0,s) ).Prev(0).Pos( P + V3(0,s,0) ).Prev(0).Pos( P - V3(0,0,s) ).Prev(0).Prev(6);
	br.Pos( P - V3(s,0,0) ).Pos( P + V3(0,s,0) ).Prev(0).Pos( P + V3(s,0,0) ).Prev(0).Pos( P - V3(0,s,0) ).Prev(0).Prev(6);
}

void EdWorld::DrawWires_Patches( EdObject* hl, bool tonedown )
{
	float ga = tonedown ? 0.5f : 1;
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
	
	br.SetPrimitiveType( PT_Lines ).UnsetTexture();
	for( size_t i = 0; i < m_patches.size(); ++i )
	{
		EdPatch* ptc = m_patches[ i ];
		GR2D_SetWorldMatrix( m_groupMgr.GetMatrix( ptc->group ) );
		
		if( ptc->selected )
			br.Col( 0.9f, 0.5, 0.1f, 0.9f * ga );
		else if( ptc == hl )
			br.Col( 0.1f, 0.5, 0.9f, 0.7f * ga );
		else
			br.Col( 0.1f, 0.5, 0.9f, 0.25f * ga );
		
		// grid lines
		for( int y = 0; y < ptc->ysize; ++y )
		{
			for( int x = 0; x < ptc->xsize - 1; ++x )
			{
				br.Pos( ptc->vertices[ x + y * MAX_PATCH_WIDTH ].pos + ptc->position );
				br.Pos( ptc->vertices[ x + 1 + y * MAX_PATCH_WIDTH ].pos + ptc->position );
			}
		}
		for( int x = 0; x < ptc->xsize; ++x )
		{
			for( int y = 0; y < ptc->ysize - 1; ++y )
			{
				br.Pos( ptc->vertices[ x + y * MAX_PATCH_WIDTH ].pos + ptc->position );
				br.Pos( ptc->vertices[ x + ( y + 1 ) * MAX_PATCH_WIDTH ].pos + ptc->position );
			}
		}
		// inner edges
		for( int y = 0; y < ptc->ysize - 1; ++y )
		{
			for( int x = 0; x < ptc->xsize - 1; ++x )
			{
				if( ptc->edgeflip[ y ] & ( 1 << x ) )
				{
					br.Pos( ptc->vertices[ x + ( y + 1 ) * MAX_PATCH_WIDTH ].pos + ptc->position );
					br.Pos( ptc->vertices[ x + 1 + y * MAX_PATCH_WIDTH ].pos + ptc->position );
				}
				else
				{
					br.Pos( ptc->vertices[ x + y * MAX_PATCH_WIDTH ].pos + ptc->position );
					br.Pos( ptc->vertices[ x + 1 + ( y + 1 ) * MAX_PATCH_WIDTH ].pos + ptc->position );
				}
			}
		}
	}
	
	br.Flush();
	GR2D_SetWorldMatrix( Mat4::Identity );
}

void EdWorld::DrawWires_Entities( EdObject* hl )
{
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
	
	br.SetPrimitiveType( PT_Lines );
	for( size_t i = 0; i < m_entities.size(); ++i )
	{
		EdEntity* ent = m_entities[ i ];
		if( ent->selected )
			br.Col( 0.9f, 0.5, 0.1f, 0.9f );
		else if( ent == hl )
			br.Col( 0.1f, 0.5, 0.9f, 0.7f );
		else
			br.Col( 0.1f, 0.5, 0.9f, 0.25f );
		
		float q = 0.2f;
		Vec3 P = ent->Pos();
		br.Pos( P - V3(q,0,0) ).Pos( P + V3(0,0,q) ).Prev(0).Pos( P + V3(q,0,0) ).Prev(0).Pos( P - V3(0,0,q) ).Prev(0).Prev(6);
		br.Pos( P - V3(0,q,0) ).Pos( P + V3(0,0,q) ).Prev(0).Pos( P + V3(0,q,0) ).Prev(0).Pos( P - V3(0,0,q) ).Prev(0).Prev(6);
		br.Pos( P - V3(q,0,0) ).Pos( P + V3(0,q,0) ).Prev(0).Pos( P + V3(q,0,0) ).Prev(0).Pos( P - V3(0,q,0) ).Prev(0).Prev(6);
	}
	
	Mat4& iv = g_EdScene->camera.mInvView;
	Vec3 axes[2] = { iv.TransformNormal( V3(1,0,0) ), iv.TransformNormal( V3(0,1,0) ) };
	
	br.Col( 1 );
	for( size_t i = 0; i < m_entities.size(); ++i )
	{
		br.SetTexture( m_entities[ i ]->m_iconTex );
		br.Sprite( m_entities[ i ]->Pos(), axes[0]*0.1f, axes[1]*0.1f );
	}
	
	// debug draw highlighted/selected entities
	for( size_t i = 0; i < m_entities.size(); ++i )
	{
		EdEntity* ent = m_entities[ i ];
		if( ent->selected )
			m_entities[ i ]->DebugDraw();
	}
	if( hl && hl->selected == false && hl->m_type == ObjType_Entity )
		((EdEntity*)hl)->DebugDraw();
	
	br.Flush();
}


static bool ObjInArray( EdObject* obj, EdObject** list )
{
	while( *list )
	{
		if( obj == *list )
			return true;
		list++;
	}
	return false;
}
template< typename T > bool _RayIntersect( T& item, const Vec3& pos, const Vec3& dir, float outdst[1] )
{
	return item->RayIntersect( pos, dir, outdst );
}
int Obj2Mask( EdObject* obj )
{
	if( obj->m_type == ObjType_Block ) return SelMask_Blocks;
	if( obj->m_type == ObjType_Patch ) return SelMask_Patches;
	if( obj->m_type == ObjType_Entity ) return SelMask_Entities;
	return 0;
}
//bool _RayIntersect( EdBlock& B, const Vec3& pos, const Vec3& dir, float outdst[1] ){ return B.RayIntersect( pos, dir, outdst ); }
template< class T > bool RayItemsIntersect( T& items,
	const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outitem[1], EdObject** skip, int mask )
{
	float ndst[1], mindst = FLT_MAX;
	int curblk = -1;
	if( searchfrom < 0 )
		searchfrom = items.size();
	for( int i = searchfrom - 1; i >= 0; --i )
	{
		if( skip && ObjInArray( items[ i ], skip ) )
			continue;
		if( ( Obj2Mask( items[ i ] ) & mask ) == 0 )
			continue;
		if( _RayIntersect( items[ i ], pos, dir, ndst ) && ndst[0] < mindst )
		{
			curblk = i;
			mindst = ndst[0];
		}
	}
	for( int i = items.size() - 1; i >= searchfrom; --i )
	{
		if( skip && ObjInArray( items[ i ], skip ) )
			continue;
		if( ( Obj2Mask( items[ i ] ) & mask ) == 0 )
			continue;
		if( _RayIntersect( items[ i ], pos, dir, ndst ) && ndst[0] < mindst )
		{
			curblk = i;
			mindst = ndst[0];
		}
	}
	if( outdst ) outdst[0] = mindst;
	if( outitem ) outitem[0] = curblk;
	return curblk != -1;
}

bool EdWorld::RayObjectsIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outobj[1], EdObject** skip, int mask )
{
	return RayItemsIntersect( m_objects, pos, dir, searchfrom, outdst, outobj, skip, mask );
}

bool EdWorld::RayBlocksIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outblock[1], EdObject** skip, int mask )
{
	return RayItemsIntersect( m_blocks, pos, dir, searchfrom, outdst, outblock, skip, mask );
}

bool EdWorld::RayEntitiesIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outent[1], EdObject** skip, int mask )
{
	return RayItemsIntersect( m_entities, pos, dir, searchfrom, outdst, outent, skip, mask );
}

bool EdWorld::RayPatchesIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outent[1], EdObject** skip, int mask )
{
	return RayItemsIntersect( m_patches, pos, dir, searchfrom, outdst, outent, skip, mask );
}

EdEntity* EdWorld::CreateScriptedEntity( const StringView& name, sgsVariable params )
{
	EdEntity* e = ENT_FindProtoByName( StackString< 128 >( name ) );
	if( e == NULL )
		return NULL;
	
	e = e->CloneEntity();
	if( e == NULL )
		return NULL;
	
	if( e->IsScriptedEnt() == false )
	{
		delete e;
		return NULL;
	}
	
	SGRX_CAST( EdEntScripted*, es, e );
	es->m_data = params;
	es->Data2Fields();
	es->Fields2Data();
	return es;
}

void EdWorld::AddObject( EdObject* obj )
{
	m_objects.push_back( obj );
	if( obj->m_type == ObjType_Block )
		m_blocks.push_back( (EdBlock*) obj );
	if( obj->m_type == ObjType_Entity )
		m_entities.push_back( (EdEntity*) obj );
	if( obj->m_type == ObjType_Patch )
		m_patches.push_back( (EdPatch*) obj );
	obj->RegenerateMesh();
}

void EdWorld::DeleteObject( EdObject* obj )
{
	obj->Acquire();
	size_t at = m_objects.find_first_at( obj );
	m_objects.uerase( at );
	
	if( g_UIFrame )
	{
		EDGUIEvent e = { EDGUI_EVENT_DELOBJECT, NULL };
		e.key.key = at;
		g_UIFrame->ViewEvent( &e );
	}
	
	if( obj->m_type == ObjType_Block )
	{
		at = m_blocks.find_first_at( (EdBlock*) obj );
		if( at != NOT_FOUND )
			m_blocks.uerase( at );
	}
	
	if( obj->m_type == ObjType_Patch )
	{
		at = m_patches.find_first_at( (EdPatch*) obj );
		if( at != NOT_FOUND )
			m_patches.uerase( at );
	}
	
	if( obj->m_type == ObjType_Entity )
	{
		SGRX_CAST( EdEntity*, E, obj );
		E->BeforeDelete();
		at = m_entities.find_first_at( E );
		if( at != NOT_FOUND )
			m_entities.uerase( at );
	}
	obj->Release();
}

void EdWorld::DeleteSelectedObjects()
{
	size_t i = m_objects.size();
	while( i > 0 )
	{
		i--;
		if( m_objects[ i ]->selected )
		{
			m_objects.uerase( i );
			if( g_UIFrame )
			{
				EDGUIEvent e = { EDGUI_EVENT_DELOBJECT, NULL };
				e.key.key = i;
				g_UIFrame->ViewEvent( &e );
			}
		}
	}
	
	i = m_blocks.size();
	while( i > 0 )
	{
		i--;
		if( m_blocks[ i ]->selected )
			m_blocks.uerase( i );
	}
	
	i = m_patches.size();
	while( i > 0 )
	{
		i--;
		if( m_patches[ i ]->selected )
			m_patches.uerase( i );
	}
	
	i = m_entities.size();
	while( i > 0 )
	{
		i--;
		if( m_entities[ i ]->selected )
		{
			((EdEntity*)m_entities[ i ])->BeforeDelete();
			m_entities.uerase( i );
		}
	}
}

bool EdWorld::DuplicateSelectedObjectsAndMoveSelection()
{
	size_t sz = m_objects.size();
	for( size_t i = 0; i < sz; ++i )
	{
		if( m_objects[ i ]->selected )
		{
			EdObject* obj = m_objects[ i ]->Clone();
			m_objects[ i ]->selected = false;
			AddObject( obj );
		}
	}
	return m_objects.size() != sz;
}

int EdWorld::GetNumSelectedObjects()
{
	// no specific perf requirements currently
	int ns = 0;
	for( size_t i = 0; i < m_objects.size(); ++i )
		if( m_objects[ i ]->selected )
			ns++;
	return ns;
}

int EdWorld::GetOnlySelectedObject()
{
	int sb = -1;
	int ns = 0;
	for( size_t i = 0; i < m_objects.size(); ++i )
	{
		if( m_objects[ i ]->selected )
		{
			if( ns++ )
				return -1;
			sb = i;
		}
	}
	return sb;
}

bool EdWorld::GetSelectedObjectAABB( Vec3 outaabb[2] )
{
	bool ret = false;
	outaabb[0] = V3(FLT_MAX);
	outaabb[1] = V3(-FLT_MAX);
	for( size_t i = 0; i < m_objects.size(); ++i )
	{
		EdObject* obj = m_objects[ i ];
		if( obj->selected == false )
			continue;
		Mat4 gwm = m_groupMgr.GetMatrix( obj->group );
		for( int v = 0; v < obj->GetNumVerts(); ++v )
		{
			Vec3 p = gwm.TransformPos( obj->GetLocalVertex( v ) );
			outaabb[0] = Vec3::Min( outaabb[0], p );
			outaabb[1] = Vec3::Max( outaabb[1], p );
		}
		ret = obj->GetNumVerts() != 0;
	}
	return ret;
}

void EdWorld::SelectObject( int oid, bool mod )
{
	if( mod )
	{
		if( oid != -1 )
			m_objects[ oid ]->selected = !m_objects[ oid ]->selected;
	}
	else
	{
		for( size_t i = 0; i < m_objects.size(); ++i )
			m_objects[ i ]->selected = false;
		if( oid != -1 )
			m_objects[ oid ]->selected = true;
	}
}

Vec3 EdWorld::FindCenterOfGroup( int32_t grp )
{
	Vec3 cp = V3(0);
	int count = 0;
	for( size_t i = 0; i < m_objects.size(); ++i )
	{
		if( m_objects[ i ]->group == grp )
		{
			cp += m_objects[ i ]->FindCenter();
			count++;
		}
	}
	if( count )
		cp /= count;
	return cp;
}

void EdWorld::FixTransformsOfGroup( int32_t grp )
{
	for( size_t i = 0; i < m_objects.size(); ++i )
	{
		if( m_objects[ i ]->group == grp )
			m_objects[ i ]->RegenerateMesh();
	}
}

void EdWorld::CopyObjectsToGroup( int32_t grpfrom, int32_t grpto )
{
	size_t oldsize = m_objects.size();
	for( size_t i = 0; i < oldsize; ++i )
	{
		if( m_objects[ i ]->group == grpfrom )
		{
			EdObject* obj = m_objects[ i ]->Clone();
			obj->group = grpto;
			obj->RegenerateMesh();
			m_objects.push_back( obj );
		}
	}
}

void EdWorld::TransferObjectsToGroup( int32_t grpfrom, int32_t grpto )
{
	for( size_t i = 0; i < m_objects.size(); ++i )
	{
		if( m_objects[ i ]->group == grpfrom )
		{
			m_objects[ i ]->group = grpto;
			m_objects[ i ]->RegenerateMesh();
		}
	}
}

void EdWorld::DeleteObjectsInGroup( int32_t grp )
{
	for( size_t i = 0; i < m_objects.size(); ++i )
	{
		if( m_objects[ i ]->group == grp )
		{
			m_objects.uerase( i-- );
		}
	}
}

void EdWorld::ExportGroupAsOBJ( int32_t grp, const StringView& name )
{
	OBJExporter objex;
	for( size_t i = 0; i < m_objects.size(); ++i )
	{
		if( m_objects[ i ]->group == grp )
		{
			m_objects[ i ]->Export( objex );
		}
	}
	objex.Save( name, "Exported from SGRX editor" );
}



static int edscrAddScriptedEntity( SGS_CTX )
{
	SGSFN( "AddScriptedEntity" );
	StringView name = sgs_GetVar<StringView>()( C, 0 );
	sgsVariable params = sgs_GetVar<sgsVariable>()( C, 1 );
	if( name == "Mesh" || name == "Light" || name == "Light sample" )
		return sgs_Msg( C, SGS_WARNING, "Requested entity is not scripted: %s", StackString<128>(name).str );
	EdObject* E = g_EdWorld->CreateScriptedEntity( name, params );
	if( E == NULL )
		return sgs_Msg( C, SGS_WARNING, "Could not find entity: %s", StackString<128>(name).str );
	g_EdWorld->AddObject( E );
	return 0;
}

static sgs_RegFuncConst g_editor_rfc[] =
{
	{ "AddScriptedEntity", edscrAddScriptedEntity },
	{ NULL, NULL },
};



EDGUIMultiObjectProps::EDGUIMultiObjectProps() :
	m_group( true, "Multiple objects" ),
	m_tex( g_UISurfTexPicker, "" ),
	m_selsurf( false )
{
	m_tex.caption = "Texture";
	m_group.Add( &m_tex );
	Add( &m_group );
}

void EDGUIMultiObjectProps::Prepare( bool selsurf )
{
	m_selsurf = selsurf;
	String tex;
	for( size_t i = 0; i < g_EdWorld->m_objects.size(); ++i )
	{
		EdObject* obj = g_EdWorld->m_objects[ i ];
		if( obj->selected == false )
			continue;
		if( obj->m_type == ObjType_Block )
		{
			SGRX_CAST( EdBlock*, B, obj );
			for( size_t s = 0; s < B->surfaces.size(); ++s )
			{
				if( selsurf && B->IsSurfaceSelected( s ) == false )
					continue;
				StringView tt = B->surfaces[ s ].texname;
				if( tt && tex != tt )
				{
					if( tex.size() )
					{
						m_tex.SetValue( "" );
						return;
					}
					tex = tt;
				}
			}
		}
		else if( obj->m_type == ObjType_Patch )
		{
			SGRX_CAST( EdPatch*, P, obj );
			StringView tt = P->layers[0].texname;
			if( tt && tex != tt )
			{
				if( tex.size() )
				{
					m_tex.SetValue( "" );
					return;
				}
				tex = tt;
			}
		}
	}
	m_tex.SetValue( tex );
}

int EDGUIMultiObjectProps::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_tex )
		{
			for( size_t i = 0; i < g_EdWorld->m_objects.size(); ++i )
			{
				EdObject* obj = g_EdWorld->m_objects[ i ];
				if( obj->selected == false )
					continue;
				if( obj->m_type == ObjType_Block )
				{
					SGRX_CAST( EdBlock*, B, obj );
					for( size_t s = 0; s < B->surfaces.size(); ++s )
					{
						if( m_selsurf && B->IsSurfaceSelected( s ) == false )
							continue;
						B->surfaces[ s ].texname = m_tex.m_value;
					}
					obj->RegenerateMesh();
				}
				else if( obj->m_type == ObjType_Patch )
				{
					SGRX_CAST( EdPatch*, P, obj );
					P->layers[0].texname = m_tex.m_value;
					obj->RegenerateMesh();
				}
			}
			g_UIFrame->SetEditMode( g_UIFrame->m_editMode );
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}



EDGUIMainFrame::EDGUIMainFrame() :
	m_editTF( NULL ),
	m_editMode( NULL ),
	m_keyMod( 0 ),
	m_UIMenuSplit( true, 26, 0 ),
	m_UIMenuLRSplit( false, 0, 0.4f ),
	m_UIParamSplit( false, 0, 0.7f ),
	m_UIRenderView( g_EdScene, this )
{
	tyname = "mainframe";
	
	Add( &m_UIMenuSplit );
	m_UIMenuLRSplit.SetFirstPane( &m_UIMenuButtonsLft );
	m_UIMenuLRSplit.SetSecondPane( &m_UIMenuButtonsRgt );
	m_UIMenuSplit.SetFirstPane( &m_UIMenuLRSplit );
	m_UIMenuSplit.SetSecondPane( &m_UIParamSplit );
	m_UIParamSplit.SetFirstPane( &m_UIRenderView );
	m_UIParamSplit.SetSecondPane( &m_UIParamScroll );
	m_UIParamScroll.Add( &m_UIParamList );
	
	// menu
	m_MB_Cat0.caption = "File:";
	m_MBNew.caption = "New";
	m_MBOpen.caption = "Open";
	m_MBSave.caption = "Save";
	m_MBSaveAs.caption = "Save As";
	m_MBCompile.caption = "Compile";
	m_MB_Cat1.caption = "Edit:";
	m_MBDrawBlock.caption = "Draw Block";
	m_MBEditObjects.caption = "Edit Objects";
	m_MBPaintSurfs.caption = "Paint Surfaces";
	m_MBAddEntity.caption = "Add Entity";
	m_MBEditGroups.caption = "Edit groups";
	m_MBLevelInfo.caption = "Level Info";
	m_UIMenuButtonsLft.Add( &m_MB_Cat0 );
	m_UIMenuButtonsLft.Add( &m_MBNew );
	m_UIMenuButtonsLft.Add( &m_MBOpen );
	m_UIMenuButtonsLft.Add( &m_MBSave );
	m_UIMenuButtonsLft.Add( &m_MBSaveAs );
	m_UIMenuButtonsLft.Add( &m_MBCompile );
	m_UIMenuButtonsRgt.Add( &m_MB_Cat1 );
	m_UIMenuButtonsRgt.Add( &m_MBDrawBlock );
	m_UIMenuButtonsRgt.Add( &m_MBEditObjects );
	m_UIMenuButtonsRgt.Add( &m_MBPaintSurfs );
	m_UIMenuButtonsRgt.Add( &m_MBAddEntity );
	m_UIMenuButtonsRgt.Add( &m_MBEditGroups );
	m_UIMenuButtonsRgt.Add( &m_MBLevelInfo );
	
	m_txMarker = GR_GetTexture( "editor/marker.png" );
}

void EDGUIMainFrame::PostInit()
{
	SetEditMode( &m_emEditLevel );
}

int EDGUIMainFrame::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_BTNCLICK:
		if(0);
		
		else if( e->target == &m_MBNew ) Level_New();
		else if( e->target == &m_MBOpen ) Level_Open();
		else if( e->target == &m_MBSave ) Level_Save();
		else if( e->target == &m_MBSaveAs ) Level_SaveAs();
		else if( e->target == &m_MBCompile ) Level_Compile();
		
		else if( e->target == &m_MBDrawBlock ) SetEditMode( &m_emDrawBlock );
		else if( e->target == &m_MBEditObjects ) SetEditMode( &m_emEditObjs );
		else if( e->target == &m_MBPaintSurfs ) SetEditMode( &m_emPaintSurfs );
		else if( e->target == &m_MBAddEntity ) SetEditMode( &m_emAddEntity );
		else if( e->target == &m_MBEditGroups ) SetEditMode( &m_emEditGroup );
		else if( e->target == &m_MBLevelInfo ) SetEditMode( &m_emEditLevel );
		
		return 1;
		
	case EDGUI_EVENT_PROPCHANGE:
		if( e->target == g_UILevelOpenPicker )
		{
			Level_Real_Open( g_UILevelOpenPicker->GetValue() );
		}
		if( e->target == g_UILevelSavePicker )
		{
			Level_Real_Save( g_UILevelSavePicker->GetValue() );
		}
		return 1;
	}
	if( m_editMode )
	{
		if( m_editMode->OnUIEvent( e ) )
			return 1;
	}
	return EDGUIFrame::OnEvent( e );
}

bool EDGUIMainFrame::ViewEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_KEYDOWN || e->type == EDGUI_EVENT_KEYUP )
	{
		int mod = 0;
		switch( e->key.engkey )
		{
		case SDLK_LSHIFT: mod = KMOD_LSHIFT; break;
		case SDLK_RSHIFT: mod = KMOD_RSHIFT; break;
		case SDLK_LCTRL: mod = KMOD_LCTRL; break;
		case SDLK_RCTRL: mod = KMOD_RCTRL; break;
		case SDLK_LALT: mod = KMOD_LALT; break;
		case SDLK_RALT: mod = KMOD_RALT; break;
		case SDLK_LGUI: mod = KMOD_LGUI; break;
		case SDLK_RGUI: mod = KMOD_RGUI; break;
		}
		if( e->type == EDGUI_EVENT_KEYDOWN )
			m_keyMod |= mod;
		else
			m_keyMod &= ~mod;
	}
	
	if( m_editTF )
	{
		if( m_editTF->OnViewEvent( e ) )
			return false;
	}
	
	if( m_editMode )
		m_editMode->OnViewEvent( e );
	
	return true;
}

void EDGUIMainFrame::_DrawCursor( bool drawimg, float height )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.UnsetTexture();
	if( IsCursorAiming() )
	{
		Vec2 pos = GetCursorPlanePos();
		if( drawimg )
		{
			br.SetTexture( m_txMarker ).Col( 0.9f, 0.1f, 0, 0.9f );
			br.Box( pos.x, pos.y, 1, 1, height );
		}
		br.UnsetTexture().SetPrimitiveType( PT_Lines );
		// up
		br.Col( 0, 0.1f, 0.9f, 0.9f ).Pos( pos.x, pos.y, height );
		br.Col( 0, 0.1f, 0.9f, 0.0f ).Pos( pos.x, pos.y, 4 + height );
		// -X
		br.Col( 0.5f, 0.1f, 0, 0.9f ).Pos( pos.x, pos.y, height );
		br.Col( 0.5f, 0.1f, 0, 0.0f ).Pos( pos.x - 4, pos.y, height );
		// +X
		br.Col( 0.9f, 0.1f, 0, 0.9f ).Pos( pos.x, pos.y, height );
		br.Col( 0.9f, 0.1f, 0, 0.0f ).Pos( pos.x + 4, pos.y, height );
		// -Y
		br.Col( 0.1f, 0.5f, 0, 0.9f ).Pos( pos.x, pos.y, height );
		br.Col( 0.1f, 0.5f, 0, 0.0f ).Pos( pos.x, pos.y - 4, height );
		// +Y
		br.Col( 0.1f, 0.9f, 0, 0.9f ).Pos( pos.x, pos.y, height );
		br.Col( 0.1f, 0.9f, 0, 0.0f ).Pos( pos.x, pos.y + 4, height );
	}
}

void EDGUIMainFrame::DrawCursor( bool drawimg )
{
	_DrawCursor( drawimg, 0 );
	if( m_UIRenderView.crplaneheight )
		_DrawCursor( drawimg, m_UIRenderView.crplaneheight );
}

void EDGUIMainFrame::DebugDraw()
{
	if( m_editMode )
		m_editMode->Draw();
	if( m_editTF )
		m_editTF->Draw();
}


void EDGUIMainFrame::AddToParamList( EDGUIItem* item )
{
	m_UIParamList.Add( item );
}

void EDGUIMainFrame::ClearParamList()
{
	while( m_UIParamList.m_subitems.size() )
		m_UIParamList.Remove( m_UIParamList.m_subitems.last() );
}

void EDGUIMainFrame::RefreshMouse()
{
	m_frame->_HandleMouseMove( false );
}

Vec3 EDGUIMainFrame::GetCursorRayPos()
{
	return m_UIRenderView.crpos;
}

Vec3 EDGUIMainFrame::GetCursorRayDir()
{
	return m_UIRenderView.crdir;
}

Vec2 EDGUIMainFrame::GetCursorPlanePos()
{
	return Snapped( m_UIRenderView.cursor_hpos );
}

float EDGUIMainFrame::GetCursorPlaneHeight()
{
	return m_UIRenderView.crplaneheight;
}

void EDGUIMainFrame::SetCursorPlaneHeight( float z )
{
	m_UIRenderView.crplaneheight = z;
}

bool EDGUIMainFrame::IsCursorAiming()
{
	return m_UIRenderView.cursor_aim;
}

void EDGUIMainFrame::Snap( Vec2& v )
{
	if( m_keyMod & KMOD_ALT )
		return;
	m_snapProps.Snap( v );
}

void EDGUIMainFrame::Snap( Vec3& v )
{
	if( m_keyMod & KMOD_ALT )
		return;
	m_snapProps.Snap( v );
}

Vec2 EDGUIMainFrame::Snapped( const Vec2& v )
{
	Vec2 o = v;
	Snap( o );
	return o;
}

Vec3 EDGUIMainFrame::Snapped( const Vec3& v )
{
	Vec3 o = v;
	Snap( o );
	return o;
}


void EDGUIMainFrame::ResetEditorState()
{
	PostInit();
}

void EDGUIMainFrame::Level_New()
{
	g_UIScrFnPicker->m_levelName = m_fileName = "";
	g_EdWorld->Reset();
	ResetEditorState();
}

void EDGUIMainFrame::Level_Open()
{
	g_UILevelOpenPicker->Reload();
	g_UILevelOpenPicker->Open( this, "" );
	m_frame->Add( g_UILevelOpenPicker );
}

void EDGUIMainFrame::Level_Save()
{
	if( m_fileName.size() )
	{
		Level_Real_Save( m_fileName );
	}
	else
	{
		Level_SaveAs();
	}
}

void EDGUIMainFrame::Level_SaveAs()
{
	g_UILevelSavePicker->Reload();
	g_UILevelSavePicker->Open( this, "" );
	m_frame->Add( g_UILevelSavePicker );
}

void EDGUIMainFrame::Level_Compile()
{
	if( m_fileName.size() )
	{
		Level_Real_Compile();
	}
	else
	{
		Level_SaveAs();
	}
}

void EDGUIMainFrame::Level_Real_Open( const String& str )
{
	LOG << "Trying to open level: " << str;
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, sizeof(bfr), "levels/%.*s.tle", TMIN( (int) str.size(), 200 ), str.data() );
	String data;
	if( !FS_LoadTextFile( bfr, data ) )
	{
		LOG_ERROR << "FAILED TO LOAD LEVEL FILE: " << bfr;
		return;
	}
	
	ResetEditorState();
	
	TextReader tr( &data );
	g_EdWorld->Serialize( tr );
	if( tr.error )
	{
		LOG_ERROR << "FAILED TO READ LEVEL FILE (at " << (int) tr.pos << "): " << bfr;
		return;
	}
	
	g_UIScrFnPicker->m_levelName = m_fileName = str;
}

void EDGUIMainFrame::Level_Real_Save( const String& str )
{
	LOG << "Trying to save level: " << str;
	String data;
	TextWriter arch( &data );
	
	arch << *g_EdWorld;
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, sizeof(bfr), "levels/%.*s.tle", TMIN( (int) str.size(), 200 ), str.data() );
	if( !FS_SaveTextFile( bfr, data ) )
	{
		LOG_ERROR << "FAILED TO SAVE LEVEL FILE: " << bfr;
		return;
	}
	
	g_UIScrFnPicker->m_levelName = m_fileName = str;
}

void EDGUIMainFrame::Level_Real_Compile()
{
	LOG << "Compiling level";
	LevelCache lcache;
	
	lcache.AmbientColor = HSV( g_EdWorld->m_ctlAmbientColor.m_value );
	lcache.LightmapClearColor = HSV( g_EdWorld->m_ctlLightmapClearColor.m_value );
	lcache.RADNumBounces = g_EdWorld->m_ctlRADNumBounces.m_value;
	lcache.LightmapDetail = g_EdWorld->m_ctlLightmapDetail.m_value;
	lcache.LightmapBlurSize = g_EdWorld->m_ctlLightmapBlurSize.m_value;
	lcache.AODistance = g_EdWorld->m_ctlAODistance.m_value;
	lcache.AOMultiplier = g_EdWorld->m_ctlAOMultiplier.m_value;
	lcache.AOFalloff = g_EdWorld->m_ctlAOFalloff.m_value;
	lcache.AOEffect = g_EdWorld->m_ctlAOEffect.m_value;
//	lcache.AODivergence = g_EdWorld->m_ctlAODivergence.m_value;
	lcache.AOColor = HSV( g_EdWorld->m_ctlAOColor.m_value );
	lcache.AONumSamples = g_EdWorld->m_ctlAONumSamples.m_value;
	
	// DIRECTIONAL LIGHT
	if( g_EdWorld->m_ctlDirLightColor.m_value.z && g_EdWorld->m_ctlDirLightNumSamples.m_value )
	{
		LC_Light L;
		L.type = LM_LIGHT_DIRECT;
		L.range = 1024;
		Vec2 dir = g_EdWorld->m_ctlDirLightDir.m_value;
		L.dir = -V3( dir.x, dir.y, -1 ).Normalized();
		L.color = HSV( g_EdWorld->m_ctlDirLightColor.m_value );
		L.light_radius = g_EdWorld->m_ctlDirLightDivergence.m_value / 180.0f;
		L.num_shadow_samples = g_EdWorld->m_ctlDirLightNumSamples.m_value;
		lcache.AddLight( L );
	}
	
	for( size_t i = 0; i < g_EdWorld->m_blocks.size(); ++i )
		g_EdWorld->m_blocks[ i ]->GenerateMesh( lcache );
	
	for( size_t i = 0; i < g_EdWorld->m_patches.size(); ++i )
		g_EdWorld->m_patches[ i ]->GenerateMesh( lcache );
	
	for( size_t i = 0; i < g_EdWorld->m_entities.size(); ++i )
		g_EdWorld->m_entities[ i ]->UpdateCache( lcache );
	
	lcache.GenerateSamples( 1.0f );
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, sizeof(bfr), "levels/%.*s", TMIN( (int) m_fileName.size(), 200 ), m_fileName.data() );
	
	if( !lcache.SaveCache( bfr ) )
		LOG_ERROR << "FAILED TO SAVE CACHE";
	else
		LOG << "Level is compiled";
}

void EDGUIMainFrame::SetEditMode( EdEditMode* em )
{
	if( m_editMode )
		m_editMode->OnExit();
	m_editMode = em;
	ClearParamList();
	em->OnEnter();
}

void EDGUIMainFrame::SetEditTransform( EdEditTransform* et )
{
	if( m_editTF )
	{
		m_editTF->OnExit();
		if( m_editMode )
			m_editMode->OnTransformEnd();
	}
	m_editTF = et && et->OnEnter() ? et : NULL;
}

void EDGUIMainFrame::SetModeHighlight( EDGUIButton* mybtn )
{
	m_MBDrawBlock.SetHighlight( mybtn == &m_MBDrawBlock );
	m_MBEditObjects.SetHighlight( mybtn == &m_MBEditObjects );
	m_MBPaintSurfs.SetHighlight( mybtn == &m_MBPaintSurfs );
	m_MBAddEntity.SetHighlight( mybtn == &m_MBAddEntity );
	m_MBEditGroups.SetHighlight( mybtn == &m_MBEditGroups );
	m_MBLevelInfo.SetHighlight( mybtn == &m_MBLevelInfo );
}


//
// EDITOR ENTRY POINT
//

SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_DECALS | RPF_OBJ_STATIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_DECALS | RPF_OBJ_DYNAMIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_PROJECTORS, RPF_ENABLED, 1, 0, 0, "projector" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
};

struct TACStrikeEditor : IGame
{
	bool OnInitialize()
	{
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
		g_ScriptCtx = new ScriptContext;
		g_ScriptCtx->RegisterBatchRenderer();
		sgs_RegFuncConsts( g_ScriptCtx->C, g_ent_scripted_rfc, -1 );
		sgs_RegFuncConsts( g_ScriptCtx->C, g_editor_rfc, -1 );
		ScrItem_InstallAPI( g_ScriptCtx->C );
		
		LOG << "\nLoading scripted entities:";
		LOG << g_ScriptCtx->ExecFile( "editor/entities.sgs" );
		LOG << "\nLoading scripted items:";
		LOG << g_ScriptCtx->ExecFile( "data/scritems.sgs" );
		LOG << "\nLoading completed\n\n";
		
		g_UISurfTexPicker = new EDGUISDTexPicker;
		g_UIMeshPicker = new EDGUIMeshPicker;
		g_UICharPicker = new EDGUICharUsePicker;
		g_UIPartSysPicker = new EDGUIPartSysPicker;
		g_UISoundPicker = new EDGUISoundPicker;
		g_UIScrItemPicker = new EDGUIScrItemPicker( g_ScriptCtx );
		g_UIScrFnPicker = new EDGUIScrFnPicker( g_ScriptCtx );
		g_UILevelOpenPicker = new EDGUILevelOpenPicker;
		g_UILevelSavePicker = new EDGUILevelSavePicker;
		
		// core layout
		g_EdLGCont = new EdLevelGraphicsCont;
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = Vec3::Create(3,3,3);
		g_EdScene->camera.UpdateMatrices();
		g_EdPhyWorld = PHY_CreateWorld();
		g_EdWorld = new EdWorld();
		g_EdWorld->RegenerateMeshes();
		g_UIFrame = new EDGUIMainFrame();
		g_UIFrame->PostInit();
		g_UIFrame->Resize( GR_GetWidth(), GR_GetHeight() );
		
		// param area
		g_UIFrame->AddToParamList( g_EdWorld );
		
		return true;
	}
	void OnDestroy()
	{
		delete g_UILevelSavePicker;
		g_UILevelSavePicker = NULL;
		delete g_UILevelOpenPicker;
		g_UILevelOpenPicker = NULL;
		delete g_UIScrItemPicker;
		g_UIScrItemPicker = NULL;
		delete g_UIScrFnPicker;
		g_UIScrFnPicker = NULL;
		delete g_UIPartSysPicker;
		g_UIPartSysPicker = NULL;
		delete g_UISoundPicker;
		g_UISoundPicker = NULL;
		delete g_UICharPicker;
		g_UICharPicker = NULL;
		delete g_UIMeshPicker;
		g_UIMeshPicker = NULL;
		delete g_UISurfTexPicker;
		g_UISurfTexPicker = NULL;
		delete g_UIFrame;
		g_UIFrame = NULL;
		delete g_EdWorld;
		g_EdWorld = NULL;
		g_EdPhyWorld = NULL;
		g_EdScene = NULL;
		delete g_EdLGCont;
		g_EdLGCont = NULL;
		delete g_ScriptCtx;
		g_ScriptCtx = NULL;
	}
	void OnEvent( const Event& e )
	{
		g_UIFrame->EngineEvent( &e );
		g_EdWorld->m_groupMgr.ProcessDestroyQueue();
	}
	void OnTick( float dt, uint32_t gametime )
	{
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		g_UIFrame->m_UIRenderView.UpdateCamera( dt );
		g_UIFrame->Draw();
	}
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

