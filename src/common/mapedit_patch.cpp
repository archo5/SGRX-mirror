

#include "mapedit.hpp"



void EdPatchLayerInfo::EditUI( EdPatch* P, int lid )
{
	char bfr[ 32 ];
	sgrx_snprintf( bfr, 32, "Layer #%d", lid );
	IMGUI_GROUP_BEGIN( bfr, true )
	{
		g_NUISurfMtlPicker->Property( "Pick surface material", "Material", texname );
		
		ImGui::BeginChangeMask( 0 );
		
		Vec2 off = { xoff, yoff };
		IMGUIEditVec2( "Offset", off, 0, 1 );
		xoff = off.x; yoff = off.y;
		
		Vec2 scasp = { scale, aspect };
		IMGUIEditVec2( "Scale / aspect", scasp, 0.01f, 100 );
		scale = scasp.x; aspect = scasp.y;
		
		IMGUIEditFloat( "Angle", angle, 0, 360 );
		
		ImGui::EndChangeMask();
		
		bool texgen = false;
		ImGui::Text( "Generate:" );
		ImGui::SameLine();
		if( ImGui::Button( "Fit" ) ){ texgen = true; P->TexGenFit( lid ); }
		ImGui::SameLine();
		if( ImGui::Button( "Fit/Ntrl." ) ){ texgen = true; P->TexGenFitNat( lid ); }
		ImGui::SameLine();
		if( ImGui::Button( "Natural" ) ){ texgen = true; P->TexGenNatural( lid ); }
		ImGui::SameLine();
		if( ImGui::Button( "Planar" ) ){ texgen = true; P->TexGenPlanar( lid ); }
		
		if( texgen )
		{
			P->TexGenPostProc( lid );
			ImGui::TriggerChangeCheck();
		}
	}
	IMGUI_GROUP_END;
}



static const uint16_t ones_mask[ 17 ] = { 0,
	0x0001, 0x0003, 0x0007, 0x000f,
	0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff,
	0x1fff, 0x3fff, 0x7fff, 0xffff,
};


// at must be 0 to 14
static uint16_t insert_zero_bit( uint16_t v, int at )
{
	uint16_t msk = ones_mask[ at ];
	return ( v & msk ) | ( ( v & ~msk ) << 1 );
}
static uint16_t remove_bit( uint16_t v, int at )
{
	return ( v & ones_mask[ at ] ) | ( ( v & ~ones_mask[ at + 1 ] ) >> 1 );
}


bool EdPatch::InsertXLine( int at )
{
	if( at < 0 || at > (int) xsize || xsize >= MAX_PATCH_WIDTH )
		return false;
	
	int befoff = at == 0 ? 1 : -1;
	int aftoff = at == xsize ? -1 : 1;
	for( int i = 0; i < (int) ysize; ++i )
	{
		EdPatchVtx* vp = vertices + MAX_PATCH_WIDTH * i + at;
		if( at < xsize )
			memmove( vp + 1, vp, sizeof(*vp) * ( xsize - at ) );
		
		_InterpolateVertex( vp, vp + befoff, vp + aftoff, 0.5f );
		
		edgeflip[ i ] = insert_zero_bit( edgeflip[ i ], at );
		vertsel[ i ] = insert_zero_bit( vertsel[ i ], at );
	}
	
	xsize++;
	return true;
}

bool EdPatch::InsertYLine( int at )
{
	if( at < 0 || at > (int) ysize || ysize >= MAX_PATCH_WIDTH )
		return false;
	
	if( at < ysize )
	{
		memmove( vertices + MAX_PATCH_WIDTH * ( at + 1 ),
			vertices + MAX_PATCH_WIDTH * at,
			sizeof(*vertices) * MAX_PATCH_WIDTH * ( ysize - at ) );
		memmove( edgeflip + at + 1, edgeflip + at, sizeof(*edgeflip) * ( ysize - at ) );
		memmove( vertsel + at + 1, vertsel + at, sizeof(*vertsel) * ( ysize - at ) );
	}
	edgeflip[ at ] = 0;
	vertsel[ at ] = 0;
	
	int befoff = at == 0 ? MAX_PATCH_WIDTH : -MAX_PATCH_WIDTH;
	int aftoff = at == ysize ? -MAX_PATCH_WIDTH : MAX_PATCH_WIDTH;
	for( int i = 0; i < (int) xsize; ++i )
	{
		EdPatchVtx* vp = vertices + MAX_PATCH_WIDTH * at + i;
		_InterpolateVertex( vp, vp + befoff, vp + aftoff, 0.5f );
	}
	
	ysize++;
	return true;
}

bool EdPatch::RemoveXLine( int at )
{
	if( at < 0 || at >= (int) xsize || xsize <= 2 )
		return false;
	
	if( at < xsize - 1 )
	{
		for( int i = 0; i < (int) ysize; ++i )
		{
			EdPatchVtx* vp = vertices + MAX_PATCH_WIDTH * i + at;
			memmove( vp, vp + 1, sizeof(*vertices) * ( xsize - at - 1 ) );
			edgeflip[ i ] = remove_bit( edgeflip[ i ], at );
			vertsel[ i ] = remove_bit( vertsel[ i ], at );
		}
	}
	
	xsize--;
	return true;
}

