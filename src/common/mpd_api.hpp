

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS 1
#endif
#ifdef _MSC_VER
#  include <stdint.h>
#  if _MSC_VER >= 1700
#    include <inttypes.h>
#  else
#    define PRId64 "lld"
#  endif
#else
#  include <inttypes.h>
#endif


enum mpd_Type
{
	mpdt_None,
	mpdt_Struct,
	mpdt_Pointer,
	mpdt_Enum,
	mpdt_ConstString,
	mpdt_Int8,
	mpdt_Int16,
	mpdt_Int32,
	mpdt_Int64,
	mpdt_UInt8,
	mpdt_UInt16,
	mpdt_UInt32,
	mpdt_UInt64,
	mpdt_Float32,
	mpdt_Float64,
};

struct mpd_StringView
{
	const char* str;
	size_t size;
	
	static mpd_StringView create( const char* str ){ mpd_StringView out = { str, strlen( str ) }; return out; }
	static mpd_StringView create( const char* str, size_t size ){ mpd_StringView out = { str, size }; return out; }
};

//
// --- DUMP INFO ---
//
inline void __mpd_reprint( const char* text, int count )
{
	size_t sz = strlen( text );
	while( count --> 0 )
		fwrite( text, 1, sz, stdout );
}

#define MPD_STATICDUMP_ARGS(ty) ty const* pdata, int limit, int level
#define MPD_DUMPLEV( add ) __mpd_reprint( "\t", level + add )
#define MPD_DUMP_PROP( ty, name, getter ) __mpd_reprint( "\t", level + 1 ); printf( "%s = ", #name ); mpd_DumpData<ty>( getter, limit, level + 1 ); printf( "\n" );
#define MPD_DUMPDATA_ARGS(ty) ty const& data, int limit, int level
#define MPD_DUMPDATA_WRAPPER(ty,nty) template<> inline void mpd_DumpData<nty>( MPD_DUMPDATA_ARGS(nty) ){ ty##_MPD::dump( &data, limit, level ); }
#define MPD_DUMPDATA_USEARGS (void) data; (void) limit; (void) level;
#define MPD_DUMPDATA_USESTATICARGS (void) pdata; (void) limit; (void) level;
template< class T > void mpd_DumpData( T const& data, int limit = 5, int level = 0 ){ printf( "<unknown>" ); }
template<> inline void mpd_DumpData<int8_t>( MPD_DUMPDATA_ARGS(int8_t) ){ MPD_DUMPDATA_USEARGS; printf( "int8 (%d)", (int) data ); }
template<> inline void mpd_DumpData<int16_t>( MPD_DUMPDATA_ARGS(int16_t) ){ MPD_DUMPDATA_USEARGS; printf( "int16 (%d)", (int) data ); }
template<> inline void mpd_DumpData<int32_t>( MPD_DUMPDATA_ARGS(int32_t) ){ MPD_DUMPDATA_USEARGS; printf( "int32 (%d)", (int) data ); }
template<> inline void mpd_DumpData<int64_t>( MPD_DUMPDATA_ARGS(int64_t) ){ MPD_DUMPDATA_USEARGS; printf( "int64 (%d)", (int) data ); }
template<> inline void mpd_DumpData<uint8_t>( MPD_DUMPDATA_ARGS(uint8_t) ){ MPD_DUMPDATA_USEARGS; printf( "uint8 (%d)", (int) data ); }
template<> inline void mpd_DumpData<uint16_t>( MPD_DUMPDATA_ARGS(uint16_t) ){ MPD_DUMPDATA_USEARGS; printf( "uint16 (%d)", (int) data ); }
template<> inline void mpd_DumpData<uint32_t>( MPD_DUMPDATA_ARGS(uint32_t) ){ MPD_DUMPDATA_USEARGS; printf( "uint32 (%d)", (int) data ); }
template<> inline void mpd_DumpData<uint64_t>( MPD_DUMPDATA_ARGS(uint64_t) ){ MPD_DUMPDATA_USEARGS; printf( "uint64 (%d)", (int) data ); }
template<> inline void mpd_DumpData<float>( MPD_DUMPDATA_ARGS(float) ){ MPD_DUMPDATA_USEARGS; printf( "float32 (%f)", data ); }
template<> inline void mpd_DumpData<double>( MPD_DUMPDATA_ARGS(double) ){ MPD_DUMPDATA_USEARGS; printf( "float64 (%f)", data ); }
template<> inline void mpd_DumpData<mpd_StringView>( MPD_DUMPDATA_ARGS(mpd_StringView) )
{
	MPD_DUMPDATA_USEARGS;
	printf( "[%d]\"", (int) data.size );
	fwrite( data.str, data.size, 1, stdout );
	printf( "\"" );
}

