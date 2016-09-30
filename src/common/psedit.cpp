

#include <engine.hpp>
#include <enganim.hpp>
#include <imgui.hpp>


SceneHandle g_EdScene;
ParticleSystem* g_PSys;
struct IMGUIRenderView* g_NUIRenderView;
struct IMGUIFilePicker* g_NUIPartSysPicker;
IMGUITexturePicker* g_NUITexturePicker;


#define SGRXPATH_SRC_EDITOR_PSYSISFX SGRXPATH_SRC_EDITOR "/psys_isfx.txt"


#define MAX_PARTICLES 10000
#define MAX_TIME 120.0f
#define MAX_POS 8192.0f
#define MAX_NORMAL 100.0f
#define MAX_VEL 100.0f


float curve_values_getter_x( void* data, int i ){ return ((Vec2*)data)[ i ].x; }
float curve_values_getter_y( void* data, int i ){ return ((Vec2*)data)[ i ].y; }

int g_WhichCurve = 0;
void EditPSCurve( int cid, const char* name, ParticleSystem::Emitter::Curve& curve )
{
	IMGUI_GROUP( name, true,
	{
		if( ImGui::Selectable( "Edit curve" ) )
			g_WhichCurve = cid;
		int curveSize = curve.values.size();
		IMGUIEditInt( "# of sample points", curveSize, 1, 1024 );
		if( curveSize < 1 ) curveSize = 1;
		if( curveSize > 1024 ) curveSize = 1024;
		curve.values.resize_using( curveSize, V2(0.0f) );
		ImGui::PlotLines( "Base curve", curve_values_getter_x, curve.values.data(), curve.values.size(),
			0, NULL, 0, 1 );
		ImGui::PlotLines( "Random curve", curve_values_getter_y, curve.values.data(), curve.values.size(),
			0, NULL, 0, 1 );
		IMGUIEditVec2( "Random value/divergence", curve.randomValDvg, -100, 100 );
		IMGUIEditVec2( "Min./max. values", curve.valueRange, -1000, 1000 );
	});
}

void EditCurveWindow( ParticleSystem::Emitter& em )
{
	static const char* curve_names[] = {
		"Size", "Color - hue", "Color - saturation", "Color - value", "Opacity"
	};
	ParticleSystem::Emitter::Curve* pcurve = NULL;
	if( g_WhichCurve == 0 ) pcurve = &em.curve_Size;
	if( g_WhichCurve == 1 ) pcurve = &em.curve_ColorHue;
	if( g_WhichCurve == 2 ) pcurve = &em.curve_ColorSat;
	if( g_WhichCurve == 3 ) pcurve = &em.curve_ColorVal;
	if( g_WhichCurve == 4 ) pcurve = &em.curve_Opacity;
	
	if( pcurve )
	{
		if( ImGui::Begin( "Curve Editor", NULL, ImGuiWindowFlags_NoScrollbar ) )
		{
			ImGui::SetWindowPos( ImVec2( 24, 48 ), ImGuiSetCond_Appearing );
			ImGui::SetWindowSize( ImVec2( 600, 160 ), ImGuiSetCond_Appearing );
			ImGui::Text( "%s", curve_names[ g_WhichCurve ] );
			
			ImVec2 rect = ImGui::GetContentRegionAvail();
			rect.x -= 20;
			if( rect.x < 100 )
				rect.x = 100;
			if( rect.y < 40 )
				rect.y = 40;
			if( ImGui::BeginChildFrame( 0, rect ) )
			{
				ImVec2 imin = ImGui::GetWindowPos() + ImVec2( 5, 5 );
				ImVec2 imax = imin + ImGui::GetWindowSize() - ImVec2( 10, 10 );
				ImVec2 cp = ImGui::GetMousePos();
				Vec2 min = V2( imin.x, imin.y );
				Vec2 max = V2( imax.x, imax.y );
				
				ImDrawList* idl = ImGui::GetWindowDrawList();
				
				idl->PathClear();
				for( size_t i = 0; i < pcurve->values.size(); ++i )
				{
					float xq = safe_fdiv( i, pcurve->values.size() - 1 );
					Vec2 pos = min + ( max - min ) * V2( xq, 1 - pcurve->values[ i ].x );
					idl->PathLineTo( ImVec2( pos.x, pos.y ) );
				}
				idl->PathStroke( ImColor( 0.1f, 0.9f, 0.0f ), false, 1.0f );
				idl->PathClear();
				
				idl->PathClear();
				for( size_t i = 0; i < pcurve->values.size(); ++i )
				{
					float xq = safe_fdiv( i, pcurve->values.size() - 1 );
					Vec2 pos = min + ( max - min ) * V2( xq, 1 - pcurve->values[ i ].y );
					idl->PathLineTo( ImVec2( pos.x, pos.y ) );
				}
				idl->PathStroke( ImColor( 0.9f, 0.1f, 0.0f, 0.6f ), false, 1.0f );
				idl->PathClear();
				
				float x = TREVLERP<float>( imin.x, imax.x, cp.x );
				float y = clamp( 1 - TREVLERP<float>( imin.y, imax.y, cp.y ), 0, 1 );
				int num = floor( x * ( pcurve->values.size() - 1 ) + 0.5f );
				if( num < 0 )
					num = 0;
				if( num > (int) pcurve->values.size() - 1 )
					num = (int) pcurve->values.size() - 1;
				x = TLERP( imin.x, imax.x,safe_fdiv( num, pcurve->values.size() - 1 ) );
				idl->AddLine( ImVec2( x, imin.y ), ImVec2( x, imax.y ), ImColor( 0.9f, 0.9f, 0.9f, 0.5f ), 1 );
				
				if( ImGui::IsMouseHoveringWindow() )
				{
					if( ImGui::IsMouseDown( 0 ) ) pcurve->values[ num ].x = y;
					if( ImGui::IsMouseDown( 1 ) ) pcurve->values[ num ].y = y;
				}
			}
			ImGui::EndChildFrame();
			
		}
		ImGui::End();
	}
}


