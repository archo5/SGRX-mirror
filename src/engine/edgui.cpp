

#define USE_VEC2
#define USE_VEC3
#define USE_ARRAY
#include "edgui.hpp"



EDGUIItem::EDGUIItem() :
	tyname( "item" ),
	type( EDGUI_ITEM_NULL ),
	id1( 0 ),
	id2( 0 ),
	backColor( EDGUI_THEME_MAIN_BACK_COLOR ),
	textColor( EDGUI_THEME_MAIN_TEXT_COLOR ),
	m_parent( NULL ),
	m_frame( NULL ),
	x0( 0 ), y0( 0 ), x1( 0 ), y1( 0 ),
	m_mouseOn( false ),
	m_clicked( false )
{
}

EDGUIItem::~EDGUIItem()
{
	if( m_parent )
		m_parent->Remove( this );
	Clear();
}

int EDGUIItem::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PAINT:
		if( backColor )
		{
			GR2D_GetBatchRenderer().UnsetTexture().Colu( backColor ).Quad( float(x0), float(y0), float(x1), float(y1) );
		}
		if( textColor && caption.size() )
		{
			GR2D_GetBatchRenderer().Colu( textColor );
			GR2D_DrawTextLine( round(( x0 + x1 ) / 2.0f), round(( y0 + y1 ) / 2.0f), caption, HALIGN_CENTER, VALIGN_CENTER );
		}
		for( size_t i = 0; i < m_subitems.size(); ++i )
		{
			m_subitems[ i ]->OnEvent( e );
		}
		return 1;
		
	case EDGUI_EVENT_PRELAYOUT:
		OnChangeLayout();
		return 0;
	case EDGUI_EVENT_LAYOUT:
		SetRectFromEvent( e, true );
		return 1;
		
	case EDGUI_EVENT_SETFOCUS:
	case EDGUI_EVENT_LOSEFOCUS:
		return 0;
		
	case EDGUI_EVENT_HITTEST:
		return e->mouse.x >= x0 && e->mouse.x < x1 && e->mouse.y >= y0 && e->mouse.y < y1;
		
	case EDGUI_EVENT_MOUSEENTER: m_mouseOn = true; Invalidate(); return 1;
	case EDGUI_EVENT_MOUSELEAVE: m_mouseOn = false; Invalidate(); return 1;
	case EDGUI_EVENT_BTNDOWN:
		if( e->mouse.button == 0 )
			m_clicked = true;
		if( m_frame->m_keyboardFocus != this )
		{
			EDGUIEvent se = { EDGUI_EVENT_SETFOCUS, this };
			OnEvent( &se );
		}
		Invalidate();
		return 1;
	case EDGUI_EVENT_BTNUP:
		if( e->mouse.button == 0 ) m_clicked = false;
		if( m_mouseOn )
		{
			EDGUIEvent se = *e;
			se.type = EDGUI_EVENT_BTNCLICK;
			se.target = this;
			BubblingEvent( &se );
		}
		Invalidate();
		return 1;
	}
	
	return 1;
}

bool EDGUIItem::Add( EDGUIItem* subitem )
{
	if( m_subitems.find_first_at( subitem ) != NOT_FOUND )
		return false;
	
	if( subitem->m_parent )
	{
		if( !subitem->m_parent->Remove( subitem ) )
			return false;
	}
	
	m_subitems.push_back( subitem );
	subitem->m_parent = this;
	subitem->_SetFrame( m_frame );
	
	EDGUIEvent ev = { EDGUI_EVENT_ADDED, subitem };
	subitem->OnEvent( &ev );
	
	ReshapeLayout();
	return true;
}

bool EDGUIItem::Remove( EDGUIItem* subitem )
{
	size_t pos = m_subitems.find_first_at( subitem );
	if( this != subitem->m_parent || pos == NOT_FOUND )
		return false;
	m_subitems.erase( pos );
	subitem->m_parent = NULL;
	subitem->_SetFrame( NULL );
	
	ReshapeLayout();
	return true;
}

void EDGUIItem::Clear()
{
	while( m_subitems.size() )
		Remove( m_subitems.last() );
}

void EDGUIItem::SubstChildPtr( const EDGUIItem* find, EDGUIItem* repl )
{
	for( size_t i = 0; i < m_subitems.size(); ++i )
		if( m_subitems[ i ] == find )
			m_subitems[ i ] = repl;
	repl->m_parent = this;
}

bool EDGUIItem::Hit( int x, int y )
{
	EDGUIEvent e = { EDGUI_EVENT_HITTEST };
	e.mouse.x = x;
	e.mouse.y = y;
	return !!OnEvent( &e );
}

void EDGUIItem::BubblingEvent( EDGUIEvent* e )
{
	EDGUIItem* cc = this;
	while( cc )
	{
		if( !cc->OnEvent( e ) )
			break;
		cc = cc->m_parent;
	}
}

void EDGUIItem::SetRectFromEvent( EDGUIEvent* e, bool updatesub )
{
	Invalidate();
	x0 = e->layout.x0;
	y0 = e->layout.y0;
	x1 = e->layout.x1;
	y1 = e->layout.y1;
	Invalidate();
	if( updatesub )
	{
		for( size_t i = 0; i < m_subitems.size(); ++i )
		{
			m_subitems[ i ]->OnEvent( e );
		}
	}
}

void EDGUIItem::OnChangeLayout()
{
	EDGUIEvent e = { EDGUI_EVENT_LAYOUT, this };
	e.layout.x0 = x0;
	e.layout.y0 = y0;
	e.layout.x1 = x1;
	e.layout.y1 = y1;
//	LOG << "OC LAYOUT: " << tyname << LOG_SEP(", ") << x0 << y0 << x1 << y1;
	OnEvent( &e );
}

void EDGUIItem::ReshapeLayout()
{
	EDGUIEvent e = { EDGUI_EVENT_PRELAYOUT, this };
	BubblingEvent( &e );
}

void EDGUIItem::SetSubitemLayout( EDGUIItem* subitem, int _x0, int _y0, int _x1, int _y1 )
{
//	LOG << "SUB LAYOUT: " << tyname << " -> " << subitem->tyname << LOG_SEP(", ") << _x0 << _y0 << _x1 << _y1;
	EDGUIEvent e = { EDGUI_EVENT_LAYOUT, this };
	e.layout.x0 = _x0;
	e.layout.y0 = _y0;
	e.layout.x1 = _x1;
	e.layout.y1 = _y1;
	subitem->OnEvent( &e );
}

void EDGUIItem::_SetFrame( EDGUIFrame* frame )
{
//	LOG << "SETTING FRAME " << m_frame << " -> " << frame << " at " << tyname;
	if( !frame && m_frame )
		m_frame->_Unlink( this );
	m_frame = frame;
	for( size_t i = 0; i < m_subitems.size(); ++i )
		m_subitems[ i ]->_SetFrame( frame );
}

void EDGUIItem::Edited( EDGUIItem* tgt )
{
	EDGUIEvent ev = { EDGUI_EVENT_PROPEDIT, tgt ? tgt : this };
	BubblingEvent( &ev );
}

void EDGUIItem::Changed( EDGUIItem* tgt )
{
	EDGUIEvent ev = { EDGUI_EVENT_PROPCHANGE, tgt ? tgt : this };
	BubblingEvent( &ev );
}

void EDGUIItem::SetCaption( const StringView& text )
{
	caption = text;
}


EDGUIFrame::EDGUIFrame() :
	m_mouseX( 0 ),
	m_mouseY( 0 ),
	m_hover( NULL ),
	m_keyboardFocus( NULL ),
	m_clickCount( 0 ),
	m_lastClickTime( 0 ),
	m_lastClickedButton( -1 ),
	m_lastClickItem( NULL )
{
	m_frame = this;
	tyname = "frame";
	type = EDGUI_ITEM_FRAME;
	for( int i = 0; i < 3; ++i )
	{
		m_clickTargets[i] = NULL;
		m_clickOffsets[i][0] = 0;
		m_clickOffsets[i][1] = 0;
	}
}

int EDGUIFrame::OnEvent( EDGUIEvent* e )
{
	return EDGUIItem::OnEvent( e );
}

