

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include "../ext/d3dx/d3d11.h"
#ifdef ENABLE_SHADER_COMPILING
#  include <d3dcompiler.h>
#endif

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

static int create_buf( ID3D11Device* device, size_t numbytes, bool dyn, D3D11_BIND_FLAG bindtype, void* data, ID3D11Buffer** out )
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
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
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

static int upload_buf( ID3D11DeviceContext* ctx, ID3D11Buffer* buf, bool discard, const void* data, size_t size )
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
	return 0;
}


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


struct D3D11Shader : SGRX_IShader
{
	ID3D11VertexShader* m_VS;
	ID3D11PixelShader* m_PS;
	ByteArray m_VSBC; // bytecode
	ByteArray m_PSBC;
	struct D3D11Renderer* m_renderer;
	
	D3D11Shader( struct D3D11Renderer* r ) : m_VS( NULL ), m_PS( NULL ), m_renderer( r ){}
	~D3D11Shader()
	{
		SAFE_RELEASE( m_VS );
		SAFE_RELEASE( m_PS );
	}
};


struct D3D11VertexDecl : SGRX_IVertexDecl
{
	D3D11_INPUT_ELEMENT_DESC m_elements[ VDECL_MAX_ITEMS ];
	struct D3D11Renderer* m_renderer;
};


struct D3D11Mesh : SGRX_IMesh
{
	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;
	struct D3D11Renderer* m_renderer;
	
	ID3D11InputLayout* m_inputLayouts[ MAX_MESH_PARTS ];
	
	D3D11Mesh() : m_VB( NULL ), m_IB( NULL ), m_renderer( NULL )
	{
		memset( m_inputLayouts, 0, sizeof(m_inputLayouts) );
	}
	~D3D11Mesh();
	
	bool InitVertexBuffer( size_t size );
	bool InitIndexBuffer( size_t size, bool i32 );
	bool UpdateVertexData( const void* data, size_t size, VertexDeclHandle vd, bool tristrip );
	bool UpdateIndexData( const void* data, size_t size );
	bool SetPartData( SGRX_MeshPart* parts, int count );
	
	void _UpdatePartInputLayouts();
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
	
	SGRX_IShader* m_sh_pp_final;
	SGRX_IShader* m_sh_pp_dshp;
	SGRX_IShader* m_sh_pp_blur_h;
	SGRX_IShader* m_sh_pp_blur_v;
	SGRX_IShader* m_sh_debug_draw;
	Array< ShaderHandle > m_pass_shaders;
	
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
	cb_vs_batchverts m_cbdata_vs_batchverts;
	ID3D11Buffer* m_cbuf_vs_batchverts;
	
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
	NOP( DXGI_FORMAT_D24_UNORM_S8_UINT ); // GCC is just amazing
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
	
	NOP( (int) &R->m_cbdata_vs_batchverts ); // did I mention GCC is just superb?
	if( create_buf( device, sizeof(R->m_cbdata_vs_batchverts), true, D3D11_BIND_CONSTANT_BUFFER, &R->m_cbdata_vs_batchverts, &R->m_cbuf_vs_batchverts ) )
		return NULL;
	
	return R;
}

void D3D11Renderer::Destroy()
{
	SAFE_RELEASE( m_cbuf_vs_batchverts );
	
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
	ShaderHandle sh_pp_final = GR_GetShader( "testFRpost" );
	ShaderHandle sh_pp_dshp = GR_GetShader( "pp_bloom_dshp" );
	ShaderHandle sh_pp_blur_h = GR_GetShader( "pp_bloom_blur_h" );
	ShaderHandle sh_pp_blur_v = GR_GetShader( "pp_bloom_blur_v" );
	ShaderHandle sh_debug_draw = GR_GetShader( "debug_draw" );
	sh_pp_final->Acquire();
	sh_pp_dshp->Acquire();
	sh_pp_blur_h->Acquire();
	sh_pp_blur_v->Acquire();
	sh_debug_draw->Acquire();
	m_sh_pp_final = sh_pp_final;
	m_sh_pp_dshp = sh_pp_dshp;
	m_sh_pp_blur_h = sh_pp_blur_h;
	m_sh_pp_blur_v = sh_pp_blur_v;
	m_sh_debug_draw = sh_debug_draw;
}

