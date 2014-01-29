#include "cine_Seq_08.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "world.h"
#include "component_bt.h"
#include "component_automat.h"

CineSeq08::CineSeq08(Entity* entity) : BehaviourTree(entity)
{
	create();

	//Ejecutar camara 3
	CameraSystem::get().addCamToQueue(3);
	_clock.setTarget(4.0f);
	
	//Animacion de shadow abriendo puerta
	((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat())->changeState("doorCG");

	//Sonido de la puerta abriendose
	const btVector3& doorPos = EntityManager::get().getComponent<TransformComponent>(EntityManager::get().getEntityWithName("door"))->getPosition();
	SoundSystem::get().playSFX3D("opening_door", "data/sfx/open_door.ogg", "door", doorPos, btVector3(0,0,0), false, 0.0f, 1.0f);
}


CineSeq08::~CineSeq08(void)
{
}


void CineSeq08::create()
{
	createRoot("CineSeq08", SEQUENCE, NULL, NULL);
	addChild("CineSeq08", "openDoor", ACTION, NULL, (btaction)&CineSeq08::openDoor);
	addChild("CineSeq08", "fadeOut", ACTION, NULL, (btaction)&CineSeq08::fadeOut);
	addChild("CineSeq08", "die", ACTION, NULL, (btaction)&CineSeq08::die);
}

void CineSeq08::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq08::openDoor()
{
	//Nos mantenemos aqui hasta un pelin antes de que acabe la cine 3
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	CameraSystem::get().toggleBlackScreen();

	_clock.setTarget(4.0f);
	
	return LEAVE;
}

int CineSeq08::fadeOut()
{
	//Esperamos a que acabe el fade out
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//Audio
	SoundSystem::get().stopAllSounds(true);
	SoundSystem::get().playSFX("last_war", "data/sfx/voices/goddess/13_the last_EDITED.ogg", "goddess", 0.7f, 0.7f, false);
	CameraSystem::get().activateSubt("st13");

	CameraSystem::get().setBlackToLoadingScreen(true);

	//Una pequenya espera, a ver si suena siempre la voz de diosa
	_clock.setTarget(1.0f);

	return LEAVE;
}

int CineSeq08::die()
{
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//Mandar a cargar el siguiente nivel
	World::instance()->loadScene("patio");
	CameraSystem::get().deactivateCurrentSubt();

	BTSystem::get().addEntityToRemove(_entity);
	return LEAVE;
}


//OTROS
