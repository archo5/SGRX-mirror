

#pragma once
#define __STDC_FORMAT_MACROS 1
#include <float.h>
#include "engine.hpp"


#define IMGUI_API ENGINE_EXPORT
#include "../../ext/src/imgui/imgui.h"


ENGINE_EXPORT void SGRX_IMGUI_Init();
ENGINE_EXPORT void SGRX_IMGUI_Free();
ENGINE_EXPORT void SGRX_IMGUI_ClearEvents();
ENGINE_EXPORT void SGRX_IMGUI_Event( const Event& e );
ENGINE_EXPORT void SGRX_IMGUI_NewFrame();
ENGINE_EXPORT void SGRX_IMGUI_Render();


