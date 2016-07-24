// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "entities_ts.hpp"

static int _sgs_method__TSCharacter__SendMessage( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, SendMessage ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSendMessage( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<sgsVariable>()(C,1) ); return 0;
}

static int _sgs_method__TSCharacter__InitializeMesh( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, InitializeMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->InitializeMesh( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__TSCharacter__IsTouchingPoint( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, IsTouchingPoint ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->IsTouchingPoint( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2) )); return 1;
}

static int _sgs_method__TSCharacter__IsPlayingAnim( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, IsPlayingAnim ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->IsPlayingAnim(  )); return 1;
}

static int _sgs_method__TSCharacter__PlayAnim( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, PlayAnim ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PlayAnim( sgs_GetVar<StringView>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__TSCharacter__StopAnim( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, StopAnim ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->StopAnim(  ); return 0;
}

static int _sgs_method__TSCharacter__PlayPickupAnim( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, PlayPickupAnim ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PlayPickupAnim( sgs_GetVar<Vec3>()(C,0) ); return 0;
}

static int _sgs_method__TSCharacter__SetSkin( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, SetSkin ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->SetSkin( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__TSCharacter__SetACVar( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, SetACVar ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSetACVar( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<float>()(C,1) ); return 0;
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

static int _sgs_method__TSCharacter__Hit( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, Hit ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Hit( sgs_GetVar<float>()(C,0) ); return 0;
}

static int _sgs_method__TSCharacter__GetQueryPosition_FT( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetQueryPosition_FT ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetQueryPosition_FT(  )); return 1;
}

static int _sgs_method__TSCharacter__GetPosition_FT( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetPosition_FT ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetPosition_FT(  )); return 1;
}

static int _sgs_method__TSCharacter__GetViewDir_FT( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetViewDir_FT ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetViewDir_FT(  )); return 1;
}

static int _sgs_method__TSCharacter__GetAimDir_FT( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetAimDir_FT ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetAimDir_FT(  )); return 1;
}

static int _sgs_method__TSCharacter__GetWorldPosition( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetWorldPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetWorldPosition(  )); return 1;
}

static int _sgs_method__TSCharacter__GetQueryPosition( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetQueryPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetQueryPosition(  )); return 1;
}

static int _sgs_method__TSCharacter__GetAimDir( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetAimDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetAimDir(  )); return 1;
}

static int _sgs_method__TSCharacter__GetAttachmentPos( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetAttachmentPos ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsGetAttachmentPos( sgs_GetVar<StringView>()(C,0), sgs_GetVar<Vec3>()(C,1) )); return 1;
}

