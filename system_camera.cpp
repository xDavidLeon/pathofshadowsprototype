#include "system_camera.h"
#include "component_animation.h"
#include "camera_controller_dbg.h"
#include <cassert>
#include "d3ddefs.h"
#include "entity_manager.h"
#include "system_renderer.h"
#include <string>

CameraSystem::CameraSystem(void)
{
	_currentCamera = _playerCamera = _dbgCamera = _cinCamera = NULL;
	_t = 0.0f;
	_activeCine = -1;
	_pauseCine = _pauseOnEnd = _barsLocked = false;

	// crear una camara de auxiliar para rellenar y usar en caso de que las demas predisenyadas tengan colision
	_valid_camera = new CameraInfo();
	_valid_camera->cameraEntity = EntityManager::get().createEntity();
	
	EntityManager::get().addComponent(new CameraComponent(_valid_camera->cameraEntity, CAM_TYPE::CAM_CIN	,btVector3( 0,0,0 ), btVector3(0,0,0)
														, 0, (float) g_App.GetWidth() / (float) g_App.GetHeight(), 0, 0), _valid_camera->cameraEntity);
	
	readKillCamerasFile();

	_blackTextureName = "hardcoded/black_pixel";
	_blackTexture = TTextureManager::get().getTexture(_blackTextureName);
	_loadingT = TTextureManager::get().getTexture("hardcoded/loading_screen");

	//Franjas negras
	_barsActivated = false;
	D3DSURFACE_DESC desc;  _blackTexture->GetLevelDesc(0, &desc);
	unsigned int dX, dY;
	dX = desc.Width/2;
	dY = desc.Height/2;

	float halfResX = (float)g_App.GetWidth()*0.5f;
	float halfResY = (float)g_App.GetHeight()*0.5f;
	float barsHeight = halfResY*0.25f;
	D3DXMATRIX T, S;
	D3DXMatrixTranslation(&T, -((float)dX), -((float)dY*2.0f), 0); //franja de arriba: pivote en el centro del borde inferior de la textura
	float scaleX = halfResX/(float)dX;
	float scaleY = barsHeight/(float)dY;
	D3DXMatrixScaling(&S, scaleX, scaleY, 0.0f); //Escalado (z es irrelevante)
	_upperBarT = T*S;
	D3DXMatrixTranslation(&T, -((float)dX), 0, 0); //franja de abajo: pivote en el centro del borde superior de la textura
	_lowerBarT = T*S;
	D3DXMatrixTranslation(&T, halfResX, 0, 0); //desplazamiento hacia el centro horizontal de la pantalla
	_upperBarT = _upperBarT*T;
	_lowerBarT = _lowerBarT*T;
	D3DXMatrixTranslation(&T, 0, halfResY*2.0f, 0); //la de arriba ya esta arriba; falta que la de abajo este abajo
	_lowerBarT = _lowerBarT*T;
	_upperBarY_d = _upperBarT._42;
	_lowerBarY_d = _lowerBarT._42;
	_upperBarY_a = _upperBarY_d + barsHeight; //Posicion vertical a la que deberia estar la franja de arriba cuando este desplegada
	_lowerBarY_a = _lowerBarY_d - barsHeight; //Posicion vertical a la que deberia estar la franja de abajo cuando este desplegada

	//Pantalla negra y loading
	_blackScreenActivated = false;
	_blackScreenTexture = _blackTexture;
	_blackScreenAlpha = 0.0f;
	D3DXMatrixTranslation(&_blackM, -((float)dX), -((float)dY), 0); //Pivote en centro de textura
	scaleY = halfResY/(float)dY;
	D3DXMatrixScaling(&S, scaleX, scaleY, 0.0f); //Escalado para que ocupe toda la pantalla (z es irrelevante)
	_blackM = _blackM*S;
	D3DXMatrixTranslation(&T, halfResX, halfResY, 0); //desplazamiento hacia el centro de la pantalla
	_blackM = _blackM*T;
	_blackScreenM = _blackM;

	_loadingT->GetLevelDesc(0, &desc);
	dX = desc.Width/2;
	dY = desc.Height/2;
	D3DXMatrixTranslation(&_loadingM, -((float)dX), -((float)dY), 0); //Pivote en centro de textura
	scaleX = halfResX/(float)dX;
	scaleY = halfResY/(float)dY;
	D3DXMatrixScaling(&S, scaleX, scaleY, 0.0f); //Escalado para que ocupe toda la pantalla (z es irrelevante)
	_loadingM = _loadingM*S;
	D3DXMatrixTranslation(&T, halfResX, halfResY, 0); //desplazamiento hacia el centro de la pantalla
	_loadingM = _loadingM*T;

	//logo del principio y press ENTER
	_logoAlpha = 0;
	_logoT = TTextureManager::get().getTexture("hardcoded/logo");

	_logoT->GetLevelDesc(0, &desc);
	dX = desc.Width/2;
	dY = desc.Height/2;
	D3DXMatrixTranslation(&_logoM, -((float)dX), -((float)dY), 0);
	float Ypercent = 0.6f;
	float desiredRadius = Ypercent*halfResY; //radio que queremos que tenga la textura
	float scale = desiredRadius/dY;
	D3DXMatrixScaling(&S, scale, scale, 0.0f);
	_logoM = _logoM*S;
	D3DXMatrixTranslation(&T, halfResX, halfResY-halfResY*0.3f, 0);
	_logoM = _logoM*T;

		//pressEnter
	_pressEnterT = TTextureManager::get().getTexture("hardcoded/press_enter");

	_pressEnterT->GetLevelDesc(0, &desc);
	dX = desc.Width/2;
	dY = desc.Height/2;
	D3DXMatrixTranslation(&_pressEnterM, -((float)dX), -((float)dY), 0);
	float Xpercent = 0.3f;
	desiredRadius = Xpercent*halfResX; //radio que queremos que tenga la textura
	scale = desiredRadius/dX;
	D3DXMatrixScaling(&S, scale, scale, 0.0f);
	_pressEnterM = _pressEnterM*S;
	D3DXMatrixTranslation(&T, halfResX, halfResY*2.0f-halfResY*0.45f, 0);
	_pressEnterM = _pressEnterM*T;

	//creditos
	float escaledoUltraPerroEnY = 1.0f;
	_creditsActivated = false;
	_creditsT = TTextureManager::get().getTexture("hardcoded/credits");
	_creditsT->GetLevelDesc(0, &desc);
	dX = desc.Width/2;
	D3DXMatrixTranslation(&_creditsM, -((float)dX), 0, 0); //"centro" en X de textura
	D3DXMatrixTranslation(&T, halfResX, halfResY*2.0f, 0);
	Xpercent = 0.5f; //Ancho de la pantalla
	desiredRadius = Xpercent*halfResX; //X que queremos que tenga la textura
	scale = desiredRadius/dX;
	//D3DXMatrixScaling(&S, scale, scale, 0.0f);
	D3DXMatrixScaling(&S, scale, scale*escaledoUltraPerroEnY, 0.0f);
	_creditsM = _creditsM*S*T;
	_creditsInitM = _creditsM;

	_creditsY = 0.0;
	_creditsMaxY = desc.Height*escaledoUltraPerroEnY*scale+halfResY*2.0f;
	_creditsVel = halfResY * 0.3f;

	_languageFolder = "hardcoded/ENG/";
	if (g_App.GetLanguage() == 1) _languageFolder = "hardcoded/ENG/";
	else if (g_App.GetLanguage() == 2) _languageFolder = "hardcoded/SPA/";

	//subtitulos
	initSubs();

	_subtTextures.at("st1")->GetLevelDesc(0, &desc);
	dX = desc.Width/2;
	dY = desc.Height/2;
	D3DXMatrixTranslation(&_subsM, -(float)dX, -(float)dY, 0); //"centro inferior" de textura
	D3DXMatrixTranslation(&T, halfResX, halfResY*2.0f-(float)dY*scale, 0);
	D3DXMatrixScaling(&S, scale, scale, 0.0f); //El mismo scale que los creditos
	_subsM = _subsM*S*T;	
}

