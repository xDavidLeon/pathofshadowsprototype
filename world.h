#ifndef INC_WORLD_H_
#define INC_WORLD_H_

#include <vector>
#include <ctime>
#include "game_state.h"

class Entity;
class World : public GameState {
	
public:
	static World*	instance();
	static void		deleteInstance();

	void		loadScene(const std::string& sceneName);
	void		reload(void);
	void		resetCurrentLevel();
	void		init();
	void		render();
	void		update(float delta);
	void		setMustReload() { _mustReload = true; }
	void		mustResetCurrentLevel() { _mustReset = true; }
	
	void		setDebugMode(bool d);
	bool		isDebugModeOn(void);
	void		setPlayer(Entity* p);
	Entity*		getPlayer(){ return _player; }
	void		setPlayerLocked(bool lock);
	std::string	getCurrentSceneName() { return _currentSceneName;};

	void		toggleDbgCamera(bool setAtCamPlayer = true);
	void		toggleKillCamera();
	void		toggleDeathCamera();
	void		onKeyDown(WPARAM key);

	void		setTimeScale( float timeScale ) { _timeScale = timeScale; };
	float		getTimeScale() { return _timeScale; };
	
	float		world_time;

	bool		_specialVision;

	void		toggleSpecialVision();

	float		getElapsedTimeRInSeconds(){ return elapsed_timeR; }
	float		getElapsedTimeUInSeconds(){ return elapsed_timeU; }

	void		enableCrow();
	void		disableCrow();

	void		enableCredits(bool enable){ _creditsActive = enable; }

protected:
	World();
	World(const World &);
	World &operator= (const World &);
	~World( );

private:
	static World*		_instance;

	Entity				*_player, *_crow;
	float				_timeScale;

	std::string			_currentSceneName;
	double				_last_call;

	bool _mustReload, _mustReset; //reload: cargar de disco xml de _currentSceneName, reset: init personajes del nivel
	bool _creditsActive;

	float elapsed_timeR, elapsed_timeU;
	double _timeBeforeR, _timeBeforeU;

	void	initBasics(void);
	void	initCurrentScene(void);
	//void	initPhysics(void);
	void	initAI(void);
	void	cleanUp(void);
};

#endif