void EDGUIFrame::EngineEvent( const Event* eev )
{
	EDGUIEvent ee = { EDGUI_EVENT_ENGINE };
	ee.eev = eev;
	EDGUIItem::OnEvent( &ee );
	
	if( eev->type == SDL_MOUSEMOTION )
	{
		m_mouseX = eev->motion.x;
		m_mouseY = eev->motion.y;
		if( m_hover )
		{
			EDGUIEvent e = { EDGUI_EVENT_MOUSEMOVE, m_hover };
			e.mouse.x = m_mouseX;
			e.mouse.y = m_mouseY;
			m_hover->OnEvent( &e );
		}
		_HandleMouseMove( true );
	}
	else if( eev->type == SDL_MOUSEWHEEL )
	{
		if( m_hover )
		{
			EDGUIEvent e = { EDGUI_EVENT_MOUSEWHEEL, m_hover };
			e.mouse.x = eev->wheel.x;
			e.mouse.y = eev->wheel.y;
			m_hover->BubblingEvent( &e );
		}
	}
	else if( eev->type == SDL_MOUSEBUTTONUP || eev->type == SDL_MOUSEBUTTONDOWN )
	{
		int btn = eev->button.button;
		if( btn == SGRX_MB_LEFT ) btn = EDGUI_MB_LEFT;
		else if( btn == SGRX_MB_RIGHT ) btn = EDGUI_MB_RIGHT;
		else if( btn == SGRX_MB_MIDDLE ) btn = EDGUI_MB_MIDDLE;
		else
			btn = -1;
		if( btn >= 0 )
		{
			bool down = eev->type == SDL_MOUSEBUTTONDOWN;
			EDGUIEvent ev = { down ? EDGUI_EVENT_BTNDOWN : EDGUI_EVENT_BTNUP };
			ev.mouse.x = m_mouseX;
			ev.mouse.y = m_mouseY;
			ev.mouse.button = btn;
			ev.mouse.clicks = 1;
			
			if( !down )
			{
				if( m_lastClickedButton != btn || m_clickTargets[ btn ] != m_lastClickItem || GetTimeMsec() - m_lastClickTime > DOUBLE_CLICK_MSEC )
				{
					m_lastClickedButton = btn;
					m_clickCount = 0;
				}
				m_clickCount++;
				ev.mouse.clicks = m_clickCount;
				
				m_lastClickTime = GetTimeMsec();
				m_lastClickItem = m_clickTargets[ btn ];
				
				if( m_clickTargets[ btn ] )
				{
					m_clickTargets[ btn ]->OnEvent( &ev );
					m_clickTargets[ btn ] = NULL;
				}
				_HandleMouseMove( true );
			}
			else if( m_hover )
			{
				m_clickTargets[ btn ] = m_hover;
				m_clickOffsets[ btn ][0] = m_mouseX - m_hover->x0;
				m_clickOffsets[ btn ][1] = m_mouseY - m_hover->y0;
				m_hover->OnEvent( &ev );
			}
		}
	}
	else if( eev->type == SDL_KEYDOWN || eev->type == SDL_KEYUP )
	{
		if( !m_keyboardFocus )
			return;
		
		bool down = eev->type == SDL_KEYDOWN;
		int engkey = eev->key.keysym.sym;
		int engmod = eev->key.keysym.mod;
		
		EDGUIEvent ev = { down ? EDGUI_EVENT_KEYDOWN : EDGUI_EVENT_KEYUP, m_keyboardFocus };
		ev.key.key = EDGUI_KEY_UNKNOWN;
		ev.key.engkey = engkey;
		ev.key.engmod = engmod;
		ev.key.repeat = !!eev->key.repeat;
		
		if(0);
		else if( engkey == SDLK_RETURN ) ev.key.key = EDGUI_KEY_ENTER;
		else if( engkey == SDLK_BACKSPACE ) ev.key.key = EDGUI_KEY_DELLEFT;
		else if( engkey == SDLK_DELETE ) ev.key.key = EDGUI_KEY_DELRIGHT;
		else if( engkey == SDLK_LEFT ) ev.key.key = EDGUI_KEY_LEFT;
		else if( engkey == SDLK_RIGHT ) ev.key.key = EDGUI_KEY_RIGHT;
		else if( engkey == SDLK_a && engmod & KMOD_CTRL ){ ev.key.key = EDGUI_KEY_SELECTALL; }
		else if( engkey == SDLK_x && engmod & KMOD_CTRL ){ ev.key.key = EDGUI_KEY_CUT; }
		else if( engkey == SDLK_c && engmod & KMOD_CTRL ){ ev.key.key = EDGUI_KEY_COPY; }
		else if( engkey == SDLK_v && engmod & KMOD_CTRL ){ ev.key.key = EDGUI_KEY_PASTE; }
		
		if( engmod & KMOD_SHIFT )
			ev.key.key |= EDGUI_KEYMOD_SHIFT;
		
		m_keyboardFocus->OnEvent( &ev );
	}
	else if( eev->type == SDL_TEXTINPUT )
	{
		if( !m_keyboardFocus )
			return;
		
		EDGUIEvent ev = { EDGUI_EVENT_TEXTINPUT, m_keyboardFocus };
		sgrx_sncopy( ev.text.text, 8, eev->text.text );
		ev.text.text[7] = 0;
		
		m_keyboardFocus->OnEvent( &ev );
	}
}

void EDGUIFrame::Resize( int w, int h, int x, int y )
{
	x0 = x;
	y0 = y;
	x1 = x + w;
	y1 = y + h;
	
	OnChangeLayout();
}

void EDGUIFrame::Draw()
{
	EDGUIEvent ev = { EDGUI_EVENT_PAINT, this };
	OnEvent( &ev );
}

bool EDGUIFrame::PushScissorRect( int _x0, int _y0, int _x1, int _y1 )
{
	Rect last;
	if( m_rects.size() )
		last = m_rects.last();
	else
	{
		last.x0 = x0;
		last.y0 = y0;
		last.x1 = x1;
		last.y1 = y1;
	}
	if( last.x1 <= _x0 || _x1 <= last.x0 || last.y1 <= _y0 || _y1 <= last.y0 )
		return false;
	Rect R = { TMAX( _x0, last.x0 ), TMAX( _y0, last.y0 ), TMIN( _x1, last.x1 ), TMIN( _y1, last.y1 ) };
	m_rects.push_back( R );
	GR2D_GetBatchRenderer().Flush();
	GR2D_SetScissorRect( R.x0, R.y0, R.x1, R.y1 );
	return true;
}

void EDGUIFrame::PopScissorRect()
{
	GR2D_GetBatchRenderer().Flush();
	m_rects.pop_back();
	if( m_rects.size() )
	{
		Rect last = m_rects.last();
		GR2D_SetScissorRect( last.x0, last.y0, last.x1, last.y1 );
	}
	else
		GR2D_UnsetScissorRect();
}

void EDGUIFrame::_HandleMouseMove( bool optional )
{
	if( optional )
	{
		for( int i = 0; i < 3; ++i )
			if( m_clickTargets[ i ] )
				return;
	}
	
	EDGUIItem* prevhover = m_hover;
	m_hover = _GetItemAtPosition( m_mouseX, m_mouseY );
	
	if( m_hover != prevhover )
	{
		if( prevhover || m_hover )
		{
			// try to find common parent
			EDGUIItem* phi = prevhover, *pphi = NULL;
			size_t i;
			while( phi )
			{
				for( i = 0; i < m_hoverTrail.size(); ++i )
					if( m_hoverTrail[ i ] == phi )
						break;
				if( i < m_hoverTrail.size() )
					break;
				pphi = phi;
				phi = phi->m_parent;
			}
			
		//	printf("ht0=%p htsz=%d pphi=%p phi=%p prev=%p curr=%p\n", m_hoverTrail.size()>=2 ? m_hoverTrail[1] : NULL, (int) m_hoverTrail.size(), pphi, phi, prevhover, m_hover );
			if( !phi )
			{
				// no common parent, run bubbling event on previous hover, bubbling event on current hover, update all styles
				if( pphi )
				{
					EDGUIEvent e = { EDGUI_EVENT_MOUSELEAVE, pphi };
					e.mouse.x = m_mouseX;
					e.mouse.y = m_mouseY;
					pphi->BubblingEvent( &e );
				}
				if( m_hover )
				{
					EDGUIEvent e = { EDGUI_EVENT_MOUSEENTER, m_hover };
					e.mouse.x = m_mouseX;
					e.mouse.y = m_mouseY;
					m_hover->BubblingEvent( &e );
				}
			}
			else
			{
				EDGUIEvent e;
				// found common parent, run events through that (parent gets none), update styles up from parent (not parent itself)
				EDGUIItem *cc;
				
				e.type = EDGUI_EVENT_MOUSELEAVE;
				e.target = prevhover;
				e.mouse.x = m_mouseX;
				e.mouse.y = m_mouseY;
				
				cc = prevhover;
				while( cc && cc != phi )
				{
					if( !cc->OnEvent( &e ) )
						break;
					cc = cc->m_parent;
				}
				
				e.type = EDGUI_EVENT_MOUSEENTER;
				e.target = m_hover;
				e.mouse.x = m_mouseX;
				e.mouse.y = m_mouseY;
				
				cc = m_hover;
				while( cc && cc != phi )
				{
					if( !cc->OnEvent( &e ) )
						break;
					cc = cc->m_parent;
				}
			}
		}
	}
}

EDGUIItem* EDGUIFrame::_GetItemAtPosition( int x, int y )
{
	m_hoverTrail.clear();
	m_hoverTrail.push_back( this );
	EDGUIItem* item = this, *atpos = NULL;
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

void EDGUIFrame::_Unlink( EDGUIItem* item )
{
	if( m_keyboardFocus == item )
		m_keyboardFocus = NULL;
	if( m_clickTargets[0] == item )
		m_clickTargets[0] = NULL;
	if( m_clickTargets[1] == item )
		m_clickTargets[1] = NULL;
	if( m_clickTargets[2] == item )
		m_clickTargets[2] = NULL;
	if( m_lastClickItem == item )
		m_lastClickItem = NULL;
}

void EDGUIFrame::_SetFocus( EDGUIItem* item )
{
	if( m_keyboardFocus )
	{
		EDGUIEvent se = { EDGUI_EVENT_LOSEFOCUS, m_keyboardFocus };
		m_keyboardFocus->OnEvent( &se );
	}
	m_keyboardFocus = item;
}


EDGUILayoutRow::EDGUILayoutRow()
{
	type = EDGUI_ITEM_LAYOUT_ROW;
	tyname = "layout-row";
}

int EDGUILayoutRow::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_LAYOUT:
		{
			SetRectFromEvent( e, false );
			int at = y0;
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				SetSubitemLayout( m_subitems[ i ], x0, at, x1, at );
				at = m_subitems[ i ]->y1;
			}
			y1 = at;
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT, this };
			if( m_parent )
				m_parent->OnEvent( &se );
		}
		return 1;
		
	case EDGUI_EVENT_PRELAYOUT:
		return 1;
		
	case EDGUI_EVENT_POSTLAYOUT:
		{
			y1 = m_subitems.size() ? m_subitems.last()->y1 : y0;
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT, this };
			if( m_parent )
				m_parent->OnEvent( &se );
		}
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}


EDGUILayoutColumn::EDGUILayoutColumn()
{
	type = EDGUI_ITEM_LAYOUT_COL;
	tyname = "layout-col";
}

int EDGUILayoutColumn::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_LAYOUT:
		{
			SetRectFromEvent( e, false );
			if( m_subitems.size() )
			{
				int pat = x0;
				int delta = ( x1 - x0 ) * 65536 / m_subitems.size();
				int at = x0 * 65536 + delta;
				y1 = y0;
				for( size_t i = 0; i < m_subitems.size(); ++i )
				{
					int cur = at / 65536;
					SetSubitemLayout( m_subitems[ i ], pat, y0, cur, y0 );
					
					pat = cur;
					at += delta;
					if( m_subitems[ i ]->y1 > y1 )
						y1 = m_subitems[ i ]->y1;
				}
			}
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT, this };
			if( m_parent )
				m_parent->OnEvent( &se );
		}
		return 1;
		
	case EDGUI_EVENT_PRELAYOUT:
		return 1;
		
	case EDGUI_EVENT_POSTLAYOUT:
		{
			y1 = y0;
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				if( m_subitems[ i ]->y1 > y1 )
					y1 = m_subitems[ i ]->y1;
			}
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT, this };
			if( m_parent )
				m_parent->OnEvent( &se );
		}
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}


