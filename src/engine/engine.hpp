

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
ENGINE_EXPORT void Sys_FatalError( const StringView& err );

ENGINE_EXPORT bool Window_HasClipboardText();
ENGINE_EXPORT bool Window_GetClipboardText( String& out );
ENGINE_EXPORT bool Window_SetClipboardText( const StringView& text );

typedef SDL_Event Event;


struct IF_GCC(ENGINE_EXPORT) IProcessor
{
	ENGINE_EXPORT virtual void Process( void* data ) = 0;
};

struct SGRX_RefCounted
{
	SGRX_RefCounted() : m_refcount(0){}
	virtual ~SGRX_RefCounted(){}
	
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	int32_t m_refcount;
};

struct SGRX_RCRsrc : SGRX_RefCounted
{
	String m_key; // [storage for] hash table key
};

struct SGRX_RCXFItem : SGRX_RefCounted
{
	virtual void SetTransform( const Mat4& mtx ) = 0;
};

typedef Handle< SGRX_RCXFItem > XFItemHandle;


struct Command
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
	
	ENGINE_EXPORT void _SetState( float x );
	ENGINE_EXPORT void _Advance();
	
	StringView name;
	float threshold;
	float value, prev_value;
	bool state, prev_state;
};

struct Command_Func : Command
{
	virtual void Function() = 0;
	Command_Func( const StringView& sv, float thr = 0.1f ) : Command( sv, thr ){}
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
	virtual bool OnConfigure( int argc, char** argv ){ return true; }
	virtual bool OnInitialize(){ return true; }
	virtual void OnDestroy(){}
	virtual void OnEvent( const Event& e ){}
	virtual void OnTick( float dt, uint32_t gametime ) = 0;
	
	virtual bool OnLoadTexture( const StringView& key, ByteArray& outdata, uint32_t& outusageflags );
	virtual void GetShaderCacheFilename( const StringView& type, const char* sfx, const StringView& key, String& name );
	virtual bool GetCompiledShader( const StringView& type, const char* sfx, const StringView& key, ByteArray& outdata );
	virtual bool SetCompiledShader( const StringView& type, const char* sfx, const StringView& key, const ByteArray& data );
	virtual bool OnLoadShader( const StringView& type, const StringView& key, String& outdata );
	virtual bool OnLoadShaderFile( const StringView& type, const StringView& path, String& outdata );
	virtual bool ParseShaderIncludes( const StringView& type, const StringView& path, String& outdata );
	virtual bool OnLoadMesh( const StringView& key, ByteArray& outdata );
};

struct IF_GCC(ENGINE_EXPORT) IDirEntryHandler
{
	ENGINE_EXPORT virtual bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir ) = 0;
};

struct IF_GCC(ENGINE_EXPORT) IFileSystem
{
	FINLINE void Acquire(){ ++m_refcount; }
	FINLINE void Release(){ --m_refcount; if( m_refcount <= 0 ) delete this; }
	
	ENGINE_EXPORT IFileSystem();
	ENGINE_EXPORT virtual ~IFileSystem();
	ENGINE_EXPORT virtual bool LoadBinaryFile( const StringView& path, ByteArray& out ) = 0;
	ENGINE_EXPORT virtual bool SaveBinaryFile( const StringView& path, const void* data, size_t size ) = 0;
	ENGINE_EXPORT virtual bool LoadTextFile( const StringView& path, String& out ) = 0;
	ENGINE_EXPORT virtual bool SaveTextFile( const StringView& path, const StringView& data ) = 0;
	ENGINE_EXPORT virtual bool FileExists( const StringView& path ) = 0;
	ENGINE_EXPORT virtual bool DirCreate( const StringView& path ) = 0;
	ENGINE_EXPORT virtual void IterateDirectory( const StringView& path, IDirEntryHandler* deh ) = 0;
	
	int32_t m_refcount;
};

