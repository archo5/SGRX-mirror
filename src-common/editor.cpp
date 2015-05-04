

#define MAPEDIT_DEFINE_GLOBALS
#include "mapedit.hpp"



EdWorld::EdWorld() :
	m_ctlGroup( true, "Level properties" ),
	m_ctlAmbientColor( V3(0,0,0.1f), 2, V3(0), V3(1,1,100) ),
	m_ctlDirLightDir( V2(0,0), 2, V2(-8192), V2(8192) ),
	m_ctlDirLightColor( V3(0,0,0.0f), 2, V3(0), V3(1,1,100) ),
	m_ctlDirLightDivergence( 10, 2, 0, 180 ),
	m_ctlDirLightNumSamples( 15, 0, 256 ),
	m_ctlLightmapClearColor( V3(0,0,0), 2, V3(0), V3(1,1,100) ),
	m_ctlRADNumBounces( 2, 0, 256 ),
	m_ctlLightmapDetail( 1, 2, 0.01f, 10 ),
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
	m_vd = GR_GetVertexDecl( EdVtx_DECL );
	
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

void EdWorld::Reset()
{
	m_blocks.clear();
	m_entities.clear();
}

void EdWorld::TestData()
{
	EdBlock b1;
	
	b1.position = V2(0);
	b1.z0 = 0;
	b1.z1 = 2;
	
	Vec3 poly[] = { {-2,-2,0}, {2,-2,0}, {2,2,0}, {-2,2,0} };
	b1.poly.assign( poly, 4 );
	
	EdSurface surf;
	surf.texname = "metal0";
	for( int i = 0; i < 6; ++i )
		b1.surfaces.push_back( surf );
	
	m_blocks.push_back( b1 );
	b1.z1 = 1;
	b1.position = V2( 0.1f, 1 );
	m_blocks.push_back( b1 );
	
	RegenerateMeshes();
}

void EdWorld::RegenerateMeshes()
{
	for( size_t i = 0; i < m_blocks.size(); ++i )
		m_blocks[ i ].RegenerateMesh();
}

void EdWorld::DrawWires_Blocks( int hlblock, int selblock )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	br.SetPrimitiveType( PT_Lines ).UnsetTexture();
	for( size_t i = 0; i < m_blocks.size(); ++i )
	{
		if( (int) i == selblock )
			br.Col( 0.9f, 0.5, 0.1f, 1 );
		else if( (int) i == hlblock )
			br.Col( 0.1f, 0.5, 0.9f, 0.9f );
		else
			br.Col( 0.1f, 0.5, 0.9f, 0.5f );
		
		const EdBlock& B = m_blocks[ i ];
		for( size_t v = 0; v < B.poly.size(); ++v )
		{
			size_t v1 = ( v + 1 ) % B.poly.size();
			
			br.Pos( B.position.x + B.poly[ v ].x,  B.position.y + B.poly[ v ].y, B.z0 );
			br.Pos( B.position.x + B.poly[ v1 ].x, B.position.y + B.poly[ v1 ].y, B.z0 );
			
			br.Pos( B.position.x + B.poly[ v ].x,  B.position.y + B.poly[ v ].y, B.z1 );
			br.Pos( B.position.x + B.poly[ v1 ].x, B.position.y + B.poly[ v1 ].y, B.z1 );
			
			br.Prev( 3 ).Prev( 2 );
		}
	}
	
	br.Flush();
}