CameraSystem::~CameraSystem(void)
{
}

void CameraSystem::initSubs()
{
	_subtTextures.insert(std::pair<string, TTexture>("st1", TTextureManager::get().getTexture(_languageFolder+"st1")));
	_subtTextures.insert(std::pair<string, TTexture>("st2", TTextureManager::get().getTexture(_languageFolder+"st2")));
	_subtTextures.insert(std::pair<string, TTexture>("st3", TTextureManager::get().getTexture(_languageFolder+"st3")));
	_subtTextures.insert(std::pair<string, TTexture>("st4", TTextureManager::get().getTexture(_languageFolder+"st4")));
	_subtTextures.insert(std::pair<string, TTexture>("st5", TTextureManager::get().getTexture(_languageFolder+"st5")));
	_subtTextures.insert(std::pair<string, TTexture>("st6", TTextureManager::get().getTexture(_languageFolder+"st6")));
	_subtTextures.insert(std::pair<string, TTexture>("st7", TTextureManager::get().getTexture(_languageFolder+"st7")));
	_subtTextures.insert(std::pair<string, TTexture>("st8", TTextureManager::get().getTexture(_languageFolder+"st8")));
	_subtTextures.insert(std::pair<string, TTexture>("st9", TTextureManager::get().getTexture(_languageFolder+"st9")));
	_subtTextures.insert(std::pair<string, TTexture>("st10", TTextureManager::get().getTexture(_languageFolder+"st10")));
	_subtTextures.insert(std::pair<string, TTexture>("st11", TTextureManager::get().getTexture(_languageFolder+"st11")));
	_subtTextures.insert(std::pair<string, TTexture>("st12", TTextureManager::get().getTexture(_languageFolder+"st12")));
	_subtTextures.insert(std::pair<string, TTexture>("st13", TTextureManager::get().getTexture(_languageFolder+"st13")));
	_subtTextures.insert(std::pair<string, TTexture>("st14", TTextureManager::get().getTexture(_languageFolder+"st14")));
	_subtTextures.insert(std::pair<string, TTexture>("st15", TTextureManager::get().getTexture(_languageFolder+"st15")));

	_currentSubt = _dyingSubt = "";
	_currentSubtA = _dyingSubtA = 0.0f;
}

