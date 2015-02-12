

#pragma once
#define __STDC_FORMAT_MACROS 1
#include <float.h>
#include "engine.hpp"


// - core
#define EDGUI_THEME_OVERLAY_COLOR COLOR_RGBA( 0, 0, 0, 64 )
#define EDGUI_THEME_MAIN_BACK_COLOR COLOR_RGBA( 40, 40, 40, 255 )
#define EDGUI_THEME_MAIN_TEXT_COLOR COLOR_RGBA( 220, 220, 220, 255 )
// - label
#define EDGUI_THEME_LABEL_HEIGHT 24
// - button
#define EDGUI_THEME_BUTTON_HEIGHT 24
#define EDGUI_THEME_BUTTON_BACK_COLOR COLOR_RGBA( 60, 60, 60, 255 )
#define EDGUI_THEME_BUTTON_BACK_COLOR_MOUSEON COLOR_RGBA( 70, 70, 70, 255 )
#define EDGUI_THEME_BUTTON_BACK_COLOR_CLICKED COLOR_RGBA( 30, 30, 30, 255 )
#define EDGUI_THEME_BUTTON_BACKHL_COLOR COLOR_RGBA( 120, 60, 60, 255 )
#define EDGUI_THEME_BUTTON_BACKHL_COLOR_MOUSEON COLOR_RGBA( 120, 70, 70, 255 )
#define EDGUI_THEME_BUTTON_BACKHL_COLOR_CLICKED COLOR_RGBA( 100, 30, 30, 255 )
#define EDGUI_THEME_BUTTON_TEXT_COLOR COLOR_RGBA( 240, 240, 240, 255 )
// - split pane
#define EDGUI_THEME_SPLITPANE_BORDER_SIZE 4
#define EDGUI_THEME_SPLITPANE_BORDER_COLOR COLOR_RGBA( 120, 120, 120, 255 )
// - property
#define EDGUI_THEME_PROPERTY_HEIGHT 24
// - property/bool
#define EDGUI_THEME_PROP_BOOL_OFF_ACTIVE_COLOR COLOR_RGBA( 180, 20, 0, 255 )
#define EDGUI_THEME_PROP_BOOL_OFF_INACTIVE_COLOR COLOR_RGBA( 100, 20, 0, 255 )
#define EDGUI_THEME_PROP_BOOL_ON_ACTIVE_COLOR COLOR_RGBA( 20, 180, 0, 255 )
#define EDGUI_THEME_PROP_BOOL_ON_INACTIVE_COLOR COLOR_RGBA( 20, 100, 0, 255 )
// - number wheel
#define EDGUI_THEME_NUMWHEEL_WHEEL_SIZE 20
#define EDGUI_THEME_NUMWHEEL_CENTER_SIZE 20
#define EDGUI_THEME_NUMWHEEL_WHEEL_COLOR COLOR_RGBA( 127, 0, 0, 64 )
#define EDGUI_THEME_NUMWHEEL_OUTLINE_COLOR COLOR_RGBA( 192, 192, 192, 128 )

#define DOUBLE_CLICK_MSEC 500


// EVENTS
#define EDGUI_EVENT_ENGINE     1
#define EDGUI_EVENT_PAINT      2
#define EDGUI_EVENT_LAYOUT     3
#define EDGUI_EVENT_POSTLAYOUT 4
#define EDGUI_EVENT_HITTEST    5
#define EDGUI_EVENT_MOUSEMOVE  10
#define EDGUI_EVENT_MOUSEENTER 11
#define EDGUI_EVENT_MOUSELEAVE 12
#define EDGUI_EVENT_BTNDOWN    13
#define EDGUI_EVENT_BTNUP      14
#define EDGUI_EVENT_BTNCLICK   15
#define EDGUI_EVENT_PROPEDIT   21
#define EDGUI_EVENT_PROPCHANGE 22

// ITEM TYPES
#define EDGUI_ITEM_NULL        0
#define EDGUI_ITEM_FRAME       1
#define EDGUI_ITEM_LAYOUT_ROW  40
#define EDGUI_ITEM_LAYOUT_COL  41
#define EDGUI_ITEM_SPLIT_PANE  42
#define EDGUI_ITEM_LABEL       48
#define EDGUI_ITEM_BUTTON      50
#define EDGUI_ITEM_NUMWHEEL    60
#define EDGUI_ITEM_PROP_NULL   100
#define EDGUI_ITEM_PROP_BOOL   101
#define EDGUI_ITEM_PROP_INT    102
#define EDGUI_ITEM_PROP_FLOAT  103
#define EDGUI_ITEM_PROP_STRING 104



struct EXPORT EDGUIEvent
{
	int type;
	struct EDGUIItem* target;
	union
	{
		const Event* eev;
		struct {
			int x0, y0, x1, y1;
		} layout;
		struct {
			int x, y, button, clicks;
		} mouse;
	};
};

typedef Array< struct EDGUIItem* > EDGUIItemArray;

struct EXPORT EDGUIItem
{
	EDGUIItem();
	virtual ~EDGUIItem();
	virtual int OnEvent( EDGUIEvent* e );
	
