#include "camera_controller_3rd.h"
#include "camera.h"
#include "iostatus.h"
#include "world.h"
#include "config_manager.h"
#include "component_enemy_data.h"
#include "component_charcontroller.h"
#include "system_camera.h"
#include "entity_manager.h"
#include "system_physics.h"

const double PI = 3.141592;

CameraController3rd::CameraController3rd(TCamera* camera )
{ 
	_camera = camera;
	min_deltaY = 0.5f;
	max_deltaY = 0.5f;
	distance_near = 0.8f;
	distance_far = 1.8f;
	side_dist_far = 0.4f;
	side_dist_near = 0.4f;
	up_dist_far = 0.55f;
	up_dist_near = 0.5f;
	distance_zoom = 0.0f;
	_lockOn = false;
	_lockedEntity = NULL;

	normal_fov = camera->getFov();
	running_fov = D3DXToRadian( 80.0f );
}

void CameraController3rd::init(void)
{
	_current_dist_to_target = distance_far;
	_distance_to_target = distance_far;
	_desiredDistance = distance_far;
	_sideDist = side_dist_far;
	_desiredSideDist = side_dist_far;
	_upDist = up_dist_far;
	_desiredUpDist = up_dist_far;
	_front = _targetTransform->getBasis()[2].normalize();
	_front.setX(-_front.getX());
	_frontXZ = _front;  _frontXZ.setY(0); _frontXZ.normalize();
	character = EntityManager::get().getComponent<CharacterControllerComponent>(_targetEntity);
	_mouse_sensibility = 0.1f;
	_joystick_sensibility = 2.0f;
	_centerCamera = false;
	_desiredFov = normal_fov;
	_position = _targetTransform->getOrigin() - _distance_to_target * _front;
	_collided = false;
}

void CameraController3rd::update(float delta)
{
	//Modificar la distancia al objetivo si conviene
	checkCameraDistance();

	if(_lockOn)
	{
		//Shadow mira hacia donde mira la camara
		TransformComponent * player_trans = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer());
		btVector3 cam_front;  CameraSystem::get().getPlayerCamera().getFront(cam_front);
		cam_front.setY(0);
		player_trans->approximateFront_v(cam_front, 10*delta);

		//Si hay enemigo fijado, hacer que el player mire hacia el
		if(_lockedEntity)
		{
			btVector3 dir =  EntityManager::get().getComponent<TransformComponent>(_lockedEntity)->getPosition() - player_trans->getPosition();
			lookTo(dir, 0.8f); //Camara mira hacia la entity fijada
		}
		else
		{
			lookForEnemies();
			if(!CameraSystem::get().isLockedCamera3rd()) rotateCamera(delta); //Rotar camara segun input 
		}
	}
	else
	{
		if(!CameraSystem::get().isLockedCamera3rd()) rotateCamera(delta); //Rotar camara segun input 
	}

	//Centrar camara
	checkCenterCamera();

	//Se resitua la camara en una posicion guay
	btVector3 targetPos = _targetTransform->getOrigin();
	targetPos.setY(targetPos.getY() + _upDist);
	btVector3 left = btVector3(0,1,0).cross(_front);  left.normalize();
	targetPos = targetPos + _sideDist*left;

	placeByCollisionsPosition(delta, targetPos);

	_camera->lookAt( _position, targetPos, btVector3(0,1,0));
}

void CameraController3rd::checkCameraDistance()
{
	if(_distance_to_target != _desiredDistance)
		_distance_to_target = 0.9f*_distance_to_target + 0.1f*_desiredDistance;

	if(_sideDist != _desiredSideDist)
		_sideDist = 0.9f*_sideDist + 0.1f*_desiredSideDist;

	if(_upDist != _desiredUpDist)
		_upDist = 0.9f*_upDist + 0.1f*_desiredUpDist;
}

void CameraController3rd::setCameraFov(float weight)
{
	float fov = normal_fov + ((running_fov-normal_fov) * weight);

	_camera->changeFov( fov );
}

