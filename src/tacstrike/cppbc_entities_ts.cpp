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
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__TSCamera__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_moveTime ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_pauseTime ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_fov ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCamera::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<TSCamera*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ static_cast<TSCamera*>( obj->data )->m_moveTime = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ static_cast<TSCamera*>( obj->data )->m_pauseTime = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ static_cast<TSCamera*>( obj->data )->m_fov = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
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
		SGS_CASE( "timeSinceLastHit" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_timeSinceLastHit ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->GetPosition() ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->GetPosition(), depth ).push( C ); }
		sgs_StringConcat( C, 2 );
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


static int _sgs_method__TSEnemyController__CanSeePoint( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, CanSeePoint ) ) return 0;
	sgs_PushVar(C,data->CanSeePoint( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__LookingAtPoint( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, LookingAtPoint ) ) return 0;
	sgs_PushVar(C,data->LookingAtPoint( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__HasFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, HasFact ) ) return 0;
	sgs_PushVar(C,data->sgsHasFact( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__HasRecentFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, HasRecentFact ) ) return 0;
	sgs_PushVar(C,data->sgsHasRecentFact( sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<TimeVal>()(C,1) )); return 1;
}

static int _sgs_method__TSEnemyController__GetRecentFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetRecentFact ) ) return 0;
	return data->sgsGetRecentFact( C, sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<TimeVal>()(C,1) );
}

static int _sgs_method__TSEnemyController__InsertFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, InsertFact ) ) return 0;
	data->sgsInsertFact( sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<TimeVal>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<uint32_t>()(C,4) ); return 0;
}

static int _sgs_method__TSEnemyController__UpdateFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, UpdateFact ) ) return 0;
	sgs_PushVar(C,data->sgsUpdateFact( C, sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<TimeVal>()(C,4), sgs_GetVar<uint32_t>()(C,5), sgs_GetVar<bool>()(C,6) )); return 1;
}

static int _sgs_method__TSEnemyController__InsertOrUpdateFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, InsertOrUpdateFact ) ) return 0;
	data->sgsInsertOrUpdateFact( C, sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<TimeVal>()(C,4), sgs_GetVar<uint32_t>()(C,5), sgs_GetVar<bool>()(C,6) ); return 0;
}

static int _sgs_method__TSEnemyController__MovingUpdateFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, MovingUpdateFact ) ) return 0;
	sgs_PushVar(C,data->sgsMovingUpdateFact( C, sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<TimeVal>()(C,4), sgs_GetVar<uint32_t>()(C,5), sgs_GetVar<bool>()(C,6) )); return 1;
}

static int _sgs_method__TSEnemyController__MovingInsertOrUpdateFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, MovingInsertOrUpdateFact ) ) return 0;
	data->sgsMovingInsertOrUpdateFact( C, sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<TimeVal>()(C,4), sgs_GetVar<uint32_t>()(C,5), sgs_GetVar<bool>()(C,6) ); return 0;
}

static int _sgs_method__TSEnemyController__QueryCoverLines( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, QueryCoverLines ) ) return 0;
	data->sgsQueryCoverLines( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<Vec3>()(C,4), sgs_GetVar<bool>()(C,5) ); return 0;
}

static int _sgs_method__TSEnemyController__GetCoverPosition( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetCoverPosition ) ) return 0;
	sgs_PushVar(C,data->sgsGetCoverPosition( C, sgs_GetVar<Vec3>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2) )); return 1;
}

static int _sgs_method__TSEnemyController__IsWalkable( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, IsWalkable ) ) return 0;
	sgs_PushVar(C,data->sgsIsWalkable( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<Vec3>()(C,1) )); return 1;
}

static int _sgs_method__TSEnemyController__FindPath( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, FindPath ) ) return 0;
	sgs_PushVar(C,data->sgsFindPath( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__GetNextPathPoint( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetNextPathPoint ) ) return 0;
	sgs_PushVar(C,data->sgsGetNextPathPoint(  )); return 1;
}

static int _sgs_method__TSEnemyController__RemoveNextPathPoint( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, RemoveNextPathPoint ) ) return 0;
	sgs_PushVar(C,data->sgsRemoveNextPathPoint(  )); return 1;
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
		SGS_CASE( "CanSeePoint" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__CanSeePoint ); return SGS_SUCCESS; }
		SGS_CASE( "LookingAtPoint" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__LookingAtPoint ); return SGS_SUCCESS; }
		SGS_CASE( "HasFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__HasFact ); return SGS_SUCCESS; }
		SGS_CASE( "HasRecentFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__HasRecentFact ); return SGS_SUCCESS; }
		SGS_CASE( "GetRecentFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__GetRecentFact ); return SGS_SUCCESS; }
		SGS_CASE( "InsertFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__InsertFact ); return SGS_SUCCESS; }
		SGS_CASE( "UpdateFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__UpdateFact ); return SGS_SUCCESS; }
		SGS_CASE( "InsertOrUpdateFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__InsertOrUpdateFact ); return SGS_SUCCESS; }
		SGS_CASE( "MovingUpdateFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__MovingUpdateFact ); return SGS_SUCCESS; }
		SGS_CASE( "MovingInsertOrUpdateFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__MovingInsertOrUpdateFact ); return SGS_SUCCESS; }
		SGS_CASE( "QueryCoverLines" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__QueryCoverLines ); return SGS_SUCCESS; }
		SGS_CASE( "GetCoverPosition" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__GetCoverPosition ); return SGS_SUCCESS; }
		SGS_CASE( "IsWalkable" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__IsWalkable ); return SGS_SUCCESS; }
		SGS_CASE( "FindPath" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__FindPath ); return SGS_SUCCESS; }
		SGS_CASE( "GetNextPathPoint" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__GetNextPathPoint ); return SGS_SUCCESS; }
		SGS_CASE( "RemoveNextPathPoint" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__RemoveNextPathPoint ); return SGS_SUCCESS; }
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

