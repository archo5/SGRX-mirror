

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
#define __out_ecount_part_opt(x,y)
#define __in_ecount(x)
#define __out_ecount(x)
#define __in_range(x,y)
#endif
#include <windows.h>
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4005 )
#endif
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
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "renderer.hpp"


#ifdef ENABLE_SHADER_COMPILING
void* g_D3DCompilerDLL = NULL;
pD3DCompile g_pfnD3DCompile = NULL;

#define SGRX_D3DCOMPILER_DLL "d3dcompiler_42.dll"

void LoadD3DCompiler()
{
	g_D3DCompilerDLL = Sys_LoadLib( SGRX_D3DCOMPILER_DLL );
	if( !g_D3DCompilerDLL )
	{
		LOG_WARNING << LOG_DATE << "    " SGRX_D3DCOMPILER_DLL " is unavailable - shader compilation is disabled";
		return;
	}
	g_pfnD3DCompile = (pD3DCompile) Sys_GetProc( g_D3DCompilerDLL, "D3DCompile" );
	if( !g_pfnD3DCompile )
	{
		LOG_ERROR << LOG_DATE << "    D3DCompile function was not found - strange system configuration";
		return;
	}
}

void UnloadD3DCompiler()
{
	g_pfnD3DCompile = NULL;
	if( g_D3DCompilerDLL )
	{
		Sys_UnloadLib( g_D3DCompilerDLL );
		g_D3DCompilerDLL = NULL;
	}
}
#endif


#define SAFE_RELEASE( x ) if( x ){ (x)->Release(); x = NULL; }


#define RENDERER_EXPORT __declspec(dllexport)


const GUID g_IID_ID3D11Texture2D = {0x6f15aaf2,0xd208,0x4e89,{0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c}};
const GUID g_IID_IDXGIDevice = {0x54ec77fa,0x1377,0x44e6,{0x8c,0x32,0x88,0xfd,0x5f,0x44,0xc8,0x4c}};
const GUID g_IID_IDXGIAdapter = {0x2411e7e1,0x12ac,0x4ccf,{0xbd,0x14,0x97,0x98,0xe8,0x53,0x4d,0xc0}};
const GUID g_IID_IDXGIFactory = {0x7b7166ec,0x21c7,0x44ae,{0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69}};
const GUID g_IID_IDXGIFactory1 = {0x770aae78,0xf26f,0x4dba,{0xa8,0x29,0x25,0x3c,0x83,0xd1,0xb3,0x87}};
const GUID g_IID_ID3D11Debug = {0x79cf2233,0x7536,0x4948,{0x9d,0x36,0x1e,0x46,0x92,0xdc,0x57,0x60}};


static DXGI_FORMAT texfmt2d3d( int fmt )
{
	switch( fmt )
	{
	case TEXFMT_BGRX8: return DXGI_FORMAT_B8G8R8X8_TYPELESS;
	case TEXFMT_BGRA8: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case TEXFMT_RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TEXFMT_R5G6B5: return DXGI_FORMAT_B5G6R5_UNORM;
	
	case TEXFMT_DXT1: return DXGI_FORMAT_BC1_UNORM;
	case TEXFMT_DXT3: return DXGI_FORMAT_BC2_UNORM;
	case TEXFMT_DXT5: return DXGI_FORMAT_BC3_UNORM;
	}
	return DXGI_FORMAT_UNKNOWN;
}


static HRESULT _D3DCall( HRESULT result, const char* call, int line )
{
	if( FAILED( result ) )
	{
		SGRX_Log log = LOG_ERROR;
		log << "D3D11 call failed: " << call
			<< "\nresult: ";
		log.writef( "0x%08X", result );
		log << "\nfile: " << __FILE__
			<< "\nline: " << line;
	}
	return result;
}
#define D3DCALL( hrcall ) _D3DCall( hrcall, #hrcall, __LINE__ )


struct RTInfo : SGRX_Log::Loggable< RTInfo >
{
	RTInfo() : type(0), width(0), height(0), depth(0), format(0), mipcount(1),
		d3dfmt( DXGI_FORMAT_UNKNOWN ){}
	RTInfo( TextureInfo* ti )
	{
		type = ti->type;
		width = ti->width;
		height = ti->height;
		depth = ti->depth;
		format = ti->format;
		mipcount = ti->mipcount;
		
		switch( format )
		{
		case TEXFMT_RT_COLOR_HDR16:
			d3dfmt = DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;
		case TEXFMT_RT_COLOR_LDR8:
			d3dfmt = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case TEXFMT_RT_DEPTH_F32:
			d3dfmt = DXGI_FORMAT_D32_FLOAT;
			break;
		default:
			d3dfmt = DXGI_FORMAT_UNKNOWN;
			break;
		}
	}
	RTInfo( int width, int height, DXGI_FORMAT d3dfmt )
		: type( TEXTYPE_2D ),
		width( width ), height( height ), depth( 0 ),
		format( 0 ), mipcount( 1 ), d3dfmt( d3dfmt )
	{
	}
	bool IsFormatGood() const
	{
		if( d3dfmt == DXGI_FORMAT_UNKNOWN )
			return false;
		return true;
	}
	void Log( SGRX_Log& log ) const
	{
		log << "type=" << ( type == TEXTYPE_VOLUME ? "volume" : ( type == TEXTYPE_CUBE ? "cube" : "2D" ) )
			<< ", format=" << format
			<< ", d3dfmt=" << d3dfmt
			<< ", mipcount=" << mipcount;
		switch( type )
		{
		case TEXTYPE_2D:
			log << ", width=" << width << ", height=" << height;
			break;
		case TEXTYPE_CUBE:
			log << ", width=" << width;
			break;
		case TEXTYPE_VOLUME:
			log << ", width=" << width << ", height=" << height << ", depth=" << depth;
			break;
		}
	}
	DXGI_FORMAT GetTextureFormat() const
	{
		return d3dfmt == DXGI_FORMAT_D32_FLOAT ? DXGI_FORMAT_R32_TYPELESS : d3dfmt;
	}
	DXGI_FORMAT GetSRVFormat() const
	{
		return d3dfmt == DXGI_FORMAT_D32_FLOAT ? DXGI_FORMAT_R32_FLOAT : d3dfmt;
	}
	
	int type, width, height, depth, format, mipcount;
	DXGI_FORMAT d3dfmt;
};


static int create_rtt_( ID3D11Device* device, const RTInfo& rti, int msamples, bool ds, bool srs, ID3D11Resource** outtex )
{
	switch( rti.type )
	{
	case TEXTYPE_2D:
	case TEXTYPE_CUBE: {
		bool isCube = rti.type == TEXTYPE_CUBE;
		D3D11_TEXTURE2D_DESC dtd;
		memset( &dtd, 0, sizeof(dtd) );
		
		dtd.Width = rti.width;
		dtd.Height = isCube ? rti.width : rti.height;
		dtd.MipLevels = rti.mipcount;
		dtd.ArraySize = isCube ? 6 : 1;
		dtd.Format = rti.GetTextureFormat();
		dtd.SampleDesc.Count = msamples > 1 ? TMAX( 1, TMIN( 16, msamples ) ) : 1;
		dtd.SampleDesc.Quality = msamples > 1 ? 1 : 0;
		dtd.Usage = D3D11_USAGE_DEFAULT;
		dtd.BindFlags = ds ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
		if( srs )
			dtd.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		if( isCube )
			dtd.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		
		return FAILED( D3DCALL( device->CreateTexture2D( &dtd, NULL, (ID3D11Texture2D**) outtex ) ) );
	} break;
	default:
		LOG_ERROR << "create_rtt - bad texture type: " << rti.type;
		return 1;
	}
}
#define create_rtt( dev, rti, ms, ds, srs, outtex ) \
	create_rtt_( dev, rti, ms, ds, srs, (ID3D11Resource**) (outtex) )

