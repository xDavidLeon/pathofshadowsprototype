#include "cine_Seq_03.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "world.h"
#include "component_bt.h"
#include "lua_helper.h"

CineSeq03::CineSeq03(Entity* entity) : BehaviourTree(entity)
{
	create();

	_btGoddess = static_cast<BTGoddess*>(EntityManager::get().getComponent<BTComponent>(EntityManager::get().getEntityWithName("goddess_bt"))->getBT());
	
	//Desactivar T3 (teleport)
	LuaHelper::get().disableTutorial("t003");

	//Audio 4
	SoundSystem::get().stopSound("control",false,false);
	SoundSystem::get().playSFX("your_power", "data/sfx/voices/goddess/4_your power will_EDITED.ogg", "goddess", 0.7f, 0.7f, false);
	CameraSystem::get().activateSubt("st4", 10.0f);

	//Tiempo que dura audio4
	_clock.setTarget(11.0f);
}


CineSeq03::~CineSeq03(void)
{
}


void CineSeq03::create()
{
	createRoot("CineSeq03", SEQUENCE, NULL, NULL);
	addChild("CineSeq03", "audio4", ACTION, NULL, (btaction)&CineSeq03::audio4);
	addChild("CineSeq03", "goddessTeleport", ACTION, NULL, (btaction)&CineSeq03::goddessTeleport);
	addChild("CineSeq03", "die", ACTION, NULL, (btaction)&CineSeq03::die);
}

void CineSeq03::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq03::audio4()
{
	//Esperamos a que acabe la explicacion de la diosa
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//Cambiar pos de la diosa
	_btGoddess->changePlace();

	//Tiempo para que empiece a hablar otra vez
	_clock.setTarget(5.0f);
}

int CineSeq03::goddessTeleport()
{
	//Esperamos a que acabe el teleport de la diosa
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//Audio5
	SoundSystem::get().stopSound("your_power",false,false);
	SoundSystem::get().playSFX("sometimes", "data/sfx/voices/goddess/5_sometimes_EDITED.ogg", "goddess", 0.7f, 0.7f, false);
	CameraSystem::get().activateSubt("st5", 7.0f);

	//Tutorial4
	LuaHelper::get().enableTutorial("t004");
}

int CineSeq03::die()
{
	BTSystem::get().addEntityToRemove(_entity);
	return LEAVE;
}


//OTROS
