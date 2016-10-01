

#include "mapedit.hpp"



Vec2 ED_GetCursorPos()
{
	MapEditorRenderView& rv = g_UIFrame->m_NUIRenderView;
	Vec2 cp = Game_GetCursorPos() - V2( rv.m_vp.x0, rv.m_vp.y0 );
	return V2
	(
		cp.x / ( rv.m_vp.x1 - rv.m_vp.x0 ) * 2 - 1,
		cp.y / ( rv.m_vp.y1 - rv.m_vp.y0 ) * -2 + 1
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


int EdBasicEditTransform::ViewUI()
{
	if( ImGui::IsWindowHovered() )
	{
		if( ImGui::GetIO().MouseDelta != ImVec2(0,0) )
		{
			RecalcTransform();
			ApplyTransform();
		}
		if( ImGui::IsMouseClicked( 0 ) )
		{
			g_UIFrame->SetEditTransform( NULL );
		}
		if( ImGui::IsMouseClicked( 1 ) )
		{
			RestoreState();
			g_UIFrame->SetEditTransform( NULL );
		}
	}
	return 1;
}

EdBlockEditTransform::EdBlockEditTransform() :
	m_xfmode( Translate ),
	m_cmode( Camera ),
	m_origin( V3(0) ),
	m_subpointCenter(false),
	m_xtdMask( V3(0) )
{
	m_xtdAABB[0] = V3(0);
	m_xtdAABB[1] = V3(0);
}

bool EdBlockEditTransform::OnEnter()
{
	if( g_EdWorld->GetNumSelectedObjects() == 0 )
		return false;
	m_cmode = Camera;
	return EdEditTransform::OnEnter();
}

int EdBlockEditTransform::ViewUI()
{
	if( ImGui::IsWindowFocused() )
	{
		if( ImGui::IsKeyPressed( SDLK_x ) )
		{
			if( m_cmode == XAxis ) m_cmode = XPlane;
			else if( m_cmode == XPlane ) m_cmode = Camera;
			else m_cmode = XAxis;
			RecalcTransform();
			ApplyTransform();
		}
		if( ImGui::IsKeyPressed( SDLK_y ) )
		{
			if( m_cmode == YAxis ) m_cmode = YPlane;
			else if( m_cmode == YPlane ) m_cmode = Camera;
			else m_cmode = YAxis;
			RecalcTransform();
			ApplyTransform();
		}
		if( ImGui::IsKeyPressed( SDLK_z ) )
		{
			if( m_cmode == ZAxis ) m_cmode = ZPlane;
			else if( m_cmode == ZPlane ) m_cmode = Camera;
			else m_cmode = ZAxis;
			RecalcTransform();
			ApplyTransform();
		}
	}
	return EdBasicEditTransform::ViewUI();
}

void EdBlockEditTransform::SaveState()
{
	Vec3 cp = V3(0);
	int cpc = 0;
	m_objStateMap.clear();
	m_objectStateData.clear();
	for( size_t i = 0; i < g_EdWorld->m_selection.size(); ++i )
	{
		EdObjIdx idx = g_EdWorld->m_selection.item( i ).key;
		SavedObject SO = { idx, m_objectStateData.size() };
		m_objStateMap.push_back( SO );
		ByteWriter bw( &m_objectStateData );
		
		EdObject* obj = idx.GetEdObject();
		GameObject* go = idx.GetGameObject();
		
		if( obj )
		{
			obj->Serialize( bw );
			
			if( m_subpointCenter )
			{
				int numel = obj->GetNumElements();
				for( int el = 0; el < numel; ++el )
				{
					if( obj->IsElementSelected( el ) )
					{
						cp += obj->GetElementPoint( el );
						cpc++;
					}
				}
			}
			else
			{
				cp += obj->FindCenter();
				cpc++;
			}
		}
		else if( go )
		{
			EDGO_SerializeSpatial( go, bw );
			
			cp += go->GetWorldPosition();
			cpc++;
		}
	}
	m_origin = g_UIFrame->Snapped( cp / cpc );
}

void EdBlockEditTransform::RestoreState()
{
	for( size_t i = 0; i < m_objStateMap.size(); ++i )
	{
		const SavedObject& SO = m_objStateMap[ i ];
		ByteReader br( m_objectStateData, SO.offset );
		
		EdObject* obj = SO.idx.GetEdObject();
		GameObject* go = SO.idx.GetGameObject();
		
		if( obj )
		{
			obj->Serialize( br );
			obj->RegenerateMesh();
		}
		else if( go )
			EDGO_SerializeSpatial( go, br );
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

Mat4 EdBlockEditTransform::GetRotationMatrix( const Vec2& a, const Vec2& b )
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
	
	Vec2 sso = ED_GetScreenPos( m_origin );
	float angleA = ( a - sso ).Angle();
	float angleB = ( b - sso ).Angle();
	float angleDiff = ( angleB - angleA ) * -sign( Vec3Dot( g_EdScene->camera.direction, axis ) );
	g_UIFrame->m_snapProps.SnapAngleRad( angleDiff );
	
	return Mat4::CreateTranslation( -m_origin )
		* Mat4::CreateRotationAxisAngle( axis, angleDiff )
		* Mat4::CreateTranslation( m_origin );
}

int EdBlockMoveTransform::ViewUI()
{
	int x0 = g_UIFrame->m_NUIRenderView.m_vp.x0;
	int y1 = g_UIFrame->m_NUIRenderView.m_vp.y1;
	char bfr[ 1024 ];
	const char* actions[] = { "Moving", "Extending", "Rotating" };
	// TODO: more detail?
	sgrx_snprintf( bfr, 1024, "%s blocks: %g ; %g ; %g", actions[ m_xfmode ], m_transform.x, m_transform.y, m_transform.z );
	
	ImDrawList* idl = ImGui::GetWindowDrawList();
	idl->PushClipRectFullScreen();
	idl->AddRectFilled( ImVec2( x0, y1 - 16 ), ImVec2( x0 + 200, y1 ), ImColor(0.f,0.f,0.f,0.5f), 8.0f, 0x2 );
	ImGui::SetCursorScreenPos( ImVec2( x0 + 4, y1 - 16 - 1 ) );
	ImGui::Text( bfr );
	idl->PopClipRect();
	
	return EdBlockEditTransform::ViewUI();
}

void EdBlockMoveTransform::Draw()
{
	float D = 16;
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
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
	RestoreState();
	
	for( size_t i = 0; i < m_objStateMap.size(); ++i )
	{
		const SavedObject& SO = m_objStateMap[ i ];
		
		EdObject* obj = SO.idx.GetEdObject();
		GameObject* go = SO.idx.GetGameObject();
		
		// m_extend only
		Vec3 dstbb[2] = { m_xtdAABB[0], m_xtdAABB[1] };
		if( m_xtdMask.x == 0 ) dstbb[0].x += m_transform.x; else if( m_xtdMask.x == 1 ) dstbb[1].x += m_transform.x;
		if( m_xtdMask.y == 0 ) dstbb[0].y += m_transform.y; else if( m_xtdMask.y == 1 ) dstbb[1].y += m_transform.y;
		if( m_xtdMask.z == 0 ) dstbb[0].z += m_transform.z; else if( m_xtdMask.z == 1 ) dstbb[1].z += m_transform.z;
		// <<<
		
		if( obj )
		{
			switch( m_xfmode )
			{
			case Translate:
				// simple translation only
				obj->SetPosition( obj->GetPosition() + m_transform );
				break;
			case Extend:
				obj->SetPosition( ED_RemapPos( obj->GetPosition(), m_xtdAABB, dstbb ) );
				obj->ScaleVertices( TREVLERP( V3(0), m_xtdAABB[1] - m_xtdAABB[0], dstbb[1] - dstbb[0] ) );
				break;
			case Rotate:
				obj->TransformVertices( m_rotateTransform );
				break;
			}
			obj->RegenerateMesh();
		}
		else if( go )
		{
			switch( m_xfmode )
			{
			case Translate:
				go->SetWorldPosition( go->GetWorldPosition() + m_transform );
				break;
			case Extend:
				go->SetWorldPosition( ED_RemapPos( go->GetWorldPosition(), m_xtdAABB, dstbb ) );
				break;
			case Rotate:
				go->SetWorldMatrix( go->GetWorldMatrix() * m_rotateTransform );
				break;
			}
		}
	}
}

void EdBlockMoveTransform::RecalcTransform()
{
	switch( m_xfmode )
	{
	case Translate:
	case Extend:
		m_transform = g_UIFrame->Snapped( GetMovementVector( m_startCursorPos, ED_GetCursorPos() ) );
		break;
	case Rotate:
		m_rotateTransform = GetRotationMatrix( m_startCursorPos, ED_GetCursorPos() );
		m_transform = RAD2DEG( m_rotateTransform.GetXYZAngles() );
		break;
	}
}


EdVertexMoveTransform::EdVertexMoveTransform() : m_project(false)
{
	m_subpointCenter = true;
}

int EdVertexMoveTransform::ViewUI()
{
	int x0 = g_UIFrame->m_NUIRenderView.m_vp.x0;
	int y1 = g_UIFrame->m_NUIRenderView.m_vp.y1;
	char bfr[ 1024 ];
	sgrx_snprintf( bfr, 1024, "(P)rojection: [%s] | Moving vertices: %g ; %g ; %g",
		m_project ? "ON" : "OFF", m_transform.x, m_transform.y, m_transform.z );
	
	ImDrawList* idl = ImGui::GetWindowDrawList();
	idl->PushClipRectFullScreen();
	idl->AddRectFilled( ImVec2( x0, y1 - 16 ), ImVec2( x0 + 200, y1 ), ImColor(0.f,0.f,0.f,0.5f), 8.0f, 0x2 );
	ImGui::SetCursorScreenPos( ImVec2( x0 + 4, y1 - 16 - 1 ) );
	ImGui::Text( bfr );
	idl->PopClipRect();
	
	if( ImGui::IsWindowFocused() )
	{
		if( ImGui::IsKeyPressed( SDLK_p ) )
			m_project = !m_project;
	}
	
	return EdBlockMoveTransform::ViewUI();
}

void EdVertexMoveTransform::ApplyTransform()
{
	RestoreState();
	
	for( size_t i = 0; i < m_objStateMap.size(); ++i )
	{
		const SavedObject& SO = m_objStateMap[ i ];
		
		EdObject* obj = SO.idx.GetEdObject();
		if( obj )
		{
			switch( m_xfmode )
			{
			case Translate:
				obj->MoveSelectedVertices( m_transform );
				break;
			case Rotate:
				obj->TransformVertices( m_rotateTransform, true );
				break;
			case Extend: // not supported here (?)
				break;
			}
			if( m_project )
			{
				obj->ProjectSelectedVertices();
			}
			obj->RegenerateMesh();
		}
	}
}



EdDrawBlockEditMode::EdDrawBlockEditMode() :
	m_blockDrawMode( BD_Polygon ),
	m_newZ0( 0 ),
	m_newZ1( 2 )
{
}

void EdDrawBlockEditMode::OnEnter()
{
	g_UIFrame->SetCursorPlaneHeight( m_newZ0 );
	m_drawnVerts.clear();
}

void EdDrawBlockEditMode::EditUI()
{
	ImGui::Text( "Create objects" );
	ImGui::Separator();
	ImGui::Text( "Mode:" );
	ImGui::Columns( 3 );
	ImGui::RadioButton( "Polygon", &m_blockDrawMode, BD_Polygon ); ImGui::NextColumn();
	ImGui::RadioButton( "Box strip", &m_blockDrawMode, BD_BoxStrip ); ImGui::NextColumn();
	ImGui::RadioButton( "Mesh path", &m_blockDrawMode, BD_MeshPath ); ImGui::NextColumn();
	ImGui::RadioButton( "Entity", &m_blockDrawMode, BD_Entity ); ImGui::NextColumn();
	ImGui::RadioButton( "Game object", &m_blockDrawMode, BD_GameObject );
	ImGui::Columns( 1 );
	ImGui::Separator();
	
	g_UIFrame->m_snapProps.EditUI();
	if( IMGUIEditFloat( "Bottom Z", m_newZ0, -8192, 8192 ) )
		g_UIFrame->SetCursorPlaneHeight( m_newZ0 );
	IMGUIEditFloat( "Top Z", m_newZ1, -8192, 8192 );
	
	if( m_blockDrawMode == BD_Entity )
	{
		m_entGroup.CurEntity()->EditUI();
		m_entGroup.EditUI();
	}
	else if( m_blockDrawMode == BD_GameObject )
	{
		// ...
	}
	else
	{
		m_newSurf.EditUI();
	}
}

void EdDrawBlockEditMode::ViewUI()
{
	if( ImGui::IsWindowHovered() )
	{
		if( ImGui::IsMouseClicked( 0 ) && g_UIFrame->IsCursorAiming() )
		{
			if( m_blockDrawMode == BD_Entity )
			{
				_AddNewEntity();
			}
			else if( m_blockDrawMode == BD_GameObject )
			{
				GameObject* obj = g_Level->CreateGameObject();
				obj->m_src_guid = SGRX_GUID::Generate();
				obj->SetWorldPosition( g_UIFrame->GetCursorPos() );
			}
			else if( m_drawnVerts.size() < 14 )
			{
				m_drawnVerts.push_back( g_UIFrame->GetCursorPlanePos() );
			}
		}
	}
	
	if( ImGui::IsWindowFocused() )
	{
		if( ImGui::IsKeyReleased( SDLK_RETURN ) )
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
			else if( m_blockDrawMode == BD_MeshPath )
			{
				float planeHeight = g_UIFrame->GetCursorPlaneHeight();
				EdMeshPath MP;
				for( size_t i = 0; i < m_drawnVerts.size(); ++i )
				{
					EdMeshPathPoint P;
					P.pos = V3( m_drawnVerts[ i ].x, m_drawnVerts[ i ].y, planeHeight );
					MP.m_points.push_back( P );
				}
				Vec3 c = g_UIFrame->Snapped( MP.FindCenter() );
				for( size_t i = 0; i < MP.m_points.size(); ++i )
					MP.m_points[ i ].pos -= c;
				MP.m_position = c;
				g_EdWorld->AddObject( MP.Clone() );
				m_drawnVerts.clear();
			}
		}
		if( ImGui::IsKeyReleased( SDLK_ESCAPE ) )
		{
			m_drawnVerts.clear();
		}
		if( ImGui::IsKeyReleased( SDLK_BACKSPACE ) && m_drawnVerts.size() )
		{
			m_drawnVerts.pop_back();
		}
		
		bool altdown = ImGui::GetIO().KeyAlt;
	//	bool shiftdown = ImGui::GetIO().KeyShift;
		bool ctrldown = ImGui::GetIO().KeyCtrl;
		if( !ctrldown && !altdown )
		{
			if( ImGui::IsKeyPressed( SDLK_1, false ) ) m_blockDrawMode = BD_Polygon;
			if( ImGui::IsKeyPressed( SDLK_2, false ) ) m_blockDrawMode = BD_BoxStrip;
			if( ImGui::IsKeyPressed( SDLK_3, false ) ) m_blockDrawMode = BD_MeshPath;
			if( ImGui::IsKeyPressed( SDLK_4, false ) ) m_blockDrawMode = BD_Entity;
			if( ImGui::IsKeyPressed( SDLK_5, false ) ) m_blockDrawMode = BD_GameObject;
		}
	}
}

void EdDrawBlockEditMode::Draw()
{
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
	
	float planeHeight = g_UIFrame->GetCursorPlaneHeight();
	
	if( m_blockDrawMode == BD_Entity )
	{
		g_EdWorld->DrawWires_Entities( EdObjIdx() );
	}
	else if( m_blockDrawMode == BD_GameObject )
	{
		g_EdWorld->DrawWires_GameObjects( EdObjIdx() );
	}
	else
	{
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
			if( m_blockDrawMode == BD_MeshPath )
			{
				br.SetPrimitiveType( PT_LineStrip );
				for( size_t i = 0; i < m_drawnVerts.size(); ++i )
				br.Pos( m_drawnVerts[ i ], planeHeight );
			}
			else
			{
				br.PolyOutline( m_drawnVerts.data(), m_drawnVerts.size(), planeHeight, sizeof(*m_drawnVerts.data()) );
			}
		}
		for( size_t i = 0; i < m_drawnVerts.size(); ++i )
		{
			br.Col( 0.9f, 0.1f, 0, 0.8f );
			br.CircleOutline( m_drawnVerts[i].x, m_drawnVerts[i].y, 0.02f, planeHeight, 16 );
		}
		if( ( m_blockDrawMode == BD_Polygon || m_blockDrawMode == BD_MeshPath ) &&
			g_UIFrame->IsCursorAiming() )
		{
			Vec2 pos = g_UIFrame->GetCursorPlanePos();
			if( m_drawnVerts.size() > 1 )
			{
				br.Col( 0.9f, 0.1f, 0, 0.4f ).SetPrimitiveType( PT_LineStrip );
				br.Pos( m_drawnVerts.last(), planeHeight );
				br.Pos( pos, planeHeight );
				if( m_blockDrawMode == BD_Polygon )
					br.Pos( m_drawnVerts[0], planeHeight );
			}
		}
	}
	g_UIFrame->DrawCursor();
}

void EdDrawBlockEditMode::_AddNewBlock()
{
	EdBlock B;
	B.position = V3(0);
	B.z0 = m_newZ0;
	B.z1 = m_newZ1;
	B.poly.resize( m_drawnVerts.size() );
	for( size_t i = 0; i < m_drawnVerts.size(); ++i )
		B.poly[ i ] = V3( m_drawnVerts[ i ].x, m_drawnVerts[ i ].y, 0 );
	if( PolyArea( m_drawnVerts.data(), m_drawnVerts.size() ) < 0 )
		B.poly.reverse();
	B.GenCenterPos( g_UIFrame->m_snapProps );
	for( size_t i = 0; i < m_drawnVerts.size() + 2; ++i )
	{
		B.surfaces.push_back( new EdSurface( m_newSurf ) );
	}
	B.subsel.resize( B.GetNumElements() );
	B.ClearSelection();
	g_EdWorld->AddObject( B.Clone() );
}

void EdDrawBlockEditMode::_AddNewEntity()
{
	EdEntity* N = (EdEntity*) m_entGroup.CurEntity()->Clone();
	N->SetPosition( g_UIFrame->GetCursorPos() );
	g_EdWorld->SetEntityID( N );
	g_EdWorld->AddObject( N );
}


EdEditBlockEditMode::EdEditBlockEditMode() :
	m_selMask( SelMask_ALL )
{}

void EdEditBlockEditMode::OnEnter()
{
	m_numSel = g_EdWorld->GetNumSelectedObjects();
	m_curObj = g_EdWorld->GetOnlySelectedObject();
	g_EdWorld->GetSelectedObjectAABB( m_selAABB );
	_ReloadBlockProps();
}

void EdEditBlockEditMode::OnTransformEnd()
{
	_ReloadBlockProps();
}

void EdEditBlockEditMode::ViewUI()
{
	_MouseMove();
	
	bool lmbdown = ImGui::IsMouseDown( 0 );
	bool altdown = ImGui::GetIO().KeyAlt;
	bool shiftdown = ImGui::GetIO().KeyShift;
	bool ctrldown = ImGui::GetIO().KeyCtrl;
	
	m_hlBBEl = GetClosestActivePoint();
	
	if( ImGui::IsWindowHovered() )
	{
		if( ImGui::IsMouseClicked( 0 ) && !altdown )
		{
			g_EdWorld->SelectObject( m_hlObj, ctrldown ? SELOBJ_TOGGLE : SELOBJ_ONLY );
			m_numSel = g_EdWorld->GetNumSelectedObjects();
			m_curObj = g_EdWorld->GetOnlySelectedObject();
			_ReloadBlockProps();
		}
		
		if( lmbdown && altdown )
		{
			g_EdWorld->SelectObject( m_hlObj, shiftdown ? SELOBJ_ENABLE : SELOBJ_DISABLE );
		}
	}
	
	if( ImGui::IsWindowFocused() )
	{
		// SELECT ALL/NONE
		if( ImGui::IsKeyPressed( SDLK_a, false ) && ctrldown )
		{
			if( altdown )
				g_EdWorld->m_selection.clear();
			else
			{
				EdObjIdxArray eoiarr;
				g_EdWorld->GetAllObjects( eoiarr );
				for( size_t i = 0; i < eoiarr.size(); ++i )
					g_EdWorld->m_selection.set( eoiarr[ i ], NoValue() );
			}
		}
		// GRAB (MOVE)
		if( ImGui::IsKeyPressed( SDLK_g, false ) )
		{
			m_transform.m_xfmode = EdBlockEditTransform::Translate;
			g_UIFrame->SetEditTransform( &m_transform );
		}
		// ROTATE
		if( ImGui::IsKeyPressed( SDLK_r, false ) )
		{
			m_transform.m_xfmode = EdBlockEditTransform::Rotate;
			g_UIFrame->SetEditTransform( &m_transform );
		}
		// EXTEND
		if( ImGui::IsKeyPressed( SDLK_e, false ) && m_hlBBEl != -1 )
		{
			m_transform.m_xfmode = EdBlockEditTransform::Extend;
			m_transform.m_xtdAABB[0] = m_selAABB[0];
			m_transform.m_xtdAABB[1] = m_selAABB[1];
			m_transform.m_xtdMask = GetActivePointFactor( m_hlBBEl );
			g_UIFrame->SetEditTransform( &m_transform );
		}
		// DELETE
		if( ImGui::IsKeyPressed( SDLK_DELETE, false ) )
		{
			g_EdWorld->DeleteSelectedObjects();
			m_curObj = EdObjIdx();
			_ReloadBlockProps();
		}
		// DUPLICATE
		if( ImGui::IsKeyPressed( SDLK_d, false ) && ctrldown )
		{
			if( g_EdWorld->DuplicateSelectedObjectsAndMoveSelection() )
			{
				m_curObj = g_EdWorld->GetOnlySelectedObject();
				_ReloadBlockProps();
				m_transform.m_xfmode = EdBlockEditTransform::Translate;
				g_UIFrame->SetEditTransform( &m_transform );
			}
		}
		
		// MOVE BACK
		if( ImGui::IsKeyPressed( SDL_SCANCODE_UP, false ) && altdown ) _Do( SA_MoveBack );
		// MOVE FORWARD
		if( ImGui::IsKeyPressed( SDL_SCANCODE_DOWN, false ) && altdown ) _Do( SA_MoveFwd );
		
		// SELECTION
		if( !ctrldown && !altdown )
		{
			if( ImGui::IsKeyPressed( SDLK_1, false ) ) m_selMask ^= SelMask_Blocks;
			if( ImGui::IsKeyPressed( SDLK_2, false ) ) m_selMask ^= SelMask_Patches;
			if( ImGui::IsKeyPressed( SDLK_3, false ) ) m_selMask ^= SelMask_Entities;
			if( ImGui::IsKeyPressed( SDLK_4, false ) ) m_selMask ^= SelMask_MeshPaths;
			if( ImGui::IsKeyPressed( SDLK_5, false ) ) m_selMask ^= SelMask_GameObjects;
		}
	}
	
	// TO VERTEX MODE
	if( ImGui::IsKeyPressed( SDLK_v, false ) && altdown )
	{
		g_UIFrame->SetEditMode( &g_UIFrame->m_emEditVertex );
	}
	// TO PAINT MODE
	if( ImGui::IsKeyPressed( SDLK_q, false ) && altdown )
	{
		g_UIFrame->SetEditMode( &g_UIFrame->m_emPaintVerts );
	}
	
	// PAINT
	{
		ImVec2 r0 = ImGui::GetWindowPos() + ImVec2( 1, 1 );
		ImDrawList* idl = ImGui::GetWindowDrawList();
		char bfr[ 1024 ];
		
		idl->PushClipRectFullScreen();
		int bgRectHeight = 32 + ( m_numSel && m_hlBBEl != -1 ? 16 : 0 );
		idl->AddRectFilled( r0, r0 + ImVec2( 610, bgRectHeight ), ImColor( 0.f, 0.f, 0.f, 0.5f ), 8.0f, 0x4 );
		
		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2(0,0) );
		ImGui::SetCursorScreenPos( r0 + ImVec2( 4, -1 ) );
		ImGui::Text( "Vertex mode [Alt+V], Paint mode [Alt+Q], Select all [Ctrl+A], Select none [Ctrl+Alt+A]" );
		ImGui::SetCursorScreenPos( r0 + ImVec2( 4, 16 -1 ) );
		ImGui::Text( "Selection mask: [Blocks: " );
		ImGui::SameLine(); IMGUIYesNo( ( m_selMask & SelMask_Blocks ) != 0 );
		ImGui::SameLine(); ImGui::Text( ", Patches: " );
		ImGui::SameLine(); IMGUIYesNo( ( m_selMask & SelMask_Patches ) != 0 );
		ImGui::SameLine(); ImGui::Text( ", Entities: " );
		ImGui::SameLine(); IMGUIYesNo( ( m_selMask & SelMask_Entities ) != 0 );
		ImGui::SameLine(); ImGui::Text( ", Mesh paths: " );
		ImGui::SameLine(); IMGUIYesNo( ( m_selMask & SelMask_MeshPaths ) != 0 );
		ImGui::SameLine(); ImGui::Text( ", Game objects: " );
		ImGui::SameLine(); IMGUIYesNo( ( m_selMask & SelMask_GameObjects ) != 0 );
		ImGui::SameLine(); ImGui::Text( "]" );
		
		if( m_numSel )
		{
			ImGui::SetCursorScreenPos( r0 + ImVec2( 4, 32 -1 ) );
			sgrx_snprintf( bfr, 1024, "Press E to extend selection along %s%s%s",
				GetActivePointExtName( m_hlBBEl ),
				_CanDo( SA_MoveBack ) ? ", Move back [Alt+Up]" : "",
				_CanDo( SA_MoveFwd ) ? ", Move forward [Alt+Down]" : "" );
			ImGui::Text( bfr );
		}
		ImGui::PopStyleVar();
		
		idl->PopClipRect();
	}
}