bool EdPatch::RemoveYLine( int at )
{
	if( at < 0 || at >= (int) ysize || ysize <= 2 )
		return false;
	
	if( at < ysize - 1 )
	{
		memmove( vertices + MAX_PATCH_WIDTH * at,
			vertices + MAX_PATCH_WIDTH * ( at + 1 ),
			sizeof(*vertices) * MAX_PATCH_WIDTH * ( MAX_PATCH_WIDTH - at - 1 ) );
		memmove( edgeflip + at, edgeflip + at + 1, sizeof(*edgeflip) * ( MAX_PATCH_WIDTH - at - 1 ) );
		memmove( vertsel + at, vertsel + at + 1, sizeof(*vertsel) * ( MAX_PATCH_WIDTH - at - 1 ) );
	}
	
	ysize--;
	return true;
}

void EdPatch::_InterpolateVertex( EdPatchVtx* out, EdPatchVtx* v0, EdPatchVtx* v1, float s )
{
	out->pos = TLERP( v0->pos, v1->pos, s );
	for( int i = 0; i < MAX_PATCH_LAYERS; ++i )
	{
		out->tex[ i ] = TLERP( v0->tex[ i ], v1->tex[ i ], s );
		out->col[ i ] = Vec4ToCol32( *(BVec4*)&v0->col[ i ] * (1-s) + *(BVec4*)&v1->col[ i ] * s );
	}
}

EdObject* EdPatch::Clone()
{
	EdPatch* ptc = new EdPatch( *this );
	for( int i = 0; i < MAX_PATCH_LAYERS; ++i )
	{
		ptc->layers[ i ].surface_guid = SGRX_GUID::Null;
	}
	return ptc;
}

bool EdPatch::RayIntersect( const Vec3& rpos, const Vec3& dir, float outdst[1] ) const
{
	float mindst = FLT_MAX;
	for( int y = 0; y < ysize - 1; ++y )
	{
		for( int x = 0; x < xsize - 1; ++x )
		{
			float ndst[1];
			Vec3 pts[6];
			if( edgeflip[ y ] & ( 1 << x ) )
			{
				pts[0] = pts[5] = vertices[ x + ( y + 1 ) * MAX_PATCH_WIDTH ].pos + position;
				pts[4] = vertices[ x + y * MAX_PATCH_WIDTH ].pos + position;
				pts[2] = pts[3] = vertices[ x + 1 + y * MAX_PATCH_WIDTH ].pos + position;
				pts[1] = vertices[ x + 1 + ( y + 1 ) * MAX_PATCH_WIDTH ].pos + position;
			}
			else
			{
				pts[0] = pts[5] = vertices[ x + y * MAX_PATCH_WIDTH ].pos + position;
				pts[4] = vertices[ x + 1 + y * MAX_PATCH_WIDTH ].pos + position;
				pts[2] = pts[3] = vertices[ x + 1 + ( y + 1 ) * MAX_PATCH_WIDTH ].pos + position;
				pts[1] = vertices[ x + ( y + 1 ) * MAX_PATCH_WIDTH ].pos + position;
			}
			if( RayPolyIntersect( rpos, dir, pts, 3, ndst ) && ndst[0] < mindst )
				mindst = ndst[0];
			if( RayPolyIntersect( rpos, dir, pts+3, 3, ndst ) && ndst[0] < mindst )
				mindst = ndst[0];
		}
	}
	if( outdst ) outdst[0] = mindst;
	return mindst < FLT_MAX;
}

void EdPatch::RegenerateMesh()
{
	if( !g_EdWorld )
		return;
	
	VertexDeclHandle vd = GR_GetVertexDecl( LCVertex_DECL );
	Array< LCVertex > outverts;
	Array< uint16_t > outidcs;
	outverts.reserve( xsize * ysize );
	outidcs.reserve( ( xsize - 1 ) * ( ysize - 1 ) * 6 );
	
	bool first = true;
	for( int layer = 0; layer < MAX_PATCH_LAYERS; ++layer )
	{
		EdPatchLayerInfo& LI = layers[ layer ];
		EdLGCSurfaceInfo S;
		
		outverts.clear();
		outidcs.clear();
		Vec2 lmsize = GenerateMeshData( outverts, outidcs, layer );
		
		bool ovr = first == false || renderMode == PRM_Decal;
		S.vdata = outverts.data();
		S.vcount = outverts.size();
		S.idata = outidcs.data();
		S.icount = outidcs.size();
		S.mtlname = LI.texname;
		S.lmsize = lmsize;
		S.xform = g_EdWorld->GetGroupMatrix( group );
		S.rflags = LM_MESHINST_VCOL
			| (m_isLMSolid ? LM_MESHINST_CASTLMS : 0)
			| (ovr ? (LM_MESHINST_DECAL|LM_MESHINST_TRANSPARENT) : 0)
			| (renderMode == PRM_Transparent ? LM_MESHINST_TRANSPARENT : 0)
			| (m_isPhySolid && first ? LM_MESHINST_SOLID : 0);
		S.lmdetail = lmquality;
		S.decalLayer = layer + blend;
		
		if( LI.surface_guid.NotNull() )
			g_EdLGCont->UpdateSurface( LI.surface_guid, LGC_CHANGE_ALL, &S );
		else
		{
			LI.surface_guid = SGRX_GUID::Generate();
			g_EdLGCont->RequestSurface( LI.surface_guid, &S );
		}
		
		if( LI.texname.size() )
			first = false;
	}
}

