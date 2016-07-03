

#include "imgui.hpp"



static TextureHandle g_FontTexture;


static void sgrx_imgui_impl_RenderDrawLists( ImDrawData* draw_data )
{
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = int( io.DisplaySize.x * io.DisplayFramebufferScale.x );
	int fb_height = int( io.DisplaySize.y * io.DisplayFramebufferScale.y );
	if( fb_width == 0 || fb_height == 0 )
		return;
	draw_data->ScaleClipRects( io.DisplayFramebufferScale );
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	for( int n = 0; n < draw_data->CmdListsCount; ++n )
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[ n ];
		const ImDrawVert* vtx_buffer = &cmd_list->VtxBuffer.front();
		const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
		
		for( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); ++cmd_i )
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[ cmd_i ];
			if( pcmd->UserCallback )
			{
				pcmd->UserCallback( cmd_list, pcmd );
			}
			else
			{
				GR2D_SetScissorRect( pcmd->ClipRect.x, pcmd->ClipRect.y, pcmd->ClipRect.z, pcmd->ClipRect.w );
				br.Reset();
				br.RenderState.scissorEnable = true;
				br.SetPrimitiveType( PT_Triangles );
				br.SetTexture( (SGRX_ITexture*) pcmd->TextureId );
				for( unsigned idx = 0; idx < pcmd->ElemCount; ++idx )
				{
					const ImDrawVert& v = vtx_buffer[ idx_buffer[ idx ] ];
					br.Colu( v.col );
					br.Tex( v.uv.x, v.uv.y );
					br.Pos( v.pos.x, v.pos.y );
				}
			}
			idx_buffer += pcmd->ElemCount;
		}
	}
	br.Reset();
	GR2D_UnsetScissorRect();
}

static const char* sgrx_imgui_impl_GetClipboardText()
{
	static String clipboard;
	if( !Window_GetClipboardText( clipboard ) )
		clipboard.clear();
	clipboard.push_back( '\0' );
	return clipboard.data();
}

static void sgrx_imgui_impl_SetClipboardText(const char* text)
{
	Window_SetClipboardText( text );
}


void SGRX_IMGUI_Init()
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ ImGuiKey_Tab ] = SDLK_TAB;
	io.KeyMap[ ImGuiKey_LeftArrow ] = SDL_SCANCODE_LEFT;
	io.KeyMap[ ImGuiKey_RightArrow ] = SDL_SCANCODE_RIGHT;
	io.KeyMap[ ImGuiKey_UpArrow ] = SDL_SCANCODE_UP;
	io.KeyMap[ ImGuiKey_DownArrow ] = SDL_SCANCODE_DOWN;
	io.KeyMap[ ImGuiKey_PageUp ] = SDL_SCANCODE_PAGEUP;
	io.KeyMap[ ImGuiKey_PageDown ] = SDL_SCANCODE_PAGEDOWN;
	io.KeyMap[ ImGuiKey_Home ] = SDL_SCANCODE_HOME;
	io.KeyMap[ ImGuiKey_End ] = SDL_SCANCODE_END;
	io.KeyMap[ ImGuiKey_Delete ] = SDLK_DELETE;
	io.KeyMap[ ImGuiKey_Backspace ] = SDLK_BACKSPACE;
	io.KeyMap[ ImGuiKey_Enter ] = SDLK_RETURN;
	io.KeyMap[ ImGuiKey_Escape ] = SDLK_ESCAPE;
	io.KeyMap[ ImGuiKey_A ] = SDLK_a;
	io.KeyMap[ ImGuiKey_C ] = SDLK_c;
	io.KeyMap[ ImGuiKey_V ] = SDLK_v;
	io.KeyMap[ ImGuiKey_X ] = SDLK_x;
	io.KeyMap[ ImGuiKey_Y ] = SDLK_y;
	io.KeyMap[ ImGuiKey_Z ] = SDLK_z;
	
	io.DisplaySize.x = GR_GetWidth();
	io.DisplaySize.y = GR_GetHeight();
	
	io.IniFilename = NULL;
	io.RenderDrawListsFn = sgrx_imgui_impl_RenderDrawLists;
	io.SetClipboardTextFn = sgrx_imgui_impl_SetClipboardText;
	io.GetClipboardTextFn = sgrx_imgui_impl_GetClipboardText;
	
	String fontpath;
	if( FS_FindRealPath( "fonts/lato-regular.ttf", fontpath ) )
	{
		io.Fonts->AddFontFromFileTTF( StackPath( fontpath ), 14.0f );
	}
}

void SGRX_IMGUI_Free()
{
	ImGui::GetIO().Fonts->TexID = 0;
	g_FontTexture = NULL;
}

void SGRX_IMGUI_ClearEvents()
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel = 0;
}