void D3D11Renderer::UnloadInternalResources()
{
	SetRenderPasses( NULL, 0 );
	
	m_sh_pp_final->Release();
	m_sh_pp_dshp->Release();
	m_sh_pp_blur_h->Release();
	m_sh_pp_blur_v->Release();
	m_sh_debug_draw->Release();
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
		dtd.CPUAccessFlags = 0;
		dtd.SampleDesc.Count = 1;
		
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
		if( create_rtt( m_dev, width, height, 0, d3dfmt, false, &CT, &CRV ) )
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

bool D3D11Renderer::CompileShader( const StringView& code, ByteArray& outcomp, String& outerrors )
{
#ifdef ENABLE_SHADER_COMPILING
	HRESULT hr;
	ID3DBlob *outbuf = NULL, *outerr = NULL;
	
	static const D3D_SHADER_MACRO vsmacros[] = { { "VS", "1" }, { NULL, NULL } };
	static const D3D_SHADER_MACRO psmacros[] = { { "PS", "1" }, { NULL, NULL } };
	
	ByteWriter bw( &outcomp );
	bw.marker( "CSH\x7f", 4 );
	
	int32_t shsize;
	
	hr = D3DCompile( code.data(), code.size(), "source", vsmacros, NULL, "main", "vs_3_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &outbuf, &outerr );
	if( FAILED( hr ) )
	{
		if( outerr )
		{
			const char* errtext = (const char*) outerr->GetBufferPointer();
			outerrors.append( STRLIT_BUF( "Errors in vertex shader compilation:\n" ) );
			outerrors.append( errtext, strlen( errtext ) );
		}
		else
			outerrors.append( STRLIT_BUF( "Unknown error in vertex shader compilation" ) );
		
		SAFE_RELEASE( outbuf );
		SAFE_RELEASE( outerr );
		return false;
	}
	
	shsize = outbuf->GetBufferSize();
	
	bw << shsize;
	bw.memory( outbuf->GetBufferPointer(), shsize );
	
	SAFE_RELEASE( outbuf );
	SAFE_RELEASE( outerr );
	
	hr = D3DCompile( code.data(), code.size(), "source", psmacros, NULL, "main", "ps_3_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &outbuf, &outerr );
	if( FAILED( hr ) )
	{
		if( outerr )
		{
			const char* errtext = (const char*) outerr->GetBufferPointer();
			outerrors.append( STRLIT_BUF( "Errors in pixel shader compilation:\n" ) );
			outerrors.append( errtext, strlen( errtext ) );
		}
		else
			outerrors.append( STRLIT_BUF( "Unknown error in pixel shader compilation" ) );
		
		SAFE_RELEASE( outbuf );
		SAFE_RELEASE( outerr );
		return false;
	}
	
	shsize = outbuf->GetBufferSize();
	
	bw << shsize;
	bw.memory( outbuf->GetBufferPointer(), shsize );
	
	SAFE_RELEASE( outbuf );
	SAFE_RELEASE( outerr );
	
	return true;
#else
	LOG << "D3D11 SHADER COMPILATION IS NOT ALLOWED IN THIS BUILD";
	return false;
#endif // ENABLE_SHADER_COMPILING
}

SGRX_IShader* D3D11Renderer::CreateShader( ByteArray& code )
{
	HRESULT hr;
	ByteReader br( &code );
	br.marker( "CSH\x7f", 4 );
	
	int32_t vslen = 0, pslen = 0;
	br << vslen;
	br.padding( vslen );
	br << pslen;
	br.padding( pslen );
	if( br.error )
		return NULL;
	
	uint8_t* vsbuf = &code[ 8 ];
	uint8_t* psbuf = &code[ 12 + vslen ];
	ID3D11VertexShader* VS = NULL;
	ID3D11PixelShader* PS = NULL;
	D3D11Shader* out = NULL;
	
	hr = m_dev->CreateVertexShader( vsbuf, vslen, NULL, &VS );
	if( FAILED( hr ) || !VS )
		{ LOG_ERROR << "Failed to create a D3D11 vertex shader"; goto cleanup; }
	
	hr = m_dev->CreatePixelShader( psbuf, pslen, NULL, &PS );
	if( FAILED( hr ) || !PS )
		{ LOG_ERROR << "Failed to create a D3D11 pixel shader"; goto cleanup; }
	
	out = new D3D11Shader( this );
	out->m_VS = VS;
	out->m_PS = PS;
	out->m_VSBC.append( vsbuf, vslen );
	out->m_PSBC.append( psbuf, pslen );
	return out;
	
cleanup:
	SAFE_RELEASE( VS );
	SAFE_RELEASE( PS );
	return NULL;
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

SGRX_IVertexDecl* D3D11Renderer::CreateVertexDecl( const VDeclInfo& vdinfo )
{
	D3D11_INPUT_ELEMENT_DESC elements[ VDECL_MAX_ITEMS + 1 ];
	for( int i = 0; i < vdinfo.count; ++i )
	{
		elements[ i ].SemanticName = vdeclusage_to_semtype[ vdinfo.usages[ i ] ];
		elements[ i ].SemanticIndex = vdeclusage_to_semindex[ vdinfo.usages[ i ] ];
		elements[ i ].Format = vdecltype_to_format[ vdinfo.types[ i ] ];
		elements[ i ].InputSlot = 0;
		elements[ i ].AlignedByteOffset = vdinfo.offsets[ i ];
		elements[ i ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elements[ i ].InstanceDataStepRate = 0;
		
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
	
	D3D11VertexDecl* vdecl = new D3D11VertexDecl;
	memset( &vdecl->m_elements, 0, sizeof(vdecl->m_elements) );
	memcpy( &vdecl->m_elements, elements, sizeof(elements[0]) * vdinfo.count );
	vdecl->m_info = vdinfo;
	vdecl->m_renderer = this;
	return vdecl;
}


D3D11Mesh::~D3D11Mesh()
{
	m_renderer->m_ownMeshes.unset( this );
	SAFE_RELEASE( m_VB );
	SAFE_RELEASE( m_IB );
	for( int i = 0; i < MAX_MESH_PARTS; ++i )
		SAFE_RELEASE( m_inputLayouts[ i ] );
}

bool D3D11Mesh::InitVertexBuffer( size_t size )
{
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	SAFE_RELEASE( m_VB );
	if( create_buf( m_renderer->m_dev, size, dyn, D3D11_BIND_VERTEX_BUFFER, NULL, &m_VB ) )
		return false;
	m_vertexDataSize = size;
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

bool D3D11Mesh::UpdateVertexData( const void* data, size_t size, VertexDeclHandle vd, bool tristrip )
{
	if( size > m_vertexDataSize )
	{
		LOG_ERROR << "given vertex data is too big";
		return false;
	}
	
	upload_buf( m_renderer->m_ctx, m_VB, true, data, size );
	
	m_vertexDecl = vd;
	m_dataFlags = ( m_dataFlags & ~MDF_TRIANGLESTRIP ) | ( MDF_TRIANGLESTRIP * tristrip );
	
	_UpdatePartInputLayouts();
	
	return true;
}

bool D3D11Mesh::UpdateIndexData( const void* data, size_t size )
{
	if( size > m_indexDataSize )
	{
		LOG_ERROR << "given index data is too big";
		return false;
	}
	
	upload_buf( m_renderer->m_ctx, m_IB, true, data, size );
	
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
	for( int i = 0; i < MAX_MESH_PARTS; ++i )
		SAFE_RELEASE( m_inputLayouts[ i ] );
	
	if( !m_vertexDecl )
		return;
	D3D11VertexDecl* VD = (D3D11VertexDecl*) m_vertexDecl.item;
	
	for( int i = 0; i < m_numParts; ++i )
	{
		SGRX_Material* MTL = m_parts[ i ].material;
		if( !MTL )
			continue;
		SGRX_SurfaceShader* SSH = MTL->shader;
		if( !SSH )
			continue;
		D3D11Shader* SHD = NULL;
		for( size_t s = 0; s < SSH->m_shaders.size(); ++s )
		{
			if( SSH->m_shaders[ s ] )
				SHD = (D3D11Shader*) SSH->m_shaders[ s ].item;
		}
		if( !SHD )
			continue;
		
		HRESULT hr = m_renderer->m_dev->CreateInputLayout( VD->m_elements, VD->m_info.count, SHD->m_VSBC.data(), SHD->m_VSBC.size(), &m_inputLayouts[ i ] );
		if( FAILED( hr ) || !m_inputLayouts[ i ] )
		{
			LOG_ERROR << "Failed to create an input layout (mesh=" << m_key << ")";
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
	upload_buf( m_ctx, m_cbuf_vs_batchverts, true, &m_cbdata_vs_batchverts, sizeof(m_cbdata_vs_batchverts) );
}

void D3D11Renderer::DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt, SGRX_ITexture* tex, SGRX_IShader* shd, Vec4* shdata, size_t shvcount )
{
}


bool D3D11Renderer::SetRenderPasses( SGRX_RenderPass* passes, int count )
{
	for( int i = 0; i < count; ++i )
	{
		SGRX_RenderPass& PASS = passes[ i ];
		if( PASS.type != RPT_SHADOWS && PASS.type != RPT_OBJECT && PASS.type != RPT_SCREEN )
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
	Array< ShaderHandle > psh = m_pass_shaders;
	m_pass_shaders.clear();
	m_pass_shaders.reserve( count );
	
	for( int i = 0; i < (int) m_renderPasses.size(); ++i )
	{
		SGRX_RenderPass& PASS = m_renderPasses[ i ];
		m_pass_shaders.push_back( PASS.type == RPT_SCREEN ? GR_GetShader( PASS.shader_name ) : ShaderHandle() );
	}
	
	return true;
}

void D3D11Renderer::RenderScene( SGRX_RenderScene* RS )
{
}


