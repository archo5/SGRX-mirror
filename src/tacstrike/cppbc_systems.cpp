// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "systems.hpp"

static int _sgs_method__InfoEmissionSystem__sgsUpdate( SGS_CTX )
{
	InfoEmissionSystem* data; if( !SGS_PARSE_METHOD( C, InfoEmissionSystem::_sgs_interface, data, InfoEmissionSystem, sgsUpdate ) ) return 0;
	data->sgsUpdate( sgs_GetVar<Entity::Handle>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<uint32_t>()(C,3) ); return 0;
}

static int _sgs_method__InfoEmissionSystem__sgsRemove( SGS_CTX )
{
	InfoEmissionSystem* data; if( !SGS_PARSE_METHOD( C, InfoEmissionSystem::_sgs_interface, data, InfoEmissionSystem, sgsRemove ) ) return 0;
	data->sgsRemove( sgs_GetVar<Entity::Handle>()(C,0) ); return 0;
}

int InfoEmissionSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<InfoEmissionSystem*>( obj->data )->~InfoEmissionSystem();
	return SGS_SUCCESS;
}

int InfoEmissionSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int InfoEmissionSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "Update" ){ sgs_PushCFunction( C, _sgs_method__InfoEmissionSystem__sgsUpdate ); return SGS_SUCCESS; }
		SGS_CASE( "Remove" ){ sgs_PushCFunction( C, _sgs_method__InfoEmissionSystem__sgsRemove ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int InfoEmissionSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int InfoEmissionSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 50 ];
	sprintf( bfr, "InfoEmissionSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface InfoEmissionSystem::_sgs_interface[1] =
{{
	"InfoEmissionSystem",
	NULL, InfoEmissionSystem::_sgs_gcmark, InfoEmissionSystem::_sgs_getindex, InfoEmissionSystem::_sgs_setindex, NULL, NULL, InfoEmissionSystem::_sgs_dump, NULL, NULL, NULL, 
}};


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
	NULL, MessagingSystem::_sgs_gcmark, MessagingSystem::_sgs_getindex, MessagingSystem::_sgs_setindex, NULL, NULL, MessagingSystem::_sgs_dump, NULL, NULL, NULL, 
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
	NULL, ObjectiveSystem::_sgs_gcmark, ObjectiveSystem::_sgs_getindex, ObjectiveSystem::_sgs_setindex, NULL, NULL, ObjectiveSystem::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__FlareSystem__sgsUpdate( SGS_CTX )
{
	FlareSystem* data; if( !SGS_PARSE_METHOD( C, FlareSystem::_sgs_interface, data, FlareSystem, sgsUpdate ) ) return 0;
	data->sgsUpdate( sgs_GetVarObj<void>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<Vec3>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<bool>()(C,4) ); return 0;
}

static int _sgs_method__FlareSystem__sgsRemove( SGS_CTX )
{
	FlareSystem* data; if( !SGS_PARSE_METHOD( C, FlareSystem::_sgs_interface, data, FlareSystem, sgsRemove ) ) return 0;
	data->sgsRemove( sgs_GetVarObj<void>()(C,0) ); return 0;
}

int FlareSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<FlareSystem*>( obj->data )->~FlareSystem();
	return SGS_SUCCESS;
}

int FlareSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int FlareSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "Update" ){ sgs_PushCFunction( C, _sgs_method__FlareSystem__sgsUpdate ); return SGS_SUCCESS; }
		SGS_CASE( "Remove" ){ sgs_PushCFunction( C, _sgs_method__FlareSystem__sgsRemove ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int FlareSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int FlareSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 43 ];
	sprintf( bfr, "FlareSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface FlareSystem::_sgs_interface[1] =
{{
	"FlareSystem",
	NULL, FlareSystem::_sgs_gcmark, FlareSystem::_sgs_getindex, FlareSystem::_sgs_setindex, NULL, NULL, FlareSystem::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__ScriptedSequenceSystem__sgsStart( SGS_CTX )
{
	ScriptedSequenceSystem* data; if( !SGS_PARSE_METHOD( C, ScriptedSequenceSystem::_sgs_interface, data, ScriptedSequenceSystem, sgsStart ) ) return 0;
	data->sgsStart( sgs_GetVar<sgsVariable>()(C,0), sgs_GetVar<float>()(C,1) ); return 0;
}

int ScriptedSequenceSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ScriptedSequenceSystem*>( obj->data )->~ScriptedSequenceSystem();
	return SGS_SUCCESS;
}

int ScriptedSequenceSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int ScriptedSequenceSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "Start" ){ sgs_PushCFunction( C, _sgs_method__ScriptedSequenceSystem__sgsStart ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<ScriptedSequenceSystem*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "time" ){ sgs_PushVar( C, static_cast<ScriptedSequenceSystem*>( obj->data )->m_time ); return SGS_SUCCESS; }
		SGS_CASE( "subtitle" ){ sgs_PushVar( C, static_cast<ScriptedSequenceSystem*>( obj->data )->m_subtitle ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ScriptedSequenceSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "func" ){ static_cast<ScriptedSequenceSystem*>( obj->data )->m_func = sgs_GetVarP<sgsVariable>()( C, val );
			static_cast<ScriptedSequenceSystem*>( obj->data )->_StartCutscene(); return SGS_SUCCESS; }
		SGS_CASE( "time" ){ static_cast<ScriptedSequenceSystem*>( obj->data )->m_time = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "subtitle" ){ static_cast<ScriptedSequenceSystem*>( obj->data )->m_subtitle = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ScriptedSequenceSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 54 ];
	sprintf( bfr, "ScriptedSequenceSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface ScriptedSequenceSystem::_sgs_interface[1] =
{{
	"ScriptedSequenceSystem",
	NULL, ScriptedSequenceSystem::_sgs_gcmark, ScriptedSequenceSystem::_sgs_getindex, ScriptedSequenceSystem::_sgs_setindex, NULL, NULL, ScriptedSequenceSystem::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__MusicSystem__sgsSetTrack( SGS_CTX )
{
	MusicSystem* data; if( !SGS_PARSE_METHOD( C, MusicSystem::_sgs_interface, data, MusicSystem, sgsSetTrack ) ) return 0;
	data->sgsSetTrack( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__MusicSystem__sgsSetVar( SGS_CTX )
{
	MusicSystem* data; if( !SGS_PARSE_METHOD( C, MusicSystem::_sgs_interface, data, MusicSystem, sgsSetVar ) ) return 0;
	data->sgsSetVar( sgs_GetVar<StringView>()(C,0), sgs_GetVar<float>()(C,1) ); return 0;
}

int MusicSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<MusicSystem*>( obj->data )->~MusicSystem();
	return SGS_SUCCESS;
}

int MusicSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int MusicSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "SetTrack" ){ sgs_PushCFunction( C, _sgs_method__MusicSystem__sgsSetTrack ); return SGS_SUCCESS; }
		SGS_CASE( "SetVar" ){ sgs_PushCFunction( C, _sgs_method__MusicSystem__sgsSetVar ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int MusicSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int MusicSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 43 ];
	sprintf( bfr, "MusicSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface MusicSystem::_sgs_interface[1] =
{{
	"MusicSystem",
	NULL, MusicSystem::_sgs_gcmark, MusicSystem::_sgs_getindex, MusicSystem::_sgs_setindex, NULL, NULL, MusicSystem::_sgs_dump, NULL, NULL, NULL, 
}};