struct IF_GCC(ENGINE_EXPORT) BasicFileSystem : IFileSystem
{
	ENGINE_EXPORT BasicFileSystem( const StringView& root );
	ENGINE_EXPORT virtual bool LoadBinaryFile( const StringView& path, ByteArray& out );
	ENGINE_EXPORT virtual bool SaveBinaryFile( const StringView& path, const void* data, size_t size );
	ENGINE_EXPORT virtual bool LoadTextFile( const StringView& path, String& out );
	ENGINE_EXPORT virtual bool SaveTextFile( const StringView& path, const StringView& data );
	ENGINE_EXPORT virtual bool FileExists( const StringView& path );
	ENGINE_EXPORT virtual bool DirCreate( const StringView& path );
	ENGINE_EXPORT virtual void IterateDirectory( const StringView& path, IDirEntryHandler* deh );
	
	String m_fileRoot;
};
typedef Handle< IFileSystem > FileSysHandle;

ENGINE_EXPORT StringView Game_GetDir();
ENGINE_EXPORT Array< FileSysHandle >& Game_FileSystems();

ENGINE_EXPORT bool FS_LoadBinaryFile( const StringView& path, ByteArray& out );
ENGINE_EXPORT bool FS_SaveBinaryFile( const StringView& path, const void* data, size_t size );
ENGINE_EXPORT bool FS_LoadTextFile( const StringView& path, String& out );
ENGINE_EXPORT bool FS_SaveTextFile( const StringView& path, const StringView& data );
ENGINE_EXPORT bool FS_FileExists( const StringView& path );
ENGINE_EXPORT bool FS_DirCreate( const StringView& path );
ENGINE_EXPORT void FS_IterateDirectory( const StringView& path, IDirEntryHandler* deh );


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

struct IF_GCC(ENGINE_EXPORT) SGRX_ITexture : SGRX_RCRsrc
{
	ENGINE_EXPORT virtual ~SGRX_ITexture();
	ENGINE_EXPORT virtual bool UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h ) = 0;
	
	TextureInfo m_info;
	bool m_isRenderTexture;
};

struct TextureHandle : Handle< SGRX_ITexture >
{
	TextureHandle() : Handle(){}
	TextureHandle( const TextureHandle& h ) : Handle( h ){}
	TextureHandle( SGRX_ITexture* tex ) : Handle( tex ){}
	
	ENGINE_EXPORT const TextureInfo& GetInfo() const;
	ENGINE_EXPORT bool UploadRGBA8Part( void* data, int mip = 0, int w = -1, int h = -1, int x = 0, int y = 0 );
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IVertexShader : SGRX_RCRsrc
{
	ENGINE_EXPORT virtual ~SGRX_IVertexShader();
};

struct VertexShaderHandle : Handle< SGRX_IVertexShader >
{
	VertexShaderHandle() : Handle(){}
	VertexShaderHandle( const VertexShaderHandle& h ) : Handle( h ){}
	VertexShaderHandle( SGRX_IVertexShader* shdr ) : Handle( shdr ){}
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IPixelShader : SGRX_RCRsrc
{
	ENGINE_EXPORT virtual ~SGRX_IPixelShader();
};

struct PixelShaderHandle : Handle< SGRX_IPixelShader >
{
	PixelShaderHandle() : Handle(){}
	PixelShaderHandle( const PixelShaderHandle& h ) : Handle( h ){}
	PixelShaderHandle( SGRX_IPixelShader* shdr ) : Handle( shdr ){}
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
	
	int GetOffset( int usage ) const
	{
		for( int i = 0; i < count; ++i )
			if( usages[ i ] == usage )
				return offsets[ i ];
		return -1;
	}
	int GetType( int usage ) const
	{
		for( int i = 0; i < count; ++i )
			if( usages[ i ] == usage )
				return types[ i ];
		return -1;
	}
};

#define SGRX_VDECL_DECAL "pf3nf30f3tb4cb4xb4"
struct SGRX_Vertex_Decal
{
	Vec3 position;
	Vec3 normal;
	Vec3 texcoord;
	uint32_t tangent;
	uint32_t color;
	uint32_t padding0;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IVertexDecl : SGRX_RCRsrc
{
	ENGINE_EXPORT virtual ~SGRX_IVertexDecl();
	
	VDeclInfo m_info;
};

struct VertexDeclHandle : Handle< SGRX_IVertexDecl >
{
	VertexDeclHandle() : Handle(){}
	VertexDeclHandle( const VertexDeclHandle& h ) : Handle( h ){}
	VertexDeclHandle( SGRX_IVertexDecl* vd ) : Handle( vd ){}
	
