

#include "mapedit.hpp"


bool EdSurface::EditUI( EdBlock* B, int sid )
{
	bool del = false;
	IMGUI_GROUP( "Surface properties", true,
	{
		g_NUISurfMtlPicker->Property( "Pick surface material", "Material", texname );
		
		Vec2 off = V2( xoff, yoff );
		IMGUIEditVec2( "Offset", off, 0, 1 );
		xoff = off.x; yoff = off.y;
		
		Vec2 scasp = V2( scale, aspect );
		IMGUIEditVec2( "Scale/aspect", scasp, 0.001f, 1000 );
		scale = scasp.x; aspect = scasp.y;
		
		IMGUIEditFloat( "Angle", angle, 0, 360 );
		IMGUIEditFloat( "Lightmap quality", lmquality, 0.01f, 100 );
		IMGUIEditInt( "Fit count on X", xfit, 0, 100 );
		IMGUIEditInt( "Fit count on Y", yfit, 0, 100 );
		
		if( ImGui::Button( "Reset offset/scale/aspect" ) )
		{
			xoff = 0;
			yoff = 0;
			scale = 1;
			aspect = 1;
			ImGui::TriggerChangeCheck();
		}
		if( B )
		{
			ImGui::SameLine();
			if( ImGui::Button( "Apply fit" ) )
			{
				EdSurface& S = *B->surfaces[ sid ];
				float xmin = FLT_MAX;
				float xmax = -FLT_MAX;
				float ymin = FLT_MAX;
				float ymax = -FLT_MAX;
				LCVertex vertices[ MAX_BLOCK_POLYGONS ];
				int vcount = B->GenerateSurface( vertices, sid, false, false );
				for( int i = 0; i < vcount; ++i )
				{
					float x = vertices[ i ].tx0;
					float y = vertices[ i ].ty0;
					if( xmin > x ) xmin = x;
					if( xmax < x ) xmax = x;
					if( ymin > y ) ymin = y;
					if( ymax < y ) ymax = y;
				}
				float xdst = xmax - xmin;
				float ydst = ymax - ymin;
				if( S.xfit )
				{
					S.scale *= xdst;
					S.xoff -= xmin / S.scale;
					if( S.yfit != 0 )
						S.aspect *= xdst;
				}
				if( S.yfit )
				{
					if( S.xfit == 0 )
						S.scale *= ydst;
					else
						S.aspect /= ydst;
					S.yoff -= ymin / S.scale * S.aspect;
				}
				S.xoff = fmodf( S.xoff, 1 ); if( S.xoff < 0 ) S.xoff++;
				S.yoff = fmodf( S.yoff, 1 ); if( S.yoff < 0 ) S.yoff++;
				S.xfit = 0;
				S.yfit = 0;
				ImGui::TriggerChangeCheck();
			}
			int patchbtn = 0;
			if( ImGui::Button( "Make blend patch from surf." ) ) patchbtn = 1;
			ImGui::SameLine();
			if( ImGui::Button( "Convert block to patch" ) ) patchbtn = 2;
			if( patchbtn )
			{
				EdPatch* p = EdPatch::CreatePatchFromSurface( *B, sid );
				if( p )
				{
					B->selected = false;
					p->selected = true;
					if( patchbtn == 2 )
					{
						p->renderMode = PRM_Solid;
						p->m_isLMSolid = true;
						p->m_isPhySolid = true;
						g_EdWorld->DeleteObject( B );
						del = true;
					}
					else
					{
						p->renderMode = PRM_Decal;
					}
					g_EdWorld->AddObject( p );
					g_UIFrame->SetEditMode( &g_UIFrame->m_emEditObjs );
				}
			}
		}
	});
	return del;
}


void EdBlock::EditUI()
{
	ImGui::BeginChangeCheck();
	
	g_EdWorld->m_groupMgr.GroupProperty( "Group", group );
	IMGUIEditVec3( "Position", position, -8192, 8192 );
	
	Vec2 zz = V2( z0, z1 );
	IMGUIEditVec2( "Bottom/Top Z", zz, -8192, 8192 );
	z0 = zz.x; z1 = zz.y;
	
	IMGUI_GROUP( "Vertices", false,
	{
		char bfr[ 32 ];
		for( size_t i = 0; i < poly.size(); ++i )
		{
			sgrx_snprintf( bfr, 32, "#%d", (int) i );
			IMGUIEditVec3( bfr, poly[ i ], -8192, 8192 );
		}
	});
	
	bool del = false;
	IMGUI_GROUP( "Surfaces", false,
	{
		for( size_t i = 0; i < surfaces.size(); ++i )
		{
			ImGui::PushID( i );
			del = surfaces[ i ]->EditUI( this, i );
			ImGui::PopID();
			if( del )
				break;
		}
	});
	
	if( ImGui::EndChangeCheck() && !del )
		RegenerateMesh();
}

