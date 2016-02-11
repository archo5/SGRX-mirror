// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "level.hpp"

int LevelScrObj::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<LevelScrObj*>( obj->data )->C = C;
	static_cast<LevelScrObj*>( obj->data )->~LevelScrObj();
	return SGS_SUCCESS;
}

int LevelScrObj::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LevelScrObj*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int LevelScrObj::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LevelScrObj*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<LevelScrObj*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<LevelScrObj*>( obj->data )->_data ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int LevelScrObj::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LevelScrObj*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<LevelScrObj*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int LevelScrObj::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LevelScrObj*>( obj->data )->C, C );
	char bfr[ 43 ];
	sprintf( bfr, "LevelScrObj (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<LevelScrObj*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<LevelScrObj*>( obj->data )->_data, depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface LevelScrObj__sgs_interface =
{
	"LevelScrObj",
	NULL, LevelScrObj::_sgs_gcmark, LevelScrObj::_getindex, LevelScrObj::_setindex, NULL, NULL, LevelScrObj::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface LevelScrObj::_sgs_interface(LevelScrObj__sgs_interface);


static int _sgs_method__IActorController__GetInput( SGS_CTX )
{
	IActorController* data; if( !SGS_PARSE_METHOD( C, IActorController::_sgs_interface, data, IActorController, GetInput ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInput( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__IActorController__Reset( SGS_CTX )
{
	IActorController* data; if( !SGS_PARSE_METHOD( C, IActorController::_sgs_interface, data, IActorController, Reset ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Reset(  ); return 0;
}

int IActorController::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<IActorController*>( obj->data )->C = C;
	static_cast<IActorController*>( obj->data )->~IActorController();
	return SGS_SUCCESS;
}

int IActorController::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IActorController*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int IActorController::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IActorController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<IActorController*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<IActorController*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "GetInput" ){ sgs_PushCFunc( C, _sgs_method__IActorController__GetInput ); return SGS_SUCCESS; }
		SGS_CASE( "Reset" ){ sgs_PushCFunc( C, _sgs_method__IActorController__Reset ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int IActorController::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IActorController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<IActorController*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int IActorController::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IActorController*>( obj->data )->C, C );
	char bfr[ 48 ];
	sprintf( bfr, "IActorController (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<IActorController*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<IActorController*>( obj->data )->_data, depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface IActorController__sgs_interface =
{
	"IActorController",
	NULL, IActorController::_sgs_gcmark, IActorController::_getindex, IActorController::_setindex, NULL, NULL, IActorController::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface IActorController::_sgs_interface(IActorController__sgs_interface, &LevelScrObj::_sgs_interface);


static int _sgs_method__Entity__CallEvent( SGS_CTX )
{
	Entity* data; if( !SGS_PARSE_METHOD( C, Entity::_sgs_interface, data, Entity, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

int Entity::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<Entity*>( obj->data )->C = C;
	static_cast<Entity*>( obj->data )->~Entity();
	return SGS_SUCCESS;
}

int Entity::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Entity*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int Entity::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Entity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__Entity__CallEvent ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Entity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Entity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Entity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<Entity*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Entity::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Entity*>( obj->data )->C, C );
	char bfr[ 38 ];
	sprintf( bfr, "Entity (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->m_viewName, depth ).push( C ); }
		sgs_StringConcat( C, 10 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface Entity__sgs_interface =
{
	"Entity",
	NULL, Entity::_sgs_gcmark, Entity::_getindex, Entity::_setindex, NULL, NULL, Entity::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Entity::_sgs_interface(Entity__sgs_interface, &LevelScrObj::_sgs_interface);


int IGameLevelSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<IGameLevelSystem*>( obj->data )->C = C;
	static_cast<IGameLevelSystem*>( obj->data )->~IGameLevelSystem();
	return SGS_SUCCESS;
}

int IGameLevelSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IGameLevelSystem*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int IGameLevelSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IGameLevelSystem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<IGameLevelSystem*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<IGameLevelSystem*>( obj->data )->_data ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int IGameLevelSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IGameLevelSystem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<IGameLevelSystem*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int IGameLevelSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IGameLevelSystem*>( obj->data )->C, C );
	char bfr[ 48 ];
	sprintf( bfr, "IGameLevelSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<IGameLevelSystem*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<IGameLevelSystem*>( obj->data )->_data, depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface IGameLevelSystem__sgs_interface =
{
	"IGameLevelSystem",
	NULL, IGameLevelSystem::_sgs_gcmark, IGameLevelSystem::_getindex, IGameLevelSystem::_setindex, NULL, NULL, IGameLevelSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface IGameLevelSystem::_sgs_interface(IGameLevelSystem__sgs_interface, &LevelScrObj::_sgs_interface);


static int _sgs_method__Actor__CallEvent( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, CallEvent ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->OnEvent( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__Actor__GetInputV3( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, GetInputV3 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV3( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__Actor__GetInputV2( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, GetInputV2 ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputV2( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__Actor__GetInputF( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, GetInputF ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputF( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__Actor__GetInputB( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, GetInputB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetInputB( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__Actor__IsAlive( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, IsAlive ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->IsAlive(  )); return 1;
}

static int _sgs_method__Actor__Reset( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, Reset ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Reset(  ); return 0;
}

int Actor::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<Actor*>( obj->data )->C = C;
	static_cast<Actor*>( obj->data )->~Actor();
	return SGS_SUCCESS;
}

int Actor::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actor*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int Actor::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actor*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "typeName" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->m_typeName ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->m_viewName ); return SGS_SUCCESS; }
		SGS_CASE( "CallEvent" ){ sgs_PushCFunc( C, _sgs_method__Actor__CallEvent ); return SGS_SUCCESS; }
		SGS_CASE( "GetInputV3" ){ sgs_PushCFunc( C, _sgs_method__Actor__GetInputV3 ); return SGS_SUCCESS; }
		SGS_CASE( "GetInputV2" ){ sgs_PushCFunc( C, _sgs_method__Actor__GetInputV2 ); return SGS_SUCCESS; }
		SGS_CASE( "GetInputF" ){ sgs_PushCFunc( C, _sgs_method__Actor__GetInputF ); return SGS_SUCCESS; }
		SGS_CASE( "GetInputB" ){ sgs_PushCFunc( C, _sgs_method__Actor__GetInputB ); return SGS_SUCCESS; }
		SGS_CASE( "IsAlive" ){ sgs_PushCFunc( C, _sgs_method__Actor__IsAlive ); return SGS_SUCCESS; }
		SGS_CASE( "Reset" ){ sgs_PushCFunc( C, _sgs_method__Actor__Reset ); return SGS_SUCCESS; }
		SGS_CASE( "ctrl" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->_getCtrl() ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetPosition() ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Actor::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actor*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Actor*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "viewName" ){ static_cast<Actor*>( obj->data )->m_viewName = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<Actor*>( obj->data )->SetPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int Actor::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actor*>( obj->data )->C, C );
	char bfr[ 37 ];
	sprintf( bfr, "Actor (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeName = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->m_typeName, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nviewName = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->m_viewName, depth ).push( C ); }
		{ sgs_PushString( C, "\nctrl = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->_getCtrl(), depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetPosition(), depth ).push( C ); }
		sgs_StringConcat( C, 14 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_ObjInterface Actor__sgs_interface =
{
	"Actor",
	NULL, Actor::_sgs_gcmark, Actor::_getindex, Actor::_setindex, NULL, NULL, Actor::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Actor::_sgs_interface(Actor__sgs_interface, &Entity::_sgs_interface);


static int _sgs_method__GameLevel__SetLevel( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, SetLevel ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->SetNextLevel( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__GameLevel__CreateEntity( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, CreateEntity ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsCreateEntity( sgs_GetVar<StringView>()(C,0), sgs_GetVar<sgsVariable>()(C,1) )); return 1;
}

static int _sgs_method__GameLevel__DestroyEntity( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, DestroyEntity ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsDestroyEntity( sgs_GetVar<sgsVariable>()(C,0) ); return 0;
}

static int _sgs_method__GameLevel__FindEntity( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, FindEntity ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsFindEntity( sgs_GetVar<StringView>()(C,0) )); return 1;
}

static int _sgs_method__GameLevel__CallEntity( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, CallEntity ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->CallEntityByName( sgs_GetVar<StringView>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__GameLevel__SetCameraPosDir( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, SetCameraPosDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->sgsSetCameraPosDir( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<Vec3>()(C,1) ); return 0;
}

static int _sgs_method__GameLevel__WorldToScreen( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, WorldToScreen ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	return data->sgsWorldToScreen( sgs_GetVar<Vec3>()(C,0) );
}

static int _sgs_method__GameLevel__WorldToScreenPx( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, WorldToScreenPx ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	return data->sgsWorldToScreenPx( sgs_GetVar<Vec3>()(C,0) );
}

static int _sgs_method__GameLevel__GetCursorWorldPoint( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, GetCursorWorldPoint ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	return data->sgsGetCursorWorldPoint(  );
}

static int _sgs_method__GameLevel__GetTickTime( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, GetTickTime ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetTickTime(  )); return 1;
}

static int _sgs_method__GameLevel__GetPhyTime( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, GetPhyTime ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetPhyTime(  )); return 1;
}

int GameLevel::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GameLevel*>( obj->data )->C = C;
	static_cast<GameLevel*>( obj->data )->~GameLevel();
	return SGS_SUCCESS;
}

int GameLevel::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameLevel*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int GameLevel::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameLevel*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "SetLevel" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__SetLevel ); return SGS_SUCCESS; }
		SGS_CASE( "CreateEntity" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__CreateEntity ); return SGS_SUCCESS; }
		SGS_CASE( "DestroyEntity" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__DestroyEntity ); return SGS_SUCCESS; }
		SGS_CASE( "FindEntity" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__FindEntity ); return SGS_SUCCESS; }
		SGS_CASE( "CallEntity" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__CallEntity ); return SGS_SUCCESS; }
		SGS_CASE( "SetCameraPosDir" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__SetCameraPosDir ); return SGS_SUCCESS; }
		SGS_CASE( "WorldToScreen" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__WorldToScreen ); return SGS_SUCCESS; }
		SGS_CASE( "WorldToScreenPx" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__WorldToScreenPx ); return SGS_SUCCESS; }
		SGS_CASE( "GetCursorWorldPoint" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__GetCursorWorldPoint ); return SGS_SUCCESS; }
		SGS_CASE( "GetTickTime" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__GetTickTime ); return SGS_SUCCESS; }
		SGS_CASE( "GetPhyTime" ){ sgs_PushCFunc( C, _sgs_method__GameLevel__GetPhyTime ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameLevel::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameLevel*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int GameLevel::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameLevel*>( obj->data )->C, C );
	char bfr[ 41 ];
	sprintf( bfr, "GameLevel (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

static sgs_ObjInterface GameLevel__sgs_interface =
{
	"GameLevel",
	NULL, GameLevel::_sgs_gcmark, GameLevel::_getindex, GameLevel::_setindex, NULL, NULL, GameLevel::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface GameLevel::_sgs_interface(GameLevel__sgs_interface);

