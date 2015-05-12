

#include "mapedit.hpp"



Vec3 EdBlock::GetLocalVertex( int i )
{
	Vec3 vp = poly[ i % poly.size() ];
	if( i < (int) poly.size() )
		vp.z = z0;
	else
		vp.z += z1;
	return vp + position;
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

Vec3 EdBlock::GetSurfaceCenter( int i )
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

Vec3 EdBlock::GetElementPoint( int i )
{
	int nverts = GetNumVerts();
	if( i < nverts )
		return GetLocalVertex( i );
	else
		return GetSurfaceCenter( i - nverts );
}


bool EdBlock::IsVertexSelected( int i )
{
	ASSERT( i >= 0 && i < GetNumVerts() );
	return subsel[ i ];
}

void EdBlock::SelectVertex( int i, bool sel )
{
	ASSERT( i >= 0 && i < GetNumVerts() );
	subsel[ i ] = sel;
}

int EdBlock::GetOnlySelectedVertex()
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

bool EdBlock::IsElementSelected( int i )
{
	ASSERT( i >= 0 && i < GetNumElements() );
	return subsel[ i ];
}

void EdBlock::SelectElement( int i, bool sel )
{
	ASSERT( i >= 0 && i < GetNumElements() );
	subsel[ i ] = sel;
}

void EdBlock::UISelectElement( int i, bool mod )
{
	if( i >= 0 && i < GetNumElements() )
	{
		if( mod )
			subsel[ i ] = !subsel[ i ];
		else
		{
			ClearSelection();
			subsel[ i ] = true;
		}
		
	}
	else if( mod == false )
		ClearSelection();
}

void EdBlock::ClearSelection()
{
	TMEMSET( subsel.data(), subsel.size(), false );
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

Vec3 EdBlock::FindCenter()
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

bool EdBlock::RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1], int* outsurf )
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

void EdBlock::RegenerateMesh()
{
	if( !g_EdWorld || poly.size() < 3 || poly.size() > MAX_BLOCK_POLYGONS - 2 )
		return;
	
	if( !cached_mesh )
		cached_mesh = GR_CreateMesh();
	if( !cached_meshinst )
	{
		cached_meshinst = g_EdScene->CreateMeshInstance();
		cached_meshinst->mesh = cached_mesh;
		lmm_prepmeshinst( cached_meshinst );
	}
	cached_meshinst->matrix = g_EdWorld->m_groupMgr.GetMatrix( group );
	for( size_t i = 0; i < surfaces.size(); ++i )
		surfaces[ i ].Precache();
	
	VertexDeclHandle vd = GR_GetVertexDecl( LCVertex_DECL );
	Array< LCVertex > vertices;
	Array< uint16_t > indices;
	SGRX_MeshPart meshparts[ MAX_BLOCK_POLYGONS ];
	int numparts = 0;
	
	// SIDES
	if( z0 != z1 )
	{
		for( size_t i = 0; i < poly.size(); ++i )
		{
			SGRX_MeshPart mp = { vertices.size(), 0, indices.size(), 0 };
			
			Vec3 tgx, tgy;
			_GetTexVecs( i, tgx, tgy );
			size_t i1 = ( i + 1 ) % poly.size();
			uint16_t v1 = _AddVtx( poly[i], z0, surfaces[i], tgx, tgy, vertices, mp.vertexOffset );
			uint16_t v2 = _AddVtx( poly[i], z1 + poly[i].z, surfaces[i], tgx, tgy, vertices, mp.vertexOffset );
			uint16_t v3 = _AddVtx( poly[i1], z1 + poly[i1].z, surfaces[i], tgx, tgy, vertices, mp.vertexOffset );
			uint16_t v4 = _AddVtx( poly[i1], z0, surfaces[i], tgx, tgy, vertices, mp.vertexOffset );
			if( vertices.size() < mp.vertexOffset + 4 )
			{
				vertices.resize( mp.vertexOffset );
				continue;
			}
			_PostFitTexcoords( surfaces[i], &vertices[ vertices.size() - 4 ], 4 );
			indices.push_back( v1 );
			indices.push_back( v2 );
			indices.push_back( v3 );
			indices.push_back( v3 );
			indices.push_back( v4 );
			indices.push_back( v1 );
			
			mp.vertexCount = vertices.size() - mp.vertexOffset;
			mp.indexCount = indices.size() - mp.indexOffset;
			
			MaterialHandle mh = GR_CreateMaterial();
			mh->shader = GR_GetSurfaceShader( "default" );
			mh->textures[ 0 ] = surfaces[ i ].cached_texture;
			mp.material = mh;
			
			meshparts[ numparts++ ] = mp;
		}
	}
	
	// TOP
	for(;;)
	{
		SGRX_MeshPart mp = { vertices.size(), 0, indices.size(), 0 };
		
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size(), tgx, tgy );
		for( size_t i = 0; i < poly.size(); ++i )
			_AddVtx( poly[i], z1 + poly[i].z, surfaces[ poly.size() ], tgx, tgy, vertices, mp.vertexOffset );
		if( vertices.size() < mp.vertexOffset + 3 )
		{
			vertices.resize( mp.vertexOffset );
			break;
		}
		_PostFitTexcoords( surfaces[ poly.size() ], &vertices[ mp.vertexOffset ], vertices.size() - mp.vertexOffset );
		for( size_t i = 2; i < poly.size(); ++i )
		{
			indices.push_back( 0 );
			indices.push_back( i );
			indices.push_back( i - 1 );
		}
		
		mp.vertexCount = vertices.size() - mp.vertexOffset;
		mp.indexCount = indices.size() - mp.indexOffset;
		
		MaterialHandle mh = GR_CreateMaterial();
		mh->shader = GR_GetSurfaceShader( "default" );
		mh->textures[ 0 ] = surfaces[ poly.size() ].cached_texture;
		mp.material = mh;
		
		meshparts[ numparts++ ] = mp;
		break;
	}
	
	// BOTTOM
	for(;;)
	{
		SGRX_MeshPart mp = { vertices.size(), 0, indices.size(), 0 };
		
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size() + 1, tgx, tgy );
		for( size_t i = 0; i < poly.size(); ++i )
			_AddVtx( poly[i], z0, surfaces[ poly.size() + 1 ], tgx, tgy, vertices, mp.vertexOffset );
		if( vertices.size() < mp.vertexOffset + 3 )
		{
			vertices.resize( mp.vertexOffset );
			break;
		}
		_PostFitTexcoords( surfaces[ poly.size() + 1 ], &vertices[ mp.vertexOffset ], vertices.size() - mp.vertexOffset );
		for( size_t i = 2; i < poly.size(); ++i )
		{
			indices.push_back( 0 );
			indices.push_back( i - 1 );
			indices.push_back( i );
		}
		
		mp.vertexCount = vertices.size() - mp.vertexOffset;
		mp.indexCount = indices.size() - mp.indexOffset;
		
		MaterialHandle mh = GR_CreateMaterial();
		mh->shader = GR_GetSurfaceShader( "default" );
		mh->textures[ 0 ] = surfaces[ poly.size() + 1 ].cached_texture;
		mp.material = mh;
		
		meshparts[ numparts++ ] = mp;
		break;
	}
	
	cached_mesh->SetAABBFromVertexData( vertices.data(), vertices.size_bytes(), vd );
	cached_mesh->SetVertexData( vertices.data(), vertices.size_bytes(), vd, false );
	cached_mesh->SetIndexData( indices.data(), indices.size_bytes(), false );
	cached_mesh->SetPartData( meshparts, numparts );
}