void EdBlock::VertEditUI( int vid )
{
	if( vid >= (int) poly.size() )
		vid -= poly.size();
	if( vid < 0 || vid >= (int) poly.size() )
		return;
	
	char bfr[ 32 ];
	sgrx_snprintf( bfr, 32, "Vertex #%d", vid );
	IMGUI_GROUP_BEGIN( bfr, true )
	{
		IMGUIEditVec3( "Position", poly[ vid ], -8192, 8192 );
		ImGui::Text( "Insert" );
		ImGui::SameLine();
		bool bef = ImGui::Button( "before" );
		ImGui::SameLine();
		bool aft = ImGui::Button( "after" );
		if( bef || aft )
		{
			size_t insat = vid, sz = poly.size();
			if( aft )
				insat = ( insat + 1 ) % sz;
			size_t befat = ( insat + sz - 1 ) % sz;
			
			Vec2 p0 = poly[ ( befat + sz - 1 ) % sz ].ToVec2();
			Vec2 p1 = poly[ befat ].ToVec2();
			Vec2 p2 = poly[ insat ].ToVec2();
			Vec2 p3 = poly[ ( insat + 1 ) % sz ].ToVec2();
			Vec2 edge_normal = ( p2 - p1 ).Perp().Normalized();
			Vec2 mid = ( p1 + p2 ) / 2;
			Vec2 mid2 = mid;
			float factor = 0.5f;
			if( intersect_lines( p0, p1, p3, p2, &mid2 ) )
			{
				if( Vec2Dot( mid2, edge_normal ) < Vec2Dot( mid, edge_normal ) )
					factor = -0.1f;
				mid = TLERP( mid, mid2, factor );
			}
			else
				mid += edge_normal;
			Vec3 mid_fin = { mid.x, mid.y, ( poly[ befat ].z + poly[ insat ].z ) * 0.5f };
			
			size_t oldpolysize = poly.size();
			EdSurface Scopy = *surfaces[ befat ];
			Scopy.surface_guid = SGRX_GUID::Generate();
			poly.insert( insat, mid_fin );
			if( (int) insat < vid )
				vid++;
			surfaces.insert( insat, new EdSurface( Scopy ) );
			
			subsel.insert( oldpolysize * 2 + insat, false );
			subsel.insert( oldpolysize + insat, false );
			subsel.insert( insat, false );
			
			ImGui::TriggerChangeCheck();
		}
	}
	IMGUI_GROUP_END;
}


Vec3 EdBlock::GetLocalVertex( int i ) const
{
	Vec3 vp = poly[ i % poly.size() ];
	if( i < (int) poly.size() )
		vp.z = z0;
	else
		vp.z += z1;
	return vp + position;
}

void EdBlock::SetLocalVertex( int i, const Vec3& pos )
{
	Vec3& dest = poly[ i % poly.size() ];
	Vec3 vp = pos - position;
	if( i < (int) poly.size() )
	{
		dest = V3( vp.x, vp.y, 0 );
		z1 = vp.z;
	}
	else
		dest = vp - V3( 0, 0, z1 );
}

void EdBlock::ScaleVertices( const Vec3& f )
{
	for( size_t i = 0; i < poly.size(); ++i )
		poly[ i ] *= f;
	z0 *= f.z;
	z1 *= f.z;
}

void EdBlock::MoveSelectedVertices( const Vec3& tfv )
{
	int numbottom = 0, numtop = 0;
	int pc = poly.size();
	for( int i = 0; i < pc; ++i )
	{
		if( subsel[ i ] ) numbottom++;
		if( subsel[ i + pc ] ) numtop++;
	}
	
	Vec3 t = tfv;
	
	if( numbottom || IsSurfaceSelected( GetNumSurfs() - 1 ) )
		z0 += t.z;
	if( numtop == pc || IsSurfaceSelected( GetNumSurfs() - 2 ) )
	{
		z1 += t.z;
		t.z = 0;
	}
	if( numtop == 0 )
		t.z = 0;
	
	// either top or bottom is fully selected
	if( numtop == pc || numbottom == pc || 
		IsSurfaceSelected( GetNumSurfs() - 1 ) ||
		IsSurfaceSelected( GetNumSurfs() - 2 ) )
	{
		position += t;
		return;
	}
	
	for( int i = 0; i < pc; ++i )
	{
		if( subsel[ i ] || // bottom vertex
			subsel[ i + pc ] || // top vertex
			subsel[ i + pc + pc ] || // following surface
			subsel[ ( i + pc - 1 ) % pc + pc + pc ] || // preceding surface
			subsel[ pc + pc + pc ] || // bottom surface
			subsel[ pc + pc + pc + 1 ] )
		{
			poly[ i ] += t;
		}
	}
}

