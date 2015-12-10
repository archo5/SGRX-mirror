// SGS/CPP-BC
// warning: do not modify this file, it may be regenerated during any build

#include "gamegui.hpp"

int GameUIControl::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GameUIControl*>( obj->data )->~GameUIControl();
	return SGS_SUCCESS;
}

int GameUIControl::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int GameUIControl::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "x" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->x ); return SGS_SUCCESS; }
		SGS_CASE( "y" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->y ); return SGS_SUCCESS; }
		SGS_CASE( "width" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->width ); return SGS_SUCCESS; }
		SGS_CASE( "height" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->height ); return SGS_SUCCESS; }
		SGS_CASE( "rx0" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->rx0 ); return SGS_SUCCESS; }
		SGS_CASE( "ry0" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->ry0 ); return SGS_SUCCESS; }
		SGS_CASE( "rx1" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->rx1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry1" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->ry1 ); return SGS_SUCCESS; }
		SGS_CASE( "metadata" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->metadata ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->parent ); return SGS_SUCCESS; }
		SGS_CASE( "eventCallback" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->eventCallback ); return SGS_SUCCESS; }
		SGS_CASE( "animators" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->animators ); return SGS_SUCCESS; }
		SGS_CASE( "shaders" ){ sgs_PushVar( C, static_cast<GameUIControl*>( obj->data )->shaders ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameUIControl::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "x" ){ static_cast<GameUIControl*>( obj->data )->x = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "y" ){ static_cast<GameUIControl*>( obj->data )->y = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "width" ){ static_cast<GameUIControl*>( obj->data )->width = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "height" ){ static_cast<GameUIControl*>( obj->data )->height = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rx0" ){ static_cast<GameUIControl*>( obj->data )->rx0 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry0" ){ static_cast<GameUIControl*>( obj->data )->ry0 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rx1" ){ static_cast<GameUIControl*>( obj->data )->rx1 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry1" ){ static_cast<GameUIControl*>( obj->data )->ry1 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "metadata" ){ static_cast<GameUIControl*>( obj->data )->metadata = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "eventCallback" ){ static_cast<GameUIControl*>( obj->data )->eventCallback = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameUIControl::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 45 ];
	sprintf( bfr, "GameUIControl (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nx = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->x, depth ).push( C ); }
		{ sgs_PushString( C, "\ny = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->y, depth ).push( C ); }
		{ sgs_PushString( C, "\nwidth = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->width, depth ).push( C ); }
		{ sgs_PushString( C, "\nheight = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->height, depth ).push( C ); }
		{ sgs_PushString( C, "\nrx0 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->rx0, depth ).push( C ); }
		{ sgs_PushString( C, "\nry0 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->ry0, depth ).push( C ); }
		{ sgs_PushString( C, "\nrx1 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->rx1, depth ).push( C ); }
		{ sgs_PushString( C, "\nry1 = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->ry1, depth ).push( C ); }
		{ sgs_PushString( C, "\nmetadata = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->metadata, depth ).push( C ); }
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->parent, depth ).push( C ); }
		{ sgs_PushString( C, "\neventCallback = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->eventCallback, depth ).push( C ); }
		{ sgs_PushString( C, "\nanimators = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->animators, depth ).push( C ); }
		{ sgs_PushString( C, "\nshaders = " ); sgs_DumpData( C, static_cast<GameUIControl*>( obj->data )->shaders, depth ).push( C ); }
		sgs_StringConcat( C, 26 );
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


static int _sgs_method__GameUIScreen__CreateScreen( SGS_CTX )
{
	GameUIScreen* data; if( !SGS_PARSE_METHOD( C, GameUIScreen::_sgs_interface, data, GameUIScreen, CreateScreen ) ) return 0;
	sgs_PushVar(C,data->CreateScreen( sgs_GetVar<int>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3), sgs_GetVar<float>()(C,4), sgs_GetVar<float>()(C,5), sgs_GetVar<float>()(C,6) )); return 1;
}

static int _sgs_method__GameUIScreen__CreateControl( SGS_CTX )
{
	GameUIScreen* data; if( !SGS_PARSE_METHOD( C, GameUIScreen::_sgs_interface, data, GameUIScreen, CreateControl ) ) return 0;
	sgs_PushVar(C,data->CreateControl( sgs_GetVar<float>()(C,0), sgs_GetVar<float>()(C,1), sgs_GetVar<float>()(C,2), sgs_GetVar<float>()(C,3) )); return 1;
}

int GameUIScreen::_sgs_destruct( SGS_CTX, sgs_VarObj* obj )
{
	static_cast<GameUIScreen*>( obj->data )->~GameUIScreen();
	return SGS_SUCCESS;
}

int GameUIScreen::_sgs_gcmark( SGS_CTX, sgs_VarObj* obj )
{
	return SGS_SUCCESS;
}

int GameUIScreen::_sgs_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "mode" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->mode ); return SGS_SUCCESS; }
		SGS_CASE( "x" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->x ); return SGS_SUCCESS; }
		SGS_CASE( "y" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->y ); return SGS_SUCCESS; }
		SGS_CASE( "width" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->width ); return SGS_SUCCESS; }
		SGS_CASE( "height" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->height ); return SGS_SUCCESS; }
		SGS_CASE( "xalign" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->xalign ); return SGS_SUCCESS; }
		SGS_CASE( "yalign" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->yalign ); return SGS_SUCCESS; }
		SGS_CASE( "rx0" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->rx0 ); return SGS_SUCCESS; }
		SGS_CASE( "ry0" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->ry0 ); return SGS_SUCCESS; }
		SGS_CASE( "rx1" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->rx1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry1" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->ry1 ); return SGS_SUCCESS; }
		SGS_CASE( "metadata" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->metadata ); return SGS_SUCCESS; }
		SGS_CASE( "parent" ){ sgs_PushVar( C, static_cast<GameUIScreen*>( obj->data )->parent ); return SGS_SUCCESS; }
		SGS_CASE( "CreateScreen" ){ sgs_PushCFunc( C, _sgs_method__GameUIScreen__CreateScreen ); return SGS_SUCCESS; }
		SGS_CASE( "CreateControl" ){ sgs_PushCFunc( C, _sgs_method__GameUIScreen__CreateControl ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameUIScreen::_sgs_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "mode" ){ static_cast<GameUIScreen*>( obj->data )->mode = sgs_GetVar<int>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "x" ){ static_cast<GameUIScreen*>( obj->data )->x = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "y" ){ static_cast<GameUIScreen*>( obj->data )->y = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "width" ){ static_cast<GameUIScreen*>( obj->data )->width = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "height" ){ static_cast<GameUIScreen*>( obj->data )->height = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "xalign" ){ static_cast<GameUIScreen*>( obj->data )->xalign = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "yalign" ){ static_cast<GameUIScreen*>( obj->data )->yalign = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rx0" ){ static_cast<GameUIScreen*>( obj->data )->rx0 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry0" ){ static_cast<GameUIScreen*>( obj->data )->ry0 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "rx1" ){ static_cast<GameUIScreen*>( obj->data )->rx1 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "ry1" ){ static_cast<GameUIScreen*>( obj->data )->ry1 = sgs_GetVar<float>()( C, 1 ); return SGS_SUCCESS; }
		SGS_CASE( "metadata" ){ static_cast<GameUIScreen*>( obj->data )->metadata = sgs_GetVar<sgsVariable>()( C, 1 ); return SGS_SUCCESS; }
	SGS_END_INDEXFUNC;
}

int GameUIScreen::_sgs_dump( SGS_CTX, sgs_VarObj* obj, int depth )
{
	char bfr[ 44 ];
	sprintf( bfr, "GameUIScreen (%p) %s", obj->data, depth > 0 ? "\n{" : " ..." );
	sgs_PushString( C, bfr );
	if( depth > 0 )
	{
		{ sgs_PushString( C, "\nmode = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->mode, depth ).push( C ); }
		{ sgs_PushString( C, "\nx = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->x, depth ).push( C ); }
		{ sgs_PushString( C, "\ny = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->y, depth ).push( C ); }
		{ sgs_PushString( C, "\nwidth = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->width, depth ).push( C ); }
		{ sgs_PushString( C, "\nheight = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->height, depth ).push( C ); }
		{ sgs_PushString( C, "\nxalign = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->xalign, depth ).push( C ); }
		{ sgs_PushString( C, "\nyalign = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->yalign, depth ).push( C ); }
		{ sgs_PushString( C, "\nrx0 = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->rx0, depth ).push( C ); }
		{ sgs_PushString( C, "\nry0 = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->ry0, depth ).push( C ); }
		{ sgs_PushString( C, "\nrx1 = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->rx1, depth ).push( C ); }
		{ sgs_PushString( C, "\nry1 = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->ry1, depth ).push( C ); }
		{ sgs_PushString( C, "\nmetadata = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->metadata, depth ).push( C ); }
		{ sgs_PushString( C, "\nparent = " ); sgs_DumpData( C, static_cast<GameUIScreen*>( obj->data )->parent, depth ).push( C ); }
		sgs_StringConcat( C, 26 );
		sgs_PadString( C );
		sgs_PushString( C, "\n}" );
		sgs_StringConcat( C, 3 );
	}
	return SGS_SUCCESS;
}

sgs_ObjInterface GameUIScreen::_sgs_interface[1] =
{{
	"GameUIScreen",
	GameUIScreen::_sgs_destruct, GameUIScreen::_sgs_gcmark, GameUIScreen::_getindex, GameUIScreen::_setindex, NULL, NULL, GameUIScreen::_sgs_dump, NULL, NULL, NULL, 
}};

