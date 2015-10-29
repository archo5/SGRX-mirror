// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "entities_ts.hpp"

static int _sgs_method__TSCamera__CallEvent( SGS_CTX )
{
	TSCamera* data; if( !SGS_PARSE_METHOD( C, TSCamera::_sgs_interface, data, TSCamera, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int TSCamera::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSCamera*>( obj->data )->~TSCamera();
	return SGS_SUCCESS;
}

int TSCamera::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int TSCamera::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__TSCamera__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_moveTime ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_pauseTime ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_fov ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCamera::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<TSCamera*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ static_cast<TSCamera*>( obj->data )->m_moveTime = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ static_cast<TSCamera*>( obj->data )->m_pauseTime = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ static_cast<TSCamera*>( obj->data )->m_fov = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCamera::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 40 ];
	sprintf( bfr, "TSCamera (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface TSCamera::_sgs_interface[1] =
{{
	"TSCamera",
	TSCamera::_sgs_destruct, TSCamera::_sgs_gcmark, TSCamera::_sgs_getindex, TSCamera::_sgs_setindex, NULL, NULL, TSCamera::_sgs_dump, NULL, NULL, NULL, 
}};


int TSCharacter::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSCharacter*>( obj->data )->~TSCharacter();
	return SGS_SUCCESS;
}

int TSCharacter::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int TSCharacter::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 43 ];
	sprintf( bfr, "TSCharacter (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		sgs_StringConcat( C, 0 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface TSCharacter::_sgs_interface[1] =
{{
	"TSCharacter",
	TSCharacter::_sgs_destruct, TSCharacter::_sgs_gcmark, TSCharacter::_sgs_getindex, TSCharacter::_sgs_setindex, NULL, NULL, TSCharacter::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__TSEnemyController__HasFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, HasFact ) ) return 0;
	sgs_PushVar(C,data->sgsHasFact( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__HasRecentFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, HasRecentFact ) ) return 0;
	sgs_PushVar(C,data->sgsHasRecentFact( sgs_GetVar<int>()(C,0), sgs_GetVar<TimeVal>()(C,1) )); return 1;
}

static int _sgs_method__TSEnemyController__GetRecentFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetRecentFact ) ) return 0;
	return data->sgsGetRecentFact( sgs_GetVar<int>()(C,0), sgs_GetVar<TimeVal>()(C,1) );
}

int TSEnemyController::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSEnemyController*>( obj->data )->~TSEnemyController();
	return SGS_SUCCESS;
}

int TSEnemyController::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int TSEnemyController::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "state" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->m_enemyState ); return SGS_SUCCESS; }
		SGS_CASE( "HasFact" ){ sgs_PushCFunction( C, _sgs_method__TSEnemyController__HasFact ); return SGS_SUCCESS; }
		SGS_CASE( "HasRecentFact" ){ sgs_PushCFunction( C, _sgs_method__TSEnemyController__HasRecentFact ); return SGS_SUCCESS; }
		SGS_CASE( "GetRecentFact" ){ sgs_PushCFunction( C, _sgs_method__TSEnemyController__GetRecentFact ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSEnemyController::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int TSEnemyController::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 49 ];
	sprintf( bfr, "TSEnemyController (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		sgs_StringConcat( C, 0 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface TSEnemyController::_sgs_interface[1] =
{{
	"TSEnemyController",
	TSEnemyController::_sgs_destruct, TSEnemyController::_sgs_gcmark, TSEnemyController::_sgs_getindex, TSEnemyController::_sgs_setindex, NULL, NULL, TSEnemyController::_sgs_dump, NULL, NULL, NULL, 
}};

