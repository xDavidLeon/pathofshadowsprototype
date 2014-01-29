#ifndef INC_CAMERA_CONTROLLER_H_
#define INC_CAMERA_CONTROLLER_H_
#include <btBulletDynamicsCommon.h>

class TCamera;
class Entity;
class CameraController {

public:
	CameraController();
	CameraController(TCamera* camera);
	~CameraController();
	virtual void update(float delta) = 0;
	void setTargetEntity(Entity* e);
	virtual void init(void) = 0;

protected:
	TCamera* _camera;
	btTransform * _targetTransform;
	Entity* _targetEntity;
};

#endif

