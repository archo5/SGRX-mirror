

#include "mapedit.hpp"



bool ExtractLCVDataFromMesh( SGRX_IMesh* mesh, Array<LCVertex>& vertices, Array<uint16_t>& indices )
{
	if( !mesh->m_vertexDecl )
		return false;
	
	VDeclInfo& VDI = mesh->m_vertexDecl->m_info;
	int vc = mesh->GetBufferVertexCount();
	const void* vdata = mesh->m_vdata.data();
	
	// preallocate memory
	Array<Vec4> v4a;
	v4a.resize( vc );
	Vec4* v4data = v4a.data();
	SGRX_CAST( Vec2*, v2data, v4data ); // reuse same memory (allocated for largest alignment)
	SGRX_CAST( Vec3*, v3data, v4data );
	SGRX_CAST( uint32_t*, b4data, v4data );
	
	// clear vertex data
	vertices.resize( vc );
	for( int i = 0; i < vc; ++i )
	{
		vertices[ i ].SetDefault();
	}
	
	// position
	if( VD_ExtractFloat3( VDI, vc, vdata, v3data, VDECLUSAGE_POSITION ) )
	{
		for( int i = 0; i < vc; ++i )
			vertices[ i ].pos = v3data[ i ];
	}
	else return false;
	
	// normal
	if( VD_ExtractFloat3( VDI, vc, vdata, v3data, VDECLUSAGE_NORMAL ) )
	{
		for( int i = 0; i < vc; ++i )
			vertices[ i ].nrm = v3data[ i ];
	}
	else return false;
	
	// tangent
	if( VD_ExtractFloat4( VDI, vc, vdata, v4data, VDECLUSAGE_TANGENT ) )
	{
		for( int i = 0; i < vc; ++i )
			vertices[ i ].tng = v4data[ i ];
	}
	else return false;
	
	// color
	if( VD_ExtractByte4Clamped( VDI, vc, vdata, b4data, VDECLUSAGE_COLOR ) )
	{
		for( int i = 0; i < vc; ++i )
			vertices[ i ].color = b4data[ i ];
	}
	
	// texcoord 0
	if( VD_ExtractFloat2( VDI, vc, vdata, v2data, VDECLUSAGE_TEXTURE0 ) )
	{
		for( int i = 0; i < vc; ++i )
		{
			vertices[ i ].tx0 = v2data[ i ].x;
			vertices[ i ].ty0 = v2data[ i ].y;
		}
	}
	
	// texcoord 1
	if( VD_ExtractFloat2( VDI, vc, vdata, v2data, VDECLUSAGE_TEXTURE1 ) )
	{
		for( int i = 0; i < vc; ++i )
		{
			vertices[ i ].tx1 = v2data[ i ].x;
			vertices[ i ].ty1 = v2data[ i ].y;
		}
	}
	
	// indices
	int ic = mesh->GetBufferIndexCount();
	indices.resize( ic );
	if( mesh->m_dataFlags & MDF_INDEX_32 )
	{
		SGRX_CAST( uint32_t*, idcs, indices.data() );
		for( int i = 0; i < ic; ++i )
			indices[ i ] = idcs[ i ];
	}
	else
	{
		memcpy( indices.data(), mesh->m_idata.data(), ic * 2 );
	}
	
	return true;
}

struct LCVDataEdit
{
	LCVDataEdit( const Array<LCVertex>& iva, const Array<uint16_t>& iia,
		const SGRX_MeshPart& MP, const Array<SGRX_MeshPart>& parts )
		: triarea_total( 0 ), tex1off( 0 )
	{
		// reindex the input arrays to extract mesh part data
		{
			uint32_t iend = MP.indexOffset + MP.indexCount;
			for( uint32_t i = MP.indexOffset; i < iend; ++i )
			{
				inIA.push_back( inVA.find_or_add( iva[ iia[ i ] + MP.vertexOffset ] ) );
			}
			
			while( inIA.size() % 3 )
				inIA.pop_back();
		}
		
		// calculate triangle area from the whole mesh (for lightmap coord scaling stability)
		for( size_t p = 0; p < parts.size(); ++p )
		{
			const SGRX_MeshPart& mp = parts[ p ];
			uint32_t iend = mp.indexOffset + mp.indexCount;
			for( uint32_t i = mp.indexOffset; i + 2 < iend; i += 3 )
			{
				Vec3 p0 = iva[ iia[ i+0 ] + mp.vertexOffset ].pos;
				Vec3 p1 = iva[ iia[ i+1 ] + mp.vertexOffset ].pos;
				Vec3 p2 = iva[ iia[ i+2 ] + mp.vertexOffset ].pos;
				triarea_total += TriangleArea( p0, p1, p2 );
			}
		}
	}
	
