

#pragma once
#define __STDC_FORMAT_MACROS 1
#include <float.h>
#include "engine.hpp"


#define NK_MEMSET memset
#define NK_MEMCOPY memcpy
#define NK_SQRT sqrt
#define NK_SIN sin
#define NK_COS cos
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_API ENGINE_EXPORT
#include "../../ext/src/nuklear.h"



typedef struct nk_context SGRX_nk_context;

ENGINE_EXPORT void SGRX_nk_init( SGRX_nk_context* ctx );
ENGINE_EXPORT void SGRX_nk_free( SGRX_nk_context* ctx );
ENGINE_EXPORT void SGRX_nk_event( SGRX_nk_context* ctx, const Event& e );
ENGINE_EXPORT void SGRX_nk_render( SGRX_nk_context* ctx );


