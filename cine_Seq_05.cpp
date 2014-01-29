#include "cine_Seq_05.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "world.h"
#include "component_bt.h"
#include "component_automat.h"
#include "component_transform.h"

CineSeq05::CineSeq05(Entity* entity) : BehaviourTree(entity)
{
	create();

	//Ejecutar camara 7
	CameraSystem::get().addCamToQueue(7);
	_clock.setTarget(4.5f);
	
	//Animacion de shadow abriendo puerta
	((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat())->changeState("doorCG");

	//Sonido de la puerta abriendose
	const btVector3& doorPos = EntityManager::get().getComponent<TransformComponent>(EntityManager::get().getEntityWithName("door"))->getPosition();
	SoundSystem::get().playSFX3D("opening_door", "data/sfx/open_door.ogg", "door", doorPos, btVector3(0,0,0), false, 0.0f, 1.0f);
}


CineSeq05::~CineSeq05(void)
{
}


void CineSeq05::create()
{
	createRoot("CineSeq05", SEQUENCE, NULL, NULL);
	addChild("CineSeq05", "openDoor", ACTION, NULL, (btaction)&CineSeq05::openDoor);
	addChild("CineSeq05", "fadeOut", ACTION, NULL, (btaction)&CineSeq05::fadeOut);
	addChild("CineSeq05", "aldeaLoaded", ACTION, NULL, (btaction)&CineSeq05::loadNextLevel);
	addChild("CineSeq05", "die", ACTION, NULL, (btaction)&CineSeq05::die);
}

void CineSeq05::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq05::openDoor()
{
	//Nos mantenemos aqui hasta un pelin antes de que acabe la cine 7
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//CameraSystem::get().setCinePaused(true);
	CameraSystem::get().toggleBlackScreen();

	_clock.setTarget(4.0f);
	
	return LEAVE;
}

int CineSeq05::fadeOut()
{
	//Esperamos a que acabe el fade out
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//Audio10
	//SoundSystem::get().stopSound("hide_the_body",false,false);
	SoundSystem::get().stopAllSounds(true);
	SoundSystem::get().playSFX("2nd_chance", "data/sfx/voices/goddess/10_i give_EDITED.ogg", "goddess", 0.7f, 0.7f, false);	
	CameraSystem::get().activateSubt("st9");
	
	//Mandar a cargar el siguiente nivel
	CameraSystem::get().setBlackToLoadingScreen(true);

	_clock.setTarget(1.0f);

	return LEAVE;
}

int CineSeq05::loadNextLevel()
{
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;
	World::instance()->loadScene("aldea");
	CameraSystem::get().deactivateCurrentSubt();
}

int CineSeq05::die()
{
	//if(SoundSystem::get().getSoundInfo("2nd_chance")->isPlaying) return STAY;
	//SoundSystem::get().playSFX("this_village", "data/sfx/voices/goddess/1_come_EDITED.ogg", "goddess", 0.7f, 0.7f, false); //temp
	//CameraSystem::get().activateSubt("st10", 7);
	
	BTSystem::get().addEntityToRemove(_entity);
	return LEAVE;
}


//OTROS
