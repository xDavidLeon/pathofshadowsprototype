#include "bt_patrol.h"
#include "d3dx9math.h"
//#include "d3ddefs.h"

#include "world.h"
#include "component_model.h"
#include <vector>
#include "dijkstra.h"
#include "component_player_controller.h"
#include "component_shadow_actions.h"

BTPatrol::BTPatrol(Entity* entity) : BTCommon(entity)
{
	create();

	_goingToFirstWP = false;
	_idleTimeFixed = 10.0f;
	_idleWalkFixed = 2.0f;
	_idleTimeVariable = 5.0f;
}

void BTPatrol::create()
{
	createRoot("enemy_patrol", PRIORITY, NULL, NULL);
		addChild("enemy_patrol", "panic_sequence", SEQUENCE, (btcondition)&BTCommon::checkIfHasToPanic, NULL);
			addChild("panic_sequence", "fall", ACTION, NULL, (btaction)&BTCommon::fall);
			addChild("panic_sequence", "beScared", ACTION, NULL, (btaction)&BTCommon::beScared);
			addChild("panic_sequence", "standUp", ACTION, NULL, (btaction)&BTCommon::standUp);
		addChild("enemy_patrol", "takeDamage", ACTION, (btcondition)&BTCommon::checkDamageReceived, (btaction)&BTCommon::takeDamage);
			addChild("takeDamage", "die", ACTION, NULL, (btaction)&BTCommon::die);
		addChild("enemy_patrol", "attack", ACTION, (btcondition)&BTCommon::checkWarDistance, (btaction)&BTCommon::attack);
		addChild("enemy_patrol", "choose_strategy", PRIORITY, (btcondition)&BTCommon::checkPlayerViewed, NULL);
			addChild("choose_strategy", "alert_and_chase", PRIORITY, (btcondition)&BTCommon::checkIfWantToAlert, NULL);
				addChild("alert_and_chase", "alertAllies", ACTION, (btcondition)&BTCommon::checkAlertNotThrown, (btaction)&BTCommon::alertAllies);
				addChild("alert_and_chase", "unsheathe_and_chase", PRIORITY, NULL, NULL);
					addChild("unsheathe_and_chase", "unsheathe", ACTION, (btcondition)&BTCommon::checkSheathed, (btaction)&BTCommon::unsheathe);
					addChild("unsheathe_and_chase", "chasePlayer", ACTION, NULL, (btaction)&BTCommon::chasePlayer);
			addChild("choose_strategy", "unsheathe_and_chase2", PRIORITY, NULL, NULL);
				addChild("unsheathe_and_chase2", "unsheathe2", ACTION, (btcondition)&BTCommon::checkSheathed, (btaction)&BTCommon::unsheathe);
				addChild("unsheathe_and_chase2", "chasePlayer2", ACTION, NULL, (btaction)&BTCommon::chasePlayer);
		addChild("enemy_patrol", "track_target", PRIORITY, (btcondition)&BTCommon::checkTrackingTarget, NULL);
			addChild("track_target", "trackNextPoint", ACTION, (btcondition)&BTCommon::checkHasTrackPoints, (btaction)&BTCommon::trackNextPoint);
			addChild("track_target", "stopTracking", ACTION, NULL, (btaction)&BTCommon::stopTracking);

		//addChild("enemy_patrol", "look_and_go_with_chaser", SEQUENCE, (btcondition)&BTCommon::checkAllyChasingPlayer, NULL);
		//	addChild("look_and_go_with_chaser", "lookAtChaser", ACTION, NULL, (btaction)&BTCommon::lookAtChaser);
		//	addChild("look_and_go_with_chaser", "unsheatheIfHasTo3", ACTION, NULL, (btaction)&BTCommon::unsheatheIfHasTo);
		//	addChild("look_and_go_with_chaser", "goWhereAllyGoes", ACTION, NULL, (btaction)&BTCommon::goWhereAllyGoes);

		addChild("enemy_patrol", "look_corpse", PRIORITY, (btcondition)&BTCommon::checkCorpseSeen, NULL);
			addChild("look_corpse", "go_and_look_corpse", PRIORITY, (btcondition)&BTCommon::checkIfFirstCorpse, NULL);
				addChild("go_and_look_corpse", "lookAtCorpse_a", ACTION, (btcondition)&BTCommon::checkNearCorpse, (btaction)&BTCommon::lookAtCorpse);
				addChild("go_and_look_corpse", "goToCorpse", ACTION, NULL, (btaction)&BTCommon::goToCorpse);
			addChild("look_corpse", "lookAtCorpse_b", ACTION, NULL, (btaction)&BTCommon::lookAtCorpse);
		addChild("enemy_patrol", "remember_corpse", SEQUENCE, (btcondition)&BTCommon::checkSeeingCorpse, NULL);
			addChild("remember_corpse", "rememberCorpse", ACTION, NULL, (btaction)&BTCommon::rememberCorpse);
			addChild("remember_corpse", "unsheatheIfHasTo2", ACTION, NULL, (btaction)&BTCommon::unsheatheIfHasTo);
		addChild("enemy_patrol", "look_and_decide", SEQUENCE, (btcondition)&BTCommon::checkWarningInBlackboard, NULL);
			addChild("look_and_decide", "lookToAlly", ACTION, NULL, (btaction)&BTCommon::lookToAlly);
			addChild("look_and_decide", "alerted_decide", PRIORITY, NULL, NULL);
				addChild("alerted_decide", "go_with_ally", SEQUENCE, (btcondition)&BTCommon::checkWantToGo, NULL);
					addChild("go_with_ally", "unsheatheIfHasTo", ACTION, NULL, (btaction)&BTCommon::unsheatheIfHasTo);
					addChild("go_with_ally", "goWithAlly", ACTION, NULL, (btaction)&BTCommon::goWithAlly);
				addChild("alerted_decide", "ignoreWarning", ACTION, NULL, (btaction)&BTCommon::ignoreWarning);
		addChild("enemy_patrol", "search_and_go", SEQUENCE, (btcondition)&BTCommon::checkLoudNoiseHeard, NULL);
			addChild("search_and_go", "lookAtSoundSource", ACTION, NULL, (btaction)&BTCommon::lookAtSoundSource);
			addChild("search_and_go", "goToSoundSource", ACTION, NULL, (btaction)&BTCommon::goToSoundSource);
		addChild("enemy_patrol", "lookForPlayer", ACTION, (btcondition)&BTCommon::checkPlayerPartiallyViewed, (btaction)&BTCommon::lookForPlayer);
		addChild("enemy_patrol", "search_player", PRIORITY, (btcondition)&BTCommon::checkSearching, NULL);
			addChild("search_player", "stop_searching", SEQUENCE, (btcondition)&BTCommon::checkStoppedSearching, NULL);
				addChild("stop_searching", "stopSearching", ACTION, NULL, (btaction)&BTCommon::stopSearching);
				addChild("stop_searching", "sheathe", ACTION, NULL, (btaction)&BTCommon::sheathe);
			addChild("search_player", "lookAt", ACTION, (btcondition)&BTCommon::checkHasLookAt, (btaction)&BTCommon::lookAt);
			addChild("search_player", "chooseLookAt", ACTION, NULL, (btaction)&BTCommon::chooseLookAt);

		addChild("enemy_patrol", "patrolling", PRIORITY, NULL, NULL);
			addChild("patrolling", "generatePathToWP", ACTION, (btcondition)&BTPatrol::checkIfNotPatroling, (btaction)&BTPatrol::generatePathToWP);
			addChild("patrolling", "idle", ACTION, (btcondition)&BTPatrol::checkIdle, (btaction)&BTPatrol::idle);
			addChild("patrolling", "goToFirstWP", ACTION, (btcondition)&BTPatrol::checkGoingToFirstWP, (btaction)&BTPatrol::goToFirstWP);
			addChild("patrolling", "changeCurrentWP", ACTION, (btcondition)&BTPatrol::checkcurrentWPReached, (btaction)&BTPatrol::changeCurrentWP);
			addChild("patrolling", "goToCurrentWP", ACTION, NULL, (btaction)&BTPatrol::goToCurrentWP);
}

