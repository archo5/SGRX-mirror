

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#ifndef __in
#define __in
#define __out
#define __inout
#define __in_opt
#define __out_opt
#define __inout_opt
#define __in_bcount(x)
#define __out_bcount(x)
#define __in_bcount_opt(x)
#define __out_bcount_opt(x)
#define __in_ecount_opt(x)
#define __out_ecount_opt(x)
#define __in_ecount(x)
#define __out_ecount(x)
#define __in_range(x,y)
#endif
#include <windows.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#undef DXGI_ERROR_INVALID_CALL // GCC sucks
#undef DXGI_ERROR_NOT_FOUND
#undef DXGI_ERROR_MORE_DATA
#undef DXGI_ERROR_UNSUPPORTED
#undef DXGI_ERROR_DEVICE_REMOVED
#undef DXGI_ERROR_DEVICE_HUNG
#undef DXGI_ERROR_DEVICE_RESET
#undef DXGI_ERROR_WAS_STILL_DRAWING
#undef DXGI_ERROR_FRAME_STATISTICS_DISJOINT
#undef DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_ERROR_DRIVER_INTERNAL_ERROR
#undef DXGI_ERROR_NONEXCLUSIVE
#undef DXGI_ERROR_NOT_CURRENTLY_AVAILABLE
#undef DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED
#undef DXGI_ERROR_REMOTE_OUTOFMEMORY
#include <d3d11.h>
#ifdef ENABLE_SHADER_COMPILING
#  include <d3dcompiler.h>
#endif
#pragma GCC diagnostic pop

#include "renderer.hpp"


ALIGN16(struct) cb_objpass_core_data // b0
{
	Mat4 mView;
	Mat4 mProj;
	Mat4 mInvView;
	Vec3 gCameraPos;
	Vec4 timeVals;
	
	Vec3 gAmbLightColor;
	Vec3 gDirLightDir;
	Vec3 gDirLightColor;
};

ALIGN16(struct) cb_objpass_instance_data // b1
{
	Mat4 mWorld;
	Mat4 mWorldView;
	Vec2 gLightCounts;
	Vec4 gInstanceData[16];
};

ALIGN16(struct) cb_objpass_skin_matrices // b2
{
	Mat4 mSkin[32];
};

// array of these is b3
ALIGN16(struct) cb_objpass_point_light
{
	Vec3 viewPos;
	float range;
	Vec3 color;
	float power;
};

// array of these is b4
ALIGN16(struct) cb_objpass_spotlight
{
	Vec3 viewPos;
	float range;
	Vec3 color;
	float power;
	Vec3 viewDir;
	float angle;
	Vec2 smSize;
	Vec2 smInvSize;
};


#define SAFE_RELEASE( x ) if( x ){ (x)->Release(); x = NULL; }


#define RENDERER_EXPORT __declspec(dllexport)


const GUID g_ID3D11Texture2D = {0x6f15aaf2, 0xd208, 0x4e89, {0x9a,0xb4, 0x48,0x95,0x35,0xd3,0x4f,0x9c}};


static DXGI_FORMAT texfmt2d3d( int fmt )
{
	switch( fmt )
	{
	case TEXFORMAT_BGRX8: return DXGI_FORMAT_B8G8R8X8_TYPELESS;
	case TEXFORMAT_BGRA8: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case TEXFORMAT_RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TEXFORMAT_R5G6B5: return DXGI_FORMAT_B5G6R5_UNORM;
	
	case TEXFORMAT_DXT1: return DXGI_FORMAT_BC1_UNORM;
	case TEXFORMAT_DXT3: return DXGI_FORMAT_BC2_UNORM;
	case TEXFORMAT_DXT5: return DXGI_FORMAT_BC3_UNORM;
	}
	return (DXGI_FORMAT) 0;
}

static int create_rstate( ID3D11Device* device, D3D11_RASTERIZER_DESC* rdesc, ID3D11RasterizerState** out )
{
	HRESULT hr = device->CreateRasterizerState( rdesc, out );
	if( FAILED( hr ) || !*out )
	{
		LOG_ERROR << "Failed to create D3D11 rasterizer state";
		return -1;
	}
	return 0;
}

static int create_blendstate( ID3D11Device* device, D3D11_BLEND_DESC* rdesc, ID3D11BlendState** out )
{
	HRESULT hr = device->CreateBlendState( rdesc, out );
	if( FAILED( hr ) || !*out )
	{
		LOG_ERROR << "Failed to create D3D11 blend state";
		return -1;
	}
	return 0;
}

static int create_dsstate( ID3D11Device* device, D3D11_DEPTH_STENCIL_DESC* dsdesc, ID3D11DepthStencilState** out )
{
	HRESULT hr = device->CreateDepthStencilState( dsdesc, out );
	if( FAILED( hr ) || !*out )
	{
		LOG_ERROR << "Failed to create D3D11 depth stencil state";
		return -1;
	}
	return 0;
}

static int create_sampstate( ID3D11Device* device, D3D11_SAMPLER_DESC* rdesc, ID3D11SamplerState** out )
{
	HRESULT hr = device->CreateSamplerState( rdesc, out );
	if( FAILED( hr ) || !*out )
	{
		LOG_ERROR << "Failed to create D3D11 sampler state";
		return -1;
	}
	return 0;
}

static int create_rtt_( ID3D11Device* device, int width, int height, int msamples, DXGI_FORMAT fmt, bool ds, ID3D11Texture2D** outtex, void** outview )
{
	const char* what = ds ? "depth/stencil" : "render target";
	
	D3D11_TEXTURE2D_DESC dtd;
	memset( &dtd, 0, sizeof(dtd) );
	
	dtd.Width = width;
	dtd.Height = height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = fmt;
	dtd.SampleDesc.Count = msamples > 1 ? TMAX( 1, TMIN( 16, msamples ) ) : 1;
	dtd.SampleDesc.Quality = msamples > 1 ? 1 : 0;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = ds ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
	
	HRESULT hr = device->CreateTexture2D( &dtd, NULL, outtex );
	if( FAILED( hr ) || *outtex == NULL )
	{
		LOG_ERROR << "Failed to create D3D11 " << what << " texture (HRESULT=" << (uint32_t) hr << ", w=" << width
			<< ", h=" << height << ", ms=" << msamples << ", fmt=" << fmt << ", ds=" << ds << ")";
		return -1;
	}
	
	if( ds )
		hr = device->CreateDepthStencilView( *outtex, NULL, (ID3D11DepthStencilView**) outview );
	else
		hr = device->CreateRenderTargetView( *outtex, NULL, (ID3D11RenderTargetView**) outview );
	if( FAILED( hr ) || *outview == NULL )
	{
		SAFE_RELEASE( *outtex );
		LOG_ERROR << "Failed to create D3D11 " << what << " view";
		return -1;
	}
	
	return 0;
}
#define create_rtt( dev, w, h, ms, fmt, ds, outtex, outview ) create_rtt_( dev, w, h, ms, fmt, ds, outtex, (void**) outview )

static int create_buf( ID3D11Device* device, size_t numbytes, bool dyn, D3D11_BIND_FLAG bindtype, const void* data, ID3D11Buffer** out )
{
	const char* what = "?";
	if( bindtype == D3D11_BIND_VERTEX_BUFFER ) what = "vertex";
	if( bindtype == D3D11_BIND_INDEX_BUFFER ) what = "index";
	if( bindtype == D3D11_BIND_CONSTANT_BUFFER ) what = "constant";
	
	D3D11_BUFFER_DESC dbd;
	memset( &dbd, 0, sizeof(dbd) );
	
	dbd.ByteWidth = numbytes;
	dbd.Usage = dyn ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	dbd.BindFlags = bindtype;
	dbd.CPUAccessFlags = dyn ? D3D11_CPU_ACCESS_WRITE : 0;
	
	D3D11_SUBRESOURCE_DATA srd;
	memset( &srd, 0, sizeof(srd) );
	srd.pSysMem = data;
	
	HRESULT hr = device->CreateBuffer( &dbd, data ? &srd : NULL, out );
	if( FAILED( hr ) || *out == NULL )
	{
		LOG_ERROR << "Failed to create D3D11 " << what << " buffer (size=" << numbytes <<", dyn=" << dyn << ")";
		return -1;
	}
	
	return 0;
}

static int upload_buf( ID3D11DeviceContext* ctx, ID3D11Buffer* buf, bool dyn, bool discard, const void* data, size_t size )
{
	if( dyn )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		HRESULT hr = ctx->Map( buf, 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &msr );
		if( FAILED( hr ) )
		{
			LOG_ERROR << "failed to lock D3D11 buffer";
			return -1;
		}
		
		memcpy( msr.pData, data, size );
		
		ctx->Unmap( buf, 0 );
	}
	else
	{
		ctx->UpdateSubresource( buf, 0, NULL, data, size, 0 );
	}
	return 0;
}


struct D3D11DynBuffer
{
	D3D11DynBuffer() : buffer( NULL ), currSize( 0 ){}
	~D3D11DynBuffer()
	{
		if( buffer )
			LOG << "UNFREED DYNAMIC BUFFER";
	}
	void Free()
	{
		SAFE_RELEASE( buffer );
	}
	void Upload( ID3D11Device* dev, ID3D11DeviceContext* ctx, D3D11_BIND_FLAG bindtype, const void* data, size_t size )
	{
		if( !size )
			return;
		if( size > currSize )
		{
			SAFE_RELEASE( buffer );
			create_buf( dev, size, true, bindtype, data, &buffer );
			currSize = size;
		}
		else
			upload_buf( ctx, buffer, true, true, data, size );
	}
	
	operator ID3D11Buffer* () const { return buffer; }
	ID3D11Buffer** PPBuf() { return &buffer; }
	
	ID3D11Buffer* buffer;
	size_t currSize;
	
private:
	D3D11DynBuffer( const D3D11DynBuffer& );
	D3D11DynBuffer& operator = ( const D3D11DynBuffer& );
};


