#include "cine_Seq_04.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "world.h"
#include "component_bt.h"
#include "component_automat.h"
#include "lua_helper.h"

CineSeq04::CineSeq04(Entity* entity) : BehaviourTree(entity)
{
	create();

	_btGoddess = static_cast<BTGoddess*>(EntityManager::get().getComponent<BTComponent>(EntityManager::get().getEntityWithName("goddess_bt"))->getBT());
	
	//Desactivar T4 (crear sombra)
	LuaHelper::get().disableTutorial("t003");

	//Audio 6
	SoundSystem::get().stopSound("sometimes",false,false);
	SoundSystem::get().playSFX("one_guard", "data/sfx/voices/goddess/6_one guard_EDITED.ogg", "goddess", 0.7f, 0.7f, false);
	CameraSystem::get().activateSubt("st6", 5.0f);

	//Tiempo que dura audio6
	_clock.setTarget(7.0f);

	//Encarar diosa al malo
	_btGoddess->setLookat(EntityManager::get().getComponent<TransformComponent>(EntityManager::get().getEntityWithName("txu_g001"))->getPosition());

	//Tutorial 5
	LuaHelper::get().enableTutorial("t005");
	LuaHelper::get().disableTutorial("t004");
}


CineSeq04::~CineSeq04(void)
{
}


void CineSeq04::create()
{
	createRoot("CineSeq04", SEQUENCE, NULL, NULL);
	addChild("CineSeq04", "audio6", ACTION, NULL, (btaction)&CineSeq04::audio6);
	addChild("CineSeq04", "tutorialHideCorpse", ACTION, NULL, (btaction)&CineSeq04::tutorialHideCorpse);
	addChild("CineSeq04", "die", ACTION, NULL, (btaction)&CineSeq04::die);
}

void CineSeq04::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq04::audio6()
{
	if(_clock.hasTarget())
	{
		//Esperamos a que acabe de hablar la diosa
		if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;
		else
		{
			//Cambiar pos de la diosa
			_btGoddess->hasToGo();
		}
	}

	if(EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat()->getState() == "silentMurder")
	{
		//Tiempo para empezar a hablar
		_clock.setTarget(5.0f);
		return LEAVE;
	}
	else return STAY;
}

int CineSeq04::tutorialHideCorpse()
{
	//Esperamos a shadow mate al xu
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//T5 fuera, T7 y T8
	LuaHelper::get().disableTutorial("t005");
	LuaHelper::get().enableTutorial("t007");
	LuaHelper::get().enableTutorial("t006");

	//Audio7
	SoundSystem::get().stopSound("one_guard",false,false);
	SoundSystem::get().playSFX("hide_the_body", "data/sfx/voices/goddess/7_hide the_EDITED.ogg", "goddess", 0.7f, 0.7f, false);
	CameraSystem::get().activateSubt("st7", 3.0f);

	return LEAVE;
}

int CineSeq04::die()
{
	BTSystem::get().addEntityToRemove(_entity);
	return LEAVE;
}


//OTROS
