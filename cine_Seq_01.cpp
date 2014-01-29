#include "cine_Seq_01.h"
#include "system_bt.h"
#include "entity_manager.h"
#include "component_automat.h"
#include "world.h"
#include "system_camera.h"
#include "component_particle_effect.h"
#include "entity_factory.h"
#include "component_player_controller.h"
#include "system_unique.h"
#include "system_renderer.h"

CineSeq01::CineSeq01(Entity* entity) : BehaviourTree(entity)
{
	create();

	_started = _logoFirstEnabled = _playerBorn = false;

	//Bloquear player
	World::instance()->setPlayerLocked(true);

	//Inutilizamos mecanicas del player
	EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->_canRecharge = false;

	//Congelamos al player
	World::instance()->getPlayer()->enabled = false;

	//Vida al minimo
	EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->setLife(0.1f);

	//Encolamos las 2 cinematicas iniciales y pausamos
	CameraSystem::get().setCinePaused(true);
	CameraSystem::get().addCamToQueue(1);
	CameraSystem::get().addCamToQueue(2);
}


CineSeq01::~CineSeq01(void)
{
}

void CineSeq01::create()
{
	createRoot("cineSeq01", SEQUENCE, NULL, NULL);
	addChild("cineSeq01", "cineCam1", ACTION, NULL, (btaction)&CineSeq01::cineCam1);
	addChild("cineSeq01", "playerBirth", ACTION, NULL, (btaction)&CineSeq01::playerBirth);
	addChild("cineSeq01", "die", ACTION, NULL, (btaction)&CineSeq01::die);
}

void CineSeq01::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 350, text_color, "cine state: %s", _action.c_str());
}


//ACCIONES
int CineSeq01::cineCam1()
{
	//Poner el alpha del logo al maximo directamente
	if(!CameraSystem::get().getBSAlpha() && !_logoFirstEnabled)
	{
		CameraSystem::get().enableLogo();
		_logoFirstEnabled = true;
	}

	//Esperar a que se pulse enter o espacio
	if((CIOStatus::instance()->becomesPressed(CIOStatus::instance()->TELEPORT) || CIOStatus::instance()->becomesPressed_key(13)) && !_started)
	{
		SoundSystem::get().playStream("intro", "data/music/intro.mp3", "intro", 0, 0.5f, false);

		CameraSystem::get().setCinePaused(false);

		TransformComponent *playerT = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer());

		//Se crea una sombra magica donde nace el player
		Entity * e = EntityFactory::get().createMagicShadow(D3DXVECTOR3(playerT->getPosition()),D3DXVECTOR3(0,1,0),0);
		ShadowComponent * tShadowComp = EntityManager::get().getComponent<ShadowComponent>(e);
		tShadowComp->radius = 2.5f;
		tShadowComp->stopGrowing();
		_started = true;
		CameraSystem::get().disableLogo();
	}

	if(!_started) return STAY;

	//Nos mantenemos aqui mientras dure la cine 1
	if(CameraSystem::get().getActiveCamId() == 1) return STAY;

	//Cuando llegue al final mandamos nacer al player
	World::instance()->getPlayer()->enabled = true; //player activado
	((AUTPlayer*)(EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat()))->prepareForRebirth();

	//Efecto de particulas donde nace el player, cuando acaba el 1er travelling
	TransformComponent *playerT = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer());
	_shadowEffectBirth = EntityFactory::get().createParticleEffect(D3DXVECTOR3(playerT->getPosition()) - D3DXVECTOR3(0,0.6f,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_STATIC);

	return LEAVE;
}

int CineSeq01::playerBirth()
{
	//Nos mantenemos aqui mientras dure la cine 2
	if(CameraSystem::get().getActiveCamId() == 2) return STAY;

	//Cargarse el efecto de particulas pepino
	EntityManager::get().getComponent<ParticleEffectComponent>(_shadowEffectBirth)->stop();

	//Cuando acabe lanzamos audio1 y Tutorial1
	SoundSystem::get().playSFX("come_closer", "data/sfx/voices/goddess/1_come_EDITED.ogg", "goddess", 0.7f, 0.7f, false);
	CameraSystem::get().activateSubt("st1", 3.0f);

	//T1
	UniqueSystem::get().tutorialAppear(EntityManager::get().getEntityWithName("t001"), true);

	//desbloquear player
	World::instance()->setPlayerLocked(false);

	//desbloquear camara 3rd
	CameraSystem::get().setLockCamera3rd(false);

	return LEAVE;
}

#include "system_sound.h"
int CineSeq01::die()
{
//	if(SoundSystem::get().getSoundInfo("come_closer")->isPlaying) return STAY;
	//SoundSystem::SoundInfo * info = SoundSystem::get().getSoundInfo("intro");
	//info->desired_volume /= 2.0f;
	//SoundSystem::get().loadMusicStream("intro", "data/music/flicker.mp3", "intro", 0, 0.3f, true);
	BTSystem::get().addEntityToRemove(_entity);
	return LEAVE;
}


//OTROS