void EdWorld::DrawPoly_BlockSurf( int block, int surf, bool sel )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	br.Flush();
	br.SetPrimitiveType( PT_TriangleStrip ).UnsetTexture();
	
	if( sel )
		br.Col( 0.9f, 0.5, 0.1f, 0.2f );
	else
		br.Col( 0.1f, 0.5, 0.9f, 0.1f );
	
	const EdBlock& B = m_blocks[ block ];
	if( surf == (int) B.poly.size() )
	{
		for( size_t i = 0; i < B.poly.size(); ++i )
		{
			size_t v;
			if( i % 2 == 0 )
				v = i / 2;
			else
				v = B.poly.size() - 1 - i / 2;
			br.Pos( B.poly[ v ].x + B.position.x, B.poly[ v ].y + B.position.y, B.z1 );
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
			br.Pos( B.poly[ v ].x + B.position.x, B.poly[ v ].y + B.position.y, B.z0 );
		}
//		br.Prev( B.poly.size() - 1 );
	}
	else
	{
		size_t v = surf, v1 = ( surf + 1 ) % B.poly.size();
		br.Pos( B.position.x + B.poly[ v ].x,  B.position.y + B.poly[ v ].y, B.z0 );
		br.Pos( B.position.x + B.poly[ v1 ].x, B.position.y + B.poly[ v1 ].y, B.z0 );
		
		br.Pos( B.position.x + B.poly[ v ].x, B.position.y + B.poly[ v ].y, B.z1 );
		br.Pos( B.position.x + B.poly[ v1 ].x,  B.position.y + B.poly[ v1 ].y, B.z1 );
	}
	
	br.Flush();
}

void EdWorld::DrawPoly_BlockVertex( int block, int vert, bool sel )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	br.SetPrimitiveType( PT_Lines ).UnsetTexture();
	
	if( sel )
		br.Col( 0.9f, 0.5, 0.1f, 0.9f );
	else
		br.Col( 0.1f, 0.5, 0.9f, 0.5f );
	
	const EdBlock& B = m_blocks[ block ];
	Vec3 P = V3( B.poly[ vert ].x + B.position.x, B.poly[ vert ].y + B.position.y, B.z0 );
	
	float s = 0.5f;
	br.Pos( P - V3(s,0,0) ).Pos( P + V3(0,0,s) ).Prev(0).Pos( P + V3(s,0,0) ).Prev(0).Pos( P - V3(0,0,s) ).Prev(0).Prev(6);
	br.Pos( P - V3(0,s,0) ).Pos( P + V3(0,0,s) ).Prev(0).Pos( P + V3(0,s,0) ).Prev(0).Pos( P - V3(0,0,s) ).Prev(0).Prev(6);
	br.Pos( P - V3(s,0,0) ).Pos( P + V3(0,s,0) ).Prev(0).Pos( P + V3(s,0,0) ).Prev(0).Pos( P - V3(0,s,0) ).Prev(0).Prev(6);
}

void EdWorld::DrawWires_Entities( int hlmesh, int selmesh )
{
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
	
	br.SetPrimitiveType( PT_Lines ).UnsetTexture();
	for( size_t i = 0; i < m_entities.size(); ++i )
	{
		if( (int) i == selmesh )
			br.Col( 0.9f, 0.5, 0.1f, 0.9f );
		else if( (int) i == hlmesh )
			br.Col( 0.1f, 0.5, 0.9f, 0.7f );
		else
			br.Col( 0.1f, 0.5, 0.9f, 0.25f );
		
		float q = 0.2f;
		Vec3 P = m_entities[ i ]->Pos();
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
	if( selmesh >= 0 )
		m_entities[ selmesh ]->DebugDraw();
	if( hlmesh >= 0 && hlmesh != selmesh )
		m_entities[ hlmesh ]->DebugDraw();
	
	br.Flush();
}

bool EdWorld::RayBlocksIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outblock[1] )
{
	float ndst[1], mindst = FLT_MAX;
	int curblk = -1;
	if( searchfrom < 0 )
		searchfrom = m_blocks.size();
	for( int i = searchfrom - 1; i >= 0; --i )
	{
		if( m_blocks[ i ].RayIntersect( pos, dir, ndst ) && ndst[0] < mindst )
		{
			curblk = i;
			mindst = ndst[0];
		}
	}
	for( int i = m_blocks.size() - 1; i >= searchfrom; --i )
	{
		if( m_blocks[ i ].RayIntersect( pos, dir, ndst ) && ndst[0] < mindst )
		{
			curblk = i;
			mindst = ndst[0];
		}
	}
	outdst[0] = mindst;
	outblock[0] = curblk;
	return curblk != -1;
}

