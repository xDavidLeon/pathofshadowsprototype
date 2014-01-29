#ifndef INC_IK_BONE_H_
#define INC_IK_BONE_H_

#include "ik_handler.h"
#include "model.h"
#include "system_physics.h"
#include "system_camera.h"

struct TIKBone {
	CModel     *model;
	std::string bone_c_name;
	CalBone    *bone_a;
	CalBone    *bone_b;
	CalBone    *bone_c;

	CalBone	   *bone_twist_arm;
	CalBone	   *bone_twist_forearm;

	TIKHandler  ik_handler;
	float amount; 
	float time;
	float delay;
	bool blendingIn;
	bool blendingOut;
	float init_amount;
	bool blocked;

	// Custom
	D3DXVECTOR3 delta_c;

	void init( CModel *new_model, const std::string &new_bone_c_name ) {
		model = new_model;
		bone_c_name = new_bone_c_name;

		// Me especifican el bone 'c'
		int bone_c_id = model->getCoreModel()->getCoreSkeleton()->getCoreBoneId( bone_c_name );
		assert( bone_c_id != -1 );
		bone_c = model->getSkeleton()->getBone( bone_c_id );

		// Obtener b
		int bone_b_id = bone_c->getCoreBone()->getParentId();
		assert( bone_b_id != -1 );
		bone_b = model->getSkeleton()->getBone( bone_b_id );

		// Obtener el bone que hara de 'a'
		int bone_a_id = bone_b->getCoreBone()->getParentId();
		assert( bone_a_id != -1 );
		bone_a = model->getSkeleton()->getBone( bone_a_id );

		// Por ejemplo, levantar el pie 1/2 metro
		delta_c = D3DXVECTOR3( 0, 0.0, 0 );

		amount = 0.0f;
		init_amount = 0.0f;
		time = 0.0f;
		delay = 0.0f;
		blendingIn = false;
		blendingOut = false;
		blocked = false;
	}

	void initTwist( CModel *new_model, const std::string &new_bone_c_name, const std::string &new_twist_arm_name, const std::string &new_twist_forearm_name ) {
		model = new_model;
		bone_c_name = new_bone_c_name;

		// Me especifican el bone 'c'
		int bone_c_id = model->getCoreModel()->getCoreSkeleton()->getCoreBoneId( bone_c_name );
		assert( bone_c_id != -1 );
		bone_c = model->getSkeleton()->getBone( bone_c_id );

		// Obtener b
		int bone_b_id = bone_c->getCoreBone()->getParentId();
		assert( bone_b_id != -1 );
		bone_b = model->getSkeleton()->getBone( bone_b_id );

		// Obtener el bone que hara de 'a'
		int bone_a_id = bone_b->getCoreBone()->getParentId();
		assert( bone_a_id != -1 );
		bone_a = model->getSkeleton()->getBone( bone_a_id );

		int bone_twist_id = model->getCoreModel()->getCoreSkeleton()->getCoreBoneId( new_twist_arm_name );
		assert( bone_twist_id != -1 );
		bone_twist_arm = model->getSkeleton()->getBone( bone_twist_id );

		bone_twist_id = model->getCoreModel()->getCoreSkeleton()->getCoreBoneId( new_twist_forearm_name );
		assert( bone_twist_id != -1 );
		bone_twist_forearm = model->getSkeleton()->getBone( bone_twist_id );

		// Por ejemplo, levantar el pie 1/2 metro
		delta_c = D3DXVECTOR3( 0, 0.0, 0 );
	
		amount = 0.0f;
		init_amount = 0.0f;
		time = 0.0f;
		delay = 0.0f;
		blendingIn = false;
		blendingOut = false;
		blocked = false;
	}

	void update( const btVector3 &destination ) {

		if( CIOStatus::instance()->isPressed( VK_ADD ) )
			delta_c.y += 0.02f;
		else if( CIOStatus::instance()->isPressed( VK_SUBTRACT ) )
			delta_c.z -= 0.02f;

		// Calcular la nueva posicion de B
		ik_handler.init( toDX(bone_a->getTranslationAbsolute())
					   , toDX(bone_b->getTranslationAbsolute())
			           , toDX(bone_c->getTranslationAbsolute()) );
		ik_handler.a = toDX(bone_a->getTranslationAbsolute());
		ik_handler.c = toDX(bone_c->getTranslationAbsolute()) + delta_c;
		ik_handler.updateB( );

		// Traspasar la posicion del IK a los bones del model

		// Corregir 'b'
		TBoneAdjust adj_b;
		adj_b.init( model, bone_a->getCoreBone()->getName().c_str() );
		adj_b.local_axis.set( 1,0,0 );
		adj_b.world_target = toCal( ik_handler.b );
		adj_b.unit_amount = 1.0f;
		adj_b.update( 1.0f );

		TBoneAdjust adj_c;
		adj_c.init( model, bone_b->getCoreBone()->getName().c_str() );
		adj_c.local_axis.set( 1,0,0 );
		adj_c.world_target = toCal( ik_handler.c );
		adj_c.unit_amount = 1.0f;
		adj_c.update( 1.0f );
	}

