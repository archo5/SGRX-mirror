// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "entities.hpp"

static int _sgs_method__Trigger__CallEvent( SGS_CTX )
{
	Trigger* data; if( !SGS_PARSE_METHOD( C, Trigger::_sgs_interface, data, Trigger, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__Trigger__Invoke( SGS_CTX )
{
	Trigger* data; if( !SGS_PARSE_METHOD( C, Trigger::_sgs_interface, data, Trigger, Invoke ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__Trigger__SetupTrigger( SGS_CTX )
{
	Trigger* data; if( !SGS_PARSE_METHOD( C, Trigger::_sgs_interface, data, Trigger, SetupTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int Trigger::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<Trigger*>( obj->data )->C = C;
	static_cast<Trigger*>( obj->data )->~Trigger();
	return SGS_SUCCESS;
}

int Trigger::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Trigger*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int Trigger::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Trigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_transform.position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<Trigger*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Trigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Trigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Trigger*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<Trigger*>( obj->data )->m_transform.position = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<Trigger*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<Trigger*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<Trigger*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<Trigger*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<Trigger*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<Trigger*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<Trigger*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<Trigger*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<Trigger*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<Trigger*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Trigger::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Trigger*>( obj->data )->C, C );
	char bfr[ 39 ];
	sprintf( bfr, "Trigger (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_transform.position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_done, depth ).push( C ); }
		sgs_StringConcat( C, 28 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst Trigger__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__Trigger__CallEvent },
	{ "Invoke", _sgs_method__Trigger__Invoke },
	{ "SetupTrigger", _sgs_method__Trigger__SetupTrigger },
	{ NULL, NULL },
};

static int Trigger__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		Trigger__sgs_funcs,
		-1, "Trigger." );
	return 1;
}

static sgs_ObjInterface Trigger__sgs_interface =
{
	"Trigger",
	Trigger::_sgs_destruct, Trigger::_sgs_gcmark, Trigger::_sgs_getindex, Trigger::_sgs_setindex, NULL, NULL, Trigger::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Trigger::_sgs_interface(Trigger__sgs_interface, Trigger__sgs_ifn, &Entity::_sgs_interface);


static int _sgs_method__BoxTrigger__CallEvent( SGS_CTX )
{
	BoxTrigger* data; if( !SGS_PARSE_METHOD( C, BoxTrigger::_sgs_interface, data, BoxTrigger, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__BoxTrigger__Invoke( SGS_CTX )
{
	BoxTrigger* data; if( !SGS_PARSE_METHOD( C, BoxTrigger::_sgs_interface, data, BoxTrigger, Invoke ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__BoxTrigger__SetupTrigger( SGS_CTX )
{
	BoxTrigger* data; if( !SGS_PARSE_METHOD( C, BoxTrigger::_sgs_interface, data, BoxTrigger, SetupTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int BoxTrigger::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<BoxTrigger*>( obj->data )->C = C;
	static_cast<BoxTrigger*>( obj->data )->~BoxTrigger();
	return SGS_SUCCESS;
}

int BoxTrigger::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<BoxTrigger*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int BoxTrigger::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<BoxTrigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_transform.position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "matrix" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_matrix ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<BoxTrigger*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int BoxTrigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<BoxTrigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<BoxTrigger*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<BoxTrigger*>( obj->data )->m_transform.position = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<BoxTrigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<BoxTrigger*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<BoxTrigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<BoxTrigger*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<BoxTrigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<BoxTrigger*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<BoxTrigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<BoxTrigger*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<BoxTrigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<BoxTrigger*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<BoxTrigger*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<BoxTrigger*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<BoxTrigger*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<BoxTrigger*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "matrix" ){ static_cast<BoxTrigger*>( obj->data )->m_matrix = sgs_GetVar<Mat4>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<BoxTrigger*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int BoxTrigger::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<BoxTrigger*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "BoxTrigger (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_transform.position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_done, depth ).push( C ); }
		{ sgs_PushString( C, "\nmatrix = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_matrix, depth ).push( C ); }
		sgs_StringConcat( C, 30 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst BoxTrigger__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__BoxTrigger__CallEvent },
	{ "Invoke", _sgs_method__BoxTrigger__Invoke },
	{ "SetupTrigger", _sgs_method__BoxTrigger__SetupTrigger },
	{ NULL, NULL },
};

static int BoxTrigger__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		BoxTrigger__sgs_funcs,
		-1, "BoxTrigger." );
	return 1;
}

static sgs_ObjInterface BoxTrigger__sgs_interface =
{
	"BoxTrigger",
	BoxTrigger::_sgs_destruct, BoxTrigger::_sgs_gcmark, BoxTrigger::_sgs_getindex, BoxTrigger::_sgs_setindex, NULL, NULL, BoxTrigger::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface BoxTrigger::_sgs_interface(BoxTrigger__sgs_interface, BoxTrigger__sgs_ifn, &Trigger::_sgs_interface);


static int _sgs_method__ProximityTrigger__CallEvent( SGS_CTX )
{
	ProximityTrigger* data; if( !SGS_PARSE_METHOD( C, ProximityTrigger::_sgs_interface, data, ProximityTrigger, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__ProximityTrigger__Invoke( SGS_CTX )
{
	ProximityTrigger* data; if( !SGS_PARSE_METHOD( C, ProximityTrigger::_sgs_interface, data, ProximityTrigger, Invoke ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__ProximityTrigger__SetupTrigger( SGS_CTX )
{
	ProximityTrigger* data; if( !SGS_PARSE_METHOD( C, ProximityTrigger::_sgs_interface, data, ProximityTrigger, SetupTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int ProximityTrigger::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ProximityTrigger*>( obj->data )->C = C;
	static_cast<ProximityTrigger*>( obj->data )->~ProximityTrigger();
	return SGS_SUCCESS;
}

int ProximityTrigger::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ProximityTrigger*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ProximityTrigger::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ProximityTrigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "radius" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_radius ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<ProximityTrigger*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ProximityTrigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ProximityTrigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ProximityTrigger*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ProximityTrigger*>( obj->data )->m_position = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<ProximityTrigger*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<ProximityTrigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<ProximityTrigger*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ProximityTrigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<ProximityTrigger*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<ProximityTrigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<ProximityTrigger*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ProximityTrigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<ProximityTrigger*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<ProximityTrigger*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<ProximityTrigger*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<ProximityTrigger*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<ProximityTrigger*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "radius" ){ static_cast<ProximityTrigger*>( obj->data )->m_radius = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<ProximityTrigger*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ProximityTrigger::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ProximityTrigger*>( obj->data )->C, C );
	char bfr[ 48 ];
	sprintf( bfr, "ProximityTrigger (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_done, depth ).push( C ); }
		{ sgs_PushString( C, "\nradius = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_radius, depth ).push( C ); }
		sgs_StringConcat( C, 30 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ProximityTrigger__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__ProximityTrigger__CallEvent },
	{ "Invoke", _sgs_method__ProximityTrigger__Invoke },
	{ "SetupTrigger", _sgs_method__ProximityTrigger__SetupTrigger },
	{ NULL, NULL },
};

static int ProximityTrigger__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ProximityTrigger__sgs_funcs,
		-1, "ProximityTrigger." );
	return 1;
}

static sgs_ObjInterface ProximityTrigger__sgs_interface =
{
	"ProximityTrigger",
	ProximityTrigger::_sgs_destruct, ProximityTrigger::_sgs_gcmark, ProximityTrigger::_sgs_getindex, ProximityTrigger::_sgs_setindex, NULL, NULL, ProximityTrigger::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ProximityTrigger::_sgs_interface(ProximityTrigger__sgs_interface, ProximityTrigger__sgs_ifn, &Trigger::_sgs_interface);


static int _sgs_method__SlidingDoor__CallEvent( SGS_CTX )
{
	SlidingDoor* data; if( !SGS_PARSE_METHOD( C, SlidingDoor::_sgs_interface, data, SlidingDoor, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__SlidingDoor__Invoke( SGS_CTX )
{
	SlidingDoor* data; if( !SGS_PARSE_METHOD( C, SlidingDoor::_sgs_interface, data, SlidingDoor, Invoke ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__SlidingDoor__SetupTrigger( SGS_CTX )
{
	SlidingDoor* data; if( !SGS_PARSE_METHOD( C, SlidingDoor::_sgs_interface, data, SlidingDoor, SetupTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int SlidingDoor::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SlidingDoor*>( obj->data )->C = C;
	static_cast<SlidingDoor*>( obj->data )->~SlidingDoor();
	return SGS_SUCCESS;
}

int SlidingDoor::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SlidingDoor*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SlidingDoor::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SlidingDoor*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_transform.position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "isSwitch" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_isSwitch ); return SGS_SUCCESS; }
		SGS_CASE( "switchPred" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_switchPred ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<SlidingDoor*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int SlidingDoor::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SlidingDoor*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<SlidingDoor*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<SlidingDoor*>( obj->data )->m_transform.position = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<SlidingDoor*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<SlidingDoor*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<SlidingDoor*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<SlidingDoor*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<SlidingDoor*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<SlidingDoor*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<SlidingDoor*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<SlidingDoor*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<SlidingDoor*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "switchPred" ){ static_cast<SlidingDoor*>( obj->data )->m_switchPred = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<SlidingDoor*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int SlidingDoor::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SlidingDoor*>( obj->data )->C, C );
	char bfr[ 43 ];
	sprintf( bfr, "SlidingDoor (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_transform.position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_done, depth ).push( C ); }
		{ sgs_PushString( C, "\nisSwitch = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_isSwitch, depth ).push( C ); }
		{ sgs_PushString( C, "\nswitchPred = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_switchPred, depth ).push( C ); }
		sgs_StringConcat( C, 32 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SlidingDoor__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__SlidingDoor__CallEvent },
	{ "Invoke", _sgs_method__SlidingDoor__Invoke },
	{ "SetupTrigger", _sgs_method__SlidingDoor__SetupTrigger },
	{ NULL, NULL },
};

static int SlidingDoor__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SlidingDoor__sgs_funcs,
		-1, "SlidingDoor." );
	return 1;
}

static sgs_ObjInterface SlidingDoor__sgs_interface =
{
	"SlidingDoor",
	SlidingDoor::_sgs_destruct, SlidingDoor::_sgs_gcmark, SlidingDoor::_sgs_getindex, SlidingDoor::_sgs_setindex, NULL, NULL, SlidingDoor::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SlidingDoor::_sgs_interface(SlidingDoor__sgs_interface, SlidingDoor__sgs_ifn, &Trigger::_sgs_interface);


static int _sgs_method__PickupItem__CallEvent( SGS_CTX )
{
	PickupItem* data; if( !SGS_PARSE_METHOD( C, PickupItem::_sgs_interface, data, PickupItem, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int PickupItem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<PickupItem*>( obj->data )->C = C;
	static_cast<PickupItem*>( obj->data )->~PickupItem();
	return SGS_SUCCESS;
}

int PickupItem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<PickupItem*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int PickupItem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<PickupItem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_transform.position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<PickupItem*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int PickupItem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<PickupItem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<PickupItem*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<PickupItem*>( obj->data )->m_transform.position = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<PickupItem*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<PickupItem*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<PickupItem*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<PickupItem*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<PickupItem*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<PickupItem*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int PickupItem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<PickupItem*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "PickupItem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_transform.position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_viewName, depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst PickupItem__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__PickupItem__CallEvent },
	{ NULL, NULL },
};

static int PickupItem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		PickupItem__sgs_funcs,
		-1, "PickupItem." );
	return 1;
}

static sgs_ObjInterface PickupItem__sgs_interface =
{
	"PickupItem",
	PickupItem::_sgs_destruct, PickupItem::_sgs_gcmark, PickupItem::_sgs_getindex, PickupItem::_sgs_setindex, NULL, NULL, PickupItem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface PickupItem::_sgs_interface(PickupItem__sgs_interface, PickupItem__sgs_ifn, &Entity::_sgs_interface);


static int _sgs_method__Actionable__CallEvent( SGS_CTX )
{
	Actionable* data; if( !SGS_PARSE_METHOD( C, Actionable::_sgs_interface, data, Actionable, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int Actionable::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<Actionable*>( obj->data )->C = C;
	static_cast<Actionable*>( obj->data )->~Actionable();
	return SGS_SUCCESS;
}

int Actionable::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actionable*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int Actionable::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actionable*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.placePos ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_enabled ); return SGS_SUCCESS; }
		SGS_CASE( "timeEstimate" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeEstimate ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeActual ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_onSuccess ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<Actionable*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Actionable::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actionable*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Actionable*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<Actionable*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<Actionable*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<Actionable*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<Actionable*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<Actionable*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<Actionable*>( obj->data )->m_enabled = sgs_GetVar<bool>()( C, 1 );
			static_cast<Actionable*>( obj->data )->sgsSetEnabled(); return SGS_SUCCESS; }
		SGS_CASE( "timeEstimate" ){ static_cast<Actionable*>( obj->data )->m_info.timeEstimate = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ static_cast<Actionable*>( obj->data )->m_info.timeActual = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ static_cast<Actionable*>( obj->data )->m_onSuccess = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<Actionable*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Actionable::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actionable*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "Actionable (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.placePos, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeEstimate = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.timeEstimate, depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeActual = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.timeActual, depth ).push( C ); }
		{ sgs_PushString( C, "\nonSuccess = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_onSuccess, depth ).push( C ); }
		sgs_StringConcat( C, 28 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst Actionable__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__Actionable__CallEvent },
	{ NULL, NULL },
};

static int Actionable__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		Actionable__sgs_funcs,
		-1, "Actionable." );
	return 1;
}

static sgs_ObjInterface Actionable__sgs_interface =
{
	"Actionable",
	NULL, Actionable::_sgs_gcmark, Actionable::_sgs_getindex, Actionable::_sgs_setindex, NULL, NULL, Actionable::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Actionable::_sgs_interface(Actionable__sgs_interface, Actionable__sgs_ifn, &Entity::_sgs_interface);


static int _sgs_method__MultiEntity__CallEvent( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__MICreate( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MICreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MICreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__MIDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MIDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MIDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__MIExists( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MIExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->MIExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__MISetMesh( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__MISetEnabled( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__MISetMatrix( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__MISetShaderConst( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetShaderConst ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetShaderConst( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<Vec4>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__MISetLayers( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetLayers ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetLayers( sgs_GetVar<int>()(C,0), sgs_GetVar<uint32_t>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSCreate( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSCreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSCreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__PSExists( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->PSExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__PSLoad( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSLoad ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSLoad( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSSetMatrix( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSSetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSSetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSSetMatrixFromMeshAABB( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSSetMatrixFromMeshAABB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSSetMatrixFromMeshAABB( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSPlay( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSPlay ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSPlay( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__PSStop( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSStop ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSStop( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__PSTrigger( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSTrigger( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__DSCreate( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, DSCreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSCreate( sgs_GetVar<StringView>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVar<StringView>()(C,2), sgs_GetVar<uint32_t>()(C,3) ); return 0;
}

static int _sgs_method__MultiEntity__DSDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, DSDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSDestroy(  ); return 0;
}

static int _sgs_method__MultiEntity__DSResize( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, DSResize ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSResize( sgs_GetVar<uint32_t>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__DSClear( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, DSClear ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSClear(  ); return 0;
}

static int _sgs_method__MultiEntity__RBCreateFromMesh( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBCreateFromMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBCreateFromMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIRigidBodyInfo>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__RBCreateFromConvexPointSet( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBCreateFromConvexPointSet ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBCreateFromConvexPointSet( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVarObj<SGRX_SIRigidBodyInfo>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__RBDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__RBExists( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__RBSetEnabled( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBSetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__RBGetPosition( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBGetPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetPosition( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__RBSetPosition( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBSetPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetPosition( sgs_GetVar<int>()(C,0), sgs_GetVar<Vec3>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__RBGetRotation( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBGetRotation ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetRotation( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__RBSetRotation( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBSetRotation ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetRotation( sgs_GetVar<int>()(C,0), sgs_GetVar<Quat>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__RBGetMatrix( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBGetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetMatrix( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__RBApplyForce( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBApplyForce ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBApplyForce( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<Vec3>()(C,2), sgs_GetVar<Vec3>()(C,3) ); return 0;
}

static int _sgs_method__MultiEntity__JTCreateHingeB2W( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTCreateHingeB2W ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateHingeB2W( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIHingeJointInfo>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__JTCreateHingeB2B( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTCreateHingeB2B ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateHingeB2B( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<int>()(C,2), sgs_GetVarObj<SGRX_SIHingeJointInfo>()(C,3) ); return 0;
}

static int _sgs_method__MultiEntity__JTCreateConeTwistB2W( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTCreateConeTwistB2W ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateConeTwistB2W( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIConeTwistJointInfo>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__JTCreateConeTwistB2B( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTCreateConeTwistB2B ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateConeTwistB2B( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<int>()(C,2), sgs_GetVarObj<SGRX_SIConeTwistJointInfo>()(C,3) ); return 0;
}

static int _sgs_method__MultiEntity__JTDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__JTExists( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->JTExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__JTSetEnabled( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTSetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTSetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

int MultiEntity::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<MultiEntity*>( obj->data )->C = C;
	static_cast<MultiEntity*>( obj->data )->~MultiEntity();
	return SGS_SUCCESS;
}

int MultiEntity::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MultiEntity*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int MultiEntity::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MultiEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_transform.position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_transform.rotation ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_transform.GetRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_transform.scale ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_transform.GetMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<MultiEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MultiEntity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MultiEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<MultiEntity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<MultiEntity*>( obj->data )->m_transform.position = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<MultiEntity*>( obj->data )->m_transform.rotation = sgs_GetVar<Quat>()( C, 1 );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<MultiEntity*>( obj->data )->m_transform.SetRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<MultiEntity*>( obj->data )->m_transform.scale = sgs_GetVar<Vec3>()( C, 1 );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<MultiEntity*>( obj->data )->m_transform.SetMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<MultiEntity*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<MultiEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MultiEntity::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MultiEntity*>( obj->data )->C, C );
	char bfr[ 43 ];
	sprintf( bfr, "MultiEntity (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_transform.position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_transform.rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_transform.GetRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_transform.scale, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_transform.GetMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_viewName, depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst MultiEntity__sgs_funcs[] =
{
	{ "CallEvent", _sgs_method__MultiEntity__CallEvent },
	{ "MICreate", _sgs_method__MultiEntity__MICreate },
	{ "MIDestroy", _sgs_method__MultiEntity__MIDestroy },
	{ "MIExists", _sgs_method__MultiEntity__MIExists },
	{ "MISetMesh", _sgs_method__MultiEntity__MISetMesh },
	{ "MISetEnabled", _sgs_method__MultiEntity__MISetEnabled },
	{ "MISetMatrix", _sgs_method__MultiEntity__MISetMatrix },
	{ "MISetShaderConst", _sgs_method__MultiEntity__MISetShaderConst },
	{ "MISetLayers", _sgs_method__MultiEntity__MISetLayers },
	{ "PSCreate", _sgs_method__MultiEntity__PSCreate },
	{ "PSDestroy", _sgs_method__MultiEntity__PSDestroy },
	{ "PSExists", _sgs_method__MultiEntity__PSExists },
	{ "PSLoad", _sgs_method__MultiEntity__PSLoad },
	{ "PSSetMatrix", _sgs_method__MultiEntity__PSSetMatrix },
	{ "PSSetMatrixFromMeshAABB", _sgs_method__MultiEntity__PSSetMatrixFromMeshAABB },
	{ "PSPlay", _sgs_method__MultiEntity__PSPlay },
	{ "PSStop", _sgs_method__MultiEntity__PSStop },
	{ "PSTrigger", _sgs_method__MultiEntity__PSTrigger },
	{ "DSCreate", _sgs_method__MultiEntity__DSCreate },
	{ "DSDestroy", _sgs_method__MultiEntity__DSDestroy },
	{ "DSResize", _sgs_method__MultiEntity__DSResize },
	{ "DSClear", _sgs_method__MultiEntity__DSClear },
	{ "RBCreateFromMesh", _sgs_method__MultiEntity__RBCreateFromMesh },
	{ "RBCreateFromConvexPointSet", _sgs_method__MultiEntity__RBCreateFromConvexPointSet },
	{ "RBDestroy", _sgs_method__MultiEntity__RBDestroy },
	{ "RBExists", _sgs_method__MultiEntity__RBExists },
	{ "RBSetEnabled", _sgs_method__MultiEntity__RBSetEnabled },
	{ "RBGetPosition", _sgs_method__MultiEntity__RBGetPosition },
	{ "RBSetPosition", _sgs_method__MultiEntity__RBSetPosition },
	{ "RBGetRotation", _sgs_method__MultiEntity__RBGetRotation },
	{ "RBSetRotation", _sgs_method__MultiEntity__RBSetRotation },
	{ "RBGetMatrix", _sgs_method__MultiEntity__RBGetMatrix },
	{ "RBApplyForce", _sgs_method__MultiEntity__RBApplyForce },
	{ "JTCreateHingeB2W", _sgs_method__MultiEntity__JTCreateHingeB2W },
	{ "JTCreateHingeB2B", _sgs_method__MultiEntity__JTCreateHingeB2B },
	{ "JTCreateConeTwistB2W", _sgs_method__MultiEntity__JTCreateConeTwistB2W },
	{ "JTCreateConeTwistB2B", _sgs_method__MultiEntity__JTCreateConeTwistB2B },
	{ "JTDestroy", _sgs_method__MultiEntity__JTDestroy },
	{ "JTExists", _sgs_method__MultiEntity__JTExists },
	{ "JTSetEnabled", _sgs_method__MultiEntity__JTSetEnabled },
	{ NULL, NULL },
};

static int MultiEntity__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		MultiEntity__sgs_funcs,
		-1, "MultiEntity." );
	return 1;
}

static sgs_ObjInterface MultiEntity__sgs_interface =
{
	"MultiEntity",
	NULL, MultiEntity::_sgs_gcmark, MultiEntity::_sgs_getindex, MultiEntity::_sgs_setindex, NULL, NULL, MultiEntity::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface MultiEntity::_sgs_interface(MultiEntity__sgs_interface, MultiEntity__sgs_ifn, &Entity::_sgs_interface);

