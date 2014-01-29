#include "system_debug.h"
#include "world.h"
#include "dijkstra.h"
#include "component_automat.h"
#include "component_player_controller.h"
#include "iostatus.h"
#include "system_renderer.h"
#include "system_physics.h"
#include "entity_manager.h"
#include "system_camera.h"
#include "system_bt.h"
#include "vision_interface.h"

DebugSystem::DebugSystem(void)
{
	init();
}

DebugSystem::~DebugSystem(void)
{
}

void DebugSystem::update(float delta)
{
	if (is_active == false) return;

	if (CIOStatus::instance()->becomesPressed_key('1')) cycleDrawMode();
	else if (CIOStatus::instance()->becomesPressed_key('2')) cycleAxisMode();
	else if (CIOStatus::instance()->becomesPressed_key('3')) togglePostFX();
	else if (CIOStatus::instance()->becomesPressed_key('4')) toggleLights();
	else if (CIOStatus::instance()->becomesPressed_key('5')) toggleParticleSystem();
	else if (CIOStatus::instance()->becomesPressed_key('6')) toggleRenderAutomat();
	else if (CIOStatus::instance()->becomesPressed_key('9')) RendererSystem::get().loadShaders();
	else if (CIOStatus::instance()->becomesPressed_key('0')) World::instance()->toggleDbgCamera(true);
	//else if (CIOStatus::instance()->becomesPressed_key('T')) is_triggers_active = !is_triggers_active;
	else if (CIOStatus::instance()->isPressed('P'))	
	{
		float f = 1;
		g_App.effect->GetFloat( "AmbientReductionFactor", &f );
		f -= delta;
		if (f <= 0) f = 0.1f;
		g_App.effect->SetFloat( "AmbientReductionFactor", f );
	}
	else if (CIOStatus::instance()->isPressed('O'))	
	{
		float f = 1;
		g_App.effect->GetFloat( "AmbientReductionFactor", &f );
		f += delta;
		g_App.effect->SetFloat( "AmbientReductionFactor", f );
	}
	else if (CIOStatus::instance()->isPressed('K'))	
	{
		float f = 1;
		g_App.effect->GetFloat( "BloomIntensity", &f );
		f -= delta;
		if (f <= 0) f = 0.1f;
		g_App.effect->SetFloat( "BloomIntensity", f );
	}
	else if (CIOStatus::instance()->isPressed('L'))	
	{
		float f = 1;
		g_App.effect->GetFloat( "BloomIntensity", &f );
		f += delta;
		g_App.effect->SetFloat( "BloomIntensity", f );
	}

	update_times.init();
}