LevelCache::Vertex EdBlock::_MakeGenVtx( const Vec3& vpos, float z, const EdSurface& S, const Vec3& tgx, const Vec3& tgy )
{
	LevelCache::Vertex V = { { vpos.x + position.x, vpos.y + position.y, z + position.z }, { 0, 0, 1 }, 0xffffffff, 0, 0, 0, 0 };
	
	Vec2 tx =
	{
		Vec3Dot( V.pos, tgx ),
		Vec3Dot( V.pos, tgy )
	};
	tx = tx.Rotate( DEG2RAD( S.angle ) );
	V.tx0 = tx.x / S.scale + S.xoff;
	V.ty0 = tx.y / S.scale * S.aspect + S.yoff;
	
	return V;
}

void EdBlock::GenerateMesh( LevelCache& LC )
{
	if( poly.size() < 3 || poly.size() > MAX_BLOCK_POLYGONS - 2 )
		return;
	
	// GENERATE PLANES
	Vec3 toppoly[ MAX_BLOCK_POLYGONS ];
	int topverts = 0;
	Vec4 planes[ MAX_BLOCK_POLYGONS ];
	int numplanes = 0;
	
	planes[ numplanes++ ] = V4( 0, 0, -1, -z0 );
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
	
	// ADD SOLID
	size_t solid = LC.AddSolid( planes, numplanes );
	
	// GENERATE MESH
	if( z0 != z1 )
	{
		for( size_t i = 0; i < poly.size(); ++i )
		{
			Vec3 tgx, tgy;
			_GetTexVecs( i, tgx, tgy );
			size_t i1 = ( i + 1 ) % poly.size();
			LevelCache::Vertex verts[] =
			{
				_MakeGenVtx( poly[i], z0, surfaces[i], tgx, tgy ),
				_MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[i], tgx, tgy ),
				_MakeGenVtx( poly[i1], z1 + poly[i1].z, surfaces[i], tgx, tgy ),
				_MakeGenVtx( poly[i1], z0, surfaces[i], tgx, tgy ),
			};
			_PostFitTexcoords( surfaces[i], verts, 4 );
			LC.AddPoly( verts, 4, surfaces[ i ].texname, surfaces[ i ].lmquality, solid );
		}
	}
	
	// TOP
	{
		LevelCache::Vertex verts[ MAX_BLOCK_POLYGONS - 2 ];
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size(), tgx, tgy );
		for( size_t i = 0; i < poly.size(); ++i )
			verts[ poly.size() - 1 - i ] = _MakeGenVtx( poly[i], z1 + poly[i].z, surfaces[ poly.size() ], tgx, tgy );
		_PostFitTexcoords( surfaces[ poly.size() ], verts, poly.size() );
		LC.AddPoly( verts, poly.size(), surfaces[ poly.size() ].texname, surfaces[ poly.size() ].lmquality, solid );
	}
	
	// BOTTOM
	{
		LevelCache::Vertex verts[ MAX_BLOCK_POLYGONS - 2 ];
		Vec3 tgx, tgy;
		_GetTexVecs( poly.size() + 1, tgx, tgy );
		for( size_t i = 0; i < poly.size(); ++i )
			verts[ i ] = _MakeGenVtx( poly[i], z0, surfaces[ poly.size() + 1 ], tgx, tgy );
		_PostFitTexcoords( surfaces[ poly.size() + 1 ], verts, poly.size() );
		LC.AddPoly( verts, poly.size(), surfaces[ poly.size() + 1 ].texname, surfaces[ poly.size() + 1 ].lmquality, solid );
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
	tyname = "surfaceprops";
	m_group.caption = "Vertex properties";
	m_pos.caption = "Offset";
	m_insbef.caption = "Insert before";
	m_insaft.caption = "Insert after";
	
	m_group.Add( &m_pos );
	m_group.Add( &m_insbef );
	m_group.Add( &m_insaft );
	m_group.SetOpen( true );
	Add( &m_group );
}

