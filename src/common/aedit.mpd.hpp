
#pragma once
#ifndef MPD_API_HPP
#  define MPD_API_HPP "mpd_api.hpp"
#endif
#include MPD_API_HPP

struct Vec2_MPD : struct_MPD<Vec2_MPD, Vec2 >
{
	enum PIDS
	{
		PID_x,
		PID_y,
	};

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

struct Vec3_MPD : struct_MPD<Vec3_MPD, Vec3 >
{
	enum PIDS
	{
		PID_x,
		PID_y,
		PID_z,
	};

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

struct String_MPD : struct_MPD<String_MPD, String >
{
	enum PIDS
	{
		PID_data,
		PID_size,
	};

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

struct Vec3Array_MPD : struct_MPD<Vec3Array_MPD, Array<Vec3> >
{
	enum PIDS
	{
		PID_size,
	};

	static const char* name(){ return "Vec3Array"; }
	static const Vec3Array_MPD* inst(){ static const Vec3Array_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 1; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( Array<Vec3> const*, int );
	static bool setprop( Array<Vec3>*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ static const mpd_TypeInfo types[] = { { "int32_t", mpdt_Int32, 0 }, { "Vec3", mpdt_Struct, Vec3_MPD::inst() } }; return types; }
	static mpd_Variant getindex( Array<Vec3> const*, const mpd_Variant& );
	static bool setindex( Array<Vec3>*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(Array<Vec3>) );
};

MPD_DUMPDATA_WRAPPER(Vec3Array, Array<Vec3>);
template<> struct mpd_MetaType<Array<Vec3> > : Vec3Array_MPD {};
template<> struct mpd_MetaType<Array<Vec3> const> : Vec3Array_MPD {};
template<> struct mpd_MetaType<Vec3Array_MPD> : Vec3Array_MPD {};

struct SGRX_TextureOutputFormat_MPD : struct_MPD<SGRX_TextureOutputFormat_MPD, SGRX_TextureOutputFormat >
{
	static const char* name(){ return "SGRX_TextureOutputFormat"; }
	static const SGRX_TextureOutputFormat_MPD* inst(){ static const SGRX_TextureOutputFormat_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 0; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( SGRX_TextureOutputFormat const*, int );
	static bool setprop( SGRX_TextureOutputFormat*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( SGRX_TextureOutputFormat const*, const mpd_Variant& );
	static bool setindex( SGRX_TextureOutputFormat*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 2; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(SGRX_TextureOutputFormat) );
};

MPD_DUMPDATA_WRAPPER(SGRX_TextureOutputFormat, SGRX_TextureOutputFormat);
template<> struct mpd_MetaType<SGRX_TextureOutputFormat > : SGRX_TextureOutputFormat_MPD {};
template<> struct mpd_MetaType<SGRX_TextureOutputFormat const> : SGRX_TextureOutputFormat_MPD {};
template<> struct mpd_MetaType<SGRX_TextureOutputFormat_MPD> : SGRX_TextureOutputFormat_MPD {};

struct SGRX_TextureAsset_MPD : struct_MPD<SGRX_TextureAsset_MPD, SGRX_TextureAsset >
{
	enum PIDS
	{
		PID_sourceFile,
		PID_outputCategory,
		PID_outputName,
		PID_outputType,
		PID_isSRGB,
		PID_mips,
		PID_lerp,
		PID_clampx,
		PID_clampy,
	};

	static const char* name(){ return "SGRX_TextureAsset"; }
	static const SGRX_TextureAsset_MPD* inst(){ static const SGRX_TextureAsset_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 9; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( SGRX_TextureAsset const*, int );
	static bool setprop( SGRX_TextureAsset*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( SGRX_TextureAsset const*, const mpd_Variant& );
	static bool setindex( SGRX_TextureAsset*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(SGRX_TextureAsset) );
};

MPD_DUMPDATA_WRAPPER(SGRX_TextureAsset, SGRX_TextureAsset);
template<> struct mpd_MetaType<SGRX_TextureAsset > : SGRX_TextureAsset_MPD {};
template<> struct mpd_MetaType<SGRX_TextureAsset const> : SGRX_TextureAsset_MPD {};
template<> struct mpd_MetaType<SGRX_TextureAsset_MPD> : SGRX_TextureAsset_MPD {};


#ifdef MPD_IMPL
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

const mpd_KeyValue* Vec3Array_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* Vec3Array_MPD::props()
{
	static const mpd_KeyValue size_metadata[] =
	{
		{ "label", 5, "Size", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "size", 4, { "int32_t", mpdt_Int32, 0 }, size_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant Vec3Array_MPD::getprop( Array<Vec3> const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (int32_t const&) obj->size();
	default: return mpd_Variant();
	}
}
bool Vec3Array_MPD::setprop( Array<Vec3>* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->resize(val.get_int32()); return true;
	default: return false;
	}
}
mpd_Variant Vec3Array_MPD::getindex( Array<Vec3> const* obj, const mpd_Variant& key )
{
	return (Vec3 const&)(*(obj))[mpd_var_get<int32_t >(key)];
}
bool Vec3Array_MPD::setindex( Array<Vec3>* obj, const mpd_Variant& key, const mpd_Variant& val )
{
	(*(obj))[mpd_var_get<int32_t >(key)] = mpd_var_get<Vec3 >(val);
	return true;
}
const mpd_EnumValue* Vec3Array_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void Vec3Array_MPD::dump( MPD_STATICDUMP_ARGS(Array<Vec3>) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct Vec3Array\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( int32_t, size, pdata->size() );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* SGRX_TextureOutputFormat_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Texture output format", 21, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* SGRX_TextureOutputFormat_MPD::props(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_PropInfo none = { 0, 0, { 0, mpdt_None, 0 }, &kvnone }; return &none; }
mpd_Variant SGRX_TextureOutputFormat_MPD::getprop( SGRX_TextureOutputFormat const*, int ){ return mpd_Variant(); }
bool SGRX_TextureOutputFormat_MPD::setprop( SGRX_TextureOutputFormat*, int, const mpd_Variant& ){ return false; }
mpd_Variant SGRX_TextureOutputFormat_MPD::getindex( SGRX_TextureOutputFormat const*, const mpd_Variant& ){ return mpd_Variant(); }
bool SGRX_TextureOutputFormat_MPD::setindex( SGRX_TextureOutputFormat*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* SGRX_TextureOutputFormat_MPD::values()
{
	static const mpd_KeyValue SGRX_TOF_PNG_RGBA32_metadata[] =
	{
		{ "label", 5, "PNG/RGBA32", 10, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue SGRX_TOF_STX_RGBA32_metadata[] =
	{
		{ "label", 5, "STX/RGBA32", 10, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_EnumValue data[] =
	{
		{ "SGRX_TOF_PNG_RGBA32", 19, SGRX_TOF_PNG_RGBA32, SGRX_TOF_PNG_RGBA32_metadata },
		{ "SGRX_TOF_STX_RGBA32", 19, SGRX_TOF_STX_RGBA32, SGRX_TOF_STX_RGBA32_metadata },
		{ 0, 0, 0, 0 },
	};
	return data;
}
void SGRX_TextureOutputFormat_MPD::dump( MPD_STATICDUMP_ARGS(SGRX_TextureOutputFormat) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "SGRX_TextureOutputFormat::%s (%d)", value2name( *pdata ), (int)*pdata );
}

const mpd_KeyValue* SGRX_TextureAsset_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Texture asset", 13, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* SGRX_TextureAsset_MPD::props()
{
	static const mpd_KeyValue sourceFile_metadata[] =
	{
		{ "label", 5, "Source file", 11, 0, (float) 0 },
		{ "edit", 4, "file", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue outputCategory_metadata[] =
	{
		{ "label", 5, "Output category", 15, 0, (float) 0 },
		{ "edit", 4, "category", 8, 0, (float) 0 },
		{ "edit_requestReload", 18, "true", 4, 1, (float) 1 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue outputName_metadata[] =
	{
		{ "label", 5, "Output name", 11, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue outputType_metadata[] =
	{
		{ "label", 5, "Output type", 11, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue isSRGB_metadata[] =
	{
		{ "label", 5, "Is SGRB?", 8, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue mips_metadata[] =
	{
		{ "label", 5, "Generate mipmaps?", 17, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue lerp_metadata[] =
	{
		{ "label", 5, "Use linear interpolation?", 25, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue clampx_metadata[] =
	{
		{ "label", 5, "Clamp X", 7, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue clampy_metadata[] =
	{
		{ "label", 5, "Clamp Y", 7, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "sourceFile", 10, { "String", mpdt_Struct, String_MPD::inst() }, sourceFile_metadata },
		{ "outputCategory", 14, { "String", mpdt_Struct, String_MPD::inst() }, outputCategory_metadata },
		{ "outputName", 10, { "String", mpdt_Struct, String_MPD::inst() }, outputName_metadata },
		{ "outputType", 10, { "SGRX_TextureOutputFormat", mpdt_Enum, SGRX_TextureOutputFormat_MPD::inst() }, outputType_metadata },
		{ "isSRGB", 6, { "bool", mpdt_Bool, 0 }, isSRGB_metadata },
		{ "mips", 4, { "bool", mpdt_Bool, 0 }, mips_metadata },
		{ "lerp", 4, { "bool", mpdt_Bool, 0 }, lerp_metadata },
		{ "clampx", 6, { "bool", mpdt_Bool, 0 }, clampx_metadata },
		{ "clampy", 6, { "bool", mpdt_Bool, 0 }, clampy_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant SGRX_TextureAsset_MPD::getprop( SGRX_TextureAsset const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (String const&) obj->sourceFile;
	case 1: return (String const&) obj->outputCategory;
	case 2: return (String const&) obj->outputName;
	case 3: return mpd_Variant( (SGRX_TextureOutputFormat const&) obj->outputType, mpd_Variant::Enum );
	case 4: return (bool const&) obj->isSRGB;
	case 5: return (bool const&) obj->mips;
	case 6: return (bool const&) obj->lerp;
	case 7: return (bool const&) obj->clampx;
	case 8: return (bool const&) obj->clampy;
	default: return mpd_Variant();
	}
}
bool SGRX_TextureAsset_MPD::setprop( SGRX_TextureAsset* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->sourceFile = mpd_var_get<String >(val); return true;
	case 1: obj->outputCategory = mpd_var_get<String >(val); return true;
	case 2: obj->outputName = mpd_var_get<String >(val); return true;
	case 3: obj->outputType = mpd_var_get<SGRX_TextureOutputFormat >(val); return true;
	case 4: obj->isSRGB = mpd_var_get<bool >(val); return true;
	case 5: obj->mips = mpd_var_get<bool >(val); return true;
	case 6: obj->lerp = mpd_var_get<bool >(val); return true;
	case 7: obj->clampx = mpd_var_get<bool >(val); return true;
	case 8: obj->clampy = mpd_var_get<bool >(val); return true;
	default: return false;
	}
}
mpd_Variant SGRX_TextureAsset_MPD::getindex( SGRX_TextureAsset const*, const mpd_Variant& ){ return mpd_Variant(); }
bool SGRX_TextureAsset_MPD::setindex( SGRX_TextureAsset*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* SGRX_TextureAsset_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void SGRX_TextureAsset_MPD::dump( MPD_STATICDUMP_ARGS(SGRX_TextureAsset) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct SGRX_TextureAsset\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( String, sourceFile, pdata->sourceFile );
		MPD_DUMP_PROP( String, outputCategory, pdata->outputCategory );
		MPD_DUMP_PROP( String, outputName, pdata->outputName );
		MPD_DUMP_PROP( SGRX_TextureOutputFormat, outputType, pdata->outputType );
		MPD_DUMP_PROP( bool, isSRGB, pdata->isSRGB );
		MPD_DUMP_PROP( bool, mips, pdata->mips );
		MPD_DUMP_PROP( bool, lerp, pdata->lerp );
		MPD_DUMP_PROP( bool, clampx, pdata->clampx );
		MPD_DUMP_PROP( bool, clampy, pdata->clampy );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

#endif
