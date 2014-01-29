#include "entity_factory.h"
#include "component_model.h"
#include "component_transform.h"
#include "component_light.h"
#include "component_camera.h"
#include "mesh_manager.h"
#include "component_charcontroller.h"
#include "component_bt.h"
#include "bt_gatekeeper.h"
#include "component_particle_effect.h"
#include "component_shadow.h"
#include "entity_manager.h"
#include "texture_manager.h"
#include "bt_goddess.h"
#include "cine_Seq_01.h"
#include "cine_Seq_02.h"
#include "cine_Seq_03.h"
#include "cine_Seq_04.h"
#include "cine_Seq_05.h"
#include "cine_Seq_05_2.h"
#include "cine_Seq_06.h"
#include "cine_Seq_07.h"
#include "cine_Seq_08.h"
#include "cine_Seq_09.h"
#include "cine_Seq_10.h"

EntityFactory::EntityFactory(void)
{
}


EntityFactory::~EntityFactory(void)
{
}

//Crea un gatekeeper en la pos mirando hacia donde mira el player
Entity* EntityFactory::createEnemy(const btTransform& trans)
{
	Entity* enemy = EntityManager::get().createEntity();
	enemy->name = std::string("xu" + std::to_string(static_cast<long long>(enemy->eid)));
	enemy->type = "ENEMY";

	//Transform Component
	TransformComponent* transformC = new TransformComponent(enemy,trans);
	EntityManager::get().addComponent(transformC,enemy);

	//Model + Animation Component (+ materials)
	char* character = "xu";
	ModelComponent* modelC = new ModelComponent(enemy);
	EntityManager::get().addComponent(modelC,enemy);

	AnimationComponent* animC = new AnimationComponent(enemy, character);
	EntityManager::get().addComponent(animC,enemy);
	modelC->setCModel(animC->getModel());
	TMesh* mesh = TMeshManager::get().getMesh(character); 
	modelC->setMesh(mesh);

	TMaterial * m = new TMaterial();
	m->diffuse = TTextureManager::get( ).getTexture( "xu_d_2" );
	m->main_texture = m->diffuse;
	m->diffuse2 = TTextureManager::get( ).getTexture( "xu_d_1" );
	m->diffuse3 = TTextureManager::get( ).getTexture( "xu_d_0" );
	m->normalmap = TTextureManager::get( ).getTexture( "enemic_n" );
	m->specular = NULL;
	m->name = "tech_skin";
	modelC->addMaterial(m);

	//(aabb)
	const char *values = "-0.581745 -0.00498399 0.507839 0.409982 1.52444 -0.800522";
	D3DXVECTOR3 min, max;
	int n = sscanf( values, "%f %f %f %f %f %f ", &min[0], &min[1], &min[2], &max[0], &max[1], &max[2]);

	modelC->getMesh()->aabb.center = (max + min) * 0.5f;
	modelC->getMesh()->aabb.half = ((max-min)*0.5f);

	//Character Controller Component
	float mass = 0.0f;
	btKinematicCharacterController* charController = PhysicsSystem::get().addCharacterController(transformC->transform,modelC->getMesh()->aabb.btAABBCenter(),modelC->getMesh()->aabb.btAABBHalf(), colisionTypes::CHARARTER, -1, mass);
	CharacterControllerComponent* charControllerC = new CharacterControllerComponent(enemy, charController);
	EntityManager::get().addComponent(charControllerC, enemy);
	transformC->transform = &charControllerC->controller->getGhostObject()->getWorldTransform();
	btTransform* offsetT = new btTransform();
	offsetT->setIdentity();
	offsetT->getOrigin() -= btVector3(0,modelC->getMesh()->aabb.half.y,0);
	modelC->setOffset(offsetT);

	//Behaviour Tree Component
	BTComponent* btc = new BTComponent(enemy, btTypes::GATEKEEPER);
	EntityManager::get().addComponent(btc, enemy);
	((BTGatekeeper*)btc->getBT())->setGKPlace(trans.getOrigin());
	btVector3 frontXinv;
	transformC->getFrontXinv(frontXinv);
	((BTGatekeeper*)btc->getBT())->setGKLookAt(frontXinv);

	return enemy;
}

