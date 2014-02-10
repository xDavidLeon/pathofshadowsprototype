#include "aut_player.h"
#include "globals.h"
#include "world.h"
#include "d3ddefs.h"
#include "iostatus.h"
#include "component_charcontroller.h"
#include "component_enemy_data.h"
#include "component_bt.h"
#include "component_model.h"
#include "system_camera.h"
#include "camera_controller_3rd.h"
#include "component_bt.h"
#include "bt_common.h"
#include "entity_factory.h"
#include "system_debug.h"
#include "component_animation.h"
#include "system_bt.h"
#include "component_enemy_data.h"
#include "entity_manager.h"
#include "mesh_manager.h"
#include "system_playercontroller.h"
#include "system_light.h"
#include "vision_interface.h"

#include "dijkstra.h" //para pruebas
#include <deque>


AUTPlayer::AUTPlayer(Entity* entity) : Automat(entity)
{
	_ios = CIOStatus::instance();

	_lastVictim = NULL;
	_playerContC = new PlayerControllerComponent(entity);
	EntityManager::get().addComponent(_playerContC, entity);

	_lockedInPlace = false;
	_enableIdleCG = false;
	_transformC = EntityManager::get().getComponent<TransformComponent>(entity);
	_shadowAcComp = new ShadowActionsComponent(entity);
	EntityManager::get().addComponent(_shadowAcComp,entity);
	_animation_component = EntityManager::get().getComponent<AnimationComponent>(entity);
	assert( _animation_component );
	_telepGround = true;
	_target = NULL;
	//Distancia a la que es posible matar silenciosamente (en manhattan dist.)
	_silentKillDistMh = 2.5f;
	_panicKillDistMh = 3.5f;
	_silentKillVictim = NULL;
	_victimType = NORMAL;
	_frame_catched = false;
	ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(_entity);
		Entity * particle = EntityFactory::get().createParticleEffect(D3DXVECTOR3(0,0,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_HAND,m);
		_particleShadowHand = EntityManager::get().getComponent<ParticleEffectComponent>(particle);
		particle->name = "shadows_hand";
		_particleShadowHand->stop();

	////************************ BUFANDA ******************//
	//_bufanda = EntityManager::get().createEntity();
	//_bufanda->name = "shadow_blend";
	//_bufanda->type = "BUFANDA";
	//EntityManager::get().addEntityByName(_bufanda);
	//ModelComponent* model = new ModelComponent(_bufanda);
	//EntityManager::get().addComponent(model,_bufanda);
	//
	//btTransform bt_trans;
	//bt_trans.setIdentity();
	//TransformComponent* transform = new TransformComponent(_bufanda,bt_trans);
	//EntityManager::get().addComponent(transform,_bufanda);
	//
	//AnimationComponent* animComp = new AnimationComponent(_bufanda, _bufanda->name);
	//EntityManager::get().addComponent(animComp, _bufanda);
	//
	//model->setCModel(animComp->getModel());
	//TMesh* mesh = TMeshManager::get( ).getMesh( _bufanda->name ); 
	//model->setMesh(mesh);

	//TMaterial * material = new TMaterial();
	//material->diffuse = TTextureManager::get( ).getTexture("hardcoded/shadow_d");
	//material->emissive = TTextureManager::get( ).getTexture("hardcoded/shadow_i");
	//material->mask = TTextureManager::get( ).getTexture("hardcoded/shadow_bufanda");
	//material->main_texture = material->diffuse;
	//material->name = "tech_shadow_scarf";
	//material->lightmap = NULL;
	//material->specular = NULL;
	//material->bumpmap = NULL;
	//material->normalmap = NULL;

	//model->addMaterial(material);

	//model->enabled = false;
	////************************ BUFANDA ******************//	

	//************************ SNAKE KILL ******************//
	_snake = EntityManager::get().createEntity();
	_snake->name = "shadow_snake";
	_snake->type = "SNAKE";
	EntityManager::get().addEntityByName(_snake);
	ModelComponent* model = new ModelComponent(_snake);
	EntityManager::get().addComponent(model,_snake);
	
	btTransform bt_trans;
	bt_trans.setIdentity();
	TransformComponent* transform = new TransformComponent(_snake,bt_trans);
	transform->setPosition(_transformC->getPosition());
	EntityManager::get().addComponent(transform,_snake);
	
	AnimationComponent* animComp = new AnimationComponent(_snake, _snake->name);
	EntityManager::get().addComponent(animComp, _snake);
	
	model->setCModel(animComp->getModel());
	TMesh* mesh = TMeshManager::get( ).getMesh( _snake->name ); 
	model->setMesh(mesh);
	model->render_flags["no_cull"] = true;
	model->render_flags["forward"] = true;

	TMaterial * material = new TMaterial();
	material->diffuse = TTextureManager::get( ).getTexture("hardcoded/shadow_snake");
	material->emissive = NULL;
	material->mask = NULL;
	material->main_texture = material->diffuse;
	material->name = "tech_fwd_skin";
	material->lightmap = NULL;
	material->specular = NULL;
	material->bumpmap = NULL;
	material->normalmap = NULL;
	model->addMaterial(material);

	model->enabled = false;



	//************************ SNAKE TELEPORT ******************//
	_stele = EntityManager::get().createEntity();
	_stele->name = "shadow_stele";
	_stele->type = "SNAKE";
	EntityManager::get().addEntityByName(_stele);
	model = new ModelComponent(_stele);
	EntityManager::get().addComponent(model,_stele);
	
	bt_trans;
	bt_trans.setIdentity();
	transform = new TransformComponent(_stele,bt_trans);
	transform->setPosition(_transformC->getPosition());
	EntityManager::get().addComponent(transform,_stele);
	
	animComp = new AnimationComponent(_stele, _stele->name);
	EntityManager::get().addComponent(animComp, _stele);
	
	model->setCModel(animComp->getModel());
	mesh = TMeshManager::get( ).getMesh( _stele->name ); 
	model->setMesh(mesh);
	model->render_flags["no_cull"] = true;
	model->render_flags["forward"] = true;

	material = new TMaterial();
	material->diffuse = TTextureManager::get( ).getTexture("hardcoded/shadow_snake");
	material->emissive = NULL;
	material->mask = NULL;
	material->main_texture = material->diffuse;
	material->name = "tech_fwd_skin";
	material->lightmap = NULL;
	material->specular = NULL;
	material->bumpmap = NULL;
	material->normalmap = NULL;
	model->addMaterial(material);

	model->enabled = false;


	// SFX
	_sound_channel = SoundSystem::get().playSFX3D("steps" + entity->eid,"data/sfx/grass_walk.wav", "steps" + entity->eid, transform->getPosition(), btVector3(0,0,0), true, 0.0f, 0.0f);
	_sound_channel->grow_factor = 3.0f;
	_sound_channel->tag = "grass";
	_sound_channel->tag2 = "walk";
	_sound_channel2 = SoundSystem::get().playSFX3D("steps2" + entity->eid,"data/sfx/grass_run.wav", "steps2" + entity->eid, transform->getPosition(), btVector3(0,0,0), true, 0.0f, 0.0f);
	_sound_channel2->grow_factor = 3.0f;
	_sound_channel2->tag = "grass";
	_sound_channel2->tag2 = "run";
	_sound_channel_current = _sound_channel;
}

AUTPlayer::~AUTPlayer()
{
	_sound_channel_current->desired_volume = 0.0f;
	_sound_channel->desired_volume = 0.0f;
	_sound_channel2->desired_volume = 0.0f;
	_ios = NULL;
	_playerContC = NULL;
	_transformC = NULL;
	_shadowAcComp = NULL;
	_animation_component = NULL;
	_silentKillVictim = NULL;
	_target = NULL;
	_retarget.setTarget(1.0f);
}

void AUTPlayer::init()
{
	//cine
	addState("reborn",(statehandler)&AUTPlayer::reborn);
	addState("idleCG",(statehandler)&AUTPlayer::idleCG);
	addState("doorCG",(statehandler)&AUTPlayer::doorCG);
	addState("crouchRecharge",(statehandler)&AUTPlayer::crouchRecharge);
	addState("recharging",(statehandler)&AUTPlayer::recharging);
	addState("riseRecharge",(statehandler)&AUTPlayer::riseRecharge);

	//basic state
	addState("idleBasic",(statehandler)&AUTPlayer::idleBasic);
	addState("idleInactive",(statehandler)&AUTPlayer::idleInactive);
	addState("walk",(statehandler)&AUTPlayer::walk);
	addState("run",(statehandler)&AUTPlayer::run);
	addState("accelerate",(statehandler)&AUTPlayer::accelerate);
	addState("brake",(statehandler)&AUTPlayer::brake);
	addState("silentMurder",(statehandler)&AUTPlayer::silentMurder);
	addState("decoy",(statehandler)&AUTPlayer::decoy);
	addState("leavingShadowWall",(statehandler)&AUTPlayer::leavingShadowWall);
	addState("crouchSVision",(statehandler)&AUTPlayer::crouchSVision);
	addState("specialVision",(statehandler)&AUTPlayer::specialVision);
	addState("riseSVision",(statehandler)&AUTPlayer::riseSVision);
	addState("summonCrow",(statehandler)&AUTPlayer::summonCrow);

	//shadow state
	addState("idleShadow",(statehandler)&AUTPlayer::idleShadow);
	addState("movingShadow",(statehandler)&AUTPlayer::movingShadow);
	addState("creatingShadow",(statehandler)&AUTPlayer::creatingShadow);
	addState("stopCreateShadow",(statehandler)&AUTPlayer::stopCreateShadow);
	addState("accelerateTeleport",(statehandler)&AUTPlayer::accelerateTeleport);
	addState("teleporting",(statehandler)&AUTPlayer::teleporting);
	addState("brakeTeleport",(statehandler)&AUTPlayer::brakeTeleport);

	//others
	addState("falling",(statehandler)&AUTPlayer::falling);
	addState("landing",(statehandler)&AUTPlayer::landing);
	addState("grounding",(statehandler)&AUTPlayer::grounding);
	addState("dying",(statehandler)&AUTPlayer::dying);
	addState("idleKill",(statehandler)&AUTPlayer::idleKill);
	addState("walkKill",(statehandler)&AUTPlayer::walkKill);
	addState("blendedWall",(statehandler)&AUTPlayer::blendedWall);
	

	//Que empiece o no en reborn depende del primer trigger
	changeState("idleBasic");

	_animation_component->blendCycle("idle", 1.0f, 0.0f);
	//_animation_component->executeAction("reborn", 0.0f, 0.5f);
	computeRandomWpt(_state);
	_animation_component->blendLookAtIn( 5.0f );

	_retarget.setTarget(0.75f);
	_inactiveTimeTrigger = 10.0f;
	_inactiveAnimationTime = _animation_component->getDuration("idle_inactive");
	_inactive.setTarget(_inactiveTimeTrigger);
}

void AUTPlayer::updateSteps()
{
	SoundSystem::get().set3DPosition(_sound_channel,_transformC->getPosition());
	SoundSystem::get().set3DPosition(_sound_channel2,_transformC->getPosition());
	
	std::string ground = SoundSystem::get().getGroundSound(_transformC);
	if (ground == "" || ground == "NULL") return;
	std::string tag2 = "";
	if ( _state.compare("run") == 0) tag2 = "run";
	else tag2 = "walk";

	if (ground != _sound_channel_current->tag || tag2 != _sound_channel_current->tag2)
	{
		SoundSystem::SoundInfo * otherChannel = _sound_channel;
		if (_sound_channel_current == _sound_channel) otherChannel = _sound_channel2;

		otherChannel->tag = ground;
		otherChannel->tag2 = tag2;
		
		if (ground == "grass")
		{
			if ( _state.compare("run") == 0) SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/grass_run.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
			else SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/grass_walk.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
		}
		else if (ground == "gravel")
		{
			if ( _state.compare("run") == 0) SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/gravel_run.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
			else SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/gravel_walk.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
		}
		else if (ground == "outside")
		{
			if ( _state.compare("run") == 0) SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/outside_run.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
			else SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/outside_walk.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
		}
		else if (ground == "stone")
		{
			if ( _state.compare("run") == 0) SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/stone_run.ogg",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
			else SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/stone_walk.ogg",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
		}
		else if (ground == "wood")
		{
			if ( _state.compare("run") == 0) SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/wood_run.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
			else SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/wood_walk.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
		}
		else if (ground == "picon")
		{
			if ( _state.compare("run") == 0) SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/picon_run.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
			else SoundSystem::get().playSFX3D(otherChannel->id,"data/sfx/picon_walk.wav",otherChannel->channel_name,_transformC->getPosition(),btVector3(0,0,0),true,_sound_channel_current->volume,_sound_channel_current->desired_volume);
		}
		SoundSystem::get().stopSound(_sound_channel_current->id,false,false);
		_sound_channel_current = otherChannel;
	}
}

void AUTPlayer::update(float delta)
{
	_playerContC->_noiseDistSq = 0.0f;
	Automat::update(delta);
	updateAnimations(delta);
	updateSteps();
}

void AUTPlayer::updateAnimations( float delta )
{
	if(_state.compare("idleBasic") == 0 || _state.compare("idleInactive") == 0 || _state.compare("walk") == 0 || _state.compare("run") == 0 || _state.compare("idleKill") == 0 || _state.compare("walkKill") == 0 )
	{
		headController(delta);
	}
}

void AUTPlayer::updateTransitionAnimations(std::string state)
{
	float speed = _ios->left.normalized_magnitude;

	#ifdef EDU_DBG
		//dbg("%s\n",state.c_str());
	#endif

	// Para el tiempo de inactividad
	if(state.compare("idleBasic") == 0)
	{
		float variancia = _inactiveTimeTrigger * 0.25f;
		_inactive.setTarget(randomFloat( _inactiveTimeTrigger - variancia, _inactiveTimeTrigger + variancia ) );
	}
	else if(state.compare("reborn") == 0)
	{
		_animation_component->clearCycles(0);
		_animation_component->getModel()->update(0);
		_animation_component->blendLookAtOut( 0.0f );
	}
	/*else if(state.compare("dying") == 0)
	{
		CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(_entity);
		ccC->deactivateCollisions();
	}*/
	else if(state.compare("doorCG") == 0)
	{
		stopMovement();

		Entity* door = EntityManager::get().getEntityWithName("door");

		TransformComponent* transformC = EntityManager::get().getComponent<TransformComponent>(_entity);
		TransformComponent* transformCDoor = EntityManager::get().getComponent<TransformComponent>(door);

		(*transformC->transform) = (*transformCDoor->transform);
		transformC->moveLocal(btVector3(-0.2f,0,-3.7));

		_animation_component->blendLookAtOut(0);
		_animation_component->blendAimOut(0);
		_animation_component->removeActions(0);
		_animation_component->clearCycles(0);
		_animation_component->executeAction("open", 0, 0.5f);
		_animation_component->blendCycle("idle_cg", 1.0f, 0.5f);
		EntityManager::get().getComponent<AnimationComponent>(door)->executeAction("open", 0.0f, 0.0f, 1.0f, true);


		CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(_entity);
		ccC->deactivateCollisions();

	}
	else if(state.compare("idleCG") == 0)
	{
		_animation_component->blendLookAtOut( 0.0f );
		_animation_component->clearCycles( 0.0f );
		_animation_component->blendCycle( "idle_cg", 1.0f, 0.0f); // CONSERVAMOS ESTE CICLO PARA SALIR DESDE ABAJO
	}


	// TRANSICIONES
	//comprobar de que estado venimos y a donde vamos para clear y blend cycles
	if(_state.compare("idleBasic") == 0 )
	{
		stopMovement();
	
		if(state.compare("idleInactive") == 0)
		{
			_animation_component->blendLookAtOut( 0.1f );
			_animation_component->clearCycle( "idle", 1.5f );
			_animation_component->blendCycle( "idle_inactive", 1.0f, 1.5f );
			computeRandomWpt(state);
		}
		if(state.compare("idleKill") == 0)
		{
			_animation_component->blendLookAtOut( 0.1f );
			_animation_component->clearCycle( "idle", 1.5f );
			_animation_component->blendCycle( "idle_kill", 1.0f, 1.5f );
		}
		else if(state.compare("walk") == 0)
		{
			_animation_component->clearCycle( "idle", 0.5f );
			_animation_component->blendCycle( "walk", 1.0f, 0.5f );
			computeRandomWpt(state);
		}
		else if(state.compare("accelerate") == 0)
		{
			_animation_component->clearCycle( "idle", 0.1f );
			_animation_component->executeAction("accelerate", 0.1f, 0.0f, 1.0f, true);
			_animation_component->blendLookAtOut( 0.1f );
			
		}
		else if(state.compare("idleShadow") == 0)
		{
			_animation_component->clearCycle( "idle", 0.5f );
			_animation_component->blendLookAtOut( 0.5f );
			//_animation_component->executeAction( "aim", 0.5f, 0.5f );
			_animation_component->blendAimIn( 0.5f );
			_animation_component->blendCycle( "aimming", 1.0f, 0.5f );
			_animation_component->blendCycle( "idle_legs", 1.0f, 0.2f );
		}
		else if(state.compare("decoy") == 0)
		{
			/*_animation_component->blendLookAtOut( 0.5f );*/
			_animation_component->executeAction( "decoy", 0.5f, 0.5f);
		}
		else if(state.compare("silentMurder") == 0)
		{
			setSilentKill();
		}
		else if(state.compare("crouchSVision") == 0)
		{
			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "vision", 0.5f, 0.5f, 1.0f, true);
			_animation_component->clearCycle( "idle", 0.5f );
		}
		else if(state.compare("summonCrow") == 0)
		{
			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "summon", 0.5f, 0.5f);
		}
	}
	else if(_state.compare("idleCG") == 0)
	{
		stopMovement();
	
		if(state.compare("idleBasic") == 0)
		{
			_animation_component->blendLookAtIn( 5.0f );
			_animation_component->clearCycles( 1.0f );
			_animation_component->blendCycle( "idle", 1.0f, 1.0f );
			computeRandomWpt(state);
		}

		else if(state.compare("crouchRecharge") == 0)
		{
			_animation_component->blendLookAtOut( 0.2f );
			_animation_component->executeAction( "recharge", 0.5f, 0.5f );

			SoundSystem::get().playSFX("recharge", "data/sfx/recargar_shadow.ogg", "shadow_sfx", 0.7f, 0.7f, false);
		}
	}
	else if(_state.compare("idleInactive") == 0)
	{
		stopMovement();
	
		if(state.compare("idleBasic") == 0)
		{
			_animation_component->blendLookAtIn( 5.0f );
			_animation_component->clearCycle( "idle_inactive", 2.5f );
			_animation_component->blendCycle( "idle", 1.0f, 2.5f );
			computeRandomWpt(state);
		}
		if(state.compare("idleKill") == 0)
		{
			_animation_component->blendLookAtIn( 5.0f );
			_animation_component->blendCycle( "idle_kill", 1.0f, 0.5f);
			_animation_component->clearCycle( "idle_inactive", 0.5f );
		}
		else if(state.compare("walk") == 0)
		{
			_animation_component->blendLookAtIn( 5.0f );
			_animation_component->clearCycle( "idle_inactive", 0.5f );
			_animation_component->blendCycle( "walk", 1.0f, 0.5f );
			computeRandomWpt(state);
		}
		else if(state.compare("accelerate") == 0)
		{
			_animation_component->clearCycle( "idle_inactive", 0.1f );
			_animation_component->executeAction("accelerate", 0.1f, 0.0f, 1.0f, true);
			_animation_component->blendLookAtOut( 0.1f );
			
		}
		else if(state.compare("idleShadow") == 0)
		{
			_animation_component->clearCycle( "idle_inactive", 0.5f );
			_animation_component->blendLookAtOut( 0.5f );
			//_animation_component->executeAction( "aim", 0.5f, 0.5f );
			_animation_component->blendAimIn( 0.5f );
			_animation_component->blendCycle( "aimming", 1.0f, 0.5f );
			_animation_component->blendCycle( "idle_legs", 1.0f, 0.2f );
		}
		else if(state.compare("decoy") == 0)
		{
			_animation_component->clearCycle( "idle_inactive", 0.5f );
			_animation_component->blendCycle( "idle", 1.0f, 0.5f );

			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "decoy", 0.5f, 0.5f);
		}
		else if(state.compare("silentMurder") == 0)
		{
			_animation_component->clearCycle( "idle_inactive", 0.5f );
			_animation_component->blendCycle( "idle", 1.0f, 0.5f);

			setSilentKill();
		}
		else if(state.compare("crouchSVision") == 0)
		{
			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "vision", 0.5f, 0.5f, 1.0f, true);
			_animation_component->clearCycle( "idle_inactive", 0.5f );
		}
		else if(state.compare("summonCrow") == 0)
		{
			_animation_component->clearCycle( "idle_inactive", 0.5f );
			_animation_component->blendCycle( "idle", 1.0f, 0.5f );
			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "summon", 0.5f, 0.5f);
		}
	}
	else if(_state.compare("idleKill") == 0)
	{
		stopMovement();
	
		if(state.compare("idleBasic") == 0)
		{
			_animation_component->blendLookAtIn( 2.0f );
			_animation_component->clearCycle( "idle_kill", 1.5f );
			_animation_component->blendCycle( "idle", 1.0f, 1.5f );
			computeRandomWpt(state);
		}
		else if(state.compare("walkKill") == 0)
		{
			_animation_component->clearCycle( "idle_kill", 0.5f );
			_animation_component->blendCycle( "walk_kill", 1.0f, 0.5f );
		}
		else if(state.compare("accelerate") == 0)
		{
			_animation_component->clearCycle( "idle_kill", 0.1f );
			_animation_component->executeAction("accelerate", 0.1f, 0.0f, 1.0f, true);
			_animation_component->blendLookAtOut( 0.1f );
			
		}
		else if(state.compare("idleShadow") == 0)
		{
			_animation_component->clearCycle( "idle_kill", 0.5f );
			_animation_component->blendLookAtOut( 0.5f );
			//_animation_component->executeAction( "aim", 0.5f, 0.5f );
			_animation_component->blendAimIn( 0.5f );
			_animation_component->blendCycle( "aimming", 1.0f, 0.5f );
			_animation_component->blendCycle( "idle_legs", 1.0f, 0.2f );
		}
		else if(state.compare("decoy") == 0)
		{
			_animation_component->clearCycle( "idle_kill", 0.5f );
			_animation_component->blendCycle( "idle", 1.0f, 0.5f );

			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "decoy", 0.5f, 0.5f);
		}
		else if(state.compare("silentMurder") == 0)
		{
			_animation_component->clearCycle( "idle_kill", 0.5f );
			_animation_component->blendCycle( "idle", 1.0f, 0.5f);

			setSilentKill();
		}
		else if(state.compare("crouchSVision") == 0)
		{
			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "vision", 0.5f, 0.5f, 1.0f, true);
			_animation_component->clearCycle( "idle_kill", 0.5f );
		}
		else if(state.compare("summonCrow") == 0)
		{
			_animation_component->clearCycle( "idle_kill", 0.5f );
			_animation_component->blendCycle( "idle", 1.0f, 0.5f );
			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "summon", 0.5f, 0.5f);
		}
	}
	else if(_state.compare("walk") == 0)
	{
	
		// si dejamos de movernos quitamos el sonido de los pasos
		//FMOD_Channel_SetVolume(_channel_steps, 0.0f);
		//sound->blendStepsVolume(0.0f, 0.2f);
		_sound_channel_current->desired_volume = 0.0f;
		((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setCameraFov(0.0f);
		
		if(state.compare("idleBasic") == 0)
		{
			_animation_component->blendCycle( "idle", 1.0f, 0.5f);
			_animation_component->clearCycle( "walk", 0.5f );
			computeRandomWpt(state);
		}
		else if(state.compare("walkKill") == 0)
		{
			_animation_component->clearCycle( "walk", 0.5f );
			_animation_component->blendCycle( "walk_kill", 1.0f, 0.5f );
		}
		else if(state.compare("silentMurder") == 0)
		{
			stopMovement();
	
			_animation_component->blendCycle( "idle", 1.0f, 0.5f);
			_animation_component->clearCycle( "walk", 0.5f );

			setSilentKill();
		}
		else if(state.compare("decoy") == 0)
		{
			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "decoy", 0.5f, 0.5f);
		}
		else if(state.compare("movingShadow") == 0)
		{
			_animation_component->blendCycle( "idle_legs", 1.0f, 0.2f ); // descomentar mentre no fem un walk_legs correcte
			
			_animation_component->clearCycle( "walk", 0.5f);
			_animation_component->blendCycle( "walk_legs", 1.0f, 0.1f);
			_animation_component->blendLookAtOut( 0.5f );
			//_animation_component->executeAction( "aim", 0.5f, 0.5f, 1.0f );
			_animation_component->blendAimIn( 0.5f );
			_animation_component->blendCycle( "aimming", 1.0f, 0.5f );
		}
		else if(state.compare("accelerate") == 0)
		{
			_animation_component->clearCycle("walk", 0.1f);
			_animation_component->executeAction( "accelerate", 0.1f, 0.0f, 1.0f, true );
			_animation_component->blendLookAtOut( 0.1f );
		}
	}
	else if(_state.compare("walkKill") == 0)
	{
	
		// si dejamos de movernos quitamos el sonido de los pasos
		_sound_channel_current->desired_volume = 0.0f;
		//FMOD_Channel_SetVolume(_channel_steps, 0.0f);
		//sound->blendStepsVolume(0.0f, 0.2f);
		((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setCameraFov(0.0f);
		
		if(state.compare("idleKill") == 0)
		{
			_animation_component->blendCycle( "idle_kill", 1.0f, 0.5f);
			_animation_component->clearCycle( "walk_kill", 0.5f );
		}
		else if(state.compare("walk") == 0)
		{
			_animation_component->clearCycle( "walk_kill", 0.5f );
			_animation_component->blendCycle( "walk", 1.0f, 0.5f );
			computeRandomWpt(state);
		}
		else if(state.compare("silentMurder") == 0)
		{
			stopMovement();
	
			_animation_component->blendCycle( "idle", 1.0f, 0.5f);
			_animation_component->clearCycle( "walk_kill", 0.5f );

			setSilentKill();
		}
		else if(state.compare("decoy") == 0)
		{
			_animation_component->blendLookAtOut( 0.5f );
			_animation_component->executeAction( "decoy", 0.5f, 0.5f);
		}
		else if(state.compare("movingShadow") == 0)
		{
			_animation_component->blendCycle( "idle_legs", 1.0f, 0.5f ); // descomentar mentre no fem un walk_legs correcte
			
			_animation_component->clearCycle( "walk_kill", 0.5f);
			_animation_component->blendCycle( "walk_legs", 1.0f, 0.5f);
			_animation_component->blendLookAtOut( 0.5f );
			//_animation_component->executeAction( "aim", 0.5f, 0.5f, 1.0f );
			_animation_component->blendAimIn( 0.5f );
			_animation_component->blendCycle( "aimming", 1.0f, 0.1f );
		}
		else if(state.compare("accelerate") == 0)
		{
			_animation_component->clearCycle("walk_kill", 0.1f);
			_animation_component->executeAction( "accelerate", 0.1f, 0.0f, 1.0f, true );
			_animation_component->blendLookAtOut( 0.1f );
		}
	}
	else if(_state.compare("idleShadow") == 0)
	{
		if(state.compare("idleBasic") == 0)
		{
			_animation_component->clearCycle( "aimming", 0.5f );
			_animation_component->clearCycle( "idle_legs", 0.5f );
			_animation_component->blendAimOut( 0.5f );
			
			if(_animation_component->actionOn("aim"))
				_animation_component->removeAction("aim", 0.0f);
						
			_animation_component->blendCycle( "idle", 1.0f, 0.5f );
			_animation_component->blendLookAtIn( 3.0f );
		}
		else if(state.compare("movingShadow") == 0)
		{
			//_animation_component->clearCycle( "idle_legs", 0.5f ); // comentar mentre no fem un walk_legs correcte
			//_animation_component->clearCycle( "aimming", 0.5f );
			_animation_component->blendCycle( "walk_legs", 1.0f, 0.1f );
			_animation_component->blendCycle( "aimming", 1.0f, 0.5f );
		}
		else if(state.compare("creatingShadow") == 0)
		{
			_animation_component->blockAim();
			//_animation_component->clearCycle( "aimming", 0.5f );
			//_animation_component->blendCycle( "creating_shadow", 1.0f, 0.5f );
		}
		else if(state.compare("accelerateTeleport") == 0)
		{
			_animation_component->clearCycle( "idle_legs", 0.1f ); // descomentar mentre no fem un walk_legs correcte

			if(_animation_component->actionOn("stop_shadow"))
				_animation_component->removeAction("stop_shadow", 0.0f);
			if(_animation_component->actionOn("aim"))
				_animation_component->removeAction("aim", 0.0f);

			_animation_component->clearCycle( "aimming", 0.1f );
			//_animation_component->executeAction( "teleport", 0.0f, 0.0f );
			//_animation_component->blendCycle( "blending", 1.0f, 0.5f );
			_animation_component->blendAimOut( 0.2f );
		}		
	}
	else if(_state.compare("movingShadow") == 0)
	{
		if(state.compare("walk") == 0)
		{
			_animation_component->clearCycle( "idle_legs", 0.5f ); // descomentar mentre no fem un walk_legs correcte
			
			_animation_component->clearCycle( "aimming", 0.5f );
			_animation_component->blendAimOut( 0.2f );
			
			if(_animation_component->actionOn("aim"))
				_animation_component->removeAction("aim", 0.0f);
						
			_animation_component->clearCycle( "walk_legs", 0.5f );
			_animation_component->blendCycle( "walk", 1.0f, 0.5f );
			_animation_component->blendLookAtIn( 3.0f );
		}
		else if(state.compare("idleShadow") == 0)
		{
			_animation_component->clearCycle( "walk_legs", 0.5f );
			_animation_component->blendCycle( "idle_legs", 1.0f, 0.5f );
		}
		else if(state.compare("creatingShadow") == 0)
		{
			_animation_component->blockAim();
			//_animation_component->clearCycle( "aimming", 0.5f );
			_animation_component->clearCycle( "walk_legs", 0.5f );
			_animation_component->blendCycle( "idle_legs", 1.0f, 0.5f );
			//_animation_component->blendCycle( "creating_shadow", 1.0f, 0.5f );
		}
		else if(state.compare("accelerateTeleport") == 0)
		{
			_animation_component->clearCycle( "walk_legs", 0.5f );
			_animation_component->clearCycle( "idle_legs", 0.1f ); // descomentar mentre no fem un walk_legs correcte

			if(_animation_component->actionOn("stop_shadow"))
				_animation_component->removeAction("stop_shadow", 0.0f);
			if(_animation_component->actionOn("aim"))
				_animation_component->removeAction("aim", 0.0f);

			_animation_component->clearCycle( "aimming", 0.1f );
			_animation_component->executeAction( "teleport", 0.0f, 0.1f );
			//_animation_component->blendCycle( "blending", 1.0f, 0.5f );
			_animation_component->blendAimOut( 0.2f );
		}		
	}
	else if(_state.compare("accelerate") == 0)
	{
		if(state.compare("run") == 0)
		{
			_animation_component->blendCycle("run", 1.0f, 0.0f);
			_animation_component->blendLookAtIn( 3.0f );
		}
		else if(state.compare("brake") == 0)
		{
			_animation_component->blendCycle("walk", 1.0f, 0.1f);
			_animation_component->removeAction("accelerate", 0.0f);
			_animation_component->executeAction("brake", 0.1f, 0.3f);
			_animation_component->blendLookAtIn( 3.0f );
		}
		else if(state.compare("idleBasic") == 0)
		{
			_animation_component->clearCycles(0.1);
			_animation_component->blendCycle("idle", 1.0f, 0.1f);
			_animation_component->removeAction("accelerate", 0.0f);
			_animation_component->executeAction("brake", 0.1f, 0.3f);
			_animation_component->blendLookAtIn( 3.0f );
		}
	}
	else if(_state.compare("run") == 0)
	{
		if(state.compare("brake") == 0)
		{
			_animation_component->clearCycle("run", 0.1f);
			_animation_component->blendCycle("walk", 1.0f, 0.1f);
			_animation_component->executeAction("brake", 0.1f, 0.1f);
			_animation_component->blendLookAtIn( 3.0f );
		}
	}
	else if(_state.compare("brake") == 0)
	{
		if(state.compare("accelerate") == 0)
		{
			_animation_component->clearCycle("walk", 0.1f);
			_animation_component->executeAction( "accelerate", 0.05f, 0.0f, 1.0f, true );	
			_animation_component->blendLookAtOut( 0.1f );
		}
		else if(state.compare("idleBasic") == 0)
		{
			_animation_component->clearCycles(0.1);
			_animation_component->blendCycle("idle", 1.0f, 0.1f);
			_animation_component->blendLookAtIn( 3.0f );
		}
	}
	else if(_state.compare("creatingShadow") == 0)
	{
		if(state.compare("stopCreateShadow") == 0)
		{
			_animation_component->blendAimOut(0.5f);

			//_animation_component->clearCycle( "creating_shadow", 1.0f );
			_animation_component->executeAction( "stop_shadow", 0.5f, 0.5f);
			//_animation_component->blendCycle( "aimming", 1.0f, 1.0f );
		}
	}
	else if(_state.compare("accelerateTeleport") == 0)
	{
		if(state.compare("teleporting") == 0)
		{
			_animation_component->blendAimOut(0.0f);

			ModelComponent* mC = EntityManager::get().getComponent<ModelComponent>(_entity);
			mC->enabled = false;
		}
	}
	else if(_state.compare("teleporting") == 0)
	{
		if(state.compare("brakeTeleport") == 0)
		{
			/*_shadowAcComp->exitBlend();*/
			TransformComponent* transformC = EntityManager::get().getComponent<TransformComponent>(_entity);
			TransformComponent* transformCStele = EntityManager::get().getComponent<TransformComponent>(_stele);

			(*transformCStele->transform) = (*transformC->transform);
			transformCStele->moveLocal(btVector3(0,0.5f,0));
	
			AnimationComponent* anim_snake = EntityManager::get().getComponent<AnimationComponent>(_stele);
			anim_snake->executeAction("stop_teleport", 0, 0);
			ModelComponent* model_snake = EntityManager::get().getComponent<ModelComponent>(_stele);
			model_snake->enabled = true;
			model_snake->diffuseColor.w = 1.0f;

			ModelComponent* mC = EntityManager::get().getComponent<ModelComponent>(_entity);
			mC->enabled = true;
			_animation_component->executeAction( "stop_teleport", 0.0f, 0.0f);
			_animation_component->blendLookAtIn( 7.0f );
		}
	}
	else if(_state.compare("brakeTeleport") == 0)
	{
		if(state.compare("idleBasic") == 0)
		{
			ModelComponent* model_snake = EntityManager::get().getComponent<ModelComponent>(_stele);
			model_snake->enabled = false;
			_animation_component->blendCycle("idle", 1.0f, 0.1f);
		}
	}
	else if(_state.compare("blendedWall") == 0)
	{
		_shadowAcComp->exitBlend();

		ModelComponent* mC = EntityManager::get().getComponent<ModelComponent>(_entity);
		mC->enabled = true;

		// ROTAR COMFORME LA NORMAL 
		btVector3 normal = _shadowAcComp->getNormal();

		//quitamos la componente vertical a la normal (problemas con las normales diagonales: techos, etc.)
		normal.setY(0.0f);
		normal.normalize();

		btVector3 front = _transformC->getFront();
		front.setX( front.getX() * -1 ); // Mierda del front

		btVector3 rotationAxis = front.cross(normal).normalize();

		bool valid_axis = !rotationAxis.isZero() && (rotationAxis.getX() == rotationAxis.getX()); //comprobamos que las axis son validas para que no pete el calculo

		float rotationAngle = front.angle(normal);

		if(valid_axis)
			_transformC->rotate(rotationAxis, rotationAngle);

		_transformC->setPosition(_shadowAcComp->getExitPos());

		
		if(_animation_component->actionOn("teleport"))
				_animation_component->removeAction("teleport", 0.0f);


		if(state.compare("leavingShadowWall") == 0)
		{
			
		}
		else if(state.compare("silentMurder") == 0)
		{
			_animation_component->clearCycles( 0.5f );
			_animation_component->blendCycle( "idle", 1.0f, 0.5f);

			/*CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(_entity);
			ccC->deactivateCollisions();*/

			setSilentKill();
		}
	}
	else if(_state.compare("leavingShadowWall") == 0)
	{
		if(state.compare("falling") == 0)
		{
			_animation_component->clearCycle( "blending", 0.05f );
			_animation_component->blendCycle( "falling", 1.0f, 0.05f);
		}
	}
	else if(_state.compare("falling") == 0)
	{
		if(state.compare("landing") == 0)
		{
			_animation_component->clearCycle("falling", 0.05f);
			_animation_component->executeAction( "land", 0.05f, 0.3f);
		}
	}
	else if(_state.compare("landing") == 0)
	{
		if(state.compare("grounding") == 0)
		{
			_animation_component->blendCycle( "idle", 1.0f, 0.3f);
			_animation_component->blendLookAtIn( 7.0f );
		}
	}
	else if(_state.compare("crouchSVision") == 0)
	{
		if(state.compare("specialVision") == 0)
		{
			SoundSystem::get().playSFX3D("special_vision","data/sfx/specialvision1.wav","special_vision",_transformC->getPosition(),btVector3(0,0,0),false,0.5f,0.5f);
		
			World::instance()->toggleSpecialVision();
			_animation_component->blendCycle( "visioning", 1.0f, 0.0f );
		}
	}
	else if(_state.compare("specialVision") == 0) 
	{
		if(state.compare("riseSVision") == 0)
		{
			SoundSystem::get().stopSound("special_vision",false,false);
			World::instance()->toggleSpecialVision();
			_animation_component->executeAction( "stop_vision", 0.2f, 0.2f );
			_animation_component->clearCycle( "visioning", 1.0f );
			_animation_component->blendCycle( "idle", 1.0f, 1.0f );
			_animation_component->blendLookAtIn( 3.0f );
		}
	}
	else if(_state.compare("silentMurder") == 0) 
	{
		if(state.compare("idleBasic") == 0)
		{
			if(_animation_component->getSilentKillAnimationName() == "kill_air")
			{
				CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(_entity);
				ccC->activateCollisions();
			}

			_animation_component->blendCycle( "idle", 1.0f, 0.5f );
			_animation_component->blendLookAtIn( 3.0f );
			computeRandomWpt(state);

			ModelComponent* model_snake = EntityManager::get().getComponent<ModelComponent>(_snake);
			if(model_snake->enabled)
			{
				model_snake->enabled = false;
				BTSystem::get().destroyEnemy(_silentKillVictim);
				_silentKillVictim = NULL;
			}
			

			World::instance()->toggleKillCamera();
			//World::instance()->setTimeScale( 1.0f );

		}
	}
	else if(_state.compare("stopCreateShadow") == 0) 
	{
		if(state.compare("idleShadow") == 0)
		{
			_animation_component->blendAimIn(0.2f);
			_animation_component->unblockAim();
		}
	}
	else if(_state.compare("summonCrow") == 0)
	{
		_animation_component->blendLookAtIn( 5.0f );
	}
	
}

void AUTPlayer::setSilentKill()
{
	_animation_component->blendLookAtOut( 0.5f );
			
	TransformComponent* transformC = EntityManager::get().getComponent<TransformComponent>(_entity);
	TransformComponent* transformCEnemy = EntityManager::get().getComponent<TransformComponent>(_silentKillVictim);

	VictimType _victimType = _animation_component->chooseSilentKillAnimation(transformC->getPosition(), transformCEnemy->getPosition());

	_animation_component->executeAction( _animation_component->getSilentKillAnimationName(), 0.5f, 0.5f);
	EntityManager::get().getComponent<ShadowActionsComponent>(World::instance()->getPlayer())->setVisibility(playerVisibility::BLENDED);
		
	_animation_component->movePlayerToKill(_animation_component->getSilentKillAnimationName(), EntityManager::get().getComponent<TransformComponent>(_silentKillVictim)); 
	CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(_silentKillVictim);
	ccC->deactivateCollisions();

	AnimationComponent* anim_enemy = EntityManager::get().getComponent<AnimationComponent>(_silentKillVictim);
	anim_enemy->removeActions(0);

	// Dependiendo del estado del enemigo lanza animacion con o sin espada en la mano
	EnemyDataComponent* enemy_component = EntityManager::get().getComponent<EnemyDataComponent>(_silentKillVictim);
			
	if(enemy_component->_attentionDegree == attentionDegrees::NORMAL || enemy_component->_attentionDegree == attentionDegrees::PANIC)
		anim_enemy->executeAction( _animation_component->getSilentKillAnimationName(), 0.5f, 0.0f, 1.0f, true);
	else if(enemy_component->_attentionDegree == attentionDegrees::CAUTION || enemy_component->_attentionDegree == attentionDegrees::PERMANENT_CAUTION || enemy_component->_attentionDegree == attentionDegrees::ALERT )
		anim_enemy->executeAction( _animation_component->getSilentKillAnimationName() + "_sword", 0.5f, 0.0f, 1.0f, true);
			
	anim_enemy->blendLookAtOut(0.5f);
	anim_enemy->setSilentKillAnimationName(_animation_component->getSilentKillAnimationName());
	std::string kill_sound_file = "data/sfx/" +  _animation_component->getSilentKillAnimationName() + ".ogg";

	if( _victimType == SHADOW)
	{
		AnimationComponent* anim_snake = EntityManager::get().getComponent<AnimationComponent>(_snake);
		anim_snake->moveSnakeToKill(EntityManager::get().getComponent<TransformComponent>(_silentKillVictim));
		anim_snake->executeAction(_animation_component->getSilentKillAnimationName(), 0, 0);
		ModelComponent* model_snake = EntityManager::get().getComponent<ModelComponent>(_snake);
		model_snake->enabled = true;
		Entity * e = EntityFactory::get().createParticleEffect(D3DXVECTOR3(transformCEnemy->getPosition()) - D3DXVECTOR3(0,0.5f,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_STATIC);
		e->name = "murder_shadows";
		ParticleEffectComponent * p = EntityManager::get().getComponent<ParticleEffectComponent>(e);
		p->timer_emission = 4.0f;

		//Vamos a matar un pavo con poder sombrio y eso da mucho miedito! Lanzar evento de panico!!
		BTSystem::get().pressPanicButton();
	}

	if( _victimType == PANIC)
	{
		transformC->rotateY(M_PI);
		SoundSystem::SoundInfo * panicSound = SoundSystem::get().getSoundInfo("panic");
		if (panicSound != NULL) SoundSystem::get().stopSound("panic");
	}

	SoundSystem::get().playSFX( ("player_" + kill_sound_file), kill_sound_file, ("player_" + kill_sound_file), 1.0f, 1.0f, false );
			
	// Poner camera cinematografica
	World::instance()->toggleKillCamera();
	//World::instance()->setTimeScale( 0.5f );

	EntityManager::get().getComponent<EnemyDataComponent>(_silentKillVictim)->silentKill();

	
		ccC = EntityManager::get().getComponent<CharacterControllerComponent>(_entity);
		ccC->deactivateCollisions();
	

}

void AUTPlayer::stopMovement()
{
	CharacterControllerComponent* charController = EntityManager::get().getComponent<CharacterControllerComponent>(_entity);
	charController->velocity = btVector3(0,0,0);
	charController->controller->setWalkDirection(btVector3(0,0,0));
	_sound_channel_current->desired_volume = 0.0f;
}

void AUTPlayer::render()
{
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );

	/*if (World::instance()->isDebugModeOn())
	{
	}*/
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 120, text_color, "player life: %.2f", _playerContC->_life);
	printf2D( g_App.GetWidth()*3/5, 140, text_color, "player state: %s", _state.c_str());
	_shadowAcComp->renderPlayerVisibility(g_App.GetWidth()*3/5, 160, text_color);
	//unsigned color;
	//if(_shadowAcComp->canTeleport()) color = D3DCOLOR_ARGB( 255, 0, 0, 0 );
	//else color = D3DCOLOR_ARGB( 255, 255, 0, 0 );
	//_shadowAcComp->renderCapsuleCols(color);

	//_shadowAcComp->renderDebug();

	//Directional light vector
	const btVector3& from = _transformC->getPosition();
	LightSystem::get().renderDirLights(from+btVector3(0,-0.6f,0));

	//const btVector3& from = _transformC->getPosition();
	//btVector3 kk = _transformC->getFront();  kk.setX(-kk.getX());
	//const btVector3& to = from+kk*1.5f;
	//btVector3 offset = to-from;  offset.normalize();  offset = offset*0.5;
	//drawLine_bt(from+offset, to, D3DCOLOR_ARGB(255, 255,0,0));
}

void AUTPlayer::changeState(std::string state)
{
	updateTransitionAnimations(state);
	Automat::changeState(state);
}

void AUTPlayer::prepareForRebirth()
{
	//_inactive.setTarget(10.5f);
	_animation_component->clearCycles(0);
	changeState("reborn");
	_animation_component->executeAction("reborn", 0.0f, 1.0f);
}

void AUTPlayer::reset()
{
	if(_state.compare("blended") == 0) changeState("leavingShadow");
	else if(_state.compare("blendedWall") == 0) changeState("leavingShadowWall");

	changeState("idleBasic");

	//#ifdef EDU_DBG
	//	_transformC->move(btVector3(40,5,-23));
	//	//_transformC->move(btVector3(0,5,-5));

	//	_animation_component->blendCycle("falling", 1.0f, 0.0f);
	//	changeState("falling");
	//	_retarget.setTarget(0.75f);
	//	_animation_component->blendLookAtIn( 15.0f );
	//#endif
}

void AUTPlayer::headController( float delta ){
	//dbg("GENERATE_WPT\n");
	// seria interessant generar els waypoints en funcio a zones interessants de l'escena
		
	TransformComponent* transformC = EntityManager::get().getComponent<TransformComponent>(_entity);
	
	float _halfFov = D3DXToRadian(60.0f);
	float _visionDistance = 5.0f;
	float time = 5.0f /  ((_ios->getSpeed() + 1.0f)*3.0f);

	btVector3 target_position;

	getNearEnemyTarget( delta );

	if( _target ) // Si tenemos una transformada objetivo
	{
		target_position = (_target->getPosition() + btVector3(0,0.0f,0)) - _transformC->getPosition();

		btQuaternion rotation = _transformC->transform->getRotation();
		rotation = rotation.inverse();

		target_position = target_position.rotate(rotation.getAxis(), rotation.getAngle());
	
		//check if "reach"....... cambiar de target?
	}
	else // sino de momento random
	{
		target_position = _random_look_at; 

		//check if "reach"
		if ( /*_transformC->isInsideVisionCone( target_position, _halfFov) && _transformC->getPosition().distance2( target_position ) < _visionDistance 
			 ||*/ timeGetTime() - _time_forget > time * 1000.0f)
		{
			computeRandomWpt(_state);

			target_position = _random_look_at;
			_time_forget = (float)timeGetTime();
		}
	}
		_animation_component->lookTowards( target_position, delta );
	//}
	
}

void AUTPlayer::computeRandomWpt(std::string& state)
{
	if( state.compare("idleBasic") == 0 )
	{
		_random_look_at = btVector3 ((FLOAT)getrandom( -5.0f , 1.0f), -1.0f , 1.2f);
	}
	else if( state.compare("walk") == 0 )
	{
		_random_look_at = btVector3 ((FLOAT)getrandom( -5.0f , 3.0f), -1.0f ,  3.2f);
	}
	else if( state.compare("run") == 0 )
	{
		_random_look_at = btVector3 ((FLOAT)getrandom( -10.0f , 10.0f), -1.0f , 7.0f);
	}
}

void AUTPlayer::getNearEnemyTarget( float delta )
{
	if(_target)
	{
		btVector3 front = _transformC->getFront();
		front.setX( front.getX() * -1 );
		float angle = front.angle(_target->getPosition() - _transformC->getPosition());
		if(angle > 1.0f) 
			_target = NULL;
	}

	if(_retarget.count(delta))
	{

		std::map<Entity*,Component*>* entitiesWithED = EntityManager::get().getAllEntitiesPosessingComponent<EnemyDataComponent>();
		if(!entitiesWithED) return;

		float distance = FLT_MAX;
		float min_distance = distance;
		float trigger_distance;

		std::map<Entity*,Component*>::iterator iter;
		for (iter = entitiesWithED->begin(); iter != entitiesWithED->end(); ++iter)
		{
			if( iter->second->enabled ) 
			{

				Entity* entity = iter->first;
				TransformComponent* transformEnemy = EntityManager::get().getComponent<TransformComponent>(entity);
				distance = transformEnemy->getPosition().distance2(_transformC->getPosition());
		
				//dbg("distance: %f\n", distance);
				if( _target ) trigger_distance = 35.0f;
				else trigger_distance = 30.0f;

				if(distance < trigger_distance )
				{	
					btVector3 front = _transformC->getFront();
					front.setX( front.getX() * -1 );

					float angle = front.angle(transformEnemy->getPosition() - _transformC->getPosition());

					//dbg("angle: %f\n", angle);
		
					if(angle < 1.0f && distance < min_distance)
					{
						_target = transformEnemy;
						min_distance = distance;
					}
				}
			}
		}

		if( min_distance == FLT_MAX ) _target = NULL;
	}
}


//CINE -----------------------------------------------------------------------------------
void AUTPlayer::reborn(float delta)
{
	PlayerControllerSystem::get().update(delta, true, false, false);
	_shadowAcComp->setVisibility(playerVisibility::ONSHADOW);
	_shadowAcComp->updateVisibility(delta);
	
	if( _animation_component->isFrameNumber("reborn", 400) )
	{
		_animation_component->blendCycle("idle", 1.0f, 0.0f);
	}
	else if( !_animation_component->actionOn("reborn") )
	{
		_animation_component->blendLookAtIn( 5.0f );
		changeState("idleBasic");
	}
}

void AUTPlayer::idleCG(float delta)
{
	//PlayerControllerSystem::get().update(delta, true, false, false);
	if(!_enableIdleCG) changeState("idleBasic");
}

void AUTPlayer::doorCG(float delta)
{
	//PlayerControllerSystem::get().update(delta, true, false, false);

	/*if( !_animation_component->actionOn("open") )
	{
		changeState("idleCG");
	} */
}

void AUTPlayer::crouchRecharge(float delta)
{
	//esperar a que acabe la accion de agacharse
	if( _animation_component->isFrameNumber("recharge", 62) )
	{
		changeState("recharging");
	}
}

void AUTPlayer::recharging(float delta)
{
	EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->_life += 1.0f;

	//esperar a que acabe la accion de recargar
	if( _animation_component->isFrameNumber("recharge", 112) )
	{
		changeState("riseRecharge");
	}
}

void AUTPlayer::riseRecharge(float delta)
{
	//esperar a que acabe la accion de levantarse
	if( _animation_component->actionOn("recharge") )
	{
		changeState("idleCG");
	}
}


//"BASIC STATE" -----------------------------------------------------------------------------------
void AUTPlayer::idleInactive(float delta)
{
	if(_lockedInPlace)
	{
		PlayerControllerSystem::get().update(delta, true, false, false);
		return;
	}

	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	//vibrate( 0, 0 );
	//_noise = 0.0f;

	//Se actualiza el mov. Si se recibe input para moverse se pasa a walk
	if(PlayerControllerSystem::get().update(delta, true)) 
	{
		//Si ademas esta presionado el boton de acelerar
		if( _ios->isSprinting() )
		{
			changeState("accelerate");
		}
		else
			changeState("walk");
	}
	
	else if( _inactive.count(delta) )
	{
		changeState("idleBasic");
	}

//Transiciones a otros "estados generales"
	///"Estado sombra" (idle sombra)
	else if(_ios->isPressed(_ios->AIM) )
	{
		_shadowAcComp->enableAiming();
		_particleShadowHand->play();
		changeState("idleShadow");
	}
	else if(_target)
		changeState("idleKill");
//Transiciones del "estado basico"
	else checkBasicTransitions();
}

void AUTPlayer::idleKill(float delta)
{
	if(_lockedInPlace)
	{
		PlayerControllerSystem::get().update(delta, true, false, false);
		return;
	}

	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	//vibrate( 0, 0 );
	//_noise = 0.0f;

	//Se actualiza el mov. Si se recibe input para moverse se pasa a walk
	if(PlayerControllerSystem::get().update(delta, true)) 
	{
		//Si ademas esta presionado el boton de acelerar
		if( _ios->isSprinting() )
		{
			changeState("accelerate");
		}
		else
			changeState("walkKill");
	}
	
	
//Transiciones a otros "estados generales"
	///"Estado sombra" (idle sombra)
	else if(_ios->isPressed(_ios->AIM) )
	{
		_shadowAcComp->enableAiming();
		_particleShadowHand->play();
		changeState("idleShadow");

	}

	else if(_target == NULL)
	{
		changeState("idleBasic");
	}

//Transiciones del "estado basico"
	else checkBasicTransitions();

}

void AUTPlayer::idleBasic(float delta)
{
	if(_lockedInPlace)
	{
		//PlayerControllerSystem::get().update(delta, true, false, false);
		if(_enableIdleCG) changeState("idleCG");
		return;
	}

	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	//vibrate( 0, 0 );
	//_noise = 0.0f;

	//Se actualiza el mov. Si se recibe input para moverse se pasa a walk
	if(PlayerControllerSystem::get().update(delta, true)) 
	{
		//Si ademas esta presionado el boton de acelerar
		if( _ios->isSprinting() )
		{
			changeState("accelerate");
		}
		else
			changeState("walk");
	}
	
	else if( _inactive.count(delta) )
	{
		_inactive.setTarget(_inactiveAnimationTime);
		changeState("idleInactive");
	}

	//Transiciones a otros "estados generales"
	///"Estado sombra" (idle sombra)
	else if(_ios->isPressed(_ios->AIM) )
	{
		_shadowAcComp->enableAiming();
		_particleShadowHand->play();
		changeState("idleShadow");
	}
	else if(_target)
		changeState("idleKill");

//Transiciones del "estado basico"
	else checkBasicTransitions();
}

void AUTPlayer::accelerate(float delta)
{
	if(_lockedInPlace)
	{
		changeState("brake");
		return;
	}

	float speed = _ios->getSpeed();
	float current_speed_normalized = (PlayerControllerSystem::get().getCurrentSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/) / (PlayerControllerSystem::get().getMaxSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/);

	//dbg("current speed normalized %f\n", current_speed_normalized);
	_sound_channel_current->desired_volume = current_speed_normalized*0.75f;
	//SoundSystem::get().blendStepsVolume(current_speed_normalized, 0.2f);
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setCameraFov(current_speed_normalized);

	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);
	
	if(!PlayerControllerSystem::get().update(delta, true, true) || !_ios->isSprinting())
	{
		changeState("brake");
	}
	
	else if( _animation_component->actionBlocked("accelerate") )
	{
		changeState("run");
	}
}

