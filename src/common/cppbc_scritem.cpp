// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build
// generated: 07/24/15 22:37:31

#include "scritem.hpp"

int SGRX_ScriptedItem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGRX_ScriptedItem*>( obj->data )->~SGRX_ScriptedItem();
	return SGS_SUCCESS;
}

int SGRX_ScriptedItem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int SGRX_ScriptedItem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int SGRX_ScriptedItem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int SGRX_ScriptedItem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 49 ];
	sprintf( bfr, "SGRX_ScriptedItem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface SGRX_ScriptedItem::_sgs_interface[1] =
{{
	"SGRX_ScriptedItem",
	SGRX_ScriptedItem::_sgs_destruct, SGRX_ScriptedItem::_sgs_gcmark, SGRX_ScriptedItem::_sgs_getindex, SGRX_ScriptedItem::_sgs_setindex, NULL, NULL, SGRX_ScriptedItem::_sgs_dump, NULL, NULL, NULL, 
}};

