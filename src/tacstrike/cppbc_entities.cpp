// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "entities.hpp"

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
		SGS_CASE( "timeEstimate" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_info.timeEstimate ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_info.timeActual ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_onSuccess ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int PickupItem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "timeEstimate" ){ static_cast<PickupItem*>( obj->data )->m_info.timeEstimate = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ static_cast<PickupItem*>( obj->data )->m_info.timeActual = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ static_cast<PickupItem*>( obj->data )->m_onSuccess = sgs_GetVarP<sgsVariable>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int PickupItem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 42 ];
	sprintf( bfr, "PickupItem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

sgs_ObjInterface PickupItem::_sgs_interface[1] =
{{
	"PickupItem",
	PickupItem::_sgs_destruct, PickupItem::_sgs_gcmark, PickupItem::_sgs_getindex, PickupItem::_sgs_setindex, NULL, NULL, PickupItem::_sgs_dump, NULL, NULL, NULL, 
}};


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
		SGS_CASE( "timeEstimate" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeEstimate ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeActual ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_onSuccess ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Actionable::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
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
		sgs_StringConcat( C, 0 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface Actionable::_sgs_interface[1] =
{{
	"Actionable",
	Actionable::_sgs_destruct, Actionable::_sgs_gcmark, Actionable::_sgs_getindex, Actionable::_sgs_setindex, NULL, NULL, Actionable::_sgs_dump, NULL, NULL, NULL, 
}};

