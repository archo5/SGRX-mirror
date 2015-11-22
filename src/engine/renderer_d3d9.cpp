

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <d3d9.h>
#include <d3dx/d3dx9.h>

#include "renderer.hpp"


#define SAFE_RELEASE( x ) if( x ){ (x)->Release(); x = NULL; }


#define RENDERER_EXPORT __declspec(dllexport)


static IDirect3D9* g_D3D = NULL;


static D3DFORMAT texfmt2d3d( int fmt )
{
	switch( fmt )
	{
	case TEXFORMAT_BGRX8: return D3DFMT_X8R8G8B8;
	case TEXFORMAT_BGRA8:
	case TEXFORMAT_RGBA8: return D3DFMT_A8R8G8B8;
	case TEXFORMAT_R5G6B5: return D3DFMT_R5G6B5;
	
	case TEXFORMAT_DXT1: return D3DFMT_DXT1;
	case TEXFORMAT_DXT3: return D3DFMT_DXT3;
	case TEXFORMAT_DXT5: return D3DFMT_DXT5;
	}
	return (D3DFORMAT) 0;
}

static DWORD F2DW( float f )
{
	return *(DWORD*)&f;
}

static void swap4b2ms( uint32_t* data, int size, int mask1, int shift1R, int mask2, int shift2L )
{
	int i;
	for( i = 0; i < size; ++i )
	{
		uint32_t O = data[i];
		uint32_t N = ( O & ~( mask1 | mask2 ) ) | ( ( O & mask1 ) >> shift1R ) | ( ( O & mask2 ) << shift2L );
		data[i] = N;
	}
}

static void texdatacopy( D3DLOCKED_RECT* plr, TextureInfo* texinfo, void* data, int side, int mip )
{
	int ret;
	uint8_t *src, *dst;
	size_t i, off, copyrowsize = 0, copyrowcount = 0;
	TextureInfo mipTI;
	
	off = TextureData_GetMipDataOffset( texinfo, side, mip );
	ret = TextureInfo_GetMipInfo( texinfo, mip, &mipTI );
	ASSERT( ret );
	
//	printf( "read side=%d mip=%d at %d\n", side, mip, off );
	
	src = ((uint8_t*)data) + off;
	dst = (uint8_t*)plr->pBits;
	TextureInfo_GetCopyDims( &mipTI, &copyrowsize, &copyrowcount );
	
	for( i = 0; i < copyrowcount; ++i )
	{
		memcpy( dst, src, copyrowsize );
		if( texinfo->format == TEXFORMAT_RGBA8 )
			swap4b2ms( (uint32_t*) dst, copyrowsize / 4, 0xff0000, 16, 0xff, 16 );
		src += copyrowsize;
		dst += plr->Pitch;
	}
}

static void _ss_reset_states( IDirect3DDevice9* dev, int w, int h )
{
	dev->SetRenderState( D3DRS_LIGHTING, 0 );
	dev->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, 1 );
}


struct D3D9Texture : SGRX_ITexture
{
	union
	{
		IDirect3DBaseTexture9* base;
		IDirect3DTexture9* tex2d;
		IDirect3DCubeTexture9* cube;
		IDirect3DVolumeTexture9* vol;
	}
	m_ptr;
	struct D3D9Renderer* m_renderer;
	
	D3D9Texture( bool isRenderTexture = false )
	{
		m_isRenderTexture = isRenderTexture;
	}
	virtual ~D3D9Texture();
	
	bool UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h )
	{
		LOG_FUNCTION;
		
	//	RECT rct = { x, y, x + w, y + h };
		D3DLOCKED_RECT lr;
		HRESULT hr = m_ptr.tex2d->LockRect( mip, &lr, NULL, 0 );
		if( FAILED( hr ) )
		{
			LOG_ERROR << "failed to lock D3D9 texture";
			return false;
		}
		*(uint8_t**)&lr.pBits += lr.Pitch * y + x * 4;
		
		for( int j = 0; j < h; ++j )
		{
			uint8_t* dst = (uint8_t*)lr.pBits + lr.Pitch * j;
			memcpy( dst, ((uint32_t*)data) + w * j, w * 4 );
			if( m_info.format == TEXFORMAT_RGBA8 )
				swap4b2ms( (uint32_t*) dst, w, 0xff0000, 16, 0xff, 16 );
		}
		
		hr = m_ptr.tex2d->UnlockRect( mip );
		if( FAILED( hr ) )
		{
			LOG_ERROR << "failed to unlock D3D9 texture";
			return false;
		}
		
		return true;
	}
	
	bool OnDeviceLost(){ if( m_isRenderTexture ){ LOG << "\n\n\n T O D O\n\n\n"; } return true; } // TODO
	bool OnDeviceReset(){ return true; }
};

struct D3D9RenderTexture : D3D9Texture
{
	// color texture (CT) = m_ptr.tex2d
	IDirect3DSurface9* CS; /* color (output 0) surface */
	IDirect3DTexture9* DT; /* depth (output 2) texture (not used for shadowmaps, such data in CT/CS already) */
	IDirect3DSurface9* DS; /* depth (output 2) surface (not used for shadowmaps, such data in CT/CS already) */
	int format;
	
	D3D9RenderTexture() : D3D9Texture(true){}
	virtual ~D3D9RenderTexture()
	{
		SAFE_RELEASE( CS );
		SAFE_RELEASE( DT );
		SAFE_RELEASE( DS );
	}
};

struct D3D9DepthStencilSurface : SGRX_IDepthStencilSurface
{
	IDirect3DSurface9* DSS; /* depth/stencil surface */
	struct D3D9Renderer* m_renderer;
	
	D3D9DepthStencilSurface() : DSS(NULL), m_renderer(NULL){}
	virtual ~D3D9DepthStencilSurface();
};


struct D3D9VertexShader : SGRX_IVertexShader
{
	IDirect3DVertexShader9* m_VS;
	struct D3D9Renderer* m_renderer;
	
	D3D9VertexShader( struct D3D9Renderer* r ) : m_VS( NULL ), m_renderer( r ){}
	~D3D9VertexShader()
	{
		SAFE_RELEASE( m_VS );
	}
};

struct D3D9PixelShader : SGRX_IPixelShader
{
	IDirect3DPixelShader9* m_PS;
	struct D3D9Renderer* m_renderer;
	
	D3D9PixelShader( struct D3D9Renderer* r ) : m_PS( NULL ), m_renderer( r ){}
	~D3D9PixelShader()
	{
		SAFE_RELEASE( m_PS );
	}
};


struct D3D9RenderState : SGRX_IRenderState
{
	D3D9RenderState( const SGRX_RenderState& state )
	{
		SetState( state );
	}
};


struct D3D9VertexDecl : SGRX_IVertexDecl
{
	IDirect3DVertexDeclaration9* m_vdecl;
	struct D3D9Renderer* m_renderer;
	
	~D3D9VertexDecl()
	{
		SAFE_RELEASE( m_vdecl );
	}
};


struct D3D9Mesh : SGRX_IMesh
{
	IDirect3DVertexBuffer9* m_VB;
	IDirect3DIndexBuffer9* m_IB;
	struct D3D9Renderer* m_renderer;
	
	D3D9Mesh() : m_VB( NULL ), m_IB( NULL ), m_renderer( NULL ){}
	~D3D9Mesh();
	
	bool InitVertexBuffer( size_t size, VertexDeclHandle vd );
	bool InitIndexBuffer( size_t size, bool i32 );
	bool UpdateVertexData( const void* data, size_t size, bool tristrip );
	bool UpdateIndexData( const void* data, size_t size );
	
	bool OnDeviceLost();
	bool OnDeviceReset();
};


SGRX_RendererInfo g_D3D9RendererInfo =
{
	true, // compile shaders
	"d3d9", // shader cache folder suffix
	"HLSL_D3D9", // shader type (define)
};

struct D3D9Renderer : IRenderer
{
	D3D9Renderer()
	{
		m_world.SetIdentity();
		m_view.SetIdentity();
		m_crs.InitOpposite();
		memset( &m_viewport, 0, sizeof(m_viewport) );
	}
	void Destroy();
	const SGRX_RendererInfo& GetInfo(){ return g_D3D9RendererInfo; }
	bool LoadInternalResources();
	void UnloadInternalResources();
	
	void Swap();
	void Modify( const RenderSettings& settings );
	void SetCurrent(){} // does nothing since there's no thread context pointer
	
	void SetRenderTargets( const SGRX_RTClearInfo& info, SGRX_IDepthStencilSurface* dss, TextureHandle rts[4] );
	void SetViewport( int x0, int y0, int x1, int y1 );
	void SetScissorRect( int* rect );
	