void AUTPlayer::brake(float delta)
{
	if(_lockedInPlace)
	{
		changeState("idleBasic");
		return;
	}

	float speed = _ios->getSpeed();
	float current_speed_normalized = (PlayerControllerSystem::get().getCurrentSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/) / (PlayerControllerSystem::get().getMaxSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/);

	//dbg("current speed normalized %f\n", current_speed_normalized);

	//SoundSystem::get().blendStepsVolume(current_speed_normalized, 0.2f);
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setCameraFov(current_speed_normalized);

	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);
	
	
	if( !PlayerControllerSystem::get().update(delta, true, true) || !_animation_component->actionOn("brake") )
	{
		changeState("walk");
	}
	else if( PlayerControllerSystem::get().update(delta, true, true) && _ios->isSprinting() )
	{
		changeState("accelerate");
	}
	
}

void AUTPlayer::walk(float delta)
{
	if(_lockedInPlace)
	{
		changeState("idleBasic");
		return;
	}

	//Se actualiza el mov. Si no hay se hace la transicion a idleBasic
	if(!PlayerControllerSystem::get().update(delta, true, true))
	{
		changeState("idleBasic"); return;
	}
	
	// Blending animaciones walk & run
	float current_speed_normalized = (PlayerControllerSystem::get().getCurrentSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/) / (PlayerControllerSystem::get().getMaxSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/);

	//dbg("current speed normalized %f\n", current_speed_normalized);
	_sound_channel_current->desired_volume = current_speed_normalized*0.75f;
	//SoundSystem::get().blendStepsVolume(current_speed_normalized, 0.2f);
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setCameraFov(current_speed_normalized);


	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	if( _ios->isSprinting() )
	{
		changeState("accelerate");
	}

//Transiciones a otros "estados generales"
	///Asesinato silencioso
	if(_ios->becomesPressed(_ios->SILENT_KILL) )
	{
		if(checkPanicVictim()) changeState("silentMurder");
		else if(checkVictim()) changeState("silentMurder");
	}

	///Senyuelo
	else if(_ios->becomesPressed(_ios->DECOY))
	{
		if(!_animation_component->actionOn("decoy"))	
		{
			changeState("decoy");
		}
	}
	
	///"Estado sombra" (moving sombra)
	else if(_ios->isPressed(_ios->AIM) )
	{
		_shadowAcComp->enableAiming();
		_particleShadowHand->play();
		changeState("movingShadow");
	}
	else if( _target )
	changeState("walkKill");

	if(_animation_component->actionOn("decoy")) 
	{
		if ( _animation_component->isFrameNumber("decoy",15) && special_action != "decoy") 
		{
			special_action = "decoy";
			SoundSystem::get().playSFX3D("player_decoy", "data/sfx/decoy.ogg", "", _transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f);
			ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(_entity);
			Entity * parE2 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_transformC->getPosition()),ParticleEffectComponent::PARTICLE_EFFECT::DECOY, m);
		}
		else special_action = "";
	}
}

