// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "level.hpp"

static int _sgs_method__Entity__OnEvent( SGS_CTX )
{
	Entity* data; if( !SGS_PARSE_METHOD( C, Entity::_sgs_interface, data, Entity, OnEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int Entity::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<Entity*>( obj->data )->~Entity();
	return SGS_SUCCESS;
}

int Entity::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int Entity::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__Entity__OnEvent ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Entity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<Entity*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Entity::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 38 ];
	sprintf( bfr, "Entity (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface Entity::_sgs_interface[1] =
{{
	"Entity",
	NULL, Entity::_sgsent_gcmark, Entity::_sgsent_getindex, Entity::_sgsent_setindex, NULL, NULL, Entity::_sgsent_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__GameLevel__SetNextLevel( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, SetNextLevel ) ) return 0;
	data->SetNextLevel( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__GameLevel__sgsFindEntity( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, sgsFindEntity ) ) return 0;
	sgs_PushVar(C,data->sgsFindEntity( sgs_GetVar<StringView>()(C,0) )); return 1;
}

static int _sgs_method__GameLevel__CallEntityByName( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, CallEntityByName ) ) return 0;
	data->CallEntityByName( sgs_GetVar<StringView>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__GameLevel__sgsSetCameraPosDir( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, sgsSetCameraPosDir ) ) return 0;
	data->sgsSetCameraPosDir( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<Vec3>()(C,1) ); return 0;
}

int GameLevel::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GameLevel*>( obj->data )->~GameLevel();
	return SGS_SUCCESS;
}

int GameLevel::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int GameLevel::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "SetLevel" ){ sgs_PushCFunction( C, _sgs_method__GameLevel__SetNextLevel ); return SGS_SUCCESS; }
		SGS_CASE( "FindEntity" ){ sgs_PushCFunction( C, _sgs_method__GameLevel__sgsFindEntity ); return SGS_SUCCESS; }
		SGS_CASE( "CallEntity" ){ sgs_PushCFunction( C, _sgs_method__GameLevel__CallEntityByName ); return SGS_SUCCESS; }
		SGS_CASE( "SetCameraPosDir" ){ sgs_PushCFunction( C, _sgs_method__GameLevel__sgsSetCameraPosDir ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameLevel::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int GameLevel::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 41 ];
	sprintf( bfr, "GameLevel (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface GameLevel::_sgs_interface[1] =
{{
	"GameLevel",
	NULL, GameLevel::_sgs_gcmark, GameLevel::_getindex, GameLevel::_setindex, NULL, NULL, GameLevel::_sgs_dump, NULL, NULL, NULL, 
}};
