

#pragma once
#include "engine.hpp"


#define SHADER_NAME_LENGTH 64
#define MAX_NUM_PASSES     16
#define MAX_MI_TEXTURES    4
#define MAX_MI_CONSTANTS   16


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

struct EXPORT IShader
{
	virtual void Destroy() = 0;
};

EXPORT const char* VDeclInfo_Parse( VDeclInfo* info, const char* text );
EXPORT int GetAABBFromVertexData( VDeclInfo* info, const char* vdata, size_t vdsize, Vec3& outMin, Vec3& outMax );

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


struct SGRX_Mesh;
struct SGRX_MeshInstance;
struct SGRX_Light;
struct SGRX_Scene;

struct MeshHandle : Handle< SGRX_Mesh >
{
};


struct SGRX_Camera
{
	Vec3 position;
	Vec3 direction;
	Vec3 up;
	float angle;
	float aspect;
	float aamix;
	float znear;
	float zfar;
	
	Mat4 mView;
	Mat4 mProj;
	Mat4 mInvView;
};

struct SGRX_Viewport
{
	int x1, y1, x2, y2;
};

struct SGRX_MeshInstLight
{
	SGRX_MeshInstance* MI;
	SGRX_Light* L;
};

struct SGRX_Light
{
	SGRX_Scene* scene;
	
	int type;
	int enabled;
	Vec3 position;
	Vec3 direction;
	Vec3 updir;
	Vec3 color;
	float range;
	float power;
	float angle;
	float aspect;
	TextureHandle cookieTexture;
	TextureHandle shadowTexture;
	Mat4 viewMatrix;
	Mat4 projMatrix;
	Mat4 viewProjMatrix;
	int hasShadows;
	
	/* frame cache */
	SGRX_MeshInstLight* mibuf_begin;
	SGRX_MeshInstLight* mibuf_end;
};

struct SGRX_Scene
{
	HashTable< SGRX_MeshInstance*, bool > m_meshInstances;
	HashTable< SGRX_Light*, bool > m_lights;
	
//	sgs_VarObj* cullScenes;
	SGRX_Camera* m_camera;
	
	Vec3 fogColor;
	float fogHeightFactor;
	float fogDensity;
	float fogHeightDensity;
	float fogStartHeight;
	float fogMinDist;
	
	Vec3 ambientLightColor;
	Vec3 dirLightColor;
	Vec3 dirLightDir;
	
	TextureHandle skyTexture;
};

struct SGRX_MeshInstance
{
	SGRX_Scene* m_scene;
	
	MeshHandle mesh;
	Mat4 matrix;
	Vec4 color;
	uint32_t enabled : 1;
	uint32_t cpuskin : 1; /* TODO */
	
	TextureHandle textures[ MAX_MI_TEXTURES ];
	Vec4 constants[ MAX_MI_CONSTANTS ];
	
	Array< Mat4 > skin_matrices;
	
	/* frame cache */
	SGRX_MeshInstLight* lightbuf_begin;
	SGRX_MeshInstLight* lightbuf_end;
};

struct SGRX_RenderPass
{
	uint8_t type;
	uint8_t flags;
	int16_t maxruns;
	uint16_t pointlight_count;
	uint8_t spotlight_count;
	uint8_t num_inst_textures;
	char shname[ SHADER_NAME_LENGTH ];
};


///
/// RENDERER
///

struct RendererInfo
{
	bool swapRB;
	bool compileShaders;
	StringView shaderType;
};

struct EXPORT IRenderer
{
	virtual void Destroy() = 0;
	virtual const RendererInfo& GetInfo() = 0;
	
	virtual void Swap() = 0;
	virtual void Modify( const RenderSettings& settings ) = 0;
	virtual void SetCurrent() = 0;
	virtual void Clear( float* color_v4f, bool clear_zbuffer = true ) = 0;
	
	virtual void SetWorldMatrix( const Mat4& mtx ) = 0;
	virtual void SetViewMatrix( const Mat4& mtx ) = 0;
	
	virtual ITexture* CreateTexture( TextureInfo* texinfo, void* data = NULL ) = 0;
	virtual bool CompileShader( const StringView& code, ByteArray& outcomp, String& outerrors ) = 0;
	virtual IShader* CreateShader( ByteArray& code ) = 0; // StringView for uncompiled, byte buffer for compiled shaders
	
	virtual void DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt, ITexture* tex ) = 0;
	
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

