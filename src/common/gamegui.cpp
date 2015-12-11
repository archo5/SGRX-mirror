

#include "gamegui.hpp"



int GUI_DefaultEventCallback( SGS_CTX )
{
	return 0;
}


GameUIControl::GameUIControl() :
	mode(GUI_ScrMode_Abs), x(0), y(0), width(0), height(0),
	xalign(0), yalign(0), rx0(0), ry0(0), rx1(0), ry1(0), z(0),
	m_system(NULL)
{
}

GameUIControl::~GameUIControl()
{
	if( parent )
		parent->m_subitems.remove_first( this );
	m_system->_OnRemove( this );
}

GameUIControl* GameUIControl::Create( SGS_CTX )
{
//	sgsVariable obj( C ); // TODO FIX cppbc
//	GameUIControl* CTL = SGS_CREATECLASS( C, &obj.var, GameUIControl, () );
	GameUIControl* CTL = SGS_CREATECLASS( C, NULL, GameUIControl, () );
	
	CTL->eventCallback.C = C;
	CTL->eventCallback.set( GUI_DefaultEventCallback );
	
	CTL->metadata.C = C;
	sgs_CreateDict( C, &CTL->metadata.var, 0 );
	
	CTL->shaders.C = C;
	sgs_CreateArray( C, &CTL->shaders.var, 0 );
	
//	obj._acquire();
	sgs_ObjAcquire( C, CTL->m_sgsObject );
	sgs_Pop( C, 1 );
	return CTL;
}

int GameUIControl::OnEvent( const GameUIEvent& e )
{
	// call the event callback
	SGS_SCOPE;
	sgsVariable obj = Handle( this ).get_variable();
	sgs_CreateLiteClassFrom( C, NULL, &e );
	if( obj.thiscall( eventCallback, 1, 1 ) )
	{
		return sgs_GetInt( C, -1 );
	}
	return 0;
}

void GameUIControl::BubblingEvent( const GameUIEvent& e )
{
	GameUIControl* cc = this;
	while( cc )
	{
		if( !cc->OnEvent( e ) )
			break;
		cc = cc->parent;
	}
}

static int sort_ui_subitems( const void* A, const void* B )
{
	SGRX_CAST( GameUIControl*, uia, A );
	SGRX_CAST( GameUIControl*, uib, B );
	return uia->z == uib->z ? 0 : ( uia->z < uib->z ? -1 : 1 );
}

void GameUIControl::Draw( float dt )
{
	qsort( m_subitems.data(), m_subitems.size(), sizeof(GameUIControl*), sort_ui_subitems );
	
	{
		// recalculate positions
		if( parent )
		{
			if( mode == GUI_ScrMode_Fit || mode == GUI_ScrMode_Crop )
			{
				float pw = parent->rx1 - parent->rx0, ph = parent->ry1 - parent->ry0;
				float parent_aspect = safe_fdiv( pw, ph );
				float self_aspect = safe_fdiv( width, height );
				
				float tw = pw, th = ph;
				if( ( self_aspect > parent_aspect ) ^ ( mode == GUI_ScrMode_Crop ) )
				{
					// match width
					th = tw / self_aspect;
				}
				else
				{
					// match height
					tw = th * self_aspect;
				}
				float tx = ( pw - tw ) / 2;
				float ty = ( ph - th ) / 2;
				tx *= xalign + 1;
				ty *= yalign + 1;
				tx += x;
				ty += ty;
				rx0 = parent->rx0 + tx;
				ry0 = parent->ry0 + ty;
				rx1 = rx0 + tw;
				ry1 = ry0 + th;
			}
			else
			{
				rx0 = parent->IX( x );
				ry0 = parent->IY( y );
				rx1 = parent->IX( x + width );
				ry1 = parent->IY( y + height );
			}
		}
		else
		{
			rx0 = x;
			ry0 = y;
			rx1 = x + width;
			ry1 = y + height;
		}
		
		// call all shaders
		SGS_SCOPE;
		sgsVariable obj = Handle( this ).get_variable();
		sgs_PushIterator( C, shaders.var );
		while( sgs_IterAdvance( C, sgs_StackItem( C, -1 ) ) )
		{
			sgsVariable shader( C );
			sgs_IterGetData( C, sgs_StackItem( C, -1 ), NULL, &shader.var );
			sgs_PushVar( C, obj );
			sgs_PushVar( C, dt );
			GR2D_GetBatchRenderer().Reset();
			shader.thiscall( shader, 2, 0 );
		}
	}
	
	for( size_t i = 0; i < m_subitems.size(); ++i )
	{
		m_subitems[ i ]->Draw( dt );
	}
}

