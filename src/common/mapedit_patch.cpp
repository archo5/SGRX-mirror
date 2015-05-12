

#include "mapedit.hpp"



static const uint16_t ones_mask[ 16 ] = { 0,
	0x0001, 0x0003, 0x0007, 0x000f,
	0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff,
	0x1fff, 0x3fff, 0x7fff,
};


// at must be 0 to 14
static uint16_t insert_zero_bit( uint16_t v, int at )
{
	uint16_t msk = ones_mask[ at ];
	return ( v & msk ) | ( ( v & msk ) << 1 );
}
static uint16_t remove_bit( uint16_t v, int at )
{
	return ( v & ones_mask[ at ] ) | ( ( v & ones_mask[ at + 1 ] ) >> 1 );
}


bool EdPatch::InsertXLine( int at )
{
	if( at < 1 || at > (int) xsize - 1 || xsize >= MAX_PATCH_WIDTH )
		return false;
	for( int i = 0; i < (int) ysize; ++i )
	{
		EdPatchVtx* vp = vertices + MAX_PATCH_WIDTH * i + at;
		_InterpolateVertex( vp, vp-1, vp+1, 0.5f );
		memmove( vp + 1, vp, sizeof(*vertices) * ( xsize - at - 1 ) );
		edgeflip[ i ] = insert_zero_bit( edgeflip[ i ], at );
		vertsel[ i ] = insert_zero_bit( vertsel[ i ], at );
	}
	xsize++;
	return true;
}

bool EdPatch::InsertYLine( int at )
{
	if( at < 1 || at > (int) ysize - 1 || ysize >= MAX_PATCH_WIDTH )
		return false;
	memmove( vertices + MAX_PATCH_WIDTH * ( at + 1 ),
		vertices + MAX_PATCH_WIDTH * at,
		sizeof(*vertices) * MAX_PATCH_WIDTH * ( MAX_PATCH_WIDTH - at - 1 ) );
	memmove( edgeflip + at + 1, edgeflip + at, sizeof(*edgeflip) * ( MAX_PATCH_WIDTH - at - 1 ) );
	memmove( vertsel + at + 1, vertsel + at, sizeof(*vertsel) * ( MAX_PATCH_WIDTH - at - 1 ) );
	for( int i = 0; i < (int) xsize; ++i )
	{
		EdPatchVtx* vp = vertices + MAX_PATCH_WIDTH * at + i;
		_InterpolateVertex( vp, vp - MAX_PATCH_WIDTH, vp + MAX_PATCH_WIDTH, 0.5f );
	}
	ysize++;
	return true;
}

bool EdPatch::RemoveXLine( int at )
{
	if( at < 1 || at >= (int) xsize - 1 || xsize <= 2 )
		return false;
	for( int i = 0; i < (int) ysize; ++i )
	{
		EdPatchVtx* vp = vertices + MAX_PATCH_WIDTH * i + at;
		memmove( vp, vp + 1, sizeof(*vertices) * ( xsize - at - 1 ) );
		edgeflip[ i ] = remove_bit( edgeflip[ i ], at );
		vertsel[ i ] = remove_bit( vertsel[ i ], at );
	}
	xsize--;
	return true;
}

bool EdPatch::RemoveYLine( int at )
{
	if( at < 1 || at > (int) ysize - 1 || ysize >= MAX_PATCH_WIDTH )
		return false;
	memmove( vertices + MAX_PATCH_WIDTH * at,
		vertices + MAX_PATCH_WIDTH * ( at + 1 ),
		sizeof(*vertices) * MAX_PATCH_WIDTH * ( MAX_PATCH_WIDTH - at - 1 ) );
	memmove( edgeflip + at, edgeflip + at + 1, sizeof(*edgeflip) * ( MAX_PATCH_WIDTH - at - 1 ) );
	memmove( vertsel + at, vertsel + at + 1, sizeof(*vertsel) * ( MAX_PATCH_WIDTH - at - 1 ) );
	ysize--;
	return true;
}

void EdPatch::_InterpolateVertex( EdPatchVtx* out, EdPatchVtx* v0, EdPatchVtx* v1, float s )
{
	out->pos = TLERP( v0->pos, v1->pos, s );
	for( int i = 0; i < MAX_PATCH_LAYERS; ++i )
	{
		out->tex[ i ] = TLERP( v0->tex[ i ], v1->tex[ i ], s );
		out->col[ i ] = TLERP( v0->col[ i ], v1->col[ i ], s );
	}
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
	
	for( int layer = 0; layer < MAX_PATCH_LAYERS; ++layer )
	{
		EdPatchLayerInfo& LI = layers[ layer ];
		if( LI.texname.size() == 0 )
		{
			LI.cached_meshinst = NULL;
			LI.cached_mesh = NULL;
			LI.cached_texture = NULL;
			continue;
		}
		
		LI.Precache();
		if( !LI.cached_mesh )
			LI.cached_mesh = GR_CreateMesh();
		if( !LI.cached_meshinst )
		{
			LI.cached_meshinst = g_EdScene->CreateMeshInstance();
			LI.cached_meshinst->mesh = LI.cached_mesh;
			lmm_prepmeshinst( LI.cached_meshinst );
		}
		LI.cached_meshinst->matrix = g_EdWorld->m_groupMgr.GetMatrix( group );
		
		MaterialHandle mh = GR_CreateMaterial();
		mh->shader = GR_GetSurfaceShader( "default" );
		mh->textures[ 0 ] = LI.cached_texture;
		
		outverts.clear();
		outidcs.clear();
		int i = 0;
		for( int y = 0; y < (int) ysize; ++y )
		{
			for( int x = 0; x < (int) xsize; ++x )
			{
				Vec2 tx = vertices[ i ].tex[ layer ];
				LCVertex vert = { vertices[ i ].pos, V3(0,0,1), vertices[ i ].col[ layer ], tx.x, tx.y, 0, 0 };
				outverts.push_back( vert );
				i++;
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
		
		SGRX_MeshPart mp = { 0, outverts.size(), 0, outidcs.size(), mh };
		LI.cached_mesh->SetPartData( &mp, 1 );
	}
}

EdPatch* EdPatch::CreatePatchFromSurface( EdBlock& B, int sid )
{
	if( B.GetSurfaceNumVerts( sid ) != 4 )
		return NULL;
	
	EdPatch* patch = new EdPatch;
	patch->xsize = 2;
	patch->ysize = 2;
	patch->position = B.position;
	LCVertex verts[4];
	B.GenerateSurface( verts, sid );
	EdPatchVtx pverts[4];
	memset( pverts, 0, sizeof(pverts) );
	for( int i = 0; i < 4; ++i )
	{
		pverts[ i ].pos = verts[ i ].pos;
		pverts[ i ].tex[0] = V2( verts[ i ].tx0, verts[ i ].ty0 );
		pverts[ i ].col[0] = 0xffffffff;
	}
	return patch;
}