EDGUILayoutSplitPane::EDGUILayoutSplitPane( bool vertical, int splitoff, float splitfac ) :
	m_vertical( vertical ),
	m_splitoff( splitoff ),
	m_splitfac( splitfac ),
	m_first( NULL ),
	m_second( NULL )
{
	type = EDGUI_ITEM_SPLIT_PANE;
	tyname = "split-pane";
	backColor = EDGUI_THEME_SPLITPANE_BORDER_COLOR;
}

int EDGUILayoutSplitPane::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_LAYOUT:
		{
			SetRectFromEvent( e, false );
			if( !m_vertical )
			{
				int cx = (int) round( x0 * (1-m_splitfac) + x1 * m_splitfac ) + m_splitoff;
				int cx0 = cx - EDGUI_THEME_SPLITPANE_BORDER_SIZE / 2;
				int cx1 = cx0 + EDGUI_THEME_SPLITPANE_BORDER_SIZE;
				
				if( m_first ) SetSubitemLayout( m_first, x0, y0, cx0, y1 );
				if( m_second ) SetSubitemLayout( m_second, cx1, y0, x1, y1 );
			}
			else
			{
				int cy = (int) round( y0 * (1-m_splitfac) + y1 * m_splitfac ) + m_splitoff;
				int cy0 = cy - EDGUI_THEME_SPLITPANE_BORDER_SIZE / 2;
				int cy1 = cy0 + EDGUI_THEME_SPLITPANE_BORDER_SIZE;
				
				if( m_first ) SetSubitemLayout( m_first, x0, y0, x1, cy0 );
				if( m_second ) SetSubitemLayout( m_second, x0, cy1, x1, y1 );
			}
		}
		return 1;
		
	case EDGUI_EVENT_PAINT:
		if( backColor )
		{
			if( !m_vertical )
			{
				int cx = (int) round( x0 * (1-m_splitfac) + x1 * m_splitfac ) + m_splitoff;
				int cx0 = cx - EDGUI_THEME_SPLITPANE_BORDER_SIZE / 2;
				int cx1 = cx0 + EDGUI_THEME_SPLITPANE_BORDER_SIZE;
				GR2D_GetBatchRenderer().UnsetTexture().Colu( backColor ).Quad( float(cx0), float(y0), float(cx1), float(y1) );
			}
			else
			{
				int cy = (int) round( y0 * (1-m_splitfac) + y1 * m_splitfac ) + m_splitoff;
				int cy0 = cy - EDGUI_THEME_SPLITPANE_BORDER_SIZE / 2;
				int cy1 = cy0 + EDGUI_THEME_SPLITPANE_BORDER_SIZE;
				GR2D_GetBatchRenderer().UnsetTexture().Colu( backColor ).Quad( float(x0), float(cy0), float(x1), float(cy1) );
			}
		}
		if( m_first ) m_first->OnEvent( e );
		if( m_second ) m_second->OnEvent( e );
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}

void EDGUILayoutSplitPane::SetPane( bool second, EDGUIItem* item )
{
	EDGUIItem*& tgtpane = second ? m_second : m_first;
	if( tgtpane && tgtpane->m_parent == this )
	{
		Remove( tgtpane );
	}
	tgtpane = item;
	if( item )
	{
		Add( item );
	}
	OnChangeLayout();
}

void EDGUILayoutSplitPane::SetFirstPane( EDGUIItem* item ){ SetPane( false, item ); }
void EDGUILayoutSplitPane::SetSecondPane( EDGUIItem* item ){ SetPane( true, item ); }


EDGUIVScroll::EDGUIVScroll( float ipos ) : m_offset( ipos ), m_length( 0 )
{
	type = EDGUI_ITEM_VSCROLL;
	tyname = "vscroll";
}

int EDGUIVScroll::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_LAYOUT:
		{
			int off = GetScrollOffset();
			SetRectFromEvent( e, false );
			m_length = 0;
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				SetSubitemLayout( m_subitems[ i ],
					e->layout.x0, e->layout.y0 - off,
					e->layout.x1, e->layout.y1 - off );
				m_length = TMAX( m_length, float(m_subitems[ i ]->y1 - m_subitems[ i ]->y0) );
			}
			// HACK
			off = GetScrollOffset();
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				SetSubitemLayout( m_subitems[ i ],
					e->layout.x0, e->layout.y0 - off,
					e->layout.x1, e->layout.y1 - off );
			}
			return 1;
		}
	case EDGUI_EVENT_MOUSEWHEEL:
		m_offset -= e->mouse.y * 32;
		m_offset = GetScrollOffset();
		OnChangeLayout();
		break;
	case EDGUI_EVENT_PAINT:
		{
			float ht = y1 - y0;
			float maxoff = TMAX( m_length - ht, 0.0f );
			if( m_frame->PushScissorRect( x0, y0, x1, y1 ) )
			{
				for( size_t i = 0; i < m_subitems.size(); ++i )
				{
					m_subitems[ i ]->OnEvent( e );
				}
				if( maxoff )
				{
					float off = GetScrollOffset();
					int sw = TMAX( 0, TMIN( 4, x1 - x0 ) );
					int x1a = x1 - sw;
					GR2D_GetBatchRenderer().Reset().Colu( EDGUI_THEME_SCROLL_BACK_COLOR )
						.Quad( float(x1a), float(y0), float(x1), float(y1) );
					int sy0 = (int) TLERP( float(y0), float(y1), safe_fdiv( off, maxoff + ht ) );
					int sy1 = (int) TLERP( float(y0), float(y1), safe_fdiv( off + ht, maxoff + ht ) );
					GR2D_GetBatchRenderer().Reset().Colu( EDGUI_THEME_SCROLL_BAR_COLOR )
						.Quad( float(x1a), float(sy0), float(x1), float(sy1) );
				}
				m_frame->PopScissorRect();
			}
		}
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}

float EDGUIVScroll::GetScrollOffset()
{
	float maxoff = TMAX( m_length - ( y1 - y0 ), 0.0f );
	return clamp( m_offset, 0, maxoff );
}


EDGUILabel::EDGUILabel()
{
	type = EDGUI_ITEM_LABEL;
	tyname = "label";
}

int EDGUILabel::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_BTNCLICK:
		return 0;
	case EDGUI_EVENT_LAYOUT:
		x0 = e->layout.x0;
		x1 = e->layout.x1;
		y0 = e->layout.y0;
		y1 = y0 + EDGUI_THEME_LABEL_HEIGHT;
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}


EDGUIGroup::EDGUIGroup( bool open, const StringView& sv ) :
	m_open( open )
{
	type = EDGUI_ITEM_GROUP;
	tyname = "group";
	caption = sv;
	SetOpen( open );
	_UpdateButton();
}

int EDGUIGroup::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_MOUSELEAVE:
		EDGUILayoutRow::OnEvent( e );
		_UpdateButton();
		return 1;
	case EDGUI_EVENT_MOUSEMOVE:
	case EDGUI_EVENT_MOUSEENTER:
		{
			bool prevmon = m_mouseOn;
			int at = y1;
			y1 = y0 + EDGUI_THEME_GROUP_HEIGHT;
			m_mouseOn = Hit( e->mouse.x, e->mouse.y );
			y1 = at;
			_UpdateButton();
			if( m_mouseOn != prevmon )
				Invalidate();
		}
		return 1;
	case EDGUI_EVENT_BTNDOWN:
	case EDGUI_EVENT_BTNUP:
		EDGUILayoutRow::OnEvent( e );
		_UpdateButton();
		return 1;
		
	case EDGUI_EVENT_BTNCLICK:
		if( e->target != this )
			return 1;
		SetOpen( !m_open );
		return 0;
		
	case EDGUI_EVENT_LAYOUT:
		{
			x0 = e->layout.x0;
			y0 = e->layout.y0;
			x1 = e->layout.x1;
			y1 = y0 + EDGUI_THEME_GROUP_HEIGHT;
			int at = y1;
			if( m_open )
			{
				for( size_t i = 0; i < m_subitems.size(); ++i )
				{
					SetSubitemLayout( m_subitems[ i ], x0 + 8, at, x1, at );
					at = m_subitems[ i ]->y1;
				}
				y1 = at;
			}
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT, this };
			if( m_parent )
				m_parent->OnEvent( &se );
		}
		return 1;
		
	case EDGUI_EVENT_PAINT:
		{
			int y1a = y0 + EDGUI_THEME_GROUP_HEIGHT;
			if( backColor )
			{
				GR2D_GetBatchRenderer().Reset().Colu( backColor ).Quad( float(x0), float(y0), float(x1), float(y1a) );
			}
			if( textColor && m_name.size() )
			{
				GR2D_GetBatchRenderer().Reset().Colu( textColor );
				GR2D_DrawTextLine( round(x0 + 2.0f), round(( y0 + y1a ) / 2.0f), m_name, HALIGN_LEFT, VALIGN_CENTER );
			}
			if( m_open )
			{
				for( size_t i = 0; i < m_subitems.size(); ++i )
				{
					m_subitems[ i ]->OnEvent( e );
				}
			}
		}
		return 1;
	}
	return EDGUILayoutRow::OnEvent( e );
}

void EDGUIGroup::SetOpen( bool open )
{
	m_open = open;
	m_name = String_Concat( open ? "[--] " : "[+] ", caption );
	if( m_parent )
		m_parent->ReshapeLayout();
}

void EDGUIGroup::_UpdateButton()
{
	backColor = m_clicked ?
		EDGUI_THEME_GRPLBL_BACK_COLOR_CLICKED :
		( m_mouseOn ? EDGUI_THEME_GRPLBL_BACK_COLOR_MOUSEON : EDGUI_THEME_GRPLBL_BACK_COLOR );
}


EDGUIButton::EDGUIButton() : m_highlight( false )
{
	type = EDGUI_ITEM_BUTTON;
	tyname = "button";
	backColor = EDGUI_THEME_BUTTON_BACK_COLOR;
	textColor = EDGUI_THEME_BUTTON_TEXT_COLOR;
}

