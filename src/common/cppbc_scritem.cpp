// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "scritem.hpp"

int SGRX_SIRigidBodyInfo::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->C = C;
	static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->~SGRX_SIRigidBodyInfo();
	return SGS_SUCCESS;
}

int SGRX_SIRigidBodyInfo::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGRX_SIRigidBodyInfo::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->C, C );
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
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "position" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->position = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->rotation = sgs_GetVar<Quat>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "friction" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->friction = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "restitution" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->restitution = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "mass" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->mass = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "inertia" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->inertia = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "linearDamping" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->linearDamping = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "angularDamping" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->angularDamping = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "linearFactor" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->linearFactor = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "angularFactor" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->angularFactor = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "kinematic" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->kinematic = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "canSleep" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->canSleep = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->enabled = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "group" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->group = sgs_GetVar<uint16_t>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "mask" ){ static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->mask = sgs_GetVar<uint16_t>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_SIRigidBodyInfo::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIRigidBodyInfo*>( obj->data )->C, C );
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

static sgs_RegFuncConst SGRX_SIRigidBodyInfo__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int SGRX_SIRigidBodyInfo__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGRX_SIRigidBodyInfo__sgs_funcs,
		-1, "SGRX_SIRigidBodyInfo." );
	return 1;
}

static sgs_ObjInterface SGRX_SIRigidBodyInfo__sgs_interface =
{
	"SGRX_SIRigidBodyInfo",
	SGRX_SIRigidBodyInfo::_sgs_destruct, SGRX_SIRigidBodyInfo::_sgs_gcmark, SGRX_SIRigidBodyInfo::_sgs_getindex, SGRX_SIRigidBodyInfo::_sgs_setindex, NULL, NULL, SGRX_SIRigidBodyInfo::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGRX_SIRigidBodyInfo::_sgs_interface(SGRX_SIRigidBodyInfo__sgs_interface, SGRX_SIRigidBodyInfo__sgs_ifn);


int SGRX_SIHingeJointInfo::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGRX_SIHingeJointInfo*>( obj->data )->C = C;
	static_cast<SGRX_SIHingeJointInfo*>( obj->data )->~SGRX_SIHingeJointInfo();
	return SGS_SUCCESS;
}

int SGRX_SIHingeJointInfo::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIHingeJointInfo*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGRX_SIHingeJointInfo::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIHingeJointInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "pivotA" ){ sgs_PushVar( C, static_cast<SGRX_SIHingeJointInfo*>( obj->data )->pivotA ); return SGS_SUCCESS; }
		SGS_CASE( "pivotB" ){ sgs_PushVar( C, static_cast<SGRX_SIHingeJointInfo*>( obj->data )->pivotB ); return SGS_SUCCESS; }
		SGS_CASE( "axisA" ){ sgs_PushVar( C, static_cast<SGRX_SIHingeJointInfo*>( obj->data )->axisA ); return SGS_SUCCESS; }
		SGS_CASE( "axisB" ){ sgs_PushVar( C, static_cast<SGRX_SIHingeJointInfo*>( obj->data )->axisB ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_SIHingeJointInfo::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIHingeJointInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "pivotA" ){ static_cast<SGRX_SIHingeJointInfo*>( obj->data )->pivotA = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "pivotB" ){ static_cast<SGRX_SIHingeJointInfo*>( obj->data )->pivotB = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "axisA" ){ static_cast<SGRX_SIHingeJointInfo*>( obj->data )->axisA = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "axisB" ){ static_cast<SGRX_SIHingeJointInfo*>( obj->data )->axisB = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_SIHingeJointInfo::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIHingeJointInfo*>( obj->data )->C, C );
	char bfr[ 53 ];
	sprintf( bfr, "SGRX_SIHingeJointInfo (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\npivotA = " ); sgs_DumpData( C, static_cast<SGRX_SIHingeJointInfo*>( obj->data )->pivotA, depth ).push( C ); }
		{ sgs_PushString( C, "\npivotB = " ); sgs_DumpData( C, static_cast<SGRX_SIHingeJointInfo*>( obj->data )->pivotB, depth ).push( C ); }
		{ sgs_PushString( C, "\naxisA = " ); sgs_DumpData( C, static_cast<SGRX_SIHingeJointInfo*>( obj->data )->axisA, depth ).push( C ); }
		{ sgs_PushString( C, "\naxisB = " ); sgs_DumpData( C, static_cast<SGRX_SIHingeJointInfo*>( obj->data )->axisB, depth ).push( C ); }
		sgs_StringConcat( C, 8 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SGRX_SIHingeJointInfo__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int SGRX_SIHingeJointInfo__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGRX_SIHingeJointInfo__sgs_funcs,
		-1, "SGRX_SIHingeJointInfo." );
	return 1;
}

