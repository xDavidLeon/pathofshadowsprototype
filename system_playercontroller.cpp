#include "system_playercontroller.h"
#include "iostatus.h"
#include "entity_manager.h"
#include "component_charcontroller.h"
#include "world.h"
#include "component_transform.h"
#include "component_animation.h"
#include "system_sound.h"
#include "component_player_controller.h"
#include "system_camera.h"

PlayerControllerSystem::PlayerControllerSystem(void)
{
	_aimSpeed = 0.005f;
	_stdSpeed = 0.025f;
	_maxSpeed = 0.08f;
	_currentSpeed = _stdSpeed;
	_desiredSpeed = _currentSpeed;
	accelerationFactor = 0.5f;
	_rotationSpeed = 5.0f;
}

PlayerControllerSystem::~PlayerControllerSystem(void)
{
}

void PlayerControllerSystem::setPlayer(Entity* p)
{
	_player = p;
	_charController = EntityManager::get().getComponent<CharacterControllerComponent>(_player);
}

//Se encarga del movimiento del player seg�n el input, y de orientar su mesh en consecuencia
//Si se recibe input direccional y el personaje se puede mover se devuelve true, sin� false
bool PlayerControllerSystem::update(float delta, bool orientateMesh, bool canRun, bool canMove)
{	
	//if (World::instance()()) //En modo debug dejamos al player "en pausa"
	//{
	//	
	//	_charController->controller->setWalkDirection(btVector3(0,0,0));
	//	return false;
	//}

	float pressedX = 0.0f;
	float pressedY = 0.0f;

	if(canMove)
	{
		//Obtenemos input direccional
		pressedX = CIOStatus::instance()->getHorizontalAxis();
		pressedY = CIOStatus::instance()->getVerticalAxis();
	}

	if(canRun)
	{
		//En teclado alternamos entre lento o rapido segun la tecla alt
		if(CIOStatus::instance()->isSprinting())
		{
			_desiredSpeed = _maxSpeed;
			//Hacer ruido
			EntityManager::get().getComponent<PlayerControllerComponent>(_player)->_noiseDistSq = runNoiseSq;
		}
		else if(!CIOStatus::instance()->isSprinting())
			_desiredSpeed = _stdSpeed;

		/*if( CIOStatus::instance()->left.normalized_magnitude > 0.0f )
			_desiredSpeed = (_maxSpeed - _stdSpeed) * CIOStatus::instance()->left.normalized_magnitude + _stdSpeed;*/
	}
	else if(CIOStatus::instance()->isPressed(CIOStatus::AIM))
		_desiredSpeed = _aimSpeed;
	else
		_desiredSpeed = _stdSpeed;

	//Interpolar vel (teclado)
	if(_currentSpeed != _desiredSpeed)
		_currentSpeed = 0.97f*_currentSpeed + 0.03f*_desiredSpeed;

	//Direccion del movimiento, segun la camara, en espacio de mundo
	btVector3 aux;
	CameraSystem::get().getPlayerCamera().getLeft(aux);
	btVector3 dirX = -pressedX * aux; //-pressedX pq estamos en RH
	CameraSystem::get().getPlayerCamera().getFront(aux);
	aux.setY(0.0f);
	btVector3 dirY = pressedY * aux;

	aux = dirX + dirY;
	if(aux != btVector3(0,0,0))
		aux.normalize(); //Ahora aux contine la direccion de movimiento del player segun la camara

	//Movemos el player segun el vector obtenido
	_charController->desiredVelocity.setValue(aux.getX(),0,aux.getZ());
	_charController->approximateVelocity();
	_charController->controller->setWalkDirection(_charController->velocity*_currentSpeed);

	if(aux == btVector3(0,0,0)) return false; //Si el vector de direccion es 0 no modificamos la transform.
	if(!orientateMesh) return true;

	//Hacemos que la mesh apunte (se aproxime hacia) al mismo vector que el movimiento
	TransformComponent * player_trans = EntityManager::get().getComponent<TransformComponent>(_player);
	
	float angle_out;
	player_trans->approximateFront_v(aux, _rotationSpeed*delta, angle_out);

	EntityManager::get().getComponent<AnimationComponent>(_player)->turnPlayer(angle_out);

	return true;
}
