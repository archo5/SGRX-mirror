// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build
// generated: 07/28/15 09:30:07

#include "scritem.hpp"

int SGRX_SIRigidBodyInfo::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->~SGRX_SIRigidBodyInfo();
	return SGS_SUCCESS;
}

int SGRX_SIRigidBodyInfo::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int SGRX_SIRigidBodyInfo::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->rotation ); return SGS_SUCCESS; }
		SGS_CASE( "friction" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->friction ); return SGS_SUCCESS; }
		SGS_CASE( "restitution" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->restitution ); return SGS_SUCCESS; }
		SGS_CASE( "mass" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->mass ); return SGS_SUCCESS; }
		SGS_CASE( "inertia" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->inertia ); return SGS_SUCCESS; }
		SGS_CASE( "linearDamping" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->linearDamping ); return SGS_SUCCESS; }
		SGS_CASE( "angularDamping" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->angularDamping ); return SGS_SUCCESS; }
		SGS_CASE( "linearFactor" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->linearFactor ); return SGS_SUCCESS; }
		SGS_CASE( "angularFactor" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->angularFactor ); return SGS_SUCCESS; }
		SGS_CASE( "kinematic" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->kinematic ); return SGS_SUCCESS; }
		SGS_CASE( "canSleep" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->canSleep ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->enabled ); return SGS_SUCCESS; }
		SGS_CASE( "group" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->group ); return SGS_SUCCESS; }
		SGS_CASE( "mask" ){ sgs_PushVar( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->mask ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_SIRigidBodyInfo::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "position" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->position = sgs_GetVarP<Vec3>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->rotation = sgs_GetVarP<Quat>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "friction" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->friction = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "restitution" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->restitution = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "mass" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->mass = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "inertia" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->inertia = sgs_GetVarP<Vec3>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "linearDamping" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->linearDamping = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "angularDamping" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->angularDamping = sgs_GetVarP<float>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "linearFactor" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->linearFactor = sgs_GetVarP<Vec3>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "angularFactor" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->angularFactor = sgs_GetVarP<Vec3>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "kinematic" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->kinematic = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "canSleep" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->canSleep = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->enabled = sgs_GetVarP<bool>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "group" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->group = sgs_GetVarP<uint16_t>()( C, val ); return SGS_SUCCESS; }
		SGS_CASE( "mask" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->mask = sgs_GetVarP<uint16_t>()( C, val ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_SIRigidBodyInfo::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 52 ];
	sprintf( bfr, "SGRX_SIRigidBodyInfo (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nfriction = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->friction, depth ).push( C ); }
		{ sgs_PushString( C, "\nrestitution = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->restitution, depth ).push( C ); }
		{ sgs_PushString( C, "\nmass = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->mass, depth ).push( C ); }
		{ sgs_PushString( C, "\ninertia = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->inertia, depth ).push( C ); }
		{ sgs_PushString( C, "\nlinearDamping = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->linearDamping, depth ).push( C ); }
		{ sgs_PushString( C, "\nangularDamping = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->angularDamping, depth ).push( C ); }
		{ sgs_PushString( C, "\nlinearFactor = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->linearFactor, depth ).push( C ); }
		{ sgs_PushString( C, "\nangularFactor = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->angularFactor, depth ).push( C ); }
		{ sgs_PushString( C, "\nkinematic = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->kinematic, depth ).push( C ); }
		{ sgs_PushString( C, "\ncanSleep = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->canSleep, depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\ngroup = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->group, depth ).push( C ); }
		{ sgs_PushString( C, "\nmask = " ); sgs_DumpData( C, static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->mask, depth ).push( C ); }
		sgs_StringConcat( C, 30 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface SGRX_SIRigidBodyInfo::_sgs_interface[1] =
{{
	"SGRX_SIRigidBodyInfo",
	SGRX_SIRigidBodyInfo::_sgs_destruct, SGRX_SIRigidBodyInfo::_sgs_gcmark, SGRX_SIRigidBodyInfo::_sgs_getindex, SGRX_SIRigidBodyInfo::_sgs_setindex, NULL, NULL, SGRX_SIRigidBodyInfo::_sgs_dump, NULL, NULL, NULL, 
}};


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

static int _sgs_method__SGRX_ScriptedItem__MIExists( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MIExists ) ) return 0;
	sgs_PushVar(C,data->MIExists( sgs_GetVar<int>()(C,0) )); return 1;
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

static int _sgs_method__SGRX_ScriptedItem__PSExists( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSExists ) ) return 0;
	sgs_PushVar(C,data->PSExists( sgs_GetVar<int>()(C,0) )); return 1;
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

static int _sgs_method__SGRX_ScriptedItem__PSSetMatrixFromMeshAABB( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSSetMatrixFromMeshAABB ) ) return 0;
	data->PSSetMatrixFromMeshAABB( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1) ); return 0;
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

static int _sgs_method__SGRX_ScriptedItem__RBMakeInfo( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBMakeInfo ) ) return 0;
	sgs_PushVar(C,data->RBMakeInfo(  )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__RBCreateFromMesh( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBCreateFromMesh ) ) return 0;
	data->RBCreateFromMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIRigidBodyInfo>()(C,2) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBDestroy( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBDestroy ) ) return 0;
	data->RBDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBExists( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBExists ) ) return 0;
	sgs_PushVar(C,data->RBExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__RBSetEnabled( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBSetEnabled ) ) return 0;
	data->RBSetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBGetPosition( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBGetPosition ) ) return 0;
	sgs_PushVar(C,data->RBGetPosition( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__RBSetPosition( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBSetPosition ) ) return 0;
	data->RBSetPosition( sgs_GetVar<int>()(C,0), sgs_GetVar<Vec3>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBGetMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBGetMatrix ) ) return 0;
	sgs_PushVar(C,data->RBGetMatrix( sgs_GetVar<int>()(C,0) )); return 1;
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
		SGS_CASE( "MIExists" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__MIExists ); return SGS_SUCCESS; }
		SGS_CASE( "MISetMesh" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__MISetMesh ); return SGS_SUCCESS; }
		SGS_CASE( "MISetEnabled" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__MISetEnabled ); return SGS_SUCCESS; }
		SGS_CASE( "MISetMatrix" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__MISetMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "PSCreate" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSCreate ); return SGS_SUCCESS; }
		SGS_CASE( "PSDestroy" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "PSExists" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSExists ); return SGS_SUCCESS; }
		SGS_CASE( "PSLoad" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSLoad ); return SGS_SUCCESS; }
		SGS_CASE( "PSSetMatrix" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSSetMatrix ); return SGS_SUCCESS; }
		SGS_CASE( "PSSetMatrixFromMeshAABB" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSSetMatrixFromMeshAABB ); return SGS_SUCCESS; }
		SGS_CASE( "PSPlay" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSPlay ); return SGS_SUCCESS; }
		SGS_CASE( "PSStop" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSStop ); return SGS_SUCCESS; }
		SGS_CASE( "PSTrigger" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__PSTrigger ); return SGS_SUCCESS; }
		SGS_CASE( "DSCreate" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DSCreate ); return SGS_SUCCESS; }
		SGS_CASE( "DSDestroy" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DSDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "DSResize" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DSResize ); return SGS_SUCCESS; }
		SGS_CASE( "DSClear" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__DSClear ); return SGS_SUCCESS; }
		SGS_CASE( "RBMakeInfo" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__RBMakeInfo ); return SGS_SUCCESS; }
		SGS_CASE( "RBCreateFromMesh" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__RBCreateFromMesh ); return SGS_SUCCESS; }
		SGS_CASE( "RBDestroy" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__RBDestroy ); return SGS_SUCCESS; }
		SGS_CASE( "RBExists" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__RBExists ); return SGS_SUCCESS; }
		SGS_CASE( "RBSetEnabled" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__RBSetEnabled ); return SGS_SUCCESS; }
		SGS_CASE( "RBGetPosition" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__RBGetPosition ); return SGS_SUCCESS; }
		SGS_CASE( "RBSetPosition" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__RBSetPosition ); return SGS_SUCCESS; }
		SGS_CASE( "RBGetMatrix" ){ sgs_PushCFunction( C, _sgs_method__SGRX_ScriptedItem__RBGetMatrix ); return SGS_SUCCESS; }
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

