

#pragma once
#include "engine.hpp"


///
/// TEXTURE
///

#define TEXTYPE_2D     1 /* 1 side, width x height */
#define TEXTYPE_CUBE   2 /* 6 sides, width x width */
#define TEXTYPE_VOLUME 3 /* 1 side, width x height x depth */

#define TEXFORMAT_UNKNOWN 0
#define TEXFORMAT_RGBA8  1
#define TEXFORMAT_BGRA8  2
#define TEXFORMAT_BGRX8  3
#define TEXFORMAT_R5G6B5 5
#define TEXFORMAT_DXT1   11
#define TEXFORMAT_DXT3   13
#define TEXFORMAT_DXT5   15
#define TEXFORMAT_ISBLOCK4FORMAT( x ) ((x)==TEXFORMAT_DXT1||(x)==TEXFORMAT_DXT3||(x)==TEXFORMAT_DXT5)

struct TextureInfo
{
	uint32_t flags; /* TEXFLAGS */
	int type; /* TEXTYPE */
	int width;
	int height;
	int depth;
	int format; /* TEXFORMAT */
	int mipcount;
};

EXPORT size_t TextureInfo_GetTextureSideSize( TextureInfo* TI );
EXPORT void TextureInfo_GetCopyDims( TextureInfo* TI, size_t* outcopyrowsize, size_t* outcopyrowcount );
EXPORT bool TextureInfo_GetMipInfo( TextureInfo* TI, int mip, TextureInfo* outinfo );

struct TextureData
{
	TextureInfo info;
	ByteArray data;
};

EXPORT bool TextureData_Load( TextureData* TD, ByteArray& texdata, const StringView& filename = "<memory>" );
EXPORT void TextureData_Free( TextureData* TD );
EXPORT size_t TextureData_GetMipDataOffset( TextureInfo* texinfo, void* data, int side, int mip );
EXPORT size_t TextureData_GetMipDataSize( TextureInfo* texinfo, int mip );

struct EXPORT ITexture
{
	TextureInfo m_info;
	
	virtual void Destroy() = 0;
	virtual void SetData( int mip, void* data ){ /* TODO */ }
};


///
/// BASIC RENDERING
///

enum EPrimitiveType
{
	PT_None,
	PT_Points,
	PT_Lines,
	PT_LineStrip,
	PT_Triangles,
	PT_TriangleStrip,
	PT_TriangleFan,
};

struct BatchRenderer
{
	struct Vertex
	{
		float x, y, z;
		uint32_t color;
		float u, v;
	};
	
	BatchRenderer( struct IRenderer* r );
	~BatchRenderer(){ if( m_renderer ) Flush(); }
	
	BatchRenderer& AddVertices( Vertex* verts, int count );
	BatchRenderer& AddVertex( const Vertex& vert );
	FINLINE BatchRenderer& Col( float x ){ return Col( x, x, x, x ); }
	FINLINE BatchRenderer& Col( float x, float a ){ return Col( x, x, x, a ); }
	FINLINE BatchRenderer& Col( float r, float g, float b ){ return Col( r, g, b, 1.0f ); }
	FINLINE BatchRenderer& Col( float r, float g, float b, float a ){ return Colb( r * 255, g * 255, b * 255, a * 255 ); }
	BatchRenderer& Colb( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	FINLINE BatchRenderer& Tex( float x, float y ){ m_proto.u = x; m_proto.v = y; return *this; }
	FINLINE BatchRenderer& Pos( float x, float y, float z = 0.0f ){ m_proto.x = x; m_proto.y = y; m_proto.z = z; AddVertex( m_proto ); return *this; }
	
	BatchRenderer& SetPrimitiveType( EPrimitiveType pt );
	BatchRenderer& SetTexture( SGRX_Texture* tex );
	BatchRenderer& Flush();
	
	IRenderer* m_renderer;
	SGRX_Texture* m_texture;
	EPrimitiveType m_primType;
	Vertex m_proto;
	bool m_swapRB;
	Array< Vertex > m_verts;
};


///
/// RENDERER
///

struct RendererInfo
{
	bool swapRB;
};

struct IRenderer
{
	virtual void Destroy() = 0;
	virtual const RendererInfo& GetInfo() = 0;
	
	virtual void Swap() = 0;
	virtual void Modify( const RenderSettings& settings ) = 0;
	virtual void SetCurrent() = 0;
	virtual void Clear( float* color_v4f, bool clear_zbuffer = true ) = 0;
	
	virtual ITexture* CreateTexture( TextureInfo* texinfo, void* data = NULL ) = 0;
	
	virtual void DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt, ITexture* tex ) = 0;
	
//	void set_current();
//	void poke_resource( sgs_VarObj*, int ); /* !=0 => add, otherwise remove */
//	void swap();
//	void clear( float* );
//	void set_render_state( int, int, int, int, int ); /* type, arg0, arg1, arg2, arg3 */
//	void set_matrix( int, float* ); /* type, float[16] */
//	void set_rt( SS_Texture* tex );
//	
//	int create_texture_argb8( SS_Texture*, SS_Image*, uint32_t );
//	int create_texture_a8( SS_Texture*, uint8_t*, int, int, int ); /* data, width, height, pitch */
//	int create_texture_rnd( SS_Texture*, int, int, uint32_t ); /* width, height, flags */
//	int destroy_texture( SS_Texture* );
//	int apply_texture( SS_Texture* );
//	
//	int init_vertex_format( SS_VertexFormat* );
//	int free_vertex_format( SS_VertexFormat* );
//	int draw_basic_vertices( void*, uint32_t, int ); /* SS_BasicVertex*, count, ptype */
//	int draw_ext( SS_VertexFormat*, void*, uint32_t, void*, uint32_t, int, uint32_t, uint32_t, int );
};
typedef bool (*pfnRndInitialize) ( const char** );
typedef void (*pfnRndFree) ();
typedef IRenderer* (*pfnRndCreateRenderer) ( const RenderSettings&, void* );

