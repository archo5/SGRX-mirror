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


static int _sgs_method__SGSMeshInstHandle__SetMesh( SGS_CTX )
{
	SGSMeshInstHandle* data; if( !SGS_PARSE_METHOD( C, SGSMeshInstHandle::_sgs_interface, data, SGSMeshInstHandle, SetMesh ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->SetMesh( sgs_GetVar<sgsVariable>()(C,0), sgs_GetVar<bool>()(C,1) ); return 0;
}

static int _sgs_method__SGSMeshInstHandle__SetMITexture( SGS_CTX )
{
	SGSMeshInstHandle* data; if( !SGS_PARSE_METHOD( C, SGSMeshInstHandle::_sgs_interface, data, SGSMeshInstHandle, SetMITexture ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->SetMITexture( sgs_GetVar<int>()(C,0), sgs_GetVar<TextureHandle>()(C,1) ); return 0;
}

int SGSMeshInstHandle::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<SGSMeshInstHandle*>( obj->data )->C = C;
	static_cast<SGSMeshInstHandle*>( obj->data )->~SGSMeshInstHandle();
	return SGS_SUCCESS;
}

int SGSMeshInstHandle::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSMeshInstHandle*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int SGSMeshInstHandle::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSMeshInstHandle*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "layers" ){ sgs_PushVar( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->layers ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->enabled ); return SGS_SUCCESS; }
		SGS_CASE( "allowStaticDecals" ){ sgs_PushVar( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->allowStaticDecals ); return SGS_SUCCESS; }
		SGS_CASE( "sortidx" ){ sgs_PushVar( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->sortidx ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->matrix ); return SGS_SUCCESS; }
		SGS_CASE( "mesh" ){ sgs_PushVar( C, static_cast<SGSMeshInstHandle*>( obj->data )->sgsGetMesh() ); return SGS_SUCCESS; }
		SGS_CASE( "lightingMode" ){ sgs_PushVar( C, static_cast<SGSMeshInstHandle*>( obj->data )->sgsGetLightingMode() ); return SGS_SUCCESS; }
		SGS_CASE( "materialCount" ){ sgs_PushVar( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->GetMaterialCount() ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGSMeshInstHandle::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSMeshInstHandle*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "layers" ){ static_cast<SGSMeshInstHandle*>( obj->data )->h->layers = sgs_GetVar<uint32_t>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<SGSMeshInstHandle*>( obj->data )->h->enabled = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "allowStaticDecals" ){ static_cast<SGSMeshInstHandle*>( obj->data )->h->allowStaticDecals = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "sortidx" ){ static_cast<SGSMeshInstHandle*>( obj->data )->h->sortidx = sgs_GetVar<uint32_t>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<SGSMeshInstHandle*>( obj->data )->h->matrix = sgs_GetVar<Mat4>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "mesh" ){ static_cast<SGSMeshInstHandle*>( obj->data )->h->SetMesh( sgs_GetVar<MeshHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "lightingMode" ){ static_cast<SGSMeshInstHandle*>( obj->data )->sgsSetLightingMode( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "materialCount" ){ static_cast<SGSMeshInstHandle*>( obj->data )->h->SetMaterialCount( sgs_GetVar<uint16_t>()( C, 1 ) ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int SGSMeshInstHandle::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<SGSMeshInstHandle*>( obj->data )->C, C );
	char bfr[ 49 ];
	sprintf( bfr, "SGSMeshInstHandle (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlayers = " ); sgs_DumpData( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->layers, depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->enabled, depth ).push( C ); }
		{ sgs_PushString( C, "\nallowStaticDecals = " ); sgs_DumpData( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->allowStaticDecals, depth ).push( C ); }
		{ sgs_PushString( C, "\nsortidx = " ); sgs_DumpData( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->sortidx, depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->matrix, depth ).push( C ); }
		{ sgs_PushString( C, "\nmesh = " ); sgs_DumpData( C, static_cast<SGSMeshInstHandle*>( obj->data )->sgsGetMesh(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlightingMode = " ); sgs_DumpData( C, static_cast<SGSMeshInstHandle*>( obj->data )->sgsGetLightingMode(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmaterialCount = " ); sgs_DumpData( C, static_cast<SGSMeshInstHandle*>( obj->data )->h->GetMaterialCount(), depth ).push( C ); }
		sgs_StringConcat( C, 16 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst SGSMeshInstHandle__sgs_funcs[] =
{
	{ "SetMesh", _sgs_method__SGSMeshInstHandle__SetMesh },
	{ "SetMITexture", _sgs_method__SGSMeshInstHandle__SetMITexture },
	{ NULL, NULL },
};

static int SGSMeshInstHandle__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		SGSMeshInstHandle__sgs_funcs,
		-1, "SGSMeshInstHandle." );
	return 1;
}

static sgs_ObjInterface SGSMeshInstHandle__sgs_interface =
{
	"SGSMeshInstHandle",
	SGSMeshInstHandle::_sgs_destruct, SGSMeshInstHandle::_sgs_gcmark, SGSMeshInstHandle::_sgs_getindex, SGSMeshInstHandle::_sgs_setindex, NULL, NULL, SGSMeshInstHandle::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface SGSMeshInstHandle::_sgs_interface(SGSMeshInstHandle__sgs_interface, SGSMeshInstHandle__sgs_ifn);


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

static int _sgs_method__GameUIControl__IP( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, IP ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->IP( sgs_GetVar<Vec2>()(C,0) )); return 1;
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

static int _sgs_method__GameUIControl__InvIP( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, InvIP ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->InvIP( sgs_GetVar<Vec2>()(C,0) )); return 1;
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

static int _sgs_method__GameUIControl__DQuadWH( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DQuadWH ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DQuadWH( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3) ); return 0;
}

static int _sgs_method__GameUIControl__DQuadTexRect( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DQuadTexRect ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DQuadTexRect( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<float>()(C,4), sgs_GetVar<float>()(C,5), sgs_GetVar<float>()(C,6), sgs_GetVar<float>()(C,7) ); return 0;
}

static int _sgs_method__GameUIControl__DQuadExt( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DQuadExt ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DQuadExt( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<float>()(C,4), sgs_GetVar<float>()(C,5), sgs_GetVar<float>()(C,6), sgs_GetVar<float>()(C,7) ); return 0;
}

static int _sgs_method__GameUIControl__DTurnedBox( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DTurnedBox ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DTurnedBox( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<float>()(C,4) ); return 0;
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
	{ "IP", _sgs_method__GameUIControl__IP },
	{ "InvIX", _sgs_method__GameUIControl__InvIX },
	{ "InvIY", _sgs_method__GameUIControl__InvIY },
	{ "InvIS", _sgs_method__GameUIControl__InvIS },
	{ "InvIP", _sgs_method__GameUIControl__InvIP },
	{ "CreateScreen", _sgs_method__GameUIControl__CreateScreen },
	{ "CreateControl", _sgs_method__GameUIControl__CreateControl },
	{ "DReset", _sgs_method__GameUIControl__DReset },
	{ "DCol", _sgs_method__GameUIControl__DCol },
	{ "DTex", _sgs_method__GameUIControl__DTex },
	{ "DQuad", _sgs_method__GameUIControl__DQuad },
	{ "DQuadWH", _sgs_method__GameUIControl__DQuadWH },
	{ "DQuadTexRect", _sgs_method__GameUIControl__DQuadTexRect },
	{ "DQuadExt", _sgs_method__GameUIControl__DQuadExt },
	{ "DTurnedBox", _sgs_method__GameUIControl__DTurnedBox },
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


static int _sgs_method__Entity__GetChild( SGS_CTX )
{
	Entity* data; if( !SGS_PARSE_METHOD( C, Entity::_sgs_interface, data, Entity, GetChild ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetChild( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__Entity__LocalToWorld( SGS_CTX )
{
	Entity* data; if( !SGS_PARSE_METHOD( C, Entity::_sgs_interface, data, Entity, LocalToWorld ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->LocalToWorld( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__Entity__WorldToLocal( SGS_CTX )
{
	Entity* data; if( !SGS_PARSE_METHOD( C, Entity::_sgs_interface, data, Entity, WorldToLocal ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->WorldToLocal( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__Entity__LocalToWorldDir( SGS_CTX )
{
	Entity* data; if( !SGS_PARSE_METHOD( C, Entity::_sgs_interface, data, Entity, LocalToWorldDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->LocalToWorldDir( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__Entity__WorldToLocalDir( SGS_CTX )
{
	Entity* data; if( !SGS_PARSE_METHOD( C, Entity::_sgs_interface, data, Entity, WorldToLocalDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->WorldToLocalDir( sgs_GetVar<Vec3>()(C,0) )); return 1;
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
		SGS_CASE( "childCount" ){ sgs_PushVar( C, static_cast<Entity*>( obj->data )->_ch.size() ); return SGS_SUCCESS; }
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
		SGS_CASE( "position" ){ static_cast<Entity*>( obj->data )->SetWorldPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nchildCount = " ); sgs_DumpData( C, static_cast<Entity*>( obj->data )->_ch.size(), depth ).push( C ); }
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
		sgs_StringConcat( C, 38 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst Entity__sgs_funcs[] =
{
	{ "GetChild", _sgs_method__Entity__GetChild },
	{ "LocalToWorld", _sgs_method__Entity__LocalToWorld },
	{ "WorldToLocal", _sgs_method__Entity__WorldToLocal },
	{ "LocalToWorldDir", _sgs_method__Entity__LocalToWorldDir },
	{ "WorldToLocalDir", _sgs_method__Entity__WorldToLocalDir },
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


static int _sgs_method__IActorController__FixedTick( SGS_CTX )
{
	IActorController* data; if( !SGS_PARSE_METHOD( C, IActorController::_sgs_interface, data, IActorController, FixedTick ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->FixedTick( sgs_GetVar<float>()(C,0) ); return 0;
}

static int _sgs_method__IActorController__Tick( SGS_CTX )
{
	IActorController* data; if( !SGS_PARSE_METHOD( C, IActorController::_sgs_interface, data, IActorController, Tick ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Tick( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1) ); return 0;
}

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
		SGS_CASE( "entity" ){ sgs_PushVar( C, static_cast<IActorController*>( obj->data )->sgsGetEntity() ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int IActorController::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<IActorController*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
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
		{ sgs_PushString( C, "\nentity = " ); sgs_DumpData( C, static_cast<IActorController*>( obj->data )->sgsGetEntity(), depth ).push( C ); }
		sgs_StringConcat( C, 2 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst IActorController__sgs_funcs[] =
{
	{ "FixedTick", _sgs_method__IActorController__FixedTick },
	{ "Tick", _sgs_method__IActorController__Tick },
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
	IActorController::_sgs_destruct, IActorController::_sgs_gcmark, IActorController::_sgs_getindex, IActorController::_sgs_setindex, NULL, NULL, IActorController::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface IActorController::_sgs_interface(IActorController__sgs_interface, IActorController__sgs_ifn);


static int _sgs_method__Actor__GetChild( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, GetChild ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetChild( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__Actor__LocalToWorld( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, LocalToWorld ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->LocalToWorld( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__Actor__WorldToLocal( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, WorldToLocal ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->WorldToLocal( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__Actor__LocalToWorldDir( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, LocalToWorldDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->LocalToWorldDir( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__Actor__WorldToLocalDir( SGS_CTX )
{
	Actor* data; if( !SGS_PARSE_METHOD( C, Actor::_sgs_interface, data, Actor, WorldToLocalDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->WorldToLocalDir( sgs_GetVar<Vec3>()(C,0) )); return 1;
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
		SGS_CASE( "childCount" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->_ch.size() ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
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
		SGS_CASE( "ctrl" ){ sgs_PushVar( C, static_cast<Actor*>( obj->data )->ctrl ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<Actor*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int Actor::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<Actor*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<Actor*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<Actor*>( obj->data )->SetWorldPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
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
		SGS_CASE( "ctrl" ){ static_cast<Actor*>( obj->data )->sgsSetCtrl( sgs_GetVar<ActorCtrlScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nchildCount = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->_ch.size(), depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
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
		{ sgs_PushString( C, "\nctrl = " ); sgs_DumpData( C, static_cast<Actor*>( obj->data )->ctrl, depth ).push( C ); }
		sgs_StringConcat( C, 40 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst Actor__sgs_funcs[] =
{
	{ "GetChild", _sgs_method__Actor__GetChild },
	{ "LocalToWorld", _sgs_method__Actor__LocalToWorld },
	{ "WorldToLocal", _sgs_method__Actor__WorldToLocal },
	{ "LocalToWorldDir", _sgs_method__Actor__LocalToWorldDir },
	{ "WorldToLocalDir", _sgs_method__Actor__WorldToLocalDir },
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


static int _sgs_method__GOResource__GetWorldMatrix( SGS_CTX )
{
	GOResource* data; if( !SGS_PARSE_METHOD( C, GOResource::_sgs_interface, data, GOResource, GetWorldMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetWorldMatrix(  )); return 1;
}

int GOResource::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GOResource*>( obj->data )->C = C;
	static_cast<GOResource*>( obj->data )->~GOResource();
	return SGS_SUCCESS;
}

int GOResource::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOResource*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int GOResource::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<GOResource*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<GOResource*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<GOResource*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<GOResource*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<GOResource*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ sgs_PushVar( C, static_cast<GOResource*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ sgs_PushVar( C, static_cast<GOResource*>( obj->data )->GetMatrixMode() ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<GOResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GOResource::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<GOResource*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ static_cast<GOResource*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ static_cast<GOResource*>( obj->data )->SetMatrixMode( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<GOResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GOResource::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOResource*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "GOResource (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<GOResource*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<GOResource*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<GOResource*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<GOResource*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<GOResource*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalMatrix = " ); sgs_DumpData( C, static_cast<GOResource*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmatrixMode = " ); sgs_DumpData( C, static_cast<GOResource*>( obj->data )->GetMatrixMode(), depth ).push( C ); }
		sgs_StringConcat( C, 14 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst GOResource__sgs_funcs[] =
{
	{ "GetWorldMatrix", _sgs_method__GOResource__GetWorldMatrix },
	{ NULL, NULL },
};

static int GOResource__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		GOResource__sgs_funcs,
		-1, "GOResource." );
	return 1;
}

static sgs_ObjInterface GOResource__sgs_interface =
{
	"GOResource",
	NULL, GOResource::_sgs_gcmark, GOResource::_sgs_getindex, GOResource::_sgs_setindex, NULL, NULL, GOResource::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface GOResource::_sgs_interface(GOResource__sgs_interface, GOResource__sgs_ifn, &LevelScrObj::_sgs_interface);


static int _sgs_method__GOResourceTable__GetNames( SGS_CTX )
{
	GOResourceTable* data; if( !SGS_PARSE_METHOD( C, GOResourceTable::_sgs_interface, data, GOResourceTable, GetNames ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsGetNames(  )); return 1;
}

int GOResourceTable::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GOResourceTable*>( obj->data )->C = C;
	static_cast<GOResourceTable*>( obj->data )->~GOResourceTable();
	return SGS_SUCCESS;
}

int GOResourceTable::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOResourceTable*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int GOResourceTable::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOResourceTable*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<GOResourceTable*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<GOResourceTable*>( obj->data )->_data ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<GOResourceTable*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GOResourceTable::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOResourceTable*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<GOResourceTable*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<GOResourceTable*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GOResourceTable::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOResourceTable*>( obj->data )->C, C );
	char bfr[ 47 ];
	sprintf( bfr, "GOResourceTable (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<GOResourceTable*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<GOResourceTable*>( obj->data )->_data, depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst GOResourceTable__sgs_funcs[] =
{
	{ "GetNames", _sgs_method__GOResourceTable__GetNames },
	{ NULL, NULL },
};

static int GOResourceTable__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		GOResourceTable__sgs_funcs,
		-1, "GOResourceTable." );
	return 1;
}

static sgs_ObjInterface GOResourceTable__sgs_interface =
{
	"GOResourceTable",
	NULL, GOResourceTable::_sgs_gcmark, GOResourceTable::getindex, GOResourceTable::_sgs_setindex, NULL, NULL, GOResourceTable::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface GOResourceTable::_sgs_interface(GOResourceTable__sgs_interface, GOResourceTable__sgs_ifn, &LevelScrObj::_sgs_interface);


int GOBehavior::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GOBehavior*>( obj->data )->C = C;
	static_cast<GOBehavior*>( obj->data )->~GOBehavior();
	return SGS_SUCCESS;
}

int GOBehavior::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOBehavior*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int GOBehavior::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOBehavior*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<GOBehavior*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<GOBehavior*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<GOBehavior*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<GOBehavior*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<GOBehavior*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "resources" ){ sgs_PushVar( C, static_cast<GOBehavior*>( obj->data )->_get_resources() ); return SGS_SUCCESS; }
		SGS_CASE( "behaviors" ){ sgs_PushVar( C, static_cast<GOBehavior*>( obj->data )->_get_behaviors() ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<GOBehavior*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GOBehavior::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOBehavior*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<GOBehavior*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<GOBehavior*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GOBehavior::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOBehavior*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "GOBehavior (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<GOBehavior*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<GOBehavior*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<GOBehavior*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<GOBehavior*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<GOBehavior*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\nresources = " ); sgs_DumpData( C, static_cast<GOBehavior*>( obj->data )->_get_resources(), depth ).push( C ); }
		{ sgs_PushString( C, "\nbehaviors = " ); sgs_DumpData( C, static_cast<GOBehavior*>( obj->data )->_get_behaviors(), depth ).push( C ); }
		sgs_StringConcat( C, 14 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst GOBehavior__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int GOBehavior__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		GOBehavior__sgs_funcs,
		-1, "GOBehavior." );
	return 1;
}

static sgs_ObjInterface GOBehavior__sgs_interface =
{
	"GOBehavior",
	NULL, GOBehavior::_sgs_gcmark, GOBehavior::_sgs_getindex, GOBehavior::_sgs_setindex, NULL, NULL, GOBehavior::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface GOBehavior::_sgs_interface(GOBehavior__sgs_interface, GOBehavior__sgs_ifn, &LevelScrObj::_sgs_interface);


int GOBehaviorTable::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GOBehaviorTable*>( obj->data )->C = C;
	static_cast<GOBehaviorTable*>( obj->data )->~GOBehaviorTable();
	return SGS_SUCCESS;
}

int GOBehaviorTable::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOBehaviorTable*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int GOBehaviorTable::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOBehaviorTable*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<GOBehaviorTable*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<GOBehaviorTable*>( obj->data )->_data ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<GOBehaviorTable*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GOBehaviorTable::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOBehaviorTable*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<GOBehaviorTable*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<GOBehaviorTable*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GOBehaviorTable::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GOBehaviorTable*>( obj->data )->C, C );
	char bfr[ 47 ];
	sprintf( bfr, "GOBehaviorTable (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<GOBehaviorTable*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<GOBehaviorTable*>( obj->data )->_data, depth ).push( C ); }
		sgs_StringConcat( C, 4 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst GOBehaviorTable__sgs_funcs[] =
{
	{ NULL, NULL },
};

static int GOBehaviorTable__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		GOBehaviorTable__sgs_funcs,
		-1, "GOBehaviorTable." );
	return 1;
}

static sgs_ObjInterface GOBehaviorTable__sgs_interface =
{
	"GOBehaviorTable",
	NULL, GOBehaviorTable::_sgs_gcmark, GOBehaviorTable::getindex, GOBehaviorTable::_sgs_setindex, NULL, NULL, GOBehaviorTable::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface GOBehaviorTable::_sgs_interface(GOBehaviorTable__sgs_interface, GOBehaviorTable__sgs_ifn, &LevelScrObj::_sgs_interface);


static int _sgs_method__GameObject__RemoveResource( SGS_CTX )
{
	GameObject* data; if( !SGS_PARSE_METHOD( C, GameObject::_sgs_interface, data, GameObject, RemoveResource ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RemoveResource( sgs_GetVar<sgsString>()(C,0) ); return 0;
}

static int _sgs_method__GameObject__RemoveBehavior( SGS_CTX )
{
	GameObject* data; if( !SGS_PARSE_METHOD( C, GameObject::_sgs_interface, data, GameObject, RemoveBehavior ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->RemoveBehavior( sgs_GetVar<sgsString>()(C,0) ); return 0;
}

static int _sgs_method__GameObject__AddResource( SGS_CTX )
{
	GameObject* data; if( !SGS_PARSE_METHOD( C, GameObject::_sgs_interface, data, GameObject, AddResource ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsAddResource( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<uint32_t>()(C,1), sgs_GetVar<bool>()(C,2) )); return 1;
}

static int _sgs_method__GameObject__AddBehavior( SGS_CTX )
{
	GameObject* data; if( !SGS_PARSE_METHOD( C, GameObject::_sgs_interface, data, GameObject, AddBehavior ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsAddBehavior( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<sgsString>()(C,1), sgs_GetVar<bool>()(C,2) )); return 1;
}

static int _sgs_method__GameObject__RequireResource( SGS_CTX )
{
	GameObject* data; if( !SGS_PARSE_METHOD( C, GameObject::_sgs_interface, data, GameObject, RequireResource ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsRequireResource( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<uint32_t>()(C,1) )); return 1;
}

static int _sgs_method__GameObject__RequireBehavior( SGS_CTX )
{
	GameObject* data; if( !SGS_PARSE_METHOD( C, GameObject::_sgs_interface, data, GameObject, RequireBehavior ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsRequireBehavior( sgs_GetVar<sgsString>()(C,0), sgs_GetVar<sgsString>()(C,1) )); return 1;
}

int GameObject::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GameObject*>( obj->data )->C = C;
	static_cast<GameObject*>( obj->data )->~GameObject();
	return SGS_SUCCESS;
}

int GameObject::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameObject*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int GameObject::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameObject*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->m_id ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "resources" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->_get_resources() ); return SGS_SUCCESS; }
		SGS_CASE( "behaviors" ){ sgs_PushVar( C, static_cast<GameObject*>( obj->data )->_get_behaviors() ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<GameObject*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GameObject::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameObject*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<GameObject*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<GameObject*>( obj->data )->m_name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<GameObject*>( obj->data )->m_id = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<GameObject*>( obj->data )->SetWorldPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<GameObject*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<GameObject*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<GameObject*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<GameObject*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<GameObject*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<GameObject*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<GameObject*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<GameObject*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<GameObject*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<GameObject*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GameObject::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameObject*>( obj->data )->C, C );
	char bfr[ 42 ];
	sprintf( bfr, "GameObject (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->m_id, depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nresources = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->_get_resources(), depth ).push( C ); }
		{ sgs_PushString( C, "\nbehaviors = " ); sgs_DumpData( C, static_cast<GameObject*>( obj->data )->_get_behaviors(), depth ).push( C ); }
		sgs_StringConcat( C, 32 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst GameObject__sgs_funcs[] =
{
	{ "RemoveResource", _sgs_method__GameObject__RemoveResource },
	{ "RemoveBehavior", _sgs_method__GameObject__RemoveBehavior },
	{ "AddResource", _sgs_method__GameObject__AddResource },
	{ "AddBehavior", _sgs_method__GameObject__AddBehavior },
	{ "RequireResource", _sgs_method__GameObject__RequireResource },
	{ "RequireBehavior", _sgs_method__GameObject__RequireBehavior },
	{ NULL, NULL },
};

static int GameObject__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		GameObject__sgs_funcs,
		-1, "GameObject." );
	return 1;
}

static sgs_ObjInterface GameObject__sgs_interface =
{
	"GameObject",
	NULL, GameObject::_sgs_gcmark, GameObject::_sgs_getindex, GameObject::_sgs_setindex, NULL, NULL, GameObject::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface GameObject::_sgs_interface(GameObject__sgs_interface, GameObject__sgs_ifn, &LevelScrObj::_sgs_interface);


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

static int _sgs_method__GameLevel__GetCursorMeshInst( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, GetCursorMeshInst ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	return data->sgsGetCursorMeshInst( sgs_GetVar<uint32_t>()(C,0) );
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

static int _sgs_method__GameLevel__PlaySound( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, PlaySound ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->PlaySound( sgs_GetVar<StringView>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<Vec3>()(C,2) ); return 0;
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

static int _sgs_method__GameLevel__UpdateCameraMatrices( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, UpdateCameraMatrices ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->UpdateCameraMatrices(  ); return 0;
}

static int _sgs_method__GameLevel__CreateGameObject( SGS_CTX )
{
	GameLevel* data; if( !SGS_PARSE_METHOD( C, GameLevel::_sgs_interface, data, GameLevel, CreateGameObject ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->sgsCreateGameObject(  )); return 1;
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
		SGS_CASE( "cameraPosition" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.position ); return SGS_SUCCESS; }
		SGS_CASE( "cameraDirection" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.direction ); return SGS_SUCCESS; }
		SGS_CASE( "cameraUpdir" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.updir ); return SGS_SUCCESS; }
		SGS_CASE( "cameraZNear" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.znear ); return SGS_SUCCESS; }
		SGS_CASE( "cameraZFar" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.zfar ); return SGS_SUCCESS; }
		SGS_CASE( "cameraAngle" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.angle ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->m_levelName ); return SGS_SUCCESS; }
		SGS_CASE( "nextLevel" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->m_nextLevel ); return SGS_SUCCESS; }
		SGS_CASE( "persistent" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->m_persistent ); return SGS_SUCCESS; }
		SGS_CASE( "paused" ){ sgs_PushVar( C, static_cast<GameLevel*>( obj->data )->m_paused ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<GameLevel*>( obj->data )->m_metadata.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int GameLevel::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<GameLevel*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "cameraPosition" ){ static_cast<GameLevel*>( obj->data )->GetScene()->camera.position = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "cameraDirection" ){ static_cast<GameLevel*>( obj->data )->GetScene()->camera.direction = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "cameraUpdir" ){ static_cast<GameLevel*>( obj->data )->GetScene()->camera.updir = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "cameraZNear" ){ static_cast<GameLevel*>( obj->data )->GetScene()->camera.znear = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "cameraZFar" ){ static_cast<GameLevel*>( obj->data )->GetScene()->camera.zfar = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "cameraAngle" ){ static_cast<GameLevel*>( obj->data )->GetScene()->camera.angle = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "nextLevel" ){ static_cast<GameLevel*>( obj->data )->m_nextLevel = sgs_GetVar<String>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "persistent" ){ static_cast<GameLevel*>( obj->data )->m_persistent = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "paused" ){ static_cast<GameLevel*>( obj->data )->m_paused = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\ncameraPosition = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.position, depth ).push( C ); }
		{ sgs_PushString( C, "\ncameraDirection = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.direction, depth ).push( C ); }
		{ sgs_PushString( C, "\ncameraUpdir = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.updir, depth ).push( C ); }
		{ sgs_PushString( C, "\ncameraZNear = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.znear, depth ).push( C ); }
		{ sgs_PushString( C, "\ncameraZFar = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.zfar, depth ).push( C ); }
		{ sgs_PushString( C, "\ncameraAngle = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->GetScene()->camera.angle, depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->m_levelName, depth ).push( C ); }
		{ sgs_PushString( C, "\nnextLevel = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->m_nextLevel, depth ).push( C ); }
		{ sgs_PushString( C, "\npersistent = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->m_persistent, depth ).push( C ); }
		{ sgs_PushString( C, "\npaused = " ); sgs_DumpData( C, static_cast<GameLevel*>( obj->data )->m_paused, depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst GameLevel__sgs_funcs[] =
{
	{ "CreateEntity", _sgs_method__GameLevel__CreateEntity },
	{ "DestroyEntity", _sgs_method__GameLevel__DestroyEntity },
	{ "FindEntity", _sgs_method__GameLevel__FindEntity },
	{ "SetCameraPosDir", _sgs_method__GameLevel__SetCameraPosDir },
	{ "WorldToScreen", _sgs_method__GameLevel__WorldToScreen },
	{ "WorldToScreenPx", _sgs_method__GameLevel__WorldToScreenPx },
	{ "GetCursorWorldPoint", _sgs_method__GameLevel__GetCursorWorldPoint },
	{ "GetCursorMeshInst", _sgs_method__GameLevel__GetCursorMeshInst },
	{ "Query", _sgs_method__GameLevel__Query },
	{ "QuerySphere", _sgs_method__GameLevel__QuerySphere },
	{ "QueryOBB", _sgs_method__GameLevel__QueryOBB },
	{ "PlaySound", _sgs_method__GameLevel__PlaySound },
	{ "GetTickTime", _sgs_method__GameLevel__GetTickTime },
	{ "GetPhyTime", _sgs_method__GameLevel__GetPhyTime },
	{ "UpdateCameraMatrices", _sgs_method__GameLevel__UpdateCameraMatrices },
	{ "CreateGameObject", _sgs_method__GameLevel__CreateGameObject },
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


static int _sgs_method__LevelMapSystem__DrawUIRect( SGS_CTX )
{
	LevelMapSystem* data; if( !SGS_PARSE_METHOD( C, LevelMapSystem::_sgs_interface, data, LevelMapSystem, DrawUIRect ) ) return 0;
	data->DrawUIRect( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<float>()(C,4) ); return 0;
}

static int _sgs_method__LevelMapSystem__Update( SGS_CTX )
{
	LevelMapSystem* data; if( !SGS_PARSE_METHOD( C, LevelMapSystem::_sgs_interface, data, LevelMapSystem, Update ) ) return 0;
	data->sgsUpdate( sgs_GetVar<EntityScrHandle>()(C,0), sgs_GetVar<int>()(C,1), sgs_GetVar<Vec3>()(C,2), sgs_GetVar<Vec3>()(C,3), sgs_GetVar<float>()(C,4), sgs_GetVar<float>()(C,5) ); return 0;
}

static int _sgs_method__LevelMapSystem__Remove( SGS_CTX )
{
	LevelMapSystem* data; if( !SGS_PARSE_METHOD( C, LevelMapSystem::_sgs_interface, data, LevelMapSystem, Remove ) ) return 0;
	data->sgsRemove( sgs_GetVar<EntityScrHandle>()(C,0) ); return 0;
}

int LevelMapSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<LevelMapSystem*>( obj->data )->~LevelMapSystem();
	return SGS_SUCCESS;
}

int LevelMapSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int LevelMapSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewPos" ){ sgs_PushVar( C, static_cast<LevelMapSystem*>( obj->data )->viewPos ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int LevelMapSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "viewPos" ){ static_cast<LevelMapSystem*>( obj->data )->viewPos = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int LevelMapSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 46 ];
	sprintf( bfr, "LevelMapSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nviewPos = " ); sgs_DumpData( C, static_cast<LevelMapSystem*>( obj->data )->viewPos, depth ).push( C ); }
		sgs_StringConcat( C, 2 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst LevelMapSystem__sgs_funcs[] =
{
	{ "DrawUIRect", _sgs_method__LevelMapSystem__DrawUIRect },
	{ "Update", _sgs_method__LevelMapSystem__Update },
	{ "Remove", _sgs_method__LevelMapSystem__Remove },
	{ NULL, NULL },
};

static int LevelMapSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		LevelMapSystem__sgs_funcs,
		-1, "LevelMapSystem." );
	return 1;
}

static sgs_ObjInterface LevelMapSystem__sgs_interface =
{
	"LevelMapSystem",
	NULL, LevelMapSystem::_sgs_gcmark, LevelMapSystem::_sgs_getindex, LevelMapSystem::_sgs_setindex, NULL, NULL, LevelMapSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface LevelMapSystem::_sgs_interface(LevelMapSystem__sgs_interface, LevelMapSystem__sgs_ifn);


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


static int _sgs_method__HelpTextSystem__DrawText( SGS_CTX )
{
	HelpTextSystem* data; if( !SGS_PARSE_METHOD( C, HelpTextSystem::_sgs_interface, data, HelpTextSystem, DrawText ) ) return 0;
	data->DrawText(  ); return 0;
}

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
	{ "DrawText", _sgs_method__HelpTextSystem__DrawText },
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


static int _sgs_method__BulletSystem__Add( SGS_CTX )
{
	BulletSystem* data; if( !SGS_PARSE_METHOD( C, BulletSystem::_sgs_interface, data, BulletSystem, Add ) ) return 0;
	data->Add( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<uint32_t>()(C,4) ); return 0;
}

static int _sgs_method__BulletSystem__Zap( SGS_CTX )
{
	BulletSystem* data; if( !SGS_PARSE_METHOD( C, BulletSystem::_sgs_interface, data, BulletSystem, Zap ) ) return 0;
	sgs_PushVar(C,data->Zap( sgs_GetVar<Vec3>()(C,0), sgs_GetVar<Vec3>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<uint32_t>()(C,3) )); return 1;
}

int BulletSystem::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<BulletSystem*>( obj->data )->~BulletSystem();
	return SGS_SUCCESS;
}

int BulletSystem::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int BulletSystem::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int BulletSystem::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
	SGS_END_INDEXFUNC;
}

int BulletSystem::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 44 ];
	sprintf( bfr, "BulletSystem (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
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

static sgs_RegFuncConst BulletSystem__sgs_funcs[] =
{
	{ "Add", _sgs_method__BulletSystem__Add },
	{ "Zap", _sgs_method__BulletSystem__Zap },
	{ NULL, NULL },
};

static int BulletSystem__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		BulletSystem__sgs_funcs,
		-1, "BulletSystem." );
	return 1;
}

static sgs_ObjInterface BulletSystem__sgs_interface =
{
	"BulletSystem",
	NULL, BulletSystem::_sgs_gcmark, BulletSystem::_sgs_getindex, BulletSystem::_sgs_setindex, NULL, NULL, BulletSystem::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface BulletSystem::_sgs_interface(BulletSystem__sgs_interface, BulletSystem__sgs_ifn);


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


static int _sgs_method__ReflectionPlaneEntity__GetChild( SGS_CTX )
{
	ReflectionPlaneEntity* data; if( !SGS_PARSE_METHOD( C, ReflectionPlaneEntity::_sgs_interface, data, ReflectionPlaneEntity, GetChild ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetChild( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__ReflectionPlaneEntity__LocalToWorld( SGS_CTX )
{
	ReflectionPlaneEntity* data; if( !SGS_PARSE_METHOD( C, ReflectionPlaneEntity::_sgs_interface, data, ReflectionPlaneEntity, LocalToWorld ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->LocalToWorld( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__ReflectionPlaneEntity__WorldToLocal( SGS_CTX )
{
	ReflectionPlaneEntity* data; if( !SGS_PARSE_METHOD( C, ReflectionPlaneEntity::_sgs_interface, data, ReflectionPlaneEntity, WorldToLocal ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->WorldToLocal( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__ReflectionPlaneEntity__LocalToWorldDir( SGS_CTX )
{
	ReflectionPlaneEntity* data; if( !SGS_PARSE_METHOD( C, ReflectionPlaneEntity::_sgs_interface, data, ReflectionPlaneEntity, LocalToWorldDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->LocalToWorldDir( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__ReflectionPlaneEntity__WorldToLocalDir( SGS_CTX )
{
	ReflectionPlaneEntity* data; if( !SGS_PARSE_METHOD( C, ReflectionPlaneEntity::_sgs_interface, data, ReflectionPlaneEntity, WorldToLocalDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->WorldToLocalDir( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

int ReflectionPlaneEntity::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ReflectionPlaneEntity*>( obj->data )->C = C;
	static_cast<ReflectionPlaneEntity*>( obj->data )->~ReflectionPlaneEntity();
	return SGS_SUCCESS;
}

int ReflectionPlaneEntity::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ReflectionPlaneEntity*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ReflectionPlaneEntity::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ReflectionPlaneEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "childCount" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_ch.size() ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldScale() ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalPosition() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalRotation() ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalRotationXYZ() ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalScale() ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_sgsGetParent() ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetInfoMask() ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->m_infoTarget ); return SGS_SUCCESS; }
		SGS_CASE( "infoTarget" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldInfoTarget() ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->name ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ sgs_PushVar( C, static_cast<ReflectionPlaneEntity*>( obj->data )->m_id ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ReflectionPlaneEntity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ReflectionPlaneEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetWorldPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotation" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "rotationXYZ" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "scale" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "transform" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetWorldMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localPosition" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetLocalPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotation" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetLocalRotation( sgs_GetVar<Quat>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localRotationXYZ" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetLocalRotationXYZ( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localScale" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetLocalScale( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localTransform" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->_SetParent( sgs_GetVar<EntityScrHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "infoMask" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->SetInfoMask( sgs_GetVar<uint32_t>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "localInfoTarget" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->m_infoTarget = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "name" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->name = sgs_GetVar<sgsString>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "id" ){ static_cast<ReflectionPlaneEntity*>( obj->data )->sgsSetID( sgs_GetVar<sgsString>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ReflectionPlaneEntity::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ReflectionPlaneEntity*>( obj->data )->C, C );
	char bfr[ 53 ];
	sprintf( bfr, "ReflectionPlaneEntity (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nchildCount = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_ch.size(), depth ).push( C ); }
		{ sgs_PushString( C, "\nposition = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotation = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrotationXYZ = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nscale = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntransform = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalPosition = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalPosition(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotation = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalRotation(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalRotationXYZ = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalRotationXYZ(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalScale = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalScale(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalTransform = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->_sgsGetParent(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoMask = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetInfoMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalInfoTarget = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->m_infoTarget, depth ).push( C ); }
		{ sgs_PushString( C, "\ninfoTarget = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->GetWorldInfoTarget(), depth ).push( C ); }
		{ sgs_PushString( C, "\nname = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->name, depth ).push( C ); }
		{ sgs_PushString( C, "\nid = " ); sgs_DumpData( C, static_cast<ReflectionPlaneEntity*>( obj->data )->m_id, depth ).push( C ); }
		sgs_StringConcat( C, 38 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ReflectionPlaneEntity__sgs_funcs[] =
{
	{ "GetChild", _sgs_method__ReflectionPlaneEntity__GetChild },
	{ "LocalToWorld", _sgs_method__ReflectionPlaneEntity__LocalToWorld },
	{ "WorldToLocal", _sgs_method__ReflectionPlaneEntity__WorldToLocal },
	{ "LocalToWorldDir", _sgs_method__ReflectionPlaneEntity__LocalToWorldDir },
	{ "WorldToLocalDir", _sgs_method__ReflectionPlaneEntity__WorldToLocalDir },
	{ NULL, NULL },
};

static int ReflectionPlaneEntity__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ReflectionPlaneEntity__sgs_funcs,
		-1, "ReflectionPlaneEntity." );
	return 1;
}

static sgs_ObjInterface ReflectionPlaneEntity__sgs_interface =
{
	"ReflectionPlaneEntity",
	ReflectionPlaneEntity::_sgs_destruct, ReflectionPlaneEntity::_sgs_gcmark, ReflectionPlaneEntity::_sgs_getindex, ReflectionPlaneEntity::_sgs_setindex, NULL, NULL, ReflectionPlaneEntity::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ReflectionPlaneEntity::_sgs_interface(ReflectionPlaneEntity__sgs_interface, ReflectionPlaneEntity__sgs_ifn, &Entity::_sgs_interface);


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


static int _sgs_method__MultiEntity__GetChild( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, GetChild ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetChild( sgs_GetVar<int>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__LocalToWorld( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, LocalToWorld ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->LocalToWorld( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__WorldToLocal( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, WorldToLocal ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->WorldToLocal( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__LocalToWorldDir( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, LocalToWorldDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->LocalToWorldDir( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

static int _sgs_method__MultiEntity__WorldToLocalDir( SGS_CTX )
{
	MultiEntity* data; if( !SGS_PARSE_METHOD( C, MultiEntity::_sgs_interface, data, MultiEntity, WorldToLocalDir ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->WorldToLocalDir( sgs_GetVar<Vec3>()(C,0) )); return 1;
}

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
		SGS_CASE( "childCount" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->_ch.size() ); return SGS_SUCCESS; }
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
		SGS_CASE( "mi0mesh" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->sgsGetMI0Mesh() ); return SGS_SUCCESS; }
		SGS_CASE( "mi0sampleOffset" ){ sgs_PushVar( C, static_cast<MultiEntity*>( obj->data )->mi0sampleOffset ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<MultiEntity*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MultiEntity::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MultiEntity*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<MultiEntity*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "position" ){ static_cast<MultiEntity*>( obj->data )->SetWorldPosition( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
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
		SGS_CASE( "mi0mesh" ){ static_cast<MultiEntity*>( obj->data )->sgsSetMI0Mesh( sgs_GetVar<MeshHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "mi0sampleOffset" ){ static_cast<MultiEntity*>( obj->data )->mi0sampleOffset = sgs_GetVar<Vec3>()( C, 1 ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nchildCount = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->_ch.size(), depth ).push( C ); }
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
		{ sgs_PushString( C, "\nmi0mesh = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->sgsGetMI0Mesh(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmi0sampleOffset = " ); sgs_DumpData( C, static_cast<MultiEntity*>( obj->data )->mi0sampleOffset, depth ).push( C ); }
		sgs_StringConcat( C, 42 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst MultiEntity__sgs_funcs[] =
{
	{ "GetChild", _sgs_method__MultiEntity__GetChild },
	{ "LocalToWorld", _sgs_method__MultiEntity__LocalToWorld },
	{ "WorldToLocal", _sgs_method__MultiEntity__WorldToLocal },
	{ "LocalToWorldDir", _sgs_method__MultiEntity__LocalToWorldDir },
	{ "WorldToLocalDir", _sgs_method__MultiEntity__WorldToLocalDir },
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


static int _sgs_method__MeshResource__GetWorldMatrix( SGS_CTX )
{
	MeshResource* data; if( !SGS_PARSE_METHOD( C, MeshResource::_sgs_interface, data, MeshResource, GetWorldMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetWorldMatrix(  )); return 1;
}

static int _sgs_method__MeshResource__SetShaderConst( SGS_CTX )
{
	MeshResource* data; if( !SGS_PARSE_METHOD( C, MeshResource::_sgs_interface, data, MeshResource, SetShaderConst ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->SetShaderConst( sgs_GetVar<int>()(C,0), sgs_GetVar<Vec4>()(C,1) ); return 0;
}

int MeshResource::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<MeshResource*>( obj->data )->C = C;
	static_cast<MeshResource*>( obj->data )->~MeshResource();
	return SGS_SUCCESS;
}

int MeshResource::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MeshResource*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int MeshResource::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MeshResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->GetMatrixMode() ); return SGS_SUCCESS; }
		SGS_CASE( "meshInst" ){ sgs_PushPtr( C, static_cast<MeshResource*>( obj->data )->m_meshInst.item ); return SGS_SUCCESS; }
		SGS_CASE( "isStatic" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->m_isStatic ); return SGS_SUCCESS; }
		SGS_CASE( "visible" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->IsVisible() ); return SGS_SUCCESS; }
		SGS_CASE( "meshData" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->GetMeshData() ); return SGS_SUCCESS; }
		SGS_CASE( "mesh" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->GetMeshPath() ); return SGS_SUCCESS; }
		SGS_CASE( "lightingMode" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->GetLightingMode() ); return SGS_SUCCESS; }
		SGS_CASE( "lmQuality" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->m_lmQuality ); return SGS_SUCCESS; }
		SGS_CASE( "castLMS" ){ sgs_PushVar( C, static_cast<MeshResource*>( obj->data )->m_castLMS ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<MeshResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MeshResource::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MeshResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<MeshResource*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ static_cast<MeshResource*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ static_cast<MeshResource*>( obj->data )->SetMatrixMode( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "isStatic" ){ static_cast<MeshResource*>( obj->data )->m_isStatic = sgs_GetVar<bool>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "visible" ){ static_cast<MeshResource*>( obj->data )->SetVisible( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "meshData" ){ static_cast<MeshResource*>( obj->data )->SetMeshData( sgs_GetVar<MeshHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "mesh" ){ static_cast<MeshResource*>( obj->data )->SetMeshPath( sgs_GetVar<StringView>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "lightingMode" ){ static_cast<MeshResource*>( obj->data )->SetLightingMode( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "lmQuality" ){ static_cast<MeshResource*>( obj->data )->m_lmQuality = sgs_GetVar<float>()( C, 1 );
			static_cast<MeshResource*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
		SGS_CASE( "castLMS" ){ static_cast<MeshResource*>( obj->data )->m_castLMS = sgs_GetVar<bool>()( C, 1 );
			static_cast<MeshResource*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<MeshResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int MeshResource::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<MeshResource*>( obj->data )->C, C );
	char bfr[ 44 ];
	sprintf( bfr, "MeshResource (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalMatrix = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmatrixMode = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->GetMatrixMode(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmeshInst = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->m_meshInst.item, depth ).push( C ); }
		{ sgs_PushString( C, "\nisStatic = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->m_isStatic, depth ).push( C ); }
		{ sgs_PushString( C, "\nvisible = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->IsVisible(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmeshData = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->GetMeshData(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmesh = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->GetMeshPath(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlightingMode = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->GetLightingMode(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlmQuality = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->m_lmQuality, depth ).push( C ); }
		{ sgs_PushString( C, "\ncastLMS = " ); sgs_DumpData( C, static_cast<MeshResource*>( obj->data )->m_castLMS, depth ).push( C ); }
		sgs_StringConcat( C, 30 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst MeshResource__sgs_funcs[] =
{
	{ "GetWorldMatrix", _sgs_method__MeshResource__GetWorldMatrix },
	{ "SetShaderConst", _sgs_method__MeshResource__SetShaderConst },
	{ NULL, NULL },
};

static int MeshResource__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		MeshResource__sgs_funcs,
		-1, "MeshResource." );
	return 1;
}

static sgs_ObjInterface MeshResource__sgs_interface =
{
	"MeshResource",
	MeshResource::_sgs_destruct, MeshResource::_sgs_gcmark, MeshResource::_sgs_getindex, MeshResource::_sgs_setindex, NULL, NULL, MeshResource::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface MeshResource::_sgs_interface(MeshResource__sgs_interface, MeshResource__sgs_ifn, &GOResource::_sgs_interface);


static int _sgs_method__LightResource__GetWorldMatrix( SGS_CTX )
{
	LightResource* data; if( !SGS_PARSE_METHOD( C, LightResource::_sgs_interface, data, LightResource, GetWorldMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetWorldMatrix(  )); return 1;
}

int LightResource::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<LightResource*>( obj->data )->C = C;
	static_cast<LightResource*>( obj->data )->~LightResource();
	return SGS_SUCCESS;
}

int LightResource::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LightResource*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int LightResource::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LightResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetMatrixMode() ); return SGS_SUCCESS; }
		SGS_CASE( "isStatic" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->IsStatic() ); return SGS_SUCCESS; }
		SGS_CASE( "type" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetType() ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->IsEnabled() ); return SGS_SUCCESS; }
		SGS_CASE( "color" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetColor() ); return SGS_SUCCESS; }
		SGS_CASE( "intensity" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetIntensity() ); return SGS_SUCCESS; }
		SGS_CASE( "range" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetRange() ); return SGS_SUCCESS; }
		SGS_CASE( "power" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetPower() ); return SGS_SUCCESS; }
		SGS_CASE( "angle" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetAngle() ); return SGS_SUCCESS; }
		SGS_CASE( "aspect" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetAspect() ); return SGS_SUCCESS; }
		SGS_CASE( "hasShadows" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->HasShadows() ); return SGS_SUCCESS; }
		SGS_CASE( "cookieTextureData" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetCookieTextureData() ); return SGS_SUCCESS; }
		SGS_CASE( "cookieTexture" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->GetCookieTexturePath() ); return SGS_SUCCESS; }
		SGS_CASE( "innerAngle" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->m_innerAngle ); return SGS_SUCCESS; }
		SGS_CASE( "spotCurve" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->m_spotCurve ); return SGS_SUCCESS; }
		SGS_CASE( "lightRadius" ){ sgs_PushVar( C, static_cast<LightResource*>( obj->data )->m_lightRadius ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<LightResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int LightResource::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LightResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<LightResource*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ static_cast<LightResource*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ static_cast<LightResource*>( obj->data )->SetMatrixMode( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "isStatic" ){ static_cast<LightResource*>( obj->data )->SetStatic( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "type" ){ static_cast<LightResource*>( obj->data )->SetType( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<LightResource*>( obj->data )->SetEnabled( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "color" ){ static_cast<LightResource*>( obj->data )->SetColor( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "intensity" ){ static_cast<LightResource*>( obj->data )->SetIntensity( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "range" ){ static_cast<LightResource*>( obj->data )->SetRange( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "power" ){ static_cast<LightResource*>( obj->data )->SetPower( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "angle" ){ static_cast<LightResource*>( obj->data )->SetAngle( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "aspect" ){ static_cast<LightResource*>( obj->data )->SetAspect( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "hasShadows" ){ static_cast<LightResource*>( obj->data )->SetShadows( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "cookieTextureData" ){ static_cast<LightResource*>( obj->data )->SetCookieTextureData( sgs_GetVar<TextureHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "cookieTexture" ){ static_cast<LightResource*>( obj->data )->SetCookieTexturePath( sgs_GetVar<StringView>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "innerAngle" ){ static_cast<LightResource*>( obj->data )->m_innerAngle = sgs_GetVar<float>()( C, 1 );
			static_cast<LightResource*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
		SGS_CASE( "spotCurve" ){ static_cast<LightResource*>( obj->data )->m_spotCurve = sgs_GetVar<float>()( C, 1 );
			static_cast<LightResource*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
		SGS_CASE( "lightRadius" ){ static_cast<LightResource*>( obj->data )->m_lightRadius = sgs_GetVar<float>()( C, 1 );
			static_cast<LightResource*>( obj->data )->_UpEv(); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<LightResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int LightResource::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<LightResource*>( obj->data )->C, C );
	char bfr[ 45 ];
	sprintf( bfr, "LightResource (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalMatrix = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmatrixMode = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetMatrixMode(), depth ).push( C ); }
		{ sgs_PushString( C, "\nisStatic = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->IsStatic(), depth ).push( C ); }
		{ sgs_PushString( C, "\ntype = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetType(), depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->IsEnabled(), depth ).push( C ); }
		{ sgs_PushString( C, "\ncolor = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetColor(), depth ).push( C ); }
		{ sgs_PushString( C, "\nintensity = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetIntensity(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrange = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetRange(), depth ).push( C ); }
		{ sgs_PushString( C, "\npower = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetPower(), depth ).push( C ); }
		{ sgs_PushString( C, "\nangle = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetAngle(), depth ).push( C ); }
		{ sgs_PushString( C, "\naspect = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetAspect(), depth ).push( C ); }
		{ sgs_PushString( C, "\nhasShadows = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->HasShadows(), depth ).push( C ); }
		{ sgs_PushString( C, "\ncookieTextureData = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetCookieTextureData(), depth ).push( C ); }
		{ sgs_PushString( C, "\ncookieTexture = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->GetCookieTexturePath(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninnerAngle = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->m_innerAngle, depth ).push( C ); }
		{ sgs_PushString( C, "\nspotCurve = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->m_spotCurve, depth ).push( C ); }
		{ sgs_PushString( C, "\nlightRadius = " ); sgs_DumpData( C, static_cast<LightResource*>( obj->data )->m_lightRadius, depth ).push( C ); }
		sgs_StringConcat( C, 44 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst LightResource__sgs_funcs[] =
{
	{ "GetWorldMatrix", _sgs_method__LightResource__GetWorldMatrix },
	{ NULL, NULL },
};

static int LightResource__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		LightResource__sgs_funcs,
		-1, "LightResource." );
	return 1;
}

static sgs_ObjInterface LightResource__sgs_interface =
{
	"LightResource",
	LightResource::_sgs_destruct, LightResource::_sgs_gcmark, LightResource::_sgs_getindex, LightResource::_sgs_setindex, NULL, NULL, LightResource::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface LightResource::_sgs_interface(LightResource__sgs_interface, LightResource__sgs_ifn, &GOResource::_sgs_interface);


static int _sgs_method__ParticleSystemResource__GetWorldMatrix( SGS_CTX )
{
	ParticleSystemResource* data; if( !SGS_PARSE_METHOD( C, ParticleSystemResource::_sgs_interface, data, ParticleSystemResource, GetWorldMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetWorldMatrix(  )); return 1;
}

static int _sgs_method__ParticleSystemResource__Trigger( SGS_CTX )
{
	ParticleSystemResource* data; if( !SGS_PARSE_METHOD( C, ParticleSystemResource::_sgs_interface, data, ParticleSystemResource, Trigger ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->Trigger(  ); return 0;
}

int ParticleSystemResource::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<ParticleSystemResource*>( obj->data )->C = C;
	static_cast<ParticleSystemResource*>( obj->data )->~ParticleSystemResource();
	return SGS_SUCCESS;
}

int ParticleSystemResource::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ParticleSystemResource*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int ParticleSystemResource::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ParticleSystemResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->GetMatrixMode() ); return SGS_SUCCESS; }
		SGS_CASE( "particleSystemPath" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->m_partSysPath ); return SGS_SUCCESS; }
		SGS_CASE( "soundEvent" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->m_soundEventName ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<ParticleSystemResource*>( obj->data )->m_enabled ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<ParticleSystemResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ParticleSystemResource::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ParticleSystemResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<ParticleSystemResource*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ static_cast<ParticleSystemResource*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ static_cast<ParticleSystemResource*>( obj->data )->SetMatrixMode( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "particleSystemPath" ){ static_cast<ParticleSystemResource*>( obj->data )->sgsSetParticleSystem( sgs_GetVar<String>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "soundEvent" ){ static_cast<ParticleSystemResource*>( obj->data )->sgsSetSoundEvent( sgs_GetVar<String>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<ParticleSystemResource*>( obj->data )->sgsSetPlaying( sgs_GetVar<bool>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<ParticleSystemResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int ParticleSystemResource::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<ParticleSystemResource*>( obj->data )->C, C );
	char bfr[ 54 ];
	sprintf( bfr, "ParticleSystemResource (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalMatrix = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmatrixMode = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->GetMatrixMode(), depth ).push( C ); }
		{ sgs_PushString( C, "\nparticleSystemPath = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->m_partSysPath, depth ).push( C ); }
		{ sgs_PushString( C, "\nsoundEvent = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->m_soundEventName, depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<ParticleSystemResource*>( obj->data )->m_enabled, depth ).push( C ); }
		sgs_StringConcat( C, 20 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst ParticleSystemResource__sgs_funcs[] =
{
	{ "GetWorldMatrix", _sgs_method__ParticleSystemResource__GetWorldMatrix },
	{ "Trigger", _sgs_method__ParticleSystemResource__Trigger },
	{ NULL, NULL },
};

static int ParticleSystemResource__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		ParticleSystemResource__sgs_funcs,
		-1, "ParticleSystemResource." );
	return 1;
}

static sgs_ObjInterface ParticleSystemResource__sgs_interface =
{
	"ParticleSystemResource",
	ParticleSystemResource::_sgs_destruct, ParticleSystemResource::_sgs_gcmark, ParticleSystemResource::_sgs_getindex, ParticleSystemResource::_sgs_setindex, NULL, NULL, ParticleSystemResource::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface ParticleSystemResource::_sgs_interface(ParticleSystemResource__sgs_interface, ParticleSystemResource__sgs_ifn, &GOResource::_sgs_interface);


static int _sgs_method__RigidBodyResource__GetWorldMatrix( SGS_CTX )
{
	RigidBodyResource* data; if( !SGS_PARSE_METHOD( C, RigidBodyResource::_sgs_interface, data, RigidBodyResource, GetWorldMatrix ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	sgs_PushVar(C,data->GetWorldMatrix(  )); return 1;
}

int RigidBodyResource::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<RigidBodyResource*>( obj->data )->C = C;
	static_cast<RigidBodyResource*>( obj->data )->~RigidBodyResource();
	return SGS_SUCCESS;
}

int RigidBodyResource::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<RigidBodyResource*>( obj->data )->C, C );
	return SGS_SUCCESS;
}

int RigidBodyResource::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<RigidBodyResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "level" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->_sgs_getLevel() ); return SGS_SUCCESS; }
		SGS_CASE( "_data" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->_data ); return SGS_SUCCESS; }
		SGS_CASE( "object" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->_get_object() ); return SGS_SUCCESS; }
		SGS_CASE( "__name" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->m_name ); return SGS_SUCCESS; }
		SGS_CASE( "__type" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->m_type ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetLocalMatrix() ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetMatrixMode() ); return SGS_SUCCESS; }
		SGS_CASE( "linearVelocity" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetLinearVelocity() ); return SGS_SUCCESS; }
		SGS_CASE( "angularVelocity" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetAngularVelocity() ); return SGS_SUCCESS; }
		SGS_CASE( "friction" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetFriction() ); return SGS_SUCCESS; }
		SGS_CASE( "restitution" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetRestitution() ); return SGS_SUCCESS; }
		SGS_CASE( "mass" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetMass() ); return SGS_SUCCESS; }
		SGS_CASE( "inertia" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetInertia() ); return SGS_SUCCESS; }
		SGS_CASE( "linearDamping" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetLinearDamping() ); return SGS_SUCCESS; }
		SGS_CASE( "angularDamping" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetAngularDamping() ); return SGS_SUCCESS; }
		SGS_CASE( "kinematic" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->IsKinematic() ); return SGS_SUCCESS; }
		SGS_CASE( "canSleep" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->CanSleep() ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->IsEnabled() ); return SGS_SUCCESS; }
		SGS_CASE( "group" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetGroup() ); return SGS_SUCCESS; }
		SGS_CASE( "mask" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->GetMask() ); return SGS_SUCCESS; }
		SGS_CASE( "shapeType" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->shapeType ); return SGS_SUCCESS; }
		SGS_CASE( "shapeRadius" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->shapeRadius ); return SGS_SUCCESS; }
		SGS_CASE( "shapeHeight" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->shapeHeight ); return SGS_SUCCESS; }
		SGS_CASE( "shapeExtents" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->shapeExtents ); return SGS_SUCCESS; }
		SGS_CASE( "shapeMinExtents" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->shapeMinExtents ); return SGS_SUCCESS; }
		SGS_CASE( "shapeMesh" ){ sgs_PushVar( C, static_cast<RigidBodyResource*>( obj->data )->shapeMesh ); return SGS_SUCCESS; }
		if( sgs_PushIndex( C, static_cast<RigidBodyResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int RigidBodyResource::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<RigidBodyResource*>( obj->data )->C, C );
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "_data" ){ static_cast<RigidBodyResource*>( obj->data )->_data = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "localMatrix" ){ static_cast<RigidBodyResource*>( obj->data )->SetLocalMatrix( sgs_GetVar<Mat4>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "matrixMode" ){ static_cast<RigidBodyResource*>( obj->data )->SetMatrixMode( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "linearVelocity" ){ static_cast<RigidBodyResource*>( obj->data )->SetLinearVelocity( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "angularVelocity" ){ static_cast<RigidBodyResource*>( obj->data )->SetAngularVelocity( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "friction" ){ static_cast<RigidBodyResource*>( obj->data )->SetFriction( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "restitution" ){ static_cast<RigidBodyResource*>( obj->data )->SetRestitution( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "mass" ){ static_cast<RigidBodyResource*>( obj->data )->SetMass( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "inertia" ){ static_cast<RigidBodyResource*>( obj->data )->SetInertia( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "linearDamping" ){ static_cast<RigidBodyResource*>( obj->data )->SetLinearDamping( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "angularDamping" ){ static_cast<RigidBodyResource*>( obj->data )->SetAngularDamping( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "kinematic" ){ static_cast<RigidBodyResource*>( obj->data )->SetKinematic( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "canSleep" ){ static_cast<RigidBodyResource*>( obj->data )->SetCanSleep( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "enabled" ){ static_cast<RigidBodyResource*>( obj->data )->SetEnabled( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "group" ){ static_cast<RigidBodyResource*>( obj->data )->SetGroup( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "mask" ){ static_cast<RigidBodyResource*>( obj->data )->SetMask( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "shapeType" ){ static_cast<RigidBodyResource*>( obj->data )->SetShapeType( sgs_GetVar<int>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "shapeRadius" ){ static_cast<RigidBodyResource*>( obj->data )->SetShapeRadius( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "shapeHeight" ){ static_cast<RigidBodyResource*>( obj->data )->SetShapeHeight( sgs_GetVar<float>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "shapeExtents" ){ static_cast<RigidBodyResource*>( obj->data )->SetShapeExtents( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "shapeMinExtents" ){ static_cast<RigidBodyResource*>( obj->data )->SetShapeMinExtents( sgs_GetVar<Vec3>()( C, 1 ) ); return SGS_SUCCESS; }
		SGS_CASE( "shapeMesh" ){ static_cast<RigidBodyResource*>( obj->data )->SetShapeMesh( sgs_GetVar<MeshHandle>()( C, 1 ) ); return SGS_SUCCESS; }
		if( sgs_SetIndex( C, static_cast<RigidBodyResource*>( obj->data )->_data.var, sgs_StackItem( C, 0 ), sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) ) ) return SGS_SUCCESS;
	SGS_END_INDEXFUNC;
}

int RigidBodyResource::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	_sgsTmpChanger<sgs_Context*> _tmpchg( static_cast<RigidBodyResource*>( obj->data )->C, C );
	char bfr[ 49 ];
	sprintf( bfr, "RigidBodyResource (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nlevel = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->_sgs_getLevel(), depth ).push( C ); }
		{ sgs_PushString( C, "\n_data = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->_data, depth ).push( C ); }
		{ sgs_PushString( C, "\nobject = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->_get_object(), depth ).push( C ); }
		{ sgs_PushString( C, "\n__name = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->m_name, depth ).push( C ); }
		{ sgs_PushString( C, "\n__type = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->m_type, depth ).push( C ); }
		{ sgs_PushString( C, "\nlocalMatrix = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetLocalMatrix(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmatrixMode = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetMatrixMode(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlinearVelocity = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetLinearVelocity(), depth ).push( C ); }
		{ sgs_PushString( C, "\nangularVelocity = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetAngularVelocity(), depth ).push( C ); }
		{ sgs_PushString( C, "\nfriction = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetFriction(), depth ).push( C ); }
		{ sgs_PushString( C, "\nrestitution = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetRestitution(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmass = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetMass(), depth ).push( C ); }
		{ sgs_PushString( C, "\ninertia = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetInertia(), depth ).push( C ); }
		{ sgs_PushString( C, "\nlinearDamping = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetLinearDamping(), depth ).push( C ); }
		{ sgs_PushString( C, "\nangularDamping = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetAngularDamping(), depth ).push( C ); }
		{ sgs_PushString( C, "\nkinematic = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->IsKinematic(), depth ).push( C ); }
		{ sgs_PushString( C, "\ncanSleep = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->CanSleep(), depth ).push( C ); }
		{ sgs_PushString( C, "\nenabled = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->IsEnabled(), depth ).push( C ); }
		{ sgs_PushString( C, "\ngroup = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetGroup(), depth ).push( C ); }
		{ sgs_PushString( C, "\nmask = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->GetMask(), depth ).push( C ); }
		{ sgs_PushString( C, "\nshapeType = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->shapeType, depth ).push( C ); }
		{ sgs_PushString( C, "\nshapeRadius = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->shapeRadius, depth ).push( C ); }
		{ sgs_PushString( C, "\nshapeHeight = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->shapeHeight, depth ).push( C ); }
		{ sgs_PushString( C, "\nshapeExtents = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->shapeExtents, depth ).push( C ); }
		{ sgs_PushString( C, "\nshapeMinExtents = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->shapeMinExtents, depth ).push( C ); }
		{ sgs_PushString( C, "\nshapeMesh = " ); sgs_DumpData( C, static_cast<RigidBodyResource*>( obj->data )->shapeMesh, depth ).push( C ); }
		sgs_StringConcat( C, 52 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

static sgs_RegFuncConst RigidBodyResource__sgs_funcs[] =
{
	{ "GetWorldMatrix", _sgs_method__RigidBodyResource__GetWorldMatrix },
	{ NULL, NULL },
};

static int RigidBodyResource__sgs_ifn( SGS_CTX )
{
	sgs_CreateDict( C, NULL, 0 );
	sgs_StoreFuncConsts( C, sgs_StackItem( C, -1 ),
		RigidBodyResource__sgs_funcs,
		-1, "RigidBodyResource." );
	return 1;
}

static sgs_ObjInterface RigidBodyResource__sgs_interface =
{
	"RigidBodyResource",
	RigidBodyResource::_sgs_destruct, RigidBodyResource::_sgs_gcmark, RigidBodyResource::_sgs_getindex, RigidBodyResource::_sgs_setindex, NULL, NULL, RigidBodyResource::_sgs_dump, NULL, NULL, NULL, 
};
_sgsInterface RigidBodyResource::_sgs_interface(RigidBodyResource__sgs_interface, RigidBodyResource__sgs_ifn, &GOResource::_sgs_interface);

