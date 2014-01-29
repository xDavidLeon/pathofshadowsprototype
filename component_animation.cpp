#include "component_animation.h"
#include "component_model.h"
#include "component_automat.h"
#include "world.h"
#include "bone_adjust.h"
#include "entity_manager.h"
#include "system_light.h"
#include "system_shadow.h"
#include "component_light.h"
#include "component_bt.h"
#include "system_light.h"
#include "component_shadow_actions.h"


AnimationComponent::AnimationComponent(Entity* e, std::string name) : Component(e)
{
	entity = e;
	_name = name;
	_core_model = new CCoreModel("my_core_skel");
	_core_model->load( name );
	_model = new CModel( _core_model, entity );
	_model->attachMesh( _model->getCore()->mesh_id );
	_model->init();

	chain_adj.init();

	if( entity->type == "ENEMY" )
	{
		chain_adj.add( _model, "Bip001 Spine1", 0.05f );
		chain_adj.add( _model, "Bip001 Spine2", 0.1f );
		chain_adj.add( _model, "Bip001 Neck", 0.3f );
		chain_adj.add( _model, "Bip001 Head", 1.0f );
	}
	else if( entity->type == "PLAYER" )
	{
		chain_adj.add( _model, "Bip001 Spine1", 0.01f );
		chain_adj.add( _model, "Bip001 Spine2", 0.025f );
		chain_adj.add( _model, "Bip001 Neck", 0.05f );
		chain_adj.add( _model, "Bip001 Head", 1.0f );
		ik_arm.initTwist( _model, "Bip001 L Hand", "Bip001 LUpArmTwist", "Bip001 L ForeTwist");

		#ifdef EDU_DBG
		//enabled = false;
		#endif
	}
	else if( entity->type == "CROW" || entity->type == "GODDESS" )
	{
	}
	else if( entity->name == "door")
	{
	}
	// cualquier otro objeto animado
	else
	{
		blendCycle(name, 1.0f, 0.0f);
	}


	if( name.compare("xu_farolillo") == 0 )
	{
		Entity *entity = EntityManager::get().createEntity();
		LightComponent * light = new LightComponent(entity,LIGHT_TYPE::LIGHT_POINT,D3DXCOLOR(255,164,98,255));
		light->setRadius(2.5f);
		light->light_intensity = 1.0f;
		light->isTorch = true;
		EntityManager::get().addComponent(light, entity);
		entity->name = "light_farolillo";
		entity->type = "LIGHT";

		btTransform bt_trans;
		bt_trans.setIdentity();
		bt_trans.setOrigin(btVector3(0,1.0f,0.0f)); // offset para cuadrarlo, pero se sobreescribe con el attachEntityToBone
		TransformComponent* transform = new TransformComponent(entity,bt_trans);
		EntityManager::get().addComponent(transform,entity);

		TransformComponent * enemy = EntityManager::get().getComponent<TransformComponent>(e);
		transform->setParent(enemy);

		//enemy->setChild(transform);

		//attachEntityToBone(entity, "Bip001 farol");

		//LightSystem::get().addLight(entity, light->getType());
	}


	_last_front = btVector3(0,0,1);
	// para debug, se puede eliminar posteriormente
	_angle = 0.0f;

	if(entity->type != "CROW") getKillAnimations();

	//attachEntityToBone(
}

void AnimationComponent::init(void)
{
	removeActions(0);
	clearCycles(0);
 
	if( entity->type != "PLAYER" && entity->type != "ENEMY")
		blendCycle(_name, 1.0f, 0.0f);
	else
		blendCycle("idle", 1.0f, 0.0f);
		
}

AnimationComponent::~AnimationComponent(void)
{
	delete _core_model;
	delete _model;
}

void AnimationComponent::update(float delta, bool only_position)
{
	_model->update( delta, only_position );
	if(_name.compare("shadow") == 0)	
		ik_arm.updateAim(delta);
	chain_adj.update( delta );
	updateAttachedEntities(delta);
	//if( ik_enable ) ik_bone.update( ik_destination );
}

