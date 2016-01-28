

#pragma once
#include <engine.hpp>
#include <enganim.hpp>



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
SGRX_IFP32Handle SGRX_ResizeImage( SGRX_ImageFP32* image, int width, int height );


enum SGRX_AssetImageFilterType
{
	SGRX_AIF_Unknown = 0,
	SGRX_AIF_Resize,
	SGRX_AIF_Sharpen,
	SGRX_AIF_ToLinear,
	SGRX_AIF_FromLinear,
	SGRX_AIF_ExpandRange,
	SGRX_AIF_BCP,
	
	SGRX_AIF__COUNT,
};

struct SGRX_ImageFilterState
{
	bool isSRGB;
};

const char* SGRX_AssetImgFilterType_ToString( SGRX_AssetImageFilterType aift );
SGRX_AssetImageFilterType SGRX_AssetImgFilterType_FromString( const StringView& sv );

#define SGRX_IF_CLONE( name ) \
	SGRX_ImageFilter* Clone() const { return new name( *this ); }

struct SGRX_ImageFilter : SGRX_RefCounted
{
	SGRX_ImageFilter() : blend(1), cclamp(true), colors(0xff){}
	virtual SGRX_ImageFilter* Clone() const = 0;
	virtual SGRX_AssetImageFilterType GetType() const = 0;
	virtual const char* GetName() const = 0;
	virtual bool Parse( ConfigReader& cread ) = 0;
	virtual void Generate( String& out ) = 0;
	virtual SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs ) = 0;
	template< class T > T* upcast(){ return T::IsType( GetType() ) ? (T*) this : NULL; }
	
	bool ParseCMFParam( StringView key, StringView value );
	void GenerateCMFParams( String& out );
	void CMFBlend( SGRX_ImageFP32* src, SGRX_ImageFP32* dst );
	
	float blend;
	bool cclamp;
	uint8_t colors;
};
typedef Handle< SGRX_ImageFilter > SGRX_ImgFilterHandle;

struct SGRX_ImageFilter_Resize : SGRX_ImageFilter
{
	SGRX_ImageFilter_Resize() : width(256), height(256), srgb(false){}
	SGRX_IF_CLONE( SGRX_ImageFilter_Resize );
	static bool IsType( SGRX_AssetImageFilterType ift ){ return ift == SGRX_AIF_Resize; }
	SGRX_AssetImageFilterType GetType() const { return SGRX_AIF_Resize; }
	const char* GetName() const { return "resize"; }
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs );
	
	int width;
	int height;
	bool srgb;
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
	SGRX_IF_CLONE( SGRX_ImageFilter_Sharpen );
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
	SGRX_ImageFilter_Linear( bool fromlin ) : inverse( fromlin ){}
	SGRX_IF_CLONE( SGRX_ImageFilter_Linear );
	static bool IsType( SGRX_AssetImageFilterType ift )
	{ return ift == SGRX_AIF_ToLinear || ift == SGRX_AIF_FromLinear; }
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

struct SGRX_ImageFilter_ExpandRange : SGRX_ImageFilter
{
	SGRX_ImageFilter_ExpandRange() : vmin(V4(0)), vmax(V4(0,0,1,1)){}
	SGRX_IF_CLONE( SGRX_ImageFilter_ExpandRange );
	static bool IsType( SGRX_AssetImageFilterType ift ){ return ift == SGRX_AIF_ExpandRange; }
	SGRX_AssetImageFilterType GetType() const { return SGRX_AIF_ExpandRange; }
	const char* GetName() const { return "expand_range"; }
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs );
	
	Vec4 vmin;
	Vec4 vmax;
};

struct SGRX_ImageFilter_BCP : SGRX_ImageFilter
{
	SGRX_ImageFilter_BCP() : apply_bc1(true), brightness(0), contrast(1),
		apply_pow(false), power(1), apply_bc2(false), brightness_2(0), contrast_2(1)
	{ colors = 0x7; }
	SGRX_IF_CLONE( SGRX_ImageFilter_BCP );
	static bool IsType( SGRX_AssetImageFilterType ift ){ return ift == SGRX_AIF_BCP; }
	SGRX_AssetImageFilterType GetType() const { return SGRX_AIF_BCP; }
	const char* GetName() const { return "bcp"; }
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs );
	
	bool apply_bc1;
	float brightness;
	float contrast;
	bool apply_pow;
	float power;
	bool apply_bc2;
	float brightness_2;
	float contrast_2;
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
const char* SGRX_TextureOutputFormat_Ext( SGRX_TextureOutputFormat fmt );