Vec3 EdPatch::FindCenter() const
{
	Vec3 c = V3(0);
	for( int y = 0; y < ysize; ++y )
	{
		for( int x = 0; x < xsize; ++x )
		{
			c += vertices[ x + y * MAX_PATCH_WIDTH ].pos;
		}
	}
	return c / ( xsize * ysize ) + position;
}

Vec2 EdPatch::GenerateMeshData( Array< LCVertex >& outverts, Array< uint16_t >& outidcs, int layer )
{
	// generate lightmap coords
	Vec2 lmverts[ MAX_PATCH_WIDTH * MAX_PATCH_WIDTH ];
	
	Vec2 total_xy_length = V2(0);
	// map 1x1
	// - X
	for( int y = 0; y < ysize; ++y )
	{
		float total_length = 0;
		for( int x = 1; x < xsize; ++x )
		{
			Vec3 diff = vertices[ y * MAX_PATCH_WIDTH + x ].pos - vertices[ y * MAX_PATCH_WIDTH + x - 1 ].pos;
			total_length += diff.Length();
		}
		lmverts[ y * MAX_PATCH_WIDTH ].x = 0;
		float curr_length = 0;
		for( int x = 1; x < xsize; ++x )
		{
			Vec3 diff = vertices[ y * MAX_PATCH_WIDTH + x ].pos - vertices[ y * MAX_PATCH_WIDTH + x - 1 ].pos;
			curr_length += diff.Length();
			lmverts[ y * MAX_PATCH_WIDTH + x ].x = safe_fdiv( curr_length, total_length );
		}
		total_xy_length.x += total_length;
	}
	// - Y
	for( int x = 0; x < xsize; ++x )
	{
		float total_length = 0;
		for( int y = 1; y < ysize; ++y )
		{
			Vec3 diff = vertices[ y * MAX_PATCH_WIDTH + x ].pos - vertices[ ( y - 1 ) * MAX_PATCH_WIDTH + x ].pos;
			total_length += diff.Length();
		}
		lmverts[ x ].y = 0;
		float curr_length = 0;
		for( int y = 1; y < ysize; ++y )
		{
			Vec3 diff = vertices[ y * MAX_PATCH_WIDTH + x ].pos - vertices[ ( y - 1 ) * MAX_PATCH_WIDTH + x ].pos;
			curr_length += diff.Length();
			lmverts[ y * MAX_PATCH_WIDTH + x ].y = safe_fdiv( curr_length, total_length );
		}
		total_xy_length.y += total_length;
	}
	
	// divided by number of samples
	total_xy_length /= V2( ysize, xsize );
	
	// generate vertices
	for( int y = 0; y < (int) ysize; ++y )
	{
		for( int x = 0; x < (int) xsize; ++x )
		{
			Vec3 xnrm, ynrm;
			if( x == 0 )
				xnrm = vertices[ 1 + y * MAX_PATCH_WIDTH ].pos - vertices[ y * MAX_PATCH_WIDTH ].pos;
			else if( x == xsize - 1 )
				xnrm = vertices[ x + y * MAX_PATCH_WIDTH ].pos - vertices[ x - 1 + y * MAX_PATCH_WIDTH ].pos;
			else
				xnrm = vertices[ x + 1 + y * MAX_PATCH_WIDTH ].pos - vertices[ x - 1 + y * MAX_PATCH_WIDTH ].pos;
			
			if( y == 0 )
				ynrm = vertices[ x + 1 * MAX_PATCH_WIDTH ].pos - vertices[ x + y * MAX_PATCH_WIDTH ].pos;
			else if( y == ysize - 1 )
				ynrm = vertices[ x + y * MAX_PATCH_WIDTH ].pos - vertices[ x + (y - 1) * MAX_PATCH_WIDTH ].pos;
			else
				ynrm = vertices[ x + (y + 1) * MAX_PATCH_WIDTH ].pos - vertices[ x + (y - 1) * MAX_PATCH_WIDTH ].pos;
			
			Vec3 nrm = -Vec3Cross( xnrm, ynrm ).Normalized();
			
			EdPatchVtx& V = vertices[ x + y * MAX_PATCH_WIDTH ];
			Vec2 LMV = lmverts[ x + y * MAX_PATCH_WIDTH ];
			Vec2 tx = V.tex[ layer ];
			Vec4 tng = V4( xnrm.Normalized(), -1 );
			LCVertex vert = { V.pos + position, nrm, tng, V.col[ layer ], tx.x, tx.y, LMV.x, LMV.y };
			outverts.push_back( vert );
		}
	}
	for( int y = 0; y < (int) ysize - 1; ++y )
	{
		for( int x = 0; x < (int) xsize - 1; ++x )
		{
			int bv = y * xsize + x;
			if( edgeflip[ y ] & ( 1 << x ) )
			{
				// split: [/]
				uint16_t idcs[6] =
				{
					bv + xsize, bv, bv + 1,
					bv + 1, bv + 1 + xsize, bv + xsize
				};
				outidcs.append( idcs, 6 );
			}
			else
			{
				// split: [\]
				uint16_t idcs[6] =
				{
					bv, bv + 1, bv + 1 + xsize,
					bv + 1 + xsize, bv + xsize, bv
				};
				outidcs.append( idcs, 6 );
			}
		}
	}
	
	return total_xy_length;
}