void BTPatrol::render()
{
	//Transform
	unsigned p_color = D3DCOLOR_ARGB( 255, 255, 255, 0 );
	const btVector3& pos = _eD->_transformC->transform->getOrigin();
	btVector3 v_aux;
	_eD->_transformC->getLeftXinv(v_aux);
	drawLine_bt(pos, pos+v_aux, p_color);
	drawLine_bt(pos, pos+_eD->_transformC->getUp(), p_color);
	_eD->_transformC->getFrontXinv(v_aux);
	drawLine_bt(pos, pos+v_aux, p_color);

	//drawConeXZ_bt(_eD->_eyePos, _eD->_transformC->getFront(), _eD->_halfFov, _eD->_dbgColor, sqrtf(_eD->_visionDistSq));
	//drawConeXZ_bt(_eD->_eyePos, _eD->_transformC->getFront(), _eD->_halfFovPeriferial, D3DCOLOR_ARGB(255, 255, 0, 255), sqrtf(_eD->_periferialDistSq));
	if(_eD->_attentionDegree != attentionDegrees::NORMAL)
	{
		unsigned text_color = D3DCOLOR_ARGB( 255, 0, 0, 255 );
		printf2D( g_App.GetWidth()*3/5, 190, text_color, "vision percent: %.4f", _eD->_visionPercent);
		printf2D( g_App.GetWidth()*3/5, 210, text_color, "enemy state: %s", _action.c_str());
		printf2D( g_App.GetWidth()*3/5, 230, text_color, "enemy alert state: %d", _eD->_attentionDegree);
	}

	//dbg lookAt
	//drawLine_bt(_transformC->getPosition(), _transformC->getPosition() + _lookAt, D3DCOLOR_ARGB(255, 0,0,0));
	//printf2D( g_App.GetWidth()*3/5, 240, text_color, "lookAt time: %.2f", _lookAtClock.getCount());
	//printf2D( g_App.GetWidth()*3/5, 260, text_color, "forget time time: %.2f", _forgetClock.getCount());

	//dbg("elapsed_time: %f\n", World::instance()->_elapsedTime);

	_eD->renderPath(D3DCOLOR_ARGB( 255, 0, 0, 255 ), 0.5f);
	if(_eD->_path.size()) drawLine_bt(_eD->_transformC->getPosition(), _eD->_path.at(0), D3DCOLOR_ARGB( 255, 255, 255, 255 ));
}

