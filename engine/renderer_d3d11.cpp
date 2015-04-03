

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

static int create_rtt_( ID3D11Device* device, int width, int height, int msamples, DXGI_FORMAT fmt, bool ds, ID3D11Texture2D** outtex, void** outview )
{
	const char* what = ds ? "render target" : "depth/stencil";
	
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
		LOG << "Failed to create D3D11 " << what << " texture (w=" << width << ", h=" << height << ", ms=" << msamples << ", fmt=" << fmt << ", ds=" << ds << ")";
		return -1;
	}
	
	if( ds )
		hr = device->CreateDepthStencilView( *outtex, NULL, (ID3D11DepthStencilView**) outview );
	else
		hr = device->CreateRenderTargetView( *outtex, NULL, (ID3D11RenderTargetView**) outview );
	if( FAILED( hr ) || *outview == NULL )
	{
		SAFE_RELEASE( *outtex );
		LOG << "Failed to create D3D11 " << what << " view";
		return -1;
	}
	
	return 0;
}
#define create_rtt( dev, w, h, ms, fmt, ds, outtex, outview ) create_rtt_( dev, w, h, ms, fmt, ds, outtex, (void**) outview )


struct D3D11Texture : SGRX_ITexture
{
	union
	{
		ID3D11Texture2D* tex2d;
		ID3D11Texture3D* tex3d;
		ID3D11Resource* res;
	}
	m_ptr;
	struct D3D11Renderer* m_renderer;
	
	D3D11Texture( bool isRenderTexture = false )
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
	HashTable< D3D11Texture*, bool > m_ownTextures;
//	HashTable< D3D11Mesh*, bool > m_ownMeshes;
	
	// specific
	ID3D11Device* m_dev;
	ID3D11DeviceContext* m_ctx;
	IDXGISwapChain* m_swapChain;
	ID3D11Texture2D* m_backBuffer;
	ID3D11Texture2D* m_depthBuffer;
	ID3D11RenderTargetView* m_rtView;
	ID3D11DepthStencilView* m_dsView;
	
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
	if( create_rtt( device, settings.width, settings.height, msamples, DXGI_FORMAT_D24_UNORM_S8_UINT, true, &depthBuffer, &dsView ) )
		return NULL;
	
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


D3D11Texture::~D3D11Texture()
{
	m_renderer->m_ownTextures.unset( this );
	SAFE_RELEASE( m_ptr.res );
}

bool D3D11Texture::UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h )
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
		
		D3D11_TEXTURE2D_DESC dtd;
		memset( &dtd, 0, sizeof(dtd) );
		dtd.Width = texinfo->width;
		dtd.Height = texinfo->height;
		dtd.MipLevels = texinfo->mipcount;
		dtd.ArraySize = sides;
		dtd.Format = texfmt2d3d( texinfo->format );
		dtd.Usage = D3D11_USAGE_DEFAULT;
		dtd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		
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
		
		D3D11Texture* T = new D3D11Texture;
		T->m_renderer = this;
		T->m_info = *texinfo;
		T->m_ptr.tex2d = tex2d;
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
	
	
	ID3D11Texture2D *CT = NULL, *DT = NULL;
	ID3D11RenderTargetView *CRV = NULL;
	ID3D11DepthStencilView *DSV = NULL;
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
		if( create_rtt( m_dev, width, height, 0, d3dfmt, true, &CT, &CRV ) )
			goto cleanup;
		if( create_rtt( m_dev, width, height, 0, DXGI_FORMAT_D24_UNORM_S8_UINT, true, &DT, &DSV ) )
			goto cleanup;
	}
	
	RT = new D3D11RenderTexture;
	RT->m_renderer = this;
	RT->m_info = *texinfo;
	RT->m_ptr.tex2d = CT;
	RT->CRV = CRV;
	RT->DSV = DSV;
	RT->DT = DT;
	RT->DSV = DSV;
	m_ownTextures.set( RT, true );
	return RT;
	
cleanup:
	SAFE_RELEASE( CRV );
	SAFE_RELEASE( DSV );
	SAFE_RELEASE( DT );
	SAFE_RELEASE( CT );
	return NULL;
}