void AnimationComponent::updateAttachedEntities(float delta)
{
	std::map<Entity*, std::string>::iterator attached_entity;
	TransformComponent* transformC;
	CalVector bone_trans;
	//CalQuaternion bone_rot;
	CalBone* bone;
	int		 bone_id;

	for(attached_entity = _attached_entities.begin(); attached_entity != _attached_entities.end(); attached_entity++) {
		
		//multiplicar la tranformada de la entity por la del bone attached

		transformC = EntityManager::get().getComponent<TransformComponent>(attached_entity->first);

		bone_id = _model->getSkeleton()->getCoreSkeleton()->getCoreBoneId( attached_entity->second );
		bone = _model->getSkeleton()->getBone( bone_id );

		bone_trans = bone->getTranslation( );
		//bone_rot = bone->getRotation( );

		btVector3 to = toBullet(bone_trans);
		transformC->transform->setOrigin(btVector3(-to.getX(),to.getY(),to.getZ()));
		//transformC->transform->setRotation(toBullet(bone_rot ));
	}

}

void AnimationComponent::attachEntityToBone( Entity* entity, std::string bone )
{
	_attached_entities[entity] = bone;
}

void AnimationComponent::render()
{
	
	//chain_adj.render();
	////_model->renderMesh();
	//
	//TransformComponent* transform = EntityManager::get().getTransformComponent(_entity);
	//btVector3 front = EntityManager::get().getTransformComponent(_entity)->getFront();
	//front.setX( front.getX() * -1 );
	//drawLineD3X( toDX( transform->getPosition() ), toDX(transform->getPosition() + front), 0xffff00ff );

}

void AnimationComponent::renderDebug(size_t index)
{
	_model->renderSkelLines();
	chain_adj.render();
	ik_arm.drawIK();

	index *= 400;
	index += 5;

	BTComponent* enemy_bt = EntityManager::get().getComponent<BTComponent>(_model->_entity);

	if( enemy_bt )
	{
		//printf2D( index, 140, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "%s", enemy_bt->getBT()->getCurrentAction()->c_str());
		printf2D( index, 140, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "bt action: ");
		printf2D( index + 200, 140, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "%s", enemy_bt->getBT()->getPreviousAction()->c_str());
	}

	//printf2D( index + 200, 160, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "angle: %f", _angle);

	printf2D( index + 200, 160, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "%s", _model->_entity->name.c_str());


	printf2D( index, 160, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "Animations:");
			   
	printf2D( index, 180, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "Cycles:");

	std::list<CalAnimationCycle*> cycle_list = _model->getMixer()->getAnimationCycle();

	std::list<CalAnimationCycle *>::iterator iteratorAnimationCycle;
	iteratorAnimationCycle = cycle_list.begin();
	int column = 0;

	while(iteratorAnimationCycle != cycle_list.end())
	{
		CalAnimationCycle* front = (*iteratorAnimationCycle);
		std::string name = front->getCoreAnimation()->getName();
		printf2D( index, 200.0f + column * 80.0f, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "%s", name.c_str());
		printf2D( index, 200.0f + column * 80.0f + 20.0f, D3DCOLOR_ARGB( 255, 255, 255, 255 ), " weight: %f", front->getWeight(), front->getTime());
		printf2D( index, 200.0f + column * 80.0f + 40.0f, D3DCOLOR_ARGB( 255, 255, 255, 255 ), " time: %f", front->getTime());
		++column;
		iteratorAnimationCycle++;
	}

	printf2D( index + 200, 180, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "Actions:");
	std::list<CalAnimationAction*> action_list = _model->getMixer()->getAnimationActionList();
	std::list<CalAnimationAction *>::iterator iteratorAnimationAction;
	iteratorAnimationAction = action_list.begin();
	column = 0;

	while(iteratorAnimationAction != action_list.end())
	{
		CalAnimationAction* front = (*iteratorAnimationAction);
		std::string name = front->getCoreAnimation()->getName();
		printf2D( index + 200.0f, 200.0f + column * 80.0f, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "%s", name.c_str());
		printf2D( index + 200.0f, 200.0f + column * 80.0f + 20.0f, D3DCOLOR_ARGB( 255, 255, 255, 255 ), " weight: %f", front->getWeight());
		printf2D( index + 200.0f, 200.0f + column * 80.0f + 40.0f, D3DCOLOR_ARGB( 255, 255, 255, 255 ), " time: %f", front->getTime());
		printf2D( index + 200.0f, 200.0f + column * 80.0f + 60.0f, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "%f", getDuration(name));
		++column;
		iteratorAnimationAction++;
	}
	
}

