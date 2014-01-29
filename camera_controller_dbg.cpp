#include "camera_controller_dbg.h"
#include "camera.h"
#include "d3ddefs.h"
#include "world.h"
#include "entity_manager.h"

CameraControllerDbg::CameraControllerDbg(TCamera* camera)
{
	_camera = camera;
	init();
}

void CameraControllerDbg::init(void)
{
	_yaw = 0.0f;
	_pitch = 0.0f;

	_mouse_sensibility_x = 0.005f;
	_mouse_sensibility_y = 0.005f;
	_strafe_sensibility = 0.2f;
	_speed = 2.0f;
}

void CameraControllerDbg::setViewFrom(const TCamera* camera)
{
	_camera->lookAt(camera->getPosition(), camera->getTarget(), btVector3(0,1,0));
	btVector3 frontBT;  _camera->getFront(frontBT);
	D3DXVECTOR3 frontDX;  convertBulletVector3(&frontBT, frontDX);
	getYawPitchFromVector(frontDX, _yaw, _pitch);
}

void CameraControllerDbg::update(float delta)
{
	//Get the io status
	CIOStatus * ios = CIOStatus::instance();

	//Get orientation vectors (plane XZ) and pos
	D3DXVECTOR3 front_xz = getVectorFromYaw( _yaw );
	D3DXVECTOR3 right_xz = getRightXZOf( front_xz );
	D3DXVECTOR3 up_xz = D3DXVECTOR3(0,1,0);
	D3DXVECTOR3 loc = _camera->getPosition();

	//Modify parameters taking the info in ios
	_yaw -= ios->delta_mouse.x * _mouse_sensibility_x;
	_pitch -= ios->delta_mouse.y * _mouse_sensibility_y;

	float moveSpeed = _speed;
	if (ios->isPressed(VK_LSHIFT)) moveSpeed *= 10;

	// Pos y
	if( ios->isPressed( VK_LBUTTON ) ) {
		loc += up_xz * moveSpeed * delta;
	} else if( ios->isPressed( VK_RBUTTON ) ){
		loc -= up_xz * moveSpeed * delta;
	}
	
	//Pos z
	if( ios->isPressed( 'W' ) ) {
		loc += front_xz * moveSpeed * delta;
	} else if( ios->isPressed( 'S' ) ) {
		loc -= front_xz * moveSpeed * delta;
	}

	//Pos x
	if( ios->isPressed( 'A' ) ) {
		loc -= right_xz * moveSpeed * delta;
	} else if( ios->isPressed( 'D' ) ) {
		loc += right_xz * moveSpeed * delta;
	}

	//speed
	if( ios->isPressed( 'Q' )) {
		_speed -= delta*2;
		if(_speed < _speedMin) _speed = _speedMin;
	} else if( ios->isPressed( 'E' )) {
		_speed += delta*2;
		if(_speed > _speedMax) _speed = _speedMax;
	}

	//Poner player donde se encuentre la camara
	if(ios->becomesPressed(CIOStatus::TButton::BLEND))
	{
		EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->setPosition(btVector3(loc.x, loc.y, loc.z));
	}

	//front can not be the same as 'up'
	const float max_pitch = (float)(90.0f - 1e-3);
	if( _pitch > D3DXToRadian( max_pitch ) )
		_pitch = D3DXToRadian( max_pitch );
	else if( _pitch < D3DXToRadian( -max_pitch ) )
		_pitch = D3DXToRadian( -max_pitch );

	//Update _camera data (view matrix)
	D3DXVECTOR3 front = getVectorFromYawAndPitch( _yaw, _pitch );
	D3DXVECTOR3 target = loc + front;
	_camera->lookAt( btVector3(loc.x, loc.y, loc.z), btVector3(target.x, target.y, target.z), btVector3(0,1,0));
}
