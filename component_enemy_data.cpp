#include "component_enemy_data.h"
#include "world.h"
#include "behaviour_tree.h"
#include "component_charcontroller.h"
#include "component_bt.h"
#include "component_transform.h"
#include "BTDeadEnemy.h"
#include "entity_manager.h"
#include "bt_gatekeeper.h"
#include "dijkstra.h"

EnemyDataComponent::EnemyDataComponent(Entity* e, TransformComponent* tc) : Component(e)
{
	_delta = 1.0f/60.0f; 
	_transformC = tc;
	_damageTaken = false;
	_searching = false;
	_crashNTurning = false;
	_lookedAtSoundSource = false;
	_playerSeen = false;
	_alerting = false;
	_alertThrown = false;
	_soundPlace = btVector3(0,0,0);
	_angleWithPlayer = -1.0f;
	_lastPlayerNode = -1;
	_eyePos = _transformC->getPosition()+eyeOffset_v;

	_life = 1.0f;
	_lookAt = btVector3(0,0,0);

	_attentionDegree = attentionDegrees::NORMAL;
	_visionPercent = 0.0f;
	_visionThreshold = 0.5f;
	_halfFov = D3DXToRadian(40.0f);
	_halfFovPeriferial = D3DXToRadian(70.0f);
	_periferialDistSq = 49.0f; //7m

	_visionDistSq_normal = 100.0f;	//10m
	_visionFactor_caution = 2.0f;
	_visionFactor_alert = 2.0f;
	_visionDistSq_playerInLight = 400.0f; //20m
	_visionDistSq_playerInShadows = 9.0f; //3m
	_hearDistSq = 100.0f; //10m
	_warDistMh = 1.5f;

	_rotateVel = 3.0f;
	_walkVel = 0.6f;
	_jogVel = 1.5f;
	_runVel = 3.0f;
	_currentVel = _walkVel;

	_lookAtTime = 4.0f; //ratillo que esta buscando al player cada vez que se gira (duracion mayor que animacoin desenfundar)
	_forgetTime = 10.0f; //tiempo que tarda en dejar de buscar al player
	_corpseLookTime = 5.0f; //Rato que se queda mirando un cadaver de un companyero (por 1a vez, luego baja)

	_visionDistSq = _visionDistSq_normal;
	_dbgColor = D3DCOLOR_ARGB(255, 255,0,0);

	_CToffsets.push_back(0.5f); _CToffsets.push_back(0.0f); _CToffsets.push_back(-0.5f);
	_CToffset = 1;

	_silent_kill = false;

	_enemyTexture = playerVisibility::VISIBLE;
	//_soundArmor = SoundSystem::get().playSFX3D(intToString(entity->eid) + "armor", "data/sfx/armadura.wav",intToString(entity->eid) + "armor",btVector3(0,0,0),btVector3(0,0,0),true,0.0f,1.0f,5.0f,10000);
}

EnemyDataComponent::~EnemyDataComponent(void)
{
}

void EnemyDataComponent::init()
{
	_damageTaken = false;
	_searching = false;
	_crashNTurning = false;
	_lookedAtSoundSource = false;
	_playerSeen = false;
	_alerting = false;
	_alertThrown = false;
	_soundPlace = btVector3(0,0,0);
	_angleWithPlayer = -1.0f;
	_eyePos = _transformC->getPosition()+eyeOffset_v; //?? Si se modifica antes que el transformComponent mal... al menos en el caso de los gatekeepers

	_lookAt = btVector3(0,0,0);

	_attentionDegree = attentionDegrees::NORMAL;
	_visionPercent = 0.0f;
	_visionThreshold = 0.5f;

	_currentVel = _walkVel;

	_visionDistSq = _visionDistSq_normal;
	_dbgColor = D3DCOLOR_ARGB(255, 255,0,0);

	_CToffsets.push_back(0.5f); _CToffsets.push_back(0.0f); _CToffsets.push_back(-0.5f);
	_CToffset = 1;

	_silent_kill = false;

	_enemyTexture = playerVisibility::VISIBLE;
}

void EnemyDataComponent::setAttentionDegree(attentionDegrees a_d)
{
	if(_attentionDegree == a_d) return; //Si ya tiene ese valor salimos.

	switch(a_d)
	{
		case attentionDegrees::NORMAL:
			_currentVel = _walkVel;
			_dbgColor = D3DCOLOR_ARGB(255, 255,0,0);
			_visionThreshold = 0.5f;
		break;
		case attentionDegrees::CAUTION:
			_currentVel = _jogVel;
			_dbgColor = D3DCOLOR_ARGB(255, 0,0,255);
			_visionThreshold = 0.25f;
		break;
		case attentionDegrees::PERMANENT_CAUTION:
			_currentVel = _jogVel;
			_dbgColor = D3DCOLOR_ARGB(255, 0, 255,255);
			_visionThreshold = 0.25f;
		break;
		case attentionDegrees::ALERT:
			_currentVel = _runVel;
			_dbgColor = D3DCOLOR_ARGB(255, 255,0,0);
		break;
		case attentionDegrees::PANIC:
			_dbgColor = D3DCOLOR_ARGB(255, 255,255,255);
		break;
		default: return; //No deberia pasar jamas, pero bueno.
	}

	_attentionDegree = a_d;
}