bool EdWorld::RayEntitiesIntersect( const Vec3& pos, const Vec3& dir, int searchfrom, float outdst[1], int outent[1] )
{
	float ndst[1], mindst = FLT_MAX;
	int curent = -1;
	if( searchfrom < 0 )
		searchfrom = m_entities.size();
	for( int i = searchfrom - 1; i >= 0; --i )
	{
		if( m_entities[ i ]->RayIntersect( pos, dir, ndst ) && ndst[0] < mindst )
		{
			curent = i;
			mindst = ndst[0];
		}
	}
	for( int i = m_entities.size() - 1; i >= searchfrom; --i )
	{
		if( m_entities[ i ]->RayIntersect( pos, dir, ndst ) && ndst[0] < mindst )
		{
			curent = i;
			mindst = ndst[0];
		}
	}
	outdst[0] = mindst;
	outent[0] = curent;
	return curent != -1;
}



EDGUIMainFrame::EDGUIMainFrame() :
	m_cursorAim( false ),
	m_blockDrawMode( BD_Polygon ),
	m_newBlockPropZ0( 0, 2, -8192, 8192 ),
	m_newBlockPropZ1( 2, 2, -8192, 8192 ),
	m_hlBlock( -1 ),
	m_selBlock( -1 ),
	m_hlSurf( -1 ),
	m_selSurf( -1 ),
	m_hlVert( -1 ),
	m_selVert( -1 ),
	m_dragAdjacent( false ),
	m_paintBlock( -1 ),
	m_paintSurf( -1 ),
	m_isPainting( false ),
	m_hlEnt( -1 ),
	m_selEnt( -1 ),
	m_UIMenuSplit( true, 26, 0 ),
	m_UIParamSplit( false, 0, 0.7f ),
	m_UIRenderView( g_EdScene, this )
{
	tyname = "mainframe";
	
	Add( &m_UIMenuSplit );
	m_UIMenuSplit.SetFirstPane( &m_UIMenuButtons );
	m_UIMenuSplit.SetSecondPane( &m_UIParamSplit );
	m_UIParamSplit.SetFirstPane( &m_UIRenderView );
	m_UIParamSplit.SetSecondPane( &m_UIParamList );
	
	// menu
	m_MB_Cat0.caption = "File:";
	m_MBNew.caption = "New";
	m_MBOpen.caption = "Open";
	m_MBSave.caption = "Save";
	m_MBSaveAs.caption = "Save As";
	m_MBCompile.caption = "Compile";
	m_MB_Cat1.caption = "Edit:";
	m_MBDrawBlock.caption = "Draw Block";
	m_MBEditBlock.caption = "Edit Block";
	m_MBPaintSurfs.caption = "Paint surfaces";
	m_MBAddEntity.caption = "Add Entity";
	m_MBEditEntity.caption = "Edit Entity";
	m_MBLevelInfo.caption = "Level Info";
	m_newBlockPropZ0.caption = "Bottom Z";
	m_newBlockPropZ1.caption = "Top Z";
	m_UIMenuButtons.Add( &m_MB_Cat0 );
	m_UIMenuButtons.Add( &m_MBNew );
	m_UIMenuButtons.Add( &m_MBOpen );
	m_UIMenuButtons.Add( &m_MBSave );
	m_UIMenuButtons.Add( &m_MBSaveAs );
	m_UIMenuButtons.Add( &m_MBCompile );
	m_UIMenuButtons.Add( &m_MB_Cat1 );
	m_UIMenuButtons.Add( &m_MBDrawBlock );
	m_UIMenuButtons.Add( &m_MBEditBlock );
	m_UIMenuButtons.Add( &m_MBPaintSurfs );
	m_UIMenuButtons.Add( &m_MBAddEntity );
	m_UIMenuButtons.Add( &m_MBEditEntity );
	m_UIMenuButtons.Add( &m_MBLevelInfo );
	
	m_entityProps = m_entGroup.m_buttons[0].m_ent_handle;
	
	m_txMarker = GR_GetTexture( "editor/marker.png" );
	
	SetMode( ED_EditLevel );
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
		
		else if( e->target == &m_MBDrawBlock ) SetMode( ED_DrawBlock );
		else if( e->target == &m_MBEditBlock ) SetMode( ED_EditBlock );
		else if( e->target == &m_MBPaintSurfs ) SetMode( ED_PaintSurfs );
		else if( e->target == &m_MBAddEntity ) SetMode( ED_AddEntity );
		else if( e->target == &m_MBEditEntity ) SetMode( ED_EditEntity );
		else if( e->target == &m_MBLevelInfo ) SetMode( ED_EditLevel );
		
		return 1;
		
	case EDGUI_EVENT_SETENTITY:
		SetEntityType( ((EDGUIEntButton*)e->target)->m_ent_handle );
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
		if( e->target == &m_newBlockPropZ0 )
		{
			m_UIRenderView.crplaneheight = m_newBlockPropZ0.m_value;
		}
		if( e->target == &m_entityProps->m_ctlPos )
		{
			m_UIRenderView.crplaneheight = m_entityProps->Pos().z;
		}
		if( m_selEnt >= 0 && e->target == &g_EdWorld->m_entities[ m_selEnt ]->m_ctlPos )
		{
			m_UIRenderView.crplaneheight = g_EdWorld->m_entities[ m_selEnt ]->Pos().z;
		}
		return 1;
	}
	return EDGUIFrame::OnEvent( e );
}

