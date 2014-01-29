#include "scene_manager.h"
#include "mesh_manager.h"
#include "texture_manager.h"
#include "world.h"
#include "component_model.h"
#include "component_transform.h"
#include "component_rigidbody.h"
#include "component_charcontroller.h"
#include "component_automat.h"
#include "component_animation.h"
#include "component_shadow_actions.h"
#include "component_light.h"
#include "component_bt.h"
#include <map>
#include "bt_patrol.h"
#include "bt_gatekeeper.h"
#include "component_trigger.h"
#include "navigation_manager.h"
#include "system_camera.h"
#include "component_billboard.h"
#include "component_particle_effect.h"
#include "component_player_controller.h"
#include "entity_manager.h"
#include "system_renderer.h"
#include "texture_manager.h"
#include "mesh_manager.h"
#include "entity_factory.h"
#include "system_sound.h"
#include "system_unique.h"

TSceneManager::TSceneManager( ) 
	: curr_obj ( NULL ),
	curr_model(NULL),
	curr_material(NULL),
	curr_light(NULL)
{
}

void TSceneManager::onStartElement (const std::string &elem, MKeyValue &atts) {
	curr_element = elem;
	if(elem == "scene")
	{
		assert(atts.find("subFile") != atts.end());
		_subfileName = atts["subFile"];

		//Se carga el grafo de navegacion (deberia haber 1 siempre que se quiera que haya enemigos en ese escenario)
		if(atts.find("nav") != atts.end())
		{
			NavigationManager::get().releaseNavGraph();
			bool is_ok = NavigationManager::get().xmlParseFile("data/scenes/"+atts["nav"]);
			if( !is_ok ) assert(fatal( "NavigationMesh XML parser failed %s\n", getXMLError().c_str() ));
		}
	}
	else if (elem == "obj")
	{
		assert( curr_obj == NULL );
		Entity *obj = EntityManager::get().createEntity();
		curr_obj = obj;
	}
	else if (elem == "model")
	{
		assert (curr_obj);
		assert (curr_model == NULL);

		ModelComponent* model = new ModelComponent(curr_obj);
		EntityManager::get().addComponent(model,curr_obj);
		curr_model = model;
	}
	else if(elem == "mat")
	{
		assert (curr_model);
		assert (curr_material == NULL);
		TMaterial * m = new TMaterial();
		m->name = "tech_basic";
		m->diffuse = NULL;
		m->lightmap = NULL;
		m->specular = NULL;
		m->bumpmap = NULL;
		m->normalmap = NULL;
		m->emissive = NULL;

		curr_material = m;
	}
	else if (elem == "mix")
	{
		assert (curr_model);
		assert (curr_material);
	}
	else if(elem == "light")
	{
		assert(curr_obj);
		assert(curr_light == NULL);
		LightComponent * light = new LightComponent(curr_obj,LIGHT_TYPE::NONE,D3DXCOLOR(255,255,255,255));
		EntityManager::get().addComponent(light, curr_obj);
		curr_light = light;
	}
	else if (elem == "physics")
	{
		assert (curr_obj);

		TransformComponent * transformC = EntityManager::get().getComponent<TransformComponent>(curr_obj);
		ModelComponent * modelC = EntityManager::get().getComponent<ModelComponent>(curr_obj);
		assert(transformC);
		assert(modelC);

		btScalar mass = 0;
		bool isStatic = true;
		if (atts.find("static") != atts.end())
		{
			if (atts["static"] == "true") isStatic = true;
			else isStatic = false;

			if (!isStatic && modelC != NULL) mass = D3DXVec3Length( &modelC->getMesh()->aabb.half);
		}

		if (atts.find("shape") != atts.end())
		{
			string shape = atts["shape"];
			if (shape == "CONVEX_MESH") 
			{
				btRigidBody* body = PhysicsSystem::get().addConvexMeshCollider(transformC->transform,modelC->getMesh(),mass);
				RigidbodyComponent* rigidbodyC = new RigidbodyComponent(curr_obj,body);
				EntityManager::get().addComponent(rigidbodyC,curr_obj);
				btTransform* offsetT = new btTransform();
				offsetT->setIdentity();
				//offsetT->getOrigin() -= btVector3(0,modelC->getMesh()->aabb.half.getY(),0);
				modelC->setOffset(offsetT);
				transformC->transform = &rigidbodyC->body->getWorldTransform();
			}
			else if (shape == "CONCAVE_MESH") 
			{
				btRigidBody* body = PhysicsSystem::get().addConcaveMeshCollider(transformC->transform,modelC->getMesh(),mass);
 				RigidbodyComponent* rigidbodyC = new RigidbodyComponent(curr_obj,body);
				EntityManager::get().addComponent(rigidbodyC,curr_obj);
				btTransform* offsetT = new btTransform();
				offsetT->setIdentity();
				//offsetT->getOrigin() -= btVector3(0,modelC->getMesh()->aabb.half.getY(),0);
				modelC->setOffset(offsetT);
				transformC->transform = &rigidbodyC->body->getWorldTransform();
			}
			else if (shape == "PLANE") 
			{
				btRigidBody* body = PhysicsSystem::get().addStaticPlane(transformC->transform->getOrigin());
				RigidbodyComponent* rigidbodyC = new RigidbodyComponent(curr_obj,body);
				EntityManager::get().addComponent(rigidbodyC,curr_obj);
				transformC->transform = &rigidbodyC->body->getWorldTransform();
			}
			else if (shape == "BOX") 
			{
				btRigidBody* body = PhysicsSystem::get().addBoundingBox(transformC->transform,modelC->getMesh()->aabb.btAABBCenter(),modelC->getMesh()->aabb.btAABBHalf(),mass);
				RigidbodyComponent* rigidbodyC = new RigidbodyComponent(curr_obj,body);
				EntityManager::get().addComponent(rigidbodyC,curr_obj);
				transformC->transform = &rigidbodyC->body->getWorldTransform();
			}
			else if (shape == "SPHERE") 
			{
				btRigidBody* body = PhysicsSystem::get().addSphere(transformC->transform,modelC->getMesh()->aabb.btAABBHalf().length(),modelC->getMesh()->aabb.btAABBCenter(),mass);
				RigidbodyComponent* rigidbodyC = new RigidbodyComponent(curr_obj,body);
				EntityManager::get().addComponent(rigidbodyC,curr_obj);
				transformC->transform = &rigidbodyC->body->getWorldTransform();
			}
			else if (shape == "CAPSULE") 
			{
				btRigidBody* body = PhysicsSystem::get().addCapsule(transformC->transform,modelC->getMesh()->aabb.half.x,modelC->getMesh()->aabb.half.y,modelC->getMesh()->aabb.btAABBCenter(),mass);
				RigidbodyComponent* rigidbodyC = new RigidbodyComponent(curr_obj,body);
				EntityManager::get().addComponent(rigidbodyC,curr_obj);
				transformC->transform = &rigidbodyC->body->getWorldTransform();
			}
			else if (shape == "CHARACTER") 
			{
				btKinematicCharacterController* charC = PhysicsSystem::get().addCharacterController(transformC->transform,modelC->getMesh()->aabb.btAABBCenter(),modelC->getMesh()->aabb.btAABBHalf(), colisionTypes::CHARARTER, -1, mass);
				charC->setGravity(9.0);
				charC->setStepHeight(0.001);

				CharacterControllerComponent* charcontrollerC = new CharacterControllerComponent(curr_obj, charC);
				EntityManager::get().addComponent(charcontrollerC,curr_obj);
				transformC->transform = &charcontrollerC->controller->getGhostObject()->getWorldTransform();
				//btTransform* offsetT = new btTransform();
				//offsetT->setIdentity();
				//offsetT->getOrigin() -= btVector3(0,modelC->getMesh()->aabb.half.y*0.5f,0);
				//modelC->setOffset(offsetT);
				if(curr_obj->type == "PLAYER") 
				{
					World::instance()->setPlayer(curr_obj);
					AutomatComponent* autComp = new AutomatComponent(curr_obj, automatTypes::PLAYER); //Automata del player
					EntityManager::get().addComponent(autComp,curr_obj);
					Entity * e = EntityFactory::get().createParticleEffect(D3DXVECTOR3(0,0,0),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW,modelC);
					e->name = "P_Shadows";
					TransformComponent * tShadowPart = EntityManager::get().getComponent<TransformComponent>(e);
					tShadowPart->setParent(transformC);
				}
				if(curr_obj->type == "ENEMY") 
				{
				}
				EntityFactory::get().createBlob(D3DXVECTOR3(0,-1.0f,0), transformC,0.5f);
			}
		}
		else 
		{
			// DEFAULT PHYSICS = CONCAVE_MESH_SHAPE
			btRigidBody* body = PhysicsSystem::get().addConcaveMeshCollider(transformC->transform,modelC->getMesh(),mass);
			RigidbodyComponent* rigidbodyC = new RigidbodyComponent(curr_obj,body);
			EntityManager::get().addComponent(rigidbodyC,curr_obj);
		}

		if (atts.find("kinematic") != atts.end())
		{
			if (atts["kinematic"] == "true") 
			{
				RigidbodyComponent * rigidbodyC = EntityManager::get().getComponent<RigidbodyComponent>(curr_obj);
				assert(rigidbodyC);
				rigidbodyC->setKinematic(true);
			}
		}
	}
	else if (elem == "disabled")
	{
		curr_obj->enabled = false;
	}
	else if (elem == "ai")
	{
		if (atts.find("type") != atts.end())
		{
			if (atts["type"] == "patroller") 
			{
				BTComponent* btc = new BTComponent(curr_obj, btTypes::PATROLER);
				EntityManager::get().addComponent(btc, curr_obj);
			}
			else if(atts["type"] == "gatekeeper") 
			{
				BTComponent* btc = new BTComponent(curr_obj, btTypes::GATEKEEPER);
				EntityManager::get().addComponent(btc, curr_obj);
			}
		}
	}
	else if (elem == "waypoint") //(de patrol)
	{
		assert(atts.find("pos_x") != atts.end());
		assert(atts.find("pos_y") != atts.end());
		assert(atts.find("pos_z") != atts.end());

		float x = atof(atts["pos_x"].c_str());
		float y = atof(atts["pos_y"].c_str());
		float z = atof(atts["pos_z"].c_str());

		((BTPatrol*)(EntityManager::get().getComponent<BTComponent>(curr_obj))->getBT())->addWayPoint(btVector3(x,y,z));
	}
	else if (elem == "gk_pos") //(de gatekeeper)
	{
		assert(atts.find("pos_x") != atts.end());
		assert(atts.find("pos_y") != atts.end());
		assert(atts.find("pos_z") != atts.end());

		float x = atof(atts["pos_x"].c_str());
		float y = atof(atts["pos_y"].c_str());
		float z = atof(atts["pos_z"].c_str());

		((BTGatekeeper*)(EntityManager::get().getComponent<BTComponent>(curr_obj))->getBT())->setGKPlace(btVector3(x,y,z));
	}
	else if (elem == "lookAt") //(de gatekeeper)
	{
		assert(atts.find("pos_x") != atts.end());
		assert(atts.find("pos_y") != atts.end());
		assert(atts.find("pos_z") != atts.end());

		float x = atof(atts["pos_x"].c_str());
		float y = atof(atts["pos_y"].c_str());
		float z = atof(atts["pos_z"].c_str());

		((BTGatekeeper*)(EntityManager::get().getComponent<BTComponent>(curr_obj))->getBT())->setGKLookAt(btVector3(x,y,z));
	}
	else if (elem == "talk_to") //(de gatekeeper que habla)
	{
		assert(atts.find("enemy") != atts.end());

		std::string enemyName = atts["enemy"].c_str();
		Entity* enemy = EntityManager::get().getEntityWithName(enemyName);

		((BTGatekeeper*)(EntityManager::get().getComponent<BTComponent>(curr_obj))->getBT())->setAllyToTalk(enemy);
	}
	else if (elem == "cam_pos")
	{
		assert(atts.find("id") != atts.end());
		assert(atts.find("x") != atts.end());
		assert(atts.find("y") != atts.end());
		assert(atts.find("z") != atts.end());

		int cam_id = atoi(atts["id"].c_str());
		float x = atof(atts["x"].c_str());
		float y = atof(atts["y"].c_str());
		float z = atof(atts["z"].c_str());

		CameraSystem::get().addCinCameraPos(cam_id, btVector3(x,y,z));
	}
	else if (elem == "cam_lookAt")
	{
		assert(atts.find("id") != atts.end());
		assert(atts.find("x") != atts.end());
		assert(atts.find("y") != atts.end());
		assert(atts.find("z") != atts.end());

		int cam_id = atoi(atts["id"].c_str());
		float x = atof(atts["x"].c_str());
		float y = atof(atts["y"].c_str());
		float z = atof(atts["z"].c_str());

		CameraSystem::get().addCinCameraLookAt(cam_id, btVector3(x,y,z));
	}
	else if (elem == "cam_timing")
	{
		assert(atts.find("id") != atts.end());
		assert(atts.find("time") != atts.end());
		assert(atts.find("start_delay") != atts.end());
		assert(atts.find("finish_delay") != atts.end());

		int cam_id = atoi(atts["id"].c_str());
		float time = atof(atts["time"].c_str());
		float start_delay = atof(atts["start_delay"].c_str());
		float finish_delay = atof(atts["finish_delay"].c_str());
		bool startFromPlayer = false;
		bool endAtPlayer = false;
		if(atts.find("from_player") != atts.end())
		{
			if(atts["from_player"] == "true") startFromPlayer = true;
		}
		if(atts.find("to_player") != atts.end())
		{
			if(atts["to_player"] == "true") endAtPlayer = true;
		}

		CameraSystem::get().addCinCameraTiming(cam_id, btVector3(time,start_delay,finish_delay), startFromPlayer, endAtPlayer);
	}
	else if (elem == "moon")
	{
		assert(atts.find("x") != atts.end());
		assert(atts.find("y") != atts.end());
		assert(atts.find("z") != atts.end());

		float dir_x = atof(atts["x"].c_str());
		float dir_y = atof(atts["y"].c_str());
		float dir_z = atof(atts["z"].c_str());

		EntityFactory::get().createDirectionalLight(D3DXVECTOR3(dir_x,dir_y,dir_z),D3DXCOLOR(0.35f, 0.35f ,0.4f, 1.0f),1.0f);
		dirLight = btVector3(dir_x,dir_y,dir_z);
	}
	else if (elem == "fog")
	{
		assert(atts.find("start") != atts.end());
		assert(atts.find("end") != atts.end());
		
		float start = atof(atts["start"].c_str());
		float end = atof(atts["end"].c_str());

		RendererSystem::get().setFog(start,end);
		//World::instance()->getEntityFactory()->createDirectionalLight(D3DXVECTOR3(dir_x,dir_y,dir_z),D3DXCOLOR(0.35f, 0.35f ,0.4f, 1.0f),1.0f);
	}
	else if (elem == "respawn")
	{
		assert(atts.find("id") != atts.end());
		assert(atts.find("transform") != atts.end());

		const char* id =atts["id"].c_str();

		D3DXMATRIX M_dx;
		getMatrix(string(atts["transform"].c_str()), M_dx);
		btTransform M_bt;
		convertD3DXMatrix(&M_dx, M_bt);

		EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->addRespawn(id, M_bt);
	}
}

