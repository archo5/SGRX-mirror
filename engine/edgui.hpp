

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
// - group
#define EDGUI_THEME_GROUP_HEIGHT 24
#define EDGUI_THEME_GRPLBL_BACK_COLOR COLOR_RGBA( 60, 70, 90, 255 )
#define EDGUI_THEME_GRPLBL_BACK_COLOR_MOUSEON COLOR_RGBA( 70, 80, 100, 255 )
#define EDGUI_THEME_GRPLBL_BACK_COLOR_CLICKED COLOR_RGBA( 30, 40, 60, 255 )
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
// - resource picker
#define EDGUI_THEME_RSRCPICK_ITEM_BACK_COLOR COLOR_RGBA( 60, 60, 60, 128 )
#define EDGUI_THEME_RSRCPICK_ITEM_BACKHL_COLOR COLOR_RGBA( 120, 30, 30, 128 )
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
// - textbox
#define EDGUI_THEME_TEXTBOX_BORDER_COLOR COLOR_RGBA( 80, 80, 80, 255 )
#define EDGUI_THEME_TEXTBOX_CENTER_COLOR COLOR_RGBA( 48, 48, 48, 255 )

#define DOUBLE_CLICK_MSEC 500


// EVENTS
#define EDGUI_EVENT_ENGINE     1
#define EDGUI_EVENT_PAINT      2
#define EDGUI_EVENT_LAYOUT     3
#define EDGUI_EVENT_POSTLAYOUT 4
#define EDGUI_EVENT_PRELAYOUT  5
#define EDGUI_EVENT_HITTEST    6
#define EDGUI_EVENT_MOUSEMOVE  10
#define EDGUI_EVENT_MOUSEENTER 11
#define EDGUI_EVENT_MOUSELEAVE 12
#define EDGUI_EVENT_BTNDOWN    13
#define EDGUI_EVENT_BTNUP      14
#define EDGUI_EVENT_BTNCLICK   15
#define EDGUI_EVENT_MOUSEWHEEL 16
#define EDGUI_EVENT_PROPEDIT   21
#define EDGUI_EVENT_PROPCHANGE 22
#define EDGUI_EVENT_SETFOCUS   31
#define EDGUI_EVENT_LOSEFOCUS  32
#define EDGUI_EVENT_KEYDOWN    41
#define EDGUI_EVENT_KEYUP      42
#define EDGUI_EVENT_TEXTINPUT  43
#define EDGUI_EVENT_ADDED      51
#define EDGUI_EVENT_USER       256

// ITEM TYPES
#define EDGUI_ITEM_NULL        0
#define EDGUI_ITEM_FRAME       1
#define EDGUI_ITEM_LAYOUT_ROW  40
#define EDGUI_ITEM_LAYOUT_COL  41
#define EDGUI_ITEM_SPLIT_PANE  42
#define EDGUI_ITEM_LABEL       48
#define EDGUI_ITEM_GROUP       49
#define EDGUI_ITEM_BUTTON      50
#define EDGUI_ITEM_NUMWHEEL    60
#define EDGUI_ITEM_RSRCPICKER  61
#define EDGUI_ITEM_QUESTION    62
#define EDGUI_ITEM_PROP_NULL   100
#define EDGUI_ITEM_PROP_BOOL   101
#define EDGUI_ITEM_PROP_INT    102
#define EDGUI_ITEM_PROP_FLOAT  103
#define EDGUI_ITEM_PROP_STRING 104
#define EDGUI_ITEM_PROP_VEC2   105
#define EDGUI_ITEM_PROP_VEC3   106
#define EDGUI_ITEM_PROP_RSRC   107


