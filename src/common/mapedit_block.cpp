

#include "mapedit.hpp"



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
	
	if( numbottom || IsSurfaceSelected( GetNumSurfs() - 2 ) )
		z0 += t.z;
	if( numtop == pc || IsSurfaceSelected( GetNumSurfs() - 1 ) )
	{
		z1 += t.z;
		t.z = 0;
	}
	if( numtop == 0 )
		t.z = 0;
	
	// either top or bottom is fully selected
	if( numtop == pc || numbottom == pc || 
		IsSurfaceSelected( GetNumSurfs() - 2 ) ||
		IsSurfaceSelected( GetNumSurfs() - 1 ) )
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

void EdBlock::GenCenterPos( EDGUISnapProps& SP )
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
	blk->solid_id = 0;
	for( size_t i = 0; i < blk->surfaces.size(); ++i )
		blk->surfaces[ i ].surface_id = 0;
	return blk;
}

bool EdBlock::RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1], int* outsurf ) const
{
	Vec3 pts[16];
	int pcount = poly.size();
	
	Mat4 tf = g_EdWorld->m_groupMgr.GetMatrix( group );
	
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
	
	Mat4 mtx = g_EdWorld->m_groupMgr.GetMatrix( group );
	
	
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
		EdSurface& BS = surfaces[ i ];
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
	if( solid_id )
		g_EdLGCont->UpdateSolid( solid_id, &SOI );
	else
		solid_id = g_EdLGCont->CreateSolid( &SOI );
	
	
	Array< LCVertex > vertices;
	Array< uint16_t > indices;
	
	// SIDES
	for( size_t i = 0; i < poly.size(); ++i )
	{
		EdSurface& BS = surfaces[ i ];
		
		vertices.clear();
		indices.clear();
		
		Vec3 tgx, tgy;
		_GetTexVecs( i, tgx, tgy );
		size_t i1 = ( i + 1 ) % poly.size();
		uint16_t v1 = _AddVtx( poly[i], z0, BS, tgx, tgy, vertices, 0 );
		uint16_t v2 = _AddVtx( poly[i], z1 + poly[i].z, BS, tgx, tgy, vertices, 0 );
		uint16_t v3 = _AddVtx( poly[i1], z1 + poly[i1].z, BS, tgx, tgy, vertices, 0 );
		uint16_t v4 = _AddVtx( poly[i1], z0, BS, tgx, tgy, vertices, 0 );
		if( vertices.size() < 3 )
			continue;
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
		S.solid_id = solid_id;
		
		if( BS.surface_id )
			g_EdLGCont->UpdateSurface( BS.surface_id, LGC_CHANGE_ALL, &S );
		else
			BS.surface_id = g_EdLGCont->CreateSurface( &S );
	}
	
	// TOP
	for(;;)
	{
		EdSurface& BS = surfaces[ poly.size() ];
		
		vertices.clear();
		indices.clear();
		
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size(), tgx, tgy );
		for( size_t i = 0; i < poly.size(); ++i )
			_AddVtx( poly[i], z1 + poly[i].z, BS, tgx, tgy, vertices, 0 );
		if( vertices.size() < 3 )
			break;
		_PostFitTexcoords( BS, vertices.data(), vertices.size() );
		for( size_t i = 2; i < poly.size(); ++i )
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
		S.solid_id = solid_id;
		
		if( BS.surface_id )
			g_EdLGCont->UpdateSurface( BS.surface_id, LGC_CHANGE_ALL, &S );
		else
			BS.surface_id = g_EdLGCont->CreateSurface( &S );
		break;
	}
	
	// BOTTOM
	for(;;)
	{
		EdSurface& BS = surfaces[ poly.size() + 1 ];
		
		vertices.clear();
		indices.clear();
		
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size() + 1, tgx, tgy );
		for( size_t i = 0; i < poly.size(); ++i )
			_AddVtx( poly[i], z0, BS, tgx, tgy, vertices, 0 );
		if( vertices.size() < 3 )
			break;
		_PostFitTexcoords( BS, vertices.data(), vertices.size() );
		for( size_t i = 2; i < poly.size(); ++i )
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
		S.solid_id = solid_id;
		
		if( BS.surface_id )
			g_EdLGCont->UpdateSurface( BS.surface_id, LGC_CHANGE_ALL, &S );
		else
			BS.surface_id = g_EdLGCont->CreateSurface( &S );
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
			outbuf[5] = outbuf[0] = _MakeGenVtx( poly[i], z0, surfaces[i], tgx, tgy );
			outbuf[1] = _MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[i], tgx, tgy );
			outbuf[3] = outbuf[2] = _MakeGenVtx( poly[i1], z1 + poly[i1].z, surfaces[i], tgx, tgy );
			outbuf[4] = _MakeGenVtx( poly[i1], z0, surfaces[i], tgx, tgy );
		}
		else if( sid == (int) poly.size() )
		{
			for( size_t i = 1; i + 1 < poly.size(); ++i )
			{
				outbuf[ (i-1)*3+0 ] = _MakeGenVtx( poly[0], z1 + poly[0].z, surfaces[ poly.size() ], tgx, tgy );
				outbuf[ (i-1)*3+2 ] = _MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[ poly.size() ], tgx, tgy );
				outbuf[ (i-1)*3+1 ] = _MakeGenVtx( poly[i+1], z1 + poly[i+1].z, surfaces[ poly.size() ], tgx, tgy );
			}
			retval = ( poly.size() - 2 ) * 3;
		}
		else // if( sid == (int) poly.size() + 1 )
		{
			for( size_t i = 1; i + 1 < poly.size(); ++i )
			{
				outbuf[ (i-1)*3+0 ] = _MakeGenVtx( poly[0], z0, surfaces[ poly.size() ], tgx, tgy );
				outbuf[ (i-1)*3+1 ] = _MakeGenVtx( poly[i], z0, surfaces[ poly.size() ], tgx, tgy );
				outbuf[ (i-1)*3+2 ] = _MakeGenVtx( poly[i+1], z0, surfaces[ poly.size() ], tgx, tgy );
			}
			retval = ( poly.size() - 2 ) * 3;
		}
		if( fit )
			_PostFitTexcoords( surfaces[ sid ], outbuf, retval );
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
			outbuf[0] = _MakeGenVtx( poly[i], z0, surfaces[i], tgx, tgy );
			outbuf[1] = _MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[i], tgx, tgy );
			outbuf[2] = _MakeGenVtx( poly[i1], z1 + poly[i1].z, surfaces[i], tgx, tgy );
			outbuf[3] = _MakeGenVtx( poly[i1], z0, surfaces[i], tgx, tgy );
		}
		else if( sid == (int) poly.size() )
		{
			for( size_t i = 0; i < poly.size(); ++i )
			{
				outbuf[ poly.size() - 1 - i ] = _MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[ poly.size() ], tgx, tgy );
			}
			retval = poly.size();
		}
		else // if( sid == (int) poly.size() + 1 )
		{
			for( size_t i = 0; i < poly.size(); ++i )
			{
				outbuf[ i ] = _MakeGenVtx( poly[i], z0, surfaces[ poly.size() ], tgx, tgy );
			}
			retval = poly.size();
		}
		if( fit )
			_PostFitTexcoords( surfaces[ sid ], outbuf, retval );
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
				_MakeGenVtx( poly[i], z0, surfaces[i], tgx, tgy ),
				_MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[i], tgx, tgy ),
				_MakeGenVtx( poly[i1], z1 + poly[i1].z, surfaces[i], tgx, tgy ),
				_MakeGenVtx( poly[i1], z0, surfaces[i], tgx, tgy ),
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
			verts[ poly.size() - 1 - i ] = _MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[ poly.size() ], tgx, tgy );
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
			verts[ i ] = _MakeGenVtx( poly[i], z0, surfaces[ poly.size() + 1 ], tgx, tgy );
		for( size_t i = 1; i < poly.size() - 1; ++i )
		{
			objex.AddVertex( verts[0].pos, V2( verts[0].tx0, verts[0].ty0 ), nrm );
			objex.AddVertex( verts[i].pos, V2( verts[i].tx0, verts[i].ty0 ), nrm );
			objex.AddVertex( verts[i+1].pos, V2( verts[i+1].tx0, verts[i+1].ty0 ), nrm );
		}
	}
}


