// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "entities.hpp"

static int _sgs_method__Trigger__CallEvent( SGS_CTX )
{
	Trigger* data; if( !SGS_PARSE_METHOD( C, Trigger::_sgs_interface, data, Trigger, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__Trigger__Invoke( SGS_CTX )
{
	Trigger* data; if( !SGS_PARSE_METHOD( C, Trigger::_sgs_interface, data, Trigger, Invoke ) ) return 0;
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__Trigger__SetupTrigger( SGS_CTX )
{
	Trigger* data; if( !SGS_PARSE_METHOD( C, Trigger::_sgs_interface, data, Trigger, SetupTrigger ) ) return 0;
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int Trigger::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<Trigger*>( obj->data )->~Trigger();
	return SGS_SUCCESS;
}

int Trigger::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int Trigger::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__Trigger__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "Invoke" ){ sgs_PushCFunction( C, _sgs_method__Trigger__Invoke ); return SGS_SUCCESS; }
		SGS_CASE( "SetupTrigger" ){ sgs_PushCFunction( C, _sgs_method__Trigger__SetupTrigger ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Trigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<Trigger*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<Trigger*>( obj->data )->m_func = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<Trigger*>( obj->data )->m_funcOut = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<Trigger*>( obj->data )->m_once = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<Trigger*>( obj->data )->m_done = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Trigger::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 39 ];
	sprintf( bfr, "Trigger (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface Trigger::_sgs_interface[1] =
{{
	"Trigger",
	Trigger::_sgs_destruct, Trigger::_sgs_gcmark, Trigger::_sgs_getindex, Trigger::_sgs_setindex, NULL, NULL, Trigger::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__BoxTrigger__CallEvent( SGS_CTX )
{
	BoxTrigger* data; if( !SGS_PARSE_METHOD( C, BoxTrigger::_sgs_interface, data, BoxTrigger, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__BoxTrigger__Invoke( SGS_CTX )
{
	BoxTrigger* data; if( !SGS_PARSE_METHOD( C, BoxTrigger::_sgs_interface, data, BoxTrigger, Invoke ) ) return 0;
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__BoxTrigger__SetupTrigger( SGS_CTX )
{
	BoxTrigger* data; if( !SGS_PARSE_METHOD( C, BoxTrigger::_sgs_interface, data, BoxTrigger, SetupTrigger ) ) return 0;
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int BoxTrigger::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<BoxTrigger*>( obj->data )->~BoxTrigger();
	return SGS_SUCCESS;
}

int BoxTrigger::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int BoxTrigger::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__BoxTrigger__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "Invoke" ){ sgs_PushCFunction( C, _sgs_method__BoxTrigger__Invoke ); return SGS_SUCCESS; }
		SGS_CASE( "SetupTrigger" ){ sgs_PushCFunction( C, _sgs_method__BoxTrigger__SetupTrigger ); return SGS_SUCCESS; }
		SGS_CASE( "matrix" ){ sgs_PushVar( C, static_cast<BoxTrigger*>( obj->data )->m_matrix ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int BoxTrigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<BoxTrigger*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<BoxTrigger*>( obj->data )->m_func = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<BoxTrigger*>( obj->data )->m_funcOut = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<BoxTrigger*>( obj->data )->m_once = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<BoxTrigger*>( obj->data )->m_done = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "matrix" ){ static_cast<BoxTrigger*>( obj->data )->m_matrix = sgs_GetVarP<Mat4>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int BoxTrigger::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 42 ];
	sprintf( bfr, "BoxTrigger (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<BoxTrigger*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface BoxTrigger::_sgs_interface[1] =
{{
	"BoxTrigger",
	BoxTrigger::_sgs_destruct, BoxTrigger::_sgs_gcmark, BoxTrigger::_sgs_getindex, BoxTrigger::_sgs_setindex, NULL, NULL, BoxTrigger::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__ProximityTrigger__CallEvent( SGS_CTX )
{
	ProximityTrigger* data; if( !SGS_PARSE_METHOD( C, ProximityTrigger::_sgs_interface, data, ProximityTrigger, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__ProximityTrigger__Invoke( SGS_CTX )
{
	ProximityTrigger* data; if( !SGS_PARSE_METHOD( C, ProximityTrigger::_sgs_interface, data, ProximityTrigger, Invoke ) ) return 0;
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__ProximityTrigger__SetupTrigger( SGS_CTX )
{
	ProximityTrigger* data; if( !SGS_PARSE_METHOD( C, ProximityTrigger::_sgs_interface, data, ProximityTrigger, SetupTrigger ) ) return 0;
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int ProximityTrigger::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ProximityTrigger*>( obj->data )->~ProximityTrigger();
	return SGS_SUCCESS;
}

int ProximityTrigger::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int ProximityTrigger::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__ProximityTrigger__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "Invoke" ){ sgs_PushCFunction( C, _sgs_method__ProximityTrigger__Invoke ); return SGS_SUCCESS; }
		SGS_CASE( "SetupTrigger" ){ sgs_PushCFunction( C, _sgs_method__ProximityTrigger__SetupTrigger ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_position ); return SGS_SUCCESS; }
		SGS_CASE( "radius" ){ sgs_PushVar( C, static_cast<ProximityTrigger*>( obj->data )->m_radius ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ProximityTrigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<ProximityTrigger*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<ProximityTrigger*>( obj->data )->m_func = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<ProximityTrigger*>( obj->data )->m_funcOut = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<ProximityTrigger*>( obj->data )->m_once = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<ProximityTrigger*>( obj->data )->m_done = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ProximityTrigger*>( obj->data )->m_position = sgs_GetVarP<Vec3>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "radius" ){ static_cast<ProximityTrigger*>( obj->data )->m_radius = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ProximityTrigger::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 48 ];
	sprintf( bfr, "ProximityTrigger (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ProximityTrigger*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface ProximityTrigger::_sgs_interface[1] =
{{
	"ProximityTrigger",
	ProximityTrigger::_sgs_destruct, ProximityTrigger::_sgs_gcmark, ProximityTrigger::_sgs_getindex, ProximityTrigger::_sgs_setindex, NULL, NULL, ProximityTrigger::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__SlidingDoor__CallEvent( SGS_CTX )
{
	SlidingDoor* data; if( !SGS_PARSE_METHOD( C, SlidingDoor::_sgs_interface, data, SlidingDoor, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__SlidingDoor__Invoke( SGS_CTX )
{
	SlidingDoor* data; if( !SGS_PARSE_METHOD( C, SlidingDoor::_sgs_interface, data, SlidingDoor, Invoke ) ) return 0;
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__SlidingDoor__SetupTrigger( SGS_CTX )
{
	SlidingDoor* data; if( !SGS_PARSE_METHOD( C, SlidingDoor::_sgs_interface, data, SlidingDoor, SetupTrigger ) ) return 0;
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int SlidingDoor::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SlidingDoor*>( obj->data )->~SlidingDoor();
	return SGS_SUCCESS;
}

int SlidingDoor::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int SlidingDoor::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__SlidingDoor__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "Invoke" ){ sgs_PushCFunction( C, _sgs_method__SlidingDoor__Invoke ); return SGS_SUCCESS; }
		SGS_CASE( "SetupTrigger" ){ sgs_PushCFunction( C, _sgs_method__SlidingDoor__SetupTrigger ); return SGS_SUCCESS; }
		SGS_CASE( "isSwitch" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_isSwitch ); return SGS_SUCCESS; }
		SGS_CASE( "switchPred" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_switchPred ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SlidingDoor::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<SlidingDoor*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<SlidingDoor*>( obj->data )->m_func = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<SlidingDoor*>( obj->data )->m_funcOut = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<SlidingDoor*>( obj->data )->m_once = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<SlidingDoor*>( obj->data )->m_done = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "switchPred" ){ static_cast<SlidingDoor*>( obj->data )->m_switchPred = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SlidingDoor::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 43 ];
	sprintf( bfr, "SlidingDoor (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface SlidingDoor::_sgs_interface[1] =
{{
	"SlidingDoor",
	SlidingDoor::_sgs_destruct, SlidingDoor::_sgs_gcmark, SlidingDoor::_sgs_getindex, SlidingDoor::_sgs_setindex, NULL, NULL, SlidingDoor::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__PickupItem__CallEvent( SGS_CTX )
{
	PickupItem* data; if( !SGS_PARSE_METHOD( C, PickupItem::_sgs_interface, data, PickupItem, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int PickupItem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<PickupItem*>( obj->data )->~PickupItem();
	return SGS_SUCCESS;
}

int PickupItem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int PickupItem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__PickupItem__CallEvent ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int PickupItem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<PickupItem*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int PickupItem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 42 ];
	sprintf( bfr, "PickupItem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface PickupItem::_sgs_interface[1] =
{{
	"PickupItem",
	PickupItem::_sgs_destruct, PickupItem::_sgs_gcmark, PickupItem::_sgs_getindex, PickupItem::_sgs_setindex, NULL, NULL, PickupItem::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__Actionable__CallEvent( SGS_CTX )
{
	Actionable* data; if( !SGS_PARSE_METHOD( C, Actionable::_sgs_interface, data, Actionable, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int Actionable::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<Actionable*>( obj->data )->~Actionable();
	return SGS_SUCCESS;
}

int Actionable::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int Actionable::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__Actionable__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_enabled ); return SGS_SUCCESS; }
		SGS_CASE( "timeEstimate" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeEstimate ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeActual ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_onSuccess ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.placePos ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Actionable::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<Actionable*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<Actionable*>( obj->data )->m_enabled = sgs_GetVarP<bool>()( C, val );
			static_cast<Actionable*>( obj->data )->sgsSetEnabled(); return SGS_SUCCESS; }
		SGS_CASE( "timeEstimate" ){ static_cast<Actionable*>( obj->data )->m_info.timeEstimate = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ static_cast<Actionable*>( obj->data )->m_info.timeActual = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ static_cast<Actionable*>( obj->data )->m_onSuccess = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Actionable::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 42 ];
	sprintf( bfr, "Actionable (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface Actionable::_sgs_interface[1] =
{{
	"Actionable",
	NULL, Actionable::_sgs_gcmark, Actionable::_sgs_getindex, Actionable::_sgs_setindex, NULL, NULL, Actionable::_sgs_dump, NULL, NULL, NULL, 
}};

