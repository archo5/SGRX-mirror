

#include "script.hpp"
#include "engine.hpp"


ScriptVarIterator::ScriptVarIterator( SGS_CTX, sgs_Variable* var )
{
	_Init( C, var );
}

ScriptVarIterator::ScriptVarIterator( sgsVariable var )
{
	_Init( var.get_ctx(), &var.var );
}

void ScriptVarIterator::_Init( SGS_CTX, sgs_Variable* var )
{
	if( !C )
		return;
	sgs_Variable it;
	if( SGS_SUCCEEDED( sgs_GetIterator( C, *var, &it ) ) )
	{
		m_iter = sgsVariable( C, &it );
		sgs_Release( C, &it );
	}
}

sgsVariable ScriptVarIterator::GetKey()
{
	sgsVariable out( m_iter.get_ctx() );
	if( m_iter.get_ctx() )
		sgs_IterGetData( m_iter.get_ctx(), m_iter.var, &out.var, NULL );
	return out;
}

sgsVariable ScriptVarIterator::GetValue()
{
	sgsVariable out( m_iter.get_ctx() );
	if( m_iter.get_ctx() )
		sgs_IterGetData( m_iter.get_ctx(), m_iter.var, NULL, &out.var );
	return out;
}

bool ScriptVarIterator::Advance()
{
	return m_iter.get_ctx() && sgs_IterAdvance( m_iter.get_ctx(), m_iter.var ) > 0;
}


void ScriptAssignProperties( sgsVariable to, sgsVariable from, const char* exclprefix )
{
	if( !( from.not_null() && to.not_null() ) )
		return;
	
	size_t eplen = exclprefix ? strlen( exclprefix ) : 0;
	
	ScriptVarIterator it( from );
	while( it.Advance() )
	{
		sgsVariable key = it.GetKey();
		if( exclprefix && key.is_string() )
		{
			sgsString str = key.get_string();
			if( str.size() > eplen &&
				memcmp( str.c_str(), exclprefix, eplen ) == 0 )
				continue;
		}
		to.setindex( key, it.GetValue() );
	}
}


ScriptContext::ScriptContext() : C(NULL)
{
	Reset();
}

ScriptContext::~ScriptContext()
{
	sgs_DestroyEngine( C );
}


static int euler2quat( SGS_CTX )
{
	Vec3 angles;
	if( !sgs_LoadArgs( C, "x", sgs_ArgCheck_Vec3, &angles.x ) )
		return 0;
	Quat rot = Quat::CreateFromXYZ( DEG2RAD( angles ) );
	return sgs_CreateQuat( C, NULL, rot.x, rot.y, rot.z, rot.w );
}
static int quat2euler( SGS_CTX )
{
	Quat rot;
	if( !sgs_LoadArgs( C, "x", sgs_ArgCheck_Quat, &rot.x ) )
		return 0;
	Vec3 angles = RAD2DEG( rot.ToXYZ() );
	return sgs_CreateVec3( C, NULL, angles.x, angles.y, angles.z );
}
static int quat_slerp( SGS_CTX )
{
	Quat A, B;
	float q;
	if( !sgs_LoadArgs( C, "xxf", sgs_ArgCheck_Quat, &A.x, sgs_ArgCheck_Quat, &B.x, &q ) )
		return 0;
	Quat R = QuatSlerp( A, B, q );
	return sgs_CreateQuat( C, NULL, R.x, R.y, R.z, R.w );
}
static sgs_RegFuncConst g_sgrxmath_rfc[] =
{
	{ "euler2quat", euler2quat },
	{ "quat2euler", quat2euler },
	{ "quat_slerp", quat_slerp },
	SGS_RC_END(),
};
static void sgrx_math_lib( SGS_CTX )
{
	sgs_RegFuncConsts( C, g_sgrxmath_rfc, -1 );
}


static int sgsGame_GetScreenSize( SGS_CTX )
{
	SGSFN( "Game_GetScreenSize" );
	sgs_PushVar( C, Game_GetScreenSize() );
	return 1;
}

static int sgsGame_ShowCursor( SGS_CTX )
{
	SGSFN( "Game_ShowCursor" );
	sgs_Bool show = 0;
	if( !sgs_LoadArgs( C, "b", &show ) )
		return 0;
	Game_ShowCursor( show );
	return 0;
}

static int sgsGame_End( SGS_CTX )
{
	SGSFN( "Game_End" );
	Game_End();
	return 0;
}

static int Input_GetValue( SGS_CTX )
{
	char* str = NULL;
	SGSFN( "Input_GetValue" );
	if( !sgs_LoadArgs( C, "s", &str ) )
		return 0;
	InputState* is = Game_FindAction( sgs_GetVar<StringView>()( C, 0 ) );
	if( !is )
		return sgs_Msg( C, SGS_WARNING, "action not found: %s", str );
	return sgs_PushReal( C, is->value );
}

