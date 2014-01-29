#include "camera.h"
#include "camera_controller_3rd.h"
#include "camera_controller_dbg.h"
#include "globals.h"
#include "d3ddefs.h"
#include "world.h"
#include "system_sound.h"

TCamera::TCamera( )
{ 
	lookAt( btVector3( 0, 0, 10 ), btVector3( 0, 0, 1 ), btVector3( 0, 1, 0 ) );
	_stdFov = D3DXToRadian( 50.0f );
	_desiredFov = _stdFov;
	setProjectionParams(_stdFov , 4.0f / 3.0f, 1.0f, 10.0f );
	controller = NULL;
}

void TCamera::getLeft(btVector3& out_v) const
{
	out_v = left;
}

void TCamera::getUp(btVector3& out_v) const
{
	out_v = up;
}

void TCamera::getFront(btVector3& out_v) const
{
	out_v = front;
}

void TCamera::setController(CAM_TYPE control)
{
	switch (control)
	{
	case CAM_FREE:
		controller = NULL;
		break;
	case CAM_3RD:
		controller = new CameraController3rd(this);
		break;
	case CAM_DBG:
		controller = new CameraControllerDbg(this);
		break;
	case CAM_CIN:
		controller = NULL;
		break;
	default:
		controller = NULL;
		break;
	}
}

void TCamera::lookAt( const btVector3 &src
	       , const btVector3 &dst
	       , const btVector3 &aux
		   ) {
	// Save params
	position = src;
	target = dst;
	aux_up = aux;
	front = target - position;
	distance_to_target = front.length();

	front /= distance_to_target;
	left = aux_up.cross(front);
	left.normalize();

	up = front.cross(left);
	up.normalize();

	// update view
	D3DXMatrixLookAtRH( &view, &D3DXVECTOR3(position), &D3DXVECTOR3(target), &D3DXVECTOR3(aux_up) );
	updateViewProjection( );
}

void TCamera::setStdFov(float std_fov)
{
	_stdFov = std_fov;
	_desiredFov = _stdFov;
}

void TCamera::changeFov(float fov)
{
	_desiredFov = fov;
}

void TCamera::changeToStdFov()
{
	_desiredFov = _stdFov;
}


//PROJECTION---------------------------------------------------------------------------------------------------------------
void TCamera::setProjectionParams( float new_fov_in_radians
	                     , float new_aspect_ratio
	                     , float new_z_near, float new_z_far
						 ) {

	fov_in_radians = new_fov_in_radians;
	aspect_ratio   = new_aspect_ratio;
	z_near         = new_z_near;
	z_far          = new_z_far;

    D3DXMatrixPerspectiveFovRH( &projection
		                      , new_fov_in_radians, new_aspect_ratio
							  , new_z_near, new_z_far );
	updateViewProjection( );
}

void TCamera::update(float delta)
{
	btVector3 pos = getPosition();
	if (controller != NULL) controller->update(delta);
	frustum.updateFromViewProjection(view_projection);
	btVector3 newPos = getPosition();
	SoundSystem::get().updateListener(newPos,newPos-pos,front,up);
	checkFov();
}

void TCamera::setZNear( float znear )
{
	z_near = znear;
	D3DXMatrixPerspectiveFovRH( &projection
		                      , fov_in_radians, aspect_ratio
							  , z_near, z_far );
	updateViewProjection( );
}

void TCamera::checkFov()
{
	if(fov_in_radians != _desiredFov)
	{
		fov_in_radians = 0.93*fov_in_radians + 0.07*_desiredFov;
		setProjectionParams(fov_in_radians, aspect_ratio, z_near, z_far);
	}
}

void TCamera::render()
{
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );
	g_App.GetDevice()->SetTransform( D3DTS_VIEW, &getView() );
	g_App.GetDevice()->SetTransform( D3DTS_PROJECTION, &getProjection() );
}

void TCamera::updateViewProjection( ) {
	D3DXMatrixMultiply( &view_projection, &getView(), &getProjection() );
}
