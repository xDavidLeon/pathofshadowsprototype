#include "cine_Seq_10.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "world.h"
#include "component_bt.h"
#include "component_automat.h"
#include "logic_manager.h"

CineSeq10::CineSeq10(Entity* entity) : BehaviourTree(entity)
{
	create();

	//Desactivar el 1er trigger
	EntityManager::get().getEntityWithName("trigger001")->enabled = false;

	_event = false;

	//Desactivar player
	World::instance()->getPlayer()->enabled = false;

	//ToDo:Desactivar fisica del mundo

	//Ejecutar camaras de la 8 a la 17
	CameraSystem::get().addCamToQueue(8);
	CameraSystem::get().addCamToQueue(9);
	CameraSystem::get().addCamToQueue(10);
	CameraSystem::get().addCamToQueue(11);
	CameraSystem::get().addCamToQueue(12);
	CameraSystem::get().addCamToQueue(13);
	CameraSystem::get().addCamToQueue(14);
	CameraSystem::get().addCamToQueue(15);
	CameraSystem::get().addCamToQueue(16);
	CameraSystem::get().addCamToQueue(17);
	//CameraSystem::get().addCamToQueue(18);
	//CameraSystem::get().addCamToQueue(19);
	//CameraSystem::get().addCamToQueue(20);
	//CameraSystem::get().addCamToQueue(21);
	//CameraSystem::get().addCamToQueue(22);

	//Reproducir musica
	//Ya deberia venir reproduciendose desde patio
	SoundSystem::get().playStream("credits","data/music/credits.mp3","credits",0.0f,0.75f,false);

	World::instance()->enableCredits(false);
}


CineSeq10::~CineSeq10(void)
{
}


void CineSeq10::create()
{
	createRoot("CineSeq10", SEQUENCE, NULL, NULL);
	addChild("CineSeq10", "namesOnStones", ACTION, NULL, (btaction)&CineSeq10::namesOnStones);
	addChild("CineSeq10", "finalCredits", ACTION, NULL, (btaction)&CineSeq10::finalCredits);
	addChild("CineSeq10", "die", ACTION, NULL, (btaction)&CineSeq10::die);
}

void CineSeq10::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq10::namesOnStones()
{
	//Mientras no comience el ultimo travelling, esperamos
	if(CameraSystem::get().getActiveCamId() != 17) return STAY;
	
	//Quitamos barras de cine e impedimos que se activen desde CameraSystem
	CameraSystem::get().toggleBars();
	CameraSystem::get().setBarsLocked(true);

	//Pedimos que al acabar el travelling actual se pause la camara cinematica (para que no haya un salto entre la ultima y la 1a)
	CameraSystem::get().pauseOnEnd();

	//Activar créditos en scrolling
	CameraSystem::get().activateCredits();

	return LEAVE;
}

int CineSeq10::finalCredits()
{
	//Esperamos a que acabe el ultimo travelling
	if(CameraSystem::get().getActiveCamId() == 17) return STAY;

	//Te estare esperando...
	SoundSystem::get().playStream("ill_be_waiting","data/sfx/voices/goddess/15_ill be waiting_EDITED.ogg","goddess",0.7f,0.7f,false);
	CameraSystem::get().activateSubt("st15", 3.0f);
	
	//Descongelamos las barras
	CameraSystem::get().setBarsLocked(false);
	
	//Ejecutar acción de trigger 001
	LogicManager::get().runScript("trigger001()");
	
	return LEAVE;
}

int CineSeq10::die()
{
	BTSystem::get().addEntityToRemove(_entity);
	return LEAVE;
}


//OTROS
