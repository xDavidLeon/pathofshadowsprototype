#include "system_bt.h"
#include <map>
#include "entity.h"
#include "component.h"
#include "entity_manager.h"
#include "component_bt.h"
#include "dijkstra.h"
#include "world.h"
#include "component_enemy_data.h"
#include "component_model.h"
#include "system_light.h"
#include "component_automat.h"
#include "bt_gatekeeper.h"

BTSystem::BTSystem(void)
{
	_playerNode = -1;
	_visibilityClock.setTarget(0.5f);
	_panicButtonPressed = _panicButtonPressedBefore = false;
}

BTSystem::~BTSystem(void)
{
	release();
}

void BTSystem::update(float delta)
{
	//Se eliminan las entidades en el vector de entidades a borrar
	removeEntities();

	//Se eliminan los bts en el vector de bts a borrar
	removeBTs();

	//Este if sirve para que el boton del panico dure a true exactamente un update de BTSystem
	if(_panicButtonPressed)
	{
		if(_panicButtonPressedBefore) //Si en el update anterior estaba pulsado el boton del panico, lo despulsamos
		{
			_panicButtonPressed = _panicButtonPressedBefore =  false;
		}
		else _panicButtonPressedBefore = true;
	}

	//Se obtienen todas las entidades con componente BT (Behaviour Tree)
	std::map<Entity*,Component*>* entitiesWithBT = EntityManager::get().getAllEntitiesPosessingComponent<BTComponent>();
	if(!entitiesWithBT) return;

	//Se actualiza la id del nodo de navegacion mas cercano al player
	_playerNode = DijkstraGraph::get().getAccessibleNode(EntityManager::get().getPlayerPos());

	//Se comprueba si hay que actualizar texturas
	//bool updateTextures = _visibilityClock.count(delta);

	//Se llama el update de los componentes BT
	std::map<Entity*,Component*>::iterator iter;
	//#pragma omp parallel for
	for (iter = entitiesWithBT->begin(); iter != entitiesWithBT->end(); ++iter)
	{
		if(!iter->first->enabled || !iter->second->enabled) continue;
		
		iter->second->update(delta);

		//Se actualiza la textura de los malos, segun si estan en sombra estatic o no
		updateTexture(iter->first, delta);

		//if (iter->first->type == "ENEMY")
		//{
		//	EnemyDataComponent * dataComponent = EntityManager::get().getComponent<EnemyDataComponent>(iter->first);
		//	TransformComponent * tComponent = EntityManager::get().getComponent<TransformComponent>(iter->first);
		//	btVector3 p = tComponent->getPosition();
		//	FMOD_VECTOR pos = { p.getX(),p.getY(),p.getZ()};
		//	FMOD_VECTOR zero = { 0,0,0};
		//	FMOD_Channel_Set3DAttributes(dataComponent->_soundArmor->channel,&pos,&zero);
		//}
	}
}

void BTSystem::render()
{
	//if (World::instance()->isDebugModeOn() == false) return;

	//Se obtienen todas las entidades con componente BT (Behaviour Tree)
	std::map<Entity*,Component*>* entitiesWithBT = EntityManager::get().getAllEntitiesPosessingComponent<BTComponent>();
	if(!entitiesWithBT) return;

	//Se llama el update de los componentes BT
	std::map<Entity*,Component*>::iterator iter;
	for (iter = entitiesWithBT->begin(); iter != entitiesWithBT->end(); ++iter)
	{
		if(iter->second->enabled) ((BTComponent*)iter->second)->render();
	}
}

void BTSystem::addSbbWarning(Entity* entity, const btVector3& pos)
{
	btVector3* p_pos = new btVector3(pos);	//Creamos puntero a vector pq lo que nos viene es const

	if(_sbbWarnings.find(entity) != _sbbWarnings.end()) //Si la entidad ya ha avisado...
	{
		_sbbWarnings[entity] = p_pos; //...pone la nueva posicion
	}
	else //Si no hay aviso de esa entidad...
	{
		_sbbWarnings.insert(std::pair<Entity*, btVector3*>(entity, p_pos)); //..lo anyade
	}
}

void BTSystem::removeSbbWarning(Entity* entity)
{
	if(_sbbWarnings.find(entity) != _sbbWarnings.end()) //Si hay aviso de esa entidad...
	{
		delete _sbbWarnings.at(entity); //eliminamos contenido del puntero de la pos
		_sbbWarnings.erase(entity); //...lo borramos
	}
}

void BTSystem::addDeadEnemy(Entity* corpse)
{
	if(_deadEnemies.find(corpse) == _deadEnemies.end())	_deadEnemies.insert(corpse);
}

void BTSystem::eraseDeadEnemy(Entity* corpse)
{
	if(_deadEnemies.find(corpse) != _deadEnemies.end())	_deadEnemies.erase(corpse);
}

void BTSystem::addEntityToRemove(Entity* toRemove)
{
	if(_toRemove.find(toRemove) == _toRemove.end()) _toRemove.insert(toRemove);
}