int EDGUIButton::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_LAYOUT:
		x0 = e->layout.x0;
		x1 = e->layout.x1;
		y0 = e->layout.y0;
		y1 = y0 + EDGUI_THEME_BUTTON_HEIGHT;
		return 1;
		
	case EDGUI_EVENT_MOUSEENTER:
	case EDGUI_EVENT_MOUSELEAVE:
		EDGUIItem::OnEvent( e );
		OnChangeState();
		return 1;
		
	case EDGUI_EVENT_BTNDOWN:
	case EDGUI_EVENT_BTNUP:
		if( e->mouse.button == EDGUI_MB_LEFT )
		{
			EDGUIItem::OnEvent( e );
			OnChangeState();
		}
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}

void EDGUIButton::OnChangeState()
{
	if( m_highlight )
	{
		backColor = m_clicked ?
			EDGUI_THEME_BUTTON_BACKHL_COLOR_CLICKED :
			( m_mouseOn ? EDGUI_THEME_BUTTON_BACKHL_COLOR_MOUSEON : EDGUI_THEME_BUTTON_BACKHL_COLOR );
	}
	else
	{
		backColor = m_clicked ?
			EDGUI_THEME_BUTTON_BACK_COLOR_CLICKED :
			( m_mouseOn ? EDGUI_THEME_BUTTON_BACK_COLOR_MOUSEON : EDGUI_THEME_BUTTON_BACK_COLOR );
	}
	Invalidate();
}

void EDGUIButton::SetHighlight( bool hl )
{
	m_highlight = hl;
	OnChangeState();
}


EDGUIBtnList::EDGUIBtnList()
{
	type = EDGUI_ITEM_BTNLIST;
	tyname = "btnlist";
	backColor = EDGUI_THEME_BUTTON_BACK_COLOR;
	textColor = EDGUI_THEME_BUTTON_TEXT_COLOR;
}

int EDGUIBtnList::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_LAYOUT:
		x0 = e->layout.x0;
		x1 = e->layout.x1;
		y0 = e->layout.y0;
		y1 = y0 + EDGUI_THEME_BUTTON_HEIGHT * m_options.size();
		{
			int hl = m_highlight;
			int cy0 = ( hl >= 0 ? hl : m_options.size() ) * EDGUI_THEME_BUTTON_HEIGHT;
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				SetSubitemLayout( m_subitems[ i ], x0, cy0, x1, cy0 + EDGUI_THEME_BUTTON_HEIGHT );
			}
		}
		return 1;
		
	case EDGUI_EVENT_MOUSEENTER:
	case EDGUI_EVENT_MOUSEMOVE:
		SetHighlight( ( e->mouse.y - y0 ) / EDGUI_THEME_BUTTON_HEIGHT );
		break;
		
	case EDGUI_EVENT_PAINT:
		if( textColor )
		{
			GR2D_GetBatchRenderer().Reset().Colu( textColor );
			for( size_t i = 0; i < m_options.size(); ++i )
			{
				GR2D_DrawTextLine( x0 + 2,
					y0 + EDGUI_THEME_BUTTON_HEIGHT / 2 + i * EDGUI_THEME_BUTTON_HEIGHT,
					m_options[ i ], HALIGN_LEFT, VALIGN_CENTER );
			}
		}
		if( m_highlight >= 0 )
		{
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				m_subitems[ i ]->OnEvent( e );
			}
		}
		return 1;
		
	}
	return EDGUIItem::OnEvent( e );
}

void EDGUIBtnList::UpdateOptions()
{
	OnChangeLayout();
	SetHighlight( -1 );
	EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT, this };
	if( m_parent )
		m_parent->OnEvent( &se );
}

void EDGUIBtnList::SetHighlight( int hl )
{
	if( hl < -1 || hl >= (int) m_options.size() )
		hl = -1;
	m_highlight = hl;
	int cy0 = y0 + ( hl >= 0 ? hl : m_options.size() ) * EDGUI_THEME_BUTTON_HEIGHT;
	for( size_t i = 0; i < m_subitems.size(); ++i )
	{
		SetSubitemLayout( m_subitems[ i ], x0, cy0, x1, cy0 + EDGUI_THEME_BUTTON_HEIGHT );
		m_subitems[ i ]->SetCaption( hl >= 0 ? m_options[ hl ] : "" );
		_RecursiveSetID2( m_subitems[ i ], hl );
	}
}

void EDGUIBtnList::_RecursiveSetID2( EDGUIItem* item, int val )
{
	item->id2 = val;
	for( size_t i = 0; i < item->m_subitems.size(); ++i )
	{
		_RecursiveSetID2( item->m_subitems[ i ], val );
	}
}


EDGUINumberWheel::EDGUINumberWheel( EDGUIItem* owner, double min, double max, int initpwr, int numwheels ) :
	m_value( 0 ),
	m_min( min ),
	m_max( max ),
	m_cx( 0 ),
	m_cy( 0 ),
	m_initpwr( initpwr ),
	m_numwheels( numwheels ),
	m_prevMouseX( -1 ),
	m_prevMouseY( -1 ),
	m_curWheel( 0 ),
	m_owner( owner )
{
	type = EDGUI_ITEM_NUMWHEEL;
	tyname = "numberwheel";
	backColor = EDGUI_THEME_OVERLAY_COLOR;
}

int EDGUINumberWheel::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_ADDED:
		m_frame->_SetFocus( this );
		return 1;
		
	case EDGUI_EVENT_KEYDOWN:
		if( e->key.key == EDGUI_KEY_COPY )
		{
			char bfr[ 1224 ];
			sgrx_snprintf( bfr, sizeof( bfr ), "%.18g", GetValue() );
			Window_SetClipboardText( bfr );
		}
		if( e->key.key == EDGUI_KEY_PASTE )
		{
			String text;
			if( Window_GetClipboardText( text ) )
			{
				double prevval = GetValue();
				m_value = String_ParseFloat( text );
				if( m_value < m_min ) m_value = m_min;
				if( m_value > m_max ) m_value = m_max;
				if( GetValue() != prevval && m_owner )
				{
					EDGUIEvent se = { EDGUI_EVENT_PROPEDIT, this };
					m_owner->BubblingEvent( &se );
				}
			}
			else
				LOG << "Failed to retrieve text from clipboard";
		}
		return 1;
		
	case EDGUI_EVENT_BTNCLICK:
		{
			int prbx = e->mouse.x - m_cx;
			int prby = e->mouse.y - m_cy;
			float dist = sqrtf( float( prbx * prbx + prby * prby ) );
			if( dist < EDGUI_THEME_NUMWHEEL_CENTER_SIZE && m_curWheel < 0 )
			{
				if( m_owner )
				{
					EDGUIEvent se = { EDGUI_EVENT_PROPCHANGE, this };
					m_owner->BubblingEvent( &se );
				}
				m_parent->Remove( this );
			}
		}
		return 1;
		
	case EDGUI_EVENT_MOUSEMOVE:
		if( m_prevMouseX >= 0 && m_prevMouseY >= 0 )
		{
			if( !m_clicked )
			{
				int prbx = ( e->mouse.x + m_prevMouseX ) / 2 - m_cx;
				int prby = ( e->mouse.y + m_prevMouseY ) / 2 - m_cy;
				float dist = sqrtf( float( prbx * prbx + prby * prby ) ) - EDGUI_THEME_NUMWHEEL_CENTER_SIZE;
				dist /= EDGUI_THEME_NUMWHEEL_WHEEL_SIZE;
				m_curWheel = (int) floor( dist );
			}
			if( m_clicked && m_curWheel >= 0 && m_curWheel < m_numwheels )
			{
				float pa = atan2f( float(m_prevMouseY - m_cy), float(m_prevMouseX - m_cx) );
				float ca = atan2f( float(e->mouse.y - m_cy  ), float(e->mouse.x - m_cx  ) );
				if( pa - ca > M_PI ) pa -= (float) M_PI * 2.0f;
				if( ca - pa > M_PI ) ca -= (float) M_PI * 2.0f;
				float diff = ca - pa;
				
				double prevval = GetValue();
				m_value += diff * powf( 10.0f, float(m_initpwr + m_curWheel) );
				if( m_value < m_min ) m_value = m_min;
				if( m_value > m_max ) m_value = m_max;
				if( GetValue() != prevval && m_owner )
				{
					EDGUIEvent se = { EDGUI_EVENT_PROPEDIT, this };
					m_owner->BubblingEvent( &se );
				}
			}
		}
		m_prevMouseX = e->mouse.x;
		m_prevMouseY = e->mouse.y;
		return 1;
		
	case EDGUI_EVENT_PAINT:
		{
			int rad = m_numwheels * EDGUI_THEME_NUMWHEEL_WHEEL_SIZE + EDGUI_THEME_NUMWHEEL_CENTER_SIZE;
			BatchRenderer& br = GR2D_GetBatchRenderer();
			br.UnsetTexture().Colu( backColor ).Quad( x0, y0, x1, y1 );
			br.Colu( EDGUI_THEME_NUMWHEEL_WHEEL_COLOR ).CircleFill( m_cx, m_cy, rad );
			for( int i = 0; i <= m_numwheels; ++i )
			{
				rad = i * EDGUI_THEME_NUMWHEEL_WHEEL_SIZE + EDGUI_THEME_NUMWHEEL_CENTER_SIZE;
				br.Colu( EDGUI_THEME_NUMWHEEL_OUTLINE_COLOR ).CircleOutline( m_cx, m_cy, rad );
			}
			int xdif = m_cx - ( m_frame->x0 + m_frame->x1 ) / 2;
			int ydif = m_cy - ( m_frame->y0 + m_frame->y1 ) / 2;
			float xfac = ( xdif >= 0 ? -1 : 1 ) * 0.707f;
			float yfac = ( ydif >= 0 ? -1 : 1 ) * 0.707f;
			
			char bfr[ 32 ] = {0};
			br.Colu( textColor );
			if( m_curWheel >= 0 && m_curWheel < m_numwheels )
			{
				rad = m_curWheel * EDGUI_THEME_NUMWHEEL_WHEEL_SIZE + EDGUI_THEME_NUMWHEEL_CENTER_SIZE;
				sgrx_snprintf( bfr, 31, "%g", powf( 10, m_initpwr + m_curWheel ) );
				GR2D_DrawTextLine( m_cx + xfac * rad, m_cy + yfac * rad, bfr );
			}
			sgrx_snprintf( bfr, 31, "%g", GetValue() );
			GR2D_DrawTextLine( m_cx, m_cy, bfr, HALIGN_CENTER, VALIGN_CENTER );
		}
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}