	ENGINE_EXPORT const VDeclInfo& GetInfo();
};

struct SGRX_SurfaceShader : SGRX_RCRsrc
{
	ENGINE_EXPORT ~SGRX_SurfaceShader();
	
	ENGINE_EXPORT void ReloadShaders();
	
	Array< VertexShaderHandle > m_basicVertexShaders;
	Array< VertexShaderHandle > m_skinVertexShaders;
	Array< PixelShaderHandle > m_pixelShaders;
};

struct SurfaceShaderHandle : Handle< SGRX_SurfaceShader >
{
	SurfaceShaderHandle() : Handle(){}
	SurfaceShaderHandle( const SurfaceShaderHandle& h ) : Handle( h ){}
	SurfaceShaderHandle( SGRX_SurfaceShader* shdr ) : Handle( shdr ){}
};

#define NUM_MATERIAL_TEXTURES 8
struct SGRX_Material : SGRX_RCRsrc
{
	ENGINE_EXPORT SGRX_Material();
	ENGINE_EXPORT ~SGRX_Material();
	
	SurfaceShaderHandle shader;
	TextureHandle textures[ NUM_MATERIAL_TEXTURES ];
	
	uint32_t transparent : 1;
	uint32_t unlit : 1;
	uint32_t additive : 1;
};

struct MaterialHandle : Handle< SGRX_Material >
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

struct SGRX_IMesh : SGRX_RCRsrc
{
	ENGINE_EXPORT SGRX_IMesh();
	ENGINE_EXPORT virtual ~SGRX_IMesh();
	
	virtual bool InitVertexBuffer( size_t size, VertexDeclHandle vd ) = 0;
	virtual bool InitIndexBuffer( size_t size, bool i32 ) = 0;
	virtual bool UpdateVertexData( const void* data, size_t size, bool tristrip ) = 0;
	virtual bool UpdateIndexData( const void* data, size_t size ) = 0;
	ENGINE_EXPORT virtual bool SetPartData( SGRX_MeshPart* parts, int count );
	
	ENGINE_EXPORT bool SetBoneData( SGRX_MeshBone* bones, int count );
	ENGINE_EXPORT bool RecalcBoneMatrices();
	ENGINE_EXPORT bool SetAABBFromVertexData( const void* data, size_t size, VertexDeclHandle vd );
	
	bool SetVertexData( const void* data, size_t size, VertexDeclHandle vd, bool tristrip )
	{
		return InitVertexBuffer( size, vd ) && UpdateVertexData( data, size, tristrip );
	}
	bool SetIndexData( const void* data, size_t size, bool i32 )
	{
		return InitIndexBuffer( size, i32 ) && UpdateIndexData( data, size );
	}
	
	ENGINE_EXPORT void RaycastAll( const Vec3& from, const Vec3& to, struct SceneRaycastCallback* cb, struct SGRX_MeshInstance* cbmi = NULL );
	
	ENGINE_EXPORT void Clip(
		const Mat4& mtx,
		const Mat4& vpmtx,
		ByteArray& outverts,
		bool decal = false,
		float inv_zn2zf = 0,
		size_t firstPart = 0,
		size_t numParts = (size_t)-1
	);
	
	/* rendering info */
	uint32_t m_dataFlags;
	VertexDeclHandle m_vertexDecl;
	uint32_t m_vertexCount;
	uint32_t m_vertexDataSize;
	uint32_t m_indexCount;
	uint32_t m_indexDataSize;
	Array< SGRX_MeshPart > m_meshParts;
	SGRX_MeshBone m_bones[ MAX_MESH_BONES ];
	int m_numBones;
	
