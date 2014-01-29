#include "behaviour_tree.h"
#include <cassert> 
#include "globals.h"
#include "component_enemy_data.h"
#include "component_bt.h"
#include "bt_common.h"
#include "entity_manager.h"
#include "bt_goddess.h"


BehaviourTree::BehaviourTree(Entity* entity)
{
	_entity = entity;
	_action = "";
	
	if(entity->type != "GODDESS" && entity->type != "CINE_SEQ")
	{	
		_animation_component = EntityManager::get().getComponent<AnimationComponent>(entity);
		assert( _animation_component );
	}
	else
		_animation_component = NULL;

	_target = NULL;
	_random_look_at = btVector3(FLT_MAX,FLT_MAX,FLT_MAX);
}

btnode *BehaviourTree::createNode(const string& s)
{
	if (findNode(s)!=NULL) 
	{
		assert(fatal("Error: node %s already exists\n",s.c_str()));
		return NULL;	// error: node already exists
	}

	btnode *btn=new btnode(s);
	tree[s]=btn;
	return btn;
}


btnode *BehaviourTree::findNode(const string& s)
{
	if (tree.find(s)==tree.end()) return NULL;
	else return tree[s];
}


btnode *BehaviourTree::createRoot(const string& s,int type,btcondition btc,btaction bta)
{
	btnode *r=createNode(s);
	r->setParent(NULL);
	root=r;
	r->setType(type);
	if (btc!=NULL) addCondition(s,btc);
	if (bta!=NULL) addAction(s,bta);

	current=NULL;
	_previousAction = &root->getName();

	return r;
}


btnode *BehaviourTree::addChild(const string& parent,const string& son,int type,btcondition btc,btaction bta)
{
	btnode *p=findNode(parent);
	assert(p!=NULL);
	btnode *s=createNode(son);
	p->addChild(s);
	s->setParent(p);
	s->setType(type);
	if (btc!=NULL) addCondition(son,btc);
	if (bta!=NULL) addAction(son,bta);
	return s;
}


void BehaviourTree::recalc(float delta)
{
	if (current==NULL) root->recalc(this);	//If there's a current node, call its recalc. Else, call the root recalc.
	else current->recalc(this);
}

void BehaviourTree::setCurrent(btnode *nc)
{
	current=nc;
}


void BehaviourTree::addAction(const string& s,btaction act)
{
	if (actions.find(s)!=actions.end())
	{
		assert(fatal("Error: node %s already has an action\n",s.c_str()));
		return;	// if action already exists don't insert again...
	}
	actions[s]=act;
}


int BehaviourTree::execAction(const string& s)
{
	if (actions.find(s)==actions.end()) 
	{
		assert(fatal("ERROR: Missing node action for node %s\n",s.c_str()));
		return LEAVE; // error: action does not exist
	}
	
	if(_animation_component)
		if( _animation_component->enabled )
			updateAnimation( s, _action);

	if(_entity->type == "GODDESS")
			updateAnimation( s, _action);

	
	#ifdef EDU_DBG
		/*if( s.compare(_action))
			dbg("%s %s\n", _entity->name.c_str() ,s.c_str());*/
	#endif 
	
	_action = s;
	int res = (this->*actions[s])();

	//save the previous action si no estamos en el bt dead
	if(!findNode("dead_enemy")) _previousAction = &findNode(s)->getName();

	return res;
}


void BehaviourTree::addCondition(const string& s,btcondition cond)
{
	if (conditions.find(s)!=conditions.end())
	{
		assert(fatal("Error: node %s already has a condition\n",s.c_str()));
		return;	// if condition already exists don't insert again...
	}
	conditions[s]=cond;
}


bool BehaviourTree::testCondition(const string& s)
{
	if (conditions.find(s)==conditions.end()) return true;	// if no condition defined, we assume TRUE
	return (this->*conditions[s])();
}

