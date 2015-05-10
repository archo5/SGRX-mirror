

#include "mapedit.hpp"



Vec2 EdEditTransform::GetCursorPos()
{
	Vec2 cp = Game_GetCursorPos();
	return V2
	(
		cp.x / GR_GetWidth() * 2 - 1,
		cp.y / GR_GetHeight() * -2 + 1
	);
}

Vec2 EdEditTransform::GetScreenPos( const Vec3& p )
{
	Vec3 tp = g_EdScene->camera.WorldToScreen( p );
	return V2
	(
		tp.x * 2 - 1,
		tp.y * -2 + 1
	);
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
	if( g_EdWorld->GetNumSelectedBlocks() == 0 )
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
	m_blocks.clear();
	for( size_t i = 0; i < g_EdWorld->m_blocks.size(); ++i )
	{
		EdBlock& B = g_EdWorld->m_blocks[ i ];
		if( B.selected == false )
			continue;
		m_blocks.push_back(SavedBlock());
		SavedBlock& SB = m_blocks.last();
		SB.id = i;
		SB.data = B;
		SB.data.cached_mesh = NULL;
		SB.data.cached_meshinst = NULL;
		
		cp += SB.data.FindCenter();
		cpc++;
	}
	m_origin = g_UIFrame->Snapped( cp / cpc );
}

void EdBlockEditTransform::RestoreState()
{
	for( size_t i = 0; i < m_blocks.size(); ++i )
	{
		SavedBlock& SB = m_blocks[ i ];
		g_EdWorld->m_blocks[ SB.id ] = SB.data;
		g_EdWorld->m_blocks[ SB.id ].RegenerateMesh();
	}
}

Vec3 EdBlockEditTransform::GetMovementVector( const Vec2& a, const Vec2& b )
{
	Vec3 axis = V3(0);
	switch( m_cmode )
	{
	case Camera: axis = g_EdScene->camera.direction.Normalized(); break;
	case XAxis:
	case XPlane: axis = V3(1,0,0); break;
	case YAxis:
	case YPlane: axis = V3(0,1,0); break;
	case ZAxis:
	case ZPlane: axis = V3(0,0,1); break;
	}
	// plane, raycast both points
	if( m_cmode == Camera || m_cmode == XPlane || m_cmode == YPlane || m_cmode == ZPlane )
	{
		Vec4 plane = V4( axis, Vec3Dot( axis, m_origin ) );
		Vec3 ap, ad, bp, bd;
		float aid[2], bid[2];
		if( g_EdScene->camera.GetCursorRay( a.x * 0.5f + 0.5f, a.y * -0.5f + 0.5f, ap, ad ) &&
			g_EdScene->camera.GetCursorRay( b.x * 0.5f + 0.5f, b.y * -0.5f + 0.5f, bp, bd ) &&
			RayPlaneIntersect( ap, ad, plane, aid ) &&
			RayPlaneIntersect( bp, bd, plane, bid ) )
		{
			return bp + bd * bid[0] - ( ap + ad * aid[0] );
		}
	}
	// axis
	else if( m_cmode == XAxis || m_cmode == YAxis || m_cmode == ZAxis )
	{
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
		sgrx_snprintf( bfr, 1024, "Moving blocks: %g ; %g ; %g", m_transform.x, m_transform.y, m_transform.z );
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
		br.Col(1,0,0,1).Pos( m_origin ).Prev(0);
		br.Col(1,0,0,0).Pos( m_origin + V3(D,0,0) );
		br.Col(1,0,0,0).Pos( m_origin + m_transform - V3(D,0,0) );
		br.Col(1,0,0,1).Pos( m_origin + m_transform ).Prev(0);
		br.Col(1,0,0,0).Pos( m_origin + m_transform + V3(D,0,0) );
	}
	if( m_cmode == YAxis || m_cmode == XPlane || m_cmode == ZPlane )
	{
		br.Col(0,1,0,0).Pos( m_origin - V3(0,D,0) );
		br.Col(0,1,0,1).Pos( m_origin ).Prev(0);
		br.Col(0,1,0,0).Pos( m_origin + V3(0,D,0) );
		br.Col(0,1,0,0).Pos( m_origin + m_transform - V3(0,D,0) );
		br.Col(0,1,0,1).Pos( m_origin + m_transform ).Prev(0);
		br.Col(0,1,0,0).Pos( m_origin + m_transform + V3(0,D,0) );
	}
	if( m_cmode == ZAxis || m_cmode == XPlane || m_cmode == YPlane )
	{
		br.Col(0,0,1,0).Pos( m_origin - V3(0,0,D) );
		br.Col(0,0,1,1).Pos( m_origin ).Prev(0);
		br.Col(0,0,1,0).Pos( m_origin + V3(0,0,D) );
		br.Col(0,0,1,0).Pos( m_origin + m_transform - V3(0,0,D) );
		br.Col(0,0,1,1).Pos( m_origin + m_transform ).Prev(0);
		br.Col(0,0,1,0).Pos( m_origin + m_transform + V3(0,0,D) );
	}
}