	/* collision detection */
	Vec3 m_boundsMin;
	Vec3 m_boundsMax;
	
	/* vertex/index data */
	ByteArray m_vdata;
	ByteArray m_idata;
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
#define LIGHT_PROJ   3

struct SGRX_Light : SGRX_RCXFItem
{
	ENGINE_EXPORT SGRX_Light( SGRX_Scene* s );
	ENGINE_EXPORT virtual ~SGRX_Light();
	ENGINE_EXPORT void UpdateTransform();
	ENGINE_EXPORT void GenerateCamera( struct SGRX_Camera& outcam );
	ENGINE_EXPORT virtual void SetTransform( const Mat4& mtx );
	
	SGRX_Scene* _scene;
	
	// inputs
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
	bool hasShadows;
	uint32_t layers;
	PixelShaderHandle projectionShader;
	TextureHandle projectionTextures[ NUM_MATERIAL_TEXTURES ];
	Mat4 matrix;
	
	// cache
	Vec3 _tf_position;
	Vec3 _tf_direction;
	Vec3 _tf_updir;
	float _tf_range;
	Mat4 viewMatrix;
	Mat4 projMatrix;
	Mat4 viewProjMatrix;
	
	/* frame cache */
	SGRX_MeshInstLight* _mibuf_begin;
	SGRX_MeshInstLight* _mibuf_end;
};

struct LightHandle : Handle< SGRX_Light >
{
	LightHandle() : Handle(){}
	LightHandle( const LightHandle& h ) : Handle( h ){}
	LightHandle( SGRX_Light* lt ) : Handle( lt ){}
};



struct SGRX_CullSceneFrustum
{
	Vec3 position;
	Vec3 direction;
	Vec3 updir;
	float hangle;
	float vangle;
	float znear;
	float zfar;
};

struct SGRX_CullSceneCamera
{
	SGRX_CullSceneFrustum frustum;
	Mat4 viewProjMatrix;
	Mat4 invViewProjMatrix;
};

struct SGRX_CullScenePointLight
{
	Vec3 position;
	float radius;
};

struct SGRX_CullSceneMesh
{
	Mat4 transform;
	Vec3 min, max;
};

struct SGRX_CullScene
{
	ENGINE_EXPORT virtual ~SGRX_CullScene();
	virtual void Camera_Prepare( SGRX_CullSceneCamera* camera ){}
	virtual bool Camera_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield ){ return false; }
	virtual bool Camera_PointLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullScenePointLight* lights, uint32_t* outbitfield ){ return false; }
	virtual bool Camera_SpotLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneFrustum* frusta, Mat4* inv_matrices, uint32_t* outbitfield ){ return false; }
	virtual void SpotLight_Prepare( SGRX_CullSceneCamera* camera ){ Camera_Prepare( camera ); }
	virtual bool SpotLight_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield ){ return Camera_MeshList( count, camera, meshes, outbitfield ); }
};

struct SGRX_DefaultCullScene : SGRX_CullScene
{
	ENGINE_EXPORT virtual bool Camera_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield );
	ENGINE_EXPORT virtual bool Camera_PointLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullScenePointLight* lights, uint32_t* outbitfield );
	ENGINE_EXPORT virtual bool Camera_SpotLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneFrustum* frusta, Mat4* inv_matrices, uint32_t* outbitfield );
	
	Array< Vec3 > m_aabbCache;
};


struct SGRX_MeshInstance : SGRX_RCXFItem
{
	ENGINE_EXPORT SGRX_MeshInstance( SGRX_Scene* s );
	ENGINE_EXPORT virtual ~SGRX_MeshInstance();
	ENGINE_EXPORT virtual void SetTransform( const Mat4& mtx );
	
	SGRX_Scene* _scene;
	
	MeshHandle mesh;
	Mat4 matrix;
	Vec4 color;
	uint32_t layers;
	uint32_t enabled : 1;
	uint32_t cpuskin : 1; /* TODO */
	uint32_t dynamic : 1;
	uint32_t decal : 1;
	uint32_t transparent : 1;
	uint32_t unlit : 1;
//	uint32_t additive : 1;
	