struct BackupVertexData
{
	Vec4 color;
	Vec4 tangent;
	Vec4 tex[4];
};
static BackupVertexData g_initial_backup_vertex_data = {0};


struct D3D11Texture : SGRX_ITexture
{
	union
	{
		ID3D11Texture2D* tex2d;
		ID3D11Texture3D* tex3d;
		ID3D11Resource* res;
	}
	m_ptr;
	ID3D11SamplerState* m_sampState;
	ID3D11ShaderResourceView* m_rsrcView;
	struct D3D11Renderer* m_renderer;
	
	D3D11Texture( bool isRenderTexture = false ) : m_sampState(NULL), m_rsrcView(NULL), m_renderer(NULL)
	{
		m_isRenderTexture = isRenderTexture;
	}
	virtual ~D3D11Texture();
	
	bool UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h );
};

struct D3D11RenderTexture : D3D11Texture
{
	// color texture (CT) = m_ptr.tex2d
	ID3D11RenderTargetView* CRV; /* color (output 0) RT view */
	ID3D11Texture2D* DT; /* depth/stencil texture */
	ID3D11DepthStencilView* DSV; /* depth/stencil view */
	int format;
	
	D3D11RenderTexture() : D3D11Texture(true){}
	virtual ~D3D11RenderTexture()
	{
		SAFE_RELEASE( CRV );
		SAFE_RELEASE( DT );
		SAFE_RELEASE( DSV );
	}
};


struct D3D11VertexShader : SGRX_IVertexShader
{
	ID3D11VertexShader* m_VS;
	ByteArray m_VSBC; // bytecode
	struct D3D11Renderer* m_renderer;
	
	D3D11VertexShader( struct D3D11Renderer* r ) : m_VS( NULL ), m_renderer( r ){}
	~D3D11VertexShader()
	{
		SAFE_RELEASE( m_VS );
	}
};

struct D3D11PixelShader : SGRX_IPixelShader
{
	ID3D11PixelShader* m_PS;
	ByteArray m_PSBC; // bytecode
	struct D3D11Renderer* m_renderer;
	
	D3D11PixelShader( struct D3D11Renderer* r ) : m_PS( NULL ), m_renderer( r ){}
	~D3D11PixelShader()
	{
		SAFE_RELEASE( m_PS );
	}
};


struct D3D11RenderState : SGRX_IRenderState
{
	ID3D11RasterizerState* m_RS;
	ID3D11BlendState* m_BS;
	ID3D11DepthStencilState* m_DS;
	struct D3D11Renderer* m_renderer;
	
	D3D11RenderState( struct D3D11Renderer* r ) : m_RS( NULL ), m_BS( NULL ), m_DS( NULL ), m_renderer( r ){}
	~D3D11RenderState()
	{
		SAFE_RELEASE( m_RS );
		SAFE_RELEASE( m_BS );
		SAFE_RELEASE( m_DS );
	}
	virtual void SetState( const SGRX_RenderState& state );
};


struct D3D11VertexDecl : SGRX_IVertexDecl
{
	D3D11_INPUT_ELEMENT_DESC m_elements[ VDECL_MAX_ITEMS ];
	int m_elemCount;
	struct D3D11Renderer* m_renderer;
};


struct D3D11Mesh : SGRX_IMesh
{
	struct InputLayouts
	{
		ID3D11InputLayout* basic;
		ID3D11InputLayout* skin;
		
		FINLINE void Reset(){ basic = NULL; skin = NULL; }
	};
	
	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;
	struct D3D11Renderer* m_renderer;
	int m_origVertexSize;
	int m_realVertexSize;
	size_t m_realVertexDataSize;
	
	Array< InputLayouts > m_inputLayouts;
	
	D3D11Mesh() : m_VB( NULL ), m_IB( NULL ), m_renderer( NULL ), m_origVertexSize(0), m_realVertexSize(0), m_realVertexDataSize(0)
	{
	}
	~D3D11Mesh();
	
	bool InitVertexBuffer( size_t size, VertexDeclHandle vd );
	bool InitIndexBuffer( size_t size, bool i32 );
	bool UpdateVertexData( const void* data, size_t size, bool tristrip );
	bool UpdateIndexData( const void* data, size_t size );
	bool SetPartData( SGRX_MeshPart* parts, int count );
	
	void _UpdatePartInputLayouts();
};


struct RTOutInfo
{
	// TODO
	int unused;
};


RendererInfo g_D3D11RendererInfo =
{
	false, // swap R/B
	true, // compile shaders
	"d3d11", // shader type
};

struct D3D11Renderer : IRenderer
{
	struct cb_vs_batchverts
	{
		cb_vs_batchverts() : world( Mat4::Identity ), view( Mat4::Identity ){}
		Mat4 world;
		Mat4 view;
	};
	
	D3D11Renderer() : m_dbg_rt( false ){}
	void Destroy();
	const RendererInfo& GetInfo(){ return g_D3D11RendererInfo; }
	bool LoadInternalResources();
	void UnloadInternalResources();
	
	void Swap();
	void Modify( const RenderSettings& settings );
	void SetCurrent(){} // does nothing since there's no thread context pointer
	
	bool SetRenderTarget( TextureHandle rt );
	void Clear( float* color_v4f, bool clear_zbuffer );
	void SetViewport( int x0, int y0, int x1, int y1 );
	void SetScissorRect( bool enable, int* rect );
	
	SGRX_ITexture* CreateTexture( TextureInfo* texinfo, void* data = NULL );
	SGRX_ITexture* CreateRenderTexture( TextureInfo* texinfo );
	bool CompileShader( const StringView& path, EShaderType shadertype, const StringView& code, ByteArray& outcomp, String& outerrors );
	SGRX_IVertexShader* CreateVertexShader( const StringView& path, ByteArray& code );
	SGRX_IPixelShader* CreatePixelShader( const StringView& path, ByteArray& code );
	SGRX_IRenderState* CreateRenderState( const SGRX_RenderState& state );
	SGRX_IVertexDecl* CreateVertexDecl( const VDeclInfo& vdinfo );
	SGRX_IMesh* CreateMesh();
	
	void SetMatrix( bool view, const Mat4& mtx );
	void DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt, SGRX_ITexture* tex, SGRX_IPixelShader* shd, Vec4* shdata, size_t shvcount );
	
	bool SetRenderPasses( SGRX_RenderPass* passes, int count );
	void RenderScene( SGRX_RenderScene* RS );
//	void _RS_Render_Shadows();
	void _RS_RenderPass_Object( const SGRX_RenderPass& pass, size_t pass_id );
	void _RS_RenderPass_Screen( const SGRX_RenderPass& pass, size_t pass_id, /*IDirect3DBaseTexture9* tx_depth,*/ const RTOutInfo& RTOUT );
//	void _RS_DebugDraw( SGRX_DebugDraw* debugDraw, IDirect3DSurface9* test_dss, IDirect3DSurface9* orig_dss );
	
	void PostProcBlit( int w, int h, int downsample, int ppdata_location );
	
	bool ResetDevice();
	void ResetViewport();
//	void _SetTextureInt( int slot, IDirect3DBaseTexture9* tex, uint32_t flags );
	void SetTexture( int slot, SGRX_ITexture* tex );
	void SetVertexShader( SGRX_IVertexShader* shd );
	void SetPixelShader( SGRX_IPixelShader* shd );
	
//	FINLINE void VS_SetVec4Array( int at, Vec4* arr, int size ){ m_dev->SetVertexShaderConstantF( at, &arr->x, size ); }
//	FINLINE void VS_SetVec4Array( int at, float* arr, int size ){ m_dev->SetVertexShaderConstantF( at, arr, size ); }
//	FINLINE void VS_SetVec4( int at, const Vec4& v ){ m_dev->SetVertexShaderConstantF( at, &v.x, 1 ); }
//	FINLINE void VS_SetFloat( int at, float f ){ Vec4 v = {f,f,f,f}; m_dev->SetVertexShaderConstantF( at, &v.x, 1 ); }
//	FINLINE void VS_SetMat4( int at, const Mat4& v ){ m_dev->SetVertexShaderConstantF( at, v.a, 4 ); }
//	
//	FINLINE void PS_SetVec4Array( int at, Vec4* arr, int size ){ m_dev->SetPixelShaderConstantF( at, &arr->x, size ); }
//	FINLINE void PS_SetVec4Array( int at, float* arr, int size ){ m_dev->SetPixelShaderConstantF( at, arr, size ); }
//	FINLINE void PS_SetVec4( int at, const Vec4& v ){ m_dev->SetPixelShaderConstantF( at, &v.x, 1 ); }
//	FINLINE void PS_SetFloat( int at, float f ){ Vec4 v = {f,f,f,f}; m_dev->SetPixelShaderConstantF( at, &v.x, 1 ); }
//	FINLINE void PS_SetMat4( int at, const Mat4& v ){ m_dev->SetPixelShaderConstantF( at, v.a, 4 ); }
//	
//	FINLINE int GetWidth() const { return m_params.BackBufferWidth; }
//	FINLINE int GetHeight() const { return m_params.BackBufferHeight; }
	
	void MI_ApplyConstants( SGRX_MeshInstance* MI );
	void Viewport_Apply( int downsample );
	
	// state
	TextureHandle m_currentRT;
	bool m_dbg_rt;
	
	// helpers