	SGRX_ITexture* CreateTexture( TextureInfo* texinfo, void* data = NULL );
	SGRX_ITexture* CreateRenderTexture( TextureInfo* texinfo );
	SGRX_IDepthStencilSurface* CreateDepthStencilSurface( int width, int height, int format );
	bool CompileShader( const StringView& path, EShaderType shadertype, const StringView& code, ByteArray& outcomp, String& outerrors );
	SGRX_IVertexShader* CreateVertexShader( const StringView& path, ByteArray& code );
	SGRX_IPixelShader* CreatePixelShader( const StringView& path, ByteArray& code );
	SGRX_IRenderState* CreateRenderState( const SGRX_RenderState& state );
	SGRX_IVertexDecl* CreateVertexDecl( const VDeclInfo& vdinfo );
	SGRX_IMesh* CreateMesh();
	SGRX_IVertexInputMapping* CreateVertexInputMapping( SGRX_IVertexShader* vs, SGRX_IVertexDecl* vd ){ return NULL; }
	
	void SetMatrix( bool view, const Mat4& mtx );
	void DrawImmediate( SGRX_ImmDrawData& idd );
	
	virtual void DoRenderItems( SGRX_Scene* scene, uint8_t pass_id, int maxrepeat, const SGRX_Camera& cam, RenderItem* start, RenderItem* end );
	
#if 0
	void _RS_RenderPass_Projectors( size_t pass_id );
	void _RS_RenderPass_LightVols( IDirect3DBaseTexture9* tx_depth, const RTOutInfo& RTOUT );
#endif
	
	bool ResetDevice();
	void ResetViewport(){ SetViewport( 0, 0, m_currSettings.width, m_currSettings.height ); }
	void _SetTextureInt( int slot, IDirect3DBaseTexture9* tex, uint32_t flags );
	void SetTexture( int slot, const SGRX_ITexture* tex );
	void SetVertexShader( const SGRX_IVertexShader* shd );
	void SetPixelShader( const SGRX_IPixelShader* shd );
	void SetRenderState( const SGRX_RenderState& nrs );
	void SetRenderState( const SGRX_IRenderState* rsi );
	
	FINLINE void VS_SetVec4Array( int at, const Vec4* arr, int size ){ m_dev->SetVertexShaderConstantF( at, &arr->x, size ); }
	FINLINE void VS_SetVec4Array( int at, const float* arr, int size ){ m_dev->SetVertexShaderConstantF( at, arr, size ); }
	FINLINE void VS_SetVec4( int at, const Vec4& v ){ m_dev->SetVertexShaderConstantF( at, &v.x, 1 ); }
	FINLINE void VS_SetFloat( int at, float f ){ Vec4 v = {f,f,f,f}; m_dev->SetVertexShaderConstantF( at, &v.x, 1 ); }
	FINLINE void VS_SetMat4( int at, const Mat4& v ){ m_dev->SetVertexShaderConstantF( at, v.a, 4 ); }
	
	FINLINE void PS_SetVec4Array( int at, const Vec4* arr, int size ){ m_dev->SetPixelShaderConstantF( at, &arr->x, size ); }
	FINLINE void PS_SetVec4Array( int at, const float* arr, int size ){ m_dev->SetPixelShaderConstantF( at, arr, size ); }
	FINLINE void PS_SetVec4( int at, const Vec4& v ){ m_dev->SetPixelShaderConstantF( at, &v.x, 1 ); }
	FINLINE void PS_SetFloat( int at, float f ){ Vec4 v = {f,f,f,f}; m_dev->SetPixelShaderConstantF( at, &v.x, 1 ); }
	FINLINE void PS_SetMat4( int at, const Mat4& v ){ m_dev->SetPixelShaderConstantF( at, v.a, 4 ); }
	
	FINLINE int GetWidth() const { return m_params.BackBufferWidth; }
	FINLINE int GetHeight() const { return m_params.BackBufferHeight; }
	
	// state
	SGRX_RenderState m_crs;
	
	// helpers
	Mat4 m_world;
	Mat4 m_view;
	
	// storage
	HashTable< D3D9Texture*, bool > m_ownTextures;
	HashTable< D3D9Mesh*, bool > m_ownMeshes;
	HashTable< D3D9DepthStencilSurface*, bool > m_ownDSS;
	
	// specific
	IDirect3DDevice9* m_dev;
	D3DPRESENT_PARAMETERS m_params;
	IDirect3DSurface9* m_backbuf;
	IDirect3DSurface9* m_dssurf;
	
	// temp data
	SceneHandle m_currentScene;
	D3DVIEWPORT9 m_viewport;
};

extern "C" RENDERER_EXPORT bool Initialize( const char** outname )
{
	*outname = "Direct3D9";
	g_D3D = Direct3DCreate9( D3D_SDK_VERSION );
	return g_D3D != NULL;
}

extern "C" RENDERER_EXPORT void Free()
{
	g_D3D->Release();
	g_D3D = NULL;
}

static D3DMULTISAMPLE_TYPE aa_quality_to_mstype( int aaq )
{
	if( aaq <= 0 )
		return D3DMULTISAMPLE_NONE;
	if( aaq > 16 )
		return D3DMULTISAMPLE_16_SAMPLES;
	return (D3DMULTISAMPLE_TYPE) aaq;
}

extern "C" RENDERER_EXPORT IRenderer* CreateRenderer( const RenderSettings& settings, void* windowHandle )
{
	LOG_FUNCTION;
	
	D3DPRESENT_PARAMETERS d3dpp;
	IDirect3DDevice9* d3ddev;
	
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = settings.fullscreen != FULLSCREEN_NORMAL;
	d3dpp.FullScreen_RefreshRateInHz = d3dpp.Windowed ? 0 : settings.refresh_rate;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = 1;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.hDeviceWindow = (HWND) windowHandle;
	d3dpp.BackBufferWidth = settings.width;
	d3dpp.BackBufferHeight = settings.height;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.MultiSampleType = settings.aa_mode == ANTIALIAS_MULTISAMPLE ? aa_quality_to_mstype( settings.aa_quality ) : D3DMULTISAMPLE_NONE;
	d3dpp.PresentationInterval = settings.vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	
	if( FAILED( g_D3D->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		(HWND) windowHandle,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED,
		&d3dpp,
		&d3ddev ) ) )
	{
		LOG << "Failed to create the D3D9 device";
		return NULL;
	}
	
	_ss_reset_states( d3ddev, settings.width, settings.height );
	
	D3D9Renderer* R = new D3D9Renderer;
	R->m_dev = d3ddev;
	R->m_params = d3dpp;
	R->m_currSettings = settings;
	
	SGRX_RenderState rs;
	rs.Init();
	R->SetRenderState( rs ); // assume m_crs is initialized in the opposite way
	
	if( FAILED( R->m_dev->GetRenderTarget( 0, &R->m_backbuf ) ) )
	{
		LOG_ERROR << "Failed to retrieve the original render target";
		return NULL;
	}
	if( FAILED( R->m_dev->GetDepthStencilSurface( &R->m_dssurf ) ) )
	{
		LOG_ERROR << "Failed to retrieve the original depth/stencil surface";
		return NULL;
	}
	
	d3ddev->BeginScene();
	
	return R;
}


void D3D9Renderer::Destroy()
{
	LOG_FUNCTION;
	
	LOG << "D3D9Renderer::Destroy()";
	
	if( m_ownMeshes.size() )
	{
		LOG << "Unfreed meshes: " << m_ownMeshes.size();
		for( size_t i = 0; i < m_ownMeshes.size(); ++i )
		{
			SGRX_IMesh* M = m_ownMeshes.item( i ).key;
			LOG << "> " << M->m_key << " (" << M->m_meshParts.size() << " parts)";
			for( size_t p = 0; p < M->m_meshParts.size(); ++p )
			{
				SGRX_MeshPart& MP = M->m_meshParts[ p ];
				StringView TEX = MP.textures[ 0 ];
				if( TEX )
					LOG << "Part " << p << " texture 0: " << TEX;
				else
					LOG << "Part " << p << " - material has no texture";
			}
		}
	}
	if( m_ownTextures.size() )
	{
		LOG << "Unfreed textures: " << m_ownTextures.size();
		for( size_t i = 0; i < m_ownTextures.size(); ++i )
			LOG << "> " << m_ownTextures.item( i ).key->m_key;
	}
	if( m_ownDSS.size() )
	{
		LOG << "Unfreed depth/stencil surfaces: " << m_ownDSS.size();
	}
	
	m_ownMeshes.clear();
	m_ownTextures.clear();
	m_ownDSS.clear();
	
	SAFE_RELEASE( m_backbuf );
	SAFE_RELEASE( m_dssurf );
	SAFE_RELEASE( m_dev );
	delete this;
}

bool D3D9Renderer::LoadInternalResources()
{
	return true;
}

void D3D9Renderer::UnloadInternalResources()
{
}

void D3D9Renderer::Swap()
{
	m_dev->EndScene();
	if( m_dev->Present( NULL, NULL, NULL, NULL ) == D3DERR_DEVICELOST )
	{
		if( m_dev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET )
		{
			ResetDevice();
		}
	}
	m_dev->BeginScene();
}