HashTable< int, RCString > g_ISFX_Options;
void ISFX_Reload()
{
	LOG << "Reloading particle intersection effects";
	
	g_ISFX_Options.clear();
	
	String isfx_data;
	if( FS_LoadTextFile( SGRXPATH_SRC_EDITOR_PSYSISFX, isfx_data ) == false )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load " SGRXPATH_SRC_EDITOR_PSYSISFX;
		return;
	}
	ConfigReader cr( isfx_data );
	StringView key, value;
	while( cr.Read( key, value ) )
	{
		int id = String_ParseInt( key );
		char bfr[ 32 ];
		sgrx_snprintf( bfr, 32, "%d: ", id );
		String name = bfr;
		name.append( value );
		g_ISFX_Options.set( id, name );
	}
	
	LOG << "Loaded " << g_ISFX_Options.size() << " effects!";
}

IMGUIShaderPicker g_ShaderPicker;

void EditPSEmitter( ParticleSystem::Emitter& em )
{
	EditCurveWindow( em );
	
	IMGUI_GROUP( "Spawn properties", true,
	{
		IMGUIEditInt( "Max. number of particles", em.spawn_MaxCount, 0, MAX_PARTICLES );
		IMGUIEditInt( "# of particles to spawn", em.spawn_Count, 0, MAX_PARTICLES );
		IMGUIEditInt( "# + random", em.spawn_CountExt, 0, MAX_PARTICLES );
		IMGUIEditVec2( "Spawn time/rand. ext.", em.spawn_TimeExt, 0, MAX_TIME );
	});
	
	IMGUI_GROUP( "Initial position", true,
	{
		IMGUIEditVec3( "Origin", em.create_Pos, -MAX_POS, MAX_POS );
		IMGUIEditVec3( "Random extents", em.create_PosBox, 0, MAX_POS );
		IMGUIEditFloat( "Random radius", em.create_PosRadius, 0, MAX_POS );
	});
	
	IMGUI_GROUP( "Initial velocity", true,
	{
		IMGUIEditVec3( "V/micro/direction", em.create_VelMicroDir, -MAX_NORMAL, MAX_NORMAL );
		IMGUIEditFloat( "V/micro/divergence", em.create_VelMicroDvg, 0, 1 );
		IMGUIEditVec2( "V/micro/dist+rand.ext.", em.create_VelMicroDistExt, -MAX_VEL, MAX_VEL );
		IMGUIEditVec3( "V/macro/direction", em.create_VelMacroDir, -MAX_NORMAL, MAX_NORMAL );
		IMGUIEditFloat( "V/macro/divergence", em.create_VelMacroDvg, 0, 1 );
		IMGUIEditVec2( "V/macro/dist+rand.ext.", em.create_VelMacroDistExt, -MAX_VEL, MAX_VEL );
		IMGUIEditInt( "V - cluster size", em.create_VelCluster, 0, MAX_PARTICLES );
		IMGUIEditInt( "V - cluster rand.ext.", em.create_VelClusterExt, 0, MAX_PARTICLES );
	});
	
	IMGUI_GROUP( "Misc. properties", true,
	{
		IMGUIEditVec2( "Lifetime + rand.ext.", em.create_LifetimeExt, 0, MAX_TIME );
		IMGUIEditVec2( "Angle / divergence", em.create_AngleDirDvg, 0, 360 );
		IMGUIEditVec2( "Angular velocity / divergence", em.create_AngleVelDvg, -1000, 1000 );
		IMGUIEditFloat( "Angle - acceleration", em.tick_AngleAcc, -10000, 10000 );
		IMGUIEditFloat( "Gravity mulitplier", em.tick_GravityMult, 0, 100 );
		IMGUIEditBool( "World space", em.absolute );
	});
	
	IMGUI_GROUP( "Intersection", true,
	{
		IMGUIEditInt( "Limit", em.isect_Limit, 0, MAX_PARTICLES );
		IMGUIEditFloatSlider( "Friction", em.isect_Friction, 0, 1 );
		IMGUIEditFloatSlider( "Bounce", em.isect_Bounce, 0, 1 );
		
		RCString s = g_ISFX_Options.getcopy( em.isect_FXID );
		if( ImGui::Button( s ? s.c_str() : "<unknown>",
			ImVec2( ImGui::GetContentRegionAvailWidth() * 2.f/3.f, 20 ) ) )
			ImGui::OpenPopup( "edit_isfx" );
		ImGui::SameLine();
		ImGui::Text( "Hit FX" );
		if( ImGui::BeginPopup( "edit_isfx" ) )
		{
			for( size_t i = 0; i < g_ISFX_Options.size(); ++i )
			{
				if( ImGui::Selectable( g_ISFX_Options.item( i ).value.c_str() ) )
					em.isect_FXID = g_ISFX_Options.item( i ).key;
			}
			ImGui::EndPopup();
		}
		
		IMGUIEditFloatSlider( "Hit FX chance", em.isect_FXChance, 0, 1 );
		IMGUIEditBool( "Remove on hit", em.isect_Remove );
	});
	
	IMGUI_GROUP( "Size/color curves", true,
	{
		EditPSCurve( 0, "Size", em.curve_Size );
		EditPSCurve( 1, "Hue", em.curve_ColorHue );
		EditPSCurve( 2, "Saturation", em.curve_ColorSat );
		EditPSCurve( 3, "Value", em.curve_ColorVal );
		EditPSCurve( 4, "Opacity", em.curve_Opacity );
	});
	
	IMGUI_GROUP( "Rendering data", true,
	{
		ImGui::BeginChangeCheck();
		
		for( int i = 0; i < NUM_PARTICLE_TEXTURES; ++i )
		{
			char popcap[32];
			char label[32];
			sgrx_snprintf( popcap, 32, "Select texture %d", i );
			sgrx_snprintf( label, 32, "Texture %d", i );
			String name = em.render_Textures[i] ? em.render_Textures[i]->m_key : "";
			if( g_NUITexturePicker->Property( popcap, label, name ) )
				em.render_Textures[i] = GR_GetTexture( name );
		}
		g_ShaderPicker.Property( "Shader", em.render_Shader );
		IMGUIEditBool( "Additive", em.render_Additive );
		IMGUIEditBool( "Stretch", em.render_Stretch );
		
		if( ImGui::EndChangeCheck() )
		{
			g_PSys->OnRenderUpdate();
		}
	});
}