//Comprueba si hay algo a "dist" distancia del enemy en las 4 direcciones del eje XZ.
//_lookAts acaba con 4 vectores: (front,left,back,right). Si en alguna direccion hay obstaculo el vector es (0,0,0)
void EnemyDataComponent::setLookAts(float dist)
{
	_lookAts.clear();
	btVector3 from, to, f, l, b, r;
	from = _transformC->getPosition();

	//front
	_transformC->getFrontXinv(f);
	to = from + dist*f;
	if(!PhysicsSystem::get().checkCollision(from, to, PhysicsSystem::get().colMaskVision))
		_lookAts.push_back(f);
	//else _lookAts.push_back(btVector3(0,0,0));

	//left
	_transformC->getLeftXinv(l);
	to = from + dist*l;
	if(!PhysicsSystem::get().checkCollision(from, to, PhysicsSystem::get().colMaskVision))
		_lookAts.push_back(l);
	//else _lookAts.push_back(btVector3(0,0,0));

	//back
	b = -f;
	to = from - dist*b;
	if(!PhysicsSystem::get().checkCollision(from, to, PhysicsSystem::get().colMaskVision))
		_lookAts.push_back(b);
	//else _lookAts.push_back(btVector3(0,0,0));

	//right
	r = -l;
	to = from - dist*r;
	if(!PhysicsSystem::get().checkCollision(from, to, PhysicsSystem::get().colMaskVision))
		_lookAts.push_back(r);
	//else _lookAts.push_back(btVector3(0,0,0));

	_currentLookAtId = 0; //front
}

//Escoge un lookat de los posibles, capado para girar solo 90 grados
void EnemyDataComponent::chooseLookAt()
{
	int leftId, rightId;
	leftId = _currentLookAtId + 1;
	if(leftId > 3) leftId = 0; //_lookAts siempre tiene ahora 4 posiciones
	rightId = _currentLookAtId - 1;
	if(rightId < 0) rightId = 3;

	bool canTurnL, canTurnR;
	canTurnL = _lookAts.at(leftId) != btVector3(0,0,0);
	canTurnR = _lookAts.at(rightId) != btVector3(0,0,0);

	if(canTurnL && canTurnR) //lo + probable. Escoger lado
	{
		float random = rand()%2;
		if(random < 0.5f) _currentLookAtId = leftId;
		else _currentLookAtId = rightId;
	}
	else if(canTurnL && !canTurnR) //solo izq
	{
		_currentLookAtId = leftId;
	}
	else if(!canTurnL && canTurnR) //solo der
	{
		_currentLookAtId = rightId;
	}

	_lookAt = _lookAts.at(_currentLookAtId);

}


//Asesinato silencioso. Modificamos las variables necesarias para el bTree
void EnemyDataComponent::silentKill()
{
	_life = 0.0f;
	_silent_kill = true;

	BTComponent *btc = EntityManager::get().getComponent<BTComponent>(entity);
	AnimationComponent *ac = EntityManager::get().getComponent<AnimationComponent>(entity);
	EnemyDataComponent* edc = EntityManager::get().getComponent<EnemyDataComponent>(entity);

	btc->getBT()->pushState("die"); //Pasar al estado de "muriendo"

	// Dependiendo del estado del enemigo lanza animacion con o sin espada en la mano //fix guarreria de edu		
	if(edc->_attentionDegree == attentionDegrees::NORMAL || edc->_attentionDegree == attentionDegrees::PANIC)
		btc->getBT()->killAnimationName = ac->getSilentKillAnimationName();
	else if(edc->_attentionDegree == attentionDegrees::CAUTION || edc->_attentionDegree == attentionDegrees::PERMANENT_CAUTION || edc->_attentionDegree == attentionDegrees::ALERT)
		btc->getBT()->killAnimationName = ac->getSilentKillAnimationName() + "_sword";

	return;
}