	TextureHandle textures[ MAX_MI_TEXTURES ];
	Vec4 constants[ MAX_MI_CONSTANTS ];
	
	Array< Mat4 > skin_matrices;
	
	/* frame cache */
	SGRX_MeshInstLight* _lightbuf_begin;
	SGRX_MeshInstLight* _lightbuf_end;
};

struct MeshInstHandle : Handle< SGRX_MeshInstance >
{
	MeshInstHandle() : Handle(){}
	MeshInstHandle( const MeshInstHandle& h ) : Handle( h ){}
	MeshInstHandle( SGRX_MeshInstance* mi ) : Handle( mi ){}
};

struct SGRX_Camera
{
	ENGINE_EXPORT void Log( SGRX_Log& elog );
	ENGINE_EXPORT void UpdateViewMatrix();
	ENGINE_EXPORT void UpdateProjMatrix();
	ENGINE_EXPORT void UpdateMatrices();
	
	ENGINE_EXPORT Vec3 WorldToScreen( const Vec3& pos );
	ENGINE_EXPORT bool GetCursorRay( float x, float y, Vec3& pos, Vec3& dir );
	
	Vec3 position;
	Vec3 direction;
	Vec3 updir;
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

ENGINE_EXPORT uint32_t SGRX_FindOrAddVertex( ByteArray& vertbuf, size_t searchoffset, size_t& writeoffset, const uint8_t* vertex, size_t vertsize );
ENGINE_EXPORT void SGRX_DoIndexTriangleMeshVertices( UInt32Array& indices, ByteArray& vertices, size_t offset, size_t stride );
struct IF_GCC(ENGINE_EXPORT) SGRX_ProjectionMeshProcessor : IProcessor
{
	ByteArray* outVertices;
	UInt32Array* outIndices;
	Mat4 viewProjMatrix;
	float invZNearToZFar;
	
	SGRX_ProjectionMeshProcessor( ByteArray* verts, UInt32Array* indices, const Mat4& mtx, float zn2zf );
	ENGINE_EXPORT virtual void Process( void* data );
};

struct SceneRaycastInfo
{
	float factor;
	Vec3 normal;
	float u, v;
	int partID, triID;
	MeshInstHandle meshinst;
};
struct IF_GCC(ENGINE_EXPORT) SceneRaycastCallback
{
	virtual void AddResult( SceneRaycastInfo* info ) = 0;
};
struct IF_GCC(ENGINE_EXPORT) SceneRaycastCallback_Closest : SceneRaycastCallback
{
	ENGINE_EXPORT SceneRaycastCallback_Closest();
	ENGINE_EXPORT virtual void AddResult( SceneRaycastInfo* info );
	
	bool m_hit;
	SceneRaycastInfo m_closest;
};
struct IF_GCC(ENGINE_EXPORT) SceneRaycastCallback_Sorting : SceneRaycastCallback
{
	ENGINE_EXPORT SceneRaycastCallback_Sorting( Array< SceneRaycastInfo >* sortarea );
	ENGINE_EXPORT virtual void AddResult( SceneRaycastInfo* info );
	
	Array< SceneRaycastInfo >* m_sortarea;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_Scene : SGRX_RefCounted
{
	ENGINE_EXPORT SGRX_Scene();
	ENGINE_EXPORT virtual ~SGRX_Scene();
	ENGINE_EXPORT MeshInstHandle CreateMeshInstance();
	ENGINE_EXPORT LightHandle CreateLight();
	
	// finds closest match and returns only that
	ENGINE_EXPORT bool RaycastOne( const Vec3& from, const Vec3& to, SceneRaycastInfo* outinfo = NULL, uint32_t layers = 0xffffffff );
	// finds all matches and returns them in random order
	ENGINE_EXPORT void RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, uint32_t layers = 0xffffffff );
	// - sorted
	ENGINE_EXPORT void RaycastAllSort( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb,
		uint32_t layers = 0xffffffff, Array< SceneRaycastInfo >* tmpstore = NULL );
	
