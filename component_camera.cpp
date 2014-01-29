#include "component_camera.h"
#include <cassert>

CameraComponent::CameraComponent(Entity* cam_entity, CAM_TYPE type, const btVector3& pos, const btVector3& target, float field_of_view, float aspect_ratio,  float cam_near, float cam_far) : Component(cam_entity)
{
	_camera.lookAt( pos, target, btVector3( 0,1,0 ));

	_camera.setStdFov(D3DXToRadian( field_of_view ));
	_camera.setProjectionParams( D3DXToRadian( field_of_view ), aspect_ratio, cam_near, cam_far);

	_camera.setController(type);
}


CameraComponent::~CameraComponent(void)
{
}