

#pragma once
#include <engine.hpp>
#include <enganim.hpp>



struct SGRX_ImageFP32 : SGRX_RefCounted
{
	FINLINE SGRX_ImageFP32() : m_width(0), m_height(0), m_depth(0), m_sides(0){}
	FINLINE SGRX_ImageFP32( int w, int h, int d, int s ){ Resize( w, h, d, s ); }
	FINLINE void Resize( int w, int h, int d, int s )
	{
		m_pixels.resize( w * h * d * s );
		m_width = w;
		m_height = h;
		m_depth = d;
		m_sides = s;
	}
	FINLINE size_t Size() const { return m_pixels.size(); }
	FINLINE size_t SideSize() const { return m_width * m_height * m_depth; }
	FINLINE int GetWidth() const { return m_width; }
	FINLINE int GetHeight() const { return m_height; }
	FINLINE int GetDepth() const { return m_depth; }
	FINLINE int GetSides() const { return m_sides; }
	FINLINE const Vec4* GetData() const { return m_pixels.data(); }
	FINLINE Vec4* GetData(){ return m_pixels.data(); }
	FINLINE const Vec4& operator [] ( size_t i ) const { return m_pixels[ i ]; }
	FINLINE Vec4& operator [] ( size_t i ){ return m_pixels[ i ]; }
	FINLINE const Vec4& Pixel( int x, int y, int z, int s ) const
	{
		return m_pixels[
			x +
			y * m_width +
			z * m_width * m_height +
			s * m_width * m_height * m_depth
		];
	}
	FINLINE Vec4& Pixel( int x, int y, int z, int s )
	{
		return m_pixels[
			x +
			y * m_width +
			z * m_width * m_height +
			s * m_width * m_height * m_depth
		];
	}
	FINLINE Vec4 GetClamped( int x, int y, int z, int s ) const
	{
		x = TMAX( 0, TMIN( m_width - 1, x ) );
		y = TMAX( 0, TMIN( m_height - 1, y ) );
		z = TMAX( 0, TMIN( m_depth - 1, z ) );
		s = TMAX( 0, TMIN( m_sides - 1, s ) );
		return m_pixels[
			x +
			y * m_width +
			z * m_width * m_height +
			s * m_width * m_height * m_depth
		];
	}
	FINLINE Vec4 GetLerpXY( float x, float y, int s ) const
	{
		int x0 = int( floor( x * m_width ) ) % m_width; if( x0 < 0 ) x0 += m_width;
		int x1 = int( ceil(  x * m_width ) ) % m_width; if( x1 < 0 ) x1 += m_width;
		int y0 = int( floor( y * m_height ) ) % m_height; if( y0 < 0 ) y0 += m_height;
		int y1 = int( ceil(  y * m_height ) ) % m_height; if( y1 < 0 ) y1 += m_height;
		float qx = fmodf( x * m_width, 1.0f );
		float qy = fmodf( y * m_height, 1.0f );
		Vec4 c00 = Pixel( x0, y0, 0, s ), c10 = Pixel( x1, y0, 0, s );
		Vec4 c01 = Pixel( x0, y1, 0, s ), c11 = Pixel( x1, y1, 0, s );
		return TLERP( TLERP( c00, c10, qx ), TLERP( c01, c11, qx ), qy );
	}
	FINLINE Vec4 GetLerpXYZ( float x, float y, float z, int s ) const
	{
		int x0 = int( floor( x * m_width ) ) % m_width; if( x0 < 0 ) x0 += m_width;
		int x1 = int( ceil(  x * m_width ) ) % m_width; if( x1 < 0 ) x1 += m_width;
		int y0 = int( floor( y * m_height ) ) % m_height; if( y0 < 0 ) y0 += m_height;
		int y1 = int( ceil(  y * m_height ) ) % m_height; if( y1 < 0 ) y1 += m_height;
		int z0 = int( floor( z * m_depth ) ) % m_depth; if( z0 < 0 ) z0 += m_depth;
		int z1 = int( ceil(  z * m_depth ) ) % m_depth; if( z1 < 0 ) z1 += m_depth;
		float qx = fmodf( x * m_width, 1.0f );
		float qy = fmodf( y * m_height, 1.0f );
		float qz = fmodf( z * m_depth, 1.0f );
		Vec4 c000 = Pixel( x0, y0, z0, s ), c100 = Pixel( x1, y0, z0, s );
		Vec4 c010 = Pixel( x0, y1, z0, s ), c110 = Pixel( x1, y1, z0, s );
		Vec4 c001 = Pixel( x0, y0, z1, s ), c101 = Pixel( x1, y0, z1, s );
		Vec4 c011 = Pixel( x0, y1, z1, s ), c111 = Pixel( x1, y1, z1, s );
		return TLERP(
			TLERP( TLERP( c000, c100, qx ), TLERP( c010, c110, qx ), qy ),
			TLERP( TLERP( c001, c101, qx ), TLERP( c011, c111, qx ), qy ),
			qz );
	}
	SGRX_ImageFP32* CreateUninitializedCopy()
	{
		return new SGRX_ImageFP32( m_width, m_height, m_depth, m_sides );
	}
	