static int Input_GetState( SGS_CTX )
{
	char* str = NULL;
	SGSFN( "Input_GetState" );
	if( !sgs_LoadArgs( C, "s", &str ) )
		return 0;
	InputState* is = Game_FindAction( sgs_GetVar<StringView>()( C, 0 ) );
	if( !is )
		return sgs_Msg( C, SGS_WARNING, "action not found: %s", str );
	return sgs_PushBool( C, is->state );
}

static int Input_GetPressed( SGS_CTX )
{
	char* str = NULL;
	SGSFN( "Input_GetPressed" );
	if( !sgs_LoadArgs( C, "s", &str ) )
		return 0;
	InputState* is = Game_FindAction( sgs_GetVar<StringView>()( C, 0 ) );
	if( !is )
		return sgs_Msg( C, SGS_WARNING, "action not found: %s", str );
	return sgs_PushBool( C, is->IsPressed() );
}

static int Input_GetReleased( SGS_CTX )
{
	char* str = NULL;
	SGSFN( "Input_GetReleased" );
	if( !sgs_LoadArgs( C, "s", &str ) )
		return 0;
	InputState* is = Game_FindAction( sgs_GetVar<StringView>()( C, 0 ) );
	if( !is )
		return sgs_Msg( C, SGS_WARNING, "action not found: %s", str );
	return sgs_PushBool( C, is->IsReleased() );
}

static int Input_GetCursorPos( SGS_CTX )
{
	SGSFN( "Input_GetCursorPos" );
	sgs_PushVar( C, Game_GetCursorPos() );
	return 1;
}

static sgs_RegFuncConst g_engine_rfc[] =
{
	{ "Game_GetScreenSize", sgsGame_GetScreenSize },
	{ "Game_ShowCursor", sgsGame_ShowCursor },
	{ "Game_End", sgsGame_End },
	{ "Input_GetValue", Input_GetValue },
	{ "Input_GetState", Input_GetState },
	{ "Input_GetPressed", Input_GetPressed },
	{ "Input_GetReleased", Input_GetReleased },
	{ "Input_GetCursorPos", Input_GetCursorPos },
	SGS_RC_END(),
};