	void _XFVerts( Array<LCVertex>& arr, size_t from, Mat4 xf )
	{
		for( size_t i = from; i < arr.size(); ++i )
		{
			LCVertex& V = arr[ i ];
			V.pos = xf.TransformPos( V.pos );
			V.nrm = xf.TransformNormal( V.nrm ).Normalized();
			V.tng = V4( xf.TransformNormal( V.tng.ToVec3() ).Normalized(), V.tng.w );
			V.tx1 += tex1off;
		}
	}
	static void _ClipTris( Array<LCVertex>& aout, const Array<LCVertex>& ain, Vec4 plane )
	{
		Vec3 PN = plane.ToVec3();
		float PD = plane.w;
		
		for( size_t i = 0; i + 2 < ain.size(); i += 3 )
		{
			const LCVertex& v0 = ain[ i+0 ];
			const LCVertex& v1 = ain[ i+1 ];
			const LCVertex& v2 = ain[ i+2 ];
			
			float sigdst0 = Vec3Dot( v0.pos, PN ) - PD;
			float sigdst1 = Vec3Dot( v1.pos, PN ) - PD;
			float sigdst2 = Vec3Dot( v2.pos, PN ) - PD;
			
			int in_front = 0, in_back = 0;
			if( sigdst0 >= 0 ) in_front++; if( sigdst0 <= 0 ) in_back++;
			if( sigdst1 >= 0 ) in_front++; if( sigdst1 <= 0 ) in_back++;
			if( sigdst2 >= 0 ) in_front++; if( sigdst2 <= 0 ) in_back++;
			
			if( in_front == 3 )
				continue; // remove the triangle
			if( in_back == 3 )
			{
				aout.push_back( v0 );
				aout.push_back( v1 );
				aout.push_back( v2 );
				continue; // keep the triangle
			}
			
			// clip the triangle
			LCVertex npts[6];
			int npc = 0;
			if( sigdst0 < 0 )
				npts[ npc++ ] = v0;
			if( sigdst0 * sigdst1 < 0 )
				npts[ npc++ ] = v0.Interpolate( v1, TREVLERP<float>( sigdst0, sigdst1, 0.0f ) );
			if( sigdst1 < 0 )
				npts[ npc++ ] = v1;
			if( sigdst1 * sigdst2 < 0 )
				npts[ npc++ ] = v1.Interpolate( v2, TREVLERP<float>( sigdst1, sigdst2, 0.0f ) );
			if( sigdst2 < 0 )
				npts[ npc++ ] = v2;
			if( sigdst2 * sigdst0 < 0 )
				npts[ npc++ ] = v2.Interpolate( v0, TREVLERP<float>( sigdst2, sigdst0, 0.0f ) );
			
			if( npc < 3 )
				continue;
			// triangulate
			for( int i = 2; i < npc; ++i )
			{
				aout.push_back( npts[0] );
				aout.push_back( npts[i-1] );
				aout.push_back( npts[i] );
			}
		}
	}
	
