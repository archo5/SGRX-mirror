

#pragma once
#include "engine.hpp"


///
/// TEXTURE
///

EXPORT size_t TextureInfo_GetTextureSideSize( const TextureInfo* TI );
EXPORT void TextureInfo_GetCopyDims( const TextureInfo* TI, size_t* outcopyrowsize, size_t* outcopyrowcount );
EXPORT bool TextureInfo_GetMipInfo( const TextureInfo* TI, int mip, TextureInfo* outinfo );

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
	virtual bool UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h ) = 0;
};


///
/// RENDERER
///

struct RendererInfo
{
	bool swapRB;
};

struct EXPORT IRenderer
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

