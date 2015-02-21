

#pragma once
#ifdef INCLUDE_REAL_SDL
#  include <SDL2/SDL.h>
#  include <SDL2/SDL_syswm.h>
#elif defined NO_SDL
struct SDL_Event;
#else
#  include "sdl_events_min.h"
#endif
#include "utils.hpp"


struct Loggable {};
struct EXPORT SGRX_Log
{
	struct Separator
	{
		Separator( const char* s ) : sep( s ){}
		const char* sep;
	};
	enum EMod_Partial { Mod_Partial };
	enum ESpec_Date { Spec_Date };
	
	bool end_newline;
	bool need_sep;
	const char* sep;
	
	SGRX_Log();
	~SGRX_Log();
	void prelog();
	
	SGRX_Log& operator << ( const Separator& );
	SGRX_Log& operator << ( EMod_Partial );
	SGRX_Log& operator << ( ESpec_Date );
	SGRX_Log& operator << ( bool );
	SGRX_Log& operator << ( int8_t );
	SGRX_Log& operator << ( uint8_t );
	SGRX_Log& operator << ( int16_t );
	SGRX_Log& operator << ( uint16_t );
	SGRX_Log& operator << ( int32_t );
	SGRX_Log& operator << ( uint32_t );
	SGRX_Log& operator << ( int64_t );
	SGRX_Log& operator << ( uint64_t );
	SGRX_Log& operator << ( float );
	SGRX_Log& operator << ( double );
	SGRX_Log& operator << ( const void* );
	SGRX_Log& operator << ( const char* );
	SGRX_Log& operator << ( const StringView& );
	SGRX_Log& operator << ( const String& );
	SGRX_Log& operator << ( const Vec2& );
	SGRX_Log& operator << ( const Vec3& );
	SGRX_Log& operator << ( const Mat4& );
	
	SGRX_Log& operator << ( const struct SGRX_Camera& );
};
#define LOG SGRX_Log()
#define LOG_ERROR SGRX_Log() << "ERROR: "
#define LOG_WARNING SGRX_Log() << "WARNING: "
#define PARTIAL_LOG SGRX_Log::Mod_Partial
#define LOG_DATE SGRX_Log::Spec_Date
#define LOG_SEP( x ) SGRX_Log::Separator( x )


EXPORT uint32_t GetTimeMsec();

EXPORT bool Window_HasClipboardText();
EXPORT bool Window_GetClipboardText( String& out );
EXPORT bool Window_SetClipboardText( const StringView& text );

typedef SDL_Event Event;


struct EXPORT Command
{
	Command( const StringView& sv, float thr = 0.1f ) :
		name(sv),
		threshold(thr),
		value(0), prev_value(0),
		state(false), prev_state(false)
	{}
	virtual ~Command(){}
	virtual void OnChangeState(){}
	
	FINLINE bool IsPressed() const { return state && !prev_state; }
	FINLINE bool IsReleased() const { return !state && prev_state; }
	
	void _SetState( float x );
	void _Advance();
	
	StringView name;
	float threshold;
	float value, prev_value;
	bool state, prev_state;
};

struct EXPORT Command_Func : Command
{
	virtual void Function() = 0;
	void OnChangeState(){ if( state && !state ) Function(); }
};

EXPORT void Game_RegisterAction( Command* cmd );
EXPORT void Game_UnregisterAction( Command* cmd );
EXPORT void Game_BindKeyToAction( uint32_t key, Command* cmd );
EXPORT void Game_BindKeyToAction( uint32_t key, const StringView& cmd );
EXPORT Vec2 Game_GetCursorPos();


struct EXPORT IScreen
{
	virtual ~IScreen(){}
	virtual void OnStart(){}
	virtual void OnEnd(){}
	virtual bool OnEvent( const Event& e ) = 0; // return value - whether event is inhibited
	virtual bool Draw( float delta ) = 0; // return value - whether to remove
};

