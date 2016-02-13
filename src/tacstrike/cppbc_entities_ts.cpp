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
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_transform.position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_moveTime ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_pauseTime ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_fov ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<TSCamera*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int TSCamera::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCamera*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<TSCamera*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<TSCamera*>( obj->data )->m_transform.position = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<TSCamera*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<TSCamera*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<TSCamera*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<TSCamera*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<TSCamera*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<TSCamera*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<TSCamera*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<TSCamera*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<TSCamera*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<TSCamera*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ static_cast<TSCamera*>( obj->data )->m_moveTime = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ static_cast<TSCamera*>( obj->data )->m_pauseTime = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ static_cast<TSCamera*>( obj->data )->m_fov = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<TSCamera*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
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
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_transform.position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nmoveTime = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_moveTime, depth ).push( C ); }
		{ sgs_PushString( C, "\npauseTime = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_pauseTime, depth ).push( C ); }
		{ sgs_PushString( C, "\nfov = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->m_fov, depth ).push( C ); }
		sgs_StringConcat( C, 26 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst TSCamera__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__TSCamera__CallEvent },
	{ NULL, NULL },
};

static int TSCamera__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		TSCamera__sgs_funcs,
		-1, "TSCamera." );
	return 1;
}

static sgs_ObjInterface TSCamera__sgs_interface =
{
	"TSCamera",
	TSCamera::_sgs_destruct, TSCamera::_sgs_gcmark, TSCamera::_sgs_getindex, TSCamera::_sgs_setindex, NULL, NULL, TSCamera::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface TSCamera::_sgs_interface(TSCamera__sgs_interface, TSCamera__sgs_ifn, &Entity::_sgs_interface);


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

static int _sgs_method__TSCharacter__SetPlayerMode( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, SetPlayerMode ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->SetPlayerMode( sgs_GetVar<bool>()(C,0) ); return 0;
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

static int _sgs_method__TSCharacter__GetViewDir( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, GetViewDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetViewDir(  )); return 1;
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
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->GetPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "ctrl" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->_getCtrl() ); return SGS_SUCCESS; }
		SGS_CASE( "timeSinceLastHit" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_timeSinceLastHit ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<TSCharacter*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSCharacter*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<TSCharacter*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<TSCharacter*>( obj->data )->SetPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<TSCharacter*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<TSCharacter*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<TSCharacter*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<TSCharacter*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<TSCharacter*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<TSCharacter*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<TSCharacter*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<TSCharacter*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<TSCharacter*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->GetPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nctrl = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->_getCtrl(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeSinceLastHit = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->m_timeSinceLastHit, depth ).push( C ); }
		sgs_StringConcat( C, 24 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst TSCharacter__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__TSCharacter__CallEvent },
	{ "GetInputV3", _sgs_method__TSCharacter__GetInputV3 },
	{ "GetInputV2", _sgs_method__TSCharacter__GetInputV2 },
	{ "GetInputF", _sgs_method__TSCharacter__GetInputF },
	{ "GetInputB", _sgs_method__TSCharacter__GetInputB },
	{ "IsAlive", _sgs_method__TSCharacter__IsAlive },
	{ "Reset", _sgs_method__TSCharacter__Reset },
	{ "SetPlayerMode", _sgs_method__TSCharacter__SetPlayerMode },
	{ "IsPlayingAnim", _sgs_method__TSCharacter__IsPlayingAnim },
	{ "PlayAnim", _sgs_method__TSCharacter__PlayAnim },
	{ "StopAnim", _sgs_method__TSCharacter__StopAnim },
	{ "GetViewDir", _sgs_method__TSCharacter__GetViewDir },
	{ "GetAimDir", _sgs_method__TSCharacter__GetAimDir },
	{ "GetAttachmentPos", _sgs_method__TSCharacter__GetAttachmentPos },
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
_sgsInterface TSCharacter::_sgs_interface(TSCharacter__sgs_interface, TSCharacter__sgs_ifn, &Actor::_sgs_interface);


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
		SGS_CASE( "inPlayerTeam" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->m_inPlayerTeam ); return SGS_SUCCESS; }
		SGS_CASE( "state" ){ sgs_PushVar( C, static_cast<TSEnemyController*>( obj->data )->m_enemyState ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<TSEnemyController*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int TSEnemyController::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<TSEnemyController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<TSEnemyController*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "inPlayerTeam" ){ static_cast<TSEnemyController*>( obj->data )->m_inPlayerTeam = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\ninPlayerTeam = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->m_inPlayerTeam, depth ).push( C ); }
		{ sgs_PushString( C, "\nstate = " ); sgs_DumpData( C, static_cast<TSEnemyController*>( obj->data )->m_enemyState, depth ).push( C ); }
		sgs_StringConcat( C, 8 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst TSEnemyController__sgs_funcs[] =
{
	{ "GetInput", _sgs_method__TSEnemyController__GetInput },
	{ "Reset", _sgs_method__TSEnemyController__Reset },
	{ "CanSeePoint", _sgs_method__TSEnemyController__CanSeePoint },
	{ "LookingAtPoint", _sgs_method__TSEnemyController__LookingAtPoint },
	{ "HasFact", _sgs_method__TSEnemyController__HasFact },
	{ "HasRecentFact", _sgs_method__TSEnemyController__HasRecentFact },
	{ "GetRecentFact", _sgs_method__TSEnemyController__GetRecentFact },
	{ "InsertFact", _sgs_method__TSEnemyController__InsertFact },
	{ "UpdateFact", _sgs_method__TSEnemyController__UpdateFact },
	{ "InsertOrUpdateFact", _sgs_method__TSEnemyController__InsertOrUpdateFact },
	{ "QueryCoverLines", _sgs_method__TSEnemyController__QueryCoverLines },
	{ "GetCoverPosition", _sgs_method__TSEnemyController__GetCoverPosition },
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
	NULL, TSEnemyController::_sgs_gcmark, TSEnemyController::_sgs_getindex, TSEnemyController::_sgs_setindex, NULL, NULL, TSEnemyController::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface TSEnemyController::_sgs_interface(TSEnemyController__sgs_interface, TSEnemyController__sgs_ifn, &IActorController::_sgs_interface);