Vec3 EdBlock::GetSurfaceCenter( int i ) const
{
	if( i < (int) poly.size() )
	{
		int i1 = ( i + 1 ) % poly.size();
		return ( GetLocalVertex( i ) + GetLocalVertex( i1 ) +
			GetLocalVertex( i + poly.size() ) + GetLocalVertex( i1 + poly.size() ) ) * 0.25f;
	}
	else if( i == (int) poly.size() )
	{
		Vec3 c = V3(0);
		for( size_t i = 0; i < poly.size(); ++i )
			c += poly[ i ];
		c /= poly.size();
		return c + position + V3( 0, 0, z1 );
	}
	else // i == (int) poly.size() + 1
	{
		Vec3 c = V3(0);
		for( size_t i = 0; i < poly.size(); ++i )
			c += V3( poly[ i ].x, poly[ i ].y, 0 );
		c /= poly.size();
		return c + position + V3( 0, 0, z0 );
	}
}

int EdBlock::GetSurfaceNumVerts( int i )
{
	if( i < (int) poly.size() )
		return 4;
	else
		return poly.size();
}

Vec3 EdBlock::GetElementPoint( int i ) const
{
	int nverts = GetNumVerts();
	if( i < nverts )
		return GetLocalVertex( i );
	else
		return GetSurfaceCenter( i - nverts );
}


bool EdBlock::IsVertexSelected( int i ) const
{
	ASSERT( i >= 0 && i < GetNumVerts() );
	return subsel[ i ];
}

void EdBlock::SelectVertex( int i, bool sel )
{
	ASSERT( i >= 0 && i < GetNumVerts() );
	subsel[ i ] = sel;
}

int EdBlock::GetOnlySelectedVertex() const
{
	int sel = -1;
	int num = GetNumVerts();
	for( int i = 0; i < num; ++i )
	{
		if( IsVertexSelected( i ) )
		{
			if( sel == -1 )
				sel = i;
			else
				return -1;
		}
	}
	return sel;
}

bool EdBlock::IsSurfaceSelected( int i )
{
	ASSERT( i >= 0 && i < GetNumSurfs() );
	return subsel[ i + GetNumVerts() ];
}

void EdBlock::SelectSurface( int i, bool sel )
{
	ASSERT( i >= 0 && i < GetNumSurfs() );
	subsel[ i + GetNumVerts() ] = sel;
}

int EdBlock::GetNumSelectedSurfs()
{
	int from = GetNumVerts(), num = 0;
	int to = from + GetNumSurfs();
	for( int i = from; i < to; ++i )
	{
		if( subsel[ i ] )
			num++;
	}
	return num;
}

int EdBlock::GetOnlySelectedSurface()
{
	int sel = -1;
	int num = GetNumSurfs();
	for( int i = 0; i < num; ++i )
	{
		if( IsSurfaceSelected( i ) )
		{
			if( sel == -1 )
				sel = i;
			else
				return -1;
		}
	}
	return sel;
}

bool EdBlock::IsElementSelected( int i ) const
{
	ASSERT( i >= 0 && i < GetNumElements() );
	return subsel[ i ];
}

void EdBlock::SelectElement( int i, bool sel )
{
	ASSERT( i >= 0 && i < GetNumElements() );
	subsel[ i ] = sel;
}

void EdBlock::ClearSelection()
{
	TMEMSET( subsel.data(), subsel.size(), false );
}

void EdBlock::SpecialAction( ESpecialAction act )
{
	switch( act )
	{
	case SA_SurfsToPatches:
		{
			int numsurfs = GetNumSurfs();
			for( int i = 0; i < numsurfs; ++i )
			{
				if( IsSurfaceSelected( i ) )
				{
					EdPatch* p = EdPatch::CreatePatchFromSurface( *this, i );
					g_EdWorld->AddObject( p );
				}
			}
		}
		break;
		
	case SA_Remove:
		if( CanDoSpecialAction( SA_Remove ) )
		{
			int numverts = GetNumVerts();
			int selverts[ MAX_BLOCK_POLYGONS ] = {0};
			for( int i = 0; i < numverts; ++i )
			{
				if( IsVertexSelected( i ) )
					selverts[ i % poly.size() ] = 1;
			}
			for( int i = (int) poly.size() - 1; i >= 0; --i )
			{
				if( selverts[ i ] )
				{
					poly.erase( i );
					surfaces.erase( i );
				}
			}
			RegenerateMesh();
		}
		
	default:
		break;
	}
}