EDGUIVertexProps::EDGUIVertexProps() :
	m_out( NULL ),
	m_vid( 0 ),
	m_pos( V3(0), 2, V3(-8192), V3(8192) )
{
	tyname = "blockvertprops";
	m_group.caption = "Block vertex properties";
	m_pos.caption = "Offset";
	m_insbef.caption = "Insert before";
	m_insaft.caption = "Insert after";
	
	m_group.Add( &m_pos );
	m_group.Add( &m_insbef );
	m_group.Add( &m_insaft );
	m_group.SetOpen( true );
	Add( &m_group );
}

void EDGUIVertexProps::Prepare( EdBlock* block, int vid )
{
	vid %= block->poly.size();
	
	m_out = block;
	m_vid = vid;
	
	char bfr[ 32 ];
	sgrx_snprintf( bfr, sizeof(bfr), "Vertex #%d", vid );
	m_group.caption = bfr;
	m_group.SetOpen( true );
	
	m_pos.SetValue( block->poly[ vid ] );
}

int EDGUIVertexProps::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( m_out && e->target == &m_pos )
		{
			m_out->poly[ m_vid ] = m_pos.m_value;
			m_out->RegenerateMesh();
		}
		break;
	case EDGUI_EVENT_BTNCLICK:
		if( ( e->target == &m_insbef || e->target == &m_insaft ) && m_out->poly.size() < 14 )
		{
			size_t insat = m_vid, sz = m_out->poly.size();
			if( e->target == &m_insaft )
				insat = ( insat + 1 ) % sz;
			size_t befat = ( insat + sz - 1 ) % sz;
			
			Vec2 p0 = m_out->poly[ ( befat + sz - 1 ) % sz ].ToVec2();
			Vec2 p1 = m_out->poly[ befat ].ToVec2();
			Vec2 p2 = m_out->poly[ insat ].ToVec2();
			Vec2 p3 = m_out->poly[ ( insat + 1 ) % sz ].ToVec2();
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
			Vec3 mid_fin = { mid.x, mid.y, ( m_out->poly[ befat ].z + m_out->poly[ insat ].z ) * 0.5f };
			
			size_t oldpolysize = m_out->poly.size();
			EdSurface Scopy = m_out->surfaces[ befat ];
			Scopy.surface_id = 0;
			m_out->poly.insert( insat, mid_fin );
			if( (int) insat < m_vid )
				m_vid++;
			m_out->surfaces.insert( insat, Scopy );
			
			m_out->subsel.insert( oldpolysize * 2 + insat, false );
			m_out->subsel.insert( oldpolysize + insat, false );
			m_out->subsel.insert( insat, false );
			
			m_out->RegenerateMesh();
			return 1;
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}