void AUTPlayer::walkKill(float delta)
{
	if(_lockedInPlace)
	{
		changeState("idleKill");
		return;
	}

	//Se actualiza el mov. Si no hay se hace la transicion a idleBasic
	if(!PlayerControllerSystem::get().update(delta, true, true))
	{
		changeState("idleKill"); return;
	}
	
	// Blending animaciones walk & run
	float current_speed_normalized = (PlayerControllerSystem::get().getCurrentSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/) / (PlayerControllerSystem::get().getMaxSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/);

	//dbg("current speed normalized %f\n", current_speed_normalized);
	_sound_channel_current->desired_volume = current_speed_normalized*0.75f;
	//SoundSystem::get().blendStepsVolume(current_speed_normalized, 0.2f);
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setCameraFov(current_speed_normalized);


	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	if( _ios->isSprinting() )
	{
		changeState("accelerate");
	}

//Transiciones a otros "estados generales"
	///Asesinato silencioso
	if(_ios->becomesPressed(_ios->SILENT_KILL) )
	{
		if(checkPanicVictim()) changeState("silentMurder");
		else if(checkVictim()) changeState("silentMurder");
	}

	///Senyuelo
	else if(_ios->becomesPressed(_ios->DECOY))
	{
		if(!_animation_component->actionOn("decoy"))	changeState("decoy");
	}
	
	///"Estado sombra" (moving sombra)
	else if(_ios->isPressed(_ios->AIM) )
	{
		_shadowAcComp->enableAiming();
		_particleShadowHand->play();
		changeState("movingShadow");
	}
	else if(_target == NULL)
		changeState("walk");

	if(_animation_component->actionOn("decoy")) 
	{
		if ( _animation_component->isFrameNumber("decoy",15) && special_action != "decoy") 
		{
			special_action = "decoy";
			//sound->playSFX("data/sfx/decoy.ogg", NULL, 1.0f, false);
			SoundSystem::get().playSFX3D("player_decoy", "data/sfx/decoy.ogg", "", _transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f);
			ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(_entity);
			Entity * parE2 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_transformC->getPosition()),ParticleEffectComponent::PARTICLE_EFFECT::DECOY, m);
		}
		else special_action = "";
	}
}