double EDGUINumberWheel::GetValue()
{
	double rf = powf( 10, m_initpwr );
	return round( m_value / rf ) * rf;
}


EDGUIRsrcPicker::EDGUIRsrcPicker() :
	m_zoom( 1 ),
	m_horCount( 0 ),
	m_scrollOffset( 0 ),
	m_itemWidth( 0 ),
	m_itemHeight( 0 ),
	cx0( 0 ),
	cy0( 0 ),
	cx1( 0 ),
	cy1( 0 ),
	m_hlfiltered( -1 ),
	m_picked( 0 ),
	m_owner( NULL ),
	m_mouseX( 0 ),
	m_mouseY( 0 )
{
	type = EDGUI_ITEM_RSRCPICKER;
	tyname = "rsrcpicker";
	backColor = EDGUI_THEME_OVERLAY_COLOR;
	_Search( m_searchString );
	Zoom( m_zoom );
}

int EDGUIRsrcPicker::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_ADDED:
	case EDGUI_EVENT_SETFOCUS:
		m_frame->_SetFocus( this );
		return 1;
		
	case EDGUI_EVENT_LAYOUT:
		EDGUIItem::OnEvent( e );
		cx0 = x0 + 32;
		cy0 = y0 + 64;
		cx1 = x1 - 32;
		cy1 = y1 - 32;
		Zoom( m_zoom );
		return 1;
		
	case EDGUI_EVENT_MOUSEENTER:
	case EDGUI_EVENT_MOUSEMOVE:
		m_mouseX = e->mouse.x;
		m_mouseY = e->mouse.y;
		_FindHL();
		break;
		
	case EDGUI_EVENT_MOUSEWHEEL:
		m_scrollOffset -= e->mouse.y * 32;
		m_scrollOffset = GetScrollOffset();
		_FindHL();
		break;
		
	case EDGUI_EVENT_BTNCLICK:
		if( m_hlfiltered != -1 )
		{
			m_picked = m_hlfiltered;
			m_pickedOption = m_options[ m_picked ];
			_OnPickResource();
			return 1;
		}
		break;
		
	case EDGUI_EVENT_PAINT:
		{
			int soff = GetScrollOffset();
			BatchRenderer& br = GR2D_GetBatchRenderer();
			br.UnsetTexture().Colu( backColor ).Quad( x0, y0, x1, y1 );
			br.Colu( textColor );
			br.SetPrimitiveType( PT_LineStrip ).Pos( cx0, cy0 ).Pos( cx1, cy0 ).Pos( cx1, cy1 ).Pos( cx0, cy1 ).Prev(3);
			if( caption.size() )
				GR2D_DrawTextLine( x1 - 32, y0 + 32, caption, HALIGN_RIGHT, VALIGN_TOP );
			GR2D_DrawTextLine( 32, 32, String_Concat( "Type to search: ", m_searchString ) );
			if( m_horCount && m_frame->PushScissorRect( cx0, cy0, cx1, cy1 ) )
			{
				size_t i = 0;
				while( (int) ( i + 1 ) * m_itemHeight < soff )
					i++;
				i *= m_horCount;
				for( ; i < m_filtered.size(); ++i )
				{
					int tx = i % m_horCount;
					int ty = i / m_horCount;
					int rx0 = cx0 + tx * m_itemWidth;
					int ry0 = cy0 + ty * m_itemHeight - soff;
					if( ry0 > cy1 )
						break;
					int rx1 = rx0 + m_itemWidth;
					int ry1 = ry0 + m_itemHeight;
					br.Colu( m_hlfiltered == m_filtered[ i ] ? EDGUI_THEME_RSRCPICK_ITEM_BACKHL_COLOR : EDGUI_THEME_RSRCPICK_ITEM_BACK_COLOR );
					br.UnsetTexture().Quad( rx0, ry0, rx1, ry1 );
					_DrawItem( m_filtered[ i ], rx0, ry0, rx1, ry1 );
				}
				m_frame->PopScissorRect();
			}
		}
		return 1;
		
	case EDGUI_EVENT_KEYDOWN:
		if( e->key.engkey == SDLK_ESCAPE )
		{
			Close();
			return 1;
		}
		else if( e->key.engkey == SDLK_RETURN )
		{
			m_pickedOption = m_searchString;
			_OnConfirm();
			return 1;
		}
		else if( e->key.engkey == SDLK_BACKSPACE )
		{
			while( m_searchString.size() && ( m_searchString.last() & 0xC0 ) == 0x80 )
				m_searchString.pop_back();
			if( m_searchString.size() )
				m_searchString.pop_back();
			_Search( m_searchString );
			return 1;
		}
		else if( e->key.engkey == SDLK_r && e->key.engmod & KMOD_CTRL )
		{
			Reload();
		}
		break;
		
	case EDGUI_EVENT_TEXTINPUT:
		m_searchString.append( e->text.text );
		_Search( m_searchString );
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}

void EDGUIRsrcPicker::Open( EDGUIItem* owner, const StringView& val )
{
	m_owner = owner;
	SetValue( val );
}

void EDGUIRsrcPicker::Close()
{
	EDGUIFrame* frame = m_frame;
	if( m_parent )
		m_parent->Remove( this );
	if( frame )
		frame->_HandleMouseMove( false );
	m_owner = NULL;
}

void EDGUIRsrcPicker::SetValue( const StringView& sv )
{
	m_pickedOption = sv;
	m_picked = m_options.find_first_at( sv );
}

String EDGUIRsrcPicker::GetValue()
{
	return m_pickedOption;
}

void EDGUIRsrcPicker::Zoom( float z )
{
	m_zoom = z;
	_OnChangeZoom();
	m_horCount = ( cx1 - cx0 ) / m_itemWidth;
	int oiw = m_itemWidth;
	while( m_itemWidth * m_horCount < cx1 - cx0 )
		m_itemWidth++;
	m_itemHeight = m_itemHeight * m_itemWidth / oiw;
}

void EDGUIRsrcPicker::_FindHL()
{
	int soff = GetScrollOffset();
	m_hlfiltered = -1;
	if( m_horCount )
	{
		for( size_t i = 0; i < m_filtered.size(); ++i )
		{
			int tx = i % m_horCount;
			int ty = i / m_horCount;
			int rx0 = cx0 + tx * m_itemWidth;
			int ry0 = cy0 + ty * m_itemHeight - soff;
			int rx1 = rx0 + m_itemWidth;
			int ry1 = ry0 + m_itemHeight;
			if( m_mouseX >= rx0 && m_mouseX < rx1 &&
				m_mouseY >= ry0 && m_mouseY < ry1 )
			{
				m_hlfiltered = m_filtered[ i ];
				break;
			}
		}
	}
}

void EDGUIRsrcPicker::_Search( const StringView& str )
{
	m_filtered.clear();
	if( !str )
	{
		for( size_t i = 0; i < m_options.size(); ++i )
			m_filtered.push_back( i );
	}
	else
	{
		for( size_t i = 0; i < m_options.size(); ++i )
		{
			if( StringView( m_options[ i ] ).find_first_at( str ) != NOT_FOUND )
				m_filtered.push_back( i );
		}
	}
	_FindHL();
}

int EDGUIRsrcPicker::GetScrollOffset()
{
	if( !m_horCount )
		return 0;
	int so = m_scrollOffset;
	int fullsz = divideup( m_filtered.size(), m_horCount ) * m_itemHeight - ( cy1 - cy0 );
	if( so > fullsz )
		so = fullsz;
	if( so < 0 )
		so = 0;
	return so;
}

void EDGUIRsrcPicker::_OnChangeZoom()
{
	m_itemWidth = int(m_zoom * 128);
	m_itemHeight = int(m_zoom * 128);
}

void EDGUIRsrcPicker::_OnPickResource()
{
	m_owner->Edited( this );
	m_owner->Changed( this );
	Close();
}

void EDGUIRsrcPicker::_OnConfirm()
{
	if( m_filtered.size() == 1 )
	{
		m_picked = m_filtered[0];
		m_pickedOption = m_options[ m_picked ];
		_OnPickResource();
	}
}

void EDGUIRsrcPicker::_DrawItem( int i, int x0, int y0, int x1, int y1 )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Col( 0.9f, 1.0f );
	GR2D_DrawTextLine( ( x0 + x1 ) / 2, ( y0 + y1 ) / 2, m_options[ i ], HALIGN_CENTER, VALIGN_CENTER );
}


EDGUIQuestion::EDGUIQuestion() :
	m_owner( NULL ),
	m_value( -1 )
{
	type = EDGUI_ITEM_QUESTION;
	tyname = "question";
	backColor = EDGUI_THEME_OVERLAY_COLOR;
	m_btnYes.caption = "Yes";
	m_btnNo.caption = "No";
	Add( &m_btnYes );
	Add( &m_btnNo );
}