bool GameUIControl::Hit( int x, int y )
{
	return rx0 <= x && x < rx1 && ry0 <= y && y < ry1;
}

float GameUIControl::IX( float x )
{
	x = safe_fdiv( x, width ); // to normalized coords
	return TLERP( rx0, rx1, x ); // interpolate from precalc
}

float GameUIControl::IS( float s )
{
	return IX( s ) - IX( 0 );
}

float GameUIControl::IY( float y )
{
	y = safe_fdiv( y, height ); // to normalized coords
	return TLERP( ry0, ry1, y ); // interpolate from precalc
}

int GameUIControl::_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGRX_CAST( GameUIControl*, CTL, obj->data );
	SGSBOOL res = sgs_PushIndex( C, CTL->metadata.var, sgs_StackItem( C, 0 ), sgs_ObjectArg( C ) );
	if( res )
		return SGS_SUCCESS; // found
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

GameUIControl::Handle GameUIControl::CreateScreen( int mode, float width, float height,
		float xalign, float yalign, float x, float y )
{
	GameUIControl* CTL = Create( C );
	CTL->mode = mode;
	CTL->width = width;
	CTL->height = height;
	CTL->xalign = xalign;
	CTL->yalign = yalign;
	CTL->x = x;
	CTL->y = y;
	CTL->parent = Handle( this );
	CTL->m_system = m_system;
	m_subitems.push_back( CTL );
	
	Handle out( CTL );
	sgs_ObjRelease( C, CTL->m_sgsObject );
	return out;
}

GameUIControl::Handle GameUIControl::CreateControl(
		float x, float y, float width, float height )
{
	GameUIControl* CTL = GameUIControl::Create( C );
	CTL->mode = GUI_ScrMode_Abs;
	CTL->x = x;
	CTL->y = y;
	CTL->width = width;
	CTL->height = height;
	CTL->parent = Handle( this );
	CTL->m_system = m_system;
	m_subitems.push_back( CTL );
	
	GameUIControl::Handle out( CTL );
	sgs_ObjRelease( C, CTL->m_sgsObject );
	return out;
}

void GameUIControl::DReset()
{
	GR2D_GetBatchRenderer().Reset();
}

void GameUIControl::DCol( sgs_Context* ctx, float a, float b, float c, float d )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	int ssz = sgs_StackSize( ctx );
	if( ssz <= 1 )
		br.Col( a );
	else if( ssz == 2 )
		br.Col( a, b );
	else if( ssz == 3 )
		br.Col( a, b, c );
	else
		br.Col( a, b, c, d );
}

void GameUIControl::DTex( StringView name )
{
	GR2D_GetBatchRenderer().SetTexture( GR_GetTexture( name ) );
}

void GameUIControl::DQuad( float x0, float y0, float x1, float y1 )
{
	GR2D_GetBatchRenderer().Quad( IX( x0 ), IY( y0 ), IX( x1 ), IY( y1 ) );
}

void GameUIControl::DQuadExt( sgs_Context* ctx, float x0, float y0, float x1, float y1,
	float tox, float toy, float tsx /* = 1 */, float tsy /* = 1 */ )
{
	int ssz = sgs_StackSize( ctx );
	GR2D_GetBatchRenderer().QuadExt( IX( x0 ), IY( y0 ), IX( x1 ), IY( y1 ),
		tox, toy, ssz >= 7 ? tsx : 1.0f, ssz >= 8 ? tsy : 1.0f );
}

