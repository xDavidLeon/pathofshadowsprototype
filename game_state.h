#ifndef INC_GAME_STATE_H_
#define INC_GAME_STATE_H_
#include<windows.h>
class GameState
{
public:
	GameState(void);
	virtual ~GameState(void) = 0;

	virtual void init(void) = 0;
	virtual void update(float delta) = 0;
	virtual void render(void) = 0;
	virtual void onKeyDown(WPARAM key) = 0;
};

#endif