

#include "engine_int.hpp"
#include "renderer.hpp"

BatchRenderer* g_BatchRenderer = NULL;
static FontRenderer* g_FontRenderer = NULL;
static SGRX_FontSettings g_CurFontSettings = { "", 0, 0.0f, -1.2f };

extern IRenderer* g_Renderer;



void GR2D_SetColor( float r, float g, float b, float a )
{
	g_BatchRenderer->Col( r, g, b, a );
}

BatchRenderer::BatchRenderer( struct IRenderer* r ) : m_renderer( r ), m_diff( false )
{
	m_proto.x = 0;
	m_proto.y = 0;
	m_proto.z = 0;
	m_proto.u = 0;
	m_proto.v = 0;
	m_proto.color = 0xffffffff;
	worldMatrix = Mat4::Identity;
	viewMatrix = Mat4::Identity;
	
	m_vertexDecl = GR_GetVertexDecl( "pf3cb40f2" );
	m_defVShader = GR_GetVertexShader( "sys_batchvtx" );
	m_defPShader = GR_GetPixelShader( "sys_batchvtx" );
	uint32_t whiteCol = 0xffffffff;
	m_whiteTex = GR_CreateTexture( 1, 1, TEXFMT_RGBA8, 0, 1, &whiteCol );
	
	m_currState.vshader = m_nextState.vshader = m_defVShader;
	m_currState.pshader = m_nextState.pshader = m_defPShader;
	
	ResetState();
}

BatchRenderer& BatchRenderer::AddVertices( Vertex* verts, int count )
{
	m_verts.reserve( m_verts.size() + count );
	for( int i = 0; i < count; ++i )
		AddVertex( verts[ i ] );
	return *this;
}

BatchRenderer& BatchRenderer::AddVertex( const Vertex& vert )
{
	if( m_diff )
	{
		Flush();
		m_currState = m_nextState;
		m_diff = false;
	}
	m_verts.push_back( vert );
	return *this;
}

BatchRenderer& BatchRenderer::Colb( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	uint32_t col;
	col = COLOR_RGBA( r, g, b, a );
	m_proto.color = col;
	return *this;
}

BatchRenderer& BatchRenderer::Prev( int i )
{
	if( i < 0 || i >= (int) m_verts.size() )
		AddVertex( m_proto );
	else
	{
		Vertex v = m_verts[ m_verts.size() - 1 - i ];
		AddVertex( v );
	}
	return *this;
}

