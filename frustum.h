#ifndef INC_FRUSTUM_H_
#define INC_FRUSTUM_H_

#include "aabb.h"

struct TFrustum {
	D3DXPLANE planes[ 6 ];
	void updateFromViewProjection( const D3DXMATRIX &view_proj ) {
		const float *mvp  = &view_proj.m[0][0];
		// Right clipping plane.
		planes[0] = D3DXPLANE( mvp[3]-mvp[0], mvp[7]-mvp[4], mvp[11]-mvp[8], mvp[15]-mvp[12] );
		// Left clipping plane.
		planes[1] = D3DXPLANE( mvp[3]+mvp[0], mvp[7]+mvp[4], mvp[11]+mvp[8], mvp[15]+mvp[12] );
		// Bottom clipping plane.
		planes[2] = D3DXPLANE( mvp[3]+mvp[1], mvp[7]+mvp[5], mvp[11]+mvp[9], mvp[15]+mvp[13] );
		// Top clipping plane.
		planes[3] = D3DXPLANE( mvp[3]-mvp[1], mvp[7]-mvp[5], mvp[11]-mvp[9], mvp[15]-mvp[13] );
		// Far clipping plane.
		planes[4] = D3DXPLANE( mvp[3]-mvp[2], mvp[7]-mvp[6], mvp[11]-mvp[10], mvp[15]-mvp[14] );
		// Near clipping plane.
		planes[5] = D3DXPLANE( mvp[3]+mvp[2], mvp[7]+mvp[6], mvp[11]+mvp[10], mvp[15]+mvp[14] );

		// Normalize, this is not always necessary...
		for( unsigned int i = 0; i < 6; i++ )
		{
			D3DXPlaneNormalize( planes+i, planes+i );
		}
	}

	enum EInside {
		IN_FRUSTUM, OUT_OF_FRUSTUM, CROSSING_FRUSTUM
	};

	EInside isInside( const TAABB &aabb ) const {
		const D3DXVECTOR3& aabbCenter = aabb.center;
		const D3DXVECTOR3& aabbSize   = aabb.half;
 
		EInside result = IN_FRUSTUM; // Assume that the aabb will be Inside the frustum
		for(unsigned int iPlane = 0;iPlane < 6;++iPlane)
		{
			const D3DXPLANE& frustumPlane = planes[iPlane];
 
			float d = aabbCenter.x * frustumPlane.a + 
				      aabbCenter.y * frustumPlane.b + 
				      aabbCenter.z * frustumPlane.c;
 
			float r = aabbSize.x * fabsf(frustumPlane.a) + 
			          aabbSize.y * fabsf(frustumPlane.b) + 
				      aabbSize.z * fabsf(frustumPlane.c);
 
			float d_p_r = d + r;
			float d_m_r = d - r;
 
			if(d_p_r < -frustumPlane.d)
			{
				result = OUT_OF_FRUSTUM; // Outside
				break;
			}
			else if(d_m_r < -frustumPlane.d)
				result = CROSSING_FRUSTUM; // Intersect
		}
		return result;
	}

};



#endif