void DebugSystem::render(void)
{
	if (is_active == false) return;

	double deltaT = timeGetTime()-_lastTime;
	_lastTime = timeGetTime();
	_FPS = (1.0f/deltaT)*1000.0f;

	printf2D( g_App.GetWidth()*0.025f, g_App.GetHeight()*0.025f, D3DCOLOR_ARGB( 150, 255, 0, 0 ), "DEBUG MODE");

	// STATS
	printf2D( g_App.GetWidth()*0.05f, g_App.GetHeight()*0.075f, D3DCOLOR_ARGB( 150, 0, 255, 255 ), "STATS");

	// FPS
	float coolness = _FPS/60.0f;
	if (coolness > 1) coolness = 1;
	if (coolness < 0) coolness = 0;

	float r = 255 - 255*coolness;
	if (r > 255) r = 255; if (r < 0) r = 0;
	float g = 255 * coolness;
	if (g > 255) g = 255; if (g < 0) g = 0;
	printf2D( g_App.GetWidth()*0.075f, g_App.GetHeight()*0.1f, D3DCOLOR_ARGB( 150, (int)r, (int)g, 0 ), "fps - %.0f", _FPS);

	unsigned text_color = D3DCOLOR_ARGB( 150, 255, 255, 255 );

	printf2D( g_App.GetWidth()*0.075f, g_App.GetHeight()*0.125f,text_color, "%s - %.0f","world time", World::instance()->world_time);
	printf2D( g_App.GetWidth()*0.075f, g_App.GetHeight()*0.15f, text_color, "%s - %f", "time elapsed", deltaT);
	
	// RENDER
	printf2D( g_App.GetWidth()*0.05f, g_App.GetHeight()*0.2f, D3DCOLOR_ARGB( 150, 0, 255, 255 ), "RENDER");
	switch (draw_mode)
	{
	case DRAW_MODE::DEFERRED:
		printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.225f, text_color, "draw mode - deferred");
		break;
	case DRAW_MODE::FORWARD:
		printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.225f, text_color, "draw mode - forward");
		break;
	case DRAW_MODE::RT:
		printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.225f, text_color, "draw mode - render targets");
		break;
	case DRAW_MODE::WIREFRAME:
		printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.225f, text_color, "draw mode - wireframe");
		break;
	}

	printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.25f, text_color, "drawn objects - %d", painted_objects);
	if (is_postfx_active) printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.275f, text_color, "post fx - enabled");
	else printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.275f, text_color, "post fx - disabled");
	if (is_lights_active) printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.3f, text_color, "lights - enabled");
	else printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.3f, text_color, "lights - disabled");
	if (is_particles_active) printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.325f, text_color, "particles - enabled");
	else printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.325f, text_color, "particles - disabled");
	
	// OTHER
	printf2D( g_App.GetWidth()*0.05f, g_App.GetHeight()*0.375f, D3DCOLOR_ARGB( 150, 0, 255, 255 ), "OTHER");
	printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.4f, text_color, "debug bullet mode - %d", axis_mode);
	Entity* player = World::instance()->getPlayer();
	if (player != NULL) 
	{
		PlayerControllerComponent* p = EntityManager::get().getComponent<PlayerControllerComponent>(player);
		printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.425f, text_color, "player life - %.2f", p->_life);
	}

	float f = 1;
	g_App.effect->GetFloat( "AmbientReductionFactor", &f );
	printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.45f, text_color, "light level - %.2f", 1.0f/f);

	float b = 1;
	g_App.effect->GetFloat( "BloomIntensity", &b );
	printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.475f, text_color, "bloom level - %.2f", b);

	//Triggers enabled??
	if(VisionInterface::get().thereIsAlert() || !DebugSystem::get().is_triggers_active)
		printf2D( g_App.GetWidth()*0.075f, g_App.GetHeight()*0.55f, text_color, "triggers: OFF");
	else
		printf2D( g_App.GetWidth()*0.075f, g_App.GetHeight()*0.55f, text_color, "triggers: ON");

	// STATS
	printf2D( g_App.GetWidth()*0.77f, g_App.GetHeight()*0.025f, D3DCOLOR_ARGB( 150, 0, 255, 255 ), "UPDATE TIMES");
	double update_time = update_times.updateCurrentTime - update_times.updateStartTime;
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.05f, text_color, "update time - %.5f", update_time);
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.075f, text_color, "CameraSystem - %.2f", (update_times.cameraSystemTime/update_time) * 100);
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.1f, text_color, "AutomatSystem - %.2f", (update_times.automatSystemTime/update_time) * 100);
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.125f, text_color, "AnimationSystem - %.2f", (update_times.animationSystemTime/update_time) * 100);
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.15f, text_color, "ShadowSystem - %.2f", (update_times.shadowSystemTime/update_time) * 100);
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.175f, text_color, "RendererSystem - %.2f", (update_times.rendererSystemTime/update_time) * 100);
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.2f, text_color, "BTSystem - %.2f", (update_times.btSystemTime/update_time) * 100);
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.225f, text_color, "TriggerSystem - %.2f", (update_times.triggerSystemTime/update_time) * 100);
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.25f, text_color, "PhysicsSystem - %.2f", (update_times.physicsTime/update_time) * 100);
	printf2D(g_App.GetWidth()*0.8f, g_App.GetHeight()*0.275f, text_color, "SoundSystem - %.2f", (update_times.soundSystemTime/update_time) * 100);

	// OTHER
	//printf2D( g_App.GetWidth()*0.05f, g_App.GetHeight()*0.375f, D3DCOLOR_ARGB( 150, 0, 255, 255 ), "OTHER");
	//printf2D(g_App.GetWidth()*0.075f, g_App.GetHeight()*0.4f, text_color, "aabb mode - %d", axis_mode);

	if(render_graph) DijkstraGraph::get().renderGraph(D3DCOLOR_ARGB(255, 255,255,0), D3DCOLOR_ARGB(255, 0,255,0));
	if(render_automat) EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->render();
	if(render_ai) BTSystem::get().render(); //Debug enemigos


	// CAMERA DBG
	CameraSystem::get().renderDbgCameraInfo();

	if (axis_mode != AXIS_MODE::DISABLED) PhysicsSystem::get().getDynamicsWorld()->debugDrawWorld();

	render_times.init();
	painted_objects = 0;
}

