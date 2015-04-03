

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


ENGINE_EXPORT uint32_t GetTimeMsec();
ENGINE_EXPORT void Thread_Sleep( uint32_t msec );

ENGINE_EXPORT bool Window_HasClipboardText();
ENGINE_EXPORT bool Window_GetClipboardText( String& out );
ENGINE_EXPORT bool Window_SetClipboardText( const StringView& text );

typedef SDL_Event Event;


struct ENGINE_EXPORT Command
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

struct ENGINE_EXPORT Command_Func : Command
{
	virtual void Function() = 0;
	void OnChangeState(){ if( state && !state ) Function(); }
};

#define SGRX_MB_UNKNOWN 0
#define SGRX_MB_LEFT 1
#define SGRX_MB_MIDDLE 2
#define SGRX_MB_RIGHT 3
#define SGRX_MB_X1 4
#define SGRX_MB_X2 5

typedef uint64_t ActionInput;
#define ACTINPUT_UNASSIGNED 0
#define ACTINPUT_KEY 1
#define ACTINPUT_MOUSE 2
#define ACTINPUT_JOYSTICK0 3
#define ACTINPUT_MAKE( type, val ) (((uint64_t)(type)<<32ull)|(val))
#define ACTINPUT_MAKE_KEY( val ) ACTINPUT_MAKE( ACTINPUT_KEY, val )
#define ACTINPUT_MAKE_MOUSE( val ) ACTINPUT_MAKE( ACTINPUT_MOUSE, val )
#define ACTINPUT_GET_TYPE( iid ) (((iid)>>32ull)&0xffffffff)
#define ACTINPUT_GET_VALUE( iid ) ((iid)&0xffffffff)

ENGINE_EXPORT void Game_RegisterAction( Command* cmd );
ENGINE_EXPORT void Game_UnregisterAction( Command* cmd );
ENGINE_EXPORT void Game_BindKeyToAction( uint32_t key, Command* cmd );
ENGINE_EXPORT void Game_BindKeyToAction( uint32_t key, const StringView& cmd );
ENGINE_EXPORT void Game_BindMouseButtonToAction( int btn, Command* cmd );
ENGINE_EXPORT void Game_BindMouseButtonToAction( int btn, const StringView& cmd );
ENGINE_EXPORT ActionInput Game_GetActionBinding( Command* cmd );
ENGINE_EXPORT void Game_BindInputToAction( ActionInput iid, Command* cmd );
ENGINE_EXPORT void Game_UnbindInput( ActionInput iid );
ENGINE_EXPORT StringView Game_GetInputName( ActionInput iid );
ENGINE_EXPORT Vec2 Game_GetCursorPos();
ENGINE_EXPORT void Game_SetCursorPos( int x, int y );
ENGINE_EXPORT void Game_ShowCursor( bool show );


struct ENGINE_EXPORT IScreen
{
	virtual ~IScreen(){}
	virtual void OnStart(){}
	virtual void OnEnd(){}
	virtual bool OnEvent( const Event& e ) = 0; // return value - whether event is inhibited
	virtual bool Draw( float delta ) = 0; // return value - whether to remove
};

ENGINE_EXPORT bool Game_HasOverlayScreens();
ENGINE_EXPORT bool Game_HasOverlayScreen( IScreen* screen );
ENGINE_EXPORT void Game_AddOverlayScreen( IScreen* screen );
ENGINE_EXPORT void Game_RemoveOverlayScreen( IScreen* screen );
ENGINE_EXPORT void Game_RemoveAllOverlayScreens();

ENGINE_EXPORT void Game_End();


#define TEXFLAGS_SRGB    0x01
#define TEXFLAGS_HASMIPS 0x02
#define TEXFLAGS_LERP_X  0x04
#define TEXFLAGS_LERP_Y  0x08
#define TEXFLAGS_CLAMP_X 0x10
#define TEXFLAGS_CLAMP_Y 0x20

struct ENGINE_EXPORT IGame
{
	virtual void OnConfigure( int argc, char** argv ){}
	virtual void OnInitialize(){}
	virtual void OnDestroy(){}
	virtual void OnEvent( const Event& e ){}
	virtual void OnTick( float dt, uint32_t gametime ) = 0;
	
