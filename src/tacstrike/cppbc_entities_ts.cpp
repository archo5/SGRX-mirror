// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "entities_ts.hpp"

static int _sgs_method__TSCamera__CallEvent( SGS_CTX )
{
	TSCamera* data; if( !SGS_PARSE_METHOD( C, TSCamera::_sgs_interface, data, TSCamera, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int TSCamera::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSCamera*>( obj->data )->C = C;
	static_cast<TSCamera*>( obj->data )->~TSCamera();
	return SGS_SUCCESS;
}

int TSCamera::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCamera*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int TSCamera::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCamera*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__TSCamera__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_moveTime ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_pauseTime ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_fov ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCamera::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCamera*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<TSCamera*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ static_cast<TSCamera*>( obj->data )->m_moveTime = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ static_cast<TSCamera*>( obj->data )->m_pauseTime = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ static_cast<TSCamera*>( obj->data )->m_fov = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCamera::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCamera*>( obj->data )->C, C );
	char bfr[ 40 ];
	sprintf( bfr, "TSCamera (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nmoveTime = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_moveTime, depth ).push( C ); }
		{ sgs_PushString( C, "\npauseTime = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_pauseTime, depth ).push( C ); }
		{ sgs_PushString( C, "\nfov = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_fov, depth ).push( C ); }
		sgs_StringConcat( C, 14 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface TSCamera__sgs_interface =
{
	"TSCamera",
	TSCamera::_sgs_destruct, TSCamera::_sgs_gcmark, TSCamera::_sgs_getindex, TSCamera::_sgs_setindex, NULL, NULL, TSCamera::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface TSCamera::_sgs_interface(TSCamera__sgs_interface, &Entity::_sgs_interface);


static int _sgs_method__TSCharacter__CallEvent( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__TSCharacter__GetInputV3( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetInputV3 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV3( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSCharacter__GetInputV2( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetInputV2 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV2( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSCharacter__GetInputF( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetInputF ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputF( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSCharacter__GetInputB( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetInputB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputB( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSCharacter__IsAlive( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, IsAlive ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->IsAlive(  )); return 1;
}

static int _sgs_method__TSCharacter__Reset( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, Reset ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Reset(  ); return 0;
}

static int _sgs_method__TSCharacter__GetAttachmentPos( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetAttachmentPos ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsGetAttachmentPos( sgs_GetVar<StringView>()(C,0), sgs_GetVar<Vec3>()(C,1) )); return 1;
}

int TSCharacter::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSCharacter*>( obj->data )->C = C;
	static_cast<TSCharacter*>( obj->data )->~TSCharacter();
	return SGS_SUCCESS;
}

int TSCharacter::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCharacter*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int TSCharacter::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCharacter*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__TSCharacter__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "GetInputV3" ){ sgs_PushCFunc( C, _sgs_method__TSCharacter__GetInputV3 ); return SGS_SUCCESS; }
		SGS_CASE( "GetInputV2" ){ sgs_PushCFunc( C, _sgs_method__TSCharacter__GetInputV2 ); return SGS_SUCCESS; }
		SGS_CASE( "GetInputF" ){ sgs_PushCFunc( C, _sgs_method__TSCharacter__GetInputF ); return SGS_SUCCESS; }
		SGS_CASE( "GetInputB" ){ sgs_PushCFunc( C, _sgs_method__TSCharacter__GetInputB ); return SGS_SUCCESS; }
		SGS_CASE( "IsAlive" ){ sgs_PushCFunc( C, _sgs_method__TSCharacter__IsAlive ); return SGS_SUCCESS; }
		SGS_CASE( "Reset" ){ sgs_PushCFunc( C, _sgs_method__TSCharacter__Reset ); return SGS_SUCCESS; }
		SGS_CASE( "ctrl" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->_getCtrl() ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->GetPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "timeSinceLastHit" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_timeSinceLastHit ); return SGS_SUCCESS; }
		SGS_CASE( "GetAttachmentPos" ){ sgs_PushCFunc( C, _sgs_method__TSCharacter__GetAttachmentPos ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCharacter*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<TSCharacter*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<TSCharacter*>( obj->data )->SetPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCharacter*>( obj->data )->C, C );
	char bfr[ 43 ];
	sprintf( bfr, "TSCharacter (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nctrl = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->_getCtrl(), depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->GetPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeSinceLastHit = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_timeSinceLastHit, depth ).push( C ); }
		sgs_StringConcat( C, 14 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface TSCharacter__sgs_interface =
{
	"TSCharacter",
	TSCharacter::_sgs_destruct, TSCharacter::_sgs_gcmark, TSCharacter::_sgs_getindex, TSCharacter::_sgs_setindex, NULL, NULL, TSCharacter::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface TSCharacter::_sgs_interface(TSCharacter__sgs_interface, &Actor::_sgs_interface);


static int _sgs_method__TSEnemyController__GetInput( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetInput ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInput( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__Reset( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, Reset ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Reset(  ); return 0;
}

static int _sgs_method__TSEnemyController__CanSeePoint( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, CanSeePoint ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->CanSeePoint( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__LookingAtPoint( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, LookingAtPoint ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->LookingAtPoint( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__HasFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, HasFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsHasFact( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__HasRecentFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, HasRecentFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsHasRecentFact( sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<TimeVal>()(C,1) )); return 1;
}

static int _sgs_method__TSEnemyController__GetRecentFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetRecentFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	return data->sgsGetRecentFact( sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<TimeVal>()(C,1) );
}

static int _sgs_method__TSEnemyController__InsertFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, InsertFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsInsertFact( sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<TimeVal>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<uint32_t>()(C,4) ); return 0;
}

static int _sgs_method__TSEnemyController__UpdateFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, UpdateFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsUpdateFact( sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<TimeVal>()(C,4), sgs_GetVar<uint32_t>()(C,5), sgs_GetVar<bool>()(C,6) )); return 1;
}

static int _sgs_method__TSEnemyController__InsertOrUpdateFact( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, InsertOrUpdateFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsInsertOrUpdateFact( sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<TimeVal>()(C,4), sgs_GetVar<uint32_t>()(C,5), sgs_GetVar<bool>()(C,6) ); return 0;
}

static int _sgs_method__TSEnemyController__QueryCoverLines( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, QueryCoverLines ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsQueryCoverLines( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<Vec3>()(C,4), sgs_GetVar<bool>()(C,5) ); return 0;
}

static int _sgs_method__TSEnemyController__GetCoverPosition( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetCoverPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsGetCoverPosition( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2) )); return 1;
}

static int _sgs_method__TSEnemyController__IsWalkable( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, IsWalkable ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsIsWalkable( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<Vec3>()(C,1) )); return 1;
}

static int _sgs_method__TSEnemyController__FindPath( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, FindPath ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsFindPath( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__GetNextPathPoint( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetNextPathPoint ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsGetNextPathPoint(  )); return 1;
}

static int _sgs_method__TSEnemyController__RemoveNextPathPoint( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, RemoveNextPathPoint ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsRemoveNextPathPoint(  )); return 1;
}

int TSEnemyController::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSEnemyController*>( obj->data )->C = C;
	static_cast<TSEnemyController*>( obj->data )->~TSEnemyController();
	return SGS_SUCCESS;
}

int TSEnemyController::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSEnemyController*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int TSEnemyController::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSEnemyController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "GetInput" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__GetInput ); return SGS_SUCCESS; }
		SGS_CASE( "Reset" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__Reset ); return SGS_SUCCESS; }
		SGS_CASE( "state" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->m_enemyState ); return SGS_SUCCESS; }
		SGS_CASE( "CanSeePoint" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__CanSeePoint ); return SGS_SUCCESS; }
		SGS_CASE( "LookingAtPoint" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__LookingAtPoint ); return SGS_SUCCESS; }
		SGS_CASE( "HasFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__HasFact ); return SGS_SUCCESS; }
		SGS_CASE( "HasRecentFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__HasRecentFact ); return SGS_SUCCESS; }
		SGS_CASE( "GetRecentFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__GetRecentFact ); return SGS_SUCCESS; }
		SGS_CASE( "InsertFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__InsertFact ); return SGS_SUCCESS; }
		SGS_CASE( "UpdateFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__UpdateFact ); return SGS_SUCCESS; }
		SGS_CASE( "InsertOrUpdateFact" ){ sgs_PushCFunc( C, _sgs_method__TSEnemyController__InsertOrUpdateFact ); return SGS_SUCCESS; }
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
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSEnemyController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int TSEnemyController::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSEnemyController*>( obj->data )->C, C );
	char bfr[ 49 ];
	sprintf( bfr, "TSEnemyController (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\nstate = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->m_enemyState, depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface TSEnemyController__sgs_interface =
{
	"TSEnemyController",
	NULL, TSEnemyController::_sgs_gcmark, TSEnemyController::_sgs_getindex, TSEnemyController::_sgs_setindex, NULL, NULL, TSEnemyController::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface TSEnemyController::_sgs_interface(TSEnemyController__sgs_interface, &IActorController::_sgs_interface);

