#ifndef INC_AABB_H_
#define INC_AABB_H_

#include <btBulletDynamicsCommon.h>
#include "d3ddefs.h"

struct TAABB {
	D3DXVECTOR3 center;
	D3DXVECTOR3 half;

	TAABB ()
	{
		center = D3DXVECTOR3(0,0,0);
		half = D3DXVECTOR3(1,1,1);

		//center.setValue(0,0,0);
		//half.setValue(1,1,1);
		//max.setValue(0.5f,0.5f,0.5f);
		//min.setValue(-0.5f,-0.5f,-0.5f);
	}

	TAABB(D3DXVECTOR3 _center, D3DXVECTOR3 _half)
	{
		center = _center;
		half = _half;
	}

	btVector3 btAABBCenter()
	{
		return btVector3(center.x, center.y, center.z);
	}

	btVector3 btAABBHalf()
	{
		return btVector3(half.x, half.y, half.z);
	}

	// Model * ( center +/- halfsize ) = model * center + model * half_size
	TAABB getRotatedBy( const D3DXMATRIX &model ) const {
	  TAABB new_aabb;
	  D3DXVec3TransformCoord( &new_aabb.center, &center, &model );
	  new_aabb.half.x = half.x * fabsf( model(0,0) )
					  + half.y * fabsf( model(1,0) )
					  + half.z * fabsf( model(2,0) );
	  new_aabb.half.y = half.x * fabsf( model(0,1) )
					  + half.y * fabsf( model(1,1) )
					  + half.z * fabsf( model(2,1) );
	  new_aabb.half.z = half.x * fabsf( model(0,2) )
					  + half.y * fabsf( model(1,2) )
					  + half.z * fabsf( model(2,2) );
	  return new_aabb;
	}

	float getRadius() 
	{
		return D3DXVec3Length(&half);
	}
};

#endif