void EDGUIMainFrame::ViewEvent( EDGUIEvent* e )
{
	if( e->type == EDGUI_EVENT_MOUSEMOVE )
	{
		m_cursorAim = m_UIRenderView.cursor_aim;
		m_cursorWorldPos = m_UIRenderView.cursor_hpos;
		m_snapProps.Snap( m_cursorWorldPos );
	}
	
	if( m_mode == ED_DrawBlock )
	{
		if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 && m_cursorAim && m_drawnVerts.size() < 14 )
		{
			m_drawnVerts.push_back( m_cursorWorldPos );
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
	else if( m_mode == ED_EditBlock )
	{
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
			m_frame->_HandleMouseMove( false );
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
					if( RaySphereIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, V3( B.poly[i].x + B.position.x, B.poly[i].y + B.position.y, B.z0 ), 0.2f, outdst ) && outdst[0] < mindst )
					{
						mindst = outdst[0];
						m_hlVert = i;
					}
				}
				m_hlSurf = -1;
				if( B.RayIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, outdst, outblock ) && outdst[0] < mindst )
				{
					m_hlSurf = outblock[0];
					m_hlVert = -1;
				}
			}
			if( m_selBlock < 0 || ( m_hlVert < 0 && m_hlSurf < 0 ) )
			{
				m_hlBlock = -1;
				if( g_EdWorld->RayBlocksIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, m_selBlock, outdst, outblock ) )
					m_hlBlock = outblock[0];
			}
			if( m_grabbed && m_selBlock >= 0 )
			{
				EdBlock& B = g_EdWorld->m_blocks[ m_selBlock ];
				Vec2 tgtpos = m_cursorWorldPos + m_cpdiff;
				int selvert = m_selVert;
				bool itssurf = false;
				if( selvert < 0 && m_selSurf < (int) B.poly.size() )
				{
					selvert = m_selSurf;
					itssurf = true;
				}
				if( selvert >= 0 )
				{
					m_snapProps.Snap( tgtpos );
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
					B.position = tgtpos;
				g_EdWorld->m_blocks[ m_selBlock ].RegenerateMesh();
				_ReloadBlockProps();
			}
		}
		// GRAB
		if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_g && !e->key.repeat && m_selBlock >= 0 && m_cursorAim )
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
				m_cpdiff = m_origPos - m_cursorWorldPos;
			}
			else
				m_cpdiff = B.position - m_cursorWorldPos;
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
			ClearParamList();
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
			ClearParamList();
			g_EdWorld->m_ctlBlockProps.m_out = NULL; // just in case
			EdBlock B = g_EdWorld->m_blocks[ m_selBlock ];
			m_selBlock = g_EdWorld->m_blocks.size();
			m_selVert = -1;
			m_selSurf = -1;
			B.cached_mesh = NULL;
			B.cached_meshinst = NULL;
			B.RegenerateMesh();
			g_EdWorld->m_blocks.push_back( B );
			m_cpdiff = B.position - m_cursorWorldPos;
			m_grabbed = true;
			AddToParamList( g_EdWorld->GetBlockProps( m_selBlock ) );
		}
	}
	else if( m_mode == ED_PaintSurfs )
	{
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
			if( g_EdWorld->RayBlocksIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, m_paintBlock, outdst, outblock ) )
				m_paintBlock = outblock[0];
			m_paintSurf = -1;
			if( m_paintBlock >= 0 && g_EdWorld->m_blocks[ m_paintBlock ].RayIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, outdst, outblock ) )
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
	else if( m_mode == ED_AddEntity )
	{
		if( e->type == EDGUI_EVENT_BTNCLICK && e->mouse.button == 0 && m_cursorAim )
		{
			_AddNewEntity();
		}
	}
	else if( m_mode == ED_EditEntity )
	{
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
			if( g_EdWorld->RayEntitiesIntersect( m_UIRenderView.crpos, m_UIRenderView.crdir, m_selEnt, outdst, outmesh ) )
				m_hlEnt = outmesh[0];
			if( m_grabbed && m_selEnt >= 0 )
			{
				EdEntity* N = g_EdWorld->m_entities[ m_selEnt ];
				N->SetPosition( V3( m_cursorWorldPos.x + m_cpdiff.x, m_cursorWorldPos.y + m_cpdiff.y, N->Pos().z ) );
				N->RegenerateMesh();
				_ReloadEntityProps();
			}
		}
		// GRAB
		if( e->type == EDGUI_EVENT_KEYDOWN && e->key.engkey == SDLK_g && !e->key.repeat && m_selEnt >= 0 && m_cursorAim )
		{
			Vec3 P = g_EdWorld->m_entities[ m_selEnt ]->Pos();
			m_cpdiff = V2(P.x,P.y) - m_cursorWorldPos;
			m_grabbed = true;
		}
		if( e->type == EDGUI_EVENT_KEYUP && ( e->key.engkey == SDLK_g || e->key.engkey == SDLK_d ) && m_grabbed )
		{
			m_grabbed = false;
		}
		// DELETE
		if( e->type == EDGUI_EVENT_KEYUP && e->key.engkey == SDLK_DELETE && m_selEnt >= 0 )
		{
			ClearParamList();
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
			m_cpdiff = V2(P.x,P.y) - m_cursorWorldPos;
			m_grabbed = true;
			_ReloadEntityProps();
		}
	}
}

