

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
		puts(StackPath( fontpath ));
		io.Fonts->AddFontFromFileTTF( StackPath( fontpath ), 16.0f );
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