	bool Add( EDGUIItem* subitem );
	bool Remove( EDGUIItem* subitem );
	void Invalidate(){}
	bool Hit( int x, int y );
	void BubblingEvent( EDGUIEvent* e );
	void SetRectFromEvent( EDGUIEvent* e, bool updatesub );
	void OnChangeLayout();
	void SetSubitemLayout( EDGUIItem* subitem, int _x0, int _y0, int _x1, int _y1 );
	void _SetFrame( struct EDGUIFrame* frame );
	void Edited();
	void Changed();
	
	const char* tyname;
	int type;
	uint32_t id1, id2;
	uint32_t backColor;
	uint32_t textColor;
	String caption;
	EDGUIItemArray m_subitems;
	EDGUIItem* m_parent;
	struct EDGUIFrame* m_frame;
	
	int x0, y0, x1, y1;
	
	bool m_mouseOn;
	bool m_clicked;
};

struct EXPORT EDGUIFrame : EDGUIItem
{
	EDGUIFrame();
	virtual int OnEvent( EDGUIEvent* e );
	
	void EngineEvent( const Event* eev );
	void Resize( int w, int h, int x = 0, int y = 0 );
	void Draw();
	
	void _HandleMouseMove( bool optional );
	EDGUIItem* _GetItemAtPosition( int x, int y );
	
	int m_mouseX;
	int m_mouseY;
	EDGUIItem* m_hover;
	EDGUIItem* m_keyboardFocus;
	EDGUIItem* m_clickTargets[3];
	int m_clickOffsets[3][2];
	int m_clickCount;
	uint32_t m_lastClickTime;
	int m_lastClickedButton;
	EDGUIItem* m_lastClickItem;
	Array< EDGUIItem* > m_hoverTrail;
};

struct EXPORT EDGUILayoutRow : EDGUIItem
{
	EDGUILayoutRow();
	virtual int OnEvent( EDGUIEvent* e );
};

struct EXPORT EDGUILayoutColumn : EDGUIItem
{
	EDGUILayoutColumn();
	virtual int OnEvent( EDGUIEvent* e );
};

struct EXPORT EDGUILayoutSplitPane : EDGUIItem
{
	// "vertical" refers to the direction of the split, not the line
	EDGUILayoutSplitPane( bool vertical, int splitoff, float splitfac );
	virtual int OnEvent( EDGUIEvent* e );
	
	void SetPane( bool second, EDGUIItem* item );
	void SetFirstPane( EDGUIItem* item );
	void SetSecondPane( EDGUIItem* item );
	
	bool m_vertical;
	int m_splitoff;
	float m_splitfac;
	EDGUIItem* m_first;
	EDGUIItem* m_second;
};


struct EXPORT EDGUILabel : EDGUIItem
{
	EDGUILabel();
	virtual int OnEvent( EDGUIEvent* e );
};


struct EXPORT EDGUIButton : EDGUIItem
{
	EDGUIButton();
	virtual int OnEvent( EDGUIEvent* e );
	void OnChangeState();
	void SetHighlight( bool hl );
	
	bool m_highlight;
};


struct EXPORT EDGUINumberWheel : EDGUIItem
{
	EDGUINumberWheel( EDGUIItem* owner, double min = -DBL_MAX, double max = DBL_MAX, int initpwr = 0, int numwheels = 9 );
	virtual int OnEvent( EDGUIEvent* e );
	
	double GetValue();
	
	double m_value;
	double m_min;
	double m_max;
	int m_cx;
	int m_cy;
	int m_initpwr;
	int m_numwheels;
	int m_prevMouseX;
	int m_prevMouseY;
	int m_curWheel;
	EDGUIItem* m_owner;
};


struct EXPORT EDGUIProperty : EDGUIItem
{
	EDGUIProperty();
	virtual int OnEvent( EDGUIEvent* e );
	
	void _Begin( EDGUIEvent* e );
	void _End( EDGUIEvent* e );
	
	bool m_disabled;
	int m_x0bk;
};

struct EXPORT EDGUIPropBool : EDGUIProperty
{
	EDGUIPropBool( bool def = false );
	virtual int OnEvent( EDGUIEvent* e );
	
	bool m_value;
};

struct EXPORT EDGUIPropInt : EDGUIProperty
{
	EDGUIPropInt( int32_t def = 0, int32_t min = -0x80000000, int32_t max = 0x7fffffff );
	virtual int OnEvent( EDGUIEvent* e );
	void _UpdateButton();
	void SetValue( int v ){ m_value = v; _UpdateButton(); }
	
	int32_t m_value;
	int32_t m_min;
	int32_t m_max;
	
	EDGUINumberWheel m_numWheel;
	EDGUIButton m_button;
};

struct EXPORT EDGUIPropFloat : EDGUIProperty
{
	EDGUIPropFloat( float def = 0, int prec = 2, float min = -FLT_MAX, float max = FLT_MAX );
	virtual int OnEvent( EDGUIEvent* e );
	void _UpdateButton();
	void SetValue( float v ){ m_value = v; _UpdateButton(); }
	
	float m_value;
	float m_min;
	float m_max;
	
	EDGUINumberWheel m_numWheel;
	EDGUIButton m_button;
};

struct EXPORT EDGUIPropString : EDGUIProperty
{
	EDGUIPropString( String def = String() );
	virtual int OnEvent( EDGUIEvent* e );
	
	String m_value;
};