EXPORT bool Game_HasOverlayScreen( IScreen* screen );
EXPORT void Game_AddOverlayScreen( IScreen* screen );
EXPORT void Game_RemoveOverlayScreen( IScreen* screen );

#define TEXFLAGS_SRGB    0x01
#define TEXFLAGS_HASMIPS 0x02
#define TEXFLAGS_LERP_X  0x04
#define TEXFLAGS_LERP_Y  0x08
#define TEXFLAGS_CLAMP_X 0x10
#define TEXFLAGS_CLAMP_Y 0x20

struct EXPORT IGame
{
	virtual void OnConfigure( int argc, char** argv ){}
	virtual void OnInitialize(){}
	virtual void OnDestroy(){}
	virtual void OnEvent( const Event& e ){}
	virtual void OnTick( float dt, uint32_t gametime ) = 0;
	
	virtual bool OnLoadTexture( const StringView& key, ByteArray& outdata, uint32_t& outusageflags );
	virtual bool OnLoadShader( const StringView& type, const StringView& key, String& outdata );
	virtual bool OnLoadShaderFile( const StringView& type, const StringView& path, String& outdata );
	virtual bool ParseShaderIncludes( const StringView& type, const StringView& path, String& outdata );
	virtual bool OnLoadMesh( const StringView& key, ByteArray& outdata );
};


//
// RENDERER DATA
//

struct EXPORT RenderSettings
{
	int width;
	int height;
	bool fullscreen;
	bool windowed_fullscreen;
	bool vsync;
};

struct EXPORT RenderStats
{
	void Reset();
	
	uint32_t numVisMeshes;
	uint32_t numVisPLights; // point lights
	uint32_t numVisSLights; // spotlights
	uint32_t numDrawCalls;
	uint32_t numSDrawCalls; // shadow
	uint32_t numMDrawCalls; // main
	uint32_t numPDrawCalls; // post-processing
};

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

#define RT_FORMAT_BACKBUFFER 0x10000
#define RT_FORMAT_DEPTH      0x10001

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

struct EXPORT SGRX_ITexture
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	virtual ~SGRX_ITexture();
	virtual bool UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h ) = 0;
	
	TextureInfo m_info;
	bool m_isRenderTexture;
	int32_t m_refcount;
	String m_key;
};

struct EXPORT TextureHandle : Handle< SGRX_ITexture >
{
	TextureHandle() : Handle(){}
	TextureHandle( const TextureHandle& h ) : Handle( h ){}
	TextureHandle( SGRX_ITexture* tex ) : Handle( tex ){}
	
	const TextureInfo& GetInfo() const;
	bool UploadRGBA8Part( void* data, int mip = 0, int w = -1, int h = -1, int x = 0, int y = 0 );
};

struct EXPORT SGRX_IShader
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	virtual ~SGRX_IShader();
	
	int32_t m_refcount;
	String m_key;
};

struct EXPORT ShaderHandle : Handle< SGRX_IShader >
{
	ShaderHandle() : Handle(){}
	ShaderHandle( const ShaderHandle& h ) : Handle( h ){}
	ShaderHandle( SGRX_IShader* shdr ) : Handle( shdr ){}
};

#define VDECL_MAX_ITEMS 10

#define VDECLTYPE_FLOAT1 0
#define VDECLTYPE_FLOAT2 1
#define VDECLTYPE_FLOAT3 2
#define VDECLTYPE_FLOAT4 3
#define VDECLTYPE_BCOL4 4

/* usage type | expected data type */
#define VDECLUSAGE_POSITION 0 /* float3 */
#define VDECLUSAGE_COLOR    1 /* float3/float4/bcol4 */
#define VDECLUSAGE_NORMAL   2 /* float3 */
#define VDECLUSAGE_TANGENT  3 /* float4 */
#define VDECLUSAGE_BLENDWT  4 /* preferably bcol4 */
#define VDECLUSAGE_BLENDIDX 5 /* preferably bcol4 (will be passed as ubyte4, not float4) */
#define VDECLUSAGE_TEXTURE0 6 /* any .. */
#define VDECLUSAGE_TEXTURE1 7
#define VDECLUSAGE_TEXTURE2 8
#define VDECLUSAGE_TEXTURE3 9