int EDGUIQuestion::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_ADDED:
	case EDGUI_EVENT_SETFOCUS:
		m_frame->_SetFocus( this );
		return 1;
		
	case EDGUI_EVENT_LAYOUT:
		EDGUIItem::OnEvent( e );
		{
			EDGUIEvent se = { EDGUI_EVENT_LAYOUT, &m_btnYes };
			se.layout.x0 = ( x0 * 6/9 + x1 * 3/9 );
			se.layout.x1 = ( x0 * 5/9 + x1 * 4/9 );
			se.layout.y0 = ( y0 * 4/9 + y1 * 5/9 );
			se.layout.y1 = ( y0 * 3/9 + y1 * 6/9 );
			m_btnYes.OnEvent( &se );
		}
		{
			EDGUIEvent se = { EDGUI_EVENT_LAYOUT, &m_btnNo };
			se.layout.x0 = ( x0 * 4/9 + x1 * 5/9 );
			se.layout.x1 = ( x0 * 3/9 + x1 * 6/9 );
			se.layout.y0 = ( y0 * 4/9 + y1 * 5/9 );
			se.layout.y1 = ( y0 * 3/9 + y1 * 6/9 );
			m_btnNo.OnEvent( &se );
		}
		return 1;
		
	case EDGUI_EVENT_BTNCLICK:
		if( e->target == &m_btnYes )
		{
			m_value = 1;
			_OnChoose();
		}
		else if( e->target == &m_btnNo )
		{
			m_value = 0;
			_OnChoose();
		}
		return 1;
		
	case EDGUI_EVENT_PAINT:
		{
			BatchRenderer& br = GR2D_GetBatchRenderer();
			br.UnsetTexture().Colu( backColor ).Quad( x0, y0, x1, y1 );
			br.Colu( textColor );
			GR2D_DrawTextLine( (x0+x1)/2, y0*4/7+y1*3/7, caption, HALIGN_CENTER, VALIGN_CENTER );
			m_btnYes.OnEvent( e );
			m_btnNo.OnEvent( e );
		}
		return 1;
		
	case EDGUI_EVENT_KEYDOWN:
		if( e->key.engkey == SDLK_ESCAPE )
		{
			m_value = -1;
			_OnChoose();
			return 1;
		}
		break;
	}
	return EDGUIItem::OnEvent( e );
}

void EDGUIQuestion::Open( EDGUIItem* owner )
{
	m_owner = owner;
	m_value = -1;
}

void EDGUIQuestion::Close()
{
	if( m_parent )
		m_parent->Remove( this );
	m_owner = NULL;
}

void EDGUIQuestion::_OnChoose()
{
	m_owner->Edited( this );
	m_owner->Changed( this );
	Close();
}


EDGUIProperty::EDGUIProperty()
{
	type = EDGUI_ITEM_PROP_NULL;
	tyname = "property";
}

int EDGUIProperty::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_HITTEST:
		if( caption.size() && e->mouse.x <= x0 )
			return 0;
		break;
	case EDGUI_EVENT_LAYOUT:
		{
			x0 = e->layout.x0;
			x1 = e->layout.x1;
			y0 = e->layout.y0;
			y1 = y0 + EDGUI_THEME_PROPERTY_HEIGHT;
			_Begin( e );
			EDGUIEvent se = { EDGUI_EVENT_LAYOUT, e->target };
			se.layout.x0 = x0;
			se.layout.y0 = y0;
			se.layout.x1 = x1;
			se.layout.y1 = y1;
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				m_subitems[ i ]->OnEvent( &se );
			}
			_End( e );
		}
		return 1;
		
	case EDGUI_EVENT_PAINT:
		_Begin( e );
		for( size_t i = 0; i < m_subitems.size(); ++i )
		{
			m_subitems[ i ]->OnEvent( e );
		}
		_End( e );
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}

void EDGUIProperty::_Begin( EDGUIEvent* e )
{
	m_x0bk = x0;
	if( caption.size() )
	{
		if( e->type == EDGUI_EVENT_PAINT )
		{
			GR2D_GetBatchRenderer().Colu( textColor );
			GR2D_DrawTextLine( x0 + 2, ( y0 + y1 ) / 2, caption, HALIGN_LEFT, VALIGN_CENTER );
		}
		x0 = ( x0 + x1 ) / 2;
	}
}

void EDGUIProperty::_End( EDGUIEvent* e )
{
	x0 = m_x0bk;
}


EDGUIPropBool::EDGUIPropBool( bool def ) : m_value( def )
{
	type = EDGUI_ITEM_PROP_BOOL;
	tyname = "property-bool";
}

int EDGUIPropBool::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_BTNCLICK:
		_Begin( e );
		if( Hit( e->mouse.x, e->mouse.y ) )
		{
			bool newval = e->mouse.x > ( x0 + x1 ) / 2;
			if( m_value != newval )
			{
				m_value = newval;
				Edited();
				Changed();
			}
		}
		_End( e );
		return 1;
		
	case EDGUI_EVENT_PAINT:
		{
			_Begin( e );
			int mid = ( x0 + x1 ) / 2;
			GR2D_GetBatchRenderer().UnsetTexture()
				.Colu( !m_value ? EDGUI_THEME_PROP_BOOL_OFF_ACTIVE_COLOR : EDGUI_THEME_PROP_BOOL_OFF_INACTIVE_COLOR ).Quad( x0, y0, mid, y1 )
				.Colu( m_value ? EDGUI_THEME_PROP_BOOL_ON_ACTIVE_COLOR : EDGUI_THEME_PROP_BOOL_ON_INACTIVE_COLOR ).Quad( mid, y0, x1, y1 );
			
			GR2D_GetBatchRenderer().Colu( textColor );
			GR2D_DrawTextLine( ( x0 + mid ) / 2, ( y0 + y1 ) / 2, "No", HALIGN_CENTER, VALIGN_CENTER );
			GR2D_DrawTextLine( ( mid + x1 ) / 2, ( y0 + y1 ) / 2, "Yes", HALIGN_CENTER, VALIGN_CENTER );
			_End( e );
		}
		return 1;
	}
	
	return EDGUIProperty::OnEvent( e );
}


EDGUIPropInt::EDGUIPropInt( int32_t def, int32_t min, int32_t max ) :
	m_value( def ),
	m_min( min ),
	m_max( max ),
	m_numWheel( this, min, max, 0, ceil( log( TMAX( -(double)min, (double)max ) ) / log( 10.0 ) ) )
{
	tyname = "property-int";
	type = EDGUI_ITEM_PROP_INT;
	_UpdateButton();
	Add( &m_button );
}

int EDGUIPropInt::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
	case EDGUI_EVENT_PROPCHANGE:
		if( e->target == &m_numWheel )
		{
			m_value = m_numWheel.GetValue();
			_UpdateButton();
			EDGUIEvent se = { e->type, this };
			BubblingEvent( &se );
			return 0;
		}
		break;
		
	case EDGUI_EVENT_BTNCLICK:
		_Begin( e );
		if( Hit( e->mouse.x, e->mouse.y ) )
		{
			m_numWheel.m_cx = e->mouse.x;
			m_numWheel.m_cy = e->mouse.y;
			m_numWheel.m_value = m_value;
			m_frame->Add( &m_numWheel );
		}
		_End( e );
		return 1;
	}
	return EDGUIProperty::OnEvent( e );
}

void EDGUIPropInt::_UpdateButton()
{
	char bfr[ 32 ] = {0};
	sgrx_snprintf( bfr, 31, "%" PRId32, m_value );
	m_button.caption = bfr;
}


EDGUIPropFloat::EDGUIPropFloat( float def, int prec, float min, float max ) :
	m_value( def ),
	m_min( min ),
	m_max( max ),
	m_numWheel( this, min, max, -prec, ceil( log( TMAX( -min, max ) ) / log( 10.0f ) ) + prec + 1 )
{
	tyname = "property-float";
	type = EDGUI_ITEM_PROP_FLOAT;
	_UpdateButton();
	Add( &m_button );
}

int EDGUIPropFloat::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
	case EDGUI_EVENT_PROPCHANGE:
		if( e->target == &m_numWheel )
		{
			m_value = m_numWheel.GetValue();
			_UpdateButton();
			EDGUIEvent se = { e->type, this };
			BubblingEvent( &se );
			return 0;
		}
		break;
		
	case EDGUI_EVENT_BTNCLICK:
		_Begin( e );
		if( Hit( e->mouse.x, e->mouse.y ) )
		{
			m_numWheel.m_cx = e->mouse.x;
			m_numWheel.m_cy = e->mouse.y;
			m_numWheel.m_value = m_value;
			m_frame->Add( &m_numWheel );
			m_frame->_HandleMouseMove( false );
		}
		_End( e );
		return 1;
	}
	return EDGUIProperty::OnEvent( e );
}

void EDGUIPropFloat::_UpdateButton()
{
	char bfr[ 32 ] = {0};
	sgrx_snprintf( bfr, 31, "%g", m_value );
	m_button.caption = bfr;
}


EDGUIPropVec2::EDGUIPropVec2( const Vec2& def, int prec, const Vec2& min, const Vec2& max ) :
	m_value( def ),
	m_min( min ),
	m_max( max ),
	m_XnumWheel( this, min.x, max.x, -prec, ceil( log( TMAX( -min.x, max.x ) ) / log( 10.0f ) ) + prec + 1 ),
	m_YnumWheel( this, min.y, max.y, -prec, ceil( log( TMAX( -min.y, max.y ) ) / log( 10.0f ) ) + prec + 1 )
{
	tyname = "property-vec2";
	type = EDGUI_ITEM_PROP_VEC2;
	_UpdateButton();
	Add( &m_buttonlist );
	m_buttonlist.Add( &m_Xbutton );
	m_buttonlist.Add( &m_Ybutton );
}

int EDGUIPropVec2::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
	case EDGUI_EVENT_PROPCHANGE:
		if( e->target == &m_XnumWheel || e->target == &m_YnumWheel )
		{
			if( e->target == &m_XnumWheel ) m_value.x = m_XnumWheel.GetValue();
			if( e->target == &m_YnumWheel ) m_value.y = m_YnumWheel.GetValue();
			_UpdateButton();
			EDGUIEvent se = { e->type, this };
			BubblingEvent( &se );
			return 0;
		}
		break;
		
	case EDGUI_EVENT_BTNCLICK:
		_Begin( e );
		if( Hit( e->mouse.x, e->mouse.y ) )
		{
			if( e->target == &m_Xbutton )
			{
				m_XnumWheel.m_cx = e->mouse.x;
				m_XnumWheel.m_cy = e->mouse.y;
				m_XnumWheel.m_value = m_value.x;
				m_frame->Add( &m_XnumWheel );
				m_frame->_HandleMouseMove( false );
			}
			if( e->target == &m_Ybutton )
			{
				m_YnumWheel.m_cx = e->mouse.x;
				m_YnumWheel.m_cy = e->mouse.y;
				m_YnumWheel.m_value = m_value.y;
				m_frame->Add( &m_YnumWheel );
				m_frame->_HandleMouseMove( false );
			}
		}
		_End( e );
		return 1;
	}
	return EDGUIProperty::OnEvent( e );
}