EDGUISurfaceProps::EDGUISurfaceProps() :
	m_out( NULL ),
	m_sid( 0 ),
	m_tex( g_UISurfMtlPicker, "null" ),
	m_off( V2(0), 2, V2(0), V2(1) ),
	m_scaleasp( V2(1), 2, V2(0.01f), V2(100) ),
	m_angle( 0, 1, 0, 360 ),
	m_lmquality( 1, 2, 0.01f, 100.0f ),
	m_xfit( 0, 0, 100 ),
	m_yfit( 0, 0, 100 )
{
	tyname = "surfaceprops";
	m_group.caption = "Surface properties";
	m_tex.caption = "Texture";
	m_off.caption = "Offset";
	m_scaleasp.caption = "Scale/Aspect";
	m_angle.caption = "Angle";
	m_lmquality.caption = "Lightmap quality";
	m_xfit.caption = "Fit count on X";
	m_yfit.caption = "Fit count on Y";
	m_resetOffScaleAsp.caption = "Reset offset/scale/aspect";
	m_applyFit.caption = "Apply fit";
	m_makeBlendPatch.caption = "Make blend patch from surf.";
	m_convertToPatch.caption = "Convert block to patch";
	
	m_group.Add( &m_tex );
	m_group.Add( &m_off );
	m_group.Add( &m_scaleasp );
	m_group.Add( &m_angle );
	m_group.Add( &m_lmquality );
	m_group.Add( &m_xfit );
	m_group.Add( &m_yfit );
	m_group.Add( &m_resetOffScaleAsp );
	m_group.Add( &m_applyFit );
	m_group.Add( &m_makeBlendPatch );
	m_group.Add( &m_convertToPatch );
	m_group.SetOpen( true );
	Add( &m_group );
}