float TSceneManager::getFloat(const std::string &float_str)
{
	float res_float;
	sscanf(float_str.c_str(), "%f", &res_float);
	return res_float;
}

void TSceneManager::getMatrix(const std::string & matrix, D3DXMATRIX &target)
{
	const char *values = matrix.c_str();
	int n = sscanf( values, 
		"%f %f %f "
		"%f %f %f "
		"%f %f %f "
		"%f %f %f"
	, &target.m[0][0]
	, &target.m[0][1]
	, &target.m[0][2]
	, &target.m[1][0]
	, &target.m[1][1]
	, &target.m[1][2]
	, &target.m[2][0]
	, &target.m[2][1]
	, &target.m[2][2]
	, &target.m[3][0]
	, &target.m[3][1]
	, &target.m[3][2]
	);
	//assert( n == 12 || fatal( "Can't read 12 floats from matrix attribute %s. only %d\n", what, n ));
	target.m[0][3] = 0.0f;
	target.m[1][3] = 0.0f;
	target.m[2][3] = 0.0f;
	target.m[3][3] = 1.0f;
}



void TSceneManager::onData (const std::string &data)
{
	std::string myData = data;
	myData.erase(std::remove(myData.begin(),myData.end(), '\n'), myData.end());
	myData.erase(std::remove(myData.begin(),myData.end(), '\t'), myData.end());
	if(myData.size() == 0) return;
	
	assert( curr_obj );

	if (curr_element == "name")
	{
		curr_obj->name = data;
		EntityManager::get().addEntityByName(curr_obj);
	}
	else if (curr_element == "type") 
	{
		curr_obj->type = data;
		if(curr_obj->type == "TRIGGER")
		{
			TriggerComponent* triggerComp = new TriggerComponent(curr_obj);
			EntityManager::get().addComponent(triggerComp,curr_obj);
		}
	}
	else if(curr_element == "col_type")
	{
		//Si tiene col_type es que tiene body para colisiï¿½n
		RigidbodyComponent* rbC = EntityManager::get().getComponent<RigidbodyComponent>(curr_obj);
		rbC->body->getBroadphaseHandle()->m_collisionFilterMask = -1; //Colisionar con todo

		if(data == "VISIBLE_GEOM")
		{
			rbC->body->getBroadphaseHandle()->m_collisionFilterGroup = colisionTypes::VISIBLE_GEOM;
		}
		else if(data == "INVISIBLE_GEOM")
		{
			rbC->body->getBroadphaseHandle()->m_collisionFilterGroup = colisionTypes::INVISIBLE_GEOM;
		}
		else if(data == "FORBIDDEN")
		{
			rbC->body->getBroadphaseHandle()->m_collisionFilterGroup = colisionTypes::FORBIDDEN;
		}
	}
	else if (curr_element == "transform") 
	{
		D3DXMATRIX world;
		getMatrix(data,world);
		btTransform worldbt;
		convertD3DXMatrix(&world, worldbt);
		TransformComponent* transform = new TransformComponent(curr_obj,worldbt);
		EntityManager::get().addComponent(transform,curr_obj);
	}
	else if (curr_element == "character")
	{
		assert (curr_model);
		AnimationComponent* animComp = new AnimationComponent(curr_obj, data);
		EntityManager::get().addComponent(animComp,curr_obj);
		curr_model->setCModel(animComp->getModel());
		TMesh* mesh = TMeshManager::get( ).getMesh( data ); 
		curr_model->setMesh(mesh);
	}
	else if (curr_element == "mesh")
	{
		assert (curr_model);
		TMesh* mesh = TMeshManager::get( ).getMesh( _subfileName+"\\"+data ); 
		curr_model->setMesh(mesh);
		//Si tiene mesh de low se carga tb
		mesh = TMeshManager::get( ).getMesh( _subfileName+"\\low_"+data, false ); 
		if(mesh) curr_model->setMeshLow(mesh);
	}
	else if (curr_element == "visible") 
	{
		assert (curr_model);
		if (data == "true") curr_model->enabled = true;
		else curr_model->enabled = false;
	}
	else if (curr_element == "material_name")
	{
		assert( curr_model );
		assert (curr_material);
		curr_material->name = data;
		if (data == "undefined") curr_material->name = "tech_basic";
	}
	else if (curr_element == "diffuse")
	{
		assert( curr_model );
		assert (curr_material);
		curr_material->diffuse = TTextureManager::get( ).getTexture(data);
		curr_material->main_texture = curr_material->diffuse;
	}
	else if (curr_element == "diffuse2")
	{
		assert( curr_model );
		assert (curr_material);
		curr_material->diffuse2 = TTextureManager::get( ).getTexture(data);
	}
	else if (curr_element == "diffuse3")
	{
		assert( curr_model );
		assert (curr_material);
		curr_material->diffuse3 = TTextureManager::get( ).getTexture(data);
	}
	else if (curr_element == "mask")
	{
		assert( curr_model );
		assert (curr_material);
		curr_material->mask = TTextureManager::get( ).getTexture(data);
	}
	else if (curr_element == "normalmap")
	{
		assert( curr_model );
		assert (curr_material);
		curr_material->normalmap = TTextureManager::get( ).getTexture(data);
	}
	else if (curr_element == "bumpmap")
	{
		assert( curr_model );
		assert (curr_material);
		curr_material->bumpmap = TTextureManager::get( ).getTexture(data);
	}
	else if (curr_element == "emissive")
	{
		assert( curr_model );
		assert (curr_material);
		curr_material->emissive = TTextureManager::get( ).getTexture(data);
	}
	else if (curr_element == "lightmap")
	{
		assert( curr_model );
		assert (curr_material);
		curr_material->lightmap = TTextureManager::get( ).getTexture(data);
	}
	else if (curr_element == "no_cull")
	{
		assert( curr_model );
		if (data == "true")
			curr_model->render_flags["no_cull"] = true;
		else if (data == "false")
			curr_model->render_flags["no_cull"] = false;
	}
	else if (curr_element == "additive")
	{
		assert( curr_model );
		if (data == "true")
			curr_model->render_flags["additive"] = true;
		else if (data == "false")
			curr_model->render_flags["additive"] = false;
	}
	else if (curr_element == "forward")
	{
		assert( curr_model );
		if (data == "true")
			curr_model->render_flags["forward"] = true;
		else if (data == "false")
			curr_model->render_flags["forward"] = false;
	}
	else if (curr_element == "before_outline")
	{
		assert( curr_model );
		if (data == "true")
			curr_model->render_flags["before_outline"] = true;
		else if (data == "false")
			curr_model->render_flags["before_outline"] = false;
	}
	else if (curr_element == "aabb")
	{
		assert(curr_model);
		const char *values = data.c_str();
		D3DXVECTOR3 min;
		D3DXVECTOR3 max;
		int n = sscanf( values, 
		"%f %f %f "
		"%f %f %f "
		, &min[0]
		, &min[1]
		, &min[2]
		, &max[0]
		, &max[1]
		, &max[2]
		);

		curr_model->getMesh()->aabb.center = (max + min)/2.0f;
		curr_model->getMesh()->aabb.half = (max-min)/2.0f;
		d3dxvec3Absolute(curr_model->getMesh()->aabb.half);
		int x = 1;
		x++;
	}
	else if (curr_element == "light_type")
	{
		assert(curr_light);
		if (data == "POINT") curr_light->setType(LIGHT_POINT);
		else if (data == "DIRECTIONAL") curr_light->setType(LIGHT_DIRECTIONAL);
		else if (data == "SPOT") curr_light->setType(LIGHT_SPOT);
	}
	else if (curr_element == "light_color")
	{
		assert(curr_light);
		const char *values = data.c_str();
		D3DXCOLOR color;
		int n = sscanf( values, "%f %f %f ", &color.r
		, &color.g
		, &color.b
		);
		color.a = 1.0f;
		curr_light->light_color = color;
	}
	else if (curr_element == "light_direction")
	{
		assert(curr_light);
		const char *values = data.c_str();
		D3DXVECTOR3 dir;
		int n = sscanf( values, "%f %f %f ", &dir.x
		, &dir.y
		, &dir.z
		);
		curr_light->light_direction = dir;
	}
	else if (curr_element == "light_intensity")
	{
		assert(curr_light);
		const char *values = data.c_str();
		float intensity;
		int n = sscanf(values, "%f", &intensity);
		curr_light->light_intensity = intensity;
	}
	else if (curr_element == "light_radius")
	{
		assert(curr_light);
		const char *values = data.c_str();
		float radius;
		int n = sscanf(values, "%f", &radius);
		curr_light->setRadius(radius);
	}
	else if(curr_element == "trigger_radius")
	{
		EntityManager::get().getComponent<TriggerComponent>(curr_obj)->setRadius(getFloat(data));
	}
	else if(curr_element == "trigger_type")
	{
		EntityManager::get().getComponent<TriggerComponent>(curr_obj)->setType(data);
	}
	else if(curr_element == "trigger_command")
	{
		EntityManager::get().getComponent<TriggerComponent>(curr_obj)->setCommand(data);
	}
	else if (curr_element == "steps")
	{
		const btCollisionObject* collider = EntityManager::get().getComponent<RigidbodyComponent>(curr_obj)->body;

		SoundSystem::get().addColliderSound(collider, data);
	}
	else if (curr_element == "sound")
	{
		TransformComponent * transformC = EntityManager::get().getComponent<TransformComponent>(curr_obj);
		// @TEMPORAL
		std::string id = intToString(curr_obj->eid);
		if (data == "fuego")
			SoundSystem::get().playSFX3D(id, "data/sfx/antorcha.wav",id, transformC->getPosition(),btVector3(0,0,0),true, 0.0f,0.25f);
		else if (data == "agua")
			SoundSystem::get().playSFX3D(id, "data/sfx/water.ogg",id, transformC->getPosition(),btVector3(0,0,0),true, 0.0f,0.2f,3.0f,200.0f);
		else if (data == "grillos")
			SoundSystem::get().playSFX3D(id, "data/sfx/grillos.wav",id, transformC->getPosition(),btVector3(0,0,0),true, 0.0f,0.1f,4.0f,200.0f);
		//SoundSystem::get().addColliderSound(collider, data);
	}
	else if (curr_element == "component") // deprecated?
	{
		if (data == "billboard")
		{
			ModelComponent* m = EntityManager::get().getComponent<ModelComponent>(curr_obj);
			assert(m != NULL);
			TransformComponent* t = EntityManager::get().getComponent<TransformComponent>(curr_obj);
			BillboardComponent * b = new BillboardComponent(curr_obj,t, m);
			EntityManager::get().addComponent(b,curr_obj);
		}
	}
	else if (curr_element == "billboard")
	{
		ModelComponent* m = EntityManager::get().getComponent<ModelComponent>(curr_obj);
		assert(m != NULL);
		if (data == "false") return;
		TransformComponent* t = EntityManager::get().getComponent<TransformComponent>(curr_obj);
		BillboardComponent * b = new BillboardComponent(curr_obj,t, m);
		EntityManager::get().addComponent(b,curr_obj);
 	}
	else if (curr_element == "particles")
	{
		TransformComponent* t = EntityManager::get().getComponent<TransformComponent>(curr_obj);
		if (data == "fireflies") 
		{
			ParticleEffectComponent * p = new ParticleEffectComponent(curr_obj,ParticleEffectComponent::PARTICLE_EFFECT::FIREFLIES);
			EntityManager::get().addComponent(p,curr_obj);
		}
		else if (data == "light_dust") 
		{
			ParticleEffectComponent * p = new ParticleEffectComponent(curr_obj,ParticleEffectComponent::PARTICLE_EFFECT::LIGHT_DUST);
			EntityManager::get().addComponent(p,curr_obj);
		}
		else if (data == "leaves") 
		{
			ParticleEffectComponent * p = new ParticleEffectComponent(curr_obj,ParticleEffectComponent::PARTICLE_EFFECT::LEAVES);
			EntityManager::get().addComponent(p,curr_obj);
		}
		else if (data == "fire") 
		{
			ParticleEffectComponent * p = new ParticleEffectComponent(curr_obj,ParticleEffectComponent::PARTICLE_EFFECT::FIRE);
			EntityManager::get().addComponent(p,curr_obj);
		}
		else if (data == "fog") 
		{
			ParticleEffectComponent * p = new ParticleEffectComponent(curr_obj,ParticleEffectComponent::PARTICLE_EFFECT::FOG);
			EntityManager::get().addComponent(p,curr_obj);
		}
		else if (data == "smoke_fire") 
		{
			ParticleEffectComponent * p = new ParticleEffectComponent(curr_obj,ParticleEffectComponent::PARTICLE_EFFECT::SMOKE_FIRE);
			EntityManager::get().addComponent(p,curr_obj);
		}
		else if (data == "shadow_static") 
		{
			ParticleEffectComponent * p = new ParticleEffectComponent(curr_obj,ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_STATIC);
			EntityManager::get().addComponent(p,curr_obj);
		}
 	}

}