	void blendAimIn( float delayIn )
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

	void blendAimOut( float delayOut )
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
			if( amount < 1.0f ){
				time = time + delta;
				amount = init_amount + time / delay;
			}
			// si el IK tiene el control total
			else{
				amount = 1.0f;
				time = 0.0f;
				blendingIn = false;
			}
		}
		if( blendingOut )
		{
			// si estoy en un estado intermedio
			if( amount > 0.0f ){
				time = time + delta;
				amount = init_amount - time / delay;
			}
			// si el IK tiene el control total
			else{
				amount = 0.0f;
				time = 0.0f;
				blendingOut = false;
			}
		}
	}

	void updateAim( float delta ) {

		//actualizamos los valores de para hacer blendIn y blendOut
		if( blendingIn || blendingOut ) 
			updateBlending( delta );

		if( !amount ) return;
		
		if( !blocked)
		{
			float arm_aim_lenght = (bone_c->getTranslationAbsolute() - bone_a->getTranslationAbsolute()).length();
			btVector3 cam_front;  CameraSystem::get().getPlayerCamera().getFront(cam_front);
			CalVector arm_desired_direction = toCal(cam_front);

			btVector3 btRayFrom = toBullet(bone_a->getTranslationAbsolute());
			// empezamos a calcular la colision a partir del doble de la longitud del brazo (apuntando)
			// asi podemos lograr una transicion suave entre doblar y no doblar y manter una distancia con la colision
			btVector3 btRayTo = btRayFrom + (arm_aim_lenght*cam_front) * 2.0f;

			CalVector AC;

			//Creamos callback para obtener resultado de la colision
			btCollisionWorld::ClosestRayResultCallback rayCallback(btRayFrom,btRayTo);
			//Test de colision
			PhysicsSystem::get().getCollisionWorld()->rayTest(btRayFrom, btRayTo, rayCallback);
			//Si colisiona hacemos cositas molonas
			if (rayCallback.hasHit())
			{
				btVector3 hitPosition = rayCallback.m_hitPointWorld;
				// dejamos media longitud del brazo de espacio a la colision
				// de este dato sale el 2 multiplicado de arriba
				AC = toCal((hitPosition - btRayFrom) * 0.5f); 
		
			}
			// de esta manera no salta directamente del if al else, y no se produce el salto de longitud total a media longitud
			else 
				AC = arm_desired_direction * arm_aim_lenght;

		
			// Calcular la nueva posicion de B
			ik_handler.init( toDX(bone_a->getTranslationAbsolute())
						   , toDX(bone_b->getTranslationAbsolute())
						   , toDX(bone_c->getTranslationAbsolute()));

			ik_handler.a = toDX(bone_a->getTranslationAbsolute());
			ik_handler.c = toDX(bone_a->getTranslationAbsolute() + AC);
			ik_handler.updateB( );
		}
		

		// Traspasar la posicion del IK a los bones del model

		// Corregir 'b'
		TBoneAdjust adj_b;
		adj_b.init( model, bone_a->getCoreBone()->getName().c_str() );
		adj_b.local_axis.set( 1,0,0 );
		adj_b.world_target = toCal( ik_handler.b );
		adj_b.update(amount);

		TBoneAdjust adj_twist;
		adj_twist.init( model, bone_twist_arm->getCoreBone()->getName().c_str() );
		adj_twist.local_axis.set( 1,0,0 );
		adj_twist.world_target = toCal( ik_handler.b );
		adj_twist.update(amount);

		TBoneAdjust adj_c;
		adj_c.init( model, bone_b->getCoreBone()->getName().c_str() );
		adj_c.local_axis.set( 1,0,0 );
		adj_c.world_target = toCal( ik_handler.c );
		adj_c.update(amount);

		adj_twist.init( model, bone_twist_forearm->getCoreBone()->getName().c_str() );
		adj_twist.local_axis.set( 1,0,0 );
		adj_twist.world_target = toCal( ik_handler.c );
		adj_twist.update(amount);
	}

	void drawIK() {
		unsigned color =  D3DCOLOR_XRGB( 0, 255, 0 );
		if( ik_handler.state == TIKHandler::TOO_FAR )
			color = D3DCOLOR_XRGB( 255, 0, 0 );
		if( ik_handler.state == TIKHandler::TOO_CLOSE )
			color = D3DCOLOR_XRGB( 255, 0, 255 );
		D3DXVECTOR3 offset( 0.01f, 0.01f, 0.01f );
		drawLineD3X( ik_handler.a + offset, ik_handler.b + offset, color );
		drawLineD3X( ik_handler.b + offset, ik_handler.c + offset, color );
		//drawLineD3X( ik_handler.a + offset, ik_handler.c + offset, D3DCOLOR_XRGB( 0, 255, 0 ) );

		/*printf2D( 1000, 200, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "delay: %f", delay);
		printf2D( 1000, 220, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "time: %f", time);
		printf2D( 1000, 240, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "amount: %f", amount);
		printf2D( 1000, 260, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "blendingIn: %d", blendingIn);
		printf2D( 1000, 280, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "blendingOut: %d", blendingOut);*/
	}
};


#endif
