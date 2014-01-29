#include "first_load_screen.h"
#include "d3ddefs.h"
#include "game_state_manager.h"
#include "iostatus.h"
#include "system_renderer.h"
#include "system_sound.h"

FirstLoadScreen::FirstLoadScreen(void)
{
}

FirstLoadScreen::~FirstLoadScreen(void)
{
}

void FirstLoadScreen::init(void)
{
	CameraSystem::get().setBlackScreen(true);
	CameraSystem::get().setBlackToLoadingScreen(true);
}

void FirstLoadScreen::update(float delta)
{
	launchGame();
}

void FirstLoadScreen::render(void)
{
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );

	CameraSystem::get().renderBlackScreen();
}

void FirstLoadScreen::onKeyDown(WPARAM key)
{
}

void FirstLoadScreen::launchGame()
{
	GameStateManager::get().setGameState(GameStateManager::WORLD);
}
