

#pragma once
#include <float.h>
#include "engine.hpp"


#define EDGUI_EVENT_ENGINE     1
#define EDGUI_EVENT_PAINT      2
#define EDGUI_EVENT_LAYOUT     3
#define EDGUI_EVENT_POSTLAYOUT 4
#define EDGUI_EVENT_HITTEST    5
#define EDGUI_EVENT_MOUSEENTER 11
#define EDGUI_EVENT_MOUSELEAVE 12
#define EDGUI_EVENT_BTNDOWN    13
#define EDGUI_EVENT_BTNUP      14

#define EDGUI_ITEM_NULL        0
#define EDGUI_ITEM_FRAME       1
#define EDGUI_ITEM_LAYOUT_ROW  40
#define EDGUI_ITEM_LAYOUT_COL  41
#define EDGUI_ITEM_BUTTON      50
#define EDGUI_ITEM_PROP_BOOL   100
#define EDGUI_ITEM_PROP_INT    101
#define EDGUI_ITEM_PROP_FLOAT  102
#define EDGUI_ITEM_PROP_STRING 103

struct EXPORT EDGUIEvent
{
	int type;
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
	
	int type;
	uint32_t uid;
	uint32_t backColor;
	uint32_t textColor;
	String caption;
	EDGUIItemArray m_subitems;
	EDGUIItem* m_parent;
	
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
	virtual int OnEvent( EDGUIEvent* e );
};

struct EXPORT EDGUILayoutColumn : EDGUIItem
{
	virtual int OnEvent( EDGUIEvent* e );
};

struct EXPORT EDGUIButton : EDGUIItem
{
	EDGUIButton();
	virtual int OnEvent( EDGUIEvent* e );
	void OnChangeState();
};

struct EXPORT EDGUIProperty : EDGUIItem
{
	EDGUIProperty();
	virtual int OnEvent( EDGUIEvent* e );
	
	bool m_disabled;
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
	
	int32_t m_value;
	int32_t m_min;
	int32_t m_max;
};

struct EXPORT EDGUIPropFloat : EDGUIProperty
{
	EDGUIPropFloat( float def = 0, float min = -FLT_MAX, float max = FLT_MAX );
	virtual int OnEvent( EDGUIEvent* e );
	
	float m_value;
	float m_min;
	float m_max;
};

struct EXPORT EDGUIPropString : EDGUIProperty
{
	EDGUIPropString( String def = String() );
	virtual int OnEvent( EDGUIEvent* e );
	
	String m_value;
};