//Crea un gatekeeper en la pos mirando hacia donde mira el player
Entity* EntityFactory::createCrow()
{
	//Entidad, nombre, tipo
	Entity* crow = EntityManager::get().createEntity();
	crow->name = std::string("crow");
	EntityManager::get().addEntityByName(crow);
	crow->type = "CROW";

	//Transform Component
	btTransform Id; Id.setIdentity();
	TransformComponent* transformC = new TransformComponent(crow, Id);
	EntityManager::get().addComponent(transformC,crow);

	//Model + Animation Component (+ materials)
	char* character = "raven";
	ModelComponent* modelC = new ModelComponent(crow);
	EntityManager::get().addComponent(modelC,crow);

	AnimationComponent* animC = new AnimationComponent(crow, character);
	EntityManager::get().addComponent(animC,crow);
	modelC->setCModel(animC->getModel());
	TMesh* mesh = TMeshManager::get().getMesh(character); 
	modelC->setMesh(mesh);

	modelC->enabled = true;

	TMaterial * m = new TMaterial();
	m->diffuse = TTextureManager::get( ).getTexture( "hardcoded/raven_i" );
	m->main_texture = m->diffuse;
	m->lightmap = NULL;
	m->specular = NULL;
	m->specular = NULL;
	m->bumpmap = NULL;
	m->normalmap = NULL;
	m->emissive = TTextureManager::get( ).getTexture( "hardcoded/raven_d" );
	m->name = "tech_crow"; //m->name = "tech_basic";
	modelC->addMaterial(m);

	//Behaviour Tree Component
	BTComponent* btc = new BTComponent(crow, btTypes::CROW);
	EntityManager::get().addComponent(btc, crow);

	return crow;
}

Entity* EntityFactory::createDirectionalLight(D3DXVECTOR3 & direction, D3DXCOLOR color, float intensity)
{
	btTransform t;
	t.setIdentity();
	Entity* entity = EntityManager::get().createEntity();
	EntityManager::get().addComponent(new TransformComponent(entity, t),entity);
	LightComponent * light = new LightComponent(entity, LIGHT_DIRECTIONAL, color, intensity);
	light->light_direction = direction;
	D3DXMatrixOrthoLH(&light->light_ortho,g_App.GetWidth(),g_App.GetHeight(),1,1000);

	EntityManager::get().addComponent(light,entity);
	return entity;
}

Entity* EntityFactory::createPointLight(btVector3 & position, D3DXCOLOR color, float intensity, float radius, bool isTorch)
{
	btTransform t;
	t.setIdentity();
	t.setOrigin(position);
	Entity* entity = EntityManager::get().createEntity();
	EntityManager::get().addComponent(new TransformComponent(entity,t),entity);
	LightComponent * light = new LightComponent(entity, LIGHT_POINT, color, intensity);
	light->setRadius(radius);
	light->isTorch = isTorch;
	EntityManager::get().addComponent(light,entity);
	return entity;
}

Entity* EntityFactory::createSpotLight(D3DXVECTOR3 & position, D3DXVECTOR3 & direction, D3DXCOLOR color, float intensity, float lnear, float lfar, float lfov, bool generateShadows)
{
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(position.x,position.y,position.z));
	Entity* entity = EntityManager::get().createEntity();
	EntityManager::get().addComponent(new TransformComponent(entity,t),entity);
	LightComponent * light = new LightComponent(entity, LIGHT_SPOT, color, intensity);
	light->light_near = lnear;
	light->light_near = lfar;
	light->light_fov = lfov;
	light->light_direction = direction;
	D3DXMatrixLookAtRH( &light->light_view, &position, &D3DXVECTOR3(position + direction), &D3DXVECTOR3(0,1,0) );
	D3DXMatrixPerspectiveFovRH( &light->light_projection
		                      , lfov, 1.0f
							  , lnear, lfar );
	if (generateShadows) 
	{
		g_App.GetDevice()->CreateTexture(g_App.GetParameters().BackBufferWidth,g_App.GetParameters().BackBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_R32F,D3DPOOL_DEFAULT,&light->rt_shadowmap,NULL);
	}

	EntityManager::get().addComponent(light,entity);
	return entity;
}

Entity* EntityFactory::createMagicShadow(const D3DXVECTOR3 & position, const D3DXVECTOR3 & upVector, int shadow_id)
{
	Entity* entity = EntityManager::get().createEntity();
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(position.x,position.y,position.z));
	EntityManager::get().addComponent(new TransformComponent(entity, t), entity);
	ShadowComponent * shadow = new ShadowComponent(entity, upVector, shadow_id);
	EntityManager::get().addComponent(shadow, entity);
	return entity;
}

Entity* EntityFactory::createBlob(const D3DXVECTOR3 & position, TransformComponent * parent, float radius)
{
	Entity* entity = EntityManager::get().createEntity();
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(position.x,position.y,position.z));
	TransformComponent * tran = new TransformComponent(entity, t);
	tran->setParent(parent);
	EntityManager::get().addComponent(tran, entity);

	LightComponent * light = new LightComponent(entity, LIGHT_BLOB, D3DXCOLOR(0.0125f,0.025f,0.0325f,1), 1.0f);
	light->setRadius(radius);
	EntityManager::get().addComponent(light,entity);
	return entity;
}