	void Add( Mat4 xf, Vec4* planes, Vec3* normals, int plcount )
	{
		size_t voff = outVA.size();
		size_t ioff = outIA.size();
		
		if( planes && plcount > 0 )
		{
			// unpack
			stage.resize( inIA.size() );
			for( size_t i = 0; i < inIA.size(); ++i )
				stage[ i ] = inVA[ inIA[ i ] ];
			// transform
			_XFVerts( stage, 0, xf );
			// clip
			for( int i = 0; i < plcount; ++i )
			{
				stage2.clear();
				_ClipTris( stage2, stage, planes[ i ] );
				stage = stage2;
			}
			// turn normals on planes
			for( int i = 0; i < plcount; ++i )
			{
				if( normals[ i ] == V3(0) )
					continue;
				Mat4 xf = Mat4::CreateRotationBetweenVectors( normals[ i ], planes[ i ].ToVec3() );
				for( size_t v = 0; v < stage.size(); ++v )
				{
					Vec3 PN = planes[ i ].ToVec3();
					float PD = planes[ i ].w;
					// having used this plane for clipping before, signed distance checking is acceptable
					if( Vec3Dot( stage[ v ].pos, PN ) - PD > -SMALL_FLOAT )
					{
						stage[ v ].nrm = xf.TransformNormal( stage[ v ].nrm ).Normalized();
						stage[ v ].tng = V4(
							xf.TransformNormal( stage[ v ].tng.ToVec3() ).Normalized(),
							stage[ v ].tng.w );
					}
				}
			}
			// repack into output
			for( size_t i = 0; i < stage.size(); ++i )
				outIA.push_back( outVA.find_or_add( stage[ i ], voff ) );
		}
		else
		{
			// copy
			outVA.append( inVA );
			outIA.append( inIA );
			// transform
			_XFVerts( outVA, voff, xf );
			// shift indices
			for( size_t i = ioff; i < outIA.size(); ++i )
				outIA[ i ] += voff;
		}
		
		tex1off++;
	}
	
	void Finalize( Vec2 txscale, float txangle, Vec2 txoff )
	{
		for( size_t i = 0; i < outVA.size(); ++i )
		{
			Vec2 tx = V2(
				safe_fdiv( outVA[ i ].tx0, txscale.x ),
				safe_fdiv( outVA[ i ].ty0, txscale.y ) );
			tx.Rotate( txangle );
			tx += txoff;
			outVA[ i ].tx0 = tx.x;
			outVA[ i ].ty0 = tx.y;
			
			outVA[ i ].tx1 /= tex1off; // not 0 if anything added (outVA not empty)
		}
	}
	
	float triarea_total;
	float tex1off;
	Array<LCVertex> inVA;
	Array<uint16_t> inIA;
	Array<LCVertex> outVA;
	Array<uint16_t> outIA;
	Array<LCVertex> stage;
	Array<LCVertex> stage2;
	Array<uint16_t> plverts;
};



void EdMeshPathPart::EditUI()
{
	g_NUISurfMtlPicker->Property( "Pick surface material", "Material", texname );
	
	Vec2 off = { xoff, yoff };
	IMGUIEditVec2( "Offset", off, 0, 1 );
	xoff = off.x; yoff = off.y;
	
	Vec2 scasp = { scale, aspect };
	IMGUIEditVec2( "Scale / aspect", scasp, 0.01f, 100 );
	scale = scasp.x; aspect = scasp.y;
	
	IMGUIEditFloat( "Angle", angle, 0, 360 );
}


EdObject* EdMeshPath::Clone()
{
	EdMeshPath* mpc = new EdMeshPath( *this );
	for( int i = 0; i < MAX_MESHPATH_PARTS; ++i )
	{
		mpc->m_parts[ i ].surface_guid = SGRX_GUID::Null;
	}
	return mpc;
}

bool EdMeshPath::RayIntersect( const Vec3& rpos, const Vec3& rdir, float outdst[1] ) const
{
	return RaySphereIntersect( rpos, rdir, m_position, 0.2f, outdst );
}

float EdMeshPath::FindPointOnPath( float at, size_t& pi0, size_t& pi1 )
{
	for( size_t i = 1; i < m_points.size(); ++i )
	{
		float len = ( m_points[ i ].pos - m_points[ i - 1 ].pos ).Length();
		if( at < len )
		{
			pi0 = i - 1;
			pi1 = i;
			return at / len;
		}
		at -= len;
	}
	pi0 = m_points.size() - 2;
	pi1 = m_points.size() - 1;
	return 1;
}

Vec4 EdMeshPath::GetPlane( size_t pt )
{
	ASSERT( pt >= 0 && pt < m_points.size() );
	Vec3 pos = m_points[ pt ].pos;
	Vec3 nrm = V3(0);
	if( pt > 0 )
		nrm += ( pos - m_points[ pt - 1 ].pos ).Normalized();
	if( pt + 1 < m_points.size() )
		nrm += ( m_points[ pt + 1 ].pos - pos ).Normalized();
	nrm.Normalize();
	return V4( nrm, Vec3Dot( nrm, pos + m_position ) );
}