	Array< Vec4 > m_pixels;
	int m_width;
	int m_height;
	int m_depth;
	int m_sides;
};
typedef Handle< SGRX_ImageFP32 > SGRX_IFP32Handle;
SGRX_IFP32Handle SGRX_ResizeImage( SGRX_ImageFP32* image, int width, int height );


enum SGRX_AssetImageFilterType
{
	SGRX_AIF_Unknown = 0,
	SGRX_AIF_Resize,
	SGRX_AIF_Rearrange,
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
	SGRX_ImageFilter_Resize() : width(256), height(256), depth(1), srgb(false){}
	SGRX_IF_CLONE( SGRX_ImageFilter_Resize );
	static bool IsType( SGRX_AssetImageFilterType ift ){ return ift == SGRX_AIF_Resize; }
	SGRX_AssetImageFilterType GetType() const { return SGRX_AIF_Resize; }
	const char* GetName() const { return "resize"; }
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs );
	
	int width;
	int height;
	int depth;
	bool srgb;
};

enum SGRX_ImgFltRearrange_Mode
{
	SGRX_IFR_SlicesToVolume,
	SGRX_IFR_TurnCubemapYZ,
	SGRX_IFR_BlenderCubemap,
	
	SGRX_IFR__COUNT,
};
struct SGRX_ImageFilter_Rearrange : SGRX_ImageFilter
{
	// Convert a 2D slice list into a volume texture
	SGRX_ImageFilter_Rearrange() : mode(SGRX_IFR_SlicesToVolume), width(16){}
	SGRX_IF_CLONE( SGRX_ImageFilter_Rearrange );
	static bool IsType( SGRX_AssetImageFilterType ift ){ return ift == SGRX_AIF_Rearrange; }
	SGRX_AssetImageFilterType GetType() const { return SGRX_AIF_Rearrange; }
	const char* GetName() const { return "rearrange"; }
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	SGRX_IFP32Handle Process( SGRX_ImageFP32* image, SGRX_ImageFilterState& ifs );
	
	SGRX_ImgFltRearrange_Mode mode;
	int width;
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
	SGRX_TOF_STX_DXT1,
	SGRX_TOF_STX_DXT5,
	
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

enum SGRX_AssetType
{
	SGRX_AT_Texture,
	SGRX_AT_Mesh,
	SGRX_AT_AnimBundle,
	SGRX_AT_File,
};

struct SGRX_TextureAsset;
struct SGRX_MeshAsset;
struct SGRX_AnimBundleAsset;
struct SGRX_FileAsset;

struct SGRX_Asset : SGRX_RefCounted
{
	SGRX_Asset( SGRX_AssetType t ) : assetType( t ){}
	virtual ~SGRX_Asset(){}
	virtual SGRX_Asset* Clone() = 0;
	virtual bool Parse( ConfigReader& cread ) = 0;
	virtual void Generate( String& out ) = 0;
	
	SGRX_TextureAsset* ToTexture() const { return assetType == SGRX_AT_Texture ? (SGRX_TextureAsset*) this : NULL; }
	SGRX_MeshAsset* ToMesh() const { return assetType == SGRX_AT_Mesh ? (SGRX_MeshAsset*) this : NULL; }
	SGRX_AnimBundleAsset* ToAnimBundle() const { return assetType == SGRX_AT_AnimBundle ? (SGRX_AnimBundleAsset*) this : NULL; }
	SGRX_FileAsset* ToFile() const { return assetType == SGRX_AT_File ? (SGRX_FileAsset*) this : NULL; }
	void CheckGUID(){ if( assetGUID.IsNull() ) assetGUID.SetGenerated(); }
	RCString GetPath() const
	{
		char bfr[ 128 ];
		char guidbfr[ GUID_STRING_LENGTH + 1 ];
		assetGUID.ToCharArray( guidbfr );
		sgrx_snprintf( bfr, 128, SGRXPATH_CACHE "/assets/%s", guidbfr );
		return bfr;
	}
	String GetUserPath() const
	{
		String out;
		out.push_back( '\0' );
		out.append( (const char*) assetGUID.bytes, 16 );
		return out;
	}
	RCString GetMappingString() const
	{
		const char* type = "?";
		switch( assetType )
		{
		case SGRX_AT_Texture: type = "T"; break;
		case SGRX_AT_Mesh: type = "M"; break;
		case SGRX_AT_AnimBundle: type = "A"; break;
		case SGRX_AT_File: type = "F"; break;
		}
		char bfr[ 500 ];
		char guidbfr[ GUID_STRING_LENGTH + 1 ];
		assetGUID.ToCharArray( guidbfr );
		sgrx_snprintf( bfr, 500, "%s %s:%s/%s\n",
			guidbfr, type,
			StackPath(outputCategory).str,
			StackPath(outputName).str );
		return bfr;
	}
	