//
// --- TYPE CONVERSION / DATA TRANSPORT ---
//
struct mpd_KeyValue
{
	const char* key;
	size_t keysz;
	const char* value;
	size_t valuesz;
	int32_t value_i32;
	float value_float;
};

struct mpd_TypeInfo
{
	const char* name;
	mpd_Type cls;
	const struct virtual_MPD* virt;
};

struct mpd_PropInfo
{
	const char* name;
	size_t namesz;
	mpd_TypeInfo type;
	const mpd_KeyValue* metadata;
};

struct mpd_EnumValue
{
	const char* name;
	size_t namesz;
	int64_t value;
	const mpd_KeyValue* metadata;
};

struct virtual_MPD
{
	virtual const char* vname() const { return 0; }
	virtual const mpd_KeyValue* vmetadata() const { return 0; }
	virtual int vpropcount() const { return 0; }
	virtual const mpd_PropInfo* vprops() const { return 0; }
	virtual const mpd_TypeInfo* vindextypes() const { return 0; }
	virtual int vvaluecount() const { return 0; }
	virtual const mpd_EnumValue* vvalues() const { return 0; }
	virtual struct mpd_Variant vgetprop( const void* p, int i ) const;
	virtual bool vsetprop( void* obj, int prop, const mpd_Variant& data ) const { (void) obj; (void) prop; (void) data; return false; }
	virtual struct mpd_Variant vgetindex( const void* obj, const mpd_Variant& key ) const;
	virtual bool vsetindex( void* obj, const mpd_Variant& key, const mpd_Variant& val ) const { (void) obj; (void) key; (void) val; return false; }
	virtual void vdump( const void* p, int limit, int level ) const { (void) p; (void) limit; (void) level; }
	
	virtual const mpd_PropInfo* vprop( int i ){ (void) i; return 0; }
	virtual const mpd_PropInfo* vfindprop_ext( const char* name, size_t namesz ){ (void) name; (void) namesz; return 0; }
	virtual const mpd_PropInfo* vfindprop( const char* name ){ (void) name; return 0; }
	virtual int vfindpropid_ext( const char* name, size_t sz ) const { (void) name; (void) sz; return -1; }
	virtual int vfindpropid( const char* name ) const { (void) name; return -1; }
	virtual int vprop2id( const mpd_PropInfo* prop ){ (void) prop; return -1; }
	virtual const char* vvalue2name( int32_t val, const char* def = "<unknown>" ){ (void) val; (void) def; return 0; }
	virtual int64_t vname2value( mpd_StringView name, int64_t def = 0 ){ (void) name; return def; }
};

struct none_MPD : virtual_MPD
{
	static none_MPD* inst(){ static none_MPD none; return &none; }
};

template< class T > struct mpd_MetaType : none_MPD
{
};