void BTPatrol::init()
{
	BTCommon::init();
	_currentWP = 0;
}

void BTPatrol::addWayPoint(const btVector3& wp)
{
	_wayPoints.push_back(wp);
}

//Conditions--------------------------------------------------------
bool BTPatrol::checkIfNotPatroling()
{
	if(*_previousAction == "generatePathToWP" ||
	   *_previousAction == "idle" ||
	   *_previousAction == "goToFirstWP" ||
	   *_previousAction == "changeCurrentWP" ||
	   *_previousAction == "goToCurrentWP")
	{
		return false;
	}
	else
	{
		_idleClock.setTarget(_idleTimeFixed+rand()%_idleTimeVariable);
		return true;
	}
}

bool BTPatrol::checkIdle()
{
	if(_doingIdle) return true;

	if(_eD->_attentionDegree == attentionDegrees::PERMANENT_CAUTION) return false;

	if(!_idleClock.hasTarget()) return false;

	if(_idleClock.count(1.0f/60.0f))
	{
		_idleClock.setTarget(_idleWalkFixed+rand()%_idleTimeVariable);
		return true;
	}
	else return false;
}

bool BTPatrol::checkGoingToFirstWP()
{
	return _goingToFirstWP;
}

bool BTPatrol::checkcurrentWPReached()
{
	return manhattanDist(_eD->_transformC->getPosition(), _wayPoints.at(_currentWP)) < 0.5f;
}

//Actions-----------------------------------------------------------
int BTPatrol::generatePathToWP()
{
	assert(_wayPoints.size() || fatal("El patrullero: %s no tiene waypoints!!\n", _entity->name.c_str()));

	//_currentWP = rand()%_wayPoints.size();
	_currentWP = 0;
	DijkstraGraph::get().computePath(_eD->_transformC->getPosition(), _wayPoints.at(_currentWP), _pathToWP);
	_goingToFirstWP = true;
	return LEAVE;
}

int BTPatrol::idle()
{
	/*if( _animation_component->actionOn("idle_walk") )
		return STAY;

	return LEAVE;*/

	_doingIdle = true;

	if(_idleClock.count(1.0f/60.0f))
	{
		_idleClock.setTarget(_idleTimeFixed+rand()%_idleTimeVariable);
		_doingIdle = false;
	}

	return LEAVE;
}

int BTPatrol::goToFirstWP()
{
	_eD->goToNextTrackPoint(_eD->_delta, _pathToWP);
	if(!_pathToWP.size()) _goingToFirstWP = false;
	return LEAVE;
}

int BTPatrol::changeCurrentWP()
{
	_currentWP++;
	_currentWP = _currentWP%_wayPoints.size();
	return LEAVE;
}

int BTPatrol::goToCurrentWP()
{
	//Si entre 2 puntos de patrulla no hubiera acceso directo habria que poner pathfinding aqui

	//Rotar hacia el punto
	_eD->_transformC->approximateFront_p(_wayPoints.at(_currentWP), _eD->_rotateVel*_eD->_delta);

	//Ir hacia el punto
	_eD->advance(_eD->_transformC->getFront()*_eD->_delta*_eD->_currentVel);

	return LEAVE;
}