static int create_rtview( ID3D11Device* device, const RTInfo& rti, ID3D11Resource* tex, ID3D11RenderTargetView** outview, int mip = 0, int slice = 0 )
{
	D3D11_RENDER_TARGET_VIEW_DESC desc = { rti.d3dfmt, D3D11_RTV_DIMENSION_UNKNOWN, 0 };
	switch( rti.type )
	{
	case TEXTYPE_2D:
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mip;
		break;
	case TEXTYPE_CUBE:
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = mip;
		desc.Texture2DArray.FirstArraySlice = slice;
		desc.Texture2DArray.ArraySize = 1;
		break;
	case TEXTYPE_VOLUME:
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipSlice = mip;
		desc.Texture3D.FirstWSlice = slice;
		desc.Texture3D.WSize = 1;
		break;
	default:
		LOG_ERROR << "create_rtview - bad texture type: " << rti.type;
		return 1;
	}
	return FAILED( D3DCALL( device->CreateRenderTargetView( tex, &desc, outview ) ) );
}

static int create_dsview( ID3D11Device* device, const RTInfo& rti, ID3D11Resource* tex, ID3D11DepthStencilView** outview, int mip = 0, int slice = 0 )
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc = { rti.d3dfmt, D3D11_DSV_DIMENSION_UNKNOWN, 0 };
	switch( rti.type )
	{
	case TEXTYPE_2D:
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mip;
		break;
	case TEXTYPE_CUBE:
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = mip;
		desc.Texture2DArray.FirstArraySlice = slice;
		desc.Texture2DArray.ArraySize = 1;
		break;
	default:
		LOG_ERROR << "create_rtview - bad texture type: " << rti.type;
		return 1;
	}
	return FAILED( D3DCALL( device->CreateDepthStencilView( tex, &desc, outview ) ) );
}

static int create_srview( ID3D11Device* device,
	ID3D11Resource* tex,
	int type,
	DXGI_FORMAT format,
	ID3D11ShaderResourceView** outview )
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = { format };
	switch( type )
	{
	case TEXTYPE_2D:
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MostDetailedMip = 0;
		srvd.Texture2D.MipLevels = -1;
		break;
	case TEXTYPE_CUBE:
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvd.TextureCube.MostDetailedMip = 0;
		srvd.TextureCube.MipLevels = -1;
		break;
	case TEXTYPE_VOLUME:
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		srvd.Texture3D.MostDetailedMip = 0;
		srvd.Texture3D.MipLevels = -1;
		break;
	default:
		LOG_ERROR << "create_srview - bad texture type: " << type;
		return 1;
	}
	if( FAILED( D3DCALL( device->CreateShaderResourceView( tex, &srvd, outview ) ) ) )
	{
		LOG_ERROR << "could not create D3D11 shader resource view for texture"
			<< " (type=" << type
			<< ", format=" << format << ")";
		return 1;
	}
	return 0;
}

static int upload_buf( ID3D11DeviceContext* ctx, ID3D11Buffer* buf,
	bool dyn, bool discard, const void* data, size_t size )
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
		
		if( data )
			memcpy( msr.pData, data, size );
		else
			memset( msr.pData, 0, size );
		
		ctx->Unmap( buf, 0 );
	}
	else
	{
		ctx->UpdateSubresource( buf, 0, NULL, data, size, 0 );
	}
	return 0;
}

static int create_buf( ID3D11DeviceContext* ctx, ID3D11Device* device,
	size_t numbytes, bool dyn, D3D11_BIND_FLAG bindtype, const void* data, ID3D11Buffer** out )
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
	
	return dyn ? upload_buf( ctx, *out, true, true, data, numbytes ) : 0;
}


struct D3D11DynBuffer
{
	D3D11DynBuffer( D3D11_BIND_FLAG bindtype ) : buffer( NULL ), currSize( 0 ), m_bindtype( bindtype ){}
	~D3D11DynBuffer()
	{
		if( buffer )
			LOG << "UNFREED DYNAMIC BUFFER";
	}
	void Free()
	{
		SAFE_RELEASE( buffer );
	}
	void Upload( ID3D11Device* dev, ID3D11DeviceContext* ctx, const void* data, size_t size )
	{
		if( !size )
			return;
		if( size > currSize )
		{
			SAFE_RELEASE( buffer );
			create_buf( ctx, dev, size, true, m_bindtype, data, &buffer );
			currSize = size;
		}
		else
			upload_buf( ctx, buffer, true, true, data, size );
	}
	void DynSetZero( ID3D11Device* dev, ID3D11DeviceContext* ctx, size_t size )
	{
		Upload( dev, ctx, NULL, size );
	}
	
	operator ID3D11Buffer* () const { return buffer; }
	ID3D11Buffer** PPBuf() { return &buffer; }
	
	ID3D11Buffer* buffer;
	size_t currSize;
	D3D11_BIND_FLAG m_bindtype;
	
private:
	D3D11DynBuffer( const D3D11DynBuffer& );
	D3D11DynBuffer& operator = ( const D3D11DynBuffer& );
};


struct BackupVertexData
{
	Vec4 color;
	Vec4 normal;
	Vec4 tangent;
	Vec4 tex[4];
};
static BackupVertexData g_initial_backup_vertex_data = {0};


struct D3D11Texture : SGRX_ITexture
{
	ID3D11Resource* m_texture;
	ID3D11SamplerState* m_sampState;
	ID3D11ShaderResourceView* m_rsrcView;
	struct D3D11Renderer* m_renderer;
	
	D3D11Texture( bool isRenderTexture = false ) : m_sampState(NULL), m_rsrcView(NULL), m_renderer(NULL)
	{
		m_isRenderTexture = isRenderTexture;
	}
	virtual ~D3D11Texture();
	
	ID3D11Texture2D*& _Texture2D(){ return *(ID3D11Texture2D**)&m_texture; }
	ID3D11Texture3D*& _Texture3D(){ return *(ID3D11Texture3D**)&m_texture; }
	
	bool UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h );
	bool UploadRGBA8Part3D( void* data, int mip, int x, int y, int z, int w, int h, int d );
};

struct D3D11RenderTexture : D3D11Texture
{
	/* cached views */
	int RTVmip;
	int RTVside;
	ID3D11RenderTargetView* RTView;
	int DSVmip;
	int DSVside;
	ID3D11DepthStencilView* DSView;
	RTInfo m_rtinfo;
	
	D3D11RenderTexture() : D3D11Texture(true),
		RTVmip(-1), RTVside(-1), RTView(NULL),
		DSVmip(-1), DSVside(-1), DSView(NULL){}
	virtual ~D3D11RenderTexture()
	{
		SAFE_RELEASE( RTView );
		SAFE_RELEASE( DSView );
	}
	
	bool IsDepthRT(){ return IS_FLAG_SET( m_info.format, TEXFF_RTDEPTHFMT ); }
	ID3D11RenderTargetView* GetRTV( int side, int mip );
	ID3D11DepthStencilView* GetDSV( int side, int mip );
};

struct D3D11DepthStencilSurface : SGRX_IDepthStencilSurface
{
	ID3D11Texture2D* DT;
	ID3D11DepthStencilView* DSV;
	struct D3D11Renderer* m_renderer;
	
	D3D11DepthStencilSurface() : DT( NULL ), DSV( NULL ), m_renderer( NULL ){}
	virtual ~D3D11DepthStencilSurface();
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
	ID3D11RasterizerState* m_RS_FCCW; // + front counterclockwise
	ID3D11BlendState* m_BS;
	ID3D11DepthStencilState* m_DS;
	struct D3D11Renderer* m_renderer;
	
	D3D11RenderState( struct D3D11Renderer* r ) : m_RS( NULL ),
		m_RS_FCCW( NULL ), m_BS( NULL ), m_DS( NULL ), m_renderer( r ){}
	~D3D11RenderState()
	{
		SAFE_RELEASE( m_RS );
		SAFE_RELEASE( m_RS_FCCW );
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
	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;
	struct D3D11Renderer* m_renderer;
	int m_origVertexSize;
	int m_realVertexSize;
	size_t m_realVertexDataSize;
	
	D3D11Mesh() : m_VB( NULL ), m_IB( NULL ), m_renderer( NULL ), m_origVertexSize(0), m_realVertexSize(0), m_realVertexDataSize(0)
	{
	}
	~D3D11Mesh();
	
	bool InitVertexBuffer( size_t size, VertexDeclHandle vd );
	bool InitIndexBuffer( size_t size, bool i32 );
	bool UpdateVertexData( const void* data, size_t size );
	bool UpdateIndexData( const void* data, size_t size );
};


struct D3D11VertexInputMapping : SGRX_IVertexInputMapping
{
	ID3D11InputLayout* m_inputLayout;
	
