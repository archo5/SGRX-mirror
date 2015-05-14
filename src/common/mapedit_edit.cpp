

#include "mapedit.hpp"



Vec2 ED_GetCursorPos()
{
	EDGUIRenderView& rv = g_UIFrame->m_UIRenderView;
	Vec2 cp = Game_GetCursorPos() - V2( rv.x0, rv.y0 );
	return V2
	(
		cp.x / ( rv.x1 - rv.x0 ) * 2 - 1,
		cp.y / ( rv.y1 - rv.y0 ) * -2 + 1
	);
}

Vec2 ED_GetScreenPos( const Vec3& p )
{
	return g_EdScene->camera.mProj.TransformPos(
		g_EdScene->camera.mView.TransformPos( p ) ).ToVec2();
}

Vec2 ED_GetScreenDir( const Vec3& d )
{
	return g_EdScene->camera.mProj.TransformNormal(
		g_EdScene->camera.mView.TransformNormal( d ) ).ToVec2().Normalized();
}

Vec2 ED_MovePointOnLine( const Vec2& p, const Vec2& lo, const Vec2& ld )
{
	Vec2 normal = ld.Perp();
	return p + normal * ( Vec2Dot( lo, normal ) - Vec2Dot( p, normal ) );
}

Vec3 ED_RemapPos( const Vec3& p, Vec3 bbfrom[2], Vec3 bbto[2] )
{
	return TLERP( bbto[0], bbto[1], TREVLERP( bbfrom[0], bbfrom[1], p ) );
}


int EdBasicEditTransform::OnViewEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_MOUSEMOVE )
	{
		RecalcTransform();
		ApplyTransform();
	}
	if( e->type == EDGUI_EVENT_BTNCLICK )
	{
		if( e->mouse.button == EDGUI_MB_RIGHT )
			RestoreState();
		g_UIFrame->SetEditTransform( NULL );
	}
	return 1;
}

bool EdBlockEditTransform::OnEnter()
{
	if( g_EdWorld->GetNumSelectedObjects() == 0 )
		return false;
	m_cmode = Camera;
	return EdEditTransform::OnEnter();
}

int EdBlockEditTransform::OnViewEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_KEYDOWN )
	{
		switch( e->key.engkey )
		{
		case SDLK_x:
			if( m_cmode == XAxis ) m_cmode = XPlane;
			else if( m_cmode == XPlane ) m_cmode = Camera;
			else m_cmode = XAxis;
			break;
		case SDLK_y:
			if( m_cmode == YAxis ) m_cmode = YPlane;
			else if( m_cmode == YPlane ) m_cmode = Camera;
			else m_cmode = YAxis;
			break;
		case SDLK_z:
			if( m_cmode == ZAxis ) m_cmode = ZPlane;
			else if( m_cmode == ZPlane ) m_cmode = Camera;
			else m_cmode = ZAxis;
			break;
		}
		RecalcTransform();
		ApplyTransform();
	}
	return EdBasicEditTransform::OnViewEvent( e );
}

void EdBlockEditTransform::SaveState()
{
	Vec3 cp = V3(0);
	int cpc = 0;
	m_objStateMap.clear();
	m_objectStateData.clear();
	for( size_t i = 0; i < g_EdWorld->m_objects.size(); ++i )
	{
		EdObject* obj = g_EdWorld->m_objects[ i ];
		if( obj->selected == false )
			continue;
		
		SavedObject SO = { i, m_objectStateData.size() };
		m_objStateMap.push_back( SO );
		ByteWriter bw( &m_objectStateData );
		obj->Serialize( bw );
		
		cp += obj->FindCenter();
		cpc++;
	}
	m_origin = g_UIFrame->Snapped( cp / cpc );
}

void EdBlockEditTransform::RestoreState()
{
	for( size_t i = 0; i < m_objStateMap.size(); ++i )
	{
		const SavedObject& SO = m_objStateMap[ i ];
		ByteReader br( &m_objectStateData, SO.offset );
		g_EdWorld->m_objects[ SO.id ]->Serialize( br );
		g_EdWorld->m_objects[ SO.id ]->RegenerateMesh();
	}
}

Vec3 EdBlockEditTransform::GetMovementVector( const Vec2& a, const Vec2& b )
{
	Vec3 axis = V3(0);
	Vec3 mult = V3(1);
	switch( m_cmode )
	{
	case Camera: axis = g_EdScene->camera.direction.Normalized(); break;
	case XAxis: mult = axis = V3(1,0,0); break;
	case XPlane: axis = V3(1,0,0); mult = V3(0,1,1); break;
	case YAxis: mult = axis = V3(0,1,0); break;
	case YPlane: axis = V3(0,1,0); mult = V3(1,0,1); break;
	case ZAxis: mult = axis = V3(0,0,1); break;
	case ZPlane: axis = V3(0,0,1); mult = V3(1,1,0); break;
	}
	
	// generate a special plane
	if( m_cmode == XAxis || m_cmode == YAxis || m_cmode == ZAxis )
	{
		axis = Vec3Cross( axis, Vec3Cross( axis, g_EdScene->camera.direction ) ).Normalized();
	}
	
	Vec2 sso = ED_GetScreenPos( m_origin );
	Vec2 ra = sso;
	Vec2 rb = sso - a + b;
	Vec4 plane = V4( axis, Vec3Dot( axis, m_origin ) );
	Vec3 ap, ad, bp, bd;
	float aid[2], bid[2];
	if( g_EdScene->camera.GetCursorRay( ra.x * 0.5f + 0.5f, ra.y * -0.5f + 0.5f, ap, ad ) &&
		g_EdScene->camera.GetCursorRay( rb.x * 0.5f + 0.5f, rb.y * -0.5f + 0.5f, bp, bd ) &&
		RayPlaneIntersect( ap, ad, plane, aid ) &&
		RayPlaneIntersect( bp, bd, plane, bid ) )
	{
		return ( bp + bd * bid[0] - ( ap + ad * aid[0] ) ) * mult + V3(0); // 0 added to prevent "-0"
	}
	
	return V3(0);
}