BatchRenderer& BatchRenderer::RawQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float z )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x0, y0, z );
	Tex( 1, 0 ); Pos( x1, y1, z );
	Tex( 1, 1 ); Pos( x2, y2, z );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x3, y3, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::Quad( float x0, float y0, float x1, float y1, float z )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x0, y0, z );
	Tex( 1, 0 ); Pos( x1, y0, z );
	Tex( 1, 1 ); Pos( x1, y1, z );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x0, y1, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::QuadTexRect( float x0, float y0, float x1, float y1,
	float tx0, float ty0, float tx1, float ty1 )
{
	float z = 0;
	SetPrimitiveType( PT_Triangles );
	Tex( tx0, ty0 ); Pos( x0, y0, z );
	Tex( tx1, ty0 ); Pos( x1, y0, z );
	Tex( tx1, ty1 ); Pos( x1, y1, z );
	Prev( 0 );
	Tex( tx0, ty1 ); Pos( x0, y1, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::QuadExt( float x0, float y0, float x1, float y1,
	float tox, float toy, float tsx, float tsy, float z )
{
	SetPrimitiveType( PT_Triangles );
	Tex( tox, toy ); Pos( x0, y0, z );
	Tex( tox+tsx, toy ); Pos( x1, y0, z );
	Tex( tox+tsx, toy+tsy ); Pos( x1, y1, z );
	Prev( 0 );
	Tex( tox, toy+tsy ); Pos( x0, y1, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::QuadFrame( float x0, float y0, float x1, float y1, float ix0, float iy0, float ix1, float iy1, float z )
{
	RawQuad( x0, y0, x1, y0, ix1, iy0, ix0, iy0, z );
	RawQuad( x1, y0, x1, y1, ix1, iy1, ix1, iy0, z );
	RawQuad( x1, y1, x0, y1, ix0, iy1, ix1, iy1, z );
	RawQuad( x0, y1, x0, y0, ix0, iy0, ix0, iy1, z );
	return *this;
}

BatchRenderer& BatchRenderer::VPQuad( SGRX_Viewport* vp, bool texcoords )
{
	SetPrimitiveType( PT_Triangles );
	
	float x0 = vp ? safe_fdiv( vp->x0, GR_GetWidth() ) : 0;
	float y0 = vp ? safe_fdiv( vp->y0, GR_GetHeight() ) : 0;
	float x1 = vp ? safe_fdiv( vp->x1, GR_GetWidth() ) : 1;
	float y1 = vp ? safe_fdiv( vp->y1, GR_GetHeight() ) : 1;
	
	float tx0 = texcoords ? x0 : 0;
	float ty0 = texcoords ? y0 : 0;
	float tx1 = texcoords ? x1 : 1;
	float ty1 = texcoords ? y1 : 1;
	
	Tex( tx0, ty0 ); Pos( x0, y0 );
	Tex( tx1, ty0 ); Pos( x1, y0 );
	Tex( tx1, ty1 ); Pos( x1, y1 );
	Prev( 0 );
	Tex( tx0, ty1 ); Pos( x0, y1 );
	Prev( 4 );
	
	return *this;
}

BatchRenderer& BatchRenderer::TurnedBox( float x, float y, float dx, float dy, float z )
{
	float tx = -dy;
	float ty = dx;
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x - dx - tx, y - dy - ty, z );
	Tex( 1, 0 ); Pos( x - dx + tx, y - dy + ty, z );
	Tex( 1, 1 ); Pos( x + dx + tx, y + dy + ty, z );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x + dx - tx, y + dy - ty, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::Button( Vec4 rect, Vec4 bdr, Vec4 texbdr, float z )
{
	SetPrimitiveType( PT_Triangles );
	
	float pox[4] = { rect.x, rect.x + bdr.x, rect.z - bdr.z, rect.z };
	float poy[4] = { rect.y, rect.y + bdr.y, rect.w - bdr.w, rect.w };
	float txx[4] = { 0, texbdr.x, 1 - texbdr.z, 1 };
	float txy[4] = { 0, texbdr.y, 1 - texbdr.w, 1 };
	
	for( int y = 0; y < 3; ++y )
	{
		for( int x = 0; x < 3; ++x )
		{
			Tex( txx[x], txy[y] ); Pos( pox[x], poy[y], z );
			Tex( txx[x+1], txy[y] ); Pos( pox[x+1], poy[y], z );
			Tex( txx[x+1], txy[y+1] ); Pos( pox[x+1], poy[y+1], z );
			Prev(0);
			Tex( txx[x], txy[y+1] ); Pos( pox[x], poy[y+1], z );
			Prev(4);
		}
	}
	
	return *this;
}

BatchRenderer& BatchRenderer::Poly( const void* data, int count, float z, int stride )
{
	SGRX_CAST( const uint8_t*, bp, data );
	SetPrimitiveType( PT_TriangleStrip );
	for( int i = 0; i < count; ++i )
	{
		int v;
		if( i % 2 == 0 )
			v = i / 2;
		else
			v = count - 1 - i / 2;
		Pos( *(float*)(bp+v*stride), *(float*)(bp+v*stride+4), stride >= 12 ? *(float*)(bp+i*stride+8) : z );
	}
	return *this;
}

BatchRenderer& BatchRenderer::PolyOutline( const void* data, int count, float z, int stride )
{
	SGRX_CAST( const uint8_t*, bp, data );
	SetPrimitiveType( PT_LineStrip );
	for( int i = 0; i < count; ++i )
		Pos( *(float*)(bp+i*stride), *(float*)(bp+i*stride+4), stride >= 12 ? *(float*)(bp+i*stride+8) : z );
	Prev( count - 1 );
	return *this;
}

BatchRenderer& BatchRenderer::Sprite( const Vec3& pos, const Vec3& dx, const Vec3& dy )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( pos + dx + dy );
	Tex( 1, 0 ); Pos( pos - dx + dy );
	Tex( 1, 1 ); Pos( pos - dx - dy );
	Prev( 0 );
	Tex( 0, 1 ); Pos( pos + dx - dy );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::Sprite( const Vec3& pos, float sx, float sy )
{
	Vec3 dx = invMatrix.TransformNormal( V3( 1, 0, 0 ) ).Normalized() * sx;
	Vec3 dy = invMatrix.TransformNormal( V3( 0, 1, 0 ) ).Normalized() * sy;
	return Sprite( pos, dx, dy );
}

BatchRenderer& BatchRenderer::TexLine( const Vec2& p0, const Vec2& p1, float rad )
{
	SetPrimitiveType( PT_Triangles );
	Vec2 D = ( p1 - p0 ).Normalized();
	Vec2 T = D.Perp();
	
	Tex( 0   , 0 ); Pos( p0 + ( -D +T ) * rad );
	Tex( 0.5f, 0 ); Pos( p0 + (    +T ) * rad );
	Tex( 0.5f, 1 ); Pos( p0 + (    -T ) * rad );
	Prev( 0 ); // #4 requesting #3
	Tex( 0   , 1 ); Pos( p0 + ( -D -T ) * rad );
	Prev( 4 ); // #6 requesting #1
	
	Prev( 4 ); // #7 requesting #2
	Tex( 0.5f, 0 ); Pos( p1 + (    +T ) * rad );
	Tex( 0.5f, 1 ); Pos( p1 + (    -T ) * rad );
	Prev( 0 ); // #10 requesting #9
	Prev( 7 ); // #11 requesting #3
	Prev( 9 ); // #12 requesting #2
	
	Prev( 4 ); // #13 requesting #8
	Tex( 1   , 0 ); Pos( p1 + ( +D +T ) * rad );
	Tex( 1   , 1 ); Pos( p1 + ( +D -T ) * rad );
	Prev( 0 ); // #16 requesting #15
	Prev( 7 ); // #17 requesting #9
	Prev( 9 ); // #18 requesting #8
	
	return *this;
}

BatchRenderer& BatchRenderer::CircleFill( float x, float y, float r, float z, int verts )
{
	if( verts < 0 )
		verts = r * M_PI * 2;
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_Triangles );
		float ad = M_PI * 2.0f / verts;
		float a = ad, ps = 0, pc = 1;
		for( int i = 0; i < verts; ++i )
		{
			float cs = sin( a ), cc = cos( a );
			Pos( x + ps * r, y + pc * r, z );
			Pos( x + cs * r, y + cc * r, z );
			Pos( x, y, z );
			pc = cc;
			ps = cs;
			a += ad;
		}
	}
	return *this;
}

BatchRenderer& BatchRenderer::CircleOutline( float x, float y, float r, float z, int verts )
{
	if( verts < 0 )
		verts = r * M_PI * 2;
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_LineStrip );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			Pos( x + sin( a ) * r, y + cos( a ) * r, z );
			a += ad;
		}
		Prev( verts - 1 );
	}
	return *this;
}

BatchRenderer& BatchRenderer::CircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts )
{
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			if( i > 1 )
				Prev( 0 );
			Pos( pos + sin( a ) * dx + cos( a ) * dy );
			a += ad;
		}
		Prev( 0 );
		Prev( verts * 2 - 2 );
	}
	return *this;
}

BatchRenderer& BatchRenderer::HalfCircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts )
{
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI / ( verts - 1 );
		for( int i = 0; i < verts; ++i )
		{
			if( i > 1 )
				Prev( 0 );
			Pos( pos + sin( a ) * dy + cos( a ) * dx );
			a += ad;
		}
	}
	return *this;
}

BatchRenderer& BatchRenderer::SphereOutline( const Vec3& pos, float radius, int verts )
{
	CircleOutline( pos, V3(radius,0,0), V3(0,radius,0), verts );
	CircleOutline( pos, V3(0,radius,0), V3(0,0,radius), verts );
	CircleOutline( pos, V3(0,0,radius), V3(radius,0,0), verts );
	return *this;
}