	D3D11VertexInputMapping() : m_inputLayout( NULL )
	{
	}
	~D3D11VertexInputMapping()
	{
		SAFE_RELEASE( m_inputLayout );
	}
};


SGRX_RendererInfo g_D3D11RendererInfo =
{
	true, // compile shaders
	"d3d11", // shader cache folder suffix
	"HLSL_D3D11", // shader type (define)
};

struct D3D11Renderer : IRenderer
{
	struct cb_vs_batchverts
	{
		cb_vs_batchverts() : world( Mat4::Identity ), view( Mat4::Identity ){}
		Mat4 world;
		Mat4 view;
	};
	
	D3D11Renderer() :
		m_vertbuf_batchverts( D3D11_BIND_VERTEX_BUFFER ),
		m_cbuf_ps_batchverts( D3D11_BIND_CONSTANT_BUFFER ),
		m_cbuf3_ltpoint( D3D11_BIND_CONSTANT_BUFFER ),
		m_cbuf4_ltspotvs( D3D11_BIND_CONSTANT_BUFFER ),
		m_cbuf4_ltspotps( D3D11_BIND_CONSTANT_BUFFER ),
		m_cbuf6_ltcount( D3D11_BIND_CONSTANT_BUFFER )
	{}
	void Destroy();
	const SGRX_RendererInfo& GetInfo(){ return g_D3D11RendererInfo; }
	bool LoadInternalResources();
	void UnloadInternalResources();
	
	void Swap();
	void Modify( const RenderSettings& settings );
	void SetCurrent(){} // does nothing since there's no thread context pointer
	
	void SetRenderTargets( const SGRX_RTClearInfo& info, SGRX_IDepthStencilSurface* dss, SGRX_RTSpec rts[4] );
	void SetViewport( int x0, int y0, int x1, int y1 );
	void _SetViewport( int x0, int y0, int x1, int y1 );
	void SetScissorRect( int* rect );
	
	SGRX_ITexture* CreateTexture( TextureInfo* texinfo, const void* data );
	SGRX_ITexture* CreateRenderTexture( TextureInfo* texinfo );
	SGRX_IDepthStencilSurface* CreateDepthStencilSurface( int width, int height, int format );
	bool CompileShader( const StringView& path, EShaderType shadertype, const StringView& code, ByteArray& outcomp, String& outerrors );
	SGRX_IVertexShader* CreateVertexShader( const StringView& path, ByteArray& code );
	SGRX_IPixelShader* CreatePixelShader( const StringView& path, ByteArray& code );
	SGRX_IRenderState* CreateRenderState( const SGRX_RenderState& state );
	SGRX_IVertexDecl* CreateVertexDecl( const VDeclInfo& vdinfo );
	SGRX_IMesh* CreateMesh();
	SGRX_IVertexInputMapping* CreateVertexInputMapping( SGRX_IVertexShader* vs, SGRX_IVertexDecl* vd );
	
	void SetMatrix( bool view, const Mat4& mtx );
	void DrawImmediate( SGRX_ImmDrawData& idd );
	
	void DoRenderItems( SGRX_Scene* scene, SGRX_PassType passtype, int maxrepeat, const SGRX_Camera& cam, RenderItem* start, RenderItem* end );
	
	bool ResetDevice();
	void SetVertexShader( const SGRX_IVertexShader* shd );
	void SetPixelShader( const SGRX_IPixelShader* shd );
	void SetRenderState( const SGRX_IRenderState* rsi, bool fccw );
	
//	FINLINE int GetWidth() const { return m_params.BackBufferWidth; }
//	FINLINE int GetHeight() const { return m_params.BackBufferHeight; }
	
	// state
	SGRX_Mutex m_mutex;
	
	// storage
	HashTable< D3D11Texture*, bool > m_ownTextures;
	HashTable< D3D11Mesh*, bool > m_ownMeshes;
	HashTable< D3D11DepthStencilSurface*, bool > m_ownDSS;
	
	// specific
	ID3D11Device* m_dev;
	ID3D11DeviceContext* m_ctx;
	IDXGISwapChain* m_swapChain;
	ID3D11Texture2D* m_backBuffer;
	ID3D11Texture2D* m_depthBuffer;
	ID3D11RenderTargetView* m_rtView;
	ID3D11DepthStencilView* m_dsView;
	ID3D11SamplerState* m_sampState;
	ID3D11SamplerState* m_depthSampState;
	
	// rendering data
	ID3D11Buffer* m_vertbuf_defaults;
	cb_vs_batchverts m_cbdata_vs_batchverts;
	ID3D11Buffer* m_cbuf_vs_batchverts;
	D3D11DynBuffer m_vertbuf_batchverts;
	D3D11DynBuffer m_cbuf_ps_batchverts;
	ID3D11Buffer* m_cbuf0_common;
	ID3D11Buffer* m_cbuf1_inst;
	ID3D11Buffer* m_cbuf2_skin;
	D3D11DynBuffer m_cbuf3_ltpoint;
	D3D11DynBuffer m_cbuf4_ltspotvs;
	D3D11DynBuffer m_cbuf4_ltspotps;
	D3D11DynBuffer m_cbuf6_ltcount;
	