void EdEditBlockEditMode::EditUI()
{
	ImGui::Text( "Edit objects" );
	ImGui::Separator();
	ImGui::Text( "Selection mask:" );
	ImGui::Columns( 3 );
	ImGui::CheckboxFlags( "Blocks", &m_selMask, SelMask_Blocks ); ImGui::NextColumn();
	ImGui::CheckboxFlags( "Patches", &m_selMask, SelMask_Patches ); ImGui::NextColumn();
	ImGui::CheckboxFlags( "Entities", &m_selMask, SelMask_Entities ); ImGui::NextColumn();
	ImGui::CheckboxFlags( "Mesh paths", &m_selMask, SelMask_MeshPaths ); ImGui::NextColumn();
	ImGui::CheckboxFlags( "Game objects", &m_selMask, SelMask_GameObjects );
	ImGui::Columns( 1 );
	ImGui::Separator();
	g_UIFrame->m_snapProps.EditUI();
	ImGui::Separator();
	
	m_moprops.EditUI();
	ImGui::Separator();
	
	ImGui::BeginChangeCheck();
	
	if( m_curObj.Valid() )
	{
		if( m_curObj.type == ObjType_GameObject )
			EDGO_EditUI( m_curObj.GetGameObject() );
		else
			m_curObj.GetEdObject()->EditUI();
	}
	
	if( ImGui::EndChangeCheck() )
	{
		g_EdWorld->GetSelectedObjectAABB( m_selAABB );
	}
}