void CameraController3rd::rotateCamera(float delta)
{
	//Get the io status
	CIOStatus * ios = CIOStatus::instance();

	int inv_x = ConfigManager::get().inv_x;
	int inv_y = ConfigManager::get().inv_y;

	//Rotate the camera with mouse (X axis)
	if(ios->delta_mouse.x && !_centerCamera)
	{
		 _front = _front.rotate(btVector3(0,1,0), -ios->delta_mouse.x * _mouse_sensibility*delta * inv_x);
		 _frontXZ = _front;  _frontXZ.setY(0);  _frontXZ.normalize();
	}

	//Rotate camera with XBox controller
	if(ios->right.x && !_centerCamera)
	{
		_front = _front.rotate(btVector3(0,1,0), -ios->right.x * _joystick_sensibility * delta * inv_x);
		 _frontXZ = _front;  _frontXZ.setY(0);  _frontXZ.normalize();
	}

	
	//Rotate camera (Y axis) (there's a limit!)
	btVector3 up = btVector3(0,1,0);

	if( ios->delta_mouse.y && !_centerCamera)
	{
		btVector3 cam_left;  _camera->getLeft(cam_left);
		btVector3 new_front = _front;
		new_front = new_front.rotate(cam_left, ios->delta_mouse.y * _mouse_sensibility*delta * inv_y);
		
		if( new_front.dot(up) < 0.0f )
		{
			if( new_front.dot(_frontXZ) > max_deltaY ) _front = new_front;
		}
		else
		{
			if( new_front.dot(_frontXZ) > min_deltaY ) _front = new_front;
		}
	}

	//Rotate camera with XBox controller
	if(ios->right.y && !_centerCamera)
	{
		btVector3 cam_left;  _camera->getLeft(cam_left);
		btVector3 new_front = _front;
		new_front = new_front.rotate(cam_left, -ios->right.y * _joystick_sensibility*delta * inv_y);

		if( new_front.dot(up) < 0.0f )
		{
			if( new_front.dot(_frontXZ) > max_deltaY ) _front = new_front;
		}
		else
		{
			if( new_front.dot(_frontXZ) > min_deltaY ) _front = new_front;
		}
	}
}

void CameraController3rd::checkCenterCamera()
{
	//Comprobar boton para centrar camara detras del personaje
	if(CIOStatus::instance()->becomesPressed(CIOStatus::TButton::KEYBOARD_T))
		_centerCamera = true;

	if(!_centerCamera) return;

	btVector3 dest = _targetTransform->getBasis()[2].normalize(); //front del player.
	dest.setX(dest.getX()*-1); //Por algun motivo satanico en la transform la X esta al reves

	float angle = _front.angle(dest);
	if (angle > 0.05)
	{
		if(fabs(angle - PI) < 0.001f)
		{
			//Si el angulo es pi (o casi) movemos un poco el vector pq sino no se centra.
			dest.setY(dest.getY()+0.001f); 
			dest.setX(dest.getX()+0.001f);
		}

		_front = 0.80*_front + 0.2*dest;
		_front.normalize();
	}
	else //Hemos acabado de centrar
	{
		_front = dest;
		_frontXZ = _front;  _frontXZ.setY(0); _frontXZ.normalize();
		_centerCamera = false;
	}
}

//Interpola el lookat de la camara hasta el vector dir
//InterpRatio va de 0 a 1
void CameraController3rd::lookTo(const btVector3& dir, float interpRatio)
{
	btVector3 dest = dir;
	dest.normalize();
	//dest.setX(dest.getX()*-1); //Por algun motivo satanico en la transform la X esta al reves

	float angle = _front.angle(dest);
	if (angle > 0.05)
	{
		if(fabs(angle - PI) < 0.001f)
		{
			//Si el angulo es pi (o casi) movemos un poco el vector pq sino no se centra.
			dest.setY(dest.getY()+0.001f); 
			dest.setX(dest.getX()+0.001f);
		}

		_front = interpRatio*_front + (1-interpRatio)*dest;
		_front.normalize();
	}
	else //Hemos acabado de centrar
	{
		_front = dest;
		_frontXZ = _front;  _frontXZ.setY(0); _frontXZ.normalize();
	}
}