	// temp data
	D3D11_VIEWPORT m_viewport;
	ByteArray m_scratchMem;
};

extern "C" RENDERER_EXPORT bool Initialize( const char** outname )
{
#ifdef ENABLE_SHADER_COMPILING
	LoadD3DCompiler();
#endif
	*outname = "Direct3D11";
	return true;
}

extern "C" RENDERER_EXPORT void Free()
{
#ifdef ENABLE_SHADER_COMPILING
	UnloadD3DCompiler();
#endif
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
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
	
	IDXGIFactory1* pFactory = NULL;
	CreateDXGIFactory1( g_IID_IDXGIFactory1, (void**)&pFactory );
	IDXGIAdapter* pAdapter;
	IDXGIAdapter* GoodAdapter = NULL;
	for( UINT i = 0; pFactory->EnumAdapters( i, &pAdapter ) != DXGI_ERROR_NOT_FOUND; ++i )
	{
		DXGI_ADAPTER_DESC desc;
		ZeroMemory( &desc, sizeof(desc) );
		pAdapter->GetDesc( &desc );
		LOG << "=== Adapter " << i << ": "
			<< StackUnWString<128>( desc.Description ).str;
		LOG << "- vendor: " << desc.VendorId;
		LOG << "- device: " << desc.DeviceId;
		LOG << "- subsystem: " << desc.SubSysId;
		LOG << "- revision: " << desc.Revision;
		LOG << "- video memory: "
			<< uint64_t(desc.DedicatedVideoMemory);
		LOG << "- dedicated system memory: "
			<< uint64_t(desc.DedicatedSystemMemory);
		LOG << "- shared system memory: "
			<< uint64_t(desc.SharedSystemMemory);
		if( int(i) == settings.gpu )
		{
			LOG << " ^^^ CHOSEN GPU ^^^\n";
			GoodAdapter = pAdapter;
		}
	}
	
	hr = D3D11CreateDeviceAndSwapChain(
		GoodAdapter, // adapter
		GoodAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
		NULL, // software rasterizer (unused)
		0,//D3D11_CREATE_DEVICE_DEBUG, // flags
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
	
	// remove Alt+Enter behavior
	{
		IDXGIDevice* pDXGIDevice = NULL;
		IDXGIAdapter* pDXGIAdapter = NULL;
		IDXGIFactory* pIDXGIFactory = NULL;
		
		hr = device->QueryInterface( g_IID_IDXGIDevice, (void**) &pDXGIDevice );
		ASSERT( !FAILED( hr ) );
		hr = pDXGIDevice->GetParent( g_IID_IDXGIAdapter, (void**) &pDXGIAdapter );
		ASSERT( !FAILED( hr ) );
		pDXGIAdapter->GetParent( g_IID_IDXGIFactory, (void**) &pIDXGIFactory );
		pIDXGIFactory->MakeWindowAssociation( (HWND) windowHandle, DXGI_MWA_NO_ALT_ENTER );
		SAFE_RELEASE( pIDXGIFactory );
		SAFE_RELEASE( pDXGIAdapter );
		SAFE_RELEASE( pDXGIDevice );
	}
	
	// Backbuffer
	hr = swapChain->GetBuffer( 0, g_IID_ID3D11Texture2D, (void**) &backBuffer );
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
	RTInfo rti( settings.width, settings.height, DXGI_FORMAT_D24_UNORM_S8_UINT );
	if( create_rtt( device, rti, msamples, true, false, &depthBuffer ) ||
		create_dsview( device, rti, depthBuffer, &dsView ) )
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
	
	// default sampler state
	D3D11_SAMPLER_DESC sdesc =
	{
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0, 1, D3D11_COMPARISON_NEVER, {0,0,0,0}, 0, D3D11_FLOAT32_MAX
	};
	if( FAILED( D3DCALL( device->CreateSamplerState( &sdesc, &R->m_sampState ) ) ) )
		return NULL;
	// depth sampler state
	D3D11_SAMPLER_DESC sdesc2 =
	{
		D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0, 1, D3D11_COMPARISON_LESS, {0,0,0,0}, 0, D3D11_FLOAT32_MAX
	};
	if( FAILED( D3DCALL( device->CreateSamplerState( &sdesc2, &R->m_depthSampState ) ) ) )
		return NULL;
	
	// default vertex data
	if( create_buf( context, device, sizeof(g_initial_backup_vertex_data), false, D3D11_BIND_VERTEX_BUFFER, &g_initial_backup_vertex_data, &R->m_vertbuf_defaults ) )
		return NULL;
	
	// batch vertex constant buffer data
	if( create_buf( context, device, sizeof(R->m_cbdata_vs_batchverts), true, D3D11_BIND_CONSTANT_BUFFER, &R->m_cbdata_vs_batchverts, &R->m_cbuf_vs_batchverts ) )
		return NULL;
	
	// pass constant buffers
	if( create_buf( context, device, sizeof(SGRX_RPCoreData), true, D3D11_BIND_CONSTANT_BUFFER, NULL, &R->m_cbuf0_common ) ) return NULL;
	if( create_buf( context, device, sizeof(SGRX_RPInstanceData), true, D3D11_BIND_CONSTANT_BUFFER, NULL, &R->m_cbuf1_inst ) ) return NULL;
	if( create_buf( context, device, sizeof(SGRX_RPSkinMatrices), true, D3D11_BIND_CONSTANT_BUFFER, NULL, &R->m_cbuf2_skin ) ) return NULL;
	R->m_cbuf6_ltcount.DynSetZero( device, context, sizeof(Vec4) );
	R->m_cbuf3_ltpoint.DynSetZero( device, context, sizeof(SGRX_RPPointLightData) * 16 );
	R->m_cbuf4_ltspotvs.DynSetZero( device, context, sizeof(SGRX_RPSpotLightDataVS) * 2 );
	R->m_cbuf4_ltspotps.DynSetZero( device, context, sizeof(SGRX_RPSpotLightDataPS) * 2 );
	
	// initial viewport settings
	R->SetViewport( 0, 0, settings.width, settings.height );
	R->SetScissorRect( NULL );
	
	return R;
}

void D3D11Renderer::Destroy()
{
	SAFE_RELEASE( m_cbuf0_common );
	SAFE_RELEASE( m_cbuf1_inst );
	SAFE_RELEASE( m_cbuf2_skin );
	m_cbuf3_ltpoint.Free();
	m_cbuf4_ltspotvs.Free();
	m_cbuf4_ltspotps.Free();
	m_cbuf6_ltcount.Free();
	
	SAFE_RELEASE( m_sampState );
	SAFE_RELEASE( m_depthSampState );
	SAFE_RELEASE( m_vertbuf_defaults );
	SAFE_RELEASE( m_cbuf_vs_batchverts );
	m_vertbuf_batchverts.Free();
	m_cbuf_ps_batchverts.Free();
	
	SAFE_RELEASE( m_dsView );
	SAFE_RELEASE( m_rtView );
	SAFE_RELEASE( m_depthBuffer );
	SAFE_RELEASE( m_backBuffer );
	SAFE_RELEASE( m_swapChain );
	SAFE_RELEASE( m_ctx );
	
//	ID3D11Debug* dbg = NULL;
//	m_dev->QueryInterface( g_IID_ID3D11Debug, (void**) &dbg );
	
	SAFE_RELEASE( m_dev );
	
//	if( dbg )
//	{
//		dbg->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
//		dbg->Release();
//	}
	
	delete this;
}

bool D3D11Renderer::LoadInternalResources()
{
	return true;
}

void D3D11Renderer::UnloadInternalResources()
{
}

void D3D11Renderer::Swap()
{
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
	m_swapChain->Present( m_currSettings.vsync ? 1 : 0, 0 );
}

void D3D11Renderer::Modify( const RenderSettings& settings )
{
	HRESULT hr;
	
	SAFE_RELEASE( m_dsView );
	SAFE_RELEASE( m_rtView );
	SAFE_RELEASE( m_depthBuffer );
	SAFE_RELEASE( m_backBuffer );
	
	int msamples = settings.aa_mode == ANTIALIAS_MULTISAMPLE ? TMAX( 1, TMIN( 16, settings.aa_quality ) ) : 1;
	
	hr = m_swapChain->ResizeBuffers( 0, settings.width, settings.height,
		DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH );
	if( FAILED( hr ) )
	{
		LOG_ERROR << "Failed to resize D3D11 swap chain";
		return;
	}
	
	// Backbuffer
	hr = m_swapChain->GetBuffer( 0, g_IID_ID3D11Texture2D, (void**) &m_backBuffer );
	if( FAILED( hr ) || m_backBuffer == NULL )
	{
		LOG_ERROR << "Failed to retrieve D3D11 backbuffer";
		return;
	}
	
	hr = m_dev->CreateRenderTargetView( m_backBuffer, NULL, &m_rtView );
	if( FAILED( hr ) || m_rtView == NULL )
	{
		LOG_ERROR << "Failed to create D3D11 render target view";
		return;
	}
	
	// Depth/stencil buffer
	RTInfo rti( settings.width, settings.height, DXGI_FORMAT_D24_UNORM_S8_UINT );
	if( create_rtt( m_dev, rti, msamples, true, false, &m_depthBuffer ) ||
		create_dsview( m_dev, rti, m_depthBuffer, &m_dsView ) )
	{
		return;
	}
	
	m_currSettings = settings;
}


void D3D11Renderer::SetRenderTargets( const SGRX_RTClearInfo& info, SGRX_IDepthStencilSurface* dss, SGRX_RTSpec rts[4] )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
	Vec4 col = Col32ToVec4( info.clearColor );
	ID3D11DepthStencilView* dsv = NULL;
	
	if( rts[0].IsUnused() && rts[1].IsUnused() && rts[2].IsUnused() && rts[3].IsUnused() )
	{
		if( info.flags & SGRX_RT_ClearColor )
			m_ctx->ClearRenderTargetView( m_rtView, &col.x );
		m_ctx->OMSetRenderTargets( 1, &m_rtView, m_dsView );
		dsv = m_dsView;
		_SetViewport( 0, 0, m_currSettings.width, m_currSettings.height );
	}
	else
	{
		dsv = dss ? ((D3D11DepthStencilSurface*)dss)->DSV : NULL;
		ID3D11RenderTargetView* rtv[4] = { NULL, NULL, NULL, NULL };
		uint32_t w = 0, h = 0;
		for( int i = 0; i < 4; ++i )
		{
			SGRX_ITexture* rt = rts[ i ].rtt;
			if( rt )
			{
				ASSERT( rt->m_isRenderTexture );
				ASSERT( !w || w == rt->m_info.width );
				ASSERT( !h || h == rt->m_info.height );
				w = rt->m_info.width;
				h = rt->m_info.height;
				SGRX_CAST( D3D11RenderTexture*, RT, rt );
				if( RT->IsDepthRT() )
					dsv = RT->GetDSV( 0, 0 );
				else
					rtv[ i ] = RT->GetRTV( rts[ i ].side, rts[ i ].mip );
			}
		}
		if( info.flags & SGRX_RT_ClearColor )
		{
			for( int i = 0; i < 4; ++i )
			{
				if( rtv[ i ] )
					m_ctx->ClearRenderTargetView( rtv[ i ], &col.x );
			}
		}
		m_ctx->OMSetRenderTargets( 4, rtv, dsv );
		_SetViewport( 0, 0, w, h );
	}
	
	// clear buffers
	if( ( info.flags & (SGRX_RT_ClearDepth|SGRX_RT_ClearStencil) ) && dsv )
	{
		int dscf = 0;
		if( info.flags & SGRX_RT_ClearDepth )
			dscf |= D3D11_CLEAR_DEPTH;
		if( info.flags & SGRX_RT_ClearStencil )
			dscf |= D3D11_CLEAR_STENCIL;
		m_ctx->ClearDepthStencilView( dsv, dscf, info.clearDepth, info.clearStencil );
	}
}

void D3D11Renderer::SetViewport( int x0, int y0, int x1, int y1 )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
	_SetViewport( x0, y0, x1, y1 );
}

