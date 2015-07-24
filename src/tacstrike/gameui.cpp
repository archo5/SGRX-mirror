

#include <sound.hpp>
#include "gameui.hpp"
#include "level.hpp"


//
// GLOBAL INTERFACE
//
extern SoundSystemHandle g_SoundSys;
MenuTheme g_DefaultMenuTheme;


MenuControl::MenuControl() :
	type(MCT_Null), style(MCS_Default), x0(0), y0(0), x1(0), y1(0),
	visible(true), enabled(true), selected(false),
	id(0), group(-1), sl_value(0), sb_option(0)
{
}


MenuTheme::MenuTheme() :
	color_hl(V4( 0.9f, 0.2f, 0.1f, 1 )),
	color_sel(V4( 0.6f, 0.1f, 0.05f, 1 ))
{
}

void MenuTheme::DrawControl( const MenuControl& ctrl, const MenuCtrlInfo& info )
{
	if( ctrl.type == MCT_Slider )
	{
		DrawDefaultSlider( ctrl, info );
	}
	else
	{
		if( ctrl.style == MCS_Link )
			DrawLinkButton( ctrl, info );
		else
			DrawDefaultButton( ctrl, info );
	}
}

void MenuTheme::_GetCtrlColors( const MenuControl& ctrl, const MenuCtrlInfo& info, bool link, Colors& col )
{
	col.bgcol = V4( 0.1f, 0.1f, 0.1f, 0 );
	col.fgcol = V4( 0.8f, 0.8f, 0.8f, 1 );
	if( ctrl.enabled == false )
	{
		col.fgcol.w = 0.5f;
	}
	if( link )
	{
		if( info.highlighted )
		{
			col.fgcol = color_hl;
		}
		if( info.selected )
		{
			col.fgcol = color_sel;
		}
	}
	else
	{
		if( info.selected || ( info.anysel == false && info.highlighted ) )
			col.bgcol = color_sel * V4( 1, 1, 1, 0.5f );
	}
	
	float ctlalpha = info.selected || ( info.anysel == false && info.highlighted ) ? 1.0f : 0.9f;
	col.opacity = ctlalpha * info.menu->opacity;
}

void MenuTheme::_DrawButtonCore( const MenuControl& ctrl, const MenuCtrlInfo& info, const Colors& col )
{
	MENUTHEME_PREP;
	GR2D_SetFont( "core", TMAX(0.0f, fabsf(ay0 - ay1) * 0.2f + info.minw / 50) * 0.8f );
	
	br.Reset().Col( col.bgcol.x, col.bgcol.y, col.bgcol.z, col.bgcol.w * col.opacity )
		.Quad( ax0, ay0, ax1, ay1 );
	br.Reset().Col( col.fgcol.x, col.fgcol.y, col.fgcol.z, col.fgcol.w * col.opacity );
	GR2D_DrawTextLine(
		round((ax0+ax1)/2 - fabsf(ay0-ay1)/4),
		round((ay0+ay1)/2),
		ctrl.caption, HALIGN_RIGHT, VALIGN_CENTER );
}

void MenuTheme::DrawDefaultButton( const MenuControl& ctrl, const MenuCtrlInfo& info )
{
	MENUTHEME_PREP;
	Colors col;
	_GetCtrlColors( ctrl, info, false, col );
	
	_DrawButtonCore( ctrl, info, col );
	
	float fac = info.selected ? 0.6f : ( info.highlighted ? 0.9f : 0.8f );
	float MG = fabsf(ay0-ay1)/8;
	float BR = MG / 2;
	float mx0 = (ax0+ax1) * 0.5f;
	br.Reset().Col( fac, col.opacity )
		.QuadFrame( mx0+MG, ay0+MG, ax1-MG, ay1-MG, mx0+MG+BR, ay0+MG+BR, ax1-MG-BR, ay1-MG-BR );
	GR2D_DrawTextLine(
		round(TLERP( ax0, ax1, 0.75f )),
		round((ay0+ay1)/2),
		ctrl.value_text, HALIGN_CENTER, VALIGN_CENTER );
}

void MenuTheme::DrawLinkButton( const MenuControl& ctrl, const MenuCtrlInfo& info )
{
	MENUTHEME_PREP;
	Colors col;
	_GetCtrlColors( ctrl, info, true, col );
	
	GR2D_SetFont( "core", info.minw / 30 );
	
	br.Reset().Col( col.fgcol.x, col.fgcol.y, col.fgcol.z, col.fgcol.w * info.menu->opacity );
	GR2D_DrawTextLine(
		round((ax0+ax1)/2),
		round((ay0+ay1)/2),
		ctrl.caption, HALIGN_CENTER, VALIGN_CENTER );
}

