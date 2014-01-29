#include "bt_crow.h"
#include "dijkstra.h"
#include "entity_manager.h"
#include "component_particle_effect.h"
#include "entity_factory.h"
BTCrow::BTCrow(Entity* entity) : BehaviourTree(entity)
{
	create();
	init();

	_transformC = EntityManager::get().getComponent<TransformComponent>(entity);
	_crowVel = 0.2f;

	D3DXMATRIX vOffset_dx;
	D3DXMatrixTranslation(&vOffset_dx, 0, 0.7f, 0);
	convertD3DXMatrix(&vOffset_dx, vOffset_bt);
}

BTCrow::~BTCrow()
{
}

void BTCrow::init()
{
	//if(_particleCrow)	_particleCrow->stop();

	_pathToEnd.clear();
	_catmullIndex = 0.0f;
	reset();
}

void BTCrow::create()
{
	createRoot("crow", SEQUENCE, NULL, NULL);
		addChild("crow", "appear", ACTION, NULL, (btaction)&BTCrow::appear);
		addChild("crow", "rotate", ACTION, NULL, (btaction)&BTCrow::rotate);
		addChild("crow", "trackNextPoint", ACTION, NULL, (btaction)&BTCrow::trackNextPoint);
		addChild("crow", "disappear", ACTION, NULL, (btaction)&BTCrow::disappear);

			ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(_entity);
		Entity * p2 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(0,0,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_CROW,m);
		TransformComponent * t2 = EntityManager::get().getComponent<TransformComponent>(p2);
		_particleCrow = EntityManager::get().getComponent<ParticleEffectComponent>(p2);

		p2->name = "crow_particles";

		if (_particleCrow) _particleCrow->stop();
}

void BTCrow::render()
{
	if(!_entity->enabled) return;

	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 300, text_color, "crow state: %s", _action.c_str());

	unsigned p_color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
	DijkstraGraph::get().renderPath(_pathToEnd, p_color, 0.0f);

	//Transform
	p_color = D3DCOLOR_ARGB( 255, 255, 255, 0 );
	const btVector3& pos = _transformC->transform->getOrigin();
	btVector3 v_aux;
	_transformC->getLeftXinv(v_aux);
	drawLine_bt(pos, pos+v_aux, p_color);
	drawLine_bt(pos, pos+_transformC->getUp(), p_color);
	_transformC->getFrontXinv(v_aux);
	drawLine_bt(pos, pos+v_aux, p_color);
}

#include "entity_factory.h"

//ACCIONES
int BTCrow::appear()
{
	if(_pathToEnd.size() == 0)
	{
		if(!generatePathToEnd()) return PUSHSTATE;
	}

	//aparece cuervo con animacion de volar en el sitio, durante X segundos
	if(!_clock.hasTarget())
	{
		_clock.setTarget(1.0f);

		D3DXVECTOR3 p0, p1;

		D3DXVec3CatmullRom(&p0
						,&(D3DXVECTOR3)(_pathToEnd.at(0))
						,&(D3DXVECTOR3)(_pathToEnd.at(1))
						,&(D3DXVECTOR3)(_pathToEnd.at(2))
						,&(D3DXVECTOR3)(_pathToEnd.at(3))
						,0.0f);

		D3DXVec3CatmullRom(&p1
						,&(D3DXVECTOR3)(_pathToEnd.at(0))
						,&(D3DXVECTOR3)(_pathToEnd.at(1))
						,&(D3DXVECTOR3)(_pathToEnd.at(2))
						,&(D3DXVECTOR3)(_pathToEnd.at(3))
						,0.1f);

		convertDXVector3(p1-p0, _initFront);
		_initFront.normalize();
		convertDXVector3(p0, _initPos);

		createTransform(_pathToEnd.at(0)-_pathToEnd.at(1), _initPos); //Aparece en _initPos, mirando hacia el player (generacion de transform!!)
		
		//Efecto guapo de particulas de sombra
		Entity * ep1 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_transformC->getPosition()) + D3DXVECTOR3(0,-0.75f,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_EXPLOSION_CROW);
		//TransformComponent * t1 = EntityManager::get().getComponent<TransformComponent>(ep1);
		//t1->setParent(_transformC);

		if (_particleCrow) _particleCrow->play();
		//t2->setParent(_transformC);

		SoundSystem::get().playSFX3D("shadow_explosion", "data/sfx/whoosh1.ogg", "shadow_explosion", _pathToEnd.at(1), btVector3(0,0,0), false, 0.5f, 0.5f);

		// lanzar sfx aleteo cuervo
		SoundSystem::get().stopSound("crow",false,false);
		_sound = SoundSystem::get().playSFX3D("crow", "data/sfx/ravenloop.wav", "crow", _pathToEnd.at(1), btVector3(0,0,0), true, 0.0f, 1.0f);
	}
	
	if(_clock.count(World::instance()->getElapsedTimeUInSeconds()))
	{
		//Cuando acabe, ya puede trackear
		return LEAVE;
	}
	else
	{
		return STAY;
		//animacion de volar en el sitio
	}
}

int BTCrow::rotate()
{
	//dbg("init_pos:(%f,%f,%f)\n", _initPos.getX(), _initPos.getY(), _initPos.getZ());

	btVector3 lookAt = _initFront;
	lookAt.setX(-lookAt.getX());

	if(_transformC->getFront().dot(lookAt) < 0.99)
	{
		_transformC->approximateFront_v(_initFront, 0.05f);
	}
	
	if(_transformC->getFront().dot(lookAt) > 0.99)
	{
		_posBefore = _initPos;
		createTransform(_initFront, _initPos);
		return LEAVE;
	}
	
	return STAY;
}