void D3D11Renderer::_SetViewport( int x0, int y0, int x1, int y1 )
{
	D3D11_VIEWPORT vp = { x0, y0, x1 - x0, y1 - y0, 0.0f, 1.0f };
	m_viewport = vp;
	m_ctx->RSSetViewports( 1, &vp );
}

void D3D11Renderer::SetScissorRect( int* rect )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
	if( rect )
	{
		D3D11_RECT rct = { rect[0], rect[1], rect[2], rect[3] };
		m_ctx->RSSetScissorRects( 1, &rct );
	}
	else
	{
		int x = m_viewport.TopLeftX;
		int y = m_viewport.TopLeftY;
		D3D11_RECT rct = { x, y, x + m_viewport.Width, y + m_viewport.Height };
		m_ctx->RSSetScissorRects( 1, &rct );
	}
}


D3D11Texture::~D3D11Texture()
{
	m_renderer->m_ownTextures.unset( this );
	SAFE_RELEASE( m_rsrcView );
	SAFE_RELEASE( m_sampState );
	SAFE_RELEASE( m_texture );
}

bool D3D11Texture::UploadRGBA8Part( void* data, int mip, int x, int y, int w, int h )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_renderer->m_mutex );
	
	if( m_info.type != TEXTYPE_2D )
	{
		LOG_ERROR << "texture not 2D";
		return false;
	}
	
	// sure, we copied... nothing
	if( !w || !h )
		return true;
	
	bool dyn = false;
	if( dyn )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		
		bool whole = x == 0 && y == 0 && w == m_info.width && h == m_info.height;
		HRESULT hr = m_renderer->m_ctx->Map( m_texture, mip, whole ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &msr );
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
		}
		
		m_renderer->m_ctx->Unmap( m_texture, mip );
	}
	else
	{
		D3D11_BOX box = { x, y, 0, x + w, y + h, 1 };
		m_renderer->m_ctx->UpdateSubresource( m_texture, mip, &box, data, w * 4, 0 );
	}
	
	return true;
}

bool D3D11Texture::UploadRGBA8Part3D( void* data, int mip, int x, int y, int z, int w, int h, int d )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_renderer->m_mutex );
	
	if( m_info.type != TEXTYPE_VOLUME )
	{
		LOG_ERROR << "texture not 3D";
		return false;
	}
	
	// sure, we copied... nothing
	if( !w || !h || !d )
		return true;
	
	bool dyn = false;
	if( dyn )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		
		bool whole = x == 0 && y == 0 && z == 0 && w == m_info.width && h == m_info.height && d == m_info.depth;
		HRESULT hr = m_renderer->m_ctx->Map( m_texture, mip, whole ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &msr );
		if( FAILED( hr ) )
		{
			LOG_ERROR << "failed to map D3D11 texture";
			return false;
		}
		*(uint8_t**)&msr.pData += msr.DepthPitch * z + msr.RowPitch * y + 4 * x;
		
		for( int k = 0; k < d; ++k )
		{
			for( int j = 0; j < h; ++j )
			{
				uint8_t* dst = (uint8_t*)msr.pData + msr.RowPitch * j + msr.DepthPitch * k;
				memcpy( dst, ((uint32_t*)data) + w * j + w * h * k, w * 4 );
			}
		}
		
		m_renderer->m_ctx->Unmap( m_texture, mip );
	}
	else
	{
		D3D11_BOX box = { x, y, z, x + w, y + h, z + d };
		m_renderer->m_ctx->UpdateSubresource( m_texture, mip, &box, data, w * 4, w * h * 4 );
	}
	
	return true;
}

SGRX_ITexture* D3D11Renderer::CreateTexture( TextureInfo* texinfo, const void* data )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
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
		if( texinfo->type == TEXTYPE_CUBE )
			dtd.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		
		D3D11_SUBRESOURCE_DATA srd[ 6 * 16 ];
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
					
					srd[ at ].pSysMem = (char*) data + TextureData_GetMipDataOffset( texinfo, side, mip );
					srd[ at ].SysMemPitch = crs;
					at++;
				}
			}
		}
		
		if( FAILED( D3DCALL( m_dev->CreateTexture2D( &dtd, data ? srd : NULL, &tex2d ) ) ) )
		{
			LOG_ERROR << "could not create D3D11 texture (type: 2D, w: " << texinfo->width << ", h: " <<
				texinfo->height << ", mips: " << texinfo->mipcount << ", fmt: " << texinfo->format << ", d3dfmt: " << texfmt2d3d( texinfo->format );
			return NULL;
		}
		
		if( create_srview( m_dev, tex2d, texinfo->type, dtd.Format, &srv ) )
		{
			SAFE_RELEASE( tex2d );
			return NULL;
		}
		
		D3D11_SAMPLER_DESC sdesc =
		{
		//	texinfo->flags & TEXFLAGS_LERP ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT,
			texinfo->flags & TEXFLAGS_LERP ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_POINT,
			texinfo->flags & TEXFLAGS_CLAMP_X ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP,
			texinfo->flags & TEXFLAGS_CLAMP_Y ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP,
			texinfo->flags & TEXFLAGS_CLAMP_Z ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP,
			0, 16, D3D11_COMPARISON_NEVER, {0,0,0,0}, 0, D3D11_FLOAT32_MAX
		};
		if( FAILED( D3DCALL( m_dev->CreateSamplerState( &sdesc, &samp ) ) ) )
		{
			SAFE_RELEASE( tex2d );
			SAFE_RELEASE( srv );
			return NULL;
		}
		
		D3D11Texture* T = new D3D11Texture;
		T->m_renderer = this;
		T->m_info = *texinfo;
		T->_Texture2D() = tex2d;
		T->m_sampState = samp;
		T->m_rsrcView = srv;
		m_ownTextures.set( T, true );
		return T;
	}
	else if( texinfo->type == TEXTYPE_VOLUME )
	{
		ID3D11Texture3D* tex3d = NULL;
		ID3D11SamplerState* samp = NULL;
		ID3D11ShaderResourceView* srv = NULL;
		
		D3D11_TEXTURE3D_DESC dtd;
		memset( &dtd, 0, sizeof(dtd) );
		dtd.Width = texinfo->width;
		dtd.Height = texinfo->height;
		dtd.Depth = texinfo->depth;
		dtd.MipLevels = texinfo->mipcount;
		dtd.Format = texfmt2d3d( texinfo->format );
		dtd.Usage = D3D11_USAGE_DEFAULT;
		dtd.CPUAccessFlags = 0;
		dtd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		
		D3D11_SUBRESOURCE_DATA srd[ 16 ];
		memset( &srd, 0, sizeof(srd) );
		if( data )
		{
			TextureInfo mipinfo;
			for( int mip = 0; mip < texinfo->mipcount; ++mip )
			{
				size_t crs, crc, csc;
				
				TextureInfo_GetMipInfo( texinfo, mip, &mipinfo );
				TextureInfo_GetCopyDims( &mipinfo, &crs, &crc, &csc );
				
				srd[ mip ].pSysMem = (char*) data + TextureData_GetMipDataOffset( texinfo, 0, mip );
				srd[ mip ].SysMemPitch = crs;
				// 'csc' was requested to obtain a good value for 'crc':
				srd[ mip ].SysMemSlicePitch = crs * crc;
			}
		}
		
		if( FAILED( D3DCALL( m_dev->CreateTexture3D( &dtd, data ? srd : NULL, &tex3d ) ) ) )
		{
			LOG_ERROR << "could not create D3D11 texture (type: 3D, w: " << texinfo->width << ", h: "
				<< texinfo->height << ", d: " << texinfo->depth << ", mips: " << texinfo->mipcount
				<< ", fmt: " << texinfo->format << ", d3dfmt: " << texfmt2d3d( texinfo->format );
			return NULL;
		}
		
		if( create_srview( m_dev, tex3d, texinfo->type, dtd.Format, &srv ) )
		{
			SAFE_RELEASE( tex3d );
			return NULL;
		}
		
		D3D11_SAMPLER_DESC sdesc =
		{
		//	texinfo->flags & TEXFLAGS_LERP ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT,
			texinfo->flags & TEXFLAGS_LERP ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_POINT,
			texinfo->flags & TEXFLAGS_CLAMP_X ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP,
			texinfo->flags & TEXFLAGS_CLAMP_Y ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP,
			texinfo->flags & TEXFLAGS_CLAMP_Z ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP,
			0, 16, D3D11_COMPARISON_NEVER, {0,0,0,0}, 0, D3D11_FLOAT32_MAX
		};
		if( FAILED( D3DCALL( m_dev->CreateSamplerState( &sdesc, &samp ) ) ) )
		{
			SAFE_RELEASE( tex3d );
			SAFE_RELEASE( srv );
			return NULL;
		}
		
		D3D11Texture* T = new D3D11Texture;
		T->m_renderer = this;
		T->m_info = *texinfo;
		T->_Texture3D() = tex3d;
		T->m_sampState = samp;
		T->m_rsrcView = srv;
		m_ownTextures.set( T, true );
		return T;
	}
	
	LOG_ERROR << "TODO [reached a part of not-yet-defined behavior]";
	return NULL;
}