BatchRenderer& BatchRenderer::CapsuleOutline( const Vec3& pos, float radius, const Vec3& nrm, float ht, int verts )
{
	ht /= 2;
	Vec3 N = nrm.Normalized();
	if( nrm.LengthSq() < SMALL_FLOAT )
		return SphereOutline( pos, radius, verts );
	
	Vec3 refdir = V3( 0, 0, nrm.z >= 0 ? 1 : -1 );
	Vec3 rotaxis = Vec3Cross( N, refdir );
	float rotangle = acosf( clamp( Vec3Dot( N, refdir ), -1, 1 ) );
	
	Mat4 rot = Mat4::CreateRotationAxisAngle( rotaxis.Normalized(), -rotangle );
	Vec3 T = rot.TransformNormal( V3(1,0,0) ).Normalized();
	Vec3 B = rot.TransformNormal( V3(0,1,0) ).Normalized();
	Vec3 Tr = T * radius, Br = B * radius, Nr = N * radius;
	
	Vec3 p0 = pos - N * ht, p1 = pos + N * ht;
	
	HalfCircleOutline( p1, Tr, Nr, verts / 2 );
	HalfCircleOutline( p1, Br, Nr, verts / 2 );
	HalfCircleOutline( p0, Tr, -Nr, verts / 2 );
	HalfCircleOutline( p0, Br, -Nr, verts / 2 );
	CircleOutline( p0, Tr, Br, verts );
	if( ht )
	{
		CircleOutline( p1, Tr, Br, verts );
		SetPrimitiveType( PT_Lines );
		Pos( p0 - Tr ); Pos( p1 - Tr );
		Pos( p0 + Tr ); Pos( p1 + Tr );
		Pos( p0 - Br ); Pos( p1 - Br );
		Pos( p0 + Br ); Pos( p1 + Br );
	}
	
	return *this;
}

BatchRenderer& BatchRenderer::ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, float angle, int verts )
{
	return ConeOutline( pos, dir, up, radius, V2(angle), verts );
}

BatchRenderer& BatchRenderer::ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, Vec2 angles, int verts )
{
	if( verts >= 3 )
	{
		Vec2 hra = DEG2RAD( angles ) / 2.0f;
		Vec3 dx = Vec3Cross(dir,up).Normalized();
		Vec3 dy = Vec3Cross(dir,dx).Normalized();
		dx *= radius * sinf( hra.x );
		dy *= radius * sinf( hra.y );
		
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			float ca = cosf( a ), sa = sinf( a );
			float aca = fabsf( ca ), asa = fabsf( sa );
			float distangle = ( aca * hra.x + asa * hra.y ) / ( aca + asa );
			float rc = radius * cosf( distangle );
			
			Pos( pos );
			Pos( pos + dir * rc + ca * dx + sa * dy );
			
			if( i > 0 )
			{
				Prev( 2 ); // previous line position
				Prev( 1 ); // current line position (-1 from last insertion)
			}
			
			a += ad;
		}
		
		Prev( 2 ); // previous line position
		Prev( verts * 2 + ( verts - 1 ) * 2 - 1 ); // 1st line position
	}
	return *this;
}

BatchRenderer& BatchRenderer::Frustum( const Mat4& vpmtx )
{
	Mat4 inv = vpmtx.Inverted();
	return AABB( V3(-1), V3(1), inv );
}

BatchRenderer& BatchRenderer::AABB( const Vec3& bbmin, const Vec3& bbmax, const Mat4& transform )
{
	Vec3 pp[8] =
	{
		transform.TransformPos( V3(bbmin.x,bbmin.y,bbmin.z) ), transform.TransformPos( V3(bbmax.x,bbmin.y,bbmin.z) ),
		transform.TransformPos( V3(bbmin.x,bbmax.y,bbmin.z) ), transform.TransformPos( V3(bbmax.x,bbmax.y,bbmin.z) ),
		transform.TransformPos( V3(bbmin.x,bbmin.y,bbmax.z) ), transform.TransformPos( V3(bbmax.x,bbmin.y,bbmax.z) ),
		transform.TransformPos( V3(bbmin.x,bbmax.y,bbmax.z) ), transform.TransformPos( V3(bbmax.x,bbmax.y,bbmax.z) ),
	};
	SetPrimitiveType( PT_Lines );
	// X
	Pos( pp[0] ); Pos( pp[1] ); Pos( pp[2] ); Pos( pp[3] ); Pos( pp[4] ); Pos( pp[5] ); Pos( pp[6] ); Pos( pp[7] );
	// Y
	Pos( pp[0] ); Pos( pp[2] ); Pos( pp[1] ); Pos( pp[3] ); Pos( pp[4] ); Pos( pp[6] ); Pos( pp[5] ); Pos( pp[7] );
	// Z
	Pos( pp[0] ); Pos( pp[4] ); Pos( pp[1] ); Pos( pp[5] ); Pos( pp[2] ); Pos( pp[6] ); Pos( pp[3] ); Pos( pp[7] );
	return *this;
}

BatchRenderer& BatchRenderer::Tick( const Vec3& pos, float radius, const Mat4& transform )
{
	SetPrimitiveType( PT_Lines );
	Pos( transform.TransformPos( pos + V3(-radius,0,0) ) ); Pos( transform.TransformPos( pos + V3(radius,0,0) ) );
	Pos( transform.TransformPos( pos + V3(0,-radius,0) ) ); Pos( transform.TransformPos( pos + V3(0,radius,0) ) );
	Pos( transform.TransformPos( pos + V3(0,0,-radius) ) ); Pos( transform.TransformPos( pos + V3(0,0,radius) ) );
	return *this;
}

BatchRenderer& BatchRenderer::Axis( const Mat4& transform, float size )
{
	Vec3 pos = transform.TransformPos( V3(0) );
	Vec3 ax = transform.TransformNormal( V3(1,0,0) ).Normalized() * size;
	Vec3 ay = transform.TransformNormal( V3(0,1,0) ).Normalized() * size;
	Vec3 az = transform.TransformNormal( V3(0,0,1) ).Normalized() * size;
	SetPrimitiveType( PT_Lines );
	Col( 1, 0, 0 ); Pos( pos ); Pos( pos + ax );
	Col( 0, 1, 0 ); Pos( pos ); Pos( pos + ay );
	Col( 0, 0, 1 ); Pos( pos ); Pos( pos + az );
	return *this;
}