int BTCrow::trackNextPoint()
{
	//Se ejecuta Catmull Rom para que el desplazamiento no sea brusco
	D3DXVECTOR3 new_pos;

	//Incrementamos indice
	if(World::instance()->getElapsedTimeUInSeconds() == 0.0f) return STAY;
	_catmullIndex += _crowVel*World::instance()->getElapsedTimeUInSeconds();
	//dbg("_catmullIndex: %f\n", _catmullIndex);
	int idx = (int)(_catmullIndex);

	//evaluar con Catmull Rom
	if(idx < _pathToEnd.size()-3)
	{
		D3DXVec3CatmullRom(&new_pos
							,&(D3DXVECTOR3)(_pathToEnd.at(idx))
							,&(D3DXVECTOR3)(_pathToEnd.at(idx+1))
							,&(D3DXVECTOR3)(_pathToEnd.at(idx+2))
							,&(D3DXVECTOR3)(_pathToEnd.at(idx+3))
							,_catmullIndex-idx);

		//Actualizar matriz
		btVector3 new_posBT;
		convertDXVector3(new_pos, new_posBT);

		btVector3 newFront = new_posBT - _posBefore;  newFront.normalize();
		//newFront.setX(-newFront.getX());

		_posBefore = new_posBT;

		btVector3 currFront; _transformC->getFrontXinv(currFront);

		newFront = 0.6f*currFront+0.4f*newFront;

		createTransform(newFront, new_posBT);

		SoundSystem::get().set3DPosition(_sound,_transformC->getPosition());

		return STAY;
	}
	else //hemos llegado
	{
		_catmullIndex = 0.0f;
		return LEAVE;
	}
}

int BTCrow::disappear()
{
	//Efecto guapo de particulas
	//return STAY misntras dure
		if (_particleCrow) 	_particleCrow->stop();

				//Efecto guapo de particulas de sombra
		Entity * ep1 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_transformC->getPosition()) + D3DXVECTOR3(0,-0.75f,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_EXPLOSION_CROW);
				SoundSystem::get().playSFX3D("shadow_explosion", "data/sfx/whoosh1.ogg", "shadow_explosion", _pathToEnd.at(1), btVector3(0,0,0), false, 0.5f, 0.5f);

		//if (_particleCrow) _particleCrow->play();

	SoundSystem::get().stopSound("crow",false,true);
	World::instance()->disableCrow();
	return LEAVE;
}

//OTROS

//Se genera el path del cuervo
bool BTCrow::generatePathToEnd()
{
	if (DijkstraGraph::get().navMeshAvailable() == false) return false;

	std::deque<btVector3> entirePath;
	_pathToEnd.clear();

	TransformComponent* TC_player = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer());

	//Pillamos path hasta el final
	DijkstraGraph::get().computePath(TC_player->getPosition(), DijkstraGraph::get().getFinalNodePos(), entirePath);

	if(entirePath.size() < 2)
	{
		World::instance()->disableCrow(); //ya estamos casi en el final--> desactivarCuervo!!!!!
		return false;
	}

	//Si se puede acceder al segundo nodo, nos cargamos el 1o
	if(!PhysicsSystem::get().checkCollision(TC_player->getPosition(), entirePath.at(1), PhysicsSystem::get().colMaskNavigation))
		entirePath.pop_front();

	//El inicio esta cerca del player, yendo hacia el 1er nodo
	btVector3 init_v = entirePath.at(0) - TC_player->getPosition(); init_v.normalize(); init_v*=0.7f;
		
	//Anyadiremos un offset vertical
	btVector3 v_offset(0, 1.0f, 0), v_aux;

	//El path del cuervo tendra 2 o 3 puntos (+2 de ayuda para el catmull): inicio, nodo 1, nodo 2 (si hay)
	_pathToEnd.push_back(TC_player->getPosition()+v_offset); //ayuda de inicio
	_pathToEnd.push_back(TC_player->getPosition()+init_v+v_offset); //punto de inicio
	_pathToEnd.push_back(entirePath.at(0)+v_offset); //nodo 1

	//Casos especiales
	if(entirePath.size() == 1) //ayudaI, inicio, nodo 1, ayudaN1
	{
		_pathToEnd.push_back(entirePath.at(0)+init_v+v_offset); //ayudaN1 inventada
	}
	else if(entirePath.size() == 2) //ayudaI, inicio, nodo 1, nodo2, ayudaN2
	{
		_pathToEnd.push_back(entirePath.at(1)); //nodo2
		v_aux = entirePath.at(1) - entirePath.at(0); v_aux.normalize();
		_pathToEnd.push_back(entirePath.at(1)+v_aux); //ayudaN2 inventada
	}
	else //ayudaI, inicio, nodo 1, nodo2, ayudaN2 (==nodo 3)
	{
		_pathToEnd.push_back(entirePath.at(1)+v_offset); //nodo2
		_pathToEnd.push_back(entirePath.at(2)+v_offset); //ayudaN2 (==nodo 3)
	}

	return true;
}

void BTCrow::createTransform(const btVector3& front_v, const btVector3& pos)
{
	btVector3 front = front_v;  front.setX(-front.getX()); front.normalize();
	btVector3 aux_up(0,1,0);
	btVector3 left = aux_up.cross(front);
	btVector3 up = front.cross(left);

	btMatrix3x3 B(  left.getX(),  left.getY(),  left.getZ(),
					up.getX(),    up.getY(),    up.getZ(),
					front.getX(), front.getY(), front.getZ());

	_transformC->transform->setBasis(B);
	_transformC->transform->setOrigin(pos);

	*_transformC->transform = vOffset_bt * *_transformC->transform;
}
