

#include <d3d9.h>

#include "renderer.hpp"


#define SAFE_RELEASE( x ) if( x ){ (x)->Release(); x = NULL; }


static IDirect3D9* g_D3D = NULL;


static void _ss_reset_states( IDirect3DDevice9* dev, int w, int h )
{
	dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	dev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	dev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	dev->SetRenderState( D3DRS_LIGHTING, 0 );
	dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	dev->SetRenderState( D3DRS_ZENABLE, 0 );
	dev->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
	dev->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, 1 );
	dev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	{
		float wm[ 16 ] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
		float vm[ 16 ] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 100,  0, 0, 0, 1 };
		float pm[ 16 ] = { 2.0f/(float)w, 0, 0, 0,  0, 2.0f/(float)h, 0, 0,  0, 0, 1.0f/999.0f, 1.0f/-999.0f,  0, 0, 0, 1 };
		dev->SetTransform( D3DTS_WORLD, (D3DMATRIX*) wm );
		dev->SetTransform( D3DTS_VIEW, (D3DMATRIX*) vm );
		dev->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*) pm );
	}
}


struct D3D9Texture : ITexture
{
	union
	{
		IDirect3DBaseTexture9* base;
		IDirect3DTexture9* tex2d;
		IDirect3DCubeTexture9* cube;
		IDirect3DVolumeTexture9* vol;
	}
	m_ptr;
	
	void Destroy()
	{
		SAFE_RELEASE( m_ptr.base );
		delete this;
	}
};


RendererInfo g_D3D9RendererInfo =
{
	true, // swap R/B
};

struct D3D9Renderer : IRenderer
{
	void Destroy();
	const RendererInfo& GetInfo(){ return g_D3D9RendererInfo; }
	
	void Swap();
	void Modify( const RenderSettings& settings );
	void SetCurrent(){} // does nothing since there's no thread context pointer
	void Clear( float* color_v4f, bool clear_zbuffer );
	
	ITexture* CreateTexture( TextureInfo* texinfo, void* data = NULL );
	
	void DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt, ITexture* tex );
	
	bool ResetDevice();
	void ResetViewport();
	void SetTexture( int i, ITexture* tex );
	
	IDirect3DDevice9* m_dev;
	D3DPRESENT_PARAMETERS m_params;
	RenderSettings m_currSettings;
	IDirect3DSurface9* m_backbuf;
	IDirect3DSurface9* m_dssurf;
};

extern "C" EXPORT bool Initialize( const char** outname )
{
	*outname = "Direct3D9";
	g_D3D = Direct3DCreate9( D3D_SDK_VERSION );
	return g_D3D != NULL;
}

extern "C" EXPORT void Free()
{
	g_D3D->Release();
}