bool EdBlock::CanDoSpecialAction( ESpecialAction act )
{
	switch( act )
	{
	case SA_SurfsToPatches:
		{
			int numsurfs = GetNumSurfs();
			for( int i = 0; i < numsurfs; ++i )
			{
				if( IsSurfaceSelected( i ) )
					return true;
			}
		}
		break;
		
	case SA_Remove:
		{
			int numverts = GetNumVerts();
			int selverts[ MAX_BLOCK_POLYGONS ] = {0};
			for( int i = 0; i < numverts; ++i )
			{
				if( IsVertexSelected( i ) )
					selverts[ i % poly.size() ] = 1;
			}
			int numsel = 0;
			for( size_t i = 0; i < poly.size(); ++i )
			{
				if( selverts[ i ] )
					numsel++;
			}
			numverts /= 2;
			// > must have at least 1 vertex selected
			// > at least 3 poly verts must remain after deletion
			return numsel > 0 && numverts - numsel >= 3;
		}
		
	default:
		break;
	}
	return false;
}


void EdBlock::_GetTexVecs( int surf, Vec3& tgx, Vec3& tgy )
{
	if( surf < (int) poly.size() )
	{
		size_t v0 = surf, v1 = ( surf + 1 ) % poly.size(); 
		Vec2 edgedir = ( poly[ v1 ] - poly[ v0 ] ).ToVec2().Normalized();
		tgx = V3( edgedir.x, edgedir.y, 0 );
		tgy = V3( 0, 0, -1 );
	}
	else if( surf == (int) poly.size() )
	{
		tgx = V3( -1, 0, 0 );
		tgy = V3( 0, 1, 0 );
	}
	else
	{
		tgx = V3( -1, 0, 0 );
		tgy = V3( 0, -1, 0 );
	}
}

uint16_t EdBlock::_AddVtx( const Vec3& vpos, float z, const EdSurface& S, const Vec3& tgx, const Vec3& tgy, Array< LCVertex >& vertices, uint16_t voff )
{
	LCVertex V = _MakeGenVtx( vpos, z, S, tgx, tgy );
	
	size_t off = vertices.find_first_at( V, voff );
	if( off != NOT_FOUND )
		return (uint16_t) off - voff;
	vertices.push_back( V );
	return (uint16_t) vertices.size() - 1 - voff;
}

void EdBlock::_PostFitTexcoords( const EdSurface& S, LCVertex* vertices, size_t vcount )
{
	float xmin = FLT_MAX, xmax = -FLT_MAX, ymin = FLT_MAX, ymax = -FLT_MAX;
	for( size_t i = 0; i < vcount; ++i )
	{
		float x = vertices[ i ].tx0;
		float y = vertices[ i ].ty0;
		if( xmin > x ) xmin = x;
		if( xmax < x ) xmax = x;
		if( ymin > y ) ymin = y;
		if( ymax < y ) ymax = y;
	}
	float xdst = xmax - xmin, ydst = ymax - ymin;
	for( size_t i = 0; i < vcount; ++i )
	{
		if( S.xfit )
		{
			vertices[ i ].tx0 = safe_fdiv( vertices[ i ].tx0 - xmin, xdst ) * S.xfit / S.scale + S.xoff;
			if( S.yfit == 0 )
			{
				vertices[ i ].ty0 = safe_fdiv( vertices[ i ].ty0, xdst ) * S.xfit / S.scale * S.aspect + S.yoff;
			}
		}
		if( S.yfit )
		{
			vertices[ i ].ty0 = safe_fdiv( vertices[ i ].ty0 - ymin, ydst ) * S.yfit / S.scale * S.aspect + S.yoff;
			if( S.xfit == 0 )
			{
				vertices[ i ].tx0 = safe_fdiv( vertices[ i ].tx0, ydst ) * S.yfit / S.scale + S.xoff;
			}
		}
	}
}

void EdBlock::GenCenterPos( EdSnapProps& SP )
{
	if( !poly.size() )
		return;
	
	Vec2 cp = {0,0}, oldpos = position.ToVec2(); // TODO_FULL_TRANSFORM ?
	for( size_t i = 0; i < poly.size(); ++i )
		cp += poly[i].ToVec2();
	cp /= poly.size();
	cp += position.ToVec2(); // TODO_FULL_TRANSFORM ?
	
	SP.Snap( cp );
	
	position.x = cp.x; // TODO_FULL_TRANSFORM ?
	position.y = cp.y;
	for( size_t i = 0; i < poly.size(); ++i )
		poly[i].SetXY( poly[i].ToVec2() - cp - oldpos );
}

Vec3 EdBlock::FindCenter() const
{
	Vec3 center = V3(0);
	for( size_t i = 0; i < poly.size(); ++i )
		center += V3( poly[ i ].x, poly[ i ].y, poly[ i ].z * 0.5f );
	if( poly.size() )
		center /= poly.size();
	center += position;
	center.z += ( z0 + z1 ) * 0.5f;
	return center;
}