#define SHADER_NAME_LENGTH 64
#define MAX_NUM_PASSES     16
#define MAX_MI_TEXTURES    4
#define MAX_MI_CONSTANTS   16

struct VDeclInfo
{
	uint8_t offsets[ VDECL_MAX_ITEMS ];
	uint8_t types  [ VDECL_MAX_ITEMS ];
	uint8_t usages [ VDECL_MAX_ITEMS ];
	uint8_t count;
	uint8_t size;
};

struct EXPORT SGRX_IVertexDecl
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	virtual ~SGRX_IVertexDecl();
	
	VDeclInfo m_info;
	int32_t m_refcount;
	String m_text;
};

struct EXPORT VertexDeclHandle : Handle< struct SGRX_IVertexDecl >
{
	VertexDeclHandle() : Handle(){}
	VertexDeclHandle( const VertexDeclHandle& h ) : Handle( h ){}
	VertexDeclHandle( struct SGRX_IVertexDecl* vd ) : Handle( vd ){}
	
	const VDeclInfo& GetInfo();
};

struct SGRX_Mesh;
struct SGRX_MeshInstance;
struct SGRX_Light;
struct SGRX_Scene;

#define INDEX_16 0
#define INDEX_32 1

/* mesh data flags */
#define MDF_INDEX_32      0x01
#define MDF_TRIANGLESTRIP 0x02
#define MDF_DYNAMIC       0x04 /* dynamic buffer updating */
#define MDF_TRANSPARENT   0x10 /* mesh is required to be rendered transparent */
#define MDF_UNLIT         0x20 /* mesh doesn't require the lighting passes to be applied */
#define MDF_NOCULL        0x40 /* mesh has culling disabled */
#define MDF_SKINNED       0x80 /* mesh has bone data (name, offset, parent id) */

#define MDF__PUBFLAGMASK (0x01|0x02|0x10|0x20|0x40|0x80)
#define MDF__PUBFLAGBASE  0

#define NUM_MATERIAL_TEXTURES 8
#define MAX_MESH_PARTS 16
#define MAX_MESH_BONES 32

struct SGRX_MeshPart
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	
	ShaderHandle shaders[ MAX_NUM_PASSES ];
	ShaderHandle shaders_skin[ MAX_NUM_PASSES ];
	TextureHandle textures[ NUM_MATERIAL_TEXTURES ];
	char shader_name[ SHADER_NAME_LENGTH ];
};

struct SGRX_MeshBone
{
	SGRX_MeshBone() : parent_id(-1){}
	
	String name;
	Mat4 skinOffset;
	Mat4 boneOffset;
	Mat4 invSkinOffset;
	int parent_id;
};

struct EXPORT SGRX_IMesh
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	SGRX_IMesh();
	virtual ~SGRX_IMesh();
	virtual bool SetVertexData( const void* data, size_t size, VertexDeclHandle vd, bool tristrip ) = 0;
	virtual bool SetIndexData( const void* data, size_t size, bool i32 ) = 0;
	virtual bool InitVertexBuffer( size_t size ) = 0;
	virtual bool InitIndexBuffer( size_t size, bool i32 ) = 0;
	virtual bool UpdateVertexData( const void* data, size_t size, VertexDeclHandle vd, bool tristrip ) = 0;
	virtual bool UpdateIndexData( const void* data, size_t size ) = 0;
	
	bool SetPartData( SGRX_MeshPart* parts, int count );
	bool SetBoneData( SGRX_MeshBone* bones, int count );
	bool RecalcBoneMatrices();
	bool SetAABBFromVertexData( const void* data, size_t size, VertexDeclHandle vd );
	
	/* rendering info */
	uint32_t m_dataFlags;
	VertexDeclHandle m_vertexDecl;
	uint32_t m_vertexCount;
	uint32_t m_vertexDataSize;
	uint32_t m_indexCount;
	uint32_t m_indexDataSize;
	SGRX_MeshPart m_parts[ MAX_MESH_PARTS ];
	SGRX_MeshBone m_bones[ MAX_MESH_BONES ];
	int m_numParts;
	int m_numBones;
	
	/* collision detection */
	Vec3 m_boundsMin;
	Vec3 m_boundsMax;
	
	String m_key;
	int32_t m_refcount;
};