void BehaviourTree::updateAnimation(const string& action, const string& previousAction)
{
	//	if(action.compare("goToFirstWP") == 0 || action.compare("goToSoundSource") == 0 || action.compare("lookAtSoundSource") == 0 || action.compare("lookAt") == 0 || action.compare("goToCurrentWP") == 0)
	if(EntityManager::get().getComponent<EnemyDataComponent>(_entity)) headController();

	if(previousAction.compare(action) == 0)  return;

	EnemyDataComponent* enemy_dataC = EntityManager::get().getComponent<EnemyDataComponent>(_entity);
	BTCommon* bt_common = ((BTCommon*)EntityManager::get().getComponent<BTComponent>(_entity)->getBT());
	TransformComponent* transformC = EntityManager::get().getComponent<TransformComponent>(_entity);
	
	// Raven
	if( _entity->type == "CROW")
	{
		if(action.compare("appear") == 0)
		{
			_animation_component->clearCycles(0.0f);
			_animation_component->blendCycle( "idle_fly", 1.0f, 0.0f );
		}
		else if(action.compare("trackNextPoint") == 0)
		{
			_animation_component->clearCycles(1.5f);
			_animation_component->blendCycle( "fly", 1.0f, 1.5f );
		}
	}
	else if( _entity->type == "GODDESS" )
	{
		if(action.compare("flyToPlace") == 0)
		{
			Entity* crow = ((BTGoddess*)EntityManager::get().getComponent<BTComponent>(_entity)->getBT())->getCrowEntity();
			EntityManager::get().getComponent<AnimationComponent>(crow)->executeAction( "reborn", 0.5f, 0.5f );
			EntityManager::get().getComponent<AnimationComponent>(crow)->blendCycle( "idle_fly", 1.0f, 1.5f );
			EntityManager::get().getComponent<AnimationComponent>(crow)->clearCycle( "idle", 1.5f );

		}
		else if(action.compare("beBorn") == 0)
		{
			Entity* crow = ((BTGoddess*)EntityManager::get().getComponent<BTComponent>(_entity)->getBT())->getGoddessEntity();
			EntityManager::get().getComponent<AnimationComponent>(crow)->executeAction( "appear", 0.0f, 3.0f );
		}
		else if(action.compare("givePowers") == 0)
		{
			Entity* goddes = ((BTGoddess*)EntityManager::get().getComponent<BTComponent>(_entity)->getBT())->getGoddessEntity();
			EntityManager::get().getComponent<AnimationComponent>(goddes)->executeAction( "give", 0.5f, 0.5f );
		}
		else if(action.compare("dissappear") == 0)
		{
			Entity* goddes = ((BTGoddess*)EntityManager::get().getComponent<BTComponent>(_entity)->getBT())->getGoddessEntity();
			EntityManager::get().getComponent<AnimationComponent>(goddes)->executeAction( "disappear", 0.5f, 0.0f, 1.0f, true );
		}
		else if(action.compare("appear") == 0)
		{
			Entity* goddes = ((BTGoddess*)EntityManager::get().getComponent<BTComponent>(_entity)->getBT())->getGoddessEntity();
			EntityManager::get().getComponent<AnimationComponent>(goddes)->executeAction( "appear", 0.0f, 0.5f );
		}
		else if(action.compare("poof") == 0)
		{
			Entity* goddes = ((BTGoddess*)EntityManager::get().getComponent<BTComponent>(_entity)->getBT())->getGoddessEntity();
			EntityManager::get().getComponent<AnimationComponent>(goddes)->executeAction( "disappear", 0.5f, 0.0f, 1.0f, true );
		}
	}
	else
	{
		std::string sound_file = "data/sfx/voices/";
		std::string sound_type;
		std::string enemy_type = _animation_component->getEnemyType();

		if(action.compare("goToSoundSource") == 0)
		{
			if(getrandom(0,1) < 1)
				sound_type = "heard_";
			else
				sound_type = "what_";

			sound_file += sound_type + enemy_type + ".mp3";

			//PARAR SONIDO ANTERIOR

			SoundSystem::get().playSFX3D( (_entity->name).c_str(), sound_file.c_str(), "", transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f );

			_animation_component->clearCycles(0.2f);
			_animation_component->blendCycle( "caution", 1.0f, 0.2f );
		}
		else if(action.compare("stopTracking") == 0)
		{
			_animation_component->clearCycles(0.5f);
			_animation_component->blendCycle( "idle_caution", 1.0f, 0.5f );
		}
		else if(action.compare("goToGKPlace") == 0)
		{
			if( enemy_dataC->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
			{
				sound_type = "got_";
				sound_file += sound_type + enemy_type + ".mp3";

				// PARAR SONIDO ANTERIOR

				SoundSystem::get().playSFX3D( (_entity->name).c_str(), sound_file.c_str(), "", transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f );
			}

			_animation_component->clearCycles(0.5f);

			if( enemy_dataC->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
				_animation_component->blendCycle( "caution", 1.0f, 0.5f );
			else
				_animation_component->blendCycle( "walk", 1.0f, 0.5f );
		}
		else if(action.compare("lookAtGK") == 0)
		{
			_animation_component->clearCycles(0.5f);

			if( enemy_dataC->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
				_animation_component->blendCycle( "idle_caution", 1.0f, 0.5f );
			else
				_animation_component->blendCycle( "idle", 1.0f, 0.5f );
		}
		else if(action.compare("goToFirstWP") == 0)
		{
			_animation_component->clearCycles(0.5f);

			if( enemy_dataC->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
				_animation_component->blendCycle( "caution", 1.0f, 0.5f );
			else
				_animation_component->blendCycle( "walk", 1.0f, 0.5f );
		}
		else if(action.compare("goToCurrentWP") == 0)
		{
			_animation_component->clearCycles(0.5f);

			if( enemy_dataC->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
				_animation_component->blendCycle( "caution", 1.0f, 0.5f );
			else
				_animation_component->blendCycle( "walk", 1.0f, 0.5f );
		}
		else if(action.compare("idle") == 0)
		{
			_animation_component->clearCycles(0.5f);

			if( previousAction.compare("goToCurrentWP") == 0 )
			{
				//_animation_component->blendCycle( "idle", 1.0f, 0.5f );
				//_animation_component->executeAction( "idle_walk", 0.2f, 0.2f );
				_animation_component->blendCycle( "idle_walk", 1.0f, 0.5f );
			}
			else
			{
				if( enemy_dataC->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
					_animation_component->blendCycle( "idle_caution", 1.0f, 0.5f );
				else
				{
					if( getrandom(0,1) < 1 )
						_animation_component->blendCycle( "idle", 1.0f, 0.5f );
					else
						_animation_component->blendCycle( "idle_guard", 1.0f, 0.5f );
				}
			}
		}
		else if(action.compare("lookForPlayer") == 0)
		{
			_animation_component->clearCycles(0.5f);

			if( enemy_dataC->_attentionDegree == attentionDegrees::NORMAL)
				_animation_component->blendCycle( "idle", 1.0f, 0.5f );
			else
				_animation_component->blendCycle( "idle_caution", 1.0f, 0.5f );

		}
		else if(action.compare("attack") == 0)
		{
			_animation_component->clearCycles(0.5f);
			_animation_component->blendCycle( "idle_fight", 1.0f, 0.5f );
		}
		else if(action.compare("lookToAlly") == 0)
		{
			_animation_component->clearCycles(0.5f);
			_animation_component->blendCycle( "idle", 1.0f, 0.5f );
		}
		else if(action.compare("goWithAlly") == 0)
		{
			_animation_component->clearCycles(0.5f);
			_animation_component->blendCycle( "caution", 1.0f, 0.5f );
		}
		else if(action.compare("goToCorpse") == 0)
		{
			sound_type = "body_";
			sound_file += sound_type + enemy_type + ".mp3";

			//PARAR SONIDO ANTERIOR

			SoundSystem::get().playSFX3D( (_entity->name).c_str(), sound_file.c_str(), "", transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f );


			_animation_component->clearCycles(0.5f);
			_animation_component->blendCycle( "caution", 1.0f, 0.5f );
		}
		else if(action.compare("idle2") == 0)
		{
			_animation_component->clearCycles(0.5f);
			_animation_component->blendCycle( "idle2", 1.0f, 0.5f );
		}
		else if(action.compare("talkLeft") == 0)
		{
			_animation_component->clearCycles(0.5f);
			_animation_component->blendCycle( "talk_left", 1.0f, 0.5f );
		}
		else if(action.compare("talkRight") == 0)
		{
			_animation_component->clearCycles(0.5f);
			_animation_component->blendCycle( "talk_right", 1.0f, 0.5f );
		}
		
		if(action.compare("chasePlayer") == 0 || action.compare("chasePlayer2") == 0)
		{
			_animation_component->clearCycles(0.5f);
			_animation_component->blendCycle( "run", 1.0f, 0.5f );

			if(previousAction.compare("trackNextPoint") != 0 ) 
			{
				if(getrandom(0,1) < 1)
					sound_type = "get_";
				else
					sound_type = "there_";

				sound_file += sound_type + enemy_type + ".mp3";

				//PARAR SONIDO ANTERIOR

				SoundSystem::get().playSFX3D( (_entity->name).c_str(), sound_file.c_str(), "", transformC->getPosition(), btVector3(0,0,0), false, 1.0f, 1.0f );

				
				//dbg("PREVIOUS ACTION %s\n", previousAction.c_str());
			}
		}
				
	}
	
}

void BehaviourTree::headController(){
	//dbg("GENERATE_WPT\n");
	// seria interessant generar els waypoints en funcio a zones interessants de l'escena
	float delta = 1.0f/60.0f;	

	TransformComponent* transformC = EntityManager::get().getComponent<TransformComponent>(_entity);
	
	float _halfFov = D3DXToRadian(60.0f);
	float _visionDistance = 5.0f;
	float time = 2.0f;

	btVector3 target_position;
	if( _target ) // Si tenemos una transformada objetivo
	{
		target_position = _target->getPosition(); 
		_animation_component->lookAt( target_position, delta );
		//check if "reach"....... cambiar de target?
	}
	else // sino de momento random
	{
		target_position = _random_look_at; 

		//check if "reach"
		if ( /*_transformC->isInsideVisionCone( target_position, _halfFov) && _transformC->getPosition().distance2( target_position ) < _visionDistance 
			 ||*/ timeGetTime() - _time_forget > time * 1000.0f)
		{
			computeRandomWpt(_action);

			target_position = _random_look_at;
			_time_forget = (float)timeGetTime();
		}

		_animation_component->lookTowards( target_position, delta );
	}
	
}

void BehaviourTree::computeRandomWpt(const std::string& state)
{
	// generate
	//float distance_look = _ios->getSpeed() + 1.0f;
	// hack pel teclat
	_random_look_at = btVector3 ((FLOAT)getrandom( -8.0f  , 8.0f), 1.0f , 5.0f);
	//_random_look_at = *_transformC->transform * _random_look_at;


	//dbg("distance_look %f\n", distance_look);
	//dbg("speed %f\n", _ios->getSpeed());
}