size_t EdMeshPath::PlaceItem( LCVDataEdit& edit, float at, float off )
{
	at += m_intervalScaleOffset.y;
	size_t pi0, pi1;
	float q = FindPointOnPath( at, pi0, pi1 );
	Vec3 dtp = ( m_points[ pi1 ].pos - m_points[ pi0 ].pos ).Normalized();
	Vec3 pos = TLERP( m_points[ pi0 ].pos, m_points[ pi1 ].pos, q ) + m_position;
	Mat4 rotmtx = Mat4::Identity;
	switch( m_turnMode )
	{
	case MPATH_TurnMode_H:
		rotmtx = Mat4::CreateRotationZ( atan2( dtp.y, dtp.x ) );
		break;
	case MPATH_TurnMode_Fwd:
		{
			Vec3 bx = dtp;
			Vec3 bz = V3(0,0,1);
			float xzdot = Vec3Dot( bx, bz );
			if( xzdot > 0.999f )
				bz = V3(-1,0,1);
			else if( xzdot < -0.999f )
				bz = V3(1,0,1);
			Vec3 by = Vec3Cross( bz, bx ).Normalized();
			bz = Vec3Cross( bx, by ).Normalized();
			rotmtx = Mat4::Basis( bx, by, bz );
		}
		break;
	}
	Mat4 xf = Mat4::CreateRotationXYZ( DEG2RAD( m_rotAngles ) )
		* Mat4::CreateScale( m_scaleUni * m_scaleSep ) * rotmtx
		* Mat4::CreateTranslation( pos + dtp * off );
	// DEBUG
//	if(off) pos += V3(0,0,0.5f);
	
	if( m_pipeModeOvershoot )
	{
		Vec4 planes[2] = { -GetPlane( pi0 ), GetPlane( pi1 ) };
		Vec3 normals[2] =
		{
			m_doSmoothing || m_points[ pi0 ].smooth ? -dtp : V3( 0 ),
			m_doSmoothing || m_points[ pi0 ].smooth ? dtp : V3( 0 ),
		};
		edit.Add( xf, planes, normals, 2 );
	}
	else
	{
		edit.Add( xf, NULL, NULL, 0 );
	}
	
	return pi1;
}

