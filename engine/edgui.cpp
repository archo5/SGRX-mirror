

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
	x0( 0 ), y0( 0 ), x1( 0 ), y1( 0 ),
	m_mouseOn( false ),
	m_clicked( false )
{
}

EDGUIItem::~EDGUIItem()
{
}

int EDGUIItem::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PAINT:
		if( backColor )
		{
			GR2D_GetBatchRenderer().UnsetTexture().Colu( backColor ).Quad( x0, y0, x1, y1 );
		}
		if( textColor && caption.size() )
		{
			GR2D_GetBatchRenderer().Colu( textColor );
			GR2D_DrawTextLine( ( x0 + x1 ) / 2, ( y0 + y1 ) / 2, caption, HALIGN_CENTER, VALIGN_CENTER );
		}
		for( size_t i = 0; i < m_subitems.size(); ++i )
		{
			m_subitems[ i ]->OnEvent( e );
		}
		return 1;
		
	case EDGUI_EVENT_LAYOUT:
		SetRectFromEvent( e, true );
		return 1;
		
	case EDGUI_EVENT_HITTEST:
		return e->mouse.x >= x0 && e->mouse.x < x1 && e->mouse.y >= y0 && e->mouse.y < y1;
		
	case EDGUI_EVENT_MOUSEENTER: m_mouseOn = true; Invalidate(); return 1;
	case EDGUI_EVENT_MOUSELEAVE: m_mouseOn = false; Invalidate(); return 1;
	case EDGUI_EVENT_BTNDOWN: if( e->mouse.button == 0 ) m_clicked = true; Invalidate(); return 1;
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
	
	OnChangeLayout();
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
	
	OnChangeLayout();
	return true;
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
	LOG << "OC LAYOUT: " << tyname << LOG_SEP(", ") << x0 << y0 << x1 << y1;
	OnEvent( &e );
}

void EDGUIItem::SetSubitemLayout( EDGUIItem* subitem, int _x0, int _y0, int _x1, int _y1 )
{
	LOG << "SUB LAYOUT: " << tyname << " -> " << subitem->tyname << LOG_SEP(", ") << _x0 << _y0 << _x1 << _y1;
	EDGUIEvent e = { EDGUI_EVENT_LAYOUT, this };
	e.layout.x0 = _x0;
	e.layout.y0 = _y0;
	e.layout.x1 = _x1;
	e.layout.y1 = _y1;
	subitem->OnEvent( &e );
}

void EDGUIItem::_SetFrame( EDGUIFrame* frame )
{
	m_frame = frame;
	for( size_t i = 0; i < m_subitems.size(); ++i )
		m_subitems[ i ]->_SetFrame( frame );
}

void EDGUIItem::Edited()
{
	EDGUIEvent ev = { EDGUI_EVENT_PROPEDIT, this };
	BubblingEvent( &ev );
}

