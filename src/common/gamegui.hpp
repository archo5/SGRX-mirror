

#pragma once
#include "engine.hpp"
#include "script.hpp"



#define GUI_ScrMode_Crop 0
#define GUI_ScrMode_Fit 1
#define GUI_ScrMode_Abs 2


struct GameUIControl
{
	typedef sgsHandle< GameUIControl > Handle;
	
	SGS_OBJECT;
	
	SGS_PROPERTY float x;
	SGS_PROPERTY float y;
	SGS_PROPERTY float width;
	SGS_PROPERTY float height;
	SGS_PROPERTY float rx0;
	SGS_PROPERTY float ry0;
	SGS_PROPERTY float rx1;
	SGS_PROPERTY float ry1;
	SGS_PROPERTY sgsVariable metadata;
	SGS_PROPERTY_FUNC( READ ) sgsHandle< struct GameUIScreen > parent;
	SGS_PROPERTY sgsVariable eventCallback;
	SGS_PROPERTY_FUNC( READ ) sgsVariable animators; // array
	SGS_PROPERTY_FUNC( READ ) sgsVariable shaders; // array
	
	GameUIControl();
	static GameUIControl* Create( SGS_CTX );
	void OnEvent( const Event& e );
	void Tick( float dt );
	void Draw();
	
	// ---
	SGS_IFUNC( GETINDEX ) int _getindex( SGS_ARGS_GETINDEXFUNC );
	SGS_IFUNC( SETINDEX ) int _setindex( SGS_ARGS_SETINDEXFUNC );
};


struct GameUIScreen
{
	typedef sgsHandle< GameUIScreen > Handle;
	
	SGS_OBJECT;
	
	SGS_PROPERTY int mode;
	SGS_PROPERTY float x;
	SGS_PROPERTY float y;
	SGS_PROPERTY float width;
	SGS_PROPERTY float height;
	SGS_PROPERTY float xalign;
	SGS_PROPERTY float yalign;
	SGS_PROPERTY float rx0;
	SGS_PROPERTY float ry0;
	SGS_PROPERTY float rx1;
	SGS_PROPERTY float ry1;
	SGS_PROPERTY sgsVariable metadata;
	SGS_PROPERTY_FUNC( READ ) Handle parent;
	
	Array< GameUIControl* > m_ctrls;
	Array< GameUIScreen* > m_screens;
	
	static GameUIScreen* Create( SGS_CTX );
	void Acquire();
	void Release();
	void OnEvent( const Event& e );
	void Tick( float dt );
	void Draw();
	
	float IX( float x );
	float IY( float y );
	
	// ---
	SGS_IFUNC( GETINDEX ) int _getindex( SGS_ARGS_GETINDEXFUNC );
	SGS_IFUNC( SETINDEX ) int _setindex( SGS_ARGS_SETINDEXFUNC );
	
	SGS_METHOD Handle CreateScreen( int mode, float width, float height,
		float xalign /* = 0 */, float yalign /* = 0 */, float x /* = 0 */, float y /* = 0 */ );
	SGS_METHOD GameUIControl::Handle CreateControl(
		float x, float y, float width, float height );
};