int EdBlockMoveTransform::OnViewEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_PAINT )
	{
		int x0 = g_UIFrame->m_UIRenderView.x0;
		int y1 = g_UIFrame->m_UIRenderView.y1;
		char bfr[ 1024 ];
		if( m_extend )
		{
			// TODO: more detail?
			sgrx_snprintf( bfr, 1024, "Extending blocks: %g ; %g ; %g", m_transform.x, m_transform.y, m_transform.z );
		}
		else
		{
			sgrx_snprintf( bfr, 1024, "Moving blocks: %g ; %g ; %g", m_transform.x, m_transform.y, m_transform.z );
		}
		GR2D_SetColor( 1, 1 );
		GR2D_DrawTextLine( x0, y1, bfr, HALIGN_LEFT, VALIGN_BOTTOM );
	}
	return EdBlockEditTransform::OnViewEvent( e );
}

void EdBlockMoveTransform::Draw()
{
	float D = 16;
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.SetPrimitiveType( PT_Lines );
	if( m_cmode == XAxis || m_cmode == YPlane || m_cmode == ZPlane )
	{
		br.Col(1,0,0,0).Pos( m_origin - V3(D,0,0) );
		br.Col(1,0,0,0.5f).Pos( m_origin ).Prev(0);
		br.Col(1,0,0,0).Pos( m_origin + V3(D,0,0) );
		br.Col(1,0,0,0).Pos( m_origin + m_transform - V3(D,0,0) );
		br.Col(1,0,0,1).Pos( m_origin + m_transform ).Prev(0);
		br.Col(1,0,0,0).Pos( m_origin + m_transform + V3(D,0,0) );
	}
	if( m_cmode == YAxis || m_cmode == XPlane || m_cmode == ZPlane )
	{
		br.Col(0,1,0,0).Pos( m_origin - V3(0,D,0) );
		br.Col(0,1,0,0.5f).Pos( m_origin ).Prev(0);
		br.Col(0,1,0,0).Pos( m_origin + V3(0,D,0) );
		br.Col(0,1,0,0).Pos( m_origin + m_transform - V3(0,D,0) );
		br.Col(0,1,0,1).Pos( m_origin + m_transform ).Prev(0);
		br.Col(0,1,0,0).Pos( m_origin + m_transform + V3(0,D,0) );
	}
	if( m_cmode == ZAxis || m_cmode == XPlane || m_cmode == YPlane )
	{
		br.Col(0,0,1,0).Pos( m_origin - V3(0,0,D) );
		br.Col(0,0,1,0.5f).Pos( m_origin ).Prev(0);
		br.Col(0,0,1,0).Pos( m_origin + V3(0,0,D) );
		br.Col(0,0,1,0).Pos( m_origin + m_transform - V3(0,0,D) );
		br.Col(0,0,1,1).Pos( m_origin + m_transform ).Prev(0);
		br.Col(0,0,1,0).Pos( m_origin + m_transform + V3(0,0,D) );
	}
}

void EdBlockMoveTransform::ApplyTransform()
{
	for( size_t i = 0; i < m_objStateMap.size(); ++i )
	{
		const SavedObject& SO = m_objStateMap[ i ];
		EdObject* obj = g_EdWorld->m_objects[ SO.id ];
		ByteReader br( &m_objectStateData, SO.offset );
		obj->Serialize( br );
		if( m_extend )
		{
			Vec3 dstbb[2] = { m_xtdAABB[0], m_xtdAABB[1] };
			if( m_xtdMask.x == 0 ) dstbb[0].x += m_transform.x; else if( m_xtdMask.x == 1 ) dstbb[1].x += m_transform.x;
			if( m_xtdMask.y == 0 ) dstbb[0].y += m_transform.y; else if( m_xtdMask.y == 1 ) dstbb[1].y += m_transform.y;
			if( m_xtdMask.z == 0 ) dstbb[0].z += m_transform.z; else if( m_xtdMask.z == 1 ) dstbb[1].z += m_transform.z;
			obj->SetPosition( ED_RemapPos( obj->GetPosition(), m_xtdAABB, dstbb ) );
			obj->ScaleVertices( TREVLERP( V3(0), m_xtdAABB[1] - m_xtdAABB[0], dstbb[1] - dstbb[0] ) );
		}
		else
		{
			// simple translation only
			obj->SetPosition( obj->GetPosition() + m_transform );
		}
		obj->RegenerateMesh();
	}
}

void EdBlockMoveTransform::RecalcTransform()
{
	m_transform = g_UIFrame->Snapped( GetMovementVector( m_startCursorPos, ED_GetCursorPos() ) );
}