struct mpd_Variant
{
	mpd_Variant() : type( mpdt_None ), mpdata( none_MPD::inst() ){}
	template< class T > mpd_Variant( T* v ) : type( mpdt_Pointer ), mpdata( mpd_MetaType<T>::inst() ){ data.p = const_cast<void*>((const void*) v); }
	template< class T > mpd_Variant( T& v ) : type( mpdt_Struct ), mpdata( mpd_MetaType<T>::inst() ){ data.p = const_cast<void*>((const void*) &v); }
	mpd_Variant( mpd_Variant& p ) : type( p.type ), mpdata( p.mpdata ), data( p.data ){}
	mpd_Variant( const mpd_Variant& p ) : type( p.type ), mpdata( p.mpdata ), data( p.data ){}
	mpd_Variant( int8_t v ) : type( mpdt_Int8 ), mpdata( none_MPD::inst() ){ data.i = v; }
	mpd_Variant( int16_t v ) : type( mpdt_Int16 ), mpdata( none_MPD::inst() ){ data.i = v; }
	mpd_Variant( int32_t v ) : type( mpdt_Int32 ), mpdata( none_MPD::inst() ){ data.i = v; }
	mpd_Variant( int64_t v ) : type( mpdt_Int64 ), mpdata( none_MPD::inst() ){ data.i = v; }
	mpd_Variant( uint8_t v ) : type( mpdt_UInt8 ), mpdata( none_MPD::inst() ){ data.u = v; }
	mpd_Variant( uint16_t v ) : type( mpdt_UInt16 ), mpdata( none_MPD::inst() ){ data.u = v; }
	mpd_Variant( uint32_t v ) : type( mpdt_UInt32 ), mpdata( none_MPD::inst() ){ data.u = v; }
	mpd_Variant( uint64_t v ) : type( mpdt_UInt64 ), mpdata( none_MPD::inst() ){ data.u = v; }
	mpd_Variant( float v ) : type( mpdt_Float32 ), mpdata( none_MPD::inst() ){ data.f = v; }
	mpd_Variant( double v ) : type( mpdt_Float64 ), mpdata( none_MPD::inst() ){ data.f = v; }
	mpd_Variant( const char* str ) : type( mpdt_ConstString ), mpdata( none_MPD::inst() ){ data.s.str = str; data.s.size = strlen( str ); }
	mpd_Variant( const char* str, size_t size ) : type( mpdt_ConstString ), mpdata( none_MPD::inst() ){ data.s.str = str; data.s.size = size; }
	mpd_Variant( mpd_StringView sv ) : type( mpdt_ConstString ), mpdata( none_MPD::inst() ){ data.s = sv; }
	
	mpd_Type get_type() const { return type; }
	
	mpd_Variant get_target() const
	{
		if( type == mpdt_Pointer )
		{
			mpd_Variant p = *this;
			p.type = data.p ? mpdt_Struct : mpdt_None;
			return p;
		}
		return *this;
	}
	mpd_StringView get_stringview() const
	{
		if( type == mpdt_ConstString )
			return data.s;
		mpd_StringView sv = { NULL, 0 };
		return sv;
	}
	template< class T > T _get_numeric() const
	{
		mpd_Variant p = get_target();
		switch( p.type )
		{
		case mpdt_Enum:
		case mpdt_Int8:
		case mpdt_Int16:
		case mpdt_Int32:
		case mpdt_Int64:
			return (T) p.data.i;
		case mpdt_UInt8:
		case mpdt_UInt16:
		case mpdt_UInt32:
		case mpdt_UInt64:
			return (T) p.data.u;
		case mpdt_Float32:
		case mpdt_Float64:
			return (T) p.data.f;
		default:
			return (T) 0;
		}
	}
	int8_t get_int8() const { return _get_numeric<int8_t>(); }
	int16_t get_int16() const { return _get_numeric<int16_t>(); }
	int32_t get_int32() const { return _get_numeric<int32_t>(); }
	int64_t get_int64() const { return _get_numeric<int64_t>(); }
	uint8_t get_uint8() const { return _get_numeric<uint8_t>(); }
	uint16_t get_uint16() const { return _get_numeric<uint16_t>(); }
	uint32_t get_uint32() const { return _get_numeric<uint32_t>(); }
	uint64_t get_uint64() const { return _get_numeric<uint64_t>(); }
	float get_float32() const { return _get_numeric<float>(); }
	double get_float64() const { return _get_numeric<double>(); }
	template< class T > T get_obj() const
	{
		typedef mpd_MetaType<T> MT;
		if( type == mpdt_Struct && mpdata == MT::inst() )
			return *(T*) data.p;
		else
			return T();
	}
	
