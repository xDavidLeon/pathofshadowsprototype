#pragma once
#include "game_state.h"
#include "texture_manager.h"
#include "system_sound.h"

class SplashScreen :
	public GameState
{
public:
	SplashScreen(void);
	~SplashScreen(void);

	void init(void);
	void update(float delta);
	void render(void);
	void onKeyDown(WPARAM key);

private:
	TTexture _texture_splash;
	SoundSystem::SoundInfo * _title_track;

	float _total_time;
	float _alpha_multiplier;

	bool _start_pressed;

	void launchGame();
};