ID3D11RenderTargetView* D3D11RenderTexture::GetRTV( int side, int mip )
{
	if( m_info.type == TEXTYPE_CUBE )
		ASSERT( side >= 0 && side < 6 );
	else
		ASSERT( side == 0 ); // TODO VOLUME?
	ASSERT( mip >= 0 && mip < m_info.mipcount );
	
	if( side != RTVside || mip != RTVmip )
	{
		SAFE_RELEASE( RTView );
		if( create_rtview( m_renderer->m_dev, m_rtinfo, m_texture, &RTView, mip, side ) )
			return NULL;
		RTVside = side;
		RTVmip = mip;
	}
	return RTView;
}

ID3D11DepthStencilView* D3D11RenderTexture::GetDSV( int side, int mip )
{
	if( m_info.type == TEXTYPE_CUBE )
		ASSERT( side >= 0 && side < 6 );
	else
		ASSERT( side == 0 );
	ASSERT( mip >= 0 && mip < m_info.mipcount );
	
	if( side != DSVside || mip != DSVmip )
	{
		SAFE_RELEASE( DSView );
		if( create_dsview( m_renderer->m_dev, m_rtinfo, m_texture, &DSView, mip, side ) )
			return NULL;
		DSVside = side;
		DSVmip = mip;
	}
	return DSView;
}

SGRX_ITexture* D3D11Renderer::CreateRenderTexture( TextureInfo* texinfo )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
	RTInfo rti( texinfo );
	if( rti.IsFormatGood() == false )
	{
		LOG_ERROR << "format ID was not recognized / supported: " << texinfo->format;
		return NULL;
	}
	
	Handle<D3D11RenderTexture> RT = new D3D11RenderTexture;
	
	if( IS_FLAG_SET( texinfo->format, TEXFF_RTDEPTHFMT ) )
	{
		LOG_FUNCTION_ARG( "DEPTH_RT" );
		
		if( create_rtt( m_dev, rti, 0, true, true, &RT->m_texture ) )
			return NULL;
	}
	else
	{
		LOG_FUNCTION_ARG( "COLOR_RT" );
		
		if( create_rtt( m_dev, rti, 0, false, true, &RT->m_texture ) )
			return NULL;
	}
	
	// shader resource view
	if( create_srview( m_dev, RT->m_texture, rti.type, rti.GetSRVFormat(), &RT->m_rsrcView ) )
	{
		return NULL;
	}
	
	// sampler state
	D3D11_SAMPLER_DESC sdesc = {
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP,
		0, 1, D3D11_COMPARISON_NEVER, {0,0,0,0}, 0, D3D11_FLOAT32_MAX
	};
	if( FAILED( D3DCALL( m_dev->CreateSamplerState( &sdesc, &RT->m_sampState ) ) ) )
	{
		return NULL;
	}
	
	RT->m_renderer = this;
	RT->m_info = *texinfo;
	RT->m_rtinfo = rti;
	m_ownTextures.set( RT, true );
	return RT.Disown();
}


D3D11DepthStencilSurface::~D3D11DepthStencilSurface()
{
	SAFE_RELEASE( DT );
	SAFE_RELEASE( DSV );
	m_renderer->m_ownDSS.unset( this );
}

SGRX_IDepthStencilSurface* D3D11Renderer::CreateDepthStencilSurface( int width, int height, int format )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
	// function returns matching depth surface for color RT format
	if( !IS_FLAG_SET( format, TEXFF_RTCOLFMT ) )
		return NULL;
	
	Handle<D3D11DepthStencilSurface> DSS = new D3D11DepthStencilSurface;
	RTInfo rti( width, height, DXGI_FORMAT_D24_UNORM_S8_UINT );
	if( create_rtt( m_dev, rti, 0, true, false, &DSS->DT ) ||
		create_dsview( m_dev, rti, DSS->DT, &DSS->DSV ) )
	{
		// error message already printed
		return NULL;
	}
	DSS->m_renderer = this;
	m_ownDSS.set( DSS, true );
	return DSS.Disown();
}


bool D3D11Renderer::CompileShader( const StringView& path, EShaderType shadertype, const StringView& code, ByteArray& outcomp, String& outerrors )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
#ifdef ENABLE_SHADER_COMPILING
	if( g_pfnD3DCompile )
	{
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
		
		hr = g_pfnD3DCompile( code.data(), code.size(), StackPath( path ), macros, NULL, "main", profile, D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, &outbuf, &outerr );
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
	else
#endif // ENABLE_SHADER_COMPILING
	{
		LOG << "<<< D3D11 SHADER COMPILATION IS NOT ALLOWED IN THIS BUILD >>>";
		LOG << "Uncompiled shader: " << path;
		return false;
	}
}

