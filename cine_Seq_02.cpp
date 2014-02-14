#include "cine_Seq_02.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "component_automat.h"
#include "world.h"
#include "system_camera.h"
#include "lua_helper.h"
#include "component_bt.h"
#include "component_model.h"
#include "lua_helper.h"

#include "component_light.h"
#include "component_charcontroller.h"


CineSeq02::CineSeq02(Entity* entity) : BehaviourTree(entity)
{
	create();

	_event = false;

	_btGoddess = static_cast<BTGoddess*>(EntityManager::get().getComponent<BTComponent>(EntityManager::get().getEntityWithName("goddess_bt"))->getBT());
	
	//Poner player en idleCG
	((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat())->setEnableIdleCG(true);

	//Desactivar tutorial 1
	LuaHelper::get().disableTutorial("t001");

	//Encolamos las 3 cinematicas.
	CameraSystem::get().addCamToQueue(3);
	CameraSystem::get().addCamToQueue(4);
	CameraSystem::get().addCamToQueue(5);

	//Resituar player y bloquearlo
	LuaHelper::get().setPlayerPos("rc001");
	World::instance()->setPlayerLocked(true);

	//Tiempo para apagar antorchas
	_clock.setTarget(12.0f);

	//Dar senyal a la diosa para que nazca
	_btGoddess->goddessBorn();

	//sonido volar cuervo
	SoundSystem::get().playSFX("crow_fly_to_place", "data/sfx/ravendiosa.ogg", "goddess", 0.7f, 0.7f, false);
}


CineSeq02::~CineSeq02(void)
{
}


void CineSeq02::create()
{
	createRoot("cineSeq02", SEQUENCE, NULL, NULL);
	addChild("cineSeq02", "lightsOff", ACTION, NULL, (btaction)&CineSeq02::lightsOff);
	addChild("cineSeq02", "goddessAppears", ACTION, NULL, (btaction)&CineSeq02::goddessAppears);
	addChild("cineSeq02", "shadowRecharges", ACTION, NULL, (btaction)&CineSeq02::shadowRecharges);
	addChild("cineSeq02", "goddessGivesPowers", ACTION, NULL, (btaction)&CineSeq02::goddessGivesPowers);
	addChild("cineSeq02", "audio3", ACTION, NULL, (btaction)&CineSeq02::audio3);
	addChild("cineSeq02", "tutorialTeleport", ACTION, NULL, (btaction)&CineSeq02::tutorialTeleport);
	addChild("cineSeq02", "die", ACTION, NULL, (btaction)&CineSeq02::die);
}

void CineSeq02::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq02::lightsOff()
{
	if(_clock.hasTarget())
	{
		if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;
		else
		{
			//sonido apagar antonchas
			SoundSystem::get().playSFX("putoff", "data/sfx/apagar_antorcha.ogg", "goddess", 0.7f, 0.7f, false);

			//Apagar antorcha 25
			LightSystem::get().putOutTorch("point light025", "fire_008", "fire_014", "volume_light022", 1.0f);
			//Apagar antorcha 26
			LightSystem::get().putOutTorch("point light026", "fire_007", "fire_013", "volume_light023", 1.0f);

			//quitar material de "iluminado" de la mesh de las antorchas
			EntityManager::get().getComponent<ModelComponent>(EntityManager::get().getEntityWithName("assets002_"))->deleteMaterial("farol_d");		
		}
	}

	//Nos mantenemos aqui mientras dure la cine 3 (vuelo de cuervo y transformacion)
	if(CameraSystem::get().getActiveCamId() == 3) return STAY;

	//espera para audio 2
	_clock.setTarget(1.0f);

	return LEAVE;
}

int CineSeq02::goddessAppears()
{
	//Aqui ya empieza la camara 4. Esperamos un poco para que la diosa empieze a hablar
	if(_clock.hasTarget())
	{
		if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;
		else
		{
			SoundSystem::SoundInfo * sound = SoundSystem::get().getSoundInfo("intro");
			if (sound != NULL) sound->desired_volume = 0.25f;
			//Audio2
			SoundSystem::get().stopSound("come_closer",false,false);
			SoundSystem::get().playSFX("reincarnated", "data/sfx/voices/goddess/2_rencarnated_EDITED.ogg", "goddess", 0.7f, 0.7f, false);
			CameraSystem::get().activateSubt("st2", 9.0f);
		}
	}

	//Nos mantenemos aqui mientras dure la cine 4 (palabras de la diosa)
	if(CameraSystem::get().getActiveCamId() == 4) return STAY;

	_clock.setTarget(1.0f);

	return LEAVE;
}

int CineSeq02::shadowRecharges()
{
	//Esperamos a que acabe la explicacion de la diosa
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//Accion de shadow recargandose
	((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat())->changeState("crouchRecharge");

	//Accion de diosa entregando poderes
	_btGoddess->givePowersToPlayer();

	//tiempo de duracion de entrega de poderes
	_clock.setTarget(3.0f);

	return LEAVE;
}

int CineSeq02::goddessGivesPowers()
{
	//
	if(_clock.hasTarget())
	{
		if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;
		else
		{
			//Lanzar audio 3
			SoundSystem::get().stopSound("reincarnated",false,false);
			SoundSystem::get().playSFX("control", "data/sfx/voices/goddess/3_you can now control_EDITED.ogg", "goddess", 0.7f, 0.7f, false);
			CameraSystem::get().activateSubt("st3", 6.0f);
		}
	}

	//Nos mantenemos aqui mientras dure la cine 5
	if(CameraSystem::get().getActiveCamId() == 5) return STAY;

	//cambiar estado player
	((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat())->setEnableIdleCG(false);

	//desbloquear player
	World::instance()->setPlayerLocked(false);

	//activar recarga de poderes automatica
	EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->_canRecharge = true;	

	//timer audio 3
	_clock.setTarget(3.0f);

	return LEAVE;
}

int CineSeq02::audio3()
{
	if(!_clock.count(World::instance()->getElapsedTimeUInSeconds())) return STAY;

	//Cambiar pos de la diosa
	_btGoddess->changePlace();

	//Tutorial2 (aim)
	LuaHelper::get().enableTutorial("t002");

	return LEAVE;
}

int CineSeq02::tutorialTeleport()
{
	if(CIOStatus::instance()->becomesPressed(CIOStatus::instance()->AIM))
	{
		//quitar T2, meter T3
		LuaHelper::get().disableTutorial("t002");
		LuaHelper::get().enableTutorial("t003");
	}
	else if(CIOStatus::instance()->becomesReleased(CIOStatus::instance()->AIM))
	{
		//meter T2, quitar T3
		LuaHelper::get().enableTutorial("t002");
		LuaHelper::get().disableTutorial("t003");
	}

	if(CIOStatus::instance()->isPressed(CIOStatus::instance()->AIM) && CIOStatus::instance()->becomesPressed(CIOStatus::instance()->TELEPORT))
	{
		return LEAVE;
	}
	else return STAY;
}


int CineSeq02::die()
{
	BTSystem::get().addEntityToRemove(_entity);
	return LEAVE;
}


//OTROS
