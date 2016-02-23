// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "gfw.hpp"

int SGSTextureHandle::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGSTextureHandle*>( obj->data )->C = C;
	static_cast<SGSTextureHandle*>( obj->data )->~SGSTextureHandle();
	return SGS_SUCCESS;
}

int SGSTextureHandle::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSTextureHandle*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGSTextureHandle::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSTextureHandle*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "key" ){ sgs_PushVar( C, static_cast<SGSTextureHandle*>( obj->data )->h->m_key ); return SGS_SUCCESS; }
		SGS_CASE( "typeID" ){ sgs_PushVar( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().type ); return SGS_SUCCESS; }
		SGS_CASE( "mipmapCount" ){ sgs_PushVar( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().mipcount ); return SGS_SUCCESS; }
		SGS_CASE( "width" ){ sgs_PushVar( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().width ); return SGS_SUCCESS; }
		SGS_CASE( "height" ){ sgs_PushVar( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().height ); return SGS_SUCCESS; }
		SGS_CASE( "depth" ){ sgs_PushVar( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().depth ); return SGS_SUCCESS; }
		SGS_CASE( "formatID" ){ sgs_PushVar( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().format ); return SGS_SUCCESS; }
		SGS_CASE( "isRenderTexture" ){ sgs_PushVar( C, static_cast<SGSTextureHandle*>( obj->data )->h->m_isRenderTexture ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGSTextureHandle::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSTextureHandle*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int SGSTextureHandle::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSTextureHandle*>( obj->data )->C, C );
	char bfr[ 48 ];
	sprintf( bfr, "SGSTextureHandle (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nkey = " ); sgs_DumpData( C, static_cast<SGSTextureHandle*>( obj->data )->h->m_key, depth ).push( C ); }
		{ sgs_PushString( C, "\ntypeID = " ); sgs_DumpData( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().type, depth ).push( C ); }
		{ sgs_PushString( C, "\nmipmapCount = " ); sgs_DumpData( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().mipcount, depth ).push( C ); }
		{ sgs_PushString( C, "\nwidth = " ); sgs_DumpData( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().width, depth ).push( C ); }
		{ sgs_PushString( C, "\nheight = " ); sgs_DumpData( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().height, depth ).push( C ); }
		{ sgs_PushString( C, "\ndepth = " ); sgs_DumpData( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().depth, depth ).push( C ); }
		{ sgs_PushString( C, "\nformatID = " ); sgs_DumpData( C, static_cast<SGSTextureHandle*>( obj->data )->h.GetInfo().format, depth ).push( C ); }
		{ sgs_PushString( C, "\nisRenderTexture = " ); sgs_DumpData( C, static_cast<SGSTextureHandle*>( obj->data )->h->m_isRenderTexture, depth ).push( C ); }
		sgs_StringConcat( C, 16 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SGSTextureHandle__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int SGSTextureHandle__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGSTextureHandle__sgs_funcs,
		-1, "SGSTextureHandle." );
	return 1;
}

static sgs_ObjInterface SGSTextureHandle__sgs_interface =
{
	"SGSTextureHandle",
	SGSTextureHandle::_sgs_destruct, SGSTextureHandle::_sgs_gcmark, SGSTextureHandle::_sgs_getindex, SGSTextureHandle::_sgs_setindex, NULL, NULL, SGSTextureHandle::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGSTextureHandle::_sgs_interface(SGSTextureHandle__sgs_interface, SGSTextureHandle__sgs_ifn);


int SGSMeshHandle::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGSMeshHandle*>( obj->data )->C = C;
	static_cast<SGSMeshHandle*>( obj->data )->~SGSMeshHandle();
	return SGS_SUCCESS;
}

int SGSMeshHandle::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSMeshHandle*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGSMeshHandle::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSMeshHandle*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "key" ){ sgs_PushVar( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_key ); return SGS_SUCCESS; }
		SGS_CASE( "dataFlags" ){ sgs_PushVar( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_dataFlags ); return SGS_SUCCESS; }
		SGS_CASE( "numParts" ){ sgs_PushVar( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_meshParts.size() ); return SGS_SUCCESS; }
		SGS_CASE( "numBones" ){ sgs_PushVar( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_numBones ); return SGS_SUCCESS; }
		SGS_CASE( "boundsMin" ){ sgs_PushVar( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_boundsMin ); return SGS_SUCCESS; }
		SGS_CASE( "boundsMax" ){ sgs_PushVar( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_boundsMax ); return SGS_SUCCESS; }
		SGS_CASE( "totalVertexCount" ){ sgs_PushVar( C, static_cast<SGSMeshHandle*>( obj->data )->h->GetBufferVertexCount() ); return SGS_SUCCESS; }
		SGS_CASE( "totalIndexCount" ){ sgs_PushVar( C, static_cast<SGSMeshHandle*>( obj->data )->h->GetBufferIndexCount() ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGSMeshHandle::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSMeshHandle*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int SGSMeshHandle::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSMeshHandle*>( obj->data )->C, C );
	char bfr[ 45 ];
	sprintf( bfr, "SGSMeshHandle (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nkey = " ); sgs_DumpData( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_key, depth ).push( C ); }
		{ sgs_PushString( C, "\ndataFlags = " ); sgs_DumpData( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_dataFlags, depth ).push( C ); }
		{ sgs_PushString( C, "\nnumParts = " ); sgs_DumpData( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_meshParts.size(), depth ).push( C ); }
		{ sgs_PushString( C, "\nnumBones = " ); sgs_DumpData( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_numBones, depth ).push( C ); }
		{ sgs_PushString( C, "\nboundsMin = " ); sgs_DumpData( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_boundsMin, depth ).push( C ); }
		{ sgs_PushString( C, "\nboundsMax = " ); sgs_DumpData( C, static_cast<SGSMeshHandle*>( obj->data )->h->m_boundsMax, depth ).push( C ); }
		{ sgs_PushString( C, "\ntotalVertexCount = " ); sgs_DumpData( C, static_cast<SGSMeshHandle*>( obj->data )->h->GetBufferVertexCount(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntotalIndexCount = " ); sgs_DumpData( C, static_cast<SGSMeshHandle*>( obj->data )->h->GetBufferIndexCount(), depth ).push( C ); }
		sgs_StringConcat( C, 16 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SGSMeshHandle__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int SGSMeshHandle__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGSMeshHandle__sgs_funcs,
		-1, "SGSMeshHandle." );
	return 1;
}

static sgs_ObjInterface SGSMeshHandle__sgs_interface =
{
	"SGSMeshHandle",
	SGSMeshHandle::_sgs_destruct, SGSMeshHandle::_sgs_gcmark, SGSMeshHandle::_sgs_getindex, SGSMeshHandle::_sgs_setindex, NULL, NULL, SGSMeshHandle::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGSMeshHandle::_sgs_interface(SGSMeshHandle__sgs_interface, SGSMeshHandle__sgs_ifn);


int GameUIEvent::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GameUIEvent*>( obj->data )->~GameUIEvent();
	return SGS_SUCCESS;
}

int GameUIEvent::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int GameUIEvent::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "type" ){ sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->type ); return SGS_SUCCESS; }
		SGS_CASE( "target" ){ sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->target ); return SGS_SUCCESS; }
		SGS_CASE( "x" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsMouseEvent() ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->mouse.x ); return SGS_SUCCESS; }
		SGS_CASE( "y" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsMouseEvent() ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->mouse.y ); return SGS_SUCCESS; }
		SGS_CASE( "button" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsButtonEvent() ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->mouse.button ); return SGS_SUCCESS; }
		SGS_CASE( "key" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->key.key ); return SGS_SUCCESS; }
		SGS_CASE( "engkey" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->key.engkey ); return SGS_SUCCESS; }
		SGS_CASE( "engmod" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->key.engmod ); return SGS_SUCCESS; }
		SGS_CASE( "repeat" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->key.repeat ); return SGS_SUCCESS; }
		SGS_CASE( "isActivate" ){ sgs_PushVar( C, static_cast<GameUIEvent*>( obj->data )->IsActivateEvent() ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameUIEvent::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "type" ){ static_cast<GameUIEvent*>( obj->data )->type = sgs_GetVar<int>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "target" ){ static_cast<GameUIEvent*>( obj->data )->target = sgs_GetVar<sgsHandle< GameUIControl > >()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "x" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsMouseEvent() ) ){ return SGS_EINPROC; }  return SGS_SUCCESS; }
		SGS_CASE( "y" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsMouseEvent() ) ){ return SGS_EINPROC; }  return SGS_SUCCESS; }
		SGS_CASE( "button" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsButtonEvent() ) ){ return SGS_EINPROC; }  return SGS_SUCCESS; }
		SGS_CASE( "key" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ){ return SGS_EINPROC; }  return SGS_SUCCESS; }
		SGS_CASE( "engkey" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ){ return SGS_EINPROC; }  return SGS_SUCCESS; }
		SGS_CASE( "engmod" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ){ return SGS_EINPROC; }  return SGS_SUCCESS; }
		SGS_CASE( "repeat" ){ if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ){ return SGS_EINPROC; }  return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameUIEvent::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 43 ];
	sprintf( bfr, "GameUIEvent (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntype = " ); sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->type, depth ).push( C ); }
		{ sgs_PushString( C, "\ntarget = " ); sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->target, depth ).push( C ); }
		{ sgs_PushString( C, "\nx = " ); if( !( static_cast<GameUIEvent*>( obj->data )->IsMouseEvent() ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->mouse.x, depth ).push( C ); }
		{ sgs_PushString( C, "\ny = " ); if( !( static_cast<GameUIEvent*>( obj->data )->IsMouseEvent() ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->mouse.y, depth ).push( C ); }
		{ sgs_PushString( C, "\nbutton = " ); if( !( static_cast<GameUIEvent*>( obj->data )->IsButtonEvent() ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->mouse.button, depth ).push( C ); }
		{ sgs_PushString( C, "\nkey = " ); if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->key.key, depth ).push( C ); }
		{ sgs_PushString( C, "\nengkey = " ); if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->key.engkey, depth ).push( C ); }
		{ sgs_PushString( C, "\nengmod = " ); if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->key.engmod, depth ).push( C ); }
		{ sgs_PushString( C, "\nrepeat = " ); if( !( static_cast<GameUIEvent*>( obj->data )->IsKeyEvent() ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->key.repeat, depth ).push( C ); }
		{ sgs_PushString( C, "\nisActivate = " ); sgs_DumpData( C, static_cast<GameUIEvent*>( obj->data )->IsActivateEvent(), depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst GameUIEvent__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int GameUIEvent__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		GameUIEvent__sgs_funcs,
		-1, "GameUIEvent." );
	return 1;
}

static sgs_ObjInterface GameUIEvent__sgs_interface =
{
	"GameUIEvent",
	GameUIEvent::_sgs_destruct, GameUIEvent::_sgs_gcmark, GameUIEvent::_sgs_getindex, GameUIEvent::_sgs_setindex, NULL, NULL, GameUIEvent::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface GameUIEvent::_sgs_interface(GameUIEvent__sgs_interface, GameUIEvent__sgs_ifn);


static int _sgs_method__GameUIControl__IsIn( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, IsIn ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->_isIn( sgs_GetVarObj<GameUIControl>()(C,0) )); return 1;
}

static int _sgs_method__GameUIControl__AddCallback( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, AddCallback ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->AddCallback( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<sgsVariable>()(C,1) ); return 0;
}

static int _sgs_method__GameUIControl__RemoveCallback( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, RemoveCallback ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RemoveCallback( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<sgsVariable>()(C,1) ); return 0;
}

static int _sgs_method__GameUIControl__InvokeCallbacks( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, InvokeCallbacks ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->InvokeCallbacks( sgs_GetVar<sgsString>()(C,0) ); return 0;
}

static int _sgs_method__GameUIControl__Hit( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, Hit ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->Hit( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1) )); return 1;
}