void AUTPlayer::run(float delta)
{
	if(_lockedInPlace)
	{
		changeState("brake");
		return;
	}
	
	//Se actualiza el mov. Si no hay se hace la transicion a walk
	if(!PlayerControllerSystem::get().update(delta, true, true) || !_ios->isSprinting() )
	{
		changeState("brake");
	}
	
	// Blending animaciones walk & run
	float current_speed_normalized = (PlayerControllerSystem::get().getCurrentSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/) / (PlayerControllerSystem::get().getMaxSpeed() /*- PlayerControllerSystem::get().getStdSpeed()*/);

	//dbg("current speed normalized %f\n", current_speed_normalized);
	_sound_channel_current->desired_volume = current_speed_normalized*0.75f;
	//SoundSystem::get().blendStepsVolume(current_speed_normalized, 0.2f);
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setCameraFov(current_speed_normalized);

	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);
}

void AUTPlayer::checkBasicTransitions()
{
	///Asesinato silencioso
	if(_ios->becomesPressed(_ios->SILENT_KILL) )
	{
		if(checkPanicVictim()) changeState("silentMurder");
		else if(checkVictim()) changeState("silentMurder");
	}
	///Senyuelo
	else if(_ios->becomesPressed(_ios->DECOY))
	{
		if(!_animation_component->actionOn("decoy")) 
		{
			changeState("decoy");
		}
		
	}
	///Vision especial
	else if(_ios->becomesPressed(_ios->SPECIAL_VISION)  && _shadowAcComp->canUseSV())
	{
		changeState("crouchSVision");
	}
	//Invocar cuervo
	else if(_ios->becomesPressed(_ios->CROW))
	{
		changeState("summonCrow");
	}
	
	if(_animation_component->actionOn("decoy")) 
	{
		if ( _animation_component->isFrameNumber("decoy",15) && special_action != "decoy") 
		{
			special_action = "decoy";
			//sound->playSFX("data/sfx/decoy.ogg", NULL, 1.0f, false);
			SoundSystem::get().playSFX3D("player_decoy", "data/sfx/decoy.ogg", "", _transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f);
			ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(_entity);
			Entity * parE2 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_transformC->getPosition()),ParticleEffectComponent::PARTICLE_EFFECT::DECOY, m);
		}
		else special_action = "";
	}
}