void EdBlockMoveTransform::ApplyTransform()
{
	for( size_t i = 0; i < m_blocks.size(); ++i )
	{
		SavedBlock& SB = m_blocks[ i ];
		EdBlock& B = g_EdWorld->m_blocks[ SB.id ];
		B = SB.data;
		B.position += m_transform;
		B.RegenerateMesh();
	}
}

void EdBlockMoveTransform::RecalcTransform()
{
	m_transform = g_UIFrame->Snapped( GetMovementVector( m_startCursorPos, GetCursorPos() ) );
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
	B.RegenerateMesh();
	g_EdWorld->m_blocks.push_back( B );
}


EdEditBlockEditMode::EdEditBlockEditMode() :
	m_hlBlock( -1 ),
	m_selBlock( -1 ),
	m_hlSurf( -1 ),
	m_selSurf( -1 ),
	m_hlVert( -1 ),
	m_selVert( -1 ),
	m_dragAdjacent( false ),
	m_grabbed( false )
{}

void EdEditBlockEditMode::OnEnter()
{
	g_UIFrame->SetModeHighlight( &g_UIFrame->m_MBEditBlock );
	m_hlBlock = -1;
	m_selBlock = -1;
	m_grabbed = false;
}

void EdEditBlockEditMode::OnViewEvent( EDGUIEvent* e )
{
	Vec3 cursorRayPos = g_UIFrame->GetCursorRayPos();
	Vec3 cursorRayDir = g_UIFrame->GetCursorRayDir();
	Vec2 cursorPlanePos = g_UIFrame->GetCursorPlanePos();
	bool cursorAim = g_UIFrame->IsCursorAiming();
	
	if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 )
	{
		g_EdWorld->SelectBlock( m_hlBlock, ( g_UIFrame->m_keyMod & KMOD_CTRL ) != 0 );
		m_selBlock = g_EdWorld->GetOnlySelectedBlock();
		_ReloadBlockProps();
	}
	if( e->type == EDGUI_EVENT_MOUSEMOVE )
	{
		g_EdWorld->RayBlocksIntersect( cursorRayPos, cursorRayDir, m_selBlock, NULL, &m_hlBlock );
	}
	if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_g )
	{
		g_UIFrame->SetEditTransform( &m_transform );
	}