	mpd_Variant getpropbyid( int id ) const
	{
		if( ( type == mpdt_Struct || type == mpdt_Pointer ) && data.p )
			return mpdata->vgetprop( data.p, id );
		return mpd_Variant();
	}
	mpd_Variant getprop( const char* name ) const { return getpropbyid( mpdata->vfindpropid( name ) ); }
	mpd_Variant getprop_ext( const char* name, size_t sz ) const { return getpropbyid( mpdata->vfindpropid_ext( name, sz ) ); }
	bool setpropbyid( int id, const mpd_Variant& val ) const
	{
		if( ( type == mpdt_Struct || type == mpdt_Pointer ) && data.p )
			return mpdata->vsetprop( data.p, id, val );
		return false;
	}
	bool setprop( const char* name, const mpd_Variant& val ) const { return setpropbyid( mpdata->vfindpropid( name ), val ); }
	bool setprop_ext( const char* name, size_t sz, const mpd_Variant& val ) const { return setpropbyid( mpdata->vfindpropid_ext( name, sz ), val ); }
	mpd_Variant getindex( const mpd_Variant& key ) const { return mpdata->vgetindex( data.p, key ); }
	bool setindex( const mpd_Variant& key, const mpd_Variant& val ) const { return mpdata->vsetindex( data.p, key, val ); }
	void dump( int limit = 5, int level = 0 ) const
	{
		switch( type )
		{
		default: printf( "<error>" ); break;
		case mpdt_None: printf( "<none>" ); break;
		case mpdt_Struct:
		case mpdt_Pointer:
		case mpdt_Enum:
			mpdata->vdump( data.p, limit, level );
			break;
		case mpdt_ConstString:
			printf( "[%d]\"", (int) data.s.size );
			fwrite( data.s.str, data.s.size, 1, stdout );
			printf( "\"" );
			break;
		case mpdt_Int8: mpd_DumpData( (int8_t) data.i, limit, level ); break;
		case mpdt_Int16: mpd_DumpData( (int16_t) data.i, limit, level ); break;
		case mpdt_Int32: mpd_DumpData( (int32_t) data.i, limit, level ); break;
		case mpdt_Int64: mpd_DumpData( (int64_t) data.i, limit, level ); break;
		case mpdt_UInt8: mpd_DumpData( (uint8_t) data.u, limit, level ); break;
		case mpdt_UInt16: mpd_DumpData( (uint16_t) data.u, limit, level ); break;
		case mpdt_UInt32: mpd_DumpData( (uint32_t) data.u, limit, level ); break;
		case mpdt_UInt64: mpd_DumpData( (uint64_t) data.u, limit, level ); break;
		case mpdt_Float32: mpd_DumpData( (float) data.f, limit, level ); break;
		case mpdt_Float64: mpd_DumpData( (double) data.f, limit, level ); break;
		}
	}
	
private:
	mpd_Type type;
	const virtual_MPD* mpdata;
	union mpdVariant_Data
	{
		mpd_StringView s;
		void* p;
		int64_t i;
		uint64_t u;
		double f;
	}
	data;
};
template<> inline void mpd_DumpData<mpd_Variant>( MPD_DUMPDATA_ARGS(mpd_Variant) ){ data.dump( limit, level ); }

mpd_Variant virtual_MPD::vgetprop( const void*, int ) const
{
	return mpd_Variant();
}
mpd_Variant virtual_MPD::vgetindex( const void*, const mpd_Variant& ) const
{
	return mpd_Variant();
}

