#include "lua_helper.h"
#include "world.h"
#include "camera_controller_3rd.h"
#include "component_player_controller.h"
#include "component_transform.h"
#include "system_camera.h"
#include "entity_manager.h"
#include "entity_factory.h"
#include "system_debug.h"
#include "system_sound.h"
#include "system_unique.h"
#include "system_bt.h"

LuaHelper::LuaHelper(void)
{
}

LuaHelper::~LuaHelper(void)
{
}

void LuaHelper::toggleDbgCamera()
{
	World::instance()->toggleDbgCamera();
}

void LuaHelper::setCameraDistance(float dist)
{
	((CameraController3rd*)(CameraSystem::get().getPlayerCamera().controller))->setDesiredDistance(dist);
}

int LuaHelper::createEnemy(float dist)
{
	TransformComponent* tC = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer());
	btTransform enemyT = *tC->transform;
	btVector3 front;  tC->getFrontXinv(front);
	enemyT.setOrigin(enemyT.getOrigin() + front*dist);
	Entity* enemy = EntityFactory::get().createEnemy(enemyT);

	return enemy->eid;
}

void LuaHelper::destroyEnemy(int id)
{
	Entity* entity = EntityManager::get().getEntityWithId(id);
	BTSystem::get().destroyEnemy(entity);
}

void LuaHelper::loadScene(const std::string& sceneName)
{
	World::instance()->loadScene(sceneName);
}

void LuaHelper::toggleAI()
{
	DebugSystem::get().toggleAILocked();
}

void LuaHelper::addCamToQueue(int cam_id)
{
	CameraSystem::get().addCamToQueue(cam_id);
}

void LuaHelper::renderGraph()
{
	DebugSystem::get().toggleRenderGraph();
}

void LuaHelper::enableEntity(const std::string& e_name)
{
	EntityManager::get().enableEntity(e_name, true);
}

void LuaHelper::disableEntity(const std::string& e_name)
{
	EntityManager::get().enableEntity(e_name, false);
}

void LuaHelper::setPlayerLife(float life)
{
	EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->setLife(life);
}

void LuaHelper::setPlayerPos(const std::string& rp_name)
{
	Entity* playerE = World::instance()->getPlayer();
	const btTransform *rpT = EntityManager::get().getComponent<PlayerControllerComponent>(playerE)->getRespawn(rp_name.c_str());
	if(rpT == NULL) return;
	//transform
	EntityManager::get().getComponent<TransformComponent>(playerE)->transform->setBasis(rpT->getBasis());
	EntityManager::get().getComponent<TransformComponent>(playerE)->transform->setOrigin(rpT->getOrigin());
	//character controller
	EntityManager::get().getComponent<CharacterControllerComponent>(playerE)->controller->getGhostObject()->setWorldTransform(*rpT);
}

void LuaHelper::execCineSeq(int cine_id)
{
	EntityFactory::get().createCineSeq(cine_id);
}

void LuaHelper::playGoddessVoice(const std::string& voice_wav_name)
{
	SoundSystem::get().playSFX(voice_wav_name, "data/sfx/voices/goddess/"+voice_wav_name+".ogg", "goddess", 1.0f, 1.0f, false);
}

void LuaHelper::disableTutorial(std::string t_name)
{
	//Si el controller esta conectado usamos tutoriales de controller, sino de teclado
	if(CIOStatus::instance()->isPlayer1Connected())
		t_name = t_name + "_c";
	else
		t_name = t_name + "_k";

	UniqueSystem::get().tutorialAppear(EntityManager::get().getEntityWithName(t_name), false);
}

void LuaHelper::enableTutorial(std::string t_name)
{
	//Si el controller esta conectado usamos tutoriales de controller, sino de teclado
	if(CIOStatus::instance()->isPlayer1Connected())
		t_name = t_name + "_c";
	else
		t_name = t_name + "_k";

	UniqueSystem::get().tutorialAppear(EntityManager::get().getEntityWithName(t_name), true);
}

void LuaHelper::enableSubtitle(const std::string& s_name, float time)
{
	CameraSystem::get().activateSubt(s_name, time);
}

void LuaHelper::updatePlayerRespawn(const std::string& rp_name)
{
	Entity* playerE = World::instance()->getPlayer();
	const btTransform *rpT = EntityManager::get().getComponent<PlayerControllerComponent>(playerE)->getRespawn(rp_name.c_str());
	if(rpT == NULL) return;
	EntityManager::get().getComponent<TransformComponent>(playerE)->setInitTransform(*rpT);
}

void LuaHelper::toggleAnimation()
{
	DebugSystem::get().toggleAnimLocked();
}

void LuaHelper::toggleEntity(const std::string& e_name)
{
	Entity* e = EntityManager::get().getEntityWithName(e_name);
	if(!e) return;
	e->enabled = !e->enabled;
}

void LuaHelper::toggleRAutomat()
{
	DebugSystem::get().toggleRenderAutomat();
}

void LuaHelper::toggleRAI()
{
	DebugSystem::get().toggleRenderAI();
}

void LuaHelper::toggleTriggers()
{
	DebugSystem::get().toggleTriggers();
}