TCamera& CameraSystem::getCamera(Entity* entity){  return EntityManager::get().getComponent<CameraComponent>(entity)->getCamera(); }

void CameraSystem::addCinCameraPos(int cam_id, const btVector3& pos)
{
	if(_cinePositions.find(cam_id) == _cinePositions.end()) 
		_cinePositions.insert(std::pair<int, std::deque<btVector3>>(cam_id, std::deque<btVector3>()));

	_cinePositions.at(cam_id).push_back(pos);
}

void CameraSystem::addCinCameraLookAt(int cam_id, const btVector3& lookAt)
{
	if(_cineLookAts.find(cam_id) == _cineLookAts.end()) 
		_cineLookAts.insert(std::pair<int, std::deque<btVector3>>(cam_id, std::deque<btVector3>()));

	_cineLookAts.at(cam_id).push_back(lookAt);
}

void CameraSystem::addCinCameraTiming(int cam_id, const btVector3& timing, bool interpFromPlayer, bool interpToPlayer)
{
	if(_cineTimings.find(cam_id) == _cineTimings.end()) 
		_cineTimings.insert(std::pair<int, btVector3>(cam_id, btVector3()));

	_cineTimings.at(cam_id) = timing;

	if(_cineStartEndInterp.find(cam_id) == _cineStartEndInterp.end()) 
		_cineStartEndInterp.insert(std::pair<int, std::pair<bool, bool>>(cam_id, std::pair<bool, bool>(interpFromPlayer, interpToPlayer)));
}

void CameraSystem::update(float delta)
{
	if (!_currentCamera) return;
	EntityManager::get().getComponent<CameraComponent>(_currentCamera)->update(delta);
}

void CameraSystem::render(void)
{
	if (!_currentCamera) return;
	if(_activeCine == -1)
	{
		if(_cameraQueue.size())
		{
			activateCineCamera(_cameraQueue.at(0));
			_cameraQueue.pop_front();
		}
		else
		{
			//al acabar cinematicas
			if(_barsActivated && !_barsLocked) toggleBars();
		}
	}
	else
	{
		if(!_pauseCine)
		{
			if(!_barsActivated && !_barsLocked) toggleBars();
			executeCineCamera(_activeCine);
		}
	}
	EntityManager::get().getComponent<CameraComponent>(_currentCamera)->render();
}

void CameraSystem::renderBars()
{
	if(_barsActivated) //Poner en posicion "activada"
	{
		float yNow = _upperBarT._42;
		if(yNow != _upperBarY_a) yNow = 0.6f*yNow + 0.4f*_upperBarY_a;
		_upperBarT._42 = yNow;

		yNow = _lowerBarT._42;
		if(yNow != _lowerBarY_a) yNow = 0.6f*yNow + 0.4f*_lowerBarY_a;
		_lowerBarT._42 = yNow;
	}
	else //Poner en posicion "desactivada"
	{
		if(_upperBarT._42 == _upperBarY_d && _lowerBarT._42 == _lowerBarY_d) return;

		float yNow = _upperBarT._42;
		if(yNow != _upperBarY_d) yNow = 0.6f*yNow + 0.4f*_upperBarY_d;
		_upperBarT._42 = yNow;

		yNow = _lowerBarT._42;
		if(yNow != ((float)g_App.GetHeight())) yNow = 0.6f*yNow + 0.4f*((float)g_App.GetHeight());
		_lowerBarT._42 = yNow;
	}
	RendererSystem::get().drawSprite( _blackTexture, _upperBarT, D3DCOLOR_ARGB(255, 255,255,255));
	RendererSystem::get().drawSprite( _blackTexture, _lowerBarT, D3DCOLOR_ARGB(255, 255,255,255));
}

void CameraSystem::renderBlackScreen()
{
	if(_blackScreenActivated) //Oscurecer
	{
		_blackScreenAlpha += 100.0f*World::instance()->getElapsedTimeRInSeconds();
		if(_blackScreenAlpha > 255.0f) _blackScreenAlpha = 255.0f;
	}
	else //aclarar
	{
		_blackScreenAlpha -= 200.0f*World::instance()->getElapsedTimeRInSeconds();
		if(_blackScreenAlpha < 0.0f){ _blackScreenAlpha = 0.0f; return;}
	}
	RendererSystem::get().drawSprite( _blackScreenTexture, _blackScreenM, D3DCOLOR_ARGB((int)_blackScreenAlpha, 255,255,255));
}

void CameraSystem::setBlackToLoadingScreen(bool loading)
{
	if(loading)
	{
		_blackScreenTexture = _loadingT;
		_blackScreenM = _loadingM;
	}
	else 
	{
		_blackScreenTexture = _blackTexture;
		_blackScreenM = _blackM;
	}
}