SGRX_IVertexShader* D3D11Renderer::CreateVertexShader( const StringView& path, ByteArray& code )
{
	LOG_FUNCTION;
	
	HRESULT hr;
	ByteReader br( code );
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
	LOG_FUNCTION;
	
	HRESULT hr;
	ByteReader br( code );
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
	SAFE_RELEASE( m_RS_FCCW );
	SAFE_RELEASE( m_BS );
	SAFE_RELEASE( m_DS );
	
	D3D11_RASTERIZER_DESC rdesc =
	{
		fillModes[ state.wireFill ],
		cullModes[ state.cullMode ],
		FALSE,
		state.depthBias * (1000/1e-5f), state.depthBiasClamp, state.slopeDepthBias,
		TRUE,
		state.scissorEnable,
		state.multisampleEnable,
		TRUE
	};
	if( FAILED( D3DCALL( m_renderer->m_dev->CreateRasterizerState( &rdesc, &m_RS ) ) ) )
	{
		// error, use default or something
	}
	rdesc.FrontCounterClockwise = TRUE;
	if( FAILED( D3DCALL( m_renderer->m_dev->CreateRasterizerState( &rdesc, &m_RS_FCCW ) ) ) )
	{
		// error, use default or something
	}
	
	// batch vertex blending states
	D3D11_BLEND_DESC bdesc = { FALSE, state.separateBlend };
	for( int i = 0; i < SGRX_RS_MAX_RENDER_TARGETS; ++i )
	{
		const SGRX_RenderState::BlendState& bs = state.blendStates[ state.separateBlend ? i : 0 ];
		D3D11_RENDER_TARGET_BLEND_DESC tbdesc =
		{
			bs.blendEnable,
			blendFactors[ bs.srcBlend ], blendFactors[ bs.dstBlend ], blendOps[ bs.blendOp ],
			blendFactors[ bs.srcBlendAlpha ], blendFactors[ bs.dstBlendAlpha ], blendOps[ bs.blendOpAlpha ],
			bs.colorWrite,
		};
		bdesc.RenderTarget[ i ] = tbdesc;
	}
	if( FAILED( D3DCALL( m_renderer->m_dev->CreateBlendState( &bdesc, &m_BS ) ) ) )
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
	if( FAILED( D3DCALL( m_renderer->m_dev->CreateDepthStencilState( &dsdesc, &m_DS ) ) ) )
	{
		// error, use default or something
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

inline const char* format_to_str( DXGI_FORMAT fmt )
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
	LOG_FUNCTION;
	
	int i;
	bool hascolor = false;
	bool hasnrm = false;
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
		if( vdinfo.usages[ i ] == VDECLUSAGE_NORMAL )
			hasnrm = true;
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
	if( !hasnrm )
	{
		elements[ i ].SemanticName = "NORMAL";
		elements[ i ].SemanticIndex = 0;
		elements[ i ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		elements[ i ].InputSlot = 1;
		elements[ i ].AlignedByteOffset = 16;
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
		elements[ i ].AlignedByteOffset = 32;
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
		elements[ i ].AlignedByteOffset = 48;
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
		elements[ i ].AlignedByteOffset = 64;
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
		elements[ i ].AlignedByteOffset = 80;
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
		elements[ i ].AlignedByteOffset = 96;
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
}

bool D3D11Mesh::InitVertexBuffer( size_t size, VertexDeclHandle vd )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_renderer->m_mutex );
	
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	SAFE_RELEASE( m_VB );
	
	m_origVertexSize = vd->m_info.size;
	m_realVertexSize = divideup( m_origVertexSize, 16 ) * 16;
	m_vertexDataSize = size;
	m_realVertexDataSize = divideup( size, m_origVertexSize ) * m_realVertexSize;
	
	if( create_buf( m_renderer->m_ctx, m_renderer->m_dev, m_realVertexDataSize, dyn, D3D11_BIND_VERTEX_BUFFER, NULL, &m_VB ) )
		return false;
	
	m_vertexDecl = vd;
	
	return true;
}

bool D3D11Mesh::InitIndexBuffer( size_t size, bool i32 )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_renderer->m_mutex );
	
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	SAFE_RELEASE( m_IB );
	if( create_buf( m_renderer->m_ctx, m_renderer->m_dev, size, dyn, D3D11_BIND_INDEX_BUFFER, NULL, &m_IB ) )
		return false;
	m_dataFlags = ( m_dataFlags & ~MDF_INDEX_32 ) | ( MDF_INDEX_32 * i32 );
	m_indexDataSize = size;
	return true;
}

bool D3D11Mesh::UpdateVertexData( const void* data, size_t size )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_renderer->m_mutex );
	
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
	
	return true;
}

bool D3D11Mesh::UpdateIndexData( const void* data, size_t size )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_renderer->m_mutex );
	
	if( size > m_indexDataSize )
	{
		LOG_ERROR << "given index data is too big";
		return false;
	}
	
	bool dyn = !!( m_dataFlags & MDF_DYNAMIC );
	upload_buf( m_renderer->m_ctx, m_IB, dyn, true, data, size );
	
	return true;
}

SGRX_IMesh* D3D11Renderer::CreateMesh()
{
	LOG_FUNCTION;
	
	D3D11Mesh* mesh = new D3D11Mesh;
	mesh->m_renderer = this;
	m_ownMeshes.set( mesh, true );
	return mesh;
}

SGRX_IVertexInputMapping* D3D11Renderer::CreateVertexInputMapping( SGRX_IVertexShader* vs, SGRX_IVertexDecl* vd )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
	SGRX_CAST( D3D11VertexShader*, VS, vs );
	SGRX_CAST( D3D11VertexDecl*, VD, vd );
	
	ID3D11InputLayout* IL = NULL;
	
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
	
	HRESULT hr = m_dev->CreateInputLayout( VD->m_elements, VD->m_elemCount, VS->m_VSBC.data(), VS->m_VSBC.size(), &IL );
	if( FAILED( hr ) || !IL )
	{
		LOG_ERROR << "Failed to create an input layout (sh.key=" << VS->m_key << ", v.d.key=" << VD->m_key << ")";
		return NULL;
	}
	
	D3D11VertexInputMapping* vim = new D3D11VertexInputMapping;
	vim->m_inputLayout = IL;
	return vim;
}
	

void D3D11Renderer::SetMatrix( bool view, const Mat4& mtx )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
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

void D3D11Renderer::DrawImmediate( SGRX_ImmDrawData& idd )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
	SetVertexShader( idd.vertexShader );
	SetPixelShader( idd.pixelShader );
	SetRenderState( idd.renderState, false );
	
	m_vertbuf_batchverts.Upload( m_dev, m_ctx, idd.vertices, idd.vertexDecl->m_info.size * idd.vertexCount );
	if( idd.shdata && idd.shvcount )
		m_cbuf_ps_batchverts.Upload( m_dev, m_ctx, idd.shdata, sizeof(*idd.shdata) * idd.shvcount );
	
	m_ctx->VSSetConstantBuffers( 0, 1, &m_cbuf_vs_batchverts );
	
	m_ctx->PSSetConstantBuffers( 0, 1, m_cbuf_ps_batchverts.PPBuf() );
	
	ID3D11ShaderResourceView* srvs[ SGRX_MAX_TEXTURES ];
	ID3D11SamplerState* smps[ SGRX_MAX_TEXTURES ];
	for( int i = 0; i < SGRX_MAX_TEXTURES; ++i )
	{
		SGRX_ITexture* tex = idd.textures[ i ];
		srvs[ i ] = tex ? ((D3D11Texture*)tex)->m_rsrcView : NULL;
		smps[ i ] = tex ? ((D3D11Texture*)tex)->m_sampState : m_sampState;
	}
	m_ctx->PSSetShaderResources( 0, SGRX_MAX_TEXTURES, srvs );
	m_ctx->PSSetSamplers( 0, SGRX_MAX_TEXTURES, smps );
	
	m_ctx->IASetPrimitiveTopology( conv_prim_type( idd.primType ) );
	m_ctx->IASetInputLayout( ((D3D11VertexInputMapping*) idd.vertexInputMapping)->m_inputLayout );
	ID3D11Buffer* vbufs[2] = { m_vertbuf_batchverts, m_vertbuf_defaults };
	const UINT strides[2] = { idd.vertexDecl->m_info.size, sizeof(BackupVertexData) };
	const UINT offsets[2] = { 0, 0 };
	m_ctx->IASetVertexBuffers( 0, 2, vbufs, strides, offsets );
	
	m_ctx->Draw( idd.vertexCount, 0 );
}