EdObject* EdBlock::Clone()
{
	EdBlock* blk = new EdBlock( *this );
	blk->solid_guid = SGRX_GUID::Null;
	for( size_t i = 0; i < blk->surfaces.size(); ++i )
	{
		blk->surfaces[ i ] = new EdSurface( *blk->surfaces[ i ] );
		blk->surfaces[ i ]->surface_guid = SGRX_GUID::Null;
	}
	return blk;
}

bool EdBlock::RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1], int* outsurf ) const
{
	Vec3 pts[16];
	int pcount = poly.size();
	
	Mat4 tf = g_EdWorld->GetGroupMatrix( group );
	
	// TOP
	for( size_t i = 0; i < poly.size(); ++i )
	{
		pts[i] = tf.TransformPos( V3( poly[i].x + position.x, poly[i].y + position.y, poly[i].z + z1 + position.z ) );
	}
	if( RayPolyIntersect( rpos, dir, pts, pcount, outdst ) )
	{
		if( outsurf )
			*outsurf = poly.size();
		return true;
	}
	
	// BOTTOM
	for( size_t i = 0; i < poly.size(); ++i )
	{
		pts[ poly.size() - i - 1 ] = tf.TransformPos( V3( poly[i].x + position.x, poly[i].y + position.y, z0 + position.z ) );
	}
	if( RayPolyIntersect( rpos, dir, pts, pcount, outdst ) )
	{
		if( outsurf )
			*outsurf = poly.size() + 1;
		return true;
	}
	
	// SIDES
	pcount = 4;
	for( size_t i = 0; i < poly.size(); ++i )
	{
		size_t i1 = ( i + 1 ) % poly.size();
		pts[0] = tf.TransformPos( V3( poly[i].x + position.x, poly[i].y + position.y, z1 + position.z ) );
		pts[1] = tf.TransformPos( V3( poly[i].x + position.x, poly[i].y + position.y, z0 + position.z ) );
		pts[2] = tf.TransformPos( V3( poly[i1].x + position.x, poly[i1].y + position.y, z0 + position.z ) );
		pts[3] = tf.TransformPos( V3( poly[i1].x + position.x, poly[i1].y + position.y, z1 + position.z ) );
		if( RayPolyIntersect( rpos, dir, pts, pcount, outdst ) )
		{
			if( outsurf )
				*outsurf = i;
			return true;
		}
	}
	return false;
}

Vec2 CalcLMSizeAndNormalize( Array< LCVertex >& verts )
{
	Vec2 ltmin = V2(FLT_MAX), ltmax = V2(-FLT_MAX);
	for( size_t i = 0; i < verts.size(); ++i )
	{
		Vec2 lt = V2( verts[ i ].tx1, verts[ i ].ty1 );
		ltmin = Vec2::Min( ltmin, lt );
		ltmax = Vec2::Max( ltmax, lt );
	}
	Vec2 sz = ltmax - ltmin;
	Vec2 invsz = V2( safe_fdiv( 1, sz.x ), safe_fdiv( 1, sz.y ) );
	for( size_t i = 0; i < verts.size(); ++i )
	{
		verts[ i ].tx1 = ( verts[ i ].tx1 - ltmin.x ) * invsz.x;
		verts[ i ].ty1 = ( verts[ i ].ty1 - ltmin.y ) * invsz.y;
	}
	return sz;
}

