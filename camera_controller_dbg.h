#ifndef INC_CAMERA_CONTROLLER_DBG_H_
#define INC_CAMERA_CONTROLLER_DBG_H_

#include "camera_controller.h"
#include <d3dx9.h>

static const float _speedMin = 0.5f;
static const float _speedMax = 15.0f;

class CameraControllerDbg : public CameraController {

public:
	float       _yaw;
	float       _pitch;

	float       _mouse_sensibility_x;
	float       _mouse_sensibility_y;
	float       _strafe_sensibility;
	float       _speed;

	CameraControllerDbg(TCamera* camera);

	void setViewFrom(const TCamera* camera);
	void update(float delta);
	void init(void);
};

#endif