void AUTPlayer::silentMurder(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);
	//Eliminar inercia
	PlayerControllerSystem::get().update(delta, true, true, false);

	
	CameraSystem::CameraSequence* cam_sequence = CameraSystem::get().getBestCameraSequence();
	unsigned frame = cam_sequence->cameraInfos[cam_sequence->index]->frame;

	if(_animation_component->isFrameNumber(_animation_component->getSilentKillAnimationName(), frame) )
	{
		CameraSystem::get().changeKillCamera();
	}

	if(_animation_component->isFrameNumber(_animation_component->getSilentKillAnimationName(), frame) && _lastVictim != _silentKillVictim)
	{
		_lastVictim = _silentKillVictim;
		TransformComponent * tVictim = EntityManager::get().getComponent<TransformComponent>(_silentKillVictim);
		EnemyDataComponent* edc = EntityManager::get().getComponent<EnemyDataComponent>(_silentKillVictim);
		edc->_visionPercent = 0.0f;
		edc->_searching = false; // para que el vision interface deje de pintar ese malo si estaba buscando
		edc->setAttentionDegree(attentionDegrees::NORMAL);

		if( _victimType == PANIC || _victimType == NORMAL || _victimType == SHADOW )
		{
			CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(_entity);
			ccC->activateCollisions();
		}

		btVector3 f;
		tVictim->getFrontXinv(f);
		std::string killName = _animation_component->getSilentKillAnimationName();
		if (killName != "kill_shadow") 
		{
			int r = randomFloat(1,10);
			std::string deathSound = "data/sfx/9_death_" + intToString(r) + ".mp3";
			if (killName == "kill") 
			{
				
				SoundSystem::get().playSFX3D(killName,deathSound,killName,tVictim->getPosition(),btVector3(0,0,0),false,1.0f,1.0f);
				EntityFactory::get().createParticleEffect(D3DXVECTOR3(tVictim->getPosition()) + D3DXVECTOR3(0,0.25f,0),ParticleEffectComponent::PARTICLE_EFFECT::BLOOD_SPLATTER,NULL,D3DXVECTOR3(0,0,0),D3DXVECTOR3(f));
			}
			else if (killName == "kill2") 
			{
				SoundSystem::get().playSFX3D(killName,deathSound,killName,tVictim->getPosition(),btVector3(0,0,0),false,1.0f,1.0f);
				EntityFactory::get().createParticleEffect(D3DXVECTOR3(tVictim->getPosition()) + D3DXVECTOR3(0,0.75f,0),ParticleEffectComponent::PARTICLE_EFFECT::BLOOD_SPLATTER,NULL,D3DXVECTOR3(0,0,0),D3DXVECTOR3(f));
			}
			else if (killName == "kill3") 
			{
				SoundSystem::get().playSFX3D(killName,deathSound,killName,tVictim->getPosition(),btVector3(0,0,0),false,1.0f,1.0f);

				EntityFactory::get().createParticleEffect(D3DXVECTOR3(tVictim->getPosition()) + D3DXVECTOR3(0,0.25f,0),ParticleEffectComponent::PARTICLE_EFFECT::BLOOD_SPLATTER,NULL,D3DXVECTOR3(0,0,0),D3DXVECTOR3(f));
			}
			else if (killName == "kill_shadow2") 
			{
				SoundSystem::get().playSFX3D(killName,deathSound,killName,tVictim->getPosition(),btVector3(0,0,0),false,1.0f,1.0f);

				f.rotate(btVector3(0,1,0),M_PI_2);
				EntityFactory::get().createParticleEffect(D3DXVECTOR3(tVictim->getPosition()) + D3DXVECTOR3(0,0.75f,0),ParticleEffectComponent::PARTICLE_EFFECT::BLOOD_SPLATTER,NULL,D3DXVECTOR3(0,0,0),D3DXVECTOR3(f));
			}
			//else if (killName == "kill_panic") 
			//{
			//	SoundSystem::get().playSFX3D(killName,deathSound,killName,tVictim->getPosition(),btVector3(0,0,0),false,1.0f,1.0f);

			//	f.rotate(btVector3(0,0,1),M_PI_2);
			//	EntityFactory::get().createParticleEffect(D3DXVECTOR3(tVictim->getPosition()) + D3DXVECTOR3(0,0.25f,0),ParticleEffectComponent::PARTICLE_EFFECT::BLOOD_SPLATTER,NULL,D3DXVECTOR3(0,0,0),D3DXVECTOR3(f));
			//}
		}	

	}

	if( !_animation_component->actionOn(_animation_component->getSilentKillAnimationName()) )
	{	
		changeState("idleBasic");
	}

	// TODO Lanzar particula sangre
	//EntityFactory::get().createParticleEffect(
}

