#include "world.h"
#include "config_manager.h"
#include "system_debug.h"
#include "system_camera.h"
#include "system_playercontroller.h"
#include "mesh_manager.h"
#include "system_physics.h"
#include "entity_manager.h"
#include "texture_manager.h"
#include "system_bt.h"
#include "system_sound.h"
#include "camera_controller_3rd.h"
#include "system_shadow.h"
#include "scene_manager.h"
#include "entity_factory.h"
#include "system_renderer.h"
#include "console_lua.h"
#include "vision_interface.h"
#include "component_shadow_actions.h"
#include "system_automat.h"
#include "system_animation.h"
#include "system_trigger.h"
#include "system_unique.h"
#include "component_bt.h"
#include "component_automat.h"
#include "component_enemy_data.h"


#include "logic_manager.h"

World* World::_instance = 0;

World* World::instance ()
{
  if (_instance == 0)
  {
    _instance = new World;
  }
  return _instance;
}

World::World()
{
	//Se lee el archivo de configuracion. Los datos se mantienen en ConfigManager
	//ConfigManager::get().readConfigFile();
	//Ahora los datos que antes se pillaban del config file ahora se pillan de la ventana de inicio
	ConfigManager::get().readConfig();
	_currentSceneName = ConfigManager::get().init_scene;

	_mustReload = _mustReset = _creditsActive = false;
	//DebugSystem::get().debug_ON = true;

	#ifdef EDU_DBG
		DebugSystem::get().is_triggers_active = true;
		DebugSystem::get().debug_ON = true;
	#elif ALMUVA_DBG
		DebugSystem::get().debug_ON = true;
		//DebugSystem::get().is_triggers_active = false;
		//DebugSystem::get().render_graph = true;
		//DebugSystem::get().render_automat = true;
		DebugSystem::get().render_ai = true;
		//_creditsActive = true;
	#else
		//DebugSystem::get().is_triggers_active = false;
	#endif

	if (g_App.GetDebug() > 0) DebugSystem::get().debug_ON = true;
}

void World::setDebugMode(bool d)
{
	DebugSystem::get().doEnable(d);
}

bool World::isDebugModeOn(void)
{
	return DebugSystem::get().is_active;
}

void World::toggleDbgCamera(bool setAtCamPlayer)
{
	DebugSystem::get().is_player_locked = CameraSystem::get().toggleDbgCamera(setAtCamPlayer);
}

void World::toggleKillCamera()
{
	CameraSystem::get().toggleKillCamera();
}

void World::toggleDeathCamera()
{
	CameraSystem::get().toggleDeathCamera();
}

void World::setPlayer(Entity* p)
{
	_player = p;
	PlayerControllerSystem::get().setPlayer(p);
}

void World::init()
{
	initBasics();
	//initPhysics();
	initCurrentScene();
}

void World::cleanUp(void)
{
	TMeshManager::get().releaseAll();
	PhysicsSystem::get().releaseAll();
	EntityManager::get().releaseAll();
	//TTextureManager::get().releaseAll(); //sino peta al buscar texturas (en variables) despues de reload
	CameraSystem::get().releaseCineCameras();
	LightSystem::get().releaseLights();
	TriggerSystem::get().release();
	BTSystem::get().release();
	UniqueSystem::get().release();
}

void World::reload(void)
{
	cleanUp();
	//initPhysics();
	initCurrentScene();

	_mustReload = false;
}

void World::resetCurrentLevel()
{
	//Se obtienen todas las entidades con componente charactercontroller
	std::map<Entity*,Component*>* characters = EntityManager::get().getAllEntitiesPosessingComponent<CharacterControllerComponent>();
	if(!characters) return;

	//Se llama el init de cada componente de cada character (entidad), si la entidad esta activada
	std::map<Entity*,Component*>::iterator iter;
	for (iter = characters->begin(); iter != characters->end(); ++iter)
	{
		if(!iter->first->enabled) continue; //Si la entidad no esta activa nos la pasamos
		if(EntityManager::get().getComponent<EnemyDataComponent>(iter->first)) //si la entity tiene EnemyDataComponent...
			if(!EntityManager::get().getComponent<EnemyDataComponent>(iter->first)->enabled) //...y esta desactivado (enemigo muerto), nos lo pasamos
				continue; 

		std::set<Component*>* char_components = EntityManager::get().getAllComponentsOfEntity(iter->first);
		std::set<Component*>::iterator iter2;
		for (iter2 = char_components->begin(); iter2 != char_components->end(); ++iter2)
			(*iter2)->init();
	}

	//Nos aseguramos que la camara activa actual es la del player
	CameraSystem::get().setCurrentCamera(CameraSystem::get().getPlayerCameraEntity());
	//Reseteamos la camara del player
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->init();

	//Eliminar sombras magicas
	ShadowSystem::get().destroyShadows();

	_mustReset = false;
}

