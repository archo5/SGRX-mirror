

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include "../ext/d3dx/d3d11.h"

#define USE_VEC3
#define USE_VEC4
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#define USE_SERIALIZATION
#include "renderer.hpp"


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


RendererInfo g_D3D11RendererInfo =
{
	false, // swap R/B
	true, // compile shaders
	"d3d11", // shader type
};

struct D3D11Renderer : IRenderer
{
	D3D11Renderer() : m_dbg_rt( false ){ m_view.SetIdentity(); m_proj.SetIdentity(); }
	void Destroy();
	const RendererInfo& GetInfo(){ return g_D3D11RendererInfo; }
	void LoadInternalResources();
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
	bool CompileShader( const StringView& code, ByteArray& outcomp, String& outerrors );
	SGRX_IShader* CreateShader( ByteArray& code );
	SGRX_IVertexDecl* CreateVertexDecl( const VDeclInfo& vdinfo );
	SGRX_IMesh* CreateMesh();
	
	void SetMatrix( bool view, const Mat4& mtx );
	void DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt, SGRX_ITexture* tex, SGRX_IShader* shd, Vec4* shdata, size_t shvcount );
	
	bool SetRenderPasses( SGRX_RenderPass* passes, int count );
	void RenderScene( SGRX_RenderScene* RS );
	uint32_t _RS_Cull_Camera_MeshList();
	uint32_t _RS_Cull_Camera_PointLightList();
	uint32_t _RS_Cull_Camera_SpotLightList();
	uint32_t _RS_Cull_SpotLight_MeshList( SGRX_Light* L );
//	void _RS_Compile_MeshLists();
//	void _RS_Render_Shadows();
//	void _RS_RenderPass_Object( const SGRX_RenderPass& pass, size_t pass_id );
//	void _RS_RenderPass_Screen( const SGRX_RenderPass& pass, IDirect3DBaseTexture9* tx_depth, const RTOutInfo& RTOUT );
//	void _RS_DebugDraw( SGRX_DebugDraw* debugDraw, IDirect3DSurface9* test_dss, IDirect3DSurface9* orig_dss );
	
	void PostProcBlit( int w, int h, int downsample, int ppdata_location );
	
	bool ResetDevice();
	void ResetViewport();
//	void _SetTextureInt( int slot, IDirect3DBaseTexture9* tex, uint32_t flags );
	void SetTexture( int slot, SGRX_ITexture* tex );
	void SetShader( SGRX_IShader* shd );
	
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
	Mat4 m_view, m_proj;
	
//	SGRX_IShader* m_sh_pp_final;
//	SGRX_IShader* m_sh_pp_dshp;
//	SGRX_IShader* m_sh_pp_blur_h;
//	SGRX_IShader* m_sh_pp_blur_v;
//	SGRX_IShader* m_sh_debug_draw;
//	
//	// storage
//	HashTable< D3D9Texture*, bool > m_ownTextures;
//	HashTable< D3D9Mesh*, bool > m_ownMeshes;
//	
//	// specific
	ID3D11Device* m_dev;
	ID3D11DeviceContext* m_ctx;
	IDXGISwapChain* m_swapChain;
	ID3D11Texture2D* m_backBuffer = NULL;
	ID3D11Texture2D* m_depthBuffer = NULL;
	ID3D11RenderTargetView* m_rtView = NULL;
	ID3D11DepthStencilView* m_dsView = NULL;