//	RTData m_drd;
	
	SGRX_IVertexShader* m_sh_bv_vs;
	SGRX_IPixelShader* m_sh_bv_ps;
	SGRX_IVertexShader* m_sh_pp_vs;
	SGRX_IPixelShader* m_sh_pp_final;
	SGRX_IPixelShader* m_sh_pp_dshp;
	SGRX_IPixelShader* m_sh_pp_blur_h;
	SGRX_IPixelShader* m_sh_pp_blur_v;
	Array< PixelShaderHandle > m_pass_shaders;
	
	// storage
	HashTable< D3D11Texture*, bool > m_ownTextures;
	HashTable< D3D11Mesh*, bool > m_ownMeshes;
	
	// specific
	ID3D11Device* m_dev;
	ID3D11DeviceContext* m_ctx;
	IDXGISwapChain* m_swapChain;
	ID3D11Texture2D* m_backBuffer;
	ID3D11Texture2D* m_depthBuffer;
	ID3D11RenderTargetView* m_rtView;
	ID3D11DepthStencilView* m_dsView;
	
	// rendering data
	D3D11Texture* m_defaultTexture;
	ID3D11RasterizerState* m_rstate_batchverts;
	ID3D11BlendState* m_bstate_batchverts_normal;
	ID3D11BlendState* m_bstate_batchverts_additive;
	ID3D11InputLayout* m_inputLayout_batchverts;
	ID3D11Buffer* m_vertbuf_defaults;
	cb_vs_batchverts m_cbdata_vs_batchverts;
	ID3D11Buffer* m_cbuf_vs_batchverts;
	D3D11DynBuffer m_vertbuf_batchverts;
	D3D11DynBuffer m_cbuf_ps_batchverts;
	ID3D11Buffer* m_cbuf0_common;
	ID3D11Buffer* m_cbuf1_inst;
	ID3D11Buffer* m_cbuf2_skin;
	D3D11DynBuffer m_cbuf3_ltpoint;
	D3D11DynBuffer m_cbuf4_ltspot;
	
	// temp data
	SceneHandle m_currentScene;
	bool m_enablePostProcessing;
	SGRX_Viewport* m_viewport;
	ByteArray m_scratchMem;
//	Array< SGRX_MeshInstance* > m_visible_meshes;
//	Array< SGRX_MeshInstance* > m_visible_spot_meshes;
//	Array< SGRX_Light* > m_visible_point_lights;
//	Array< SGRX_Light* > m_visible_spot_lights;
//	Array< SGRX_MeshInstLight > m_inst_light_buf;
//	Array< SGRX_MeshInstLight > m_light_inst_buf;
};

extern "C" RENDERER_EXPORT bool Initialize( const char** outname )
{
	*outname = "Direct3D11";
	return true;
}

extern "C" RENDERER_EXPORT void Free()
{
}

extern "C" RENDERER_EXPORT IRenderer* CreateRenderer( const RenderSettings& settings, void* windowHandle )
{
	HRESULT hr;
	ID3D11Device* device = NULL;
	ID3D11DeviceContext* context = NULL;
	IDXGISwapChain* swapChain = NULL;
	ID3D11Texture2D* backBuffer = NULL;
	ID3D11Texture2D* depthBuffer = NULL;
	ID3D11RenderTargetView* rtView = NULL;
	ID3D11DepthStencilView* dsView = NULL;
	
	int msamples = settings.aa_mode == ANTIALIAS_MULTISAMPLE ? TMAX( 1, TMIN( 16, settings.aa_quality ) ) : 1;
	
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	memset( &swapChainDesc, 0, sizeof(swapChainDesc) );
	
	swapChainDesc.BufferDesc.Width = settings.width;
	swapChainDesc.BufferDesc.Height = settings.height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = settings.fullscreen != FULLSCREEN_NORMAL ? 0 : settings.refresh_rate;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // DXGI_FORMAT_B8G8R8A8_UNORM ?
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	swapChainDesc.SampleDesc.Count = msamples;
	swapChainDesc.SampleDesc.Quality = settings.aa_mode == ANTIALIAS_MULTISAMPLE ? 1 : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1; // 2?
	swapChainDesc.OutputWindow = (HWND) windowHandle;
	swapChainDesc.Windowed = settings.fullscreen != FULLSCREEN_NORMAL;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	hr = D3D11CreateDeviceAndSwapChain(
		NULL, // adapter
		D3D_DRIVER_TYPE_HARDWARE,
		NULL, // software rasterizer (unused)
		D3D11_CREATE_DEVICE_DEBUG, // flags
		NULL, // feature levels
		0, // ^^
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain,
		&device,
		NULL,
		&context
	);
	if( FAILED( hr ) )
	{
		LOG_ERROR << "Failed to create the D3D11 device";
		return NULL;
	}
	
	// Backbuffer
	hr = swapChain->GetBuffer( 0, g_ID3D11Texture2D, (void**) &backBuffer );
	if( FAILED( hr ) || backBuffer == NULL )
	{
		LOG_ERROR << "Failed to retrieve D3D11 backbuffer";
		return NULL;
	}
	
	hr = device->CreateRenderTargetView( backBuffer, NULL, &rtView );
	if( FAILED( hr ) || rtView == NULL )
	{
		LOG_ERROR << "Failed to create D3D11 render target view";
		return NULL;
	}
	
	// Depth/stencil buffer
	if( create_rtt( device, settings.width, settings.height, msamples, DXGI_FORMAT_D24_UNORM_S8_UINT, true, &depthBuffer, &dsView ) )
		return NULL;
	
	context->OMSetRenderTargets( 1, &rtView, NULL );
	
	D3D11Renderer* R = new D3D11Renderer;
	R->m_currSettings = settings;
	R->m_dev = device;
	R->m_ctx = context;
	R->m_swapChain = swapChain;
	R->m_backBuffer = backBuffer;
	R->m_depthBuffer = depthBuffer;
	R->m_rtView = rtView;
	R->m_dsView = dsView;
	
	// default vertex data
	if( create_buf( device, sizeof(g_initial_backup_vertex_data), false, D3D11_BIND_VERTEX_BUFFER, &g_initial_backup_vertex_data, &R->m_vertbuf_defaults ) )
		return NULL;
	
	// batch vertex constant buffer data
	if( create_buf( device, sizeof(R->m_cbdata_vs_batchverts), true, D3D11_BIND_CONSTANT_BUFFER, &R->m_cbdata_vs_batchverts, &R->m_cbuf_vs_batchverts ) )
		return NULL;
	
	// batch vertex rasterizer state
	D3D11_RASTERIZER_DESC rdesc_batchverts = { D3D11_FILL_SOLID, D3D11_CULL_NONE, TRUE, 0, 0, 0, TRUE, TRUE, TRUE, TRUE };
	if( create_rstate( device, &rdesc_batchverts, &R->m_rstate_batchverts ) )
		return NULL;
	
	// batch vertex blending states
	D3D11_BLEND_DESC bdesc_batchverts = { FALSE, FALSE, {
		{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, 0xf }
	} };
	if( create_blendstate( device, &bdesc_batchverts, &R->m_bstate_batchverts_normal ) )
		return NULL;
	bdesc_batchverts.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	if( create_blendstate( device, &bdesc_batchverts, &R->m_bstate_batchverts_additive ) )
		return NULL;
	
	// pass constant buffers
	if( create_buf( device, sizeof(cb_objpass_core_data), true, D3D11_BIND_CONSTANT_BUFFER, NULL, &R->m_cbuf0_common ) ) return NULL;
	if( create_buf( device, sizeof(cb_objpass_instance_data), true, D3D11_BIND_CONSTANT_BUFFER, NULL, &R->m_cbuf1_inst ) ) return NULL;
	if( create_buf( device, sizeof(cb_objpass_skin_matrices), true, D3D11_BIND_CONSTANT_BUFFER, NULL, &R->m_cbuf2_skin ) ) return NULL;
	
	// initial viewport settings
	R->SetViewport( 0, 0, settings.width, settings.height );
	R->SetScissorRect( 0, NULL );
	
	return R;
}

void D3D11Renderer::Destroy()
{
	SAFE_RELEASE( m_cbuf0_common );
	SAFE_RELEASE( m_cbuf1_inst );
	SAFE_RELEASE( m_cbuf2_skin );
	m_cbuf3_ltpoint.Free();
	m_cbuf4_ltspot.Free();
	
	SAFE_RELEASE( m_vertbuf_defaults );
	SAFE_RELEASE( m_cbuf_vs_batchverts );
	m_vertbuf_batchverts.Free();
	m_cbuf_ps_batchverts.Free();
	SAFE_RELEASE( m_rstate_batchverts );
	SAFE_RELEASE( m_bstate_batchverts_normal );
	SAFE_RELEASE( m_bstate_batchverts_additive );
	
	SAFE_RELEASE( m_dsView );
	SAFE_RELEASE( m_rtView );
	SAFE_RELEASE( m_depthBuffer );
	SAFE_RELEASE( m_backBuffer );
	SAFE_RELEASE( m_swapChain );
	SAFE_RELEASE( m_ctx );
	SAFE_RELEASE( m_dev );
	delete this;
}

bool D3D11Renderer::LoadInternalResources()
{
	// default texture (white)
	TextureInfo tinfo = { TEXTYPE_2D, 1, 1, 1, 1, TEXFORMAT_RGBA8, 0 };
	uint32_t tdata = 0xffffffff;
	m_defaultTexture = (D3D11Texture*) CreateTexture( &tinfo, &tdata );
	if( m_defaultTexture == NULL )
		return NULL;
	
	// shaders
	VertexShaderHandle sh_bv_vs = GR_GetVertexShader( "sys_bv_vs" );
	PixelShaderHandle sh_bv_ps = GR_GetPixelShader( "sys_bv_ps" );
	VertexShaderHandle sh_pp_vs = GR_GetVertexShader( "sys_pp_vs" );
	PixelShaderHandle sh_pp_final = GR_GetPixelShader( "sys_pp_final" );
	PixelShaderHandle sh_pp_dshp = GR_GetPixelShader( "sys_pp_bloom_dshp" );
	PixelShaderHandle sh_pp_blur_h = GR_GetPixelShader( "sys_pp_bloom_blur_h" );
	PixelShaderHandle sh_pp_blur_v = GR_GetPixelShader( "sys_pp_bloom_blur_v" );
	if( !sh_bv_vs ||
		!sh_bv_ps ||
		!sh_pp_vs ||
		!sh_pp_final ||
		!sh_pp_dshp ||
		!sh_pp_blur_h ||
		!sh_pp_blur_v )
	{
		return false;
	}
	sh_bv_vs->Acquire();
	sh_bv_ps->Acquire();
	sh_pp_vs->Acquire();
	sh_pp_final->Acquire();
	sh_pp_dshp->Acquire();
	sh_pp_blur_h->Acquire();
	sh_pp_blur_v->Acquire();
	m_sh_bv_vs = sh_bv_vs;
	m_sh_bv_ps = sh_bv_ps;
	m_sh_pp_vs = sh_pp_vs;
	m_sh_pp_final = sh_pp_final;
	m_sh_pp_dshp = sh_pp_dshp;
	m_sh_pp_blur_h = sh_pp_blur_h;
	m_sh_pp_blur_v = sh_pp_blur_v;
	
	SetVertexShader( m_sh_bv_vs );
	
	D3D11_INPUT_ELEMENT_DESC bv_elems[3] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	SGRX_CAST( D3D11VertexShader*, VS, m_sh_bv_vs );
	HRESULT hr = m_dev->CreateInputLayout( bv_elems, SGRX_ARRAY_SIZE(bv_elems), VS->m_VSBC.data(), VS->m_VSBC.size(), &m_inputLayout_batchverts );
	if( FAILED( hr ) || !m_inputLayout_batchverts )
	{
		LOG_ERROR << "Failed to create D3D11 batch vertex input layout";
		return false;
	}
	
	return true;
}

