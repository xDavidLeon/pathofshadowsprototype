#ifndef INC_CONTROLLER_3RD_H_
#define INC_CONTROLLER_3RD_H_

#include "camera_controller.h"
#include "component_charcontroller.h"
#include "counter_clock.h"


class btTransform;
class CameraController3rd : public CameraController {

public:
	float		_distance_to_target;
	btVector3	_position;
	btVector3	_front;
	btVector3   _frontXZ;
	float		_mouse_sensibility;
	float		_joystick_sensibility;
	bool		_centerCamera;

	CameraController3rd(TCamera* camera);

	void update(float delta);
	void init(void);

	void setDistanceFar();
	void setDistanceNear();
	void setDistanceBlended();
	void zoomIn();
	void setDesiredDistance(float dst);
	void lockOn(bool lockon);
	void setCameraFov(float weight);

	Entity* getLockedEntity(){ return _lockedEntity; };
	void clearLockedEntity() { _lockedEntity = NULL; };

private:
	//"constantes"
	float min_deltaY;
	float max_deltaY;
	float distance_near;
	float distance_far;
	float side_dist_far;
	float side_dist_near;
	float up_dist_far;
	float up_dist_near;
	float distance_zoom;

	float normal_fov;
	float running_fov;

	CharacterControllerComponent * character;

	float _current_dist_to_target;
	float _desiredDistance;
	float _sideDist;
	float _desiredSideDist;
	float _upDist;
	float _desiredUpDist;

	float _desiredFov;
	bool _collided;
	CounterClock _interpolating;

	bool _lockOn;
	Entity* _lockedEntity;

	void checkCameraDistance();
	void rotateCamera(float delta);
	void checkCenterCamera();
	void lookTo(const btVector3& dir, float interpRatio);
	void lookForEnemies();
	void placeByCollisionsPosition(float delta, btVector3& targetPos);
};

#endif

