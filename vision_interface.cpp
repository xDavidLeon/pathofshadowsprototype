#include "vision_interface.h"
#include "globals.h"
#include "d3ddefs.h"
#include "component_transform.h"
#include "system_camera.h"
#include "entity_manager.h"
#include "system_renderer.h"
#include "world.h"
#include "component_enemy_data.h"

VisionInterface::VisionInterface(void)
{
	_viewedByEnemy = _enemyAlert = _enemySearching = false;
	_compassAlpha = _arrowAlpha = 0;
	_normalColor = 255;
	_alertR = 100;
	_alertG = 0;
	_alertB = 0;
	_currentR = _currentG = _currentB = _normalColor;

	_compassTexture = TTextureManager::get().getTexture("hardcoded/brujula");
	_arrowTexture = TTextureManager::get().getTexture("hardcoded/arrow");

	D3DSURFACE_DESC desc;  _compassTexture->GetLevelDesc(0, &desc);
	_compassRadius = desc.Width/2;

	float halfResX = (float)g_App.GetWidth()*0.5f;
	float halfResY = (float)g_App.GetHeight()*0.5f;

	float Ypercent = 0.7f; //Queremos que la brujula ocupe un 50 porciento del espacio vertical de pantalla
	float desiredRadius = Ypercent*halfResY; //radio que queremos que tenga la textura
	float scaleC = desiredRadius/_compassRadius;

	D3DXMATRIX T, S;
	D3DXMatrixTranslation(&_compassM, -_compassRadius, -_compassRadius, 0); //pivote en centro de compassTexture
	D3DXMatrixScaling(&S, scaleC, scaleC, 0.0f); //Escalado (z es irrelevante)
	D3DXMatrixTranslation(&_centerScreenM, halfResX, halfResY, 0); //centro de la pantalla
	_compassM = _compassM*S*_centerScreenM;

	Ypercent = 0.4f;
	desiredRadius = Ypercent*halfResY;
	float scaleA = desiredRadius/_compassRadius;
	D3DXMatrixScaling(&S, scaleA, scaleA, 0.0f); //Escalado (z es irrelevante)

	_arrowTexture->GetLevelDesc(0, &desc);
	float arrowX = desc.Width/2;
	float arrowY = desc.Height/2;
	D3DXMatrixTranslation(&_arrowCenterTextureM, -arrowX, -arrowY, 1.0f); //pivote en centro de arrowTexture
	_arrowCenterTextureM = _arrowCenterTextureM*S;

	_compassRadius *= scaleC*1.02f; //le anyadimos un poco de margen
}

VisionInterface::~VisionInterface(void)
{
}

void VisionInterface::render()
{
	std::map<Entity*,Component*>* enemies = EntityManager::get().getAllEntitiesPosessingComponent<EnemyDataComponent>();
	if(!enemies) return;

	float angle, angle_abs, dx, dy;
	btVector3 cam_front_xz, cam_left_xz, delta_xz;
	const btVector3& playerPos = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->getPosition();
	CameraSystem::get().getPlayerCamera().getFront(cam_front_xz);  cam_front_xz.setY(0);
	CameraSystem::get().getPlayerCamera().getLeft(cam_left_xz);    cam_left_xz.setY(0);

	D3DXMATRIX M, R;
	float proj_in_front, proj_in_left;
	int alpha;

	TransformComponent* player_tC = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer());

	//dbg("%d\n", _viewedByEnemy);

	if(!_viewedByEnemy && !_enemyAlert && !_enemySearching)
	{
		_compassAlpha -= (int)(200.0f*World::instance()->getElapsedTimeRInSeconds());
		if(_compassAlpha < 0) _compassAlpha = 0;
	}

	if(_enemyAlert)
	{
		_currentR = 0.7f*_currentR + 0.3f*_alertR;
		_currentG = 0.7f*_currentG + 0.3f*_alertG;
		_currentB = 0.7f*_currentB + 0.3f*_alertB;
	}
	else if(_enemySearching)
	{
		_currentR = 0.7f*_currentR + 0.3f*_normalColor;
		_currentG = 0.7f*_currentG + 0.3f*_normalColor;
		_currentB = 0.7f*_currentB;
	}
	else
	{
		_currentR = 0.7f*_currentR + 0.3f*_normalColor;
		_currentG = 0.7f*_currentG + 0.3f*_normalColor;
		_currentB = 0.7f*_currentB + 0.3f*_normalColor;
	}

	if(_compassAlpha) RendererSystem::get().drawSprite(_compassTexture, _compassM, D3DCOLOR_ARGB(_compassAlpha, _currentR,_currentG,_currentB));

	_viewedByEnemy = _enemyAlert = _enemySearching = false;
	//_compassAlpha = 0;

	EnemyDataComponent* edc;
	TransformComponent* etc;

	std::map<Entity*,Component*>::iterator iter;
	for(iter = enemies->begin(); iter != enemies->end(); iter++)
	{
		edc = (EnemyDataComponent*)iter->second;
		if(edc->_visionPercent || edc->_attentionDegree == attentionDegrees::ALERT)
		{
			_viewedByEnemy = true;
			if(edc->_attentionDegree == attentionDegrees::ALERT && edc->_searching) _enemySearching = true;	
		}
		else continue;
		

		etc = EntityManager::get().getComponent<TransformComponent>(iter->first);

		//calculamos posicion en pantalla normalizada de 0 a 1 de la flechita
		delta_xz = etc->getPosition() - playerPos; delta_xz.setY(0);
		proj_in_front = cam_front_xz.dot(delta_xz);
		proj_in_left  = cam_left_xz.dot(delta_xz);
		angle = atan2f( proj_in_left, proj_in_front );
		angle_abs = fabs(angle);
		dx = -sinf(angle);
		dy = -cosf(angle);

		//if(angle < 0) angle+=M_PI*2;
		//dbg("%f\n", (angle/M_PI)*180);
		//dbg("dx: %f, dy: %f\n", dx, dy);

		//Situar y rotar la flechita
		D3DXMatrixTranslation(&M, dx*_compassRadius, dy*_compassRadius, 0);
		D3DXMatrixRotationZ(&R, -angle);
		M = _arrowCenterTextureM*R*_centerScreenM*M;

		if(_enemySearching)
		{
			_compassAlpha = 255;
			RendererSystem::get().drawSprite( _arrowTexture, M, D3DCOLOR_ARGB(255, _currentR,_currentG,_currentB));
		}
		else if(edc->_attentionDegree == attentionDegrees::ALERT)
		{
			_enemyAlert = true; //solo que uno este en alerta se pasa a modo alerta
			_compassAlpha = 255;
			RendererSystem::get().drawSprite( _arrowTexture, M, D3DCOLOR_ARGB(255, _currentR,_currentG,_currentB));
		}
		else
		{
			alpha = (int)((edc->_visionPercent*0.75f+0.25f)*255.0f); //normalizado de (0.25 a 1)*255
			//alpha = (int)(edc->_visionPercent*255.0f); //de 0 a 255
			if(alpha > _compassAlpha) _compassAlpha = alpha;
			RendererSystem::get().drawSprite( _arrowTexture, M, D3DCOLOR_ARGB(alpha, _normalColor,_normalColor,_normalColor));
		}
	}
	
}
