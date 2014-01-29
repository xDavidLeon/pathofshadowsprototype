#ifndef DEBUG_SYS
#define DEBUG_SYS

#include "entity.h"

class DebugSystem
{
	DebugSystem(void);
	~DebugSystem(void);

public:
	struct DEBUG_UPDATE_TIMES
	{
		double updateStartTime;
		double updateCurrentTime;
		double cameraSystemTime;
		double automatSystemTime;
		double animationSystemTime;
		double shadowSystemTime;
		double rendererSystemTime;
		double btSystemTime;
		double triggerSystemTime;
		double physicsTime;
		double soundSystemTime;

		DEBUG_UPDATE_TIMES () 
		{
			init();
		}
		
		void init()
		{
			double updateStartTime = 0;
			double updateCurrentTime = 0;
			double cameraSystemTime = 0;
			double automatSystemTime = 0;
			double animationSystemTime = 0;
			double shadowSystemTime = 0;
			double rendererSystemTime = 0;
			double btSystemTime = 0;
			double triggerSystemTime = 0;
			double physicsSystemTime = 0;
			double soundSystemTime = 0;
		}
	};

	struct DEBUG_RENDER_TIMES
	{
		double updateStartTime;
		double updateCurrentTime;
		double cameraSystemTime;
		double automatSystemTime;
		double animationSystemTime;
		double shadowSystemTime;
		double rendererSystemTime;
		double btSystemTime;
		double triggerSystemTime;
		double physicsTime;
		double soundSystemTime;

		DEBUG_RENDER_TIMES () 
		{
			init();
		}
		
		void init()
		{
			double updateStartTime = 0;
			double updateCurrentTime = 0;
			double cameraSystemTime = 0;
			double automatSystemTime = 0;
			double animationSystemTime = 0;
			double shadowSystemTime = 0;
			double rendererSystemTime = 0;
			double btSystemTime = 0;
			double triggerSystemTime = 0;
			double physicsSystemTime = 0;
			double soundSystemTime = 0;
		}
	};

	enum DRAW_MODE
	{
		DEFERRED = 0,
		FORWARD = 1,
		WIREFRAME = 2,
		RT = 3
	};

	enum AXIS_MODE
	{
		DISABLED = 0,
		AABB_AXIS = 1,
		BULLET_WIREFRAME = 2,
		BULLET_AABB = 3
	};

	static DebugSystem & get()
	{
		static DebugSystem cs;
		return cs;
	}

	void init(void);
	void render(void);
	void update(float delta);
	void cycleDrawMode(void);
	void cycleAxisMode(void);
	void toggleRenderGraph(void);
	void toggleRenderAutomat();
	void toggleRenderAI();
	void toggleAILocked(void);
	void toggleAnimLocked();
	void toggleTriggers(void);
	void togglePostFX(void);
	void toggleDebug(void);
	void toggleLights(void);
	void toggleParticleSystem(void);
	void doEnable(bool e);
	void updateDebugStartTime();
	void updateDebugCurrentTime();
	
	bool is_active;
	bool debug_ON;
	DRAW_MODE draw_mode;
	AXIS_MODE axis_mode;
	int painted_objects;
	bool is_player_locked;
	bool is_ai_locked;
	bool is_anim_locked;
	bool render_graph;
	bool render_automat;
	bool render_ai;
	bool is_triggers_active;
	bool is_postfx_active;
	bool is_lights_active;
	bool is_particles_active;

	float elapsedTimeInSeconds, timeBefore;

	DEBUG_UPDATE_TIMES update_times;
	DEBUG_RENDER_TIMES render_times;

private:
	double	_lastTime;
	double	_FPS;

};

#endif
