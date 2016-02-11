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
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__Trigger__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "Invoke" ){ sgs_PushCFunc( C, _sgs_method__Trigger__Invoke ); return SGS_SUCCESS; }
		SGS_CASE( "SetupTrigger" ){ sgs_PushCFunc( C, _sgs_method__Trigger__SetupTrigger ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Trigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Trigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Trigger*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<Trigger*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<Trigger*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<Trigger*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<Trigger*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<Trigger*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_done, depth ).push( C ); }
		sgs_StringConcat( C, 18 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface Trigger__sgs_interface =
{
	"Trigger",
	Trigger::_sgs_destruct, Trigger::_sgs_gcmark, Trigger::_getindex, Trigger::_setindex, NULL, NULL, Trigger::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Trigger::_sgs_interface(Trigger__sgs_interface, &Entity::_sgs_interface);


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
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__BoxTrigger__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "Invoke" ){ sgs_PushCFunc( C, _sgs_method__BoxTrigger__Invoke ); return SGS_SUCCESS; }
		SGS_CASE( "SetupTrigger" ){ sgs_PushCFunc( C, _sgs_method__BoxTrigger__SetupTrigger ); return SGS_SUCCESS; }
		SGS_CASE( "matrix" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_matrix ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int BoxTrigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<BoxTrigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<BoxTrigger*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<BoxTrigger*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<BoxTrigger*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<BoxTrigger*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<BoxTrigger*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<BoxTrigger*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "matrix" ){ static_cast<BoxTrigger*>( obj->data )->m_matrix = sgs_GetVar<Mat4>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_done, depth ).push( C ); }
		{ sgs_PushString( C, "\nmatrix = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->m_matrix, depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface BoxTrigger__sgs_interface =
{
	"BoxTrigger",
	BoxTrigger::_sgs_destruct, BoxTrigger::_sgs_gcmark, BoxTrigger::_getindex, BoxTrigger::_setindex, NULL, NULL, BoxTrigger::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface BoxTrigger::_sgs_interface(BoxTrigger__sgs_interface, &Trigger::_sgs_interface);


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
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__ProximityTrigger__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "Invoke" ){ sgs_PushCFunc( C, _sgs_method__ProximityTrigger__Invoke ); return SGS_SUCCESS; }
		SGS_CASE( "SetupTrigger" ){ sgs_PushCFunc( C, _sgs_method__ProximityTrigger__SetupTrigger ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_position ); return SGS_SUCCESS; }
		SGS_CASE( "radius" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_radius ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ProximityTrigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ProximityTrigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ProximityTrigger*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<ProximityTrigger*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<ProximityTrigger*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<ProximityTrigger*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<ProximityTrigger*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<ProximityTrigger*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ProximityTrigger*>( obj->data )->m_position = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "radius" ){ static_cast<ProximityTrigger*>( obj->data )->m_radius = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_done, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_position, depth ).push( C ); }
		{ sgs_PushString( C, "\nradius = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->m_radius, depth ).push( C ); }
		sgs_StringConcat( C, 22 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface ProximityTrigger__sgs_interface =
{
	"ProximityTrigger",
	ProximityTrigger::_sgs_destruct, ProximityTrigger::_sgs_gcmark, ProximityTrigger::_getindex, ProximityTrigger::_setindex, NULL, NULL, ProximityTrigger::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ProximityTrigger::_sgs_interface(ProximityTrigger__sgs_interface, &Trigger::_sgs_interface);


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
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__SlidingDoor__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "Invoke" ){ sgs_PushCFunc( C, _sgs_method__SlidingDoor__Invoke ); return SGS_SUCCESS; }
		SGS_CASE( "SetupTrigger" ){ sgs_PushCFunc( C, _sgs_method__SlidingDoor__SetupTrigger ); return SGS_SUCCESS; }
		SGS_CASE( "isSwitch" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_isSwitch ); return SGS_SUCCESS; }
		SGS_CASE( "switchPred" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_switchPred ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SlidingDoor::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SlidingDoor*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<SlidingDoor*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<SlidingDoor*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<SlidingDoor*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<SlidingDoor*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<SlidingDoor*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<SlidingDoor*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "switchPred" ){ static_cast<SlidingDoor*>( obj->data )->m_switchPred = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_done, depth ).push( C ); }
		{ sgs_PushString( C, "\nisSwitch = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_isSwitch, depth ).push( C ); }
		{ sgs_PushString( C, "\nswitchPred = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_switchPred, depth ).push( C ); }
		sgs_StringConcat( C, 22 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface SlidingDoor__sgs_interface =
{
	"SlidingDoor",
	SlidingDoor::_sgs_destruct, SlidingDoor::_sgs_gcmark, SlidingDoor::_getindex, SlidingDoor::_setindex, NULL, NULL, SlidingDoor::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SlidingDoor::_sgs_interface(SlidingDoor__sgs_interface, &Trigger::_sgs_interface);


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
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__PickupItem__CallEvent ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int PickupItem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<PickupItem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<PickupItem*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<PickupItem*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_viewName, depth ).push( C ); }
		sgs_StringConcat( C, 10 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface PickupItem__sgs_interface =
{
	"PickupItem",
	PickupItem::_sgs_destruct, PickupItem::_sgs_gcmark, PickupItem::_getindex, PickupItem::_setindex, NULL, NULL, PickupItem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface PickupItem::_sgs_interface(PickupItem__sgs_interface, &Entity::_sgs_interface);


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
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__Actionable__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_enabled ); return SGS_SUCCESS; }
		SGS_CASE( "timeEstimate" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeEstimate ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeActual ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_onSuccess ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.placePos ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Actionable::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actionable*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Actionable*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<Actionable*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<Actionable*>( obj->data )->m_enabled = sgs_GetVar<bool>()( C, 1 );
			static_cast<Actionable*>( obj->data )->sgsSetEnabled(); return SGS_SUCCESS; }
		SGS_CASE( "timeEstimate" ){ static_cast<Actionable*>( obj->data )->m_info.timeEstimate = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ static_cast<Actionable*>( obj->data )->m_info.timeActual = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ static_cast<Actionable*>( obj->data )->m_onSuccess = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeEstimate = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.timeEstimate, depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeActual = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.timeActual, depth ).push( C ); }
		{ sgs_PushString( C, "\nonSuccess = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_onSuccess, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.placePos, depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface Actionable__sgs_interface =
{
	"Actionable",
	NULL, Actionable::_sgs_gcmark, Actionable::_getindex, Actionable::_setindex, NULL, NULL, Actionable::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Actionable::_sgs_interface(Actionable__sgs_interface, &Entity::_sgs_interface);


static int _sgs_method__ScriptedEntity__CallEvent( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__SetMatrix( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, SetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->SetMatrix( sgs_GetVar<Mat4>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__MICreate( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, MICreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MICreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__MIDestroy( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, MIDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MIDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__MIExists( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, MIExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->MIExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ScriptedEntity__MISetMesh( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, MISetMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__MISetEnabled( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, MISetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__MISetMatrix( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, MISetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__MISetShaderConst( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, MISetShaderConst ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetShaderConst( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<Vec4>()(C,2) ); return 0;
}

static int _sgs_method__ScriptedEntity__PSCreate( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, PSCreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSCreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__PSDestroy( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, PSDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__PSExists( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, PSExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->PSExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ScriptedEntity__PSLoad( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, PSLoad ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSLoad( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__PSSetMatrix( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, PSSetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSSetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__PSSetMatrixFromMeshAABB( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, PSSetMatrixFromMeshAABB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSSetMatrixFromMeshAABB( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__PSPlay( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, PSPlay ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSPlay( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__PSStop( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, PSStop ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSStop( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__PSTrigger( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, PSTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSTrigger( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__DSCreate( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, DSCreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSCreate( sgs_GetVar<StringView>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVar<StringView>()(C,2), sgs_GetVar<uint32_t>()(C,3) ); return 0;
}

static int _sgs_method__ScriptedEntity__DSDestroy( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, DSDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSDestroy(  ); return 0;
}

static int _sgs_method__ScriptedEntity__DSResize( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, DSResize ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSResize( sgs_GetVar<uint32_t>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__DSClear( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, DSClear ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSClear(  ); return 0;
}

static int _sgs_method__ScriptedEntity__RBCreateFromMesh( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBCreateFromMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBCreateFromMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIRigidBodyInfo>()(C,2) ); return 0;
}

static int _sgs_method__ScriptedEntity__RBCreateFromConvexPointSet( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBCreateFromConvexPointSet ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBCreateFromConvexPointSet( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVarObj<SGRX_SIRigidBodyInfo>()(C,2) ); return 0;
}

static int _sgs_method__ScriptedEntity__RBDestroy( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__RBExists( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ScriptedEntity__RBSetEnabled( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBSetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__RBGetPosition( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBGetPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetPosition( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ScriptedEntity__RBSetPosition( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBSetPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetPosition( sgs_GetVar<int>()(C,0), sgs_GetVar<Vec3>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__RBGetRotation( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBGetRotation ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetRotation( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ScriptedEntity__RBSetRotation( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBSetRotation ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetRotation( sgs_GetVar<int>()(C,0), sgs_GetVar<Quat>()(C,1) ); return 0;
}

static int _sgs_method__ScriptedEntity__RBGetMatrix( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBGetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetMatrix( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ScriptedEntity__RBApplyForce( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, RBApplyForce ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBApplyForce( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<Vec3>()(C,2), sgs_GetVar<Vec3>()(C,3) ); return 0;
}

static int _sgs_method__ScriptedEntity__JTCreateHingeB2W( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, JTCreateHingeB2W ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateHingeB2W( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIHingeJointInfo>()(C,2) ); return 0;
}

static int _sgs_method__ScriptedEntity__JTCreateHingeB2B( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, JTCreateHingeB2B ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateHingeB2B( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<int>()(C,2), sgs_GetVarObj<SGRX_SIHingeJointInfo>()(C,3) ); return 0;
}

static int _sgs_method__ScriptedEntity__JTCreateConeTwistB2W( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, JTCreateConeTwistB2W ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateConeTwistB2W( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIConeTwistJointInfo>()(C,2) ); return 0;
}

static int _sgs_method__ScriptedEntity__JTCreateConeTwistB2B( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, JTCreateConeTwistB2B ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateConeTwistB2B( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<int>()(C,2), sgs_GetVarObj<SGRX_SIConeTwistJointInfo>()(C,3) ); return 0;
}

static int _sgs_method__ScriptedEntity__JTDestroy( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, JTDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__ScriptedEntity__JTExists( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, JTExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->JTExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ScriptedEntity__JTSetEnabled( SGS_CTX )
{
	ScriptedEntity* data; if( !SGS_PARSE_METHOD( C, ScriptedEntity::_sgs_interface, data, ScriptedEntity, JTSetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTSetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

int ScriptedEntity::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ScriptedEntity*>( obj->data )->C = C;
	static_cast<ScriptedEntity*>( obj->data )->~ScriptedEntity();
	return SGS_SUCCESS;
}

int ScriptedEntity::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ScriptedEntity*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ScriptedEntity::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ScriptedEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ScriptedEntity*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ScriptedEntity*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ScriptedEntity*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ScriptedEntity*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ScriptedEntity*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "SetMatrix" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__SetMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "MICreate" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__MICreate ); return SGS_SUCCESS; }
		SGS_CASE( "MIDestroy" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__MIDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "MIExists" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__MIExists ); return SGS_SUCCESS; }
		SGS_CASE( "MISetMesh" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__MISetMesh ); return SGS_SUCCESS; }
		SGS_CASE( "MISetEnabled" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__MISetEnabled ); return SGS_SUCCESS; }
		SGS_CASE( "MISetMatrix" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__MISetMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "MISetShaderConst" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__MISetShaderConst ); return SGS_SUCCESS; }
		SGS_CASE( "PSCreate" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__PSCreate ); return SGS_SUCCESS; }
		SGS_CASE( "PSDestroy" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__PSDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "PSExists" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__PSExists ); return SGS_SUCCESS; }
		SGS_CASE( "PSLoad" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__PSLoad ); return SGS_SUCCESS; }
		SGS_CASE( "PSSetMatrix" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__PSSetMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "PSSetMatrixFromMeshAABB" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__PSSetMatrixFromMeshAABB ); return SGS_SUCCESS; }
		SGS_CASE( "PSPlay" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__PSPlay ); return SGS_SUCCESS; }
		SGS_CASE( "PSStop" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__PSStop ); return SGS_SUCCESS; }
		SGS_CASE( "PSTrigger" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__PSTrigger ); return SGS_SUCCESS; }
		SGS_CASE( "DSCreate" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__DSCreate ); return SGS_SUCCESS; }
		SGS_CASE( "DSDestroy" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__DSDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "DSResize" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__DSResize ); return SGS_SUCCESS; }
		SGS_CASE( "DSClear" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__DSClear ); return SGS_SUCCESS; }
		SGS_CASE( "RBCreateFromMesh" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBCreateFromMesh ); return SGS_SUCCESS; }
		SGS_CASE( "RBCreateFromConvexPointSet" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBCreateFromConvexPointSet ); return SGS_SUCCESS; }
		SGS_CASE( "RBDestroy" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "RBExists" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBExists ); return SGS_SUCCESS; }
		SGS_CASE( "RBSetEnabled" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBSetEnabled ); return SGS_SUCCESS; }
		SGS_CASE( "RBGetPosition" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBGetPosition ); return SGS_SUCCESS; }
		SGS_CASE( "RBSetPosition" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBSetPosition ); return SGS_SUCCESS; }
		SGS_CASE( "RBGetRotation" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBGetRotation ); return SGS_SUCCESS; }
		SGS_CASE( "RBSetRotation" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBSetRotation ); return SGS_SUCCESS; }
		SGS_CASE( "RBGetMatrix" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBGetMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "RBApplyForce" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__RBApplyForce ); return SGS_SUCCESS; }
		SGS_CASE( "JTCreateHingeB2W" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__JTCreateHingeB2W ); return SGS_SUCCESS; }
		SGS_CASE( "JTCreateHingeB2B" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__JTCreateHingeB2B ); return SGS_SUCCESS; }
		SGS_CASE( "JTCreateConeTwistB2W" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__JTCreateConeTwistB2W ); return SGS_SUCCESS; }
		SGS_CASE( "JTCreateConeTwistB2B" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__JTCreateConeTwistB2B ); return SGS_SUCCESS; }
		SGS_CASE( "JTDestroy" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__JTDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "JTExists" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__JTExists ); return SGS_SUCCESS; }
		SGS_CASE( "JTSetEnabled" ){ sgs_PushCFunc( C, _sgs_method__ScriptedEntity__JTSetEnabled ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ScriptedEntity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ScriptedEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ScriptedEntity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<ScriptedEntity*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ScriptedEntity::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ScriptedEntity*>( obj->data )->C, C );
	char bfr[ 46 ];
	sprintf( bfr, "ScriptedEntity (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ScriptedEntity*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ScriptedEntity*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ScriptedEntity*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ScriptedEntity*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<ScriptedEntity*>( obj->data )->m_viewName, depth ).push( C ); }
		sgs_StringConcat( C, 10 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface ScriptedEntity__sgs_interface =
{
	"ScriptedEntity",
	NULL, ScriptedEntity::_sgs_gcmark, ScriptedEntity::_getindex, ScriptedEntity::_setindex, NULL, NULL, ScriptedEntity::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ScriptedEntity::_sgs_interface(ScriptedEntity__sgs_interface, &Entity::_sgs_interface);