//	IDirect3DSurface9* m_dssurf;
//	
//	// temp data
//	SceneHandle m_currentScene;
//	bool m_enablePostProcessing;
//	SGRX_Viewport* m_viewport;
//	ByteArray m_scratchMem;
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
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ID3D11Device* device = NULL;
	ID3D11DeviceContext* context = NULL;
	IDXGISwapChain* swapChain = NULL;
	ID3D11Texture2D* backBuffer = NULL;
	ID3D11Texture2D* depthBuffer = NULL;
	ID3D11RenderTargetView* rtView = NULL;
	ID3D11DepthStencilView* dsView = NULL;
	
	memset( &swapChainDesc, 0, sizeof(swapChainDesc) );
	
	swapChainDesc.BufferDesc.Width = settings.width;
	swapChainDesc.BufferDesc.Height = settings.height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = settings.fullscreen != FULLSCREEN_NORMAL ? 0 : settings.refresh_rate;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // DXGI_FORMAT_B8G8R8A8_UNORM ?
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	swapChainDesc.SampleDesc.Count = settings.aa_mode == ANTIALIAS_MULTISAMPLE ? TMAX( 1, TMIN( 16, settings.aa_quality ) ) : 1;
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
		0, // flags
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
		LOG << "Failed to create the D3D11 device";
		return NULL;
	}
	
	// Backbuffer
	hr = swapChain->GetBuffer( 0, g_ID3D11Texture2D, (void**) &backBuffer );
	if( FAILED( hr ) || backBuffer == NULL )
	{
		LOG << "Failed to retrieve D3D11 backbuffer";
		return NULL;
	}
	
	hr = device->CreateRenderTargetView( backBuffer, NULL, &rtView );
	if( FAILED( hr ) || rtView == NULL )
	{
		LOG << "Failed to create D3D11 render target view";
		return NULL;
	}
	
	// Depth/stencil buffer
	D3D11_TEXTURE2D_DESC dtd;
	memset( &dtd, 0, sizeof(dtd) );
	dtd.Width = settings.width;
	dtd.Height = settings.height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dtd.SampleDesc = swapChainDesc.SampleDesc;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hr = device->CreateTexture2D( &dtd, NULL, &depthBuffer );
	if( FAILED( hr ) || depthBuffer == NULL )
	{
		LOG << "Failed to create D3D11 depth/stencil buffer";
		return NULL;
	}
	
	hr = device->CreateDepthStencilView( depthBuffer, NULL, &dsView );
	if( FAILED( hr ) || rtView == NULL )
	{
		LOG << "Failed to create D3D11 depth/stencil view";
		return NULL;
	}
	
	context->OMSetRenderTargets( 1, &rtView, NULL );
	
	D3D11Renderer* R = new D3D11Renderer;
	R->m_dev = device;
	R->m_ctx = context;
	R->m_swapChain = swapChain;
	R->m_backBuffer = backBuffer;
	R->m_depthBuffer = depthBuffer;
	R->m_rtView = rtView;
	R->m_dsView = dsView;
	
	return R;
}

void D3D11Renderer::Destroy()
{
	SAFE_RELEASE( m_dsView );
	SAFE_RELEASE( m_rtView );
	SAFE_RELEASE( m_depthBuffer );
	SAFE_RELEASE( m_backBuffer );
	SAFE_RELEASE( m_swapChain );
	SAFE_RELEASE( m_ctx );
	SAFE_RELEASE( m_dev );
	delete this;
}

void D3D11Renderer::LoadInternalResources()
{
}

void D3D11Renderer::UnloadInternalResources()
{
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
	D3D11_RECT rct = { rect[0], rect[1], rect[2], rect[3] };
	m_ctx->RSSetScissorRects( enable ? 1 : 0, &rct );
}


SGRX_ITexture* D3D11Renderer::CreateTexture( TextureInfo* texinfo, void* data )
{
	int mip, side;
	HRESULT hr;
	// TODO: filter unsupported formats / dimensions
	
	if( texinfo->type == TEXTYPE_2D )
	{
		ID3D11Texture2D* tex2d = NULL;
		
		D3D11_TEXTURE2D_DESC dtd;
		memset( &dtd, 0, sizeof(dtd) );
		dtd.Width = texinfo->width;
		dtd.Height = texinfo->height;
		dtd.MipLevels = texinfo->mipcount;
		dtd.ArraySize = 1;
		dtd.Format = texfmt2d3d( texinfo->format );
		dtd.Usage = D3D11_USAGE_DEFAULT;
		dtd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		
		D3D11_SUBRESOURCE_DATA srd[ 20 ];
		if( data )
		{
		}
		
		hr = m_dev->CreateTexture2D( &dtd, data ? srd : NULL, &tex2d );
		if( FAILED( hr ) || !tex2d )
		{
			LOG_ERROR << "could not create D3D11 texture (type: 2D, w: " << texinfo->width << ", h: " <<
				texinfo->height << ", mips: " << texinfo->mipcount << ", fmt: " << texinfo->format << ", d3dfmt: " << texfmt2d3d( texinfo->format );
			return NULL;
		}
	}
	else if( texinfo->type == TEXTYPE_CUBE )
	{
	}
	
	LOG_ERROR << "TODO [reached a part of not-yet-defined behavior]";
	return NULL;
}