int EdBlockVertexMoveTransform::OnViewEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_PAINT )
	{
		int x0 = g_UIFrame->m_UIRenderView.x0;
		int y1 = g_UIFrame->m_UIRenderView.y1;
		char bfr[ 1024 ];
		sgrx_snprintf( bfr, 1024, "Moving vertices: %g ; %g ; %g", m_transform.x, m_transform.y, m_transform.z );
		GR2D_SetColor( 1, 1 );
		GR2D_DrawTextLine( x0, y1, bfr, HALIGN_LEFT, VALIGN_BOTTOM );
		return 1;
	}
	return EdBlockMoveTransform::OnViewEvent( e );
}

void EdBlockVertexMoveTransform::ApplyTransform()
{
	for( size_t i = 0; i < m_objStateMap.size(); ++i )
	{
		const SavedObject& SO = m_objStateMap[ i ];
		EdObject* obj = g_EdWorld->m_objects[ SO.id ];
		ByteReader br( &m_objectStateData, SO.offset );
		obj->Serialize( br );
		obj->MoveSelectedVertices( m_transform );
		obj->RegenerateMesh();
	}
}



EdDrawBlockEditMode::EdDrawBlockEditMode() :
	m_blockDrawMode( BD_Polygon ),
	m_newBlockPropZ0( 0, 2, -8192, 8192 ),
	m_newBlockPropZ1( 2, 2, -8192, 8192 )
{
	m_newBlockPropZ0.caption = "Bottom Z";
	m_newBlockPropZ1.caption = "Top Z";
}

void EdDrawBlockEditMode::OnEnter()
{
	g_UIFrame->SetModeHighlight( &g_UIFrame->m_MBDrawBlock );
	g_UIFrame->SetCursorPlaneHeight( m_newBlockPropZ0.m_value );
	m_drawnVerts.clear();
	g_UIFrame->AddToParamList( &g_UIFrame->m_snapProps );
	g_UIFrame->AddToParamList( &m_newBlockPropZ0 );
	g_UIFrame->AddToParamList( &m_newBlockPropZ1 );
	g_UIFrame->AddToParamList( &m_newSurfProps );
}

int EdDrawBlockEditMode::OnUIEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_newBlockPropZ0 )
		{
			g_UIFrame->SetCursorPlaneHeight( m_newBlockPropZ0.m_value );
		}
		break;
	}
	return EdEditMode::OnUIEvent( e );
}

void EdDrawBlockEditMode::OnViewEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 && g_UIFrame->IsCursorAiming() && m_drawnVerts.size() < 14 )
	{
		m_drawnVerts.push_back( g_UIFrame->GetCursorPlanePos() );
	}
	if( e->type == EDGUI_EVENT_KEYUP )
	{
		if( e->key.engkey == SDLK_RETURN )
		{
			if( m_blockDrawMode == BD_Polygon )
			{
				if( m_drawnVerts.size() >= 3 && m_drawnVerts.size() <= 14 )
				{
					_AddNewBlock();
					m_drawnVerts.clear();
				}
			}
			else if( m_blockDrawMode == BD_BoxStrip )
			{
				if( m_drawnVerts.size() >= 2 )
				{
					Array< Vec2 > verts;
					TSWAP( verts, m_drawnVerts );
					for( size_t i = 1; i < verts.size(); ++i )
					{
						Vec2 p0 = verts[ i - 1 ];
						Vec2 p1 = verts[ i ];
						m_drawnVerts.push_back( V2( p0.x, p0.y ) );
						m_drawnVerts.push_back( V2( p1.x, p0.y ) );
						m_drawnVerts.push_back( V2( p1.x, p1.y ) );
						m_drawnVerts.push_back( V2( p0.x, p1.y ) );
						_AddNewBlock();
						m_drawnVerts.clear();
					}
				}
			}
		}
		if( e->key.engkey == SDLK_ESCAPE )
		{
			m_drawnVerts.clear();
		}
		if( e->key.engkey == SDLK_BACKSPACE && m_drawnVerts.size() )
		{
			m_drawnVerts.pop_back();
		}
		if( e->key.engkey == SDLK_1 ) m_blockDrawMode = BD_Polygon;
		if( e->key.engkey == SDLK_2 ) m_blockDrawMode = BD_BoxStrip;
	}
}

void EdDrawBlockEditMode::Draw()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.UnsetTexture();
	
	float planeHeight = g_UIFrame->GetCursorPlaneHeight();
	
	if( m_blockDrawMode == BD_Polygon && m_drawnVerts.size() >= 3 )
	{
		br.Col( 0.9f, 0.1f, 0, 0.3f );
		br.Poly( m_drawnVerts.data(), m_drawnVerts.size(), planeHeight, sizeof(*m_drawnVerts.data()) );
	}
	if( m_blockDrawMode == BD_BoxStrip )
	{
		for( size_t i = 1; i < m_drawnVerts.size(); ++i )
		{
			br.Col( 0.9f, 0.1f, 0, 0.3f );
			Vec2 p0 = m_drawnVerts[ i - 1 ];
			Vec2 p1 = m_drawnVerts[ i ];
			br.Quad( p0.x, p0.y, p1.x, p1.y, planeHeight );
		}
	}
	if( m_drawnVerts.size() >= 2 )
	{
		br.Col( 0.9f, 0.1f, 0, 0.7f );
		br.PolyOutline( m_drawnVerts.data(), m_drawnVerts.size(), planeHeight, sizeof(*m_drawnVerts.data()) );
	}
	for( size_t i = 0; i < m_drawnVerts.size(); ++i )
	{
		br.Col( 0.9f, 0.1f, 0, 0.8f );
		br.CircleOutline( m_drawnVerts[i].x, m_drawnVerts[i].y, 0.02f, planeHeight, 16 );
	}
	if( m_blockDrawMode == BD_Polygon && g_UIFrame->IsCursorAiming() )
	{
		Vec2 pos = g_UIFrame->GetCursorPlanePos();
		if( m_drawnVerts.size() > 1 )
		{
			br.Col( 0.9f, 0.1f, 0, 0.4f ).SetPrimitiveType( PT_LineStrip );
			br.Pos( m_drawnVerts.last().x, m_drawnVerts.last().y, planeHeight );
			br.Pos( pos.x, pos.y, planeHeight );
			br.Pos( m_drawnVerts[0].x, m_drawnVerts[0].y, planeHeight );
		}
	}
	g_UIFrame->DrawCursor();
}