CModel* AnimationComponent::getModel() const
{
	return _model;
}

void AnimationComponent::setMixerWorldTransform(TransformComponent* tranform_component)
{
	CalVector loc = toCal( tranform_component->getPosition() );
	ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(entity);
	if( model->getMesh() )	loc.y -= model->getMesh()->aabb.half.y;
	

	loc /= _model->scale;

	CalQuaternion q = toCal(tranform_component->transform->getRotation());

	//// Pasarlas al mixer 
	_model->getMixer()->setWorldTransform( loc, q );
}

bool AnimationComponent::blendCycle(const std::string name, float weight, float delay)
{
	return _model->getMixer()->blendCycle( _model->getCore()->_animations[name], weight, delay );
}

bool AnimationComponent::clearCycle(const std::string name, float delay)
{
	return _model->getMixer()->clearCycle( _model->getCore()->_animations[name], delay );
}

bool AnimationComponent::clearCycles(float delay)
{
	std::list<CalAnimationCycle*> cycle_list = _model->getMixer()->getAnimationCycle();

	std::list<CalAnimationCycle *>::iterator iteratorAnimationCycle;
	iteratorAnimationCycle = cycle_list.begin();

	while(iteratorAnimationCycle != cycle_list.end())
	{
		std::string name = (*iteratorAnimationCycle)->getCoreAnimation()->getName();
		
		_model->getMixer()->clearCycle( _model->getCore()->_animations[name], delay );

		iteratorAnimationCycle++;
	}
	
	return true;
}

bool AnimationComponent::executeAction(const std::string name, float delayIn, float delayOut, float weightTarget, bool autoLock)
{
	return _model->getMixer()->executeAction( _model->getCore()->_animations[name], delayIn, delayOut, weightTarget, autoLock );
}

bool AnimationComponent::removeAction(const std::string name, float blendOut)
{
	return _model->getMixer()->removeAction( _model->getCore()->_animations[name], blendOut );
}

bool AnimationComponent::removeActions(float blendOut)
{
	std::list<CalAnimationAction*> action_list = _model->getMixer()->getAnimationActionList();
	std::list<CalAnimationAction *>::iterator iteratorAnimationAction;
	iteratorAnimationAction = action_list.begin();

	while(iteratorAnimationAction != action_list.end())
	{
		(*iteratorAnimationAction)->remove( blendOut );
		
		iteratorAnimationAction++;
	}

	return true;
}

bool AnimationComponent::actionOn(const std::string name)
{
	return _model->getMixer()->actionOn( _model->getCore()->_animations[name] );
}

bool AnimationComponent::actionBlocked(const std::string name)
{
	CalCoreAnimation* animation = _model->getCore()->getCoreAnimation( _model->getCore()->_animations[name] );
	
	std::list<CalAnimationAction*> action_list = _model->getMixer()->getAnimationActionList();
	std::list<CalAnimationAction *>::iterator iteratorAnimationAction;
	iteratorAnimationAction = action_list.begin();

	while(iteratorAnimationAction != action_list.end())
	{
		// find the specified action and remove it
		if((*iteratorAnimationAction)->getCoreAnimation() == animation )
		{
			if( (*iteratorAnimationAction)->getState() == 6 ) //STOPED
			{
				(*iteratorAnimationAction)->remove( 0 );
				return true;
			}
			else
				return false;
		}
		iteratorAnimationAction++;
	}
	return false;
}