void CameraSystem::renderLogo()
{
	if(_logoActivated)
	{
		_logoAlpha += 10;
		if(_logoAlpha > 255.0f) _logoAlpha = 255;
	}
	else //aclarar
	{
		_logoAlpha -= 10;
		if(_logoAlpha < 0.0f){ _logoAlpha = 0; return;}
	}
	RendererSystem::get().drawSprite( _logoT, _logoM, D3DCOLOR_ARGB(_logoAlpha, 255,255,255));
	float parpadeo = (sinf(timeGetTime()*0.002f)+1.0f)*0.5f;
	//dbg("alpha: %f", parpadeo);
	int parpadeo_i = parpadeo*_logoAlpha;
	//dbg(" (%d)\n", parpadeo_i);
	RendererSystem::get().drawSprite( _pressEnterT, _pressEnterM, D3DCOLOR_ARGB(parpadeo_i, 255,255,255));
	
}

void CameraSystem::activateCredits()
{
	_creditsActivated = true;
	_creditsY = 0.0f;
	_creditsM = _creditsInitM;
}

void CameraSystem::renderCredits()
{
	if(!_creditsActivated) return;

	if(_creditsY >= _creditsMaxY){ _creditsActivated = false; return;}
	_creditsM._42 -= _creditsVel*World::instance()->getElapsedTimeRInSeconds();
	_creditsY += _creditsVel*World::instance()->getElapsedTimeRInSeconds();

	RendererSystem::get().drawSprite( _creditsT, _creditsM, D3DCOLOR_ARGB(255, 255,255,255));

	//dbg("cY: %f (%f)\n", _creditsY, _creditsM._42);
}

void CameraSystem::activateSubt(const string& subtName, float subTime)
{
	if (g_App.isUsingSubtitles() == false) return;
	if(_subtTextures.find(subtName) == _subtTextures.end()) return;

	deactivateCurrentSubt();
	_currentSubt = subtName;
	_currentSubtA = 0.0f;

	_subTimer.setTarget(subTime); //para el counterclock -1.0f es "no tener target"
}

void CameraSystem::deactivateCurrentSubt()
{
	if (g_App.isUsingSubtitles() == false) return;
	_dyingSubt = _currentSubt;
	_dyingSubtA = _currentSubtA;
	_currentSubt = "";
	_currentSubtA = 0.0f;
}

void CameraSystem::renderSubs()
{
	if (g_App.isUsingSubtitles() == false) return;
	if(_dyingSubt != "")
	{
		_dyingSubtA -= 400.0f*World::instance()->getElapsedTimeRInSeconds();
		
		if(_dyingSubtA <= 0.0f)
		{
			_dyingSubt = "";
			_dyingSubtA = 0.0f;
			return;
		}

		RendererSystem::get().drawSprite( _subtTextures.at(_dyingSubt), _subsM, D3DCOLOR_ARGB((int)_dyingSubtA, 255,255,255));
	}
	else if(_currentSubt != "") //solo pintamos el subtitulo actual si el anterior ya no se ve
	{
		_currentSubtA += 400.0f*World::instance()->getElapsedTimeRInSeconds();
		if(_currentSubtA >= 255.0f) _currentSubtA = 255.0f; 

		RendererSystem::get().drawSprite( _subtTextures.at(_currentSubt), _subsM, D3DCOLOR_ARGB((int)_currentSubtA, 255,255,255));

		if(_subTimer.hasTarget())
		{
			if(_subTimer.count(World::instance()->getElapsedTimeRInSeconds())) deactivateCurrentSubt();
		}
	}
}

void CameraSystem::setBlackScreen(bool black)
{
	if(black)
	{
		_blackScreenAlpha = 255.0f;
		_blackScreenActivated = true;
	}
	else
	{
		_blackScreenAlpha = 0.0f;
		_blackScreenActivated = false;
	}
}

void CameraSystem::renderDbgCameraInfo()
{
	if(_currentCamera != _dbgCamera) return;

	const btVector3 camPos = getDbgCamera().getPosition();
	DWORD color = D3DCOLOR_ARGB( 255, 200, 200, 150 );

	printf2D( g_App.GetWidth()*0.77f, 10, color, "CAM DGB");
	printf2D( g_App.GetWidth()*0.77f, 30, color, "->pos: %.2f, %.2f, %.2f", camPos.getX(), camPos.getY(), camPos.getZ());
	printf2D( g_App.GetWidth()*0.77f, 50, color, "->vel: %.2f", ((CameraControllerDbg*)(getDbgCamera().controller))->_speed);
}

void CameraSystem::lookAt(Entity* cam_entity, const btVector3& pos, const btVector3& lookAt)
{
	EntityManager::get().getComponent<CameraComponent>(cam_entity)->getCamera().lookAt( pos, lookAt, btVector3( 0, 1, 0 ) );
}