size_t g_CurEmitter = NOT_FOUND;

void EditPSEmBtn( size_t i, ParticleSystem::Emitter& em )
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "Emitter %d, tex0=%s", (int) i,
		StackPath(em.render_Textures[0] ? em.render_Textures[0]->m_key : "<none>").str );
	if( ImGui::Selectable( bfr, false, 0, ImVec2( ImGui::GetContentRegionAvailWidth() - 98, 20 ) ) )
	{
		g_CurEmitter = i;
	}
}

void EditParticleSystem()
{
	ParticleSystem& ps = *g_PSys;
	
	if( g_CurEmitter < ps.emitters.size() )
	{
		if( ImGui::Selectable( "Back to system" ) )
		{
			g_CurEmitter = NOT_FOUND;
			return;
		}
		ImGui::Separator();
		
		ImGui::Text( "Emitter %d", (int) g_CurEmitter );
		ImGui::Separator();
		
		ImGui::Columns( 2 );
		if( ImGui::Button( "Duplicate", ImVec2( ImGui::GetContentRegionAvailWidth(), 20 ) ) )
		{
			ParticleSystem::Emitter em = ps.emitters[ g_CurEmitter ];
			ps.emitters.push_back( em );
			g_CurEmitter = ps.emitters.size() - 1;
		}
		ImGui::NextColumn();
		if( ImGui::Button( "Delete", ImVec2( ImGui::GetContentRegionAvailWidth(), 20 ) ) )
		{
			ps.emitters.erase( g_CurEmitter );
			g_CurEmitter = NOT_FOUND;
			return;
		}
		ImGui::Columns( 1 );
		ImGui::Separator();
		
		EditPSEmitter( ps.emitters[ g_CurEmitter ] );
		return;
	}
	
	IMGUI_GROUP( "System", true,
	{
		IMGUIEditVec3( "Gravity", ps.gravity, -1000, 1000 );
		IMGUIEditInt( "# Max. lighting groups", ps.maxGroupCount, 1, 1000 );
		IMGUIEditFloat( "Global scale", ps.globalScale, 0.001f, 1000 );
		IMGUIEditBool( "Looping", ps.looping );
		IMGUIEditVec2( "Retrigger time/ext.", ps.retriggerTimeExt, 0, 1000 );
	});
	
	IMGUI_GROUP( "Emitters", true,
	{
		IMGUIEditArray( ps.emitters, EditPSEmBtn, "Add emitter" );
	});
}