	virtual bool OnLoadTexture( const StringView& key, ByteArray& outdata, uint32_t& outusageflags );
	virtual void GetShaderCacheFilename( const StringView& type, const StringView& key, String& name );
	virtual bool GetCompiledShader( const StringView& type, const StringView& key, ByteArray& outdata );
	virtual bool SetCompiledShader( const StringView& type, const StringView& key, const ByteArray& data );
	virtual bool OnLoadShader( const StringView& type, const StringView& key, String& outdata );
	virtual bool OnLoadShaderFile( const StringView& type, const StringView& path, String& outdata );
	virtual bool ParseShaderIncludes( const StringView& type, const StringView& path, String& outdata );
	virtual bool OnLoadMesh( const StringView& key, ByteArray& outdata );
};


//
// RENDERER DATA
//

#define FULLSCREEN_NONE 0
#define FULLSCREEN_NORMAL 1
#define FULLSCREEN_WINDOWED 2

#define ANTIALIAS_NONE 0
#define ANTIALIAS_MULTISAMPLE 1

struct RenderSettings
{
	int display;
	int width;
	int height;
	int refresh_rate;
	int fullscreen;
	bool vsync;
	int aa_mode;
	int aa_quality;
};

struct DisplayMode
{
	int width;
	int height;
	int refresh_rate;
	
	bool operator == ( const DisplayMode& o ) const { return width == o.width && height == o.height && refresh_rate == o.refresh_rate; }
	void Log( SGRX_Log& L ) const
	{
		L << "Display mode (width: " << width << ", height: " << height << ", refresh rate: " << refresh_rate << ")";
	}
	typedef SGRX_Log::Loggable< DisplayMode > LogT;
};

//inline SGRX_Log& SGRX_Log::operator << ( const DisplayMode& dm )
//{
//	*this << "Display mode (width: " << dm.width << ", height: " << dm.height << ", refresh rate: " << dm.refresh_rate << ")";
//	return *this;
//}


ENGINE_EXPORT bool GR_SetVideoMode( const RenderSettings& rs );
ENGINE_EXPORT void GR_GetVideoMode( RenderSettings& rs );

ENGINE_EXPORT int GR_GetDisplayCount();
ENGINE_EXPORT const char* GR_GetDisplayName( int id );
ENGINE_EXPORT bool GR_ListDisplayModes( int display, Array< DisplayMode >& out );

struct ENGINE_EXPORT RenderStats
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

struct ENGINE_EXPORT SGRX_ITexture
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

struct ENGINE_EXPORT TextureHandle : Handle< SGRX_ITexture >
{
	TextureHandle() : Handle(){}
	TextureHandle( const TextureHandle& h ) : Handle( h ){}
	TextureHandle( SGRX_ITexture* tex ) : Handle( tex ){}
	
	const TextureInfo& GetInfo() const;
	bool UploadRGBA8Part( void* data, int mip = 0, int w = -1, int h = -1, int x = 0, int y = 0 );
};

struct ENGINE_EXPORT SGRX_IShader
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	virtual ~SGRX_IShader();
	
	int32_t m_refcount;
	String m_key;
};

struct ENGINE_EXPORT ShaderHandle : Handle< SGRX_IShader >
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
	
	int GetOffset( int usage )
	{
		for( int i = 0; i < count; ++i )
			if( usages[ i ] == usage )
				return offsets[ i ];
		return -1;
	}
	int GetType( int usage )
	{
		for( int i = 0; i < count; ++i )
			if( usages[ i ] == usage )
				return types[ i ];
		return -1;
	}
};

struct ENGINE_EXPORT SGRX_IVertexDecl
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	virtual ~SGRX_IVertexDecl();
	
	VDeclInfo m_info;
	int32_t m_refcount;
	String m_text;
};

struct ENGINE_EXPORT VertexDeclHandle : Handle< SGRX_IVertexDecl >
{
	VertexDeclHandle() : Handle(){}
	VertexDeclHandle( const VertexDeclHandle& h ) : Handle( h ){}
	VertexDeclHandle( SGRX_IVertexDecl* vd ) : Handle( vd ){}
	
	const VDeclInfo& GetInfo();
};

struct ENGINE_EXPORT SGRX_SurfaceShader
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	~SGRX_SurfaceShader();
	
	void ReloadShaders();
	
	Array< ShaderHandle > m_shaders;
	
	int32_t m_refcount;
	String m_key;
};

struct ENGINE_EXPORT SurfaceShaderHandle : Handle< SGRX_SurfaceShader >
{
	SurfaceShaderHandle() : Handle(){}
	SurfaceShaderHandle( const SurfaceShaderHandle& h ) : Handle( h ){}
	SurfaceShaderHandle( SGRX_SurfaceShader* shdr ) : Handle( shdr ){}
};

