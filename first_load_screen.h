#ifndef FIRST_LOAD_SCREEN
#define FIRST_LOAD_SCREEN

#include "game_state.h"
#include "system_camera.h"

class FirstLoadScreen : public GameState
{
public:
	FirstLoadScreen(void);
	~FirstLoadScreen(void);

	void init(void);
	void update(float delta);
	void render(void);
	void onKeyDown(WPARAM key);

private:
	TTexture _texture_splash;

	float _total_time;
	float _alpha_multiplier;

	bool _start_pressed;

	void launchGame();
};

#endif