void AUTPlayer::decoy(float delta) //senyuelo
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	//Hacer que produzca ruido
	_playerContC->_noiseDistSq = decoyNoiseSq;

	//Si se esta moviendo vamos a idle basic, sino a moving basic
	if(PlayerControllerSystem::get().update(delta, true))
	{
		if(_target == NULL) changeState("walk");
		else changeState("walkKill");
	}
	else
	{
		if(_target == NULL) changeState("idleBasic");	
		else changeState("idleKill");	
	}
	
}
					
void AUTPlayer::blendedWall(float delta) //fundido en sombra en pared
{
	//Restar vida
	_playerContC->_life -= _playerContC->_costBlended;
	//Actualizar visibilidad
	_shadowAcComp->updateVisibilityBlended(delta);

	//dbg("blend button: %i  life: %i  hidden: %i\n", _ios->becomesPressed(_ios->BLEND), !_playerContC->hasLifeForBlend(), !_shadowAcComp->isHidden());

	/*if(_ios->becomesPressed(_ios->BLEND) || !_playerContC->hasLifeForBlend() || !_shadowAcComp->isHidden() )
	{*/
		if( checkAerialVictim())		
			changeState("silentMurder");
		else
			changeState("leavingShadowWall");
	//}

	/*else if(_ios->becomesPressed(_ios->SILENT_KILL))
	{
		if( checkAerialVictim())		
			changeState("silentMurder");
	}*/
}