#define NUM_MATERIAL_TEXTURES 8
struct ENGINE_EXPORT SGRX_Material
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	SGRX_Material();
	~SGRX_Material();
	
	SurfaceShaderHandle shader;
	TextureHandle textures[ NUM_MATERIAL_TEXTURES ];
	
	uint32_t transparent : 1;
	uint32_t unlit : 1;
	uint32_t additive : 1;
	
	int32_t m_refcount;
	String m_key;
};

struct ENGINE_EXPORT MaterialHandle : Handle< SGRX_Material >
{
	MaterialHandle() : Handle(){}
	MaterialHandle( const MaterialHandle& h ) : Handle( h ){}
	MaterialHandle( SGRX_Material* shdr ) : Handle( shdr ){}
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
#define MDF_NOCULL        0x40 /* mesh has culling disabled */
#define MDF_SKINNED       0x80 /* mesh has bone data (name, offset, parent id) */

#define MDF__PUBFLAGMASK (0x01|0x02|0x10|0x20|0x40|0x80)
#define MDF__PUBFLAGBASE  0

#define MAX_MESH_PARTS 16
#define MAX_MESH_BONES 64

struct SGRX_MeshPart
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	MaterialHandle material;
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

struct ENGINE_EXPORT SGRX_IMesh
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	SGRX_IMesh();
	virtual ~SGRX_IMesh();
	
	virtual bool InitVertexBuffer( size_t size ) = 0;
	virtual bool InitIndexBuffer( size_t size, bool i32 ) = 0;
	virtual bool UpdateVertexData( const void* data, size_t size, VertexDeclHandle vd, bool tristrip ) = 0;
	virtual bool UpdateIndexData( const void* data, size_t size ) = 0;
	virtual bool SetPartData( SGRX_MeshPart* parts, int count );
	
	bool SetBoneData( SGRX_MeshBone* bones, int count );
	bool RecalcBoneMatrices();
	bool SetAABBFromVertexData( const void* data, size_t size, VertexDeclHandle vd );
	
	bool SetVertexData( const void* data, size_t size, VertexDeclHandle vd, bool tristrip )
	{
		return InitVertexBuffer( size ) && UpdateVertexData( data, size, vd, tristrip );
	}
	bool SetIndexData( const void* data, size_t size, bool i32 )
	{
		return InitIndexBuffer( size, i32 ) && UpdateIndexData( data, size );
	}
	
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
	
	/* vertex/index data */
	ByteArray m_vdata;
	ByteArray m_idata;
	
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

struct ENGINE_EXPORT SGRX_Light
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

struct ENGINE_EXPORT LightHandle : Handle< SGRX_Light >
{
	LightHandle() : Handle(){}
	LightHandle( const LightHandle& h ) : Handle( h ){}
	LightHandle( SGRX_Light* lt ) : Handle( lt ){}
};



struct ENGINE_EXPORT SGRX_CullSceneFrustum
{
	Vec3 position;
	Vec3 direction;
	Vec3 up;
	float hangle;
	float vangle;
	float znear;
	float zfar;
};

struct ENGINE_EXPORT SGRX_CullSceneCamera
{
	SGRX_CullSceneFrustum frustum;
	Mat4 viewProjMatrix;
	Mat4 invViewProjMatrix;
};

struct ENGINE_EXPORT SGRX_CullScenePointLight
{
	Vec3 position;
	float radius;
};

struct ENGINE_EXPORT SGRX_CullSceneMesh
{
	Mat4 transform;
	Vec3 min, max;
};

struct ENGINE_EXPORT SGRX_CullScene
{
	virtual ~SGRX_CullScene();
	virtual void Camera_Prepare( SGRX_CullSceneCamera* camera ){}
	virtual bool Camera_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield ){ return false; }
	virtual bool Camera_PointLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullScenePointLight* lights, uint32_t* outbitfield ){ return false; }
	virtual bool Camera_SpotLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneFrustum* frusta, Mat4* inv_matrices, uint32_t* outbitfield ){ return false; }
	virtual void SpotLight_Prepare( SGRX_CullSceneCamera* camera ){ Camera_Prepare( camera ); }
	virtual bool SpotLight_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield ){ return Camera_MeshList( count, camera, meshes, outbitfield ); }
};

struct ENGINE_EXPORT SGRX_DefaultCullScene
{
	virtual bool Camera_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield );
	virtual bool Camera_PointLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullScenePointLight* lights, uint32_t* outbitfield );
	virtual bool Camera_SpotLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneFrustum* frusta, Mat4* inv_matrices, uint32_t* outbitfield );
	
	Array< Vec3 > m_aabbCache;
};


struct ENGINE_EXPORT SGRX_MeshInstance
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
	uint32_t dynamic : 1;
	uint32_t transparent : 1;
	uint32_t unlit : 1;
