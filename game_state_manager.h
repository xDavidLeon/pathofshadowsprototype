#pragma once
#include "game_state.h"
class GameStateManager
{
public:
	enum GAME_STATE
	{
		NONE,
		SPLASH_SCREEN,
		TITLE_SCREEN,
		WORLD,
		FIRSTLOAD_SCREEN
	};

	static GameStateManager & get()
	{
		static GameStateManager singleton;
		return singleton;
	}

	void setGameState(GAME_STATE state);

	void update(float delta);
	void render(void);

	void onKeyDown(WPARAM param);

	GAME_STATE game_state;
protected:
	GameStateManager();
	GameStateManager(const GameStateManager &);
	GameStateManager &operator= (const GameStateManager &);
	~GameStateManager( );
private:
	static GameStateManager*		_instance;

	GameState* _current_game_state;

};