void EDGUIItem::Changed()
{
	EDGUIEvent ev = { EDGUI_EVENT_PROPCHANGE, this };
	BubblingEvent( &ev );
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
	else if( eev->type == SDL_MOUSEBUTTONUP || eev->type == SDL_MOUSEBUTTONDOWN )
	{
		int btn = eev->button.button;
		if( btn == 1 ) btn = 0;
		else if( btn == 3 ) btn = 1;
		else if( btn == 2 ) btn = 2;
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
					m_clickTargets[ btn ]->BubblingEvent( &ev );
					m_clickTargets[ btn ] = NULL;
				}
				_HandleMouseMove( true );
			}
			else if( m_hover )
			{
				m_clickTargets[ btn ] = m_hover;
				m_clickOffsets[ btn ][0] = m_mouseX - m_hover->x0;
				m_clickOffsets[ btn ][1] = m_mouseY - m_hover->y0;
				m_hover->BubblingEvent( &ev );
			}
		}
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
				EDGUIItem *cc, *pcc1, *pcc2;
				
				e.type = EDGUI_EVENT_MOUSELEAVE;
				e.target = prevhover;
				e.mouse.x = m_mouseX;
				e.mouse.y = m_mouseY;
				
				pcc1 = cc = prevhover;
				while( cc != phi )
				{
					if( !cc->OnEvent( &e ) )
						break;
					pcc1 = cc;
					cc = cc->m_parent;
				}
				
				e.type = EDGUI_EVENT_MOUSEENTER;
				e.target = m_hover;
				e.mouse.x = m_mouseX;
				e.mouse.y = m_mouseY;
				
				pcc2 = cc = m_hover;
				while( cc != phi )
				{
					if( !cc->OnEvent( &e ) )
						break;
					pcc2 = cc;
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
				int cx = round( x0 * (1-m_splitfac) + x1 * m_splitfac ) + m_splitoff;
				int cx0 = cx - EDGUI_THEME_SPLITPANE_BORDER_SIZE / 2;
				int cx1 = cx0 + EDGUI_THEME_SPLITPANE_BORDER_SIZE;
				
				if( m_first ) SetSubitemLayout( m_first, x0, y0, cx0, y1 );
				if( m_second ) SetSubitemLayout( m_second, cx1, y0, x1, y1 );
			}
			else
			{
				int cy = round( y0 * (1-m_splitfac) + y1 * m_splitfac ) + m_splitoff;
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
				int cx = round( x0 * (1-m_splitfac) + x1 * m_splitfac ) + m_splitoff;
				int cx0 = cx - EDGUI_THEME_SPLITPANE_BORDER_SIZE / 2;
				int cx1 = cx0 + EDGUI_THEME_SPLITPANE_BORDER_SIZE;
				GR2D_GetBatchRenderer().UnsetTexture().Colu( backColor ).Quad( cx0, y0, cx1, y1 );
			}
			else
			{
				int cy = round( y0 * (1-m_splitfac) + y1 * m_splitfac ) + m_splitoff;
				int cy0 = cy - EDGUI_THEME_SPLITPANE_BORDER_SIZE / 2;
				int cy1 = cy0 + EDGUI_THEME_SPLITPANE_BORDER_SIZE;
				GR2D_GetBatchRenderer().UnsetTexture().Colu( backColor ).Quad( x0, cy0, x1, cy1 );
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


EDGUILabel::EDGUILabel()
{
	type = EDGUI_ITEM_LABEL;
	tyname = "label";
}

int EDGUILabel::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_LAYOUT:
		x0 = e->layout.x0;
		x1 = e->layout.x1;
		y0 = e->layout.y0;
		y1 = y0 + EDGUI_THEME_LABEL_HEIGHT;
		return 1;
	}
	return EDGUIItem::OnEvent( e );
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
	case EDGUI_EVENT_BTNDOWN:
	case EDGUI_EVENT_BTNUP:
		EDGUIItem::OnEvent( e );
		OnChangeState();
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
	case EDGUI_EVENT_BTNCLICK:
		{
			int prbx = e->mouse.x - m_cx;
			int prby = e->mouse.y - m_cy;
			float dist = sqrt( prbx * prbx + prby * prby );
			if( dist < EDGUI_THEME_NUMWHEEL_CENTER_SIZE && m_curWheel < 0 )
			{
				if( m_owner )
					m_owner->Changed();
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
				float dist = sqrt( prbx * prbx + prby * prby ) - EDGUI_THEME_NUMWHEEL_CENTER_SIZE;
				dist /= EDGUI_THEME_NUMWHEEL_WHEEL_SIZE;
				m_curWheel = floor( dist );
			}
			if( m_clicked && m_curWheel >= 0 && m_curWheel < m_numwheels )
			{
				float pa = atan2( m_prevMouseY - m_cy, m_prevMouseX - m_cx );
				float ca = atan2( e->mouse.y - m_cy  , e->mouse.x - m_cx   );
				if( pa - ca > M_PI ) pa -= M_PI * 2;
				if( ca - pa > M_PI ) ca -= M_PI * 2;
				float diff = ca - pa;
				
				double prevval = GetValue();
				m_value += diff * pow( 10, m_initpwr + m_curWheel );
				if( m_value < m_min ) m_value = m_min;
				if( m_value > m_max ) m_value = m_max;
				if( GetValue() != prevval && m_owner )
					m_owner->Edited();
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
				snprintf( bfr, 31, "%g", pow( 10, m_initpwr + m_curWheel ) );
				GR2D_DrawTextLine( m_cx + xfac * rad, m_cy + yfac * rad, bfr );
			}
			snprintf( bfr, 31, "%g", GetValue() );
			GR2D_DrawTextLine( m_cx, m_cy, bfr, HALIGN_CENTER, VALIGN_CENTER );
		}
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}

double EDGUINumberWheel::GetValue()
{
	double rf = pow( 10, m_initpwr );
	return round( m_value / rf ) * rf;
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
	_UpdateButton();
	Add( &m_button );
}

int EDGUIPropInt::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
	case EDGUI_EVENT_PROPCHANGE:
		m_value = m_numWheel.GetValue();
		_UpdateButton();
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
	snprintf( bfr, 31, "%" PRId32, m_value );
	m_button.caption = bfr;
}


EDGUIPropFloat::EDGUIPropFloat( float def, int prec, float min, float max ) :
	m_value( def ),
	m_min( min ),
	m_max( max ),
	m_numWheel( this, min, max, -prec, ceil( log( TMAX( -min, max ) ) / log( 10.0f ) ) + prec + 1 )
{
	_UpdateButton();
	Add( &m_button );
}

int EDGUIPropFloat::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
	case EDGUI_EVENT_PROPCHANGE:
		m_value = m_numWheel.GetValue();
		_UpdateButton();
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

void EDGUIPropFloat::_UpdateButton()
{
	char bfr[ 32 ] = {0};
	snprintf( bfr, 31, "%g", m_value );
	m_button.caption = bfr;
}


