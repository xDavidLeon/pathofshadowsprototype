#include "cine_Seq_05_2.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "world.h"
#include "component_bt.h"
#include "component_automat.h"
#include "component_transform.h"

CineSeq05_2::CineSeq05_2(Entity* entity) : BehaviourTree(entity)
{
	create();

	//Esperar por si la frase anterior aun se esta reproduciendo
	_clock.setTarget(4.0f);
}


CineSeq05_2::~CineSeq05_2(void)
{
}


void CineSeq05_2::create()
{
	createRoot("CineSeq05_2", SEQUENCE, NULL, NULL);
	addChild("CineSeq05_2", "waitToTalk", ACTION, NULL, (btaction)&CineSeq05_2::waitToTalk);
	addChild("CineSeq05_2", "die", ACTION, NULL, (btaction)&CineSeq05_2::die);
}

void CineSeq05_2::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq05_2::waitToTalk()
{
	//Nos mantenemos aqui hasta que haya acabado la espera
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//la frase
	SoundSystem::get().playSFX("this_village", "data/sfx/voices/goddess/10_2_this_village_EDITED.ogg", "goddess", 0.7f, 0.7f, false);	
	CameraSystem::get().activateSubt("st10", 9.0f);
	
	return LEAVE;
}

int CineSeq05_2::die()
{
	BTSystem::get().addEntityToRemove(_entity);
	return LEAVE;
}


//OTROS
