#include "bt_common.h"
#include "world.h"
#include "component_shadow_actions.h"
#include "component_player_controller.h"
#include "behaviour_tree.h"
#include "dijkstra.h"
#include "component_enemy_data.h"
#include "component_transform.h"
#include "component_automat.h"
#include "component_bt.h"
#include "camera_controller_3rd.h"
#include "vision_interface.h"
#include "bt_gatekeeper.h"
#include "bt_patrol.h"

#include "entity_manager.h"
#include "system_bt.h"
#include "system_playercontroller.h"
#include "entity_factory.h"

BTCommon::BTCommon(Entity* entity) : BehaviourTree(entity)
{
	_eD = new EnemyDataComponent(entity, EntityManager::get().getComponent<TransformComponent>(entity));
	EntityManager::get().addComponent(_eD, entity);

	init();
}

BTCommon::~BTCommon()
{
}

void BTCommon::init()
{
	_trackingPoint = _trackingPlayer = _trackingCorpse = _unsheathed = _attacking = _stoppedSearching = _turning = false;
	_targetCorpse = btVector3(0,0,0);
	_pointToSee = btVector3(0,0,0);
	_corpseE = NULL;
	_chasePlayerNow = false;
	_chaserAlly = NULL;
	_lookChaserAlly.setTarget(-1.0f);
}