EDGUIPropVec2& EDGUIPropVec2::operator = ( const EDGUIPropVec2& o )
{
	this->~EDGUIPropVec2();
	new (this) EDGUIPropVec2( o );
	SubstChildPtr( &o.m_buttonlist, &m_buttonlist );
	m_buttonlist.SubstChildPtr( &o.m_Xbutton, &m_Xbutton );
	m_buttonlist.SubstChildPtr( &o.m_Ybutton, &m_Ybutton );
	m_XnumWheel.m_owner = this;
	m_YnumWheel.m_owner = this;
	return *this;
}

void EDGUIPropVec2::_UpdateButton()
{
	char bfr[ 32 ] = {0};
	sgrx_snprintf( bfr, 31, "%g", m_value.x );
	m_Xbutton.caption = bfr;
	sgrx_snprintf( bfr, 31, "%g", m_value.y );
	m_Ybutton.caption = bfr;
}


EDGUIPropVec3::EDGUIPropVec3( const Vec3& def, int prec, const Vec3& min, const Vec3& max ) :
	m_value( def ),
	m_min( min ),
	m_max( max ),
	m_XnumWheel( this, min.x, max.x, -prec, ceil( log( TMAX( -min.x, max.x ) ) / log( 10.0f ) ) + prec + 1 ),
	m_YnumWheel( this, min.y, max.y, -prec, ceil( log( TMAX( -min.y, max.y ) ) / log( 10.0f ) ) + prec + 1 ),
	m_ZnumWheel( this, min.z, max.z, -prec, ceil( log( TMAX( -min.z, max.z ) ) / log( 10.0f ) ) + prec + 1 )
{
	tyname = "property-vec3";
	type = EDGUI_ITEM_PROP_VEC3;
	_UpdateButton();
	Add( &m_buttonlist );
	m_buttonlist.Add( &m_Xbutton );
	m_buttonlist.Add( &m_Ybutton );
	m_buttonlist.Add( &m_Zbutton );
}

int EDGUIPropVec3::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
	case EDGUI_EVENT_PROPCHANGE:
		if( e->target == &m_XnumWheel || e->target == &m_YnumWheel || e->target == &m_ZnumWheel )
		{
			if( e->target == &m_XnumWheel ) m_value.x = m_XnumWheel.GetValue();
			if( e->target == &m_YnumWheel ) m_value.y = m_YnumWheel.GetValue();
			if( e->target == &m_ZnumWheel ) m_value.z = m_ZnumWheel.GetValue();
			_UpdateButton();
			EDGUIEvent se = { e->type, this };
			BubblingEvent( &se );
			return 0;
		}
		break;
		
	case EDGUI_EVENT_BTNCLICK:
		_Begin( e );
		if( Hit( e->mouse.x, e->mouse.y ) )
		{
			if( e->target == &m_Xbutton )
			{
				m_XnumWheel.m_cx = e->mouse.x;
				m_XnumWheel.m_cy = e->mouse.y;
				m_XnumWheel.m_value = m_value.x;
				m_frame->Add( &m_XnumWheel );
				m_frame->_HandleMouseMove( false );
			}
			if( e->target == &m_Ybutton )
			{
				m_YnumWheel.m_cx = e->mouse.x;
				m_YnumWheel.m_cy = e->mouse.y;
				m_YnumWheel.m_value = m_value.y;
				m_frame->Add( &m_YnumWheel );
				m_frame->_HandleMouseMove( false );
			}
			if( e->target == &m_Zbutton )
			{
				m_ZnumWheel.m_cx = e->mouse.x;
				m_ZnumWheel.m_cy = e->mouse.y;
				m_ZnumWheel.m_value = m_value.z;
				m_frame->Add( &m_ZnumWheel );
				m_frame->_HandleMouseMove( false );
			}
		}
		_End( e );
		return 1;
	}
	return EDGUIProperty::OnEvent( e );
}

EDGUIPropVec3& EDGUIPropVec3::operator = ( const EDGUIPropVec3& o )
{
	this->~EDGUIPropVec3();
	new (this) EDGUIPropVec3( o );
	SubstChildPtr( &o.m_buttonlist, &m_buttonlist );
	m_buttonlist.SubstChildPtr( &o.m_Xbutton, &m_Xbutton );
	m_buttonlist.SubstChildPtr( &o.m_Ybutton, &m_Ybutton );
	m_buttonlist.SubstChildPtr( &o.m_Zbutton, &m_Zbutton );
	m_XnumWheel.m_owner = this;
	m_YnumWheel.m_owner = this;
	m_ZnumWheel.m_owner = this;
	return *this;
}

void EDGUIPropVec3::_UpdateButton()
{
	char bfr[ 32 ] = {0};
	sgrx_snprintf( bfr, 31, "%g", m_value.x );
	m_Xbutton.caption = bfr;
	sgrx_snprintf( bfr, 31, "%g", m_value.y );
	m_Ybutton.caption = bfr;
	sgrx_snprintf( bfr, 31, "%g", m_value.z );
	m_Zbutton.caption = bfr;
}


EDGUIPropVec4::EDGUIPropVec4( const Vec4& def, int prec, const Vec4& min, const Vec4& max ) :
	m_value( def ),
	m_min( min ),
	m_max( max ),
	m_XnumWheel( this, min.x, max.x, -prec, ceil( log( TMAX( -min.x, max.x ) ) / log( 10.0f ) ) + prec + 1 ),
	m_YnumWheel( this, min.y, max.y, -prec, ceil( log( TMAX( -min.y, max.y ) ) / log( 10.0f ) ) + prec + 1 ),
	m_ZnumWheel( this, min.z, max.z, -prec, ceil( log( TMAX( -min.z, max.z ) ) / log( 10.0f ) ) + prec + 1 ),
	m_WnumWheel( this, min.w, max.w, -prec, ceil( log( TMAX( -min.w, max.w ) ) / log( 10.0f ) ) + prec + 1 )
{
	tyname = "property-vec4";
	type = EDGUI_ITEM_PROP_VEC4;
	_UpdateButton();
	Add( &m_buttonlist );
	m_buttonlist.Add( &m_Xbutton );
	m_buttonlist.Add( &m_Ybutton );
	m_buttonlist.Add( &m_Zbutton );
	m_buttonlist.Add( &m_Wbutton );
}

int EDGUIPropVec4::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
	case EDGUI_EVENT_PROPCHANGE:
		if( e->target == &m_XnumWheel || e->target == &m_YnumWheel ||
			e->target == &m_ZnumWheel || e->target == &m_WnumWheel )
		{
			if( e->target == &m_XnumWheel ) m_value.x = m_XnumWheel.GetValue();
			if( e->target == &m_YnumWheel ) m_value.y = m_YnumWheel.GetValue();
			if( e->target == &m_ZnumWheel ) m_value.z = m_ZnumWheel.GetValue();
			if( e->target == &m_WnumWheel ) m_value.w = m_WnumWheel.GetValue();
			_UpdateButton();
			EDGUIEvent se = { e->type, this };
			BubblingEvent( &se );
			return 0;
		}
		break;
		
	case EDGUI_EVENT_BTNCLICK:
		_Begin( e );
		if( Hit( e->mouse.x, e->mouse.y ) )
		{
			if( e->target == &m_Xbutton )
			{
				m_XnumWheel.m_cx = e->mouse.x;
				m_XnumWheel.m_cy = e->mouse.y;
				m_XnumWheel.m_value = m_value.x;
				m_frame->Add( &m_XnumWheel );
				m_frame->_HandleMouseMove( false );
			}
			if( e->target == &m_Ybutton )
			{
				m_YnumWheel.m_cx = e->mouse.x;
				m_YnumWheel.m_cy = e->mouse.y;
				m_YnumWheel.m_value = m_value.y;
				m_frame->Add( &m_YnumWheel );
				m_frame->_HandleMouseMove( false );
			}
			if( e->target == &m_Zbutton )
			{
				m_ZnumWheel.m_cx = e->mouse.x;
				m_ZnumWheel.m_cy = e->mouse.y;
				m_ZnumWheel.m_value = m_value.z;
				m_frame->Add( &m_ZnumWheel );
				m_frame->_HandleMouseMove( false );
			}
			if( e->target == &m_Wbutton )
			{
				m_WnumWheel.m_cx = e->mouse.x;
				m_WnumWheel.m_cy = e->mouse.y;
				m_WnumWheel.m_value = m_value.w;
				m_frame->Add( &m_WnumWheel );
				m_frame->_HandleMouseMove( false );
			}
		}
		_End( e );
		return 1;
	}
	return EDGUIProperty::OnEvent( e );
}

EDGUIPropVec4& EDGUIPropVec4::operator = ( const EDGUIPropVec4& o )
{
	this->~EDGUIPropVec4();
	new (this) EDGUIPropVec4( o );
	SubstChildPtr( &o.m_buttonlist, &m_buttonlist );
	m_buttonlist.SubstChildPtr( &o.m_Xbutton, &m_Xbutton );
	m_buttonlist.SubstChildPtr( &o.m_Ybutton, &m_Ybutton );
	m_buttonlist.SubstChildPtr( &o.m_Zbutton, &m_Zbutton );
	m_buttonlist.SubstChildPtr( &o.m_Wbutton, &m_Wbutton );
	m_XnumWheel.m_owner = this;
	m_YnumWheel.m_owner = this;
	m_ZnumWheel.m_owner = this;
	m_WnumWheel.m_owner = this;
	return *this;
}

void EDGUIPropVec4::_UpdateButton()
{
	char bfr[ 32 ] = {0};
	sgrx_snprintf( bfr, 31, "%g", m_value.x );
	m_Xbutton.caption = bfr;
	sgrx_snprintf( bfr, 31, "%g", m_value.y );
	m_Ybutton.caption = bfr;
	sgrx_snprintf( bfr, 31, "%g", m_value.z );
	m_Zbutton.caption = bfr;
	sgrx_snprintf( bfr, 31, "%g", m_value.w );
	m_Wbutton.caption = bfr;
}