void World::initBasics(void)
{
	_specialVision = false;
	_timeScale = 1.0f;
	_last_call = timeGetTime();
	//SoundSystem::get().init();
}

//void World::initPhysics(void)
//{
//	// Test Ground
//	//PhysicsSystem::get().addStaticPlane(btVector3(0,-10,0));
//}

void World::initCurrentScene()
{
	_player = NULL;

	//Pantalla en negro
	CameraSystem::get().setBlackScreen(true);

	//Se carga la escena actual
	bool is_ok = TSceneManager::get()->xmlParseFile("data/scenes/"+_currentSceneName+".xml");
	if(!is_ok) fatalErrorWindow(std::string("No se ha encontrado el archivo scenes/" + _currentSceneName + ".xml o bien contiene errores").c_str());

	float aspect_ratio = (float) g_App.GetWidth() / (float) g_App.GetHeight();

	//Camera player
	Entity* player_camera = EntityFactory::get().createCamera(CAM_TYPE::CAM_3RD
														,btVector3( 0,20,30 ), btVector3(0,0,0)
														,65.0f, aspect_ratio, 0.25f, 300.0f);
	CameraSystem::get().setPlayerCamera(player_camera);
	CameraSystem::get().setCurrentCamera(player_camera);

	//Asignar player a su camara
	CameraSystem::get().getPlayerCamera().controller->setTargetEntity(_player);
	CameraSystem::get().getPlayerCamera().controller->init();

	//Camera debug
	Entity* debug_camera = EntityFactory::get().createCamera(CAM_TYPE::CAM_DBG
														,btVector3( 0,20,30 ), btVector3(0,0,0)
														,65.0f, aspect_ratio, 0.25f, 300.0f);
	CameraSystem::get().setDbgCamera(debug_camera);

	//Camera cine
	Entity* cine_camera = EntityFactory::get().createCamera(CAM_TYPE::CAM_CIN
														,btVector3( 0,20,30 ), btVector3(0,0,0)
														,65.0f, aspect_ratio, 0.25f, 300.0f);
	CameraSystem::get().setCinCamera(cine_camera);

	//Lights
	//Ahora la dir. light se carga del xml

	// TEST
	//_entityFactory->createSpotLight(D3DXVECTOR3(0,10,0),D3DXVECTOR3(-1,-1,-1),D3DXCOLOR(1,0,0,1),1.0f,1.0f,30.0f,M_PI/2,false);

	//sonido
	
	SoundSystem::get().playStream("wind", "data/sfx/wind.wav", "wind", 0, 0.15f, true);

	world_time = 0;
	_timeBeforeR = ((float)(timeGetTime()))/1000;
	_timeBeforeU = _timeBeforeR;

	if(_currentSceneName == "cementerio" || _currentSceneName == "cementerio_0") EntityFactory::get().createGoddess();

	CameraSystem::get().setBlackToLoadingScreen(false);
	CameraSystem::get().toggleBlackScreen();

	_crow = EntityFactory::get().createCrow();
	disableCrow();

	if (_currentSceneName == "aldea") 
	{
		SoundSystem::get().playStream("aldea","data/music/aldea.mp3","aldea",0,0.25f,true);
		EntityFactory::get().createCineSeq(52);
	}
	else if (_currentSceneName == "patio")
	{
		SoundSystem::get().playStream("patio","data/music/patio.wav","patio",0,0.25f,true);
	}
	else if(_currentSceneName == "cementerio")
	{
		if(_creditsActive) LogicManager::get().runScript("trigger028()");
		if(!DebugSystem::get().debug_ON) CameraSystem::get().setLockCamera3rd(true);
	}
}

void World::loadScene(const std::string& sceneName)
{
	_currentSceneName = sceneName;
	_mustReload = true;
}

void World::render()
{
	//el time_elapsed se usa para el SystemCamera (cinematicas)
	double timeNow = ((double)timeGetTime())/1000.0f; // /1000: en segundos
	elapsed_timeR = timeNow - _timeBeforeR;
	//elapsed_timeR = (float)elapsed_timeR;
	//dbg("et: %.4f\n", elapsed_timeR);
	if(elapsed_timeR > 2.0f) elapsed_timeR = 0.0f;
	_timeBeforeR = timeNow;

	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );

	CameraSystem::get().render();

	// Render Systems
	RendererSystem::get().render();

	#ifdef EDU_DBG
		/*if (isDebugModeOn())*/ AnimationSystem::get().renderDebug(); 
		/*if (isDebugModeOn())*/ //BTSystem::get().render();
		/*if (isDebugModeOn())*/ ////_soundSystem->render();
		/*if (isDebugModeOn())*/ EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->render();
		DebugSystem::get().render_ai = true;
	#endif

	//if (_DEBUG_OPTIONS.ACTIVE) renderDebug();

	if(DebugSystem::get().debug_ON) ConsoleLua::get().render();

	//Crosshair del player e interfaz de visibilidad cuando esta en shadow mode
	if(!CameraSystem::get().isCineActive())
	{
		EntityManager::get().getComponent<ShadowActionsComponent>(World::instance()->getPlayer())->render();
		VisionInterface::get().render();
	}
	
	//Franjas de cinematica
	CameraSystem::get().renderBars();
	//Fundido en negro
	CameraSystem::get().renderBlackScreen();
	//logo
	CameraSystem::get().renderLogo();
	//Creditos en scroll
	CameraSystem::get().renderCredits();
	//subtitulos
	CameraSystem::get().renderSubs();

	DebugSystem::get().render();
}

