// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "entities_ts.hpp"

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
		SGS_CASE( "state" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_enemyState ); return SGS_SUCCESS; }
		SGS_CASE( "moveTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_moveTime ); return SGS_SUCCESS; }
		SGS_CASE( "pauseTime" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_pauseTime ); return SGS_SUCCESS; }
		SGS_CASE( "fov" ){ sgs_PushVar( C, static_cast<TSCamera*>( obj->data )->m_fov ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCamera::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
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
		sgs_StringConcat( C, 0 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface TSCamera::_sgs_interface[1] =
{{
	"TSCamera",
	TSCamera::_sgs_destruct, TSCamera::_sgs_gcmark, TSCamera::_sgs_getindex, TSCamera::_sgs_setindex, NULL, NULL, TSCamera::_sgs_dump, NULL, NULL, NULL, 
}};


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
		SGS_CASE( "state" ){ sgs_PushVar( C, static_cast<TSCharacter*>( obj->data )->m_enemyState ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int TSCharacter::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 43 ];
	sprintf( bfr, "TSCharacter (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface TSCharacter::_sgs_interface[1] =
{{
	"TSCharacter",
	TSCharacter::_sgs_destruct, TSCharacter::_sgs_gcmark, TSCharacter::_sgs_getindex, TSCharacter::_sgs_setindex, NULL, NULL, TSCharacter::_sgs_dump, NULL, NULL, NULL, 
}};


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
		SGS_CASE( "state" ){ sgs_PushVar( C, static_cast<TSEnemy*>( obj->data )->m_enemyState ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int TSEnemy::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int TSEnemy::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 39 ];
	sprintf( bfr, "TSEnemy (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface TSEnemy::_sgs_interface[1] =
{{
	"TSEnemy",
	TSEnemy::_sgs_destruct, TSEnemy::_sgs_gcmark, TSEnemy::_sgs_getindex, TSEnemy::_sgs_setindex, NULL, NULL, TSEnemy::_sgs_dump, NULL, NULL, NULL, 
}};

