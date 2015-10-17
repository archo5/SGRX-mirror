

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
	MCT_RadioBtn,
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
	bool selected;
	int id;
	int group;
	
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
	void ReselectHL(){ FindHL( m_mouseX, m_mouseY ); }
	void RecalcSize( float w, float h, float aspect = 1.0f );
	void AddButton( const StringView& caption, int style, float x0, float y0, float x1, float y1, int id = 0 );
	void AddRadioBtn( const StringView& caption, int style, float x0, float y0, float x1, float y1, int group, int id = 0 );
	void AddSlider( const StringView& caption, int style, float x0, float y0, float x1, float y1, int id = 0 );
	void Clear();
	
	int GetCountInGroup( int group );
	int GetSelectedInGroup( int group );
	int SelectInGroup( int group, int which ); // returns control number in array
	int SelectNextInGroup( int group );
	int SelectPrevInGroup( int group );
	
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


enum ConfigPropType
{
	CFGPROP_Label = 0,
	CFGPROP_Int = 1,
	CFGPROP_Float = 2,
	CFGPROP_Bool = 3,
	CFGPROP_Enum = 101,
	CFGPROP_Input = 102,
};

struct ConfigurableProperty
{
	ConfigurableProperty( const StringView& nm, ConfigPropType ty ) : name(nm), type(ty){}
	virtual ~ConfigurableProperty(){}
	virtual void GenerateConfig( String& out ) = 0;
	virtual void Parse( const StringView& value ) = 0;
	
	StringView name;
	ConfigPropType type;
};

struct ConfigurableProp_Label : ConfigurableProperty
{
	ConfigurableProp_Label( const StringView& lbl )
		: ConfigurableProperty( lbl, CFGPROP_Label ){}
	virtual void GenerateConfig( String& out );
	virtual void Parse( const StringView& str );
};

struct ConfigurableProp_Int : ConfigurableProperty
{
	ConfigurableProp_Int( const StringView& nm, int32_t v = 0 )
		: ConfigurableProperty( nm, CFGPROP_Int ), value( v ){}
	virtual void GenerateConfig( String& out );
	virtual void Parse( const StringView& str );
	
	int32_t value;
};

struct ConfigurableProp_Float : ConfigurableProperty
{
	ConfigurableProp_Float( const StringView& nm, float v = 0 )
		: ConfigurableProperty( nm, CFGPROP_Float ), value( v ){}
	virtual void GenerateConfig( String& out );
	virtual void Parse( const StringView& str );
	
	float value;
};

struct ConfigurableProp_Bool : ConfigurableProperty
{
	ConfigurableProp_Bool( const StringView& nm, bool v = false )
		: ConfigurableProperty( nm, CFGPROP_Bool ), value( v ){}
	virtual void GenerateConfig( String& out );
	virtual void Parse( const StringView& str );
	
	bool value;
};

struct ConfigurableProp_Enum : ConfigurableProp_Int
{
	ConfigurableProp_Enum( const StringView& nm, StringView* vnms, int32_t nmc, int32_t v = 0 )
		: ConfigurableProp_Int( nm, v ), value_names( vnms ), max_value( nmc ){ type = CFGPROP_Enum; }
	virtual void GenerateConfig( String& out );
	virtual void Parse( const StringView& str );
	
	StringView* value_names;
	int32_t max_value;
};

struct ConfigurableProp_Input : ConfigurableProperty
{
	ConfigurableProp_Input( const StringView& nm, ActionInput v )
		: ConfigurableProperty( nm, CFGPROP_Input ), value( v ){}
	virtual void GenerateConfig( String& out );
	virtual void Parse( const StringView& str );
	
	ActionInput value;
};

bool ParseConfigProp( ConfigurableProperty** props, size_t count, const StringView& key, const StringView& value );
void GenerateConfigProps( ConfigurableProperty** props, size_t count, String& out );

bool LoadConfigPropFile( const StringView& path, bool infs, ConfigurableProperty** props, size_t count );
bool SaveConfigPropFile( const StringView& path, bool infs, ConfigurableProperty** props, size_t count );



