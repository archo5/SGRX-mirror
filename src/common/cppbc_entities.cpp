// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "entities.hpp"

static int _sgs_method__Trigger__Invoke( SGS_CTX )
{
	Trigger* data; if( !SGS_PARSE_METHOD( C, Trigger::_sgs_interface, data, Trigger, Invoke ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__Trigger__SetupTrigger( SGS_CTX )
{
	Trigger* data; if( !SGS_PARSE_METHOD( C, Trigger::_sgs_interface, data, Trigger, SetupTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int Trigger::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<Trigger*>( obj->data )->C = C;
	static_cast<Trigger*>( obj->data )->~Trigger();
	return SGS_SUCCESS;
}

int Trigger::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Trigger*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int Trigger::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Trigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_id ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->m_done ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<Trigger*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Trigger::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Trigger*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Trigger*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<Trigger*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<Trigger*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<Trigger*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<Trigger*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<Trigger*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<Trigger*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<Trigger*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<Trigger*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<Trigger*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<Trigger*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<Trigger*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<Trigger*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<Trigger*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<Trigger*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<Trigger*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<Trigger*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<Trigger*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<Trigger*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<Trigger*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<Trigger*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Trigger::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Trigger*>( obj->data )->C, C );
	char bfr[ 39 ];
	sprintf( bfr, "Trigger (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_id, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_done, depth ).push( C ); }
		sgs_StringConcat( C, 42 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst Trigger__sgs_funcs[] =
{
	{ "Invoke", _sgs_method__Trigger__Invoke },
	{ "SetupTrigger", _sgs_method__Trigger__SetupTrigger },
	{ NULL, NULL },
};

static int Trigger__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		Trigger__sgs_funcs,
		-1, "Trigger." );
	return 1;
}

static sgs_ObjInterface Trigger__sgs_interface =
{
	"Trigger",
	Trigger::_sgs_destruct, Trigger::_sgs_gcmark, Trigger::_sgs_getindex, Trigger::_sgs_setindex, NULL, NULL, Trigger::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Trigger::_sgs_interface(Trigger__sgs_interface, Trigger__sgs_ifn, &Entity::_sgs_interface);


static int _sgs_method__SlidingDoor__Invoke( SGS_CTX )
{
	SlidingDoor* data; if( !SGS_PARSE_METHOD( C, SlidingDoor::_sgs_interface, data, SlidingDoor, Invoke ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Invoke( sgs_GetVar<bool>()(C,0) ); return 0;
}

static int _sgs_method__SlidingDoor__SetupTrigger( SGS_CTX )
{
	SlidingDoor* data; if( !SGS_PARSE_METHOD( C, SlidingDoor::_sgs_interface, data, SlidingDoor, SetupTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSetupTrigger( sgs_GetVar<bool>()(C,0), sgs_GetVar<sgsVariable>()(C,1), sgs_GetVar<sgsVariable>()(C,2) ); return 0;
}

int SlidingDoor::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SlidingDoor*>( obj->data )->C = C;
	static_cast<SlidingDoor*>( obj->data )->~SlidingDoor();
	return SGS_SUCCESS;
}

int SlidingDoor::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SlidingDoor*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SlidingDoor::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SlidingDoor*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_id ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_funcOut ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_once ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_done ); return SGS_SUCCESS; }
		SGS_CASE( "isSwitch" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_isSwitch ); return SGS_SUCCESS; }
		SGS_CASE( "switchPred" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->m_switchPred ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<SlidingDoor*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int SlidingDoor::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SlidingDoor*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<SlidingDoor*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<SlidingDoor*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<SlidingDoor*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<SlidingDoor*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<SlidingDoor*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<SlidingDoor*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "func" ){ static_cast<SlidingDoor*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "funcOut" ){ static_cast<SlidingDoor*>( obj->data )->m_funcOut = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "once" ){ static_cast<SlidingDoor*>( obj->data )->m_once = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "done" ){ static_cast<SlidingDoor*>( obj->data )->m_done = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "switchPred" ){ static_cast<SlidingDoor*>( obj->data )->m_switchPred = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<SlidingDoor*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int SlidingDoor::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SlidingDoor*>( obj->data )->C, C );
	char bfr[ 43 ];
	sprintf( bfr, "SlidingDoor (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_id, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_done, depth ).push( C ); }
		{ sgs_PushString( C, "\nisSwitch = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_isSwitch, depth ).push( C ); }
		{ sgs_PushString( C, "\nswitchPred = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->m_switchPred, depth ).push( C ); }
		sgs_StringConcat( C, 46 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SlidingDoor__sgs_funcs[] =
{
	{ "Invoke", _sgs_method__SlidingDoor__Invoke },
	{ "SetupTrigger", _sgs_method__SlidingDoor__SetupTrigger },
	{ NULL, NULL },
};

static int SlidingDoor__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SlidingDoor__sgs_funcs,
		-1, "SlidingDoor." );
	return 1;
}

static sgs_ObjInterface SlidingDoor__sgs_interface =
{
	"SlidingDoor",
	SlidingDoor::_sgs_destruct, SlidingDoor::_sgs_gcmark, SlidingDoor::_sgs_getindex, SlidingDoor::_sgs_setindex, NULL, NULL, SlidingDoor::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SlidingDoor::_sgs_interface(SlidingDoor__sgs_interface, SlidingDoor__sgs_ifn, &Trigger::_sgs_interface);


int PickupItem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<PickupItem*>( obj->data )->C = C;
	static_cast<PickupItem*>( obj->data )->~PickupItem();
	return SGS_SUCCESS;
}

int PickupItem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<PickupItem*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int PickupItem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<PickupItem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->m_id ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<PickupItem*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int PickupItem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<PickupItem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<PickupItem*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<PickupItem*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<PickupItem*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<PickupItem*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<PickupItem*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<PickupItem*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<PickupItem*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<PickupItem*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<PickupItem*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<PickupItem*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<PickupItem*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<PickupItem*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<PickupItem*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<PickupItem*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<PickupItem*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<PickupItem*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<PickupItem*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int PickupItem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<PickupItem*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "PickupItem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_id, depth ).push( C ); }
		sgs_StringConcat( C, 34 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst PickupItem__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int PickupItem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		PickupItem__sgs_funcs,
		-1, "PickupItem." );
	return 1;
}

static sgs_ObjInterface PickupItem__sgs_interface =
{
	"PickupItem",
	PickupItem::_sgs_destruct, PickupItem::_sgs_gcmark, PickupItem::_sgs_getindex, PickupItem::_sgs_setindex, NULL, NULL, PickupItem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface PickupItem::_sgs_interface(PickupItem__sgs_interface, PickupItem__sgs_ifn, &Entity::_sgs_interface);


int Actionable::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<Actionable*>( obj->data )->C = C;
	static_cast<Actionable*>( obj->data )->~Actionable();
	return SGS_SUCCESS;
}

int Actionable::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actionable*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int Actionable::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actionable*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.placePos ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_id ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_enabled ); return SGS_SUCCESS; }
		SGS_CASE( "timeEstimate" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeEstimate ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_info.timeActual ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->m_onSuccess ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<Actionable*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Actionable::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actionable*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Actionable*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<Actionable*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<Actionable*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<Actionable*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<Actionable*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<Actionable*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<Actionable*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<Actionable*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<Actionable*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<Actionable*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<Actionable*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<Actionable*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<Actionable*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<Actionable*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<Actionable*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<Actionable*>( obj->data )->m_enabled = sgs_GetVar<bool>()( C, 1 );
			static_cast<Actionable*>( obj->data )->sgsSetEnabled(); return SGS_SUCCESS; }
		SGS_CASE( "timeEstimate" ){ static_cast<Actionable*>( obj->data )->m_info.timeEstimate = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "timeActual" ){ static_cast<Actionable*>( obj->data )->m_info.timeActual = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "onSuccess" ){ static_cast<Actionable*>( obj->data )->m_onSuccess = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<Actionable*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Actionable::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actionable*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "Actionable (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.placePos, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_id, depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeEstimate = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.timeEstimate, depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeActual = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.timeActual, depth ).push( C ); }
		{ sgs_PushString( C, "\nonSuccess = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_onSuccess, depth ).push( C ); }
		sgs_StringConcat( C, 42 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst Actionable__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int Actionable__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		Actionable__sgs_funcs,
		-1, "Actionable." );
	return 1;
}

static sgs_ObjInterface Actionable__sgs_interface =
{
	"Actionable",
	NULL, Actionable::_sgs_gcmark, Actionable::_sgs_getindex, Actionable::_sgs_setindex, NULL, NULL, Actionable::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Actionable::_sgs_interface(Actionable__sgs_interface, Actionable__sgs_ifn, &Entity::_sgs_interface);


int MeshEntity::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<MeshEntity*>( obj->data )->C = C;
	static_cast<MeshEntity*>( obj->data )->~MeshEntity();
	return SGS_SUCCESS;
}

int MeshEntity::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MeshEntity*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int MeshEntity::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MeshEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->m_id ); return SGS_SUCCESS; }
		SGS_CASE( "isStatic" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->IsStatic() ); return SGS_SUCCESS; }
		SGS_CASE( "visible" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->IsVisible() ); return SGS_SUCCESS; }
		SGS_CASE( "mesh" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetMesh() ); return SGS_SUCCESS; }
		SGS_CASE( "solid" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->IsSolid() ); return SGS_SUCCESS; }
		SGS_CASE( "lightingMode" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->GetLightingMode() ); return SGS_SUCCESS; }
		SGS_CASE( "lmQuality" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->m_lmQuality ); return SGS_SUCCESS; }
		SGS_CASE( "castsLMS" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->m_castsLMS ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<MeshEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MeshEntity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MeshEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<MeshEntity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<MeshEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<MeshEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<MeshEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<MeshEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<MeshEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<MeshEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<MeshEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<MeshEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<MeshEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<MeshEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<MeshEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<MeshEntity*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<MeshEntity*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<MeshEntity*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<MeshEntity*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "isStatic" ){ static_cast<MeshEntity*>( obj->data )->SetStatic( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "visible" ){ static_cast<MeshEntity*>( obj->data )->SetVisible( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "mesh" ){ static_cast<MeshEntity*>( obj->data )->SetMesh( sgs_GetVar<MeshHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "solid" ){ static_cast<MeshEntity*>( obj->data )->SetSolid( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "lightingMode" ){ static_cast<MeshEntity*>( obj->data )->SetLightingMode( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "lmQuality" ){ static_cast<MeshEntity*>( obj->data )->m_lmQuality = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "castsLMS" ){ static_cast<MeshEntity*>( obj->data )->m_castsLMS = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<MeshEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MeshEntity::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MeshEntity*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "MeshEntity (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->m_id, depth ).push( C ); }
		{ sgs_PushString( C, "\nisStatic = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->IsStatic(), depth ).push( C ); }
		{ sgs_PushString( C, "\nvisible = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->IsVisible(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmesh = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetMesh(), depth ).push( C ); }
		{ sgs_PushString( C, "\nsolid = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->IsSolid(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlightingMode = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->GetLightingMode(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlmQuality = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->m_lmQuality, depth ).push( C ); }
		{ sgs_PushString( C, "\ncastsLMS = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->m_castsLMS, depth ).push( C ); }
		sgs_StringConcat( C, 48 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst MeshEntity__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int MeshEntity__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		MeshEntity__sgs_funcs,
		-1, "MeshEntity." );
	return 1;
}

static sgs_ObjInterface MeshEntity__sgs_interface =
{
	"MeshEntity",
	NULL, MeshEntity::_sgs_gcmark, MeshEntity::_sgs_getindex, MeshEntity::_sgs_setindex, NULL, NULL, MeshEntity::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface MeshEntity::_sgs_interface(MeshEntity__sgs_interface, MeshEntity__sgs_ifn, &Entity::_sgs_interface);


int LightEntity::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<LightEntity*>( obj->data )->C = C;
	static_cast<LightEntity*>( obj->data )->~LightEntity();
	return SGS_SUCCESS;
}

int LightEntity::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LightEntity*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int LightEntity::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LightEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->m_id ); return SGS_SUCCESS; }
		SGS_CASE( "isStatic" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->IsStatic() ); return SGS_SUCCESS; }
		SGS_CASE( "type" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetType() ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->IsEnabled() ); return SGS_SUCCESS; }
		SGS_CASE( "color" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetColor() ); return SGS_SUCCESS; }
		SGS_CASE( "intensity" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetIntensity() ); return SGS_SUCCESS; }
		SGS_CASE( "range" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetRange() ); return SGS_SUCCESS; }
		SGS_CASE( "power" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetPower() ); return SGS_SUCCESS; }
		SGS_CASE( "angle" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetAngle() ); return SGS_SUCCESS; }
		SGS_CASE( "aspect" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetAspect() ); return SGS_SUCCESS; }
		SGS_CASE( "hasShadows" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->HasShadows() ); return SGS_SUCCESS; }
		SGS_CASE( "cookieTexture" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetCookieTexture() ); return SGS_SUCCESS; }
		SGS_CASE( "flareSize" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetFlareSize() ); return SGS_SUCCESS; }
		SGS_CASE( "flareOffset" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->GetFlareOffset() ); return SGS_SUCCESS; }
		SGS_CASE( "innerAngle" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->m_innerAngle ); return SGS_SUCCESS; }
		SGS_CASE( "spotCurve" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->m_spotCurve ); return SGS_SUCCESS; }
		SGS_CASE( "lightRadius" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->lightRadius ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<LightEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int LightEntity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LightEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<LightEntity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<LightEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<LightEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<LightEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<LightEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<LightEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<LightEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<LightEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<LightEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<LightEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<LightEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<LightEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<LightEntity*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<LightEntity*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<LightEntity*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<LightEntity*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "isStatic" ){ static_cast<LightEntity*>( obj->data )->SetStatic( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "type" ){ static_cast<LightEntity*>( obj->data )->SetType( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<LightEntity*>( obj->data )->SetEnabled( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "color" ){ static_cast<LightEntity*>( obj->data )->SetColor( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "intensity" ){ static_cast<LightEntity*>( obj->data )->SetIntensity( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "range" ){ static_cast<LightEntity*>( obj->data )->SetRange( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "power" ){ static_cast<LightEntity*>( obj->data )->SetPower( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "angle" ){ static_cast<LightEntity*>( obj->data )->SetAngle( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "aspect" ){ static_cast<LightEntity*>( obj->data )->SetAspect( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "hasShadows" ){ static_cast<LightEntity*>( obj->data )->SetShadows( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "cookieTexture" ){ static_cast<LightEntity*>( obj->data )->SetCookieTexture( sgs_GetVar<TextureHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "flareSize" ){ static_cast<LightEntity*>( obj->data )->SetFlareSize( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "flareOffset" ){ static_cast<LightEntity*>( obj->data )->SetFlareOffset( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "innerAngle" ){ static_cast<LightEntity*>( obj->data )->m_innerAngle = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "spotCurve" ){ static_cast<LightEntity*>( obj->data )->m_spotCurve = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "lightRadius" ){ static_cast<LightEntity*>( obj->data )->lightRadius = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<LightEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int LightEntity::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LightEntity*>( obj->data )->C, C );
	char bfr[ 43 ];
	sprintf( bfr, "LightEntity (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->m_id, depth ).push( C ); }
		{ sgs_PushString( C, "\nisStatic = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->IsStatic(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntype = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetType(), depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->IsEnabled(), depth ).push( C ); }
		{ sgs_PushString( C, "\ncolor = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetColor(), depth ).push( C ); }
		{ sgs_PushString( C, "\nintensity = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetIntensity(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrange = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetRange(), depth ).push( C ); }
		{ sgs_PushString( C, "\npower = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetPower(), depth ).push( C ); }
		{ sgs_PushString( C, "\nangle = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetAngle(), depth ).push( C ); }
		{ sgs_PushString( C, "\naspect = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetAspect(), depth ).push( C ); }
		{ sgs_PushString( C, "\nhasShadows = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->HasShadows(), depth ).push( C ); }
		{ sgs_PushString( C, "\ncookieTexture = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetCookieTexture(), depth ).push( C ); }
		{ sgs_PushString( C, "\nflareSize = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetFlareSize(), depth ).push( C ); }
		{ sgs_PushString( C, "\nflareOffset = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->GetFlareOffset(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninnerAngle = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->m_innerAngle, depth ).push( C ); }
		{ sgs_PushString( C, "\nspotCurve = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->m_spotCurve, depth ).push( C ); }
		{ sgs_PushString( C, "\nlightRadius = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->lightRadius, depth ).push( C ); }
		sgs_StringConcat( C, 66 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst LightEntity__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int LightEntity__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		LightEntity__sgs_funcs,
		-1, "LightEntity." );
	return 1;
}

static sgs_ObjInterface LightEntity__sgs_interface =
{
	"LightEntity",
	NULL, LightEntity::_sgs_gcmark, LightEntity::_sgs_getindex, LightEntity::_sgs_setindex, NULL, NULL, LightEntity::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface LightEntity::_sgs_interface(LightEntity__sgs_interface, LightEntity__sgs_ifn, &Entity::_sgs_interface);


int SGRX_RigidBodyInfo::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGRX_RigidBodyInfo*>( obj->data )->C = C;
	static_cast<SGRX_RigidBodyInfo*>( obj->data )->~SGRX_RigidBodyInfo();
	return SGS_SUCCESS;
}

int SGRX_RigidBodyInfo::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_RigidBodyInfo*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGRX_RigidBodyInfo::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_RigidBodyInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->position ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->rotation ); return SGS_SUCCESS; }
		SGS_CASE( "friction" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->friction ); return SGS_SUCCESS; }
		SGS_CASE( "restitution" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->restitution ); return SGS_SUCCESS; }
		SGS_CASE( "mass" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->mass ); return SGS_SUCCESS; }
		SGS_CASE( "inertia" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->inertia ); return SGS_SUCCESS; }
		SGS_CASE( "linearDamping" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->linearDamping ); return SGS_SUCCESS; }
		SGS_CASE( "angularDamping" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->angularDamping ); return SGS_SUCCESS; }
		SGS_CASE( "linearFactor" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->linearFactor ); return SGS_SUCCESS; }
		SGS_CASE( "angularFactor" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->angularFactor ); return SGS_SUCCESS; }
		SGS_CASE( "kinematic" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->kinematic ); return SGS_SUCCESS; }
		SGS_CASE( "canSleep" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->canSleep ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->enabled ); return SGS_SUCCESS; }
		SGS_CASE( "group" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->group ); return SGS_SUCCESS; }
		SGS_CASE( "mask" ){ sgs_PushVar( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->mask ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_RigidBodyInfo::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_RigidBodyInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "position" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->position = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->rotation = sgs_GetVar<Quat>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "friction" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->friction = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "restitution" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->restitution = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "mass" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->mass = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "inertia" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->inertia = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "linearDamping" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->linearDamping = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "angularDamping" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->angularDamping = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "linearFactor" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->linearFactor = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "angularFactor" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->angularFactor = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "kinematic" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->kinematic = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "canSleep" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->canSleep = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->enabled = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "group" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->group = sgs_GetVar<uint16_t>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "mask" ){ static_cast<SGRX_RigidBodyInfo*>( obj->data )->mask = sgs_GetVar<uint16_t>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_RigidBodyInfo::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_RigidBodyInfo*>( obj->data )->C, C );
	char bfr[ 50 ];
	sprintf( bfr, "SGRX_RigidBodyInfo (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->position, depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->rotation, depth ).push( C ); }
		{ sgs_PushString( C, "\nfriction = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->friction, depth ).push( C ); }
		{ sgs_PushString( C, "\nrestitution = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->restitution, depth ).push( C ); }
		{ sgs_PushString( C, "\nmass = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->mass, depth ).push( C ); }
		{ sgs_PushString( C, "\ninertia = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->inertia, depth ).push( C ); }
		{ sgs_PushString( C, "\nlinearDamping = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->linearDamping, depth ).push( C ); }
		{ sgs_PushString( C, "\nangularDamping = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->angularDamping, depth ).push( C ); }
		{ sgs_PushString( C, "\nlinearFactor = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->linearFactor, depth ).push( C ); }
		{ sgs_PushString( C, "\nangularFactor = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->angularFactor, depth ).push( C ); }
		{ sgs_PushString( C, "\nkinematic = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->kinematic, depth ).push( C ); }
		{ sgs_PushString( C, "\ncanSleep = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->canSleep, depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\ngroup = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->group, depth ).push( C ); }
		{ sgs_PushString( C, "\nmask = " ); sgs_DumpData( C, static_cast<SGRX_RigidBodyInfo*>( obj->data )->mask, depth ).push( C ); }
		sgs_StringConcat( C, 30 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SGRX_RigidBodyInfo__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int SGRX_RigidBodyInfo__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGRX_RigidBodyInfo__sgs_funcs,
		-1, "SGRX_RigidBodyInfo." );
	return 1;
}

static sgs_ObjInterface SGRX_RigidBodyInfo__sgs_interface =
{
	"SGRX_RigidBodyInfo",
	SGRX_RigidBodyInfo::_sgs_destruct, SGRX_RigidBodyInfo::_sgs_gcmark, SGRX_RigidBodyInfo::_sgs_getindex, SGRX_RigidBodyInfo::_sgs_setindex, NULL, NULL, SGRX_RigidBodyInfo::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGRX_RigidBodyInfo::_sgs_interface(SGRX_RigidBodyInfo__sgs_interface, SGRX_RigidBodyInfo__sgs_ifn);


int SGRX_HingeJointInfo::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGRX_HingeJointInfo*>( obj->data )->C = C;
	static_cast<SGRX_HingeJointInfo*>( obj->data )->~SGRX_HingeJointInfo();
	return SGS_SUCCESS;
}

int SGRX_HingeJointInfo::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_HingeJointInfo*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGRX_HingeJointInfo::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_HingeJointInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "pivotA" ){ sgs_PushVar( C, static_cast<SGRX_HingeJointInfo*>( obj->data )->pivotA ); return SGS_SUCCESS; }
		SGS_CASE( "pivotB" ){ sgs_PushVar( C, static_cast<SGRX_HingeJointInfo*>( obj->data )->pivotB ); return SGS_SUCCESS; }
		SGS_CASE( "axisA" ){ sgs_PushVar( C, static_cast<SGRX_HingeJointInfo*>( obj->data )->axisA ); return SGS_SUCCESS; }
		SGS_CASE( "axisB" ){ sgs_PushVar( C, static_cast<SGRX_HingeJointInfo*>( obj->data )->axisB ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_HingeJointInfo::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_HingeJointInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "pivotA" ){ static_cast<SGRX_HingeJointInfo*>( obj->data )->pivotA = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "pivotB" ){ static_cast<SGRX_HingeJointInfo*>( obj->data )->pivotB = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "axisA" ){ static_cast<SGRX_HingeJointInfo*>( obj->data )->axisA = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "axisB" ){ static_cast<SGRX_HingeJointInfo*>( obj->data )->axisB = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_HingeJointInfo::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_HingeJointInfo*>( obj->data )->C, C );
	char bfr[ 51 ];
	sprintf( bfr, "SGRX_HingeJointInfo (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\npivotA = " ); sgs_DumpData( C, static_cast<SGRX_HingeJointInfo*>( obj->data )->pivotA, depth ).push( C ); }
		{ sgs_PushString( C, "\npivotB = " ); sgs_DumpData( C, static_cast<SGRX_HingeJointInfo*>( obj->data )->pivotB, depth ).push( C ); }
		{ sgs_PushString( C, "\naxisA = " ); sgs_DumpData( C, static_cast<SGRX_HingeJointInfo*>( obj->data )->axisA, depth ).push( C ); }
		{ sgs_PushString( C, "\naxisB = " ); sgs_DumpData( C, static_cast<SGRX_HingeJointInfo*>( obj->data )->axisB, depth ).push( C ); }
		sgs_StringConcat( C, 8 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SGRX_HingeJointInfo__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int SGRX_HingeJointInfo__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGRX_HingeJointInfo__sgs_funcs,
		-1, "SGRX_HingeJointInfo." );
	return 1;
}

static sgs_ObjInterface SGRX_HingeJointInfo__sgs_interface =
{
	"SGRX_HingeJointInfo",
	SGRX_HingeJointInfo::_sgs_destruct, SGRX_HingeJointInfo::_sgs_gcmark, SGRX_HingeJointInfo::_sgs_getindex, SGRX_HingeJointInfo::_sgs_setindex, NULL, NULL, SGRX_HingeJointInfo::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGRX_HingeJointInfo::_sgs_interface(SGRX_HingeJointInfo__sgs_interface, SGRX_HingeJointInfo__sgs_ifn);


int SGRX_ConeTwistJointInfo::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->C = C;
	static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->~SGRX_ConeTwistJointInfo();
	return SGS_SUCCESS;
}

int SGRX_ConeTwistJointInfo::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGRX_ConeTwistJointInfo::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "frameA" ){ sgs_PushVar( C, static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->frameA ); return SGS_SUCCESS; }
		SGS_CASE( "frameB" ){ sgs_PushVar( C, static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->frameB ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_ConeTwistJointInfo::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "frameA" ){ static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->frameA = sgs_GetVar<Mat4>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "frameB" ){ static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->frameB = sgs_GetVar<Mat4>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGRX_ConeTwistJointInfo::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->C, C );
	char bfr[ 55 ];
	sprintf( bfr, "SGRX_ConeTwistJointInfo (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nframeA = " ); sgs_DumpData( C, static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->frameA, depth ).push( C ); }
		{ sgs_PushString( C, "\nframeB = " ); sgs_DumpData( C, static_cast<SGRX_ConeTwistJointInfo*>( obj->data )->frameB, depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SGRX_ConeTwistJointInfo__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int SGRX_ConeTwistJointInfo__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGRX_ConeTwistJointInfo__sgs_funcs,
		-1, "SGRX_ConeTwistJointInfo." );
	return 1;
}

static sgs_ObjInterface SGRX_ConeTwistJointInfo__sgs_interface =
{
	"SGRX_ConeTwistJointInfo",
	SGRX_ConeTwistJointInfo::_sgs_destruct, SGRX_ConeTwistJointInfo::_sgs_gcmark, SGRX_ConeTwistJointInfo::_sgs_getindex, SGRX_ConeTwistJointInfo::_sgs_setindex, NULL, NULL, SGRX_ConeTwistJointInfo::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGRX_ConeTwistJointInfo::_sgs_interface(SGRX_ConeTwistJointInfo__sgs_interface, SGRX_ConeTwistJointInfo__sgs_ifn);


static int _sgs_method__MultiEntity__MICreate( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MICreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MICreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__MIDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MIDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MIDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__MIExists( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MIExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->MIExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__MISetMesh( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__MISetEnabled( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__MISetMatrix( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__MISetShaderConst( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetShaderConst ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetShaderConst( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<Vec4>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__MISetLayers( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, MISetLayers ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->MISetLayers( sgs_GetVar<int>()(C,0), sgs_GetVar<uint32_t>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSCreate( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSCreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSCreate( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__PSExists( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->PSExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__PSLoad( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSLoad ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSLoad( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSSetMatrix( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSSetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSSetMatrix( sgs_GetVar<int>()(C,0), sgs_GetVar<Mat4>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSSetMatrixFromMeshAABB( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSSetMatrixFromMeshAABB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSSetMatrixFromMeshAABB( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__PSPlay( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSPlay ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSPlay( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__PSStop( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSStop ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSStop( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__PSTrigger( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, PSTrigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PSTrigger( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__DSCreate( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, DSCreate ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSCreate( sgs_GetVar<StringView>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVar<StringView>()(C,2), sgs_GetVar<uint32_t>()(C,3) ); return 0;
}

static int _sgs_method__MultiEntity__DSDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, DSDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSDestroy(  ); return 0;
}

static int _sgs_method__MultiEntity__DSResize( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, DSResize ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSResize( sgs_GetVar<uint32_t>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__DSClear( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, DSClear ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DSClear(  ); return 0;
}

static int _sgs_method__MultiEntity__RBCreateFromMesh( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBCreateFromMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBCreateFromMesh( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_RigidBodyInfo>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__RBCreateFromConvexPointSet( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBCreateFromConvexPointSet ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBCreateFromConvexPointSet( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVarObj<SGRX_RigidBodyInfo>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__RBDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__RBExists( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__RBSetEnabled( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBSetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__RBGetPosition( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBGetPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetPosition( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__RBSetPosition( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBSetPosition ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetPosition( sgs_GetVar<int>()(C,0), sgs_GetVar<Vec3>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__RBGetRotation( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBGetRotation ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetRotation( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__RBSetRotation( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBSetRotation ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBSetRotation( sgs_GetVar<int>()(C,0), sgs_GetVar<Quat>()(C,1) ); return 0;
}

static int _sgs_method__MultiEntity__RBGetMatrix( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBGetMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->RBGetMatrix( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__RBApplyForce( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, RBApplyForce ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RBApplyForce( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<Vec3>()(C,2), sgs_GetVar<Vec3>()(C,3) ); return 0;
}

static int _sgs_method__MultiEntity__JTCreateHingeB2W( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTCreateHingeB2W ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateHingeB2W( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_HingeJointInfo>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__JTCreateHingeB2B( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTCreateHingeB2B ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateHingeB2B( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<int>()(C,2), sgs_GetVarObj<SGRX_HingeJointInfo>()(C,3) ); return 0;
}

static int _sgs_method__MultiEntity__JTCreateConeTwistB2W( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTCreateConeTwistB2W ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateConeTwistB2W( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVarObj<SGRX_ConeTwistJointInfo>()(C,2) ); return 0;
}

static int _sgs_method__MultiEntity__JTCreateConeTwistB2B( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTCreateConeTwistB2B ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTCreateConeTwistB2B( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<int>()(C,2), sgs_GetVarObj<SGRX_ConeTwistJointInfo>()(C,3) ); return 0;
}

static int _sgs_method__MultiEntity__JTDestroy( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTDestroy ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTDestroy( sgs_GetVar<int>()(C,0) ); return 0;
}

static int _sgs_method__MultiEntity__JTExists( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTExists ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->JTExists( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__JTSetEnabled( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, JTSetEnabled ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->JTSetEnabled( sgs_GetVar<int>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

int MultiEntity::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<MultiEntity*>( obj->data )->C = C;
	static_cast<MultiEntity*>( obj->data )->~MultiEntity();
	return SGS_SUCCESS;
}

int MultiEntity::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MultiEntity*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int MultiEntity::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MultiEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->m_id ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<MultiEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MultiEntity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MultiEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<MultiEntity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<MultiEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<MultiEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<MultiEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<MultiEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<MultiEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<MultiEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<MultiEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<MultiEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<MultiEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<MultiEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<MultiEntity*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<MultiEntity*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<MultiEntity*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<MultiEntity*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<MultiEntity*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<MultiEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MultiEntity::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MultiEntity*>( obj->data )->C, C );
	char bfr[ 43 ];
	sprintf( bfr, "MultiEntity (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_id, depth ).push( C ); }
		sgs_StringConcat( C, 34 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst MultiEntity__sgs_funcs[] =
{
	{ "MICreate", _sgs_method__MultiEntity__MICreate },
	{ "MIDestroy", _sgs_method__MultiEntity__MIDestroy },
	{ "MIExists", _sgs_method__MultiEntity__MIExists },
	{ "MISetMesh", _sgs_method__MultiEntity__MISetMesh },
	{ "MISetEnabled", _sgs_method__MultiEntity__MISetEnabled },
	{ "MISetMatrix", _sgs_method__MultiEntity__MISetMatrix },
	{ "MISetShaderConst", _sgs_method__MultiEntity__MISetShaderConst },
	{ "MISetLayers", _sgs_method__MultiEntity__MISetLayers },
	{ "PSCreate", _sgs_method__MultiEntity__PSCreate },
	{ "PSDestroy", _sgs_method__MultiEntity__PSDestroy },
	{ "PSExists", _sgs_method__MultiEntity__PSExists },
	{ "PSLoad", _sgs_method__MultiEntity__PSLoad },
	{ "PSSetMatrix", _sgs_method__MultiEntity__PSSetMatrix },
	{ "PSSetMatrixFromMeshAABB", _sgs_method__MultiEntity__PSSetMatrixFromMeshAABB },
	{ "PSPlay", _sgs_method__MultiEntity__PSPlay },
	{ "PSStop", _sgs_method__MultiEntity__PSStop },
	{ "PSTrigger", _sgs_method__MultiEntity__PSTrigger },
	{ "DSCreate", _sgs_method__MultiEntity__DSCreate },
	{ "DSDestroy", _sgs_method__MultiEntity__DSDestroy },
	{ "DSResize", _sgs_method__MultiEntity__DSResize },
	{ "DSClear", _sgs_method__MultiEntity__DSClear },
	{ "RBCreateFromMesh", _sgs_method__MultiEntity__RBCreateFromMesh },
	{ "RBCreateFromConvexPointSet", _sgs_method__MultiEntity__RBCreateFromConvexPointSet },
	{ "RBDestroy", _sgs_method__MultiEntity__RBDestroy },
	{ "RBExists", _sgs_method__MultiEntity__RBExists },
	{ "RBSetEnabled", _sgs_method__MultiEntity__RBSetEnabled },
	{ "RBGetPosition", _sgs_method__MultiEntity__RBGetPosition },
	{ "RBSetPosition", _sgs_method__MultiEntity__RBSetPosition },
	{ "RBGetRotation", _sgs_method__MultiEntity__RBGetRotation },
	{ "RBSetRotation", _sgs_method__MultiEntity__RBSetRotation },
	{ "RBGetMatrix", _sgs_method__MultiEntity__RBGetMatrix },
	{ "RBApplyForce", _sgs_method__MultiEntity__RBApplyForce },
	{ "JTCreateHingeB2W", _sgs_method__MultiEntity__JTCreateHingeB2W },
	{ "JTCreateHingeB2B", _sgs_method__MultiEntity__JTCreateHingeB2B },
	{ "JTCreateConeTwistB2W", _sgs_method__MultiEntity__JTCreateConeTwistB2W },
	{ "JTCreateConeTwistB2B", _sgs_method__MultiEntity__JTCreateConeTwistB2B },
	{ "JTDestroy", _sgs_method__MultiEntity__JTDestroy },
	{ "JTExists", _sgs_method__MultiEntity__JTExists },
	{ "JTSetEnabled", _sgs_method__MultiEntity__JTSetEnabled },
	{ NULL, NULL },
};

static int MultiEntity__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		MultiEntity__sgs_funcs,
		-1, "MultiEntity." );
	return 1;
}

static sgs_ObjInterface MultiEntity__sgs_interface =
{
	"MultiEntity",
	NULL, MultiEntity::_sgs_gcmark, MultiEntity::_sgs_getindex, MultiEntity::_sgs_setindex, NULL, NULL, MultiEntity::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface MultiEntity::_sgs_interface(MultiEntity__sgs_interface, MultiEntity__sgs_ifn, &Entity::_sgs_interface);