void EDGUISurfaceProps::Prepare( EdBlock* block, int sid )
{
	m_out = block;
	m_sid = sid;
	EdSurface& S = block->surfaces[ sid ];
	
	char bfr[ 32 ];
	sgrx_snprintf( bfr, sizeof(bfr), "Surface #%d", sid );
	LoadParams( S, bfr );
}

void EDGUISurfaceProps::LoadParams( EdSurface& S, const char* name )
{
	m_group.caption = name;
	m_group.SetOpen( true );
	
	m_tex.SetValue( S.texname );
	m_off.SetValue( V2( S.xoff, S.yoff ) );
	m_scaleasp.SetValue( V2( S.scale, S.aspect ) );
	m_angle.SetValue( S.angle );
	m_lmquality.SetValue( S.lmquality );
	m_xfit.SetValue( S.xfit );
	m_yfit.SetValue( S.yfit );
}

void EDGUISurfaceProps::BounceBack( EdSurface& S )
{
	S.texname = m_tex.m_value;
	S.xoff = m_off.m_value.x;
	S.yoff = m_off.m_value.y;
	S.scale = m_scaleasp.m_value.x;
	S.aspect = m_scaleasp.m_value.y;
	S.angle = m_angle.m_value;
	S.lmquality = m_lmquality.m_value;
	S.xfit = m_xfit.m_value;
	S.yfit = m_yfit.m_value;
}

int EDGUISurfaceProps::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_BTNCLICK:
		if( m_out && ( e->target == &m_makeBlendPatch || e->target == &m_convertToPatch ) )
		{
			EdPatch* p = EdPatch::CreatePatchFromSurface( *m_out, m_sid );
			if( p )
			{
				p->selected = true;
				if( e->target != &m_makeBlendPatch )
				{
					p->blend = PATCH_IS_SOLID;
					g_EdWorld->DeleteObject( m_out );
					g_UIFrame->SetEditMode( &g_UIFrame->m_emEditObjs );
				}
				g_EdWorld->AddObject( p );
			}
		}
		if( m_out && e->target == &m_resetOffScaleAsp )
		{
			EdSurface& S = m_out->surfaces[ m_sid ];
			S.xoff = 0;
			S.yoff = 0;
			S.scale = 1;
			S.aspect = 1;
			m_out->RegenerateMesh();
			Prepare( m_out, m_sid );
		}
		if( m_out && e->target == &m_applyFit )
		{
			EdSurface& S = m_out->surfaces[ m_sid ];
			float xmin = FLT_MAX, xmax = -FLT_MAX, ymin = FLT_MAX, ymax = -FLT_MAX;
			LCVertex vertices[ MAX_BLOCK_POLYGONS ];
			int vcount = m_out->GenerateSurface( vertices, m_sid, false, false );
			for( int i = 0; i < vcount; ++i )
			{
				float x = vertices[ i ].tx0;
				float y = vertices[ i ].ty0;
				if( xmin > x ) xmin = x;
				if( xmax < x ) xmax = x;
				if( ymin > y ) ymin = y;
				if( ymax < y ) ymax = y;
			}
			float xdst = xmax - xmin, ydst = ymax - ymin;
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
			m_out->RegenerateMesh();
			Prepare( m_out, m_sid );
		}
		break;
		
	case EDGUI_EVENT_PROPEDIT:
		if( m_out && (
			e->target == &m_tex ||
			e->target == &m_off ||
			e->target == &m_scaleasp ||
			e->target == &m_angle ||
			e->target == &m_lmquality ||
			e->target == &m_xfit ||
			e->target == &m_yfit
		) )
		{
			if( e->target == &m_tex )
			{
				m_out->surfaces[ m_sid ].texname = m_tex.m_value;
			}
			else if( e->target == &m_off )
			{
				m_out->surfaces[ m_sid ].xoff = m_off.m_value.x;
				m_out->surfaces[ m_sid ].yoff = m_off.m_value.y;
			}
			else if( e->target == &m_scaleasp )
			{
				m_out->surfaces[ m_sid ].scale = m_scaleasp.m_value.x;
				m_out->surfaces[ m_sid ].aspect = m_scaleasp.m_value.y;
			}
			else if( e->target == &m_angle )
			{
				m_out->surfaces[ m_sid ].angle = m_angle.m_value;
			}
			else if( e->target == &m_lmquality )
			{
				m_out->surfaces[ m_sid ].lmquality = m_lmquality.m_value;
			}
			else if( e->target == &m_xfit )
			{
				m_out->surfaces[ m_sid ].xfit = m_xfit.m_value;
			}
			else if( e->target == &m_yfit )
			{
				m_out->surfaces[ m_sid ].yfit = m_yfit.m_value;
			}
			m_out->RegenerateMesh();
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}


