#include "bt_gatekeeper.h"
#include "d3dx9math.h"
//#include "d3ddefs.h"

#include "world.h"
#include "component_model.h"
#include <vector>
#include "dijkstra.h"
#include "component_player_controller.h"
#include "component_shadow_actions.h"
#include "component_bt.h"

BTGatekeeper::BTGatekeeper(Entity* entity) : BTCommon(entity)
{
	create();

	_goingToGKPlace = false;
	_GKPlace = btVector3(0,0,0);
	_GKLookAt = btVector3(0,0,0);

	_secondIdleEnabled = false;
	_allyToTalk = NULL;
	_talkingToLeft = false;
	_thereWasAllyToTalk = false;
}

void BTGatekeeper::create()
{
	createRoot("enemy_gatekeeper", PRIORITY, NULL, NULL);
		addChild("enemy_gatekeeper", "panic_sequence", SEQUENCE, (btcondition)&BTCommon::checkIfHasToPanic, NULL);
			addChild("panic_sequence", "fall", ACTION, NULL, (btaction)&BTCommon::fall);
			addChild("panic_sequence", "beScared", ACTION, NULL, (btaction)&BTCommon::beScared);
			addChild("panic_sequence", "standUp", ACTION, NULL, (btaction)&BTCommon::standUp);
		addChild("enemy_gatekeeper", "takeDamage", ACTION, (btcondition)&BTCommon::checkDamageReceived, (btaction)&BTCommon::takeDamage);
			addChild("takeDamage", "die", ACTION, NULL, (btaction)&BTCommon::die);
		addChild("enemy_gatekeeper", "attack", ACTION, (btcondition)&BTCommon::checkWarDistance, (btaction)&BTCommon::attack);
		addChild("enemy_gatekeeper", "choose_strategy", PRIORITY, (btcondition)&BTCommon::checkPlayerViewed, NULL);
			addChild("choose_strategy", "alert_and_chase", PRIORITY, (btcondition)&BTCommon::checkIfWantToAlert, NULL);
				addChild("alert_and_chase", "alertAllies", ACTION, (btcondition)&BTCommon::checkAlertNotThrown, (btaction)&BTCommon::alertAllies);
				addChild("alert_and_chase", "unsheathe_and_chase", PRIORITY, NULL, NULL);					
					addChild("unsheathe_and_chase", "unsheathe", ACTION, (btcondition)&BTCommon::checkSheathed, (btaction)&BTCommon::unsheathe);
					addChild("unsheathe_and_chase", "chasePlayer", ACTION, NULL, (btaction)&BTCommon::chasePlayer);							
			addChild("choose_strategy", "unsheathe_and_chase2", PRIORITY, NULL, NULL);
				addChild("unsheathe_and_chase2", "unsheathe2", ACTION, (btcondition)&BTCommon::checkSheathed, (btaction)&BTCommon::unsheathe);
				addChild("unsheathe_and_chase2", "chasePlayer2", ACTION, NULL, (btaction)&BTCommon::chasePlayer);
		addChild("enemy_gatekeeper", "track_target", PRIORITY, (btcondition)&BTCommon::checkTrackingTarget, NULL);
			addChild("track_target", "trackNextPoint", ACTION, (btcondition)&BTCommon::checkHasTrackPoints, (btaction)&BTCommon::trackNextPoint);
			addChild("track_target", "stopTracking", ACTION, NULL, (btaction)&BTCommon::stopTracking);

		//addChild("enemy_gatekeeper", "look_and_go_with_chaser", SEQUENCE, (btcondition)&BTCommon::checkAllyChasingPlayer, NULL);
		//	addChild("look_and_go_with_chaser", "lookAtChaser", ACTION, NULL, (btaction)&BTCommon::lookAtChaser);
		//	addChild("look_and_go_with_chaser", "unsheatheIfHasTo3", ACTION, NULL, (btaction)&BTCommon::unsheatheIfHasTo);
		//	addChild("look_and_go_with_chaser", "goWhereAllyGoes", ACTION, NULL, (btaction)&BTCommon::goWhereAllyGoes);

		addChild("enemy_gatekeeper", "look_corpse", PRIORITY, (btcondition)&BTCommon::checkCorpseSeen, NULL);
			addChild("look_corpse", "go_and_look_corpse", PRIORITY, (btcondition)&BTCommon::checkIfFirstCorpse, NULL);
				addChild("go_and_look_corpse", "lookAtCorpse_a", ACTION, (btcondition)&BTCommon::checkNearCorpse, (btaction)&BTCommon::lookAtCorpse);
				addChild("go_and_look_corpse", "goToCorpse", ACTION, NULL, (btaction)&BTCommon::goToCorpse);
			addChild("look_corpse", "lookAtCorpse_b", ACTION, NULL, (btaction)&BTCommon::lookAtCorpse);
		addChild("enemy_gatekeeper", "remember_corpse", SEQUENCE, (btcondition)&BTCommon::checkSeeingCorpse, NULL);
			addChild("remember_corpse", "rememberCorpse", ACTION, NULL, (btaction)&BTCommon::rememberCorpse);
			addChild("remember_corpse", "unsheatheIfHasTo2", ACTION, NULL, (btaction)&BTCommon::unsheatheIfHasTo);
		addChild("enemy_gatekeeper", "look_and_decide", SEQUENCE, (btcondition)&BTCommon::checkWarningInBlackboard, NULL);
			addChild("look_and_decide", "lookToAlly", ACTION, NULL, (btaction)&BTCommon::lookToAlly);
			addChild("look_and_decide", "alerted_decide", PRIORITY, NULL, NULL);
				addChild("alerted_decide", "go_with_ally", SEQUENCE, (btcondition)&BTCommon::checkWantToGo, NULL);
					addChild("go_with_ally", "unsheatheIfHasTo", ACTION, NULL, (btaction)&BTCommon::unsheatheIfHasTo);
					addChild("go_with_ally", "goWithAlly", ACTION, NULL, (btaction)&BTCommon::goWithAlly);
				addChild("alerted_decide", "ignoreWarning", ACTION, NULL, (btaction)&BTCommon::ignoreWarning);
		addChild("enemy_gatekeeper", "search_and_go", SEQUENCE, (btcondition)&BTCommon::checkLoudNoiseHeard, NULL);
			addChild("search_and_go", "lookAtSoundSource", ACTION, NULL, (btaction)&BTCommon::lookAtSoundSource);
			addChild("search_and_go", "goToSoundSource", ACTION, NULL, (btaction)&BTCommon::goToSoundSource);
		addChild("enemy_gatekeeper", "lookForPlayer", ACTION, (btcondition)&BTCommon::checkPlayerPartiallyViewed, (btaction)&BTCommon::lookForPlayer);
		addChild("enemy_gatekeeper", "search_player", PRIORITY, (btcondition)&BTCommon::checkSearching, NULL);
			addChild("search_player", "stop_searching", SEQUENCE, (btcondition)&BTCommon::checkStoppedSearching, NULL);
				addChild("stop_searching", "stopSearching", ACTION, NULL, (btaction)&BTCommon::stopSearching);
				addChild("stop_searching", "sheathe", ACTION, NULL, (btaction)&BTCommon::sheathe);
			addChild("search_player", "lookAt", ACTION, (btcondition)&BTCommon::checkHasLookAt, (btaction)&BTCommon::lookAt);
			addChild("search_player", "chooseLookAt", ACTION, NULL, (btaction)&BTCommon::chooseLookAt);

		addChild("enemy_gatekeeper", "gatekeeping", PRIORITY, NULL, NULL);
			addChild("gatekeeping", "generatePathToGK", ACTION, (btcondition)&BTGatekeeper::checkIfNotGatekeeping, (btaction)&BTGatekeeper::generatePathToGK);
			addChild("gatekeeping", "goToGKPlace", ACTION, (btcondition)&BTGatekeeper::checkgoingToGKPlace, (btaction)&BTGatekeeper::goToGKPlace);
			addChild("gatekeeping", "lookAtGK", ACTION, (btcondition)&BTGatekeeper::checkLookingAtGK, (btaction)&BTGatekeeper::lookAtGK);
			addChild("gatekeeping", "talk", PRIORITY, (btcondition)&BTGatekeeper::checkTalk, NULL);
				addChild("talk", "talkLeft", ACTION, (btcondition)&BTGatekeeper::checkTalkLeft, (btaction)&BTGatekeeper::talkLeft);
				addChild("talk", "talkRight", ACTION, NULL, (btaction)&BTGatekeeper::talkRight);
			addChild("gatekeeping", "idle2", ACTION, (btcondition)&BTGatekeeper::checkIdleTimeOut, (btaction)&BTGatekeeper::idle2);
			addChild("gatekeeping", "idle", ACTION, NULL, (btaction)&BTGatekeeper::idle);
}