#define EDGUI_KEY_UNKNOWN   0
#define EDGUI_KEY_LEFT      1  // shift makes selection stick
#define EDGUI_KEY_RIGHT     2  // ^
#define EDGUI_KEY_UP        3  // ^
#define EDGUI_KEY_DOWN      4  // ^
#define EDGUI_KEY_DELLEFT   5
#define EDGUI_KEY_DELRIGHT  6
#define EDGUI_KEY_TAB       7  // shift makes focus move backwards
#define EDGUI_KEY_CUT       8
#define EDGUI_KEY_COPY      9
#define EDGUI_KEY_PASTE     10
#define EDGUI_KEY_UNDO      11
#define EDGUI_KEY_REDO      12
#define EDGUI_KEY_SELECTALL 13
#define EDGUI_KEY_PAGEUP    14
#define EDGUI_KEY_PAGEDOWN  15
#define EDGUI_KEY_ENTER     16
#define EDGUI_KEY_ACTIVATE  17

#define EDGUI_KEYMOD_FILTER 0x0FF
#define EDGUI_KEYMOD_SHIFT  0x100



struct ENGINE_EXPORT EDGUIEvent
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
		struct {
			int key, engkey, engmod;
			bool repeat;
		} key;
		struct {
			char text[8];
		} text;
	};
};

typedef Array< struct EDGUIItem* > EDGUIItemArray;

struct ENGINE_EXPORT EDGUIItem
{
	EDGUIItem();
	virtual ~EDGUIItem();
	virtual int OnEvent( EDGUIEvent* e );
	
	bool Add( EDGUIItem* subitem );
	bool Remove( EDGUIItem* subitem );
	void Clear();
	void SubstChildPtr( const EDGUIItem* find, EDGUIItem* repl );
	void Invalidate(){}
	bool Hit( int x, int y );
	void BubblingEvent( EDGUIEvent* e );
	void SetRectFromEvent( EDGUIEvent* e, bool updatesub );
	void OnChangeLayout();
	void ReshapeLayout();
	void SetSubitemLayout( EDGUIItem* subitem, int _x0, int _y0, int _x1, int _y1 );
	void _SetFrame( struct EDGUIFrame* frame );
	void Edited( EDGUIItem* tgt = NULL );
	void Changed( EDGUIItem* tgt = NULL );
	
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

struct ENGINE_EXPORT EDGUIFrame : EDGUIItem
{
	struct Rect { int x0, y0, x1, y1; };
	
	EDGUIFrame();
	virtual int OnEvent( EDGUIEvent* e );
	
	void EngineEvent( const Event* eev );
	void Resize( int w, int h, int x = 0, int y = 0 );
	void Draw();
	
	bool PushScissorRect( int x0, int y0, int x1, int y1 );
	void PopScissorRect();
	
	void _HandleMouseMove( bool optional );
	EDGUIItem* _GetItemAtPosition( int x, int y );
	void _Unlink( EDGUIItem* item );
	void _SetFocus( EDGUIItem* item );
	
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
	Array< Rect > m_rects;
};

struct ENGINE_EXPORT EDGUILayoutRow : EDGUIItem
{
	EDGUILayoutRow();
	virtual int OnEvent( EDGUIEvent* e );
};

struct ENGINE_EXPORT EDGUILayoutColumn : EDGUIItem
{
	EDGUILayoutColumn();
	virtual int OnEvent( EDGUIEvent* e );
};

struct ENGINE_EXPORT EDGUILayoutSplitPane : EDGUIItem
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


struct ENGINE_EXPORT EDGUILabel : EDGUIItem
{
	EDGUILabel();
	virtual int OnEvent( EDGUIEvent* e );
};


struct ENGINE_EXPORT EDGUIGroup : EDGUILayoutRow
{
	EDGUIGroup( bool open = true, const StringView& sv = StringView() );
	virtual int OnEvent( EDGUIEvent* e );
	void SetOpen( bool open );
	void _UpdateButton();
	
	bool m_open;
	String m_name;
};


struct ENGINE_EXPORT EDGUIButton : EDGUIItem
{
	EDGUIButton();
	virtual int OnEvent( EDGUIEvent* e );
	void OnChangeState();
	void SetHighlight( bool hl );
	