static int _sgs_method__GameUIControl__IX( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, IX ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->IX( sgs_GetVar<float>()(C,0) )); return 1;
}

static int _sgs_method__GameUIControl__IY( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, IY ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->IY( sgs_GetVar<float>()(C,0) )); return 1;
}

static int _sgs_method__GameUIControl__IS( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, IS ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->IS( sgs_GetVar<float>()(C,0) )); return 1;
}

static int _sgs_method__GameUIControl__InvIX( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, InvIX ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->InvIX( sgs_GetVar<float>()(C,0) )); return 1;
}

static int _sgs_method__GameUIControl__InvIY( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, InvIY ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->InvIY( sgs_GetVar<float>()(C,0) )); return 1;
}

static int _sgs_method__GameUIControl__InvIS( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, InvIS ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->InvIS( sgs_GetVar<float>()(C,0) )); return 1;
}

static int _sgs_method__GameUIControl__CreateScreen( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, CreateScreen ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->CreateScreen( sgs_GetVar<int>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<float>()(C,4), sgs_GetVar<float>()(C,5), sgs_GetVar<float>()(C,6) )); return 1;
}

static int _sgs_method__GameUIControl__CreateControl( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, CreateControl ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->CreateControl( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3) )); return 1;
}

static int _sgs_method__GameUIControl__DReset( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DReset ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DReset(  ); return 0;
}

static int _sgs_method__GameUIControl__DCol( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DCol ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DCol( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3) ); return 0;
}

static int _sgs_method__GameUIControl__DTex( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DTex ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DTex( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__GameUIControl__DQuad( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DQuad ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DQuad( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3) ); return 0;
}

static int _sgs_method__GameUIControl__DQuadExt( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DQuadExt ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DQuadExt( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<float>()(C,4), sgs_GetVar<float>()(C,5), sgs_GetVar<float>()(C,6), sgs_GetVar<float>()(C,7) ); return 0;
}

static int _sgs_method__GameUIControl__DCircleFill( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DCircleFill ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DCircleFill( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<int>()(C,4) ); return 0;
}

static int _sgs_method__GameUIControl__DButton( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DButton ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DButton( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<Vec4>()(C,4), sgs_GetVar<Vec4>()(C,5) ); return 0;
}

static int _sgs_method__GameUIControl__DAALine( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DAALine ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DAALine( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<float>()(C,4) ); return 0;
}

static int _sgs_method__GameUIControl__DAARectOutline( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DAARectOutline ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DAARectOutline( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<float>()(C,4) ); return 0;
}

static int _sgs_method__GameUIControl__DAACircleOutline( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DAACircleOutline ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DAACircleOutline( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3) ); return 0;
}

static int _sgs_method__GameUIControl__DFont( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DFont ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DFont( sgs_GetVar<StringView>()(C,0), sgs_GetVar<float>()(C,1) ); return 0;
}

static int _sgs_method__GameUIControl__DText( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DText ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DText( sgs_GetVar<StringView>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<int>()(C,3), sgs_GetVar<int>()(C,4) ); return 0;
}

static int _sgs_method__GameUIControl__DTextLen( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DTextLen ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->DTextLen( sgs_GetVar<StringView>()(C,0) )); return 1;
}

int GameUIControl::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GameUIControl*>( obj->data )->C = C;
	static_cast<GameUIControl*>( obj->data )->~GameUIControl();
	return SGS_SUCCESS;
}