void D3D9Renderer::Modify( const RenderSettings& settings )
{
	bool needreset =
		m_currSettings.width != settings.width ||
		m_currSettings.height != settings.height ||
		m_currSettings.refresh_rate != settings.refresh_rate ||
		m_currSettings.fullscreen != settings.fullscreen ||
		m_currSettings.vsync != settings.vsync ||
		m_currSettings.aa_mode != settings.aa_mode ||
		m_currSettings.aa_quality != settings.aa_quality;
	m_currSettings = settings;
	
	if( needreset )
	{
		m_params.BackBufferWidth = settings.width;
		m_params.BackBufferHeight = settings.height;
		m_params.Windowed = settings.fullscreen != FULLSCREEN_NORMAL;
		m_params.FullScreen_RefreshRateInHz = m_params.Windowed ? 0 : settings.refresh_rate;
		m_params.PresentationInterval = settings.vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
		m_params.MultiSampleType = settings.aa_mode == ANTIALIAS_MULTISAMPLE ? aa_quality_to_mstype( settings.aa_quality ) : D3DMULTISAMPLE_NONE;
		
		m_dev->EndScene();
		ResetDevice();
		ResetViewport();
		m_dev->BeginScene();
	}
}


void D3D9Renderer::SetRenderTargets( const SGRX_RTClearInfo& info, SGRX_IDepthStencilSurface* dss, TextureHandle rts[4] )
{
	if( rts[0] == NULL && rts[1] == NULL && rts[2] == NULL && rts[3] == NULL )
	{
		m_dev->SetRenderTarget( 0, m_backbuf );
		m_dev->SetRenderTarget( 1, NULL );
		m_dev->SetRenderTarget( 2, NULL );
		m_dev->SetRenderTarget( 3, NULL );
		m_dev->SetDepthStencilSurface( dss ? ((D3D9DepthStencilSurface*)dss)->DSS : m_dssurf );
		ResetViewport();
	}
	else
	{
		uint32_t w = 0, h = 0;
		for( int i = 0; i < 4; ++i )
		{
			SGRX_ITexture* rt = rts[ i ];
			if( rt )
			{
				ASSERT( !w || w == rt->m_info.width );
				ASSERT( !h || h == rt->m_info.height );
				w = rt->m_info.width;
				h = rt->m_info.height;
				ASSERT( rt->m_isRenderTexture );
				SGRX_CAST( D3D9RenderTexture*, RTT, rt );
				m_dev->SetRenderTarget( i, RTT->CS );
			}
			else
			{
				m_dev->SetRenderTarget( i, NULL );
			}
		}
		m_dev->SetDepthStencilSurface( dss ? ((D3D9DepthStencilSurface*)dss)->DSS : NULL );
		SetViewport( 0, 0, w, h );
	}
	
	// clear buffers
	uint32_t cc = info.clearColor;
	swap4b2ms( &cc, 1, 0xff0000, 16, 0xff, 16 );
	uint32_t flags = 0;
	if( info.flags & SGRX_RT_ClearColor )
		flags |= D3DCLEAR_TARGET;
	if( info.flags & SGRX_RT_ClearDepth )
		flags |= D3DCLEAR_ZBUFFER;
	if( info.flags & SGRX_RT_ClearStencil )
		flags |= D3DCLEAR_STENCIL;
	m_dev->Clear( 0, NULL, flags, cc, info.clearDepth, info.clearStencil );
}

void D3D9Renderer::SetViewport( int x0, int y0, int x1, int y1 )
{
	D3DVIEWPORT9 vp = { x0, y0, x1 - x0, y1 - y0, 0.0, 1.0 };
	m_viewport = vp;
	m_dev->SetViewport( &vp );
}

void D3D9Renderer::SetScissorRect( int* rect )
{
	if( rect )
	{
		RECT r = { rect[0], rect[1], rect[2], rect[3] };
		m_dev->SetScissorRect( &r );
	}
	else
	{
		RECT r = { m_viewport.X, m_viewport.Y, m_viewport.X + m_viewport.Width, m_viewport.Y + m_viewport.Height };
		m_dev->SetScissorRect( &r );
	}
}


D3D9Texture::~D3D9Texture()
{
	m_renderer->m_ownTextures.unset( this );
	SAFE_RELEASE( m_ptr.base );
}

SGRX_ITexture* D3D9Renderer::CreateTexture( TextureInfo* texinfo, void* data )
{
	LOG_FUNCTION;
	
	HRESULT hr;
	// TODO: filter unsupported formats / dimensions
	
	if( texinfo->type == TEXTYPE_2D )
	{
		IDirect3DTexture9* d3dtex;
		
		hr = m_dev->CreateTexture( texinfo->width, texinfo->height, texinfo->mipcount, 0, texfmt2d3d( texinfo->format ), D3DPOOL_MANAGED, &d3dtex, NULL );
		if( FAILED( hr ) )
		{
			LOG_ERROR << "could not create D3D9 texture (type: 2D, w: " << texinfo->width << ", h: " <<
				texinfo->height << ", mips: " << texinfo->mipcount << ", fmt: " << texinfo->format << ", d3dfmt: " << texfmt2d3d( texinfo->format );
			return NULL;
		}
		
		if( data )
		{
			// load all mip levels into it
			for( int mip = 0; mip < texinfo->mipcount; ++mip )
			{
				D3DLOCKED_RECT lr;
				hr = d3dtex->LockRect( mip, &lr, NULL, D3DLOCK_DISCARD );
				if( FAILED( hr ) )
				{
					LOG_ERROR << "failed to lock D3D9 texture";
					return NULL;
				}
				
				texdatacopy( &lr, texinfo, data, 0, mip );
				
				hr = d3dtex->UnlockRect( mip );
				if( FAILED( hr ) )
				{
					LOG_ERROR << "failed to unlock D3D9 texture";
					return NULL;
				}
			}
		}
		
		D3D9Texture* T = new D3D9Texture;
		T->m_renderer = this;
		T->m_info = *texinfo;
		T->m_ptr.tex2d = d3dtex;
		m_ownTextures.set( T, true );
		return T;
	}
	else if( texinfo->type == TEXTYPE_CUBE )
	{
		IDirect3DCubeTexture9* d3dtex;
		
		hr = m_dev->CreateCubeTexture( texinfo->width, texinfo->mipcount, 0, texfmt2d3d( texinfo->format ), D3DPOOL_MANAGED, &d3dtex, NULL );
		
		if( data )
		{
			// load all mip levels into it
			for( int side = 0; side < 6; ++side )
			{
				for( int mip = 0; mip < texinfo->mipcount; ++mip )
				{
					D3DLOCKED_RECT lr;
					hr = d3dtex->LockRect( (D3DCUBEMAP_FACES) side, mip, &lr, NULL, D3DLOCK_DISCARD );
					if( FAILED( hr ) )
					{
						LOG_ERROR << "failed to lock D3D9 texture";
						return NULL;
					}
					
					texdatacopy( &lr, texinfo, data, side, mip );
					
					hr = d3dtex->UnlockRect( (D3DCUBEMAP_FACES) side, mip );
					if( FAILED( hr ) )
					{
						LOG_ERROR << "failed to unlock D3D9 texture";
						return NULL;
					}
				}
			}
		}
		
		D3D9Texture* T = new D3D9Texture;
		T->m_renderer = this;
		T->m_info = *texinfo;
		T->m_ptr.cube = d3dtex;
		m_ownTextures.set( T, true );
		return T;
	}
	
	LOG_ERROR << "TODO [reached a part of not-yet-defined behavior]";
	return NULL;
}


SGRX_ITexture* D3D9Renderer::CreateRenderTexture( TextureInfo* texinfo )
{
	LOG_FUNCTION;
	
	D3DFORMAT d3dfmt = (D3DFORMAT) 0;
	HRESULT hr = 0;
	int width = texinfo->width, height = texinfo->height, format = texinfo->format;
	
	switch( format )
	{
	case RT_FORMAT_COLOR_HDR16:
		d3dfmt = D3DFMT_A16B16G16R16F;
		break;
	case RT_FORMAT_COLOR_LDR8:
		d3dfmt = D3DFMT_A8R8G8B8;
		break;
	case RT_FORMAT_DEPTH:
		d3dfmt = D3DFMT_R32F;
		break;
	default:
		LOG_ERROR << "format ID was not recognized / supported: " << format;
		return NULL;
	}
	
	
	IDirect3DTexture9 *CT = NULL, *DT = NULL;
	IDirect3DSurface9 *CS = NULL, *DS = NULL;
	D3D9RenderTexture* RT = NULL;
	
	if( format == RT_FORMAT_DEPTH )
	{
		hr = m_dev->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, d3dfmt, D3DPOOL_DEFAULT, &CT, NULL );
		if( FAILED( hr ) || !CT )
		{
			LOG_ERROR << "failed to create D3D9 render target texture (HRESULT=" << (void*) hr << ")";
			goto cleanup;
		}
		
		hr = CT->GetSurfaceLevel( 0, &CS );
		if( FAILED( hr ) || !CS )
		{
			LOG_ERROR << "failed to get surface from D3D9 render target texture (HRESULT=" << (void*) hr << ")";
			goto cleanup;
		}
	}
	else
	{
		hr = m_dev->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, d3dfmt, D3DPOOL_DEFAULT, &CT, NULL );
		if( FAILED( hr ) || !CT )
		{
			LOG_ERROR << "failed to create D3D9 render target texture (HRESULT=" << (void*) hr << ")";
			goto cleanup;
		}
		
		hr = m_dev->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &DT, NULL );
		if( FAILED( hr ) || !DT )
		{
			LOG_ERROR << "failed to create D3D9 r32f depth stencil texture (HRESULT=" << (void*) hr << ")";
			goto cleanup;
		}
		
		hr = CT->GetSurfaceLevel( 0, &CS );
		if( FAILED( hr ) || !CS )
		{
			LOG_ERROR << "failed to get surface from D3D9 render target texture (HRESULT=" << (void*) hr << ")";
			goto cleanup;
		}
		
		hr = DT->GetSurfaceLevel( 0, &DS );
		if( FAILED( hr ) || !CS )
		{
			LOG_ERROR << "failed to get surface from D3D9 depth stencil texture (HRESULT=" << (void*) hr << ")";
			goto cleanup;
		}
	}
	
	RT = new D3D9RenderTexture;
	RT->m_renderer = this;
	RT->m_info = *texinfo;
	RT->m_ptr.tex2d = CT;
	RT->CS = CS;
	RT->DS = DS;
	RT->DT = DT;
	m_ownTextures.set( RT, true );
	return RT;
	