static const float cos135deg = cosf( FLT_PI * 0.75f );
BatchRenderer& BatchRenderer::AAPoly( const Vec2* polydata, size_t polysize, float z )
{
	if( polysize < 3 )
		return *this;
	
	Vec2 center = V2(0);
	for( size_t i = 0; i < polysize; ++i )
		center += polydata[ i ];
	center /= polysize;
	
	uint32_t colorf = m_proto.color;
	uint32_t colora = colorf & 0x00ffffff;
	Vec2 prev_n;
	SetPrimitiveType( PT_Triangles );
	for( size_t i = 0; i <= polysize; ++i )
	{
		size_t i0 = ( i + polysize - 1 ) % polysize;
		size_t i1 = i % polysize;
		size_t i2 = ( i + 1 ) % polysize;
		Vec2 p0 = polydata[ i0 ];
		Vec2 p1 = polydata[ i1 ];
		Vec2 p2 = polydata[ i2 ];
		Vec2 d01 = ( p1 - p0 ).Normalized();
		Vec2 d12 = ( p2 - p1 ).Normalized();
		float dot = clamp( Vec2Dot( d01, d12 ), -1, 1 );
		Vec2 n;
		if( dot < cos135deg )
		{
			// TODO MAYBE FIX
			n = V2(0);
		}
		else
		{
			n = ( d12 + d01 ).Normalized().Perp2() / cosf( 0.5f * acosf( dot ) );
		}
		
		n *= 0.5f; // half in each direction for a full pixel in both
		if( i != 0 )
		{
			Vec2 curr_n = n;
			Vec2 i0in = p0 - prev_n, i0out = p0 + prev_n;
			Vec2 i1in = p1 - curr_n, i1out = p1 + curr_n;
			
			// 3 triangles: (center, in = opaque, out = transparent)
			// - center, i0in, i1in
			// - i0out, i1out, i0in(3)
			// - i1out(1), i1in(4), i0in(2)
			
			m_proto.color = colorf; Pos( center, z ); Pos( i0in, z ); Pos( i1in, z );
			m_proto.color = colora; Pos( i0out, z ); Pos( i1out, z ); Prev( 3 );
			Prev( 1 ); Prev( 4 ); Prev( 2 );
		}
		prev_n = n;
	}
	m_proto.color = colorf;
	return *this;
}

BatchRenderer& BatchRenderer::AAStroke( const Vec2* linedata, size_t linesize, float width, bool closed, float z )
{
	if( linesize < 2 )
		return *this;
	
	float width0 = width - 1;
	float width1 = width + 1;
	uint32_t colorf = m_proto.color;
	uint32_t colora = colorf & 0x00ffffff;
	Vec2 prev_n;
	SetPrimitiveType( PT_Triangles );
	for( size_t i = 0; i < linesize + (closed?1:0); ++i )
	{
		size_t i0 = ( i + linesize - 1 ) % linesize;
		size_t i1 = i % linesize;
		size_t i2 = ( i + 1 ) % linesize;
		if( closed == false )
		{
			if( i1 == 0 ) i0 = i1;
			if( i1 == linesize - 1 ) i2 = i1;
		}
		Vec2 p0 = linedata[ i0 ];
		Vec2 p1 = linedata[ i1 ];
		Vec2 p2 = linedata[ i2 ];
		Vec2 d01 = ( p1 - p0 ).Normalized();
		Vec2 d12 = ( p2 - p1 ).Normalized();
		float dot = clamp( Vec2Dot( d01, d12 ), -1, 1 );
		Vec2 n;
		if( dot < cos135deg )
		{
			// TODO MAYBE FIX
			n = V2(0);
		}
		else
		{
			n = ( d12 + d01 ).Normalized().Perp2() / cosf( 0.5f * acosf( dot ) );
		}
		
		n *= 0.5f; // half in each direction for a full pixel in both
		if( i != 0 )
		{
			Vec2 curr_n = n;
			Vec2 pin0 = p0 + prev_n * width0, pout0 = p0 + prev_n * width1;
			Vec2 pin1 = p1 + curr_n * width0, pout1 = p1 + curr_n * width1;
			Vec2 nin0 = p0 - prev_n * width0, nout0 = p0 - prev_n * width1;
			Vec2 nin1 = p1 - curr_n * width0, nout1 = p1 - curr_n * width1;
			
			// 6 triangles: (in = opaque, out = transparent)
			// - pout0, pout1, pin1
			// - pin1(0), pin0, pout0(4)
			// - nout0, nout1, nin1
			// - nin1(0), nin0, nout0(4)
			// - pin0(7), pin1(9), nin1(4)
			// - nin1(0), nin0(5), pin0(4)
			
			m_proto.color = colora; Pos( pout0 ); Pos( pout1 );
			m_proto.color = colorf; Pos( pin1 ); Prev( 0 ); Pos( pin0 ); Prev( 4 );
			m_proto.color = colora; Pos( nout0 ); Pos( nout1 );
			m_proto.color = colorf; Pos( nin1 ); Prev( 0 ); Pos( nin0 ); Prev( 4 );
			Prev( 7 ); Prev( 9 ); Prev( 4 );
			Prev( 0 ); Prev( 5 ); Prev( 4 );
		}
		prev_n = n;
	}
	m_proto.color = colorf;
	return *this;
}

BatchRenderer& BatchRenderer::AARect( float x0, float y0, float x1, float y1, float z )
{
	Vec2 verts[] = { V2(x0,y0), V2(x1,y0), V2(x1,y1), V2(x0,y1) };
	AAPoly( verts, 4, z );
	return *this;
}

BatchRenderer& BatchRenderer::AARectOutline( float x0, float y0, float x1, float y1, float width, float z )
{
	Vec2 verts[] = { V2(x0,y0), V2(x1,y0), V2(x1,y1), V2(x0,y1) };
	AAStroke( verts, 4, width, true, z );
	return *this;
}

