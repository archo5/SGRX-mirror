

#pragma once
#include "engine.hpp"


///
/// TEXTURE
///

ENGINE_EXPORT size_t TextureInfo_GetTextureSideSize( const TextureInfo* TI );
ENGINE_EXPORT void TextureInfo_GetCopyDims( const TextureInfo* TI, size_t* outcopyrowsize, size_t* outcopyrowcount );
ENGINE_EXPORT bool TextureInfo_GetMipInfo( const TextureInfo* TI, int mip, TextureInfo* outinfo );

struct TextureData
{
	TextureInfo info;
	ByteArray data;
};

ENGINE_EXPORT bool TextureData_Load( TextureData* TD, ByteArray& texdata, const StringView& filename = "<memory>" );
ENGINE_EXPORT void TextureData_Free( TextureData* TD );
ENGINE_EXPORT size_t TextureData_GetMipDataOffset( TextureInfo* texinfo, int side, int mip );
ENGINE_EXPORT size_t TextureData_GetMipDataSize( TextureInfo* texinfo, int mip );

ENGINE_EXPORT const char* VDeclInfo_Parse( VDeclInfo* info, const char* text );
ENGINE_EXPORT bool GetAABBFromVertexData( const VDeclInfo& info, const char* vdata, size_t vdsize, Vec3& outMin, Vec3& outMax );

struct MeshFilePartData
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	
	uint8_t flags;
	uint8_t blendMode;
	
	uint8_t materialTextureCount; /* 0 - 8 */
	uint8_t materialStringSizes[ SGRX_MAX_MESH_TEXTURES + 1 ];
	char* materialStrings[ SGRX_MAX_MESH_TEXTURES + 1 ];
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

#define MAX_MESH_FILE_PARTS 16
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
	
	MeshFilePartData parts[ MAX_MESH_FILE_PARTS ];
	/* size w/o padding = 51/63 + 64/100 x16 = 1075/1663 */
	MeshFileBoneData bones[ SGRX_MAX_MESH_BONES ];
	/* size w/o padding = 1075/1663 + 70/74 x32 = 3315/4031 */
};

ENGINE_EXPORT const char* MeshData_Parse( char* buf, size_t size, MeshFileData* out );


// ! REQUIRES ByteArray data source to be preserved ! //
struct AnimFileParser
{
	struct Anim
	{
		char* name;
		float speed;
		uint8_t nameSize;
		uint8_t trackCount;
		uint8_t markerCount;
		uint32_t frameCount;
		uint32_t trackDataOff;
		uint16_t markerDataOff;
	};
	struct Track
	{
		char* name;
		uint8_t nameSize;
		float* dataPtr; // points to 10 * frameCount floats (AoS)
	};
	struct Marker
	{
		char name[ MAX_ANIM_MARKER_NAME_LENGTH ]; // engine.hpp
		int frame;
	};
	
	AnimFileParser( ByteArray& data )
	{
		ByteReader br( &data );
		error = Parse( br );
		if( !error && br.error )
			error = "failed to read data";
	}
	
	ENGINE_EXPORT const char* Parse( ByteReader& br );
	
	const char* error;
	Array< Marker > markerData;
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
inline SGRX_Log& operator << ( SGRX_Log& L, const AnimFileParser::Marker& marker )
{
	L << "MARKER";
	L << "\n\tname = " << StringView( marker.name, sgrx_snlen( marker.name, MAX_ANIM_MARKER_NAME_LENGTH ) );
	L << "\n\tframe = " << marker.frame;
	return L;
}


ENGINE_EXPORT void SGRX_Cull_Camera_Prepare( SGRX_Scene* S );
ENGINE_EXPORT uint32_t SGRX_Cull_Camera_MeshList( Array< SGRX_MeshInstance* >& MIBuf, ByteArray& scratchMem, SGRX_Scene* S );
ENGINE_EXPORT uint32_t SGRX_Cull_Camera_PointLightList( Array< SGRX_Light* >& LIBuf, ByteArray& scratchMem, SGRX_Scene* S );
ENGINE_EXPORT uint32_t SGRX_Cull_Camera_SpotLightList( Array< SGRX_Light* >& LIBuf, ByteArray& scratchMem, SGRX_Scene* S );
ENGINE_EXPORT void SGRX_Cull_SpotLight_Prepare( SGRX_Scene* S, SGRX_Light* L );
ENGINE_EXPORT uint32_t SGRX_Cull_SpotLight_MeshList( Array< SGRX_MeshInstance* >& MIBuf, ByteArray& scratchMem, SGRX_Scene* S, SGRX_Light* L );


///
/// RENDERER
///

struct RenderItem
{
	uint64_t key;
	SGRX_MeshInstance* MI;
	uint32_t part_id;
	