	ENGINE_EXPORT void GatherMeshes( const SGRX_Camera& cam, IProcessor* meshInstProc, uint32_t layers = 0xffffffff );
	ENGINE_EXPORT void GenerateProjectionMesh( const SGRX_Camera& cam, ByteArray& outverts, UInt32Array& outindices, uint32_t layers = 0xffffffff );
	
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
};

struct SceneHandle : Handle< SGRX_Scene >
{
	SceneHandle() : Handle(){}
	SceneHandle( const SceneHandle& h ) : Handle( h ){}
	SceneHandle( struct SGRX_Scene* sc ) : Handle( sc ){}
};

struct SGRX_SceneTree
{
	struct Node
	{
		size_t parent_id;
		String name;
		Mat4 transform;
	};
	struct Item
	{
		size_t node_id;
		XFItemHandle item;
	};
	
	size_t _NormalizeIndex( size_t id ){ return id < nodes.size() ? id : NOT_FOUND; }
	Node* FindNodeByName( const StringView& name ){ size_t id = FindNodeIDByName( name ); return id < nodes.size() ? &nodes[ id ] : NULL; }
	Node* FindNodeByPath( const StringView& path ){ size_t id = FindNodeIDByPath( path ); return id < nodes.size() ? &nodes[ id ] : NULL; }
	ENGINE_EXPORT size_t FindNodeIDByName( const StringView& name );
	ENGINE_EXPORT size_t FindNodeIDByPath( const StringView& path );
	ENGINE_EXPORT void UpdateTransforms();
	
	Array< Node > nodes;
	Array< Mat4 > transforms;
	Array< Item > items;
};

struct LightTree
{
	struct Sample
	{
		Vec3 pos;
		Vec3 color[6]; // X,Y,Z / +,-
		template< class T > void Serialize( T& arch ){ arch << pos; for( int i = 0; i < 6; ++i ) arch << color[i]; }
	};
	
	void Clear(){ m_samples.clear(); m_tris.clear(); m_triadj.clear(); m_adjdata.clear(); }
	ENGINE_EXPORT void InsertSamples( const Sample* samples, size_t count );
	ENGINE_EXPORT void Interpolate( Sample& S, int32_t* outlastfound = NULL );
	
	Array< Sample > m_samples;
	Array< int32_t > m_tris; // 3 x triangle count
	Array< int32_t > m_triadj; // 2 x triangle count (offset,count)
	Array< int32_t > m_adjdata; // ? indexed by m_triadj
};

/* render pass constants */
#define RPT_OBJECT     1
#define RPT_SCREEN     2
#define RPT_SHADOWS    3
#define RPT_PROJECTORS 4

#define RPF_OBJ_STATIC      0x01
#define RPF_OBJ_DYNAMIC     0x02
#define RPF_OBJ_ALL        (RPF_OBJ_STATIC|RPF_OBJ_DYNAMIC)
#define RPF_MTL_SOLID       0x04
#define RPF_MTL_TRANSPARENT 0x08
#define RPF_MTL_ALL        (RPF_MTL_SOLID|RPF_MTL_TRANSPARENT)
#define RPF_CALC_DIRAMB     0x10
#define RPF_LIGHTOVERLAY    0x20
#define RPF_DECALS          0x40
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
};