BatchRenderer& BatchRenderer::AAEllipsoid( float x, float y, float rx, float ry, float z, int verts )
{
	if( verts < 0 )
		verts = TMAX( rx, ry ) * M_PI * 2;
	if( verts >= 3 )
	{
		m_polyCache.clear();
		m_polyCache.resize( verts );
		float ad = M_PI * 2.0f / verts;
		float a = ad;
		for( int i = 0; i < verts; ++i )
		{
			float cs = sinf( a ), cc = cosf( a );
			m_polyCache[ i ] = V2( x + cs * rx, y + cc * ry );
			a += ad;
		}
		AAPoly( m_polyCache.data(), m_polyCache.size(), z );
	}
	return *this;
}

BatchRenderer& BatchRenderer::AAEllipsoidOutline( float x, float y, float rx, float ry, float width, float z, int verts )
{
	if( verts < 0 )
		verts = TMAX( rx, ry ) * M_PI * 2;
	if( verts >= 3 )
	{
		m_polyCache.clear();
		m_polyCache.resize( verts );
		float ad = M_PI * 2.0f / verts;
		float a = ad;
		for( int i = 0; i < verts; ++i )
		{
			float cs = sinf( a ), cc = cosf( a );
			m_polyCache[ i ] = V2( x + cs * rx, y + cc * ry );
			a += ad;
		}
		AAStroke( m_polyCache.data(), m_polyCache.size(), width, true, z );
	}
	return *this;
}

BatchRenderer& BatchRenderer::AACircle( float x, float y, float r, float z, int verts )
{
	return AAEllipsoid( x, y, r, r, z, verts );
}

BatchRenderer& BatchRenderer::AACircleOutline( float x, float y, float r, float width, float z, int verts )
{
	return AAEllipsoidOutline( x, y, r, r, width, z, verts );
}

bool BatchRenderer::CheckSetTexture( int i, const TextureHandle& tex )
{
	ASSERT( i >= 0 && i < SGRX_MAX_TEXTURES );
	if( tex != m_nextState.textures[ i ] )
	{
		m_nextState.textures[ i ] = tex;
		_UpdateDiff();
		return true;
	}
	_UpdateDiff();
	return false;
}

BatchRenderer& BatchRenderer::SetVertexShader( const VertexShaderHandle& shd )
{
	m_nextState.vshader = shd ? shd : m_defVShader;
	_UpdateDiff();
	return *this;
}

BatchRenderer& BatchRenderer::SetPixelShader( const PixelShaderHandle& shd )
{
	m_nextState.pshader = shd ? shd : m_defPShader;
	_UpdateDiff();
	return *this;
}

inline bool _is_noncont_primtype( EPrimitiveType pt )
{
	return pt == PT_LineStrip || pt == PT_TriangleStrip;
}

BatchRenderer& BatchRenderer::SetPrimitiveType( EPrimitiveType pt )
{
	if( _is_noncont_primtype( pt ) || _is_noncont_primtype( m_nextState.primType ) )
		Flush();
	m_nextState.primType = pt;
	_UpdateDiff();
	return *this;
}

BatchRenderer& BatchRenderer::QuadsToTris()
{
	if( m_nextState.primType == PT_Triangles && m_verts.size() % 4 == 0 )
	{
		size_t oldsize = m_verts.size();
		size_t newsize = oldsize / 4 * 6;
		m_verts.resize( newsize );
		for( size_t i = oldsize, j = newsize; i > 0; )
		{
			i -= 4;
			j -= 6;
			Vertex vts[4] = { m_verts[ i ], m_verts[ i + 1 ], m_verts[ i + 2 ], m_verts[ i + 3 ] };
			m_verts[ j+0 ] = vts[0];
			m_verts[ j+1 ] = vts[1];
			m_verts[ j+2 ] = vts[2];
			m_verts[ j+3 ] = vts[2];
			m_verts[ j+4 ] = vts[4];
			m_verts[ j+5 ] = vts[0];
		}
	}
	else
		LOG_ERROR << __FUNCTION__ << " - incorrect vertex count: " << m_verts.size();
	return *this;
}

BatchRenderer& BatchRenderer::Flush()
{
	if( m_verts.size() )
	{
		RenderStateHandle rsh = GR_GetRenderState( RenderState );
		VtxInputMapHandle vimh = GR_GetVertexInputMapping( m_currState.vshader, m_vertexDecl );
		
		GR_PreserveResource( rsh );
		GR_PreserveResource( vimh );
		
		SGRX_ImmDrawData immdd =
		{
			m_verts.data(), m_verts.size(),
			m_currState.primType,
			m_vertexDecl,
			vimh,
			m_currState.vshader,
			m_currState.pshader,
			rsh,
			ShaderData.data(), ShaderData.size()
		};
		for( int i = 0; i < SGRX_MAX_TEXTURES; ++i )
			immdd.textures[ i ] = m_currState.textures[ i ];
		if( immdd.textures[ 0 ] == NULL )
			immdd.textures[ 0 ] = m_whiteTex;
		
		m_renderer->DrawImmediate( immdd );
		
		m_verts.clear();
	}
	return *this;
}

BatchRenderer& BatchRenderer::Reset()
{
	ShaderData.clear();
	for( size_t i = 0; i < SGRX_MAX_TEXTURES; ++i )
		CheckSetTexture( i, NULL );
	SetVertexShader( NULL );
	SetPixelShader( NULL );
	SetPrimitiveType( PT_None );
	m_proto.color = 0xffffffff;
	m_proto.u = 0;
	m_proto.v = 0;
	return *this;
}

void BatchRenderer::_UpdateDiff()
{
	m_diff = m_currState.IsDiff( m_nextState );
}

void BatchRenderer::_RecalcMatrices()
{
	invMatrix = Mat4::Identity;
	( worldMatrix * viewMatrix ).InvertTo( invMatrix );
}



void GR2D_SetWorldMatrix( const Mat4& mtx )
{
	g_BatchRenderer->Flush();
	g_BatchRenderer->worldMatrix = mtx;
	g_BatchRenderer->_RecalcMatrices();
	g_Renderer->SetMatrix( false, mtx );
}

void GR2D_SetViewMatrix( const Mat4& mtx )
{
	g_BatchRenderer->Flush();
	g_BatchRenderer->viewMatrix = mtx;
	g_BatchRenderer->_RecalcMatrices();
	g_Renderer->SetMatrix( true, mtx );
}

