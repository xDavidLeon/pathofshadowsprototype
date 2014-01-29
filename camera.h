#ifndef INC_CAMERA_H_
#define INC_CAMERA_H_

#include <d3dx9.h>
#include "iostatus.h"
#include "angular.h"
#include "camera_controller.h"
#include <btBulletDynamicsCommon.h>
#include "frustum.h"

enum CAM_TYPE
{
	CAM_FREE,
	CAM_3RD,
	CAM_DBG,
	CAM_CIN
};

class TCamera {

public:

	TCamera();
	CameraController* controller;

	TFrustum frustum;

	// View 
	void lookAt( const btVector3 &src
		       , const btVector3 &dst
		       , const btVector3 &aux
			   );

	const btVector3& getPosition()   const { return position;   }
	const btVector3& getTarget()     const { return target;     }
	const D3DXMATRIX&  getView()       const { return view;       }
	const D3DXMATRIX&  getProjection() const { return projection; }
	const D3DXMATRIX&  getViewProjection() const { return view_projection; }

	float getDistanceToTarget()        const { return distance_to_target; }
	void getLeft(btVector3& out_v) const;
	void getFront(btVector3& out_v) const;
	void getUp(btVector3& out_v) const;

	void setStdFov(float std_fov);
	void changeFov(float fov);
	void changeToStdFov();

	// Proj
	void  setProjectionParams( float new_fov_in_radians
		                     , float new_aspect_ratio
		                     , float new_z_near, float new_z_far
							 );
	float getZNear( ) const { return z_near; }
	float getZFar( ) const  { return z_far; }
	void setZNear( float znear );
	float getAspectRatio( ) const  { return aspect_ratio; }
	float getFov( ) const   { return fov_in_radians; }

	void setController(CAM_TYPE control);

	void update(float delta);
	void render();

protected:
	// View 
	btVector3   position;
	btVector3   target;
	btVector3   aux_up;
	float		  distance_to_target;
	D3DXMATRIX    view;
	D3DXMATRIX		projection;
	D3DXMATRIX    view_projection;
	float _stdFov;
	float _desiredFov;

	//	D3DXQUATERNION orientation;	// 
	//	float yaw, pitch, roll;		// Euler angles

	// Prespective parameters
	float			fov_in_radians;	// Vertical fov
	float			aspect_ratio;
	float			z_near;
	float			z_far;

	void updateViewProjection( );
	void checkFov();

	btVector3   front;
	btVector3   left;
	btVector3   up;
};

#endif