Vec3 EdPatch::GetElementPoint( int i ) const
{
	int vcount = xsize * ysize;
	int fcount = GetNumQuads();
	int numxe = GetNumXEdges();
	int numye = GetNumYEdges();
	if( i < vcount )
		return GetLocalVertex( i );
	i -= vcount;
	if( i < fcount )
		return GetQuadCenter( i );
	i -= fcount;
	if( i < numxe )
		return GetXEdgeCenter( i );
	i -= numxe;
	if( i < numye )
		return GetYEdgeCenter( i );
	return V3(0);
}

Vec3 EdPatch::GetQuadCenter( int i ) const
{
	int x = i % ( xsize - 1 );
	int y = i / ( xsize - 1 );
	return (
		vertices[ x + y * MAX_PATCH_WIDTH ].pos +
		vertices[ x + 1 + y * MAX_PATCH_WIDTH ].pos +
		vertices[ x + 1 + ( y + 1 ) * MAX_PATCH_WIDTH ].pos +
		vertices[ x + ( y + 1 ) * MAX_PATCH_WIDTH ].pos
	) * 0.25f + position;
}

Vec3 EdPatch::GetXEdgeCenter( int i ) const
{
	int x = i % ( xsize - 1 );
	int y = i / ( xsize - 1 );
	return ( vertices[ x + y * MAX_PATCH_WIDTH ].pos + vertices[ x + 1 + y * MAX_PATCH_WIDTH ].pos ) * 0.5f + position;
}

Vec3 EdPatch::GetYEdgeCenter( int i ) const
{
	int x = i % xsize;
	int y = i / xsize;
	return ( vertices[ x + y * MAX_PATCH_WIDTH ].pos + vertices[ x + ( y + 1 ) * MAX_PATCH_WIDTH ].pos ) * 0.5f + position;
}

void EdPatch::SelectElement( int i, bool sel )
{
	int vcount = xsize * ysize;
	int fcount = GetNumQuads();
	int numxe = GetNumXEdges();
	int numye = GetNumYEdges();
	if( i < vcount )
	{
		if( sel )
			vertsel[ i / xsize ] |= ( 1 << ( i % xsize ) );
		else
			vertsel[ i / xsize ] &= ~( 1 << ( i % xsize ) );
		return;
	}
	i -= vcount;
	if( i < fcount )
	{
		int x = i % ( xsize - 1 );
		int y = i / ( xsize - 1 );
		if( sel )
		{
			vertsel[ y ] |= ( 1 << x );
			vertsel[ y ] |= ( 1 << ( x + 1 ) );
			vertsel[ y + 1 ] |= ( 1 << x );
			vertsel[ y + 1 ] |= ( 1 << ( x + 1 ) );
		}
		else
		{
			vertsel[ y ] &= ~( 1 << x );
			vertsel[ y ] &= ~( 1 << ( x + 1 ) );
			vertsel[ y + 1 ] &= ~( 1 << x );
			vertsel[ y + 1 ] &= ~( 1 << ( x + 1 ) );
		}
		return;
	}
	i -= fcount;
	if( i < numxe )
	{
		i /= xsize - 1;
		for( int x = 0; x < xsize; ++x )
		{
			vertsel[ i ] = sel ? 0xffff : 0;
		}
		return;
	}
	i -= numxe;
	if( i < numye )
	{
		i %= xsize;
		for( int y = 0; y < ysize; ++y )
		{
			if( sel )
				vertsel[ y ] |= 1 << i;
			else
				vertsel[ y ] &= ~( 1 << i );
		}
		return;
	}
}

void EdPatch::ScaleVertices( const Vec3& f )
{
	for( int y = 0; y < ysize; ++y )
	{
		for( int x = 0; x < xsize; ++x )
		{
			vertices[ x + y * MAX_PATCH_WIDTH ].pos *= f;
		}
	}
}

void EdPatch::TransformVertices( const Mat4& xf, bool selected )
{
	Vec3 oldpos = position;
	position += xf.GetTranslation();
	for( int y = 0; y < ysize; ++y )
	{
		for( int x = 0; x < xsize; ++x )
		{
			if( !selected || ( vertsel[ y ] & ( 1 << x ) ) )
				vertices[ x + y * MAX_PATCH_WIDTH ].pos = xf.TransformPos( vertices[ x + y * MAX_PATCH_WIDTH ].pos + oldpos ) - position;
			else
				vertices[ x + y * MAX_PATCH_WIDTH ].pos += oldpos - position;
		}
	}
}

int EdPatch::GetOnlySelectedVertex() const
{
	int sel = -1;
	for( int y = 0; y < ysize; ++y )
	{
		for( int x = 0; x < xsize; ++x )
		{
			if( vertsel[ y ] & ( 1 << x ) )
			{
				if( sel == -1 )
					sel = x + y * xsize;
				else
					return -1;
			}
		}
	}
	return sel;
}