void D3D11Renderer::DoRenderItems( SGRX_Scene* scene, SGRX_PassType passtype, int maxrepeat, const SGRX_Camera& cam, RenderItem* start, RenderItem* end )
{
	LOG_FUNCTION;
	
	SGRX_ScopedMtxLock LOCK( &m_mutex );
	
	if( passtype == SGRX_PassType_Shadow )
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
	upload_buf( m_ctx, m_cbuf0_common, true, true, &coredata, sizeof(coredata) );
	
	ID3D11Buffer* cbufs[7] =
	{
		m_cbuf0_common,
		m_cbuf1_inst,
		m_cbuf6_ltcount,
		m_cbuf2_skin,
		m_cbuf3_ltpoint,
		m_cbuf4_ltspotvs,
		m_cbuf4_ltspotps,
	};
	m_ctx->VSSetConstantBuffers( 0, 7, cbufs );
	m_ctx->PSSetConstantBuffers( 0, 7, cbufs );
	
	RenderItem* RI = start;
	while( RI < end )
	{
		SGRX_MeshInstance* MI = RI->MI;
		uint16_t part_id = RI->part_id;
		
		D3D11Mesh* M = (D3D11Mesh*) MI->GetMesh();
		const SGRX_MeshPart& MP = M->m_meshParts[ part_id ];
	//	D3D11VertexDecl* VD = (D3D11VertexDecl*) M->m_vertexDecl.item;
		SGRX_DrawItem* DI = &MI->m_drawItems[ part_id ];
		const SGRX_Material& MTL = MI->GetMaterial( part_id );
		const SGRX_XShdInst::Pass& XPS = DI->XSH->passes[ passtype - 1 ]; /* TODO HACK */
		if( XPS.pixelShader == NULL )
		{
			RI++;
			continue;
		}
		
		SetRenderState( XPS.renderState, scene->frontCCW );
		SetVertexShader( XPS.vertexShader );
		SetPixelShader( XPS.pixelShader );
		SGRX_CAST( D3D11VertexInputMapping*, VIM, XPS.vtxInputMap.item );
		m_ctx->IASetInputLayout( VIM->m_inputLayout );
		
		// instance state
		ID3D11ShaderResourceView* srvs[ SGRX_MAX_TEXTURES ] = { NULL };
		ID3D11SamplerState* smps[ SGRX_MAX_TEXTURES ] = { NULL };
		for( int i = 0; i < SGRX_MAX_TEXTURES; ++i )
		{
			D3D11Texture* tex = (D3D11Texture*) MTL.textures[ i ].item;
			if( m_overrideTextures[ i ] )
				tex = (D3D11Texture*) m_overrideTextures[ i ].item;
			srvs[ i ] = tex ? ((D3D11Texture*)tex)->m_rsrcView : NULL;
			smps[ i ] = tex ? ((D3D11Texture*)tex)->m_sampState : m_sampState;
		}
		m_ctx->PSSetShaderResources( 0, SGRX_MAX_TEXTURES, srvs );
		m_ctx->PSSetSamplers( 0, SGRX_MAX_TEXTURES, smps );
		
		Mat4 mWorldView;
		mWorldView.Multiply( MI->matrix, cam.mView );
		SGRX_RPInstanceData instdata =
		{
			MI->matrix,
			mWorldView,
		};
		for( int i = 0; i < MAX_MI_CONSTANTS; ++i )
			instdata.gInstanceData[ i ] = MI->constants[ i ];
		upload_buf( m_ctx, m_cbuf1_inst, true, true, &instdata, sizeof(instdata) );
		
		if( MI->skin_matrices.size() )
		{
			size_t sz = TMIN( MI->skin_matrices.size(), size_t(64) ) * sizeof(Mat4);
			upload_buf( m_ctx, m_cbuf2_skin, true, true, MI->skin_matrices.data(), sz );
		}
		
		ID3D11Buffer* vbufs[2] = { M->m_VB, m_vertbuf_defaults };
		const UINT strides[2] = { M->m_realVertexSize, sizeof(BackupVertexData) };
		const UINT offsets[2] = { 0, 0 };
		m_ctx->IASetVertexBuffers( 0, 2, vbufs, strides, offsets );
		m_ctx->IASetIndexBuffer( M->m_IB, M->m_dataFlags & MDF_INDEX_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0 );
		m_ctx->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		
		for( int numruns = 0; numruns < maxrepeat; ++numruns )
		{
			if( passtype != SGRX_PassType_Shadow )
			{
				SGRX_RPPointLightData PLData[ 16 ] = {0};
				SGRX_RPSpotLightDataPS SLDataPS[ 2 ] = {0};
				SGRX_RPSpotLightDataVS SLDataVS[ 2 ] = {0};
				SGRX_Light* SLDataLT[ 2 ] = {0};
				LightCount LC = SGRX_Renderer_FindLights( cam, DI,
					/* TMIN( int(PASS.numPL), */ passtype == SGRX_PassType_Spot ? 0 : 16 /* ) */,
					/* TMIN( int(PASS.numSL), */ passtype == SGRX_PassType_Spot ? 2 : 0 /* ) */,
					PLData, SLDataPS, SLDataVS, SLDataLT );
				
				if( passtype != SGRX_PassType_Base && LC.numPL + LC.numSL <= 0 )
					break;
				
				if( LC.numPL )
				{
					m_cbuf3_ltpoint.Upload( m_dev, m_ctx, PLData, sizeof(SGRX_RPPointLightData) * LC.numPL );
				}
				if( LC.numSL )
				{
					ID3D11ShaderResourceView* slt_srvs[4] = { NULL };
					ID3D11SamplerState* slt_smps[4] =
					{
						m_sampState, m_depthSampState,
						m_sampState, m_depthSampState
					};
					for( int i = 0; i < LC.numSL; ++i )
					{
						slt_srvs[ i * 2 + 0 ] = SLDataLT[ i ]->cookieTexture ?
							((D3D11Texture*)SLDataLT[ i ]->cookieTexture.item)->m_rsrcView : NULL;
						slt_smps[ i * 2 + 0 ] = SLDataLT[ i ]->cookieTexture ?
							((D3D11Texture*)SLDataLT[ i ]->cookieTexture.item)->m_sampState : m_sampState;
						slt_srvs[ i * 2 + 1 ] = SLDataLT[ i ]->shadowTexture ?
							((D3D11Texture*)SLDataLT[ i ]->shadowTexture.item)->m_rsrcView : NULL;
					//	slt_smps[ i * 2 + 1 ] = m_depthSampState;
					}
					m_ctx->PSSetShaderResources( 12, 4, slt_srvs );
					m_ctx->PSSetSamplers( 12, 4, slt_smps );
					m_cbuf4_ltspotvs.Upload( m_dev, m_ctx, SLDataVS, sizeof(SGRX_RPSpotLightDataVS) * LC.numSL );
					m_cbuf4_ltspotps.Upload( m_dev, m_ctx, SLDataPS, sizeof(SGRX_RPSpotLightDataPS) * LC.numSL );
				}
				
				Vec4 lightcounts = { LC.numPL, LC.numSL, 0, 0 };
				m_cbuf6_ltcount.Upload( m_dev, m_ctx, &lightcounts, sizeof(lightcounts) );
			}
			
			m_ctx->DrawIndexed( MP.indexCount, MP.indexOffset, MP.vertexOffset );
			
			m_stats.numDrawCalls++;
			if( passtype == SGRX_PassType_Shadow )
				m_stats.numSDrawCalls++;
			else
				m_stats.numMDrawCalls++;
		}
		RI++;
	}
}


void D3D11Renderer::SetVertexShader( const SGRX_IVertexShader* shd )
{
	SGRX_CAST( const D3D11VertexShader*, S, shd );
	m_ctx->VSSetShader( S ? S->m_VS : NULL, NULL, 0 );
}

void D3D11Renderer::SetPixelShader( const SGRX_IPixelShader* shd )
{
	SGRX_CAST( const D3D11PixelShader*, S, shd );
	m_ctx->PSSetShader( S ? S->m_PS : NULL, NULL, 0 );
}

void D3D11Renderer::SetRenderState( const SGRX_IRenderState* rsi, bool fccw )
{
	ASSERT( rsi );
	SGRX_CAST( const D3D11RenderState*, RS, rsi );
	
	m_ctx->RSSetState( fccw ? RS->m_RS_FCCW : RS->m_RS );
	m_ctx->OMSetBlendState( RS->m_BS, &RS->m_info.blendFactor.x, 0xffffffff );
	m_ctx->OMSetDepthStencilState( RS->m_DS, RS->m_info.stencilRef );
}


