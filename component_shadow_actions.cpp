#include "component_shadow_actions.h"
#include "btBulletCollisionCommon.h"
#include "d3ddefs.h"
#include <math.h>
#include "globals.h"
#include "camera_controller_3rd.h"
#include "component_transform.h"
#include "component_model.h"
#include "system_light.h"
#include "component_player_controller.h"
#include "entity_manager.h"
#include "system_camera.h"
#include "system_shadow.h"
#include "system_physics.h"
#include "world.h"

ShadowActionsComponent::ShadowActionsComponent(Entity* e) : Component(e)
{
	_transC = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer());

	_maxColDist = 10.0f;

	//Offsets para calcular si en cierto sitio cabe la capsula de colision del player
	//Si en este punto no se ha cargado mesh lanzo error
	ModelComponent* mC = EntityManager::get().getComponent<ModelComponent>(World::instance()->getPlayer());
	assert(mC || fatal("ShadowActionsComponent: La model component del player es NULL!"));
	if(mC->getMesh()->aabb.half.x >= mC->getMesh()->aabb.half.z)
		_capsuleOffsetH = mC->getMesh()->aabb.half.x;
	else
		_capsuleOffsetH = mC->getMesh()->aabb.half.z;

	_capsuleOffsetV = mC->getMesh()->aabb.half.y;
	_capsuleOffsetV += _capsuleOffsetV*0.1f; //Le sumo un 10% para algo de espacio extra

	//matriz de la crosshair
	D3DXMATRIX scale;
	D3DXMatrixScaling(&scale, 13.0f, 13.0f, 0.0f);

	_mCrosshair = scale*d3dxidentity;
	_shadow_explosion = false;

	_shadow_preview = EntityManager::get().createEntity();
	btTransform t;
	t.setIdentity();
	_blob_transform = new TransformComponent(_shadow_preview, t);
	EntityManager::get().addComponent(_blob_transform, _shadow_preview);
	_blob_preview = new LightComponent(_shadow_preview, LIGHT_BLOB, D3DXCOLOR(0.0125f,0.025f,0.0325f, 1.0f));
	_blob_preview->setRadius(0.25f);
	EntityManager::get().addComponent(_blob_preview,_shadow_preview);
	_blob_preview->enabled = false;
	init();
}

void ShadowActionsComponent::init()
{
	enabled = false;
	_playerVisibility = playerVisibility::ONSHADOW;

	_lastCol = btVector3(0,0,0);
	_blobPos = btVector3(0,0,0);

	_lastNormal = btVector3(0,0,0);
	_previousCol = btVector3(10,0,0);
	_lastCreatedShadow = NULL;

	_exitBlendPos = btVector3(0,0,0);
	_telepMode = false;
	_canTeleport = false;
}

ShadowActionsComponent::~ShadowActionsComponent(void)
{
}

void ShadowActionsComponent::enableAiming()
{
	//Camara en pos cercana
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setDistanceNear();
	setTeleportMode(true);

	enabled = true;
	_blob_preview->enabled = true;
	_blob_transform->setPosition(_blobPos);
}

void ShadowActionsComponent::disableAiming()
{
	//Si estamos haciendo crecer una sombra, le pedimos que pare
	if(_lastCreatedShadow)
	{
		_lastCreatedShadow->stopGrowing();
		_lastCreatedShadow = NULL;
	}

	//Camara en pos lejana
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setDistanceFar();

	setTeleportMode(false);

	enabled = false;
	_blob_preview->enabled = false;
}

void ShadowActionsComponent::setLockOn(bool lockon)
{
	((CameraController3rd*)(CameraSystem::get().getPlayerCamera().controller))->lockOn(lockon);
}

void ShadowActionsComponent::setTeleportMode(bool activate)
{
	_telepMode = activate;

	if(activate)
	{
		//Camara en pos "zoom in"
		//CameraSystem::get().getPlayerCamera().changeFov(0.7);
		_lastCol = btVector3(0,0,0); //Para que se detecte cambio y se entre en el primer update de updateTeleportMode()
	}
	else
	{
		//Camara en pos cercana
		//CameraSystem::get().getPlayerCamera().changeToStdFov();
	}
}