void EdPatch::MoveSelectedVertices( const Vec3& t )
{
	for( int y = 0; y < ysize; ++y )
	{
		for( int x = 0; x < xsize; ++x )
		{
			if( vertsel[ y ] & ( 1 << x ) )
				vertices[ x + y * MAX_PATCH_WIDTH ].pos += t;
		}
	}
}

void EdPatch::GetPaintVertex( int v, int layer, Vec3& outpos, Vec4& outcol )
{
	ASSERT( v >= 0 && v < xsize * ysize );
	ASSERT( layer >= 0 && layer < MAX_PATCH_LAYERS );
	EdPatchVtx& pv = vertices[ ( v % xsize ) + v / xsize * MAX_PATCH_WIDTH ];
	outpos = pv.pos + position;
	outcol = Col32ToVec4( pv.col[ layer ] );
}

void EdPatch::SetPaintVertex( int v, int layer, const Vec3& pos, Vec4 col )
{
	ASSERT( v >= 0 && v < xsize * ysize );
	ASSERT( layer >= 0 && layer < MAX_PATCH_LAYERS );
	EdPatchVtx& pv = vertices[ ( v % xsize ) + v / xsize * MAX_PATCH_WIDTH ];
	pv.pos = pos - position;
	pv.col[ layer ] = Vec4ToCol32( col );
}

#define PATCH_SELECT_VERTS( cond ) \
	do{ for( int y = 0; y < ysize; ++y ) \
		for( int x = 0; x < xsize; ++x ) \
			SelectElement( x + y * xsize, cond ); \
	}while(0)

void EdPatch::SpecialAction( ESpecialAction act )
{
	switch( act )
	{
	case SA_Invert:
		for( int y = 0; y < ysize; ++y )
		{
			for( int x = 0; x < xsize / 2; ++x )
			{
				TSWAP( vertices[ y * MAX_PATCH_WIDTH + x ], vertices[ y * MAX_PATCH_WIDTH + xsize - 1 - x ] );
			}
		}
		break;
	case SA_Subdivide:
		for( int y = 0; y < ysize; ++y )
		{
			for( int x = 0; x < xsize - 1; ++x )
			{
				if( IsXEdgeSel( x, y ) )
					InsertXLine( x + 1 );
			}
		}
		for( int y = 0; y < ysize - 1; ++y )
		{
			for( int x = 0; x < xsize; ++x )
			{
				if( IsYEdgeSel( x, y ) )
					InsertYLine( y + 1 );
			}
		}
		break;
	case SA_Unsubdivide:
		for( int y = 0; y < ysize - 1; ++y )
		{
			for( int x = 0; x < xsize - 1; ++x )
			{
				bool remx = x > 0 && IsYEdgeSel( x, y );
				bool remy = y > 0 && IsXEdgeSel( x, y );
				if( remx ) RemoveXLine( x );
				if( remy ) RemoveYLine( y );
			}
		}
		break;
	case SA_EdgeFlip:
		for( int y = 0; y < ysize - 1; ++y )
		{
			for( int x = 0; x < xsize - 1; ++x )
			{
				if( IsQuadSel( x, y ) )
					edgeflip[ y ] ^= ( 1 << x );
			}
		}
		break;
	case SA_Extend:
		{
			int acount = 0, bcount = 0, ccount = 0, dcount = 0;
			for( int y = 0; y < ysize; ++y )
			{
				for( int x = 0; x < xsize; ++x )
				{
					bool vsel = IsVertSel( x, y );
					if( vsel == ( y == 0 ) ) acount++;
					if( vsel == ( y == ysize - 1 ) ) bcount++;
					if( vsel == ( x == 0 ) ) ccount++;
					if( vsel == ( x == xsize - 1 ) ) dcount++;
				}
			}
			int tc = xsize * ysize;
			if( xsize == MAX_PATCH_WIDTH ){ ccount = 0; dcount = 0; }
			if( ysize == MAX_PATCH_WIDTH ){ acount = 0; bcount = 0; }
			if( acount == tc )
			{
				InsertYLine( 0 );
				PATCH_SELECT_VERTS( y == 0 );
			}
			else if( bcount == tc )
			{
				InsertYLine( ysize );
				PATCH_SELECT_VERTS( y == ysize - 1 );
			}
			else if( ccount == tc )
			{
				InsertXLine( 0 );
				PATCH_SELECT_VERTS( x == 0 );
			}
			else if( dcount == tc )
			{
				InsertXLine( xsize );
				PATCH_SELECT_VERTS( x == xsize - 1 );
			}
		}
		break;
		
	case SA_Remove:
		while( IsXLineSel( 0 ) ) RemoveXLine( 0 );
		while( IsXLineSel( xsize - 1 ) ) RemoveXLine( xsize - 1 );
		while( IsYLineSel( 0 ) ) RemoveYLine( 0 );
		while( IsYLineSel( ysize - 1 ) ) RemoveYLine( ysize - 1 );
		break;
		
	case SA_ExtractPart:
		if( !CanDoSpecialAction( act ) ) return;
		break;
		
	case SA_DuplicatePart:
		if( !CanDoSpecialAction( act ) ) return;
		break;
		
#define SETBYTE( bref, x, v ) \
	if( v ){ (bref) |= 1 << (x); }else{ (bref) &= ~(1 << (x)); }
#define PATCH_BYTESWAP( bto, x1, y1, bfrom, x0, y0 ) { \
	bool v = ( bfrom[ y0 ] & ( 1 << (x0) ) ) != 0; \
	SETBYTE( bto[ y1 ], x1, v ); }
	
	case SA_RotateCCW:
	case SA_RotateCW:
		{
			EdPatchVtx nverts[ MAX_PATCH_WIDTH * MAX_PATCH_WIDTH ];
			uint16_t nedgeflip[ MAX_PATCH_WIDTH ];
			uint16_t nvertsel[ MAX_PATCH_WIDTH ];
			memset( nverts, 0, sizeof(nverts) );
			memset( nedgeflip, 0, sizeof(nedgeflip) );
			memset( nvertsel, 0, sizeof(nvertsel) );
			for( int y = 0; y < ysize; ++y )
			{
				for( int x = 0; x < xsize; ++x )
				{
					if( act == SA_RotateCCW )
					{
						nverts[ y + ( xsize - 1 - x ) * MAX_PATCH_WIDTH ] =
							vertices[ x + y * MAX_PATCH_WIDTH ];
						PATCH_BYTESWAP( nedgeflip, y, xsize - 1 - x, edgeflip, x, y );
						PATCH_BYTESWAP( nvertsel, y, xsize - 1 - x, vertsel, x, y );
					}
					else
					{
						nverts[ ( ysize - 1 - y ) + x * MAX_PATCH_WIDTH ] =
							vertices[ x + y * MAX_PATCH_WIDTH ];
						PATCH_BYTESWAP( nedgeflip, ysize - 1 - y, x, edgeflip, x, y );
						PATCH_BYTESWAP( nvertsel, ysize - 1 - y, x, vertsel, x, y );
					}
				}
			}
			memcpy( vertices, nverts, sizeof(nverts) );
			memcpy( edgeflip, nedgeflip, sizeof(nedgeflip) );
			memcpy( vertsel, nvertsel, sizeof(nvertsel) );
			TSWAP( xsize, ysize );
		}
		break;
		
	default:
		break;
	}
	RegenerateMesh();
}