//devuelve true si se pasa a camDbg, false si se pasa a playerCam
bool CameraSystem::toggleDbgCamera(bool setAtCamPlayer)
{
	if(_currentCamera == _playerCamera) 
	{
		if(setAtCamPlayer) ((CameraControllerDbg*)EntityManager::get().getComponent<CameraComponent>(_dbgCamera)->getCamera().controller)->setViewFrom(&getCurrentCamera());
		_currentCamera = _dbgCamera;
		return true;
	}
	else
	{
		_currentCamera = _playerCamera;
		return false;
	}
}

void CameraSystem::toggleKillCamera()
{
	if(_currentCamera == _playerCamera)
	{
		_currentCamera = getAndPlaceKillCamera(getBestCameraSequence(_killCameras));
	}
	else _currentCamera = _playerCamera;
}

void CameraSystem::changeKillCamera()
{
	_best_camera_sequence->advanceInSequence();
	_currentCamera = getAndPlaceKillCamera(_best_camera_sequence);
}

void CameraSystem::toggleDeathCamera()
{
	if(_currentCamera == _playerCamera)
	{
		_currentCamera = getAndPlaceDeathCamera(getBestCameraSequence(_deathCameras));
	}
	else _currentCamera = _playerCamera;
}

CameraSystem::CameraSequence* CameraSystem::getBestCameraSequence(std::map<std::string, KillCameraInfo*> cameras)
{
	KillCameraInfo* kill_cam_info = cameras[EntityManager::get().getComponent<AnimationComponent>(World::instance()->getPlayer())->getSilentKillAnimationName()];

	CameraInfo* camera_info;
	CameraSequence* best_camera_sequence = kill_cam_info->cameraSequences[0];
	CameraSequence* camera_sequence;
	btTransform* target_transform = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->transform;

	std::vector<CameraSequence*> valid_sequences;

	float max_cam_target_distance2 = 2.0f;
	bool target_collision = false;
	bool collision = false;

	float distance;
	float aculated_distance;
	float min_distance = FLT_MAX;

	for( int i = 0; i < kill_cam_info->cameraSequences.size(); i++ )
	{
		aculated_distance = 0;
		camera_sequence = kill_cam_info->cameraSequences[i];
		target_collision = false;

		for( int i = 0; i < camera_sequence->cameraInfos.size(); i++ )
		{
			camera_info = camera_sequence->cameraInfos[i];
	
			btVector3 lookAt = (*target_transform) * camera_info->lookAt;
			//posicion a la que deberia situarse la camra en condiciones normales
			btVector3 desiredPosition = (*target_transform) * camera_info->position;
			btVector3 position, direction;
	
			//Creamos callback para obtener resultado de la colision
			btCollisionWorld::ClosestRayResultCallback rayCallback(lookAt,desiredPosition);
			rayCallback.m_collisionFilterMask = PhysicsSystem::get().colMaskNavigation; //Para no colisionar con la misma capsula del player
			PhysicsSystem::get().getCollisionWorld()->rayTest(lookAt,desiredPosition, rayCallback); //(Test de colision)
	
			//Si colisiona modificamos la desired dist de la camara
			if (rayCallback.hasHit())
			{
				collision = true;
				//dbg("hit");
				position = rayCallback.m_hitPointWorld;
				direction = lookAt - position;
				position += direction * 0.3f; 
	
				distance = desiredPosition.distance2(position);

				aculated_distance += distance;

				if( target_transform->getOrigin().distance2(position) < max_cam_target_distance2 )
					target_collision = true;
			}
		}

		if( !collision )
			valid_sequences.push_back(camera_sequence);

		else if( aculated_distance < min_distance && !target_collision)
		{
			min_distance = aculated_distance;
			best_camera_sequence = camera_sequence;
		}
	}

	if( valid_sequences.size() > 0 )
	{
		_best_camera_sequence = valid_sequences[getLessPlayedSequenceIndex(valid_sequences)];
	}
	else
		_best_camera_sequence = best_camera_sequence;


	return _best_camera_sequence;
}

size_t CameraSystem::getLessPlayedSequenceIndex(std::vector<CameraSequence*> camera_sequences)
{
	size_t index = 0;
	unsigned min_times_played = UINT_MAX;

	for (size_t i = 0; i<camera_sequences.size(); ++i)
	{
		if(camera_sequences[i]->num_times_played < min_times_played)
		{	
			index = i;
			min_times_played = camera_sequences[i]->num_times_played;
		}
	}
	
	camera_sequences[index]->num_times_played++;
	return index;
}

 CameraSystem::CameraInfo* CameraSystem::getValidCameraInfo(btTransform* target_transform, CameraSequence* camera_sequence)
{
	std::vector<CameraInfo*> valid_cameras;
	CameraInfo* camera_info;

	camera_info = camera_sequence->cameraInfos[camera_sequence->index];

	btVector3 lookAt = (*target_transform) * camera_info->lookAt;
	//posicion a la que deberia situarse la camra en condiciones normales
	btVector3 desiredPosition = (*target_transform) * camera_info->position;
	btVector3 position, direction;

	//Creamos callback para obtener resultado de la colision
	btCollisionWorld::ClosestRayResultCallback rayCallback(lookAt,desiredPosition);
	rayCallback.m_collisionFilterMask = PhysicsSystem::get().colMaskNavigation; //Para no colisionar con la misma capsula del player
	PhysicsSystem::get().getCollisionWorld()->rayTest(lookAt,desiredPosition, rayCallback); //(Test de colision)

	//Si colisiona modificamos la desired dist de la camara
	if (rayCallback.hasHit())
	{
		//dbg("hit");
		position = rayCallback.m_hitPointWorld;
		direction = lookAt - position;
		position += direction * 0.3f; 

		_valid_camera->position = position;
		_valid_camera->lookAt = lookAt;
	}
	else 
	{
		_valid_camera->position = desiredPosition;
		_valid_camera->lookAt = lookAt;
	}
	
	return _valid_camera;
		
 }

