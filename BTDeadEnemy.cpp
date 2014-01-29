#include "BTDeadEnemy.h"
#include "d3ddefs.h"
#include "component_bt.h"
#include "component_automat.h"
#include "aut_player.h"
#include "entity_manager.h"
#include "system_bt.h"
#include "system_shadow.h"
#include "component_enemy_data.h"

BTDeadEnemy::BTDeadEnemy(Entity* entity, const btVector3& pos) : BehaviourTree(entity)
{
	BTSystem::get().addDeadEnemy(entity);
	_posBT = pos;
	convertBulletVector3(&pos, _posDX);
	_immerseCount.setTarget(5.0f);

	_animation_component->enabled = false;
	//Pruebas para detener las animaciones de este enemigo
	//EntityManager::get().getComponent<ModelComponent>(entity)->getCModel()->getCoreModel()->removeCoreAnimation();
	//model->getCModel()->getCoreModel()->

	create();
}

BTDeadEnemy::~BTDeadEnemy(void)
{
}

void BTDeadEnemy::create()
{
	createRoot("dead_enemy", PRIORITY, NULL, NULL);
	addChild("dead_enemy", "immerse_and_delete", SEQUENCE, (btcondition)&BTDeadEnemy::checkInMagicShadow, NULL);
	addChild("immerse_and_delete", "immerse", ACTION, NULL, (btaction)&BTDeadEnemy::immerse);
	addChild("immerse_and_delete", "autoDelete", ACTION, NULL, (btaction)&BTDeadEnemy::autoDelete);
	addChild("dead_enemy", "idle", ACTION, NULL, (btaction)&BTDeadEnemy::idle);
}

void BTDeadEnemy::render()
{
	//Transform
	unsigned p_color = D3DCOLOR_ARGB( 255, 255, 255, 0 );
	EnemyDataComponent* _eD = EntityManager::get().getComponent<EnemyDataComponent>(_entity);
	const btVector3& pos = _eD->_transformC->transform->getOrigin();
	btVector3 v_aux;
	_eD->_transformC->getLeftXinv(v_aux);
	drawLine_bt(pos, pos+v_aux, p_color);
	drawLine_bt(pos, pos+_eD->_transformC->getUp(), p_color);
	_eD->_transformC->getFrontXinv(v_aux);
	drawLine_bt(pos, pos+v_aux, p_color);

	//unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	//printf2D( g_App.GetWidth()*3/5, 190, text_color, "enemy state: %s", _action.c_str());
}

//Conditions
bool BTDeadEnemy::checkInMagicShadow()
{
	if( ShadowSystem::get().checkPosInShadows(_posDX, 0.5f) )
	{
		std::string sound_file = "data/sfx/absorb.ogg";
		SoundSystem::get().playSFX3D( ("enemy_" + sound_file).c_str(), sound_file.c_str(), "", _posBT, btVector3(0,0,0), false, 1.0f, 1.0f );

		return true;
	}
	return false;
}

//Actions
int BTDeadEnemy::immerse()
{
	if(_immerseCount.count(1.0f/60.0f)) return LEAVE;
	else
	{
		_posBT.setY(_posBT.getY()-0.002f);
		EntityManager::get().getComponent<TransformComponent>(_entity)->setPosition(_posBT);

		//Para mover el modelo, actualizamos la posicion del mixer pero congelando las animaciones (only_position = true)		
		EntityManager::get().getComponent<AnimationComponent>(_entity)->update(1.0f/60.0f, true);

		return STAY;
	}
}

int BTDeadEnemy::autoDelete()
{
	BTSystem::get().destroyEnemy(_entity);
	return LEAVE;
}

int BTDeadEnemy::idle()
{
	return LEAVE;
}