void ShadowActionsComponent::update(float delta, bool creatingShadow)
{
	if(!creatingShadow)
	{
		//Lanzamos rayo y actualizamos datos
		updateRayCollision();

		//Hacemos mirar a la mesh a donde la camara
		TransformComponent * player_trans = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer());
		btVector3 cam_front;  CameraSystem::get().getPlayerCamera().getFront(cam_front);
		cam_front.setY(0);
		player_trans->approximateFront_v(cam_front, 10*delta);

		//Si se apunta para teletranspostarse, llamamos el update del teleport mode
		if(_telepMode) updateTeleportMode();
	}

	//Giramos la mirilla
	D3DXMATRIX rotation;
	if(canTeleport())
		D3DXMatrixRotationAxis(&rotation, &D3DXVECTOR3(0,0,1), -0.07f);
	else
		D3DXMatrixRotationAxis(&rotation, &D3DXVECTOR3(0,0,1), -0.005f);
	_mCrosshair = _mCrosshair * rotation;

	_blob_transform->setPosition(_blobPos);

}

//Comprueba si se puede teletransportar al lugar de la ultima colision
void ShadowActionsComponent::updateTeleportMode()
{
	if(_lastCol == btVector3(0,0,0) || !_lastColValid)
	{
		_canTeleport = false;
		return;
	}

	//Primero miramos si el punto de col. esta dentro de una sombra valida
	_canTeleport = ShadowSystem::get().checkValidPosToTeleport(D3DXVECTOR3(_lastCol + 0.05*_lastNormal)); //Lo flipas pavo neng que sin offset colisiona con el mismo punto A VECES

	//Si se esta apuntando a un sitio donde no hay sombras validas, el teletransporte no es posible
	if(!_canTeleport) return;

	//Comprobamos (para asegurar la estabilidad del juego) que se podra reactivar la malla de colision del player cerca de alli donde se quiere teletr.
	_canTeleport = checkIfCanExitBlend();
}

//Dada una posicion en una superficie, calcula si es posible hacer aparecer la capsula de colision del player cerca sin que atraviese la geometria de colision, y donde
bool ShadowActionsComponent::checkIfCanExitBlend()
{
	//La posicion de salida de blending inicial se calcula a partir del punto de colision y su normal
	_exitBlendPos = _lastCol + _capsuleOffsetH*_lastNormal;
	//Lo que movemos la pos en cada intento de que la posicion sea valida
	float deltaH = _capsuleOffsetH*0.25;
	float deltaV = _capsuleOffsetV*0.25;
	unsigned i = 0; unsigned n_tries = 10; //n_tries == numero de intentos que damos al algoritmo para que funcione
	std::vector<bool> col_results = std::vector<bool>(6);
	btVector3 btRayTo;

	while(i<n_tries)
	{
		//Comprobamos las 6 colisiones que simulan la capsula del player
		col_results.at(0) = PhysicsSystem::get().checkCollision(_exitBlendPos, _exitBlendPos + _capsuleOffsetV*_transC->getUp());    //up
		col_results.at(1) = PhysicsSystem::get().checkCollision(_exitBlendPos, _exitBlendPos - _capsuleOffsetV*_transC->getUp());    //down
		col_results.at(2) = PhysicsSystem::get().checkCollision(_exitBlendPos, _exitBlendPos + _capsuleOffsetH*_transC->getLeft());  //left
		col_results.at(3) = PhysicsSystem::get().checkCollision(_exitBlendPos, _exitBlendPos - _capsuleOffsetH*_transC->getLeft());  //right
		col_results.at(4) = PhysicsSystem::get().checkCollision(_exitBlendPos, _exitBlendPos + _capsuleOffsetH*_transC->getFront()); //front
		col_results.at(5) = PhysicsSystem::get().checkCollision(_exitBlendPos, _exitBlendPos - _capsuleOffsetH*_transC->getFront()); //back

		//if trapped, assume we can not exit blending
		if(col_results.at(0) && col_results.at(1) && col_results.at(2) && col_results.at(3) && col_results.at(4) && col_results.at(5)) return false;
		//else, if there're no collisions, we are free yet
		else if(!col_results.at(0) && !col_results.at(1) && !col_results.at(2) && !col_results.at(3) && !col_results.at(4) && !col_results.at(5)) return true;

		//check y axis
		if(col_results.at(0) && !col_results.at(1)) //move down
			_exitBlendPos = _exitBlendPos - deltaV*_transC->getUp();
		else if(!col_results.at(0) && col_results.at(1)) //move up
			_exitBlendPos = _exitBlendPos + deltaV*_transC->getUp();

		//check x axis
		if(col_results.at(2) && !col_results.at(3)) //move right
			_exitBlendPos = _exitBlendPos - deltaH*_transC->getLeft();
		else if(!col_results.at(2) && col_results.at(3)) //move left
			_exitBlendPos = _exitBlendPos + deltaH*_transC->getLeft();

		//check z axis
		if(col_results.at(4) && !col_results.at(5)) //move back
			_exitBlendPos = _exitBlendPos - deltaH*_transC->getFront();
		else if(!col_results.at(4) && col_results.at(5)) //move up
			_exitBlendPos = _exitBlendPos + deltaH*_transC->getFront();

		i++;
	}

	return false; //Si hemos llegado hasta aqui es que no se ha podido situar la capsula en una zona propicia
}

