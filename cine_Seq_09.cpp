#include "cine_Seq_09.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "world.h"
#include "component_bt.h"
#include "component_automat.h"

CineSeq09::CineSeq09(Entity* entity) : BehaviourTree(entity)
{
	create();

	_event = false;

	//Ejecutar camara 1
	CameraSystem::get().addCamToQueue(1);
	_clock.setTarget(17.0f);

	//Animacion de shadow abriendo puerta
	((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat())->changeState("doorCG");

	//Sonido de la puerta abriendose
	const btVector3& doorPos = EntityManager::get().getComponent<TransformComponent>(EntityManager::get().getEntityWithName("door"))->getPosition();
	SoundSystem::get().playSFX3D("opening_door", "data/sfx/open_door.ogg", "door", doorPos, btVector3(0,0,0), false, 0.0f, 1.0f);
	
	SoundSystem::get().stopSound("patio");
	SoundSystem::get().playStream("credits","data/music/credits.mp3","credits",0.0f,0.75f,false);
}


CineSeq09::~CineSeq09(void)
{
}


void CineSeq09::create()
{
	createRoot("CineSeq09", SEQUENCE, NULL, NULL);
	addChild("CineSeq09", "openDoor", ACTION, NULL, (btaction)&CineSeq09::openDoor);
	addChild("CineSeq09", "fadeOut", ACTION, NULL, (btaction)&CineSeq09::fadeOut);
	addChild("CineSeq09", "die", ACTION, NULL, (btaction)&CineSeq09::die);
}

void CineSeq09::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq09::openDoor()
{
	//Nos mantenemos aqui hasta un pelin antes de que acabe el travelling
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	CameraSystem::get().toggleBlackScreen();

	_clock.setTarget(3.0f);
	
	return LEAVE;
}

int CineSeq09::fadeOut()
{
	//Esperamos a que acabe el fade out
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	SoundSystem::get().stopAllSounds(false);
	SoundSystem::get().playSFX("just_the_beginning", "data/sfx/voices/goddess/14_this is just_EDITED.ogg", "goddess", 1.0f, 1.0f, false);
	CameraSystem::get().activateSubt("st14");
	SoundSystem::get().getSoundInfo("credits")->desired_volume = 0.5f;
	_clock.setTarget(7.0f);

	_event = false;

	return LEAVE;
}

int CineSeq09::die()
{
	if(_clock.getCount() >= 5.0f && !_event)
	{
		//SoundSystem::get().getSoundInfo("credits")->desired_volume = 0.75f;
		World::instance()->enableCredits(true);
		CameraSystem::get().setBlackToLoadingScreen(true);
		_event = true;
		SoundSystem::get().getSoundInfo("credits")->desired_volume = 0.1f;
		CameraSystem::get().deactivateCurrentSubt();
	}

	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;
	

	//Mandar a cargar el siguiente nivel
	World::instance()->loadScene("cementerio");

	BTSystem::get().addEntityToRemove(_entity);
	//g_App.KillD3D();
	//g_App.KillWindow(); //Cerrar aplicacion. Esto no funciona. Ni idea de como hacerlo
	return LEAVE;
}


//OTROS