struct SGRX_RevInfo
{
	SGRX_RevInfo() : ts_source(0), ts_output(0), rev_output(0), rev_asset(1){}
	
	// output state info
	uint32_t ts_source;
	uint32_t ts_output;
	uint32_t rev_output;
	// asset info
	uint32_t rev_asset;
};

struct SGRX_TextureAsset
{
	SGRX_TextureAsset();
	void Clone( const SGRX_TextureAsset& other );
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetFullName( String& out );
	void GetDesc( String& out );
	
	SGRX_RevInfo ri;
	
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

struct SGRX_MeshAssetPart : SGRX_RefCounted
{
	SGRX_MeshAssetPart() :
		shader("default"),
		mtlFlags(0),
		mtlBlendMode(SGRX_MtlBlend_None),
		optTransform(0)
	{}
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetDesc( int i, String& out );
	
	String meshName;
	String shader;
	String textures[ 8 ];
	uint8_t mtlFlags;
	uint8_t mtlBlendMode;
	int optTransform;
};
typedef Handle< SGRX_MeshAssetPart > SGRX_MeshAPHandle;

struct SGRX_MeshAsset
{
	SGRX_MeshAsset() :
		rotateY2Z(false),
		flipUVY(false),
		transform(false)
	{}
	void Clone( const SGRX_MeshAsset& other );
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetFullName( String& out );
	void GetDesc( String& out );
	
	SGRX_RevInfo ri;
	
	String sourceFile;
	String outputCategory;
	String outputName;
	bool rotateY2Z;
	bool flipUVY;
	bool transform;
	Array< SGRX_MeshAPHandle > parts;
};

struct SGRX_ABAnimSource
{
	void GetDesc( String& out );
	
	String file;
	String prefix;
};

struct SGRX_ABAnimation
{
	SGRX_ABAnimation() : startFrame(-1), endFrame(-1){}
	void GetDesc( String& out );
	
	String source;
	String name;
	int startFrame;
	int endFrame;
};

struct SGRX_AnimBundleAsset
{
	SGRX_AnimBundleAsset(){}
	void Clone( const SGRX_AnimBundleAsset& other );
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetFullName( String& out );
	void GetDesc( String& out );
	uint32_t LastSourceModTime();
	
	SGRX_RevInfo ri;
	
	String outputCategory;
	String outputName;
	String bundlePrefix;
	String previewMesh;
	Array< SGRX_ABAnimation > anims;
	Array< SGRX_ABAnimSource > sources;
};

struct SGRX_AssetScript
{
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	bool Load( const StringView& path );
	bool Save( const StringView& path );
	StringView GetCategoryPath( const StringView& name );
	// - additional state
	bool LoadAssetInfo( const StringView& path );
	bool SaveAssetInfo( const StringView& path );
	bool LoadOutputInfo( const StringView& path );
	bool SaveOutputInfo( const StringView& path );
	
	MeshHandle GetMesh( StringView path );
	
	HashTable< String, String > categories;
	Array< SGRX_TextureAsset > textureAssets;
	Array< SGRX_MeshAsset > meshAssets;
	Array< SGRX_AnimBundleAsset > animBundleAssets;
};


#ifndef ASSIMP_IMPORTER_TYPE
#define ASSIMP_IMPORTER_TYPE void
#endif
#ifndef ASSIMP_SCENE_TYPE
#define ASSIMP_SCENE_TYPE const void
#endif
enum SceneImportOptimizedFor
{
	SIOF_Meshes,
	SIOF_Anims,
};
struct SGRX_Scene3D : SGRX_RefCounted
{
	SGRX_Scene3D( const StringView& path, SceneImportOptimizedFor siof = SIOF_Meshes );
	~SGRX_Scene3D();
	
	void GetMeshList( Array< String >& out );
	void GetAnimList( Array< String >& out );
	
	String m_path;
	ASSIMP_IMPORTER_TYPE* m_imp;
	ASSIMP_SCENE_TYPE* m_scene;
	MeshHandle m_mesh;
	SGRX_AnimBundle m_animBundle;
};
typedef Handle< SGRX_Scene3D > ImpScene3DHandle;

SGRX_IFP32Handle SGRX_ProcessTextureAsset( const SGRX_TextureAsset& TA );
TextureHandle SGRX_FP32ToTexture( SGRX_ImageFP32* image, const SGRX_TextureAsset& TA );
MeshHandle SGRX_ProcessMeshAsset( const SGRX_AssetScript* AS, const SGRX_MeshAsset& MA );
AnimHandle SGRX_ProcessSingleAnim( const SGRX_AnimBundleAsset& ABA, int i );
void SGRX_ProcessAssets( SGRX_AssetScript& script, bool force = false );