	FINLINE bool IsSolid() const { return ( key >> 62 ) == 0; }
	FINLINE bool IsDecal() const { return ( key >> 62 ) == 1; }
	FINLINE bool IsTransparent() const { return ( key >> 62 ) == 2; }
};


struct PointLightData
{
	Vec3 viewPos;
	float range;
	Vec3 color;
	float power;
};

struct SpotLightDataPS
{
	Vec3 viewPos;
	float range;
	Vec3 color;
	float power;
	Vec3 viewDir;
	float angle;
	Vec2 SMSize;
	Vec2 invSMSize;
};

struct SpotLightDataVS
{
	Mat4 SMMatrix;
};

struct LightCount
{
	int numPL;
	int numSL;
};

ENGINE_EXPORT LightCount SGRX_Renderer_FindLights( const SGRX_Camera& CAM, SGRX_DrawItem* DI, int maxPL, int maxSL,
	PointLightData* outPL, SpotLightDataPS* outSL_PS, SpotLightDataVS* outSL_VS, SGRX_Light** outSL_LT );


struct RendererInfo
{
	bool swapRB;
	bool compileShaders;
	StringView shaderTarget;
};

enum EShaderType
{
	ShaderType_Vertex,
	ShaderType_Pixel,
};

struct IF_GCC(ENGINE_EXPORT) IRenderer : SGRX_IRenderControl
{
	ENGINE_EXPORT IRenderer();
	ENGINE_EXPORT virtual ~IRenderer();
	ENGINE_EXPORT virtual void Destroy() = 0;
	ENGINE_EXPORT virtual const RendererInfo& GetInfo() = 0;
	ENGINE_EXPORT virtual bool LoadInternalResources() = 0;
	ENGINE_EXPORT virtual void UnloadInternalResources() = 0;
	ENGINE_EXPORT virtual void Swap() = 0;
	ENGINE_EXPORT virtual void Modify( const RenderSettings& settings ) = 0;
	ENGINE_EXPORT virtual void SetCurrent() = 0;
	
	ENGINE_EXPORT virtual void SetRenderTargets( const SGRX_RTClearInfo& info, SGRX_IDepthStencilSurface* dss, TextureHandle rts[4] ) = 0;
	ENGINE_EXPORT virtual void SetViewport( int x0, int y0, int x1, int y1 ) = 0;
	ENGINE_EXPORT virtual void SetScissorRect( bool enable, int* rect ) = 0;
	
	ENGINE_EXPORT virtual SGRX_ITexture* CreateTexture( TextureInfo* texinfo, void* data = NULL ) = 0;
	ENGINE_EXPORT virtual SGRX_ITexture* CreateRenderTexture( TextureInfo* texinfo ) = 0;
	ENGINE_EXPORT virtual SGRX_IDepthStencilSurface* CreateDepthStencilSurface( int width, int height, int format ) = 0;
	ENGINE_EXPORT virtual bool CompileShader( const StringView& path, EShaderType shadertype, const StringView& code, ByteArray& outcomp, String& outerrors ) = 0;
	ENGINE_EXPORT virtual SGRX_IVertexShader* CreateVertexShader( const StringView& path, ByteArray& code ) = 0;
	ENGINE_EXPORT virtual SGRX_IPixelShader* CreatePixelShader( const StringView& path, ByteArray& code ) = 0;
	ENGINE_EXPORT virtual SGRX_IRenderState* CreateRenderState( const SGRX_RenderState& state ) = 0;
	ENGINE_EXPORT virtual SGRX_IVertexDecl* CreateVertexDecl( const VDeclInfo& vdinfo ) = 0;
	ENGINE_EXPORT virtual SGRX_IMesh* CreateMesh() = 0;
	ENGINE_EXPORT virtual SGRX_IVertexInputMapping* CreateVertexInputMapping( SGRX_IVertexShader* vs, SGRX_IVertexDecl* vd ) = 0;
	