	bool m_highlight;
};


struct ENGINE_EXPORT EDGUINumberWheel : EDGUIItem
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


struct ENGINE_EXPORT EDGUIRsrcPicker : EDGUIItem
{
	EDGUIRsrcPicker();
	virtual int OnEvent( EDGUIEvent* e );
	
	void Open( EDGUIItem* owner, const StringView& val );
	void Close();
	void SetValue( const StringView& sv );
	String GetValue();
	void Zoom( float z );
	void _FindHL();
	void _Search( const StringView& str );
	int GetScrollOffset();
	virtual void Reload(){}
	virtual void _OnChangeZoom();
	virtual void _OnPickResource();
	virtual void _OnConfirm();
	virtual void _DrawItem( int i, int x0, int y0, int x1, int y1 );
	
	float m_zoom;
	int m_horCount;
	int m_scrollOffset;
	int m_itemWidth;
	int m_itemHeight;
	int cx0, cy0, cx1, cy1;
	int m_hlfiltered;
	int m_picked;
	String m_searchString;
	String m_pickedOption;
	Array< String > m_options;
	Array< int > m_filtered;
	EDGUIItem* m_owner;
	int m_mouseX;
	int m_mouseY;
};


struct ENGINE_EXPORT EDGUIQuestion : EDGUIItem
{
	EDGUIQuestion();
	virtual int OnEvent( EDGUIEvent* e );
	
	void Open( EDGUIItem* owner );
	void Close();
	virtual void _OnChoose();
	
	EDGUIItem* m_owner;
	EDGUIButton m_btnYes;
	EDGUIButton m_btnNo;
	int m_value;
};


struct ENGINE_EXPORT EDGUIProperty : EDGUIItem
{
	EDGUIProperty();
	virtual int OnEvent( EDGUIEvent* e );
	
	void _Begin( EDGUIEvent* e );
	void _End( EDGUIEvent* e );
	virtual bool TakeValue( EDGUIProperty* src ){ return false; }
	
	bool m_disabled;
	int m_x0bk;
};

#define PROP_INTERFACE( classname ) \
	virtual bool TakeValue( EDGUIProperty* src ){ if( src->type != type ) return false; m_value = ((classname*)src)->m_value; _UpdateButton(); return true; }

struct ENGINE_EXPORT EDGUIPropBool : EDGUIProperty
{
	EDGUIPropBool( bool def = false );
	virtual int OnEvent( EDGUIEvent* e );
	void SetValue( int v ){ m_value = v; }
	void _UpdateButton(){}
	PROP_INTERFACE( EDGUIPropBool );
	
	template< class T > void Serialize( T& arch ){ arch << m_value; }
	
	bool m_value;
};

struct ENGINE_EXPORT EDGUIPropInt : EDGUIProperty
{
	EDGUIPropInt( int32_t def = 0, int32_t min = -0x80000000, int32_t max = 0x7fffffff );
	virtual int OnEvent( EDGUIEvent* e );
	void _UpdateButton();
	void SetValue( int v ){ m_value = v; _UpdateButton(); }
	PROP_INTERFACE( EDGUIPropInt );
	
	template< class T > void Serialize( T& arch ){ arch << m_value; if( T::IsReader ) SetValue( m_value ); }
	
	int32_t m_value;
	int32_t m_min;
	int32_t m_max;
	
	EDGUINumberWheel m_numWheel;
	EDGUIButton m_button;
};

struct ENGINE_EXPORT EDGUIPropFloat : EDGUIProperty
{
	EDGUIPropFloat( float def = 0, int prec = 2, float min = -FLT_MAX, float max = FLT_MAX );
	virtual int OnEvent( EDGUIEvent* e );
	void _UpdateButton();
	void SetValue( float v ){ m_value = v; _UpdateButton(); }
	PROP_INTERFACE( EDGUIPropFloat );
	