struct BatchRenderer
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
		PixelShaderHandle shader;
		EPrimitiveType primType;
	};
	
	ENGINE_EXPORT BatchRenderer( struct IRenderer* r );
	~BatchRenderer(){ if( m_renderer ) Flush(); }
	
	ENGINE_EXPORT BatchRenderer& AddVertices( Vertex* verts, int count );
	ENGINE_EXPORT BatchRenderer& AddVertex( const Vertex& vert );
	FINLINE BatchRenderer& Col( float x ){ return Col( x, x, x, x ); }
	FINLINE BatchRenderer& Col( float x, float a ){ return Col( x, x, x, a ); }
	FINLINE BatchRenderer& Col( float r, float g, float b ){ return Col( r, g, b, 1.0f ); }
	FINLINE BatchRenderer& Col( float r, float g, float b, float a ){ return Colb( COLOR_F2B( r ), COLOR_F2B( g ), COLOR_F2B( b ), COLOR_F2B( a ) ); }
	ENGINE_EXPORT BatchRenderer& Colb( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	FINLINE BatchRenderer& Colu( uint32_t c ){ return Colb( COLOR_EXTRACT_R( c ), COLOR_EXTRACT_G( c ), COLOR_EXTRACT_B( c ), COLOR_EXTRACT_A( c ) ); }
	FINLINE BatchRenderer& Tex( float x, float y ){ m_proto.u = x; m_proto.v = y; return *this; }
	FINLINE BatchRenderer& Pos( float x, float y, float z = 0.0f ){ m_proto.x = x; m_proto.y = y; m_proto.z = z; AddVertex( m_proto ); return *this; }
	FINLINE BatchRenderer& Pos( const Vec2& pos, float z = 0.0f ){ return Pos( pos.x, pos.y, z ); }
	FINLINE BatchRenderer& Pos( const Vec3& pos ){ return Pos( pos.x, pos.y, pos.z ); }
	
	ENGINE_EXPORT BatchRenderer& Prev( int i );
	ENGINE_EXPORT BatchRenderer& RawQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float z = 0 );
	ENGINE_EXPORT BatchRenderer& Quad( float x0, float y0, float x1, float y1, float z = 0 );
	ENGINE_EXPORT BatchRenderer& QuadFrame( float x0, float y0, float x1, float y1, float ix0, float iy0, float ix1, float iy1, float z = 0 );
	FINLINE BatchRenderer& QuadWH( float x, float y, float w, float h, float z = 0 ){ return Quad( x, y, x + w, y + h, z ); }
	FINLINE BatchRenderer& Box( float x, float y, float w, float h, float z = 0 ){ w *= 0.5f; h *= 0.5f; return Quad( x - w, y - h, x + w, y + h, z ); }
	ENGINE_EXPORT BatchRenderer& TurnedBox( float x, float y, float dx, float dy, float z = 0 );
	ENGINE_EXPORT BatchRenderer& Poly( const void* data, int count, float z = 0, int stride = sizeof(Vec2) );
	ENGINE_EXPORT BatchRenderer& PolyOutline( const void* data, int count, float z = 0, int stride = sizeof(Vec2) );
	ENGINE_EXPORT BatchRenderer& Sprite( const Vec3& pos, const Vec3& dx, const Vec3& dy );
	ENGINE_EXPORT BatchRenderer& Sprite( const Vec3& pos, float sx, float sy );
	ENGINE_EXPORT BatchRenderer& TexLine( const Vec2& p0, const Vec2& p1, float rad );
	ENGINE_EXPORT BatchRenderer& CircleFill( float x, float y, float r, float z = 0, int verts = -1 );
	ENGINE_EXPORT BatchRenderer& CircleOutline( float x, float y, float r, float z = 0, int verts = -1 );
	ENGINE_EXPORT BatchRenderer& CircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts );
	ENGINE_EXPORT BatchRenderer& SphereOutline( const Vec3& pos, float radius, int verts );
	ENGINE_EXPORT BatchRenderer& ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, float angle, int verts );
	ENGINE_EXPORT BatchRenderer& AABB( const Vec3& bbmin, const Vec3& bbmax, const Mat4& transform = Mat4::Identity );
	ENGINE_EXPORT BatchRenderer& Tick( const Vec3& pos, float radius, const Mat4& transform = Mat4::Identity );
	
	ENGINE_EXPORT bool CheckSetTexture( const TextureHandle& tex );
	ENGINE_EXPORT BatchRenderer& SetTexture( const TextureHandle& tex );
	ENGINE_EXPORT BatchRenderer& SetShader( const PixelShaderHandle& shd );
	FINLINE BatchRenderer& UnsetTexture(){ return SetTexture( NULL ); }
	ENGINE_EXPORT BatchRenderer& SetPrimitiveType( EPrimitiveType pt );
	ENGINE_EXPORT BatchRenderer& Flush();
	ENGINE_EXPORT BatchRenderer& Reset();
	ENGINE_EXPORT void _UpdateDiff();
	ENGINE_EXPORT void _RecalcMatrices();
	
	Array< Vec4 > ShaderData;
	
	IRenderer* m_renderer;
	State m_currState;
	State m_nextState;
	bool m_diff;
	Vertex m_proto;
	bool m_swapRB;
	Array< Vertex > m_verts;
	Mat4 worldMatrix;
	Mat4 viewMatrix;
	Mat4 invMatrix;
};