void EdBlock::RegenerateMesh()
{
	if( !g_EdWorld || poly.size() < 3 || poly.size() > MAX_BLOCK_POLYGONS - 2 )
		return;
	
	Mat4 mtx = g_EdWorld->GetGroupMatrix( group );
	
	
	// GENERATE PLANES
	Vec3 toppoly[ MAX_BLOCK_POLYGONS ];
	int topverts = 0;
	Vec4 planes[ MAX_BLOCK_POLYGONS ];
	int numplanes = 0;
	
	planes[ numplanes++ ] = V4( 0, 0, -1, - position.z - z0 );
	for( size_t i = 0; i < poly.size(); ++i )
	{
		Vec3 vpos = poly[ i ] + position;
		
		size_t i1 = ( i + 1 ) % poly.size();
		Vec2 dir = ( poly[ i1 ] - poly[ i ] ).ToVec2().Perp().Normalized();
		if( !dir.NearZero() )
		{
			planes[ numplanes++ ] = V4( dir.x, dir.y, 0, Vec2Dot( vpos.ToVec2(), dir ) );
		}
		
		toppoly[ topverts++ ] = vpos + V3( 0, 0, z1 );
	}
	if( PolyGetPlane( toppoly, topverts, planes[ numplanes ] ) )
		numplanes++;
	else
		LOG_WARNING << "NO PLANE FOR TOP POLY at " << position;
	
	// HACK: if any surface is "clip", assume no solid clipping
	bool hasclip = false;
	for( size_t i = 0; i < poly.size(); ++i )
	{
		EdSurface& BS = *surfaces[ i ];
		if( BS.texname == SV("clip") )
		{
			hasclip = true;
			break;
		}
	}
	if( hasclip )
		numplanes = 0;
	
	// ADD SOLID
	EdLGCSolidInfo SOI;
	SOI.planes = planes;
	SOI.pcount = numplanes;
	if( solid_guid.NotNull() )
		g_EdLGCont->UpdateSolid( solid_guid, &SOI );
	else
	{
		solid_guid = SGRX_GUID::Generate();
		g_EdLGCont->RequestSolid( solid_guid, &SOI );
	}
	
	
	Array< LCVertex > vertices;
	Array< uint16_t > indices;
	
	// SIDES
	for( size_t i = 0; i < poly.size(); ++i )
	{
		EdSurface& BS = *surfaces[ i ];
		
		vertices.clear();
		indices.clear();
		
		Vec3 tgx, tgy;
		_GetTexVecs( i, tgx, tgy );
		size_t i1 = ( i + 1 ) % poly.size();
		uint16_t v1 = _AddVtx( poly[i], z0, BS, tgx, tgy, vertices, 0 );
		uint16_t v2 = _AddVtx( poly[i], z1 + poly[i].z, BS, tgx, tgy, vertices, 0 );
		uint16_t v3 = _AddVtx( poly[i1], z1 + poly[i1].z, BS, tgx, tgy, vertices, 0 );
		uint16_t v4 = _AddVtx( poly[i1], z0, BS, tgx, tgy, vertices, 0 );
		if( vertices.size() >= 3 )
		{
			_PostFitTexcoords( BS, vertices.data(), vertices.size() );
			indices.push_back( v1 );
			indices.push_back( v2 );
			indices.push_back( v3 );
			if( vertices.size() == 4 )
			{
				indices.push_back( v3 );
				indices.push_back( v4 );
				indices.push_back( v1 );
			}
		}
		else vertices.clear();
		
		EdLGCSurfaceInfo S;
		S.vdata = vertices.data();
		S.vcount = vertices.size();
		S.idata = indices.data();
		S.icount = indices.size();
		S.mtlname = BS.texname;
		S.lmsize = CalcLMSizeAndNormalize( vertices );
		S.xform = mtx;
		S.rflags = LM_MESHINST_CASTLMS | LM_MESHINST_SOLID;
		S.lmdetail = BS.lmquality;
		S.solid_guid = solid_guid;
		
		if( BS.surface_guid.NotNull() )
			g_EdLGCont->UpdateSurface( BS.surface_guid, LGC_CHANGE_ALL, &S );
		else
		{
			BS.surface_guid = SGRX_GUID::Generate();
			g_EdLGCont->RequestSurface( BS.surface_guid, &S );
		}
	}
	
	// TOP
	for(;;)
	{
		EdSurface& BS = *surfaces[ poly.size() ];
		
		vertices.clear();
		indices.clear();
		
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size(), tgx, tgy );
		for( size_t i = 0; i < poly.size(); ++i )
			_AddVtx( poly[i], z1 + poly[i].z, BS, tgx, tgy, vertices, 0 );
		if( vertices.size() < 3 )
			vertices.clear();
		_PostFitTexcoords( BS, vertices.data(), vertices.size() );
		for( size_t i = 2; i < vertices.size(); ++i )
		{
			indices.push_back( 0 );
			indices.push_back( i );
			indices.push_back( i - 1 );
		}
		
		EdLGCSurfaceInfo S;
		S.vdata = vertices.data();
		S.vcount = vertices.size();
		S.idata = indices.data();
		S.icount = indices.size();
		S.mtlname = BS.texname;
		S.lmsize = CalcLMSizeAndNormalize( vertices );
		S.xform = mtx;
		S.rflags = LM_MESHINST_CASTLMS | LM_MESHINST_SOLID;
		S.lmdetail = BS.lmquality;
		S.solid_guid = solid_guid;
		
		if( BS.surface_guid.NotNull() )
			g_EdLGCont->UpdateSurface( BS.surface_guid, LGC_CHANGE_ALL, &S );
		else
		{
			BS.surface_guid = SGRX_GUID::Generate();
			g_EdLGCont->RequestSurface( BS.surface_guid, &S );
		}
		break;
	}
	
	// BOTTOM
	for(;;)
	{
		EdSurface& BS = *surfaces[ poly.size() + 1 ];
		
		vertices.clear();
		indices.clear();
		
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size() + 1, tgx, tgy );
		for( size_t i = 0; i < poly.size(); ++i )
			_AddVtx( poly[i], z0, BS, tgx, tgy, vertices, 0 );
		if( vertices.size() < 3 )
			vertices.clear();
		_PostFitTexcoords( BS, vertices.data(), vertices.size() );
		for( size_t i = 2; i < vertices.size(); ++i )
		{
			indices.push_back( 0 );
			indices.push_back( i - 1 );
			indices.push_back( i );
		}
		
		EdLGCSurfaceInfo S;
		S.vdata = vertices.data();
		S.vcount = vertices.size();
		S.idata = indices.data();
		S.icount = indices.size();
		S.mtlname = BS.texname;
		S.lmsize = CalcLMSizeAndNormalize( vertices );
		S.xform = mtx;
		S.rflags = LM_MESHINST_CASTLMS | LM_MESHINST_SOLID;
		S.lmdetail = BS.lmquality;
		S.solid_guid = solid_guid;
		
		if( BS.surface_guid.NotNull() )
			g_EdLGCont->UpdateSurface( BS.surface_guid, LGC_CHANGE_ALL, &S );
		else
		{
			BS.surface_guid = SGRX_GUID::Generate();
			g_EdLGCont->RequestSurface( BS.surface_guid, &S );
		}
		break;
	}
}