struct MeshHandle : Handle< SGRX_IMesh >
{
	MeshHandle() : Handle(){}
	MeshHandle( const MeshHandle& h ) : Handle( h ){}
	MeshHandle( struct SGRX_IMesh* mesh ) : Handle( mesh ){}
};

struct SGRX_MeshInstLight
{
	SGRX_MeshInstance* MI;
	SGRX_Light* L;
};

#define LIGHT_POINT  1
#define LIGHT_SPOT   2

struct EXPORT SGRX_Light
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	
	SGRX_Light( SGRX_Scene* s );
	~SGRX_Light();
	void RecalcMatrices();
	
	SGRX_Scene* _scene;
	
	int type;
	bool enabled;
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
	bool hasShadows;
	
	/* frame cache */
	SGRX_MeshInstLight* _mibuf_begin;
	SGRX_MeshInstLight* _mibuf_end;
	
	int32_t _refcount;
};

struct EXPORT LightHandle : Handle< SGRX_Light >
{
	LightHandle() : Handle(){}
	LightHandle( const LightHandle& h ) : Handle( h ){}
	LightHandle( SGRX_Light* lt ) : Handle( lt ){}
};



struct EXPORT SGRX_CullSceneFrustum
{
	Vec3 position;
	Vec3 direction;
	Vec3 up;
	float hangle;
	float vangle;
	float znear;
	float zfar;
};

struct EXPORT SGRX_CullSceneCamera
{
	SGRX_CullSceneFrustum frustum;
	Mat4 viewProjMatrix;
	Mat4 invViewProjMatrix;
};

struct EXPORT SGRX_CullScenePointLight
{
	Vec3 position;
	float radius;
};

struct EXPORT SGRX_CullSceneMesh
{
	Mat4 transform;
	Vec3 min, max;
};

struct EXPORT SGRX_CullScene
{
	virtual ~SGRX_CullScene();
	virtual void Camera_Prepare( SGRX_CullSceneCamera* camera ){}
	virtual bool Camera_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield ){ return false; }
	virtual bool Camera_PointLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullScenePointLight* lights, uint32_t* outbitfield ){ return false; }
	virtual bool Camera_SpotLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneFrustum* frusta, Mat4* inv_matrices, uint32_t* outbitfield ){ return false; }
	virtual void SpotLight_Prepare( SGRX_CullSceneCamera* camera ){ Camera_Prepare( camera ); }
	virtual bool SpotLight_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield ){ return Camera_MeshList( count, camera, meshes, outbitfield ); }
};

struct EXPORT SGRX_DefaultCullScene
{
	virtual bool Camera_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield );
	virtual bool Camera_PointLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullScenePointLight* lights, uint32_t* outbitfield );
	virtual bool Camera_SpotLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneFrustum* frusta, Mat4* inv_matrices, uint32_t* outbitfield );
	
	Array< Vec3 > m_aabbCache;
};