cleanup:
	SAFE_RELEASE( CS );
	SAFE_RELEASE( DS );
	SAFE_RELEASE( DT );
	SAFE_RELEASE( CT );
	return NULL;
}


D3D9DepthStencilSurface::~D3D9DepthStencilSurface()
{
	m_renderer->m_ownDSS.unset( this );
	SAFE_RELEASE( DSS );
}

SGRX_IDepthStencilSurface* D3D9Renderer::CreateDepthStencilSurface( int width, int height, int format )
{
	LOG_FUNCTION;
	
	HRESULT hr = 0;

	switch( format )
	{
	case RT_FORMAT_COLOR_HDR16:
	case RT_FORMAT_COLOR_LDR8:
	case RT_FORMAT_DEPTH:
		break;
	default:
		LOG_ERROR << "format ID was not recognized / supported: " << format;
		return NULL;
	}
	
	D3D9DepthStencilSurface* outDSS = NULL;
	
	IDirect3DSurface9 *DSS = NULL;
	
	hr = m_dev->CreateDepthStencilSurface( width, height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &DSS, NULL );
	if( FAILED( hr ) || !DSS )
	{
		LOG_ERROR << "failed to create D3D9 d24s8 depth+stencil surface (HRESULT=" << (void*) hr << ")";
		goto cleanup;
	}
	
	outDSS = new D3D9DepthStencilSurface;
	outDSS->DSS = DSS;
	outDSS->m_renderer = this;
	m_ownDSS.set( outDSS, true );
	return outDSS;
	
cleanup:
	SAFE_RELEASE( DSS );
	return NULL;
}


bool D3D9Renderer::CompileShader( const StringView& path, EShaderType shadertype, const StringView& code, ByteArray& outcomp, String& outerrors )
{
	LOG_FUNCTION;
	
	HRESULT hr;
	ID3DXBuffer *outbuf = NULL, *outerr = NULL;
	
	static const D3DXMACRO vsmacros[] = { { "VS", "1" }, { NULL, NULL } };
	static const D3DXMACRO psmacros[] = { { "PS", "1" }, { NULL, NULL } };
	
	ByteWriter bw( &outcomp );
	
	const D3DXMACRO* macros = NULL;
	const char* tyname = "unknown";
	const char* profile = "---";
	switch( shadertype )
	{
	case ShaderType_Vertex:
		macros = vsmacros;
		tyname = "vertex";
		profile = "vs_3_0";
		bw.marker( "CVSH\x7f", 5 );
		break;
	case ShaderType_Pixel:
		macros = psmacros;
		tyname = "pixel";
		profile = "ps_3_0";
		bw.marker( "CPSH\x7f", 5 );
		break;
	}
	
	hr = D3DXCompileShader( code.data(), code.size(), macros, NULL, "main", profile, D3DXSHADER_OPTIMIZATION_LEVEL3, &outbuf, &outerr, NULL );
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
}

SGRX_IVertexShader* D3D9Renderer::CreateVertexShader( const StringView& path, ByteArray& code )
{
	LOG_FUNCTION;
	
	HRESULT hr;
	ByteReader br( &code );
	br.marker( "CVSH\x7f", 5 );
	
	int32_t len = 0;
	br << len;
	uint8_t* buf = &code[ 9 ];
	if( br.error )
		return NULL;
	
	IDirect3DVertexShader9* VS = NULL;
	D3D9VertexShader* out = NULL;
	
	hr = m_dev->CreateVertexShader( (const DWORD*) buf, &VS );
	if( FAILED( hr ) || !VS )
		{ LOG << "Failed to create a D3D9 vertex shader - " << path; goto cleanup; }
	
	out = new D3D9VertexShader( this );
	out->m_VS = VS;
	return out;
	
cleanup:
	SAFE_RELEASE( VS );
	return NULL;
}

SGRX_IPixelShader* D3D9Renderer::CreatePixelShader( const StringView& path, ByteArray& code )
{
	LOG_FUNCTION;
	
	HRESULT hr;
	ByteReader br( &code );
	br.marker( "CPSH\x7f", 5 );
	
	int32_t len = 0;
	br << len;
	uint8_t* buf = &code[ 9 ];
	if( br.error )
		return NULL;
	
	IDirect3DPixelShader9* PS = NULL;
	D3D9PixelShader* out = NULL;
	
	hr = m_dev->CreatePixelShader( (const DWORD*) buf, &PS );
	if( FAILED( hr ) || !PS )
		{ LOG << "Failed to create a D3D9 pixel shader - " << path; goto cleanup; }
	
	out = new D3D9PixelShader( this );
	out->m_PS = PS;
	return out;
	
cleanup:
	SAFE_RELEASE( PS );
	return NULL;
}


SGRX_IRenderState* D3D9Renderer::CreateRenderState( const SGRX_RenderState& state )
{
	return new D3D9RenderState( state );
}


static int vdecltype_to_eltype[] =
{
	D3DDECLTYPE_FLOAT1,
	D3DDECLTYPE_FLOAT2,
	D3DDECLTYPE_FLOAT3,
	D3DDECLTYPE_FLOAT4,
	D3DDECLTYPE_UBYTE4N, // D3DCOLOR with R/B fixed
};

static int vdeclusage_to_elusage[] =
{
	D3DDECLUSAGE_POSITION,
	D3DDECLUSAGE_COLOR,
	D3DDECLUSAGE_NORMAL,
	D3DDECLUSAGE_TANGENT,
	D3DDECLUSAGE_BLENDWEIGHT,
	D3DDECLUSAGE_BLENDINDICES,
	D3DDECLUSAGE_TEXCOORD,
	D3DDECLUSAGE_TEXCOORD,
	D3DDECLUSAGE_TEXCOORD,
	D3DDECLUSAGE_TEXCOORD,
};

static int vdeclusage_to_elusageindex[] = { 0, 0, 0, 0, 0, 0, 0, 1, 2, 3 };

SGRX_IVertexDecl* D3D9Renderer::CreateVertexDecl( const VDeclInfo& vdinfo )
{
	LOG_FUNCTION;
	
	D3DVERTEXELEMENT9 elements[ VDECL_MAX_ITEMS + 1 ], end[1] = { D3DDECL_END() };
	for( int i = 0; i < vdinfo.count; ++i )
	{
		elements[ i ].Stream = 0;
		elements[ i ].Offset = vdinfo.offsets[ i ];
		elements[ i ].Type = vdecltype_to_eltype[ vdinfo.types[ i ] ];
		elements[ i ].Method = D3DDECLMETHOD_DEFAULT;
		elements[ i ].Usage = vdeclusage_to_elusage[ vdinfo.usages[ i ] ];
		elements[ i ].UsageIndex = vdeclusage_to_elusageindex[ vdinfo.usages[ i ] ];
		if( vdinfo.usages[ i ] == VDECLUSAGE_BLENDIDX && vdinfo.types[ i ] == VDECLTYPE_BCOL4 )
			elements[ i ].Type = D3DDECLTYPE_UBYTE4;
		if( ( vdinfo.usages[ i ] == VDECLUSAGE_BLENDWT || elements[ i ].Usage == D3DDECLUSAGE_TEXCOORD )
			&& vdinfo.types[ i ] == VDECLTYPE_BCOL4 )
			elements[ i ].Type = D3DDECLTYPE_UBYTE4N;
	}
	memcpy( elements + vdinfo.count, end, sizeof(*end) );
	
	IDirect3DVertexDeclaration9* VD = NULL;
	if( FAILED( m_dev->CreateVertexDeclaration( elements, &VD ) ) || !VD )
	{
		LOG_ERROR << "Failed to create D3D9 vertex declaration";
		return NULL;
	}
	
	D3D9VertexDecl* vdecl = new D3D9VertexDecl;
	vdecl->m_vdecl = VD;
	vdecl->m_info = vdinfo;
	vdecl->m_renderer = this;
	return vdecl;
}