#include "entity_factory.h"
bool ShadowActionsComponent::teleportToTarget()
{

	float diff = (_transC->getPosition()-_exitBlendPos).length2();
	if (diff < 0.15f && _shadow_explosion == false) 
	{
		//Entity * parE2 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_exitBlendPos) - D3DXVECTOR3(0,1.0f,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_EXPLOSION);

		btVector3 f;
		_transC->getFrontXinv(f);
		ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(entity);
		Entity * parE2 = EntityFactory::get().createParticleEffect(D3DXVECTOR3(_transC->getPosition() +f),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_TELEPORT_END,m,D3DXVECTOR3(0,0,0),D3DXVECTOR3(f));
		_shadow_explosion = true;
	}
	if(diff<0.1)
	{
		_transC->setPosition(_exitBlendPos);
		//ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(entity);

		_shadow_explosion = false;
		return true;
	}

	//_transC->setPosition(_transC->getPosition()*0.975 + _exitBlendPos*0.025);

	//movimiento con v uniforme
	btVector3 dir = _exitBlendPos - _transC->getPosition(); dir.normalize();
	_transC->setPosition(_transC->getPosition() + 0.5f*dir);


	return false;
}

#include "component_automat.h"
void ShadowActionsComponent::updateVisibility(float delta)
{
	if(_playerVisibility == playerVisibility::TELEPORTING) return;

	bool dirLightHit = LightSystem::get().posInDirectionalLight(_transC->getPosition() + btVector3(0,-0.6f,0));
	
	//comprobamos si esta al alcance de alguna pointLight
	if(LightSystem::get().posInPointLight(_transC->getPosition() + btVector3(0,-0.6f,0)))
	{
		_playerVisibility = playerVisibility::ILLUMINATED;
		//Restamos un poco de vida al player cuando esta iluminado directamente
		EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->damageInLight();
	}
	//Comrpobamos si esta en sombra magica
	else if(ShadowSystem::get().checkPlayerInShadows())
	{
		_playerVisibility = playerVisibility::ONSHADOW;
		if (dirLightHit == false) EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->rechargeLife();
	}

	//Comprobamos si la luz de la luna le alcanza
	else 
	{
		if(dirLightHit)
			_playerVisibility = playerVisibility::VISIBLE;
		else
		{
			_playerVisibility = playerVisibility::ONSHADOW;
			EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->rechargeLife();
		}

	}

	// Cambiamos textura pj
	ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(entity);
	//TTexture diffuse1 = model->getFirstMaterial()->diffuse;
	//TTexture diffuse2 = model->getFirstMaterial()->diffuse2;
	//TTexture diffuse3 = model->getFirstMaterial()->diffuse3;

	Entity * bufanda = EntityManager::get().getEntityWithName("shadow_blend");
	D3DXVECTOR4 difColor = model->diffuseColor;
	float difSpeed = 1.0f;
	switch (_playerVisibility)
	{
	case playerVisibility::BLENDED:
		difColor += D3DXVECTOR4(-delta,-delta,-delta,0)*difSpeed;
		if (difColor.x < 0.2f) difColor = D3DXVECTOR4(0.2f,0.2f,0.2f,1.0f);
		model->diffuseColor = difColor;
		//model->setCurrentTextures(diffuse1);
		//model->setCurrentMaterialsName("tech_shadow_hidden");
		break;
	case playerVisibility::ILLUMINATED:
		difColor += D3DXVECTOR4(+delta,+delta,+delta,0)*difSpeed;
		if (difColor.x > 1.0f) difColor = D3DXVECTOR4(1,1,1,1.0f);
		model->diffuseColor = difColor;
		
		//model->setCurrentMaterialsName("tech_skin_pointLight");
		//model->setCurrentTextures(diffuse2);
		break;
	case playerVisibility::VISIBLE:
		difColor += D3DXVECTOR4(+delta,+delta,+delta,0)*difSpeed;
		if (difColor.x > 1.0f) difColor = D3DXVECTOR4(1,1,1,1.0f);
		model->diffuseColor = difColor;

		//model->setCurrentMaterialsName("tech_shadow_dirLight");
		//model->setCurrentTextures(diffuse2);
		break;
	case playerVisibility::ONSHADOW:
		difColor += D3DXVECTOR4(-delta,-delta,-delta,0)*difSpeed;
		if (difColor.x < 0.2f) difColor = D3DXVECTOR4(0.2f,0.2f,0.2f,1.0f);
		model->diffuseColor = difColor;
		
		//model->setCurrentMaterialsName("tech_shadow_hidden");
		//model->setCurrentTextures(diffuse1);
		break;
	}
	
	//_playerVisibility = playerVisibility::VISIBLE;
}

