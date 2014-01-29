#include "bt_goddess.h"
#include "entity_manager.h"
#include "world.h"
#include "entity_factory.h"

BTGoddess::BTGoddess(Entity* entity) : BehaviourTree(entity)
{
	create();

	_goddessModel = _crowModel = NULL;
	_crowTransformC = _goddessTransformC = NULL;
	_hasToGo = _isBorn = _hasToChangePlace = _hasToBeBorn = _hasToGivePowers = false;
	_rotVel = 0.03f;

	_respawns.push_back("g001");
	_respawns.push_back("g002");

	_lookAt = btVector3(0,0,0);
}


BTGoddess::~BTGoddess(void)
{
}

void BTGoddess::create()
{
	createRoot("goddess", PRIORITY, NULL, NULL);
		addChild("goddess", "poof", ACTION, (btcondition)&BTGoddess::checkHasToGo, (btaction)&BTGoddess::poof);
		addChild("goddess", "teaching", PRIORITY, (btcondition)&BTGoddess::checkIsBorn, NULL);
			addChild("teaching", "givePowers", ACTION, (btcondition)&BTGoddess::checkHasToGivePowers, (btaction)&BTGoddess::givePowers);
			addChild("teaching", "change_place", SEQUENCE, (btcondition)&BTGoddess::checkHasToChangePlace, NULL);
				addChild("change_place", "dissappear", ACTION, NULL, (btaction)&BTGoddess::dissappear);
				addChild("change_place", "appear", ACTION, NULL, (btaction)&BTGoddess::appear);
			addChild("teaching", "idleGoddess", ACTION, NULL, (btaction)&BTGoddess::idleGoddess);
		addChild("goddess", "waiting", PRIORITY, NULL, NULL);
			addChild("waiting", "born_seq", SEQUENCE, (btcondition)&BTGoddess::checkHasToBeBorn, NULL);
				addChild("born_seq", "flyToPlace", ACTION, NULL, (btaction)&BTGoddess::flyToPlace);
				addChild("born_seq", "beBorn", ACTION, NULL, (btaction)&BTGoddess::beBorn);
			addChild("waiting", "idleCrow", ACTION, NULL, (btaction)&BTGoddess::idleCrow);
}

void BTGoddess::render()
{
	unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
	printf2D( g_App.GetWidth()*3/5, 370, text_color, "goddess state: %s", _action.c_str());

	//unsigned p_color = D3DCOLOR_ARGB( 255, 255, 255, 255 );
	//DijkstraGraph::get().renderPath(_pathToEnd, p_color, 0.0f);

	////Transform
	//p_color = D3DCOLOR_ARGB( 255, 255, 255, 0 );
	//const btVector3& pos = _transformC->transform->getOrigin();
	//btVector3 v_aux;
	//_transformC->getLeftXinv(v_aux);
	//drawLine_bt(pos, pos+v_aux, p_color);
	//drawLine_bt(pos, pos+_transformC->getUp(), p_color);
	//_transformC->getFrontXinv(v_aux);
	//drawLine_bt(pos, pos+v_aux, p_color);
}

void BTGoddess::setEntitites(Entity* crow, Entity* goddess)
{
	_crowModel = crow;
	_goddessModel = goddess;

	_crowTransformC = EntityManager::get().getComponent<TransformComponent>(_crowModel);
	_goddessTransformC = EntityManager::get().getComponent<TransformComponent>(_goddessModel);

	EntityManager::get().getComponent<AnimationComponent>(_crowModel)->blendCycle("idle", 1.0f, 0.0f);
	EntityManager::get().getComponent<AnimationComponent>(_goddessModel)->blendCycle("flying", 1.0f, 0.0f);

}

void BTGoddess::setLookat(const btVector3& la)
{
	_lookAt = la - _goddessTransformC->getPosition();
	_lookAt.setY(0.0f); _lookAt.normalize();
}

void BTGoddess::goddessBorn()
{
	_hasToBeBorn = true;
}

void BTGoddess::changePlace()
{
	_hasToChangePlace = true;
}

void BTGoddess::givePowersToPlayer()
{
	_hasToGivePowers = true;
}

void BTGoddess::hasToGo()
{
	_hasToGo = true;
}

////////////////////////////////////Condiciones
bool BTGoddess::checkHasToGo()
{
	return _hasToGo;
}

bool BTGoddess::checkIsBorn()
{
	return _isBorn;
}