	SGRX_AssetType assetType;
	
	SGRX_RevInfo ri;
	SGRX_GUID assetGUID;
	String outputCategory;
	String outputName;
};
typedef Handle< SGRX_Asset > AssetHandle;

struct SGRX_TextureAsset : SGRX_Asset
{
	SGRX_TextureAsset();
	SGRX_Asset* Clone();
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetFullName( String& out );
	void GetDesc( String& out );
	
	String sourceFile;
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

struct SGRX_MeshAsset : SGRX_Asset
{
	SGRX_MeshAsset() :
		SGRX_Asset( SGRX_AT_Mesh ),
		rotateY2Z(false),
		flipUVY(false),
		transform(false)
	{}
	SGRX_Asset* Clone();
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetFullName( String& out );
	void GetDesc( String& out );
	
	String sourceFile;
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

struct SGRX_AnimBundleAsset : SGRX_Asset
{
	SGRX_AnimBundleAsset() : SGRX_Asset( SGRX_AT_AnimBundle ){}
	SGRX_Asset* Clone();
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetFullName( String& out );
	void GetDesc( String& out );
	uint32_t LastSourceModTime();
	
	String bundlePrefix;
	String previewMesh;
	Array< SGRX_ABAnimation > anims;
	Array< SGRX_ABAnimSource > sources;
};

struct SGRX_FileAsset : SGRX_Asset
{
	SGRX_FileAsset() : SGRX_Asset( SGRX_AT_File ){}
	SGRX_Asset* Clone();
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetFullName( String& out );
	void GetDesc( String& out );
	
	String sourceFile;
};

struct SGRX_AssetScript
{
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	bool Load( const StringView& path );
	bool Save( const StringView& path );
	StringView GetCategoryPath( const StringView& name );
	SGRX_Asset* FindAsset( SGRX_AssetType type, StringView cat, StringView name );
	// - additional state
	bool LoadAssetInfo( const StringView& path );
	bool SaveAssetInfo( const StringView& path );
	bool LoadOutputInfo( const StringView& path );
	bool SaveOutputInfo( const StringView& path );
	
	HashTable< RCString, RCString > categories;
	Array< AssetHandle > assets;
};


#ifndef ASSIMP_IMPORTER_TYPE
#define ASSIMP_IMPORTER_TYPE void
#endif
#ifndef ASSIMP_SCENE_TYPE
#define ASSIMP_SCENE_TYPE void
#endif
#ifndef ASSIMP_NODE_TYPE
#define ASSIMP_NODE_TYPE void
#endif
#ifndef ASSIMP_MESH_TYPE
#define ASSIMP_MESH_TYPE void
#endif
enum SceneImportOptimizedFor
{
	SIOF_Meshes,
	SIOF_Anims,
};

struct AIMeshInfo
{
	const ASSIMP_NODE_TYPE* node;
	const ASSIMP_MESH_TYPE* mesh;
};
typedef HashTable< String, AIMeshInfo > AIMeshTable;

struct SGRX_Scene3D : SGRX_RefCounted
{
	SGRX_Scene3D( const StringView& path, SceneImportOptimizedFor siof = SIOF_Meshes );
	~SGRX_Scene3D();
	
	void GetMeshList( Array< String >& out );
	void GetAnimList( Array< String >& out );
	
	AIMeshInfo FindAssimpMesh( StringView name );
	
	String m_path;
	ASSIMP_IMPORTER_TYPE* m_imp;
	const ASSIMP_SCENE_TYPE* m_scene;
	AIMeshTable m_aiMeshTable;
	MeshHandle m_mesh;
	SGRX_AnimBundle m_animBundle;
};
typedef Handle< SGRX_Scene3D > ImpScene3DHandle;

SGRX_IFP32Handle SGRX_ProcessTextureAsset( const SGRX_TextureAsset* TA );
TextureHandle SGRX_FP32ToTexture( SGRX_ImageFP32* image, const SGRX_TextureAsset* TA );
MeshHandle SGRX_ProcessMeshAsset( const SGRX_AssetScript* AS, const SGRX_MeshAsset& MA );
AnimHandle SGRX_ProcessSingleAnim( const SGRX_AnimBundleAsset& ABA, int i );
void SGRX_ProcessAssets( SGRX_AssetScript& script, bool force = false );
void SGRX_RemoveAssets( SGRX_AssetScript& script, SGRX_AssetType type );