void DebugSystem::init(void)
{
	is_active = false;
	debug_ON = false;
	is_postfx_active = true;
	draw_mode = DRAW_MODE::DEFERRED;
	axis_mode = AXIS_MODE::DISABLED;
	painted_objects = 0;
	is_player_locked = false;
	is_ai_locked = false;
	render_graph = false;
	render_automat = false;
	render_ai = false;
	is_triggers_active = true;
	is_particles_active = true;
	_lastTime = clock();
	_FPS = 0;
	is_lights_active = true;
	//timeBefore = ((float)(timeGetTime()))/1000;
}

void DebugSystem::cycleDrawMode(void)
{
	int d = draw_mode + 1;
	draw_mode = (DRAW_MODE) (d%4);
}

void DebugSystem::cycleAxisMode(void)
{
	int d = axis_mode + 1;
	axis_mode = (AXIS_MODE) (d%4);

	switch (axis_mode)
	{
	case AXIS_MODE::DISABLED:
		PhysicsSystem::get().getDynamicsWorld()->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_NoDebug);
		break;
	case AXIS_MODE::BULLET_AABB:
		PhysicsSystem::get().getDynamicsWorld()->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawAabb);
		break;
	case AXIS_MODE::BULLET_WIREFRAME:
		PhysicsSystem::get().getDynamicsWorld()->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
		break;
	case AXIS_MODE::AABB_AXIS:
		PhysicsSystem::get().getDynamicsWorld()->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb);
		break;
	default:
		PhysicsSystem::get().getDynamicsWorld()->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_NoDebug);
		break;
	}
}

void DebugSystem::toggleRenderGraph(void)
{
	render_graph = !render_graph;
}

void DebugSystem::toggleRenderAutomat(void)
{
	render_automat = !render_automat;
}

void DebugSystem::toggleRenderAI()
{
	render_ai = !render_ai;
}

void DebugSystem::toggleAILocked(void)
{
	is_ai_locked = !is_ai_locked;
}

void DebugSystem::toggleAnimLocked()
{
	is_anim_locked = !is_anim_locked;
}

void DebugSystem::toggleTriggers(void)
{
	is_triggers_active = !is_triggers_active;
}

void DebugSystem::togglePostFX(void)
{
	is_postfx_active = !is_postfx_active;
}

void DebugSystem::toggleLights(void)
{
	is_lights_active = !is_lights_active;	
}

void DebugSystem::toggleParticleSystem(void)
{
	is_particles_active = !is_particles_active;	
}

void DebugSystem::toggleDebug(void)
{
	if(is_active)
	{
		is_active = false;
		//init();
	}
	else is_active = true;
}

void DebugSystem::doEnable(bool e)
{
	if (is_active != e) toggleDebug();
}

void DebugSystem::updateDebugStartTime()
{
	update_times.updateStartTime = timeGetTime(); update_times.updateCurrentTime = update_times.updateStartTime;
}

void DebugSystem::updateDebugCurrentTime()
{
	update_times.updateCurrentTime = timeGetTime();
}