//Se llama cuando un malo muere, por combate o asesinato silencioso
void EnemyDataComponent::dead()
{
	//Esto no deberia llamarse cuando esta muerto no? que sentido tiene que la sombra del malo desaparezca al morir?
	//EntityManager::get().getComponent<TransformComponent>(entity)->destroyChildren();

	////Desactivar colisiones (fisicas...) del malo
	//CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(entity);
	//PhysicsSystem::get().getDynamicsWorld()->removeCollisionObject(ccC->controller->getGhostObject());
	//PhysicsSystem::get().getDynamicsWorld()->removeAction(ccC->controller);

	//Desactivar el enemy data component, convencion para decir que esta muerto
	enabled = false;
	//SoundSystem::get().stopSound(_soundArmor->id,false,false);

	//Si es un gatekeeper y tiene a otro para hablar, decirle que avise al otro de que ya no tiene interlocutor :(
	if(dynamic_cast<BTGatekeeper*>(EntityManager::get().getComponent<BTComponent>(entity)->getBT()))
	{
		((BTGatekeeper*)(EntityManager::get().getComponent<BTComponent>(entity)->getBT()))->setAllyToTalk(NULL, false, true);
	}

	//Le cambiamos el behaviour tree al tio por el de un cadaver
	EntityManager::get().getComponent<BTComponent>(entity)->changeBT(new BTDeadEnemy(entity, _transformC->getPosition()));
}

//Desplaza la posicion del malo segun el vector de entrada y modifica la posicion de los "ojos"
void EnemyDataComponent::advance(const btVector3& v_delta)
{
	_transformC->move(v_delta);
	_eyePos = _transformC->getPosition()+eyeOffset_v;
}

//Va al siguiente punto de path, con optimizaciones y eso
void EnemyDataComponent::goToNextTrackPoint(float delta, std::deque<btVector3>& path)
{
	//Test para saber si podemos eliminar directamente el siguiente punto de path
	if(path.size() > 1 && !_crashNTurning)
	{
		//Test extra: comprobar si el vector que va desde el enemy al siguiente point es muy diferente del que va del enemy al siguiente del siguiente
		btVector3 v1, v2;
		const btVector3& enemyPos = _transformC->getPosition();
		const btVector3& nextNode = path.at(1);
		v1 = path.at(0) - enemyPos; v1.normalize();
		v2 = nextNode - enemyPos; v2.normalize();

		if(v1.dot(v2) < 0.85f)
		{
			//Lanzamos rayo al 2o punto mas alla
			if(!PhysicsSystem::get().checkCollision(enemyPos, nextNode, PhysicsSystem::get().colMaskNavigation))
				path.pop_front(); //Si hay via libre nos deshacemos del punto intermedio
		}
	}

	//Rotar hacia el punto
	float angle_to_point;
	_transformC->approximateFront_p(path.at(0), _rotateVel*delta, angle_to_point);
	//Si aun on hemos rotado lo suficiente para correr, salimos
	if(angle_to_point > minAngleToRun) return;

	//Crash&Turn hasta el siguiente punto, si no se esta haciendo ya
	if(!_crashNTurning)
	{
		if(crashNTurn(path))
		{
			_crashNTurning = true;
		}
	}

	//Si llevamos chashAndTurnando demasiado rato volvemnos a calcular ruta, pq somos muuuu tontos
	if(_crashNTurning)
	{
		if(_crashAndTurningCounter.count(World::instance()->getElapsedTimeUInSeconds()))
		{
			DijkstraGraph::get().computePath(_transformC->getPosition(), path.at(path.size()-1), path);
			//dbg("demasiado C&R, recalculo.\n");
		}
	}
	else
	{
		_crashAndTurningCounter.setTarget(3.0f);
	}

	//Correr hacia el punto
	advance(_transformC->getFront()*delta*_currentVel);

	//Si llegamos al punto lo sacamos de la cola de prioridad
	float minDistToPoint = 1.0f;
	if(_attentionDegree == attentionDegrees::ALERT) minDistToPoint = 1.5f;
	if(manhattanDist(_transformC->getPosition(), path.at(0)) < minDistToPoint)
	{
		path.pop_front();
		if(_crashNTurning) _crashNTurning = false;
	}
}