Entity* CameraSystem::getAndPlaceKillCamera( CameraSequence* camera_sequence )
{
	btTransform* player_trans = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->transform;
	
	CameraInfo* camera_info = getValidCameraInfo(player_trans, camera_sequence);
	
	lookAt(_cinCamera, camera_info->position, camera_info->lookAt );
	getCamera( _cinCamera ).setZNear(0.8f);

	return _cinCamera;
}

Entity* CameraSystem::getAndPlaceDeathCamera(CameraSequence* camera_sequence)
{
	btTransform* player_trans = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->transform;
	
	CameraInfo* camera_info = getValidCameraInfo(player_trans, camera_sequence);

	lookAt(_cinCamera, camera_info->position, camera_info->lookAt );
	getCamera( _cinCamera ).setZNear(0.8f);

	return _cinCamera;
}

void CameraSystem::addCamToQueue(int cam_id)
{
	_cameraQueue.push_back(cam_id);
}

//Activa la camara cine con el travelling que corresponda a "cam_id"
void CameraSystem::activateCineCamera(int cam_id)
{
	assert(_cinePositions.find(cam_id) != _cinePositions.end());

	_activeCine = cam_id;

	//preparamos las constantes
	_T = _cineTimings.at(cam_id).getX();

	//Miramos si la cinematica de esa camara empieza desde la camara del player  (OJO: esto da por hecho que las cinematicas se reproducen 1 sola vez)
	if(_cineStartEndInterp.at(cam_id).first == true) //Si es que sí anyadimos a esa cinematica los puntos de inico y de control
	{
		btVector3 aux1, aux2;

		//Eliminamos punto de control inicial
		_cinePositions.at(cam_id).pop_front();
		_cineLookAts.at(cam_id).pop_front();

		//Anyadimos datos de camara actuales
		aux1 = getCamera(_playerCamera).getPosition();
		getCamera(_playerCamera).getFront(aux2);  //aux2.setX(aux2.getX());
		aux2 = aux1 + aux2;
		_cinePositions.at(cam_id).push_front(aux1);
		_cineLookAts.at(cam_id).push_front(aux2);

		//Anyadimos puntos de control iniciales
		aux1 = _cinePositions.at(cam_id).at(0) - _cinePositions.at(cam_id).at(1);
		aux2 = _cineLookAts.at(cam_id).at(0) - _cineLookAts.at(cam_id).at(1);
		_cinePositions.at(cam_id).push_front(_cinePositions.at(cam_id).at(0)+aux1);
		_cineLookAts.at(cam_id).push_front(_cineLookAts.at(cam_id).at(0)+aux2);
	}

	//Miramos si la cinematica de esa camara acaba donde la camara del player  (OJO: esto da por hecho que las cinematicas se reproducen 1 sola vez)
	if(_cineStartEndInterp.at(cam_id).second == true) //Si es que sí anyadimos a esa cinematica los puntos de fin y de control
	{
		btVector3 aux1, aux2;

		//Eliminamos punto de control final
		_cinePositions.at(cam_id).pop_back();
		_cineLookAts.at(cam_id).pop_back();

		//Anyadimos datos de camara actuales
		aux1 = getCamera(_playerCamera).getPosition();
		getCamera(_playerCamera).getFront(aux2);  //aux2.setX(aux2.getX());
		aux2 = aux1 + aux2;
		_cinePositions.at(cam_id).push_back(aux1);
		_cineLookAts.at(cam_id).push_back(aux2);

		//Anyadimos puntos de control finales
		int size1 = _cinePositions.at(cam_id).size();
		int size2 = _cineLookAts.at(cam_id).size();
		aux1 = _cinePositions.at(cam_id).at(size1-1) - _cinePositions.at(cam_id).at(size1-2);
		aux2 = _cineLookAts.at(cam_id).at(size2-1) - _cineLookAts.at(cam_id).at(size2-2);
		_cinePositions.at(cam_id).push_back(_cinePositions.at(cam_id).at(size1-1)+aux1);
		_cineLookAts.at(cam_id).push_back(_cineLookAts.at(cam_id).at(size2-1)+aux2);
	}

	int N = _cinePositions.at(cam_id).size()-3; // Catmull usa 4 puntos de evaluacion (idx actual y 3 siguientes)
	pos_factor = (float)N/_T;
	N = _cineLookAts.at(cam_id).size()-3;
	lookAt_factor = (float)N/_T;

	_waitClock.setTarget(_cineTimings.at(cam_id).getY()); //Tiempo de espera antes de arrancar
	_currentCamera = _cinCamera; //A partir de ahora el render se hara de la camara cine
	lookAt(_cinCamera, _cinePositions.at(cam_id).at(1), _cineLookAts.at(cam_id).at(1)); //Lookat con los parametros iniciales (la pos. 0 es para que funcione catmull; la camara no se sitia ahi nunca!)
}