struct EXPORT SGRX_MeshInstance
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	
	SGRX_MeshInstance( SGRX_Scene* s );
	~SGRX_MeshInstance();
	
	SGRX_Scene* _scene;
	
	MeshHandle mesh;
	Mat4 matrix;
	Vec4 color;
	uint32_t enabled : 1;
	uint32_t cpuskin : 1; /* TODO */
	
	TextureHandle textures[ MAX_MI_TEXTURES ];
	Vec4 constants[ MAX_MI_CONSTANTS ];
	
	Array< Mat4 > skin_matrices;
	
	/* frame cache */
	SGRX_MeshInstLight* _lightbuf_begin;
	SGRX_MeshInstLight* _lightbuf_end;
	
	int32_t _refcount;
};

struct EXPORT MeshInstHandle : Handle< SGRX_MeshInstance >
{
	MeshInstHandle() : Handle(){}
	MeshInstHandle( const MeshInstHandle& h ) : Handle( h ){}
	MeshInstHandle( SGRX_MeshInstance* mi ) : Handle( mi ){}
};

struct EXPORT SGRX_Camera : Loggable
{
	void Log( SGRX_Log& elog );
	void UpdateViewMatrix();
	void UpdateProjMatrix();
	void UpdateMatrices();
	
	Vec3 WorldToScreen( const Vec3& pos );
	bool GetCursorRay( float x, float y, Vec3& pos, Vec3& dir );
	
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
SGRX_Log& operator << ( SGRX_Log& log, const SGRX_Camera& cam );

struct SGRX_Viewport
{
	int x1, y1, x2, y2;
};

struct EXPORT SGRX_Scene
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	SGRX_Scene();
	~SGRX_Scene();
	MeshInstHandle CreateMeshInstance();
//	bool RemoveMeshInstance( MeshInstHandle mih );
	LightHandle CreateLight();
//	bool RemoveLight( LightHandle lh );
	
	HashTable< SGRX_MeshInstance*, MeshInstHandle > m_meshInstances;
	HashTable< SGRX_Light*, LightHandle > m_lights;
	
	SGRX_CullScene* cullScene;
	SGRX_Camera camera;
	
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
	
	int32_t m_refcount;
};

struct EXPORT SceneHandle : Handle< SGRX_Scene >
{
	SceneHandle() : Handle(){}
	SceneHandle( const SceneHandle& h ) : Handle( h ){}
	SceneHandle( struct SGRX_Scene* sc ) : Handle( sc ){}
};

/* render pass constants */
#define RPT_OBJECT     1
#define RPT_SCREEN     2
#define RPT_SHADOWS    3

#define RPF_OBJ_STATIC      0x01
#define RPF_OBJ_DYNAMIC     0x02
#define RPF_OBJ_ALL        (RPF_OBJ_STATIC|RPF_OBJ_DYNAMIC)
#define RPF_MTL_SOLID       0x04
#define RPF_MTL_TRANSPARENT 0x08
#define RPF_MTL_ALL        (RPF_MTL_SOLID|RPF_MTL_TRANSPARENT)
#define RPF_CALC_DIRAMB     0x10
#define RPF_LIGHTOVERLAY    0x20
#define RPF_ENABLED         0x80

struct SGRX_RenderPass
{
	uint8_t type;
	uint8_t flags;
	int16_t maxruns;
	uint16_t pointlight_count;
	uint8_t spotlight_count;
//	uint8_t num_inst_textures;
	StringView shader_name;
	
	// cache
	ShaderHandle _shader;
	
};

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

