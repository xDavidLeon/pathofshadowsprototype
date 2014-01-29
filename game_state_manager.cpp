#include "game_state_manager.h"
#include "d3ddefs.h"
#include "splash_screen.h"
#include "title_screen.h"
#include "world.h"
#include "first_load_screen.h"

GameStateManager::GameStateManager()
{
	_current_game_state = NULL;
	game_state = NONE;
}

GameStateManager::~GameStateManager()
{
}

void GameStateManager::update(float delta)
{
	_current_game_state->update(delta);
}

void GameStateManager::render(void)
{
	_current_game_state->render();
}

void GameStateManager::setGameState(GAME_STATE state)
{
	if (game_state == state) return;
	
	switch (state)
	{
	case TITLE_SCREEN:
		_current_game_state = new TitleScreen();
		_current_game_state->init();
		break;
	case WORLD:
		_current_game_state = World::instance();
		_current_game_state->init();
		break;
	case SPLASH_SCREEN:
		_current_game_state = new SplashScreen();
		_current_game_state->init();
		break;
	case FIRSTLOAD_SCREEN:
		_current_game_state = new FirstLoadScreen();
		_current_game_state->init();
		break;
	}

}

void GameStateManager::onKeyDown(WPARAM key)
{
	_current_game_state->onKeyDown(key);
}