D3D9Mesh::~D3D9Mesh()
{
	m_renderer->m_ownMeshes.unset( this );
	SAFE_RELEASE( m_VB );
	SAFE_RELEASE( m_IB );
}

bool D3D9Mesh::InitVertexBuffer( size_t size, VertexDeclHandle vd )
{
	LOG_FUNCTION;
	
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	SAFE_RELEASE( m_VB );
	m_renderer->m_dev->CreateVertexBuffer( size, dyn ? D3DUSAGE_DYNAMIC : 0, 0, dyn ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &m_VB, NULL );
	if( !m_VB )
	{
		LOG_ERROR << "failed to create D3D9 vertex buffer (size=" << size << ")";
		return false;
	}
	m_vertexDataSize = size;
	m_vertexDecl = vd;
	return true;
}

bool D3D9Mesh::InitIndexBuffer( size_t size, bool i32 )
{
	LOG_FUNCTION;
	
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	SAFE_RELEASE( m_IB );
	m_renderer->m_dev->CreateIndexBuffer( size, dyn ? D3DUSAGE_DYNAMIC : 0, i32 ? D3DFMT_INDEX32 : D3DFMT_INDEX16, dyn ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &m_IB, NULL );
	if( !m_IB )
	{
		LOG_ERROR << "failed to create D3D9 index buffer (size=" << size << ", i32=" << i32 << ")";
		return false;
	}
	m_dataFlags = ( m_dataFlags & ~MDF_INDEX_32 ) | ( MDF_INDEX_32 * i32 );
	m_indexDataSize = size;
	return true;
}

bool D3D9Mesh::UpdateVertexData( const void* data, size_t size, bool tristrip )
{
	LOG_FUNCTION;
	
	void* vb_data;
	
	if( size > m_vertexDataSize )
	{
		LOG_ERROR << "given vertex data is too big";
		return false;
	}
	
	if( FAILED( m_VB->Lock( 0, 0, &vb_data, D3DLOCK_DISCARD ) ) )
	{
		LOG_ERROR << "failed to lock D3D9 vertex buffer";
		return false;
	}
	
	memcpy( vb_data, data, size );
	
	if( FAILED( m_VB->Unlock() ) )
	{
		LOG_ERROR << "failed to unlock D3D9 vertex buffer";
		return false;
	}
	
	return true;
}

bool D3D9Mesh::UpdateIndexData( const void* data, size_t size )
{
	LOG_FUNCTION;
	
	void* ib_data;
	
	if( size > m_indexDataSize )
	{
		LOG_ERROR << "given index data is too big";
		return false;
	}
	
	if( FAILED( m_IB->Lock( 0, 0, &ib_data, D3DLOCK_DISCARD ) ) )
	{
		LOG_ERROR << "failed to lock D3D9 index buffer";
		return false;
	}
	
	memcpy( ib_data, data, size );
	
	if( FAILED( m_IB->Unlock() ) )
	{
		LOG_ERROR << "failed to unlock D3D9 index buffer";
		return false;
	}
	
	return true;
}

bool D3D9Mesh::OnDeviceLost()
{
	LOG_FUNCTION;
	
	void *src_data, *dst_data;
	const char* reason = NULL;
	if( m_dataFlags & MDF_DYNAMIC )
	{
		int i32 = m_dataFlags & MDF_INDEX_32;
		IDirect3DVertexBuffer9* tmpVB = NULL;
		IDirect3DIndexBuffer9* tmpIB = NULL;
		
		if( m_vertexDataSize )
		{
			if( FAILED( m_renderer->m_dev->CreateVertexBuffer( m_vertexDataSize, D3DUSAGE_DYNAMIC, 0, D3DPOOL_SYSTEMMEM, &tmpVB, NULL ) ) )
				{ reason = "failed to create temp. VB"; goto fail; }
			src_data = dst_data = NULL;
			if( FAILED( m_VB->Lock( 0, 0, &src_data, 0 ) ) ){ reason = "failed to lock orig. VB"; goto fail; }
			if( FAILED( tmpVB->Lock( 0, 0, &dst_data, 0 ) ) ){ reason = "failed to lock temp. VB"; goto fail; }
			memcpy( dst_data, src_data, m_vertexDataSize );
			if( FAILED( tmpVB->Unlock() ) ){ reason = "failed to unlock orig. VB"; goto fail; }
			if( FAILED( m_VB->Unlock() ) ){ reason = "failed to unlock temp. VB"; goto fail; }
		}
		SAFE_RELEASE( m_VB );
		m_VB = tmpVB;
		
		if( m_indexDataSize )
		{
			if( FAILED( m_renderer->m_dev->CreateIndexBuffer( m_indexDataSize, D3DUSAGE_DYNAMIC, i32 ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &tmpIB, NULL ) ) )
				{ reason = "failed to create temp. IB"; goto fail; }
			src_data = dst_data = NULL;
			if( FAILED( m_IB->Lock( 0, 0, &src_data, 0 ) ) ){ reason = "failed to lock orig. IB"; goto fail; }
			if( FAILED( tmpIB->Lock( 0, 0, &dst_data, 0 ) ) ){ reason = "failed to lock temp. IB"; goto fail; }
			memcpy( dst_data, src_data, m_indexDataSize );
			if( FAILED( tmpIB->Unlock() ) ){ reason = "failed to unlock orig. IB"; goto fail; }
			if( FAILED( m_IB->Unlock() ) ){ reason = "failed to unlock temp. IB"; goto fail; }
		}
		SAFE_RELEASE( m_IB );
		m_IB = tmpIB;
	}
	
	return true;
	
fail:
	LOG_ERROR << "failed to handle lost device mesh: %s" << ( reason ? reason : "<unknown reason>" );
	return false;
}

bool D3D9Mesh::OnDeviceReset()
{
	LOG_FUNCTION;
	
	void *src_data, *dst_data;
	if( m_dataFlags & MDF_DYNAMIC )
	{
		int i32 = m_dataFlags & MDF_INDEX_32;
		IDirect3DVertexBuffer9* tmpVB = NULL;
		IDirect3DIndexBuffer9* tmpIB = NULL;
		
		if( m_vertexDataSize )
		{
			if( FAILED( m_renderer->m_dev->CreateVertexBuffer( m_vertexDataSize, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &tmpVB, NULL ) ) ) goto fail;
			src_data = dst_data = NULL;
			if( FAILED( m_VB->Lock( 0, 0, &src_data, 0 ) ) ) goto fail;
			if( FAILED( tmpVB->Lock( 0, 0, &dst_data, D3DLOCK_DISCARD ) ) ) goto fail;
			memcpy( dst_data, src_data, m_vertexDataSize );
			if( FAILED( tmpVB->Unlock() ) ) goto fail;
			if( FAILED( m_VB->Unlock() ) ) goto fail;
		}
		SAFE_RELEASE( m_VB );
		m_VB = tmpVB;
		
		if( m_indexDataSize )
		{
			if( FAILED( m_renderer->m_dev->CreateIndexBuffer( m_indexDataSize, D3DUSAGE_DYNAMIC, i32 ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_DEFAULT, &tmpIB, NULL ) ) ) goto fail;
			src_data = dst_data = NULL;
			if( FAILED( m_IB->Lock( 0, 0, &src_data, 0 ) ) ) goto fail;
			if( FAILED( tmpIB->Lock( 0, 0, &dst_data, D3DLOCK_DISCARD ) ) ) goto fail;
			memcpy( dst_data, src_data, m_indexDataSize );
			if( FAILED( tmpIB->Unlock() ) ) goto fail;
			if( FAILED( m_IB->Unlock() ) ) goto fail;
		}
		SAFE_RELEASE( m_IB );
		m_IB = tmpIB;
	}
	
	return true;
	
fail:
	LOG_ERROR << "failed to handle reset device mesh";
	return false;
}

SGRX_IMesh* D3D9Renderer::CreateMesh()
{
	LOG_FUNCTION;
	
	D3D9Mesh* mesh = new D3D9Mesh;
	mesh->m_renderer = this;
	m_ownMeshes.set( mesh, true );
	return mesh;
}


void D3D9Renderer::SetMatrix( bool view, const Mat4& mtx )
{
	if( view )
	{
		m_view = mtx;
	}
	else
	{
		m_world = mtx;
	}
}

FINLINE D3DPRIMITIVETYPE conv_prim_type( EPrimitiveType pt )
{
	switch( pt )
	{
	case PT_Points: return D3DPT_POINTLIST;
	case PT_Lines: return D3DPT_LINELIST;
	case PT_LineStrip: return D3DPT_LINESTRIP;
	case PT_Triangles: return D3DPT_TRIANGLELIST;
	case PT_TriangleStrip: return D3DPT_TRIANGLESTRIP;
	default: return (D3DPRIMITIVETYPE) 0;
	}
}

FINLINE uint32_t get_prim_count( EPrimitiveType pt, uint32_t numverts )
{
	switch( pt )
	{
	case PT_Points: return numverts;
	case PT_Lines: return numverts / 2;
	case PT_LineStrip: if( numverts < 2 ) return 0; return numverts - 1;
	case PT_Triangles: return numverts / 3;
	case PT_TriangleStrip: if( numverts < 3 ) return 0; return numverts - 2;
	default: return 0;
	}
}

