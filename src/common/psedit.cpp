

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_MAT4
#define USE_ARRAY
#define USE_SERIALIZATION
#include <engine.hpp>
#include <enganim.hpp>
#include <edgui.hpp>
#include "edcomui.hpp"


struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
struct EdParticleSystem* g_EdPS;
struct EdDualGraph* g_UIGraphEd;
struct EDGUISDTexPicker* g_UITexPicker;
struct EDGUIShaderPicker* g_UIShaderPicker;
struct EDGUIMeshPicker* g_UIMeshPicker;
struct EDGUIPSOpenPicker* g_UIPSOpenPicker;
struct EDGUIPSSavePicker* g_UIPSSavePicker;



struct EDGUIShaderPicker : EDGUIRsrcPicker
{
	EDGUIShaderPicker()
	{
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading shaders";
		m_options.clear();
		DirectoryIterator tdi( "shaders_d3d9" );
		while( tdi.Next() )
		{
			StringView fn = tdi.Name();
			LOG << fn;
			if( !tdi.IsDirectory() )
			{
				if( fn.ends_with( ".shd" ) && fn.starts_with( "mtl_" ) )
				{
					m_options.push_back( fn.part( 4, fn.size() - 8 ) );
				}
			}
		}
		_Search( m_searchString );
	}
};


struct EDGUIPSPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUIPSPicker(){ Reload(); }
	void Reload()
	{
		LOG << "Reloading particle systems";
		m_options.clear();
		FS_IterateDirectory( "psys", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[P]: " << name;
		if( !isdir && name.ends_with( ".psy" ) )
		{
			m_options.push_back( name.part( 0, name.size() - 4 ) );
		}
		return true;
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( e->type == EDGUI_EVENT_PROPCHANGE && e->target == &m_confirm )
		{
			if( m_confirm.m_value == 1 )
			{
				EDGUIRsrcPicker::_OnPickResource();
			}
			return 1;
		}
		return EDGUIRsrcPicker::OnEvent( e );
	}
	
	EDGUIQuestion m_confirm;
};

struct EDGUIPSOpenPicker : EDGUIPSPicker
{
	EDGUIPSOpenPicker()
	{
		caption = "Pick a particle system to open";
		m_confirm.caption = "Do you really want to open the particle system? All unsaved changes will be lost!";
	}
	virtual void _OnPickResource()
	{
		m_confirm.Open( this );
		m_frame->Add( &m_confirm );
	}
};

struct EDGUIPSSavePicker : EDGUIPSPicker
{
	EDGUIPSSavePicker()
	{
		caption = "Pick a particle system to save or write the name";
		m_confirm.caption = "Do you really want to overwrite the particle system?";
	}
	virtual void _OnPickResource()
	{
		if( m_options.find_first_at( m_pickedOption ) == NOT_FOUND )
			EDGUIRsrcPicker::_OnPickResource();
		else
		{
			m_confirm.Open( this );
			m_frame->Add( &m_confirm );
		}
	}
	virtual void _OnConfirm()
	{
		_OnPickResource();
	}
};


struct EdDualGraph : EDGUIItem
{
	EdDualGraph() :
		m_mouseX(0), m_mouseY(0),
		m_A(false), m_B(false),
		m_data(NULL), m_size(0)
	{}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_MOUSEMOVE:
			m_mouseX = e->mouse.x;
			m_mouseY = e->mouse.y;
			PaintGraph();
			break;
		case EDGUI_EVENT_BTNDOWN:
			if( e->mouse.button == 0 ) m_A = true;
			if( e->mouse.button == 1 ) m_B = true;
			PaintGraph();
			break;
		case EDGUI_EVENT_BTNUP:
			if( e->mouse.button == 0 ) m_A = false;
			if( e->mouse.button == 1 ) m_B = false;
			PaintGraph();
			break;
		case EDGUI_EVENT_LAYOUT:
			x0 = e->layout.x0 + 8;
			y0 = e->layout.y0 + 8;
			x1 = e->layout.x1 - 8;
			y1 = e->layout.y1 - 8;
			return 1;
		case EDGUI_EVENT_PAINT:
			EDGUIItem::OnEvent( e );
			if( m_data )
			{
				BatchRenderer& br = GR2D_GetBatchRenderer();
				br.Reset().Col( 0, 0.5f );
				br.Quad( x0, y0, x1, y1 );
				if( m_size > 1 )
				{
					br.Col( 0.1f, 0.7f, 0 );
					br.SetPrimitiveType( PT_LineStrip );
					for( size_t i = 0; i < m_size; ++i )
					{
						float x = x0 + ( x1 - x0 ) * i / (float) ( m_size - 1 );
						float y = y1 + ( y0 - y1 ) * m_data[ i ].x;
						br.Pos( x, y );
					}
					br.Col( 0.7f, 0.1f, 0 );
					br.SetPrimitiveType( PT_LineStrip );
					for( size_t i = 0; i < m_size; ++i )
					{
						float x = x0 + ( x1 - x0 ) * i / (float) ( m_size - 1 );
						float y = y1 + ( y0 - y1 ) * m_data[ i ].y;
						br.Pos( x, y );
					}
					br.Flush();
				}
			}
			return 1;
		}
		return EDGUIItem::OnEvent( e );
	}
	
	void PaintGraph()
	{
		if( !m_data || !m_size || x1 <= x0 || y1 <= y0 )
			return;
		
		// convert mouse position into graph position
		float x = float( m_mouseX - x0 ) / float( x1 - x0 ) * ( m_size - 1 );
		float y = clamp( float( m_mouseY - y1 ) / float( y0 - y1 ), 0, 1 );
		
		int at = round( clamp( x, 0, m_size - 1 ) );
		if( m_A ) m_data[ at ].x = y;
		if( m_B ) m_data[ at ].y = y;
	}
	
	void SetCurve( Vec2* p, size_t s )
	{
		m_data = p;
		m_size = s;
	}
	void UnsetCurve(){ SetCurve( NULL, 0 ); }
	
	int m_mouseX;
	int m_mouseY;
	bool m_A;
	bool m_B;
	
	Vec2* m_data;
	size_t m_size;
};