void EDGUIMainFrame::_ReloadBlockProps()
{
	ClearParamList();
	if( m_selBlock >= 0 )
	{
		m_UIRenderView.crplaneheight = g_EdWorld->m_blocks[ m_selBlock ].z0;
		if( m_selVert >= 0 )
		{
			AddToParamList( g_EdWorld->GetVertProps( m_selBlock, m_selVert ) );
		}
		else if( m_selSurf >= 0 )
			AddToParamList( g_EdWorld->GetSurfProps( m_selBlock, m_selSurf ) );
		else
			AddToParamList( g_EdWorld->GetBlockProps( m_selBlock ) );
	}
}

void EDGUIMainFrame::_ReloadEntityProps()
{
	ClearParamList();
	if( m_selEnt >= 0 )
	{
		m_UIRenderView.crplaneheight = g_EdWorld->m_entities[ m_selEnt ]->Pos().z;
		AddToParamList( g_EdWorld->GetEntityProps( m_selEnt ) );
	}
}

void EDGUIMainFrame::_DrawCursor( bool drawimg, float height )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.UnsetTexture();
	if( m_cursorAim )
	{
		Vec2 pos = m_cursorWorldPos;
		if( m_drawnVerts.size() > 1 )
		{
			br.Col( 0.9f, 0.1f, 0, 0.4f ).SetPrimitiveType( PT_LineStrip );
			br.Pos( m_drawnVerts.last().x, m_drawnVerts.last().y, height );
			br.Pos( pos.x, pos.y, height );
			br.Pos( m_drawnVerts[0].x, m_drawnVerts[0].y, height );
		}
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
	BatchRenderer& br = GR2D_GetBatchRenderer();
	if( m_mode == ED_DrawBlock )
	{
		br.UnsetTexture();
		if( m_blockDrawMode == BD_Polygon && m_drawnVerts.size() >= 3 )
		{
			br.Flush();
			br.Col( 0.9f, 0.1f, 0, 0.3f ).SetPrimitiveType( PT_TriangleStrip );
			for( size_t i = 0; i < m_drawnVerts.size(); ++i )
			{
				size_t v;
				if( i % 2 == 0 )
					v = i / 2;
				else
					v = m_drawnVerts.size() - 1 - i / 2;
				br.Pos( m_drawnVerts[v].x, m_drawnVerts[v].y, m_UIRenderView.crplaneheight );
			}
			br.Flush();
		}
		if( m_blockDrawMode == BD_BoxStrip )
		{
			for( size_t i = 1; i < m_drawnVerts.size(); ++i )
			{
				br.Col( 0.9f, 0.1f, 0, 0.3f ).SetPrimitiveType( PT_TriangleStrip );
				Vec2 p0 = m_drawnVerts[ i - 1 ];
				Vec2 p1 = m_drawnVerts[ i ];
				float z = m_UIRenderView.crplaneheight;
				br.Pos( p0.x, p0.y, z );
				br.Pos( p1.x, p0.y, z );
				br.Pos( p0.x, p1.y, z );
				br.Pos( p1.x, p1.y, z );
				br.Flush();
			}
		}
		if( m_drawnVerts.size() >= 2 )
		{
			br.Flush();
			br.Col( 0.9f, 0.1f, 0, 0.7f ).SetPrimitiveType( PT_LineStrip );
			for( size_t i = 0; i < m_drawnVerts.size(); ++i )
				br.Pos( m_drawnVerts[i].x, m_drawnVerts[i].y, m_UIRenderView.crplaneheight );
			br.Pos( m_drawnVerts[0].x, m_drawnVerts[0].y, m_UIRenderView.crplaneheight );
			br.Flush();
		}
		for( size_t i = 0; i < m_drawnVerts.size(); ++i )
		{
			br.Col( 0.9f, 0.1f, 0, 0.8f );
			br.CircleOutline( m_drawnVerts[i].x, m_drawnVerts[i].y, 0.02f, m_UIRenderView.crplaneheight, 16 );
		}
		if( m_blockDrawMode == BD_Polygon && m_cursorAim )
		{
			Vec2 pos = m_cursorWorldPos;
			if( m_drawnVerts.size() > 1 )
			{
				br.Flush();
				br.Col( 0.9f, 0.1f, 0, 0.4f ).SetPrimitiveType( PT_LineStrip );
				br.Pos( m_drawnVerts.last().x, m_drawnVerts.last().y, m_UIRenderView.crplaneheight );
				br.Pos( pos.x, pos.y, m_UIRenderView.crplaneheight );
				br.Pos( m_drawnVerts[0].x, m_drawnVerts[0].y, m_UIRenderView.crplaneheight );
				br.Flush();
			}
		}
		DrawCursor();
	}
	else if( m_mode == ED_EditBlock )
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
			DrawCursor( false );
	}
	else if( m_mode == ED_PaintSurfs )
	{
		if( m_paintBlock >= 0 && m_paintSurf >= 0 )
		{
			g_EdWorld->DrawPoly_BlockSurf( m_paintBlock, m_paintSurf, m_isPainting );
		}
	}
	else if( m_mode == ED_AddEntity )
	{
		g_EdWorld->DrawWires_Entities( -1, -1 );
		DrawCursor( false );
	}
	else if( m_mode == ED_EditEntity )
	{
		g_EdWorld->DrawWires_Entities( m_hlEnt, m_selEnt );
		if( m_grabbed )
			DrawCursor( false );
	}
	br.Flush();
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