#if 0
	if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 )
	{
		if( m_selBlock != -1 && m_hlVert != -1 )
		{
			if( m_selVert == m_hlVert )
				m_selVert = -1;
			else
			{
				m_selVert = m_hlVert;
				m_selSurf = -1;
			}
		}
		else if( m_selBlock != -1 && m_hlSurf != -1 )
		{
			if( m_selSurf == m_hlSurf )
				m_selSurf = -1;
			else
			{
				m_selSurf = m_hlSurf;
				m_selVert = -1;
			}
		}
		else
		{
			m_selBlock = m_hlBlock;
			m_selSurf = -1;
			m_selVert = -1;
		}
		_ReloadBlockProps();
		g_UIFrame->RefreshMouse();
	}
	if( e->type == EDGUI_EVENT_MOUSEMOVE )
	{
		float outdst[1];
		int outblock[1];
		if( m_selBlock >= 0 )
		{
			EdBlock& B = g_EdWorld->m_blocks[ m_selBlock ];
			float mindst = FLT_MAX;
			m_hlVert = -1;
			for( size_t i = 0; i < B.poly.size(); ++i )
			{
				if( RaySphereIntersect( cursorRayPos, cursorRayDir, V3( B.poly[i].x + B.position.x, B.poly[i].y + B.position.y, B.z0 + B.position.z ), 0.2f, outdst )
					&& outdst[0] < mindst )
				{
					mindst = outdst[0];
					m_hlVert = i;
				}
			}
			m_hlSurf = -1;
			if( B.RayIntersect( cursorRayPos, cursorRayDir, outdst, outblock ) && outdst[0] < mindst )
			{
				m_hlSurf = outblock[0];
				m_hlVert = -1;
			}
		}
		if( m_selBlock < 0 || ( m_hlVert < 0 && m_hlSurf < 0 ) )
		{
			m_hlBlock = -1;
			if( g_EdWorld->RayBlocksIntersect( cursorRayPos, cursorRayDir, m_selBlock, outdst, outblock ) )
				m_hlBlock = outblock[0];
		}
		if( m_grabbed && m_selBlock >= 0 )
		{
			EdBlock& B = g_EdWorld->m_blocks[ m_selBlock ];
			Vec2 tgtpos = cursorPlanePos + m_cpdiff;
			int selvert = m_selVert;
			bool itssurf = false;
			if( selvert < 0 && m_selSurf < (int) B.poly.size() )
			{
				selvert = m_selSurf;
				itssurf = true;
			}
			if( selvert >= 0 )
			{
				g_UIFrame->Snap( tgtpos );
				B.poly[ selvert ].SetXY( tgtpos );
				if( m_dragAdjacent || itssurf )
				{
					size_t bps = B.poly.size();
					Vec2 edgeNrm0 = ( m_origPos - m_origPos0 ).Perp().Normalized();
					Vec2 edgeNrm1 = ( m_origPos1 - m_origPos ).Perp().Normalized();
					Vec2 diff = tgtpos - m_origPos;
					if( itssurf )
					{
						diff = edgeNrm1 * Vec2Dot( edgeNrm1, diff );
						B.poly[ selvert ].SetXY( m_origPos + edgeNrm1 * Vec2Dot( edgeNrm1, diff ) );
					}
					B.poly[ ( selvert + bps - 1 ) % bps ].SetXY( m_origPos0 + edgeNrm0 * Vec2Dot( edgeNrm0, diff ) );
					B.poly[ ( selvert + 1 ) % bps ].SetXY( m_origPos1 + edgeNrm1 * Vec2Dot( edgeNrm1, diff ) );
				}
			}
			else
				B.position = V3(tgtpos.x,tgtpos.y,B.position.z); // TODO_FULL_TRANSFORM
			g_EdWorld->m_blocks[ m_selBlock ].RegenerateMesh();
			_ReloadBlockProps();
		}
	}
	// GRAB
	if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_g && !e->key.repeat && m_selBlock >= 0 && cursorAim )
	{
		EdBlock& B = g_EdWorld->m_blocks[ m_selBlock ];
		int selvert = m_selVert;
		if( selvert < 0 && m_selSurf < (int) B.poly.size() )
			selvert = m_selSurf;
		if( selvert >= 0 )
		{
			size_t bps = B.poly.size();
			m_origPos = B.poly[ selvert ].ToVec2();
			m_origPos0 = B.poly[ ( selvert + bps - 1 ) % bps ].ToVec2();
			m_origPos1 = B.poly[ ( selvert + 1 ) % bps ].ToVec2();
			m_cpdiff = m_origPos - cursorPlanePos;
		}
		else
			m_cpdiff = B.position.ToVec2() - cursorPlanePos; // TODO_FULL_TRANSFORM
		m_dragAdjacent = ( e->key.engmod & KMOD_CTRL ) != 0;
		m_grabbed = true;
	}
	if( e->type == EDGUI_EVENT_KEYUP && ( e->key.engkey == SDLK_g || e->key.engkey == SDLK_d ) && m_grabbed )
	{
		m_grabbed = false;
	}
	// DELETE
	if( e->type == EDGUI_EVENT_KEYUP && e->key.engkey == SDLK_DELETE && m_selBlock >= 0 )
	{
		g_UIFrame->ClearParamList();
		if( m_selVert >= 0 )
		{
			g_EdWorld->m_ctlVertProps.m_out = NULL; // just in case
			EdBlock& B = g_EdWorld->m_blocks[ m_selBlock ];
			B.poly.erase( m_selVert );
			B.surfaces.erase( m_selVert );
			if( m_hlVert == m_selVert )
				m_hlVert = -1;
			m_hlSurf = -1;
			m_selSurf = -1;
			m_selVert = -1;
			B.RegenerateMesh();
		}
		else
		{
			g_EdWorld->m_ctlBlockProps.m_out = NULL; // just in case
			g_EdWorld->m_blocks.erase( m_selBlock );
			if( m_hlBlock == m_selBlock )
				m_hlBlock = -1;
			m_selBlock = -1;
			m_selVert = -1;
			m_selSurf = -1;
		}
	}
	// DUPLICATE
	if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_d && !e->key.repeat && ( e->key.engmod & KMOD_CTRL ) && m_selBlock >= 0 )
	{
		g_UIFrame->ClearParamList();
		g_EdWorld->m_ctlBlockProps.m_out = NULL; // just in case
		EdBlock B = g_EdWorld->m_blocks[ m_selBlock ];
		m_selBlock = g_EdWorld->m_blocks.size();
		m_selVert = -1;
		m_selSurf = -1;
		B.cached_mesh = NULL;
		B.cached_meshinst = NULL;
		B.RegenerateMesh();
		g_EdWorld->m_blocks.push_back( B );
		m_cpdiff = B.position.ToVec2() - cursorPlanePos; // TODO_FULL_TRANSFORM
		m_grabbed = true;
		g_UIFrame->AddToParamList( g_EdWorld->GetBlockProps( m_selBlock ) );
	}