template< class T > T mpd_var_get( const mpd_Variant& v ){ return v.get_obj<T>(); }
template<> int8_t mpd_var_get<int8_t>( const mpd_Variant& v ){ return v.get_int8(); }
template<> int16_t mpd_var_get<int16_t>( const mpd_Variant& v ){ return v.get_int16(); }
template<> int32_t mpd_var_get<int32_t>( const mpd_Variant& v ){ return v.get_int32(); }
template<> int64_t mpd_var_get<int64_t>( const mpd_Variant& v ){ return v.get_int64(); }
template<> uint8_t mpd_var_get<uint8_t>( const mpd_Variant& v ){ return v.get_uint8(); }
template<> uint16_t mpd_var_get<uint16_t>( const mpd_Variant& v ){ return v.get_uint16(); }
template<> uint32_t mpd_var_get<uint32_t>( const mpd_Variant& v ){ return v.get_uint32(); }
template<> uint64_t mpd_var_get<uint64_t>( const mpd_Variant& v ){ return v.get_uint64(); }
template<> float mpd_var_get<float>( const mpd_Variant& v ){ return v.get_float32(); }
template<> double mpd_var_get<double>( const mpd_Variant& v ){ return v.get_float64(); }

//
// --- METADATA ---
//
void mpd_DumpMetadata( const mpd_KeyValue* md )
{
	if( md->key )
	{
		printf( " [" );
		while( md->key )
		{
			printf( "%s = %s%s", md->key, md->value, md[1].key ? ", " : "" );
			++md;
		}
		printf( "]" );
	}
}
template< class T > void mpd_DumpInfo()
{
	typedef mpd_MetaType<T> MT;
	printf( "%s %s", MT::valuecount() ? "enum" : "struct", MT::name() );
	mpd_DumpMetadata( MT::metadata() );
	printf( "\n{\n" );
	if( MT::valuecount() )
	{
		const mpd_EnumValue* v = MT::values();
		while( v->name )
		{
			printf( "\t%s = %" PRId64, v->name, v->value );
			mpd_DumpMetadata( v->metadata );
			puts("");
			++v;
		}
	}
	else
	{
		const mpd_TypeInfo* indextypes = MT::indextypes();
		if( indextypes )
		{
			printf( "\tindex [%s => %s]\n", indextypes[0].name, indextypes[1].name );
		}
		const mpd_PropInfo* p = MT::props();
		while( p->name )
		{
			printf( "\t%s %s", p->type.name, p->name );
			mpd_DumpMetadata( p->metadata );
			puts("");
			++p;
		}
	}
	printf( "}\n" );
}

void mpd_DumpInfo( const virtual_MPD* type, int limit = 5, int level = 0, bool _int_subprop = false )
{
	if( !_int_subprop )
		__mpd_reprint( "\t", level );
	printf( "%s %s", type->vvaluecount() ? "enum" : "struct", type->vname() );
	mpd_DumpMetadata( type->vmetadata() );
	printf( "\n" );
	
	__mpd_reprint( "\t", level );
	printf( "{\n" );
	level++;
	
	if( type->vvaluecount() )
	{
		const mpd_EnumValue* v = type->vvalues();
		while( v->name )
		{
			__mpd_reprint( "\t", level );
			printf( "%s = %" PRId64, v->name, v->value );
			mpd_DumpMetadata( v->metadata );
			puts("");
			++v;
		}
	}
	else
	{
		const mpd_TypeInfo* indextypes = type->vindextypes();
		if( indextypes )
		{
			__mpd_reprint( "\t", level );
			printf( "index [%s => %s]\n", indextypes[0].name, indextypes[1].name );
		}
		const mpd_PropInfo* p = type->vprops();
		while( p->name )
		{
			__mpd_reprint( "\t", level );
			printf( "%s", p->name );
			mpd_DumpMetadata( p->metadata );
			printf( ": " );
			if( p->type.cls == mpdt_Struct && p->type.virt && level < limit )
			{
				mpd_DumpInfo( p->type.virt, limit, level, true );
			}
			else
				printf( "%s\n", p->type.name );
			++p;
		}
	}
	
	level--;
	__mpd_reprint( "\t", level );
	printf( "}\n" );
}

