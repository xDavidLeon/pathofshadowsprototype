#include "cine_Seq_06.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "world.h"
#include "component_bt.h"
#include "component_automat.h"
#include "lua_helper.h"

CineSeq06::CineSeq06(Entity* entity) : BehaviourTree(entity)
{
	create();

	//Ejecutar camara 1
	CameraSystem::get().addCamToQueue(1);
	
	//Poner player en idleCG
	((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat())->setEnableIdleCG(true);

	//situar y bloquear player
	LuaHelper::get().setPlayerPos("rc001");
	World::instance()->setPlayerLocked(true);

	//Actualizar player respawn
	LuaHelper::get().updatePlayerRespawn("r001");
}


CineSeq06::~CineSeq06(void)
{
}


void CineSeq06::create()
{
	createRoot("CineSeq06", SEQUENCE, NULL, NULL);
	addChild("CineSeq06", "camera5", ACTION, NULL, (btaction)&CineSeq06::camera5);
	addChild("CineSeq06", "die", ACTION, NULL, (btaction)&CineSeq06::die);
}

void CineSeq06::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq06::camera5()
{
	if(CameraSystem::get().getActiveCamId() == 1) return STAY;

	return LEAVE;
}

int CineSeq06::die()
{
	World::instance()->setPlayerLocked(false);
	((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat())->setEnableIdleCG(false);
	BTSystem::get().addEntityToRemove(_entity);

	return LEAVE;
}


//OTROS
