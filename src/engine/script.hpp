

#pragma once
#define SGS_DLL 1
#define __STDC_FORMAT_MACROS 1
#define HEADER_SGSCRIPT_H "sgscript.h"
#define HEADER_SGS_UTIL_H "sgs_util.h"
#include "../../ext/src/sgscript/sgscript.h"
#include "../../ext/src/sgscript/sgsxgmath.h"
#include "../../ext/src/sgscript/sgs_cppbc.h"
#include "../../ext/src/sgscript/sgs_prof.h"

#include "utils.hpp"


#ifdef SCRIPT_BUILDING
#  define SCRIPT_EXPORT __declspec(dllexport)
#else
#  define SCRIPT_EXPORT __declspec(dllimport)
#endif


inline Hash HashVar( const sgsString& s ){ return HashVar( StringView( s.c_str(), s.size() ) ); }

SGS_DECL_DUMPDATA_INT( String );
template<> inline void sgs_PushVar<String>( SGS_CTX, const String& v ){ sgs_PushStringBuf( C, v.data(), (sgs_SizeVal) v.size() ); }
template<> struct sgs_GetVar<String> { String operator () ( SGS_CTX, sgs_StkIdx item ){
	char* str; sgs_SizeVal size; if( sgs_ParseString( C, item, &str, &size ) )
		return String( str, (size_t) size ); return String(); }};
template<> inline void sgs_PushVar<StringView>( SGS_CTX, const StringView& v ){ sgs_PushStringBuf( C, v.data(), (sgs_SizeVal) v.size() ); }
template<> struct sgs_GetVar<StringView> { StringView operator () ( SGS_CTX, sgs_StkIdx item ){
	char* str; sgs_SizeVal size; if( sgs_ParseString( C, item, &str, &size ) )
		return StringView( str, (size_t) size ); return StringView(); }};

// Vec2 interface
template<> inline void sgs_PushVar<Vec2>( SGS_CTX, const Vec2& v ){ sgs_CreateVec2( C, NULL, v.x, v.y ); }
template<> struct sgs_GetVar<Vec2> { Vec2 operator () ( SGS_CTX, sgs_StkIdx item ){
	float vtmp[2] = {0.0f}; sgs_ParseVec2( C, item, vtmp, 0 ); return V2( vtmp[0], vtmp[1] ); }};
SGS_DECL_DUMPDATA_INT( Vec2 );

// Vec3 interface
template<> inline void sgs_PushVar<Vec3>( SGS_CTX, const Vec3& v ){ sgs_CreateVec3( C, NULL, v.x, v.y, v.z ); }
template<> struct sgs_GetVar<Vec3> { Vec3 operator () ( SGS_CTX, sgs_StkIdx item ){
	float vtmp[3] = {0.0f}; sgs_ParseVec3( C, item, vtmp, 0 ); return V3( vtmp[0], vtmp[1], vtmp[2] ); }};
SGS_DECL_DUMPDATA_INT( Vec3 );

// Vec4 interface
template<> inline void sgs_PushVar<Vec4>( SGS_CTX, const Vec4& v ){ sgs_CreateVec4( C, NULL, v.x, v.y, v.z, v.w ); }
template<> struct sgs_GetVar<Vec4> { Vec4 operator () ( SGS_CTX, sgs_StkIdx item ){
	float vtmp[4] = {0.0f}; sgs_ParseVec4( C, item, vtmp, 0 ); return V4( vtmp[0], vtmp[1], vtmp[2], vtmp[3] ); }};
SGS_DECL_DUMPDATA_INT( Vec4 );

// Quat interface
template<> inline void sgs_PushVar<Quat>( SGS_CTX, const Quat& v ){ sgs_CreateQuat( C, NULL, v.x, v.y, v.z, v.w ); }
template<> struct sgs_GetVar<Quat> { Quat operator () ( SGS_CTX, sgs_StkIdx item ){
	float vtmp[4] = {0,0,0,1}; sgs_ParseQuat( C, item, vtmp, 0 ); Quat q = { vtmp[0], vtmp[1], vtmp[2], vtmp[3] }; return q; }};