void GameUIControl::DButton( float x0, float y0, float x1, float y1, Vec4 bdr, Vec4 texbdr )
{
	GR2D_GetBatchRenderer().Button(
		V4( IX( x0 ), IY( y0 ), IX( x1 ), IY( y1 ) ),
		bdr, texbdr );
}

void GameUIControl::DFont( StringView name, float size )
{
	GR2D_SetFont( name, IS( size ) );
}

void GameUIControl::DText( sgs_Context* ctx, StringView text, float x, float y, int ha, int va )
{
	if( sgs_StackSize( ctx ) <= 1 )
		GR2D_DrawTextLine( text );
	else
		GR2D_DrawTextLine( IX( x ), IY( y ), text, ha, va );
}



static int TEXTURE( SGS_CTX )
{
	char* bfr;
	sgs_SizeVal size;
	SGSFN( "TEXTURE" );
	if( !sgs_LoadArgs( C, "m", &bfr, &size ) )
		return 0;
	
	sgs_PushIndex( C, sgs_Registry( C, SGS_REG_ROOT ), sgs_MakeNull(), 0 );
	SGRX_CAST( GameUISystem*, sys, sgs_GetPtr( C, -1 ) );
	sgs_Pop( C, 1 );
	
	sys->PrecacheTexture( StringView( bfr, size ) );
	sgs_SetStackSize( C, 1 );
	return 1; // return the string itself
}

sgs_RegIntConst sgs_iconsts[] =
{
	{ "HALIGN_LEFT", HALIGN_LEFT },
	{ "HALIGN_CENTER", HALIGN_CENTER },
	{ "HALIGN_RIGHT", HALIGN_RIGHT },
	{ "VALIGN_TOP", VALIGN_TOP },
	{ "VALIGN_CENTER", VALIGN_CENTER },
	{ "VALIGN_BOTTOM", VALIGN_BOTTOM },
	
	{ "GUI_ScrMode_Abs", GUI_ScrMode_Abs },
	{ "GUI_ScrMode_Fit", GUI_ScrMode_Fit },
	{ "GUI_ScrMode_Crop", GUI_ScrMode_Crop },
};

sgs_RegFuncConst sgs_funcs[] =
{
	{ "TEXTURE", TEXTURE },
};

GameUISystem::GameUISystem() :
	m_rootCtrl(NULL), m_hoverCtrl(NULL), m_kbdFocusCtrl(NULL),
	m_mouseX(0), m_mouseY(0)
{
	SGS_CTX = m_scriptCtx.C;
	m_rootCtrl = GameUIControl::Create( C );
	m_rootCtrl->m_system = this;
	m_clickCtrl[0] = NULL;
	m_clickCtrl[1] = NULL;
	
	sgs_SetIndex( C, sgs_Registry( C, SGS_REG_ROOT ), sgs_MakeNull(), sgs_MakePtr( this ), 0 );
	sgs_RegIntConsts( C, sgs_iconsts, SGRX_ARRAY_SIZE(sgs_iconsts) );
	sgs_RegFuncConsts( C, sgs_funcs, SGRX_ARRAY_SIZE(sgs_funcs) );
	m_scriptCtx.SetGlobal( "ROOT", m_rootCtrl->GetHandle() );
}

GameUISystem::~GameUISystem()
{
	sgs_ObjRelease( m_scriptCtx.C, m_rootCtrl->m_sgsObject );
	m_rootCtrl = NULL;
}

void GameUISystem::Load( const StringView& sv )
{
	m_scriptCtx.ExecFile( sv );
}