LevelCache::Vertex EdBlock::_MakeGenVtx( const Vec3& vpos, float z, const EdSurface& S, const Vec3& tgx, const Vec3& tgy )
{
	Vec3 nrm = -Vec3Cross( tgx, tgy ).Normalized();
	LevelCache::Vertex V =
	{
		{ vpos.x + position.x, vpos.y + position.y, z + position.z },
		nrm, V4( tgx.Normalized(), -1 ), 0xffffffff, 0, 0, 0, 0
	};
	
	float tdx = Vec3Dot( V.pos, tgx ), tdy = Vec3Dot( V.pos, tgy );
	Vec2 tx = V2( tdx, tdy );
	tx = tx.Rotate( DEG2RAD( S.angle ) );
	V.tx0 = tx.x / S.scale + S.xoff;
	V.ty0 = tx.y / S.scale * S.aspect + S.yoff;
	V.tx1 = tdx * S.lmquality  *  2.0f; // magic factor
	V.ty1 = tdy * S.lmquality  *  2.0f; // magic factor
	
	return V;
}

int EdBlock::GenerateSurface( LCVertex* outbuf, int sid, bool tri, bool fit )
{
	if( tri )
	{
		Vec3 tgx, tgy;
		_GetTexVecs( sid, tgx, tgy );
		int retval = 6;
		if( sid < (int) poly.size() )
		{
			int i = sid;
			size_t i1 = ( i + 1 ) % poly.size();
			outbuf[5] = outbuf[0] = _MakeGenVtx( poly[i], z0, *surfaces[i], tgx, tgy );
			outbuf[1] = _MakeGenVtx( poly[i], z1 + poly[i].z, *surfaces[i], tgx, tgy );
			outbuf[3] = outbuf[2] = _MakeGenVtx( poly[i1], z1 + poly[i1].z, *surfaces[i], tgx, tgy );
			outbuf[4] = _MakeGenVtx( poly[i1], z0, *surfaces[i], tgx, tgy );
		}
		else if( sid == (int) poly.size() )
		{
			for( size_t i = 1; i + 1 < poly.size(); ++i )
			{
				outbuf[ (i-1)*3+0 ] = _MakeGenVtx( poly[0], z1 + poly[0].z, *surfaces[ poly.size() ], tgx, tgy );
				outbuf[ (i-1)*3+2 ] = _MakeGenVtx( poly[i], z1 + poly[i].z, *surfaces[ poly.size() ], tgx, tgy );
				outbuf[ (i-1)*3+1 ] = _MakeGenVtx( poly[i+1], z1 + poly[i+1].z, *surfaces[ poly.size() ], tgx, tgy );
			}
			retval = ( poly.size() - 2 ) * 3;
		}
		else // if( sid == (int) poly.size() + 1 )
		{
			for( size_t i = 1; i + 1 < poly.size(); ++i )
			{
				outbuf[ (i-1)*3+0 ] = _MakeGenVtx( poly[0], z0, *surfaces[ poly.size() ], tgx, tgy );
				outbuf[ (i-1)*3+1 ] = _MakeGenVtx( poly[i], z0, *surfaces[ poly.size() ], tgx, tgy );
				outbuf[ (i-1)*3+2 ] = _MakeGenVtx( poly[i+1], z0, *surfaces[ poly.size() ], tgx, tgy );
			}
			retval = ( poly.size() - 2 ) * 3;
		}
		if( fit )
			_PostFitTexcoords( *surfaces[ sid ], outbuf, retval );
		return retval;
	}
	else
	{
		Vec3 tgx, tgy;
		_GetTexVecs( sid, tgx, tgy );
		int retval = 4;
		if( sid < (int) poly.size() )
		{
			int i = sid;
			size_t i1 = ( i + 1 ) % poly.size();
			outbuf[0] = _MakeGenVtx( poly[i], z0, *surfaces[i], tgx, tgy );
			outbuf[1] = _MakeGenVtx( poly[i], z1 + poly[i].z, *surfaces[i], tgx, tgy );
			outbuf[2] = _MakeGenVtx( poly[i1], z1 + poly[i1].z, *surfaces[i], tgx, tgy );
			outbuf[3] = _MakeGenVtx( poly[i1], z0, *surfaces[i], tgx, tgy );
		}
		else if( sid == (int) poly.size() )
		{
			for( size_t i = 0; i < poly.size(); ++i )
			{
				outbuf[ poly.size() - 1 - i ] = _MakeGenVtx( poly[i], z1 + poly[i].z, *surfaces[ poly.size() ], tgx, tgy );
			}
			retval = poly.size();
		}
		else // if( sid == (int) poly.size() + 1 )
		{
			for( size_t i = 0; i < poly.size(); ++i )
			{
				outbuf[ i ] = _MakeGenVtx( poly[i], z0, *surfaces[ poly.size() ], tgx, tgy );
			}
			retval = poly.size();
		}
		if( fit )
			_PostFitTexcoords( *surfaces[ sid ], outbuf, retval );
		return retval;
	}
}