void D3D9Renderer::DrawImmediate( SGRX_ImmDrawData& idd )
{
	float w = m_viewport.Width;
	float h = m_viewport.Height;
	Mat4 hpomtx = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  w ? -1.0f / w : 0, h ? 1.0f / h : 0, 0, 1 };
	
	SetVertexShader( idd.vertexShader );
	SetPixelShader( idd.pixelShader );
	SetRenderState( idd.renderState );
	VS_SetMat4( 0, m_world );
	VS_SetMat4( 4, m_view * hpomtx );
	for( int i = 0; i < SGRX_MAX_TEXTURES; ++i )
		SetTexture( i, idd.textures[ i ] );
	PS_SetVec4Array( 0, idd.shdata, idd.shvcount );
	m_dev->SetVertexDeclaration( ((D3D9VertexDecl*) idd.vertexDecl)->m_vdecl );
	m_dev->DrawPrimitiveUP( conv_prim_type( idd.primType ), get_prim_count( idd.primType, idd.vertexCount ), idd.vertices, idd.vertexDecl->m_info.size );
}


/*
	R E N D E R I N G
	
	=== CONSTANT INFO ===
- POINT LIGHT: (total size: 2 constants, max. count: 16)
	VEC4 [px, py, pz, radius]
	VEC4 [cr, cg, cb, power]
- SPOT LIGHT: (total size: 4 constants for each shader, 2 textures, max. count: 4)
	TEXTURES [cookie, shadowmap]
[pixel shader]:
	VEC4 [px, py, pz, radius]
	VEC4 [cr, cg, cb, power]
	VEC4 [dx, dy, dz, angle]
	VEC4 [ux, uy, uz, -]
[vertex shader]:
	VEC4x4 [shadow map matrix]
- AMBIENT + DIRECTIONAL (SUN) LIGHT: (total size: 3 constants)
	VEC4 [ar, ag, ab, -]
	VEC4 [dx, dy, dz, -]
	VEC4 [cr, cg, cb, -]
	TEXTURES [some number of shadowmaps]
- DIR. AMBIENT DATA: (total size: 6 constants)
	VEC4 colorXP, colorXN, colorYP, colorYN, colorZP, colorZN
- SKY DATA: (total size: 1 constant)
	VEC4 [skyblend, -, -, -]
- FOG DATA: (total size: 2 constants)
	VEC4 [cr, cg, cb, min.dst]
	VEC4 [h1, d1, h2, d2] (heights, densities)
- VERTEX SHADER
	0-3: world * camera view matrix
	4-7: camera projection matrix
	8-11: world matrix
	12-15: camera view matrix
	23: light counts (point, spot)
	24-39: VS spot light data
- PIXEL SHADER
	0-3: camera inverse view matrix
	4: camera position
	11: sky data
	12-13: fog data
	14-19: dir.amb. data
	20-22: dir. light
	23: light counts (point, spot)
	24-39: PS spot light data
	56-87: point light data
	---
	100-115: instance data
*/

void D3D9Renderer::DoRenderItems( SGRX_Scene* scene, uint8_t pass_id, int maxrepeat, const SGRX_Camera& cam, RenderItem* start, RenderItem* end )
{
	SGRX_RenderPass& PASS = scene->m_passes[ pass_id ];
	if( PASS.isShadowPass )
		maxrepeat = 1;
	
	SGRX_RPCoreData coredata =
	{
		cam.mView,
		cam.mProj,
		cam.mInvView,
		cam.position, 0,
		scene->m_timevals,
		V4( m_viewport.Width, m_viewport.Height,
			safe_fdiv( 1.0f, m_viewport.Width ),
			safe_fdiv( 1.0f, m_viewport.Height ) ),
		
		scene->ambientLightColor,
		-cam.mView.TransformNormal( scene->dirLightDir ).Normalized(),
		scene->dirLightColor,
	};
	VS_SetVec4Array( 0, (Vec4*) &coredata, sizeof(SGRX_RPCoreData) / sizeof(Vec4) );
	PS_SetVec4Array( 0, (Vec4*) &coredata, sizeof(SGRX_RPCoreData) / sizeof(Vec4) );
	
#if 0
	Vec3 fogGamma = scene->fogColor.Pow( 2.0 );
	Vec4 fogdata[ 2 ] =
	{
		{ fogGamma.x, fogGamma.y, fogGamma.z, scene->fogHeightFactor },
		{ scene->fogDensity, scene->fogHeightDensity, scene->fogStartHeight, scene->fogMinDist },
	};
	PS_SetVec4Array( 12, fogdata, 2 );
#endif
	
	RenderItem* RI = start;
	while( RI < end )
	{
		SGRX_MeshInstance* MI = RI->MI;
		uint16_t part_id = RI->part_id;
		
		D3D9Mesh* M = (D3D9Mesh*) MI->GetMesh();
		const SGRX_MeshPart& MP = M->m_meshParts[ part_id ];
		D3D9VertexDecl* VD = (D3D9VertexDecl*) M->m_vertexDecl.item;
		SGRX_DrawItem* DI = &MI->m_drawItems[ part_id ];
		const SGRX_Material& MTL = MI->GetMaterial( part_id );
		
		SGRX_SRSData& SRS = MI->GetSRSData( pass_id, part_id );
		if( SRS.RS == NULL || SRS.VS == NULL || SRS.PS == NULL )
		{
			RI++;
			continue;
		}
		SetRenderState( SRS.RS );
		SetVertexShader( SRS.VS );
		SetPixelShader( SRS.PS );
		
		// instance state
		for( int i = 0; i < SGRX_MAX_TEXTURES; ++i )
			SetTexture( i, MTL.textures[ i ] );
		if( MI->skin_matrices.size() )
			VS_SetVec4Array( 42, (const Vec4*) &MI->skin_matrices[0], MI->skin_matrices.size() * 4 );
		VS_SetVec4Array( 25, &MI->constants[0], 16 );
		PS_SetVec4Array( 25, &MI->constants[0], 16 );
		
		Mat4 mWorldView;
		mWorldView.Multiply( MI->matrix, cam.mView );
		VS_SetMat4( 17, MI->matrix );
		VS_SetMat4( 21, mWorldView );
		PS_SetMat4( 17, MI->matrix );
		PS_SetMat4( 21, mWorldView );
		
		m_dev->SetVertexDeclaration( VD->m_vdecl );
		m_dev->SetStreamSource( 0, M->m_VB, 0, VD->m_info.size );
		m_dev->SetIndices( M->m_IB );
		
		for( int numruns = 0; numruns < maxrepeat; ++numruns )
		{
			if( PASS.isShadowPass == false )
			{
				SGRX_RPPointLightData PLData[ 32 ];
				SGRX_RPSpotLightDataPS SLDataPS[ 2 ];
				SGRX_RPSpotLightDataVS SLDataVS[ 2 ];
				SGRX_Light* SLDataLT[ 2 ];
				LightCount LC = SGRX_Renderer_FindLights( cam, DI,
					TMIN( int(PASS.numPL), 32 ),
					TMIN( int(PASS.numSL), 2 ),
					PLData, SLDataPS, SLDataVS, SLDataLT );
				
				if( PASS.isBasePass == false && LC.numPL + LC.numSL <= 0 )
					break;
				
				if( LC.numPL )
				{
					VS_SetVec4Array( 170, (Vec4*) PLData, 2 * LC.numPL );
					PS_SetVec4Array( 170, (Vec4*) PLData, 2 * LC.numPL );
				}
				if( LC.numSL )
				{
					for( int i = 0; i < LC.numSL; ++i )
					{
						SetTexture( 12 + i * 2 + 0, SLDataLT[ i ]->cookieTexture );
						SetTexture( 12 + i * 2 + 1, SLDataLT[ i ]->shadowTexture );
					}
					VS_SetVec4Array( 202, (Vec4*) SLDataVS, 4 * LC.numSL );
					PS_SetVec4Array( 202, (Vec4*) SLDataVS, 4 * LC.numSL );
					VS_SetVec4Array( 210, (Vec4*) SLDataPS, 4 * LC.numSL );
					PS_SetVec4Array( 210, (Vec4*) SLDataPS, 4 * LC.numSL );
				}
				
				Vec4 lightcounts = { LC.numPL, LC.numSL, 0, 0 };
				VS_SetVec4( 41, lightcounts );
				PS_SetVec4( 41, lightcounts );
			}
			
			m_dev->DrawIndexedPrimitive(
				M->m_dataFlags & MDF_TRIANGLESTRIP ? D3DPT_TRIANGLESTRIP : D3DPT_TRIANGLELIST,
				MP.vertexOffset, 0, MP.vertexCount, MP.indexOffset, M->m_dataFlags & MDF_TRIANGLESTRIP ? MP.indexCount - 2 : MP.indexCount / 3 );
			
			m_stats.numDrawCalls++;
			m_stats.numMDrawCalls += PASS.isShadowPass == false;
			m_stats.numSDrawCalls += PASS.isShadowPass != false;
		}
		RI++;
	}
}