void GameUISystem::EngineEvent( const Event& eev )
{
	if( eev.type == SDL_MOUSEMOTION )
	{
		m_mouseX = eev.motion.x;
		m_mouseY = eev.motion.y;
		if( m_hoverCtrl )
		{
			GameUIEvent ev = { GUI_Event_MouseMove, m_hoverCtrl->GetHandle() };
			ev.mouse.x = m_mouseX;
			ev.mouse.y = m_mouseY;
			m_hoverCtrl->OnEvent( ev );
		}
		_HandleMouseMove( true );
	}
	else if( eev.type == SDL_MOUSEWHEEL )
	{
		if( m_hoverCtrl )
		{
			GameUIEvent ev = { GUI_Event_MouseWheel, m_hoverCtrl->GetHandle() };
			ev.mouse.x = eev.wheel.x;
			ev.mouse.y = eev.wheel.y;
			m_hoverCtrl->BubblingEvent( ev );
		}
	}
	else if( eev.type == SDL_MOUSEBUTTONUP || eev.type == SDL_MOUSEBUTTONDOWN )
	{
		int btn = eev.button.button;
		if( btn == SGRX_MB_LEFT ) btn = GUI_MB_Left;
		else if( btn == SGRX_MB_RIGHT ) btn = GUI_MB_Right;
		else
			btn = -1;
		if( btn >= 0 )
		{
			bool down = eev.type == SDL_MOUSEBUTTONDOWN;
			GameUIEvent ev = { down ? GUI_Event_BtnDown : GUI_Event_BtnUp };
			ev.mouse.x = m_mouseX;
			ev.mouse.y = m_mouseY;
			ev.mouse.button = btn;
			
			if( !down )
			{
				if( m_clickCtrl[ btn ] )
				{
					m_clickCtrl[ btn ]->OnEvent( ev );
					m_clickCtrl[ btn ] = NULL;
				}
				_HandleMouseMove( true );
			}
			else if( m_hoverCtrl )
			{
				m_clickCtrl[ btn ] = m_hoverCtrl;
				m_hoverCtrl->OnEvent( ev );
			}
		}
	}
	else if( eev.type == SDL_KEYDOWN || eev.type == SDL_KEYUP )
	{
		if( !m_kbdFocusCtrl )
			return;
		
		bool down = eev.type == SDL_KEYDOWN;
		int engkey = eev.key.keysym.sym;
		int engmod = eev.key.keysym.mod;
		
		GameUIEvent ev =
		{
			down ? GUI_Event_KeyDown : GUI_Event_KeyUp,
			m_kbdFocusCtrl->GetHandle()
		};
		ev.key.key = GUI_Key_Unknown;
		ev.key.engkey = engkey;
		ev.key.engmod = engmod;
		ev.key.repeat = !!eev.key.repeat;
		
		if(0);
		else if( engkey == SDLK_RETURN ) ev.key.key = GUI_Key_Enter;
		else if( engkey == SDLK_KP_ENTER ) ev.key.key = GUI_Key_Enter;
		else if( engkey == SDLK_ESCAPE ) ev.key.key = GUI_Key_Escape;
		else if( engkey == SDLK_BACKSPACE ) ev.key.key = GUI_Key_DelLeft;
		else if( engkey == SDLK_DELETE ) ev.key.key = GUI_Key_DelRight;
		else if( engkey == SDLK_LEFT ) ev.key.key = GUI_Key_Left;
		else if( engkey == SDLK_RIGHT ) ev.key.key = GUI_Key_Right;
		else if( engkey == SDLK_a && engmod & KMOD_CTRL ){ ev.key.key = GUI_Key_SelectAll; }
		else if( engkey == SDLK_x && engmod & KMOD_CTRL ){ ev.key.key = GUI_Key_Cut; }
		else if( engkey == SDLK_c && engmod & KMOD_CTRL ){ ev.key.key = GUI_Key_Copy; }
		else if( engkey == SDLK_v && engmod & KMOD_CTRL ){ ev.key.key = GUI_Key_Paste; }
		
		if( engmod & KMOD_SHIFT )
			ev.key.key |= GUI_KeyMod_Shift;
		
		m_kbdFocusCtrl->OnEvent( ev );
	}
	else if( eev.type == SDL_TEXTINPUT )
	{
		if( !m_kbdFocusCtrl )
			return;
		
		GameUIEvent ev = { GUI_Event_TextInput, m_kbdFocusCtrl->GetHandle() };
		sgrx_sncopy( ev.text.text, 8, eev.text.text );
		ev.text.text[7] = 0;
		
		m_kbdFocusCtrl->OnEvent( ev );
	}
}