void EDGUIVertexProps::Prepare( EdBlock& B, int vid )
{
	vid %= B.poly.size();
	
	m_out = &B;
	m_vid = vid;
	
	char bfr[ 32 ];
	snprintf( bfr, sizeof(bfr), "Vertex #%d", vid );
	m_group.caption = bfr;
	m_group.SetOpen( true );
	
	m_pos.SetValue( B.poly[ vid ] );
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
	m_tex( g_UISurfTexPicker, "metal0" ),
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
	
	m_group.Add( &m_tex );
	m_group.Add( &m_off );
	m_group.Add( &m_scaleasp );
	m_group.Add( &m_angle );
	m_group.Add( &m_lmquality );
	m_group.Add( &m_xfit );
	m_group.Add( &m_yfit );
	m_group.SetOpen( true );
	Add( &m_group );
}

void EDGUISurfaceProps::Prepare( EdBlock& B, int sid )
{
	m_out = &B;
	m_sid = sid;
	EdSurface& S = B.surfaces[ sid ];
	
	char bfr[ 32 ];
	snprintf( bfr, sizeof(bfr), "Surface #%d", sid );
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

void EDGUIBlockProps::Prepare( EdBlock& B )
{
	m_out = &B;
	m_blkGroup.m_rsrcPicker = &g_EdWorld->m_groupMgr.m_grpPicker;
	
	Clear();
	
	Add( &m_group );
	m_blkGroup.SetValue( g_EdWorld->m_groupMgr.GetPath( B.group ) );
	m_group.Add( &m_blkGroup );
	m_z0.SetValue( B.z0 );
	m_group.Add( &m_z0 );
	m_z1.SetValue( B.z1 );
	m_group.Add( &m_z1 );
	m_pos.SetValue( B.position );
	m_group.Add( &m_pos );
	m_group.Add( &m_vertGroup );
	
	m_vertProps.clear();
	m_vertProps.resize( B.poly.size() );
	for( size_t i = 0; i < B.poly.size(); ++i )
	{
		char bfr[ 4 ];
		snprintf( bfr, sizeof(bfr), "#%d", (int) i );
		m_vertProps[ i ] = EDGUIPropVec3( B.poly[ i ], 2, V3(-8192), V3(8192) );
		m_vertProps[ i ].caption = bfr;
		m_vertProps[ i ].id1 = i;
		m_vertGroup.Add( &m_vertProps[ i ] );
	}
	
	m_surfProps.clear();
	m_surfProps.resize( B.surfaces.size() );
	for( size_t i = 0; i < B.surfaces.size(); ++i )
	{
		m_surfProps[ i ].Prepare( B, i );
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


