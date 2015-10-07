

#pragma once
#include <engine.hpp>



struct SGRX_ImageFP32 : SGRX_RefCounted
{
	FINLINE SGRX_ImageFP32() : m_width(0), m_height(0){}
	FINLINE SGRX_ImageFP32( int w, int h ){ Resize( w, h ); }
	FINLINE void Resize( int w, int h )
	{
		m_pixels.resize( w * h );
		m_width = w;
		m_height = h;
	}
	FINLINE size_t Size() const { return m_pixels.size(); }
	FINLINE int GetWidth() const { return m_width; }
	FINLINE int GetHeight() const { return m_height; }
	FINLINE const Vec4* GetData() const { return m_pixels.data(); }
	FINLINE Vec4* GetData(){ return m_pixels.data(); }
	FINLINE const Vec4& operator [] ( size_t i ) const { return m_pixels[ i ]; }
	FINLINE Vec4& operator [] ( size_t i ){ return m_pixels[ i ]; }
	FINLINE const Vec4& Pixel( int x, int y ) const { return m_pixels[ x + y * m_width ]; }
	FINLINE Vec4& Pixel( int x, int y ){ return m_pixels[ x + y * m_width ]; }
	FINLINE Vec4 GetClamped( int x, int y ) const
	{
		x = TMAX( 0, TMIN( m_width - 1, x ) );
		y = TMAX( 0, TMIN( m_height - 1, y ) );
		return m_pixels[ x + y * m_width ];
	}
	FINLINE Vec4 GetLerp( float x, float y ) const
	{
		int x0 = int( floor( x * m_width ) ) % m_width; if( x0 < 0 ) x0 += m_width;
		int x1 = int( ceil(  x * m_width ) ) % m_width; if( x1 < 0 ) x1 += m_width;
		int y0 = int( floor( y * m_height ) ) % m_height; if( y0 < 0 ) y0 += m_height;
		int y1 = int( ceil(  y * m_height ) ) % m_height; if( y1 < 0 ) y1 += m_height;
		float qx = fmodf( x * m_width, 1.0f );
		float qy = fmodf( y * m_height, 1.0f );
		Vec4 c00 = Pixel( x0, y0 ), c10 = Pixel( x1, y0 );
		Vec4 c01 = Pixel( x0, y1 ), c11 = Pixel( x1, y1 );
		return TLERP( TLERP( c00, c10, qx ), TLERP( c01, c11, qx ), qy );
	}
	
	Array< Vec4 > m_pixels;
	int m_width;
	int m_height;
};
typedef Handle< SGRX_ImageFP32 > SGRX_IFP32Handle;


enum SGRX_AssetImageFilterType
{
	SGRX_AIF_Unknown = 0,
	SGRX_AIF_Resize,
	SGRX_AIF_Sharpen,
	SGRX_AIF_ToLinear,
	SGRX_AIF_FromLinear,
	
	SGRX_AIF__COUNT,
};

struct SGRX_ImageFilterState
{
	bool isSRGB;
};

const char* SGRX_AssetImgFilterType_ToString( SGRX_AssetImageFilterType aift );
SGRX_AssetImageFilterType SGRX_AssetImgFilterType_FromString( const StringView& sv );

struct SGRX_ImageFilter : SGRX_RefCounted
{
	virtual SGRX_AssetImageFilterType GetType() const = 0;
	virtual const char* GetName() const = 0;
	virtual bool Parse( ConfigReader& cread ) = 0;
	virtual void Generate( String& out ) = 0;
	virtual SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs ) = 0;
	template< class T > T* upcast(){ return T::IsType( GetType() ) ? (T*) this : NULL; }
};
typedef Handle< SGRX_ImageFilter > SGRX_ImgFilterHandle;

struct SGRX_ImageFilter_Resize : SGRX_ImageFilter
{
	SGRX_ImageFilter_Resize() : width(256), height(256){}
	static bool IsType( SGRX_AssetImageFilterType ift ){ return ift == SGRX_AIF_Resize; }
	SGRX_AssetImageFilterType GetType() const { return SGRX_AIF_Resize; }
	const char* GetName() const { return "resize"; }
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs );
	
	int width;
	int height;
};

enum SGRX_ImgFltSharpen_Mode
{
	SGRX_IFS_0_1,
	SGRX_IFS_1_1,
	SGRX_IFS_1_2,
	
	SGRX_IFS__COUNT,
};
const char* SGRX_ImgFltSharpen_ToString( SGRX_ImgFltSharpen_Mode ifsm );
SGRX_ImgFltSharpen_Mode SGRX_ImgFltSharpen_FromString( const StringView& sv );

struct SGRX_ImageFilter_Sharpen : SGRX_ImageFilter
{
	SGRX_ImageFilter_Sharpen() : factor(0.1f), mode(SGRX_IFS_1_2){}
	static bool IsType( SGRX_AssetImageFilterType ift ){ return ift == SGRX_AIF_Sharpen; }
	SGRX_AssetImageFilterType GetType() const { return SGRX_AIF_Sharpen; }
	const char* GetName() const { return "sharpen"; }
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs );
	
	float factor;
	SGRX_ImgFltSharpen_Mode mode;
};

struct SGRX_ImageFilter_Linear : SGRX_ImageFilter
{
	static bool IsType( SGRX_AssetImageFilterType ift )
	{ return ift == SGRX_AIF_ToLinear || ift == SGRX_AIF_FromLinear; }
	SGRX_ImageFilter_Linear( bool fromlin ) : inverse( fromlin ){}
	SGRX_AssetImageFilterType GetType() const
	{
		return inverse ? SGRX_AIF_FromLinear : SGRX_AIF_ToLinear;
	}
	const char* GetName() const
	{
		return inverse ? "from_linear" : "to_linear";
	}
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs );
	
	bool inverse;
};

enum SGRX_TextureOutputFormat
{
	SGRX_TOF_Unknown = 0,
	
	SGRX_TOF_PNG_RGBA32,
	
	SGRX_TOF_STX_RGBA32,
	
	SGRX_TOF__COUNT,
};

const char* SGRX_TextureOutputFormat_ToString( SGRX_TextureOutputFormat fmt );
SGRX_TextureOutputFormat SGRX_TextureOutputFormat_FromString( const StringView& sv );

struct SGRX_TextureAsset
{
	SGRX_TextureAsset();
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetDesc( String& out );
	
	String sourceFile;
	String outputCategory;
	String outputName;
	SGRX_TextureOutputFormat outputType;
	bool isSRGB;
	bool mips;
	bool lerp;
	bool clampx;
	bool clampy;
	
	Array< SGRX_ImgFilterHandle > filters;
};

struct SGRX_MeshAsset
{
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetDesc( String& out );
	
	String sourceFile;
	String outputCategory;
	String outputName;
};

struct SGRX_AssetScript
{
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	bool Load( const StringView& path );
	bool Save( const StringView& path );
	
	HashTable< String, String > categories;
	Array< SGRX_TextureAsset > textureAssets;
	Array< SGRX_MeshAsset > meshAssets;
};


SGRX_IFP32Handle SGRX_ProcessTextureAsset( const SGRX_TextureAsset& TA );
TextureHandle SGRX_FP32ToTexture( SGRX_ImageFP32* image, const SGRX_TextureAsset& TA );
void SGRX_ProcessAssets( const SGRX_AssetScript& script );