void EDGUIMainFrame::ResetEditorState()
{
	ClearParamList();
	m_hlBlock = -1;
	m_selBlock = -1;
	m_hlVert = -1;
	m_selVert = -1;
	m_hlSurf = -1;
	m_selSurf = -1;
	m_hlEnt = -1;
	m_selEnt = -1;
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
	snprintf( bfr, sizeof(bfr), "levels/%.*s.tle", TMIN( (int) str.size(), 200 ), str.data() );
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
	snprintf( bfr, sizeof(bfr), "levels/%.*s.tle", TMIN( (int) str.size(), 200 ), str.data() );
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
		g_EdWorld->m_blocks[ i ].GenerateMesh( lcache );
	
	for( size_t i = 0; i < g_EdWorld->m_entities.size(); ++i )
		g_EdWorld->m_entities[ i ]->UpdateCache( lcache );
	
	char bfr[ 256 ];
	snprintf( bfr, sizeof(bfr), "levels/%.*s", TMIN( (int) m_fileName.size(), 200 ), m_fileName.data() );
	
	if( !lcache.SaveCache( bfr ) )
		LOG_ERROR << "FAILED TO SAVE CACHE";
	else
		LOG << "Level is compiled";
}

void EDGUIMainFrame::SetMode( ED_EditMode newmode )
{
	m_mode = newmode;
	m_MBDrawBlock.SetHighlight( newmode == ED_DrawBlock );
	m_MBEditBlock.SetHighlight( newmode == ED_EditBlock );
	m_MBAddEntity.SetHighlight( newmode == ED_AddEntity );
	m_MBEditEntity.SetHighlight( newmode == ED_EditEntity );
	m_MBLevelInfo.SetHighlight( newmode == ED_EditLevel );
	ClearParamList();
	if( newmode == ED_DrawBlock )
	{
		m_UIRenderView.crplaneheight = m_newBlockPropZ0.m_value;
		m_drawnVerts.clear();
		AddToParamList( &m_snapProps );
		AddToParamList( &m_newBlockPropZ0 );
		AddToParamList( &m_newBlockPropZ1 );
		AddToParamList( &m_newSurfProps );
	}
	else if( newmode == ED_EditBlock )
	{
		m_hlBlock = -1;
		m_selBlock = -1;
		m_grabbed = false;
	}
	else if( newmode == ED_PaintSurfs )
	{
		m_paintBlock = -1;
		m_paintSurf = -1;
		m_isPainting = false;
		AddToParamList( &m_paintSurfProps );
	}
	else if( newmode == ED_AddEntity )
	{
		m_UIRenderView.crplaneheight = m_entityProps->Pos().z;
		AddToParamList( &m_entGroup );
		AddToParamList( m_entityProps );
	}
	else if( newmode == ED_EditEntity )
	{
		m_hlEnt = -1;
		m_selEnt = -1;
		m_grabbed = false;
	}
	else if( newmode == ED_EditLevel )
	{
		AddToParamList( g_EdWorld );
	}
}

