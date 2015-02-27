

#pragma once
#define USE_SERIALIZATION
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

EXPORT const char* VDeclInfo_Parse( VDeclInfo* info, const char* text );
EXPORT int GetAABBFromVertexData( const VDeclInfo& info, const char* vdata, size_t vdsize, Vec3& outMin, Vec3& outMax );

struct MeshFilePartData
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	
	uint8_t materialTextureCount; /* 0 - 8 */
	uint8_t materialStringSizes[ NUM_MATERIAL_TEXTURES + 1 ];
	char* materialStrings[ NUM_MATERIAL_TEXTURES + 1 ];
	/* size w/o padding = 28+[36/72] = 64/100 */
};

struct MeshFileBoneData
{
	char* boneName;
	uint8_t boneNameSize;
	uint8_t parent_id;
	Mat4 boneOffset;
	/* size w/o padding = 66+[4/8] = 70/74 */
};

struct MeshFileData
{
	uint32_t dataFlags;
	uint32_t vertexDataSize;
	uint32_t indexDataSize;
	char* vertexData;
	char* indexData;
	char* formatData;
	
	Vec3 boundsMin;
	Vec3 boundsMax;
	
	uint8_t numParts;
	uint8_t numBones;
	uint8_t formatSize;
	/* size w/o padding = 39+[12/24] = 51/63 */
	
	MeshFilePartData parts[ MAX_MESH_PARTS ];
	/* size w/o padding = 51/63 + 64/100 x16 = 1075/1663 */
	MeshFileBoneData bones[ MAX_MESH_BONES ];
	/* size w/o padding = 1075/1663 + 70/74 x32 = 3315/4031 */
};

const char* MeshData_Parse( char* buf, size_t size, MeshFileData* out );


// ! REQUIRES ByteArray data source to be preserved ! //
struct AnimFileParser
{
	struct Anim
	{
		char* name;
		float speed;
		uint8_t nameSize;
		uint8_t trackCount;
		uint32_t frameCount;
		uint32_t trackDataOff;
	};
	struct Track
	{
		char* name;
		uint8_t nameSize;
		float* dataPtr; // points to 10 * frameCount floats (AoS)
	};
	
	AnimFileParser( ByteArray& data )
	{
		ByteReader br( &data );
		error = Parse( br );
		if( !error && br.error )
			error = "failed to read data";
	}
	
	const char* Parse( ByteReader& br );
	
	const char* error;
	Array< Track > trackData;
	Array< Anim > animData;
};
inline SGRX_Log& operator << ( SGRX_Log& L, const AnimFileParser::Anim& anim )
{
	L << "ANIM";
	L << "\n\tname = " << StringView( anim.name, anim.nameSize );
	L << "\n\tspeed = " << anim.speed;
	L << "\n\ttrackCount = " << anim.trackCount;
	L << "\n\tframeCount = " << anim.frameCount;
	L << "\n\ttrackDataOff = " << anim.trackDataOff;
	return L;
}
inline SGRX_Log& operator << ( SGRX_Log& L, const AnimFileParser::Track& track )
{
	L << "TRACK";
	L << "\n\tname = " << StringView( track.name, track.nameSize );
	L << "\n\tdataPtr = " << track.dataPtr;
	return L;
}


EXPORT void SGRX_Cull_Camera_Prepare( SGRX_Scene* S );
EXPORT uint32_t SGRX_Cull_Camera_MeshList( Array< SGRX_MeshInstance* >& MIBuf, ByteArray& scratchMem, SGRX_Scene* S );
EXPORT uint32_t SGRX_Cull_Camera_PointLightList( Array< SGRX_Light* >& LIBuf, ByteArray& scratchMem, SGRX_Scene* S );
EXPORT uint32_t SGRX_Cull_Camera_SpotLightList( Array< SGRX_Light* >& LIBuf, ByteArray& scratchMem, SGRX_Scene* S );
EXPORT void SGRX_Cull_SpotLight_Prepare( SGRX_Scene* S, SGRX_Light* L );
EXPORT uint32_t SGRX_Cull_SpotLight_MeshList( Array< SGRX_MeshInstance* >& MIBuf, ByteArray& scratchMem, SGRX_Scene* S, SGRX_Light* L );


///
/// RENDERER
///

struct RendererInfo
{
	bool swapRB;
	bool compileShaders;
	StringView shaderType;
};

#define RS_ZENABLE 1

struct EXPORT IRenderer
{
	IRenderer() : m_inDebugDraw( false ){}
	
	virtual void Destroy() = 0;
	virtual const RendererInfo& GetInfo() = 0;
	virtual void LoadInternalResources() = 0;
	virtual void UnloadInternalResources() = 0;
	
	virtual void Swap() = 0;
	virtual void Modify( const RenderSettings& settings ) = 0;
	virtual void SetCurrent() = 0;
	virtual void Clear( float* color_v4f, bool clear_zbuffer = true ) = 0;
	virtual void SetRenderState( int state, uint32_t val ) = 0;
	
	virtual void SetWorldMatrix( const Mat4& mtx ) = 0;
	virtual void SetViewMatrix( const Mat4& mtx ) = 0;
	virtual void SetViewport( int x0, int y0, int x1, int y1 ) = 0;
	virtual void SetScissorRect( bool enable, int* rect ) = 0;
	
	virtual SGRX_ITexture* CreateTexture( TextureInfo* texinfo, void* data = NULL ) = 0;
	virtual SGRX_ITexture* CreateRenderTexture( TextureInfo* texinfo ) = 0;
	virtual bool CompileShader( const StringView& code, ByteArray& outcomp, String& outerrors ) = 0;
	virtual SGRX_IShader* CreateShader( ByteArray& code ) = 0; // StringView for uncompiled, byte buffer for compiled shaders
	virtual SGRX_IVertexDecl* CreateVertexDecl( const VDeclInfo& vdinfo ) = 0;
	virtual SGRX_IMesh* CreateMesh() = 0;
	
	virtual bool SetRenderTarget( TextureHandle rt ) = 0;
	virtual void DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt, SGRX_ITexture* tex, SGRX_IShader* shd, Vec4* shdata, size_t shvcount ) = 0;
	virtual bool SetRenderPasses( SGRX_RenderPass* passes, int count ) = 0;
	virtual void RenderScene( SceneHandle scene, bool enablePostProcessing, SGRX_Viewport* viewport, SGRX_PostDraw* postDraw, SGRX_DebugDraw* debugDraw ) = 0;
	
	RenderStats m_stats;
	RenderSettings m_currSettings;
	Array< SGRX_RenderPass > m_renderPasses;
	
	bool m_inDebugDraw;
	
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