void SGRX_IMGUI_Event( const Event& e )
{
	ImGuiIO& io = ImGui::GetIO();
	if( e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP )
	{
		bool down = e.type == SDL_MOUSEBUTTONDOWN;
		if( e.button.button == SGRX_MB_LEFT )
			io.MouseDown[0] = down;
		if( e.button.button == SGRX_MB_RIGHT )
			io.MouseDown[1] = down;
		if( e.button.button == SGRX_MB_MIDDLE )
			io.MouseDown[2] = down;
	}
	else if( e.type == SDL_MOUSEWHEEL )
	{
		io.MouseWheel += e.wheel.y;
	}
	else if( e.type == SDL_MOUSEMOTION )
	{
		io.MousePos = ImVec2( e.motion.x, e.motion.y );
	}
	else if( e.type == SDL_TEXTINPUT )
	{
		io.AddInputCharactersUTF8( e.text.text );
	}
	else if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP )
	{
		int key = e.key.keysym.sym & ~SDLK_SCANCODE_MASK;
		io.KeysDown[ key ] = (e.type == SDL_KEYDOWN);
	}
}

void SGRX_IMGUI_NewFrame( float dt )
{
	ImGuiIO& io = ImGui::GetIO();
	
	if( !g_FontTexture )
	{
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsAlpha8( &pixels, &width, &height );
		
		Array< uint32_t > cpixels;
		cpixels.resize( width * height );
		for( int i = 0; i < width * height; ++i )
			cpixels[ i ] = COLOR_RGBA( 255, 255, 255, pixels[ i ] );
		g_FontTexture = GR_CreateTexture( width, height, TEXFORMAT_RGBA8, TEXFLAGS_LERP, 1, cpixels.data() );
		io.Fonts->TexID = g_FontTexture.item;
	}
	
	io.DeltaTime = dt;
	io.KeyShift = io.KeysDown[ SDL_SCANCODE_LSHIFT ] || io.KeysDown[ SDL_SCANCODE_RSHIFT ];
	io.KeyCtrl = io.KeysDown[ SDL_SCANCODE_LCTRL ] || io.KeysDown[ SDL_SCANCODE_RCTRL ];
	io.KeyAlt = io.KeysDown[ SDL_SCANCODE_LALT ] || io.KeysDown[ SDL_SCANCODE_RALT ];
	io.KeySuper = io.KeysDown[ SDL_SCANCODE_LGUI ] || io.KeysDown[ SDL_SCANCODE_RGUI ];
	
	ImGui::NewFrame();
}

void _IMGUI_DrawError();
void SGRX_IMGUI_Render()
{
	_IMGUI_DrawError();
	ImGui::Render();
}


bool IMGUIEditBool( const char* label, bool& v )
{
	return ImGui::Checkbox( label, &v );
}

bool IMGUIEditInt_( const char* label, int& v, int vmin, int vmax )
{
	return ImGui::DragInt( label, &v, 0.2f, vmin, vmax );
}

bool IMGUIEditMask_( const char* label, uint64_t& flags, int count )
{
	if( count < 0 || count > 64 ) count = 64;
	static const char* labels[ 64 ] =
	{
		"##00", "##01", "##02", "##03", "##04", "##05", "##06", "##07",
		"##10", "##11", "##12", "##13", "##14", "##15", "##16", "##17",
		"##20", "##21", "##22", "##23", "##24", "##25", "##26", "##27",
		"##30", "##31", "##32", "##33", "##34", "##35", "##36", "##37",
		"##40", "##41", "##42", "##43", "##44", "##45", "##46", "##47",
		"##50", "##51", "##52", "##53", "##54", "##55", "##56", "##57",
		"##60", "##61", "##62", "##63", "##64", "##65", "##66", "##67",
		"##70", "##71", "##72", "##73", "##74", "##75", "##76", "##77",
	};
	
	ImGui::BeginGroup();
	
	bool ret = false;
	for( int i = 0; i < count; ++i )
	{
		if( i % 8 != 0 )
			ImGui::SameLine();
		
		uint64_t flag = 1ULL << i;
		bool v = ( flags & flag ) == flag;
		bool pressed = ImGui::Checkbox( labels[ i ], &v );
		if( pressed )
		{
			ret = true;
			if( v )
				flags |= flag;
			else
				flags &= ~flag;
		}
	}
	
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::Text( "%s", label );
	
	return ret;
}

bool IMGUIEditFloat( const char* label, float& v, float vmin, float vmax, int prec )
{
	return ImGui::DragFloat( label, &v, pow( 0.1f, prec ), vmin, vmax, "%g", 2 );
}

bool IMGUIEditVec2( const char* label, Vec2& v, float vmin, float vmax, int prec )
{
	return ImGui::DragFloat2( label, &v.x, pow( 0.1f, prec ), vmin, vmax, "%g", 2 );
}

bool IMGUIEditVec3( const char* label, Vec3& v, float vmin, float vmax, int prec )
{
	return ImGui::DragFloat3( label, &v.x, pow( 0.1f, prec ), vmin, vmax, "%g", 2 );
}