void GR2D_SetViewport( int x0, int y0, int x1, int y1 )
{
	g_BatchRenderer->Flush();
	g_Renderer->SetViewport( x0, y0, x1, y1 );
}

void GR2D_UnsetViewport()
{
	g_BatchRenderer->Flush();
	g_Renderer->SetViewport( 0, 0, GR_GetWidth(), GR_GetHeight() );
}

void GR2D_SetScissorRect( int x0, int y0, int x1, int y1 )
{
	int rect[4] = { x0, y0, x1, y1 };
	GR2D_SetScissorRect( rect );
}

static bool useScissor;
static int scissorRect[4];
void GR2D_SetScissorRect( int rect[4] )
{
	useScissor = rect != NULL;
	if( rect )
		memcpy( scissorRect, rect, sizeof(scissorRect) );
	g_BatchRenderer->Flush();
	g_Renderer->SetScissorRect( rect );
}

void GR2D_UnsetScissorRect()
{
	useScissor = false;
	g_BatchRenderer->Flush();
	g_Renderer->SetScissorRect( NULL );
}

bool GR2D_GetScissorRect( int rect[4] )
{
	memcpy( rect, scissorRect, sizeof(scissorRect) );
	return useScissor;
}


BatchRenderer& GR2D_GetBatchRenderer()
{
	return *g_BatchRenderer;
}



SGRX_LineSet::SGRX_LineSet() : m_nextPos(0), m_curCount(0)
{
	IncreaseLimit( 1024 );
}

void SGRX_LineSet::IncreaseLimit( size_t maxlines )
{
	if( maxlines * 2 > m_lines.size() )
	{
		m_lines.resize( maxlines * 2 );
	}
}

void SGRX_LineSet::DrawLine( const Vec3& p1, const Vec3& p2, uint32_t col )
{
	DrawLine( p1, p2, col, col );
}

void SGRX_LineSet::DrawLine( const Vec3& p1, const Vec3& p2, uint32_t c1, uint32_t c2 )
{
	Point pt1 = { p1, c1 };
	Point pt2 = { p2, c2 };
	m_lines[ m_nextPos ] = pt1;
	m_lines[ m_nextPos+1 ] = pt2;
	m_curCount = TMAX( m_nextPos + 2, m_curCount );
	m_nextPos = ( m_nextPos + 2 ) % m_lines.size();
}

void SGRX_LineSet::Flush()
{
	BatchRenderer& br = g_BatchRenderer->Reset().SetPrimitiveType( PT_Lines );
	for( size_t i = 0; i < m_curCount; ++i )
	{
		br.Colu( m_lines[ i ].color ).Pos( m_lines[ i ].pos );
	}
	br.Flush();
	m_nextPos = 0;
	m_curCount = 0;
}



void GR2D_GetFontSettings( SGRX_FontSettings* settings )
{
	*settings = g_CurFontSettings;
//	SGRX_IFont* fnt = g_FontRenderer->m_currentFont;
//	settings->font = fnt ? fnt->m_key : "";
//	settings->size = g_FontRenderer->m_currentSize;
//	settings->letterspacing = g_CurFontSettings.letterspacing;
//	settings->lineheight = g_CurFontSettings.lineheight;
}

void GR2D_SetFontSettings( const SGRX_FontSettings* settings )
{
	g_CurFontSettings = *settings;
	GR2D_SetFont( settings->font, settings->size );
}

bool GR2D_SetFont( const StringView& name, int pxsize )
{
	g_CurFontSettings.font = name;
	g_CurFontSettings.size = pxsize;
	return g_FontRenderer->SetFont( name, pxsize );
}

void GR2D_SetLetterSpacing( float lsp )
{
	g_CurFontSettings.letterspacing = lsp;
}

void GR2D_SetLineHeight( float lht )
{
	g_CurFontSettings.lineheight = lht;
}

void GR2D_SetTextCursor( const Vec2& pos )
{
	g_FontRenderer->SetCursor( pos );
}

Vec2 GR2D_GetTextCursor()
{
	return g_FontRenderer->m_cursor;
}

int GR2D_GetTextLength( const StringView& text )
{
	if( !g_FontRenderer )
		return 0;
	return g_FontRenderer->GetTextWidth( text );
}

int GR2D_DrawTextLine( const StringView& text )
{
	return g_FontRenderer->PutText( g_BatchRenderer, text );
}

int GR2D_DrawTextLine( float x, float y, const StringView& text )
{
	g_FontRenderer->SetCursor( V2( x, y ) );
	return g_FontRenderer->PutText( g_BatchRenderer, text );
}

int GR2D_DrawTextLine( const StringView& text, int halign, int valign )
{
	Vec2 pos = GR2D_GetTextCursor();
	int ret = GR2D_DrawTextLine( pos.x, pos.y, text, halign, valign );
	GR2D_SetTextCursor( GR2D_GetTextCursor().x, pos.y );
	return ret;
}

int GR2D_DrawTextLine( float x, float y, const StringView& text, int halign, int valign )
{
	if( !g_FontRenderer->m_currentFont )
		return 0;
	float length = 0;
	if( halign != 0 )
		length = g_FontRenderer->GetTextWidth( text );
	return GR2D_DrawTextLine( x - round( halign * 0.5f * length ), round( y - valign * 0.5f * g_FontRenderer->m_currentSize ), text );
}

struct TextBlock
{
	int start;
	int end;
	int pxwidth;
	int pxheight;
	SGRX_TextSettings settings;
};

struct TextLine
{
	int blockstart;
	int blockend;
	int pxwidth;
	int pxheight;
};

static int _blocks_width( const Array< TextBlock >& blocks, const TextLine& cur_line )
{
	int len = 0;
	for( int i = cur_line.blockend; i < int(blocks.size()); ++i )
		len += blocks[ i ].pxwidth;
	return len;
}