//	uint32_t additive : 1;
	
	TextureHandle textures[ MAX_MI_TEXTURES ];
	Vec4 constants[ MAX_MI_CONSTANTS ];
	
	Array< Mat4 > skin_matrices;
	
	/* frame cache */
	SGRX_MeshInstLight* _lightbuf_begin;
	SGRX_MeshInstLight* _lightbuf_end;
	
	int32_t _refcount;
};

struct ENGINE_EXPORT MeshInstHandle : Handle< SGRX_MeshInstance >
{
	MeshInstHandle() : Handle(){}
	MeshInstHandle( const MeshInstHandle& h ) : Handle( h ){}
	MeshInstHandle( SGRX_MeshInstance* mi ) : Handle( mi ){}
};

struct ENGINE_EXPORT SGRX_Camera
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

ENGINE_EXPORT SGRX_Log& operator << ( SGRX_Log& log, const SGRX_Camera& cam );

struct SGRX_Viewport
{
	int x1, y1, x2, y2;
};

struct ENGINE_EXPORT SGRX_Scene
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

struct ENGINE_EXPORT SceneHandle : Handle< SGRX_Scene >
{
	SceneHandle() : Handle(){}
	SceneHandle( const SceneHandle& h ) : Handle( h ){}
	SceneHandle( struct SGRX_Scene* sc ) : Handle( sc ){}
};

struct ENGINE_EXPORT LightTree
{
	struct Sample
	{
		Vec3 pos;
		Vec3 color[6]; // X,Y,Z / +,-
		template< class T > void Serialize( T& arch ){ arch << pos; for( int i = 0; i < 6; ++i ) arch << color[i]; }
	};
	
	void Clear(){ m_samples.clear(); m_tris.clear(); m_triadj.clear(); m_adjdata.clear(); }
	void InsertSamples( const Sample* samples, size_t count );
	void Interpolate( Sample& S, int32_t* outlastfound = NULL );
	
	Array< Sample > m_samples;
	Array< int32_t > m_tris; // 3 x triangle count
	Array< int32_t > m_triadj; // 2 x triangle count (offset,count)
	Array< int32_t > m_adjdata; // ? indexed by m_triadj
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
	StringView shader_name;
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

struct ENGINE_EXPORT BatchRenderer
{
	struct Vertex
	{
		float x, y, z;
		uint32_t color;
		float u, v;
	};
	struct State
	{
		State() : primType(PT_None){}
		
		TextureHandle texture;
		ShaderHandle shader;
		EPrimitiveType primType;
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
	FINLINE BatchRenderer& Pos( const Vec2& pos, float z = 0.0f ){ return Pos( pos.x, pos.y, z ); }
	FINLINE BatchRenderer& Pos( const Vec3& pos ){ return Pos( pos.x, pos.y, pos.z ); }
	
	BatchRenderer& Prev( int i );
	BatchRenderer& RawQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float z = 0 );
	BatchRenderer& Quad( float x0, float y0, float x1, float y1, float z = 0 );
	BatchRenderer& QuadFrame( float x0, float y0, float x1, float y1, float ix0, float iy0, float ix1, float iy1, float z = 0 );
	FINLINE BatchRenderer& QuadWH( float x, float y, float w, float h, float z = 0 ){ return Quad( x, y, x + w, y + h, z ); }
	FINLINE BatchRenderer& Box( float x, float y, float w, float h, float z = 0 ){ w *= 0.5f; h *= 0.5f; return Quad( x - w, y - h, x + w, y + h, z ); }
	BatchRenderer& TurnedBox( float x, float y, float dx, float dy, float z = 0 );
	BatchRenderer& Sprite( const Vec3& pos, const Vec3& dx, const Vec3& dy );
	BatchRenderer& TexLine( const Vec2& p0, const Vec2& p1, float rad );
	BatchRenderer& CircleFill( float x, float y, float r, float z = 0, int verts = -1 );
	BatchRenderer& CircleOutline( float x, float y, float r, float z = 0, int verts = -1 );
	BatchRenderer& CircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts );
	BatchRenderer& SphereOutline( const Vec3& pos, float radius, int verts );
	BatchRenderer& ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, float angle, int verts );
	BatchRenderer& AABB( const Vec3& bbmin, const Vec3& bbmax, const Mat4& transform = Mat4::Identity );
	BatchRenderer& Tick( const Vec3& pos, float radius, const Mat4& transform = Mat4::Identity );
	
	bool CheckSetTexture( const TextureHandle& tex );
	BatchRenderer& SetTexture( const TextureHandle& tex );
	BatchRenderer& SetShader( const ShaderHandle& shd );
	BatchRenderer& UnsetTexture(){ return SetTexture( NULL ); }
	BatchRenderer& SetPrimitiveType( EPrimitiveType pt );
	BatchRenderer& Flush();
	BatchRenderer& Reset();
	void _UpdateDiff();
	
	Array< Vec4 > ShaderData;
	
	IRenderer* m_renderer;
	State m_currState;
	State m_nextState;
	bool m_diff;
	Vertex m_proto;
	bool m_swapRB;
	Array< Vertex > m_verts;
};