#endif
}

void EdEditBlockEditMode::Draw()
{
	g_EdWorld->DrawWires_Blocks( m_hlBlock, m_selBlock );
	if( m_selBlock >= 0 )
	{
		if( m_selSurf >= 0 )
			g_EdWorld->DrawPoly_BlockSurf( m_selBlock, m_selSurf, true );
		if( m_hlSurf >= 0 )
			g_EdWorld->DrawPoly_BlockSurf( m_selBlock, m_hlSurf, false );
		if( m_selVert >= 0 )
			g_EdWorld->DrawPoly_BlockVertex( m_selBlock, m_selVert, true );
		if( m_hlVert >= 0 )
			g_EdWorld->DrawPoly_BlockVertex( m_selBlock, m_hlVert, true );
	}
	if( m_grabbed )
		g_UIFrame->DrawCursor( false );
}

void EdEditBlockEditMode::_ReloadBlockProps()
{
	g_UIFrame->ClearParamList();
	if( m_selBlock >= 0 )
	{
		g_UIFrame->m_UIRenderView.crplaneheight = g_EdWorld->m_blocks[ m_selBlock ].z0;
		if( m_selVert >= 0 )
		{
			g_UIFrame->AddToParamList( g_EdWorld->GetVertProps( m_selBlock, m_selVert ) );
		}
		else if( m_selSurf >= 0 )
			g_UIFrame->AddToParamList( g_EdWorld->GetSurfProps( m_selBlock, m_selSurf ) );
		else
			g_UIFrame->AddToParamList( g_EdWorld->GetBlockProps( m_selBlock ) );
	}
}

void EdEditVertexEditMode::OnEnter()
{
}

void EdEditVertexEditMode::OnViewEvent( EDGUIEvent* e )
{
}

void EdEditVertexEditMode::Draw()
{
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
		if( m_paintBlock >= 0 && g_EdWorld->m_blocks[ m_paintBlock ].RayIntersect( cursorRayPos, cursorRayDir, outdst, outblock ) )
			m_paintSurf = outblock[0];
		if( m_isPainting )
			dopaint = true;
	}
	if( e->type == EDGUI_EVENT_KEYDOWN && !e->key.repeat && e->key.engkey == SDLK_g && m_paintBlock >= 0 && m_paintSurf >= 0 )
	{
		m_paintSurfProps.LoadParams( g_EdWorld->m_blocks[ m_paintBlock ].surfaces[ m_paintSurf ] );
	}
	if( dopaint && m_paintBlock >= 0 && m_paintSurf >= 0 )
	{
		m_paintSurfProps.BounceBack( g_EdWorld->m_blocks[ m_paintBlock ].surfaces[ m_paintSurf ] );
		g_EdWorld->m_blocks[ m_paintBlock ].RegenerateMesh();
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
	
	EdEntity* N = m_entityProps->Clone();
	N->SetPosition( V3( pos.x, pos.y, N->Pos().z ) );
	N->RegenerateMesh();
	g_EdWorld->m_entities.push_back( N );
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
		EdEntity* N = g_EdWorld->m_entities[ m_selEnt ]->Clone();
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