struct EXPORT BatchRenderer
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
	FINLINE BatchRenderer& Colu( uint32_t c ){ return Colb( COLOR_EXTRACT_R( c ), COLOR_EXTRACT_G( c ), COLOR_EXTRACT_B( c ), COLOR_EXTRACT_A( c ) ); }
	FINLINE BatchRenderer& Tex( float x, float y ){ m_proto.u = x; m_proto.v = y; return *this; }
	FINLINE BatchRenderer& Pos( float x, float y, float z = 0.0f ){ m_proto.x = x; m_proto.y = y; m_proto.z = z; AddVertex( m_proto ); return *this; }
	FINLINE BatchRenderer& Pos( const Vec3& pos ){ return Pos( pos.x, pos.y, pos.z ); }
	
	BatchRenderer& Prev( int i );
	BatchRenderer& Quad( float x0, float y0, float x1, float y1 );
	FINLINE BatchRenderer& QuadWH( float x, float y, float w, float h ){ return Quad( x, y, x + w, y + h ); }
	FINLINE BatchRenderer& Box( float x, float y, float w, float h ){ w *= 0.5f; h *= 0.5f; return Quad( x - w, y - h, x + w, y + h ); }
	BatchRenderer& TurnedBox( float x, float y, float dx, float dy );
	BatchRenderer& CircleFill( float x, float y, float r, int verts = -1 );
	BatchRenderer& CircleOutline( float x, float y, float r, int verts = -1 );
	
	BatchRenderer& SetPrimitiveType( EPrimitiveType pt );
	bool CheckSetTexture( const TextureHandle& tex );
	BatchRenderer& SetTexture( const TextureHandle& tex );
	BatchRenderer& UnsetTexture(){ return SetTexture( NULL ); }
	BatchRenderer& Flush();
	
	IRenderer* m_renderer;
	TextureHandle m_texture;
	EPrimitiveType m_primType;
	Vertex m_proto;
	bool m_swapRB;
	Array< Vertex > m_verts;
};

struct EXPORT SGRX_DebugDraw
{
	virtual void DebugDraw() = 0;
	void _OnEnd();
};


EXPORT int GR_GetWidth();
EXPORT int GR_GetHeight();

EXPORT TextureHandle GR_CreateTexture( int width, int height, int format, int mips = 1 );
EXPORT TextureHandle GR_GetTexture( const StringView& path );
EXPORT TextureHandle GR_CreateRenderTexture( int width, int height, int format );
EXPORT ShaderHandle GR_GetShader( const StringView& path );
EXPORT VertexDeclHandle GR_GetVertexDecl( const StringView& vdecl );
EXPORT MeshHandle GR_CreateMesh();
EXPORT MeshHandle GR_GetMesh( const StringView& path );

EXPORT SceneHandle GR_CreateScene();
EXPORT bool GR_SetRenderPasses( SGRX_RenderPass* passes, int count );
EXPORT void GR_RenderScene( SceneHandle sh, bool enablePostProcessing = true, SGRX_Viewport* viewport = NULL, SGRX_DebugDraw* debugDraw = NULL );
EXPORT RenderStats& GR_GetRenderStats();

EXPORT void GR2D_SetWorldMatrix( const Mat4& mtx );
EXPORT void GR2D_SetViewMatrix( const Mat4& mtx );
EXPORT bool GR2D_SetFont( const StringView& name, int pxsize );
EXPORT void GR2D_SetColor( float r, float g, float b, float a = 1.0f );
inline void GR2D_SetColor( float x, float a ){ GR2D_SetColor( x, x, x, a ); }
inline void GR2D_SetColor( float x ){ GR2D_SetColor( x, x, x, x ); }
EXPORT void GR2D_SetTextCursor( float x, float y );
EXPORT Vec2 GR2D_GetTextCursor();
EXPORT int GR2D_GetTextLength( const StringView& text );
EXPORT int GR2D_DrawTextLine( const StringView& text );
EXPORT int GR2D_DrawTextLine( float x, float y, const StringView& text );
#define HALIGN_LEFT 0
#define HALIGN_CENTER 1
#define HALIGN_RIGHT 2
#define VALIGN_TOP 0
#define VALIGN_CENTER 1
#define VALIGN_BOTTOM 2
EXPORT int GR2D_DrawTextLine( float x, float y, const StringView& text, int halign, int valign );

EXPORT BatchRenderer& GR2D_GetBatchRenderer();


extern "C" EXPORT int SGRX_EntryPoint( int argc, char** argv, int debug );