void EDGUIMainFrame::SetEntityType( const EdEntityHandle& eh )
{
	m_entityProps = eh;
	if( m_mode == ED_AddEntity )
	{
		ClearParamList();
		AddToParamList( &m_entGroup );
		AddToParamList( m_entityProps );
	}
}

void EDGUIMainFrame::_AddNewBlock()
{
	EdBlock B;
	B.position = V2(0,0);
	B.z0 = m_newBlockPropZ0.m_value;
	B.z1 = m_newBlockPropZ1.m_value;
	B.poly.resize( m_drawnVerts.size() );
	for( size_t i = 0; i < m_drawnVerts.size(); ++i )
		B.poly[ i ] = V3( m_drawnVerts[ i ].x, m_drawnVerts[ i ].y, 0 );
	if( PolyArea( m_drawnVerts.data(), m_drawnVerts.size() ) < 0 )
		B.poly.reverse();
	B.GenCenterPos( m_snapProps );
	for( size_t i = 0; i < m_drawnVerts.size() + 2; ++i )
	{
		EdSurface S;
		m_newSurfProps.BounceBack( S );
		B.surfaces.push_back( S );
	}
	B.RegenerateMesh();
	g_EdWorld->m_blocks.push_back( B );
}

void EDGUIMainFrame::_AddNewEntity()
{
	EdEntity* N = m_entityProps->Clone();
	N->SetPosition( V3( m_cursorWorldPos.x, m_cursorWorldPos.y, N->Pos().z ) );
	N->RegenerateMesh();
	g_EdWorld->m_entities.push_back( N );
}