static int _sgs_method__TSCharacter__GetAttachmentMatrix( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetAttachmentMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsGetAttachmentMatrix( sgs_GetVar<StringView>()(C,0) )); return 1;
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
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "__guid" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_src_guid.ToString() ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "resources" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->_get_resources() ); return SGS_SUCCESS; }
		SGS_CASE( "behaviors" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->_get_behaviors() ); return SGS_SUCCESS; }
		SGS_CASE( "moveRefPos" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->GetMoveRefPos() ); return SGS_SUCCESS; }
		SGS_CASE( "curWeapon" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->sgsCurWeapon() ); return SGS_SUCCESS; }
		SGS_CASE( "health" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_health ); return SGS_SUCCESS; }
		SGS_CASE( "damageMultiplier" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_damageMultiplier ); return SGS_SUCCESS; }
		SGS_CASE( "isCrouching" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_isCrouching ); return SGS_SUCCESS; }
		SGS_CASE( "group" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_group ); return SGS_SUCCESS; }
		SGS_CASE( "timeSinceLastHit" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_timeSinceLastHit ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<TSCharacter*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCharacter*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<TSCharacter*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "health" ){ static_cast<TSCharacter*>( obj->data )->m_health = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "damageMultiplier" ){ static_cast<TSCharacter*>( obj->data )->m_damageMultiplier = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "group" ){ static_cast<TSCharacter*>( obj->data )->m_group = sgs_GetVar<uint32_t>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewDir" ){ static_cast<TSCharacter*>( obj->data )->SetViewDir( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "footPosition" ){ static_cast<TSCharacter*>( obj->data )->SetFootPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<TSCharacter*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
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
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\n__guid = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_src_guid.ToString(), depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\nresources = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->_get_resources(), depth ).push( C ); }
		{ sgs_PushString( C, "\nbehaviors = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->_get_behaviors(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmoveRefPos = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->GetMoveRefPos(), depth ).push( C ); }
		{ sgs_PushString( C, "\ncurWeapon = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->sgsCurWeapon(), depth ).push( C ); }
		{ sgs_PushString( C, "\nhealth = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_health, depth ).push( C ); }
		{ sgs_PushString( C, "\ndamageMultiplier = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_damageMultiplier, depth ).push( C ); }
		{ sgs_PushString( C, "\nisCrouching = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_isCrouching, depth ).push( C ); }
		{ sgs_PushString( C, "\ngroup = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_group, depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeSinceLastHit = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_timeSinceLastHit, depth ).push( C ); }
		sgs_StringConcat( C, 30 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst TSCharacter__sgs_funcs[] =
{
	{ "SendMessage", _sgs_method__TSCharacter__SendMessage },
	{ "InitializeMesh", _sgs_method__TSCharacter__InitializeMesh },
	{ "IsTouchingPoint", _sgs_method__TSCharacter__IsTouchingPoint },
	{ "IsPlayingAnim", _sgs_method__TSCharacter__IsPlayingAnim },
	{ "PlayAnim", _sgs_method__TSCharacter__PlayAnim },
	{ "StopAnim", _sgs_method__TSCharacter__StopAnim },
	{ "PlayPickupAnim", _sgs_method__TSCharacter__PlayPickupAnim },
	{ "SetSkin", _sgs_method__TSCharacter__SetSkin },
	{ "SetACVar", _sgs_method__TSCharacter__SetACVar },
	{ "IsAlive", _sgs_method__TSCharacter__IsAlive },
	{ "Reset", _sgs_method__TSCharacter__Reset },
	{ "Hit", _sgs_method__TSCharacter__Hit },
	{ "GetQueryPosition_FT", _sgs_method__TSCharacter__GetQueryPosition_FT },
	{ "GetPosition_FT", _sgs_method__TSCharacter__GetPosition_FT },
	{ "GetViewDir_FT", _sgs_method__TSCharacter__GetViewDir_FT },
	{ "GetAimDir_FT", _sgs_method__TSCharacter__GetAimDir_FT },
	{ "GetWorldPosition", _sgs_method__TSCharacter__GetWorldPosition },
	{ "GetQueryPosition", _sgs_method__TSCharacter__GetQueryPosition },
	{ "GetAimDir", _sgs_method__TSCharacter__GetAimDir },
	{ "GetAttachmentPos", _sgs_method__TSCharacter__GetAttachmentPos },
	{ "GetAttachmentMatrix", _sgs_method__TSCharacter__GetAttachmentMatrix },
	{ NULL, NULL },
};

static int TSCharacter__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		TSCharacter__sgs_funcs,
		-1, "TSCharacter." );
	return 1;
}

static sgs_ObjInterface TSCharacter__sgs_interface =
{
	"TSCharacter",
	TSCharacter::_sgs_destruct, TSCharacter::_sgs_gcmark, TSCharacter::_sgs_getindex, TSCharacter::_sgs_setindex, NULL, NULL, TSCharacter::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface TSCharacter::_sgs_interface(TSCharacter__sgs_interface, TSCharacter__sgs_ifn, &GOBehavior::_sgs_interface);


static int _sgs_method__TSPlayerController__SendMessage( SGS_CTX )
{
	TSPlayerController* data; if( !SGS_PARSE_METHOD( C, TSPlayerController::_sgs_interface, data, TSPlayerController, SendMessage ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSendMessage( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<sgsVariable>()(C,1) ); return 0;
}

static int _sgs_method__TSPlayerController__GetInput( SGS_CTX )
{
	TSPlayerController* data; if( !SGS_PARSE_METHOD( C, TSPlayerController::_sgs_interface, data, TSPlayerController, GetInput ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInput( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSPlayerController__Reset( SGS_CTX )
{
	TSPlayerController* data; if( !SGS_PARSE_METHOD( C, TSPlayerController::_sgs_interface, data, TSPlayerController, Reset ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Reset(  ); return 0;
}

static int _sgs_method__TSPlayerController__GetInputV3( SGS_CTX )
{
	TSPlayerController* data; if( !SGS_PARSE_METHOD( C, TSPlayerController::_sgs_interface, data, TSPlayerController, GetInputV3 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV3( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSPlayerController__GetInputV2( SGS_CTX )
{
	TSPlayerController* data; if( !SGS_PARSE_METHOD( C, TSPlayerController::_sgs_interface, data, TSPlayerController, GetInputV2 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV2( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSPlayerController__GetInputF( SGS_CTX )
{
	TSPlayerController* data; if( !SGS_PARSE_METHOD( C, TSPlayerController::_sgs_interface, data, TSPlayerController, GetInputF ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputF( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSPlayerController__GetInputB( SGS_CTX )
{
	TSPlayerController* data; if( !SGS_PARSE_METHOD( C, TSPlayerController::_sgs_interface, data, TSPlayerController, GetInputB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputB( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSPlayerController__CalcUIAimInfo( SGS_CTX )
{
	TSPlayerController* data; if( !SGS_PARSE_METHOD( C, TSPlayerController::_sgs_interface, data, TSPlayerController, CalcUIAimInfo ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->CalcUIAimInfo(  ); return 0;
}

int TSPlayerController::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSPlayerController*>( obj->data )->C = C;
	static_cast<TSPlayerController*>( obj->data )->~TSPlayerController();
	return SGS_SUCCESS;
}

int TSPlayerController::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSPlayerController*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int TSPlayerController::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSPlayerController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "__guid" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->m_src_guid.ToString() ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "resources" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->_get_resources() ); return SGS_SUCCESS; }
		SGS_CASE( "behaviors" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->_get_behaviors() ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->enabled ); return SGS_SUCCESS; }
		SGS_CASE( "ahShouldDrawClosestPoint" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->_shouldDrawCP() ); return SGS_SUCCESS; }
		SGS_CASE( "ahClosestPoint" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->m_aimHelper.m_closestPoint ); return SGS_SUCCESS; }
		SGS_CASE( "ahAimPoint" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->m_aimHelper.m_aimPoint ); return SGS_SUCCESS; }
		SGS_CASE( "ahAimFactor" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->m_aimHelper.m_aimFactor ); return SGS_SUCCESS; }
		SGS_CASE( "ahCPDistance" ){ sgs_PushVar( C, static_cast<TSPlayerController*>( obj->data )->m_aimHelper.m_pDist ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<TSPlayerController*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int TSPlayerController::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSPlayerController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<TSPlayerController*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<TSPlayerController*>( obj->data )->enabled = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<TSPlayerController*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int TSPlayerController::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSPlayerController*>( obj->data )->C, C );
	char bfr[ 50 ];
	sprintf( bfr, "TSPlayerController (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\n__guid = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->m_src_guid.ToString(), depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\nresources = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->_get_resources(), depth ).push( C ); }
		{ sgs_PushString( C, "\nbehaviors = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->_get_behaviors(), depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\nahShouldDrawClosestPoint = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->_shouldDrawCP(), depth ).push( C ); }
		{ sgs_PushString( C, "\nahClosestPoint = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->m_aimHelper.m_closestPoint, depth ).push( C ); }
		{ sgs_PushString( C, "\nahAimPoint = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->m_aimHelper.m_aimPoint, depth ).push( C ); }
		{ sgs_PushString( C, "\nahAimFactor = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->m_aimHelper.m_aimFactor, depth ).push( C ); }
		{ sgs_PushString( C, "\nahCPDistance = " ); sgs_DumpData( C, static_cast<TSPlayerController*>( obj->data )->m_aimHelper.m_pDist, depth ).push( C ); }
		sgs_StringConcat( C, 28 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst TSPlayerController__sgs_funcs[] =
{
	{ "SendMessage", _sgs_method__TSPlayerController__SendMessage },
	{ "GetInput", _sgs_method__TSPlayerController__GetInput },
	{ "Reset", _sgs_method__TSPlayerController__Reset },
	{ "GetInputV3", _sgs_method__TSPlayerController__GetInputV3 },
	{ "GetInputV2", _sgs_method__TSPlayerController__GetInputV2 },
	{ "GetInputF", _sgs_method__TSPlayerController__GetInputF },
	{ "GetInputB", _sgs_method__TSPlayerController__GetInputB },
	{ "CalcUIAimInfo", _sgs_method__TSPlayerController__CalcUIAimInfo },
	{ NULL, NULL },
};

static int TSPlayerController__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		TSPlayerController__sgs_funcs,
		-1, "TSPlayerController." );
	return 1;
}

static sgs_ObjInterface TSPlayerController__sgs_interface =
{
	"TSPlayerController",
	TSPlayerController::_sgs_destruct, TSPlayerController::_sgs_gcmark, TSPlayerController::_sgs_getindex, TSPlayerController::_sgs_setindex, NULL, NULL, TSPlayerController::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface TSPlayerController::_sgs_interface(TSPlayerController__sgs_interface, TSPlayerController__sgs_ifn, &BhControllerBase::_sgs_interface);


static int _sgs_method__TPSPlayerController__SendMessage( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, SendMessage ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSendMessage( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<sgsVariable>()(C,1) ); return 0;
}

static int _sgs_method__TPSPlayerController__GetInput( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, GetInput ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInput( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TPSPlayerController__Reset( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, Reset ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Reset(  ); return 0;
}

static int _sgs_method__TPSPlayerController__GetInputV3( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, GetInputV3 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV3( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TPSPlayerController__GetInputV2( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, GetInputV2 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV2( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TPSPlayerController__GetInputF( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, GetInputF ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputF( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TPSPlayerController__GetInputB( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, GetInputB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputB( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TPSPlayerController__Update( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, Update ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Update(  ); return 0;
}

static int _sgs_method__TPSPlayerController__GetCameraPos( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, GetCameraPos ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetCameraPos( sgs_GetVarObj<TSCharacter>()(C,0), sgs_GetVar<bool>()(C,1) )); return 1;
}

static int _sgs_method__TPSPlayerController__UpdateMoveAim( SGS_CTX )
{
	TPSPlayerController* data; if( !SGS_PARSE_METHOD( C, TPSPlayerController::_sgs_interface, data, TPSPlayerController, UpdateMoveAim ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->UpdateMoveAim( sgs_GetVar<bool>()(C,0) ); return 0;
}

int TPSPlayerController::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TPSPlayerController*>( obj->data )->C = C;
	static_cast<TPSPlayerController*>( obj->data )->~TPSPlayerController();
	return SGS_SUCCESS;
}

int TPSPlayerController::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TPSPlayerController*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int TPSPlayerController::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TPSPlayerController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "__guid" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->m_src_guid.ToString() ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "resources" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->_get_resources() ); return SGS_SUCCESS; }
		SGS_CASE( "behaviors" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->_get_behaviors() ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->enabled ); return SGS_SUCCESS; }
		SGS_CASE( "lastFrameReset" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->lastFrameReset ); return SGS_SUCCESS; }
		SGS_CASE( "direction" ){ sgs_PushVar( C, static_cast<TPSPlayerController*>( obj->data )->m_angles.ToVec3() ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<TPSPlayerController*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int TPSPlayerController::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TPSPlayerController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<TPSPlayerController*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<TPSPlayerController*>( obj->data )->enabled = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "lastFrameReset" ){ static_cast<TPSPlayerController*>( obj->data )->lastFrameReset = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "direction" ){ static_cast<TPSPlayerController*>( obj->data )->SetDir( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<TPSPlayerController*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int TPSPlayerController::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TPSPlayerController*>( obj->data )->C, C );
	char bfr[ 51 ];
	sprintf( bfr, "TPSPlayerController (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\n__guid = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->m_src_guid.ToString(), depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\nresources = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->_get_resources(), depth ).push( C ); }
		{ sgs_PushString( C, "\nbehaviors = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->_get_behaviors(), depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\nlastFrameReset = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->lastFrameReset, depth ).push( C ); }
		{ sgs_PushString( C, "\ndirection = " ); sgs_DumpData( C, static_cast<TPSPlayerController*>( obj->data )->m_angles.ToVec3(), depth ).push( C ); }
		sgs_StringConcat( C, 22 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst TPSPlayerController__sgs_funcs[] =
{
	{ "SendMessage", _sgs_method__TPSPlayerController__SendMessage },
	{ "GetInput", _sgs_method__TPSPlayerController__GetInput },
	{ "Reset", _sgs_method__TPSPlayerController__Reset },
	{ "GetInputV3", _sgs_method__TPSPlayerController__GetInputV3 },
	{ "GetInputV2", _sgs_method__TPSPlayerController__GetInputV2 },
	{ "GetInputF", _sgs_method__TPSPlayerController__GetInputF },
	{ "GetInputB", _sgs_method__TPSPlayerController__GetInputB },
	{ "Update", _sgs_method__TPSPlayerController__Update },
	{ "GetCameraPos", _sgs_method__TPSPlayerController__GetCameraPos },
	{ "UpdateMoveAim", _sgs_method__TPSPlayerController__UpdateMoveAim },
	{ NULL, NULL },
};

static int TPSPlayerController__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		TPSPlayerController__sgs_funcs,
		-1, "TPSPlayerController." );
	return 1;
}

static sgs_ObjInterface TPSPlayerController__sgs_interface =
{
	"TPSPlayerController",
	TPSPlayerController::_sgs_destruct, TPSPlayerController::_sgs_gcmark, TPSPlayerController::_sgs_getindex, TPSPlayerController::_sgs_setindex, NULL, NULL, TPSPlayerController::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface TPSPlayerController::_sgs_interface(TPSPlayerController__sgs_interface, TPSPlayerController__sgs_ifn, &BhControllerBase::_sgs_interface);


static int _sgs_method__TSEnemyController__SendMessage( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, SendMessage ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSendMessage( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<sgsVariable>()(C,1) ); return 0;
}

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

static int _sgs_method__TSEnemyController__GetInputV3( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetInputV3 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV3( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__GetInputV2( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetInputV2 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV2( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__GetInputF( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetInputF ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputF( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__TSEnemyController__GetInputB( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetInputB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputB( sgs_GetVar<uint32_t>()(C,0) )); return 1;
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

static int _sgs_method__TSEnemyController__GetCover( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetCover ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	return data->sgsGetCover( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<uint32_t>()(C,2), sgs_GetVar<uint32_t>()(C,3) );
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

static int _sgs_method__TSEnemyController__HasPath( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, HasPath ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsHasPath(  )); return 1;
}

static int _sgs_method__TSEnemyController__GetPathPointCount( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, GetPathPointCount ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsGetPathPointCount(  )); return 1;
}

static int _sgs_method__TSEnemyController__AdvancePath( SGS_CTX )
{
	TSEnemyController* data; if( !SGS_PARSE_METHOD( C, TSEnemyController::_sgs_interface, data, TSEnemyController, AdvancePath ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsAdvancePath( sgs_GetVar<float>()(C,0) )); return 1;
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
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "__guid" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->m_src_guid.ToString() ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "resources" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->_get_resources() ); return SGS_SUCCESS; }
		SGS_CASE( "behaviors" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->_get_behaviors() ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->enabled ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<TSEnemyController*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int TSEnemyController::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSEnemyController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<TSEnemyController*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<TSEnemyController*>( obj->data )->enabled = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<TSEnemyController*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
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
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\n__guid = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->m_src_guid.ToString(), depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\nresources = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->_get_resources(), depth ).push( C ); }
		{ sgs_PushString( C, "\nbehaviors = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->_get_behaviors(), depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->enabled, depth ).push( C ); }
		sgs_StringConcat( C, 18 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst TSEnemyController__sgs_funcs[] =
{
	{ "SendMessage", _sgs_method__TSEnemyController__SendMessage },
	{ "GetInput", _sgs_method__TSEnemyController__GetInput },
	{ "Reset", _sgs_method__TSEnemyController__Reset },
	{ "GetInputV3", _sgs_method__TSEnemyController__GetInputV3 },
	{ "GetInputV2", _sgs_method__TSEnemyController__GetInputV2 },
	{ "GetInputF", _sgs_method__TSEnemyController__GetInputF },
	{ "GetInputB", _sgs_method__TSEnemyController__GetInputB },
	{ "CanSeePoint", _sgs_method__TSEnemyController__CanSeePoint },
	{ "LookingAtPoint", _sgs_method__TSEnemyController__LookingAtPoint },
	{ "HasFact", _sgs_method__TSEnemyController__HasFact },
	{ "HasRecentFact", _sgs_method__TSEnemyController__HasRecentFact },
	{ "GetRecentFact", _sgs_method__TSEnemyController__GetRecentFact },
	{ "InsertFact", _sgs_method__TSEnemyController__InsertFact },
	{ "UpdateFact", _sgs_method__TSEnemyController__UpdateFact },
	{ "InsertOrUpdateFact", _sgs_method__TSEnemyController__InsertOrUpdateFact },
	{ "GetCover", _sgs_method__TSEnemyController__GetCover },
	{ "IsWalkable", _sgs_method__TSEnemyController__IsWalkable },
	{ "FindPath", _sgs_method__TSEnemyController__FindPath },
	{ "HasPath", _sgs_method__TSEnemyController__HasPath },
	{ "GetPathPointCount", _sgs_method__TSEnemyController__GetPathPointCount },
	{ "AdvancePath", _sgs_method__TSEnemyController__AdvancePath },
	{ "GetNextPathPoint", _sgs_method__TSEnemyController__GetNextPathPoint },
	{ "RemoveNextPathPoint", _sgs_method__TSEnemyController__RemoveNextPathPoint },
	{ NULL, NULL },
};

static int TSEnemyController__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		TSEnemyController__sgs_funcs,
		-1, "TSEnemyController." );
	return 1;
}

static sgs_ObjInterface TSEnemyController__sgs_interface =
{
	"TSEnemyController",
	TSEnemyController::_sgs_destruct, TSEnemyController::_sgs_gcmark, TSEnemyController::_sgs_getindex, TSEnemyController::_sgs_setindex, NULL, NULL, TSEnemyController::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface TSEnemyController::_sgs_interface(TSEnemyController__sgs_interface, TSEnemyController__sgs_ifn, &BhControllerBase::_sgs_interface);