Entity* EntityFactory::createCamera(CAM_TYPE type, const btVector3& pos, const btVector3& target, float fov_in_degrees, float aspect_ratio,  float cam_near, float cam_far)
{
	Entity* entity = EntityManager::get().createEntity();
	EntityManager::get().addComponent(new CameraComponent(entity, type, pos, target, fov_in_degrees, aspect_ratio, cam_near, cam_far),entity);
	return entity;
}

Entity* EntityFactory::createParticleEffect(const D3DXVECTOR3 & position, ParticleEffectComponent::PARTICLE_EFFECT effect_type, ModelComponent * model,  D3DXVECTOR3 dest ,D3DXVECTOR3 vel  )
{
	Entity* entity = EntityManager::get().createEntity();
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(position.x,position.y,position.z));
	EntityManager::get().addComponent(new TransformComponent(entity, t), entity);
	EntityManager::get().addComponent(new ParticleEffectComponent(entity,effect_type, model, dest, vel),entity);

	return entity;
}

void EntityFactory::createCineSeq(int cine_id)
{
	Entity* cine = EntityManager::get().createEntity();
	cine->type = "CINE_SEQ";
	BTComponent* btc = new BTComponent(cine, btTypes::CINE_SEQ);

	switch(cine_id)
	{
	case 1: 
		btc->changeBT(new CineSeq01(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 2: 
		btc->changeBT(new CineSeq02(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 3: 
		btc->changeBT(new CineSeq03(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 4: 
		btc->changeBT(new CineSeq04(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 5: 
		btc->changeBT(new CineSeq05(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 52:
		btc->changeBT(new CineSeq05_2(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 6: 
		btc->changeBT(new CineSeq06(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 7: 
		btc->changeBT(new CineSeq07(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 8: 
		btc->changeBT(new CineSeq08(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 9: 
		btc->changeBT(new CineSeq09(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	case 10: 
		btc->changeBT(new CineSeq10(cine));
		EntityManager::get().addComponent(btc, cine);
		break;
	}
	
}

void EntityFactory::createGoddess()
{
	//Si se llama esto deberia haber cargado ya un character goddess_crow
	Entity* goddess_crow = EntityManager::get().getEntityWithName("goddess_crow");
	if(!goddess_crow) return;

	D3DXMATRIX vOffset_dx;
	btTransform vOffset_bt;
	D3DXMatrixTranslation(&vOffset_dx, 0, 0.8f, 0);
	convertD3DXMatrix(&vOffset_dx, vOffset_bt);
	
	*EntityManager::get().getComponent<TransformComponent>(goddess_crow)->transform = *EntityManager::get().getComponent<TransformComponent>(goddess_crow)->transform * vOffset_bt;

//entidad goddess (animaciones)
	//Entidad, nombre, tipo
	Entity* goddess = EntityManager::get().createEntity();
	goddess->name = std::string("goddess");
	EntityManager::get().addEntityByName(goddess);
	goddess->type = "GODDESS";
	goddess->enabled = false;

	//Transform Component
	btTransform Id; Id.setIdentity();
	TransformComponent* transformC = new TransformComponent(goddess, Id);
	EntityManager::get().addComponent(transformC,goddess);

	//Model + Animation Component (+ materials)
	char* character = "goddess";
	ModelComponent* modelC = new ModelComponent(goddess);
	EntityManager::get().addComponent(modelC,goddess);

	AnimationComponent* animC = new AnimationComponent(goddess, character);
	EntityManager::get().addComponent(animC,goddess);
	modelC->setCModel(animC->getModel());
	TMesh* mesh = TMeshManager::get().getMesh(character); 
	modelC->setMesh(mesh);
	modelC->enabled = true;

	TMaterial * m = new TMaterial();
	m->diffuse = TTextureManager::get( ).getTexture( "hardcoded/god_ghost_d" );
	m->main_texture = m->diffuse;
	m->lightmap = NULL;
	m->specular = NULL;
	m->specular = NULL;
	m->bumpmap = NULL;
	m->normalmap = NULL;
	m->emissive = TTextureManager::get( ).getTexture( "hardcoded/god_ghost_d" );
	m->name = "tech_skin";
	modelC->addMaterial(m);


//Entidad goddess_bt (la que engloba las otras)
	Entity* goddess_bt = EntityManager::get().createEntity();
	goddess_bt->name = std::string("goddess_bt");
	EntityManager::get().addEntityByName(goddess_bt);
	goddess_bt->type = "GODDESS";

	//Behaviour Tree Component
	BTComponent* btc = new BTComponent(goddess_bt, btTypes::GODDESS);
	EntityManager::get().addComponent(btc, goddess_bt);

	//Se le dan las referencias de los models con animacion y tal
	((BTGoddess*)btc->getBT())->setEntitites(goddess_crow, goddess);
}