void D3D11Renderer::UnloadInternalResources()
{
	SetRenderPasses( NULL, 0 );
	
	SAFE_RELEASE( m_inputLayout_batchverts );
	
	m_defaultTexture->Release();
	m_sh_bv_vs->Release();
	m_sh_bv_ps->Release();
	m_sh_pp_vs->Release();
	m_sh_pp_final->Release();
	m_sh_pp_dshp->Release();
	m_sh_pp_blur_h->Release();
	m_sh_pp_blur_v->Release();
}

void D3D11Renderer::Swap()
{
	m_swapChain->Present( 0, 0 );
}

void D3D11Renderer::Modify( const RenderSettings& settings )
{
}


bool D3D11Renderer::SetRenderTarget( TextureHandle rt )
{
	return false;
}

void D3D11Renderer::Clear( float* color_v4f, bool clear_zbuffer )
{
	m_ctx->ClearRenderTargetView( m_rtView, color_v4f );
	if( clear_zbuffer )
		m_ctx->ClearDepthStencilView( m_dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
}

void D3D11Renderer::SetViewport( int x0, int y0, int x1, int y1 )
{
	D3D11_VIEWPORT vp = { x0, y0, x1 - x0, y1 - y0, 0.0f, 1.0f };
	m_ctx->RSSetViewports( 1, &vp );
}

void D3D11Renderer::SetScissorRect( bool enable, int* rect )
{
	if( enable )
	{
		D3D11_RECT rct = { rect[0], rect[1], rect[2], rect[3] };
		m_ctx->RSSetScissorRects( 1, &rct );
	}
	else
	{
		D3D11_RECT rct = { 0, 0, m_currSettings.width, m_currSettings.height };
		m_ctx->RSSetScissorRects( 1, &rct );
	}
}


D3D11Texture::~D3D11Texture()
{
	m_renderer->m_ownTextures.unset( this );
	SAFE_RELEASE( m_rsrcView );
	SAFE_RELEASE( m_sampState );
	SAFE_RELEASE( m_ptr.res );
}

bool D3D11Texture::UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h )
{
	// sure, we copied... nothing
	if( !w || !h )
		return true;
	
	bool dyn = false;
	if( dyn )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		
		bool whole = x == 0 && y == 0 && w == m_info.width && h == m_info.height;
		HRESULT hr = m_renderer->m_ctx->Map( m_ptr.res, mip, whole ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &msr );
		if( FAILED( hr ) )
		{
			LOG_ERROR << "failed to map D3D11 texture";
			return false;
		}
		*(uint8_t**)&msr.pData += msr.RowPitch * y + x * 4;
		
		for( int j = 0; j < h; ++j )
		{
			uint8_t* dst = (uint8_t*)msr.pData + msr.RowPitch * j;
			memcpy( dst, ((uint32_t*)data) + w * j, w * 4 );
	//		if( m_info.format == TEXFORMAT_RGBA8 )
	//			swap4b2ms( (uint32_t*) dst, w, 0xff0000, 16, 0xff, 16 );
		}
		
		m_renderer->m_ctx->Unmap( m_ptr.res, mip );
	}
	else
	{
		D3D11_BOX box = { x, y, 0, x + w, y + h, 1 };
		m_renderer->m_ctx->UpdateSubresource( m_ptr.res, mip, &box, data, w * 4, 0 );
	}
	
	return true;
}

SGRX_ITexture* D3D11Renderer::CreateTexture( TextureInfo* texinfo, void* data )
{
	HRESULT hr;
	// TODO: filter unsupported formats / dimensions
	
	if( texinfo->type == TEXTYPE_2D || texinfo->type == TEXTYPE_CUBE )
	{
		int at = 0, sides = texinfo->type == TEXTYPE_CUBE ? 6 : 1;
		ID3D11Texture2D* tex2d = NULL;
		ID3D11SamplerState* samp = NULL;
		ID3D11ShaderResourceView* srv = NULL;
		
		D3D11_TEXTURE2D_DESC dtd;
		memset( &dtd, 0, sizeof(dtd) );
		dtd.Width = texinfo->width;
		dtd.Height = texinfo->height;
		dtd.MipLevels = texinfo->mipcount;
		dtd.ArraySize = sides;
		dtd.Format = texfmt2d3d( texinfo->format );
		dtd.Usage = D3D11_USAGE_DEFAULT;
		dtd.CPUAccessFlags = 0;
		dtd.SampleDesc.Count = 1;
		dtd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		
		D3D11_SUBRESOURCE_DATA srd[ 128 ];
		memset( &srd, 0, sizeof(srd) );
		if( data )
		{
			TextureInfo mipinfo;
			for( int side = 0; side < sides; ++side )
			{
				for( int mip = 0; mip < texinfo->mipcount; ++mip )
				{
					size_t crs, crc;
					
					TextureInfo_GetMipInfo( texinfo, mip, &mipinfo );
					TextureInfo_GetCopyDims( &mipinfo, &crs, &crc );
					
					srd[ at ].pSysMem = (char*) data + TextureData_GetMipDataOffset( texinfo, 0, mip );
					srd[ at ].SysMemPitch = crs;
					at++;
				}
			}
		}
		
		hr = m_dev->CreateTexture2D( &dtd, data ? srd : NULL, &tex2d );
		if( FAILED( hr ) || !tex2d )
		{
			LOG_ERROR << "could not create D3D11 texture (type: 2D, w: " << texinfo->width << ", h: " <<
				texinfo->height << ", mips: " << texinfo->mipcount << ", fmt: " << texinfo->format << ", d3dfmt: " << texfmt2d3d( texinfo->format );
			return NULL;
		}
		
		hr = m_dev->CreateShaderResourceView( tex2d, NULL, &srv );
		if( FAILED( hr ) || !srv )
		{
			LOG_ERROR << "could not create D3D11 shader resource view for texture (type: 2D, w: " << texinfo->width << ", h: " <<
				texinfo->height << ", mips: " << texinfo->mipcount << ", fmt: " << texinfo->format << ", d3dfmt: " << texfmt2d3d( texinfo->format );
			SAFE_RELEASE( tex2d );
			return NULL;
		}
		
		D3D11_SAMPLER_DESC sdesc = {
			D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP,
			0, 1, D3D11_COMPARISON_NEVER, {0,0,0,0}, 0, D3D11_FLOAT32_MAX
		};
		if( create_sampstate( m_dev, &sdesc, &samp ) )
		{
			SAFE_RELEASE( tex2d );
			SAFE_RELEASE( srv );
			return NULL;
		}
		
		D3D11Texture* T = new D3D11Texture;
		T->m_renderer = this;
		T->m_info = *texinfo;
		T->m_ptr.tex2d = tex2d;
		T->m_sampState = samp;
		T->m_rsrcView = srv;
		m_ownTextures.set( T, true );
		return T;
	}
	
	LOG_ERROR << "TODO [reached a part of not-yet-defined behavior]";
	return NULL;
}


SGRX_ITexture* D3D11Renderer::CreateRenderTexture( TextureInfo* texinfo )
{
	int width = texinfo->width, height = texinfo->height, format = texinfo->format;
	
	DXGI_FORMAT d3dfmt;
	switch( format )
	{
	case RT_FORMAT_BACKBUFFER:
		d3dfmt = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case RT_FORMAT_DEPTH:
		d3dfmt = DXGI_FORMAT_D32_FLOAT;
		break;
	default:
		LOG_ERROR << "format ID was not recognized / supported: " << format;
		return NULL;
	}
	
	D3D11_SAMPLER_DESC sdesc = {
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP,
		0, 1, D3D11_COMPARISON_NEVER, {0,0,0,0}, 0, D3D11_FLOAT32_MAX
	};
	
	ID3D11Texture2D *CT = NULL, *DT = NULL;
	ID3D11RenderTargetView *CRV = NULL;
	ID3D11DepthStencilView *DSV = NULL;
	ID3D11SamplerState* SAMP = NULL;
	ID3D11ShaderResourceView* CSRV = NULL;
	D3D11RenderTexture* RT = NULL;
	
	if( format == RT_FORMAT_DEPTH )
	{
		if( create_rtt( m_dev, width, height, 0, d3dfmt, true, &DT, &DSV ) )
			goto cleanup;
		
		CT = DT;
		DT->AddRef();
	}
	else
	{
		if( create_rtt( m_dev, width, height, 0, d3dfmt, false, &CT, &CRV ) )
			goto cleanup;
		if( create_rtt( m_dev, width, height, 0, DXGI_FORMAT_D24_UNORM_S8_UINT, true, &DT, &DSV ) )
			goto cleanup;
	}
	
	// shader resource view
	{
		HRESULT hr = m_dev->CreateShaderResourceView( CT, NULL, &CSRV );
		if( FAILED( hr ) || !CSRV )
		{
			LOG_ERROR << "Failed to create D3D11 shader resource view for renderable texture";
			goto cleanup;
		}
	}
	
	// sampler state
	if( create_sampstate( m_dev, &sdesc, &SAMP ) )
	{
		goto cleanup;
	}
	
	RT = new D3D11RenderTexture;
	RT->m_renderer = this;
	RT->m_info = *texinfo;
	RT->m_ptr.tex2d = CT;
	RT->m_sampState = SAMP;
	RT->m_rsrcView = CSRV;
	RT->CRV = CRV;
	RT->DSV = DSV;
	RT->DT = DT;
	RT->DSV = DSV;
	m_ownTextures.set( RT, true );
	return RT;
	
cleanup:
	SAFE_RELEASE( CSRV );
	SAFE_RELEASE( SAMP );
	SAFE_RELEASE( CRV );
	SAFE_RELEASE( DSV );
	SAFE_RELEASE( DT );
	SAFE_RELEASE( CT );
	return NULL;
}

