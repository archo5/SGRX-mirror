// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "entities_ts.hpp"

static int _sgs_method__TSCamera__OnEvent( SGS_CTX )
{
	TSCamera* data; if( !SGS_PARSE_METHOD( C, TSCamera::_sgs_interface, data, TSCamera, OnEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int TSCamera::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSCamera*>( obj->data )->~TSCamera();
	return SGS_SUCCESS;
}

int TSCamera::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int TSCamera::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__TSCamera__OnEvent ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_moveTime ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_pauseTime ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_fov ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCamera::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<TSCamera*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ static_cast<TSCamera*>( obj->data )->m_moveTime = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ static_cast<TSCamera*>( obj->data )->m_pauseTime = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ static_cast<TSCamera*>( obj->data )->m_fov = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCamera::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 40 ];
	sprintf( bfr, "TSCamera (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<TSCamera*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface TSCamera::_sgs_interface[1] =
{{
	"TSCamera",
	TSCamera::_sgsent_destruct, TSCamera::_sgsent_gcmark, TSCamera::_sgsent_getindex, TSCamera::_sgsent_setindex, NULL, NULL, TSCamera::_sgsent_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__TSCharacter__OnEvent( SGS_CTX )
{
	TSCharacter* data; if( !SGS_PARSE_METHOD( C, TSCharacter::_sgs_interface, data, TSCharacter, OnEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int TSCharacter::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSCharacter*>( obj->data )->~TSCharacter();
	return SGS_SUCCESS;
}

int TSCharacter::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int TSCharacter::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__TSCharacter__OnEvent ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<TSCharacter*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 43 ];
	sprintf( bfr, "TSCharacter (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<TSCharacter*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface TSCharacter::_sgs_interface[1] =
{{
	"TSCharacter",
	TSCharacter::_sgsent_destruct, TSCharacter::_sgsent_gcmark, TSCharacter::_sgsent_getindex, TSCharacter::_sgsent_setindex, NULL, NULL, TSCharacter::_sgsent_dump, NULL, NULL, NULL, 
}};


static int _sgs_method__TSEnemy__OnEvent( SGS_CTX )
{
	TSEnemy* data; if( !SGS_PARSE_METHOD( C, TSEnemy::_sgs_interface, data, TSEnemy, OnEvent ) ) return 0;
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int TSEnemy::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<TSEnemy*>( obj->data )->~TSEnemy();
	return SGS_SUCCESS;
}

int TSEnemy::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int TSEnemy::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<TSEnemy*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<TSEnemy*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<TSEnemy*>( obj->data )->_sgs_getTypeName() ); return SGS_SUCCESS; }
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<TSEnemy*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunction( C, _sgs_method__TSEnemy__OnEvent ); return SGS_SUCCESS; }
		SGS_CASE( "state" ){ sgs_PushVar( C, static_cast<TSEnemy*>( obj->data )->m_enemyState ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSEnemy::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewName" ){ static_cast<TSEnemy*>( obj->data )->m_viewName = sgs_GetVarP<String>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSEnemy::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 39 ];
	sprintf( bfr, "TSEnemy (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<TSEnemy*>( obj->data )->_sgs_getTypeName(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<TSEnemy*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface TSEnemy::_sgs_interface[1] =
{{
	"TSEnemy",
	TSEnemy::_sgsent_destruct, TSEnemy::_sgsent_gcmark, TSEnemy::_sgsent_getindex, TSEnemy::_sgsent_setindex, NULL, NULL, TSEnemy::_sgsent_dump, NULL, NULL, NULL, 
}};

