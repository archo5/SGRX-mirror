

#pragma once
#include <engine.hpp>


enum MenuCtrlStyle
{
	MCS_Default = 0,
	MCS_Link,
};

enum MenuCtrlType
{
	MCT_Null = 0,
	MCT_Button,
	MCT_Slider,
};

struct ScreenMenu;
struct MenuCtrlInfo
{
	float x0;
	float y0;
	float x1;
	float y1;
	int i;
	bool highlighted;
	bool selected;
	bool anysel;
	ScreenMenu* menu;
	float minw;
};


struct MenuControl
{
	MenuControl();
	
	MenuCtrlType type;
	int style;
	String caption;
	float x0, y0, x1, y1;
	bool visible;
	bool enabled;
	int id;
	
	String value_text;
	float sl_value;
	uint64_t sb_option;
};


struct MenuTheme
{
	struct Colors
	{
		Vec4 bgcol;
		Vec4 fgcol;
		float opacity;
	};
	
	MenuTheme();
	virtual ~MenuTheme(){}
	virtual void DrawControl( const MenuControl& ctrl, const MenuCtrlInfo& info );
	virtual void _GetCtrlColors( const MenuControl& ctrl, const MenuCtrlInfo& info, bool link, Colors& col );
	virtual void _DrawButtonCore( const MenuControl& ctrl, const MenuCtrlInfo& info, const Colors& col );
	virtual void DrawDefaultButton( const MenuControl& ctrl, const MenuCtrlInfo& info );
	virtual void DrawLinkButton( const MenuControl& ctrl, const MenuCtrlInfo& info );
	virtual void DrawDefaultSlider( const MenuControl& ctrl, const MenuCtrlInfo& info );
	
	Vec4 color_hl;
	Vec4 color_sel;
};
#define MENUTHEME_PREP \
	BatchRenderer& br = GR2D_GetBatchRenderer(); \
	float ax0 = info.x0, ay0 = info.y0, ax1 = info.x1, ay1 = info.y1;


struct ScreenMenu
{
	enum CtrlEvent { EV_MBDown, EV_MBUp, EV_MouseMove };
	
	ScreenMenu();
	~ScreenMenu();
	
	void OnStart();
	void OnEnd();
	int UpdateCtrl( CtrlEvent evtype );
	int OnEvent( const Event& e );
	void Draw( float delta );
	void FindHL( float x, float y );
	void RecalcSize( float w, float h, float aspect = 1.0f );
	void AddButton( const StringView& caption, int style, float x0, float y0, float x1, float y1, int id = 0 );
	void AddSlider( const StringView& caption, int style, float x0, float y0, float x1, float y1, int id = 0 );
	
	float GetMinw(){ return TMIN( x1 - x0, y1 - y0 ); }
	float IX( float v ) const { return TLERP( x0, x1, v ); }
	float IY( float v ) const { return TLERP( y0, y1, v ); }
	Vec2 I( const Vec2& v ) const { return V2( IX( v.x ), IY( v.y ) ); }
	
	MenuTheme* theme;
	float x0, y0, x1, y1;
	float opacity;
	Array< MenuControl > controls;
	
	int m_HL;
	int m_lastHL;
	int m_selected;
	
	float m_mouseX;
	float m_mouseY;
};



