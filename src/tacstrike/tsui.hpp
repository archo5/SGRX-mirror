

#pragma once
#include "gameui.hpp"


extern struct TSMenuTheme g_TSMenuTheme;
extern struct TSSplashScreen g_SplashScreen;
extern struct TSQuitGameQuestionScreen g_QuitGameQuestionScreen;
extern struct TSPauseMenuScreen g_PauseMenu;


enum TSMenuCtrlStyle
{
	MCS_BigTopLink = 100,
};


struct TSMenuTheme : MenuTheme
{
	virtual void DrawControl( const MenuControl& ctrl, const MenuCtrlInfo& info );
	void DrawBigTopLinkButton( const MenuControl& ctrl, const MenuCtrlInfo& info );
};


struct TSSplashScreen : IScreen
{
	TSSplashScreen();
	void OnStart();
	void OnEnd();
	bool OnEvent( const Event& e );
	bool Draw( float delta );
	
	float m_timer;
	TextureHandle m_tx_crage;
	TextureHandle m_tx_back;
};


struct TSQuestionScreen : IScreen
{
	TSQuestionScreen();
	void OnStart();
	void OnEnd();
	bool OnEvent( const Event& e );
	bool Draw( float delta );
	virtual bool Action( int mode );
	
	float animFactor;
	float animTarget;
	String question;
	ScreenMenu menu;
};


struct TSQuitGameQuestionScreen : TSQuestionScreen
{
	TSQuitGameQuestionScreen();
	virtual bool Action( int mode );
};


struct TSPauseMenuScreen : IScreen
{
	TSPauseMenuScreen();
	void OnStart();
	void OnEnd();
	void Unpause();
	bool OnEvent( const Event& e );
	bool Draw( float delta );
	
	bool notfirst;
	ScreenMenu topmenu;
	ScreenMenu pausemenu;
	ScreenMenu objmenu;
};


