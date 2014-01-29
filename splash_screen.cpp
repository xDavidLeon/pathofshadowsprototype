#include "splash_screen.h"
#include "d3ddefs.h"
#include "game_state_manager.h"
#include "iostatus.h"
#include "system_renderer.h"
#include "system_sound.h"

SplashScreen::SplashScreen(void)
{
	_total_time = 0;
	_alpha_multiplier = 0;
	_start_pressed = false;
}

SplashScreen::~SplashScreen(void)
{
	delete _texture_splash;
}

void SplashScreen::init(void)
{
	_texture_splash = TTextureManager::get().getTextureResized("hardcoded/logo_hydra", g_App.GetWidth(), g_App.GetHeight());
	_title_track = SoundSystem::get().playStream("title_track", "data/music/dark_fallout.ogg","title_track",0, 1.0f,true);
}

void SplashScreen::update(float delta)
{
	_total_time += delta;
	if (_start_pressed || _total_time > 4.0f)
	{
		_alpha_multiplier -= delta/2.0f;
		if (_alpha_multiplier <= 0) 
		{
			_alpha_multiplier = 0;
			launchGame();
		}
	}
	else
	{
		_alpha_multiplier += delta/6.0f;
		if (_alpha_multiplier >= 1) _alpha_multiplier = 1;
	}

	if (CIOStatus::instance()->isPressed(CIOStatus::START) || CIOStatus::instance()->isPressed(CIOStatus::CREATE_SHADOW)) _start_pressed = true;
}

void SplashScreen::render(void)
{
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );

	g_App.GetDevice()->SetRenderState (D3DRS_ALPHABLENDENABLE, TRUE);

	RendererSystem::get().drawSprite(D3DXVECTOR3(0,0,0),D3DXVECTOR3(0,0,0),_texture_splash,D3DCOLOR_RGBA(255,255,255,(int)(255*_alpha_multiplier)));
}

void SplashScreen::onKeyDown(WPARAM key)
{
	_start_pressed = true;
}

void SplashScreen::launchGame()
{
	GameStateManager::get().setGameState(GameStateManager::TITLE_SCREEN);
}