float AnimationComponent::getDuration(const std::string name)
{
	float duration;
	_model->getMixer()->animationDuration( _model->getCore()->_animations[name], &duration );
	return duration;
}

int AnimationComponent::getNumFrames(const std::string name)
{
	return getDuration(name) * 30;
}

float AnimationComponent::getTime(const std::string name)
{
	CalCoreAnimation* animation = _model->getCore()->getCoreAnimation( _model->getCore()->_animations[name] );
	
	std::list<CalAnimationAction*> action_list = _model->getMixer()->getAnimationActionList();
	std::list<CalAnimationAction *>::iterator iteratorAnimationAction;
	iteratorAnimationAction = action_list.begin();

	while(iteratorAnimationAction != action_list.end())
	{
		// find the specified action and remove it
		if((*iteratorAnimationAction)->getCoreAnimation() == animation )
		{
			return (*iteratorAnimationAction)->getTime();
		}
		iteratorAnimationAction++;
	}

	return false;
}

bool AnimationComponent::setTimeFactor(float factor)
{
	_model->getMixer()->setTimeFactor(factor);
	return true;
}

bool AnimationComponent::isTimeInsideInterval(const std::string name, float first_time_factor, float second_time_factor)
{
	float time = getTime(name);
	float duration = getDuration(name);

	if( time > first_time_factor * duration && time < second_time_factor * duration)
		return true;

	return false;
}

bool AnimationComponent::isFrameNumber(const std::string name, int frame)
{
	float float_frame = getTime(name) * 30.0f;
	size_t int_frame = (size_t)float_frame;
				
	return int_frame == frame;
}

bool AnimationComponent::lookAt(const btVector3 &position, float delta)
{
	float blend_speed = 1.0f;

	std::vector< TBoneAdjust >::iterator i = chain_adj.adjs.begin();
	
	while( i != chain_adj.adjs.end() )
	{
		i->world_target.blend(blend_speed * delta, toCal(position));
		++i;
	}
	return true; 
}

bool AnimationComponent::lookTowards(const btVector3 &direction, float delta)
{
	TransformComponent* transform = EntityManager::get().getComponent<TransformComponent>(entity);
	btVector3 world_direction = transform->transform->getBasis() * direction;
	btVector3 position = transform->getPosition();

	// PUTA MERDA DE L'EIX X QUE HEM D'ESBRINAR

	// aqui comprobar si el angulo es demasiado grande
	btVector3 front = transform->getFront();
	front.setX( front.getX() * -1 );
	_angle = front.angle(_last_front);
	
	float blend_speed;

	if( _angle > 0.08f ) // este dato es sacado de la rotacion del xu
	{
		#ifdef EDU_DBG
			//dbg("angle: %f\n", _angle); 
		#endif 
		
		//blend_speed = 1.0f/delta;
		blend_speed = 10.0f;
		/*dbg("blend: %f\n", blend_speed); */
		world_direction = front * direction.getZ();
		world_direction.setY(world_direction.getY() + direction.getY());
	}
	else blend_speed = 1.0f;
	
	std::vector< TBoneAdjust >::iterator i = chain_adj.adjs.begin();
	while( i != chain_adj.adjs.end() )
	{
		i->world_target.blend(blend_speed * delta, toCal(position + world_direction));
		++i;
	}

	_last_front = front;

	return true; 
}

void AnimationComponent::blendAimIn( float delay )
{
	ik_arm.blendAimIn( delay );
}

void AnimationComponent::blendAimOut( float delay )
{
	ik_arm.blendAimOut( delay );
}

void AnimationComponent::blendLookAtIn( float delay )
{
	//if(chain_adj.amount != 1.0f)
		chain_adj.blendLookAtIn( delay );
}

void AnimationComponent::blendLookAtOut( float delay )
{
	//if(chain_adj.amount != 0.0f)
		chain_adj.blendLookAtOut( delay );
}