struct ENGINE_EXPORT SGRX_PostDraw
{
	virtual void PostDraw() = 0;
};

struct ENGINE_EXPORT SGRX_DebugDraw
{
	virtual void DebugDraw() = 0;
};

struct ENGINE_EXPORT SGRX_RenderScene
{
	SGRX_RenderScene( const Vec4& tv, const SceneHandle& sh, bool enablePP = true ) :
		timevals( tv ),
		scene( sh ),
		enablePostProcessing( enablePP ),
		viewport( NULL ),
		postdraw( NULL ),
		debugdraw( NULL )
	{}
	
	Vec4 timevals;
	SceneHandle scene;
	bool enablePostProcessing;
	SGRX_Viewport* viewport;
	SGRX_PostDraw* postdraw;
	SGRX_DebugDraw* debugdraw;
};


ENGINE_EXPORT int GR_GetWidth();
ENGINE_EXPORT int GR_GetHeight();

ENGINE_EXPORT TextureHandle GR_CreateTexture( int width, int height, int format, int mips = 1 );
ENGINE_EXPORT TextureHandle GR_GetTexture( const StringView& path );
ENGINE_EXPORT TextureHandle GR_CreateRenderTexture( int width, int height, int format );
ENGINE_EXPORT ShaderHandle GR_GetShader( const StringView& path );
ENGINE_EXPORT SurfaceShaderHandle GR_GetSurfaceShader( const StringView& name );
ENGINE_EXPORT MaterialHandle GR_CreateMaterial();
ENGINE_EXPORT VertexDeclHandle GR_GetVertexDecl( const StringView& vdecl );
ENGINE_EXPORT MeshHandle GR_CreateMesh();
ENGINE_EXPORT MeshHandle GR_GetMesh( const StringView& path );

ENGINE_EXPORT SceneHandle GR_CreateScene();
ENGINE_EXPORT bool GR_SetRenderPasses( SGRX_RenderPass* passes, int count );
ENGINE_EXPORT void GR_RenderScene( SGRX_RenderScene& info );
ENGINE_EXPORT RenderStats& GR_GetRenderStats();

ENGINE_EXPORT void GR2D_SetWorldMatrix( const Mat4& mtx );
ENGINE_EXPORT void GR2D_SetViewMatrix( const Mat4& mtx );
ENGINE_EXPORT void GR2D_SetViewport( int x0, int y0, int x1, int y1 );
ENGINE_EXPORT void GR2D_UnsetViewport();
ENGINE_EXPORT void GR2D_SetScissorRect( int x0, int y0, int x1, int y1 );
ENGINE_EXPORT void GR2D_UnsetScissorRect();
ENGINE_EXPORT bool GR2D_SetFont( const StringView& name, int pxsize );
ENGINE_EXPORT void GR2D_SetColor( float r, float g, float b, float a = 1.0f );
inline void GR2D_SetColor( float x, float a ){ GR2D_SetColor( x, x, x, a ); }
inline void GR2D_SetColor( float x ){ GR2D_SetColor( x, x, x, x ); }
ENGINE_EXPORT void GR2D_SetTextCursor( float x, float y );
ENGINE_EXPORT Vec2 GR2D_GetTextCursor();
ENGINE_EXPORT int GR2D_GetTextLength( const StringView& text );
ENGINE_EXPORT int GR2D_DrawTextLine( const StringView& text );
ENGINE_EXPORT int GR2D_DrawTextLine( float x, float y, const StringView& text );
#define HALIGN_LEFT 0
#define HALIGN_CENTER 1
#define HALIGN_RIGHT 2
#define VALIGN_TOP 0
#define VALIGN_CENTER 1
#define VALIGN_BOTTOM 2
ENGINE_EXPORT int GR2D_DrawTextLine( float x, float y, const StringView& text, int halign, int valign );

ENGINE_EXPORT BatchRenderer& GR2D_GetBatchRenderer();


extern "C" ENGINE_EXPORT int SGRX_EntryPoint( int argc, char** argv, int debug );

