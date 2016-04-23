

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
		io.KeyShift = ((e.key.keysym.mod & KMOD_SHIFT) != 0);
		io.KeyCtrl = ((e.key.keysym.mod & KMOD_CTRL) != 0);
		io.KeyAlt = ((e.key.keysym.mod & KMOD_ALT) != 0);
		io.KeySuper = ((e.key.keysym.mod & KMOD_GUI) != 0);
	}
}

void SGRX_IMGUI_NewFrame()
{
	if( !g_FontTexture )
	{
		ImGuiIO& io = ImGui::GetIO();
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
	
	ImGui::NewFrame();
}

void SGRX_IMGUI_Render()
{
	ImGui::Render();
}


bool IMGUIEditFloat( const char* label, float& v, float vmin, float vmax, int prec )
{
	return ImGui::DragFloat( label, &v, pow( 0.1f, prec ), vmin, vmax, "%g", 2 );
}

bool IMGUIEditVec3( const char* label, Vec3& v, float vmin, float vmax, int prec )
{
	return ImGui::DragFloat3( label, &v.x, pow( 0.1f, prec ), vmin, vmax, "%g", 2 );
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

bool IMGUIEditString( const char* label, String& str, int maxsize )
{
	str.reserve( maxsize );
	bool ret = ImGui::InputText( label, str.data(), str.capacity() );
	str.resize( sgrx_snlen( str.data(), str.capacity() ) );
	return ret;
}



inline void lmm_prepmeshinst( MeshInstHandle mih )
{
	mih->SetLightingMode( SGRX_LM_Dynamic );
	for( int i = 10; i < 16; ++i )
		mih->constants[ i ] = V4(0.15f);
}


bool IMGUIMeshPickerCore::Use( const char* label, String& str )
{
	if( ImGui::Button( str.size() ? StackPath(str).str : "<click to select>",
		ImVec2( ImGui::GetContentRegionAvail().x * (2.f/3.f), 20 ) ) )
	{
		ImGui::OpenPopup( m_caption );
	}
	ImGui::SameLine();
	ImGui::Text( "%s", label );
	
	bool opened = true;
	bool changed = false;
	ImGui::SetNextWindowPos( ImVec2( 20, 20 ), ImGuiSetCond_Appearing );
	ImGui::SetNextWindowSize( ImGui::GetIO().DisplaySize - ImVec2( 40, 40 ), ImGuiSetCond_Appearing );
	if( ImGui::BeginPopupModal( m_caption, &opened, 0 ) )
	{
		String prev = m_searchString;
		IMGUIEditString( "Search query", m_searchString, 256 );
		if( prev != m_searchString )
			_Search( m_searchString );
		
		ImGui::Separator();
		
		ImDrawList* idl = ImGui::GetWindowDrawList();
		int width = 128;
		int height = 128;
		ImVec2 btnSize( width, height );
		int ncols = ImGui::GetContentRegionAvail().x / width;
		for( size_t i = 0; i < m_filtered.size(); ++i )
		{
			int x = i % ncols;
		//	int y = i / ncols;
			
			if( x )
				ImGui::SameLine();
			ImVec2 cp = ImGui::GetCursorPos()
				- ImVec2( ImGui::GetScrollX(), ImGui::GetScrollY() )
				+ ImGui::GetWindowPos();
			
			const Entry& E = m_entries[ m_filtered[ i ] ];
			ImGui::PushID( E.path.c_str() );
			if( ImGui::Button( "##btn", btnSize ) )
			{
				str = E.path;
				opened = false;
				changed = true;
			}
			ImGui::PopID();
			
			_StaticDrawItemData data =
			{
				this, m_filtered[ i ], cp.x, cp.y, cp.x + width, cp.y + height,
			};
			idl->AddCallback( IMGUIMeshPickerCore::_StaticDrawItem, new _StaticDrawItemData(data) );
		}
		
		if( !opened )
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	
	return changed;
}

void IMGUIMeshPickerCore::_Search( StringView text )
{
	m_filtered.clear();
	if( !text )
	{
		for( size_t i = 0; i < m_entries.size(); ++i )
			m_filtered.push_back( i );
	}
	else if( m_looseSearch )
	{
		for( size_t i = 0; i < m_entries.size(); ++i )
		{
			if( m_entries[ i ].path.view().match_loose( text ) )
				m_filtered.push_back( i );
		}
	}
	else
	{
		for( size_t i = 0; i < m_entries.size(); ++i )
		{
			if( m_entries[ i ].path.view().find_first_at( text ) != NOT_FOUND )
				m_filtered.push_back( i );
		}
	}
}

IMGUIMeshPickerCore::IMGUIMeshPickerCore() :
	m_caption( "Pick mesh" ),
	m_customCamera( false ),
	m_looseSearch( true ),
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

void IMGUIMeshPickerCore::_StaticDrawItem( const ImDrawList* parent_list, const ImDrawCmd* cmd )
{
	SGRX_CAST( _StaticDrawItemData*, data, cmd->UserCallbackData );
	data->self->_DrawItem( data->i, data->x0, data->y0, data->x1, data->y1 );
	delete data;
	const_cast<void*&>(cmd->UserCallbackData) = NULL;
}


IMGUIMeshPicker::IMGUIMeshPicker()
{
	Reload();
}

void IMGUIMeshPicker::Reload()
{
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


