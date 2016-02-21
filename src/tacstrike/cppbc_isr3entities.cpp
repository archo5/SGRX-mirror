// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "isr3entities.hpp"

int ISR3Drone::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ISR3Drone*>( obj->data )->C = C;
	static_cast<ISR3Drone*>( obj->data )->~ISR3Drone();
	return SGS_SUCCESS;
}

int ISR3Drone::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Drone*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ISR3Drone::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Drone*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<ISR3Drone*>( obj->data )->m_id ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<ISR3Drone*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Drone::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Drone*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ISR3Drone*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<ISR3Drone*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ISR3Drone*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<ISR3Drone*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<ISR3Drone*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<ISR3Drone*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<ISR3Drone*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<ISR3Drone*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Drone::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Drone*>( obj->data )->C, C );
	char bfr[ 41 ];
	sprintf( bfr, "ISR3Drone (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<ISR3Drone*>( obj->data )->m_id, depth ).push( C ); }
		sgs_StringConcat( C, 34 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ISR3Drone__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int ISR3Drone__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ISR3Drone__sgs_funcs,
		-1, "ISR3Drone." );
	return 1;
}

static sgs_ObjInterface ISR3Drone__sgs_interface =
{
	"ISR3Drone",
	ISR3Drone::_sgs_destruct, ISR3Drone::_sgs_gcmark, ISR3Drone::_sgs_getindex, ISR3Drone::_sgs_setindex, NULL, NULL, ISR3Drone::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ISR3Drone::_sgs_interface(ISR3Drone__sgs_interface, ISR3Drone__sgs_ifn, &Entity::_sgs_interface);


int ISR3Player::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ISR3Player*>( obj->data )->C = C;
	static_cast<ISR3Player*>( obj->data )->~ISR3Player();
	return SGS_SUCCESS;
}

int ISR3Player::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Player*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ISR3Player::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Player*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<ISR3Player*>( obj->data )->m_id ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<ISR3Player*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Player::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Player*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ISR3Player*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ISR3Player*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<ISR3Player*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<ISR3Player*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<ISR3Player*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<ISR3Player*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<ISR3Player*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<ISR3Player*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<ISR3Player*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<ISR3Player*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<ISR3Player*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ISR3Player*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<ISR3Player*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<ISR3Player*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<ISR3Player*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<ISR3Player*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<ISR3Player*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Player::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Player*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "ISR3Player (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<ISR3Player*>( obj->data )->m_id, depth ).push( C ); }
		sgs_StringConcat( C, 34 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ISR3Player__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int ISR3Player__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ISR3Player__sgs_funcs,
		-1, "ISR3Player." );
	return 1;
}

static sgs_ObjInterface ISR3Player__sgs_interface =
{
	"ISR3Player",
	ISR3Player::_sgs_destruct, ISR3Player::_sgs_gcmark, ISR3Player::_sgs_getindex, ISR3Player::_sgs_setindex, NULL, NULL, ISR3Player::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ISR3Player::_sgs_interface(ISR3Player__sgs_interface, ISR3Player__sgs_ifn, &ISR3Drone::_sgs_interface);


static int _sgs_method__ISR3Enemy__HasFact( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, HasFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsHasFact( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ISR3Enemy__HasRecentFact( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, HasRecentFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsHasRecentFact( sgs_GetVar<int>()(C,0), sgs_GetVar<TimeVal>()(C,1) )); return 1;
}

static int _sgs_method__ISR3Enemy__GetRecentFact( SGS_CTX )
{
	ISR3Enemy* data; if( !SGS_PARSE_METHOD( C, ISR3Enemy::_sgs_interface, data, ISR3Enemy, GetRecentFact ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	return data->sgsGetRecentFact( sgs_GetVar<int>()(C,0), sgs_GetVar<TimeVal>()(C,1) );
}

int ISR3Enemy::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ISR3Enemy*>( obj->data )->C = C;
	static_cast<ISR3Enemy*>( obj->data )->~ISR3Enemy();
	return SGS_SUCCESS;
}

int ISR3Enemy::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Enemy*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ISR3Enemy::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Enemy*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<ISR3Enemy*>( obj->data )->m_id ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<ISR3Enemy*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Enemy::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Enemy*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ISR3Enemy*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<ISR3Enemy*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) );
			static_cast<ISR3Enemy*>( obj->data )->OnTransformUpdate(); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<ISR3Enemy*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<ISR3Enemy*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<ISR3Enemy*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<ISR3Enemy*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<ISR3Enemy*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ISR3Enemy::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ISR3Enemy*>( obj->data )->C, C );
	char bfr[ 41 ];
	sprintf( bfr, "ISR3Enemy (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<ISR3Enemy*>( obj->data )->m_id, depth ).push( C ); }
		sgs_StringConcat( C, 34 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ISR3Enemy__sgs_funcs[] =
{
	{ "HasFact", _sgs_method__ISR3Enemy__HasFact },
	{ "HasRecentFact", _sgs_method__ISR3Enemy__HasRecentFact },
	{ "GetRecentFact", _sgs_method__ISR3Enemy__GetRecentFact },
	{ NULL, NULL },
};

static int ISR3Enemy__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ISR3Enemy__sgs_funcs,
		-1, "ISR3Enemy." );
	return 1;
}

static sgs_ObjInterface ISR3Enemy__sgs_interface =
{
	"ISR3Enemy",
	ISR3Enemy::_sgs_destruct, ISR3Enemy::_sgs_gcmark, ISR3Enemy::_sgs_getindex, ISR3Enemy::_sgs_setindex, NULL, NULL, ISR3Enemy::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ISR3Enemy::_sgs_interface(ISR3Enemy__sgs_interface, ISR3Enemy__sgs_ifn, &ISR3Drone::_sgs_interface);