String g_fileName;
enum EditorMode
{
	EditPartSys,
	MiscProps,
};
int g_mode = EditPartSys;


void PSAddDefaultEmitter()
{
	g_PSys->emitters.push_back( ParticleSystem::Emitter() );
	ParticleSystem::Emitter& E = g_PSys->emitters.last();
	E.curve_ColorVal.valueRange = V2(1);
	E.curve_Opacity.valueRange = V2(0,1);
	Vec2 opa[] = { V2(0,0), V2(1,0), V2(1,0), V2(0,0) };
	E.curve_Opacity.values.append( opa, sizeof(opa)/sizeof(opa[0]) );
	E.curve_Size.valueRange = V2(1);
	E.create_VelMacroDvg = 0.5f;
	E.render_Textures[0] = GR_GetTexture( "textures/particles/spark_fast.png" );
}
void PSCreate()
{
	g_PSys = new ParticleSystem;
	PSAddDefaultEmitter();
	
	g_PSys->AddToScene( g_EdScene );
	g_PSys->OnRenderUpdate();
	g_PSys->SetTransform( Mat4::Identity );
	g_PSys->Play();
}


struct PSEditor : IGame
{
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
		// core layout
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = V3(3);
		g_EdScene->camera.znear = 0.1f;
		g_EdScene->camera.UpdateMatrices();
		
		PSCreate();
		
		SGRX_IMGUI_Init();
		
		g_NUIRenderView = new IMGUIRenderView( g_EdScene );
		g_NUIPartSysPicker = new IMGUIFilePicker( SGRXPATH_SRC "/psys", ".psy" );
		g_NUITexturePicker = new IMGUITexturePicker();
		ISFX_Reload();
		