struct EdEmitCurve : EDGUIGroup
{
	EdEmitCurve( bool open, const StringView& name = StringView() ) :
		EDGUIGroup( open, name ),
		curveSize( 1, 1, 1024 ),
		randomValDvg( V2(0), 2, V2(-100), V2(100) ),
		valueRange( V2(0,1), 2, V2(-1000), V2(1000) )
	{
		editBtn.caption = "Edit curve";
		curveSize.caption = "# of sample points";
		randomValDvg.caption = "Random value/divergence";
		valueRange.caption = "Min./max. values";
		
		Add( &editBtn );
		Add( &curveSize );
		Add( &randomValDvg );
		Add( &valueRange );
	}
	
	void SetValue( ParticleSystem::Emitter::Curve& curve )
	{
		m_curve = &curve;
		
		curveSize.SetValue( curve.values.size() );
		randomValDvg.SetValue( curve.randomValDvg );
		valueRange.SetValue( curve.valueRange );
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPCHANGE:
			if( e->target == &curveSize )
			{
				int ncs = curveSize.m_value;
				if( ncs < 1 ) ncs = 1;
				if( ncs > 1024 ) ncs = 1024;
				m_curve->values.resize( ncs );
			}
			break;
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &randomValDvg ) m_curve->randomValDvg = randomValDvg.m_value;
			if( e->target == &valueRange ) m_curve->valueRange = valueRange.m_value;
			break;
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &editBtn )
			{
				g_UIGraphEd->SetCurve( m_curve->values.data(), m_curve->values.size() );
			}
			break;
		case EDGUI_EVENT_PAINT:
			EDGUIGroup::OnEvent( e );
			{
				int rx0 = ( x0 + x1 ) / 2 + 2;
				int ry0 = y0 + 2;
				int rx1 = x1 - 2;
				int ry1 = y0 + EDGUI_THEME_GROUP_HEIGHT - 2;
				
				BatchRenderer& br = GR2D_GetBatchRenderer();
				br.Reset().Col( 0, 0.5f );
				br.Quad( rx0, ry0, rx1, ry1 );
				if( m_curve->values.size() > 1 )
				{
					br.Col( 0.1f, 0.7f, 0 );
					br.SetPrimitiveType( PT_LineStrip );
					for( size_t i = 0; i < m_curve->values.size(); ++i )
					{
						float x = rx0 + ( rx1 - rx0 ) * i / (float) ( m_curve->values.size() - 1 );
						float y = ry1 + ( ry0 - ry1 ) * m_curve->values[ i ].x;
						br.Pos( x, y );
					}
					br.Col( 0.7f, 0.1f, 0 );
					br.SetPrimitiveType( PT_LineStrip );
					for( size_t i = 0; i < m_curve->values.size(); ++i )
					{
						float x = rx0 + ( rx1 - rx0 ) * i / (float) ( m_curve->values.size() - 1 );
						float y = ry1 + ( ry0 - ry1 ) * m_curve->values[ i ].y;
						br.Pos( x, y );
					}
					br.Flush();
				}
			}
			return 1;
		}
		return EDGUIGroup::OnEvent( e );
	}
	
	ParticleSystem::Emitter::Curve* m_curve;
	
	EDGUIButton editBtn;
	EDGUIPropInt curveSize;
	EDGUIPropVec2 randomValDvg;
	EDGUIPropVec2 valueRange;
};

#define MAX_PARTICLES 10000
#define MAX_TIME 120.0f
#define MAX_POS 8192.0f
#define MAX_NORMAL 100.0f
#define MAX_VEL 100.0f

static String GetParticleTexName( const TextureHandle& tex )
{
	if( !tex )
		return String();
	StringView oname = tex->m_key;
	if( oname.starts_with( "textures/particles/" ) && oname.ends_with( ".png" ) )
		return oname.part( sizeof("textures/particles/")-1, oname.size() - (sizeof("textures/particles/.png")-1) );
	// cannot resolve:
	return oname;
}

