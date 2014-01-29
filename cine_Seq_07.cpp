#include "cine_Seq_07.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "world.h"
#include "component_bt.h"
#include "component_automat.h"
#include "lua_helper.h"

CineSeq07::CineSeq07(Entity* entity) : BehaviourTree(entity)
{
	create();

	//Ejecutar camara 1
	CameraSystem::get().addCamToQueue(2);
	
	//Bloquear player
	World::instance()->setPlayerLocked(true);

	//...
	LuaHelper::get().disableTutorial("t007");
	LuaHelper::get().enableTutorial("t013");
}


CineSeq07::~CineSeq07(void)
{
}


void CineSeq07::create()
{
	createRoot("CineSeq07", SEQUENCE, NULL, NULL);
	addChild("CineSeq07", "camera6", ACTION, NULL, (btaction)&CineSeq07::camera6);
	addChild("CineSeq07", "die", ACTION, NULL, (btaction)&CineSeq07::die);
}

void CineSeq07::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq07::camera6()
{
	if(CameraSystem::get().getActiveCamId() == 2) return STAY;
	
	return LEAVE;
}

int CineSeq07::die()
{
	World::instance()->setPlayerLocked(false);
	BTSystem::get().addEntityToRemove(_entity);
	return LEAVE;
}


//OTROS