bool EdPatch::CanDoSpecialAction( ESpecialAction act )
{
	switch( act )
	{
	case SA_EdgeFlip:
		for( int y = 0; y < ysize - 1; ++y )
		{
			for( int x = 0; x < xsize - 1; ++x )
			{
				if( IsQuadSel( x, y ) )
					return true;
			}
		}
		return false;
		
	case SA_Subdivide:
		for( int y = 0; y < ysize; ++y )
		{
			for( int x = 0; x < xsize - 1; ++x )
			{
				if( IsXEdgeSel( x, y ) ) return true;
			}
		}
		for( int y = 0; y < ysize - 1; ++y )
		{
			for( int x = 0; x < xsize; ++x )
			{
				if( IsYEdgeSel( x, y ) ) return true;
			}
		}
			
		return false;
		
	case SA_Unsubdivide:
		for( int y = 0; y < ysize - 1; ++y )
		{
			for( int x = 0; x < xsize - 1; ++x )
			{
				if( x > 0 && IsYEdgeSel( x, y ) ) return true;
				if( y > 0 && IsXEdgeSel( x, y ) ) return true;
			}
		}
		return false;
		
	case SA_Extend:
		{
			int acount = 0, bcount = 0, ccount = 0, dcount = 0;
			for( int y = 0; y < ysize; ++y )
			{
				for( int x = 0; x < xsize; ++x )
				{
					bool vsel = IsVertSel( x, y );
					if( vsel == ( y == 0 ) ) acount++;
					if( vsel == ( y == ysize - 1 ) ) bcount++;
					if( vsel == ( x == 0 ) ) ccount++;
					if( vsel == ( x == xsize - 1 ) ) dcount++;
				}
			}
			int tc = xsize * ysize;
			if( xsize == MAX_PATCH_WIDTH ){ ccount = 0; dcount = 0; }
			if( ysize == MAX_PATCH_WIDTH ){ acount = 0; bcount = 0; }
			return acount == tc || bcount == tc || ccount == tc || dcount == tc;
		}
		return false;
		
	case SA_Remove:
		return IsAnySideSel( false );
		
	case SA_ExtractPart:
		return IsAnySideSel( true );
		
	case SA_DuplicatePart:
		{
			int dims[2] = {0, 0};
			return IsAnyRectSel( false, dims ) && dims[0] >= 2 && dims[1] >= 2;
		}
		
	case SA_RotateCCW:
	case SA_RotateCW:
		return true;
		
	default:
		return false;
	}
}