static TextureHandle GetParticleTexHandle( const StringView& sv )
{
	if( !sv )
		return TextureHandle();
	char bfr[ 1024 ] = {0};
	sgrx_snprintf( bfr, 1024, "textures/particles/%.*s.png", TMIN( 980, (int) sv.size() ), sv.data() );
	return GR_GetTexture( bfr );
}

struct EdEmitter : EDGUILayoutRow
{
	EdEmitter() :
		grpSpawn( false, "Spawn properties" ),
		spawn_MaxCount( 0, 0, MAX_PARTICLES ),
		spawn_Count( 0, 0, MAX_PARTICLES ),
		spawn_CountExt( 0, 0, MAX_PARTICLES ),
		spawn_TimeExt( V2(0), 2, V2(0), V2(MAX_TIME) ),
		
		grpPosition( false, "Initial position" ),
		create_Pos( V3(0), 2, V3(-MAX_POS), V3(MAX_POS) ),
		create_PosBox( V3(0), 2, V3(0), V3(MAX_POS) ),
		create_PosRadius( 0, 2, 0, MAX_POS ),
		
		grpVelocity( false, "Initial velocity" ),
		create_VelMicroDir( V3(0), 2, V3(-MAX_NORMAL), V3(MAX_NORMAL) ),
		create_VelMicroDvg( 0, 2, 0, 1 ),
		create_VelMicroDistExt( V2(0), 2, V2(-MAX_VEL), V2(MAX_VEL) ),
		create_VelMacroDir( V3(0), 2, V3(-MAX_NORMAL), V3(MAX_NORMAL) ),
		create_VelMacroDvg( 0, 2, 0, 1 ),
		create_VelMacroDistExt( V2(0), 2, V2(-MAX_VEL), V2(MAX_VEL) ),
		create_VelCluster( 0, 0, MAX_PARTICLES ),
		create_VelClusterExt( 0, 0, MAX_PARTICLES ),
		
		grpMisc( false, "Misc. properties" ),
		create_LifetimeExt( V2(0), 2, V2(0), V2(MAX_TIME) ),
		create_AngleDirDvg( V2(0), 2, V2(0,0), V2(360,180) ),
		create_AngleVelDvg( V2(0), 2, V2(-1000), V2(1000) ),
		tick_AngleAcc( 0, 2, -10000, 10000 ),
		absolute( true ),
		
		grpSizeColor( false, "Size/color curves" ),
		curve_Size( false, "Size" ),
		curve_ColorHue( false, "Color - hue" ),
		curve_ColorSat( false, "Color - saturation" ),
		curve_ColorVal( false, "Color - value" ),
		curve_Opacity( false, "Opacity" ),
		
		grpRender( true, "Rendering data" ),
		render_Texture0( g_UITexPicker, "bullet" ),
		render_Texture1( g_UITexPicker, "" ),
		render_Texture2( g_UITexPicker, "" ),
		render_Texture3( g_UITexPicker, "" ),
		render_Shader( g_UIShaderPicker, "particle" ),
		render_Additive( false ),
		render_Stretch( false ),
		