		return true;
	}
	void OnDestroy()
	{
		delete g_NUITexturePicker;
		delete g_NUIPartSysPicker;
		delete g_NUIRenderView;
		
		delete g_PSys;
		g_PSys = NULL;
		
		g_EdScene = NULL;
		
		SGRX_IMGUI_Free();
	}
	void OnEvent( const Event& e )
	{
		SGRX_IMGUI_Event( e );
	}
	void OnTick( float dt, uint32_t gametime )
	{
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		
		g_PSys->Tick( dt );
		
		SGRX_IMGUI_NewFrame( dt );
		
		IMGUI_MAIN_WINDOW_BEGIN
		{
			bool needOpen = false;
			bool needSave = false;
			bool needSaveAs = false;
			
			if( ImGui::BeginMenuBar() )
			{
				if( ImGui::BeginMenu( "File" ) )
				{
					if( ImGui::MenuItem( "New" ) )
					{
						g_fileName = "";
						delete g_PSys;
						PSCreate();
					}
					if( ImGui::MenuItem( "Open" ) ) needOpen = true;
					if( ImGui::MenuItem( "Save" ) ) needSave = true;
					if( ImGui::MenuItem( "Save As" ) ) needSaveAs = true;
					ImGui::Separator();
					if( ImGui::MenuItem( "Exit" ) ){ Game_End(); }
					ImGui::EndMenu();
				}
				ImGui::SameLine( 0, 50 );
				ImGui::Text( "Particle system file: %s", g_fileName.size() ? StackPath(g_fileName).str : "<none>" );
				
				ImGui::SameLine( 0, 50 );
				ImGui::Text( "Edit mode:" );
				ImGui::SameLine();
				ImGui::RadioButton( "Particle system", &g_mode, EditPartSys );
				ImGui::SameLine();
				ImGui::RadioButton( "Misc. settings", &g_mode, MiscProps );
				
				ImGui::SameLine( 0, 50 );
				if( ImGui::Button( "Play" ) )
					g_PSys->Play();
				ImGui::SameLine();
				if( ImGui::Button( "Stop" ) )
					g_PSys->Stop();
				ImGui::SameLine();
				if( ImGui::Button( "Trigger" ) )
					g_PSys->Trigger();
				
				ImGui::EndMenuBar();
			}
			
			IMGUI_HSPLIT( 0.7f,
			{
				g_NUIRenderView->Process( dt );
			},
			{
				if( g_mode == EditPartSys )
				{
					EditParticleSystem();
				}
				else if( g_mode == MiscProps )
				{
					g_NUIRenderView->EditCameraParams();
				}
			});
			
			//
			// OPEN
			//
			String fn;
#define OPEN_CAPTION "Open particle system (.psy) file"
			if( needOpen )
				g_NUIPartSysPicker->OpenPopup( OPEN_CAPTION );
			if( g_NUIPartSysPicker->Popup( OPEN_CAPTION, fn, false ) )
			{
				if( g_PSys->Load( fn ) )
				{
					g_fileName = fn;
					g_PSys->Play();
				}
				else
				{
					IMGUIError( "Cannot open file: %s", StackPath(fn).str );
				}
			}
			
			//
			// SAVE
			//
			fn = g_fileName;
#define SAVE_CAPTION "Save particle system (.psy) file"
			if( needSaveAs || ( needSave && g_fileName.size() == 0 ) )
				g_NUIPartSysPicker->OpenPopup( SAVE_CAPTION );
			
			bool canSave = needSave && g_fileName.size();
			if( g_NUIPartSysPicker->Popup( SAVE_CAPTION, fn, true ) )
				canSave = fn.size();
			if( canSave )
			{
				if( g_PSys->Save( fn ) )
				{
					g_fileName = fn;
				}
				else
				{
					IMGUIError( "Cannot save file: %s", StackPath(fn).str );
				}
			}
		}
		IMGUI_MAIN_WINDOW_END;
		
		SGRX_IMGUI_Render();
		SGRX_IMGUI_ClearEvents();
	}
};


extern "C" EXPORT IGame* CreateGame()
{
	return new PSEditor;
}

