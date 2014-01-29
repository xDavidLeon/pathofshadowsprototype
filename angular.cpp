#include "angular.h"

// ----------------- Angular 
D3DXVECTOR3 getVectorFromYaw( float yaw ) {
	return D3DXVECTOR3( sinf( yaw )
		              , 0
					  , cosf( yaw ) );
}

float getYawFromVector( const D3DXVECTOR3 &v ) {
	return atan2f( v.x, v.z );
}

D3DXVECTOR3 getVectorFromYawAndPitch( float yaw, float pitch ) {
	return D3DXVECTOR3( sinf( yaw ) * cosf( pitch )
		              ,               sinf( pitch )
					  , cosf( yaw ) * cosf( pitch ) 
					  );
}

void getYawPitchFromVector( const D3DXVECTOR3 &v, float &yaw, float &pitch ) {
	yaw = atan2f( v.x, v.z );
	float mdo_xz = sqrtf( v.x*v.x + v.z*v.z );
	pitch = atan2f( v.y, mdo_xz );
}

D3DXVECTOR3 getRightXZOf( const D3DXVECTOR3 &v ) {
	return D3DXVECTOR3( -v.z, v.y, v.x );
}
D3DXVECTOR3 getLeftXZOf( const D3DXVECTOR3 &v ) {
	return D3DXVECTOR3( v.z, v.y, -v.x );
}

//Otras
float manhattanDist(const btVector3& a, const btVector3& b)
{
	return fabs(a.getX()-b.getX()) + fabs(a.getZ()-b.getZ());
}