void EdMeshPath::RegenerateMesh()
{
	m_cachedMesh = GR_GetMesh( m_meshName );
	if( !m_cachedMesh )
		return;
	
	Array< LCVertex > vertices;
	Array< uint16_t > indices;
	if( !ExtractLCVDataFromMesh( m_cachedMesh, vertices, indices ) )
		return;
	
	if( m_intervalScaleOffset.x < 0.1f )
		return;
	
	Vec3 bbmin = m_cachedMesh->m_boundsMin;
	Vec3 bbmax = m_cachedMesh->m_boundsMax;
	TransformAABB( bbmin, bbmax, Mat4::CreateRotationXYZ( DEG2RAD( m_rotAngles ) ) );
	Vec3 totalScale = m_scaleUni * m_scaleSep;
	float advance = bbmax.x - bbmin.x;
	float realAdv = advance * m_intervalScaleOffset.x * totalScale.x;
	float totalLength = 0;
	for( size_t i = 1; i < m_points.size(); ++i )
	{
		totalLength += ( m_points[ i ].pos - m_points[ i - 1 ].pos ).Length();
	}
	
	for( int pid = 0; pid < MAX_MESHPATH_PARTS && pid < (int) m_cachedMesh->m_meshParts.size(); ++pid )
	{
		if( m_parts[ pid ].texname.size() == 0 )
			continue;
		
		EdMeshPathPart& MP = m_parts[ pid ];
		EdLGCSurfaceInfo S;
		
		LCVDataEdit edit( vertices, indices, m_cachedMesh->m_meshParts[ pid ], m_cachedMesh->m_meshParts );
		
		if( m_pipeModeOvershoot )
		{
			float segmentEnd = 0;
			for( size_t i = 1; i < m_points.size(); ++i )
			{
				float segmentStart = segmentEnd;
				segmentEnd += ( m_points[ i ].pos - m_points[ i - 1 ].pos ).Length();
				float segmentMid = ( segmentStart + segmentEnd ) * 0.5f;
				
				float extra_adv = realAdv * m_pipeModeOvershoot;
				for( float at = -extra_adv; at < totalLength + extra_adv; at += realAdv )
				{
					float tMin = at + bbmin.x * totalScale.x - extra_adv;
					float tMax = at + bbmax.x * totalScale.x + extra_adv;
					
				//	printf( "p %d seg %g | segmin=%g segmax=%g tmin=%g tmax=%g\n",
				//		(int)i, at, segmentStart, segmentEnd, tMin, tMax );
					if( tMin > segmentEnd || tMax < segmentStart )
						continue; // instance doesn't intersect with segment
					
				//	printf( "BUILD segmid=%g off=%g\n", segmentMid, at - segmentMid );
					PlaceItem( edit, segmentMid, at - segmentMid );
				}
			}
		}
		else
		{
			float at = 0;
			while( at < totalLength )
			{
				PlaceItem( edit, at, 0 );
				at += realAdv;
			}
		}
		
		edit.Finalize(
			V2( MP.scale, safe_fdiv( MP.scale, MP.aspect ) ),
			DEG2RAD( MP.angle ), V2( MP.xoff, MP.yoff ) );
		
		S.vdata = edit.outVA.data();
		S.vcount = edit.outVA.size();
		S.idata = edit.outIA.data();
		S.icount = edit.outIA.size();
		S.mtlname = MP.texname;
		S.lmsize = V2( edit.tex1off, 1 ) * sqrtf( edit.triarea_total ) * 2;
		S.xform = g_EdWorld->GetGroupMatrix( group );
		S.rflags = 0
			| (m_isLMSolid ? LM_MESHINST_CASTLMS : 0)
			| (m_isPhySolid ? LM_MESHINST_SOLID : 0)
			| (m_isDynamic ? LM_MESHINST_DYNLIT : 0)
			| (m_skipCut ? LM_MESHINST_ED_SKIPCUT : 0);
		S.lmdetail = m_lmquality;
		S.decalLayer = 0;
		
		if( MP.surface_guid.NotNull() )
			g_EdLGCont->UpdateSurface( MP.surface_guid, LGC_CHANGE_ALL, &S );
		else
		{
			MP.surface_guid = SGRX_GUID::Generate();
			g_EdLGCont->RequestSurface( MP.surface_guid, &S );
		}
	}
}

Vec3 EdMeshPath::FindCenter() const
{
	Vec3 c = V3(0);
	for( size_t i = 0; i < m_points.size(); ++i )
	{
		c += m_points[ i ].pos;
	}
	if( m_points.size() != 0 )
		c /= m_points.size();
	return c + m_position;
}

int EdMeshPath::GetOnlySelectedVertex() const
{
	int sel = -1;
	for( size_t i = 0; i < m_points.size(); ++i )
	{
		if( m_points[ i ].sel )
		{
			if( sel == -1 )
				sel = i;
			else
				return -1;
		}
	}
	return sel;
}

void EdMeshPath::ScaleVertices( const Vec3& f )
{
	for( size_t i = 0; i < m_points.size(); ++i )
	{
		m_points[ i ].pos *= f;
	}
}

void EdMeshPath::TransformVertices( const Mat4& xf, bool selected )
{
	m_position += xf.GetTranslation();
	for( size_t i = 0; i < m_points.size(); ++i )
		if( !selected || m_points[ i ].sel )
			m_points[ i ].pos = xf.TransformNormal( m_points[ i ].pos );
}

void EdMeshPath::MoveSelectedVertices( const Vec3& t )
{
	for( size_t i = 0; i < m_points.size(); ++i )
	{
		if( m_points[ i ].sel )
		{
			m_points[ i ].pos += t;
		}
	}
}