SGS_DECL_DUMPDATA_INT( Quat );

// Mat4
template<> inline void sgs_PushVar<Mat4>( SGS_CTX, const Mat4& v ){ sgs_CreateMat4( C, NULL, v.a, 0 ); }
template<> struct sgs_GetVar<Mat4> { Mat4 operator () ( SGS_CTX, sgs_StkIdx item ){
	Mat4 m = Mat4::Identity; sgs_ParseMat4( C, item, m.a ); return m; }};
SGS_DECL_DUMPDATA_INT( Mat4 );


struct ScriptVarIterator
{
	SCRIPT_EXPORT ScriptVarIterator( SGS_CTX, sgs_Variable* var );
	SCRIPT_EXPORT ScriptVarIterator( sgsVariable var );
	SCRIPT_EXPORT void _Init( SGS_CTX, sgs_Variable* var );
	
	SCRIPT_EXPORT sgsVariable GetKey();
	SCRIPT_EXPORT sgsVariable GetValue();
	SCRIPT_EXPORT bool Advance();
	
	sgsVariable m_iter;
};

SCRIPT_EXPORT void ScriptAssignProperties( sgsVariable to, sgsVariable from );

struct ScriptContext
{
	SCRIPT_EXPORT ScriptContext();
	SCRIPT_EXPORT ~ScriptContext();
	
	SCRIPT_EXPORT void Reset();
	SCRIPT_EXPORT void RegisterEngineCoreAPI();
	SCRIPT_EXPORT void RegisterBatchRenderer();
	
	SCRIPT_EXPORT bool EvalFile( const StringView& path, sgsVariable* outvar = NULL );
	SCRIPT_EXPORT bool EvalBuffer( const StringView& data, sgsVariable* outvar = NULL );
	SCRIPT_EXPORT bool ExecFile( const StringView& path );
	SCRIPT_EXPORT bool ExecBuffer( const StringView& data );
	SCRIPT_EXPORT bool Include( const char* what, const char* searchpath = NULL );
	
	SCRIPT_EXPORT String Serialize( sgsVariable var );
	SCRIPT_EXPORT sgsVariable Unserialize( const StringView& sv );
	SCRIPT_EXPORT String ToSGSON( sgsVariable var, const char* tab = "\t" );
	SCRIPT_EXPORT sgsVariable ParseSGSON( const StringView& sv );
	
	SCRIPT_EXPORT sgsString CreateString( const StringView& sv );
	SCRIPT_EXPORT sgsVariable CreateStringVar( const StringView& sv );
	SCRIPT_EXPORT sgsVariable CreateArray( int args = 0 );
	SCRIPT_EXPORT sgsVariable CreateDict( int args = 0 );
	SCRIPT_EXPORT sgsVariable CreateVec2( const Vec2& v );
	SCRIPT_EXPORT sgsVariable CreateVec3( const Vec3& v );
	SCRIPT_EXPORT sgsVariable CreateQuat( const Quat& v );
	
	template< class T > void Push( const T& v ){ sgs_PushVar( C, v ); }
	
	SCRIPT_EXPORT sgsVariable Registry();
	SCRIPT_EXPORT sgsVariable GetGlobal( const StringView& name );
	SCRIPT_EXPORT sgsVariable GetGlobal( sgsString name );
	SCRIPT_EXPORT void SetGlobal( const StringView& name, sgsVariable val );
	
	SCRIPT_EXPORT void Call( sgsVariable func, int args = 0, int ret = 0 );
	SCRIPT_EXPORT bool GlobalCall( StringView name, int args = 0, int ret = 0 );
	
	// creates a new dict, sets metaobject of current to new, sets new as env
	SCRIPT_EXPORT void PushEnv();
	// tries to reverse the operation done by the previous function, returns false if no metaobj
	SCRIPT_EXPORT bool PopEnv();
	
	SCRIPT_EXPORT ScriptVarIterator GlobalIterator();
	
	SGS_CTX;
};