bool IMGUIEditVec4( const char* label, Vec4& v, float vmin, float vmax, int prec )
{
	return ImGui::DragFloat4( label, &v.x, pow( 0.1f, prec ), vmin, vmax, "%g", 2 );
}

bool IMGUIEditFloatSlider( const char* label, float& v, float vmin, float vmax, float power )
{
	return ImGui::SliderFloat( label, &v, vmin, vmax, "%g", power );
}

bool IMGUIEditQuat( const char* label, Quat& v )
{
	static int quatEditMode = 0;
	const char* qemNames[] = { "Euler angles", "Quaternion" };
	const char* qemShortNames[] = { "EA", "Q" };
	
	ImGui::PushID( label );
	if( ImGui::Button( qemShortNames[ quatEditMode ] ) )
		ImGui::OpenPopup( "selQEM" );
	
	if( ImGui::BeginPopup( "selQEM" ) )
	{
		ImGui::Text( "Quaternion editing mode" );
		ImGui::Separator();
		for( unsigned i = 0; i < SGRX_ARRAY_SIZE( qemNames ); ++i )
			if( ImGui::Selectable( qemNames[ i ] ) )
				quatEditMode = i;
		ImGui::EndPopup();
	}
	ImGui::PopID();
	
	ImGui::SameLine();
	
	if( quatEditMode == 0 )
	{
		Vec3 angles = RAD2DEG( v.ToXYZ() );
		if( ImGui::DragFloat3( label, &angles.x, 0.01f, -360, 360, "%g", 2 ) )
		{
			v = Quat::CreateFromXYZ( DEG2RAD( angles ) );
			return true;
		}
		return false;
	}
	else
	{
		return ImGui::DragFloat4( label, &v.x, 0.01f, -100, 100, "%g", 2 );
	}
}

bool IMGUIEditXFMat4( const char* label, Mat4& v )
{
	Vec3 p = v.GetTranslation();
	Quat r = v.GetRotationQuaternion();
	Vec3 s = v.GetScale();
	
	bool chg = false;
	
	IMGUI_GROUPCTL_BEGIN( label );
	
	chg |= IMGUIEditVec3( "Position", p, -8192, 8192 );
	chg |= IMGUIEditQuat( "Rotation", r );
	chg |= IMGUIEditVec3( "Scale", s, -8192, 8192 );
	
	IMGUI_GROUPCTL_END;
	
	if( chg )
	{
		v = Mat4::CreateSRT( s, r, p );
	}
	return chg;
}

bool IMGUIEditColorHSVHDR( const char* label, Vec3& v, float maxval )
{
	bool ret = false;
	
	IMGUI_GROUPCTL_BEGIN( label );
	
	ImGui::ColorButton( ImColor::HSV( v.x, v.y, 0.5f ) );
	ImGui::SameLine();
	ret |= ImGui::DragFloat2( "Hue/sat.", &v.x, 0.01f, 0, 1, "%g", 1 );
	
	ImGui::ColorButton( ImColor::HSV( v.x, v.y, v.z ) );
	ImGui::SameLine();
	ret |= ImGui::DragFloat( "Value", &v.z, 0.01f, 0, maxval, "%g", 2 );
	
	IMGUI_GROUPCTL_END;
	
	return ret;
}

bool IMGUIEditColorRGBLDR( const char* label, Vec3& v )
{
	return ImGui::ColorEdit3( label, &v.x );
}

bool IMGUIEditColorRGBA32( const char* label, uint32_t& c )
{
	int v[4] =
	{
		COLOR_EXTRACT_R( c ),
		COLOR_EXTRACT_G( c ),
		COLOR_EXTRACT_B( c ),
		COLOR_EXTRACT_A( c ),
	};
	bool ret = ImGui::DragInt4( label, v );
	c = COLOR_RGBA( v[0], v[1], v[2], v[3] );
	return ret;
}

bool IMGUIEditString( const char* label, String& str, int maxsize )
{
	str.resize_using( maxsize, '\0' );
	bool ret = ImGui::InputText( label, str.data(), str.capacity() );
	str.resize( sgrx_snlen( str.data(), str.capacity() ) );
	return ret;
}

bool IMGUIEditString( const char* label, RCString& str, int maxsize )
{
	String s = str;
	bool ret = IMGUIEditString( label, s, maxsize );
	str = s;
	return ret;
}



char g_errorStr[ 4096 ];
bool g_errorOpen = false;

