#pragma once
#include "game_state.h"
#include "texture_manager.h"
#include "system_sound.h"

class TitleScreen :
	public GameState
{
public:
	TitleScreen(void);
	~TitleScreen(void);

	void init(void);
	void update(float delta);
	void render(void);
	void onKeyDown(WPARAM key);

private:
	TTexture _texture_title_wall;
	/*TTexture _texture_title_player;
	TTexture _texture_title_logo;
	TTexture _texture_title_press_start;
	TTexture _texture_title_dragon;*/
	SoundSystem::SoundInfo * _title_track;

	float _total_time;
	float _alpha_multiplier;

	bool _start_pressed;

	//bool _start_pressed2;
	//TTexture _texture_controls;
	//bool _controls_screen;

	void launchGame();
};