void GameUISystem::_HandleMouseMove( bool optional )
{
	if( optional )
	{
		for( int i = 0; i < 3; ++i )
			if( m_clickCtrl[ i ] )
				return;
	}
	
	GameUIControl* prevhover = m_hoverCtrl;
	m_hoverCtrl = _GetItemAtPosition( m_mouseX, m_mouseY );
	
	if( m_hoverCtrl != prevhover )
	{
		if( prevhover || m_hoverCtrl )
		{
			// try to find common parent
			GameUIControl* phi = prevhover, *pphi = NULL;
			size_t i;
			while( phi )
			{
				for( i = 0; i < m_hoverTrail.size(); ++i )
					if( m_hoverTrail[ i ] == phi )
						break;
				if( i < m_hoverTrail.size() )
					break;
				pphi = phi;
				phi = phi->parent;
			}
			
		//	printf("ht0=%p htsz=%d pphi=%p phi=%p prev=%p curr=%p\n", m_hoverTrail.size()>=2 ? m_hoverTrail[1] : NULL, (int) m_hoverTrail.size(), pphi, phi, prevhover, m_hover );
			if( !phi )
			{
				// no common parent, run bubbling event on previous hover, bubbling event on current hover, update all styles
				if( pphi )
				{
					GameUIEvent e = { GUI_Event_MouseLeave, pphi->GetHandle() };
					e.mouse.x = m_mouseX;
					e.mouse.y = m_mouseY;
					pphi->BubblingEvent( e );
				}
				if( m_hoverCtrl )
				{
					GameUIEvent e = { GUI_Event_MouseEnter, m_hoverCtrl->GetHandle() };
					e.mouse.x = m_mouseX;
					e.mouse.y = m_mouseY;
					m_hoverCtrl->BubblingEvent( e );
				}
			}
			else
			{
				GameUIEvent e;
				// found common parent, run events through that (parent gets none), update styles up from parent (not parent itself)
				GameUIControl *cc;
				
				e.type = GUI_Event_MouseLeave;
				e.target = prevhover->GetHandle();
				e.mouse.x = m_mouseX;
				e.mouse.y = m_mouseY;
				
				cc = prevhover;
				while( cc && cc != phi )
				{
					if( !cc->OnEvent( e ) )
						break;
					cc = cc->parent;
				}
				
				e.type = GUI_Event_MouseEnter;
				e.target = m_hoverCtrl->GetHandle();
				e.mouse.x = m_mouseX;
				e.mouse.y = m_mouseY;
				
				cc = m_hoverCtrl;
				while( cc && cc != phi )
				{
					if( !cc->OnEvent( e ) )
						break;
					cc = cc->parent;
				}
			}
		}
	}
}

GameUIControl* GameUISystem::_GetItemAtPosition( int x, int y )
{
	m_hoverTrail.clear();
	m_hoverTrail.push_back( m_rootCtrl );
	GameUIControl* item = m_rootCtrl, *atpos = NULL;
	while( item && item != atpos )
	{
		atpos = item;
		for( size_t i = item->m_subitems.size(); i > 0; )
		{
			--i;
			if( item->m_subitems[ i ]->Hit( x, y ) )
			{
				item = item->m_subitems[ i ];
				m_hoverTrail.push_back( item );
				break;
			}
		}
	}
	return atpos;
}

void GameUISystem::Draw( float dt )
{
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	m_rootCtrl->x = 0;
	m_rootCtrl->y = 0;
	m_rootCtrl->width = GR_GetWidth();
	m_rootCtrl->height = GR_GetHeight();
	m_rootCtrl->Draw( dt );
	
	GR2D_GetBatchRenderer().Flush();
}

void GameUISystem::_OnRemove( GameUIControl* ctrl )
{
	m_hoverTrail.remove_all( ctrl );
	if( m_hoverCtrl == ctrl )
		m_hoverCtrl = NULL;
	if( m_kbdFocusCtrl == ctrl )
		m_kbdFocusCtrl = NULL;
	if( m_clickCtrl[0] == ctrl )
		m_clickCtrl[0] = NULL;
	if( m_clickCtrl[1] == ctrl )
		m_clickCtrl[1] = NULL;
}

void GameUISystem::PrecacheTexture( const StringView& texname )
{
	m_precachedTextures.push_back( GR_GetTexture( texname ) );
}