bool CameraSystem::isCineActive()
{
	return _currentCamera == _cinCamera;
}

int CameraSystem::getActiveCamId() const
{
	if(_activeCine == -1)
	{
		if(_cameraQueue.size()) return _cameraQueue.at(0);
	}
	
	return _activeCine;
}

//Lleva a cabo en travelling definido por las posiciones y lookats con 'cam_id'
//Cuando acaba devuelve true, mientras false
bool CameraSystem::executeCineCamera(int cam_id)
{
	//float elapsed_time = 1.0f/60.0f;
	float elapsed_time = World::instance()->getElapsedTimeRInSeconds();
	if(elapsed_time > 2.0f) elapsed_time = 0.0;
	//dbg("et: %.4f\n", elapsed_time);

	//Extra: pausas
	if(_t == 0.0f) //Justo antes de empezar
	{
		if(_waitClock.count(elapsed_time)) _t += elapsed_time; //espera. Cuando acabe comienza a contar el tiempo de travelling
	}
	else if(_t+elapsed_time > _T) //Justo antes de acabar
	{
		if(!_waitClock.hasTarget()) _waitClock.setTarget(_cineTimings.at(cam_id).getZ());
		if(_waitClock.count(elapsed_time))
		{
			_t = 0.0f;
			if(_pauseOnEnd) //Si nos han pedido pausar al final del travelling...
			{
				_pauseOnEnd = false;
				_pauseCine = true;
			}
			else
			{
				if(!_cameraQueue.size()) _currentCamera = _playerCamera; //Devolvemos el render a la camara del player
			}
			
			_activeCine = -1;
			return true;
		}
	}
	else //el travelling
	{
		//tiempo para el cual evaluar
		_t += elapsed_time;

		//LOOKAT
			D3DXVECTOR3 new_lookAt;

			//Convertir tiempo a indice de curva
			float idx_f = _t*lookAt_factor;
			int idx=(int)(idx_f);

			//dbg("la_f: %.2f / %d\n", idx_f, _cineLookAts.at(cam_id).size()-4);

			if(idx<_cineLookAts.at(cam_id).size()-3)
			{
				//evaluar con Catmull Rom
				D3DXVec3CatmullRom(&new_lookAt
									,&(D3DXVECTOR3)(_cineLookAts.at(cam_id).at(idx))
									,&(D3DXVECTOR3)(_cineLookAts.at(cam_id).at(idx+1))
									,&(D3DXVECTOR3)(_cineLookAts.at(cam_id).at(idx+2))
									,&(D3DXVECTOR3)(_cineLookAts.at(cam_id).at(idx+3))
									,idx_f-idx);
			}
			else //Se evalua el catmull en la posicion final
			{
				int size = _cineLookAts.at(cam_id).size();

				//posicion final
				D3DXVec3CatmullRom(&new_lookAt
									,&(D3DXVECTOR3)(_cineLookAts.at(cam_id).at(size-4))
									,&(D3DXVECTOR3)(_cineLookAts.at(cam_id).at(size-3))
									,&(D3DXVECTOR3)(_cineLookAts.at(cam_id).at(size-2))
									,&(D3DXVECTOR3)(_cineLookAts.at(cam_id).at(size-1))
									,0.9999f);
			}

		//POSITION
			D3DXVECTOR3 new_pos;

			//Convertir tiempo a indice de curva
			idx_f = _t*pos_factor;
			idx=(int)(idx_f);

			if(idx<_cinePositions.at(cam_id).size()-3)
			{
				//evaluar con Catmull Rom
				D3DXVec3CatmullRom(&new_pos
									,&(D3DXVECTOR3)(_cinePositions.at(cam_id).at(idx))
									,&(D3DXVECTOR3)(_cinePositions.at(cam_id).at(idx+1))
									,&(D3DXVECTOR3)(_cinePositions.at(cam_id).at(idx+2))
									,&(D3DXVECTOR3)(_cinePositions.at(cam_id).at(idx+3))
									,idx_f-idx);
			}
			else  //Se evalua el catmull en la posicion final
			{
				int size = _cinePositions.at(cam_id).size();

				//posicion final
				D3DXVec3CatmullRom(&new_pos
									,&(D3DXVECTOR3)(_cinePositions.at(cam_id).at(size-4))
									,&(D3DXVECTOR3)(_cinePositions.at(cam_id).at(size-3))
									,&(D3DXVECTOR3)(_cinePositions.at(cam_id).at(size-2))
									,&(D3DXVECTOR3)(_cinePositions.at(cam_id).at(size-1))
									,0.9999f);
			}
		
		//Modificamos view de la camara
			btVector3 newPos_bt, newLookAt_bt;
			convertDXVector3(new_pos, newPos_bt);
			convertDXVector3(new_lookAt, newLookAt_bt);
			lookAt(_cinCamera, newPos_bt, newLookAt_bt);
	}

	return false;
}