int _GR2D_CalcTextLayout
(
	Array< SGRX_TextSettings >& settingStack,
	Array< TextBlock >& blocks,
	Array< TextLine >& lines,
	const StringView& text,
	int width,
	int height,
	bool parse
)
{
	LOG_FUNCTION;
	
	settingStack.clear();
	SGRX_TextSettings textSettings;
	GR2D_GetFontSettings( &textSettings );
	textSettings.color = g_BatchRenderer->m_proto.color;
	settingStack.push_back( textSettings );
	
	int total_height = 0;
	int cur_word_width = 0;
	int cur_word_start = 0;
	
	TextBlock cur_block = { 0, 0, 0, 0 };
	TextLine cur_line = { 0, 0, 0, 0 };
	
	uint32_t prev_chr_val = 0;
	
#define LAST_BLOCKS_WIDTH (_blocks_width(blocks,cur_line)+cur_block.pxwidth)
#define COMMIT_BLOCK \
	if( cur_block.end > cur_block.start ) \
	{ \
	/*	LOG << "NEW BLOCK["<<__LINE__<<",len="<<cur_block.pxwidth<<"]:" << text.part(cur_block.start,cur_block.end-cur_block.start); */ \
		cur_block.settings = settingStack.last(); \
		blocks.push_back( cur_block ); \
		cur_block.start = cur_block.end; \
		cur_block.pxwidth = 0; \
		cur_block.pxheight = 0; \
	}
#define COMMIT_BLOCK_BEFCH( off ) \
	cur_word_width = 0; \
	cur_block.end = off; \
	COMMIT_BLOCK; \
	cur_block.start = cur_block.end = cur_word_start = IT.offset;
#define PUSH_BLOCKS_TO_LINE \
	for( ; cur_line.blockend < int(blocks.size()); ++cur_line.blockend ) \
	{ \
		cur_line.pxwidth += blocks[ cur_line.blockend ].pxwidth; \
		cur_line.pxheight = TMAX( cur_line.pxheight, blocks[ cur_line.blockend ].pxheight ); \
	}
		
#define COMMIT_LINE( forceht ) \
	while( cur_line.blockend > cur_line.blockstart ){ \
		TextBlock& TB = blocks[ cur_line.blockstart ]; \
		while( TB.start < TB.end && text[ TB.start ] == ' ' ){ \
			TB.pxwidth -= g_FontRenderer->GetAdvanceX( 0, ' ' ); \
			TB.start++; } \
		if( TB.start == TB.end ){ \
			blocks.erase( cur_line.blockstart ); \
			cur_line.blockend--; } \
		else break; \
	} \
	while( cur_line.blockend > cur_line.blockstart ){ \
		TextBlock& TB = blocks[ cur_line.blockend - 1 ]; \
		while( TB.start < TB.end && text[ TB.end - 1 ] == ' ' ){ \
			TB.pxwidth -= g_FontRenderer->GetAdvanceX( 0, ' ' ); \
			TB.end--; } \
		if( TB.start == TB.end ){ \
			blocks.erase( --cur_line.blockend ); } \
		else break; \
	} \
	if( cur_line.blockend > cur_line.blockstart || (forceht) ) \
	{ \
	/*	LOG << "NEW LINE:"; */ \
		cur_line.pxwidth = 0; \
		cur_line.pxheight = forceht; \
		for( int i = cur_line.blockstart; i < cur_line.blockend; ++i ){ \
			cur_line.pxwidth += blocks[ i ].pxwidth; \
			cur_line.pxheight = TMAX( cur_line.pxheight, blocks[ i ].pxheight ); \
		/*	LOG << "NEW BLOCK["<<__LINE__<<",len="<<blocks[i].pxwidth<<"]:" << text.part(blocks[i].start,blocks[i].end-blocks[i].start); */ \
		} \
		total_height += cur_line.pxheight; \
		if( total_height > height ) \
			return total_height; \
	/*	LOG << "LINE len=" << cur_line.pxwidth; */ \
		lines.push_back( cur_line ); \
		cur_line.blockstart = cur_line.blockend; \
		cur_line.pxwidth = 0; \
		cur_line.pxheight = 0; \
	}
#define SKIP_SPACES \
	prev_chr_val = 0; \
	bool lastadv; \
	while( ( lastadv = IT.Advance() ) && IT.codepoint == ' ' ); \
	if( !lastadv ) \
		break;
	
	UTF8Iterator IT( text );
	if( IT.Advance() == false )
		return total_height;
	for(;;)
	{
		const uint32_t& chr_val = IT.codepoint;
		cur_block.end = IT.m_nextoff;
		
		int char_width = g_FontRenderer->GetAdvanceX( prev_chr_val, chr_val );
		
		if( chr_val == '\n' )
		{
			cur_block.end = IT.offset;
			COMMIT_BLOCK;
			PUSH_BLOCKS_TO_LINE;
			int lht = settingStack.last().CalcLineHeight();
			COMMIT_LINE( lht );
			
			// goto next line after all subsequent spaces
			SKIP_SPACES;
			cur_block.start = cur_block.end = IT.offset;
			cur_word_start = IT.offset;
			cur_word_width = 0;
			continue;
		}
		if( chr_val == ' ' )
		{
			cur_word_width = 0;
			cur_word_start = IT.m_nextoff;
		}
		
		if( parse && chr_val == '#' )
		{
			size_t off = IT.offset;
			size_t noff = IT.m_nextoff;
			if( IT.Advance() )
			{
				if( chr_val == '{' )
				{
					if( !IT.Advance() )
						break;
					COMMIT_BLOCK_BEFCH( off );
					SGRX_TextSettings ts = settingStack.last();
					settingStack.push_back( ts );
					continue;
				}
				else if( chr_val == '}' )
				{
					if( !IT.Advance() )
						break;
					COMMIT_BLOCK_BEFCH( off );
					if( settingStack.size() > 1 )
					{
						settingStack.pop_back();
						GR2D_SetFontSettings( &settingStack.last() );
					}
					continue;
				}
				else if( chr_val == 'f' )
				{
					IT.Advance();
					if( IT.codepoint == '(' )
					{
						StringView cmd = IT.ReadUntilEndOr( ")" );
						IT.Advance();
						if( IT.codepoint == ')' )
						{
							IT.Advance();
							COMMIT_BLOCK_BEFCH( off );
							settingStack.last().font = cmd;
							GR2D_SetFontSettings( &settingStack.last() );
							continue;
						}
					}
				}
				else if( chr_val == 's' )
				{
					IT.Advance();
					if( IT.codepoint == '(' )
					{
						StringView cmd = IT.ReadUntilEndOr( ")" );
						IT.Advance();
						if( IT.codepoint == ')' )
						{
							IT.Advance();
							COMMIT_BLOCK_BEFCH( off );
							settingStack.last().size = String_ParseInt( cmd );
							GR2D_SetFontSettings( &settingStack.last() );
							continue;
						}
					}
				}
				else if( chr_val == 'l' )
				{
					IT.Advance();
					if( IT.codepoint == '(' )
					{
						StringView cmd = IT.ReadUntilEndOr( ")" );
						IT.Advance();
						if( IT.codepoint == ')' )
						{
							IT.Advance();
							COMMIT_BLOCK_BEFCH( off );
							settingStack.last().lineheight = String_ParseFloat( cmd );
							GR2D_SetFontSettings( &settingStack.last() );
							continue;
						}
					}
				}
				else if( chr_val == 'c' )
				{
					IT.Advance();
					if( IT.codepoint == '(' )
					{
						StringView cmd = IT.ReadUntilEndOr( ")" );
						IT.Advance();
						if( IT.codepoint == ')' )
						{
							IT.Advance();
							COMMIT_BLOCK_BEFCH( off );
							bool suc = false;
							Vec4 col = String_ParseVec4( cmd, &suc, "," );
							if( !suc )
								col.w = 1;
							col *= V4( V3( 1.0f / 255.0f ), 1.0f );
							settingStack.last().color = Vec4ToCol32( col );
							continue;
						}
					}
				}
				// otherwise just print the character
			}
			IT.SetOffset( noff );
		}
		
		if( chr_val != ' ' )
			cur_word_width += char_width;
		cur_block.pxwidth += char_width;
		prev_chr_val = chr_val;
		if( cur_word_width > width || cur_line.pxwidth + LAST_BLOCKS_WIDTH <= width )
		{
			// still within line
		//	printf("br1 at=%d chr=%c lw=%d bw=%d\n",IT.offset,char(chr_val),cur_line.pxwidth,cur_block.pxwidth);
			cur_block.pxheight = TMAX( cur_block.pxheight, (int) settingStack.last().CalcLineHeight() );
		}
		else // last word makes the line exceed rectangle width
		{
			// fork block for new word
			TextBlock newblock = cur_block;
			cur_block.end = cur_word_start;
			cur_block.pxwidth -= cur_word_width;
			newblock.start = cur_word_start;
			newblock.pxwidth = cur_word_width;
			
			COMMIT_BLOCK;
			PUSH_BLOCKS_TO_LINE;
			COMMIT_LINE( 0 );
			cur_block = newblock;
		//	printf("br2 at=%d chr=%c\n",IT.offset,char(chr_val));
		}
		
		if( IT.Advance() == false )
			break;
	}
	
	COMMIT_BLOCK;
	PUSH_BLOCKS_TO_LINE;
	COMMIT_LINE( 0 );
	return total_height;
}

