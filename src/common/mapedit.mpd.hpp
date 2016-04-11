
#pragma once
#ifndef MPD_API_HPP
#  define MPD_API_HPP "mpd_api.hpp"
#endif
#include MPD_API_HPP

struct Vec2_MPD : struct_MPD<Vec2_MPD, Vec2>
{
	static const char* name(){ return "Vec2"; }
	static const Vec2_MPD* inst(){ static const Vec2_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 2; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( Vec2 const*, int );
	static bool setprop( Vec2*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( Vec2 const*, const mpd_Variant& );
	static bool setindex( Vec2*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(Vec2) );
};

MPD_DUMPDATA_WRAPPER(Vec2, Vec2);
template<> struct mpd_MetaType<Vec2 > : Vec2_MPD {};
template<> struct mpd_MetaType<Vec2 const> : Vec2_MPD {};
template<> struct mpd_MetaType<Vec2_MPD> : Vec2_MPD {};

struct Vec3_MPD : struct_MPD<Vec3_MPD, Vec3>
{
	static const char* name(){ return "Vec3"; }
	static const Vec3_MPD* inst(){ static const Vec3_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 3; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( Vec3 const*, int );
	static bool setprop( Vec3*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( Vec3 const*, const mpd_Variant& );
	static bool setindex( Vec3*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(Vec3) );
};

MPD_DUMPDATA_WRAPPER(Vec3, Vec3);
template<> struct mpd_MetaType<Vec3 > : Vec3_MPD {};
template<> struct mpd_MetaType<Vec3 const> : Vec3_MPD {};
template<> struct mpd_MetaType<Vec3_MPD> : Vec3_MPD {};

struct String_MPD : struct_MPD<String_MPD, String>
{
	static const char* name(){ return "String"; }
	static const String_MPD* inst(){ static const String_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 2; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( String const*, int );
	static bool setprop( String*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ static const mpd_TypeInfo types[] = { { "int32_t", mpdt_Int32, 0 }, { "int8_t", mpdt_Int8, 0 } }; return types; }
	static mpd_Variant getindex( String const*, const mpd_Variant& );
	static bool setindex( String*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(String) );
};

MPD_DUMPDATA_WRAPPER(String, String);
template<> struct mpd_MetaType<String > : String_MPD {};
template<> struct mpd_MetaType<String const> : String_MPD {};
template<> struct mpd_MetaType<String_MPD> : String_MPD {};

struct EdWorldLightingInfo_MPD : struct_MPD<EdWorldLightingInfo_MPD, EdWorldLightingInfo>
{
	static const char* name(){ return "EdWorldLightingInfo"; }
	static const EdWorldLightingInfo_MPD* inst(){ static const EdWorldLightingInfo_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 17; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( EdWorldLightingInfo const*, int );
	static bool setprop( EdWorldLightingInfo*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( EdWorldLightingInfo const*, const mpd_Variant& );
	static bool setindex( EdWorldLightingInfo*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(EdWorldLightingInfo) );
};

MPD_DUMPDATA_WRAPPER(EdWorldLightingInfo, EdWorldLightingInfo);
template<> struct mpd_MetaType<EdWorldLightingInfo > : EdWorldLightingInfo_MPD {};
template<> struct mpd_MetaType<EdWorldLightingInfo const> : EdWorldLightingInfo_MPD {};
template<> struct mpd_MetaType<EdWorldLightingInfo_MPD> : EdWorldLightingInfo_MPD {};


const mpd_KeyValue* Vec2_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* Vec2_MPD::props()
{
	static const mpd_KeyValue x_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue y_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_PropInfo data[] =
	{
		{ "x", 1, { "float", mpdt_Float32, 0 }, x_metadata },
		{ "y", 1, { "float", mpdt_Float32, 0 }, y_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant Vec2_MPD::getprop( Vec2 const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (float const&) obj->x;
	case 1: return (float const&) obj->y;
	default: return mpd_Variant();
	}
}
bool Vec2_MPD::setprop( Vec2* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->x = mpd_var_get<float >(val); return true;
	case 1: obj->y = mpd_var_get<float >(val); return true;
	default: return false;
	}
}
mpd_Variant Vec2_MPD::getindex( Vec2 const*, const mpd_Variant& ){ return mpd_Variant(); }
bool Vec2_MPD::setindex( Vec2*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* Vec2_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void Vec2_MPD::dump( MPD_STATICDUMP_ARGS(Vec2) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct Vec2\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( float, x, pdata->x );
		MPD_DUMP_PROP( float, y, pdata->y );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* Vec3_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* Vec3_MPD::props()
{
	static const mpd_KeyValue x_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue y_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue z_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_PropInfo data[] =
	{
		{ "x", 1, { "float", mpdt_Float32, 0 }, x_metadata },
		{ "y", 1, { "float", mpdt_Float32, 0 }, y_metadata },
		{ "z", 1, { "float", mpdt_Float32, 0 }, z_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant Vec3_MPD::getprop( Vec3 const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (float const&) obj->x;
	case 1: return (float const&) obj->y;
	case 2: return (float const&) obj->z;
	default: return mpd_Variant();
	}
}
bool Vec3_MPD::setprop( Vec3* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->x = mpd_var_get<float >(val); return true;
	case 1: obj->y = mpd_var_get<float >(val); return true;
	case 2: obj->z = mpd_var_get<float >(val); return true;
	default: return false;
	}
}
mpd_Variant Vec3_MPD::getindex( Vec3 const*, const mpd_Variant& ){ return mpd_Variant(); }
bool Vec3_MPD::setindex( Vec3*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* Vec3_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void Vec3_MPD::dump( MPD_STATICDUMP_ARGS(Vec3) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct Vec3\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( float, x, pdata->x );
		MPD_DUMP_PROP( float, y, pdata->y );
		MPD_DUMP_PROP( float, z, pdata->z );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* String_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* String_MPD::props()
{
	static const mpd_KeyValue data_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue size_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_PropInfo data[] =
	{
		{ "data", 4, { "mpd_StringView", mpdt_ConstString, 0 }, data_metadata },
		{ "size", 4, { "int32_t", mpdt_Int32, 0 }, size_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant String_MPD::getprop( String const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (mpd_StringView const&) mpd_StringView::create(obj->data(), obj->size());
	case 1: return (int32_t const&) obj->size();
	default: return mpd_Variant();
	}
}
bool String_MPD::setprop( String* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->assign(val.get_stringview().str, val.get_stringview().size); return true;
	case 1: obj->resize(val.get_int32()); return true;
	default: return false;
	}
}
mpd_Variant String_MPD::getindex( String const* obj, const mpd_Variant& key )
{
	return (int8_t const&)(*(obj))[mpd_var_get<int32_t >(key)];
}
bool String_MPD::setindex( String* obj, const mpd_Variant& key, const mpd_Variant& val )
{
	(*(obj))[mpd_var_get<int32_t >(key)] = mpd_var_get<int8_t >(val);
	return true;
}
const mpd_EnumValue* String_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void String_MPD::dump( MPD_STATICDUMP_ARGS(String) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct String\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( mpd_StringView, data, mpd_StringView::create(pdata->data(), pdata->size()) );
		MPD_DUMP_PROP( int32_t, size, pdata->size() );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* EdWorldLightingInfo_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Lighting info", 13, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* EdWorldLightingInfo_MPD::props()
{
	static const mpd_KeyValue ambientColor_metadata[] =
	{
		{ "label", 5, "Ambient color", 13, 0, (float) 0 },
		{ "min", 3, """\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""", 12, 0, (float) 0 },
		{ "max", 3, """\x00""""\x00""""\x80""?""\x00""""\x00""""\x80""?""\x00""""\x00""""\xc8""B", 12, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue dirLightDir_metadata[] =
	{
		{ "label", 5, "Dir.light direction (dX,dY)", 27, 0, (float) 0 },
		{ "min", 3, """\x00""""\x00""""\x00""""\xc6""""\x00""""\x00""""\x00""""\xc6""", 8, 0, (float) 0 },
		{ "max", 3, """\x00""""\x00""""\x00""F""\x00""""\x00""""\x00""F", 8, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue dirLightColor_metadata[] =
	{
		{ "label", 5, "Dir.light color (HSV)", 21, 0, (float) 0 },
		{ "min", 3, """\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""", 12, 0, (float) 0 },
		{ "max", 3, """\x00""""\x00""""\x80""?""\x00""""\x00""""\x80""?""\x00""""\x00""""\xc8""B", 12, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue dirLightDivergence_metadata[] =
	{
		{ "label", 5, "Dir.light divergence", 20, 0, (float) 0 },
		{ "min", 3, "0", 1, 0, (float) 0 },
		{ "max", 3, "180", 3, 180, (float) 180 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue dirLightNumSamples_metadata[] =
	{
		{ "label", 5, "Dir.light sample count", 22, 0, (float) 0 },
		{ "min", 3, "0", 1, 0, (float) 0 },
		{ "max", 3, "256", 3, 256, (float) 256 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue lightmapClearColor_metadata[] =
	{
		{ "label", 5, "Lightmap clear color (HSV)", 26, 0, (float) 0 },
		{ "min", 3, """\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""", 12, 0, (float) 0 },
		{ "max", 3, """\x00""""\x00""""\x80""?""\x00""""\x00""""\x80""?""\x00""""\x00""""\xc8""B", 12, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue lightmapDetail_metadata[] =
	{
		{ "label", 5, "Lightmap detail", 15, 0, (float) 0 },
		{ "min", 3, "0.01", 4, 0, (float) 0.01 },
		{ "max", 3, "16", 2, 16, (float) 16 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue lightmapBlurSize_metadata[] =
	{
		{ "label", 5, "Lightmap blur size", 18, 0, (float) 0 },
		{ "min", 3, "0", 1, 0, (float) 0 },
		{ "max", 3, "10", 2, 10, (float) 10 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue aoDistance_metadata[] =
	{
		{ "label", 5, "AO distance", 11, 0, (float) 0 },
		{ "min", 3, "0", 1, 0, (float) 0 },
		{ "max", 3, "100", 3, 100, (float) 100 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue aoMultiplier_metadata[] =
	{
		{ "label", 5, "AO multiplier", 13, 0, (float) 0 },
		{ "min", 3, "0", 1, 0, (float) 0 },
		{ "max", 3, "2", 1, 2, (float) 2 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue aoFalloff_metadata[] =
	{
		{ "label", 5, "AO falloff", 10, 0, (float) 0 },
		{ "min", 3, "0.01", 4, 0, (float) 0.01 },
		{ "max", 3, "100", 3, 100, (float) 100 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue aoEffect_metadata[] =
	{
		{ "label", 5, "AO effect", 9, 0, (float) 0 },
		{ "min", 3, "-1", 2, -1, (float) -1 },
		{ "max", 3, "1", 1, 1, (float) 1 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue aoColor_metadata[] =
	{
		{ "label", 5, "AO color (HSV)", 14, 0, (float) 0 },
		{ "min", 3, """\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""""\x00""", 12, 0, (float) 0 },
		{ "max", 3, """\x00""""\x00""""\x80""?""\x00""""\x00""""\x80""?""\x00""""\x00""""\xc8""B", 12, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue aoNumSamples_metadata[] =
	{
		{ "label", 5, "AO sample count", 15, 0, (float) 0 },
		{ "min", 3, "0", 1, 0, (float) 0 },
		{ "max", 3, "256", 3, 256, (float) 256 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue sampleDensity_metadata[] =
	{
		{ "label", 5, "Sample density", 14, 0, (float) 0 },
		{ "min", 3, "0.01", 4, 0, (float) 0.01 },
		{ "max", 3, "100", 3, 100, (float) 100 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue skyboxTexture_metadata[] =
	{
		{ "label", 5, "Skybox texture", 14, 0, (float) 0 },
		{ "edit", 4, "texture", 7, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue lutTexture_metadata[] =
	{
		{ "label", 5, "Default post-process cLUT", 25, 0, (float) 0 },
		{ "edit", 4, "texture", 7, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "ambientColor", 12, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, ambientColor_metadata },
		{ "dirLightDir", 11, { "Vec2", mpdt_Struct, Vec2_MPD::inst() }, dirLightDir_metadata },
		{ "dirLightColor", 13, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, dirLightColor_metadata },
		{ "dirLightDivergence", 18, { "float", mpdt_Float32, 0 }, dirLightDivergence_metadata },
		{ "dirLightNumSamples", 18, { "int32_t", mpdt_Int32, 0 }, dirLightNumSamples_metadata },
		{ "lightmapClearColor", 18, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, lightmapClearColor_metadata },
		{ "lightmapDetail", 14, { "float", mpdt_Float32, 0 }, lightmapDetail_metadata },
		{ "lightmapBlurSize", 16, { "float", mpdt_Float32, 0 }, lightmapBlurSize_metadata },
		{ "aoDistance", 10, { "float", mpdt_Float32, 0 }, aoDistance_metadata },
		{ "aoMultiplier", 12, { "float", mpdt_Float32, 0 }, aoMultiplier_metadata },
		{ "aoFalloff", 9, { "float", mpdt_Float32, 0 }, aoFalloff_metadata },
		{ "aoEffect", 8, { "float", mpdt_Float32, 0 }, aoEffect_metadata },
		{ "aoColor", 7, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, aoColor_metadata },
		{ "aoNumSamples", 12, { "int32_t", mpdt_Int32, 0 }, aoNumSamples_metadata },
		{ "sampleDensity", 13, { "float", mpdt_Float32, 0 }, sampleDensity_metadata },
		{ "skyboxTexture", 13, { "String", mpdt_Struct, String_MPD::inst() }, skyboxTexture_metadata },
		{ "lutTexture", 10, { "String", mpdt_Struct, String_MPD::inst() }, lutTexture_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant EdWorldLightingInfo_MPD::getprop( EdWorldLightingInfo const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (Vec3 const&) obj->ambientColor;
	case 1: return (Vec2 const&) obj->dirLightDir;
	case 2: return (Vec3 const&) obj->dirLightColor;
	case 3: return (float const&) obj->dirLightDivergence;
	case 4: return (int32_t const&) obj->dirLightNumSamples;
	case 5: return (Vec3 const&) obj->lightmapClearColor;
	case 6: return (float const&) obj->lightmapDetail;
	case 7: return (float const&) obj->lightmapBlurSize;
	case 8: return (float const&) obj->aoDistance;
	case 9: return (float const&) obj->aoMultiplier;
	case 10: return (float const&) obj->aoFalloff;
	case 11: return (float const&) obj->aoEffect;
	case 12: return (Vec3 const&) obj->aoColor;
	case 13: return (int32_t const&) obj->aoNumSamples;
	case 14: return (float const&) obj->sampleDensity;
	case 15: return (String const&) obj->skyboxTexture;
	case 16: return (String const&) obj->lutTexture;
	default: return mpd_Variant();
	}
}
bool EdWorldLightingInfo_MPD::setprop( EdWorldLightingInfo* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->ambientColor = mpd_var_get<Vec3 >(val); return true;
	case 1: obj->dirLightDir = mpd_var_get<Vec2 >(val); return true;
	case 2: obj->dirLightColor = mpd_var_get<Vec3 >(val); return true;
	case 3: obj->dirLightDivergence = mpd_var_get<float >(val); return true;
	case 4: obj->dirLightNumSamples = mpd_var_get<int32_t >(val); return true;
	case 5: obj->lightmapClearColor = mpd_var_get<Vec3 >(val); return true;
	case 6: obj->lightmapDetail = mpd_var_get<float >(val); return true;
	case 7: obj->lightmapBlurSize = mpd_var_get<float >(val); return true;
	case 8: obj->aoDistance = mpd_var_get<float >(val); return true;
	case 9: obj->aoMultiplier = mpd_var_get<float >(val); return true;
	case 10: obj->aoFalloff = mpd_var_get<float >(val); return true;
	case 11: obj->aoEffect = mpd_var_get<float >(val); return true;
	case 12: obj->aoColor = mpd_var_get<Vec3 >(val); return true;
	case 13: obj->aoNumSamples = mpd_var_get<int32_t >(val); return true;
	case 14: obj->sampleDensity = mpd_var_get<float >(val); return true;
	case 15: obj->skyboxTexture = mpd_var_get<String >(val); return true;
	case 16: obj->lutTexture = mpd_var_get<String >(val); return true;
	default: return false;
	}
}
mpd_Variant EdWorldLightingInfo_MPD::getindex( EdWorldLightingInfo const*, const mpd_Variant& ){ return mpd_Variant(); }
bool EdWorldLightingInfo_MPD::setindex( EdWorldLightingInfo*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* EdWorldLightingInfo_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void EdWorldLightingInfo_MPD::dump( MPD_STATICDUMP_ARGS(EdWorldLightingInfo) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct EdWorldLightingInfo\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( Vec3, ambientColor, pdata->ambientColor );
		MPD_DUMP_PROP( Vec2, dirLightDir, pdata->dirLightDir );
		MPD_DUMP_PROP( Vec3, dirLightColor, pdata->dirLightColor );
		MPD_DUMP_PROP( float, dirLightDivergence, pdata->dirLightDivergence );
		MPD_DUMP_PROP( int32_t, dirLightNumSamples, pdata->dirLightNumSamples );
		MPD_DUMP_PROP( Vec3, lightmapClearColor, pdata->lightmapClearColor );
		MPD_DUMP_PROP( float, lightmapDetail, pdata->lightmapDetail );
		MPD_DUMP_PROP( float, lightmapBlurSize, pdata->lightmapBlurSize );
		MPD_DUMP_PROP( float, aoDistance, pdata->aoDistance );
		MPD_DUMP_PROP( float, aoMultiplier, pdata->aoMultiplier );
		MPD_DUMP_PROP( float, aoFalloff, pdata->aoFalloff );
		MPD_DUMP_PROP( float, aoEffect, pdata->aoEffect );
		MPD_DUMP_PROP( Vec3, aoColor, pdata->aoColor );
		MPD_DUMP_PROP( int32_t, aoNumSamples, pdata->aoNumSamples );
		MPD_DUMP_PROP( float, sampleDensity, pdata->sampleDensity );
		MPD_DUMP_PROP( String, skyboxTexture, pdata->skyboxTexture );
		MPD_DUMP_PROP( String, lutTexture, pdata->lutTexture );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

