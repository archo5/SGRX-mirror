// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "gamegui.hpp"

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
		sgs_StringConcat( C, 18 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface GameUIEvent::_sgs_interface[1] =
{{
	"GameUIEvent",
	GameUIEvent::_sgs_destruct, GameUIEvent::_sgs_gcmark, GameUIEvent::_sgs_getindex, GameUIEvent::_sgs_setindex, NULL, NULL, GameUIEvent::_sgs_dump, NULL, NULL, NULL, 
}};


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

static int _sgs_method__GameUIControl__DButton( SGS_CTX )
{
	GameUIControl* data; if( !SGS_PARSE_METHOD( C, GameUIControl::_sgs_interface, data, GameUIControl, DButton ) ) return 0;
	_sgsTmpChanger<sgs_Context*> _tmpchg( data->C, C );
	data->DButton( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<Vec4>()(C,4), sgs_GetVar<Vec4>()(C,5) ); return 0;
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
		SGS_CASE( "rx0" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->rx0 ); return SGS_SUCCESS; }
		SGS_CASE( "ry0" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->ry0 ); return SGS_SUCCESS; }
		SGS_CASE( "rx1" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->rx1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry1" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->ry1 ); return SGS_SUCCESS; }
		SGS_CASE( "z" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->z ); return SGS_SUCCESS; }
		SGS_CASE( "metadata" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->metadata ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->parent ); return SGS_SUCCESS; }
		SGS_CASE( "eventCallback" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->eventCallback ); return SGS_SUCCESS; }
		SGS_CASE( "shaders" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->shaders ); return SGS_SUCCESS; }
		SGS_CASE( "hover" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->hover ); return SGS_SUCCESS; }
		SGS_CASE( "focusable" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->focusable ); return SGS_SUCCESS; }
		SGS_CASE( "IsIn" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__IsIn ); return SGS_SUCCESS; }
		SGS_CASE( "visible" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getVisible() ); return SGS_SUCCESS; }
		SGS_CASE( "focused" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getFocused() ); return SGS_SUCCESS; }
		SGS_CASE( "clicked" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getClicked() ); return SGS_SUCCESS; }
		SGS_CASE( "clickedL" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getClickedL() ); return SGS_SUCCESS; }
		SGS_CASE( "clickedR" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->_getClickedR() ); return SGS_SUCCESS; }
		SGS_CASE( "AddCallback" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__AddCallback ); return SGS_SUCCESS; }
		SGS_CASE( "RemoveCallback" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__RemoveCallback ); return SGS_SUCCESS; }
		SGS_CASE( "InvokeCallbacks" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__InvokeCallbacks ); return SGS_SUCCESS; }
		SGS_CASE( "Hit" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__Hit ); return SGS_SUCCESS; }
		SGS_CASE( "IX" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__IX ); return SGS_SUCCESS; }
		SGS_CASE( "IY" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__IY ); return SGS_SUCCESS; }
		SGS_CASE( "IS" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__IS ); return SGS_SUCCESS; }
		SGS_CASE( "InvIX" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__InvIX ); return SGS_SUCCESS; }
		SGS_CASE( "InvIY" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__InvIY ); return SGS_SUCCESS; }
		SGS_CASE( "InvIS" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__InvIS ); return SGS_SUCCESS; }
		SGS_CASE( "CreateScreen" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__CreateScreen ); return SGS_SUCCESS; }
		SGS_CASE( "CreateControl" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__CreateControl ); return SGS_SUCCESS; }
		SGS_CASE( "DReset" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__DReset ); return SGS_SUCCESS; }
		SGS_CASE( "DCol" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__DCol ); return SGS_SUCCESS; }
		SGS_CASE( "DTex" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__DTex ); return SGS_SUCCESS; }
		SGS_CASE( "DQuad" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__DQuad ); return SGS_SUCCESS; }
		SGS_CASE( "DQuadExt" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__DQuadExt ); return SGS_SUCCESS; }
		SGS_CASE( "DButton" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__DButton ); return SGS_SUCCESS; }
		SGS_CASE( "DFont" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__DFont ); return SGS_SUCCESS; }
		SGS_CASE( "DText" ){ sgs_PushCFunc( C, _sgs_method__GameUIControl__DText ); return SGS_SUCCESS; }
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
		{ sgs_PushString( C, "\nrx0 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->rx0, depth ).push( C ); }
		{ sgs_PushString( C, "\nry0 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->ry0, depth ).push( C ); }
		{ sgs_PushString( C, "\nrx1 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->rx1, depth ).push( C ); }
		{ sgs_PushString( C, "\nry1 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->ry1, depth ).push( C ); }
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
		sgs_StringConcat( C, 58 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface GameUIControl::_sgs_interface[1] =
{{
	"GameUIControl",
	GameUIControl::_sgs_destruct, GameUIControl::_sgs_gcmark, GameUIControl::_getindex, GameUIControl::_setindex, NULL, NULL, GameUIControl::_sgs_dump, NULL, NULL, NULL, 
}};