void EdMeshPath::SpecialAction( ESpecialAction act )
{
	switch( act )
	{
	case SA_Subdivide:
		for( size_t i = 1; i < m_points.size(); ++i )
		{
			if( m_points[ i - 1 ].sel && m_points[ i ].sel )
			{
				EdMeshPathPoint mpp = m_points[ i ];
				mpp.pos = TLERP( m_points[ i - 1 ].pos, mpp.pos, 0.5f );
				m_points.insert( i++, mpp );
			}
		}
		RegenerateMesh();
		break;
	case SA_Unsubdivide:
		for( size_t i = 0; i < m_points.size(); ++i )
		{
			if( m_points[ i ].sel )
				m_points.erase( i-- );
		}
		RegenerateMesh();
		break;
	case SA_Extend:
		if( m_points[ 0 ].sel )
		{
			// extend from first
			EdMeshPathPoint mpp = m_points[ 0 ];
			m_points.insert( 0, mpp );
			m_points[ 1 ].sel = false;
		}
		else
		{
			// extend from last
			EdMeshPathPoint mpp = m_points.last();
			m_points.push_back( mpp );
			m_points[ m_points.size() - 2 ].sel = false;
		}
		break;
	default:
		break;
	}
}

bool EdMeshPath::CanDoSpecialAction( ESpecialAction act )
{
	switch( act )
	{
	case SA_Subdivide:
		for( size_t i = 1; i < m_points.size(); ++i )
		{
			if( m_points[ i - 1 ].sel && m_points[ i ].sel )
				return true;
		}
		return false;
	case SA_Unsubdivide:
		{
			int selnum = 0;
			for( size_t i = 0; i < m_points.size(); ++i )
				if( m_points[ i ].sel )
					selnum++;
			return m_points.size() - selnum >= 2;
		}
	case SA_Extend:
		{
			int v = GetOnlySelectedVertex();
			return v == 0 || v == (int) m_points.size() - 1;
		}
	default:
		return false;
	}
}


void EditMPPoint( int i, EdMeshPathPoint& p )
{
	char bfr[ 32 ];
	sgrx_snprintf( bfr, 32, "Point #%d", i );
	IMGUI_GROUP( bfr, true,
	{
		p.EditUI();
	});
}
	
void EdMeshPath::EditUI()
{
	ImGui::BeginChangeCheck();
	
	g_EdWorld->m_groupMgr.GroupProperty( "Group", group );
	g_NUIMeshPicker->Property( "Select mesh path template", "Mesh", m_meshName );
	IMGUIEditVec3( "Position", m_position, -8192, 8192 );
	IMGUIEditBool( "Casts lightmap shadows?", m_isLMSolid );
	IMGUIEditBool( "Is solid?", m_isPhySolid );
	IMGUIEditBool( "Skip cutting", m_skipCut );
	IMGUIEditBool( "Perform smoothing", m_doSmoothing );
	IMGUIEditBool( "Is dynamic?", m_isDynamic );
	IMGUIEditFloat( "Lightmap quality", m_lmquality, 0.01f, 100 );
	IMGUIEditFloat( "Interval scale", m_intervalScaleOffset.x, 0.01f, 100 );
	IMGUIEditFloat( "Interval offset", m_intervalScaleOffset.y, -128, 128 );
	IMGUIEditInt( "Pipe mode/overshoot", m_pipeModeOvershoot, 0, 32 );
	IMGUIEditVec3( "Rotation (XYZ)", m_rotAngles, 0, 360 );
	IMGUIEditFloat( "Scale (uniform)", m_scaleUni, 0.01f, 100 );
	IMGUIEditVec3( "Scale (separate)", m_scaleSep, 0.01f, 100 );
	IMGUIComboBox( "Turn mode", m_turnMode, "Horizontal\0Forward\0Horizontal/forward skew\0" );
	
	IMGUI_GROUP( "Points", false,
	{
		IMGUIEditArray( m_points, EditMPPoint, "Add point" );
	});
	for( int i = 0; i < MAX_MESHPATH_PARTS; ++i )
	{
		char bfr[ 32 ];
		sgrx_snprintf( bfr, 32, "Part #%d", i );
		IMGUI_GROUP( bfr, true,
		{
			m_parts[ i ].EditUI();
		});
	}
	
	if( ImGui::EndChangeCheck() )
		RegenerateMesh();
}

void EdMeshPath::VertEditUI( int vid )
{
	m_points[ vid ].EditUI();
}