size_t AnimationComponent::getLessPlayedAnimationIndex(std::vector<AnimationInfo*> _animation_infos)
{
	size_t index = 0;
	unsigned min_times_played = UINT_MAX;

	for (size_t i = 0; i<_animation_infos.size(); ++i)
	{
		if(_animation_infos[i]->num_times_played < min_times_played)
		{	
			index = i;
			min_times_played = _animation_infos[i]->num_times_played;
		}
	}
	
	_animation_infos[index]->num_times_played++;
	return index;
}

AUTPlayer::VictimType AnimationComponent::chooseSilentKillAnimation(const btVector3 &player_pos, const btVector3 &enemy_pos)
{
	ShadowActionsComponent* p_sac = EntityManager::get().getComponent<ShadowActionsComponent>(World::instance()->getPlayer());
	bool player_Shadow = p_sac->getVisibility() == playerVisibility::ONSHADOW;
	bool pl, dl, msh;
	btVector3 pos_vOffset = enemy_pos + btVector3(0,-0.6f,0);
	pl = LightSystem::get().posInPointLight(pos_vOffset);
	dl = LightSystem::get().posInDirectionalLight(pos_vOffset);
	msh = ShadowSystem::get().checkPosInShadows(D3DXVECTOR3(pos_vOffset));

	bool enemy_Shadow = !pl && (!dl || msh);

	AUTPlayer::VictimType victimType = ((AUTPlayer*)EntityManager::get().getComponent<AutomatComponent>(entity)->getAutomat())->_victimType;
	/*dbg("shadowplayer %i\n", player_Shadow);	
	dbg("shadowenemy %i\n", enemy_Shadow);*/

	if( victimType == AUTPlayer::VictimType::PANIC )
	{
		_silent_kill_animation = _animation_infos_panic[getLessPlayedAnimationIndex(_animation_infos_panic)]->name;
		return AUTPlayer::VictimType::PANIC;
	}
	else if( victimType == AUTPlayer::VictimType::AERIAL )
	{
		_silent_kill_animation = _animation_infos_air[getLessPlayedAnimationIndex(_animation_infos_air)]->name;
		return AUTPlayer::VictimType::AERIAL;
	}
	else if( victimType == AUTPlayer::VictimType::BLEND )
	{
		_silent_kill_animation = _animation_infos_blend[getLessPlayedAnimationIndex(_animation_infos_blend)]->name;
		return AUTPlayer::VictimType::BLEND;
	}
	else if(player_Shadow && enemy_Shadow)
	{
		_silent_kill_animation = _animation_infos_shadow[getLessPlayedAnimationIndex(_animation_infos_shadow)]->name;
		return AUTPlayer::VictimType::SHADOW;
	}
	else
	{
		_silent_kill_animation = _animation_infos[getLessPlayedAnimationIndex(_animation_infos)]->name;
		return AUTPlayer::VictimType::NORMAL;
	}
}

void AnimationComponent::getKillAnimations() {
	
	std::map<std::string, CameraSystem::KillCameraInfo*> kill_cameras = CameraSystem::get()._killCameras;
	std::map<std::string, CameraSystem::KillCameraInfo*>::iterator animation;
	
	AnimationInfo* animation_info;
  
	for(animation = kill_cameras.begin(); animation != kill_cameras.end(); animation++) {
		
		animation_info  = new AnimationInfo();
		animation_info->name = animation->first;
		animation_info->num_times_played = 0;

		if(animation->second->type_panic)
			_animation_infos_panic.push_back(animation_info);
		else if(animation->second->type_air)
			_animation_infos_air.push_back(animation_info);
		else if(animation->second->type_blend)
			_animation_infos_blend.push_back(animation_info);
		else if(animation->second->type_shadow)
			_animation_infos_shadow.push_back(animation_info);
		else
			_animation_infos.push_back(animation_info);

	}

}