int GameUIControl::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameUIControl*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int GameUIControl::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameUIControl*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->id ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->enabled ); return SGS_SUCCESS; }
		SGS_CASE( "mode" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->mode ); return SGS_SUCCESS; }
		SGS_CASE( "x" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->x ); return SGS_SUCCESS; }
		SGS_CASE( "y" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->y ); return SGS_SUCCESS; }
		SGS_CASE( "width" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->width ); return SGS_SUCCESS; }
		SGS_CASE( "height" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->height ); return SGS_SUCCESS; }
		SGS_CASE( "xalign" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->xalign ); return SGS_SUCCESS; }
		SGS_CASE( "yalign" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->yalign ); return SGS_SUCCESS; }
		SGS_CASE( "xscale" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->xscale ); return SGS_SUCCESS; }
		SGS_CASE( "yscale" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->yscale ); return SGS_SUCCESS; }
		SGS_CASE( "swidth" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getSWidth() ); return SGS_SUCCESS; }
		SGS_CASE( "sheight" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getSHeight() ); return SGS_SUCCESS; }
		SGS_CASE( "minw" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getMinW() ); return SGS_SUCCESS; }
		SGS_CASE( "maxw" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getMaxW() ); return SGS_SUCCESS; }
		SGS_CASE( "rx0" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->rx0 ); return SGS_SUCCESS; }
		SGS_CASE( "ry0" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->ry0 ); return SGS_SUCCESS; }
		SGS_CASE( "rx1" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->rx1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry1" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->ry1 ); return SGS_SUCCESS; }
		SGS_CASE( "rwidth" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->rwidth ); return SGS_SUCCESS; }
		SGS_CASE( "rheight" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->rheight ); return SGS_SUCCESS; }
		SGS_CASE( "z" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->z ); return SGS_SUCCESS; }
		SGS_CASE( "metadata" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->metadata ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->parent ); return SGS_SUCCESS; }
		SGS_CASE( "eventCallback" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->eventCallback ); return SGS_SUCCESS; }
		SGS_CASE( "shaders" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->shaders ); return SGS_SUCCESS; }
		SGS_CASE( "hover" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->hover ); return SGS_SUCCESS; }
		SGS_CASE( "focusable" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->focusable ); return SGS_SUCCESS; }
		SGS_CASE( "visible" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getVisible() ); return SGS_SUCCESS; }
		SGS_CASE( "focused" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getFocused() ); return SGS_SUCCESS; }
		SGS_CASE( "clicked" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getClicked() ); return SGS_SUCCESS; }
		SGS_CASE( "clickedL" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getClickedL() ); return SGS_SUCCESS; }
		SGS_CASE( "clickedR" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getClickedR() ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameUIControl::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameUIControl*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "id" ){ static_cast<GameUIControl*>( obj->data )->id = sgs_GetVar<uint32_t>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<GameUIControl*>( obj->data )->enabled = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "mode" ){ static_cast<GameUIControl*>( obj->data )->mode = sgs_GetVar<int>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "x" ){ static_cast<GameUIControl*>( obj->data )->x = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "y" ){ static_cast<GameUIControl*>( obj->data )->y = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "width" ){ static_cast<GameUIControl*>( obj->data )->width = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "height" ){ static_cast<GameUIControl*>( obj->data )->height = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "xalign" ){ static_cast<GameUIControl*>( obj->data )->xalign = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "yalign" ){ static_cast<GameUIControl*>( obj->data )->yalign = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "xscale" ){ static_cast<GameUIControl*>( obj->data )->xscale = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "yscale" ){ static_cast<GameUIControl*>( obj->data )->yscale = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rx0" ){ static_cast<GameUIControl*>( obj->data )->rx0 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry0" ){ static_cast<GameUIControl*>( obj->data )->ry0 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rx1" ){ static_cast<GameUIControl*>( obj->data )->rx1 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry1" ){ static_cast<GameUIControl*>( obj->data )->ry1 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "z" ){ static_cast<GameUIControl*>( obj->data )->z = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "metadata" ){ static_cast<GameUIControl*>( obj->data )->metadata = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "eventCallback" ){ static_cast<GameUIControl*>( obj->data )->eventCallback = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "hover" ){ static_cast<GameUIControl*>( obj->data )->hover = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "focusable" ){ static_cast<GameUIControl*>( obj->data )->focusable = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameUIControl::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameUIControl*>( obj->data )->C, C );
	char bfr[ 45 ];
	sprintf( bfr, "GameUIControl (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->id, depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\nmode = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->mode, depth ).push( C ); }
		{ sgs_PushString( C, "\nx = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->x, depth ).push( C ); }
		{ sgs_PushString( C, "\ny = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->y, depth ).push( C ); }
		{ sgs_PushString( C, "\nwidth = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->width, depth ).push( C ); }
		{ sgs_PushString( C, "\nheight = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->height, depth ).push( C ); }
		{ sgs_PushString( C, "\nxalign = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->xalign, depth ).push( C ); }
		{ sgs_PushString( C, "\nyalign = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->yalign, depth ).push( C ); }
		{ sgs_PushString( C, "\nxscale = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->xscale, depth ).push( C ); }
		{ sgs_PushString( C, "\nyscale = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->yscale, depth ).push( C ); }
		{ sgs_PushString( C, "\nswidth = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->_getSWidth(), depth ).push( C ); }
		{ sgs_PushString( C, "\nsheight = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->_getSHeight(), depth ).push( C ); }
		{ sgs_PushString( C, "\nminw = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->_getMinW(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmaxw = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->_getMaxW(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrx0 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->rx0, depth ).push( C ); }
		{ sgs_PushString( C, "\nry0 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->ry0, depth ).push( C ); }
		{ sgs_PushString( C, "\nrx1 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->rx1, depth ).push( C ); }
		{ sgs_PushString( C, "\nry1 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->ry1, depth ).push( C ); }
		{ sgs_PushString( C, "\nrwidth = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->rwidth, depth ).push( C ); }
		{ sgs_PushString( C, "\nrheight = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->rheight, depth ).push( C ); }
		{ sgs_PushString( C, "\nz = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->z, depth ).push( C ); }
		{ sgs_PushString( C, "\nmetadata = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->metadata, depth ).push( C ); }
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->parent, depth ).push( C ); }
		{ sgs_PushString( C, "\neventCallback = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->eventCallback, depth ).push( C ); }
		{ sgs_PushString( C, "\nshaders = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->shaders, depth ).push( C ); }
		{ sgs_PushString( C, "\nhover = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->hover, depth ).push( C ); }
		{ sgs_PushString( C, "\nfocusable = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->focusable, depth ).push( C ); }
		{ sgs_PushString( C, "\nvisible = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->_getVisible(), depth ).push( C ); }
		{ sgs_PushString( C, "\nfocused = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->_getFocused(), depth ).push( C ); }
		{ sgs_PushString( C, "\nclicked = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->_getClicked(), depth ).push( C ); }
		{ sgs_PushString( C, "\nclickedL = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->_getClickedL(), depth ).push( C ); }
		{ sgs_PushString( C, "\nclickedR = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->_getClickedR(), depth ).push( C ); }
		sgs_StringConcat( C, 66 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst GameUIControl__sgs_funcs[] =
{
	{ "IsIn", _sgs_method__GameUIControl__IsIn },
	{ "AddCallback", _sgs_method__GameUIControl__AddCallback },
	{ "RemoveCallback", _sgs_method__GameUIControl__RemoveCallback },
	{ "InvokeCallbacks", _sgs_method__GameUIControl__InvokeCallbacks },
	{ "Hit", _sgs_method__GameUIControl__Hit },
	{ "IX", _sgs_method__GameUIControl__IX },
	{ "IY", _sgs_method__GameUIControl__IY },
	{ "IS", _sgs_method__GameUIControl__IS },
	{ "InvIX", _sgs_method__GameUIControl__InvIX },
	{ "InvIY", _sgs_method__GameUIControl__InvIY },
	{ "InvIS", _sgs_method__GameUIControl__InvIS },
	{ "CreateScreen", _sgs_method__GameUIControl__CreateScreen },
	{ "CreateControl", _sgs_method__GameUIControl__CreateControl },
	{ "DReset", _sgs_method__GameUIControl__DReset },
	{ "DCol", _sgs_method__GameUIControl__DCol },
	{ "DTex", _sgs_method__GameUIControl__DTex },
	{ "DQuad", _sgs_method__GameUIControl__DQuad },
	{ "DQuadExt", _sgs_method__GameUIControl__DQuadExt },
	{ "DCircleFill", _sgs_method__GameUIControl__DCircleFill },
	{ "DButton", _sgs_method__GameUIControl__DButton },
	{ "DAALine", _sgs_method__GameUIControl__DAALine },
	{ "DAARectOutline", _sgs_method__GameUIControl__DAARectOutline },
	{ "DAACircleOutline", _sgs_method__GameUIControl__DAACircleOutline },
	{ "DFont", _sgs_method__GameUIControl__DFont },
	{ "DText", _sgs_method__GameUIControl__DText },
	{ "DTextLen", _sgs_method__GameUIControl__DTextLen },
	{ NULL, NULL },
};

static int GameUIControl__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		GameUIControl__sgs_funcs,
		-1, "GameUIControl." );
	return 1;
}

static sgs_ObjInterface GameUIControl__sgs_interface =
{
	"GameUIControl",
	GameUIControl::_sgs_destruct, GameUIControl::_sgs_gcmark, GameUIControl::_getindex, GameUIControl::_setindex, NULL, NULL, GameUIControl::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface GameUIControl::_sgs_interface(GameUIControl__sgs_interface, GameUIControl__sgs_ifn);


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
		if( sgs_PushIndex( C, static_cast<LevelScrObj*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int LevelScrObj::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LevelScrObj*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<LevelScrObj*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<LevelScrObj*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
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

static sgs_RegFuncConst LevelScrObj__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int LevelScrObj__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		LevelScrObj__sgs_funcs,
		-1, "LevelScrObj." );
	return 1;
}

static sgs_ObjInterface LevelScrObj__sgs_interface =
{
	"LevelScrObj",
	NULL, LevelScrObj::_sgs_gcmark, LevelScrObj::_sgs_getindex, LevelScrObj::_sgs_setindex, NULL, NULL, LevelScrObj::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface LevelScrObj::_sgs_interface(LevelScrObj__sgs_interface, LevelScrObj__sgs_ifn);


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
		if( sgs_PushIndex( C, static_cast<IActorController*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int IActorController::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IActorController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<IActorController*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<IActorController*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
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

static sgs_RegFuncConst IActorController__sgs_funcs[] =
{
	{ "GetInput", _sgs_method__IActorController__GetInput },
	{ "Reset", _sgs_method__IActorController__Reset },
	{ NULL, NULL },
};

static int IActorController__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		IActorController__sgs_funcs,
		-1, "IActorController." );
	return 1;
}

static sgs_ObjInterface IActorController__sgs_interface =
{
	"IActorController",
	NULL, IActorController::_sgs_gcmark, IActorController::_sgs_getindex, IActorController::_sgs_setindex, NULL, NULL, IActorController::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface IActorController::_sgs_interface(IActorController__sgs_interface, IActorController__sgs_ifn, &LevelScrObj::_sgs_interface);


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
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->m_id ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<Entity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Entity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Entity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Entity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<Entity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<Entity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<Entity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<Entity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<Entity*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<Entity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<Entity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<Entity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<Entity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<Entity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<Entity*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<Entity*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<Entity*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<Entity*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<Entity*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<Entity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
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
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->m_id, depth ).push( C ); }
		sgs_StringConcat( C, 36 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst Entity__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int Entity__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		Entity__sgs_funcs,
		-1, "Entity." );
	return 1;
}

static sgs_ObjInterface Entity__sgs_interface =
{
	"Entity",
	NULL, Entity::_sgs_gcmark, Entity::_sgs_getindex, Entity::_sgs_setindex, NULL, NULL, Entity::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Entity::_sgs_interface(Entity__sgs_interface, Entity__sgs_ifn, &LevelScrObj::_sgs_interface);


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
		if( sgs_PushIndex( C, static_cast<IGameLevelSystem*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int IGameLevelSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IGameLevelSystem*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<IGameLevelSystem*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<IGameLevelSystem*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
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

static sgs_RegFuncConst IGameLevelSystem__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int IGameLevelSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		IGameLevelSystem__sgs_funcs,
		-1, "IGameLevelSystem." );
	return 1;
}

static sgs_ObjInterface IGameLevelSystem__sgs_interface =
{
	"IGameLevelSystem",
	NULL, IGameLevelSystem::_sgs_gcmark, IGameLevelSystem::_sgs_getindex, IGameLevelSystem::_sgs_setindex, NULL, NULL, IGameLevelSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface IGameLevelSystem::_sgs_interface(IGameLevelSystem__sgs_interface, IGameLevelSystem__sgs_ifn, &LevelScrObj::_sgs_interface);


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
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->m_id ); return SGS_SUCCESS; }
		SGS_CASE( "ctrl" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->_getCtrl() ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<Actor*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Actor::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actor*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Actor*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<Actor*>( obj->data )->SetPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<Actor*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<Actor*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<Actor*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<Actor*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<Actor*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<Actor*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<Actor*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<Actor*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<Actor*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<Actor*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<Actor*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<Actor*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<Actor*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<Actor*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<Actor*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
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
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->m_id, depth ).push( C ); }
		{ sgs_PushString( C, "\nctrl = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->_getCtrl(), depth ).push( C ); }
		sgs_StringConcat( C, 38 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst Actor__sgs_funcs[] =
{
	{ "GetInputV3", _sgs_method__Actor__GetInputV3 },
	{ "GetInputV2", _sgs_method__Actor__GetInputV2 },
	{ "GetInputF", _sgs_method__Actor__GetInputF },
	{ "GetInputB", _sgs_method__Actor__GetInputB },
	{ "IsAlive", _sgs_method__Actor__IsAlive },
	{ "Reset", _sgs_method__Actor__Reset },
	{ NULL, NULL },
};

static int Actor__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		Actor__sgs_funcs,
		-1, "Actor." );
	return 1;
}

static sgs_ObjInterface Actor__sgs_interface =
{
	"Actor",
	NULL, Actor::_sgs_gcmark, Actor::_sgs_getindex, Actor::_sgs_setindex, NULL, NULL, Actor::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface Actor::_sgs_interface(Actor__sgs_interface, Actor__sgs_ifn, &Entity::_sgs_interface);


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
	sgs_PushVar(C,data->sgsCreateEntity( sgs_GetVar<StringView>()(C,0) )); return 1;
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
	return data->sgsGetCursorWorldPoint( sgs_GetVar<uint32_t>()(C,0) );
}

static int _sgs_method__GameLevel__Query( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, Query ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsQuery( sgs_GetVar<sgsVariable>()(C,0), sgs_GetVar<uint32_t>()(C,1) )); return 1;
}

static int _sgs_method__GameLevel__QuerySphere( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, QuerySphere ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsQuerySphere( sgs_GetVar<sgsVariable>()(C,0), sgs_GetVar<uint32_t>()(C,1), sgs_GetVar<Vec3>()(C,2), sgs_GetVar<float>()(C,3) )); return 1;
}

static int _sgs_method__GameLevel__QueryOBB( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, QueryOBB ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsQueryOBB( sgs_GetVar<sgsVariable>()(C,0), sgs_GetVar<uint32_t>()(C,1), sgs_GetVar<Mat4>()(C,2), sgs_GetVar<Vec3>()(C,3), sgs_GetVar<Vec3>()(C,4) )); return 1;
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
		if( sgs_PushIndex( C, static_cast<GameLevel*>( obj->data )->m_metadata.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GameLevel::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameLevel*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		if( sgs_SetIndex( C, static_cast<GameLevel*>( obj->data )->m_metadata.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
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

static sgs_RegFuncConst GameLevel__sgs_funcs[] =
{
	{ "SetLevel", _sgs_method__GameLevel__SetLevel },
	{ "CreateEntity", _sgs_method__GameLevel__CreateEntity },
	{ "DestroyEntity", _sgs_method__GameLevel__DestroyEntity },
	{ "FindEntity", _sgs_method__GameLevel__FindEntity },
	{ "SetCameraPosDir", _sgs_method__GameLevel__SetCameraPosDir },
	{ "WorldToScreen", _sgs_method__GameLevel__WorldToScreen },
	{ "WorldToScreenPx", _sgs_method__GameLevel__WorldToScreenPx },
	{ "GetCursorWorldPoint", _sgs_method__GameLevel__GetCursorWorldPoint },
	{ "Query", _sgs_method__GameLevel__Query },
	{ "QuerySphere", _sgs_method__GameLevel__QuerySphere },
	{ "QueryOBB", _sgs_method__GameLevel__QueryOBB },
	{ "GetTickTime", _sgs_method__GameLevel__GetTickTime },
	{ "GetPhyTime", _sgs_method__GameLevel__GetPhyTime },
	{ NULL, NULL },
};

static int GameLevel__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		GameLevel__sgs_funcs,
		-1, "GameLevel." );
	return 1;
}

static sgs_ObjInterface GameLevel__sgs_interface =
{
	"GameLevel",
	NULL, GameLevel::_sgs_gcmark, GameLevel::_sgs_getindex, GameLevel::_sgs_setindex, NULL, NULL, GameLevel::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface GameLevel::_sgs_interface(GameLevel__sgs_interface, GameLevel__sgs_ifn);


static int _sgs_method__MessagingSystem__Add( SGS_CTX )
{
	MessagingSystem* data; if( !SGS_PARSE_METHOD( C, MessagingSystem::_sgs_interface, data, MessagingSystem, Add ) ) return 0;
	data->sgsAddMsg( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1), sgs_GetVar<float>()(C,2) ); return 0;
}

int MessagingSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<MessagingSystem*>( obj->data )->~MessagingSystem();
	return SGS_SUCCESS;
}

int MessagingSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int MessagingSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int MessagingSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int MessagingSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 47 ];
	sprintf( bfr, "MessagingSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

static sgs_RegFuncConst MessagingSystem__sgs_funcs[] =
{
	{ "Add", _sgs_method__MessagingSystem__Add },
	{ NULL, NULL },
};

static int MessagingSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		MessagingSystem__sgs_funcs,
		-1, "MessagingSystem." );
	return 1;
}

static sgs_ObjInterface MessagingSystem__sgs_interface =
{
	"MessagingSystem",
	NULL, MessagingSystem::_sgs_gcmark, MessagingSystem::_sgs_getindex, MessagingSystem::_sgs_setindex, NULL, NULL, MessagingSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface MessagingSystem::_sgs_interface(MessagingSystem__sgs_interface, MessagingSystem__sgs_ifn);


static int _sgs_method__ObjectiveSystem__Add( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, Add ) ) return 0;
	sgs_PushVar(C,data->sgsAddObj( sgs_GetVar<StringView>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<StringView>()(C,2), sgs_GetVar<bool>()(C,3), sgs_GetVar<Vec3>()(C,4) )); return 1;
}

static int _sgs_method__ObjectiveSystem__GetTitle( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, GetTitle ) ) return 0;
	sgs_PushVar(C,data->sgsGetTitle( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ObjectiveSystem__SetTitle( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, SetTitle ) ) return 0;
	data->sgsSetTitle( sgs_GetVar<int>()(C,0), sgs_GetVar<StringView>()(C,1) ); return 0;
}

static int _sgs_method__ObjectiveSystem__GetState( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, GetState ) ) return 0;
	sgs_PushVar(C,data->sgsGetState( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ObjectiveSystem__SetState( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, SetState ) ) return 0;
	data->sgsSetState( sgs_GetVar<int>()(C,0), sgs_GetVar<int>()(C,1) ); return 0;
}

static int _sgs_method__ObjectiveSystem__SetLocation( SGS_CTX )
{
	ObjectiveSystem* data; if( !SGS_PARSE_METHOD( C, ObjectiveSystem::_sgs_interface, data, ObjectiveSystem, SetLocation ) ) return 0;
	data->sgsSetLocation( sgs_GetVar<int>()(C,0), sgs_GetVar<Vec3>()(C,1) ); return 0;
}

int ObjectiveSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ObjectiveSystem*>( obj->data )->~ObjectiveSystem();
	return SGS_SUCCESS;
}

int ObjectiveSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int ObjectiveSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int ObjectiveSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int ObjectiveSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 47 ];
	sprintf( bfr, "ObjectiveSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

static sgs_RegFuncConst ObjectiveSystem__sgs_funcs[] =
{
	{ "Add", _sgs_method__ObjectiveSystem__Add },
	{ "GetTitle", _sgs_method__ObjectiveSystem__GetTitle },
	{ "SetTitle", _sgs_method__ObjectiveSystem__SetTitle },
	{ "GetState", _sgs_method__ObjectiveSystem__GetState },
	{ "SetState", _sgs_method__ObjectiveSystem__SetState },
	{ "SetLocation", _sgs_method__ObjectiveSystem__SetLocation },
	{ NULL, NULL },
};

static int ObjectiveSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ObjectiveSystem__sgs_funcs,
		-1, "ObjectiveSystem." );
	return 1;
}

static sgs_ObjInterface ObjectiveSystem__sgs_interface =
{
	"ObjectiveSystem",
	NULL, ObjectiveSystem::_sgs_gcmark, ObjectiveSystem::_sgs_getindex, ObjectiveSystem::_sgs_setindex, NULL, NULL, ObjectiveSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ObjectiveSystem::_sgs_interface(ObjectiveSystem__sgs_interface, ObjectiveSystem__sgs_ifn);


static int _sgs_method__HelpTextSystem__Clear( SGS_CTX )
{
	HelpTextSystem* data; if( !SGS_PARSE_METHOD( C, HelpTextSystem::_sgs_interface, data, HelpTextSystem, Clear ) ) return 0;
	data->sgsClear(  ); return 0;
}

static int _sgs_method__HelpTextSystem__SetText( SGS_CTX )
{
	HelpTextSystem* data; if( !SGS_PARSE_METHOD( C, HelpTextSystem::_sgs_interface, data, HelpTextSystem, SetText ) ) return 0;
	data->sgsSetText( sgs_GetVar<StringView>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3) ); return 0;
}

int HelpTextSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<HelpTextSystem*>( obj->data )->~HelpTextSystem();
	return SGS_SUCCESS;
}

int HelpTextSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int HelpTextSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "text" ){ sgs_PushVar( C, static_cast<HelpTextSystem*>( obj->data )->m_text ); return SGS_SUCCESS; }
		SGS_CASE( "alpha" ){ sgs_PushVar( C, static_cast<HelpTextSystem*>( obj->data )->m_alpha ); return SGS_SUCCESS; }
		SGS_CASE( "fadeTime" ){ sgs_PushVar( C, static_cast<HelpTextSystem*>( obj->data )->m_fadeTime ); return SGS_SUCCESS; }
		SGS_CASE( "fadeTo" ){ sgs_PushVar( C, static_cast<HelpTextSystem*>( obj->data )->m_fadeTo ); return SGS_SUCCESS; }
		SGS_CASE( "fontSize" ){ if( !( static_cast<HelpTextSystem*>( obj->data )->renderer ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<HelpTextSystem*>( obj->data )->renderer->fontSize ); return SGS_SUCCESS; }
		SGS_CASE( "centerPos" ){ if( !( static_cast<HelpTextSystem*>( obj->data )->renderer ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<HelpTextSystem*>( obj->data )->renderer->centerPos ); return SGS_SUCCESS; }
		SGS_CASE( "lineHeightFactor" ){ if( !( static_cast<HelpTextSystem*>( obj->data )->renderer ) ){ return SGS_EINPROC; } sgs_PushVar( C, static_cast<HelpTextSystem*>( obj->data )->renderer->lineHeightFactor ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int HelpTextSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "text" ){ static_cast<HelpTextSystem*>( obj->data )->m_text = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "alpha" ){ static_cast<HelpTextSystem*>( obj->data )->m_alpha = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "fadeTime" ){ static_cast<HelpTextSystem*>( obj->data )->m_fadeTime = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "fadeTo" ){ static_cast<HelpTextSystem*>( obj->data )->m_fadeTo = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "fontSize" ){ if( !( static_cast<HelpTextSystem*>( obj->data )->renderer ) ){ return SGS_EINPROC; } static_cast<HelpTextSystem*>( obj->data )->renderer->fontSize = sgs_GetVar<int>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "centerPos" ){ if( !( static_cast<HelpTextSystem*>( obj->data )->renderer ) ){ return SGS_EINPROC; } static_cast<HelpTextSystem*>( obj->data )->renderer->centerPos = sgs_GetVar<Vec2>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "lineHeightFactor" ){ if( !( static_cast<HelpTextSystem*>( obj->data )->renderer ) ){ return SGS_EINPROC; } static_cast<HelpTextSystem*>( obj->data )->renderer->lineHeightFactor = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int HelpTextSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 46 ];
	sprintf( bfr, "HelpTextSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\ntext = " ); sgs_DumpData( C, static_cast<HelpTextSystem*>( obj->data )->m_text, depth ).push( C ); }
		{ sgs_PushString( C, "\nalpha = " ); sgs_DumpData( C, static_cast<HelpTextSystem*>( obj->data )->m_alpha, depth ).push( C ); }
		{ sgs_PushString( C, "\nfadeTime = " ); sgs_DumpData( C, static_cast<HelpTextSystem*>( obj->data )->m_fadeTime, depth ).push( C ); }
		{ sgs_PushString( C, "\nfadeTo = " ); sgs_DumpData( C, static_cast<HelpTextSystem*>( obj->data )->m_fadeTo, depth ).push( C ); }
		{ sgs_PushString( C, "\nfontSize = " ); if( !( static_cast<HelpTextSystem*>( obj->data )->renderer ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<HelpTextSystem*>( obj->data )->renderer->fontSize, depth ).push( C ); }
		{ sgs_PushString( C, "\ncenterPos = " ); if( !( static_cast<HelpTextSystem*>( obj->data )->renderer ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<HelpTextSystem*>( obj->data )->renderer->centerPos, depth ).push( C ); }
		{ sgs_PushString( C, "\nlineHeightFactor = " ); if( !( static_cast<HelpTextSystem*>( obj->data )->renderer ) ) sgs_PushString( C, "<inaccessible>" ); else sgs_DumpData( C, static_cast<HelpTextSystem*>( obj->data )->renderer->lineHeightFactor, depth ).push( C ); }
		sgs_StringConcat( C, 14 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst HelpTextSystem__sgs_funcs[] =
{
	{ "Clear", _sgs_method__HelpTextSystem__Clear },
	{ "SetText", _sgs_method__HelpTextSystem__SetText },
	{ NULL, NULL },
};

static int HelpTextSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		HelpTextSystem__sgs_funcs,
		-1, "HelpTextSystem." );
	return 1;
}

static sgs_ObjInterface HelpTextSystem__sgs_interface =
{
	"HelpTextSystem",
	NULL, HelpTextSystem::_sgs_gcmark, HelpTextSystem::_sgs_getindex, HelpTextSystem::_sgs_setindex, NULL, NULL, HelpTextSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface HelpTextSystem::_sgs_interface(HelpTextSystem__sgs_interface, HelpTextSystem__sgs_ifn);


static int _sgs_method__FlareSystem__Update( SGS_CTX )
{
	FlareSystem* data; if( !SGS_PARSE_METHOD( C, FlareSystem::_sgs_interface, data, FlareSystem, Update ) ) return 0;
	data->sgsUpdate( sgs_GetVarObj<void>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<Vec3>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<bool>()(C,4) ); return 0;
}

static int _sgs_method__FlareSystem__Remove( SGS_CTX )
{
	FlareSystem* data; if( !SGS_PARSE_METHOD( C, FlareSystem::_sgs_interface, data, FlareSystem, Remove ) ) return 0;
	data->sgsRemove( sgs_GetVarObj<void>()(C,0) ); return 0;
}

int FlareSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<FlareSystem*>( obj->data )->~FlareSystem();
	return SGS_SUCCESS;
}

int FlareSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int FlareSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "layers" ){ sgs_PushVar( C, static_cast<FlareSystem*>( obj->data )->m_layers ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int FlareSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "layers" ){ static_cast<FlareSystem*>( obj->data )->m_layers = sgs_GetVar<uint32_t>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int FlareSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 43 ];
	sprintf( bfr, "FlareSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlayers = " ); sgs_DumpData( C, static_cast<FlareSystem*>( obj->data )->m_layers, depth ).push( C ); }
		sgs_StringConcat( C, 2 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst FlareSystem__sgs_funcs[] =
{
	{ "Update", _sgs_method__FlareSystem__Update },
	{ "Remove", _sgs_method__FlareSystem__Remove },
	{ NULL, NULL },
};

static int FlareSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		FlareSystem__sgs_funcs,
		-1, "FlareSystem." );
	return 1;
}

static sgs_ObjInterface FlareSystem__sgs_interface =
{
	"FlareSystem",
	NULL, FlareSystem::_sgs_gcmark, FlareSystem::_sgs_getindex, FlareSystem::_sgs_setindex, NULL, NULL, FlareSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface FlareSystem::_sgs_interface(FlareSystem__sgs_interface, FlareSystem__sgs_ifn);


static int _sgs_method__ScriptedSequenceSystem__Start( SGS_CTX )
{
	ScriptedSequenceSystem* data; if( !SGS_PARSE_METHOD( C, ScriptedSequenceSystem::_sgs_interface, data, ScriptedSequenceSystem, Start ) ) return 0;
	data->sgsStart( sgs_GetVar<sgsVariable>()(C,0), sgs_GetVar<float>()(C,1) ); return 0;
}

int ScriptedSequenceSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ScriptedSequenceSystem*>( obj->data )->~ScriptedSequenceSystem();
	return SGS_SUCCESS;
}

int ScriptedSequenceSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int ScriptedSequenceSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "func" ){ sgs_PushVar( C, static_cast<ScriptedSequenceSystem*>( obj->data )->m_func ); return SGS_SUCCESS; }
		SGS_CASE( "time" ){ sgs_PushVar( C, static_cast<ScriptedSequenceSystem*>( obj->data )->m_time ); return SGS_SUCCESS; }
		SGS_CASE( "subtitle" ){ sgs_PushVar( C, static_cast<ScriptedSequenceSystem*>( obj->data )->m_subtitle ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ScriptedSequenceSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "func" ){ static_cast<ScriptedSequenceSystem*>( obj->data )->m_func = sgs_GetVar<sgsVariable>()( C, 1 );
			static_cast<ScriptedSequenceSystem*>( obj->data )->_StartCutscene(); return SGS_SUCCESS; }
		SGS_CASE( "time" ){ static_cast<ScriptedSequenceSystem*>( obj->data )->m_time = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "subtitle" ){ static_cast<ScriptedSequenceSystem*>( obj->data )->m_subtitle = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int ScriptedSequenceSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 54 ];
	sprintf( bfr, "ScriptedSequenceSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<ScriptedSequenceSystem*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\ntime = " ); sgs_DumpData( C, static_cast<ScriptedSequenceSystem*>( obj->data )->m_time, depth ).push( C ); }
		{ sgs_PushString( C, "\nsubtitle = " ); sgs_DumpData( C, static_cast<ScriptedSequenceSystem*>( obj->data )->m_subtitle, depth ).push( C ); }
		sgs_StringConcat( C, 6 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ScriptedSequenceSystem__sgs_funcs[] =
{
	{ "Start", _sgs_method__ScriptedSequenceSystem__Start },
	{ NULL, NULL },
};

static int ScriptedSequenceSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ScriptedSequenceSystem__sgs_funcs,
		-1, "ScriptedSequenceSystem." );
	return 1;
}

static sgs_ObjInterface ScriptedSequenceSystem__sgs_interface =
{
	"ScriptedSequenceSystem",
	NULL, ScriptedSequenceSystem::_sgs_gcmark, ScriptedSequenceSystem::_sgs_getindex, ScriptedSequenceSystem::_sgs_setindex, NULL, NULL, ScriptedSequenceSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ScriptedSequenceSystem::_sgs_interface(ScriptedSequenceSystem__sgs_interface, ScriptedSequenceSystem__sgs_ifn);


static int _sgs_method__MusicSystem__SetTrack( SGS_CTX )
{
	MusicSystem* data; if( !SGS_PARSE_METHOD( C, MusicSystem::_sgs_interface, data, MusicSystem, SetTrack ) ) return 0;
	data->sgsSetTrack( sgs_GetVar<StringView>()(C,0) ); return 0;
}

static int _sgs_method__MusicSystem__SetVar( SGS_CTX )
{
	MusicSystem* data; if( !SGS_PARSE_METHOD( C, MusicSystem::_sgs_interface, data, MusicSystem, SetVar ) ) return 0;
	data->sgsSetVar( sgs_GetVar<StringView>()(C,0), sgs_GetVar<float>()(C,1) ); return 0;
}

int MusicSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<MusicSystem*>( obj->data )->~MusicSystem();
	return SGS_SUCCESS;
}

int MusicSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int MusicSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int MusicSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int MusicSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 43 ];
	sprintf( bfr, "MusicSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

static sgs_RegFuncConst MusicSystem__sgs_funcs[] =
{
	{ "SetTrack", _sgs_method__MusicSystem__SetTrack },
	{ "SetVar", _sgs_method__MusicSystem__SetVar },
	{ NULL, NULL },
};

static int MusicSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		MusicSystem__sgs_funcs,
		-1, "MusicSystem." );
	return 1;
}

static sgs_ObjInterface MusicSystem__sgs_interface =
{
	"MusicSystem",
	NULL, MusicSystem::_sgs_gcmark, MusicSystem::_sgs_getindex, MusicSystem::_sgs_setindex, NULL, NULL, MusicSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface MusicSystem::_sgs_interface(MusicSystem__sgs_interface, MusicSystem__sgs_ifn);


int AIFact::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<AIFact*>( obj->data )->~AIFact();
	return SGS_SUCCESS;
}

int AIFact::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int AIFact::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<AIFact*>( obj->data )->id ); return SGS_SUCCESS; }
		SGS_CASE( "ref" ){ sgs_PushVar( C, static_cast<AIFact*>( obj->data )->ref ); return SGS_SUCCESS; }
		SGS_CASE( "type" ){ sgs_PushVar( C, static_cast<AIFact*>( obj->data )->type ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<AIFact*>( obj->data )->position ); return SGS_SUCCESS; }
		SGS_CASE( "created" ){ sgs_PushVar( C, static_cast<AIFact*>( obj->data )->created ); return SGS_SUCCESS; }
		SGS_CASE( "expires" ){ sgs_PushVar( C, static_cast<AIFact*>( obj->data )->expires ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int AIFact::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "id" ){ static_cast<AIFact*>( obj->data )->id = sgs_GetVar<uint32_t>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "ref" ){ static_cast<AIFact*>( obj->data )->ref = sgs_GetVar<uint32_t>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "type" ){ static_cast<AIFact*>( obj->data )->type = sgs_GetVar<uint32_t>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<AIFact*>( obj->data )->position = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "created" ){ static_cast<AIFact*>( obj->data )->created = sgs_GetVar<TimeVal>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "expires" ){ static_cast<AIFact*>( obj->data )->expires = sgs_GetVar<TimeVal>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int AIFact::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 38 ];
	sprintf( bfr, "AIFact (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<AIFact*>( obj->data )->id, depth ).push( C ); }
		{ sgs_PushString( C, "\nref = " ); sgs_DumpData( C, static_cast<AIFact*>( obj->data )->ref, depth ).push( C ); }
		{ sgs_PushString( C, "\ntype = " ); sgs_DumpData( C, static_cast<AIFact*>( obj->data )->type, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<AIFact*>( obj->data )->position, depth ).push( C ); }
		{ sgs_PushString( C, "\ncreated = " ); sgs_DumpData( C, static_cast<AIFact*>( obj->data )->created, depth ).push( C ); }
		{ sgs_PushString( C, "\nexpires = " ); sgs_DumpData( C, static_cast<AIFact*>( obj->data )->expires, depth ).push( C ); }
		sgs_StringConcat( C, 12 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst AIFact__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int AIFact__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		AIFact__sgs_funcs,
		-1, "AIFact." );
	return 1;
}

static sgs_ObjInterface AIFact__sgs_interface =
{
	"AIFact",
	AIFact::_sgs_destruct, AIFact::_sgs_gcmark, AIFact::_sgs_getindex, AIFact::_sgs_setindex, NULL, NULL, AIFact::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface AIFact::_sgs_interface(AIFact__sgs_interface, AIFact__sgs_ifn);


static int _sgs_method__AIDBSystem__AddSound( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, AddSound ) ) return 0;
	data->sgsAddSound( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<int>()(C,3) ); return 0;
}

static int _sgs_method__AIDBSystem__HasFact( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, HasFact ) ) return 0;
	sgs_PushVar(C,data->sgsHasFact( sgs_GetVar<uint32_t>()(C,0) )); return 1;
}

static int _sgs_method__AIDBSystem__HasRecentFact( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, HasRecentFact ) ) return 0;
	sgs_PushVar(C,data->sgsHasRecentFact( sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<TimeVal>()(C,1) )); return 1;
}

static int _sgs_method__AIDBSystem__GetRecentFact( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, GetRecentFact ) ) return 0;
	return data->sgsGetRecentFact( C, sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<TimeVal>()(C,1) );
}

static int _sgs_method__AIDBSystem__InsertFact( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, InsertFact ) ) return 0;
	data->sgsInsertFact( sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<TimeVal>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<uint32_t>()(C,4) ); return 0;
}

static int _sgs_method__AIDBSystem__UpdateFact( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, UpdateFact ) ) return 0;
	sgs_PushVar(C,data->sgsUpdateFact( C, sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<TimeVal>()(C,4), sgs_GetVar<uint32_t>()(C,5), sgs_GetVar<bool>()(C,6) )); return 1;
}

static int _sgs_method__AIDBSystem__InsertOrUpdateFact( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, InsertOrUpdateFact ) ) return 0;
	data->sgsInsertOrUpdateFact( C, sgs_GetVar<uint32_t>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<TimeVal>()(C,3), sgs_GetVar<TimeVal>()(C,4), sgs_GetVar<uint32_t>()(C,5), sgs_GetVar<bool>()(C,6) ); return 0;
}

static int _sgs_method__AIDBSystem__GetRoomList( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, GetRoomList ) ) return 0;
	return data->sgsGetRoomList( C );
}

static int _sgs_method__AIDBSystem__GetRoomNameByPos( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, GetRoomNameByPos ) ) return 0;
	sgs_PushVar(C,data->sgsGetRoomNameByPos( C, sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__AIDBSystem__GetRoomByPos( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, GetRoomByPos ) ) return 0;
	return data->sgsGetRoomByPos( C, sgs_GetVar<Vec3>()(C,0) );
}

static int _sgs_method__AIDBSystem__GetRoomPoints( SGS_CTX )
{
	AIDBSystem* data; if( !SGS_PARSE_METHOD( C, AIDBSystem::_sgs_interface, data, AIDBSystem, GetRoomPoints ) ) return 0;
	return data->sgsGetRoomPoints( C, sgs_GetVar<StringView>()(C,0) );
}

int AIDBSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<AIDBSystem*>( obj->data )->~AIDBSystem();
	return SGS_SUCCESS;
}

int AIDBSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int AIDBSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int AIDBSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int AIDBSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 42 ];
	sprintf( bfr, "AIDBSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

static sgs_RegFuncConst AIDBSystem__sgs_funcs[] =
{
	{ "AddSound", _sgs_method__AIDBSystem__AddSound },
	{ "HasFact", _sgs_method__AIDBSystem__HasFact },
	{ "HasRecentFact", _sgs_method__AIDBSystem__HasRecentFact },
	{ "GetRecentFact", _sgs_method__AIDBSystem__GetRecentFact },
	{ "InsertFact", _sgs_method__AIDBSystem__InsertFact },
	{ "UpdateFact", _sgs_method__AIDBSystem__UpdateFact },
	{ "InsertOrUpdateFact", _sgs_method__AIDBSystem__InsertOrUpdateFact },
	{ "GetRoomList", _sgs_method__AIDBSystem__GetRoomList },
	{ "GetRoomNameByPos", _sgs_method__AIDBSystem__GetRoomNameByPos },
	{ "GetRoomByPos", _sgs_method__AIDBSystem__GetRoomByPos },
	{ "GetRoomPoints", _sgs_method__AIDBSystem__GetRoomPoints },
	{ NULL, NULL },
};

static int AIDBSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		AIDBSystem__sgs_funcs,
		-1, "AIDBSystem." );
	return 1;
}

static sgs_ObjInterface AIDBSystem__sgs_interface =
{
	"AIDBSystem",
	NULL, AIDBSystem::_sgs_gcmark, AIDBSystem::_sgs_getindex, AIDBSystem::_sgs_setindex, NULL, NULL, AIDBSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface AIDBSystem::_sgs_interface(AIDBSystem__sgs_interface, AIDBSystem__sgs_ifn);


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
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<Trigger*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
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
		SGS_CASE( "position" ){ static_cast<Trigger*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<Trigger*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<Trigger*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<Trigger*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<Trigger*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<Trigger*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<Trigger*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<Trigger*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<Trigger*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<Trigger*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<Trigger*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_id, depth ).push( C ); }
		{ sgs_PushString( C, "\nfunc = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_func, depth ).push( C ); }
		{ sgs_PushString( C, "\nfuncOut = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_funcOut, depth ).push( C ); }
		{ sgs_PushString( C, "\nonce = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_once, depth ).push( C ); }
		{ sgs_PushString( C, "\ndone = " ); sgs_DumpData( C, static_cast<Trigger*>( obj->data )->m_done, depth ).push( C ); }
		sgs_StringConcat( C, 44 );
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
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<SlidingDoor*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
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
		SGS_CASE( "position" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<SlidingDoor*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<SlidingDoor*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<SlidingDoor*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<SlidingDoor*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
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
		sgs_StringConcat( C, 48 );
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
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<PickupItem*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
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
		SGS_CASE( "position" ){ static_cast<PickupItem*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<PickupItem*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<PickupItem*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<PickupItem*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<PickupItem*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<PickupItem*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<PickupItem*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<PickupItem*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<PickupItem*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<PickupItem*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<PickupItem*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<PickupItem*>( obj->data )->m_id, depth ).push( C ); }
		sgs_StringConcat( C, 36 );
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
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<Actionable*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
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
		SGS_CASE( "rotation" ){ static_cast<Actionable*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<Actionable*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<Actionable*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<Actionable*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<Actionable*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<Actionable*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<Actionable*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<Actionable*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<Actionable*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<Actionable*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_id, depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeEstimate = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.timeEstimate, depth ).push( C ); }
		{ sgs_PushString( C, "\ntimeActual = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_info.timeActual, depth ).push( C ); }
		{ sgs_PushString( C, "\nonSuccess = " ); sgs_DumpData( C, static_cast<Actionable*>( obj->data )->m_onSuccess, depth ).push( C ); }
		sgs_StringConcat( C, 44 );
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
	Actionable::_sgs_destruct, Actionable::_sgs_gcmark, Actionable::_sgs_getindex, Actionable::_sgs_setindex, NULL, NULL, Actionable::_sgs_dump, NULL, NULL, NULL, 
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
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
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
		SGS_CASE( "castLMS" ){ sgs_PushVar( C, static_cast<MeshEntity*>( obj->data )->m_castLMS ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<MeshEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MeshEntity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MeshEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<MeshEntity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<MeshEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<MeshEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<MeshEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<MeshEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<MeshEntity*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<MeshEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<MeshEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<MeshEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<MeshEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<MeshEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<MeshEntity*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<MeshEntity*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<MeshEntity*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<MeshEntity*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<MeshEntity*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "isStatic" ){ static_cast<MeshEntity*>( obj->data )->SetStatic( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "visible" ){ static_cast<MeshEntity*>( obj->data )->SetVisible( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "mesh" ){ static_cast<MeshEntity*>( obj->data )->SetMesh( sgs_GetVar<MeshHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "solid" ){ static_cast<MeshEntity*>( obj->data )->SetSolid( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "lightingMode" ){ static_cast<MeshEntity*>( obj->data )->SetLightingMode( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "lmQuality" ){ static_cast<MeshEntity*>( obj->data )->m_lmQuality = sgs_GetVar<float>()( C, 1 );
			static_cast<MeshEntity*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
		SGS_CASE( "castLMS" ){ static_cast<MeshEntity*>( obj->data )->m_castLMS = sgs_GetVar<bool>()( C, 1 );
			static_cast<MeshEntity*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
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
		{ sgs_PushString( C, "\ncastLMS = " ); sgs_DumpData( C, static_cast<MeshEntity*>( obj->data )->m_castLMS, depth ).push( C ); }
		sgs_StringConcat( C, 50 );
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
	MeshEntity::_sgs_destruct, MeshEntity::_sgs_gcmark, MeshEntity::_sgs_getindex, MeshEntity::_sgs_setindex, NULL, NULL, MeshEntity::_sgs_dump, NULL, NULL, NULL, 
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
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
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
		SGS_CASE( "lightRadius" ){ sgs_PushVar( C, static_cast<LightEntity*>( obj->data )->m_lightRadius ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<LightEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int LightEntity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LightEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<LightEntity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<LightEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<LightEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<LightEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<LightEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<LightEntity*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<LightEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<LightEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<LightEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<LightEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<LightEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<LightEntity*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
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
		SGS_CASE( "innerAngle" ){ static_cast<LightEntity*>( obj->data )->m_innerAngle = sgs_GetVar<float>()( C, 1 );
			static_cast<LightEntity*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
		SGS_CASE( "spotCurve" ){ static_cast<LightEntity*>( obj->data )->m_spotCurve = sgs_GetVar<float>()( C, 1 );
			static_cast<LightEntity*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
		SGS_CASE( "lightRadius" ){ static_cast<LightEntity*>( obj->data )->m_lightRadius = sgs_GetVar<float>()( C, 1 );
			static_cast<LightEntity*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
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
		{ sgs_PushString( C, "\nlightRadius = " ); sgs_DumpData( C, static_cast<LightEntity*>( obj->data )->m_lightRadius, depth ).push( C ); }
		sgs_StringConcat( C, 68 );
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
	LightEntity::_sgs_destruct, LightEntity::_sgs_gcmark, LightEntity::_sgs_getindex, LightEntity::_sgs_setindex, NULL, NULL, LightEntity::_sgs_dump, NULL, NULL, NULL, 
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
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
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
		SGS_CASE( "position" ){ static_cast<MultiEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<MultiEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<MultiEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<MultiEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<MultiEntity*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<MultiEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<MultiEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<MultiEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<MultiEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<MultiEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<MultiEntity*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->m_id, depth ).push( C ); }
		sgs_StringConcat( C, 36 );
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