void EdDrawBlockEditMode::_AddNewBlock()
{
	EdBlock B;
	B.position = V3(0);
	B.z0 = m_newBlockPropZ0.m_value;
	B.z1 = m_newBlockPropZ1.m_value;
	B.poly.resize( m_drawnVerts.size() );
	for( size_t i = 0; i < m_drawnVerts.size(); ++i )
		B.poly[ i ] = V3( m_drawnVerts[ i ].x, m_drawnVerts[ i ].y, 0 );
	if( PolyArea( m_drawnVerts.data(), m_drawnVerts.size() ) < 0 )
		B.poly.reverse();
	B.GenCenterPos( g_UIFrame->m_snapProps );
	for( size_t i = 0; i < m_drawnVerts.size() + 2; ++i )
	{
		EdSurface S;
		m_newSurfProps.BounceBack( S );
		B.surfaces.push_back( S );
	}
	B.subsel.resize( B.GetNumElements() );
	B.ClearSelection();
	g_EdWorld->AddObject( B.Clone() );
}


EdEditBlockEditMode::EdEditBlockEditMode() :
	m_hlObj( -1 ),
	m_curObj( -1 )
{}

void EdEditBlockEditMode::OnEnter()
{
	m_numSel = g_EdWorld->GetNumSelectedObjects();
	g_UIFrame->SetModeHighlight( &g_UIFrame->m_MBEditBlock );
	g_EdWorld->GetSelectedObjectAABB( m_selAABB );
	m_hlObj = -1;
	m_curObj = -1;
	m_hlBBEl = -1;
}

void EdEditBlockEditMode::OnTransformEnd()
{
	_ReloadBlockProps();
}

void EdEditBlockEditMode::OnViewEvent( EDGUIEvent* e )
{
	Vec3 cursorRayPos = g_UIFrame->GetCursorRayPos();
	Vec3 cursorRayDir = g_UIFrame->GetCursorRayDir();
	
	if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 )
	{
		g_EdWorld->SelectObject( m_hlObj, ( g_UIFrame->m_keyMod & KMOD_CTRL ) != 0 );
		m_numSel = g_EdWorld->GetNumSelectedObjects();
		m_curObj = g_EdWorld->GetOnlySelectedObject();
		_ReloadBlockProps();
	}
	if( e->type == EDGUI_EVENT_MOUSEMOVE )
	{
		g_EdWorld->RayObjectsIntersect( cursorRayPos, cursorRayDir, m_curObj, NULL, &m_hlObj );
		m_hlBBEl = GetClosestActivePoint();
	}
	if( e->type == EDGUI_EVENT_KEYDOWN )
	{
		m_hlBBEl = GetClosestActivePoint();
		// GRAB (MOVE)
		if( e->key.engkey == SDLK_g )
		{
			m_transform.m_extend = false;
			g_UIFrame->SetEditTransform( &m_transform );
		}
		// EXTEND
		if( e->key.engkey == SDLK_e && m_hlBBEl != -1 )
		{
			m_transform.m_extend = true;
			m_transform.m_xtdAABB[0] = m_selAABB[0];
			m_transform.m_xtdAABB[1] = m_selAABB[1];
			m_transform.m_xtdMask = GetActivePointFactor( m_hlBBEl );
			g_UIFrame->SetEditTransform( &m_transform );
		}
		// DELETE
		if( e->key.engkey == SDLK_DELETE )
		{
			g_EdWorld->DeleteSelectedObjects();
			m_curObj = -1;
			_ReloadBlockProps();
			g_UIFrame->RefreshMouse();
		}
		// DUPLICATE
		if( e->key.engkey == SDLK_d && e->key.engmod & KMOD_CTRL )
		{
			if( g_EdWorld->DuplicateSelectedObjectsAndMoveSelection() )
			{
				m_curObj = g_EdWorld->GetOnlySelectedObject();
				_ReloadBlockProps();
				m_transform.m_extend = false;
				g_UIFrame->SetEditTransform( &m_transform );
			}
		}
		// TO VERTEX MODE
		if( e->key.engkey == SDLK_v && e->key.engmod & KMOD_ALT )
		{
			g_UIFrame->SetEditMode( &g_UIFrame->m_emEditVertex );
		}
	}
	if( e->type == EDGUI_EVENT_PAINT )
	{
		if( m_numSel && m_hlBBEl != -1 )
		{
			int x0 = g_UIFrame->m_UIRenderView.x0;
			int y0 = g_UIFrame->m_UIRenderView.y0;
			char bfr[ 1024 ];
			sgrx_snprintf( bfr, 1024, "Press E to extend selection along %s", GetActivePointExtName( m_hlBBEl ) );
			GR2D_SetColor( 1, 1 );
			GR2D_DrawTextLine( x0, y0, bfr, HALIGN_LEFT, VALIGN_TOP );
		}
	}
}