void AnimationComponent::movePlayerToKill(const std::string silent_kill_animation, TransformComponent* tranform_component)
{
	TransformComponent* transformC = EntityManager::get().getComponent<TransformComponent>(entity);

	(*transformC->transform) = (*tranform_component->transform);
	
	std::map<std::string, CameraSystem::KillCameraInfo*> kill_cameras = CameraSystem::get()._killCameras;
	
	transformC->moveLocal(kill_cameras[silent_kill_animation]->shadow_translation);

}

void AnimationComponent::moveSnakeToKill(TransformComponent* tranform_component)
{
	TransformComponent* transformC = EntityManager::get().getComponent<TransformComponent>(entity);
	(*transformC->transform) = (*tranform_component->transform);
}

void AnimationComponent::blockAim()
{
	ik_arm.blocked = true;
}

void AnimationComponent::unblockAim()
{
	ik_arm.blocked = false;
}

void AnimationComponent::turn(float angle)
{
	//dbg("turning!! %f\n", angle);

	float abs_angle = fabs(angle);

	//dbg("fabs(abs_angle - M_PI)     %f\n", fabs(abs_angle - M_PI));
	//dbg("fabs(abs_angle - M_PI_2)   %f\n", fabs(abs_angle - M_PI_2));

	if (fabs(abs_angle - M_PI) < 0.2)
	{
		//dbg("angle look at %f debe ser %f\n", angle, M_PI);
		if(angle < 0)
			executeAction("turn_around_left", 0.2f, 0.5f);
		else
			executeAction("turn_around_right", 0.2f, 0.5f);

	}
	else if(fabs(abs_angle - M_PI_2) < 0.2)
	{
		if(angle < 0)
		{
			//dbg("angle look at %f debe ser %f\n", angle, M_PI);
			executeAction("turn_left", 0.2f, 0.5f);

		}
		else
		{
			//dbg("angle look at %f debe ser %f\n", angle, -M_PI);
			executeAction("turn_right", 0.2f, 0.5f);

		}
	}	
}

void AnimationComponent::turnPlayer(float angle)
{
	//dbg("turning!! %f\n", angle);

	float abs_angle = fabs(angle);

	//dbg("fabs(abs_angle - M_PI)     %f\n", fabs(abs_angle - M_PI));
	//dbg("fabs(abs_angle - M_PI_2)   %f\n", fabs(abs_angle - M_PI_2));

	if (fabs(abs_angle - M_PI) < 0.2)
	{
		//dbg("angle look at %f debe ser %f\n", angle, M_PI);
		if(angle < 0)
		{
			//dbg("angle look at %f debe ser MAYOR que %f\n", angle - M_PI, 0);
			//dbg("angle NEGATIVO %f --> %f\n",abs_angle - M_PI, angle );
			if( !actionOn("turn_around_left") )
				executeAction("turn_around_left", 0.5f, 0.5f);
			if( !actionOn("turn_around_left_legs") )
				executeAction("turn_around_left_legs", 0.5f, 0.5f);
		}
		else
		{
			//dbg("angle look at %f debe ser menor que %f\n", angle - M_PI, 0);
			//dbg("angle POSITIVO %f --> %f\n",abs_angle - M_PI, angle );
			if( !actionOn("turn_around_right") )
				executeAction("turn_around_right", 0.5f, 0.5f);
			if( !actionOn("turn_around_right_legs") )
				executeAction("turn_around_right_legs", 0.5f, 0.5f);
		}

	}
	//else if(fabs(abs_angle - M_PI_2) < 0.2)
	//{
	//	if(angle <= 0)
	//	{
	//		//dbg("angle look at %f debe ser %f\n", angle, M_PI);
	//		//executeAction("turn_left", 0.2f, 0.5f);

	//	}
	//	else
	//	{
	//		//dbg("angle look at %f debe ser %f\n", angle, -M_PI);
	//		//executeAction("turn_right", 0.2f, 0.5f);

	//	}
	//}	
}
std::string AnimationComponent::getEnemyType()
{
	if( getName() == "xu" || getName() == "xu_farolillo")
			return "1";
	else if( getName() == "xu_shield" || getName() == "one")
			return "2";
}