//Libera los datos de travellings (cameras cine)
void CameraSystem::releaseCineCameras()
{
	_cinePositions.clear();
	_cineLookAts.clear();
	_cineTimings.clear();
	_cineStartEndInterp.clear();

	_waitClock.setTarget(-1);
	_t = 0.0f;

	_activeCine = -1;
}

void CameraSystem::readKillCamerasFile()
{
	std::string path = "data/kill_cameras.xml";

	bool is_ok = xmlParseFile(path);
	if( !is_ok ) fatalErrorWindow(std::string("No se ha encontrado el archivo " + std::string(path) + " o bien contiene errores").c_str());
}

void CameraSystem::onStartElement (const std::string &elem, MKeyValue &atts)
{
	if (elem == "kill_camera" || elem == "death_camera")
	{
		assert(atts.find("name") != atts.end());
		actual_kill_camera_info =  new KillCameraInfo();
		actual_kill_camera_info->name = atts["name"].c_str();
	}
	else if (elem == "type")
	{
		assert(atts.find("shadow") != atts.end());
		actual_kill_camera_info->type_shadow = atoi(atts["shadow"].c_str());

		if(atts.find("air") != atts.end())
			actual_kill_camera_info->type_air = atoi(atts["air"].c_str());
		else
			actual_kill_camera_info->type_air = 0;
		
		if(atts.find("blend") != atts.end())
			actual_kill_camera_info->type_blend = atoi(atts["blend"].c_str());
		else
			actual_kill_camera_info->type_blend = 0;

		if(atts.find("panic") != atts.end())
			actual_kill_camera_info->type_panic = atoi(atts["panic"].c_str());
		else
			actual_kill_camera_info->type_panic = 0;
	}
	else if (elem == "shadow_translation")
	{
		actual_kill_camera_info->shadow_translation = btVector3(0,0,0);

		assert(atts.find("z") != atts.end());
		actual_kill_camera_info->shadow_translation.setZ( atof(atts["z"].c_str()) );
		
		if(atts.find("x") != atts.end())
			actual_kill_camera_info->shadow_translation.setX( atof(atts["x"].c_str()) );
		else
			actual_kill_camera_info->shadow_translation.setX( 0 );

		if(atts.find("y") != atts.end())
			actual_kill_camera_info->shadow_translation.setY( atof(atts["y"].c_str()) );
		else
			actual_kill_camera_info->shadow_translation.setY( 0 );
	}
	else if (elem == "sequence")
	{
		actual_camera_sequence = new CameraSequence();
		actual_camera_sequence->index = 0;
		actual_camera_sequence->num_times_played = 0;
	}
	else if (elem == "data")
	{
		actual_camera_info = new CameraInfo();
		actual_camera_info->cameraEntity = EntityManager::get().createEntity();
		actual_camera_info->cameraEntity->name = actual_kill_camera_info->name;
	}
	else if (elem == "position")
	{
		assert(atts.find("x") != atts.end());
		btVector3 position =  btVector3(atof(atts["x"].c_str()),  atof(atts["y"].c_str()),  atof(atts["z"].c_str()));
		actual_camera_info->position = position;
	}	
	else if (elem == "look_at")
	{
		assert(atts.find("x") != atts.end());
		btVector3 lookAt =  btVector3(atof(atts["x"].c_str()),  atof(atts["y"].c_str()),  atof(atts["z"].c_str()));
		actual_camera_info->lookAt = lookAt;
	}
	else if (elem == "frame")
	{
		assert(atts.find("num") != atts.end());
		actual_camera_info->frame = atoi(atts["num"].c_str());
	}
}

void CameraSystem::onEndElement (const std::string &elem) {
	if( elem == "kill_camera" ) 
	{
		_killCameras[actual_kill_camera_info->name] = actual_kill_camera_info;
	}
	else if( elem == "death_camera" ) 
	{
		_deathCameras[actual_kill_camera_info->name] = actual_kill_camera_info;
	}
	else if (elem == "sequence")
	{
		actual_kill_camera_info->cameraSequences.push_back(actual_camera_sequence);
	}
	else if( elem == "data" ) 
	{
		actual_camera_sequence->cameraInfos.push_back(actual_camera_info);
		EntityManager::get().addComponent(new CameraComponent(actual_camera_info->cameraEntity, CAM_TYPE::CAM_CIN	,btVector3( 0,0,0 ), btVector3(0,0,0)
														, 0, (float) g_App.GetWidth() / (float) g_App.GetHeight(), 0, 0), actual_camera_info->cameraEntity);
	}
}