extern "C" EXPORT IRenderer* CreateRenderer( const RenderSettings& settings, void* windowHandle )
{
	D3DPRESENT_PARAMETERS d3dpp;
	IDirect3DDevice9* d3ddev;
	
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = 1;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.hDeviceWindow = (HWND) windowHandle;
	d3dpp.BackBufferWidth = settings.width;
	d3dpp.BackBufferHeight = settings.height;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.PresentationInterval = settings.vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	
	if( FAILED( g_D3D->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		(HWND) windowHandle,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,
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
	SAFE_RELEASE( m_backbuf );
	SAFE_RELEASE( m_dssurf );
	IDirect3DResource9_Release( m_dev );
	delete this;
}

void D3D9Renderer::Swap()
{
	// TODO RT
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
	bool needreset = m_currSettings.width != settings.width || m_currSettings.height != settings.height;
	m_currSettings = settings;
	
	if( needreset )
	{
		m_params.BackBufferWidth = settings.width;
		m_params.BackBufferHeight = settings.height;
		ResetDevice();
		ResetViewport();
		m_dev->BeginScene();
	}
}

void D3D9Renderer::Clear( float* color_v4f, bool clear_zbuffer )
{
	uint32_t cc = 0;
	uint32_t flags = 0;
	if( color_v4f )
	{
		cc = COLOR_RGBA( color_v4f[2] * 255, color_v4f[1] * 255, color_v4f[0] * 255, color_v4f[3] * 255 );
		flags = D3DCLEAR_TARGET;
	}
	if( clear_zbuffer )
		flags |= D3DCLEAR_ZBUFFER;
	m_dev->Clear( 0, NULL, flags, cc, 1.0f, 0 );
}


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
	
	off = TextureData_GetMipDataOffset( texinfo, data, side, mip );
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

ITexture* D3D9Renderer::CreateTexture( TextureInfo* texinfo, void* data )
{
	int mip, side;
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
		
		// load all mip levels into it
		for( mip = 0; mip < texinfo->mipcount; ++mip )
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
		
		D3D9Texture* T = new D3D9Texture;
		T->m_info = *texinfo;
		T->m_ptr.tex2d = d3dtex;
		return T;
	}
	else if( texinfo->type == TEXTYPE_CUBE )
	{
		IDirect3DCubeTexture9* d3dtex;
		
		hr = m_dev->CreateCubeTexture( texinfo->width, texinfo->mipcount, 0, texfmt2d3d( texinfo->format ), D3DPOOL_MANAGED, &d3dtex, NULL );
		
		// load all mip levels into it
		for( side = 0; side < 6; ++side )
		{
			for( mip = 0; mip < texinfo->mipcount; ++mip )
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
		
		D3D9Texture* T = new D3D9Texture;
		T->m_info = *texinfo;
		T->m_ptr.cube = d3dtex;
		return T;
	}
	
	LOG_ERROR << "TODO [reached a part of not-yet-defined behavior]";
	return NULL;
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
	case PT_TriangleFan: return D3DPT_TRIANGLEFAN;
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
	case PT_TriangleStrip:
	case PT_TriangleFan: if( numverts < 3 ) return 0; return numverts - 2;
	default: return 0;
	}
}

void D3D9Renderer::DrawBatchVertices( BatchRenderer::Vertex* verts, uint32_t count, EPrimitiveType pt, ITexture* tex )
{
	SetTexture( 0, tex );
	m_dev->SetFVF( D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE );
	m_dev->DrawPrimitiveUP( conv_prim_type( pt ), get_prim_count( pt, count ), verts, sizeof( *verts ) );
}


bool D3D9Renderer::ResetDevice()
{
	D3DPRESENT_PARAMETERS npp;
	
	/* reset */
	npp = m_params;
	
	if( FAILED( m_dev->Reset( &npp ) ) )
	{
		LOG_ERROR << "Failed to reset D3D9 device";
	}
	
	_ss_reset_states( m_dev, m_params.BackBufferWidth, m_params.BackBufferHeight );
}

void D3D9Renderer::ResetViewport()
{
	D3DVIEWPORT9 vp = { 0, 0, m_currSettings.width, m_currSettings.height, 0.0, 1.0 };
	m_dev->SetViewport( &vp );
}

void D3D9Renderer::SetTexture( int slot, ITexture* tex )
{
	SGRX_CAST( D3D9Texture*, T, tex ); 
	m_dev->SetTexture( slot, T ? T->m_ptr.base : NULL );
	if( T )
	{
		uint32_t flags = T->m_info.flags;
		m_dev->SetSamplerState( slot, D3DSAMP_MAGFILTER, ( flags & TEXFLAGS_LERP_X ) ? D3DTEXF_LINEAR : D3DTEXF_POINT );
		m_dev->SetSamplerState( slot, D3DSAMP_MINFILTER, ( flags & TEXFLAGS_LERP_Y ) ? D3DTEXF_LINEAR : D3DTEXF_POINT );
		m_dev->SetSamplerState( slot, D3DSAMP_MIPFILTER, ( flags & TEXFLAGS_HASMIPS ) ? D3DTEXF_NONE : D3DTEXF_LINEAR );
		m_dev->SetSamplerState( slot, D3DSAMP_ADDRESSU, ( flags & TEXFLAGS_CLAMP_X ) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP );
		m_dev->SetSamplerState( slot, D3DSAMP_ADDRESSV, ( flags & TEXFLAGS_CLAMP_Y ) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP );
		m_dev->SetSamplerState( slot, D3DSAMP_SRGBTEXTURE, ( flags & TEXFLAGS_SRGB ) ? 1 : 0 );
	}
}