#if 0
void D3D9Renderer::_RS_RenderPass_Projectors( size_t pass_id )
{
	const SGRX_Camera& CAM = m_currentScene->camera;
	Mat4 camViewProj = CAM.mView * CAM.mProj;
	
	if( !_RS_UpdateProjectorMesh( m_currentScene ) )
		return;
	
	D3D9Mesh* M = (D3D9Mesh*) m_projectorMesh.item;
	if( !M->m_vertexDecl )
		return; /* mesh not initialized */
	D3D9VertexDecl* VD = (D3D9VertexDecl*) M->m_vertexDecl.item;
	
	VS_SetMat4( 0, camViewProj );
	VS_SetMat4( 8, Mat4::Identity );
	
	SGRX_RenderState rs;
	rs.Init();
	rs.blendStates[0].blendEnable = true;
	rs.blendStates[0].dstBlend = SGRX_RS_Blend_InvSrcAlpha;
	rs.depthBias = -1e-5f;
	rs.depthWriteEnable = false;
	rs.cullMode = SGRX_RS_CullMode_None;
	SetRenderState( rs );
	
	m_dev->SetVertexDeclaration( VD->m_vdecl );
	m_dev->SetStreamSource( 0, M->m_VB, 0, VD->m_info.size );
	m_dev->SetIndices( M->m_IB );
	SetVertexShader( m_sh_proj_vs );
	
	for( size_t part_id = 0; part_id < M->m_meshParts.size(); ++part_id )
	{
		const SGRX_MeshPart& MP = M->m_meshParts[ part_id ];
		SGRX_Light* L = m_projectorList[ part_id ];
		
		SGRX_IPixelShader* SHD = L->projectionShader;
		if( !SHD )
			continue;
		
		if( MP.indexCount < 3 )
			continue;
		
		SetPixelShader( SHD );
		for( int tex_id = 0; tex_id < SGRX_MAX_TEXTURES; ++tex_id )
			SetTexture( tex_id, L->projectionTextures[ tex_id ] );
		
		m_dev->DrawIndexedPrimitive(
			M->m_dataFlags & MDF_TRIANGLESTRIP ? D3DPT_TRIANGLESTRIP : D3DPT_TRIANGLELIST,
			MP.vertexOffset, 0, MP.vertexCount, MP.indexOffset, M->m_dataFlags & MDF_TRIANGLESTRIP ? MP.indexCount - 2 : MP.indexCount / 3 );
		m_stats.numDrawCalls++;
		m_stats.numSDrawCalls++;
	}
}

void D3D9Renderer::_RS_RenderPass_LightVols( IDirect3DBaseTexture9* tx_depth, const RTOutInfo& RTOUT )
{
	const SGRX_Camera& CAM = m_currentScene->camera;
	Mat4 vpmtx;
	vpmtx.Multiply( CAM.mView, CAM.mProj );
	
	SGRX_RenderState rs;
	rs.Init();
	rs.blendStates[0].blendEnable = true;
	rs.blendStates[0].dstBlend = SGRX_RS_Blend_One;
	rs.depthEnable = false;
	rs.cullMode = SGRX_RS_CullMode_None;
	SetRenderState( rs );
	
	m_dev->SetRenderTarget( 1, NULL );
	m_dev->SetRenderTarget( 2, NULL );
	m_dev->SetDepthStencilSurface( NULL );
	
	_SetTextureInt( 0, tx_depth, TEXTURE_FLAGS_FULLSCREEN );
	
	for( size_t light_id = 0; light_id < m_currentScene->m_lights.size(); ++light_id )
	{
		SGRX_Light* L = m_currentScene->m_lights.item( light_id ).key;
		if( !L->enabled ||
			L->type != LIGHT_SPOT || // only spotlights are currently supported
			!L->hasShadows || // they need to have shadows
			!L->shadowTexture ||
			!L->shadowTexture->m_isRenderTexture )
			continue;
		
		Vec3 volpts[8];
		L->GetVolumePoints( volpts );
		
		float dmin = FLT_MAX;
		float dmax = -FLT_MAX;
		for( int i = 0; i < 8; ++i )
		{
			float d = Vec3Dot( volpts[ i ], CAM.direction );
			if( d < dmin ) dmin = d;
			if( d > dmax ) dmax = d;
		}
		
		Vec3 pos00 = V3(0), pos10 = V3(0), pos01 = V3(0), pos11 = V3(0);
		Vec3 dir00 = V3(0), dir10 = V3(0), dir01 = V3(0), dir11 = V3(0);
		
		CAM.GetCursorRay( 0, 0, pos00, dir00 );
		CAM.GetCursorRay( 1, 0, pos10, dir10 );
		CAM.GetCursorRay( 0, 1, pos01, dir01 );
		CAM.GetCursorRay( 1, 1, pos11, dir11 );
		
		// normalize dirmin/dirmax along direction
		float pl;
		pl = Vec3Dot( dir00, CAM.direction ); if( pl ) dir00 /= pl;
		pl = Vec3Dot( dir10, CAM.direction ); if( pl ) dir10 /= pl;
		pl = Vec3Dot( dir01, CAM.direction ); if( pl ) dir01 /= pl;
		pl = Vec3Dot( dir11, CAM.direction ); if( pl ) dir11 /= pl;
		
		SetPixelShader( m_sh_lvsl_ps );
		PS_SetMat4( 0, vpmtx );
		PS_SetVec4( 4, V4( pos00, 0 ) );
		PS_SetVec4( 5, V4( pos10, 0 ) );
		PS_SetVec4( 6, V4( pos01, 0 ) );
		PS_SetVec4( 7, V4( pos11, 0 ) );
		PS_SetVec4( 8, V4( dir00, 0 ) );
		PS_SetVec4( 9, V4( dir10, 0 ) );
		PS_SetVec4( 10, V4( dir01, 0 ) );
		PS_SetVec4( 11, V4( dir11, 0 ) );
		PS_SetVec4( 12, V4( dmin, dmax, L->_tf_range, L->power ) );
		PS_SetVec4( 13, V4( L->_tf_position, 0 ) );
		PS_SetVec4( 14, V4( L->color, 0 ) );
		float tszx = 1, tszy = 1;
		if( L->shadowTexture )
		{
			const TextureInfo& texinfo = L->shadowTexture.GetInfo();
			tszx = texinfo.width;
			tszy = texinfo.height;
		}
		PS_SetVec4( 15, V4( tszx, tszy, 1.0f / tszx, 1.0f / tszy ) );
		PS_SetMat4( 16, L->viewProjMatrix );
		SetTexture( 8, L->cookieTexture );
		SetTexture( 9, L->shadowTexture );
		
		// TODO optimize shape
//		PostProcBlit( RTOUT.w, RTOUT.h, 1, -1 );
	}
	
//	Viewport_Apply( 1 );
}
#endif

bool D3D9Renderer::ResetDevice()
{
	D3DPRESENT_PARAMETERS npp;
	
	for( size_t i = 0; i < m_ownMeshes.size(); ++i )
	{
		D3D9Mesh* mesh = m_ownMeshes.item( i ).key;
		if( !mesh->OnDeviceLost() )
		{
			LOG_ERROR << "Failed to prepare for resetting mesh " << mesh << " (" << mesh->m_key << ")";
		}
	}
	for( size_t i = 0; i < m_ownTextures.size(); ++i )
	{
		D3D9Texture* tex = m_ownTextures.item( i ).key;
		if( !tex->OnDeviceLost() )
		{
			LOG_ERROR << "Failed to prepare for resetting texture " << tex << " (" << tex->m_key << ")";
		}
	}
	SAFE_RELEASE( m_backbuf );
	SAFE_RELEASE( m_dssurf );
	
	
	/* reset */
	npp = m_params;
	
	if( FAILED( m_dev->Reset( &npp ) ) )
	{
		LOG_ERROR << "Failed to reset D3D9 device";
		return false;
	}
	/* ----- */
	
	
	if( FAILED( m_dev->GetRenderTarget( 0, &m_backbuf ) ) )
	{
		LOG_ERROR << "Failed to retrieve the original render target";
		return NULL;
	}
	if( FAILED( m_dev->GetDepthStencilSurface( &m_dssurf ) ) )
	{
		LOG_ERROR << "Failed to retrieve the original depth/stencil surface";
		return NULL;
	}
	
	for( size_t i = 0; i < m_ownMeshes.size(); ++i )
	{
		D3D9Mesh* mesh = m_ownMeshes.item( i ).key;
		if( !mesh->OnDeviceReset() )
		{
			LOG_ERROR << "Failed to restore after resetting mesh " << mesh << " (" << mesh->m_key << ")";
		}
	}
	for( size_t i = 0; i < m_ownTextures.size(); ++i )
	{
		D3D9Texture* tex = m_ownTextures.item( i ).key;
		if( !tex->OnDeviceReset() )
		{
			LOG_ERROR << "Failed to restore after resetting texture " << tex << " (" << tex->m_key << ")";
		}
	}
	
	_ss_reset_states( m_dev, m_params.BackBufferWidth, m_params.BackBufferHeight );
	return true;
}