static Array< SGRX_TextSettings > g_settingStack;
static Array< TextBlock > blocks;
static Array< TextLine > lines;
int GR2D_DrawTextRect( int x0, int y0, int x1, int y1,
	const StringView& text, int halign, int valign, bool parse )
{
	LOG_FUNCTION;
	
	// sizing
	int width = x1 - x0;
	int height = y1 - y0;
	
	SGRX_FontSettings fsBackup;
	GR2D_GetFontSettings( &fsBackup );
	
	g_settingStack.clear();
	blocks.clear();
	lines.clear();
	int textheight = _GR2D_CalcTextLayout(
		g_settingStack, blocks, lines, text, width, height, parse );
	
	int vspace = height - textheight;
	int y = y0;
	if( valign == VALIGN_CENTER ) y += vspace / 2;
	else if( valign == VALIGN_BOTTOM ) y += vspace;
	
	for( size_t i = 0; i < lines.size(); ++i )
	{
		const TextLine& LN = lines[ i ];
		int hspace = width - LN.pxwidth;
		int x = x0;
		if( halign == HALIGN_CENTER ) x += hspace / 2;
		else if( halign & HALIGN_RIGHT ) x += hspace;
		
		GR2D_SetTextCursor( x, y );
		for( int j = LN.blockstart; j < LN.blockend; ++j )
		{
			const TextBlock& BLK = blocks[ j ];
			GR2D_SetFontSettings( &BLK.settings );
			g_BatchRenderer->Colu( BLK.settings.color );
			StringView textpart = text.part( BLK.start, BLK.end - BLK.start );
			GR2D_DrawTextLine( textpart );
		}
		y += LN.pxheight;
	}
	
	GR2D_SetFontSettings( &fsBackup );
	return textheight;
}

int GR2D_GetTextRectHeight( int x0, int y0, int x1, int y1,
	const StringView& text, bool parse )
{
	LOG_FUNCTION;
	
	// sizing
	int width = x1 - x0;
	int height = y1 - y0;
	
	SGRX_FontSettings fsBackup;
	GR2D_GetFontSettings( &fsBackup );
	
	g_settingStack.clear();
	blocks.clear();
	lines.clear();
	int textheight = _GR2D_CalcTextLayout(
		g_settingStack, blocks, lines, text, width, height, parse );
	
	GR2D_SetFontSettings( &fsBackup );
	return textheight;
}


void SGRX_INT_InitBatchRendering()
{
	g_BatchRenderer = new BatchRenderer( g_Renderer );
	if( VERBOSE ) LOG << LOG_DATE << "  Created batch renderer";
	
	sgrx_int_InitializeFontRendering();
	g_FontRenderer = new FontRenderer();
	if( VERBOSE ) LOG << LOG_DATE << "  Created font renderer";
}

void SGRX_INT_DestroyBatchRendering()
{
	delete g_FontRenderer;
	g_FontRenderer = NULL;
	sgrx_int_FreeFontRendering();
	
	delete g_BatchRenderer;
	g_BatchRenderer = NULL;
}


int DBG_GetFontTextures( Array< TextureHandle >& outTex )
{
	outTex.resize( g_FontRenderer->m_cache.m_pages.size() );
	for( size_t i = 0; i < g_FontRenderer->m_cache.m_pages.size(); ++i )
	{
		outTex[ i ] = g_FontRenderer->m_cache.m_pages[ i ].texture;
	}
	return g_FontRenderer->m_cache.m_keyNodeMap.size();
}