		m_psys( NULL ), m_emitter( NULL )
	{
		spawn_MaxCount.caption = "Max. particle count";
		spawn_Count.caption = "# of particles to spawn";
		spawn_CountExt.caption = "# + random";
		spawn_TimeExt.caption = "Spawn time / rand.ext.";
		
		create_Pos.caption = "Origin";
		create_PosBox.caption = "Random extents";
		create_PosRadius.caption = "Random radius";
		
		create_VelMicroDir.caption = "V/micro/direction";
		create_VelMicroDvg.caption = "V/micro/divergence";
		create_VelMicroDistExt.caption = "V/micro/dist+rand.ext.";
		create_VelMacroDir.caption = "V/macro/direction";
		create_VelMacroDvg.caption = "V/macro/divergence";
		create_VelMacroDistExt.caption = "V/macro/dist+rand.ext.";
		create_VelCluster.caption = "V - cluster size";
		create_VelClusterExt.caption = "V - cluster rand.ext.";
		
		create_LifetimeExt.caption = "Lifetime + rand.ext.";
		create_AngleDirDvg.caption = "Angle / divergence";
		create_AngleVelDvg.caption = "Angular velocity / divergence";
		tick_AngleAcc.caption = "Angle - acceleration";
		absolute.caption = "World space";
		
		render_Texture0.caption = "Texture #1";
		render_Texture1.caption = "Texture #2";
		render_Texture2.caption = "Texture #3";
		render_Texture3.caption = "Texture #4";
		render_Shader.caption = "Shader";
		render_Additive.caption = "Is additive+unlit?";
		render_Stretch.caption = "Is velocity-stretched?";
		
		grpSpawn.Add( &spawn_MaxCount );
		grpSpawn.Add( &spawn_Count );
		grpSpawn.Add( &spawn_CountExt );
		grpSpawn.Add( &spawn_TimeExt );
		Add( &grpSpawn );
		
		grpPosition.Add( &create_Pos );
		grpPosition.Add( &create_PosBox );
		grpPosition.Add( &create_PosRadius );
		Add( &grpPosition );
		
		grpVelocity.Add( &create_VelMicroDir );
		grpVelocity.Add( &create_VelMicroDvg );
		grpVelocity.Add( &create_VelMicroDistExt );
		grpVelocity.Add( &create_VelMacroDir );
		grpVelocity.Add( &create_VelMacroDvg );
		grpVelocity.Add( &create_VelMacroDistExt );
		grpVelocity.Add( &create_VelCluster );
		grpVelocity.Add( &create_VelClusterExt );
		Add( &grpVelocity );
		
		grpMisc.Add( &create_LifetimeExt );
		grpMisc.Add( &create_AngleDirDvg );
		grpMisc.Add( &create_AngleVelDvg );
		grpMisc.Add( &tick_AngleAcc );
		grpMisc.Add( &absolute );
		Add( &grpMisc );
		
		grpSizeColor.Add( &curve_Size );
		grpSizeColor.Add( &curve_ColorHue );
		grpSizeColor.Add( &curve_ColorSat );
		grpSizeColor.Add( &curve_ColorVal );
		grpSizeColor.Add( &curve_Opacity );
		Add( &grpSizeColor );
		
		grpRender.Add( &render_Texture0 );
		grpRender.Add( &render_Texture1 );
		grpRender.Add( &render_Texture2 );
		grpRender.Add( &render_Texture3 );
		grpRender.Add( &render_Shader );
		grpRender.Add( &render_Additive );
		grpRender.Add( &render_Stretch );
		Add( &grpRender );
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &spawn_MaxCount ){ m_emitter->spawn_MaxCount = spawn_MaxCount.m_value; _U(false); }
			if( e->target == &spawn_Count ){ m_emitter->spawn_Count = spawn_Count.m_value; _U(false); }
			if( e->target == &spawn_CountExt ){ m_emitter->spawn_CountExt = spawn_CountExt.m_value; _U(false); }
			if( e->target == &spawn_TimeExt ){ m_emitter->spawn_TimeExt = spawn_TimeExt.m_value; _U(false); }
			if( e->target == &create_Pos ){ m_emitter->create_Pos = create_Pos.m_value; _U(false); }
			if( e->target == &create_PosBox ){ m_emitter->create_PosBox = create_PosBox.m_value; _U(false); }
			if( e->target == &create_PosRadius ){ m_emitter->create_PosRadius = create_PosRadius.m_value; _U(false); }
			if( e->target == &create_VelMicroDir ){ m_emitter->create_VelMicroDir = create_VelMicroDir.m_value; _U(false); }
			if( e->target == &create_VelMicroDvg ){ m_emitter->create_VelMicroDvg = create_VelMicroDvg.m_value; _U(false); }
			if( e->target == &create_VelMicroDistExt ){ m_emitter->create_VelMicroDistExt = create_VelMicroDistExt.m_value; _U(false); }
			if( e->target == &create_VelMacroDir ){ m_emitter->create_VelMacroDir = create_VelMacroDir.m_value; _U(false); }
			if( e->target == &create_VelMacroDvg ){ m_emitter->create_VelMacroDvg = create_VelMacroDvg.m_value; _U(false); }
			if( e->target == &create_VelMacroDistExt ){ m_emitter->create_VelMacroDistExt = create_VelMacroDistExt.m_value; _U(false); }
			if( e->target == &create_VelCluster ){ m_emitter->create_VelCluster = create_VelCluster.m_value; _U(false); }
			if( e->target == &create_VelClusterExt ){ m_emitter->create_VelClusterExt = create_VelClusterExt.m_value; _U(false); }
			if( e->target == &create_LifetimeExt ){ m_emitter->create_LifetimeExt = create_LifetimeExt.m_value; _U(false); }
			if( e->target == &create_AngleDirDvg ){ m_emitter->create_AngleDirDvg = DEG2RAD( create_AngleDirDvg.m_value ); _U(false); }
			if( e->target == &create_AngleVelDvg ){ m_emitter->create_AngleVelDvg = DEG2RAD( create_AngleVelDvg.m_value ); _U(false); }
			if( e->target == &tick_AngleAcc ){ m_emitter->tick_AngleAcc = DEG2RAD( tick_AngleAcc.m_value ); _U(false); }
			if( e->target == &absolute ){ m_emitter->absolute = absolute.m_value; _U(false); }
			if( e->target == &render_Texture0 ){ m_emitter->render_Textures[0] = GetParticleTexHandle( render_Texture0.m_value ); _U(true); }
			if( e->target == &render_Texture1 ){ m_emitter->render_Textures[1] = GetParticleTexHandle( render_Texture1.m_value ); _U(true); }
			if( e->target == &render_Texture2 ){ m_emitter->render_Textures[2] = GetParticleTexHandle( render_Texture2.m_value ); _U(true); }
			if( e->target == &render_Texture3 ){ m_emitter->render_Textures[3] = GetParticleTexHandle( render_Texture3.m_value ); _U(true); }
			if( e->target == &render_Shader ){ m_emitter->render_Shader = render_Shader.m_value; _U(true); }
			if( e->target == &render_Additive ){ m_emitter->render_Additive = render_Additive.m_value; _U(true); }
			if( e->target == &render_Stretch ){ m_emitter->render_Stretch = render_Stretch.m_value; _U(true); }
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	void _U( bool renderupdate )
	{
		if( renderupdate )
			m_psys->OnRenderUpdate();
	}
	