void MenuTheme::DrawDefaultSlider( const MenuControl& ctrl, const MenuCtrlInfo& info )
{
	MENUTHEME_PREP;
	Colors col;
	_GetCtrlColors( ctrl, info, false, col );
	
	_DrawButtonCore( ctrl, info, col );
	
	float MG = fabsf(ay0-ay1)/4;
	float BR = MG / 4;
	float IMG = MG + BR * 2;
	float mx0 = (ax0+ax1) * 0.5f;
	br.Reset().Col( 0.8f, col.opacity )
		.QuadFrame( mx0+MG, ay0+MG, ax1-MG, ay1-MG, mx0+MG+BR, ay0+MG+BR, ax1-MG-BR, ay1-MG-BR );
	br.Reset().Col( 0.8f, col.opacity )
		.Quad( mx0+IMG, ay0+IMG, TLERP( mx0+IMG, ax1-IMG, ctrl.sl_value ), ay1-IMG );
	if( ctrl.value_text.size() )
	{
		GR2D_SetFont( "core", TMAX(0.0f, fabsf(ay0 - ay1) * 0.2f + info.minw / 50) * 0.6f );
		
		GR2D_SetScissorRect( mx0+IMG, ay0+IMG, TLERP( mx0+IMG, ax1-IMG, ctrl.sl_value ), ay1-IMG );
		br.Reset().Col( 0.0f, col.opacity );
		GR2D_DrawTextLine( round(TLERP( ax0, ax1, 0.75f )), round((ay0+ay1)/2), ctrl.value_text, HALIGN_CENTER, VALIGN_CENTER );
		
		GR2D_SetScissorRect( TLERP( mx0+IMG, ax1-IMG, ctrl.sl_value ), ay0+IMG, ax1-IMG, ay1-IMG );
		br.Reset().Col( 0.8f, col.opacity );
		GR2D_DrawTextLine( round(TLERP( ax0, ax1, 0.75f )), round((ay0+ay1)/2), ctrl.value_text, HALIGN_CENTER, VALIGN_CENTER );
		
		GR2D_UnsetScissorRect();
	}
}


ScreenMenu::ScreenMenu() :
	theme(&g_DefaultMenuTheme),
	opacity(1), m_HL(-1), m_lastHL(-1), m_selected(-1)
{
}

ScreenMenu::~ScreenMenu()
{
}

void ScreenMenu::OnStart()
{
}

void ScreenMenu::OnEnd()
{
}

int ScreenMenu::UpdateCtrl( CtrlEvent evtype )
{
	if( m_selected < 0 )
		return -1;
	
	MenuControl& CTRL = controls[ m_selected ];
	if( CTRL.type == MCT_Slider )
	{
		float MG = fabsf(CTRL.y0-CTRL.y1)/4;
		float BR = MG / 4;
		float IMG = MG + BR * 2;
		float mx0 = (CTRL.x0+CTRL.x1) * 0.5f;
		CTRL.sl_value = clamp( TREVLERP<float>( mx0+IMG, CTRL.x1-IMG, m_mouseX ), 0, 1 );
		return m_selected;
	}
	return -1;
}

int ScreenMenu::OnEvent( const Event& e )
{
	int ret = -1;
	switch( e.type )
	{
	case SDL_MOUSEBUTTONDOWN:
		{
			m_selected = m_HL;
			ret = UpdateCtrl( EV_MBDown );
			if( m_selected >= 0 )
				g_SoundSys->CreateEventInstance( "/ui/buttondown" )->Start();
		}
		break;
	case SDL_MOUSEBUTTONUP:
		ret = UpdateCtrl( EV_MBUp );
		if( m_selected >= 0 && m_HL == m_selected )
		{
			for( int i = 0; i < (int) controls.size(); ++i )
			{
				if( i != m_selected && controls[ i ].group == controls[ m_selected ].group )
					controls[ i ].selected = false;
			}
			controls[ m_selected ].selected = true;
			ret = m_selected;
		}
		if( m_selected >= 0 )
			g_SoundSys->CreateEventInstance( "/ui/buttonup" )->Start();
		m_selected = -1;
		break;
	case SDL_MOUSEMOTION:
		m_mouseX = TREVLERP<float>( x0, x1, (float) e.motion.x );
		m_mouseY = TREVLERP<float>( y0, y1, (float) e.motion.y );
		FindHL( m_mouseX, m_mouseY );
		ret = UpdateCtrl( EV_MouseMove );
		break;
	}
	return ret;
}

