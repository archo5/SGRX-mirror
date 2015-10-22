// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "isr3entities.hpp"

static int _sgs_method__ISR3Drone__CallEvent( SGS_CTX )
{
	ISR3Drone* data; if( !SGS_PARSE_METHOD( C, ISR3Drone::_sgs_interface, data, ISR3Drone, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int ISR3Drone::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ISR3Drone*>( obj->data )->~ISR3Drone();
	return SGS_SUCCESS;
}

int ISR3Drone::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int ISR3Drone::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__ISR3Drone__CallEvent ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ISR3Drone::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<ISR3Drone*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ISR3Drone::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 41 ];
	sprintf( bfr, "ISR3Drone (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface ISR3Drone::_sgs_interface[1] =
{{
	"ISR3Drone",
	ISR3Drone::_sgs_destruct, ISR3Drone::_sgs_gcmark, ISR3Drone::_sgs_getindex, ISR3Drone::_sgs_setindex, NULL, NULL, ISR3Drone::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__ISR3Player__CallEvent( SGS_CTX )
{
	ISR3Player* data; if( !SGS_PARSE_METHOD( C, ISR3Player::_sgs_interface, data, ISR3Player, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int ISR3Player::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ISR3Player*>( obj->data )->~ISR3Player();
	return SGS_SUCCESS;
}

int ISR3Player::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int ISR3Player::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__ISR3Player__CallEvent ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ISR3Player::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<ISR3Player*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ISR3Player::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 42 ];
	sprintf( bfr, "ISR3Player (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface ISR3Player::_sgs_interface[1] =
{{
	"ISR3Player",
	ISR3Player::_sgs_destruct, ISR3Player::_sgs_gcmark, ISR3Player::_sgs_getindex, ISR3Player::_sgs_setindex, NULL, NULL, ISR3Player::_sgs_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__ISR3Enemy__CallEvent( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, CallEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__ISR3Enemy__HasFact( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, HasFact ) ) return 0;
	sgs_PushVar(C,data->sgsHasFact( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ISR3Enemy__HasRecentFact( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, HasRecentFact ) ) return 0;
	sgs_PushVar(C,data->sgsHasRecentFact( sgs_GetVar<int>()(C,0), sgs_GetVar<TimeVal>()(C,1) )); return 1;
}

static int _sgs_method__ISR3Enemy__GetRecentFact( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, GetRecentFact ) ) return 0;
	return data->sgsGetRecentFact( sgs_GetVar<int>()(C,0), sgs_GetVar<TimeVal>()(C,1) );
}

int ISR3Enemy::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ISR3Enemy*>( obj->data )->~ISR3Enemy();
	return SGS_SUCCESS;
}

int ISR3Enemy::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int ISR3Enemy::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__ISR3Enemy__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "HasFact" ){ sgs_PushCFunction( C, _sgs_method__ISR3Enemy__HasFact ); return SGS_SUCCESS; }
		SGS_CASE( "HasRecentFact" ){ sgs_PushCFunction( C, _sgs_method__ISR3Enemy__HasRecentFact ); return SGS_SUCCESS; }
		SGS_CASE( "GetRecentFact" ){ sgs_PushCFunction( C, _sgs_method__ISR3Enemy__GetRecentFact ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ISR3Enemy::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<ISR3Enemy*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ISR3Enemy::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 41 ];
	sprintf( bfr, "ISR3Enemy (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface ISR3Enemy::_sgs_interface[1] =
{{
	"ISR3Enemy",
	ISR3Enemy::_sgs_destruct, ISR3Enemy::_sgs_gcmark, ISR3Enemy::_sgs_getindex, ISR3Enemy::_sgs_setindex, NULL, NULL, ISR3Enemy::_sgs_dump, NULL, NULL, NULL, 
}};