template< class T, class ST > struct struct_MPD : virtual_MPD
{
	typedef ST type;
	
	// helper functions
	static const mpd_PropInfo* prop( int i )
	{
		if( unsigned(i) >= unsigned(T::propcount()) )
			return NULL;
		return &T::props()[ i ];
	}
	static const mpd_PropInfo* findprop_ext( const char* name, size_t namesz )
	{
		const mpd_PropInfo* p = T::props();
		while( p->name )
		{
			if( p->namesz == namesz && memcmp( p->name, name, namesz ) == 0 )
				return p;
			++p;
		}
		return NULL;
	}
	static const mpd_PropInfo* findprop( const char* name )
	{
		return findprop_ext( name, strlen( name ) );
	}
	static int findpropid_ext( const char* name, size_t namesz )
	{
		const mpd_PropInfo* p = findprop_ext( name, namesz );
		return p ? ( p - T::props() ) : -1;
	}
	static int findpropid( const char* name )
	{
		const mpd_PropInfo* p = findprop( name );
		return p ? ( p - T::props() ) : -1;
	}
	static int prop2id( const mpd_PropInfo* prop )
	{
		int pid = ( prop - T::props() );
		if( pid >= T::propcount() )
			return -1;
		return pid;
	}
	static const char* value2name( int64_t val, const char* def = "<unknown>" )
	{
		const mpd_EnumValue* v = T::values();
		while( v->name )
		{
			if( v->value == val )
				return v->name;
			++v;
		}
		return def;
	}
	static int64_t name2value( mpd_StringView name, int64_t def = 0 )
	{
		const mpd_EnumValue* v = T::values();
		while( v->name )
		{
			if( v->namesz == name.size && memcmp( name.str, v->name, name.size ) == 0 )
				return v->value;
			++v;
		}
		return def;
	}
	
	// virtual wrappers
	// - core interface
	virtual const char* vname() const { return T::name(); }
	virtual const mpd_KeyValue* vmetadata() const { return T::metadata(); }
	virtual int vpropcount() const { return T::propcount(); }
	virtual const mpd_PropInfo* vprops() const { return T::props(); }
	virtual const mpd_TypeInfo* vindextypes() const { return T::indextypes(); }
	virtual int vvaluecount() const { return T::valuecount(); }
	virtual const mpd_EnumValue* vvalues() const { return T::values(); }
	virtual mpd_Variant vgetprop( const void* obj, int prop ) const { return T::getprop( (type const*) obj, prop ); }
	virtual bool vsetprop( void* obj, int prop, const mpd_Variant& data ) const { return T::setprop( (type*) obj, prop, data ); }
	virtual struct mpd_Variant vgetindex( const void* obj, const mpd_Variant& key ) const { return T::getindex( (type const*) obj, key ); }
	virtual bool vsetindex( void* obj, const mpd_Variant& key, const mpd_Variant& val ) const { return T::setindex( (type*) obj, key, val ); }
	virtual void vdump( const void* p, int limit, int level ) const { T::dump( (type const*) p, limit, level ); }
	// - virtual helper function wrappers
	virtual const mpd_PropInfo* vprop( int i ){ return prop( i ); }
	virtual const mpd_PropInfo* vfindprop_ext( const char* name, size_t namesz ){ return findprop_ext( name, namesz ); }
	virtual const mpd_PropInfo* vfindprop( const char* name ){ return findprop( name ); }
	virtual int vfindpropid_ext( const char* name, size_t sz ) const { return findpropid_ext( name, sz ); }
	virtual int vfindpropid( const char* name ) const { return findpropid( name ); }
	virtual int vprop2id( const mpd_PropInfo* prop ){ return prop2id( prop ); }
	virtual const char* vvalue2name( int64_t val, const char* def = "<unknown>" ){ return value2name( val, def ); }
	virtual int64_t vname2value( mpd_StringView name, int64_t def = 0 ){ return name2value( name, def ); }
};

template< class T > const char* mpd_Value2Name( T val, const char* def = "<unknown>" )
{
	return mpd_MetaType<T>::value2name( val, def );
}
template< class T > T mpd_Name2Value( mpd_StringView name, T def = (T) 0 )
{
	return (T) mpd_MetaType<T>::name2value( name, def );
}

