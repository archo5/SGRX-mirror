

#include "script.hpp"
#include "engine.hpp"


ScriptVarIterator::ScriptVarIterator( SGS_CTX, sgs_Variable* var )
{
	_Init( C, var );
}

ScriptVarIterator::ScriptVarIterator( sgsVariable& var )
{
	_Init( var.C, &var.var );
}

void ScriptVarIterator::_Init( SGS_CTX, sgs_Variable* var )
{
	if( !C )
		return;
	sgs_Variable it;
	if( SGS_SUCCEEDED( sgs_GetIteratorP( C, var, &it ) ) )
	{
		m_iter = sgsVariable( C, &it );
	}
}

sgsVariable ScriptVarIterator::GetKey()
{
	sgsVariable out( m_iter.C );
	if( m_iter.C )
		sgs_IterGetDataP( m_iter.C, &m_iter.var, &out.var, NULL );
	return out;
}

sgsVariable ScriptVarIterator::GetValue()
{
	sgsVariable out( m_iter.C );
	if( m_iter.C )
		sgs_IterGetDataP( m_iter.C, &m_iter.var, NULL, &out.var );
	return out;
}

bool ScriptVarIterator::Advance()
{
	return m_iter.C && sgs_IterAdvanceP( m_iter.C, &m_iter.var ) > 0;
}


ScriptContext::ScriptContext() : C(NULL)
{
	Reset();
}

ScriptContext::~ScriptContext()
{
	sgs_DestroyEngine( C );
}


static int BR_Reset( SGS_CTX )
{
	SGSFN( "BR_Reset" );
	GR2D_GetBatchRenderer().Reset();
	return 0;
}
static int BR_Col( SGS_CTX )
{
	float v[4] = {0,0,0,1};
	SGSFN( "BR_Col" );
	if( !sgs_LoadArgs( C, "f|fff.", v, v + 1, v + 2, v + 3 ) )
		return 0;
	int argc = sgs_StackSize( C );
	if( argc == 1 )
		v[3] = v[2] = v[1] = v[0];
	else if( argc == 2 )
	{
		v[3] = v[1];
		v[2] = v[1] = v[0];
	}
	GR2D_GetBatchRenderer().Col( v[0], v[1], v[2], v[3] );
	return 0;
}
static int BR_Pos( SGS_CTX )
{
	Vec3 pos;
	SGSFN( "BR_Pos" );
	if( !sgs_LoadArgs( C, "x", sgs_ArgCheck_Vec3, &pos.x ) )
		return 0;
	GR2D_GetBatchRenderer().Pos( pos );
	return 0;
}
static int BR_SphereOutline( SGS_CTX )
{
	Vec3 pos;
	float rad;
	int32_t verts;
	SGSFN( "BR_SphereOutline" );
	if( !sgs_LoadArgs( C, "xfl", sgs_ArgCheck_Vec3, &pos.x, &rad, &verts ) )
		return 0;
	GR2D_GetBatchRenderer().SphereOutline( pos, rad, verts );
	return 0;
}
static int BR_AABB( SGS_CTX )
{
	Vec3 pmin = {0}, pmax = {0};
	Mat4 mtx = Mat4::Identity;
	SGSFN( "BR_AABB" );
	if( !sgs_LoadArgs( C, "xx|x", sgs_ArgCheck_Vec3, &pmin.x, sgs_ArgCheck_Vec3, &pmax.x, sgs_ArgCheck_Mat4, mtx.a ) )
		return 0;
	GR2D_GetBatchRenderer().AABB( pmin, pmax, mtx );
	return 0;
}
static int BR_Tick( SGS_CTX )
{
	Vec3 pos;
	float rad;
	Mat4 mtx = Mat4::Identity;
	SGSFN( "BR_Tick" );
	if( !sgs_LoadArgs( C, "xf|x", sgs_ArgCheck_Vec3, &pos.x, &rad, sgs_ArgCheck_Mat4, mtx.a ) )
		return 0;
	GR2D_GetBatchRenderer().Tick( pos, rad, mtx );
	return 0;
}
static int BR_SetPrimitiveType( SGS_CTX )
{
	int32_t pt = 0;
	SGSFN( "BR_SetPrimitiveType" );
	if( !sgs_LoadArgs( C, "l", &pt ) )
		return 0;
	GR2D_GetBatchRenderer().SetPrimitiveType( (EPrimitiveType) pt );
	return 0;
}
static sgs_RegFuncConst g_ent_scripted_rfc[] =
{
	{ "BR_Reset", BR_Reset },
	{ "BR_Col", BR_Col },
	{ "BR_Pos", BR_Pos },
	{ "BR_SphereOutline", BR_SphereOutline },
	{ "BR_AABB", BR_AABB },
	{ "BR_Tick", BR_Tick },
	{ "BR_SetPrimitiveType", BR_SetPrimitiveType },
	SGS_RC_END(),
};
static sgs_RegIntConst g_ent_scripted_ric[] =
{
	{ "PT_Points", PT_Points },
	{ "PT_Lines", PT_Lines },
	{ "PT_LineStrip", PT_LineStrip },
	{ "PT_Triangles", PT_Triangles },
	{ "PT_TriangleStrip", PT_TriangleStrip },
	{ NULL, 0 },
};