	void Prepare( ParticleSystem* PS, size_t emid )
	{
		m_psys = PS;
		m_emitter = &PS->emitters[ emid ];
		
		spawn_MaxCount.SetValue( m_emitter->spawn_MaxCount );
		spawn_Count.SetValue( m_emitter->spawn_Count );
		spawn_CountExt.SetValue( m_emitter->spawn_CountExt );
		spawn_TimeExt.SetValue( m_emitter->spawn_TimeExt );
		
		create_Pos.SetValue( m_emitter->create_Pos );
		create_PosBox.SetValue( m_emitter->create_PosBox );
		create_PosRadius.SetValue( m_emitter->create_PosRadius );
	
		create_VelMicroDir.SetValue( m_emitter->create_VelMicroDir );
		create_VelMicroDvg.SetValue( m_emitter->create_VelMicroDvg );
		create_VelMicroDistExt.SetValue( m_emitter->create_VelMicroDistExt );
		create_VelMacroDir.SetValue( m_emitter->create_VelMacroDir );
		create_VelMacroDvg.SetValue( m_emitter->create_VelMacroDvg );
		create_VelMacroDistExt.SetValue( m_emitter->create_VelMacroDistExt );
		create_VelCluster.SetValue( m_emitter->create_VelCluster );
		create_VelClusterExt.SetValue( m_emitter->create_VelClusterExt );
		
		create_LifetimeExt.SetValue( m_emitter->create_LifetimeExt );
		create_AngleDirDvg.SetValue( RAD2DEG( m_emitter->create_AngleDirDvg ) );
		create_AngleVelDvg.SetValue( RAD2DEG( m_emitter->create_AngleVelDvg ) );
		tick_AngleAcc.SetValue( RAD2DEG( m_emitter->tick_AngleAcc ) );
		absolute.SetValue( m_emitter->absolute );
		
		curve_Size.SetValue( m_emitter->curve_Size );
		curve_ColorHue.SetValue( m_emitter->curve_ColorHue );
		curve_ColorSat.SetValue( m_emitter->curve_ColorSat );
		curve_ColorVal.SetValue( m_emitter->curve_ColorVal );
		curve_Opacity.SetValue( m_emitter->curve_Opacity );
		
		render_Texture0.SetValue( GetParticleTexName( m_emitter->render_Textures[0] ) );
		render_Texture1.SetValue( GetParticleTexName( m_emitter->render_Textures[1] ) );
		render_Texture2.SetValue( GetParticleTexName( m_emitter->render_Textures[2] ) );
		render_Texture3.SetValue( GetParticleTexName( m_emitter->render_Textures[3] ) );
		render_Shader.SetValue( m_emitter->render_Shader );
		render_Additive.SetValue( m_emitter->render_Additive );
		render_Stretch.SetValue( m_emitter->render_Stretch );
	}
	
	EDGUIGroup grpSpawn;
	EDGUIPropInt spawn_MaxCount;
	EDGUIPropInt spawn_Count;
	EDGUIPropInt spawn_CountExt;
	EDGUIPropVec2 spawn_TimeExt;
	
	EDGUIGroup grpPosition;
	EDGUIPropVec3 create_Pos;
	EDGUIPropVec3 create_PosBox;
	EDGUIPropFloat create_PosRadius;
	
	EDGUIGroup grpVelocity;
	EDGUIPropVec3 create_VelMicroDir;
	EDGUIPropFloat create_VelMicroDvg;
	EDGUIPropVec2 create_VelMicroDistExt;
	EDGUIPropVec3 create_VelMacroDir;
	EDGUIPropFloat create_VelMacroDvg;
	EDGUIPropVec2 create_VelMacroDistExt;
	EDGUIPropInt create_VelCluster;
	EDGUIPropInt create_VelClusterExt;
	
	EDGUIGroup grpMisc;
	EDGUIPropVec2 create_LifetimeExt;
	EDGUIPropVec2 create_AngleDirDvg;
	EDGUIPropVec2 create_AngleVelDvg;
	EDGUIPropFloat tick_AngleAcc;
	EDGUIPropBool absolute;
	
	EDGUIGroup grpSizeColor;
	EdEmitCurve curve_Size;
	EdEmitCurve curve_ColorHue;
	EdEmitCurve curve_ColorSat;
	EdEmitCurve curve_ColorVal;
	EdEmitCurve curve_Opacity;
	