static int BR_Reset( SGS_CTX )
{
	SGSFN( "BR_Reset" );
	GR2D_GetBatchRenderer().Reset();
	return 0;
}
static int BR_Flush( SGS_CTX )
{
	SGSFN( "BR_Flush" );
	GR2D_GetBatchRenderer().Flush();
	return 0;
}
static int BR_SetZEnable( SGS_CTX )
{
	sgs_Bool b;
	SGSFN( "BR_SetZEnable" );
	if( !sgs_LoadArgs( C, "b", &b ) )
		return 0;
	GR2D_GetBatchRenderer().RenderState.depthEnable = b != 0;
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
static int BR_ConeOutline( SGS_CTX )
{
	Vec3 pos, dir, up;
	float radius;
	Vec2 angles;
	int32_t verts;
	SGSFN( "BR_ConeOutline" );
	if( !sgs_LoadArgs( C, "xxxfxl",
		sgs_ArgCheck_Vec3, &pos.x,
		sgs_ArgCheck_Vec3, &dir.x,
		sgs_ArgCheck_Vec3, &up.x,
		&radius,
		sgs_ArgCheck_Vec2, &angles.x,
		&verts ) )
		return 0;
	GR2D_GetBatchRenderer().ConeOutline( pos, dir, up, radius, angles, verts );
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
static sgs_RegFuncConst g_batchrenderer_rfc[] =
{
	{ "BR_Reset", BR_Reset },
	{ "BR_Flush", BR_Flush },
	{ "BR_SetZEnable", BR_SetZEnable },
	{ "BR_Col", BR_Col },
	{ "BR_Pos", BR_Pos },
	{ "BR_SphereOutline", BR_SphereOutline },
	{ "BR_ConeOutline", BR_ConeOutline },
	{ "BR_AABB", BR_AABB },
	{ "BR_Tick", BR_Tick },
	{ "BR_SetPrimitiveType", BR_SetPrimitiveType },
	SGS_RC_END(),
};
static sgs_RegIntConst g_batchrenderer_ric[] =
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
	sgs_LoadLib_String( C );
	sgs_LoadLib_Math( C );
	xgm_module_entry_point( C );
	sgrx_math_lib( C );
	sgs_SetScriptFSFunc( C, sgs_scriptfs_sgrx, NULL );
}

void ScriptContext::RegisterEngineCoreAPI()
{
	sgs_RegFuncConsts( C, g_engine_rfc, -1 );
}

void ScriptContext::RegisterBatchRenderer()
{
	sgs_RegFuncConsts( C, g_batchrenderer_rfc, -1 );
	sgs_RegIntConsts( C, g_batchrenderer_ric, -1 );
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
	sgs_Serialize( C, var.var );
	return sgs_GetVar<String>()( C, -1 );
}

sgsVariable ScriptContext::Unserialize( const StringView& sv )
{
	SGS_SCOPE;
	sgsVariable out;
	sgs_PushStringBuf( C, sv.data(), sv.size() );
	if( sgs_Unserialize( C, sgs_StackItem( C, -1 ) ) )
	{
		out = sgs_GetVar<sgsVariable>()( C, -1 );
	}
	return out;
}

String ScriptContext::ToSGSON( sgsVariable var, const char* tab )
{
	sgs_SerializeSGSON( C, var.var, tab );
	String out = sgsVariable( C, -1 ).get<StringView>();
	sgs_Pop( C, 1 );
	return out;
}

sgsVariable ScriptContext::ParseSGSON( const StringView& sv )
{
	sgs_UnserializeSGSONExt( C, sv.data(), sv.size() );
	return sgsVariable( C, sgsVariable::PickAndPop );
}

sgsString ScriptContext::CreateString( const StringView& sv )
{
	return sgsString( C, sv.data(), sv.size() );
}

sgsVariable ScriptContext::CreateStringVar( const StringView& sv )
{
	return sgsVariable().set( CreateString( sv ) );
}

sgsVariable ScriptContext::CreateArray( int args )
{
	sgsVariable out( C );
	sgs_CreateArray( C, &out.var, args );
	return out;
}

sgsVariable ScriptContext::CreateDict( int args )
{
	sgsVariable out( C );
	sgs_CreateDict( C, &out.var, args );
	return out;
}

sgsVariable ScriptContext::CreateVec2( const Vec2& v )
{
	sgsVariable out( C );
	sgs_CreateVec2( C, &out.var, v.x, v.y );
	return out;
}

sgsVariable ScriptContext::CreateVec3( const Vec3& v )
{
	sgsVariable out( C );
	sgs_CreateVec3( C, &out.var, v.x, v.y, v.z );
	return out;
}

sgsVariable ScriptContext::CreateQuat( const Quat& v )
{
	sgsVariable out( C );
	sgs_CreateQuat( C, &out.var, v.x, v.y, v.z, v.w );
	return out;
}

sgsVariable ScriptContext::CreateMat4( const Mat4& v )
{
	sgsVariable out( C );
	sgs_CreateMat4( C, &out.var, v.a, 0 );
	return out;
}

sgsVariable ScriptContext::Registry()
{
	sgsVariable out( C );
	out.var = sgs_Registry( C, SGS_REG_ROOT );
	out._acquire();
	return out;
}

sgsVariable ScriptContext::GetGlobal( const StringView& name )
{
	return GetGlobal( sgsString( C, name.data(), name.size() ) );
}

sgsVariable ScriptContext::GetGlobal( sgsString name )
{
	sgsVariable key = name.get_variable();
	sgsVariable out( C );
	sgs_GetGlobal( C, key.var, &out.var );
	return out;
}

void ScriptContext::SetGlobal( const StringView& name, sgsVariable val )
{
	sgsVariable key = sgsString( C, name.data(), name.size() ).get_variable();
	sgs_SetGlobal( C, key.var, val.var );
}

void ScriptContext::Call( sgsVariable func, int args, int ret )
{
	sgs_Call( C, func.var, args, ret );
}

bool ScriptContext::GlobalCall( StringView name, int args, int ret )
{
	sgsVariable func = GetGlobal( name );
	if( sgs_IsCallableP( &func.var ) == false )
		LOG_WARNING << "Failed to retrieve callable global from " << name;
	if( func.not_null() )
		sgs_Call( C, func.var, args, ret );
	return func.not_null();
}

void ScriptContext::PushEnv()
{
	sgs_Variable cur_env, new_env;
	
	sgs_GetEnv( C, &cur_env );
	sgs_CreateDict( C, &new_env, 0 );
	sgs_ObjSetMetaObj( C, sgs_GetObjectStructP( &new_env ), sgs_GetObjectStructP( &cur_env ) );
	sgs_SetEnv( C, new_env );
	sgs_Release( C, &new_env );
	sgs_Release( C, &cur_env );
}

bool ScriptContext::PopEnv()
{
	sgs_Variable cur_env;
	
	sgs_GetEnv( C, &cur_env );
	sgs_VarObj* upper_env = sgs_ObjGetMetaObj( sgs_GetObjectStructP( &cur_env ) );
	if( upper_env )
	{
		sgs_Variable new_env;
		sgs_InitObjectPtr( &new_env, upper_env );
		
		sgs_ObjSetMetaObj( C, sgs_GetObjectStructP( &cur_env ), NULL );
		
		sgs_SetEnv( C, new_env );
		sgs_Release( C, &new_env );
		return true;
	}
	sgs_Release( C, &cur_env );
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