EDGUIBlockProps::EDGUIBlockProps() :
	m_out( NULL ),
	m_group( true, "Block properties" ),
	m_vertGroup( false, "Vertices" ),
	m_z0( 0, 2, -8192, 8192 ),
	m_z1( 2, 2, -8192, 8192 ),
	m_pos( V3(0), 2, V3(-8192), V3(8192) ),
	m_blkGroup( NULL )
{
	tyname = "blockprops";
	m_z0.caption = "Bottom height";
	m_z1.caption = "Top height";
	m_pos.caption = "Position";
	m_blkGroup.caption = "Group";
}

void EDGUIBlockProps::Prepare( EdBlock* block )
{
	m_out = block;
	m_blkGroup.m_rsrcPicker = &g_EdWorld->m_groupMgr.m_grpPicker;
	
	Clear();
	
	Add( &m_group );
	m_blkGroup.SetValue( g_EdWorld->m_groupMgr.GetPath( block->group ) );
	m_group.Add( &m_blkGroup );
	m_z0.SetValue( block->z0 );
	m_group.Add( &m_z0 );
	m_z1.SetValue( block->z1 );
	m_group.Add( &m_z1 );
	m_pos.SetValue( block->position );
	m_group.Add( &m_pos );
	m_group.Add( &m_vertGroup );
	
	m_vertProps.clear();
	m_vertProps.resize( block->poly.size() );
	for( size_t i = 0; i < block->poly.size(); ++i )
	{
		char bfr[ 4 ];
		sgrx_snprintf( bfr, sizeof(bfr), "#%d", (int) i );
		m_vertProps[ i ] = EDGUIPropVec3( block->poly[ i ], 2, V3(-8192), V3(8192) );
		m_vertProps[ i ].caption = bfr;
		m_vertProps[ i ].id1 = i;
		m_vertGroup.Add( &m_vertProps[ i ] );
	}
	
	m_surfProps.clear();
	m_surfProps.resize( block->surfaces.size() );
	for( size_t i = 0; i < block->surfaces.size(); ++i )
	{
		m_surfProps[ i ].Prepare( block, i );
		m_group.Add( &m_surfProps[ i ] );
	}
}

int EDGUIBlockProps::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_z0 || e->target == &m_z1 || e->target == &m_pos || e->target == &m_blkGroup )
		{
			if( e->target == &m_z0 )
			{
				m_out->z0 = m_z0.m_value;
			}
			else if( e->target == &m_z1 )
			{
				m_out->z1 = m_z1.m_value;
			}
			else if( e->target == &m_pos )
			{
				m_out->position = m_pos.m_value;
			}
			else if( e->target == &m_blkGroup )
			{
				EdGroup* grp = g_EdWorld->m_groupMgr.FindGroupByPath( m_blkGroup.m_value );
				if( grp )
					m_out->group = grp->m_id;
			}
			m_out->RegenerateMesh();
		}
		{
			size_t at = m_vertGroup.m_subitems.find_first_at( e->target );
			if( at != NOT_FOUND )
			{
				m_out->poly[ at ] = m_vertProps[ at ].m_value;
				m_out->RegenerateMesh();
			}
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}


