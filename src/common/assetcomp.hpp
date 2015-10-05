

#pragma once
#include <engine.hpp>



enum SGRX_AssetImageFilterType
{
	SGRX_AIF_Unknown = 0,
	SGRX_AIF_Resize,
	SGRX_AIF_Sharpen,
	SGRX_AIF_ToLinear,
	SGRX_AIF_FromLinear,
	
	SGRX_AIF__COUNT,
};

const char* SGRX_AssetImgFilterType_ToString( SGRX_AssetImageFilterType aift );
SGRX_AssetImageFilterType SGRX_AssetImgFilterType_FromString( const StringView& sv );

struct SGRX_ImageFilter : SGRX_RefCounted
{
	virtual SGRX_AssetImageFilterType GetType() const = 0;
	virtual const char* GetName() const = 0;
	virtual bool Parse( ConfigReader& cread ) = 0;
	virtual void Generate( String& out ) = 0;
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
	
	int width;
	int height;
};

struct SGRX_ImageFilter_Sharpen : SGRX_ImageFilter
{
	static bool IsType( SGRX_AssetImageFilterType ift ){ return ift == SGRX_AIF_Sharpen; }
	SGRX_AssetImageFilterType GetType() const { return SGRX_AIF_Sharpen; }
	const char* GetName() const { return "sharpen"; }
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	
	float factor;
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
	bool Parse( ConfigReader& cread );
	void Generate( String& out );
	void GetDesc( String& out );
	
	String sourceFile;
	String outputCategory;
	String outputName;
	SGRX_TextureOutputFormat outputType;
	bool isSRGB;
	
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