	EDGUIGroup grpRender;
	EDGUIPropRsrc render_Texture0;
	EDGUIPropRsrc render_Texture1;
	EDGUIPropRsrc render_Texture2;
	EDGUIPropRsrc render_Texture3;
	EDGUIPropRsrc render_Shader;
	EDGUIPropBool render_Additive;
	EDGUIPropBool render_Stretch;
	
	ParticleSystem* m_psys;
	ParticleSystem::Emitter* m_emitter;
};

struct EdParticleSystem : EDGUILayoutRow
{
	EdParticleSystem() :
		m_ctlGroup( true, "Particle system properties" ),
		m_ctlGravity( V3(0,0,-10), 2, V3(-8192), V3(8192) ),
		m_ctlMaxGroupCount( 10, 1, 128 ),
		m_ctlGlobalScale( 1, 2, 0.001f, 1000.0f ),
		m_ctlLooping( true ),
		m_ctlRetriggerTime( V2(1,0.1f), 2, V2(0), V2(1000) )
	{
		tyname = "particlesystem";
		
		m_ctlGroup.caption = "Particle system properties";
		m_ctlGravity.caption = "Gravity";
		m_ctlMaxGroupCount.caption = "# lighting groups";
		m_ctlGlobalScale.caption = "Global scale";
		m_ctlLooping.caption = "Looping";
		m_ctlRetriggerTime.caption = "Retrigger time/ext.";
		m_ctlAddEmitter.caption = "- Add emitter -";
		
		m_ctlGroup.SetOpen( true );
		Add( &m_ctlGroup );
		
		Reset();
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			for( size_t i = 0; i < m_ctlEmEditBtns.size(); ++i )
			{
				if( e->target == &m_ctlEmEditBtns[ i ] )
				{
					EDGUILayoutRow::OnEvent( e );
					EditEmitter( i );
					return 1;
				}
			}
			if( e->target == &m_ctlAddEmitter )
			{
				EDGUILayoutRow::OnEvent( e );
				AddDefaultEmitter();
				m_psys.OnRenderUpdate();
				EditEmitter( m_psys.emitters.size() - 1 );
				return 1;
			}
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_ctlGravity ) m_psys.gravity = m_ctlGravity.m_value;
			if( e->target == &m_ctlMaxGroupCount ) m_psys.maxGroupCount = m_ctlMaxGroupCount.m_value;
			if( e->target == &m_ctlGlobalScale ) m_psys.globalScale = m_ctlGlobalScale.m_value;
			if( e->target == &m_ctlLooping ) m_psys.looping = m_ctlLooping.m_value;
			if( e->target == &m_ctlRetriggerTime ) m_psys.retriggerTimeExt = m_ctlRetriggerTime.m_value;
			return 1;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	void Tick( float dt )
	{
		m_psys.Tick( dt );
		m_psys.PreRender();
	}
	
	void Data2UI()
	{
		m_ctlGroup.Clear();
		
		m_ctlGravity.SetValue( m_psys.gravity );
		m_ctlMaxGroupCount.SetValue( m_psys.maxGroupCount );
		m_ctlGlobalScale.SetValue( m_psys.globalScale );
		m_ctlLooping.SetValue( m_psys.looping );
		m_ctlRetriggerTime.SetValue( m_psys.retriggerTimeExt );
		
		m_ctlGroup.Add( &m_ctlGravity );
		m_ctlGroup.Add( &m_ctlMaxGroupCount );
		m_ctlGroup.Add( &m_ctlGlobalScale );
		m_ctlGroup.Add( &m_ctlLooping );
		m_ctlGroup.Add( &m_ctlRetriggerTime );
		
		m_ctlEmEditBtns.clear();
		m_ctlEmEditBtns.reserve( m_psys.emitters.size() );
		for( size_t i = 0; i < m_psys.emitters.size(); ++i )
		{
			m_ctlEmEditBtns.push_back( EDGUIButton() );
			EDGUIButton& B = m_ctlEmEditBtns.last();
			
			char bfr[32];
			sprintf( bfr, "Emitter #%d", (int)i + 1 );
			B.caption = bfr;
			B.id1 = i;
			
			m_ctlGroup.Add( &B );
		}
		
		m_ctlGroup.Add( &m_ctlAddEmitter );
	}
	
	void Reset()
	{
		m_psys = ParticleSystem();
		AddDefaultEmitter();
		
		m_psys.AddToScene( g_EdScene );
		m_psys.OnRenderUpdate();
		m_psys.SetTransform( Mat4::Identity );
		m_psys.Play();
	}
	void AddDefaultEmitter()
	{
		if( g_UIGraphEd ) // order of initialization
			g_UIGraphEd->UnsetCurve();
		m_psys.emitters.push_back( ParticleSystem::Emitter() );
		ParticleSystem::Emitter& E = m_psys.emitters.last();
		E.curve_ColorVal.valueRange = V2(1);
		E.curve_Opacity.valueRange = V2(0,1);
		Vec2 opa[] = { V2(0,0), V2(1,0), V2(1,0), V2(0,0) };
		E.curve_Opacity.values.append( opa, sizeof(opa)/sizeof(opa[0]) );
		E.curve_Size.valueRange = V2(1);
		E.create_VelMacroDvg = 0.5f;
		E.render_Textures[0] = GR_GetTexture( "textures/particles/spark_fast.png" );
	}
	
