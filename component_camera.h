#ifndef CAM_COMP
#define CAM_COMP

#include "component.h"
#include "camera.h"

class CameraComponent : public Component
{
	TCamera _camera;

public:
	CameraComponent(Entity* cam_entity, CAM_TYPE type, const btVector3& pos, const btVector3& target, float field_of_view, float aspect_ratio,  float cam_near, float cam_far);
	~CameraComponent(void);

	TCamera& getCamera(){ return _camera; }

	void update(float delta){ _camera.update(delta); }
	void render(){ _camera.render(); }
};

#endif