	template< class T > void Serialize( T& arch ){ arch << m_value; if( T::IsReader ) SetValue( m_value ); }
	
	float m_value;
	float m_min;
	float m_max;
	
	EDGUINumberWheel m_numWheel;
	EDGUIButton m_button;
};

struct ENGINE_EXPORT EDGUIPropVec2 : EDGUIProperty
{
	EDGUIPropVec2( const Vec2& def = V2(0), int prec = 2, const Vec2& min = V2(-FLT_MAX), const Vec2& max = V2(FLT_MAX) );
	virtual int OnEvent( EDGUIEvent* e );
	EDGUIPropVec2& operator = ( const EDGUIPropVec2& o );
	void _UpdateButton();
	void SetValue( const Vec2& v ){ m_value = v; _UpdateButton(); }
	PROP_INTERFACE( EDGUIPropVec2 );
	
	template< class T > void Serialize( T& arch ){ arch << m_value; if( T::IsReader ) SetValue( m_value ); }
	
	Vec2 m_value;
	Vec2 m_min;
	Vec2 m_max;
	
	EDGUILayoutColumn m_buttonlist;
	EDGUINumberWheel m_XnumWheel;
	EDGUIButton m_Xbutton;
	EDGUINumberWheel m_YnumWheel;
	EDGUIButton m_Ybutton;
};

struct ENGINE_EXPORT EDGUIPropVec3 : EDGUIProperty
{
	EDGUIPropVec3( const Vec3& def = V3(0), int prec = 2, const Vec3& min = V3(-FLT_MAX), const Vec3& max = V3(FLT_MAX) );
	virtual int OnEvent( EDGUIEvent* e );
	EDGUIPropVec3& operator = ( const EDGUIPropVec3& o );
	void _UpdateButton();
	void SetValue( const Vec3& v ){ m_value = v; _UpdateButton(); }
	PROP_INTERFACE( EDGUIPropVec3 );
	
	template< class T > void Serialize( T& arch ){ arch << m_value; if( T::IsReader ) SetValue( m_value ); }
	
	Vec3 m_value;
	Vec3 m_min;
	Vec3 m_max;
	
	EDGUILayoutColumn m_buttonlist;
	EDGUINumberWheel m_XnumWheel;
	EDGUIButton m_Xbutton;
	EDGUINumberWheel m_YnumWheel;
	EDGUIButton m_Ybutton;
	EDGUINumberWheel m_ZnumWheel;
	EDGUIButton m_Zbutton;
};

struct ENGINE_EXPORT EDGUIPropString : EDGUIProperty
{
	EDGUIPropString( const StringView& def = StringView() );
	virtual int OnEvent( EDGUIEvent* e );
	
	void _UpdateButton(){}
	PROP_INTERFACE( EDGUIPropString );
	
	StringView GetText(){ return m_value; }
	void SetText( const StringView& sv );
	void _UpdateSelOffsets();
	void _UpdateText();
	int _FindOffset( int x, int y );
	
	String m_value;
	int m_sel_from;
	int m_sel_to;
	int m_offset;
	int m_fsel_from;
	int m_fsel_to;
	bool m_selecting;
	String m_chars;
};

struct ENGINE_EXPORT EDGUIPropRsrc : EDGUIProperty
{
	EDGUIPropRsrc( EDGUIRsrcPicker* rsrcPicker, const StringView& def = StringView() );
	virtual int OnEvent( EDGUIEvent* e );
	void _UpdateButton();
	void SetValue( const StringView& v ){ m_value = v; _UpdateButton(); }
	PROP_INTERFACE( EDGUIPropRsrc );
	
	template< class T > void Serialize( T& arch ){ arch << m_value; if( T::IsReader ) SetValue( m_value ); }
	
	String m_value;
	
	bool m_requestReload;
	
	EDGUIRsrcPicker* m_rsrcPicker;
	EDGUIButton m_button;
};


