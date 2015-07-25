// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build
// generated: 07/25/15 20:16:47

#include "scritem.hpp"

static int _sgs_method__SGRX_ScriptedItem__SetMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, SetMatrix ) ) return 0;
	data->SetMatrix( sgs_GetVar<Mat4>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__CreateMeshInst( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, CreateMeshInst ) ) return 0;
	data->CreateMeshInst( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DestroyMeshInst( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DestroyMeshInst ) ) return 0;
	data->DestroyMeshInst( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__SetMesh( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, SetMesh ) ) return 0;
	data->SetMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__SetMeshInstMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, SetMeshInstMatrix ) ) return 0;
	data->SetMeshInstMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__CreatePartSys( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, CreatePartSys ) ) return 0;
	data->CreatePartSys( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DestroyPartSys( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DestroyPartSys ) ) return 0;
	data->DestroyPartSys( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__LoadPartSys( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, LoadPartSys ) ) return 0;
	data->LoadPartSys( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__SetPartSysMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, SetPartSysMatrix ) ) return 0;
	data->SetPartSysMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PartSysPlay( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PartSysPlay ) ) return 0;
	data->PartSysPlay( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PartSysStop( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PartSysStop ) ) return 0;
	data->PartSysStop( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PartSysTrigger( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PartSysTrigger ) ) return 0;
	data->PartSysTrigger( sgs_GetVar<int>()(C,0) ); return 0;
}

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
		SGS_CASE( "SetMatrix" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__SetMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "CreateMeshInst" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__CreateMeshInst ); return SGS_SUCCESS; }
		SGS_CASE( "DestroyMeshInst" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DestroyMeshInst ); return SGS_SUCCESS; }
		SGS_CASE( "SetMesh" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__SetMesh ); return SGS_SUCCESS; }
		SGS_CASE( "SetMeshInstMatrix" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__SetMeshInstMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "CreatePartSys" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__CreatePartSys ); return SGS_SUCCESS; }
		SGS_CASE( "DestroyPartSys" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DestroyPartSys ); return SGS_SUCCESS; }
		SGS_CASE( "LoadPartSys" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__LoadPartSys ); return SGS_SUCCESS; }
		SGS_CASE( "SetPartSysMatrix" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__SetPartSysMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "PartSysPlay" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PartSysPlay ); return SGS_SUCCESS; }
		SGS_CASE( "PartSysStop" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PartSysStop ); return SGS_SUCCESS; }
		SGS_CASE( "PartSysTrigger" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PartSysTrigger ); return SGS_SUCCESS; }
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
	SGRX_ScriptedItem::_sgs_destruct, SGRX_ScriptedItem::_sgs_gcmark, SGRX_ScriptedItem::_getindex, SGRX_ScriptedItem::_setindex, NULL, NULL, SGRX_ScriptedItem::_sgs_dump, NULL, NULL, NULL, 
}};