void ScreenMenu::Draw( float delta )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	
	for( int i = 0; i < (int) controls.size(); ++i )
	{
		MenuControl& ctrl = controls[ i ];
		if( ctrl.visible == false )
			continue;
		
		float ax0 = TLERP( x0, x1, ctrl.x0 );
		float ax1 = TLERP( x0, x1, ctrl.x1 );
		float ay0 = TLERP( y0, y1, ctrl.y0 );
		float ay1 = TLERP( y0, y1, ctrl.y1 );
		
		MenuCtrlInfo mcinfo =
		{
			ax0, ay0, ax1, ay1,
			i, i == m_HL, ctrl.selected || i == m_selected, m_selected >= 0,
			this,
			TMIN( x1 - x0, y1 - y0 ),
		};
		theme->DrawControl( ctrl, mcinfo );
	}
}

void ScreenMenu::FindHL( float x, float y )
{
	int oldhl = m_HL;
	m_HL = -1;
	for( int i = 0; i < (int) controls.size(); ++i )
	{
		MenuControl& CTRL = controls[ i ];
		if( CTRL.x0 <= x && CTRL.x1 >= x &&
			CTRL.y0 <= y && CTRL.y1 >= y )
		{
			m_HL = i;
		}
	}
	if( m_HL >= 0 && m_HL != oldhl )
	{
		g_SoundSys->CreateEventInstance( "/ui/hover" )->Start();
	}
	m_lastHL = m_HL;
}

void ScreenMenu::RecalcSize( float w, float h, float aspect )
{
	if( h == 0 )
		return;
	float screenasp = w / h;
	
	float tw = w;
	float th = h;
	if( aspect > screenasp ) // based on width
	{
		th = tw / aspect;
	}
	else // based on height
	{
		tw = th * aspect;
	}
	float mx = floor( ( w - tw ) / 2 );
	float my = floor( ( h - th ) / 2 );
	x0 = mx;
	y0 = my;
	x1 = x0 + tw;
	y1 = y0 + th;
}

void ScreenMenu::AddButton( const StringView& caption, int style, float x0, float y0, float x1, float y1, int id )
{
	controls.push_back( MenuControl() );
	MenuControl& ctrl = controls.last();
	ctrl.type = MCT_Button;
	ctrl.style = style;
	ctrl.caption = caption;
	ctrl.x0 = x0;
	ctrl.y0 = y0;
	ctrl.x1 = x1;
	ctrl.y1 = y1;
	ctrl.id = id;
}

void ScreenMenu::AddRadioBtn( const StringView& caption, int style, float x0, float y0, float x1, float y1, int group, int id )
{
	controls.push_back( MenuControl() );
	MenuControl& ctrl = controls.last();
	ctrl.type = MCT_RadioBtn;
	ctrl.style = style;
	ctrl.caption = caption;
	ctrl.x0 = x0;
	ctrl.y0 = y0;
	ctrl.x1 = x1;
	ctrl.y1 = y1;
	ctrl.group = group;
	ctrl.id = id;
}

void ScreenMenu::AddSlider( const StringView& caption, int style, float x0, float y0, float x1, float y1, int id )
{
	controls.push_back( MenuControl() );
	MenuControl& ctrl = controls.last();
	ctrl.type = MCT_Slider;
	ctrl.style = style;
	ctrl.caption = caption;
	ctrl.x0 = x0;
	ctrl.y0 = y0;
	ctrl.x1 = x1;
	ctrl.y1 = y1;
	ctrl.id = id;
}

void ScreenMenu::Clear()
{
	controls.clear();
	m_selected = -1;
	m_HL = -1;
}

int ScreenMenu::GetCountInGroup( int group )
{
	int count = 0;
	for( size_t i = 0; i < controls.size(); ++i )
		if( controls[ i ].group == group )
			count++;
	return count;
}

int ScreenMenu::GetSelectedInGroup( int group )
{
	int at = 0;
	for( size_t i = 0; i < controls.size(); ++i )
	{
		if( controls[ i ].group == group )
		{
			if( controls[ i ].selected )
				return at;
			at++;
		}
	}
	return -1;
}

int ScreenMenu::SelectInGroup( int group, int which )
{
	int at = 0, ret = -1;
	for( size_t i = 0; i < controls.size(); ++i )
	{
		if( controls[ i ].group == group )
		{
			controls[ i ].selected = false;
			if( which == at )
			{
				controls[ i ].selected = true;
				ret = i;
			}
			at++;
		}
	}
	return ret;
}

int ScreenMenu::SelectNextInGroup( int group )
{
	int count = GetCountInGroup( group );
	int which = GetSelectedInGroup( group );
	if( count )
		return SelectInGroup( group, ( TMAX( 0, TMIN( count - 1, which ) ) + 1 ) % count );
	else
		return SelectInGroup( group, -1 );
}

int ScreenMenu::SelectPrevInGroup( int group )
{
	int count = GetCountInGroup( group );
	int which = GetSelectedInGroup( group );
	if( count )
		return SelectInGroup( group, ( TMAX( 0, TMIN( count - 1, which ) ) + count - 1 ) % count );
	else
		return SelectInGroup( group, -1 );
}