bool EdPatch::IsAnyRectSel( bool invert, int outdims[2] ) const
{
	int xdims = -1, ydims = -1;
	for( int y = 0; y < ysize; ++y )
	{
		int numins = 0;
		bool prev = false;
		int start = -1;
		for( int x = 0; x < xsize; ++x )
		{
			bool cur = IsVertSel( x, y ) ^ invert;
			if( cur && prev == false )
			{
				numins++;
				start = x;
			}
			else if( cur == false && prev )
			{
				xdims = x - start;
				start = -1;
			}
			prev = cur;
		}
		if( numins > 1 )
			return false;
		if( start != -1 )
		{
			if( xdims != -1 && xdims != xsize - start )
				return false;
			xdims = xsize - start;
		}
	}
	if( xdims == -1 )
		return false;
	for( int x = 0; x < xsize; ++x )
	{
		int numins = 0;
		bool prev = false;
		int start = -1;
		for( int y = 0; y < ysize; ++y )
		{
			bool cur = IsVertSel( x, y ) ^ invert;
			if( cur && prev == false )
			{
				numins++;
				start = y;
			}
			else if( cur == false && prev )
			{
				ydims = y - start;
				start = -1;
			}
			prev = cur;
		}
		if( numins > 1 )
			return false;
		if( start != -1 )
		{
			if( ydims != -1 && ydims != ysize - start )
				return false;
			ydims = ysize - start;
		}
	}
	if( ydims == -1 )
		return false;
	if( outdims )
	{
		outdims[0] = xdims;
		outdims[1] = ydims;
	}
	return true;
}

bool EdPatch::IsAnySideSel( bool bothpatches ) const
{
	int dims[2];
	int space = bothpatches ? 1 : 2;
	if( !IsAnyRectSel( true, dims ) || dims[0] < space || dims[1] < space )
		return false;
	
	if( !IsAnyRectSel( false, dims ) )
		return false;
	
	if( bothpatches && ( dims[0] < 2 || dims[1] < 2 ) )
		return false;
	
	return true;
}

uint16_t EdPatch::GetXSelMask( int i ) const
{
	ASSERT( i >= 0 && i < xsize );
	uint16_t out = 0;
	for( int y = 0; y < ysize; ++y )
		out |= ( ( vertsel[ y ] >> i ) & 1 ) << y;
	return out;
}

uint16_t EdPatch::GetYSelMask( int i ) const
{
	ASSERT( i >= 0 && i < ysize );
	return vertsel[ i ] & ones_mask[ xsize ];
}

bool EdPatch::IsXLineSel( int i ) const
{
	return GetXSelMask( i ) == ones_mask[ ysize ];
}

bool EdPatch::IsYLineSel( int i ) const
{
	return GetYSelMask( i ) == ones_mask[ xsize ];
}

bool EdPatch::IsAllSel() const
{
	for( int i = 0; i < ysize; ++i )
		if( ( vertsel[ i ] & ones_mask[ xsize ] ) != ones_mask[ xsize ] )
			return false;
	return true;
}


Vec2 EdPatch::TexGenFit( int layer )
{
	Vec2 total_xy_length = V2(0);
	
	// map 1x1
	// - X
	for( int y = 0; y < ysize; ++y )
	{
		float total_length = 0;
		for( int x = 1; x < xsize; ++x )
		{
			Vec3 diff = vertices[ y * MAX_PATCH_WIDTH + x ].pos - vertices[ y * MAX_PATCH_WIDTH + x - 1 ].pos;
			total_length += diff.Length();
		}
		vertices[ y * MAX_PATCH_WIDTH ].tex[ layer ].x = 0;
		float curr_length = 0;
		for( int x = 1; x < xsize; ++x )
		{
			Vec3 diff = vertices[ y * MAX_PATCH_WIDTH + x ].pos - vertices[ y * MAX_PATCH_WIDTH + x - 1 ].pos;
			curr_length += diff.Length();
			vertices[ y * MAX_PATCH_WIDTH + x ].tex[ layer ].x = safe_fdiv( curr_length, total_length );
		}
		total_xy_length.x += total_length;
	}
	// - Y
	for( int x = 0; x < xsize; ++x )
	{
		float total_length = 0;
		for( int y = 1; y < ysize; ++y )
		{
			Vec3 diff = vertices[ y * MAX_PATCH_WIDTH + x ].pos - vertices[ ( y - 1 ) * MAX_PATCH_WIDTH + x ].pos;
			total_length += diff.Length();
		}
		vertices[ x ].tex[ layer ].y = 0;
		float curr_length = 0;
		for( int y = 1; y < ysize; ++y )
		{
			Vec3 diff = vertices[ y * MAX_PATCH_WIDTH + x ].pos - vertices[ ( y - 1 ) * MAX_PATCH_WIDTH + x ].pos;
			curr_length += diff.Length();
			vertices[ y * MAX_PATCH_WIDTH + x ].tex[ layer ].y = safe_fdiv( curr_length, total_length );
		}
		total_xy_length.y += total_length;
	}
	
	// divided by number of samples
	total_xy_length /= V2( ysize, xsize );
	return total_xy_length;
}

void EdPatch::TexGenScale( int layer, const Vec2& scale )
{
	// map 1x1 (prev 'if'), fix aspect ratio
	for( int y = 0; y < ysize; ++y )
	{
		for( int x = 0; x < xsize; ++x )
		{
			vertices[ y * MAX_PATCH_WIDTH + x ].tex[ layer ] *= scale;
		}
	}
}