static SGSRESULT sgs_scriptfs_sgrx( void* ud, SGS_CTX, int op, sgs_ScriptFSData* fsd )
{
	switch( op )
	{
	case SGS_SFS_FILE_EXISTS:
		return FS_FileExists( fsd->filename ) ? SGS_SUCCESS : SGS_ENOTFND;
	case SGS_SFS_FILE_OPEN:
		{
			ByteArray* data = new ByteArray;
			if( !FS_LoadBinaryFile( fsd->filename, *data ) )
			{
				delete data;
				return SGS_ENOTFND;
			}
			fsd->userhandle = data;
			fsd->size = data->size();
			return SGS_SUCCESS;
		}
	case SGS_SFS_FILE_READ:
		{
			SGRX_CAST( ByteArray*, data, fsd->userhandle );
			if( fsd->size > data->size() )
				return SGS_EINPROC;
			memcpy( fsd->output, data->data(), fsd->size );
			return SGS_SUCCESS;
		}
	case SGS_SFS_FILE_CLOSE:
		{
			SGRX_CAST( ByteArray*, data, fsd->userhandle );
			delete data;
			fsd->userhandle = NULL;
		}
	}
	return SGS_ENOTSUP;
}

void ScriptContext::Reset()
{
	if( C ) sgs_DestroyEngine( C );
	C = sgs_CreateEngine();
	xgm_module_entry_point( C );
	sgs_SetScriptFSFunc( C, sgs_scriptfs_sgrx, NULL );
}

void ScriptContext::RegisterBatchRenderer()
{
	sgs_RegFuncConsts( C, g_ent_scripted_rfc, -1 );
	sgs_RegIntConsts( C, g_ent_scripted_ric, -1 );
}

bool ScriptContext::EvalFile( const StringView& path, sgsVariable* outvar )
{
	SGS_SCOPE;
	int rvc = 0;
	bool ret = SGS_SUCCEEDED( sgs_EvalFile( C, StackPath( path ), &rvc ) );
	if( rvc && outvar )
		*outvar = sgs_GetVar<sgsVariable>()( C, -rvc );
	return ret;
}

bool ScriptContext::EvalBuffer( const StringView& data, sgsVariable* outvar )
{
	SGS_SCOPE;
	int rvc = 0;
	bool ret = SGS_SUCCEEDED( sgs_EvalBuffer( C, data.data(), data.size(), &rvc ) );
	if( rvc && outvar )
		*outvar = sgs_GetVar<sgsVariable>()( C, -rvc );
	return ret;
}

