// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "systems.hpp"

static int _sgs_method__MessagingSystem__sgsAddMsg( SGS_CTX )
{
	MessagingSystem* data; if( !SGS_PARSE_METHOD( C, MessagingSystem::_sgs_interface, data, MessagingSystem, sgsAddMsg ) ) return 0;
	data->sgsAddMsg( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVar<float>()(C,2) ); return 0;
}

int MessagingSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<MessagingSystem*>( obj->data )->~MessagingSystem();
	return SGS_SUCCESS;
}

int MessagingSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int MessagingSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "Add" ){ sgs_PushCFunction( C, _sgs_method__MessagingSystem__sgsAddMsg ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int MessagingSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int MessagingSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 47 ];
	sprintf( bfr, "MessagingSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface MessagingSystem::_sgs_interface[1] =
{{
	"MessagingSystem",
	MessagingSystem::_sgs_destruct, MessagingSystem::_sgs_gcmark, MessagingSystem::_sgs_getindex, MessagingSystem::_sgs_setindex, NULL, NULL, MessagingSystem::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__ObjectiveSystem__sgsAddObj( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, sgsAddObj ) ) return 0;
	sgs_PushVar(C,data->sgsAddObj( sgs_GetVar<StringView>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<StringView>()(C,2), sgs_GetVar<bool>()(C,3), sgs_GetVar<Vec3>()(C,4) )); return 1;
}

static int _sgs_method__ObjectiveSystem__sgsGetTitle( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, sgsGetTitle ) ) return 0;
	sgs_PushVar(C,data->sgsGetTitle( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ObjectiveSystem__sgsSetTitle( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, sgsSetTitle ) ) return 0;
	data->sgsSetTitle( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__ObjectiveSystem__sgsGetState( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, sgsGetState ) ) return 0;
	sgs_PushVar(C,data->sgsGetState( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ObjectiveSystem__sgsSetState( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, sgsSetState ) ) return 0;
	data->sgsSetState( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1) ); return 0;
}

int ObjectiveSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ObjectiveSystem*>( obj->data )->~ObjectiveSystem();
	return SGS_SUCCESS;
}

int ObjectiveSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int ObjectiveSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "Add" ){ sgs_PushCFunction( C, _sgs_method__ObjectiveSystem__sgsAddObj ); return SGS_SUCCESS; }
		SGS_CASE( "GetTitle" ){ sgs_PushCFunction( C, _sgs_method__ObjectiveSystem__sgsGetTitle ); return SGS_SUCCESS; }
		SGS_CASE( "SetTitle" ){ sgs_PushCFunction( C, _sgs_method__ObjectiveSystem__sgsSetTitle ); return SGS_SUCCESS; }
		SGS_CASE( "GetState" ){ sgs_PushCFunction( C, _sgs_method__ObjectiveSystem__sgsGetState ); return SGS_SUCCESS; }
		SGS_CASE( "SetState" ){ sgs_PushCFunction( C, _sgs_method__ObjectiveSystem__sgsSetState ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ObjectiveSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int ObjectiveSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 47 ];
	sprintf( bfr, "ObjectiveSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface ObjectiveSystem::_sgs_interface[1] =
{{
	"ObjectiveSystem",
	ObjectiveSystem::_sgs_destruct, ObjectiveSystem::_sgs_gcmark, ObjectiveSystem::_sgs_getindex, ObjectiveSystem::_sgs_setindex, NULL, NULL, ObjectiveSystem::_sgs_dump, NULL, NULL, NULL, 
}};