void EdEditBlockEditMode::Draw()
{
	g_EdWorld->DrawWires_Objects( m_hlObj, m_curObj );
	
	// if any block is selected..
	if( m_numSel && g_UIFrame->m_editTF == NULL )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
		for( int i = 0; i < NUM_AABB_ACTIVE_POINTS; ++i )
		{
			if( i == m_hlBBEl )
				br.Col( 0.1f, 0.8f, 0.9f, 1 );
			else
				br.Col( 0.1f, 0.2f, 0.4f, 1 );
			if( IsActivePointSelectable( i ) )
			{
				Vec3 pp = GetActivePoint( i );
				br.Sprite( pp, 0.05f, 0.05f );
			}
		}
	}
}

void EdEditBlockEditMode::_ReloadBlockProps()
{
	g_EdWorld->GetSelectedObjectAABB( m_selAABB );
	m_hlBBEl = GetClosestActivePoint();
	
	g_UIFrame->ClearParamList();
	if( m_curObj >= 0 )
	{
		g_UIFrame->AddToParamList( g_EdWorld->GetObjProps( m_curObj ) );
	}
}

Vec3 EdEditBlockEditMode::GetActivePointFactor( int i )
{
	ASSERT( i >= 0 && i < NUM_AABB_ACTIVE_POINTS );
	static const Vec3 aplerpfacs[ NUM_AABB_ACTIVE_POINTS ] =
	{
		V3(0,0,0), V3(0.5f,0,0), V3(1,0,0), V3(0,0.5f,0), V3(0.5f,0.5f,0), V3(1,0.5f,0), V3(0,1,0), V3(0.5f,1,0), V3(1,1,0),
		V3(0,0,0.5f), V3(0.5f,0,0.5f), V3(1,0,0.5f), V3(0,0.5f,0.5f), /*V3(0.5f,0.5f,0.5f),*/
			V3(1,0.5f,0.5f), V3(0,1,0.5f), V3(0.5f,1,0.5f), V3(1,1,0.5f),
		V3(0,0,1), V3(0.5f,0,1), V3(1,0,1), V3(0,0.5f,1), V3(0.5f,0.5f,1), V3(1,0.5f,1), V3(0,1,1), V3(0.5f,1,1), V3(1,1,1),
	};
	return aplerpfacs[ i ];
}

Vec3 EdEditBlockEditMode::GetActivePoint( int i )
{
	Vec3 fac = GetActivePointFactor( i );
	return V3
	(
		TLERP( m_selAABB[0].x, m_selAABB[1].x, fac.x ),
		TLERP( m_selAABB[0].y, m_selAABB[1].y, fac.y ),
		TLERP( m_selAABB[0].z, m_selAABB[1].z, fac.z )
	);
}

const char* EdEditBlockEditMode::GetActivePointExtName( int i )
{
	ASSERT( i >= 0 && i < NUM_AABB_ACTIVE_POINTS );
	static const char* apenames[ NUM_AABB_ACTIVE_POINTS ] =
	{
		"X/Y/Z", "Y/Z", "X/Y/Z",  "X/Z", "Z", "X/Z",  "X/Y/Z", "Y/Z", "X/Y/Z",
		"X/Y", "Y", "X/Y",  "X", /* "-", */ "X",  "X/Y", "Y", "X/Y",
		"X/Y/Z", "Y/Z", "X/Y/Z",  "X/Z", "Z", "X/Z",  "X/Y/Z", "Y/Z", "X/Y/Z",
	};
	return apenames[ i ];
}

bool EdEditBlockEditMode::IsActivePointSelectable( int i )
{
	static const float APDF = 0.1f;
	ASSERT( i >= 0 && i < NUM_AABB_ACTIVE_POINTS );
	
	Vec3 cp = g_EdScene->camera.position;
	Vec3 pos = GetActivePoint( i );
	Vec3 dir = ( pos - cp ).Normalized();
	dir = Vec3::Max( dir, -dir ); // abs
	
	static const char apsmasks[ NUM_AABB_ACTIVE_POINTS ] =
	{
		1|2|4, 2|4, 1|2|4,  1|4, 4, 1|4,  1|2|4, 2|4, 1|2|4,
		1|2, 2, 1|2,  1, /* 0, */ 1,  1|2, 2, 1|2,
		1|2|4, 2|4, 1|2|4,  1|4, 4, 1|4,  1|2|4, 2|4, 1|2|4,
	};
	char mask = apsmasks[ i ];
	if( ( m_selAABB[0].x == m_selAABB[1].x
		|| m_selAABB[0].y == m_selAABB[1].y
		|| m_selAABB[0].z == m_selAABB[1].z ) &&
		mask != 1 && mask != 2 && mask != 4 &&
		( mask == (1|2) && m_selAABB[0].x != m_selAABB[1].x && m_selAABB[0].y != m_selAABB[1].y ) == false )
	{
		// disable edges/corners on zero-volume AABB
		return false;
	}
	switch( mask )
	{
	case 1: return dir.x < 1-APDF;
	case 2: return dir.y < 1-APDF;
	case 3: return dir.z > APDF;
	case 4: return dir.z < 1-APDF;
	case 5: return dir.y > APDF;
	case 6: return dir.x > APDF;
	case 7: return dir.x < 1-APDF && dir.y < 1-APDF && dir.z < 1-APDF;
	}
	return false;
}