bool D3D11Renderer::CompileShader( const StringView& path, EShaderType shadertype, const StringView& code, ByteArray& outcomp, String& outerrors )
{
#ifdef ENABLE_SHADER_COMPILING
	HRESULT hr;
	ID3DBlob *outbuf = NULL, *outerr = NULL;
	
	static const D3D_SHADER_MACRO vsmacros[] = { { "VS", "1" }, { NULL, NULL } };
	static const D3D_SHADER_MACRO psmacros[] = { { "PS", "1" }, { NULL, NULL } };
	
	ByteWriter bw( &outcomp );
	
	const D3D_SHADER_MACRO* macros = NULL;
	const char* tyname = "unknown";
	const char* profile = "---";
	switch( shadertype )
	{
	case ShaderType_Vertex:
		macros = vsmacros;
		tyname = "vertex";
		profile = "vs_4_0";
		bw.marker( "CVSH\x7f", 5 );
		break;
	case ShaderType_Pixel:
		macros = psmacros;
		tyname = "pixel";
		profile = "ps_4_0";
		bw.marker( "CPSH\x7f", 5 );
		break;
	}
	
	hr = D3DCompile( code.data(), code.size(), StackPath( path ), macros, NULL, "main", profile, D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, &outbuf, &outerr );
	if( FAILED( hr ) )
	{
		if( outerr )
		{
			const char* errtext = (const char*) outerr->GetBufferPointer();
			outerrors.append( STRLIT_BUF( "Errors in " ) );
			outerrors.append( tyname, strlen( tyname ) );
			outerrors.append( STRLIT_BUF( " shader compilation:\n" ) );
			outerrors.append( errtext, TMIN( strlen( errtext ), (size_t) outerr->GetBufferSize() ) );
		}
		else
		{
			outerrors.append( STRLIT_BUF( "Unknown error in " ) );
			outerrors.append( tyname, strlen( tyname ) );
			outerrors.append( STRLIT_BUF( " shader compilation" ) );
		}
		
		SAFE_RELEASE( outbuf );
		SAFE_RELEASE( outerr );
		return false;
	}
	
	int32_t shsize = outbuf->GetBufferSize();
	bw << shsize;
	bw.memory( outbuf->GetBufferPointer(), shsize );
	
	SAFE_RELEASE( outbuf );
	SAFE_RELEASE( outerr );
	
	return true;
#else
	LOG << "<<< D3D11 SHADER COMPILATION IS NOT ALLOWED IN THIS BUILD >>>";
	LOG << "Uncompiled shader: " << path;
	return false;
#endif // ENABLE_SHADER_COMPILING
}

SGRX_IVertexShader* D3D11Renderer::CreateVertexShader( const StringView& path, ByteArray& code )
{
	HRESULT hr;
	ByteReader br( &code );
	br.marker( "CVSH\x7f", 5 );
	
	int32_t len = 0;
	br << len;
	if( br.error )
	{
		LOG_ERROR << "Failed to load bytecode for D3D11 vertex shader - " << path;
		return NULL;
	}
	
	uint8_t* buf = &code[ 9 ];
	ID3D11VertexShader* VS = NULL;
	D3D11VertexShader* out = NULL;
	
	hr = m_dev->CreateVertexShader( buf, len, NULL, &VS );
	if( FAILED( hr ) || !VS )
		{ LOG_ERROR << "Failed to create a D3D11 vertex shader - " << path; goto cleanup; }
	
	out = new D3D11VertexShader( this );
	out->m_VS = VS;
	out->m_VSBC.append( buf, len );
	return out;
	
cleanup:
	SAFE_RELEASE( VS );
	return NULL;
}

SGRX_IPixelShader* D3D11Renderer::CreatePixelShader( const StringView& path, ByteArray& code )
{
	HRESULT hr;
	ByteReader br( &code );
	br.marker( "CPSH\x7f", 5 );
	
	int32_t len = 0;
	br << len;
	if( br.error )
	{
		LOG_ERROR << "Failed to load bytecode for D3D11 pixel shader - " << path;
		return NULL;
	}
	
	uint8_t* buf = &code[ 9 ];
	ID3D11PixelShader* PS = NULL;
	D3D11PixelShader* out = NULL;
	
	hr = m_dev->CreatePixelShader( buf, len, NULL, &PS );
	if( FAILED( hr ) || !PS )
		{ LOG_ERROR << "Failed to create a D3D11 pixel shader - " << path; goto cleanup; }
	
	out = new D3D11PixelShader( this );
	out->m_PS = PS;
	out->m_PSBC.append( buf, len );
	return out;
	
cleanup:
	SAFE_RELEASE( PS );
	return NULL;
}


void D3D11RenderState::SetState( const SGRX_RenderState& state )
{
	static const D3D11_FILL_MODE fillModes[ 2 ] = { D3D11_FILL_SOLID, D3D11_FILL_WIREFRAME };
	static const D3D11_CULL_MODE cullModes[ 4 ] = { D3D11_CULL_NONE, D3D11_CULL_BACK, D3D11_CULL_FRONT, /**/ D3D11_CULL_NONE };
	static const D3D11_BLEND blendFactors[ 16 ] =
	{
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ONE,
		D3D11_BLEND_SRC_COLOR,
		D3D11_BLEND_INV_SRC_COLOR,
		D3D11_BLEND_DEST_COLOR,
		D3D11_BLEND_INV_DEST_COLOR,
		D3D11_BLEND_SRC_ALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_DEST_ALPHA,
		D3D11_BLEND_INV_DEST_ALPHA,
		D3D11_BLEND_BLEND_FACTOR,
		D3D11_BLEND_INV_BLEND_FACTOR,
		/**/
		D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_ONE,
	};
	static const D3D11_BLEND_OP blendOps[ 8 ] =
	{
		D3D11_BLEND_OP_ADD,
		D3D11_BLEND_OP_SUBTRACT,
		D3D11_BLEND_OP_REV_SUBTRACT,
		D3D11_BLEND_OP_MIN,
		D3D11_BLEND_OP_MAX,
		/**/
		D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD
	};
	static const D3D11_DEPTH_WRITE_MASK dwMasks[ 2 ] = { D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_DEPTH_WRITE_MASK_ALL };
	static const D3D11_COMPARISON_FUNC compFuncs[ 8 ] =
	{
		D3D11_COMPARISON_NEVER,
		D3D11_COMPARISON_ALWAYS,
		D3D11_COMPARISON_EQUAL,
		D3D11_COMPARISON_NOT_EQUAL,
		D3D11_COMPARISON_LESS,
		D3D11_COMPARISON_LESS_EQUAL,
		D3D11_COMPARISON_GREATER,
		D3D11_COMPARISON_GREATER_EQUAL,
	};
	static const D3D11_STENCIL_OP stencilOps[ 8 ] =
	{
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_ZERO,
		D3D11_STENCIL_OP_REPLACE,
		D3D11_STENCIL_OP_INVERT,
		D3D11_STENCIL_OP_INCR,
		D3D11_STENCIL_OP_DECR,
		D3D11_STENCIL_OP_INCR_SAT,
		D3D11_STENCIL_OP_DECR_SAT,
	};
	
	SAFE_RELEASE( m_RS );
	SAFE_RELEASE( m_BS );
	SAFE_RELEASE( m_DS );
	
	D3D11_RASTERIZER_DESC rdesc =
	{
		fillModes[ state.wireFill ],
		cullModes[ state.cullMode ],
		TRUE,
		state.depthBias, state.depthBiasClamp, state.slopeDepthBias,
		TRUE,
		state.scissorEnable,
		state.multisampleEnable,
		TRUE
	};
	if( create_rstate( m_renderer->m_dev, &rdesc, &m_RS ) )
	{
		// error, use default or something
	}
	
	// batch vertex blending states
	D3D11_BLEND_DESC bdesc = { TRUE, state.separateBlend };
	for( int i = 0; i < SGRX_RS_MAX_RENDER_TARGETS; ++i )
	{
		const SGRX_RenderState::BlendState& bs = state.blendStates[ i ];
		D3D11_RENDER_TARGET_BLEND_DESC tbdesc =
		{
			bs.blendEnable,
			blendFactors[ bs.srcBlend ], blendFactors[ bs.dstBlend ], blendOps[ bs.blendOp ],
			blendFactors[ bs.srcBlendAlpha ], blendFactors[ bs.dstBlendAlpha ], blendOps[ bs.blendOpAlpha ],
			bs.colorWrite,
		};
		bdesc.RenderTarget[ i ] = tbdesc;
	}
	if( create_blendstate( m_renderer->m_dev, &bdesc, &m_BS ) )
	{
		// error, use default or something
	}
	
	D3D11_DEPTH_STENCIL_DESC dsdesc =
	{
		state.depthEnable,
		dwMasks[ state.depthWriteEnable ],
		compFuncs[ state.depthFunc ],
		state.stencilEnable,
		state.stencilReadMask,
		state.stencilWriteMask,
		{
			stencilOps[ state.stencilFrontFailOp ],
			stencilOps[ state.stencilFrontDepthFailOp ],
			stencilOps[ state.stencilFrontPassOp ],
			compFuncs[ state.stencilFrontFunc ],
		},
		{
			stencilOps[ state.stencilBackFailOp ],
			stencilOps[ state.stencilBackDepthFailOp ],
			stencilOps[ state.stencilBackPassOp ],
			compFuncs[ state.stencilBackFunc ],
		},
	};
	if( create_dsstate( m_renderer->m_dev, &dsdesc, &m_DS ) )
	{
	}
}