static sgs_ObjInterface SGRX_SIHingeJointInfo__sgs_interface =
{
	"SGRX_SIHingeJointInfo",
	SGRX_SIHingeJointInfo::_sgs_destruct, SGRX_SIHingeJointInfo::_sgs_gcmark, SGRX_SIHingeJointInfo::_sgs_getindex, SGRX_SIHingeJointInfo::_sgs_setindex, NULL, NULL, SGRX_SIHingeJointInfo::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGRX_SIHingeJointInfo::_sgs_interface(SGRX_SIHingeJointInfo__sgs_interface, SGRX_SIHingeJointInfo__sgs_ifn);


int SGRX_SIConeTwistJointInfo::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->C = C;
	static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->~SGRX_SIConeTwistJointInfo();
	return SGS_SUCCESS;
}

int SGRX_SIConeTwistJointInfo::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGRX_SIConeTwistJointInfo::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "frameA" ){ sgs_PushVar( C, static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->frameA ); return SGS_SUCCESS; }
		SGS_CASE( "frameB" ){ sgs_PushVar( C, static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->frameB ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_SIConeTwistJointInfo::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "frameA" ){ static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->frameA = sgs_GetVar<Mat4>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "frameB" ){ static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->frameB = sgs_GetVar<Mat4>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_SIConeTwistJointInfo::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->C, C );
	char bfr[ 57 ];
	sprintf( bfr, "SGRX_SIConeTwistJointInfo (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nframeA = " ); sgs_DumpData( C, static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->frameA, depth ).push( C ); }
		{ sgs_PushString( C, "\nframeB = " ); sgs_DumpData( C, static_cast<SGRX_SIConeTwistJointInfo*>( obj->data )->frameB, depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SGRX_SIConeTwistJointInfo__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int SGRX_SIConeTwistJointInfo__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGRX_SIConeTwistJointInfo__sgs_funcs,
		-1, "SGRX_SIConeTwistJointInfo." );
	return 1;
}