void AUTPlayer::leavingShadow(float delta) //des-fundiendose
{
	/*if( !_animation_component->actionOn("stop_blend") )
	{*/
		changeState("idleBasic");
		
	//}
}

void AUTPlayer::leavingShadowWall(float delta) //des-fundiendose
{
	//if( !_animation_component->actionOn("stop_blend") )
	//{
		//*********** AQUI PODRIAMOS COMPROBAR SI ESTAMOS MUY ABAJO Y PASAR A IDLE BASIC ****************//

		changeState("falling");
	//}
}
		
void AUTPlayer::crouchSVision(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	if( _animation_component->actionBlocked("vision") )
	{
		changeState("specialVision");
	}
}

void AUTPlayer::specialVision(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	if(_ios->isReleased(_ios->SPECIAL_VISION) || !_shadowAcComp->isHidden())
		changeState("riseSVision");
}

void AUTPlayer::riseSVision(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	if( !_animation_component->actionOn("stop_vision") )
	{
		changeState("idleBasic");
	}
}

void AUTPlayer::summonCrow(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	if( !_animation_component->actionOn("summon") ) //animacion de invocar
	{
		changeState("idleBasic");
	}
	else if ( _animation_component->isFrameNumber("summon",25)) 
	{
		//invocar cuervo
		World::instance()->enableCrow();
	}
}


//"SHADOW STATE" -----------------------------------------------------------------------------------
void AUTPlayer::idleShadow(float delta)
{
	if(_lockedInPlace)
	{
		_shadowAcComp->disableAiming();
		changeState("idleBasic");
		return;
	}

	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	//Update del modo sombra
	_shadowAcComp->update(delta);

	//PlayerControllerSystem::get().update(delta, true, true, false);

	//Se actualiza el mov. Si se recibe input para moverse se pasa a walk
	if(PlayerControllerSystem::get().update(delta))
	{
		changeState("movingShadow");
		return;
	}


//Transiciones a otros "estados generales"
	///"Estado basico" (idle basico)
	if(_ios->isReleased(_ios->AIM) ) 
	{
		_shadowAcComp->disableAiming();
			_particleShadowHand->stop();
		changeState("idleBasic");

	}

//Transiciones del "estado sombra"
	checkShadowTransitions();
}

void AUTPlayer::movingShadow(float delta)
{
	if(_lockedInPlace)
	{
		_shadowAcComp->disableAiming();
		changeState("idleShadow");
		return;
	}

	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	//Update del modo sombra
	_shadowAcComp->update(delta);
	//Se actualiza el mov. Si no hay se hace la transicion a idleShadow
	if(!PlayerControllerSystem::get().update(delta))
	{
		changeState("idleShadow");
		return;
	}

//Transiciones a otros "estados generales"
	///"Estado basico" (moviendose sombra)
	if(_ios->isReleased(_ios->AIM) )
	{
		_shadowAcComp->disableAiming();
		_particleShadowHand->stop();
		
		changeState("walk");
		return;
	}

//Transiciones del "estado sombra"
	checkShadowTransitions();
}

void AUTPlayer::checkShadowTransitions()
{
	//Crear sombra
	if(_ios->becomesPressed(_ios->CREATE_SHADOW))
	{
		if (_playerContC->hasLifeForCreateShadow() && _shadowAcComp->canCreateShadow() )
		{
			//Restar vida
			_playerContC->_life -= _playerContC->_costCreateShadow;
			SoundSystem::get().playSFX3D("create_shadow","data/sfx/s1crea.ogg","create_shadow",_transformC->getPosition(),btVector3(0,0,0),true,1.0f,1.0f);

			changeState("creatingShadow");
		}
		else
		{
			SoundSystem::get().playSFX("error_shadow","data/sfx/spell_fail.ogg","error_shadow",1.0f,1.0f, false);
		}
	}

	//Teletransporte
	if(_ios->becomesPressed(_ios->TELEPORT))
	{
		if (_shadowAcComp->canTeleport())
		{
		/*_shadowAcComp->setTeleportMode(true);
		changeState("telepReady");*/

		//Restar vida
		_playerContC->_life -= _playerContC->_costTeleport;

		_shadowAcComp->disableAiming();
		changeState("accelerateTeleport");
		//ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(_playerContC->entity);
		SoundSystem::get().playSFX("player_teleport","data/sfx/teleport.wav","player_teleport",1.0f,1.0f,false);
		/*Entity * parE = EntityFactory::get().createParticleEffect(D3DXVECTOR3(0,0,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_TELEPORT, m, D3DXVECTOR3(_shadowAcComp->getExitPos()) - D3DXVECTOR3(0,1.0f,0));
		Entity * parE2 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(0,0.0f,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_EXPLOSION, m);*/
		}
		else
		{
			SoundSystem::get().playSFX("error_shadow","data/sfx/spell_fail.ogg","error_shadow",1.0f,1.0f, false);
		}
	}
}

void AUTPlayer::creatingShadow(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	PlayerControllerSystem::get().update(delta, false, false, false);

	if(_ios->isPressed(_ios->CREATE_SHADOW) && _shadowAcComp->canGrowShadow())
	{
		//Restar vida
		_playerContC->_life -= _playerContC->_costGrowShadow;

		_shadowAcComp->update(delta, true);
		_shadowAcComp->shadowCreation();
	}
	else 
	{
		SoundSystem::get().stopSound("create_shadow",false,false);
		SoundSystem::get().playSFX3D("stop_shadow","data/sfx/s1cola.ogg","stop_shadow",_transformC->getPosition(),btVector3(0,0,0),false,1.0f,1.0f);
		changeState("stopCreateShadow");
	}
}

void AUTPlayer::stopCreateShadow(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	_shadowAcComp->update(delta, true);
	_shadowAcComp->stopCreatingShadow();
	changeState("idleShadow");
}

void AUTPlayer::accelerateTeleport(float delta)
{
	//Esperar a que acabe accion de accelerateTeleport

	//if(!_animation_component->actionOn("teleport") )
	//{
		//vemos si vamos al suelo o al techo
		if(btVector3(0,1,0).dot(_shadowAcComp->getNormal()) > 0.8f)
		{
			//suelo
			_telepGround = true;
		}
		else
		{
			//pared/techo
			_telepGround = false;
		}

		//Desactivar colisiones del player (obtenemos el ghostObject(body) y el character controller y los sacamos del mundo fisico. Se queda asi hasta que se encuentre una forma mejor)
		CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(World::instance()->getPlayer());
   		PhysicsSystem::get().getDynamicsWorld()->removeCollisionObject(ccC->controller->getGhostObject());
		PhysicsSystem::get().getDynamicsWorld()->removeAction(ccC->controller);

		//Visibilidad de teleporting (nula)
		_shadowAcComp->setVisibility(playerVisibility::TELEPORTING);

		//Cuando acabe
		changeState("teleporting");
	//}
}

void AUTPlayer::teleporting(float delta)
{
	//teleportToTarget devuelve true cuando acaba
	if(_shadowAcComp->teleportToTarget())
	{
		_shadowAcComp->setBlendPos(_shadowAcComp->getColPos());

		if( _telepGround) 
			changeState("brakeTeleport");
		else
 			changeState("blendedWall");
	}
}

void AUTPlayer::brakeTeleport(float delta)
{
	//Esperar a que acabe accion de brakeTeleport
	if(_animation_component->isFrameNumber( "stop_teleport", 0) )
	{
		//Volver a activar las colisiones del player (pillamos el controller y el body y los volvemos a meter en el mundo fisico)
		CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(World::instance()->getPlayer());
		PhysicsSystem::get().getDynamicsWorld()->addCollisionObject(ccC->controller->getGhostObject(),colisionTypes::CHARARTER, -1);
		PhysicsSystem::get().getDynamicsWorld()->addAction(ccC->controller);	
	}
	if(_animation_component->isFrameNumber( "stop_teleport", 20) )
	{
		//"desbloquear" visibilidad
		_shadowAcComp->setVisibility(playerVisibility::ONSHADOW);

		//Cuando acabe
		changeState("idleBasic");
	}
	else
	{
		ModelComponent* model_snake = EntityManager::get().getComponent<ModelComponent>(_stele);
		model_snake->addAlpha(- (1.0f/60.0f) * 2);
	}
}

//OTHERS -----------------------------------------------------------------------------------
void AUTPlayer::falling(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	//A falta de un metodo mejor, para evitar bugeos, permitimos que se pueda dirigir al player mientras cae, para casos en los que se atasca
	PlayerControllerSystem::get().update(delta, true, false);

	//lanzar un ray al suelo y ver si la distacia es la minima para aterrizar
	btVector3 btRayFrom = _transformC->getPosition();
	btVector3 btRayTo = btRayFrom;
	btRayTo.setY(-1000.0f);

	//Creamos callback para obtener resultado de la colisión
	btCollisionWorld::ClosestRayResultCallback rayCallback(btRayFrom,btRayTo);
	//Test de colisión
	PhysicsSystem::get().getCollisionWorld()->rayTest(btRayFrom, btRayTo, rayCallback);
	//Si colisiona hacemos cositas molonas
	if (rayCallback.hasHit())
	{
		btVector3 hitPosition = rayCallback.m_hitPointWorld;

		// BUSCAR EL OBJECT (DEL QUE HEREDA btRigidBody) EN CADA UNO DE LOS RigidbodyComponent?? ESTO ES SHIT 
		//const btCollisionObject* object = rayCallback.m_collisionObject;

		//"desbloquear" visibilidad
		_shadowAcComp->setVisibility(playerVisibility::ONSHADOW);

		if( hitPosition.distance2(btRayFrom) < 2.0f )
			changeState("landing");
	}

}

void AUTPlayer::landing(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	if(_shadowAcComp->isGrounded()) changeState("grounding");
}

void AUTPlayer::grounding(float delta)
{
	//Actualizar visibilidad
	_shadowAcComp->updateVisibility(delta);

	//Por si nos movemos en el aire al caer (fix), eliminamos inercia
	PlayerControllerSystem::get().update(delta, false, false, false);

	if( !_animation_component->actionOn("land") )
		changeState("idleBasic");
}