	EDGUIItem* GetSystem()
	{
		Data2UI();
		return this;
	}
	EDGUIItem* GetEmitter( size_t which )
	{
		m_ctlEmitter.Prepare( &m_psys, which );
		return &m_ctlEmitter;
	}
	
	void EditEmitter( size_t which );
	
	void Play(){ m_psys.Play(); }
	void Stop(){ m_psys.Stop(); }
	void Trigger(){ m_psys.Trigger(); }
	
	template< class T > void Serialize( T& arch )
	{
		m_psys.Serialize( arch, false );
	}
	
	ParticleSystem m_psys;
	
	EdEmitter m_ctlEmitter;
	
	EDGUIGroup m_ctlGroup;
	EDGUIPropVec3 m_ctlGravity;
	EDGUIPropInt m_ctlMaxGroupCount;
	EDGUIPropFloat m_ctlGlobalScale;
	EDGUIPropBool m_ctlLooping;
	EDGUIPropVec2 m_ctlRetriggerTime;
	Array< EDGUIButton > m_ctlEmEditBtns;
	EDGUIButton m_ctlAddEmitter;
};



struct EDGUIMainFrame : EDGUIFrame, EDGUIRenderView::FrameInterface
{
	EDGUIMainFrame() :
		m_UIMenuSplit( true, 26, 0 ),
		m_UIGraphSplit( true, 0, 0.7f ),
		m_UIParamSplit( false, 0, 0.7f ),
		m_UIRenderView( g_EdScene, this )
	{
		tyname = "mainframe";
		
		g_UIGraphEd = &m_UIGraph;
		
		Add( &m_UIMenuSplit );
		m_UIMenuSplit.SetFirstPane( &m_UIMenuButtons );
		m_UIMenuSplit.SetSecondPane( &m_UIGraphSplit );
		m_UIGraphSplit.SetFirstPane( &m_UIParamSplit );
		m_UIGraphSplit.SetSecondPane( &m_UIGraph );
		m_UIParamSplit.SetFirstPane( &m_UIRenderView );
		m_UIParamSplit.SetSecondPane( &m_UIParamList );
		
		// menu
		m_MB_Cat0.caption = "File:";
		m_MBNew.caption = "New";
		m_MBOpen.caption = "Open";
		m_MBSave.caption = "Save";
		m_MBSaveAs.caption = "Save As";
		m_MB_Cat1.caption = "Playback:";
		m_MBPlay.caption = "Play";
		m_MBStop.caption = "Stop";
		m_MBTrigger.caption = "Trigger";
		m_MB_Cat2.caption = "Edit:";
		m_MBEditSystem.caption = "System";
		m_UIMenuButtons.Add( &m_MB_Cat0 );
		m_UIMenuButtons.Add( &m_MBNew );
		m_UIMenuButtons.Add( &m_MBOpen );
		m_UIMenuButtons.Add( &m_MBSave );
		m_UIMenuButtons.Add( &m_MBSaveAs );
		m_UIMenuButtons.Add( &m_MB_Cat1 );
		m_UIMenuButtons.Add( &m_MBPlay );
		m_UIMenuButtons.Add( &m_MBStop );
		m_UIMenuButtons.Add( &m_MBTrigger );
		m_UIMenuButtons.Add( &m_MB_Cat2 );
		m_UIMenuButtons.Add( &m_MBEditSystem );
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if(0);
			
			else if( e->target == &m_MBNew ) PS_New();
			else if( e->target == &m_MBOpen ) PS_Open();
			else if( e->target == &m_MBSave ) PS_Save();
			else if( e->target == &m_MBSaveAs ) PS_SaveAs();
			
			else if( e->target == &m_MBPlay ) g_EdPS->Play();
			else if( e->target == &m_MBStop ) g_EdPS->Stop();
			else if( e->target == &m_MBTrigger ) g_EdPS->Trigger();
			
			else if( e->target == &m_MBEditSystem )
			{
				ClearParamList();
				AddToParamList( g_EdPS->GetSystem() );
			}
			
			return 1;
			
		case EDGUI_EVENT_PROPCHANGE:
			if( e->target == g_UIPSOpenPicker )
			{
				PS_Real_Open( g_UIPSOpenPicker->GetValue() );
			}
			if( e->target == g_UIPSSavePicker )
			{
				PS_Real_Save( g_UIPSSavePicker->GetValue() );
			}
			return 1;
		}
		return EDGUIFrame::OnEvent( e );
	}
	
	bool ViewEvent( EDGUIEvent* e )
	{
		return true;
	}
	
	void DebugDraw()
	{
	}
	
	void AddToParamList( EDGUIItem* item )
	{
		m_UIParamList.Add( item );
	}
	void ClearParamList()
	{
		while( m_UIParamList.m_subitems.size() )
			m_UIParamList.Remove( m_UIParamList.m_subitems.last() );
	}
	
	void ResetEditorState()
	{
		ClearParamList();
	}
	void PS_New()
	{
		m_fileName = "";
		g_EdPS->Reset();
		ResetEditorState();
	}
	void PS_Open()
	{
		g_UIPSOpenPicker->Reload();
		g_UIPSOpenPicker->Open( this, "" );
		m_frame->Add( g_UIPSOpenPicker );
	}
	void PS_Save()
	{
		if( m_fileName.size() )
		{
			PS_Real_Save( m_fileName );
		}
		else
		{
			PS_SaveAs();
		}
	}
	void PS_SaveAs()
	{
		g_UIPSSavePicker->Reload();
		g_UIPSSavePicker->Open( this, "" );
		m_frame->Add( g_UIPSSavePicker );
	}
	void PS_Real_Open( const String& str )
	{
		LOG << "Trying to open particle system: " << str;
		
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "psys/%.*s.psy", TMIN( (int) str.size(), 200 ), str.data() );
		ByteArray data;
		if( !FS_LoadBinaryFile( bfr, data ) )
		{
			LOG_ERROR << "FAILED TO LOAD PSYS FILE: " << bfr;
			return;
		}
		
		ResetEditorState();
		
		ByteReader br( &data );
		g_EdPS->Serialize( br );
		if( br.error )
		{
			LOG_ERROR << "FAILED TO READ PSYS FILE (at " << (int) br.pos << "): " << bfr;
			return;
		}
		
		m_fileName = str;
		g_EdPS->Play();
	}
	void PS_Real_Save( const String& str )
	{
		LOG << "Trying to save particle system: " << str;
		ByteArray data;
		ByteWriter bw( &data );
		
		bw << *g_EdPS;
		
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "psys/%.*s.psy", TMIN( (int) str.size(), 200 ), str.data() );
		if( !FS_SaveBinaryFile( bfr, data.data(), data.size() ) )
		{
			LOG_ERROR << "FAILED TO SAVE PSYS FILE: " << bfr;
			return;
		}
		
		m_fileName = str;
	}
	
	String m_fileName;
	
	// core layout
	EDGUILayoutSplitPane m_UIMenuSplit;
	EDGUILayoutSplitPane m_UIGraphSplit;
	EDGUILayoutSplitPane m_UIParamSplit;
	EDGUILayoutColumn m_UIMenuButtons;
	EDGUILayoutRow m_UIParamList;
	EDGUIRenderView m_UIRenderView;
	EdDualGraph m_UIGraph;

	// menu
	EDGUILabel m_MB_Cat0;
	EDGUIButton m_MBNew;
	EDGUIButton m_MBOpen;
	EDGUIButton m_MBSave;
	EDGUIButton m_MBSaveAs;
	EDGUILabel m_MB_Cat1;
	EDGUIButton m_MBPlay;
	EDGUIButton m_MBStop;
	EDGUIButton m_MBTrigger;
	EDGUILabel m_MB_Cat2;
	EDGUIButton m_MBEditSystem;
};