//Debe diferenciarse de la otra pq la posicion exacta de blend no es la misma que la de la transformada del player
void ShadowActionsComponent::updateVisibilityBlended(float delta)
{
	//comprobamos si esta al alcance de alguna pointLight
	if(LightSystem::get().posInPointLight(_blendPos))
	{
		_playerVisibility = playerVisibility::ILLUMINATED;
	}
	//Comprobamos si esta en sombra magica
	else if(ShadowSystem::get().checkPosInShadows(D3DXVECTOR3(_blendPos)))
	{
		_playerVisibility = playerVisibility::BLENDED;
	}
	//Comprobamos si la luz de la luna le alcanza
	else 
	{
		if(LightSystem::get().posInDirectionalLight(_blendPos))
			_playerVisibility = playerVisibility::VISIBLE;
		else
		{
			_playerVisibility = playerVisibility::BLENDED;
		}
	}

	// Cambiamos textura pj
	ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(entity);

	Entity * bufanda = EntityManager::get().getEntityWithName("shadow_blend");
	D3DXVECTOR4 difColor = model->diffuseColor;
	float difSpeed = 1.0f;

	switch (_playerVisibility)
	{
	case playerVisibility::BLENDED:
		difColor += D3DXVECTOR4(-delta,-delta,-delta,0)*difSpeed;
		if (difColor.x < 0.2f) difColor = D3DXVECTOR4(0.2f,0.2f,0.2f,1.0f);
		model->diffuseColor = difColor;
		break;
	}
}

void ShadowActionsComponent::updateRayCollision()
{
	btVector3 cam_front;  CameraSystem::get().getPlayerCamera().getFront(cam_front);
	btVector3 btRayFrom = CameraSystem::get().getPlayerCamera().getPosition();
    btVector3 btRayTo = btRayFrom + _maxColDist*cam_front;

	//Creamos callback para obtener resultado de la colision
    btCollisionWorld::ClosestRayResultCallback rayCallback(btRayFrom,btRayTo);
	rayCallback.m_collisionFilterMask = PhysicsSystem::get().colMaskShadow;

	//Test de colision
	PhysicsSystem::get().getCollisionWorld()->rayTest(btRayFrom, btRayTo, rayCallback);

	bool test1, test2;

	//Si colisiona hacemos cositas molonas
    if (rayCallback.hasHit())
    {
		_blob_preview->enabled = true;
		_blobPos = rayCallback.m_hitPointWorld;
		//La colision es valida si no es contra una zona prohibida
		test1 = rayCallback.m_collisionObject->getBroadphaseHandle()->m_collisionFilterGroup != colisionTypes::FORBIDDEN;
		test2 = !LightSystem::get().posInPointLight(rayCallback.m_hitPointWorld);

		if(test1 && test2)
		{
			_previousCol = _lastCol;
			_lastCol = rayCallback.m_hitPointWorld;
			_lastNormal = rayCallback.m_hitNormalWorld;
			_lastColValid = true;
			return;
		}

    }
	else _blob_preview->enabled = false;

	_lastCol = btVector3(0,0,0);
	_lastColValid = false;
}