void AUTPlayer::dying(float delta)
{
	// Bloqueamos el player
	EntityManager::get().getComponent<CharacterControllerComponent>(World::instance()->getPlayer())->controller->setWalkDirection(btVector3(0,0,0));
	stopMovement();
	_sound_channel->desired_volume = 0;
	_sound_channel2->desired_volume = 0;

	if( _animation_component->actionBlocked("dying") )
	{
		/*CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(_entity);
		ccC->activateCollisions();*/

		World::instance()->mustResetCurrentLevel();
		VisionInterface::get().setCompassAlpha(0.0f);
	}
}


//MISC FUNCTIONS
//Comprueba si hay un enemigo delante al que matar sigilosamente
bool AUTPlayer::checkVictim()
{
	//Obtener vector de enemigos
	std::map<Entity*, Component*> *enemies = EntityManager::get().getAllEntitiesPosessingComponent<EnemyDataComponent>();
	std::map<Entity*, Component*>::iterator enemy;

	if(!enemies) return false;

	for(enemy=enemies->begin(); enemy!=enemies->end(); enemy++) //Para cada enemigo...
	{
		if(!EntityManager::get().getComponent<EnemyDataComponent>(enemy->first)->enabled) continue; //Si el EnemyDataComponent esta desactivado es que esta muerto (convencion)
		TransformComponent* TCEnemy = EntityManager::get().getComponent<TransformComponent>(enemy->first); 
		if(manhattanDist(_transformC->getPosition(), TCEnemy->getPosition()) > _silentKillDistMh) continue; //Si no esta suficientemente cerca no se puede matar.
		if(!_transformC->isInsideVisionCone(TCEnemy->getPosition(), 0.7f)) continue; //Si no esta en un rango admisible delante del player tampoco.
		if(_transformC->getFront().dot(TCEnemy->getFront()) < 0.7) continue; //Si no estan mirando casi en la misma direccion (enemigo de espaldas) tampoco.

		_silentKillVictim = enemy->first;
		_victimType = VictimType::NORMAL;
		return true;
	}

	return false;
}

//Comprueba si hay un enemigo debajo al que matar sigilosamente
bool AUTPlayer::checkAerialVictim()
{
	//Obtener vector de enemigos
	std::map<Entity*, Component*> *enemies = EntityManager::get().getAllEntitiesPosessingComponent<EnemyDataComponent>();
	std::map<Entity*, Component*>::iterator enemy;

	if(!enemies) return false;

	for(enemy=enemies->begin(); enemy!=enemies->end(); enemy++) //Para cada enemigo...
	{
		if(!EntityManager::get().getComponent<EnemyDataComponent>(enemy->first)->enabled) continue; //Si el EnemyDataComponent esta desactivado es que esta muerto (convencion)
		TransformComponent* TCEnemy = EntityManager::get().getComponent<TransformComponent>(enemy->first); 
	
		btVector3 enemy_to_player = _transformC->getPosition() - TCEnemy->getPosition();
		btVector3 up = btVector3(0,1,0);

		//dbg("angle aerial kill to %s %f\n", enemy->first->name.c_str(), up.angle(enemy_to_player)); 

		if(up.angle(enemy_to_player) > 0.45f) continue; //Si no esta debajo del player tampoco.

		_silentKillVictim = enemy->first;
		_victimType = VictimType::AERIAL;
		return true;
	}

	return false;
}

bool AUTPlayer::checkBlendVictim()
{
	//Obtener vector de enemigos
	std::map<Entity*, Component*> *enemies = EntityManager::get().getAllEntitiesPosessingComponent<EnemyDataComponent>();
	std::map<Entity*, Component*>::iterator enemy;

	if(!enemies) return false;

	for(enemy=enemies->begin(); enemy!=enemies->end(); enemy++) //Para cada enemigo...
	{
		if(!EntityManager::get().getComponent<EnemyDataComponent>(enemy->first)->enabled) continue; //Si el EnemyDataComponent esta desactivado es que esta muerto (convencion)
		TransformComponent* TCEnemy = EntityManager::get().getComponent<TransformComponent>(enemy->first); 
	
		btVector3 enemy_to_player =  (TCEnemy->getPosition() +  btVector3(0,2,0)) -  _transformC->getPosition();
		btVector3 up = btVector3(0,1,0);

		//dbg("angle blended kill to %s %f\n", enemy->first->name.c_str(), up.angle(enemy_to_player)); 

		if(up.angle(enemy_to_player) > 0.6f) continue; //Si no esta encima del player tampoco.

		_silentKillVictim = enemy->first;
		_victimType = VictimType::BLEND;
		return true;
	}

	return false;
}

//Comprueba si hay un enemigo delante al que matar sigilosamente
bool AUTPlayer::checkPanicVictim()
{
	//Obtener vector de enemigos
	std::map<Entity*, Component*> *enemies = EntityManager::get().getAllEntitiesPosessingComponent<EnemyDataComponent>();
	std::map<Entity*, Component*>::iterator enemy;

	if(!enemies) return false;

	for(enemy=enemies->begin(); enemy!=enemies->end(); enemy++) //Para cada enemigo...
	{
		if(!EntityManager::get().getComponent<EnemyDataComponent>(enemy->first)->enabled) continue; //Si el EnemyDataComponent esta desactivado es que esta muerto (convencion)
		TransformComponent* TCEnemy = EntityManager::get().getComponent<TransformComponent>(enemy->first); 
		if(manhattanDist(_transformC->getPosition(), TCEnemy->getPosition()) > _panicKillDistMh) continue; //Si no esta suficientemente cerca no se puede matar.
		if(!_transformC->isInsideVisionCone(TCEnemy->getPosition(), 0.4f)) continue; //Si no esta en un rango admisible delante del player tampoco.
		if(EntityManager::get().getComponent<EnemyDataComponent>(enemy->first)->_attentionDegree != attentionDegrees::PANIC) continue; //Si no estan mirando casi en la misma direccion (enemigo de espaldas) tampoco.
		if(EntityManager::get().getComponent<AnimationComponent>(enemy->first)->actionOn("fall")) continue;

		_silentKillVictim = enemy->first;
		_victimType = VictimType::PANIC;
		

		return true;
	}

	return false;
}

//Antes, en el update...
//Obtener enemigo mas cercano
	//std::vector<Automat*> aicontrollers = AIMgr::get()->aicontrollers;

	//D3DXVECTOR3 position = getEntity3D()->getPosition();
	//float distance = FLT_MAX; 

	//AIController* closest = aicontrollers[1];

	//size_t size = aicontrollers.size();
	//for (size_t i = 1; i<size; ++i) //saltamos el 0 (player)!!
	//{
	//	if(aicontrollers[i]->getState() != "dying")
	//	{
	//		D3DXVECTOR3 diff = (aicontrollers[i]->getEntity3D()->getPosition() - position); 
	//		float curDistance = D3DXVec3LengthSq(&diff); 
	//		if (curDistance < distance) 
	//		{ 
	//			closest = aicontrollers[i]; 
	//			distance = curDistance; 
	//			dbg("state closest: %s\n", (aicontrollers[i]->getState()).c_str());
	//		} 
	//	}
	//}

//Si se esta viendo enemigo mas cercano, mirar hacia el
	//if( head->getEntity3D()->isInsideVisionCone(closest->getEntity3D()->getPosition(), half_fov ) && distance < vision_distance)
	//{
	//	_target_character = closest->getEntity3D();
	//	head->changeState("look_at_target_character");
	//	//dbg("look_at_target_character\n");
	//	color = 0xffff0000;
	//}


//void AUTPlayer::locked( float delta )
//{
////	CIOStatus &ios = CIOStatus::get();
////	//(void) delta;
////	_noise = ios.left.normalized_magnitude * (_run_noise * 2.0f );
////
////	// 3rd person controller
////	/*TCamera* camera = World::instance()->getCamera();
////		
////	D3DXVECTOR3 front_camera = camera->getTarget() - camera->getPosition();
////	front_camera.y = 0;
////
////	D3DXVECTOR3 left_camera;
////	D3DXVec3Cross(&left_camera, &front_camera, &D3DXVECTOR3(0, 1, 0));
////
////	D3DXVec3Normalize(&front_camera, &front_camera);
////
////	D3DXVECTOR3 direction = (normalized_ly * normalized_magnitude_l * delta) * front_camera + (normalized_lx * normalized_magnitude_l * delta) * left_camera;
////
////	getEntity3D()->move(direction);
////	turn(delta, getEntity3D()->getPosition() + direction);*/
////
////
////	//1st person controller
////	advance(ios.left.y * delta);
////	strafe(ios.left.x  * delta);
////	rotate(ios.right.x * delta);
////
////	//else if( _target_character->getAIController()->getState() == "dying" )
////	//{
////	//	//aqui cambiariamos al mas cercano, como si fuera el stick derecho
////	//	//como no esta implementado pasamos a idle para la demo
////	//	changeState("idle");
////	//}
//}
//
//
//void AUTPlayer::attack( float delta )
//{
////	((EntityMesh*)getEntity3D())->setMesh(TMeshManager::get()->getMesh("player_attack"));
////	//((EntityMesh*)head->getEntity3D())->setMesh(TMeshManager::get()->getMesh("cabeza_kill"));
////	((EntityMesh*)head->getEntity3D())->setMesh(NULL);
////
////	//dbg("ATTACK\n");
////
////	if(timeOut( delta, 0.5f) )
////	{
////		if(getEntity3D()->getDistanceTo(_target_character->getPosition()) < 1.0f )
////		{
////			((AICCharacter*)(_target_character->getAIController()))->changeState("process_impact");
////		}
////		
////		changeState("locked");
////	}
//}
//
//void AUTPlayer::processImpact( float delta )
//{
////	((EntityMesh*)getEntity3D())->setMesh(TMeshManager::get()->getMesh("enemy_death"));
////	((EntityMesh*)head->getEntity3D())->setMesh(NULL);
////
////	//dbg("PROCESS_IMPACT\n");
////	color = 0x00ff00;
////
////	vibrate(60000, 60000);
////	
////	dbg("life %f", _life);
////
////	if(_life <= 0.0f)
////	{
////		changeState("dying");
////	}
////	if( timeOut( delta, 0.2f) )
////		changeState("locked");	
//}
//

//void AUTPlayer::telepReady(float delta) //Listo para el teletrans.
//{
//	//...
//
//	//Actualizar visibilidad
//	_shadowAcComp->updateVisibility(delta);
//
//	PlayerControllerSystem::get().update(delta, false, false, false); //Para acabar con la inercia
//	_shadowAcComp->update(delta);
//
//	if(_ios->isReleased(CIOStatus::TButton::KEYBOARD_E) )//|| _shadowAcComp->getVisibility()==playerVisibility::VISIBLE)
//	{
//		_shadowAcComp->setTeleportMode(false);
//		changeState("idleShadow");
//	}
//	if(_ios->becomesPressed(CIOStatus::TButton::KEYBOARD_SPACE) && _shadowAcComp->canTeleport())
//	{
//		//Restar vida
//		_playerContC->_life -= _playerContC->_costTeleport;
//
//		_shadowAcComp->setTeleportMode(false);
//		_shadowAcComp->disableAiming();
//		_telepActivated = true;
//		changeState("enteringShadow");
//	}
//}
