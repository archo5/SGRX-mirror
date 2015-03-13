

#pragma once
#define __STDC_FORMAT_MACROS 1
#define HEADER_SGSCRIPT_H "sgscript.h"
#include "../ext/src/sgscript/sgscript.h"
#include "../ext/src/sgscript/sgs_cppbc.h"

#include "utils.hpp"


SGS_DECL_DUMPDATA_INT( String );
template<> inline void sgs_PushVar<String>( SGS_CTX, const String& v ){ sgs_PushStringBuf( C, v.data(), (sgs_SizeVal) v.size() ); }
template<> struct sgs_GetVar<String> { String operator () ( SGS_CTX, sgs_StkIdx item ){
	char* str; sgs_SizeVal size; if( sgs_ParseString( C, item, &str, &size ) )
		return String( str, (size_t) size ); return String(); }};
template<> struct sgs_GetVarP<String> { String operator () ( SGS_CTX, sgs_Variable* var ){
	char* str; sgs_SizeVal size; if( sgs_ParseStringP( C, var, &str, &size ) )
		return String( str, (size_t) size ); return String(); }};


struct ScriptVarIterator
{
	ScriptVarIterator( SGS_CTX, sgs_Variable* var );
	ScriptVarIterator( sgsVariable& var );
	void _Init( SGS_CTX, sgs_Variable* var );
	
	sgsVariable GetKey();
	sgsVariable GetValue();
	bool Advance();
	
	sgsVariable m_iter;
};

struct ScriptContext
{
	ScriptContext();
	~ScriptContext();
	
	bool ExecFile( const StringView& path );
	bool ExecBuffer( const StringView& data );
	
	String Serialize( sgsVariable var );
	sgsVariable Unserialize( const StringView& sv );
	
	sgsVariable CreateDict( int args = 0 );
	
	template< class T > void Push( const T& v ){ sgs_PushVar( C, v ); }
	
	bool GlobalCall( const char* name, int args = 0, int ret = 0 );
	
	// creates a new dict, sets metaobject of current to new, sets new as env
	void PushEnv();
	// tries to reverse the operation done by the previous function, returns false if no metaobj
	bool PopEnv();
	
	ScriptVarIterator GlobalIterator();
	
	SGS_CTX;
};

