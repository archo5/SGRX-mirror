// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "isr3entities.hpp"

static int _sgs_method__ISR3Drone__CallEvent( SGS_CTX )
{
	ISR3Drone* data; if( !SGS_PARSE_METHOD( C, ISR3Drone::_sgs_interface, data, ISR3Drone, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int ISR3Drone::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ISR3Drone*>( obj->data )->C = C;
	static_cast<ISR3Drone*>( obj->data )->~ISR3Drone();
	return SGS_SUCCESS;
}

int ISR3Drone::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Drone*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ISR3Drone::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Drone*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_transform.position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<ISR3Drone*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Drone::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Drone*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ISR3Drone*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ISR3Drone*>( obj->data )->m_transform.position = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<ISR3Drone*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<ISR3Drone*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<ISR3Drone*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<ISR3Drone*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<ISR3Drone*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<ISR3Drone*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Drone::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Drone*>( obj->data )->C, C );
	char bfr[ 41 ];
	sprintf( bfr, "ISR3Drone (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_transform.position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_viewName, depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ISR3Drone__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__ISR3Drone__CallEvent },
	{ NULL, NULL },
};

static int ISR3Drone__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ISR3Drone__sgs_funcs,
		-1, "ISR3Drone." );
	return 1;
}

static sgs_ObjInterface ISR3Drone__sgs_interface =
{
	"ISR3Drone",
	ISR3Drone::_sgs_destruct, ISR3Drone::_sgs_gcmark, ISR3Drone::_sgs_getindex, ISR3Drone::_sgs_setindex, NULL, NULL, ISR3Drone::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ISR3Drone::_sgs_interface(ISR3Drone__sgs_interface, ISR3Drone__sgs_ifn, &Entity::_sgs_interface);


static int _sgs_method__ISR3Player__CallEvent( SGS_CTX )
{
	ISR3Player* data; if( !SGS_PARSE_METHOD( C, ISR3Player::_sgs_interface, data, ISR3Player, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int ISR3Player::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ISR3Player*>( obj->data )->C = C;
	static_cast<ISR3Player*>( obj->data )->~ISR3Player();
	return SGS_SUCCESS;
}

int ISR3Player::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Player*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ISR3Player::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Player*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_transform.position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<ISR3Player*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Player::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Player*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ISR3Player*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ISR3Player*>( obj->data )->m_transform.position = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<ISR3Player*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<ISR3Player*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<ISR3Player*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<ISR3Player*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<ISR3Player*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<ISR3Player*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Player::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Player*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "ISR3Player (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_transform.position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_viewName, depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ISR3Player__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__ISR3Player__CallEvent },
	{ NULL, NULL },
};

static int ISR3Player__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ISR3Player__sgs_funcs,
		-1, "ISR3Player." );
	return 1;
}

static sgs_ObjInterface ISR3Player__sgs_interface =
{
	"ISR3Player",
	ISR3Player::_sgs_destruct, ISR3Player::_sgs_gcmark, ISR3Player::_sgs_getindex, ISR3Player::_sgs_setindex, NULL, NULL, ISR3Player::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ISR3Player::_sgs_interface(ISR3Player__sgs_interface, ISR3Player__sgs_ifn, &ISR3Drone::_sgs_interface);


static int _sgs_method__ISR3Enemy__CallEvent( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__ISR3Enemy__HasFact( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, HasFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsHasFact( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ISR3Enemy__HasRecentFact( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, HasRecentFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsHasRecentFact( sgs_GetVar<int>()(C,0), sgs_GetVar<TimeVal>()(C,1) )); return 1;
}

static int _sgs_method__ISR3Enemy__GetRecentFact( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, GetRecentFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	return data->sgsGetRecentFact( sgs_GetVar<int>()(C,0), sgs_GetVar<TimeVal>()(C,1) );
}

int ISR3Enemy::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ISR3Enemy*>( obj->data )->C = C;
	static_cast<ISR3Enemy*>( obj->data )->~ISR3Enemy();
	return SGS_SUCCESS;
}

int ISR3Enemy::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Enemy*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ISR3Enemy::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Enemy*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<ISR3Enemy*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Enemy::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Enemy*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ISR3Enemy*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ISR3Enemy*>( obj->data )->m_transform.position = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<ISR3Enemy*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<ISR3Enemy*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<ISR3Enemy*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<ISR3Enemy*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<ISR3Enemy*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<ISR3Enemy*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Enemy::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Enemy*>( obj->data )->C, C );
	char bfr[ 41 ];
	sprintf( bfr, "ISR3Enemy (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_viewName, depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ISR3Enemy__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__ISR3Enemy__CallEvent },
	{ "HasFact", _sgs_method__ISR3Enemy__HasFact },
	{ "HasRecentFact", _sgs_method__ISR3Enemy__HasRecentFact },
	{ "GetRecentFact", _sgs_method__ISR3Enemy__GetRecentFact },
	{ NULL, NULL },
};

static int ISR3Enemy__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ISR3Enemy__sgs_funcs,
		-1, "ISR3Enemy." );
	return 1;
}

static sgs_ObjInterface ISR3Enemy__sgs_interface =
{
	"ISR3Enemy",
	ISR3Enemy::_sgs_destruct, ISR3Enemy::_sgs_gcmark, ISR3Enemy::_sgs_getindex, ISR3Enemy::_sgs_setindex, NULL, NULL, ISR3Enemy::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ISR3Enemy::_sgs_interface(ISR3Enemy__sgs_interface, ISR3Enemy__sgs_ifn, &ISR3Drone::_sgs_interface);

