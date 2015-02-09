

#define USE_ARRAY
#include "edgui.hpp"


#define EDGUI_THEME_MAIN_BACK_COLOR COLOR_RGBA( 40, 40, 40, 255 )
#define EDGUI_THEME_MAIN_TEXT_COLOR COLOR_RGBA( 220, 220, 220, 255 )
#define EDGUI_THEME_BUTTON_HEIGHT 24
#define EDGUI_THEME_BUTTON_BACK_COLOR COLOR_RGBA( 60, 60, 60, 255 )
#define EDGUI_THEME_BUTTON_BACK_COLOR_MOUSEON COLOR_RGBA( 70, 70, 70, 255 )
#define EDGUI_THEME_BUTTON_BACK_COLOR_CLICKED COLOR_RGBA( 30, 30, 30, 255 )
#define EDGUI_THEME_BUTTON_TEXT_COLOR COLOR_RGBA( 240, 240, 240, 255 )

#define DOUBLE_CLICK_MSEC 500



EDGUIItem::EDGUIItem() :
	type( EDGUI_ITEM_NULL ),
	uid( 0 ),
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
			GR2D_DrawTextLine( x0, y0, caption );
		}
		for( size_t i = 0; i < m_subitems.size(); ++i )
		{
			m_subitems[ i ]->OnEvent( e );
		}
		return 1;
		
	case EDGUI_EVENT_LAYOUT:
		Invalidate();
		x0 = e->layout.x0;
		y0 = e->layout.y0;
		x1 = e->layout.x1;
		y1 = e->layout.y1;
		Invalidate();
		for( size_t i = 0; i < m_subitems.size(); ++i )
		{
			m_subitems[ i ]->OnEvent( e );
		}
		return 1;
		
	case EDGUI_EVENT_HITTEST:
		return e->mouse.x >= x0 && e->mouse.x < x1 && e->mouse.y >= y0 && e->mouse.y < y1;
		
	case EDGUI_EVENT_MOUSEENTER: m_mouseOn = true; Invalidate(); return 1;
	case EDGUI_EVENT_MOUSELEAVE: m_mouseOn = false; Invalidate(); return 1;
	case EDGUI_EVENT_BTNDOWN: if( e->mouse.button == 0 ) m_clicked = true; Invalidate(); return 1;
	case EDGUI_EVENT_BTNUP: if( e->mouse.button == 0 ) m_clicked = false; Invalidate(); return 1;
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
	
	EDGUIEvent e = { EDGUI_EVENT_LAYOUT };
	e.layout.x0 = x0;
	e.layout.y0 = y0;
	e.layout.x1 = x1;
	e.layout.y1 = y1;
	OnEvent( &e );
	
	return true;
}

bool EDGUIItem::Remove( EDGUIItem* subitem )
{
	size_t pos = m_subitems.find_first_at( subitem );
	if( this != subitem->m_parent || pos == NOT_FOUND )
		return false;
	m_subitems.erase( pos );
	subitem->m_parent = NULL;
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
	
	EDGUIEvent e = { EDGUI_EVENT_LAYOUT };
	e.layout.x0 = x0;
	e.layout.y0 = y0;
	e.layout.x1 = x1;
	e.layout.y1 = y1;
	OnEvent( &e );
}

void EDGUIFrame::Draw()
{
	EDGUIEvent ev = { EDGUI_EVENT_PAINT };
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
					EDGUIEvent e = { EDGUI_EVENT_MOUSELEAVE };
					e.mouse.x = m_mouseX;
					e.mouse.y = m_mouseY;
					pphi->BubblingEvent( &e );
				}
				if( m_hover )
				{
					EDGUIEvent e = { EDGUI_EVENT_MOUSEENTER };
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


int EDGUILayoutRow::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_LAYOUT:
		{
			EDGUIItem::OnEvent( e );
			int at = y0;
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				EDGUIEvent se = { EDGUI_EVENT_LAYOUT };
				se.layout.x0 = x0;
				se.layout.x1 = x1;
				se.layout.y0 = at;
				se.layout.y1 = at;
				m_subitems[ i ]->OnEvent( &se );
				at = m_subitems[ i ]->y1;
			}
			y1 = at;
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT };
			m_parent->OnEvent( &se );
		}
		return 1;
		
	case EDGUI_EVENT_POSTLAYOUT:
		{
			y1 = m_subitems.size() ? m_subitems.last()->y1 : y0;
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT };
			m_parent->OnEvent( &se );
		}
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}


int EDGUILayoutColumn::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_LAYOUT:
		{
			EDGUIItem::OnEvent( e );
			if( m_subitems.size() )
			{
				int pat = x0;
				int delta = ( x1 - x0 ) * 65536 / m_subitems.size();
				int at = x0 * 65536 + delta;
				y1 = y0;
				for( size_t i = 0; i < m_subitems.size(); ++i )
				{
					int cur = at / 65536;
					EDGUIEvent se = { EDGUI_EVENT_LAYOUT };
					se.layout.x0 = pat;
					se.layout.x1 = cur;
					se.layout.y0 = y0;
					se.layout.y1 = y0;
					m_subitems[ i ]->OnEvent( &se );
					
					pat = cur;
					at += delta;
					if( m_subitems[ i ]->y1 > y1 )
						y1 = m_subitems[ i ]->y1;
				}
			}
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT };
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
			EDGUIEvent se = { EDGUI_EVENT_POSTLAYOUT };
			m_parent->OnEvent( &se );
		}
		return 1;
	}
	return EDGUIItem::OnEvent( e );
}


EDGUIButton::EDGUIButton()
{
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
	backColor = m_clicked ?
		EDGUI_THEME_BUTTON_BACK_COLOR_CLICKED :
		( m_mouseOn ? EDGUI_THEME_BUTTON_BACK_COLOR_MOUSEON : EDGUI_THEME_BUTTON_BACK_COLOR );
	Invalidate();
}