int EdEditBlockEditMode::GetClosestActivePoint()
{
	int np = -1;
	float minxydist = FLT_MAX;
	Vec2 scp = ED_GetCursorPos();
	Vec3 campos = g_EdScene->camera.position;
	Vec3 camdir = g_EdScene->camera.direction;
	for( int i = 0; i < NUM_AABB_ACTIVE_POINTS; ++i )
	{
		if( IsActivePointSelectable( i ) == false )
			continue;
		
		Vec3 ap = GetActivePoint( i );
		Vec2 sap = ED_GetScreenPos( ap );
		
		float curxydist = ( scp - sap ).Length();
		float curzdist = Vec3Dot( ap, camdir ) - Vec3Dot( campos, camdir );
		if( curzdist > 0 && curxydist < minxydist )
		{
			np = i;
			minxydist = curxydist;
		}
	}
	return np;
}


void EdEditVertexEditMode::OnEnter()
{
	m_hlAP.block = -1;
	m_hlAP.point = -1;
	
	m_selObjList.clear();
	for( size_t i = 0; i < g_EdWorld->m_objects.size(); ++i )
	{
		if( g_EdWorld->m_objects[ i ]->selected == false )
			continue;
		m_selObjList.push_back( i );
	}
	
	_ReloadVertSurfProps();
}

void EdEditVertexEditMode::OnViewEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 )
	{
		for( size_t b = 0; b < m_selObjList.size(); ++b )
		{
			int oid = m_selObjList[ b ];
			EdObject* obj = g_EdWorld->m_objects[ oid ];
			
			if( oid == m_hlAP.block )
				obj->UISelectElement( m_hlAP.point, ( g_UIFrame->m_keyMod & KMOD_CTRL ) != 0 );
			else if( ( g_UIFrame->m_keyMod & KMOD_CTRL ) == 0 )
				obj->ClearSelection();
		}
		
		_ReloadVertSurfProps();
	}
	if( e->type == EDGUI_EVENT_MOUSEMOVE )
	{
		m_hlAP = GetClosestActivePoint();
	}
	if( e->type == EDGUI_EVENT_KEYDOWN )
	{
		m_hlAP = GetClosestActivePoint();
		
		// GRAB (MOVE)
		if( e->key.engkey == SDLK_g )
		{
			m_transform.m_extend = false;
			g_UIFrame->SetEditTransform( &m_transform );
		}
		// TO BLOCK MODE
		if( e->key.engkey == SDLK_b && e->key.engmod & KMOD_ALT )
		{
			g_UIFrame->SetEditMode( &g_UIFrame->m_emEditBlock );
		}
	}
}

void EdEditVertexEditMode::Draw()
{
	if( g_UIFrame->m_editTF == NULL )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		for( size_t b = 0; b < m_selObjList.size(); ++b )
		{
			int oid = m_selObjList[ b ];
			int bpcount = GetNumObjectActivePoints( oid );
			for( int i = 0; i < bpcount; ++i )
			{
				if( g_EdWorld->m_objects[ oid ]->IsElementSelected( i ) )
				{
					if( oid == m_hlAP.block && i == m_hlAP.point )
						br.Col( 0.9f, 0.8f, 0.1f, 1 );
					else
						br.Col( 0.9f, 0.5f, 0.1f, 1 );
				}
				else
				{
					if( oid == m_hlAP.block && i == m_hlAP.point )
						br.Col( 0.1f, 0.8f, 0.9f, 1 );
					else
						br.Col( 0.1f, 0.2f, 0.4f, 1 );
				}
				
				Vec3 pp = GetActivePoint( oid, i );
				br.Sprite( pp, 0.05f, 0.05f );
			}
		}
	}
}

void EdEditVertexEditMode::_ReloadVertSurfProps()
{
	int numsurfselblocks = 0;
	int numsurfexblocks = 0;
	ActivePoint surfprops = { -1, -1 };
	ActivePoint vertprops = { -1, -1 };
	for( size_t b = 0; b < m_selObjList.size(); ++b )
	{
		int oid = m_selObjList[ b ];
		EdObject* obj = g_EdWorld->m_objects[ oid ];
		
		int nss = obj->GetNumSelectedSurfs();
		numsurfselblocks += nss == 1;
		numsurfexblocks += nss == 0 || nss == 1;
		
		int surf = obj->GetOnlySelectedSurface();
		if( surf != -1 )
		{
			if( surfprops.block == -1 && surfprops.point == -1 )
			{
				surfprops.block = oid;
				surfprops.point = surf;
			}
			else
			{
				surfprops.block = -1;
			}
		}
		
		int vert = obj->GetOnlySelectedVertex();
		if( vert != -1 )
		{
			if( vertprops.block == -1 && vertprops.point == -1 )
			{
				vertprops.block = oid;
				vertprops.point = vert;
			}
			else
			{
				vertprops.block = -1;
			}
		}
	}
	
	m_canExtendSurfs = numsurfexblocks && numsurfselblocks;
	
	g_UIFrame->ClearParamList();
	if( surfprops.block != -1 )
		g_UIFrame->AddToParamList( g_EdWorld->GetSurfProps( surfprops.block, surfprops.point ) );
	if( vertprops.block != -1 )
		g_UIFrame->AddToParamList( g_EdWorld->GetVertProps( vertprops.block, vertprops.point ) );
}