//Prepara la entidad (enemigo) para ser destruida en el siguiente update de BTSystem
void BTSystem::destroyEnemy(Entity* enemy)
{
	//Desactivar su BT
	EntityManager::get().getComponent<BTComponent>(enemy)->enabled = false;

	//Sacar de vector de cadaveres (si no esta no pasa nada) y meter en el de entidades por borrar
	BTSystem::get().eraseDeadEnemy(enemy);
	//BTSystem::get().addEntityToRemove(enemy); //Segun Edu esto pega un tironaco que queda feo... vamos a desactivar la entity a ver que pasa
	enemy->enabled = false;
	//...en algunos sitios la convencion de "enemigo muerto" es enemyDataComponent==disabled, asi que toco eso tb
	EntityManager::get().getComponent<EnemyDataComponent>(enemy)->enabled = false;
	//Lo siguiente es guarrillo. Los malos siguen teniendo sombra cuando se "eliminan", puesto que no se eliminan de verdad.
	//Siendo asi los transportamos al inframundo y nos aseguramos que el resto de veces nazcan ahi
	EntityManager::get().getComponent<TransformComponent>(enemy)->transform->setOrigin(btVector3(0,-1000,0));
	EntityManager::get().getComponent<TransformComponent>(enemy)->setInitTransform(*EntityManager::get().getComponent<TransformComponent>(enemy)->transform);

	//Si es un gatekeeper (se le ha matado con shadow kill) y tiene a otro para hablar, decirle que avise al otro de que ya no tiene interlocutor :(
	if(dynamic_cast<BTGatekeeper*>(EntityManager::get().getComponent<BTComponent>(enemy)->getBT()))
	{
		((BTGatekeeper*)(EntityManager::get().getComponent<BTComponent>(enemy)->getBT()))->setAllyToTalk(NULL, false, true);
		//Si ha entrado aqui es que aun tiene BT. Lo eliminamos
		EntityManager::get().getComponent<BTComponent>(enemy)->changeBT(NULL);
		//Y por si acaso...
		EntityManager::get().getComponent<BTComponent>(enemy)->enabled = false;
	}

	// PENSAR SI HAY UNA MEJOR SOLUCION, FORZAMOS RECALCULAR TARGET POR SI ES EL QUE SE VA A ELIMINAR
	((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->getAutomat())->_target = NULL;
}

void BTSystem::removeEntities()
{
	std::set<Entity*>::iterator toRemove;
	while(_toRemove.size())
	{
		toRemove = _toRemove.begin();
		EntityManager::get().removeEntity(*toRemove);
		_toRemove.erase(toRemove);
	}
}

void BTSystem::addBTToRemove(BehaviourTree* toRemove)
{
	if(_BTtoRemove.find(toRemove) == _BTtoRemove.end()) _BTtoRemove.insert(toRemove);
}

void BTSystem::removeBTs()
{
	std::set<BehaviourTree*>::iterator toRemove;
	while(_BTtoRemove.size())
	{
		toRemove = _BTtoRemove.begin();
		delete *toRemove;
		_BTtoRemove.erase(toRemove);
	}
}

void BTSystem::updateTexture(Entity* entity, float delta)
{
	if(entity->type == "CROW") return;
	if(entity->type == "CINE_SEQ") return;
	if(entity->type == "GODDESS") return;

	EnemyDataComponent* edC = EntityManager::get().getComponent<EnemyDataComponent>(entity);
	const btVector3& enemyPos = EntityManager::get().getComponent<TransformComponent>(entity)->getPosition();
	playerVisibility textureNow;

	//comprobamos si esta al alcance de alguna pointLight
	if(LightSystem::get().posInPointLight(enemyPos, 3.0f))
	{
		textureNow = playerVisibility::ILLUMINATED;
	}
	//Las sombras magicas no afectan a los malos. Por eso son magicas.

	//Comprobamos si la luz de la luna le alcanza
	else if(LightSystem::get().posInDirectionalLight(enemyPos + btVector3(0,-0.6f,0)))
			textureNow = playerVisibility::VISIBLE;
	else
			textureNow = playerVisibility::ONSHADOW;

	//if(textureNow == edC->_enemyTexture) return; //Si no ha cambiado salimos.
	edC->_enemyTexture = textureNow;

	// Cambiamos textura pj
	ModelComponent * modelC = EntityManager::get().getComponent<ModelComponent>(entity);
	float difSpeed = 3;

	switch (textureNow)
	{
	case playerVisibility::ILLUMINATED:
		modelC->setCurrentMaterialsName("tech_skin_dirLight");
		modelC->diffuseColor += D3DXVECTOR4(+delta*difSpeed,+delta*difSpeed,+delta*difSpeed,0);
		if (modelC->diffuseColor.y > 1.0f) modelC->diffuseColor = D3DXVECTOR4(1,1,1,1.0f);
		break;
	case playerVisibility::VISIBLE:
		modelC->setCurrentMaterialsName("tech_skin_dirLight");
		modelC->diffuseColor += D3DXVECTOR4(+delta*difSpeed,+delta*difSpeed,+delta*difSpeed,0);
		if (modelC->diffuseColor.y > 1.0f) modelC->diffuseColor = D3DXVECTOR4(1,1,1,1.0f);
		break;
	case playerVisibility::ONSHADOW:
		modelC->setCurrentMaterialsName("tech_skin");
		modelC->diffuseColor += D3DXVECTOR4(-delta*difSpeed,-delta*difSpeed,-delta*difSpeed,0);
		if (modelC->diffuseColor.y < 0.25f) modelC->diffuseColor = D3DXVECTOR4(0.25f,0.25f,0.25f,1.0f);
		break;
	}
}

void BTSystem::release()
{
	_sbbWarnings.clear();
	_deadEnemies.clear();
	_toRemove.clear();
	_BTtoRemove.clear();
}