	ENGINE_EXPORT virtual void SetMatrix( bool view, const Mat4& mtx ) = 0;
	ENGINE_EXPORT virtual void DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt,
		TextureHandle textures[ SGRX_MAX_TEXTURES ], SGRX_IPixelShader* shd, Vec4* shdata, size_t shvcount ) = 0;
	
	ENGINE_EXPORT virtual void DoRenderItems( SGRX_Scene* scene, uint8_t pass_id, int maxrepeat,
		const SGRX_Camera& cam, RenderItem* start, RenderItem* end ) = 0;
	
	// render control
	ENGINE_EXPORT virtual void SetRenderTargets( SGRX_IDepthStencilSurface* dss, const SGRX_RTClearInfo& info, TextureHandle rts[4] );
	ENGINE_EXPORT virtual void SortRenderItems( SGRX_Scene* scene );
	ENGINE_EXPORT virtual void RenderShadows( SGRX_Scene* scene, uint8_t pass_id );
	ENGINE_EXPORT virtual void RenderTypes( SGRX_Scene* scene, uint8_t pass_id, int maxrepeat, uint8_t types );
	ENGINE_EXPORT virtual void DrawRenderTargets( uint16_t ids[4] );
	
	// render queue helpers
	ENGINE_EXPORT uint64_t _RS_GenSortKey( const Mat4& view, SGRX_MeshInstance* MI, uint32_t part_id );
	ENGINE_EXPORT void _RS_LoadInstItems( const Mat4& view, int slot, Array<SGRX_MeshInstance*>& insts, uint8_t flags );
	
	// culling helpers
	ENGINE_EXPORT void _RS_PreProcess( SGRX_Scene* scene );
	ENGINE_EXPORT void _RS_Cull_Camera_Prepare( SGRX_Scene* scene );
	ENGINE_EXPORT uint32_t _RS_Cull_Camera_MeshList( SGRX_Scene* scene );
	ENGINE_EXPORT uint32_t _RS_Cull_Camera_PointLightList( SGRX_Scene* scene );
	ENGINE_EXPORT uint32_t _RS_Cull_Camera_SpotLightList( SGRX_Scene* scene );
	ENGINE_EXPORT uint32_t _RS_Cull_SpotLight_MeshList( SGRX_Scene* scene, SGRX_Light* L );
	ENGINE_EXPORT void _RS_Compile_MeshLists( SGRX_Scene* scene );
	
	// common data
	RenderStats m_stats;
	RenderSettings m_currSettings;
	
	bool m_inDebugDraw;
	
	// culling data
	ByteArray m_scratchMem;
	Array< SGRX_MeshInstance* > m_visible_meshes;
	Array< SGRX_MeshInstance* > m_visible_spot_meshes;
	Array< SGRX_Light* > m_visible_point_lights;
	Array< SGRX_Light* > m_visible_spot_lights;
	Array< SGRX_DrawItemLight > m_inst_light_buf;
	Array< SGRX_DrawItemLight > m_light_inst_buf;
	
	// sorting data
	Array< RenderItem > m_renderItemsBase;
	Array< RenderItem > m_renderItemsAux;
	RenderItem* m_riBaseStart;
	RenderItem* m_riBaseSD;
	RenderItem* m_riBaseDT;
	RenderItem* m_riBaseEnd;
	
	// projector interface
	bool _RS_ProjectorInit();
	void _RS_ProjectorFree();
	bool _RS_UpdateProjectorMesh( SGRX_Scene* scene );
	
	// projector data
	ByteArray m_projectorVertices;
	UInt32Array m_projectorIndices;
	Array< SGRX_MeshPart > m_projectorMeshParts;
	Array< SGRX_Light* > m_projectorList;
	MeshHandle m_projectorMesh;
	VertexDeclHandle m_projectorVertexDecl;
};
typedef bool (*pfnRndInitialize) ( const char** );
typedef void (*pfnRndFree) ();
typedef IRenderer* (*pfnRndCreateRenderer) ( const RenderSettings&, void* );