//CONDITIONS--------------------------------------------------------------------------------------------------------------------------------------------
bool BTCommon::checkIfHasToPanic()
{
	const btVector3& enemyPos = _eD->_transformC->getPosition();
	float vision_distSq = _eD->_visionDistSq;
	float half_fov = _eD->_halfFov;
	if(_eD->_attentionDegree == attentionDegrees::CAUTION || _eD->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
	{
		vision_distSq *= _eD->_visionFactor_caution;
		half_fov = _eD->_halfFovPeriferial; //si el malo esta en caution usara el cono periferico, que es mas grande!!!!
	}

	//Si se ha pulsado el boton del panico y el arma esta enfundada comprobamos si "estamos viendo" al player (entre comillas porque lo que el malo veria seria el asesinato en sombras)
	if(BTSystem::get().isPanicButtonPressed() && !_unsheathed)
	{
		const btVector3& playerPos = EntityManager::get().getPlayerPos();

		//Primero se comprueba la distancia cuadratica
		float distToPlayer_sq = playerPos.distance2(enemyPos);
		if(distToPlayer_sq > vision_distSq) return false;

		if(_eD->_transformC->isInsideVisionCone(playerPos, _eD->_halfFovPeriferial))
		{
			//Lanzar rayo por si hay oclusiones
			if(!PhysicsSystem::get().checkCollision(_eD->_eyePos, playerPos, PhysicsSystem::get().colMaskVision)) 
			{
				return true;
			}
		}
	}

	return false;
}

bool BTCommon::checkDamageReceived()
{
	//dbg("arma %i\n", _unsheathed);
	return _eD->_damageTaken;
}

bool BTCommon::checkWarDistance()
{
	if(!_eD->_playerSeen) return false; //Si no hemos visto al player, fuera.
	
	if(!_unsheathed) return false; //Si no tenemos la espada en la mano, fuera.
	if(manhattanDist(_eD->_transformC->getPosition(), EntityManager::get().getPlayerPos()) <= _eD->_warDistMh)
	{
		unsigned playerVisibility = EntityManager::get().getComponent<ShadowActionsComponent>(World::instance()->getPlayer())->getVisibility();
		if(playerVisibility == playerVisibility::TELEPORTING) return false;
		else return true;
	}
	return false;
}

bool BTCommon::checkPlayerViewed()
{
	if(_chasePlayerNow) //caso especial: queremos que el malo "vea" al player aunque no lo vea!
	{
		_eD->_visionPercent = 1.0f;
		//Grado de atencion maximo
		_eD->setAttentionDegree(attentionDegrees::ALERT);
		_trackingPlayer = true;
		_eD->_playerSeen = true;
		_eD->_searching = false;
		return true;
	}

	float visionDistSq = _eD->_visionDistSq;

	unsigned playerVisibility = EntityManager::get().getComponent<ShadowActionsComponent>(World::instance()->getPlayer())->getVisibility();
	if(playerVisibility == playerVisibility::BLENDED || playerVisibility == playerVisibility::TELEPORTING)
	{
		decrementVisionPercent();
		return false;
	}
	else if(playerVisibility == playerVisibility::ONSHADOW) visionDistSq = _eD->_visionDistSq_playerInShadows;
	else if(playerVisibility == playerVisibility::ILLUMINATED) visionDistSq = _eD->_visionDistSq_playerInLight;

	bool maximumAttention = _eD->_attentionDegree == attentionDegrees::ALERT;

	if(_eD->_attentionDegree == attentionDegrees::CAUTION || _eD->_attentionDegree == attentionDegrees::PERMANENT_CAUTION) visionDistSq *= _eD->_visionFactor_caution;
	else if(maximumAttention) visionDistSq *= _eD->_visionFactor_alert;

	const btVector3& playerPos = EntityManager::get().getPlayerPos();
	const btVector3& enemyPos = _eD->_transformC->getPosition();
	bool tooNear = false;

	//Primero se comprueba la distancia cuadratica con el player (considerar dist. manhattan en un futuro)
	float distToPlayer_sq = playerPos.distance2(enemyPos);
	if(distToPlayer_sq < visionDistSq)
	{
		//Zona de "espacio vital" de los malos. Si estas muy cerca sube el visionPercent aunque no te vean
		if(distToPlayer_sq < 1.5f)
		{
			incrementVisionPercent(0.0025f);
			tooNear = true;
		}

		//Despues mirar si el player entra en el campo de vision del enemigo
		if(_eD->_transformC->isInsideVisionCone(playerPos, _eD->_halfFov, _eD->_angleWithPlayer))
		{
			//Lanzar rayo por si hay oclusiones
			if(PhysicsSystem::get().checkCollision(_eD->_eyePos, playerPos, PhysicsSystem::get().colMaskVision)) 
			{
				decrementVisionPercent();
				return false;
			}

			if(!maximumAttention && _eD->_visionPercent < 1.0f)
			{
				//En este punto 'estamos viendo' al player. Subimos porcentaje de vision segun distancia. Si llega a 1 lo vemos del todo
				incrementVisionPercent((visionDistSq-distToPlayer_sq)*0.00005f);
				return false;
			}

			if(!_eD->_playerSeen && _eD->_attentionDegree != attentionDegrees::PANIC) //Si no lo estabamos viendo ya, hacemos lo siguiente:
			{
				//Grado de atencion maximo
				_eD->setAttentionDegree(attentionDegrees::ALERT);
				_trackingPlayer = true;
				_eD->_playerSeen = true;
			}
			_eD->_searching = false;
			return true;
			//return false; //para pruebas de visibilidad
		}
		//Si no entra en el cono principal pero si en el periferico...
		else if(_eD->_transformC->isInsideVisionCone(playerPos, _eD->_halfFovPeriferial))
		{
			//Lanzar rayo por si hay oclusiones
			if(PhysicsSystem::get().checkCollision(_eD->_eyePos, playerPos, PhysicsSystem::get().colMaskVision))
			{
				decrementVisionPercent();
				return false;
			}

			if(!maximumAttention && _eD->_visionPercent < 1.0f)
			{
				//En este punto 'estamos viendo de refilon' al player. Subimos porcentaje de vision segun distancia. Si llega a 1 lo vemos del todo
				incrementVisionPercent((visionDistSq-distToPlayer_sq)*0.000025f); //La mitad que en el cono principal
				return false;
			}
			else if(maximumAttention)
			{
				//Si el enemigo se encuentra en alerta el cono periferico tb le sirve para ver al player
				_eD->_searching = false;
				return true;
			}
		}
	}

	_eD->_alertThrown = false;
	_eD->_playerSeen = false;

	//Si estamos muy cerca del malo incrementamos su visionPercent, aunque no nos vea
	if(!tooNear) decrementVisionPercent();

	return false;
}

bool BTCommon::checkIfWantToAlert() //"Aviso antes o voy directamente??"
{
	//Si nos han forzado a "ver" al player no avisamos, pq se puede generar un bucle guapo
	if(_chasePlayerNow)
	{
		_chasePlayerNow = false;
		return false;
	}
	else return true; //de momento avisa siempre
}

bool BTCommon::checkAlertNotThrown()
{
	return !_eD->_alertThrown;
}

bool BTCommon::checkSheathed()
{
	return !_unsheathed;
}

bool BTCommon::checkTrackingTarget()
{
	return _trackingPlayer || _trackingPoint || _trackingCorpse;
}

bool BTCommon::checkHasTrackPoints()
{
	return _eD->_path.size()>0;
}

bool BTCommon::checkAllyChasingPlayer()
{
	const btVector3& enemyPos = _eD->_transformC->getPosition();
	float vision_distSq = _eD->_visionDistSq;
	float half_fov = _eD->_halfFov;
	if(_eD->_attentionDegree == attentionDegrees::CAUTION || _eD->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
	{
		vision_distSq *= _eD->_visionFactor_caution;
		half_fov = _eD->_halfFovPeriferial; //si el malo esta en caution usara el cono periferico, que es mas grande!!!!
	}

	//Recorrer enemigos
	std::map<Entity*,Component*>* entitiesWithBT = EntityManager::get().getAllEntitiesPosessingComponent<BTComponent>();
	std::map<Entity*,Component*>::iterator entityBT;
	for(entityBT=entitiesWithBT->begin(); entityBT!=entitiesWithBT->end(); entityBT++)
	{
		BehaviourTree* e_bt = EntityManager::get().getComponent<BTComponent>(entityBT->first)->getBT();
		//solo nos interesan los patrulleros o gatekeepers
		if(!dynamic_cast<BTGatekeeper*>(e_bt) && !dynamic_cast<BTPatrol*>(e_bt))
		{
			continue;
		}

		//De los gk o patrolers, a ver si vemos alguno...
		const btVector3& allyPos = EntityManager::get().getComponent<TransformComponent>(entityBT->first)->getPosition();

		//Primero se comprueba la distancia cuadratica con el aliado
		float distToAlly_sq = allyPos.distance2(enemyPos);
		if(distToAlly_sq > vision_distSq) continue;

		//Si se puede ver comprobamos si esta dentro del cono de vision
		if(_eD->_transformC->isInsideVisionCone(allyPos, half_fov))
		{
			//Lanzar rayo por si hay oclusiones
			if(PhysicsSystem::get().checkCollision(_eD->_eyePos, allyPos, PhysicsSystem::get().colMaskVision)) 
				continue;

			//si el tio esta en estado chasingPlayer devolvemos true!!!
			if(*e_bt->getCurrentAction() == "chasePlayer" || *e_bt->getCurrentAction() == "chasePlayer2") 
			{
				//Nos guardamos el TC del pavus
				_chaserAlly = entityBT->first;
				return true;
			}
		}

	}

	return false;
}

bool BTCommon::checkCorpseSeen()
{
	return _targetCorpse!=btVector3(0,0,0);
}

bool BTCommon::checkIfFirstCorpse()
{
	return _seenCorpses.size() == 0;
}

bool BTCommon::checkNearCorpse()
{
	if(_targetCorpse == btVector3(0,0,0)) return false;
	return manhattanDist(_eD->_transformC->getPosition(), _targetCorpse) < 4.0f;
}

bool BTCommon::checkSeeingCorpse()
{
	const btVector3& enemyPos = _eD->_transformC->getPosition();
	float vision_distSq = _eD->_visionDistSq;
	if(_eD->_attentionDegree == attentionDegrees::CAUTION || _eD->_attentionDegree == attentionDegrees::PERMANENT_CAUTION) vision_distSq *= _eD->_visionFactor_caution;

	//Recorrer vector de cadaveres
	const std::set<Entity*> corpses = BTSystem::get().getDeadEnemies();
	std::set<Entity*>::iterator corpse;
	for(corpse=corpses.begin(); corpse!=corpses.end(); corpse++)
	{
		//Si no lo hemos visto y no lo hemos mirado...
		if(_seenCorpses.find(*corpse)==_seenCorpses.end())
		{
			const btVector3& corpsePos = EntityManager::get().getComponent<TransformComponent>(*corpse)->getPosition();

			//Primero se comprueba la distancia cuadratica con el muerto
			float distToCorpse_sq = corpsePos.distance2(enemyPos);
			if(distToCorpse_sq > vision_distSq) continue;

			//Si se puede ver comprobamos si esta dentro del cono de vision
			if(_eD->_transformC->isInsideVisionCone(corpsePos, _eD->_halfFov))
			{
				//Lanzar rayo por si hay oclusiones
				if(PhysicsSystem::get().checkCollision(_eD->_eyePos, corpsePos, PhysicsSystem::get().colMaskVision)) 
					continue;

				_targetCorpse = corpsePos;
				_corpseE = *corpse;
				_eD->setAttentionDegree(attentionDegrees::PERMANENT_CAUTION); //Una vez visto un cadaver ya no se volvera a estar normal
				return true;
			}
		}
	}

	return false;
}

bool BTCommon::checkWarningInBlackboard()
{
	//Primero obtenemos el map de advertencias.
	std::map<Entity*, btVector3*> warnings = BTSystem::get().getSbbWarnings();

	//Miramos si alguna advertencia de otro enemigo nos llega
	std::map<Entity*, btVector3*>::iterator iter;
	for(iter = warnings.begin(); iter != warnings.end(); iter++)
	{
		if(iter->second->distance2(_eD->_transformC->getPosition()) < _eD->_hearDistSq)
		{
			//Guardamos la pos del (primer) paio que avisa, para saber luego a donde mirar
			_pointToSee = *BTSystem::get().getSbbWarnings().begin()->second;
			_lookAllyTemp.setTarget(2.0f);
			return true;
		}
	}
	
	return false;
}

bool BTCommon::checkWantToGo()
{
	//De momento siempre true
	return true;
}

bool BTCommon::checkLoudNoiseHeard()
{
	float playerNoiseSq = EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->_noiseDistSq;

	//El player esta produciendo ruido?
	if(playerNoiseSq > 0.0f)
	{   //Si el player esta mas cerca de la distancia a la cual se oye ese ruido...
		if(_eD->_transformC->getPosition().distance2(EntityManager::get().getPlayerPos()) <= playerNoiseSq)
		{
			if(_eD->_attentionDegree != attentionDegrees::PERMANENT_CAUTION && _eD->_attentionDegree != attentionDegrees::ALERT)
				_eD->setAttentionDegree(attentionDegrees::CAUTION);
			_eD->_lookAtClock.setTarget(1.0f+rand()%2); //Se quedara mirando de 1 a 2 segundos
			_eD->_soundPlace = EntityManager::get().getPlayerPos();
			return true;
		}
	}

	return false;
}

bool BTCommon::checkPlayerPartiallyViewed()
{
	return _eD->_visionPercent > _eD->_visionThreshold;
}

bool BTCommon::checkSearching()
{
	return _eD->_searching;
}

bool BTCommon::checkStoppedSearching()
{
	return _stoppedSearching;
}

bool BTCommon::checkHasLookAt()
{
	return _eD->_lookAt != btVector3(0,0,0);
}


//ACTIONS--------------------------------------------------------------------------------------------------------------------------------------------
int BTCommon::fall()
{
	//Rotar hacia el player el doble de rapido de lo normal (susto)
	const btVector3& playerPos = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->getPosition();
	float angle;
	_eD->_transformC->approximateFront_p(playerPos, _eD->_rotateVel*_eD->_delta*2.0f, angle);

	if(angle < 0.01f)
	{
		_animation_component->clearCycles(1.5f);
		_animation_component->executeAction("fall", 0.5f, 0.5f);
		_animation_component->blendCycle("panic", 1.0f, 1.5f);
		TransformComponent * tran = EntityManager::get().getComponent<TransformComponent>(_entity);
		SoundSystem::get().playSFX3D( "panic", "data/sfx/panico.wav", "panic", tran->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f );
		_panicClock.setTarget(10.0f);
		_eD->setAttentionDegree(attentionDegrees::PANIC);
		return LEAVE; //Cuando haya animacion anyadir restriccion de que la accion de caer hay acabado
	}
	else return STAY;
}

int BTCommon::beScared()
{
	//if(!_animation_component->actionOn("fall"))
	//	_eD->setAttentionDegree(attentionDegrees::PANIC);

	checkPlayerViewed(); //actualizar visibilidad

	//si se "esta viendo" al player...
	if(_eD->_visionPercent > 0.2f)
	{
		//Retroceder muy poco a la vez que se encara al player
		const btVector3& playerPos = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->getPosition();
		float angle;
		_eD->_transformC->approximateFront_p(playerPos, _eD->_rotateVel*_eD->_delta, angle);
		if(angle < 0.1f) _eD->advance(-_eD->_transformC->getFront()*_eD->_delta*0.1f);
		SoundSystem::get().set3DPosition("panic",_eD->_transformC->getPosition());
	}

	if(_panicClock.count(World::instance()->getElapsedTimeUInSeconds()))
	{
		SoundSystem::get().stopSound("panic");
		_animation_component->executeAction("panic_up", 0.5f, 0.5f);
		return LEAVE;
	}

	return STAY;
}

int BTCommon::standUp()
{
	if( _animation_component->isFrameNumber("panic_up", 90) )
	{
		_animation_component->clearCycles(0.0f);
		_animation_component->blendCycle("idle_caution", 1.0f, 0.0f);
	}
	//Esperar a que se acabe la accion de levantarse
	else if(!_animation_component->actionOn("panic_up"))
	{
		_unsheathed = true;
		_eD->setAttentionDegree(attentionDegrees::PERMANENT_CAUTION);
		return LEAVE;
	}
	
	return STAY;
}

int BTCommon::takeDamage()
{
	// asesinato silencioso, lo pongo aqui de momento
	if( _eD->_silent_kill )
	{
		assert(findNode("die"));
		current = findNode("die");
		return PUSHSTATE;
	}
		
	return STAY;
}

int BTCommon::die()
{
	std::string killAnimationName; 
	// Dependiendo del estado del enemigo lanza animacion con o sin espada en la mano
	EnemyDataComponent* enemy_component = EntityManager::get().getComponent<EnemyDataComponent>(_entity);
			
	if(enemy_component->_attentionDegree == attentionDegrees::NORMAL || enemy_component->_attentionDegree == attentionDegrees::PANIC)
		killAnimationName = _animation_component->getSilentKillAnimationName();
	else if(enemy_component->_attentionDegree == attentionDegrees::CAUTION || enemy_component->_attentionDegree == attentionDegrees::PERMANENT_CAUTION || enemy_component->_attentionDegree == attentionDegrees::ALERT)
		killAnimationName = _animation_component->getSilentKillAnimationName() + "_sword";

	if( _animation_component->actionBlocked(killAnimationName) )
	{		
		/*if(killAnimationName != "kill_shadow")*/ _eD->dead();
		_animation_component->enabled = false;
		return LEAVE;
	}
	
	return STAY;
}

int BTCommon::attack()
{
	if( !_attacking )
	{
		Entity* player = World::instance()->getPlayer();

		if(EntityManager::get().getComponent<AutomatComponent>(player)->getAutomat()->getState() != "dying" && EntityManager::get().getComponent<AutomatComponent>(player)->getAutomat()->getState() != "silentMurder")
		{		
			/*//Si hacemos que no esperen a acabar el asesinato (se tiene que debugar)
			if(EntityManager::get().getComponent<AutomatComponent>(player)->getAutomat()->getState() == "silentMurder")
			{
				World::instance()->toggleKillCamera();
				CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(player);
				ccC->activateCollisions();
			}*/

			_attacking = true;

			_animation_component->executeAction("death2", 0.0f, 0.0f, 1.0f, true);
			// Poner camera cinematografica
			EntityManager::get().getComponent<AnimationComponent>(player)->setSilentKillAnimationName("death2");
	
			TransformComponent * player_trans = EntityManager::get().getComponent<TransformComponent>(player);
			player_trans->approximateFront_p(_eD->_transformC->getPosition(), 1000);
					
			EntityManager::get().getComponent<AnimationComponent>(player)->removeActions(0.01f);
			EntityManager::get().getComponent<AnimationComponent>(player)->executeAction("dying", 0.0f, 0.0f, 1.0f, true);
			EntityManager::get().getComponent<AnimationComponent>(player)->blendLookAtOut(0);
			EntityManager::get().getComponent<AnimationComponent>(player)->blendAimOut(0);

			World::instance()->toggleDeathCamera();
		
			((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(player)->getAutomat())->stopMovement();
			((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(player)->getAutomat())->changeState("dying");

			//Si esta apuntando el player desactivamos la mirilla para que no salga en la cinematica
			if(EntityManager::get().getComponent<ShadowActionsComponent>(player)->enabled) 
				EntityManager::get().getComponent<ShadowActionsComponent>(player)->disableAiming();

			//quitamos el sonido de pasos y ponemos el de la secuencia
			//SoundSystem::get().stopSteps();
			ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(player);
			Entity * e = EntityFactory::get().createParticleEffect(D3DXVECTOR3(player_trans->getPosition()),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_EXPLOSION_MODEL,m);
			e->name = "shadow_dead";
		}
		else
		{
			_animation_component->blendCycle("idle_fight", 1.0f, 0.5f);
		}
	}
			
	return STAY;
}

int BTCommon::alertAllies()
{
	if(!_eD->_alerting) //frame X: mandar alerta para que los demas la vean
	{
		_eD->_alerting = true;
		BTSystem::get().addSbbWarning(_entity, _eD->_transformC->getPosition());
		return STAY;
	}
	else //frame X+1: todo el mundo deberia haber podio acceder a la alerta. La eliminamos
	{
		_eD->_alerting = false;
		BTSystem::get().removeSbbWarning(_entity);
		_eD->_alertThrown = true;
		return LEAVE;
	}
}

int BTCommon::unsheathe()
{
	_eD->_transformC->approximateFront_p(EntityManager::get().getPlayerPos(), _eD->_rotateVel*_eD->_delta);

	if( !_unsheathed )
	{
		_animation_component->executeAction("unsheathe", 0.5f, 0.1f);
		_unsheathed = true;
		//Se genera path hasta el player
		pathfind(EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->getPosition(), _eD->_path);
	}	
	else if( _animation_component->isFrameNumber("unsheathe", 63) )
	{
		_animation_component->clearCycles(0);
		_animation_component->blendCycle("run", 1.0f, 0.0f);
	}
	else if( !_animation_component->actionOn("unsheathe") )
	{
		return LEAVE;
	}
			
	return STAY;
}

int BTCommon::chasePlayer()
{
	//ultimo nodo desde el que se puede acceder al player
	int currentPlayerNode = BTSystem::get().getplayerNode();

	//Si no tenemos path, lo creamos
	if(!_eD->_path.size())
	{
		DijkstraGraph::get().computePath(_eD->_transformC->getPosition(), EntityManager::get().getPlayerPos(), _eD->_path);
	}
	//Si el player se ha movido hasta (cerca de) otro nodo diferente, anyadimos la posicion de ese al path...solo si se puede acceder desde el final del path actual!!!!!!
	else if(_eD->_lastPlayerNode != currentPlayerNode)
	{
		btVector3 from = _eD->_path.at(_eD->_path.size()-1); //-1:pos del player antes
		btVector3 to = DijkstraGraph::get().getNodePos(currentPlayerNode);
		//si no hay colision entre el ultimo nodo del path y el nodo accesible al player...
		if(!PhysicsSystem::get().checkCollision(from, to, PhysicsSystem::get().colMaskNavigation))
		{
			_eD->_path.at(_eD->_path.size()-1) = DijkstraGraph::get().getNodePos(currentPlayerNode); //Aqui estaba antes la pos destino. La cambiamos por el nodo nuevo...
			_eD->_path.push_back(EntityManager::get().getPlayerPos()); //...y al final la pos del player
		}
		else //Si no se puede acceder hay que hacer pathfind otra vez
		{
			DijkstraGraph::get().computePath(_eD->_transformC->getPosition(), EntityManager::get().getPlayerPos(), _eD->_path);
		}
		
	}
	//En todo caso actualizamos la pos destino (pos del player)
	else
	{
		_eD->_path.at(_eD->_path.size()-1) = EntityManager::get().getPlayerPos();
	}

	//Nos queda constancia del ultimo nodo que se ha considerado accesible desde el player
	_eD->_lastPlayerNode = currentPlayerNode;

	_eD->goToNextTrackPoint(_eD->_delta, _eD->_path);

	return LEAVE;
}

int BTCommon::trackNextPoint()
{
	_eD->goToNextTrackPoint(_eD->_delta, _eD->_path);
	return LEAVE;
}

int BTCommon::stopTracking()
{
	if(_trackingPlayer)
	{
		//Empezamos a olvidarnos del player
		_eD->_forgetClock.setTarget(_eD->_forgetTime+rand()%3);
		_trackingPlayer = false;
	}
	else if(_trackingPoint)
	{
		_eD->_forgetClock.setTarget(_eD->_forgetTime+rand()%3);
		_trackingPoint = false;
	}
	else if(_trackingCorpse)
	{
		_eD->_forgetClock.setTarget(_eD->_corpseLookTime+rand()%3);
		_trackingCorpse = false;
	}

	//Inicializar vector de direcciones donde mirar desde ahi
	_eD->setLookAts(1.5f);
	_eD->_searching = true;
	//Que empiece buscando con lookat=front con que llega al sitio
	_eD->_transformC->getFrontXinv(_eD->_lookAt);

	return LEAVE;
}

int BTCommon::lookAtChaser()
{
	//dbg("%s: lookAtChaser\n", _entity->name);
	//si hemos entrado aqui tenemos la entity* del persecutor
	const btVector3& chaserAllyPos = EntityManager::get().getComponent<TransformComponent>(_chaserAlly)->getPosition();

	//Mirar hacia el tio durante X segundos
	if(!_lookChaserAlly.hasTarget())
	{
		if(_eD->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
			_lookChaserAlly.setTarget(1.0f);
		else
			_lookChaserAlly.setTarget(3.0f);
	}
	else
	{
		if(_lookChaserAlly.count(World::instance()->getElapsedTimeUInSeconds()))
		{
			//ir hacia donde va el tio (siguiente estado de secuencia)
			_eD->setAttentionDegree(attentionDegrees::ALERT);
			return LEAVE;
		}
		else
		{
			//mirar hacia el tio
			_eD->_transformC->approximateFront_p(chaserAllyPos, _eD->_rotateVel*_eD->_delta);
		}
	}

	//Si mientras rotamos vemos al player salimos de aqui
	if(checkPlayerViewed())
	{
		_lookChaserAlly.setTarget(-1.0f);
		return LEAVE;
	}

	return STAY;
}

int BTCommon::goWhereAllyGoes()
{
	if(!_chaserAlly) return LEAVE; //esto no deberia pasar jamas de la vida, pero bueno.
	//dbg("%s: goWhereAllyGoes\n", _entity->name);
	//pillar el destino del otro pavo y pathfindeamos hasta el
	const std::deque<btVector3>& chaserAllyPath = EntityManager::get().getComponent<EnemyDataComponent>(_chaserAlly)->_path;
	const btVector3& chaserAllyDest = chaserAllyPath.at(chaserAllyPath.size()-1);
	pathfind(chaserAllyDest, _eD->_path);
	_trackingPoint = true;

	return LEAVE;
}

int BTCommon::lookAtCorpse()
{
	if(!_eD->_forgetClock.hasTarget()) _eD->_forgetClock.setTarget(_eD->_corpseLookTime+rand()%3);

	//Ir olvidando...
	if(_eD->_forgetClock.count(_eD->_delta))
	{
		//Si se acaba el tiempo, pasar del cadaver
		_targetCorpse = btVector3(0,0,0);
		_seenCorpses.insert(_corpseE);
		_corpseE = NULL;
		_eD->_corpseLookTime = 2.0f; //A partir de la segunda vez se empana menos al ver un muerto
		return LEAVE;
	}

	//Rotar hacia el cadaver
	btVector3 lookAt = _targetCorpse - _eD->_transformC->getPosition();
	_eD->_transformC->approximateFront_v(lookAt, _eD->_rotateVel*_eD->_delta);

	return LEAVE;
}

int BTCommon::goToCorpse()
{
	btVector3 dest;
	DijkstraGraph::get().getNearPos(_targetCorpse, 1.5f, dest);
	pathfind(dest, _eD->_path);
	_trackingPoint = true;
	return LEAVE;
}

int BTCommon::rememberCorpse()
{
	//Vacio. Lo que hace falta ya se hace en la funcion checkSeeingCorpse() antes de devolver true
	return LEAVE;
}

int BTCommon::lookToAlly()
{
	if(!_lookAllyTemp.count(_eD->_delta))
	{
		//Mientras mira, si ve al player salimos de la secuencia.
		if(checkPlayerViewed()) return LEAVE;

		//Rota hacia aliado
		_eD->_transformC->approximateFront_p(_pointToSee, _eD->_rotateVel*_eD->_delta);
		return STAY;
	}
	else
	{
		_pointToSee = btVector3(0,0,0);
		return LEAVE;
	}
}

int BTCommon::unsheatheIfHasTo()
{
	//Si no venimos de este mismo estado (no hay STAY) y estamos desenfundados, salimos
	if((*_previousAction != "unsheatheIfHasTo" && *_previousAction != "unsheatheIfHasTo2") && _unsheathed) return LEAVE;

	////si la espada esta enfundada...
	//if(!_unsheathed)
	//{
	//	//Accion de desenfundar
	//	_animation_component->executeAction("unsheathe", 0.5f, 0.1f, 1.0f, true);
	//	_unsheathed = true;
	//}
	////Sino, si la accion de desenfundar ya ha acabado...
	//else if(_animation_component->actionBlocked("unsheathe"))
	//{
	//	_animation_component->clearCycles(0);
	//	_animation_component->blendCycle("idle_caution", 1.0f, 0.0f);
	//	return LEAVE;
	//}

	//return STAY;

	//si la espada esta enfundada...
	if( !_unsheathed )
	{
		//Accion de desenfundar
		_animation_component->executeAction("unsheathe", 0.5f, 0.1f);
		_unsheathed = true;
	}	
	else if( _animation_component->isFrameNumber("unsheathe", 63) )
	{
		_animation_component->clearCycles(0);
		_animation_component->blendCycle("idle_caution", 1.0f, 0.0f);
	}
	//Sino, si la accion de desenfundar ya ha acabado...
	else if( !_animation_component->actionOn("unsheathe") )
	{
		return LEAVE;
	}

	return STAY;
}

int BTCommon::goWithAlly()
{
	_chasePlayerNow = true;
	return LEAVE;
}

int BTCommon::ignoreWarning()
{
	return LEAVE;
}

int BTCommon::lookAtSoundSource()
{
	//Encararse al punto donde sono el ruido...
	_eD->_transformC->approximateFront_p(_eD->_soundPlace, _eD->_rotateVel*_eD->_delta);
	
	//Desenfundar si hace falta...
	if( !_unsheathed )
	{
		/*dbg("EMPIEZO A DESENFUNDAR %s\n", _entity->name.c_str());*/
		_animation_component->executeAction("unsheathe", 0.5f, 0.3f);
		_unsheathed = true;
		std::string sound_file = "data/sfx/unsheathe.wav";
		SoundSystem::get().playSFX3D( ("enemy_" + sound_file).c_str(), sound_file.c_str(), "", _eD->_transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f );
	}	
	else if( _animation_component->isFrameNumber("unsheathe", 56) )
	{
		_animation_component->clearCycles(0);
		_animation_component->blendCycle("idle_caution", 1.0f, 0.0f);
	}	
	//// Si la animacion desenfundar esta bloqueada pero aun en marcha (ultimo frame)
	//else if( _animation_component->actionBlocked("unsheathe") && _animation_component->actionOn("unsheathe"))
	//{
	//	/*dbg("HE ACABADO DE DESENFUNDAR %s\n", _entity->name.c_str());*/
	//	_animation_component->clearCycles(0);
	//	_animation_component->blendCycle("idle_caution", 1.0f, 0.0f);
	//}


	//Si la accion de desenfundar ha acabado
	if(!_animation_component->actionOn("unsheathe"))
	{
		//Mientras mira, si ve al player salimos de la secuencia (si ja no estamos desenfundando) (Plantear si dividir el estado?)
		if(checkPlayerViewed()) 
		{
			/*dbg("ME VOY EN VIEWED %s\n", _entity->name.c_str());*/
			return LEAVE;
		}

		//...mirar hasta que pase el tiempo de observacion
		if( _eD->_lookAtClock.count(_eD->_delta))
		{
		
			/*dbg("ME VOY EN CLOCK %s duracion unsheathe %f time %f blocked %i\n", _entity->name.c_str(), _animation_component->getDuration("unsheathe"), _animation_component->getTime("unsheathe"), _animation_component->actionBlocked("unsheathe"));*/
			_trackingPoint = true;
			return LEAVE;
		}
	}

	return STAY;
}

int BTCommon::goToSoundSource()
{
	pathfind(_eD->_soundPlace, _eD->_path); //Como _trackpoint esta activo, solo falta generar path hacia punto
	return LEAVE;
} 

int BTCommon::lookForPlayer()
{
	//Rotar hacia el player
	const btVector3& playerPos = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->getPosition();
	_eD->_transformC->approximateFront_p(playerPos, _eD->_rotateVel*_eD->_delta);
	return LEAVE;
}

int BTCommon::stopSearching()
{
	//Cambiar estado de atencion
	if(_eD->_attentionDegree == attentionDegrees::ALERT)
		_eD->setAttentionDegree(attentionDegrees::PERMANENT_CAUTION); //Una vez se ha visto al player no se vuelve a normal
	else if(_eD->_attentionDegree == attentionDegrees::CAUTION)
		_eD->setAttentionDegree(attentionDegrees::NORMAL);

	_eD->_searching = false;
	_stoppedSearching = false; //Un poco raro, pero ya hemos parado de "parar de buscar"

	return LEAVE;
}

int BTCommon::sheathe()
{
	std::string sound_file;
	std::string sound_type;
	std::string enemy_type = _animation_component->getEnemyType();

	//Si se ha quedado en permanent caution no debe enfundar
	if(_eD->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
	{
		sound_type = "got_";
		sound_file += sound_type + enemy_type + ".mp3";
		SoundSystem::get().playSFX3D( (_entity->name).c_str(), sound_file.c_str(), "", _eD->_transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f );

		return LEAVE;
	}

	//animacio de enfundar
	if( _unsheathed )
	{
		sound_file = "data/sfx/sheathe.wav";
		SoundSystem::get().playSFX3D( ("enemy_" + sound_file).c_str(), sound_file.c_str(), "", _eD->_transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f );
				

		sound_file = "data/sfx/voices/";
		std::string sound_type;
		std::string enemy_type = _animation_component->getEnemyType();


		if(getrandom(0,1) < 1)
			sound_type = "wind_";
		else
			sound_type = "nothing_";
		
		sound_file += sound_type + enemy_type + ".mp3";
		SoundSystem::get().playSFX3D( (_entity->name).c_str(), sound_file.c_str(), "", _eD->_transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f );
		
		
		_animation_component->executeAction("sheathe", 0.5f, 0.3f);
		_unsheathed = false;
	}
	else if( _animation_component->isFrameNumber("sheathe", 134) )
	{
		_animation_component->clearCycles(0);
		_animation_component->blendCycle("idle", 1.0f, 0.0f);
	}	
	else if( !_animation_component->actionOn("sheathe") )
	{
		return LEAVE;
	}

	return STAY;
}

int BTCommon::lookAt()
{
	//Nos vamos olvidando de lo que ibamos buscando...
	if(_eD->_forgetClock.hasTarget()) _eD->_forgetClock.count(_eD->_delta);

	if(!_eD->_lookAtClock.hasTarget()) //Si no hemos empezado a contar en ese lookAt...
	{
		//Rotar hacia el lookAt
	
		if(!_turning)	// si estamos empezando a girar lanzamos la animacion
		{
			float angle_out;
			_eD->_transformC->approximateFront_v(_eD->_lookAt, _eD->_rotateVel*_eD->_delta, angle_out);
			_animation_component->turn(angle_out);
			_turning = true;
		}
		else
		{
			_eD->_transformC->approximateFront_v(_eD->_lookAt, _eD->_rotateVel*_eD->_delta);
		}

		//Si ya miramos hacia donde sea activamos el crono para estar mirando un ratin
		btVector3 aux = _eD->_lookAt;  aux.setX(-aux.getX());
		if(_eD->_transformC->getFront().dot(aux) > 0.99)
			_eD->_lookAtClock.setTarget(_eD->_lookAtTime); // _lookAtTime siempre debe ser mayor que la duracion de la animacion desenfundar
	}
	else
	{   //Cuando el contador llegue al objetivo eliminamos el lookAt.
		if(_eD->_lookAtClock.count(World::instance()->getElapsedTimeUInSeconds())) _eD->_lookAt = btVector3(0,0,0);
		_turning = false;
	}

	return LEAVE;
}

int BTCommon::chooseLookAt()
{
	if(!_eD->_lookAts.size()) return LEAVE; //Si no hay lookAts salimos
	if(!_eD->_forgetClock.hasTarget()) //ya podemos dejar de buscar
	{
		_stoppedSearching = true;
		return LEAVE;
	}

	//Escoger lookAt.
	//_eD->chooseLookAt();
	_eD->_lookAt = _eD->_lookAts.at(rand()%_eD->_lookAts.size());

	return LEAVE;
}


//OTHERS//////////////////////////////////////////////////////////////////////////////////////////////////

void BTCommon::pathfind(const btVector3& point, std::deque<btVector3>& path)
{
	DijkstraGraph::get().computePath(_eD->_transformC->getPosition(), point, path);
}

void BTCommon::decrementVisionPercent()
{
	//Si no ve al player bajamos el porcentaje de vision
	if(_eD->_visionPercent > 0.0f)
	{
		_eD->_visionPercent -= 0.01f;
	}
	if(_eD->_visionPercent < 0.0f) _eD->_visionPercent = 0.0f;
}

void BTCommon::incrementVisionPercent(float factorBase)
{
	//Si el player esta caminando lo notaremos mas, y si corre mucho mas
	float current_speed_normalized = (PlayerControllerSystem::get().getCurrentSpeed() / (PlayerControllerSystem::get().getMaxSpeed()));
	float factorMovement = 1.0f;

	if(current_speed_normalized > 0.8f) //corriendo
		factorMovement = 4.0f;
	else if(current_speed_normalized > 0.2f) //caminando
		factorMovement = 2.0f;

	_eD->_visionPercent += factorBase*factorMovement;
	if(_eD->_visionPercent > 1.0f) _eD->_visionPercent = 1.0f;
}