void CameraController3rd::setDistanceFar()
{
	_desiredDistance = distance_far;
	_desiredSideDist = side_dist_far;
	_desiredUpDist = up_dist_far;
	_current_dist_to_target = distance_far;
}

void CameraController3rd::setDistanceNear()
{
	_desiredDistance = distance_near;
	_desiredSideDist = side_dist_near;
	_desiredUpDist = up_dist_near;
	_current_dist_to_target = distance_near;
}

void CameraController3rd::setDistanceBlended()
{
	_desiredDistance = 0.0f;
	_desiredSideDist = 0.0f;
	_desiredUpDist = 0.0f;
}

void CameraController3rd::zoomIn()
{
	_desiredDistance = distance_zoom;
}

void CameraController3rd::setDesiredDistance(float dst)
{
	if(dst == -1)
	{
		_desiredDistance = distance_far;
	}
	else if( (dst < 0.2) || (dst > 10)) return;
	_desiredDistance = dst;
}

void CameraController3rd::lockOn(bool lockon)
{
	if(!lockon) _lockedEntity = NULL;
	_lockOn = lockon;
}

//Busca enemigos
void CameraController3rd::lookForEnemies()
{
	std::map<Entity*,Component*>* entitiesWithED = EntityManager::get().getAllEntitiesPosessingComponent<EnemyDataComponent>();
	if(!entitiesWithED) return;

	float distance = FLT_MAX;
	float min_distance = distance;

	std::map<Entity*,Component*>::iterator iter;
	for (iter = entitiesWithED->begin(); iter != entitiesWithED->end(); ++iter)
	{
		if( iter->second->enabled ) 
		{
			Entity* entity = iter->first;
			TransformComponent* transformEnemy = EntityManager::get().getComponent<TransformComponent>(entity);
			
			float angle = _front.angle(transformEnemy->getPosition() - _camera->getPosition());

			//dbg("angle: %f\n", angle);
		
			if( angle < 1.0f )
			{		
				distance = transformEnemy->getPosition().distance2(_camera->getTarget());
				//dbg("distance: %f\n", distance);
				
				if(distance < 30.0f && distance < min_distance)
				{	
					_lockedEntity = entity;
					min_distance = distance;
				}
			}			
		}
	}
}

//Modifica la dist. to target de la camara en el caso que esta colisione con alguna cosa
void CameraController3rd::placeByCollisionsPosition(float delta, btVector3& targetPos)
{
	btVector3 btRayFrom = _targetTransform->getOrigin();
	//posicion a la que deberia situarse la camra en condiciones normales
	btVector3 desiredPosition = targetPos - _current_dist_to_target * _front;
	btVector3 position, direction;

	//Creamos callback para obtener resultado de la colision
	btCollisionWorld::ClosestRayResultCallback rayCallback(btRayFrom,desiredPosition);
	rayCallback.m_collisionFilterMask = PhysicsSystem::get().colMaskNavigation; //Para no colisionar con la misma capsula del player
	PhysicsSystem::get().getCollisionWorld()->rayTest(btRayFrom,desiredPosition, rayCallback); //(Test de colision)

	//Si colisiona modificamos la desired dist de la camara
	if (rayCallback.hasHit())
	{
		position = rayCallback.m_hitPointWorld;
		direction = btRayFrom - position;
		position += direction * 0.3f; //************************************************** JUGAR UN POCO CON ESTE VALOR 
		//-->Sin normalizar se asegura que la linea anterior de como resultado un punto entre la desired_pos y el player

		_desiredDistance = (position-targetPos).length();
	}
	else _desiredDistance = _current_dist_to_target;
	
	_position = targetPos - _distance_to_target * _front;
}
