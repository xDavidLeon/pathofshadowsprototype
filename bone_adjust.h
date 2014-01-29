#ifndef INC_BONE_ADJUST_H_
#define INC_BONE_ADJUST_H_

#include <cassert>
#include "cal3d/cal3d.h"
#include "model.h"
#include "iostatus.h"
#include "d3ddefs.h"
#include "world.h"

struct TBoneAdjust{
	int		 bone_id;
	CalBone *bone;
	// Que eje local quiero que mire a world_target
	CalVector local_axis;
	// Donde quiero mirar
	CalVector world_target;

	float    unit_amount;
	float    max_unit_amount;
	float	 max_angle;

	// tmp vars, 
	CalVector world_bone_trans;
	CalQuaternion world_bone_rot;
	CalVector world_target_direction;
	
	// Eje local de rotacion
	CalVector local_rotation_axis;

	Entity* _entity;

	TBoneAdjust( ) : bone( NULL ), unit_amount( 0.0f ), max_unit_amount( 1.0f ), max_angle( D3DXToRadian(40.0f) ) { }

	void init( CModel *model, const char *bone_name ) {
		assert( model );
		bone_id = model->getSkeleton()->getCoreSkeleton()->getCoreBoneId( bone_name );
		bone = model->getSkeleton()->getBone( bone_id );
		local_axis = CalVector( 0,1,0 );
		local_axis.normalize();
		world_target = CalVector( 1,1,3 );
		_entity = model->_entity;
	}

	void update( float amount ) {

		unit_amount = amount;

		if( CIOStatus::instance()->isPressed( VK_ADD ) )
			unit_amount += 0.01f;
		if( CIOStatus::instance()->isPressed( VK_SUBTRACT ) )
			unit_amount -= 0.01f;

		// Confirm unit_amount is in the valid range
		if( unit_amount > 1.0f )
			unit_amount = 1.0f;
		else if( unit_amount < 0.0f )
			unit_amount = 0.0f;

		/// Input params
		// Donde esta el bone antes de actuar
		world_bone_trans = bone->getTranslationAbsolute( );
		world_bone_rot = bone->getRotationAbsolute( );

		// Direccion en world que tiene que asumir el local_axis al final
		world_target_direction = world_target - world_bone_trans;

		// Este quaternion me hace el paso de mundo a local bone
		CalQuaternion world_bone_rot_inverse = world_bone_rot; 
		world_bone_rot_inverse.invert();

		// Pasar la direccion de mundo a direccion en espacio local
		CalVector local_target_direction = world_target_direction;
		local_target_direction *= world_bone_rot_inverse;
		local_target_direction.normalize();

		// Do the cross product
		local_rotation_axis = local_target_direction % local_axis;
		local_rotation_axis.normalize();
		
		// Do the dot product
		float cos_angle = local_axis * local_target_direction;
		// Do not rely on float's
		if( cos_angle > 1.0f )
			cos_angle = 1.0f;
		else if( cos_angle <= -1.0f )
			cos_angle = -1.0f;
		float angle = acosf( cos_angle );

		angle *= unit_amount * max_unit_amount;

		CalQuaternion qdelta = buildQuat( local_rotation_axis, angle );

		// Now, apply the delta
		CalQuaternion qprev = bone->getRotation();
		CalQuaternion qnew = qprev * qdelta;
		
		// Check if angle > max_angle (to do)

		bone->setRotation( qnew );
		// Make the children update their rotation also
		bone->calculateState( );
	}

	void render( ) {
		D3DXMATRIX identity;
		D3DXMatrixIdentity( &identity );
		g_App.GetDevice()->SetTransform( D3DTS_WORLD, &identity );
		drawLineD3X( toDX( world_bone_trans ), toDX(world_target), 0xffff00ff );
		
	}
};

struct TBoneChainAdjust {

	typedef std::vector< TBoneAdjust > VBoneAdjusts;
	VBoneAdjusts adjs;

	float amount; 
	float time;
	float delay;
	bool blendingIn;
	bool blendingOut;
	float init_amount;

	void init()
	{
		amount		 = 0.0f;
		time		 = 0.0f;
		delay		 = 0.0f;
		blendingIn	 = false;
		blendingOut	 = false;
		init_amount	 = 0.0f;
	}

	void add( CModel *model, const char *bone, float max_percent ) {
		TBoneAdjust adj;
		adj.init( model, bone );
		adj.max_unit_amount = max_percent;
		adjs.push_back( adj );
	}
	void update( float delta ) {

		//actualizamos los valores de para hacer blendIn y blendOut
		if( blendingIn || blendingOut ) 
			updateBlending( delta );

		if( !amount ) return;
		if( amount < 0.0f || amount > 1.0f ) return;

		//if( CIOStatus::get().isPressed( 'T' ) ) {
		//	// Only 1 adjust, and at 100%
		//	adjs[ 0 ].max_unit_amount = 1.0f;
		//	adjs[ 0 ].update( );

		//} else {
		//	adjs[ 0 ].max_unit_amount = 0.3f;
			// Actualizar todos los adjs en orden
			VBoneAdjusts::iterator i = adjs.begin();
			while( i != adjs.end() ) {
				i->update( amount );
				++i;
			}
	//	}
	}

	void render( ) {
		/*if (World::instance()->isDebugModeOn())
		{*/
			printf2D( 800, 200, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "delay: %f", delay);
			printf2D( 800, 220, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "time: %f", time);
			printf2D( 800, 240, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "amount: %f", amount);
			printf2D( 800, 260, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "blendingIn: %d", blendingIn);
			printf2D( 800, 280, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "blendingOut: %d", blendingOut);
		/*}*/

		// Actualizar todos los adjs en orden
		VBoneAdjusts::iterator i = adjs.begin();
		while( i != adjs.end() ) {
			i->render( );
			++i;
		}
	}


	void blendLookAtIn( float delayIn )
	{
		delay = delayIn;
		time = 0.0f;
		blendingIn = true;
		if( blendingOut )
		{
			init_amount = amount;
			blendingOut = false;
		}
		else 
			init_amount = 0.0f;
	}

	void blendLookAtOut( float delayOut )
	{
		delay = delayOut;
		time = 0.0f;
		blendingOut = true;
		if( blendingIn )
		{
			init_amount = amount;
			blendingIn = false;
		}
		else 
			init_amount = 1.0f;
	}

	void updateBlending( float delta )
	{
		if( blendingIn ) {
			// si estoy en un estado intermedio
			if( amount < 1.0f && delay != 0.0f){
				time = time + delta;
				amount = init_amount + time / delay;
				if(amount > 1.0f)
					amount = 1.0f;
			}
			// si el IK tiene el control total
			else{
				amount = 1.0f;
				time = 0.0f;
				blendingIn = false;
			}
			
			dbg("amount in %f\n", amount);
			dbg("delay in %f\n", delay);
		}
		if( blendingOut )
		{
			// si estoy en un estado intermedio
			if( amount > 0.0f && delay != 0.0f){
				time = time + delta;
				amount = init_amount - time / delay;
				if(amount < 0.0f)
					amount = 0.0f;
			}
			// si el IK tiene el control total
			else{
				amount = 0.0f;
				time = 0.0f;
				blendingOut = false;
			}
			dbg("amount out %f\n", amount);
		}
	}
};

#endif