SGRX_IRenderState* D3D11Renderer::CreateRenderState( const SGRX_RenderState& state )
{
	D3D11RenderState* out = new D3D11RenderState( this );
	out->SetState( state );
	return out;
}


static const char* vdeclusage_to_semtype[] =
{
	"POSITION",
	"COLOR",
	"NORMAL",
	"TANGENT",
	"BLENDWEIGHT",
	"BLENDINDICES",
	"TEXCOORD",
	"TEXCOORD",
	"TEXCOORD",
	"TEXCOORD",
};

static int vdeclusage_to_semindex[] = { 0, 0, 0, 0, 0, 0, 0, 1, 2, 3 };

static DXGI_FORMAT vdecltype_to_format[] =
{
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R8G8B8A8_UNORM,
};

static const char* format_to_str( DXGI_FORMAT fmt )
{
	static char bfr[32];
	switch( fmt )
	{
	case DXGI_FORMAT_R32_FLOAT: return "r32 float [1]";
	case DXGI_FORMAT_R32G32_FLOAT: return "rg32 float [2]";
	case DXGI_FORMAT_R32G32B32_FLOAT: return "rgb32 float [3]";
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return "rgba32 float [4]";
	case DXGI_FORMAT_R8G8B8A8_UNORM: return "rgba8 unorm";
	case DXGI_FORMAT_R8G8B8A8_UINT: return "rgba8 uint";
	default:
		sprintf( bfr, "<! Unknown: %d>", (int) fmt );
		return bfr;
	}
}

