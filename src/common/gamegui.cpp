

#include "gamegui.hpp"



int GUI_DefaultEventCallback( SGS_CTX )
{
	return 0;
}


GameUIControl::GameUIControl() :
	x(0), y(0), width(0), height(0), rx0(0), ry0(0), rx1(0), ry1(0)
{
}

GameUIControl* GameUIControl::Create( SGS_CTX )
{
	sgsVariable obj( C );
	GameUIControl* CTL = SGS_CREATECLASS( C, &obj.var, GameUIControl, () );
	
	CTL->eventCallback.C = C;
	CTL->eventCallback.set( GUI_DefaultEventCallback );
	
	CTL->animators.C = C;
	sgs_CreateArray( C, &CTL->animators.var, 0 );
	
	CTL->shaders.C = C;
	sgs_CreateArray( C, &CTL->shaders.var, 0 );
	
	return CTL;
}

void GameUIControl::Tick( float dt )
{
	// recalculate positions
	rx0 = parent->IX( x );
	ry0 = parent->IY( y );
	rx1 = parent->IX( x + width );
	ry1 = parent->IY( y + height );
	
	// call all animators
	SGS_SCOPE;
	sgsVariable obj = Handle( this ).get_variable();
	sgs_PushIterator( C, animators.var );
	while( sgs_IterAdvance( C, sgs_StackItem( C, -1 ) ) )
	{
		sgs_IterPushData( C, sgs_StackItem( C, -1 ), 0, 1 );
		sgsVariable animator( C, sgsVariable::PickAndPop );
		sgs_PushVar( C, dt );
		obj.thiscall( animator, 1, 0 );
	}
}

void GameUIControl::Draw()
{
	// call all shaders
	SGS_SCOPE;
	sgsVariable obj = Handle( this ).get_variable();
	sgs_PushIterator( C, shaders.var );
	while( sgs_IterAdvance( C, sgs_StackItem( C, -1 ) ) )
	{
		sgs_IterPushData( C, sgs_StackItem( C, -1 ), 0, 1 );
		sgsVariable shader( C, sgsVariable::PickAndPop );
		obj.thiscall( shader, 0, 0 );
	}
}

int GameUIControl::_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGRX_CAST( GameUIControl*, CTL, obj->data );
	SGSBOOL res = sgs_PushIndex( C, CTL->metadata.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) );
	if( res )
		return res; // found
	return _sgs_getindex( C, obj );
}

int GameUIControl::_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGRX_CAST( GameUIControl*, CTL, obj->data );
	if( _sgs_setindex( C, obj ) != SGS_SUCCESS )
	{
		sgs_SetIndex( C, CTL->metadata.var, sgs_StackItem( C, 0 ),
			sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) );
	}
	return SGS_SUCCESS;
}



GameUIScreen* GameUIScreen::Create( SGS_CTX )
{
	sgsVariable obj( C );
	GameUIScreen* SCN = SGS_CREATECLASS( C, &obj.var, GameUIScreen, () );
	return SCN;
}

void GameUIScreen::Acquire()
{
	sgs_ObjAcquire( C, m_sgsObject );
}

void GameUIScreen::Release()
{
	sgs_ObjRelease( C, m_sgsObject );
}

void GameUIScreen::OnEvent( const Event& e )
{
}

void GameUIScreen::Tick( float dt )
{
}

void GameUIScreen::Draw()
{
}

float GameUIScreen::IX( float x )
{
	x = safe_fdiv( x, width ); // to normalized coords
	return TLERP( rx0, rx1, x ); // interpolate from precalc
}

float GameUIScreen::IY( float y )
{
	y = safe_fdiv( y, height ); // to normalized coords
	return TLERP( ry0, ry1, y ); // interpolate from precalc
}

int GameUIScreen::_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGRX_CAST( GameUIScreen*, SCN, obj->data );
	SGSBOOL res = sgs_PushIndex( C, SCN->metadata.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) );
	if( res )
		return res; // found
	return _sgs_getindex( C, obj );
}

int GameUIScreen::_setindex( SGS_ARGS_SETINDEXFUNC )
{
	SGRX_CAST( GameUIScreen*, SCN, obj->data );
	if( _sgs_setindex( C, obj ) != SGS_SUCCESS )
	{
		sgs_SetIndex( C, SCN->metadata.var, sgs_StackItem( C, 0 ),
			sgs_StackItem( C, 1 ), sgs_ObjectArg( C ) );
	}
	return SGS_SUCCESS;
}

GameUIScreen::Handle GameUIScreen::CreateScreen( int mode, float width, float height,
		float xalign, float yalign, float x, float y )
{
	GameUIScreen* SCN = Create( C );
	SCN->mode = mode;
	SCN->width = width;
	SCN->height = height;
	SCN->xalign = xalign;
	SCN->yalign = yalign;
	SCN->x = x;
	SCN->y = y;
	SCN->parent = Handle( this );
	
	Handle out( SCN );
	SCN->Release();
	return out;
}

GameUIControl::Handle GameUIScreen::CreateControl(
		float x, float y, float width, float height )
{
	GameUIControl* CTL = GameUIControl::Create( C );
	CTL->x = x;
	CTL->y = y;
	CTL->width = width;
	CTL->height = height;
	CTL->parent = Handle( this );
	
	GameUIControl::Handle out( CTL );
	sgs_ObjRelease( C, CTL->m_sgsObject );
	return out;
}