//
// EDITOR ENTRY POINT
//

SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
};

struct TACStrikeEditor : IGame
{
	void OnInitialize()
	{
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
		GR2D_SetFont( "fonts/lato-regular.ttf", 12 );
		
		g_ScriptCtx = new ScriptContext;
		g_ScriptCtx->RegisterBatchRenderer();
		sgs_RegFuncConsts( g_ScriptCtx->C, g_ent_scripted_rfc, -1 );
		
		LOG << "\n\nLoading scripted entities:\n\n";
		g_ScriptCtx->ExecFile( "editor/entities.sgs" );
		LOG << "\nLoading completed\n\n";
		
		g_UISurfTexPicker = new EDGUISurfTexPicker;
		g_UIMeshPicker = new EDGUIMeshPicker;
		g_UIPartSysPicker = new EDGUIPartSysPicker;
		g_UISoundPicker = new EDGUISoundPicker;
		g_UIScrFnPicker = new EDGUIScrFnPicker( g_ScriptCtx );
		g_UILevelOpenPicker = new EDGUILevelOpenPicker;
		g_UILevelSavePicker = new EDGUILevelSavePicker;
		
		// core layout
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = Vec3::Create(3,3,3);
		g_EdScene->camera.UpdateMatrices();
		g_EdWorld = new EdWorld();
		g_UIFrame = new EDGUIMainFrame();
		g_UIFrame->Resize( GR_GetWidth(), GR_GetHeight() );
		
		// param area
		g_UIFrame->AddToParamList( g_EdWorld );
	}
	void OnDestroy()
	{
		delete g_UILevelSavePicker;
		g_UILevelSavePicker = NULL;
		delete g_UILevelOpenPicker;
		g_UILevelOpenPicker = NULL;
		delete g_UIScrFnPicker;
		g_UIScrFnPicker = NULL;
		delete g_UIPartSysPicker;
		g_UIPartSysPicker = NULL;
		delete g_UISoundPicker;
		g_UISoundPicker = NULL;
		delete g_UIMeshPicker;
		g_UIMeshPicker = NULL;
		delete g_UISurfTexPicker;
		g_UISurfTexPicker = NULL;
		delete g_UIFrame;
		g_UIFrame = NULL;
		delete g_EdWorld;
		g_EdWorld = NULL;
		g_EdScene = NULL;
		delete g_ScriptCtx;
		g_ScriptCtx = NULL;
	}
	void OnEvent( const Event& e )
	{
		g_UIFrame->EngineEvent( &e );
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