static sgs_ObjInterface SGRX_SIConeTwistJointInfo__sgs_interface =
{
	"SGRX_SIConeTwistJointInfo",
	SGRX_SIConeTwistJointInfo::_sgs_destruct, SGRX_SIConeTwistJointInfo::_sgs_gcmark, SGRX_SIConeTwistJointInfo::_sgs_getindex, SGRX_SIConeTwistJointInfo::_sgs_setindex, NULL, NULL, SGRX_SIConeTwistJointInfo::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGRX_SIConeTwistJointInfo::_sgs_interface(SGRX_SIConeTwistJointInfo__sgs_interface, SGRX_SIConeTwistJointInfo__sgs_ifn);


static int _sgs_method__SGRX_ScriptedItem__SetMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, SetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->SetMatrix( sgs_GetVar<Mat4>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MICreate( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MICreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MICreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MIDestroy( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MIDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MIDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MIExists( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MIExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->MIExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__MISetMesh( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MISetMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MISetEnabled( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MISetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MISetMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MISetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__MISetShaderConst( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, MISetShaderConst ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetShaderConst( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<Vec4>()(C,2) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSCreate( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSCreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSCreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSDestroy( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSExists( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->PSExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__PSLoad( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSLoad ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSLoad( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSSetMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSSetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSSetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSSetMatrixFromMeshAABB( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSSetMatrixFromMeshAABB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSSetMatrixFromMeshAABB( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSPlay( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSPlay ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSPlay( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSStop( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSStop ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSStop( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__PSTrigger( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, PSTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSTrigger( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DSCreate( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DSCreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSCreate( sgs_GetVar<StringView>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVar<StringView>()(C,2), sgs_GetVar<uint32_t>()(C,3) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DSDestroy( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DSDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSDestroy(  ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DSResize( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DSResize ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSResize( sgs_GetVar<uint32_t>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__DSClear( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, DSClear ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSClear(  ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBCreateFromMesh( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBCreateFromMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBCreateFromMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIRigidBodyInfo>()(C,2) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBCreateFromConvexPointSet( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBCreateFromConvexPointSet ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBCreateFromConvexPointSet( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVarObj<SGRX_SIRigidBodyInfo>()(C,2) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBDestroy( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBExists( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__RBSetEnabled( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBSetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBGetPosition( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBGetPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetPosition( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__RBSetPosition( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBSetPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetPosition( sgs_GetVar<int>()(C,0), sgs_GetVar<Vec3>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBGetRotation( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBGetRotation ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetRotation( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__RBSetRotation( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBSetRotation ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetRotation( sgs_GetVar<int>()(C,0), sgs_GetVar<Quat>()(C,1) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__RBGetMatrix( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBGetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetMatrix( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__RBApplyForce( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, RBApplyForce ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBApplyForce( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<Vec3>()(C,2), sgs_GetVar<Vec3>()(C,3) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__JTCreateHingeB2W( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, JTCreateHingeB2W ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateHingeB2W( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIHingeJointInfo>()(C,2) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__JTCreateHingeB2B( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, JTCreateHingeB2B ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateHingeB2B( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<int>()(C,2), sgs_GetVarObj<SGRX_SIHingeJointInfo>()(C,3) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__JTCreateConeTwistB2W( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, JTCreateConeTwistB2W ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateConeTwistB2W( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_SIConeTwistJointInfo>()(C,2) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__JTCreateConeTwistB2B( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, JTCreateConeTwistB2B ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateConeTwistB2B( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<int>()(C,2), sgs_GetVarObj<SGRX_SIConeTwistJointInfo>()(C,3) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__JTDestroy( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, JTDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__SGRX_ScriptedItem__JTExists( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, JTExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->JTExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__SGRX_ScriptedItem__JTSetEnabled( SGS_CTX )
{
	SGRX_ScriptedItem* data; if( !SGS_PARSE_METHOD( C, SGRX_ScriptedItem::_sgs_interface, data, SGRX_ScriptedItem, JTSetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTSetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

int SGRX_ScriptedItem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGRX_ScriptedItem*>( obj->data )->C = C;
	static_cast<SGRX_ScriptedItem*>( obj->data )->~SGRX_ScriptedItem();
	return SGS_SUCCESS;
}

int SGRX_ScriptedItem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_ScriptedItem*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGRX_ScriptedItem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_ScriptedItem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		if( sgs_PushIndex( C, static_cast<SGRX_ScriptedItem*>( obj->data )->m_variable.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int SGRX_ScriptedItem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_ScriptedItem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		if( sgs_SetIndex( C, static_cast<SGRX_ScriptedItem*>( obj->data )->m_variable.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int SGRX_ScriptedItem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_ScriptedItem*>( obj->data )->C, C );
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

static sgs_RegFuncConst SGRX_ScriptedItem__sgs_funcs[] =
{
	{ "SetMatrix", _sgs_method__SGRX_ScriptedItem__SetMatrix },
	{ "MICreate", _sgs_method__SGRX_ScriptedItem__MICreate },
	{ "MIDestroy", _sgs_method__SGRX_ScriptedItem__MIDestroy },
	{ "MIExists", _sgs_method__SGRX_ScriptedItem__MIExists },
	{ "MISetMesh", _sgs_method__SGRX_ScriptedItem__MISetMesh },
	{ "MISetEnabled", _sgs_method__SGRX_ScriptedItem__MISetEnabled },
	{ "MISetMatrix", _sgs_method__SGRX_ScriptedItem__MISetMatrix },
	{ "MISetShaderConst", _sgs_method__SGRX_ScriptedItem__MISetShaderConst },
	{ "PSCreate", _sgs_method__SGRX_ScriptedItem__PSCreate },
	{ "PSDestroy", _sgs_method__SGRX_ScriptedItem__PSDestroy },
	{ "PSExists", _sgs_method__SGRX_ScriptedItem__PSExists },
	{ "PSLoad", _sgs_method__SGRX_ScriptedItem__PSLoad },
	{ "PSSetMatrix", _sgs_method__SGRX_ScriptedItem__PSSetMatrix },
	{ "PSSetMatrixFromMeshAABB", _sgs_method__SGRX_ScriptedItem__PSSetMatrixFromMeshAABB },
	{ "PSPlay", _sgs_method__SGRX_ScriptedItem__PSPlay },
	{ "PSStop", _sgs_method__SGRX_ScriptedItem__PSStop },
	{ "PSTrigger", _sgs_method__SGRX_ScriptedItem__PSTrigger },
	{ "DSCreate", _sgs_method__SGRX_ScriptedItem__DSCreate },
	{ "DSDestroy", _sgs_method__SGRX_ScriptedItem__DSDestroy },
	{ "DSResize", _sgs_method__SGRX_ScriptedItem__DSResize },
	{ "DSClear", _sgs_method__SGRX_ScriptedItem__DSClear },
	{ "RBCreateFromMesh", _sgs_method__SGRX_ScriptedItem__RBCreateFromMesh },
	{ "RBCreateFromConvexPointSet", _sgs_method__SGRX_ScriptedItem__RBCreateFromConvexPointSet },
	{ "RBDestroy", _sgs_method__SGRX_ScriptedItem__RBDestroy },
	{ "RBExists", _sgs_method__SGRX_ScriptedItem__RBExists },
	{ "RBSetEnabled", _sgs_method__SGRX_ScriptedItem__RBSetEnabled },
	{ "RBGetPosition", _sgs_method__SGRX_ScriptedItem__RBGetPosition },
	{ "RBSetPosition", _sgs_method__SGRX_ScriptedItem__RBSetPosition },
	{ "RBGetRotation", _sgs_method__SGRX_ScriptedItem__RBGetRotation },
	{ "RBSetRotation", _sgs_method__SGRX_ScriptedItem__RBSetRotation },
	{ "RBGetMatrix", _sgs_method__SGRX_ScriptedItem__RBGetMatrix },
	{ "RBApplyForce", _sgs_method__SGRX_ScriptedItem__RBApplyForce },
	{ "JTCreateHingeB2W", _sgs_method__SGRX_ScriptedItem__JTCreateHingeB2W },
	{ "JTCreateHingeB2B", _sgs_method__SGRX_ScriptedItem__JTCreateHingeB2B },
	{ "JTCreateConeTwistB2W", _sgs_method__SGRX_ScriptedItem__JTCreateConeTwistB2W },
	{ "JTCreateConeTwistB2B", _sgs_method__SGRX_ScriptedItem__JTCreateConeTwistB2B },
	{ "JTDestroy", _sgs_method__SGRX_ScriptedItem__JTDestroy },
	{ "JTExists", _sgs_method__SGRX_ScriptedItem__JTExists },
	{ "JTSetEnabled", _sgs_method__SGRX_ScriptedItem__JTSetEnabled },
	{ NULL, NULL },
};

static int SGRX_ScriptedItem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGRX_ScriptedItem__sgs_funcs,
		-1, "SGRX_ScriptedItem." );
	return 1;
}

static sgs_ObjInterface SGRX_ScriptedItem__sgs_interface =
{
	"SGRX_ScriptedItem",
	SGRX_ScriptedItem::_sgs_destruct, SGRX_ScriptedItem::_sgs_gcmark, SGRX_ScriptedItem::_sgs_getindex, SGRX_ScriptedItem::_sgs_setindex, NULL, NULL, SGRX_ScriptedItem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGRX_ScriptedItem::_sgs_interface(SGRX_ScriptedItem__sgs_interface, SGRX_ScriptedItem__sgs_ifn);