void ShadowActionsComponent::shadowCreation()
{
	//Caso crear sombra
	if(_lastColValid && !_lastCreatedShadow)
	{
		Entity* lastE = ShadowSystem::get().createShadow(D3DXVECTOR3(_lastCol), D3DXVECTOR3(_lastNormal));
		_lastCreatedShadow = EntityManager::get().getComponent<ShadowComponent>(lastE);
		return;
	}
	else if(_lastCreatedShadow ) //Caso agrandar sombra
	{
		_lastCreatedShadow->grow();
	}
}

void ShadowActionsComponent::stopCreatingShadow()
{
	//Caso dejar de agrandar sombra
	if(_lastCreatedShadow)
	{
		_lastCreatedShadow->stopGrowing();
		_lastCreatedShadow = NULL;
	}
}

void ShadowActionsComponent::prepareToBlend() //Paso previo a fundirse en las sombras
{
	_playerVisibility = playerVisibility::BLENDED;

	//Desactivar colisiones del player (obtenemos el ghostObject(body) y el character controller y los sacamos del mundo fisico. Se queda asi hasta que se encuentre una forma mejor)
	CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(World::instance()->getPlayer());
   	PhysicsSystem::get().getDynamicsWorld()->removeCollisionObject(ccC->controller->getGhostObject());
	PhysicsSystem::get().getDynamicsWorld()->removeAction(ccC->controller);

	//Actualizamos la posicion de blended (siempre que se llame esta funcion es para blendearse al suelo)
	btVector3 from = _transC->getPosition();
	btVector3 to = from + btVector3(0.0f, -10.0f, 0.0f);
	btCollisionWorld::ClosestRayResultCallback rayCallback(from,to);
	rayCallback.m_collisionFilterMask = PhysicsSystem::get().colMaskNavigation;
	PhysicsSystem::get().getDynamicsWorld()->getCollisionWorld()->rayTest(from, to, rayCallback); //Deberia colisionar siempre con el suelo
	_blendPos = rayCallback.m_hitPointWorld;
	
}

void ShadowActionsComponent::blend() //Fundirse en las sombras
{
	//ToDo: "Desintegracion" del personaje en particulas que van a la sombra del suelo
	
	//Camara cambia comportamiento
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setDistanceBlended();
}

void ShadowActionsComponent::exitBlend()
{
	//ToDo: "Integracion" del personaje con las particulas que salen de la sombra del suelo

	//Camara sube
	((CameraController3rd*)CameraSystem::get().getPlayerCamera().controller)->setDistanceFar();

	//Volver a activar las colisiones del player (pillamos el controller y el body y los volvemos a meter en el mundo fisico)
	CharacterControllerComponent* ccC = EntityManager::get().getComponent<CharacterControllerComponent>(World::instance()->getPlayer());
	PhysicsSystem::get().getDynamicsWorld()->addCollisionObject(ccC->controller->getGhostObject(),colisionTypes::CHARARTER, -1);
	PhysicsSystem::get().getDynamicsWorld()->addAction(ccC->controller);

	_playerVisibility = playerVisibility::ONSHADOW;
}

//Pintar mirilla, del color correspondiente
void ShadowActionsComponent::render()
{
	if(!enabled) return;
	

	if(_lastCol == btVector3(0,0,0) || !_lastColValid)
	{
		//Mirilla1: no se pueden usar poderes
		renderTextureScreenSpace("hardcoded/mirilla3", 0.0f, 0.0f, _mCrosshair);
		_blob_preview->light_color = D3DXCOLOR(0.075f,0.0325f,0.0475f,1);
	}
	else
	{
		//Mirilla3: se pueden usar poderes
		renderTextureScreenSpace("hardcoded/mirilla1", 0.0f, 0.0f, _mCrosshair);
		_blob_preview->light_color = D3DXCOLOR(0.0125f,0.025f,0.0325f,1);
	}
}

void ShadowActionsComponent::renderDebug()
{
	if(!enabled) return;
	//btVector3 player_pos = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer())->getPosition();
	//float dist = player_pos.distance(_lastCol);
	//printf2D( 1000, 100, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "dist: %.0f", dist);
	//printf2D( 1000, 120, D3DCOLOR_ARGB( 255, 255, 255, 255 ), "obj. type: %s", _temp.c_str());

	
///Render de la normal de colision
	//Le damos al device la matriz de translacion a la pos de colision
	D3DXMATRIX m;
	D3DXMatrixTranslation(&m, _lastCol.getX(), _lastCol.getY(), _lastCol.getZ());
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &m );

	//Creamos vectores de colision y normal
	D3DXVECTOR3 hit = D3DXVECTOR3(_lastCol.getX(), _lastCol.getY(), _lastCol.getZ());
	D3DXVECTOR3 normal = D3DXVECTOR3(_lastNormal.getX(), _lastNormal.getY(), _lastNormal.getZ());
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );

	//Si la colision no es valida se pinta la normal blanca, sino verde
	if(_lastColValid)
		drawLineD3X(hit, (hit+5.0f*normal), D3DCOLOR_ARGB(255, 0,255,0));
	else
		drawLineD3X(hit, (hit+5.0f*normal), D3DCOLOR_ARGB(255, 255,255,255));
}