void D3D9Renderer::_SetTextureInt( int slot, IDirect3DBaseTexture9* tex, uint32_t flags )
{
	m_dev->SetTexture( slot, tex );
	if( tex )
	{
		m_dev->SetSamplerState( slot, D3DSAMP_MAGFILTER, ( flags & TEXFLAGS_LERP ) ? D3DTEXF_LINEAR : D3DTEXF_POINT );
		m_dev->SetSamplerState( slot, D3DSAMP_MINFILTER, ( flags & TEXFLAGS_LERP ) ? D3DTEXF_LINEAR : D3DTEXF_POINT );
		m_dev->SetSamplerState( slot, D3DSAMP_MIPFILTER, ( flags & TEXFLAGS_HASMIPS ) ? D3DTEXF_LINEAR : D3DTEXF_NONE );
		m_dev->SetSamplerState( slot, D3DSAMP_ADDRESSU, ( flags & TEXFLAGS_CLAMP_X ) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP );
		m_dev->SetSamplerState( slot, D3DSAMP_ADDRESSV, ( flags & TEXFLAGS_CLAMP_Y ) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP );
		m_dev->SetSamplerState( slot, D3DSAMP_SRGBTEXTURE, ( flags & TEXFLAGS_SRGB ) ? 1 : 0 );
	}
}

void D3D9Renderer::SetTexture( int slot, const SGRX_ITexture* tex )
{
	SGRX_CAST( D3D9Texture*, T, tex ); 
	if( T )
		_SetTextureInt( slot, T->m_ptr.base, T->m_info.flags );
	else
		_SetTextureInt( slot, NULL, 0 );
}

void D3D9Renderer::SetVertexShader( const SGRX_IVertexShader* shd )
{
	ASSERT( shd );
	SGRX_CAST( const D3D9VertexShader*, S, shd );
	m_dev->SetVertexShader( S->m_VS );
}

void D3D9Renderer::SetPixelShader( const SGRX_IPixelShader* shd )
{
	ASSERT( shd );
	SGRX_CAST( const D3D9PixelShader*, S, shd );
	m_dev->SetPixelShader( S->m_PS );
}

static D3DCOLOR V4toD3DCOLOR( const Vec4& v )
{
	return D3DCOLOR_ARGB( COLOR_F2B( v.w ), COLOR_F2B( v.x ), COLOR_F2B( v.y ), COLOR_F2B( v.z ) );
}

void D3D9Renderer::SetRenderState( const SGRX_RenderState& nrs )
{
	static const D3DFILLMODE fillModes[2] = { D3DFILL_SOLID, D3DFILL_WIREFRAME };
	static const D3DCULL cullModes[4] = { D3DCULL_NONE, D3DCULL_CCW, D3DCULL_CW, /**/ D3DCULL_NONE };
	static const D3DSTENCILOP stencilOps[8] =
	{
		D3DSTENCILOP_KEEP, D3DSTENCILOP_ZERO,
		D3DSTENCILOP_REPLACE, D3DSTENCILOP_INVERT,
		D3DSTENCILOP_INCR, D3DSTENCILOP_DECR,
		D3DSTENCILOP_INCRSAT, D3DSTENCILOP_DECRSAT,
	};
	static const D3DCMPFUNC compFuncs[8] =
	{
		D3DCMP_NEVER, D3DCMP_ALWAYS,
		D3DCMP_EQUAL, D3DCMP_NOTEQUAL,
		D3DCMP_LESS, D3DCMP_LESSEQUAL,
		D3DCMP_GREATER, D3DCMP_GREATEREQUAL,
	};
	static const D3DBLENDOP blendOps[8] =
	{
		D3DBLENDOP_ADD,
		D3DBLENDOP_SUBTRACT,
		D3DBLENDOP_REVSUBTRACT,
		D3DBLENDOP_MIN,
		D3DBLENDOP_MAX,
		/**/
		D3DBLENDOP_ADD, D3DBLENDOP_ADD, D3DBLENDOP_ADD,
	};
	static const D3DBLEND blendFactors[16] =
	{
		D3DBLEND_ZERO, D3DBLEND_ONE,
		D3DBLEND_SRCCOLOR, D3DBLEND_INVSRCCOLOR,
		D3DBLEND_DESTCOLOR, D3DBLEND_INVDESTCOLOR,
		D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA,
		D3DBLEND_DESTALPHA, D3DBLEND_INVDESTALPHA,
		D3DBLEND_BLENDFACTOR, D3DBLEND_INVBLENDFACTOR,
		/**/
		D3DBLEND_ONE, D3DBLEND_ONE, D3DBLEND_ONE, D3DBLEND_ONE,
	};
	
#define RENDER_STATE_BOOL( name, rs ) if( m_crs.name != nrs.name ){ m_dev->SetRenderState( rs, nrs.name ); m_crs.name = nrs.name; }
#define RENDER_STATE_VALUE( name, rs, val ) if( m_crs.name != nrs.name ){ m_dev->SetRenderState( rs, val ); m_crs.name = nrs.name; }
#define RENDER_STATE_TABLE( name, rs, tbl ) if( m_crs.name != nrs.name ){ m_dev->SetRenderState( rs, tbl[ nrs.name ] ); m_crs.name = nrs.name; }
	
	// rasterizer
	RENDER_STATE_TABLE( wireFill, D3DRS_FILLMODE, fillModes );
	RENDER_STATE_TABLE( cullMode, D3DRS_CULLMODE, cullModes );
	// UNSUPPORTED: separateBlend
	RENDER_STATE_BOOL( scissorEnable, D3DRS_SCISSORTESTENABLE );
	RENDER_STATE_BOOL( multisampleEnable, D3DRS_MULTISAMPLEANTIALIAS );
	
	// depth buffer
	RENDER_STATE_BOOL( depthEnable, D3DRS_ZENABLE );
	RENDER_STATE_BOOL( depthWriteEnable, D3DRS_ZWRITEENABLE );
	RENDER_STATE_TABLE( depthFunc, D3DRS_ZFUNC, compFuncs );
	RENDER_STATE_VALUE( depthBias, D3DRS_DEPTHBIAS, F2DW( nrs.depthBias ) );
	RENDER_STATE_VALUE( slopeDepthBias, D3DRS_SLOPESCALEDEPTHBIAS, F2DW( nrs.slopeDepthBias ) );
	// UNSUPPORTED: depthBiasClamp
	
	// stencil buffer
	RENDER_STATE_BOOL( stencilEnable, D3DRS_STENCILENABLE );
	RENDER_STATE_VALUE( stencilReadMask, D3DRS_STENCILMASK, nrs.stencilReadMask & 0xff );
	RENDER_STATE_VALUE( stencilWriteMask, D3DRS_STENCILWRITEMASK, nrs.stencilWriteMask & 0xff );
	RENDER_STATE_TABLE( stencilFrontFailOp, D3DRS_STENCILFAIL, stencilOps );
	RENDER_STATE_TABLE( stencilFrontDepthFailOp, D3DRS_STENCILZFAIL, stencilOps );
	RENDER_STATE_TABLE( stencilFrontPassOp, D3DRS_STENCILPASS, stencilOps );
	RENDER_STATE_TABLE( stencilFrontFunc, D3DRS_STENCILFUNC, compFuncs );
	// UNSUPPORTED: stencilBackFailOp
	// UNSUPPORTED: stencilBackDepthFailOp
	// UNSUPPORTED: stencilBackPassOp
	// UNSUPPORTED: stencilBackFunc
	RENDER_STATE_VALUE( stencilRef, D3DRS_STENCILREF, nrs.stencilRef & 0xff );
	
	// blending state
	RENDER_STATE_VALUE( blendStates[0].colorWrite, D3DRS_COLORWRITEENABLE, nrs.blendStates[0].colorWrite );
	RENDER_STATE_BOOL( blendStates[0].blendEnable, D3DRS_ALPHABLENDENABLE );
	RENDER_STATE_TABLE( blendStates[0].blendOp, D3DRS_BLENDOP, blendOps );
	RENDER_STATE_TABLE( blendStates[0].srcBlend, D3DRS_SRCBLEND, blendFactors );
	RENDER_STATE_TABLE( blendStates[0].dstBlend, D3DRS_DESTBLEND, blendFactors );
	RENDER_STATE_TABLE( blendStates[0].blendOpAlpha, D3DRS_BLENDOPALPHA, blendOps );
	RENDER_STATE_TABLE( blendStates[0].srcBlendAlpha, D3DRS_SRCBLENDALPHA, blendFactors );
	RENDER_STATE_TABLE( blendStates[0].dstBlendAlpha, D3DRS_DESTBLENDALPHA, blendFactors );
	RENDER_STATE_VALUE( blendFactor, D3DRS_BLENDFACTOR, V4toD3DCOLOR( nrs.blendFactor ) );
}

void D3D9Renderer::SetRenderState( const SGRX_IRenderState* rsi )
{
	ASSERT( rsi );
	SetRenderState( rsi->m_info );
}