SGRX_IVertexDecl* D3D11Renderer::CreateVertexDecl( const VDeclInfo& vdinfo )
{
	int i;
	bool hascolor = false;
	bool hastan = false;
	bool hastex0 = false;
	bool hastex1 = false;
	bool hastex2 = false;
	bool hastex3 = false;
	D3D11_INPUT_ELEMENT_DESC elements[ VDECL_MAX_ITEMS + 1 ];
	for( i = 0; i < vdinfo.count; ++i )
	{
		elements[ i ].SemanticName = vdeclusage_to_semtype[ vdinfo.usages[ i ] ];
		elements[ i ].SemanticIndex = vdeclusage_to_semindex[ vdinfo.usages[ i ] ];
		elements[ i ].Format = vdecltype_to_format[ vdinfo.types[ i ] ];
		elements[ i ].InputSlot = 0;
		elements[ i ].AlignedByteOffset = vdinfo.offsets[ i ];
		elements[ i ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elements[ i ].InstanceDataStepRate = 0;
		
		if( vdinfo.usages[ i ] == VDECLUSAGE_COLOR )
			hascolor = true;
		if( vdinfo.usages[ i ] == VDECLUSAGE_TANGENT )
			hastan = true;
		if( vdinfo.usages[ i ] == VDECLUSAGE_TEXTURE0 )
			hastex0 = true;
		if( vdinfo.usages[ i ] == VDECLUSAGE_TEXTURE1 )
			hastex1 = true;
		if( vdinfo.usages[ i ] == VDECLUSAGE_TEXTURE2 )
			hastex2 = true;
		if( vdinfo.usages[ i ] == VDECLUSAGE_TEXTURE3 )
			hastex3 = true;
		
		if( vdinfo.usages[ i ] == VDECLUSAGE_BLENDIDX && vdinfo.types[ i ] == VDECLTYPE_BCOL4 )
			elements[ i ].Format = DXGI_FORMAT_R8G8B8A8_UINT;
		if( (  vdinfo.usages[ i ] == VDECLUSAGE_BLENDWT
			|| vdinfo.usages[ i ] == VDECLUSAGE_TEXTURE0
			|| vdinfo.usages[ i ] == VDECLUSAGE_TEXTURE1
			|| vdinfo.usages[ i ] == VDECLUSAGE_TEXTURE2
			|| vdinfo.usages[ i ] == VDECLUSAGE_TEXTURE3 )
			&& vdinfo.types[ i ] == VDECLTYPE_BCOL4 )
			elements[ i ].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	
	if( !hascolor )
	{
		elements[ i ].SemanticName = "COLOR";
		elements[ i ].SemanticIndex = 0;
		elements[ i ].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elements[ i ].InputSlot = 1;
		elements[ i ].AlignedByteOffset = 0;
		elements[ i ].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		elements[ i ].InstanceDataStepRate = 0;
		i++;
	}
	if( !hastan )
	{
		elements[ i ].SemanticName = "TANGENT";
		elements[ i ].SemanticIndex = 0;
		elements[ i ].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elements[ i ].InputSlot = 1;
		elements[ i ].AlignedByteOffset = 16;
		elements[ i ].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		elements[ i ].InstanceDataStepRate = 0;
		i++;
	}
	if( !hastex0 )
	{
		elements[ i ].SemanticName = "TEXCOORD";
		elements[ i ].SemanticIndex = 0;
		elements[ i ].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elements[ i ].InputSlot = 1;
		elements[ i ].AlignedByteOffset = 32;
		elements[ i ].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		elements[ i ].InstanceDataStepRate = 0;
		i++;
	}
	if( !hastex1 )
	{
		elements[ i ].SemanticName = "TEXCOORD";
		elements[ i ].SemanticIndex = 1;
		elements[ i ].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elements[ i ].InputSlot = 1;
		elements[ i ].AlignedByteOffset = 48;
		elements[ i ].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		elements[ i ].InstanceDataStepRate = 0;
		i++;
	}
	if( !hastex2 )
	{
		elements[ i ].SemanticName = "TEXCOORD";
		elements[ i ].SemanticIndex = 2;
		elements[ i ].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elements[ i ].InputSlot = 1;
		elements[ i ].AlignedByteOffset = 64;
		elements[ i ].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		elements[ i ].InstanceDataStepRate = 0;
		i++;
	}
	if( !hastex3 )
	{
		elements[ i ].SemanticName = "TEXCOORD";
		elements[ i ].SemanticIndex = 3;
		elements[ i ].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elements[ i ].InputSlot = 1;
		elements[ i ].AlignedByteOffset = 80;
		elements[ i ].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		elements[ i ].InstanceDataStepRate = 0;
		i++;
	}
	
	D3D11VertexDecl* vdecl = new D3D11VertexDecl;
	memset( &vdecl->m_elements, 0, sizeof(vdecl->m_elements) );
	memcpy( &vdecl->m_elements, elements, sizeof(elements[0]) * i );
	vdecl->m_elemCount = i;
	vdecl->m_info = vdinfo;
	vdecl->m_renderer = this;
	return vdecl;
}


D3D11Mesh::~D3D11Mesh()
{
	m_renderer->m_ownMeshes.unset( this );
	SAFE_RELEASE( m_VB );
	SAFE_RELEASE( m_IB );
	for( size_t i = 0; i < m_inputLayouts.size(); ++i )
	{
		SAFE_RELEASE( m_inputLayouts[ i ].basic );
		SAFE_RELEASE( m_inputLayouts[ i ].skin );
	}
}

bool D3D11Mesh::InitVertexBuffer( size_t size, VertexDeclHandle vd )
{
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	SAFE_RELEASE( m_VB );
	
	m_origVertexSize = vd->m_info.size;
	m_realVertexSize = divideup( m_origVertexSize, 16 ) * 16;
	m_vertexDataSize = size;
	m_realVertexDataSize = divideup( size, m_origVertexSize ) * m_realVertexSize;
	
	if( create_buf( m_renderer->m_dev, m_realVertexDataSize, dyn, D3D11_BIND_VERTEX_BUFFER, NULL, &m_VB ) )
		return false;
	
	m_vertexDecl = vd;
	
	_UpdatePartInputLayouts();
	
	return true;
}

bool D3D11Mesh::InitIndexBuffer( size_t size, bool i32 )
{
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	SAFE_RELEASE( m_IB );
	if( create_buf( m_renderer->m_dev, size, dyn, D3D11_BIND_INDEX_BUFFER, NULL, &m_IB ) )
		return false;
	m_dataFlags = ( m_dataFlags & ~MDF_INDEX_32 ) | ( MDF_INDEX_32 * i32 );
	m_indexDataSize = size;
	return true;
}

bool D3D11Mesh::UpdateVertexData( const void* data, size_t size, bool tristrip )
{
	if( size > m_vertexDataSize )
	{
		LOG_ERROR << "given vertex data is too big";
		return false;
	}
	
	if( m_origVertexSize != m_realVertexSize )
	{
		ByteArray& scratch = m_renderer->m_scratchMem;
		size_t xsize = divideup( size, m_origVertexSize ) * m_realVertexSize;
		scratch.resize( xsize );
		memset( scratch.data(), 0, scratch.size() );
		for( size_t i = 0; i < size / m_origVertexSize; ++i )
		{
			memcpy( &scratch[ i * m_realVertexSize ], (uint8_t*)data + i * m_origVertexSize, m_origVertexSize );
		}
		if( size % m_origVertexSize != 0 )
			memcpy( &scratch[ size / m_origVertexSize * m_realVertexSize ], (uint8_t*)data + size / m_origVertexSize * m_origVertexSize, size % m_origVertexSize );
		
		data = scratch.data();
		size = scratch.size();
	}
	
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	upload_buf( m_renderer->m_ctx, m_VB, dyn, true, data, size );
	
	m_dataFlags = ( m_dataFlags & ~MDF_TRIANGLESTRIP ) | ( MDF_TRIANGLESTRIP * tristrip );
	
	return true;
}

bool D3D11Mesh::UpdateIndexData( const void* data, size_t size )
{
	if( size > m_indexDataSize )
	{
		LOG_ERROR << "given index data is too big";
		return false;
	}
	
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	upload_buf( m_renderer->m_ctx, m_IB, dyn, true, data, size );
	
	return true;
}

bool D3D11Mesh::SetPartData( SGRX_MeshPart* parts, int count )
{
	bool ret = SGRX_IMesh::SetPartData( parts, count );
	if( !ret )
		return false;
	
	_UpdatePartInputLayouts();
	
	return true;
}

void D3D11Mesh::_UpdatePartInputLayouts()
{
	for( size_t i = 0; i < m_inputLayouts.size(); ++i )
	{
		SAFE_RELEASE( m_inputLayouts[ i ].basic );
		SAFE_RELEASE( m_inputLayouts[ i ].skin );
	}
	m_inputLayouts.clear();
	
	if( !m_vertexDecl )
		return;
	D3D11VertexDecl* VD = (D3D11VertexDecl*) m_vertexDecl.item;
	
	m_inputLayouts.resize( m_meshParts.size() );
	for( size_t i = 0; i < m_meshParts.size(); ++i )
	{
		m_inputLayouts[i].Reset();
		
		SGRX_Material& MTL = m_meshParts[ i ].material;
		SGRX_SurfaceShader* SSH = MTL.shader;
		if( !SSH )
			continue;
		
		D3D11VertexShader* SHD_VB = NULL;
		for( size_t j = 0; j < SSH->m_basicVertexShaders.size(); ++j )
		{
			if( SSH->m_basicVertexShaders[ j ] )
				SHD_VB = (D3D11VertexShader*) SSH->m_basicVertexShaders[ j ].item;
		}
		
		D3D11VertexShader* SHD_VS = NULL;
		for( size_t j = 0; j < SSH->m_skinVertexShaders.size(); ++j )
		{
			if( SSH->m_skinVertexShaders[ j ] )
				SHD_VS = (D3D11VertexShader*) SSH->m_skinVertexShaders[ j ].item;
		}
		
#if 0
		D3D11_INPUT_ELEMENT_DESC* elements = VD->m_elements;
		LOG << "CREATING INPUT LAYOUT";
		for( int v = 0; v < VD->m_elemCount; ++v )
		{
			LOG << "--- " << v << " ---";
			LOG << "\tSemanticName: " << elements[ v ].SemanticName;
			LOG << "\tSemanticIndex: " << elements[ v ].SemanticIndex;
			LOG << "\tFormat: " << format_to_str( elements[ v ].Format );
			LOG << "\tInputSlot: " << elements[ v ].InputSlot;
			LOG << "\tAlignedByteOffset: " << elements[ v ].AlignedByteOffset;
			LOG << "\tInputSlotClass: " << elements[ v ].InputSlotClass;
			LOG << "\tInstanceDataStepRate: " << elements[ v ].InstanceDataStepRate;
		}
#endif
		
		if( SHD_VB )
		{
			HRESULT hr = m_renderer->m_dev->CreateInputLayout( VD->m_elements, VD->m_elemCount, SHD_VB->m_VSBC.data(), SHD_VB->m_VSBC.size(), &m_inputLayouts[i].basic );
			if( FAILED( hr ) || !m_inputLayouts[i].basic )
			{
				LOG_ERROR << "Failed to create an input layout (basic, mesh=" << m_key << ", part=" << i << ", sh.key=" << SHD_VB->m_key << ", v.d.key=" << VD->m_key << ")";
			}
		}
		
		if( SHD_VS )
		{
			HRESULT hr = m_renderer->m_dev->CreateInputLayout( VD->m_elements, VD->m_elemCount, SHD_VS->m_VSBC.data(), SHD_VS->m_VSBC.size(), &m_inputLayouts[i].skin );
			if( FAILED( hr ) || !m_inputLayouts[i].skin )
			{
				LOG_ERROR << "Failed to create an input layout (skinned, mesh=" << m_key << ", part=" << i << ", sh.key=" << SHD_VS->m_key << ", v.d.key=" << VD->m_key << ")";
			}
		}
	}
}

SGRX_IMesh* D3D11Renderer::CreateMesh()
{
	D3D11Mesh* mesh = new D3D11Mesh;
	mesh->m_renderer = this;
	m_ownMeshes.set( mesh, true );
	return mesh;
}
	

void D3D11Renderer::SetMatrix( bool view, const Mat4& mtx )
{
	if( view )
		m_cbdata_vs_batchverts.view = mtx;
	else
		m_cbdata_vs_batchverts.world = mtx;
	upload_buf( m_ctx, m_cbuf_vs_batchverts, true, true, &m_cbdata_vs_batchverts, sizeof(m_cbdata_vs_batchverts) );
}

FINLINE D3D11_PRIMITIVE_TOPOLOGY conv_prim_type( EPrimitiveType pt )
{
	switch( pt )
	{
	case PT_Points: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	case PT_Lines: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case PT_LineStrip: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case PT_Triangles: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case PT_TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	default: return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}

void D3D11Renderer::DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt, SGRX_ITexture* tex, SGRX_IPixelShader* shd, Vec4* shdata, size_t shvcount )
{
	SetVertexShader( m_sh_bv_vs );
	SetPixelShader( shd ? shd : m_sh_bv_ps );
	
	m_vertbuf_batchverts.Upload( m_dev, m_ctx, D3D11_BIND_VERTEX_BUFFER, verts, sizeof(*verts) * count );
	if( shdata && shvcount )
		m_cbuf_ps_batchverts.Upload( m_dev, m_ctx, D3D11_BIND_CONSTANT_BUFFER, shdata, sizeof(*shdata) * shvcount );
	
	m_ctx->VSSetConstantBuffers( 0, 1, &m_cbuf_vs_batchverts );
	
	m_ctx->PSSetConstantBuffers( 0, 1, m_cbuf_ps_batchverts.PPBuf() );
	ID3D11ShaderResourceView* srvs[] = { ((D3D11Texture*)( tex ? tex : m_defaultTexture ))->m_rsrcView };
	m_ctx->PSSetShaderResources( 0, 1, srvs );
	ID3D11SamplerState* smps[] = { ((D3D11Texture*)( tex ? tex : m_defaultTexture ))->m_sampState };
	m_ctx->PSSetSamplers( 0, 1, smps );
	
	m_ctx->IASetPrimitiveTopology( conv_prim_type( pt ) );
	m_ctx->IASetInputLayout( m_inputLayout_batchverts );
	ID3D11Buffer* vbufs[2] = { m_vertbuf_batchverts, m_vertbuf_defaults };
	static const UINT strides[2] = { sizeof(BatchRenderer::Vertex), sizeof(BackupVertexData) };
	static const UINT offsets[2] = { 0, 0 };
	m_ctx->IASetVertexBuffers( 0, 2, vbufs, strides, offsets );
	
	m_ctx->OMSetBlendState( m_bstate_batchverts_normal, NULL, 0xffffffff );
	m_ctx->RSSetState( m_rstate_batchverts );
	
	m_ctx->Draw( count, 0 );
}


bool D3D11Renderer::SetRenderPasses( SGRX_RenderPass* passes, int count )
{
	for( int i = 0; i < count; ++i )
	{
		SGRX_RenderPass& PASS = passes[ i ];
		if( PASS.type != RPT_SHADOWS &&
			PASS.type != RPT_OBJECT &&
			PASS.type != RPT_SCREEN &&
			PASS.type != RPT_PROJECTORS &&
			PASS.type != RPT_LIGHTVOLS )
		{
			LOG_ERROR << "Invalid type for pass " << i;
			return false;
		}
		if( !PASS.shader_name )
		{
			LOG_ERROR << "No shader name for pass " << i;
			return false;
		}
	}
	
	m_renderPasses.assign( passes, count );
	Array< PixelShaderHandle > psh = m_pass_shaders;
	m_pass_shaders.clear();
	m_pass_shaders.reserve( count );
	
	for( int i = 0; i < (int) m_renderPasses.size(); ++i )
	{
		SGRX_RenderPass& PASS = m_renderPasses[ i ];
		m_pass_shaders.push_back( PASS.type == RPT_SCREEN ? GR_GetPixelShader( PASS.shader_name ) : PixelShaderHandle() );
	}
	
	return true;
}

void D3D11Renderer::RenderScene( SGRX_RenderScene* RS )
{
	SceneHandle scene = RS->scene;
	if( !scene )
		return;
	
	m_enablePostProcessing = RS->enablePostProcessing;
	m_viewport = RS->viewport;
	m_currentScene = scene;
	const SGRX_Camera& CAM = scene->camera;
	
	
	// TODO viewport / RTs
	RTOutInfo RTOUT;
	
	
	m_stats.Reset();
	// CULLING
	_RS_PreProcess( m_currentScene );
	
	// Upload core CB data
	cb_objpass_core_data coredata =
	{
		CAM.mView,
		CAM.mProj,
		CAM.mInvView,
		CAM.position,
		RS->timevals,
		
		scene->ambientLightColor,
		-CAM.mView.TransformNormal( scene->dirLightDir ).Normalized(),
		scene->dirLightColor,
	};
	upload_buf( m_ctx, m_cbuf0_common, true, true, &coredata, sizeof(coredata) );
	
	for( size_t pass_id = 0; pass_id < m_renderPasses.size(); ++pass_id )
	{
		const SGRX_RenderPass& pass = m_renderPasses[ pass_id ];
		
		if( pass.type == RPT_OBJECT ) _RS_RenderPass_Object( pass, pass_id );
		else if( pass.type == RPT_SCREEN ) _RS_RenderPass_Screen( pass, pass_id, /*tx_depth,*/ RTOUT );
	}
	
	// RESTORE STATE
	m_enablePostProcessing = false;
	m_viewport = NULL;
	m_currentScene = NULL;
}

void D3D11Renderer::_RS_RenderPass_Object( const SGRX_RenderPass& PASS, size_t pass_id )
{
	int obj_type = !!( PASS.flags & RPF_OBJ_STATIC ) - !!( PASS.flags & RPF_OBJ_DYNAMIC );
	int mtl_type = !!( PASS.flags & RPF_MTL_SOLID ) - !!( PASS.flags & RPF_MTL_TRANSPARENT );
	
	const SGRX_Camera& CAM = m_currentScene->camera;
	
	ID3D11Buffer* cbufs[5] =
	{
		m_cbuf0_common,
		m_cbuf1_inst,
		m_cbuf2_skin,
		m_cbuf3_ltpoint,
		m_cbuf4_ltspot,
	};
	m_ctx->VSSetConstantBuffers( 0, 5, cbufs );
	m_ctx->PSSetConstantBuffers( 0, 5, cbufs );
	
	for( size_t inst_id = 0; inst_id < m_visible_meshes.size(); ++inst_id )
	{
		SGRX_MeshInstance* MI = m_visible_meshes[ inst_id ];
		if( !MI->mesh )
			continue;
		
		D3D11Mesh* M = (D3D11Mesh*) MI->mesh.item;
		if( !M->m_vertexDecl )
			continue;
		
		/* dynamic meshes */
		if( ( MI->dynamic && obj_type > 0 ) || ( !MI->dynamic && obj_type < 0 ) )
			continue;
		
		// RENDERSTATE: NOCULL    M->m_dataFlags & MDF_NOCULL ? D3DCULL_NONE : D3DCULL_CCW
		
		/* -------------------------------------- */
		do
		{
			/* WHILE THERE ARE LIGHTS IN A LIGHT OVERLAY PASS */
			int pl_count = 0, sl_count = 0;
			Vec4 lightdata[ 64 ];
			Vec4 *pldata_it = &lightdata[0];
			Vec4 *sldata_ps_it = &lightdata[32];
			Vec4 *sldata_vs_it = &lightdata[48];
			
#if 0
	
	TODO
	
			if( PASS.pointlight_count )
			{
				while( pl_count < PASS.pointlight_count && MI->_lightbuf_begin < MI->_lightbuf_end )
				{
					int found = 0;
					SGRX_MeshInstLight* plt = MI->_lightbuf_begin;
					while( plt < MI->_lightbuf_end )
					{
						if( plt->L->type == LIGHT_POINT )
						{
							SGRX_Light* light = plt->L;
							
							found = 1;
							
							// copy data
							Vec3 viewpos = CAM.mView.TransformPos( light->position );
							Vec4 newdata[2] =
							{
								{ viewpos.x, viewpos.y, viewpos.z, light->range },
								{ light->color.x, light->color.y, light->color.z, light->power }
							};
							memcpy( pldata_it, newdata, sizeof(Vec4)*2 );
							pldata_it += 2;
							pl_count++;
							
							// extract light from array
							if( plt > MI->_lightbuf_begin )
								*plt = *MI->_lightbuf_begin;
							MI->_lightbuf_begin++;
							
							break;
						}
						plt++;
					}
					if( !found )
						break;
				}
				m_cbuf3_ltpoint.Upload( m_dev, m_ctx, D3D11_BIND_CONSTANT_BUFFER, &lightdata[0], 
				PS_SetVec4Array( 56, &lightdata[0], 2 * pl_count );
			}
			if( PASS.spotlight_count )
			{
				while( sl_count < PASS.spotlight_count && MI->_lightbuf_begin < MI->_lightbuf_end )
				{
					int found = 0;
					SGRX_MeshInstLight* plt = MI->_lightbuf_begin;
					while( plt < MI->_lightbuf_end )
					{
						if( plt->L->type == LIGHT_SPOT )
						{
							SGRX_Light* light = plt->L;
							
							found = 1;
							
							// copy data
							Vec3 viewpos = CAM.mView.TransformPos( light->position );
							Vec3 viewdir = CAM.mView.TransformPos( light->direction ).Normalized();
							float tszx = 1, tszy = 1;
							if( light->shadowTexture )
							{
								const TextureInfo& texinfo = light->shadowTexture.GetInfo();
								tszx = texinfo.width;
								tszy = texinfo.height;
							}
							Vec4 newdata[4] =
							{
								{ viewpos.x, viewpos.y, viewpos.z, light->range },
								{ light->color.x, light->color.y, light->color.z, light->power },
								{ viewdir.x, viewdir.y, viewdir.z, DEG2RAD( light->angle ) },
								{ tszx, tszy, 1.0f / tszx, 1.0f / tszy },
							};
							memcpy( sldata_ps_it, newdata, sizeof(Vec4)*4 );
							Mat4 tmp;
							tmp.Multiply( MI->matrix, light->viewProjMatrix );
							memcpy( sldata_vs_it, &tmp, sizeof(Mat4) );
							sldata_ps_it += 4;
							sldata_vs_it += 4;
							
							SetTexture( 8 + sl_count * 2, light->cookieTexture );
							SetTexture( 8 + sl_count * 2 + 1, light->shadowTexture );
							sl_count++;
							
							// extract light from array
							if( plt > MI->_lightbuf_begin )
								*plt = *MI->_lightbuf_begin;
							MI->_lightbuf_begin++;
							
							break;
						}
						plt++;
					}
					if( !found )
						break;
				}
				VS_SetVec4Array( 24, &lightdata[48], 4 * sl_count );
				PS_SetVec4Array( 24, &lightdata[32], 4 * sl_count );
			}
#endif
			
			if( PASS.flags & RPF_LIGHTOVERLAY && pl_count + sl_count <= 0 )
				break;
			
			// PER-INSTANCE DATA
			Mat4 m_world_view;
			m_world_view.Multiply( MI->matrix, CAM.mView );
			cb_objpass_instance_data instdata =
			{
				MI->matrix,
				m_world_view,
				V2( pl_count, sl_count ),
			};
			for( int i = 0; i < MAX_MI_CONSTANTS; ++i )
				instdata.gInstanceData[ i ] = MI->constants[ i ];
			upload_buf( m_ctx, m_cbuf1_inst, true, true, &instdata, sizeof(instdata) );
			
			ID3D11ShaderResourceView* srvs[16] = { NULL };
			ID3D11SamplerState* smps[16] = { NULL };
			for( int i = 0; i < MAX_MI_TEXTURES; ++i )
			{
				D3D11Texture* tex = (D3D11Texture*) MI->textures[ i ].item;
				srvs[ i + 8 ] = ( tex ? tex : m_defaultTexture )->m_rsrcView;
				smps[ i + 8 ] = ( tex ? tex : m_defaultTexture )->m_sampState;
			}
			m_ctx->PSSetShaderResources( 8, 4, srvs + 8 );
			m_ctx->PSSetSamplers( 8, 4, smps + 8 );
			
			ID3D11Buffer* vbufs[2] = { M->m_VB, m_vertbuf_defaults };
			const UINT strides[2] = { M->m_realVertexSize, sizeof(BackupVertexData) };
			const UINT offsets[2] = { 0, 0 };
			m_ctx->IASetVertexBuffers( 0, 2, vbufs, strides, offsets );
			m_ctx->IASetIndexBuffer( M->m_IB, M->m_dataFlags & MDF_INDEX_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0 );
			m_ctx->IASetPrimitiveTopology( M->m_dataFlags & MDF_TRIANGLESTRIP ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			
			for( size_t part_id = 0; part_id < M->m_meshParts.size(); ++part_id )
			{
				SGRX_MeshPart* MP = &M->m_meshParts[ part_id ];
				SGRX_Material& MTL = MP->material;
				SGRX_SurfaceShader* SSH = MTL.shader;
				if( !SSH )
					continue;
				
				bool transparent = MTL.blendMode != MBM_NONE;
				if( ( transparent && mtl_type > 0 ) || ( !transparent && mtl_type < 0 ) )
					continue;
				
				if( MP->indexCount < 3 )
					continue;
				
				m_ctx->IASetInputLayout( MI->skin_matrices.size() ? M->m_inputLayouts[ part_id ].basic : M->m_inputLayouts[ part_id ].skin );
				
			//	m_dev->SetRenderState( D3DRS_ZWRITEENABLE, ( ( PASS.flags & RPF_LIGHTOVERLAY ) || transparent ) == false );
			//	m_dev->SetRenderState( D3DRS_ALPHABLENDENABLE, ( PASS.flags & RPF_LIGHTOVERLAY ) || transparent );
			//	m_dev->SetRenderState( D3DRS_DESTBLEND, ( PASS.flags & RPF_LIGHTOVERLAY ) || MTL.additive ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );
				
				SetVertexShader( MI->skin_matrices.size() ? SSH->m_skinVertexShaders[ part_id ] : SSH->m_basicVertexShaders[ part_id ] );
				SetPixelShader( SSH->m_pixelShaders[ part_id ] );
				
				for( int i = 0; i < NUM_MATERIAL_TEXTURES; ++i )
				{
					D3D11Texture* tex = (D3D11Texture*) MTL.textures[ i ].item;
					srvs[ i ] = ( tex ? tex : m_defaultTexture )->m_rsrcView;
					smps[ i ] = ( tex ? tex : m_defaultTexture )->m_sampState;
				}
				m_ctx->PSSetShaderResources( 0, 8, srvs );
				m_ctx->PSSetSamplers( 0, 8, smps );
				
				m_ctx->DrawIndexed( MP->indexCount, MP->indexOffset, MP->vertexOffset );
				m_stats.numDrawCalls++;
				m_stats.numMDrawCalls++;
			}
			
			/* -------------------------------------- */
		}
		while( PASS.flags & RPF_LIGHTOVERLAY );
	}
}

void D3D11Renderer::_RS_RenderPass_Screen( const SGRX_RenderPass& pass, size_t pass_id, /*IDirect3DBaseTexture9* tx_depth,*/ const RTOutInfo& RTOUT )
{
}


void D3D11Renderer::SetVertexShader( SGRX_IVertexShader* shd )
{
	SGRX_CAST( D3D11VertexShader*, S, shd );
	m_ctx->VSSetShader( S ? S->m_VS : NULL, NULL, 0 );
}

void D3D11Renderer::SetPixelShader( SGRX_IPixelShader* shd )
{
	SGRX_CAST( D3D11PixelShader*, S, shd );
	m_ctx->PSSetShader( S ? S->m_PS : NULL, NULL, 0 );
}