bool ScriptContext::ExecFile( const StringView& path )
{
	return SGS_SUCCEEDED( sgs_ExecFile( C, StackPath( path ) ) );
}

bool ScriptContext::ExecBuffer( const StringView& data )
{
	return SGS_SUCCEEDED( sgs_ExecBuffer( C, data.data(), data.size() ) );
}

bool ScriptContext::Include( const char* what, const char* searchpath )
{
	return SGS_SUCCEEDED( sgs_IncludeExt( C, what, searchpath ) );
}

String ScriptContext::Serialize( sgsVariable var )
{
	SGS_SCOPE;
	String out;
	sgs_PushVar( C, var );
	if( SGS_SUCCEEDED( sgs_Serialize( C ) ) )
	{
		out = sgs_GetVar<String>()( C, -1 );
	}
	return out;
}

sgsVariable ScriptContext::Unserialize( const StringView& sv )
{
	SGS_SCOPE;
	sgsVariable out;
	sgs_PushStringBuf( C, sv.data(), sv.size() );
	if( SGS_SUCCEEDED( sgs_Unserialize( C ) ) )
	{
		out = sgs_GetVar<sgsVariable>()( C, -1 );
	}
	return out;
}

sgsString ScriptContext::CreateString( const StringView& sv )
{
	return sgsString( C, sv.data(), sv.size() );
}

sgsVariable ScriptContext::CreateStringVar( const StringView& sv )
{
	return sgsVariable().set( CreateString( sv ) );
}

sgsVariable ScriptContext::CreateDict( int args )
{
	sgsVariable out( C );
	sgs_InitDict( C, &out.var, args );
	return out;
}

sgsVariable ScriptContext::CreateVec2( const Vec2& v )
{
	sgsVariable out( C );
	sgs_InitVec2( C, &out.var, v.x, v.y );
	return out;
}

sgsVariable ScriptContext::CreateVec3( const Vec3& v )
{
	sgsVariable out( C );
	sgs_InitVec3( C, &out.var, v.x, v.y, v.z );
	return out;
}

bool ScriptContext::Call( sgsVariable func, int args, int ret )
{
	return SGS_SUCCEEDED( sgs_CallP( C, &func.var, args, ret ) );
}

bool ScriptContext::GlobalCall( const char* name, int args, int ret )
{
	return SGS_SUCCEEDED( sgs_GlobalCall( C, name, args, ret ) );
}

void ScriptContext::PushEnv()
{
	sgs_Variable cur_env, new_env;
	
	sgs_GetEnv( C, &cur_env );
	sgs_InitDict( C, &new_env, 0 );
	sgs_ObjSetMetaObj( C, sgs_GetObjectStructP( &new_env ), sgs_GetObjectStructP( &cur_env ) );
	sgs_SetEnv( C, &new_env );
	sgs_Release( C, &new_env );
	sgs_Release( C, &cur_env );
}

bool ScriptContext::PopEnv()
{
	sgs_Variable cur_env;
	
	sgs_GetEnv( C, &cur_env );
	sgs_VarObj* upper_env = sgs_ObjGetMetaObj( sgs_GetObjectStructP( &cur_env ) );
	if( upper_env )
		sgs_ObjSetMetaObj( C, sgs_GetObjectStructP( &cur_env ), NULL );
	sgs_Release( C, &cur_env );
	if( upper_env )
	{
		sgs_Variable new_env;
		sgs_InitObjectPtr( &new_env, upper_env );
		sgs_SetEnv( C, &new_env );
		sgs_Release( C, &new_env );
		return true;
	}
	return false;
}

ScriptVarIterator ScriptContext::GlobalIterator()
{
	sgs_Variable env;
	sgs_GetEnv( C, &env );
	ScriptVarIterator svi( C, &env );
	sgs_Release( C, &env );
	return svi;
}