int EdEditVertexEditMode::GetNumObjectActivePoints( int b )
{
	EdObject* obj = g_EdWorld->m_objects[ b ];
	return obj->GetNumElements();
}

Vec3 EdEditVertexEditMode::GetActivePoint( int b, int i )
{
	EdObject* obj = g_EdWorld->m_objects[ b ];
	return obj->GetElementPoint( i );
}

EdEditVertexEditMode::ActivePoint EdEditVertexEditMode::GetClosestActivePoint()
{
	ActivePoint np = { -1, -1 };
	float minxydist = FLT_MAX;
	Vec2 scp = ED_GetCursorPos();
	Vec3 campos = g_EdScene->camera.position;
	Vec3 camdir = g_EdScene->camera.direction;
	for( size_t b = 0; b < m_selObjList.size(); ++b )
	{
		int oid = m_selObjList[ b ];
		int bpcount = GetNumObjectActivePoints( oid );
		for( int i = 0; i < bpcount; ++i )
		{
			Vec3 ap = GetActivePoint( oid, i );
			Vec2 sap = ED_GetScreenPos( ap );
			
			float curxydist = ( scp - sap ).Length();
			float curzdist = Vec3Dot( ap, camdir ) - Vec3Dot( campos, camdir );
			if( curzdist > 0 && curxydist < minxydist )
			{
				np.block = oid;
				np.point = i;
				minxydist = curxydist;
			}
		}
	}
	return np;
}


EdPaintSurfsEditMode::EdPaintSurfsEditMode() :
	m_paintBlock( -1 ),
	m_paintSurf( -1 ),
	m_isPainting( false )
{
}

void EdPaintSurfsEditMode::OnEnter()
{
	g_UIFrame->SetModeHighlight( &g_UIFrame->m_MBPaintSurfs );
	m_paintBlock = -1;
	m_paintSurf = -1;
	m_isPainting = false;
	g_UIFrame->AddToParamList( &m_paintSurfProps );
}

void EdPaintSurfsEditMode::OnViewEvent( EDGUIEvent* e )
{
	Vec3 cursorRayPos = g_UIFrame->GetCursorRayPos();
	Vec3 cursorRayDir = g_UIFrame->GetCursorRayDir();
	
	bool dopaint = false;
	if( e->type == EDGUI_EVENT_BTNDOWN && e->mouse.button == 0 )
	{
		m_isPainting = true;
		dopaint = true;
	}
	if( e->type == EDGUI_EVENT_BTNUP && e->mouse.button == 0 ) m_isPainting = false;
	if( e->type == EDGUI_EVENT_MOUSEMOVE )
	{
		float outdst[1];
		int outblock[1];
		m_paintBlock = -1;
		if( g_EdWorld->RayBlocksIntersect( cursorRayPos, cursorRayDir, m_paintBlock, outdst, outblock ) )
			m_paintBlock = outblock[0];
		m_paintSurf = -1;
		if( m_paintBlock >= 0 && g_EdWorld->m_blocks[ m_paintBlock ]->RayIntersect( cursorRayPos, cursorRayDir, outdst, outblock ) )
			m_paintSurf = outblock[0];
		if( m_isPainting )
			dopaint = true;
	}
	if( e->type == EDGUI_EVENT_KEYDOWN && !e->key.repeat && e->key.engkey == SDLK_g && m_paintBlock >= 0 && m_paintSurf >= 0 )
	{
		m_paintSurfProps.LoadParams( g_EdWorld->m_blocks[ m_paintBlock ]->surfaces[ m_paintSurf ] );
	}
	if( dopaint && m_paintBlock >= 0 && m_paintSurf >= 0 )
	{
		m_paintSurfProps.BounceBack( g_EdWorld->m_blocks[ m_paintBlock ]->surfaces[ m_paintSurf ] );
		g_EdWorld->m_blocks[ m_paintBlock ]->RegenerateMesh();
	}
}

void EdPaintSurfsEditMode::Draw()
{
	if( m_paintBlock >= 0 && m_paintSurf >= 0 )
	{
		g_EdWorld->DrawPoly_BlockSurf( m_paintBlock, m_paintSurf, m_isPainting );
	}
}

EdAddEntityEditMode::EdAddEntityEditMode()
{
	m_entityProps = m_entGroup.m_buttons[0].m_ent_handle;
}

void EdAddEntityEditMode::OnEnter()
{
	g_UIFrame->SetModeHighlight( &g_UIFrame->m_MBAddEntity );
	g_UIFrame->SetCursorPlaneHeight( m_entityProps->Pos().z );
	g_UIFrame->AddToParamList( &m_entGroup );
	g_UIFrame->AddToParamList( m_entityProps );
}

int EdAddEntityEditMode::OnUIEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_SETENTITY:
		SetEntityType( ((EDGUIEntButton*)e->target)->m_ent_handle );
		break;
		
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_entityProps->m_ctlPos )
		{
			g_UIFrame->SetCursorPlaneHeight( m_entityProps->Pos().z );
		}
		break;
	}
	return EdEditMode::OnUIEvent( e );	
}

void EdAddEntityEditMode::OnViewEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 && g_UIFrame->IsCursorAiming() )
	{
		_AddNewEntity();
	}
}

void EdAddEntityEditMode::Draw()
{
	g_EdWorld->DrawWires_Entities( -1, -1 );
	g_UIFrame->DrawCursor( false );
}