void BTGatekeeper::render()
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

	/*printf2D( g_App.GetWidth()*3/5, 260, text_color, "t point: %d", _trackingPoint);
	printf2D( g_App.GetWidth()*3/5, 280, text_color, "t player: %d", _trackingPlayer);
	printf2D( g_App.GetWidth()*3/5, 300, text_color, "t corpse: %d", _trackingCorpse);
	printf2D( g_App.GetWidth()*3/5, 320, text_color, "searching: %d", _searching);
	printf2D( g_App.GetWidth()*3/5, 340, text_color, "s. searching: %d", _stoppedSearching);*/

	//dbg lookAts
	//btVector3 lookAt;
	//std::vector<btVector3>::iterator iter;
	//for(iter=_eD->_lookAts.begin(); iter!=_eD->_lookAts.end(); iter++)
	//{
	//	drawLine_bt(_eD->_transformC->getPosition(), _eD->_transformC->getPosition() + *iter, D3DCOLOR_ARGB(255, 255,255,255));
	//}

	//printf2D( g_App.GetWidth()*3/5, 240, text_color, "lookAt time: %.2f", _lookAtClock.getCount());
	//printf2D( g_App.GetWidth()*3/5, 260, text_color, "forget time time: %.2f", _forgetClock.getCount());

	//dbg("elapsed_time: %f\n", World::instance()->_time_elapsed*_delta);

	_eD->renderPath(D3DCOLOR_ARGB( 255, 0, 0, 255 ), 0.5f);
	if(_eD->_path.size()) drawLine_bt(_eD->_transformC->getPosition(), _eD->_path.at(0), D3DCOLOR_ARGB( 255, 255, 255, 255 ));
}