void EdEditBlockEditMode::Draw()
{
	g_EdWorld->DrawWires_Objects( m_hlObj );
	
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

bool EdEditBlockEditMode::_CanDo( ESpecialAction act )
{
	switch( act )
	{
	case SA_MoveBack:
	case SA_MoveFwd:
		break;
		
	default:
		break;
	}
	return false;
}

void EdEditBlockEditMode::_Do( ESpecialAction act )
{
	switch( act )
	{
	case SA_MoveBack:
	case SA_MoveFwd:
		break;
		
	default:
		break;
	}
}

void EdEditBlockEditMode::_MouseMove()
{
	g_EdWorld->RayObjectsIntersect( g_UIFrame->GetCursorRayPos(), g_UIFrame->GetCursorRayDir(), m_curObj, NULL, &m_hlObj, NULL, m_selMask );
	m_hlBBEl = GetClosestActivePoint();
}

void EdEditBlockEditMode::_ReloadBlockProps()
{
	g_EdWorld->GetSelectedObjectAABB( m_selAABB );
	m_hlBBEl = GetClosestActivePoint();
	m_moprops.Prepare( false );
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
	if( i < 0 || i >= NUM_AABB_ACTIVE_POINTS )
		return "-";
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
	if( i < 0 || i >= NUM_AABB_ACTIVE_POINTS )
		return false;
	
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
	m_hlAP.block = EdObjIdx();
	m_hlAP.point = -1;
	
	m_selObjList.clear();
	for( size_t i = 0; i < g_EdWorld->m_selection.size(); ++i )
	{
		m_selObjList.push_back( g_EdWorld->m_selection.item( i ).key );
	}
}

bool EdEditVertexEditMode::_CanDo( ESpecialAction act )
{
	for( size_t b = 0; b < m_selObjList.size(); ++b )
	{
		EdObject* obj = m_selObjList[ b ].GetEdObject();
		if( obj && obj->CanDoSpecialAction( act ) )
			return true;
	}
	return false;
}

void EdEditVertexEditMode::_Do( ESpecialAction act )
{
	for( size_t b = 0; b < m_selObjList.size(); ++b )
	{
		EdObject* obj = m_selObjList[ b ].GetEdObject();
		if( !obj )
			continue;
		switch( act )
		{
		case SA_Extend:
			// move afterwards
			if( obj->CanDoSpecialAction( act ) == false )
				break;
			
			obj->SpecialAction( act );
			m_transform.m_xfmode = EdBlockEditTransform::Translate;
			g_UIFrame->SetEditTransform( &m_transform );
			OnEnter(); // refresh selection
			break;
			
		case SA_ExtractPart:
		case SA_DuplicatePart:
			// do only for one
			if( obj->CanDoSpecialAction( act ) )
			{
				obj->SpecialAction( act );
				return;
			}
			
		default:
			obj->SpecialAction( act );
			break;
		}
	}
}

void EdEditVertexEditMode::ViewUI()
{
	m_hlAP = GetClosestActivePoint();
	
	bool altdown = ImGui::GetIO().KeyAlt;
	bool shiftdown = ImGui::GetIO().KeyShift;
	bool ctrldown = ImGui::GetIO().KeyCtrl;
	
	if( ImGui::IsWindowHovered() && ImGui::IsMouseClicked( 0 ) )
	{
		for( size_t b = 0; b < m_selObjList.size(); ++b )
		{
			EdObjIdx idx = m_selObjList[ b ];
			EdObject* obj = idx.GetEdObject();
			if( obj )
			{
				if( idx == m_hlAP.block )
					obj->UISelectElement( m_hlAP.point, ctrldown );
				else if( !ctrldown )
					obj->ClearSelection();
			}
		}
	}
	
	if( ImGui::IsWindowFocused() )
	{
		ESpecialAction satype = SA_None;
		if( ImGui::IsKeyPressed( SDLK_i ) ) satype = SA_Invert;
		if( ImGui::IsKeyPressed( SDLK_RIGHTBRACKET ) ) satype = SA_Subdivide;
		if( ImGui::IsKeyPressed( SDLK_LEFTBRACKET ) ) satype = SA_Unsubdivide;
		if( ImGui::IsKeyPressed( SDLK_f ) ) satype = SA_EdgeFlip;
		if( satype != SA_None )
		{
			for( size_t b = 0; b < m_selObjList.size(); ++b )
			{
				if( m_selObjList[ b ].GetEdObject() )
					m_selObjList[ b ].GetEdObject()->SpecialAction( satype );
			}
		}
		
		// SELECT ALL/NONE
		if( ImGui::IsKeyPressed( SDLK_a ) && ctrldown )
		{
			bool sel = !altdown;
			for( size_t b = 0; b < m_selObjList.size(); ++b )
			{
				EdObject* obj = m_selObjList[ b ].GetEdObject();
				if( obj )
				{
					int numels = obj->GetNumElements();
					for( int i = 0; i < numels; ++i )
						obj->SelectElement( i, sel );
				}
			}
		}
		
		// EXTEND
		if( ImGui::IsKeyPressed( SDLK_e ) ) _Do( SA_Extend );
		// REMOVE
		if( ImGui::IsKeyPressed( SDLK_DELETE ) ) _Do( SA_Remove );
		// EXTRACT PART
		if( ImGui::IsKeyPressed( SDLK_x ) ) _Do( SA_ExtractPart );
		// DUPLICATE PART
		if( ImGui::IsKeyPressed( SDLK_d ) && ctrldown ) _Do( SA_DuplicatePart );
		// SURFS TO PATCHES
		if( ImGui::IsKeyPressed( SDLK_s ) && altdown ) _Do( SA_SurfsToPatches );
		// ROTATE (CCW)
		if( ImGui::IsKeyPressed( SDLK_9 ) && shiftdown ) _Do( SA_RotateCCW );
		// ROTATE (CW)
		if( ImGui::IsKeyPressed( SDLK_0 ) && shiftdown ) _Do( SA_RotateCW );
		
		// GRAB (MOVE)
		if( ImGui::IsKeyPressed( SDLK_g ) )
		{
			m_transform.m_xfmode = EdBlockEditTransform::Translate;
			g_UIFrame->SetEditTransform( &m_transform );
		}
		// ROTATE
		if( ImGui::IsKeyPressed( SDLK_r ) )
		{
			m_transform.m_xfmode = EdBlockEditTransform::Rotate;
			g_UIFrame->SetEditTransform( &m_transform );
		}
		// TO BLOCK MODE
		if( ImGui::IsKeyPressed( SDLK_b ) && altdown )
		{
			g_UIFrame->SetEditMode( &g_UIFrame->m_emEditObjs );
		}
		// TO PAINT MODE
		if( ImGui::IsKeyPressed( SDLK_q ) && altdown )
		{
			g_UIFrame->SetEditMode( &g_UIFrame->m_emPaintVerts );
		}
	}
	
	
	const char* acts0 = "Block mode [Alt+B], Paint mode [Alt+Q], Select all [Ctrl+A], Select none [Ctrl+Alt+A]";
	String actlist = "Grab [G]";
	if( _CanDo( SA_Invert ) ) actlist.append( ", Invert patch [I]" );
	if( _CanDo( SA_Subdivide ) ) actlist.append( ", Subdivide [']']" );
	if( _CanDo( SA_Unsubdivide ) ) actlist.append( ", Unsubdivide ['[']" );
	if( _CanDo( SA_EdgeFlip ) ) actlist.append( ", Edge flip [F]" );
	if( _CanDo( SA_Extend ) ) actlist.append( ", Extend [E]" );
	if( _CanDo( SA_Remove ) ) actlist.append( ", Remove [Del]" );
	if( _CanDo( SA_ExtractPart ) ) actlist.append( ", Extract part [X]" );
	if( _CanDo( SA_DuplicatePart ) ) actlist.append( ", Duplicate part [Ctrl+D]" );
	if( _CanDo( SA_SurfsToPatches ) ) actlist.append( ", Surfaces to patches [Alt+S]" );
	if( _CanDo( SA_RotateCCW ) ) actlist.append( ", Rotate (CCW) [Shift+9/'(']" );
	if( _CanDo( SA_RotateCW ) ) actlist.append( ", Rotate (CW) [Shift+0/')']" );
	
	ImVec2 r0 = ImGui::GetWindowPos() + ImVec2( 1, 1 );
	ImDrawList* idl = ImGui::GetWindowDrawList();
	
	idl->PushClipRectFullScreen();
	idl->AddRectFilled( r0, r0 + ImVec2( 610, 32 ), ImColor( 0.f, 0.f, 0.f, 0.5f ), 8.0f, 0x4 );
	
	ImGui::SetCursorScreenPos( r0 + ImVec2( 4, -1 ) );
	ImGui::Text( acts0 );
	ImGui::SetCursorScreenPos( r0 + ImVec2( 4, 16 -1 ) );
	actlist.push_back( '\0' );
	ImGui::Text( actlist.data() );
	
	idl->PopClipRect();
}

void EdEditVertexEditMode::EditUI()
{
	// determine if anything can be shown
	int numsurfselblocks = 0;
	int numsurfexblocks = 0;
	ActivePoint surfprops = { EdObjIdx(), -1 };
	ActivePoint vertprops = { EdObjIdx(), -1 };
	for( size_t b = 0; b < m_selObjList.size(); ++b )
	{
		EdObjIdx idx = m_selObjList[ b ];
		EdObject* obj = idx.GetEdObject();
		
		if( obj )
		{
			int nss = obj->GetNumSelectedSurfs();
			numsurfselblocks += nss == 1;
			numsurfexblocks += nss == 0 || nss == 1;
			
			int surf = obj->GetOnlySelectedSurface();
			if( surf != -1 )
			{
				if( surfprops.block.Valid() == false && surfprops.point == -1 )
				{
					surfprops.block = idx;
					surfprops.point = surf;
				}
				else
				{
					surfprops.block = EdObjIdx();
				}
			}
			
			int vert = obj->GetOnlySelectedVertex();
			if( vert != -1 )
			{
				if( vertprops.block.Valid() == false && vertprops.point == -1 )
				{
					vertprops.block = idx;
					vertprops.point = vert;
				}
				else
				{
					vertprops.block = EdObjIdx();
				}
			}
		}
	}
	
	m_canExtendSurfs = numsurfexblocks && numsurfselblocks;
	
	ImGui::Text( "Edit vertices/surfaces" );
	ImGui::Separator();
	g_UIFrame->m_snapProps.EditUI();
	m_moprops.Prepare( true );
	m_moprops.EditUI();
	
	if( surfprops.block.Valid() )
	{
		ImGui::Separator();
		g_EdWorld->SurfEditUI( surfprops.block, surfprops.point );
	}
	
	if( vertprops.block.Valid() )
	{
		ImGui::Separator();
		g_EdWorld->VertEditUI( vertprops.block, vertprops.point );
	}
}

void EdEditVertexEditMode::Draw()
{
	g_EdWorld->DrawWires_Objects( m_hlAP.block );
	
	if( g_UIFrame->m_editTF )
		return;
	
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
	for( size_t b = 0; b < m_selObjList.size(); ++b )
	{
		EdObjIdx idx = m_selObjList[ b ];
		EdObject* obj = idx.GetEdObject();
		if( obj )
		{
			int bpcount = obj->GetNumElements();
			for( int i = 0; i < bpcount; ++i )
			{
				if( obj->IsElementSelected( i ) )
				{
					if( idx == m_hlAP.block && i == m_hlAP.point )
						br.Col( 0.9f, 0.8f, 0.1f, 1 );
					else
						br.Col( 0.9f, 0.5f, 0.1f, 1 );
				}
				else
				{
					if( idx == m_hlAP.block && i == m_hlAP.point )
						br.Col( 0.1f, 0.8f, 0.9f, 1 );
					else
						br.Col( 0.1f, 0.2f, 0.4f, 1 );
				}
				
				Vec3 pp = obj->GetElementPoint( i );
				br.Sprite( pp, 0.02f, 0.02f );
			}
		}
	}
}

EdEditVertexEditMode::ActivePoint EdEditVertexEditMode::GetClosestActivePoint()
{
	ActivePoint np = { EdObjIdx(), -1 };
	float minxydist = FLT_MAX;
	Vec2 scp = ED_GetCursorPos();
	Vec3 campos = g_EdScene->camera.position;
	Vec3 camdir = g_EdScene->camera.direction;
	for( size_t b = 0; b < m_selObjList.size(); ++b )
	{
		EdObjIdx idx = m_selObjList[ b ];
		EdObject* obj = idx.GetEdObject();
		if( obj )
		{
			int bpcount = obj->GetNumElements();
			for( int i = 0; i < bpcount; ++i )
			{
				Vec3 ap = obj->GetElementPoint( i );
				Vec2 sap = ED_GetScreenPos( ap );
				
				float curxydist = ( scp - sap ).Length();
				float curzdist = Vec3Dot( ap, camdir ) - Vec3Dot( campos, camdir );
				if( curzdist > 0 && curxydist < minxydist )
				{
					np.block = idx;
					np.point = i;
					minxydist = curxydist;
				}
			}
		}
	}
	return np;
}


EdPaintVertsEditMode::EdPaintVertsEditMode() :
	m_isPainting( false )
{
}

void EdPaintVertsEditMode::OnEnter()
{
	m_isPainting = false;
	
	m_selObjList.clear();
	int pcount = 0;
	for( size_t i = 0; i < g_EdWorld->m_selection.size(); ++i )
	{
		EdObjIdx idx = g_EdWorld->m_selection.item( i ).key;
		if( idx.GetEdObject() )
			pcount += idx.GetEdObject()->GetNumPaintVerts();
		m_selObjList.push_back( idx );
	}
	m_originalVerts.resize( pcount );
	
	_TakeSnapshot();
}

void EdPaintVertsEditMode::ViewUI()
{
	bool altdown = ImGui::GetIO().KeyAlt;
	bool hov = ImGui::IsWindowHovered();
	m_isPainting = hov && ImGui::IsMouseDown( 0 );
	bool dopaint = hov && ( ImGui::IsMouseClicked( 0 ) || ImGui::GetMouseDragDelta( 0, 0 ) != ImVec2(0,0) );
	if( hov )
	{
		if( ImGui::IsMouseReleased( 0 ) )
		{
			_TakeSnapshot();
		}
	}
	if( ImGui::IsWindowFocused() )
	{
		// TO BLOCK MODE
		if( ImGui::IsKeyPressed( SDLK_b ) && altdown )
		{
			g_UIFrame->SetEditMode( &g_UIFrame->m_emEditObjs );
		}
		// TO VERTEX MODE
		if( ImGui::IsKeyPressed( SDLK_v ) && altdown )
		{
			g_UIFrame->SetEditMode( &g_UIFrame->m_emEditVertex );
		}
	}
	
	const char* acts0 = "Block mode [Alt+B], Vertex mode [Alt+V], Hold Alt to reverse painting";
	ImVec2 r0 = ImGui::GetWindowPos() + ImVec2( 1, 1 );
	ImDrawList* idl = ImGui::GetWindowDrawList();
	idl->PushClipRectFullScreen();
	idl->AddRectFilled( r0, r0 + ImVec2( 610, 32 ), ImColor( 0.f, 0.f, 0.f, 0.5f ), 8.0f, 0x4 );
	ImGui::SetCursorScreenPos( r0 + ImVec2( 4, -1 ) );
	ImGui::Text( acts0 );
	idl->PopClipRect();
	
	if( dopaint )
		_DoPaint();
}

void EdPaintVertsEditMode::EditUI()
{
	ImGui::BeginChangeCheck();
	
	m_paintProps.EditUI();
	
	uint32_t cc = ImGui::EndChangeCheck();
	if( cc & 0x2 )
	{
		_TakeSnapshot();
	}
}

void EdPaintVertsEditMode::Draw()
{
	g_EdWorld->DrawWires_Objects( EdObjIdx(), true );
	
	if( g_UIFrame->m_editTF )
		return;
	
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
	
	Vec3 pos = V3(FLT_MAX);
	Vec3 cursorRayPos = g_UIFrame->GetCursorRayPos();
	Vec3 cursorRayDir = g_UIFrame->GetCursorRayDir();
	float outdst[1];
	if( g_EdWorld->RayObjectsIntersect( cursorRayPos, cursorRayDir, EdObjIdx(), outdst, NULL, NULL, SelMask_Patches ) )
	{
		pos = cursorRayPos + cursorRayDir * outdst[0];
		br.Col( 0.9f, 0.1f, 0.01f, 0.5f );
		br.SphereOutline( pos, m_paintProps.radius * powf( 0.5f, m_paintProps.falloff ), 32 );
		br.Col( 0.9f, 0.1f, 0.01f );
		br.SphereOutline( pos, m_paintProps.radius, 32 );
	}
	
	for( size_t i = 0; i < m_selObjList.size(); ++i )
	{
		EdObject* obj = m_selObjList[ i ].GetEdObject();
		if( obj )
		{
			int vcount = obj->GetNumPaintVerts();
			for( int v = 0; v < vcount; ++v )
			{
				Vec3 vpos;
				Vec4 vcol;
				obj->GetPaintVertex( v, 0, vpos, vcol );
				float q = m_paintProps.GetDistanceFactor( vpos, pos );
				if( q > 0 )
					br.Col( q, 0, 0, 1 );
				else
					br.Col( 0.1f, 0.2f, 0.4f, 1 );
				br.Sprite( vpos, 0.02f, 0.02f );
			}
		}
	}
}

void EdPaintVertsEditMode::_TakeSnapshot()
{
	int layer_id = m_paintProps.layerNum;
	int off = 0;
	for( size_t i = 0; i < m_selObjList.size(); ++i )
	{
		EdObject* obj = m_selObjList[ i ].GetEdObject();
		if( obj )
		{
			int vcount = obj->GetNumPaintVerts();
			for( int v = 0; v < vcount; ++v )
			{
				PaintVertex pv = { V3(0), V4(1), 0 };
				obj->GetPaintVertex( v, layer_id, pv.pos, pv.col );
				m_originalVerts[ off++ ] = pv;
			}
		}
	}
}

void EdPaintVertsEditMode::_DoPaint()
{
	Vec3 cursorRayPos = g_UIFrame->GetCursorRayPos();
	Vec3 cursorRayDir = g_UIFrame->GetCursorRayDir();
	
	float outdst[1];
	if( g_EdWorld->RayObjectsIntersect( cursorRayPos, cursorRayDir, EdObjIdx(), outdst, NULL, NULL, SelMask_Patches ) )
	{
		Vec3 pos = cursorRayPos + cursorRayDir * outdst[0];
		int layer_id = m_paintProps.layerNum;
		int off = 0;
		for( size_t i = 0; i < m_selObjList.size(); ++i )
		{
			EdObject* obj = m_selObjList[ i ].GetEdObject();
			if( obj )
			{
				int vcount = obj->GetNumPaintVerts();
				for( int v = 0; v < vcount; ++v )
				{
					Vec3 vpos;
					Vec4 vcol;
					obj->GetPaintVertex( v, layer_id, vpos, vcol );
					PaintVertex& PV = m_originalVerts[ off++ ];
					float q = m_paintProps.GetDistanceFactor( vpos, pos ) / 60.0f; // assuming FPS limit - TODO FIX
					PV.factor += q;
					vpos = PV.pos;
					vcol = PV.col;
					m_paintProps.Paint( vpos, -cursorRayDir, vcol, PV.factor );
					obj->SetPaintVertex( v, layer_id, vpos, vcol );
				}
				obj->RegenerateMesh();
			}
		}
	}
}


EdPaintSurfsEditMode::EdPaintSurfsEditMode() :
	m_paintBlock( EdObjIdx() ),
	m_paintSurf( -1 ),
	m_isPainting( false )
{
	m_paintSurfTemplate.texname = "null";
}

void EdPaintSurfsEditMode::OnEnter()
{
	m_paintBlock = EdObjIdx();
	m_paintSurf = -1;
	m_isPainting = false;
}

void EdPaintSurfsEditMode::ViewUI()
{
	if( ImGui::IsWindowHovered() )
	{
		Vec3 cursorRayPos = g_UIFrame->GetCursorRayPos();
		Vec3 cursorRayDir = g_UIFrame->GetCursorRayDir();
		
		m_isPainting = ImGui::IsMouseDown( 0 );
		ImVec2 dd = ImGui::GetMouseDragDelta( 0, 0 );
		bool dopaint = dd.x != 0 || dd.y != 0 || ImGui::IsMouseClicked( 0 );
		if( dopaint )
			ImGui::ResetMouseDragDelta( 0 );
		
		float outdst[1];
		EdObjIdx outblock[1];
		int outsurf[1];
		
		m_paintBlock = EdObjIdx();
		if( g_EdWorld->RayObjectsIntersect( cursorRayPos, cursorRayDir, m_paintBlock, outdst, outblock, NULL, SelMask_Blocks ) )
			m_paintBlock = outblock[0];
		SGRX_CAST( EdBlock*, B, m_paintBlock.GetEdObject() );
		
		m_paintSurf = -1;
		if( B && B->RayIntersect( cursorRayPos, cursorRayDir, outdst, outsurf ) )
			m_paintSurf = outsurf[0];
		
		if( ImGui::IsKeyPressed( SDLK_g, false ) && B && m_paintSurf >= 0 )
		{
			m_paintSurfTemplate = *B->surfaces[ m_paintSurf ];
		}
		if( dopaint && B && m_paintSurf >= 0 )
		{
			*B->surfaces[ m_paintSurf ] = m_paintSurfTemplate;
			B->RegenerateMesh();
		}
	}
}

void EdPaintSurfsEditMode::EditUI()
{
	m_paintSurfTemplate.EditUI();
}

void EdPaintSurfsEditMode::Draw()
{
	if( m_paintBlock.Valid() && m_paintSurf >= 0 )
	{
		g_EdWorld->DrawPoly_BlockSurf( m_paintBlock, m_paintSurf, m_isPainting );
	}
}


void EdEditGroupEditMode::EditUI()
{
	g_EdWorld->m_groupMgr.EditUI();
}

void EdEditGroupEditMode::Draw()
{
	g_EdWorld->m_groupMgr.DrawGroups();
}


