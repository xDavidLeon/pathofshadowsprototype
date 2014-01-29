#ifndef INC_ANGULAR_H_
#define INC_ANGULAR_H_

#include <d3dx9.h>
#include "btBulletDynamicsCommon.h"

// Angular
D3DXVECTOR3 getVectorFromYaw( float yaw );
float getYawFromVector( const D3DXVECTOR3 &v );
D3DXVECTOR3 getVectorFromYawAndPitch( float yaw, float pitch );
void getYawPitchFromVector( const D3DXVECTOR3 &v, float &yaw, float &pitch );

D3DXVECTOR3 getRightXZOf( const D3DXVECTOR3 &v );
D3DXVECTOR3 getLeftXZOf( const D3DXVECTOR3 &v );

//Otras
float manhattanDist(const btVector3& a, const btVector3& b);

#endif