void EdPatch::TexGenFitNat( int layer )
{
	Vec2 total_xy_length = TexGenFit( layer );
	float minl = TMIN( total_xy_length.x, total_xy_length.y );
	total_xy_length.x = safe_fdiv( total_xy_length.x, minl );
	total_xy_length.y = safe_fdiv( total_xy_length.y, minl );
	TexGenScale( layer, total_xy_length );
}

void EdPatch::TexGenNatural( int layer )
{
	Vec2 total_xy_length = TexGenFit( layer );
	TexGenScale( layer, total_xy_length );
}

void EdPatch::TexGenPlanar( int layer )
{
	// find avg.nrm. to tangents and map on those
	Vec3 avgt1 = V3(0);
	Vec3 avgt2 = V3(0);
	for( int y = 0; y < ysize; ++y )
	{
		avgt1 += vertices[ y * MAX_PATCH_WIDTH + xsize - 1 ].pos - vertices[ y * MAX_PATCH_WIDTH ].pos;
	}
	for( int x = 0; x < xsize; ++x )
	{
		avgt2 += vertices[ ( ysize - 1 ) * MAX_PATCH_WIDTH + x ].pos - vertices[ x ].pos;
	}
	avgt1.Normalize();
	avgt2.Normalize();
	
	Vec3 normal = -Vec3Cross( avgt1, avgt2 ).Normalized();
	if( fabsf( Vec3Dot( normal, V3(0,0,1) ) ) > 0.707f )
	{
		avgt1 = V3( 1, 0, 0 );
	}
	else
	{
		Vec2 side = normal.ToVec2().Normalized().Perp();
		avgt1 = V3( side.x, side.y, 0 );
	}
	avgt2 = Vec3Cross( avgt1, normal ).Normalized();
	
	for( int y = 0; y < ysize; ++y )
	{
		for( int x = 0; x < xsize; ++x )
		{
			EdPatchVtx& PV = vertices[ y * MAX_PATCH_WIDTH + x ];
			PV.tex[ layer ] = V2( Vec3Dot( PV.pos + position, avgt1 ), Vec3Dot( PV.pos + position, avgt2 ) );
		}
	}
}

void EdPatch::TexGenPostProc( int layer )
{
	EdPatchLayerInfo& L = layers[ layer ];
	for( int y = 0; y < ysize; ++y )
	{
		for( int x = 0; x < xsize; ++x )
		{
			Vec2& vtx = vertices[ y * MAX_PATCH_WIDTH + x ].tex[ layer ];
			Vec2 txa = vtx.Rotate( DEG2RAD( L.angle ) );
			vtx = V2( txa.x / L.scale + L.xoff, txa.y / L.scale * L.aspect + L.yoff );
		}
	}
}


void EdPatch::EditUI()
{
	ImGui::BeginChangeCheck();
	
	g_EdWorld->m_groupMgr.GroupProperty( "Group", group );
	IMGUIEditVec3( "Position", position, -8192, 8192 );
	IMGUIComboBox( "Render mode", renderMode, "Static\0Transparent\0Decal\0" );
	IMGUIEditBool( "Casts lightmap shadows?", m_isLMSolid );
	IMGUIEditBool( "Is solid?", m_isPhySolid );
	IMGUIEditFloat( "Lightmap quality", lmquality, 0.01f, 100 );
	IMGUIEditInt( "Layer", blend, 0, 127 );
	ImGui::Separator();
	for( int i = 0; i < MAX_PATCH_LAYERS; ++i )
	{
		layers[ i ].EditUI( this, i );
	}
	
	if( ImGui::EndChangeCheck() )
		RegenerateMesh();
}

void EdPatch::VertEditUI( int vid )
{
	vertices[ ( vid % xsize ) + vid / xsize * MAX_PATCH_WIDTH ].EditUI();
}


EdPatch* EdPatch::CreatePatchFromSurface( EdBlock& B, int sid )
{
	if( B.GetSurfaceNumVerts( sid ) != 4 )
		return NULL;
	
	EdPatch* patch = new EdPatch;
	patch->xsize = 2;
	patch->ysize = 2;
	patch->position = B.position;
	LCVertex verts[100];
	B.GenerateSurface( verts, sid, false );
	EdPatchVtx pverts[4];
	memset( pverts, 0, sizeof(pverts) );
	for( int i = 0; i < 4; ++i )
	{
		pverts[ i ].pos = verts[ i ].pos - B.position;
		for( int ll = 0; ll < MAX_PATCH_LAYERS; ++ll )
		{
			pverts[ i ].tex[ ll ] = V2( verts[ i ].tx0, verts[ i ].ty0 );
			pverts[ i ].col[ ll ] = 0xffffffff;
		}
	}
	// input comes in polygon order, we need Z (scanline)
	patch->vertices[0] = pverts[0];
	patch->vertices[1] = pverts[1];
	patch->vertices[1+MAX_PATCH_WIDTH] = pverts[2];
	patch->vertices[0+MAX_PATCH_WIDTH] = pverts[3];
	EdSurface* S = B.surfaces[ sid ];
	patch->layers[0].texname = S->texname;
	patch->layers[0].xoff = S->xoff;
	patch->layers[0].yoff = S->yoff;
	patch->layers[0].scale = S->scale;
	patch->layers[0].aspect = S->aspect;
	return patch;
}