void BTGatekeeper::setGKPlace(const btVector3& place)
{
	_GKPlace = place;
}

void BTGatekeeper::setGKLookAt(const btVector3& lookAt)
{
	_GKLookAt = lookAt-_GKPlace;
	_GKLookAt.setY(0);
	_GKLookAt.normalize();
}

//toTalk debe ser un gatekeeper
//setTheOther: nos hace falta una variable para determinar cuando queremos inicializar los parametros de talk del otro tio, sino seria un bucle infinito de llamadas entre los interlocutores
void BTGatekeeper::setAllyToTalk(Entity* toTalk, bool setTheOther, bool stopTalkingWithTheOther)
{
	//Esto sirve para cuando muere, para que avise al otro que ya no debe hablar con este
	if(stopTalkingWithTheOther && _allyToTalk) 
	{
		BTGatekeeper* _theOthersBT = static_cast<BTGatekeeper*>(EntityManager::get().getComponent<BTComponent>(_allyToTalk)->getBT());
		_theOthersBT->setAllyToTalk(NULL, false); //este es el tio que esta vivo
		//Si nos han matado mientras el otro hablaba...
		if(*_theOthersBT->getPreviousAction() == "talkLeft" || *_theOthersBT->getPreviousAction() == "talkRight")
		{
			_theOthersBT->_chasePlayerNow = true;
		}
	}

	_allyToTalk = toTalk;
	_thereWasAllyToTalk = true;

	//El 50 porciento de las veces entrara un NULL pq el otro tio aun no estara declarado. En esos casos salimos.
	if(!toTalk) return;

	//Como el punto de gatekeep nuestro ni del aliado no varia, calculamos a que lado habra que hablar
	BTGatekeeper* _theOthersBT = static_cast<BTGatekeeper*>(EntityManager::get().getComponent<BTComponent>(toTalk)->getBT());
	if(setTheOther) _theOthersBT->setAllyToTalk(_entity, false);
	const btVector3& theOthersPlace = _theOthersBT->getGKPlace();
	btVector3 toTheOther = theOthersPlace - _GKPlace;  toTheOther.normalize();
	btVector3 GKleft = btVector3(0,1,0).cross(_GKLookAt);
	if(GKleft.dot(toTheOther) > 0) _talkingToLeft = true;
}


//Conditions--------------------------------------------------------
bool BTGatekeeper::checkIfNotGatekeeping()
{
	if(*_previousAction == "generatePathToGK" ||
		*_previousAction == "goToGKPlace" ||
		*_previousAction == "lookAtGK" ||
		*_previousAction == "idle" ||
		*_previousAction == "idle2" ||
		*_previousAction == "talkLeft" ||
		*_previousAction == "talkRight")
	{
		return false;
	}
	else return true;
}