struct ENGINE_EXPORT SGRX_PostDraw
{
	virtual void PostDraw() = 0;
};

struct ENGINE_EXPORT SGRX_DebugDraw
{
	virtual void DebugDraw() = 0;
};

struct SGRX_RenderScene
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

struct SGRX_FontSettings
{
	String font;
	int size;
	float letterspacing;
};

struct SGRX_GlyphInfo
{
	uint32_t glyph_kern_id;
	int16_t bmsizex;
	int16_t bmsizey;
	int16_t bmoffx;
	int16_t bmoffy;
	int16_t advx;
	int16_t advy;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_IFont : SGRX_RCRsrc
{
	virtual void LoadGlyphInfo( int pxsize, uint32_t ch, SGRX_GlyphInfo* info ) = 0;
	virtual void GetGlyphBitmap( uint32_t* out, int pitch ) = 0;
	virtual int GetKerning( uint32_t ch1, uint32_t ch2 ){ return 0; }
	virtual int GetYOffset( int pxsize ){ return 0; }
};
typedef Handle< SGRX_IFont > FontHandle;


ENGINE_EXPORT int GR_GetWidth();
ENGINE_EXPORT int GR_GetHeight();

ENGINE_EXPORT TextureHandle GR_CreateTexture( int width, int height, int format, int mips = 1 );
ENGINE_EXPORT TextureHandle GR_GetTexture( const StringView& path );
ENGINE_EXPORT TextureHandle GR_CreateRenderTexture( int width, int height, int format );
ENGINE_EXPORT VertexShaderHandle GR_GetVertexShader( const StringView& path );
ENGINE_EXPORT PixelShaderHandle GR_GetPixelShader( const StringView& path );
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

ENGINE_EXPORT void GR2D_SetColor( float r, float g, float b, float a = 1.0f );
inline void GR2D_SetColor( float x, float a ){ GR2D_SetColor( x, x, x, a ); }
inline void GR2D_SetColor( float x ){ GR2D_SetColor( x, x, x, x ); }

ENGINE_EXPORT bool GR2D_LoadFont( const StringView& key, const StringView& path );
ENGINE_EXPORT bool GR2D_LoadSVGIconFont( const StringView& key, const StringView& path );
ENGINE_EXPORT FontHandle GR2D_GetFont( const StringView& key );
ENGINE_EXPORT void GR2D_GetFontSettings( SGRX_FontSettings* settings );
ENGINE_EXPORT void GR2D_SetFontSettings( SGRX_FontSettings* settings );
ENGINE_EXPORT bool GR2D_SetFont( const StringView& name, int pxsize );
ENGINE_EXPORT void GR2D_SetTextCursor( const Vec2& pos );
inline void GR2D_SetTextCursor( float x, float y ){ GR2D_SetTextCursor( V2( x, y ) ); }
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
ENGINE_EXPORT int GR2D_DrawTextLine( const StringView& text, int halign, int valign );
ENGINE_EXPORT int GR2D_DrawTextLine( float x, float y, const StringView& text, int halign, int valign );

ENGINE_EXPORT BatchRenderer& GR2D_GetBatchRenderer();


extern "C" ENGINE_EXPORT int SGRX_EntryPoint( int argc, char** argv, int debug );