bool BTGoddess::checkHasToGivePowers()
{
	if(_hasToGivePowers)
	{
		_hasToGivePowers = false;
		return true;
	}
	else return false;
}

bool BTGoddess::checkHasToChangePlace()
{
	if(_hasToChangePlace)
	{
		_hasToChangePlace = false;
		if(_respawns.size()) return true;
		else return false;
	}
	else return false;
}

bool BTGoddess::checkHasToBeBorn()
{
	return _hasToBeBorn;
}


////////////////////////////////////Acciones
int BTGoddess::poof()
{
	if( EntityManager::get().getComponent<AnimationComponent>(_goddessModel)->actionBlocked("disappear") )
	{
		_goddessModel->enabled = false;
		Entity * p1 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_goddessTransformC->getPosition()),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_EXPLOSION);
	}
	return LEAVE;
}

int BTGoddess::givePowers()
{
	return LEAVE;
}

int BTGoddess::dissappear()
{
	//Hacer desaparecer la mesh, efecto molon particulas
	if( EntityManager::get().getComponent<AnimationComponent>(_goddessModel)->actionBlocked("disappear") )
	{
		Entity * p1 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_goddessTransformC->getPosition()),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_EXPLOSION);

		TransformComponent* tc = EntityManager::get().getComponent<TransformComponent>(_goddessModel);
		//Get respawn
		const btTransform *rpT = EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->getRespawn(_respawns.at(0).c_str());
		_respawns.pop_front();
		//Set transform con el respawn
		EntityManager::get().getComponent<TransformComponent>(_goddessModel)->transform->setBasis(rpT->getBasis());
		EntityManager::get().getComponent<TransformComponent>(_goddessModel)->transform->setOrigin(rpT->getOrigin());

		return LEAVE;
	}
	return STAY;
}

int BTGoddess::appear()
{
	//Hacer aparecer la mesh, efecto molon particulas
	//Aparecer en _respawns.at(0)

	//TransformComponent* tc = EntityManager::get().getComponent<TransformComponent>(_goddessModel);
	////Get respawn
	//const btTransform *rpT = EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->getRespawn(_respawns.at(0).c_str());
	//_respawns.pop_front();
	////Set transform con el respawn
	//EntityManager::get().getComponent<TransformComponent>(_goddessModel)->transform->setBasis(rpT->getBasis());
	//EntityManager::get().getComponent<TransformComponent>(_goddessModel)->transform->setOrigin(rpT->getOrigin());

	return LEAVE;
}

int BTGoddess::idleGoddess()
{
	if(_lookAt == btVector3(0,0,0))
	{
		//idle de la diosa. Lo unico que hace es encararse al player
		btVector3 toPlayer =  EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->getPosition() - _goddessTransformC->getPosition();
		toPlayer.normalize();
		_goddessTransformC->approximateFront_v(toPlayer, _rotVel);
	}
	else
	{
		//mirar punto (enemigo)
		_goddessTransformC->approximateFront_v(_lookAt, _rotVel);
	}
	return LEAVE;
}

int BTGoddess::flyToPlace()
{

	if( EntityManager::get().getComponent<AnimationComponent>(_crowModel)->actionOn("reborn") )
		return STAY;

	//Animacion del cuervo de volar hasta delante del player. Alli desaparece con particulas guays.
	//SoundSystem::get().playSFX("cine2_sfx","data/sfx/aparicion_diosa.ogg","cine2_sfx",0.0f,1.0f,false);

	return LEAVE;
}

int BTGoddess::beBorn()
{
	//Aparece la animesh de la diosa, con efecto guay de particulas
	_crowModel->enabled = false;
	_goddessModel->enabled = true;

	//offset pa'bajo
	D3DXMATRIX vOffset_dx;
	btTransform vOffset_bt;
	D3DXMatrixTranslation(&vOffset_dx, 0, -0.8f, 0);
	convertD3DXMatrix(&vOffset_dx, vOffset_bt);

	const btTransform T =  *EntityManager::get().getComponent<TransformComponent>(_crowModel)->transform * vOffset_bt;
	btTransform  *goddessT =  EntityManager::get().getComponent<TransformComponent>(_goddessModel)->transform;
	goddessT->setBasis(T.getBasis());
	goddessT->setOrigin(T.getOrigin());

	Entity * p1 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_goddessTransformC->getPosition()),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_EXPLOSION);
	_isBorn = true;

	return LEAVE;
}

int BTGoddess::idleCrow()
{
	//Animacion de idleCrow, nada m�s
	return LEAVE;
}