EDGUIPropString::EDGUIPropString( const StringView& def ) :
	m_sel_from( 0 ),
	m_sel_to( 0 ),
	m_offset( 0 ),
	m_fsel_from( -1 ),
	m_fsel_to( -1 ),
	m_selecting( false )
{
	tyname = "property-string";
	type = EDGUI_ITEM_PROP_STRING;
	SetValue( def );
}

int EDGUIPropString::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_SETFOCUS:
		m_frame->_SetFocus( this );
		return 1;
	case EDGUI_EVENT_LOSEFOCUS:
		Changed();
		return 1;
		
	case EDGUI_EVENT_BTNDOWN:
		_Begin( e );
		if( e->mouse.button == 0 )
		{
			m_selecting = true;
			m_sel_to = m_sel_from = _FindOffset( e->mouse.x, e->mouse.y );
			_UpdateSelOffsets();
		}
		_End( e );
		break;
		
	case EDGUI_EVENT_MOUSEMOVE:
		_Begin( e );
		if( m_frame->m_clickTargets[0] == this && m_selecting )
		{
			m_sel_to = _FindOffset( e->mouse.x, e->mouse.y );
			_UpdateSelOffsets();
			Invalidate();
		}
		_End( e );
		return 1;
	case EDGUI_EVENT_BTNUP:
		if( e->mouse.button == 0 )
			m_selecting = false;
		return 1;
		
	case EDGUI_EVENT_PAINT:
		_Begin( e );
		{
			BatchRenderer& br = GR2D_GetBatchRenderer().UnsetTexture();
			br.Colu( m_frame->m_keyboardFocus == this ? EDGUI_THEME_TEXTBOX_BORDERHL_COLOR : EDGUI_THEME_TEXTBOX_BORDER_COLOR );
			br.Quad( x0, y0, x1, y1 );
			br.Colu( EDGUI_THEME_TEXTBOX_CENTER_COLOR );
			br.Quad( x0 + 2, y0 + 2, x1 - 2, y1 - 2 );
			br.Colu( EDGUI_THEME_MAIN_TEXT_COLOR );
			GR2D_DrawTextLine( x0 + 4, ( y0 + y1 ) / 2, m_value, HALIGN_LEFT, VALIGN_CENTER );
			br.Reset();
			int fsb = x0 + 4 - m_offset;
			if( m_sel_to != m_sel_from )
			{
				br.Colu( m_frame->m_keyboardFocus == this ? EDGUI_THEME_TEXTBOX_SELHL_COLOR : EDGUI_THEME_TEXTBOX_SEL_COLOR );
				br.Quad( fsb+m_fsel_from, y0 + 3, fsb+m_fsel_to, y1 - 3 );
			}
			if( m_frame->m_keyboardFocus == this )
			{
				br.Colu( EDGUI_THEME_TEXTBOX_TICK_COLOR );
				br.Quad( fsb+m_fsel_to, y0 + 3, fsb+m_fsel_to + 1, y1 - 3 );
			}
		}
		_End( e );
		return 1;
		
	case EDGUI_EVENT_KEYDOWN:
	case EDGUI_EVENT_KEYUP:
		{
			int key = e->key.key & EDGUI_KEYMOD_FILTER;
			int mods = e->key.key & ~EDGUI_KEYMOD_FILTER;
			bool down = e->type == EDGUI_EVENT_KEYDOWN;
			
			if( down )
			{
				if( key == EDGUI_KEY_LEFT || key == EDGUI_KEY_RIGHT )
				{
					if( m_sel_from == m_sel_to || mods & EDGUI_KEYMOD_SHIFT )
						m_sel_to += key == EDGUI_KEY_RIGHT ? 1 : -1;
					else
						m_sel_to = key == EDGUI_KEY_LEFT ? TMIN( m_sel_from, m_sel_to ) : TMAX( m_sel_from, m_sel_to );
					if( !( mods & EDGUI_KEYMOD_SHIFT ) )
						m_sel_from = m_sel_to;
					_UpdateSelOffsets();
				}
				else if( key == EDGUI_KEY_DELLEFT || key == EDGUI_KEY_DELRIGHT )
				{
					if( m_sel_from != m_sel_to )
					{
						int from = m_sel_from;
						int to = m_sel_to;
						if( from > to )
						{
							TSWAP( from, to );
						}
						m_chars.erase( from, to - from );
						m_sel_from = m_sel_to = from;
					}
					else if( key == EDGUI_KEY_DELLEFT && m_sel_from > 0 )
					{
						m_chars.erase( m_sel_from - 1 );
						m_sel_from = --m_sel_to;
					}
					else if( key == EDGUI_KEY_DELRIGHT && m_sel_from < (int) m_chars.size() )
					{
						m_chars.erase( m_sel_from );
					}
					_UpdateText();
					Edited();
					_UpdateSelOffsets();
				}
				else if( key == EDGUI_KEY_CUT || key == EDGUI_KEY_COPY )
				{
					int from = m_sel_from;
					int to = m_sel_to;
					if( from > to )
					{
						TSWAP( from, to );
					}
					// TODO encode
					Window_SetClipboardText( StringView( m_chars ).part( from, to - from ) );
					if( key == EDGUI_KEY_CUT )
					{
						m_chars.erase( from, to - from );
						m_sel_to = m_sel_from = from;
						_UpdateText();
						Edited();
						_UpdateSelOffsets();
					}
				}
				else if( key == EDGUI_KEY_PASTE )
				{
					String cliptext;
					if( Window_GetClipboardText( cliptext ) )
					{
						if( m_sel_from != m_sel_to )
						{
							int from = m_sel_from;
							int to = m_sel_to;
							if( from > to )
							{
								TSWAP( from, to );
							}
							m_chars.erase( from, to - from );
							m_sel_to = m_sel_from = from;
						}
						
						// TODO encode
						m_chars.insert( m_sel_from, cliptext.data(), cliptext.size() );
						m_sel_from = m_sel_to += cliptext.size();
						_UpdateText();
						Edited();
						_UpdateSelOffsets();
					}
				}
				else if( key == EDGUI_KEY_SELECTALL )
				{
					m_sel_from = 0;
					m_sel_to = m_chars.size();
					_UpdateSelOffsets();
				}
				else if( key == EDGUI_KEY_ENTER )
				{
					m_frame->_SetFocus( NULL );
				}
			}
		}
		break;
		
	case EDGUI_EVENT_TEXTINPUT:
		if( m_sel_from == m_sel_to )
		{
			m_chars.insert( m_sel_from, *e->text.text ); // TODO decode
			_UpdateText();
			Edited();
			m_sel_to = ++m_sel_from;
		}
		else
		{
			int from = m_sel_from;
			int to = m_sel_to;
			if( from > to )
			{
				TSWAP( from, to );
			}
			m_chars.erase( from, to - from );
			m_chars.insert( from, *e->text.text );
			_UpdateText();
			Edited();
			m_sel_to = m_sel_from = from + 1;
		}
		_UpdateSelOffsets();
		break;
		
	}
	return EDGUIProperty::OnEvent( e );
}

void EDGUIPropString::SetValue( const StringView& sv )
{
	m_sel_from = sv.size();
	m_sel_to = sv.size();
	m_offset = 0;
	m_fsel_from = -1;
	m_fsel_to = -1;
	m_selecting = false;
	m_value = sv;
	m_chars = sv; // TODO decode
	_UpdateSelOffsets();
}

void EDGUIPropString::_UpdateSelOffsets()
{
	m_sel_from = TMAX( 0, TMIN( m_sel_from, (int) m_chars.size() ) );
	m_sel_to = TMAX( 0, TMIN( m_sel_to, (int) m_chars.size() ) );
	m_fsel_from = GR2D_GetTextLength( StringView( m_chars.data(), m_sel_from ) );
	m_fsel_to = GR2D_GetTextLength( StringView( m_chars.data(), m_sel_to ) );
}

void EDGUIPropString::_UpdateText()
{
	m_value = m_chars; // TODO encode
}

int EDGUIPropString::_FindOffset( int x, int y )
{
	x -= x0 + 4 - m_offset;
	y -= y0;
	
//	fmin = 0.0;
//	fmax = 0.0;
//	pc = null;
//	for( i = 0; i < m_chars.size(); ++i )
//	{
//		ch = m_chars[ i ];
//		fmax += ctrl._cachedFont.getAdvance( pc, ch );
//		if( x <= (fmin+fmax)*0.5 )
//			break;
//		fmin = fmax;
//		pc = ch;
//	}
//	return i;
	
	int lenmin = 0;
	int lenmax = 0;
	int i;
	for( i = 0; i < (int) m_chars.size(); ++i )
	{
		lenmax = GR2D_GetTextLength( StringView( m_chars.data(), i + 1 ) );
		if( x <= ( lenmin + lenmax ) / 2 )
			break;
		lenmin = lenmax;
	}
	return i;
}


EDGUIPropRsrc::EDGUIPropRsrc( EDGUIRsrcPicker* rsrcPicker, const StringView& def ) :
	m_value( def ),
	m_requestReload( false ),
	m_rsrcPicker( rsrcPicker )
{
	tyname = "property-rsrc";
	type = EDGUI_ITEM_PROP_RSRC;
	_UpdateButton();
	Add( &m_button );
}

int EDGUIPropRsrc::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
	case EDGUI_EVENT_PROPCHANGE:
		if( e->target == m_rsrcPicker )
		{
			m_value = m_rsrcPicker->GetValue();
			_UpdateButton();
			EDGUIEvent se = { e->type, this };
			BubblingEvent( &se );
			return 0;
		}
		break;
		
	case EDGUI_EVENT_BTNCLICK:
		_Begin( e );
		if( Hit( e->mouse.x, e->mouse.y ) )
		{
			if( m_requestReload )
			{
				OnReload( false );
				m_rsrcPicker->Reload();
				OnReload( true );
			}
			m_rsrcPicker->Open( this, m_value );
			m_frame->Add( m_rsrcPicker );
		}
		_End( e );
		return 1;
	}
	return EDGUIProperty::OnEvent( e );
}

void EDGUIPropRsrc::_UpdateButton()
{
	m_button.caption = m_value;
}

void EDGUIPropRsrc::OnReload( bool after )
{
	UNUSED( after );
}