void EdParticleSystem::EditEmitter( size_t which )
{
	g_UIFrame->ClearParamList();
	g_UIFrame->AddToParamList( GetEmitter( which ) );
}





SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
};

struct PSEditor : IGame
{
	bool OnInitialize()
	{
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
		g_UITexPicker = new EDGUISDTexPicker( "textures/particles" );
		g_UIShaderPicker = new EDGUIShaderPicker;
		g_UIMeshPicker = new EDGUIMeshPicker;
		g_UIPSOpenPicker = new EDGUIPSOpenPicker;
		g_UIPSSavePicker = new EDGUIPSSavePicker;
		
		// core layout
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = V3(3);
		g_EdScene->camera.UpdateMatrices();
		g_EdPS = new EdParticleSystem;
		g_UIFrame = new EDGUIMainFrame();
		g_UIFrame->Resize( GR_GetWidth(), GR_GetHeight() );
		
		// param area
		g_UIFrame->AddToParamList( g_EdPS->GetSystem() );
		
		return true;
	}
	void OnDestroy()
	{
		delete g_UIPSSavePicker;
		g_UIPSSavePicker = NULL;
		delete g_UIPSOpenPicker;
		g_UIPSOpenPicker = NULL;
		delete g_UIMeshPicker;
		g_UIMeshPicker = NULL;
		delete g_UIShaderPicker;
		g_UIShaderPicker = NULL;
		delete g_UITexPicker;
		g_UITexPicker = NULL;
		delete g_UIFrame;
		g_UIFrame = NULL;
		delete g_EdPS;
		g_EdPS = NULL;
		g_EdScene = NULL;
	}
	void OnEvent( const Event& e )
	{
		g_UIFrame->EngineEvent( &e );
	}
	void OnTick( float dt, uint32_t gametime )
	{
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		g_UIFrame->m_UIRenderView.UpdateCamera( dt );
		g_EdPS->Tick( dt );
		g_UIFrame->Draw();
	}
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

