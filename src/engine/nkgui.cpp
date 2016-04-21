

#define NK_IMPLEMENTATION

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "nkgui.hpp"



float SGRX_nk_get_text_width( nk_handle h, float ht, const char* str, int len )
{
	GR2D_SetFont( (const char*) h.ptr, ht );
	return GR2D_GetTextLength( StringView( str, len ) );
}

void SGRX_nk_init( SGRX_nk_context* ctx )
{
	struct nk_user_font font;
	font.userdata = nk_handle_ptr( (void*) "core" );
	font.height = 12;
	font.width = SGRX_nk_get_text_width;
	
	nk_init_default( ctx, &font );
}

void SGRX_nk_free( SGRX_nk_context* ctx )
{
	nk_free( ctx );
}

void SGRX_nk_event( SGRX_nk_context* ctx, const Event& e )
{
	if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP )
	{
		bool down = e.type == SDL_KEYDOWN;
		int code = e.key.keysym.sym;
		int mod = e.key.keysym.mod;
		if( code == SDLK_LSHIFT || code == SDLK_RSHIFT ) nk_input_key( ctx, NK_KEY_SHIFT, down );
		else if( code == SDLK_RETURN || code == SDLK_KP_ENTER ) nk_input_key( ctx, NK_KEY_ENTER, down );
		else if( code == SDLK_DELETE )    nk_input_key( ctx, NK_KEY_DEL, down );
		else if( code == SDLK_TAB )       nk_input_key( ctx, NK_KEY_TAB, down );
		else if( code == SDLK_LEFT )      nk_input_key( ctx, NK_KEY_LEFT, down );
		else if( code == SDLK_RIGHT )     nk_input_key( ctx, NK_KEY_RIGHT, down );
		else if( code == SDLK_BACKSPACE ) nk_input_key( ctx, NK_KEY_BACKSPACE, down );
		else if( code == SDLK_HOME )      nk_input_key( ctx, NK_KEY_TEXT_START, down );
		else if( code == SDLK_END )       nk_input_key( ctx, NK_KEY_TEXT_END, down );
		else
		{
			if( code == SDLK_c && ( mod & KMOD_CTRL ) )
				nk_input_key( ctx, NK_KEY_COPY, down );
			else if( code == SDLK_v && ( mod & KMOD_CTRL ) )
				nk_input_key( ctx, NK_KEY_PASTE, down );
			else if( code == SDLK_x && ( mod & KMOD_CTRL ) )
				nk_input_key( ctx, NK_KEY_CUT, down );
			else if( code == SDLK_z && ( mod & KMOD_CTRL ) )
				nk_input_key( ctx, NK_KEY_TEXT_UNDO, down );
			else if( code == SDLK_r && ( mod & KMOD_CTRL ) )
				nk_input_key( ctx, NK_KEY_TEXT_REDO, down );
			else if( code == SDLK_LEFT && ( mod & KMOD_CTRL ) )
				nk_input_key( ctx, NK_KEY_TEXT_WORD_LEFT, down );
			else if( code == SDLK_RIGHT && ( mod & KMOD_CTRL ) )
				nk_input_key( ctx, NK_KEY_TEXT_WORD_RIGHT, down );
			else if( code == SDLK_b && ( mod & KMOD_CTRL ) )
				nk_input_key( ctx, NK_KEY_TEXT_LINE_START, down );
			else if( code == SDLK_e && ( mod & KMOD_CTRL ) )
				nk_input_key( ctx, NK_KEY_TEXT_LINE_END, down );
		}
	}
	else if( e.type == SDL_TEXTINPUT )
	{
		nk_input_glyph( ctx, e.text.text );
	}
	else if( e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEBUTTONDOWN )
	{
		bool down = e.type == SDL_MOUSEBUTTONDOWN;
		int x = e.button.x;
		int y = e.button.y;
		if( e.button.button == SGRX_MB_LEFT )
			nk_input_button( ctx, NK_BUTTON_LEFT, x, y, down );
		if( e.button.button == SGRX_MB_RIGHT )
			nk_input_button( ctx, NK_BUTTON_RIGHT, x, y, down );
		if( e.button.button == SGRX_MB_MIDDLE )
			nk_input_button( ctx, NK_BUTTON_MIDDLE, x, y, down );
	}
	else if( e.type == SDL_MOUSEMOTION )
	{
		nk_input_motion( ctx, e.motion.x, e.motion.y );
	}
	else if( e.type == SDL_MOUSEWHEEL )
	{
		nk_input_scroll( ctx, e.wheel.y / 120.0f );
	}
}