void EdBlock::Export( OBJExporter& objex )
{
	// GENERATE MESH
	if( z0 != z1 )
	{
		for( size_t i = 0; i < poly.size(); ++i )
		{
			Vec3 tgx, tgy;
			_GetTexVecs( i, tgx, tgy );
			Vec3 nrm = -Vec3Cross( tgx, tgy ).Normalized();
			size_t i1 = ( i + 1 ) % poly.size();
			LevelCache::Vertex verts[] =
			{
				_MakeGenVtx( poly[i], z0, *surfaces[i], tgx, tgy ),
				_MakeGenVtx( poly[i], z1 + poly[i].z, *surfaces[i], tgx, tgy ),
				_MakeGenVtx( poly[i1], z1 + poly[i1].z, *surfaces[i], tgx, tgy ),
				_MakeGenVtx( poly[i1], z0, *surfaces[i], tgx, tgy ),
			};
			objex.AddVertex( verts[0].pos, V2( verts[0].tx0, verts[0].ty0 ), nrm );
			objex.AddVertex( verts[1].pos, V2( verts[1].tx0, verts[1].ty0 ), nrm );
			objex.AddVertex( verts[2].pos, V2( verts[2].tx0, verts[2].ty0 ), nrm );
			objex.AddVertex( verts[2].pos, V2( verts[2].tx0, verts[2].ty0 ), nrm );
			objex.AddVertex( verts[3].pos, V2( verts[3].tx0, verts[3].ty0 ), nrm );
			objex.AddVertex( verts[0].pos, V2( verts[0].tx0, verts[0].ty0 ), nrm );
		}
	}
	
	// TOP
	{
		LevelCache::Vertex verts[ MAX_BLOCK_POLYGONS - 2 ];
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size(), tgx, tgy );
		Vec3 nrm = -Vec3Cross( tgx, tgy ).Normalized();
		for( size_t i = 0; i < poly.size(); ++i )
			verts[ poly.size() - 1 - i ] = _MakeGenVtx( poly[i], z1 + poly[i].z, *surfaces[ poly.size() ], tgx, tgy );
		for( size_t i = 1; i < poly.size() - 1; ++i )
		{
			objex.AddVertex( verts[0].pos, V2( verts[0].tx0, verts[0].ty0 ), nrm );
			objex.AddVertex( verts[i].pos, V2( verts[i].tx0, verts[i].ty0 ), nrm );
			objex.AddVertex( verts[i+1].pos, V2( verts[i+1].tx0, verts[i+1].ty0 ), nrm );
		}
	}
	
	// BOTTOM
	{
		LevelCache::Vertex verts[ MAX_BLOCK_POLYGONS - 2 ];
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size() + 1, tgx, tgy );
		Vec3 nrm = -Vec3Cross( tgx, tgy ).Normalized();
		for( size_t i = 0; i < poly.size(); ++i )
			verts[ i ] = _MakeGenVtx( poly[i], z0, *surfaces[ poly.size() + 1 ], tgx, tgy );
		for( size_t i = 1; i < poly.size() - 1; ++i )
		{
			objex.AddVertex( verts[0].pos, V2( verts[0].tx0, verts[0].ty0 ), nrm );
			objex.AddVertex( verts[i].pos, V2( verts[i].tx0, verts[i].ty0 ), nrm );
			objex.AddVertex( verts[i+1].pos, V2( verts[i+1].tx0, verts[i+1].ty0 ), nrm );
		}
	}
}