void _IMGUI_DrawError()
{
	if( g_errorOpen )
	{
		ImGui::OpenPopup( "Error" );
		g_errorOpen = false;
	}
	if( ImGui::BeginPopupModal( "Error", NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		ImGui::Text( g_errorStr );
		if( ImGui::Button( "OK" ) )
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void IMGUIErrorStr( StringView str )
{
	sgrx_sncopy( g_errorStr, 4096, str.data(), str.size() );
	g_errorOpen = true;
}

void IMGUIError( const char* str, ... )
{
	va_list args;
	va_start( args, str );
	sgrx_vsnprintf( g_errorStr, 4096, str, args );
	va_end( args );
	g_errorOpen = true;
}

void IMGUIYesNo( bool v )
{
	ImGui::TextColored( v ? ImColor( 0.1f, 1.f, 0.f ) : ImColor( 1.f, 0.1f, 0.f ), v ? "YES" : "NO" );
}


IMGUIRenderView::IMGUIRenderView( SGRX_Scene* scene ) :
	mouseOn( false ),
	crpos( V3(0) ),
	crdir( V3(0) ),
	cursor_aim( false ),
	cursor_hpos( V2(0) ),
	crplaneheight( 0 ),
	m_scene( scene )
{
	Vec3 dir = m_scene->camera.direction;
	Vec2 dir2 = V2( dir.x, dir.y ).Normalized();
	hangle = atan2( dir2.y, dir2.x );
	vangle = asin( m_scene->camera.direction.z );
}

bool IMGUIRenderView::CanAcceptKeyboardInput()
{
	ImGuiIO& io = ImGui::GetIO();
	return !( io.KeyCtrl || io.KeyAlt ) && ImGui::IsWindowFocused();
}

void IMGUIRenderView::Process( float deltaTime, bool editable )
{
	mouseOn = ImGui::IsMouseHoveringWindow();
	if( mouseOn && ImGui::IsMouseDown( 1 ) )
		ImGui::SetWindowFocus();
	bool caki = CanAcceptKeyboardInput() && editable;
	bool movefwd = caki && ImGui::IsKeyDown( SDLK_w );
	bool movebwd = caki && ImGui::IsKeyDown( SDLK_s );
	bool movelft = caki && ImGui::IsKeyDown( SDLK_a );
	bool movergt = caki && ImGui::IsKeyDown( SDLK_d );
	bool movefast = caki && ImGui::IsKeyDown( SDL_SCANCODE_LSHIFT );
	bool moveup = caki && ImGui::IsKeyDown( SDLK_q );
	bool movedn = caki && ImGui::IsKeyDown( SDLK_z );
	
	cursor_aim = false;
	ImVec2 gcp = ImGui::GetMousePos() - ImGui::GetWindowPos();
	ImVec2 gwsz = ImGui::GetWindowSize();
	Vec2 cp = { safe_fdiv( gcp.x, gwsz.x ), safe_fdiv( gcp.y, gwsz.y ) };
	if( m_scene->camera.GetCursorRay( cp.x, cp.y, crpos, crdir ) )
	{
		float dsts[2];
		if( RayPlaneIntersect( crpos, crdir, V4(0,0,1,crplaneheight), dsts ) && dsts[0] > 0 )
		{
			Vec3 isp = crpos + crdir * dsts[0];
			cursor_hpos = V2( isp.x, isp.y );
			cursor_aim = true;
		}
	}
	
	// update camera
	float speed = 1;
	if( movefast )
		speed *= 10;
	speed *= deltaTime;
	
	if( ImGui::IsMouseDragging( 1, 0 ) )
	{
		ImVec2 rdrag = caki ? ImGui::GetMouseDragDelta( 1, 0 ) : ImVec2(0,0);
		hangle -= rdrag.x * 0.01f;
		vangle -= rdrag.y * 0.01f;
		vangle = clamp( vangle, (float) -M_PI * 0.49f, (float) M_PI * 0.49f );
		ImGui::ResetMouseDragDelta( 1 );
	}
	
	float c_hangle = cos( hangle ), s_hangle = sin( hangle ), c_vangle = cos( vangle ), s_vangle = sin( vangle );
	Vec3 dir = { c_hangle * c_vangle, s_hangle * c_vangle, s_vangle };
	Vec3 up = m_scene->camera.updir;
	Vec3 rgt = Vec3Cross( dir, up ).Normalized();
	m_scene->camera.direction = dir;
	m_scene->camera.position += ( dir * ( movefwd - movebwd ) + rgt * ( movergt - movelft ) + up * ( moveup - movedn ) ) * speed;
	m_scene->camera.aspect = safe_fdiv( gwsz.x, gwsz.y );
	
	m_scene->camera.UpdateMatrices();
	
	ImVec2 gwp = ImGui::GetWindowPos();
	m_vp.x0 = gwp.x + 1;
	m_vp.y0 = gwp.y + 1;
	m_vp.x1 = gwp.x + gwsz.x - 1;
	m_vp.y1 = gwp.y + gwsz.y - 1;
	ImGui::GetWindowDrawList()->AddCallback( IMGUIRenderView::_StaticDraw, this );
}

void IMGUIRenderView::_StaticDraw( const ImDrawList* parent_list, const ImDrawCmd* cmd )
{
	SGRX_CAST( IMGUIRenderView*, rv, cmd->UserCallbackData );
	SGRX_RenderScene rsinfo( V4( GetTimeMsec() / 1000.0f ), rv->m_scene );
	rsinfo.viewport = &rv->m_vp;
	rsinfo.debugdraw = rv;
	GR_RenderScene( rsinfo );
}

void IMGUIRenderView::DebugDraw()
{
}

void IMGUIRenderView::EditCameraParams()
{
	IMGUI_GROUP( "Camera", true,
	{
		SGRX_Camera& CAM = m_scene->camera;
		
		IMGUIEditVec3( "Position", CAM.position, -10000, 10000 );
		if( IMGUIEditVec3( "Direction", CAM.direction, -100, 100 ) )
			CAM.direction.Normalize();
		if( IMGUIEditVec3( "Up direction", CAM.updir, -100, 100 ) )
			CAM.updir.Normalize();
		IMGUIEditFloat( "Angle", CAM.angle, 1, 179 );
		// no aspect ratio editing - it's automatically calculated
		IMGUIEditFloat( "Aspect/angle mix H-V", CAM.aamix, 0, 1 );
		IMGUIEditFloat( "Z near", CAM.znear, 0.0001f, 100000 );
		IMGUIEditFloat( "Z far", CAM.zfar, 0.0001f, 100000 );
	});
}


IMGUIPickerCore::IMGUIPickerCore() :
	m_layoutType( Layout_Grid ),
	m_itemSize( ImVec2( 256, 32 ) ),
	m_looseSearch( true )
{
}

IMGUIPickerCore::~IMGUIPickerCore()
{
}

void IMGUIPickerCore::OpenPopup( const char* caption )
{
	Reload();
	ImGui::OpenPopup( caption );
}

bool IMGUIPickerCore::Popup( const char* caption, String& str )
{
	bool opened = true;
	bool changed = false;
	ImGui::SetNextWindowPos( ImVec2( 20, 20 ), ImGuiSetCond_Appearing );
	ImGui::SetNextWindowSize( ImGui::GetIO().DisplaySize - ImVec2( 40, 40 ), ImGuiSetCond_Appearing );
	if( ImGui::BeginPopupModal( caption, &opened, 0 ) )
	{
		if( SearchUI( str ) )
		{
			opened = false;
			changed = true;
		}
		
	//	ImGui::Separator();
		ImGui::BeginChild( "subframe", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding );
		
		if( m_layoutType == Layout_Grid )
		{
			int ncols = ImGui::GetContentRegionAvail().x / m_itemSize.x;
			for( size_t i = 0; i < m_filtered.size(); ++i )
			{
				int x = i % ncols;
			//	int y = i / ncols;
				
				if( x )
					ImGui::SameLine();
				
				RCString path = GetEntryPath( m_filtered[ i ] );
				ImGui::PushID( path.c_str() );
				if( EntryUI( m_filtered[ i ], str ) )
				{
					str = path;
					opened = false;
					changed = true;
				}
				ImGui::PopID();
			}
		}
		else if( m_layoutType == Layout_List )
		{
			for( size_t i = 0; i < m_filtered.size(); ++i )
			{
				RCString path = GetEntryPath( m_filtered[ i ] );
				ImGui::PushID( path.c_str() );
				if( EntryUI( m_filtered[ i ], str ) )
				{
					str = path;
					opened = false;
					changed = true;
				}
				ImGui::PopID();
			}
		}
		
		ImGui::EndChild();
		
		if( ImGui::IsKeyPressed( SDLK_ESCAPE ) )
			opened = false;
		if( !opened )
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	
	if( changed )
		ImGui::TriggerChangeCheck();
	return changed;
}

bool IMGUIPickerCore::Popup( const char* caption, RCString& str )
{
	String s = str;
	bool ret = Popup( caption, s );
	str = s;
	return ret;
}

bool IMGUIPickerCore::Property( const char* caption, const char* label, String& str )
{
	if( ImGui::Button( str.size() ? StackPath(str).str : "<click to select>",
		ImVec2( ImGui::GetContentRegionAvail().x * (2.f/3.f), 20 ) ) )
	{
		OpenPopup( caption );
	}
	ImGui::SameLine();
	ImGui::Text( "%s", label );
	return Popup( caption, str );
}

void IMGUIPickerCore::Reload()
{
}

bool IMGUIPickerCore::SearchUI( String& str )
{
	String prev = m_searchString;
	IMGUIEditString( "Search query", m_searchString, 256 );
	if( prev != m_searchString )
		_Search( m_searchString );
	return false;
}

void IMGUIPickerCore::_Search( StringView text )
{
	size_t count = GetEntryCount();
	m_filtered.clear();
	if( !text )
	{
		for( size_t i = 0; i < count; ++i )
			m_filtered.push_back( i );
	}
	else if( m_looseSearch )
	{
		for( size_t i = 0; i < count; ++i )
		{
			StringView e = GetEntryPath( i ).view();
			if( !e || e.match_loose( text ) )
				m_filtered.push_back( i );
		}
	}
	else
	{
		for( size_t i = 0; i < count; ++i )
		{
			StringView e = GetEntryPath( i ).view();
			if( !e || e.find_first_at( text ) != NOT_FOUND )
				m_filtered.push_back( i );
		}
	}
}

bool IMGUIPickerCore::EntryUI( size_t i, String& str )
{
	if( m_layoutType == Layout_Grid )
	{
		return ImGui::Button( GetEntryPath( i ).c_str(), m_itemSize );
	}
	else if( m_layoutType == Layout_List )
	{
		return ImGui::Selectable( GetEntryPath( i ).c_str() );
	}
	return false;
}


void IMGUISoundPicker::Reload()
{
	LOG_FUNCTION_ARG("IMGUISoundPicker");
	
	LOG << "Enumerating sound events";
	sys->EnumerateSoundEvents( m_entries );
	LOG << "... " << m_entries.size() << " events found";
	_Search( m_searchString );
}


IMGUIFilePicker::IMGUIFilePicker( const char* dir, const char* ext, bool confirm )
{
	m_saveMode = false;
	m_confirm = confirm;
	m_directory = dir;
	m_extension = ext;
	Reload();
}

bool IMGUIFilePicker::Popup( const char* caption, String& str, bool save )
{
	m_saveMode = save;
	return IMGUIPickerCore::Popup( caption, str );
}

void IMGUIFilePicker::Reload()
{
	LOG_FUNCTION_ARG("IMGUIFilePicker");
	
	LOG << "Reloading files, dir=" << m_directory << ", ext=" << m_extension;
	m_entries.clear();
	FS_IterateDirectory( m_directory, this );
	_Search( m_searchString );
}

bool IMGUIFilePicker::SearchUI( String& str )
{
	IMGUIPickerCore::SearchUI( str );
	if( m_saveMode )
	{
		char bfr[ 256 ];
		sgrx_snprintf( bfr, 256, "%s/%s%s", m_directory, StackPath(m_searchString).str, m_extension );
		
		ImGui::SameLine();
		if( ImGui::Button( "Save" ) )
		{
			if( !m_searchString.size() )
			{
				ImGui::OpenPopup( "ENONAME" );
			}
			else
			{
				if( !FS_FileExists( bfr ) )
				{
					str = bfr;
					return true;
				}
				ImGui::OpenPopup( "Overwrite file" );
			}
		}
		if( ImGui::BeginPopup( "ENONAME" ) )
		{
			ImGui::Text( "Please specify a name!" );
			if( ImGui::Button( "OK" ) )
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		
		if( ConfirmPopup( "Overwrite file", "Do you really want to overwrite this file?", bfr ) )
		{
			str = bfr;
			return true;
		}
	}
	return false;
}

bool IMGUIFilePicker::HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
{
	if( name == "." || name == ".." )
		return true;
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s/%s", StackString<256>(loc).str, StackString<256>(name).str );
	LOG << "[F]: " << bfr;
	StringView fullname = bfr;
	if( isdir )
	{
		FS_IterateDirectory( fullname, this );
	}
	else if( name.ends_with( m_extension ) )
	{
		m_entries.push_back( fullname );
	}
	return true;
}

bool IMGUIFilePicker::EntryUI( size_t i, String& str )
{
	bool ret = IMGUIPickerCore::EntryUI( i, str );
	
	if( !m_confirm )
	{
		return ret;
	}
	
	const char* popupName = m_saveMode ? "Overwrite file" : "Open file";
	const char* labelText = m_saveMode ? "Do you really want to overwrite this file?"
		: "Do you really want to open this file?";
	if( ret )
		ImGui::OpenPopup( popupName );
	
	return ConfirmPopup( popupName, labelText, GetEntryPath( i ).c_str() );
}

bool IMGUIFilePicker::ConfirmPopup( const char* caption, const char* label, const char* file )
{
	bool ret = false;
	if( ImGui::BeginPopupModal( caption, NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		ImGui::Text( label );
		
		ImGui::Separator();
		ImGui::Text( "%s", file );
		ImGui::Separator();
		
		if( ImGui::Button( "Yes" ) )
		{
			ret = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine( 0, 20 );
		if( ImGui::Button( "No" ) )
		{
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::EndPopup();
	}
	
	return ret;
}


IMGUIPreviewPickerCore::IMGUIPreviewPickerCore()
{
	m_itemSize = ImVec2( 128, 128 );
}

void IMGUIPreviewPickerCore::_DrawItem( int i, int x0, int y0, int x1, int y1 )
{
}

void IMGUIPreviewPickerCore::_StaticDrawItem( const ImDrawList* parent_list, const ImDrawCmd* cmd )
{
	SGRX_CAST( _StaticDrawItemData*, data, cmd->UserCallbackData );
	data->self->_DrawItem( data->i, data->x0, data->y0, data->x1, data->y1 );
	delete data;
	const_cast<void*&>(cmd->UserCallbackData) = NULL;
}

bool IMGUIPreviewPickerCore::EntryUI( size_t i, String& str )
{
	ImVec2 cp = ImGui::GetCursorPos()
		- ImVec2( ImGui::GetScrollX(), ImGui::GetScrollY() )
		+ ImGui::GetWindowPos();
	
	bool ret = ImGui::Button( "##btn", m_itemSize );
	
	_StaticDrawItemData data =
	{
		this, i, cp.x, cp.y, cp.x + m_itemSize.x, cp.y + m_itemSize.y,
	};
	ImGui::GetWindowDrawList()->AddCallback(
		IMGUIMeshPickerCore::_StaticDrawItem, new _StaticDrawItemData(data) );
	
	return ret;
}


IMGUIMeshPickerCore::IMGUIMeshPickerCore() :
	m_customCamera( false ),
	m_scene( GR_CreateScene() )
{
}

IMGUIMeshPickerCore::~IMGUIMeshPickerCore()
{
	Clear();
}

void IMGUIMeshPickerCore::Clear()
{
	m_entries.clear();
}

void IMGUIMeshPickerCore::AddMesh( StringView path, StringView rsrcpath )
{
	LOG_FUNCTION;
	
	MeshInstHandle mih = m_scene->CreateMeshInstance();
	mih->SetMesh( path );
	mih->enabled = false;
	lmm_prepmeshinst( mih );
	mih->Precache();
	Entry e =
	{
		rsrcpath ? rsrcpath : path,
		mih,
	};
	m_entries.push_back( e );
}

void IMGUIMeshPickerCore::_DrawItem( int i, int x0, int y0, int x1, int y1 )
{
	SGRX_Viewport vp = { x0 + 10, y0 + 4, x1 - 10, y1 - 16 };
	
	if( m_entries[ i ].mesh )
	{
		SGRX_IMesh* M = m_entries[ i ].mesh->GetMesh();
		if( m_customCamera == false )
		{
			Vec3 dst = M->m_boundsMin - M->m_boundsMax;
			Vec3 idst = V3( dst.x ? 1/dst.x : 1, dst.y ? 1/dst.y : 1, dst.z ? 1/dst.z : 1 );
			if( idst.z > 0 )
				idst.z = -idst.z;
			m_scene->camera.direction = idst.Normalized();
			m_scene->camera.position = ( M->m_boundsMax + M->m_boundsMin ) * 0.5f - m_scene->camera.direction * dst.Length() * 0.8f;
			m_scene->camera.znear = 0.1f;
			m_scene->camera.angle = 60;
			m_scene->camera.UpdateMatrices();
		}
		
		SGRX_RenderScene rsinfo( V4( GetTimeMsec() / 1000.0f ), m_scene );
		rsinfo.viewport = &vp;
		m_entries[ i ].mesh->enabled = true;
		GR_RenderScene( rsinfo );
		m_entries[ i ].mesh->enabled = false;
	}
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Col( 0.9f, 1.0f );
	GR2D_DrawTextLine( ( x0 + x1 ) / 2, y1 - 8, m_entries[ i ].path, HALIGN_CENTER, VALIGN_CENTER );
}


IMGUITexturePicker::IMGUITexturePicker()
{
	Reload();
}

void IMGUITexturePicker::Reload()
{
	LOG_FUNCTION_ARG("IMGUITexturePicker");
	
	LOG << "Reloading textures";
	m_textures.clear();
	m_textures.push_back( TextureHandle() );
	FS_IterateDirectory( "textures", this );
	_Search( m_searchString );
}

bool IMGUITexturePicker::HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
{
	if( name == "." || name == ".." )
		return true;
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s/%s", StackString<256>(loc).str, StackString<256>(name).str );
	LOG << "[T]: " << bfr;
	StringView fullname = bfr;
	
	if( isdir )
	{
		FS_IterateDirectory( fullname, this );
	}
	else if( name.ends_with( ".png" ) || name.ends_with( ".stx" ) || name.ends_with( ".dds" ) )
	{
		TextureHandle th = GR_GetTexture( fullname );
		if( th )
			m_textures.push_back( th );
	}
	return true;
}

void IMGUITexturePicker::_DrawItem( int i, int x0, int y0, int x1, int y1 )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	br.Col( 1 );
	br.SetTexture( m_textures[ i ] );
	br.Quad( x0 + 10, y0 + 4, x1 - 10, y1 - 16 );
	
	br.Col( 0.9f, 1.0f );
	StringView name = m_textures[ i ] ? SV(m_textures[ i ]->m_key) : SV("<none>");
	GR2D_DrawTextLine( ( x0 + x1 ) / 2, y1 - 8, name, HALIGN_CENTER, VALIGN_CENTER );
}


IMGUIMeshPicker::IMGUIMeshPicker()
{
	Reload();
}

void IMGUIMeshPicker::Reload()
{
	LOG_FUNCTION_ARG("IMGUIMeshPicker");
	
	LOG << "Reloading meshes";
	Array< MeshInstHandle > oldHandles;
	for( size_t i = 0; i < m_entries.size(); ++i )
		oldHandles.push_back( m_entries[ i ].mesh );
	Clear();
	FS_IterateDirectory( "meshes", this );
	_Search( m_searchString );
}

bool IMGUIMeshPicker::HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
{
	if( name == "." || name == ".." )
		return true;
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s/%s", StackString<256>(loc).str, StackString<256>(name).str );
	LOG << "[M]: " << bfr;
	StringView fullname = bfr;
	if( isdir )
	{
		FS_IterateDirectory( fullname, this );
	}
	else if( name.ends_with( ".ssm" ) )
	{
		AddMesh( fullname );
	}
	return true;
}


inline String ED_GetMeshFromChar( const StringView& charpath )
{
	ByteArray ba;
	if( FS_LoadBinaryFile( charpath, ba ) == false )
		return String();
	
	// basic unserialization to retrieve mesh
	ByteReader br( ba );
	br.marker( "SGRXCHAR" );
	SerializeVersionHelper<ByteReader> arch( br, 2 );
	String mesh;
	arch( mesh );
	return mesh;
}

IMGUICharPicker::IMGUICharPicker()
{
	Reload();
}

void IMGUICharPicker::Reload()
{
	LOG_FUNCTION_ARG("IMGUICharPicker");
	
	LOG << "Reloading chars";
	Array< MeshInstHandle > oldHandles;
	for( size_t i = 0; i < m_entries.size(); ++i )
		oldHandles.push_back( m_entries[ i ].mesh );
	Clear();
	FS_IterateDirectory( SGRXPATH__CHARS, this );
	_Search( m_searchString );
}

bool IMGUICharPicker::HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
{
	if( name == "." || name == ".." )
		return true;
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s/%s", StackString<256>(loc).str, StackString<256>(name).str );
	LOG << "[Ch]: " << bfr;
	StringView fullname = bfr;
	if( isdir )
	{
		FS_IterateDirectory( fullname, this );
	}
	else if( name.ends_with( ".chr" ) )
	{
		String mesh = ED_GetMeshFromChar( fullname );
		if( mesh.size() == 0 )
			return true;
		AddMesh( fullname, mesh );
	}
	return true;
}


bool IMGUIShaderPicker::HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
{
	if( name.starts_with( "mtl_" ) && name.ends_with( ".shd" ) )
	{
		m_shaderList.push_back( name.part( 4, name.size() - 8 ) );
	}
	return true;
}

bool IMGUIShaderPicker::Property( const char* label, String& str )
{
	bool ret = false;
	
	if( ImGui::Button( str.size() ? StackPath(str).str : "<click to select shader>",
		ImVec2( ImGui::GetContentRegionAvailWidth() * 2.f/3.f, 20 ) ) )
	{
		m_shaderList.clear();
		FS_IterateDirectory( SGRXPATH_SRC_SHADERS, this );
		ImGui::OpenPopup( "pick_shader" );
	}
	ImGui::SameLine();
	ImGui::Text( label );
	
	if( ImGui::BeginPopup( "pick_shader" ) )
	{
		for( size_t i = 0; i < m_shaderList.size(); ++i )
		{
			if( ImGui::Selectable( m_shaderList[ i ].c_str() ) )
			{
				str = m_shaderList[ i ];
				ret = true;
			}
		}
		ImGui::EndPopup();
	}
	if( ret )
		ImGui::TriggerChangeCheck();
	return ret;
}


IMGUIAnimPicker::IMGUIAnimPicker()
{
	Reload();
}

void IMGUIAnimPicker::Reload()
{
	LOG_FUNCTION_ARG("IMGUIAnimPicker");
	
	LOG << "Reloading anims";
	m_entries.clear();
	m_entries.push_back( "" );
	FS_IterateDirectory( "meshes", this );
	_Search( m_searchString );
}

bool IMGUIAnimPicker::HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
{
	if( name == "." || name == ".." )
		return true;
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "%s/%s", StackString<256>(loc).str, StackString<256>(name).str );
	LOG << "[An]: " << bfr;
	StringView fullname = bfr;
	if( isdir )
	{
		FS_IterateDirectory( fullname, this );
	}
	else if( name.ends_with( ".anb" ) )
	{
		GR_EnumAnimBundle( fullname, m_entries );
	}
	return true;
}


bool IMGUIEnumPicker::Property( const char* label, int32_t& val )
{
	bool ret = false;
	
	RCString* pval = m_entries.getptr( val );
	ImGui::PushID( label );
	if( ImGui::Button( pval ? pval->c_str() : "<unknown value>",
		ImVec2( ImGui::GetContentRegionAvailWidth() * 2.f/3.f, 20 ) ) )
		ImGui::OpenPopup( "pick_shader" );
	ImGui::SameLine();
	ImGui::Text( label );
	
	if( ImGui::BeginPopup( "pick_shader" ) )
	{
		for( size_t i = 0; i < m_entries.size(); ++i )
		{
			if( ImGui::Selectable( m_entries.item( i ).value.c_str() ) )
			{
				val = m_entries.item( i ).key;
				ret = true;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();
	if( ret )
		ImGui::TriggerChangeCheck();
	return ret;
}


