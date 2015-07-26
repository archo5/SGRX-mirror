// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build
// generated: 07/26/15 13:16:31

#include "scritem.hpp"

static int _sgs_method__SGRX_ScriptedItem__SetMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, SetMatrix ) ) return 0;
	data->SetMatrix( sgs_GetVar<Mat4>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MICreate( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MICreate ) ) return 0;
	data->MICreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MIDestroy( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MIDestroy ) ) return 0;
	data->MIDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MISetMesh( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MISetMesh ) ) return 0;
	data->MISetMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MISetEnabled( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MISetEnabled ) ) return 0;
	data->MISetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MISetMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MISetMatrix ) ) return 0;
	data->MISetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSCreate( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSCreate ) ) return 0;
	data->PSCreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSDestroy( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSDestroy ) ) return 0;
	data->PSDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSLoad( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSLoad ) ) return 0;
	data->PSLoad( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSSetMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSSetMatrix ) ) return 0;
	data->PSSetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSPlay( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSPlay ) ) return 0;
	data->PSPlay( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSStop( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSStop ) ) return 0;
	data->PSStop( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSTrigger( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSTrigger ) ) return 0;
	data->PSTrigger( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DSCreate( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DSCreate ) ) return 0;
	data->DSCreate( sgs_GetVar<StringView>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVar<uint32_t>()(C,2) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DSDestroy( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DSDestroy ) ) return 0;
	data->DSDestroy(  ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DSResize( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DSResize ) ) return 0;
	data->DSResize( sgs_GetVar<uint32_t>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DSClear( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DSClear ) ) return 0;
	data->DSClear(  ); return 0;
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
		SGS_CASE( "MICreate" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__MICreate ); return SGS_SUCCESS; }
		SGS_CASE( "MIDestroy" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__MIDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "MISetMesh" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__MISetMesh ); return SGS_SUCCESS; }
		SGS_CASE( "MISetEnabled" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__MISetEnabled ); return SGS_SUCCESS; }
		SGS_CASE( "MISetMatrix" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__MISetMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "PSCreate" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSCreate ); return SGS_SUCCESS; }
		SGS_CASE( "PSDestroy" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "PSLoad" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSLoad ); return SGS_SUCCESS; }
		SGS_CASE( "PSSetMatrix" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSSetMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "PSPlay" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSPlay ); return SGS_SUCCESS; }
		SGS_CASE( "PSStop" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSStop ); return SGS_SUCCESS; }
		SGS_CASE( "PSTrigger" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSTrigger ); return SGS_SUCCESS; }
		SGS_CASE( "DSCreate" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DSCreate ); return SGS_SUCCESS; }
		SGS_CASE( "DSDestroy" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DSDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "DSResize" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DSResize ); return SGS_SUCCESS; }
		SGS_CASE( "DSClear" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DSClear ); return SGS_SUCCESS; }
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