bool BTGatekeeper::checkgoingToGKPlace()
{
	return _goingToGKPlace;
}

bool BTGatekeeper::checkLookingAtGK()
{
	btVector3 frontXinv;
	_eD->_transformC->getFrontXinv(frontXinv);
	return frontXinv.dot(_GKLookAt) < 0.95f;
}

bool BTGatekeeper::checkTalk()
{
	if(!_allyToTalk || _eD->_attentionDegree != attentionDegrees::NORMAL) //no hay aliado para hablar o no estamos en normal
	{
		return false;
	}

	//Si hay aliado para hablar y esta con atencion normal, comprobamos si esta en idle o hablando
	if(EntityManager::get().getComponent<EnemyDataComponent>(_allyToTalk)->_attentionDegree != attentionDegrees::NORMAL) return false;

	const std::string* allyAction = EntityManager::get().getComponent<BTComponent>(_allyToTalk)->getBT()->getCurrentAction();
	if( *_previousAction == "idle" ||
		*_previousAction == "idle2" ||
		*_previousAction == "talkLeft" ||
		*_previousAction == "talkRight")
	{
		return true;
	}
	else return false;
}

bool BTGatekeeper::checkTalkLeft()
{
	return _talkingToLeft;
}

bool BTGatekeeper::checkIdleTimeOut()
{
	return _secondIdleEnabled;
}

//Actions--------------------------------------------------------------------------------------------------------------------
int BTGatekeeper::generatePathToGK()
{
	assert(_GKPlace != btVector3(0,0,0) || fatal("El gatekeeper: %s no tiene GKPlace!!\n", _entity->name.c_str()));
	assert(_GKLookAt != btVector3(0,0,0) || fatal("El gatekeeper: %s no tiene GKLookAt!!\n", _entity->name.c_str()));

	DijkstraGraph::get().computePath(_eD->_transformC->getPosition(), _GKPlace, _pathToGK);
	_goingToGKPlace = true;
	return LEAVE;
}

int BTGatekeeper::goToGKPlace()
{
	_eD->goToNextTrackPoint(_eD->_delta, _pathToGK);
	if(!_pathToGK.size()) _goingToGKPlace = false;
	return LEAVE;
}

int BTGatekeeper::lookAtGK()
{
	_eD->_transformC->approximateFront_v(_GKLookAt, _eD->_rotateVel*_eD->_delta);
	return LEAVE;
}

int BTGatekeeper::talkLeft()
{
	//no hay que hacer nada
	//int kk = 23; //Encuentros en la 3a fase: debugando si meto un breakpoint aqui dejando solo el return LEAVE me para en btDeadEnemy::idle pq el contenido es el mismo...
	return LEAVE;
}

int BTGatekeeper::talkRight()
{
	//no hay que hacer nada
	//int kk = 28;
	return LEAVE;
}

int BTGatekeeper::idle()
{
	//Si antes ya estabamos en idle hacemos avanzar un contador
	if(*_previousAction == "idle")
	{
		if(_eD->_attentionDegree == attentionDegrees::CAUTION || _eD->_attentionDegree == attentionDegrees::PERMANENT_CAUTION)
		{
			_secondIdleEnabled = false;
		}
		else if(_secondIdle.count(_eD->_delta))
		{
			_secondIdleEnabled = true;
		}
	}
	//Si no lo estabamos seteamos el contador
	else _secondIdle.setTarget(12.0f+rand()%6);

	//Me jode un poco tener que atualizar esto cada update cuando esta quieto, pero mira
	_eD->_eyePos = _eD->_transformC->getPosition()+eyeOffset_v;

	return LEAVE;
}

int BTGatekeeper::idle2()
{
	//Si antes ya estabamos en idle2 hacemos avanzar un contador
	if(*_previousAction == "idle2")
	{
		if(_secondIdle.count(_eD->_delta)) _secondIdleEnabled = false;
	}
	//Si no lo estabamos seteamos el contador
	else _secondIdle.setTarget(7.0f+rand()%3); 

	//Me jode un poco tener que atualizar esto cada update cuando esta quieto, pero mira
	_eD->_eyePos = _eD->_transformC->getPosition()+eyeOffset_v;

	return LEAVE;
}