void World::update(float delta)
{
	double timeNow =((double)timeGetTime())/1000.0f; // /1000: en segundos
	elapsed_timeU = timeNow - _timeBeforeU;
	if(elapsed_timeU > 2.0f) elapsed_timeU = 0.0f;
	_timeBeforeU = timeNow;

	world_time += delta;

	delta = delta * getTimeScale();

	if(_mustReload) reload();
	if(_mustReset) resetCurrentLevel();

	DebugSystem & debug = DebugSystem::get();
	debug.updateDebugStartTime();

	if(DebugSystem::get().debug_ON)
	{
		ConsoleLua::get().update();
		if(ConsoleLua::get().isOpen()) return;
	}

	if(CameraSystem::get().currentIsDbg() == false) 
	{
		LightSystem::get().update(delta);
		UniqueSystem::get().update(delta);
		AutomatSystem::get().update(delta);

		debug.update_times.automatSystemTime = timeGetTime() - debug.update_times.updateCurrentTime;
		debug.updateDebugCurrentTime();

		ShadowSystem::get().update(delta);
		debug.update_times.shadowSystemTime = timeGetTime() - debug.update_times.updateCurrentTime;
		debug.updateDebugCurrentTime();

		PhysicsSystem::get().getDynamicsWorld()->stepSimulation(1.0f/60.0f, 1);
		debug.update_times.physicsTime = timeGetTime() - debug.update_times.updateCurrentTime;
		debug.updateDebugCurrentTime();

		if(!DebugSystem::get().is_ai_locked) 
		{
			BTSystem::get().update(delta);
			debug.update_times.btSystemTime = timeGetTime() - debug.update_times.updateCurrentTime;
			debug.updateDebugCurrentTime();
		}

		if(!DebugSystem::get().is_anim_locked) 
		{
			AnimationSystem::get().update(delta);
			debug.update_times.animationSystemTime = timeGetTime() - debug.update_times.updateCurrentTime;
			debug.updateDebugCurrentTime();
		}

		if(DebugSystem::get().is_triggers_active)
		{
			TriggerSystem::get().update(delta);
			debug.update_times.triggerSystemTime = timeGetTime() - debug.update_times.updateCurrentTime;
			debug.updateDebugCurrentTime();
		}
	}

	CameraSystem::get().update(delta);
	debug.update_times.cameraSystemTime = timeGetTime() - debug.update_times.updateCurrentTime;
	debug.updateDebugCurrentTime();

	//FMOD_SYSTEM * fmod = SoundSystem::get().getFMODSystem();
	//float fmod_usage = 0;
	//fmod->getCPUUsage(NULL,NULL,NULL,NULL,&fmod_usage);
	debug.update_times.soundSystemTime = SoundSystem::get().getCPUUsage();
	//debug.updateDebugCurrentTime();

	DebugSystem::get().update(delta);

	RendererSystem::get().update(delta);
	debug.update_times.rendererSystemTime = timeGetTime() - debug.update_times.updateCurrentTime;
	debug.updateDebugCurrentTime();

	DebugSystem::get().update(delta);
}

void World::onKeyDown(WPARAM key)
{
	switch (key)
	{
	case VK_TAB:
		if(DebugSystem::get().debug_ON) setDebugMode(!isDebugModeOn());
		break;
	case VK_BACK:
		if(ConsoleLua::get().isOpen()) return;
		_mustReset = true;
		//_mustReload = true;
		break;
	case VK_NUMPAD1:
		break;
	default:
		break;
	}
}

World::~World()
{
	delete _player;
	delete _instance;
}

void World::toggleSpecialVision(void)
{
	_specialVision = !_specialVision;
	if (_specialVision == true)
	{
		g_App.effect->SetFloat( "WiggleTime", world_time );
	}
}


void World::setPlayerLocked(bool lock)
{
	DebugSystem::get().is_player_locked = lock; 
	AutomatSystem::get().lockPlayer(lock);
}

void World::enableCrow()
{
	if (_crow == NULL) return;
	_crow->enabled = true;
	EntityManager::get().getComponent<BTComponent>(_crow)->init();
}

void World::disableCrow()
{
	_crow->enabled = false;
}