//Intenta encontrar un punto intermedio hasta el siguiente punto, en caso que no se pueda llegar a el directamente
//Devuelve true si se ha tenido que modificar la ruta. else false
bool EnemyDataComponent::crashNTurn(std::deque<btVector3>& path)
{
	//Anyadimos un offset en el eje X que va variando segun el update, para simular la anchura del pavo
	float currentOffset = _CToffsets.at(_CToffset);
	_CToffset++;  if(_CToffset>2) _CToffset=0;

	btVector3 side;  _transformC->getLeftXinv(side); 
	const btVector3& enemyPos = _transformC->getPosition() + currentOffset*side;
	const btVector3& destiny = path.at(0);

	//0: via libre hasta destiny?
	if(!PhysicsSystem::get().checkCollision(enemyPos, destiny, PhysicsSystem::get().colMaskNavigation))
			return false;

	btVector3 testPoint;
	float rayDist = 2.0f;

	btVector3 front;
	_transformC->getFrontXinv(front);

	//1: comprobamos si tenemos algun obstaculo delante
	testPoint = enemyPos + front*rayDist;
	if(!PhysicsSystem::get().checkCollision(enemyPos, testPoint, PhysicsSystem::get().colMaskNavigation))
		return false;

	//2:Caso rampa
	testPoint = enemyPos + front*rayDist;
	if(!PhysicsSystem::get().checkCollision(enemyPos, testPoint, PhysicsSystem::get().colMaskNavigation))
		return false;

	btVector3 left, candidate, dir;
	candidate = btVector3(0,0,0); //convencion de "NULL"
	_transformC->getLeftXinv(left);

	//3: 45g a la izq segun front
	dir = front+left; dir.normalize();
	testPoint = enemyPos + dir*rayDist;

	if(!PhysicsSystem::get().checkCollision(enemyPos, testPoint, PhysicsSystem::get().colMaskNavigation))
	{
		candidate = testPoint; //Guardamos el punto como candidato
		//Lanzamos otro rayo hasta el destino para asegurarnos de que se puede llegar a destiny sin problemas
		if(!PhysicsSystem::get().checkCollision(testPoint, destiny, PhysicsSystem::get().colMaskNavigation))
		{   //Si no hay obstaculo se anyade el testpoint como punto del path
			path.push_front(testPoint);
			return true;
		}
	}

	btVector3 right = -left;

	//4: 45g a la der segun front
	dir = front+right; dir.normalize();
	testPoint = enemyPos + dir*rayDist;

	if(!PhysicsSystem::get().checkCollision(enemyPos, testPoint, PhysicsSystem::get().colMaskNavigation))
	{
		candidate = testPoint; //Guardamos el punto como candidato
		//Lanzamos otro rayo hasta el destino para asegurarnos de que se puede llegar a destiny sin problemas
		if(!PhysicsSystem::get().checkCollision(testPoint, destiny, PhysicsSystem::get().colMaskNavigation))
		{   //Si no hay obstaculo se anyade el testpoint como punto del path
			path.push_front(testPoint);
			return true;
		}
	}

	//6: izq
	testPoint = enemyPos + left*rayDist;

	if(!PhysicsSystem::get().checkCollision(enemyPos, testPoint, PhysicsSystem::get().colMaskNavigation))
	{
		candidate = testPoint; //Guardamos el punto como candidato
		//Lanzamos otro rayo hasta el destino para asegurarnos de que se puede llegar a destiny sin problemas
		if(!PhysicsSystem::get().checkCollision(testPoint, destiny, PhysicsSystem::get().colMaskNavigation))
		{   //Si no hay obstaculo se anyade el testpoint como punto del path
			path.push_front(testPoint);
			return true;
		}
	}

	//7: der
	testPoint = enemyPos + right*rayDist;

	if(!PhysicsSystem::get().checkCollision(enemyPos, testPoint, PhysicsSystem::get().colMaskNavigation))
	{
		candidate = testPoint; //Guardamos el punto como candidato
		//Lanzamos otro rayo hasta el destino para asegurarnos de que se puede llegar a destiny sin problemas
		if(!PhysicsSystem::get().checkCollision(testPoint, destiny, PhysicsSystem::get().colMaskNavigation))
		{   //Si no hay obstaculo se anyade el testpoint como punto del path
			path.push_front(testPoint);
			return true;
		}
	}

	//Si en este punto no se ha encontrado una via libre, anyadimos candidate como punto del path, si tiene algun valor
	if(candidate != btVector3(0,0,0))	
	{
		path.push_front(candidate);
		return true;
	}
	else //Si resulta que no hay candidato, pues nos hacemos los ciegos y rezamos por llegar a algun sitio.
	{
		//dbg("C&T no encuentra ruta\n");
		return false;
	}

	//DijkstraGraph::get().computePath(_transformC->getPosition(), path.at(path.size()-1), path);

	//No se puede acceder de forma segura al siguiente punto del path. Caminamos a ciegas hasta el siguiente punto confiando en las colisiones.
	//Si estamos mucho rato haciendo el canelo ya saltara el if de crashNTurning mucho rato seguido
	
	//return true;
	
}

void EnemyDataComponent::renderPath(unsigned color, float offset)
{
	if(!_path.size()) return;

	btVector3 up = btVector3(0,offset,0); //Desplazamos arriba un poco para que no se solape con el grafo

	for(unsigned i=0; i<_path.size()-1; i++)
	{
		drawLine_bt(_path.at(i)+up, _path.at(i+1)+up, color);
	}
}