void EdAddEntityEditMode::SetEntityType( const EdEntityHandle& eh )
{
	m_entityProps = eh;
	g_UIFrame->ClearParamList();
	g_UIFrame->AddToParamList( &m_entGroup );
	g_UIFrame->AddToParamList( m_entityProps );
}

void EdAddEntityEditMode::_AddNewEntity()
{
	Vec2 pos = g_UIFrame->GetCursorPlanePos();
	
	EdEntity* N = m_entityProps->CloneEntity();
	N->SetPosition( V3( pos.x, pos.y, N->Pos().z ) );
	g_EdWorld->AddObject( N );
}

EdEditEntityEditMode::EdEditEntityEditMode() :
	m_hlEnt( -1 ),
	m_selEnt( -1 ),
	m_grabbed( false )
{
}

void EdEditEntityEditMode::OnEnter()
{
	g_UIFrame->SetModeHighlight( &g_UIFrame->m_MBEditEntity );
	m_hlEnt = -1;
	m_selEnt = -1;
	m_grabbed = false;
}

int EdEditEntityEditMode::OnUIEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( m_selEnt >= 0 && e->target == &g_EdWorld->m_entities[ m_selEnt ]->m_ctlPos )
		{
			g_UIFrame->SetCursorPlaneHeight( g_EdWorld->m_entities[ m_selEnt ]->Pos().z );
		}
		return 1;
	}
	return EdEditMode::OnUIEvent( e );
}

void EdEditEntityEditMode::OnViewEvent( EDGUIEvent* e )
{
	Vec3 cursorRayPos = g_UIFrame->GetCursorRayPos();
	Vec3 cursorRayDir = g_UIFrame->GetCursorRayDir();
	Vec2 cursorPlanePos = g_UIFrame->GetCursorPlanePos();
	bool cursorAim = g_UIFrame->IsCursorAiming();
	
	if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 )
	{
		m_grabbed = false;
		m_selEnt = m_hlEnt;
		_ReloadEntityProps();
	}
	if( e->type == EDGUI_EVENT_MOUSEMOVE )
	{
		float outdst[1];
		int outmesh[1];
		m_hlEnt = -1;
		if( g_EdWorld->RayEntitiesIntersect( cursorRayPos, cursorRayDir, m_selEnt, outdst, outmesh ) )
			m_hlEnt = outmesh[0];
		if( m_grabbed && m_selEnt >= 0 )
		{
			EdEntity* N = g_EdWorld->m_entities[ m_selEnt ];
			N->SetPosition( V3( cursorPlanePos.x + m_cpdiff.x, cursorPlanePos.y + m_cpdiff.y, N->Pos().z ) );
			N->RegenerateMesh();
			_ReloadEntityProps();
		}
	}
	// GRAB
	if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_g && !e->key.repeat && m_selEnt >= 0 && cursorAim )
	{
		Vec3 P = g_EdWorld->m_entities[ m_selEnt ]->Pos();
		m_cpdiff = V2(P.x,P.y) - cursorPlanePos;
		m_grabbed = true;
	}
	if( e->type == EDGUI_EVENT_KEYUP && ( e->key.engkey == SDLK_g || e->key.engkey == SDLK_d ) && m_grabbed )
	{
		m_grabbed = false;
	}
	// DELETE
	if( e->type == EDGUI_EVENT_KEYUP && e->key.engkey == SDLK_DELETE && m_selEnt >= 0 )
	{
		g_UIFrame->ClearParamList();
		g_EdWorld->m_entities.erase( m_selEnt );
		if( m_hlEnt == m_selEnt )
			m_hlEnt = -1;
		m_selEnt = -1;
	}
	// DUPLICATE
	if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_d && !e->key.repeat && ( e->key.engmod & KMOD_CTRL ) && m_selEnt >= 0 )
	{
		EdEntity* N = g_EdWorld->m_entities[ m_selEnt ]->CloneEntity();
		m_selEnt = g_EdWorld->m_entities.size();
		N->RegenerateMesh();
		g_EdWorld->m_entities.push_back( N );
		Vec3 P = N->Pos();
		m_cpdiff = V2(P.x,P.y) - cursorPlanePos;
		m_grabbed = true;
		_ReloadEntityProps();
	}
}

void EdEditEntityEditMode::Draw()
{
	g_EdWorld->DrawWires_Entities( m_hlEnt, m_selEnt );
	if( m_grabbed )
		g_UIFrame->DrawCursor( false );
}

void EdEditEntityEditMode::_ReloadEntityProps()
{
	g_UIFrame->ClearParamList();
	if( m_selEnt >= 0 )
	{
		g_UIFrame->SetCursorPlaneHeight( g_EdWorld->m_entities[ m_selEnt ]->Pos().z );
		g_UIFrame->AddToParamList( g_EdWorld->GetEntityProps( m_selEnt ) );
	}
}

void EdEditGroupEditMode::OnEnter()
{
	g_UIFrame->SetModeHighlight( &g_UIFrame->m_MBEditGroups );
	g_EdWorld->m_groupMgr.PrepareCurrentEditGroup();
	g_UIFrame->AddToParamList( &g_EdWorld->m_groupMgr );
}

void EdEditGroupEditMode::Draw()
{
	g_EdWorld->m_groupMgr.DrawGroups();
}

void EdEditLevelEditMode::OnEnter()
{
	g_UIFrame->SetModeHighlight( &g_UIFrame->m_MBLevelInfo );
	g_UIFrame->AddToParamList( g_EdWorld );
}