void ShadowActionsComponent::renderPlayerVisibility(float x, float y, unsigned color)
{
	switch(_playerVisibility)
	{
	case playerVisibility::ILLUMINATED:
			printf2D( x, y, color, "player visib.: illuminated");
			break;
		case playerVisibility::VISIBLE:
			printf2D( x, y, color, "player visib.: visible");
			break;
		case playerVisibility::ONSHADOW:
			printf2D( x, y, color, "player visib.: on shadow");
			break;
		case playerVisibility::BLENDED:
			printf2D( x, y, color, "player visib.: blended");
			break;
		case playerVisibility::TELEPORTING:
			printf2D( x, y, color, "player visib.: telep.");
			break;
	}

	////rayo de grounding
	//const btVector3& from = _transC->getPosition();
	//const btVector3& to = from - _capsuleOffsetV*_transC->getUp();
	//drawLine_bt(from, to, D3DCOLOR_ARGB(255, 255,255,255));
}

void ShadowActionsComponent::renderCapsuleCols(unsigned color)
{
	if(_exitBlendPos == btVector3(0,0,0)) return;

	btVector3 ymax, ymin, xmax, xmin, zmax, zmin;

	ymax = _transC->getUp()*_capsuleOffsetV;
	ymin = -ymax;
	xmax = _transC->getLeft()*_capsuleOffsetH;
	xmin = -xmax;
	zmax = _transC->getFront()*_capsuleOffsetH;
	zmin = -zmax;

	ymax = _exitBlendPos+ymax;
	ymin = _exitBlendPos+ymin;
	xmax = _exitBlendPos+xmax;
	xmin = _exitBlendPos+xmin;
	zmax = _exitBlendPos+zmax;
	zmin = _exitBlendPos+zmin;

	drawLineD3X(D3DXVECTOR3(ymax.getX(), ymax.getY(), ymax.getZ()), D3DXVECTOR3(ymin.getX(), ymin.getY(), ymin.getZ()), color);
	drawLineD3X(D3DXVECTOR3(xmax.getX(), xmax.getY(), xmax.getZ()), D3DXVECTOR3(xmin.getX(), xmin.getY(), xmin.getZ()), color);
	drawLineD3X(D3DXVECTOR3(zmax.getX(), zmax.getY(), zmax.getZ()), D3DXVECTOR3(zmin.getX(), zmin.getY(), zmin.getZ()), color);
}

bool ShadowActionsComponent::canCreateShadow() const
{
	return _lastColValid && 
			!_lastCreatedShadow && 
			EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->hasLifeForCreateShadow();
}

bool ShadowActionsComponent::canGrowShadow() const
{
	return EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->hasLifeForGrowShadow();
}

bool ShadowActionsComponent::canTeleport() const
{
	return _canTeleport && 
		EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->hasLifeForTeleport() && 
		_playerVisibility == playerVisibility::ONSHADOW;
}

bool ShadowActionsComponent::canBlend() const
{
	return EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->hasLifeForBlend() && 
		_playerVisibility == playerVisibility::ONSHADOW;
}

bool ShadowActionsComponent::canRecharge() const
{
	return _playerVisibility == playerVisibility::ONSHADOW;
}

bool ShadowActionsComponent::canUseSV() const
{
	return _playerVisibility == playerVisibility::ONSHADOW;
}

bool ShadowActionsComponent::isHidden() const
{
	return _playerVisibility == playerVisibility::ONSHADOW || _playerVisibility == playerVisibility::BLENDED;
}

//Dice si el player esta tocando el suelo o no
bool ShadowActionsComponent::isGrounded() const
{
	return PhysicsSystem::get().checkCollision(_transC->getPosition(), _transC->getPosition() - _capsuleOffsetV*_transC->getUp(), PhysicsSystem::get().colMaskNavigation);
}