void TSceneManager::onEndElement (const std::string &elem) {
	if( elem == "obj" ) {
		// Tengo que estar dentro de un objeto
		assert( curr_obj );

		//objs_by_name[ curr_obj->name ] = curr_obj;
		//if(curr_obj->type == GAMEOBJECT_TYPE::PLAYER) World::instance()->setPlayer(curr_obj);

		curr_obj = NULL;	// Salimos del objeto
	}
	else if(elem == "model")
	{
		assert(curr_obj);
		assert(curr_model);
		if(curr_obj->type == "TUTORIAL")
		{
			UniqueSystem::get().addTutorial(curr_obj);
			curr_model->diffuseColor.w = 0.0f;
		}
		curr_model = NULL;
	}
	else if(elem == "light")
	{
		assert(curr_obj);
		assert(curr_light);
		curr_light = NULL;
	}
	else if (elem == "mat")
	{
		assert (curr_obj);
		assert (curr_model);
		assert (curr_material);
		curr_model->addMaterial(curr_material);
		
		curr_material = NULL;
	}

	curr_element = "";
}

Entity *TSceneManager::getByName( const char *name ) {
	MObjsByName::iterator i = objs_by_name.find( name );
	if( i == objs_by_name.end() ) {
		assert( fatal( "Can't find obj %s in the scene manager\n", name ) );
		return NULL;
	}
	return i->second;
}

void TSceneManager::render( ) {
	//MObjsByName::iterator i = objs_by_name.begin();
	//while( i != objs_by_name.end() ) {
	//	i->second->render();
	//	++i;
	//}
}

void TSceneManager::update(float delta ) {
	//MObjsByName::iterator i = objs_by_name.begin();
	//while( i != objs_by_name.end() ) {
	//	i->second->update(delta);
	//	++i;
	//}
}

void TSceneManager::releaseAll( ) {
	//MObjsByName::iterator i = objs_by_name.begin();
	//while( i != objs_by_name.end() ) {
	//	delete i->second;
	//	++i;
	//}
	//objs_by_name.clear( );
	//delete curr_obj;
}