void SGRX_nk_render( SGRX_nk_context* ctx )
{
	const struct nk_command* cmd;
	BatchRenderer& br = GR2D_GetBatchRenderer();
	Array< Vec2 > tmpverts;
	nk_foreach( cmd, ctx )
	{
		switch( cmd->type )
		{
		case NK_COMMAND_NOP: break;
		case NK_COMMAND_SCISSOR: {
			SGRX_CAST( const struct nk_command_scissor*, c, cmd );
			GR2D_SetScissorRect( c->x, c->y, c->x + c->w, c->y + c->h );
		} break;
		case NK_COMMAND_LINE: {
			SGRX_CAST( const struct nk_command_line*, c, cmd );
			Vec2 lineverts[2] = { V2( c->begin.x, c->begin.y ), V2( c->end.x, c->end.y ) };
			br.Colu( nk_color_u32( c->color ) );
			br.AAStroke( lineverts, 2, c->line_thickness, false );
		} break;
		case NK_COMMAND_RECT: {
			SGRX_CAST( const struct nk_command_rect*, c, cmd );
			br.Colu( nk_color_u32( c->color ) );
			br.AARectOutline( c->x, c->y, c->x + c->w, c->y + c->h, c->line_thickness );
		} break;
		case NK_COMMAND_RECT_FILLED: {
			SGRX_CAST( const struct nk_command_rect_filled*, c, cmd );
			br.Colu( nk_color_u32( c->color ) );
			br.AARect( c->x, c->y, c->x + c->w, c->y + c->h );
		} break;
		case NK_COMMAND_CIRCLE: {
			SGRX_CAST( const struct nk_command_circle*, c, cmd );
			br.Colu( nk_color_u32( c->color ) );
			br.AAEllipsoidOutline( c->x + c->w * 0.5f, c->y + c->h * 0.5f,
				c->w * 0.5f, c->h * 0.5f, c->line_thickness );
		} break;
		case NK_COMMAND_CIRCLE_FILLED: {
			SGRX_CAST( const struct nk_command_circle_filled*, c, cmd );
			br.Colu( nk_color_u32( c->color ) );
			br.AAEllipsoid( c->x + c->w * 0.5f, c->y + c->h * 0.5f, c->w * 0.5f, c->h * 0.5f );
		} break;
		case NK_COMMAND_ARC: {
			// TODO
		} break;
		case NK_COMMAND_ARC_FILLED: {
			// TODO
		} break;
		case NK_COMMAND_TRIANGLE: {
			SGRX_CAST( const struct nk_command_triangle*, c, cmd );
			Vec2 triverts[3] = { V2( c->a.x, c->a.y ), V2( c->b.x, c->b.y ), V2( c->c.x, c->c.y ) };
			br.Colu( nk_color_u32( c->color ) );
			br.AAStroke( triverts, 3, c->line_thickness, true );
		} break;
		case NK_COMMAND_TRIANGLE_FILLED: {
			SGRX_CAST( const struct nk_command_triangle_filled*, c, cmd );
			Vec2 triverts[3] = { V2( c->a.x, c->a.y ), V2( c->b.x, c->b.y ), V2( c->c.x, c->c.y ) };
			br.Colu( nk_color_u32( c->color ) );
			br.AAPoly( triverts, 3 );
		} break;
		case NK_COMMAND_POLYGON: {
			SGRX_CAST( const struct nk_command_polygon*, c, cmd );
			tmpverts.resize( c->point_count );
			for( unsigned short i = 0; i < c->point_count; ++i )
			{
				tmpverts[ i ] = V2( c->points[ i ].x, c->points[ i ].y );
			}
			br.Colu( nk_color_u32( c->color ) );
			br.AAStroke( tmpverts.data(), tmpverts.size(), c->line_thickness, true );
		} break;
		case NK_COMMAND_POLYGON_FILLED: {
			SGRX_CAST( const struct nk_command_polygon_filled*, c, cmd );
			tmpverts.resize( c->point_count );
			for( unsigned short i = 0; i < c->point_count; ++i )
			{
				tmpverts[ i ] = V2( c->points[ i ].x, c->points[ i ].y );
			}
			br.Colu( nk_color_u32( c->color ) );
			br.AAPoly( tmpverts.data(), tmpverts.size() );
		} break;
		case NK_COMMAND_POLYLINE: {
			SGRX_CAST( const struct nk_command_polyline*, c, cmd );
			tmpverts.resize( c->point_count );
			for( unsigned short i = 0; i < c->point_count; ++i )
			{
				tmpverts[ i ] = V2( c->points[ i ].x, c->points[ i ].y );
			}
			br.Colu( nk_color_u32( c->color ) );
			br.AAStroke( tmpverts.data(), tmpverts.size(), c->line_thickness, false );
		} break;
		case NK_COMMAND_CURVE: {
			// TODO
		} break;
		case NK_COMMAND_RECT_MULTI_COLOR: {
			// 00 = left
			// 10 = top
			// 11 = right
			// 01 = bottom
			// TODO
		} break;
		case NK_COMMAND_IMAGE: {
			// TODO
		} break;
		case NK_COMMAND_TEXT: {
			SGRX_CAST( const struct nk_command_text*, c, cmd );
			if( COLOR_EXTRACT_A( nk_color_u32( c->background ) ) > 0 )
			{
				br.Colu( nk_color_u32( c->background ) );
				br.AARect( c->x, c->y, c->x + c->w, c->y + c->h );
			}
			if( COLOR_EXTRACT_A( nk_color_u32( c->foreground ) ) > 0 )
			{
				br.Colu( nk_color_u32( c->foreground ) );
				GR2D_SetFont( (const char*) c->font->userdata.ptr, c->height );
				GR2D_DrawTextLine( c->x, c->y, StringView( c->string, c->length ) );
			}
		} break;
		}
	}
	nk_clear( ctx );
}


