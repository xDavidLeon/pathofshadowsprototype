#include "component_particle_effect.h"
#include "world.h"
#include "data_saver.h"
#include "mesh_manager.h"
#include "texture_manager.h"
#include "d3ddefs.h"
#include "system_camera.h"
#include <algorithm>    // std::sort
#include "component_transform.h"
#include "entity_manager.h"
#include "component_light.h"
#include "entity_factory.h"
#include "component_model.h"
#include "component_shadow_actions.h"
#include "component_player_controller.h"
#include "system_debug.h"

static D3DVERTEXELEMENT9 particles_decl_elems[] = {
	// Stream 0, la geometria
	{0,  0, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  0},
	{0, 12, D3DDECLTYPE_FLOAT2,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0},

	// Stream 1, los datos por instancia
	{1,  0, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  1}, // Posicion - 4*3 = 12 bytes
	{1, 12, D3DDECLTYPE_FLOAT4,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     1}, // Color - 4*4 = 16 bytes
	{1, 28, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1}, // Escala - 4*3 = 12 bytes
	{1, 40, D3DDECLTYPE_FLOAT1,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  2}, // Frame - 4 bytes
	{1, 44, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  3}, // Angle - 4*3 = 12 bytes

	D3DDECL_END()
};
static LPDIRECT3DVERTEXDECLARATION9 particles_decl;

ParticleEffectComponent::ParticleEffectComponent(Entity* e, PARTICLE_EFFECT particle_effect, ModelComponent * model, D3DXVECTOR3 dest, D3DXVECTOR3 velocity) : Component(e)
{
	effect = particle_effect;
	initDefaultParams();
	initParams(model, dest, velocity);
}

ParticleEffectComponent::~ParticleEffectComponent(void)
{
	destroy();
}

void ParticleEffectComponent::initDefaultParams(void)
{
	num_frames = 1;
	aabb_radius = -1;
	is_emitting = true;
	tech_name = "tech_particles";
	additive = true;
	max_life = D3DXVECTOR2(0,0);
	aabb = TAABB(D3DXVECTOR3(0,0,0),D3DXVECTOR3(0,0,0));
	destroy_on_finish = false;
	sort_particles = false;
	timer_emission = 999;
	ignore_outline = true;
}

void ParticleEffectComponent::initParams(ModelComponent * m, D3DXVECTOR3 dest, D3DXVECTOR3 velocity)
{
	model = m;
	destiny = dest;
	velocity_original = velocity;
	switch (effect)
	{
		case DECOY:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/bell");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 3 );
		num_frames = 1;
		additive = false;
		tech_name = "tech_particles_no_world_soft";
		sort_particles = false;
		destroy_on_finish = true;
		}
		break;
	case SHADOW:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 32 );
		num_frames = 8;
		additive = false;
		tech_name = "tech_particles_no_world_soft";
		sort_particles = false;
		}
		break;
	case SHADOW_TELEPORT:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 128 );
		num_frames = 8;
		additive = false;
		tech_name = "tech_particles_no_world";
		destroy_on_finish = true;
		sort_particles = false;
		}
		break;
	case SHADOW_TELEPORT_END:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 24 );
		num_frames = 8;
		additive = false;
		tech_name = "tech_particles_soft";
		destroy_on_finish = true;
		sort_particles = false;
		}
		break;
	case SHADOW_EXPLOSION:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 32 );
		num_frames = 8;
		additive = false;
		tech_name = "tech_particles";
		destroy_on_finish = true;
		sort_particles = false;
		aabb_radius = -1;
		}
		break;
	case SHADOW_EXPLOSION_CROW:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/petalo");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 32 );
		num_frames = 1;
		additive = false;
		tech_name = "tech_particles_no_world";
		destroy_on_finish = true;
		sort_particles = false;
		aabb_radius = -1;
		}
		break;
	case SHADOW_STATIC:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 32 );
		num_frames = 8;
		additive = false;
		tech_name = "tech_particles_soft";
		//destroy_on_finish = true;
		sort_particles = false;
		aabb_radius = 4;
		destroy_on_finish = true;
		}
		break;
	case SHADOW_STATIC_FALL:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 32 );
		num_frames = 8;
		additive = false;
		tech_name = "tech_particles_soft";
		//destroy_on_finish = true;
		sort_particles = false;
		destroy_on_finish = true;
		}
		break;
	case SHADOW_EXPLOSION_MODEL:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 64 );
		num_frames = 8;
		additive = false;
		tech_name = "tech_particles_no_world_soft";
		destroy_on_finish = true;
		sort_particles = false;
		}
		break;
	case SHADOW_WALK:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 32 );
		num_frames = 8;
		additive = false;
		tech_name = "tech_particles_no_world";
		sort_particles = false;
		}
		break;
	case SHADOW_CROW:
		{
		texture = TTextureManager::get().getTexture("hardcoded/particles/petalo");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 6 );
		num_frames = 1;
		additive = false;
		tech_name = "tech_particles_no_world_soft";
		sort_particles = false;
		destroy_on_finish = false;
		}
		break;
	case SHADOW_CREATION:
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 128 );
		num_frames = 8;
		additive = false;
		aabb_radius = 1.5f;
		tech_name = "tech_particles";
		destroy_on_finish = false;
		sort_particles = false;
		break;
	case FIRE: 
		texture = TTextureManager::get().getTexture("hardcoded/particles/fire");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 32 );
		aabb_radius = 2.0f;
		tech_name = "tech_particles";
		sort_particles = false;
		break;
	case SHADOW_HAND: 
		texture = TTextureManager::get().getTexture("hardcoded/particles/magic_shadow");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 16 );
		aabb_radius = -1.0f;
		tech_name = "tech_particles_no_world";
		sort_particles = false;
		ignore_outline = false;
		destroy_on_finish = false;
		additive = false;
		num_frames = 8;
		break;
	case FIREFLIES:
		texture = TTextureManager::get().getTexture("hardcoded/particles/firefly");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 16 );
		aabb_radius = 10.0f;
		tech_name = "tech_particles";
		sort_particles = false;
		// Special params for lights
		for (int inst = 0; inst < instances.size(); inst++)
		{
			Entity * ent = EntityFactory::get().createPointLight(btVector3(0,0,0),D3DXCOLOR(255,255,255,255),0.25f,0.5f,false);
			lights.push_back(ent);
		}
		break;
	case LEAVES:
		texture = TTextureManager::get().getTexture("hardcoded/particles/petalo");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 32 );
		aabb_radius = 10.0f;
		additive = false;
		tech_name = "tech_particles";
		sort_particles = false;
		break;
	case LIGHT_DUST:
		texture = TTextureManager::get().getTexture("hardcoded/particles/pointlight");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 8 );
		aabb_radius = 6.0f;
		tech_name = "tech_particles";
		sort_particles = false;
		break;
	case BLOOD_SPLATTER:
		texture = TTextureManager::get().getTexture("hardcoded/particles/blood_2");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 200 );
		num_frames = 4;
		aabb_radius = 1.0f;
		tech_name = "tech_particles";
		sort_particles = false;
		destroy_on_finish = true;
		additive = false;
		break;
	case FOG:
		texture = TTextureManager::get().getTexture("hardcoded/particles/fog");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 32 );
		aabb_radius = 10.0f;
		additive = false;
		tech_name = "tech_particles_soft";
		sort_particles = true;
		ignore_outline = false;
		break;

	case SMOKE_FIRE:
		texture = TTextureManager::get().getTexture("hardcoded/particles/smoke_fire");
		mesh = TMeshManager::get( ).getMesh( "particle" );
		initParticles( 16 );
		aabb_radius = 3.0f;
		additive = false;
		tech_name = "tech_particles";
		sort_particles = false;
		break;
	}

	wakeUp();

	TransformComponent * transform = EntityManager::get().getComponent<TransformComponent>(entity);
	aabb.center = (D3DXVECTOR3(transform->getWorldTransform()->getOrigin()));
	aabb.half = D3DXVECTOR3(aabb_radius,aabb_radius,aabb_radius);
	timer_alive = 0;
}

void ParticleEffectComponent::wakeUp(void)
{

	switch (effect)
	{
		case BLOOD_SPLATTER:
		{
		setMaxLife(D3DXVECTOR2(0.5f, 0.8f));

		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.5f),D3DXVECTOR3(0,0,0.5f));
		setRandomRotation(false,false,true);
		//setRandomFrame(0,5);
		setRandomStartLife();
		setRandomFrame(0,3);
		setRandomVelocity(velocity_original - D3DXVECTOR3(0.5f,0.5f,0.5f),velocity_original + D3DXVECTOR3(0.5f,0.5f,0.5f));
		setColor(D3DXVECTOR4(0.25f,0.0f,0.0f,0.0f));
		timer_emission = 0.25f;
		//// Bone special params
		//updateBonePoints(model->getCModel());
		//updateBoneFloorPoints();
		//setRandomBoneId(0,bone_points.size());
		//setCenterAtBoneFloor();
		//setDestPosAtBone();
		}
		break;
		case DECOY:
		{
		setMaxLife(D3DXVECTOR2(1.5f, 1.5f));
		//setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		//setRandomRotation(false,false,true);
		//setRandomFrame(0,5);
		//setRandomStartLife();
		setColor(D3DXVECTOR4(1.0f,1.0f,1.0f,0.0f));
		instances[ 0 ].alive = 0;
		instances[ 1 ].alive = 0.5f;
		instances[ 2 ].alive = 1.0f;
		timer_emission = 1.5f;
		//// Bone special params
		updateBonePoints(model->getCModel());
		//updateBoneFloorPoints();
		//setRandomBoneId(0,bone_points.size());
		//setCenterAtBoneFloor();
		//setDestPosAtBone();
		}
		break;
	case SHADOW:
		{
		setMaxLife(D3DXVECTOR2(2.0f, 4.0f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomRotation(false,false,true);
		setRandomFrame(0,6);
		//setRandomStartLife();
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));

		// Bone special params
		updateBonePoints(model->getCModel());
		updateBoneFloorPoints();
		setRandomBoneId(0,bone_points.size());
		setCenterAtBoneFloor();
		setDestPosAtBone();
		}
		break;
	case SHADOW_HAND:
		{
		setMaxLife(D3DXVECTOR2(2.5f, 3.0f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomRotation(false,false,true);
		setRandomStartLife();
		setRandomFrame(0,6);
		setRandomVelocity(D3DXVECTOR3(-0.05f,0.05f,-0.05f),D3DXVECTOR3(0.05f,0.1f,0.05f));
		//setRandomStartLife();
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));
		// Bone special params
		int bId = model->getCModel()->getSkeleton()->getCoreSkeleton()->getCoreBoneId("Bip001 L Hand");
		updateBonePoints(model->getCModel());
		updateBonePoint(model->getCModel(), bId);
		setBoneId(bId);
		setCenterAtBone(true);
		//updateBoneFloorPoints();
		//setRandomBoneId(0,bone_points.size());
		//setCenterAtBoneFloor();
		//setDestPosAtBone();
		}
		break;
	case SHADOW_CROW:
		{
		setMaxLife(D3DXVECTOR2(3.0f, 3.5f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomRotation(true, true, true);
		setScale(D3DXVECTOR3(0.06f,0.06f,0.06f));
		setRandomStartLife();
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));
		setRandomVelocity(D3DXVECTOR3(-0.1f,-0.3f,-0.4f),D3DXVECTOR3(-0.2f,-0.15f,0.4f));
		// Bone special params
		updateBonePoints(model->getCModel());
		setRandomBoneId(0,bone_points.size());
		setCenterAtBone();
		}
		break;
	case SHADOW_TELEPORT:
		{
		setMaxLife(D3DXVECTOR2(3.0f, 3.0f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomRotation(false,false,true);
		setRandomFrame(0,6);
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));
		setRandomVelocity(D3DXVECTOR3(-0.5f,-0.5f,-0.5f),D3DXVECTOR3(0.5f,0.5f,0.5f));

		// Bone special params
		updateBonePoints(model->getCModel());
		setRandomBoneId(0,bone_points.size());
		setCenterAtBone();
		setRandomDestiny(destiny - D3DXVECTOR3(-0.5f,0,-0.5f), destiny + D3DXVECTOR3(0.5f,0,0.5f));
		//setRandomOrigin(D3DXVECTOR3(-0.5f,-0.5f,-0.5f), D3DXVECTOR3(0.5f,0.5f,0.5f));

		is_emitting = false;
		}
		break;
	case SHADOW_TELEPORT_END:
		{
		setMaxLife(D3DXVECTOR2(1.5f, 1.75f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.5f),D3DXVECTOR3(0,0,0.5f));
		setRandomRotation(false,false,true);
		setRandomFrame(0,6);
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));
		setRandomVelocity(velocity_original + D3DXVECTOR3(-0.25f,-0.1f,-0.25f),velocity_original + D3DXVECTOR3(0.25f,0.25f,0.25f));
		setRandomCenter(D3DXVECTOR3(-0.35f,-1.0f,-0.35f),D3DXVECTOR3(0.35f,0.0f,0.35f));
		// Bone special params
		//updateBonePoints(model->getCModel());
		//setRandomBoneId(0,bone_points.size());
		//setCenterAtBone(true);
		//setOffsetToCenter(D3DXVECTOR3(0,-0.5f,0),true);
		//setRandomDestiny(destiny - D3DXVECTOR3(-0.5f,0,-0.5f), destiny + D3DXVECTOR3(0.5f,0,0.5f));
		//setRandomOrigin(D3DXVECTOR3(-0.5f,-0.5f,-0.5f), D3DXVECTOR3(0.5f,0.5f,0.5f));

		is_emitting = false;
		}
		break;
	case SHADOW_EXPLOSION:
		{
		setMaxLife(D3DXVECTOR2(1.5f, 2.0f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.75f),D3DXVECTOR3(0,0,0.75f));
		setRandomVelocity(D3DXVECTOR3(-0.5f,-0.5f,-0.5f),D3DXVECTOR3(0.5f,0.5f,0.5f));
		setRandomRotation(false,false,true);
		setRandomFrame(0,6);
		setStartLife(0);
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,0.0f));
		setScale(D3DXVECTOR3(0.0f,0.0f,0.0f));
		setRandomCenter(D3DXVECTOR3(-0.1f,-0.2f,-0.1f),D3DXVECTOR3(0.1f,0.2f,0.1f));
		// Bone special params
		//setRandomCenter();

		//setRandomDestiny(destiny - D3DXVECTOR3(-0.5f,0,-0.5f), destiny + D3DXVECTOR3(0.5f,0,0.5f));
		is_emitting = false;
		}
		break;
	case SHADOW_EXPLOSION_CROW:
		{
		setMaxLife(D3DXVECTOR2(0.5f, 1.0f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomVelocity(D3DXVECTOR3(-0.75f,-0.75f,-0.75f),D3DXVECTOR3(0.75f,0.75f,0.75f));
		setRandomRotation(false,false,true);
		TransformComponent * t = EntityManager::get().getComponent<TransformComponent>(this->entity);
		setCenter(D3DXVECTOR3(t->getPosition()));
		//setRandomFrame(0,6);
		setStartLife(0);
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,0.0f));
		setScale(D3DXVECTOR3(0.0f,0.0f,0.0f));
		// Bone special params
		//setRandomCenter();

		//setRandomDestiny(destiny - D3DXVECTOR3(-0.5f,0,-0.5f), destiny + D3DXVECTOR3(0.5f,0,0.5f));
		is_emitting = false;
		}
		break;
	case SHADOW_STATIC:
		{
		setMaxLife(D3DXVECTOR2(2.0f, 4.0f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomVelocity(D3DXVECTOR3(0,0.2f,0.0f),D3DXVECTOR3(0,0.4f,0.0f));
		setRandomRotation(false,false,true);
		setRandomFrame(0,6);
		setRandomStartLife();
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));
		setRandomCenter(D3DXVECTOR3(-0.25f,0,-0.25f),D3DXVECTOR3(0.25f,0,0.25f));
		// Bone special params
		//setRandomCenter();

		//setRandomDestiny(destiny - D3DXVECTOR3(-0.5f,0,-0.5f), destiny + D3DXVECTOR3(0.5f,0,0.5f));
		}
		break;
	case SHADOW_STATIC_FALL:
		{
		setMaxLife(D3DXVECTOR2(2.0f, 4.0f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomVelocity(D3DXVECTOR3(0,-0.4f,0.0f),D3DXVECTOR3(0,-0.2f,0.0f));
		setRandomRotation(false,false,true);
		setRandomFrame(0,6);
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));
		setRandomCenter(D3DXVECTOR3(-0.25f,0,-0.25f),D3DXVECTOR3(0.25f,0,0.25f));
		// Bone special params
		//setRandomCenter();

		//setRandomDestiny(destiny - D3DXVECTOR3(-0.5f,0,-0.5f), destiny + D3DXVECTOR3(0.5f,0,0.5f));
		}
		break;
	case SHADOW_EXPLOSION_MODEL:
		{
		setMaxLife(D3DXVECTOR2(0.5f, 1.0f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomVelocity(D3DXVECTOR3(-1.0f,-1.0f,-1.0f),D3DXVECTOR3(1,1,1.0f));
		setRandomRotation(false,false,true);
		setRandomFrame(0,6);
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));
		setRandomStartFrame();
		// Bone special params
		updateBonePoints(model->getCModel());
		setRandomBoneId(0,bone_points.size());
		setCenterAtBone();
		//setRandomDestiny(destiny - D3DXVECTOR3(-0.5f,0,-0.5f), destiny + D3DXVECTOR3(0.5f,0,0.5f));
		is_emitting = true;
		timer_emission = 3.0f;
		}
		break;
	case SHADOW_WALK:
		{
		setMaxLife(D3DXVECTOR2(1.75f, 2.25f));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomRotation(false,false,true);
		setRandomFrame(0,6);
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));

		// Bone special params
		updateBonePoints(model->getCModel());
		setRandomBoneId(0,bone_points.size());
		setCenterAtBone();
		}
		break;
	case SHADOW_CREATION:
		setMaxLife(D3DXVECTOR2(3.0f, 8.0f));
		setRandomCenter(D3DXVECTOR3(-aabb_radius,0.0f,-aabb_radius),D3DXVECTOR3(aabb_radius,0.0f,aabb_radius));
		setStartLife(0.0f);
		setRandomVelocity(D3DXVECTOR3(0,0.01f,0),D3DXVECTOR3(0,0.02f,0));
		setRandomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
		setRandomRotation(false,false,true);
		setRandomFrame(0,6);
		//setRandomStartLife();
		setColor(D3DXVECTOR4(0.01f,0.01f,0.01f,1.0f));
		break;
	case FIRE:
		setMaxLife(D3DXVECTOR2(1.9f, 2.1f));
		setRandomVelocity(D3DXVECTOR3(-0.075f,0.15f,-0.075f),D3DXVECTOR3(0.075f,0.5f,0.075f));
		setRandomCenter(D3DXVECTOR3(-0.05f,0.05f,-0.05f),D3DXVECTOR3(0.05f,0.05f,0.05f));
		setRandomStartLife();
		break;
	case FIREFLIES:
		setColor(D3DXVECTOR4(0.75f,1,0.2f,1));
		setScale(D3DXVECTOR3(0.36f,0.36f,0.36f));
		setMaxLife(D3DXVECTOR2(8.0f, 12.0f));
		setRandomVelocity(D3DXVECTOR3(-0.5f,-0.25f,-0.5f),D3DXVECTOR3(0.5f,0.25f,0.5f));
		setRandomStartLife();
		setRandomCenter(D3DXVECTOR3(-5,-0.25f,-5),D3DXVECTOR3(5,0.25f,5));
		break;
	case LEAVES:
		setColor(D3DXVECTOR4(0.96f,0.52f,0.611f,0.75f));
		setScale(D3DXVECTOR3(0.06f,0.06f,0.06f));
		setMaxLife(D3DXVECTOR2(7.0f, 10.0f));
		setRandomVelocity(D3DXVECTOR3(-0.1f,-0.5f,-0.4f),D3DXVECTOR3(-0.2f,-0.15f,0.4f));
		setRandomStartLife();
		setRandomCenter(D3DXVECTOR3(-5,0,-5),D3DXVECTOR3(5,0,5));
		setRandomRotation(true, true, true);
		break;
	case LIGHT_DUST:
		setColor(D3DXVECTOR4(1,1,0.6f,0.25f));
		setScale(D3DXVECTOR3(2,2,2));
		setRandomVelocity(D3DXVECTOR3(-0.15f,-0.15f,-0.15f),D3DXVECTOR3(0.15f,0.15f,0.15f));
		setRandomStartLife();
		setRandomCenter(D3DXVECTOR3(-0.1f,-0.1f,-0.1f),D3DXVECTOR3(0.1f,0.1f,0.1f));
		break;
	case FOG:
		setMaxLife(D3DXVECTOR2(8.0f, 12.0f));
		setRandomVelocity(D3DXVECTOR3(-0.05f,-0.05f,-0.05f),D3DXVECTOR3(-0.25f,0.05f,-0.05f));
		setRandomStartLife();
		setScale(D3DXVECTOR3(4,2,4));
		setColor(D3DXVECTOR4(0.2f,0.31f,0.375,0.25f));
		setRandomCenter(D3DXVECTOR3(-5,-0.5f,-5),D3DXVECTOR3(5,0.5f,5));
		break;
	case SMOKE_FIRE:
		setMaxLife(D3DXVECTOR2(2.1f, 2.4f));
		setRandomVelocity(D3DXVECTOR3(-0.06f,0.15f,-0.06f),D3DXVECTOR3(0.12f,0.5f,0.12f));
		setRandomStartLife();
		setRandomCenter(D3DXVECTOR3(-0.10f,0.15f,-0.15f), D3DXVECTOR3(0.15f,0.2f,0.10f));
		setColor(D3DXVECTOR4(1,1,1,0.85f));
		setScale(D3DXVECTOR3(0.1f,0.1f,0.1f));
		setRandomRotation(false, false, true);
		break;

	}
}

void ParticleEffectComponent::update(float delta)
{
	if (DebugSystem::get().is_particles_active == false) return;
	TransformComponent * myT = EntityManager::get().getComponent<TransformComponent>(entity);

	sortParticles();
	aabb.center = D3DXVECTOR3(myT->getWorldTransform()->getOrigin());
	aabb.half = D3DXVECTOR3(aabb_radius,aabb_radius,aabb_radius);

	// Culling
	TCamera * cam = &CameraSystem::get().getCurrentCamera();
	float dist_camera = cam->getPosition().distance(myT->getPosition());
	if (aabb_radius > 0)
	{
		if(cam->frustum.isInside(aabb) == TFrustum::OUT_OF_FRUSTUM) return;
		if (dist_camera > 20) return;
	}
	if (timer_alive >= timer_emission) is_emitting = false;

	switch (effect)
	{
	case FIREFLIES:
		{
			Entity* e = NULL;
			TransformComponent * t = NULL;
			LightComponent * l = NULL;
			float world_time = World::instance()->world_time;

			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life && is_emitting) 
				{
					d.randomVelocity(D3DXVECTOR3(-1.0f,-0.25f,-1.0f),D3DXVECTOR3(1.0f,0.25f,1.0f));
					d.randomCenter(D3DXVECTOR3(-5,-0.25f,-5),D3DXVECTOR3(5,0.25f,5));
					d.alive = 0;
				}

				d.center.x += d.velocity.x * delta;
				d.center.y += sin(world_time/4) * d.velocity.y * delta;
				d.center.z += sin(world_time) * d.velocity.z * delta;
				if (d.alive <= d.life*0.25f) d.color.w = d.alive/(d.life*0.25f);
				else if (d.alive >= d.life*0.75f) d.color.w = ((d.life - d.alive)/(d.life*0.25f));
				if (dist_camera > 15) d.color.w *=  1 - (dist_camera-15)/5.0f;

				e = lights.at(i);
				t = EntityManager::get().getComponent<TransformComponent>(e);
				l = EntityManager::get().getComponent<LightComponent>(e);
				l->light_intensity = d.color.w * 0.25f;
				t->setPosition(myT->getPosition() + btVector3(d.center.x, d.center.y, d.center.z));

				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case DECOY:
		{
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life) 
				{
					d.alive = 0;
					d.scale = D3DXVECTOR3(0,0,0);
				}
				d.center = bone_points[10];
				d.scale = d.alive * D3DXVECTOR3(0.5f,0.5f,0.5f);
				if (d.alive <= d.life*0.25f) d.color.w = d.alive/(d.life*0.25f);
				else if (d.alive >= d.life*0.75f) d.color.w = ((d.life - d.alive)/(d.life*0.25f));
				else d.color.w = 1.0f;
				if (timer_alive <= timer_emission * 0.25f) d.color.w *= timer_alive/(timer_emission*0.25f);
				else if (timer_alive >= timer_emission*0.75f) d.color.w *= ((timer_emission - timer_alive)/(timer_emission*0.25f));
				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case BLOOD_SPLATTER:
		{
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life && is_emitting) 
				{
					d.alive = 0;
					d.center = D3DXVECTOR3(0,0,0);
					d.randomVelocity(velocity_original - D3DXVECTOR3(0.5f,0.5f,0.5f),velocity_original + D3DXVECTOR3(0.5f,0.5f,0.5f));
				}
				d.velocity.y -= 0.5f * delta;
				d.center += d.velocity * delta * 2;
				d.scale = ( 1 - (d.alive/d.life) ) * D3DXVECTOR3(0.3f,0.3f,0.3f);
				//if (d.alive <= d.life*0.20f) d.color.w = d.alive/(d.life*0.20f);
				if (d.alive >= d.life*0.9f) d.color.w = ((d.life - d.alive)/(d.life*0.1f));
				else d.color.w = 1.0f;
				//if (timer_alive >= timer_emission*0.9f) d.color.w *= ((timer_emission - timer_alive)/(timer_emission*0.1f));
				
				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
	case LEAVES:
		{
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life && is_emitting) 
				{
					d.randomVelocity(D3DXVECTOR3(-0.1f,-0.5f,-0.4f),D3DXVECTOR3(-0.2f,-0.15f,0.4f));
					d.randomCenter(D3DXVECTOR3(-5,0,-5),D3DXVECTOR3(5,0,5));
					d.alive = 0;
					d.randomRotation(true, true, true);
				}

				d.center.x += (sin(d.alive/2)+1) * d.velocity.x * delta ;
				d.center.y += d.velocity.y * delta;
				d.center.z += sin(d.alive) * d.velocity.z * delta;
				if (d.alive <= d.life*0.25f) d.color.w = d.alive/(d.life*0.25f);
				else if (d.alive >= d.life*0.75f) d.color.w = ((d.life - d.alive)/(d.life*0.25f));
				d.angle.z += delta/2;
				d.angle.x += delta/4;
				d.angle.y += delta/6;
				if (dist_camera > 15) d.color.w *=  1 - (dist_camera-15)/5.0f;

				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
			if (timer_alive >= timer_emission) is_emitting = false;
		}
		break;
	case LIGHT_DUST:
		{
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];
				d.center.x += sin(d.alive/6) * d.velocity.x * delta;
				d.center.y += cos(d.alive/3) * d.velocity.y * delta;
				d.center.z += sin(d.alive/4) * d.velocity.z * delta;
				//d.color.w = sin(d.alive/5 )*0.25f;
				d.color.w = 0.25f;
				//if (d.alive <= 2.0f) d.color.w = d.alive/2.0f;
				//else if (d.alive >= 8.0f) d.color.w = ((10.0f - d.alive)/2.0f);
				if (dist_camera > 15) d.color.w *=  1 - (dist_camera-15)/5.0f;
				
				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}

		}
		break;
	case FIRE:
		{
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life && is_emitting) 
				{
					d.randomVelocity(D3DXVECTOR3(-0.05f,0.1f,-0.05f),D3DXVECTOR3(0.05f,0.3f,0.05f));
					d.randomCenter(D3DXVECTOR3(-0.05f,0.05f,-0.05f),D3DXVECTOR3(0.05f,0.05f,0.05f));

					//d.randomSize(0.05f,0.07f);
					d.alive = 0;
				}

				d.frame = d.alive;
				d.center.x += delta * d.velocity.x;
				d.center.y += delta * d.velocity.y;
				d.center.z += delta * d.velocity.z;
				if (d.alive <= d.life*0.2f) 
				{
					d.color.w = d.alive/(d.life*0.2f) * 0.5f;
				}
				else if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f) * 0.5f;
				}

				// Empieza en 255 - 217 - 14
				// Acaba en 255 - 132 - 20
				d.color.x = 1.0f;
				d.color.y = 0.65f - (d.alive/d.life)*0.35f;
				d.color.z = 0.025f;

				d.scale = D3DXVECTOR3(0.3f,0.6f,0.3f) * (1 - d.alive/d.life);
				if (dist_camera > 15) d.color.w *=  1 - (dist_camera-15)/5.0f;


				
				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
	case SHADOW:
		{
			updateBonePoints(model->getCModel());

			ShadowActionsComponent * sh = EntityManager::get().getComponent<ShadowActionsComponent>(World::instance()->getPlayer());
			bool canUsePowers = EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->_canRecharge;
			if (sh->getVisibility() == playerVisibility::ONSHADOW && canUsePowers)
			{
				if (is_emitting == false)
				{
					for( size_t i=0; i<instances.size(); ++i ) {
						TInstanceData &d = instances[ i ];
						if (d.alive >= d.life)
						{
							//d.alive = randomFloat(0,d.life);
							d.randomFrame(0,6);
							d.randomRotation(false, false, true);
							d.randomBoneId(0,bone_points.size());
							updateBoneFloorPoint(d.bone_id);
							d.original_pos = bone_floor_points[d.bone_id];
							d.dest_pos = bone_points[d.bone_id];
							d.center = d.original_pos;
						}
					}
				}
				is_emitting = true;
				//if (is_emitting == false) initParams(model);
			}
			else 
			{
				is_emitting = false;
			}
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life && is_emitting) 
				{
					//d.randomVelocity(D3DXVECTOR3(0.0f,-0.05f,0.0f),D3DXVECTOR3(0.0f,-0.25f,0.0f));
					d.randomFrame(0,6);
					//d.center = getRandomBonePosition(cmodel);
					d.randomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
					//d.randomRotation(false, false, true);
					d.alive = 0;
					d.randomBoneId(0,bone_points.size());
					//updateBonePoint(cmodel, d.bone_id);
					updateBoneFloorPoint(d.bone_id);
					d.original_pos = bone_floor_points[d.bone_id];
					d.dest_pos = bone_points[d.bone_id];
					d.center = d.original_pos;
				}

				//float percent = d.alive/d.life;
				D3DXVECTOR3 dest = bone_points[d.bone_id];

				D3DXVECTOR3 dist = dest-d.center;
				D3DXVECTOR3 distOr = dest-d.original_pos;

				float distL = D3DXVec3Length(&dist);
				float distL2 = D3DXVec3Length(&distOr);
				d.center += dist*delta/6;

				if (d.alive <= d.life*0.2f) 
				{
					d.color.w = d.alive/(d.life*0.2f);
				}
				else if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f);
				}
				else d.color.w = 1.0f;

				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.75f,0.75f,0.75f)  * (distL/distL2);

				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
	case SHADOW_HAND:
		{
			updateBonePoint(model->getCModel(), instances[ 0].bone_id);

			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life && is_emitting) 
				{
					d.randomVelocity(D3DXVECTOR3(-0.05f,0.05f,-0.05f),D3DXVECTOR3(0.05f,0.1f,0.05f));
					//d.randomVelocity(D3DXVECTOR3(0.0f,-0.05f,0.0f),D3DXVECTOR3(0.0f,-0.25f,0.0f));
					d.randomFrame(0,6);
					//d.center = getRandomBonePosition(cmodel);
					d.randomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
					//d.randomRotation(false, false, true);
					d.alive = 0;
					//d.randomBoneId(0,bone_points.size());
					//updateBoneFloorPoint(d.bone_id);
					d.original_pos = bone_points[d.bone_id];
					//d.dest_pos = bone_points[d.bone_id];
					d.center = d.original_pos;
				}
				
				d.center += d.velocity*delta;

				if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f);
				}
				else d.color.w = 1.0f;

				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.15f,0.15f,0.15f)  * (d.alive/d.life);

				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_TELEPORT:
		{
			updateBonePoints(model->getCModel());
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life) continue;

				//float percent = d.alive/d.life;
				D3DXVECTOR3 dist = d.dest_pos-d.center;
				D3DXVECTOR3 distOr = d.dest_pos-d.original_pos;

				float distL = D3DXVec3Length(&dist);

				//if (distL < 0.1f) d.alive = d.life;
				float distL2 = D3DXVec3Length(&distOr);

				D3DXVECTOR3 distN;
				D3DXVec3Normalize(&distN, &dist);

				d.center += distN*delta*6.5f + d.velocity*delta;
				//d.center = bone_points[0] + d.original_pos;
				//d.center = bone_floor_points[d.bone_id];

				//if (d.alive <= d.life*0.2f) 
				//{
				//	d.color.w = d.alive/(d.life*0.2f);
				//}
				//else if (d.alive >= d.life*0.8f) 
				//{
				//	d.color.w = (d.life - d.alive)/(d.life*0.2f);
				//}
				//else d.color.w = 1.0f;
				d.color.w = 1.0f;
				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.55f,0.55f,0.55f) * distL/distL2 + D3DXVECTOR3(0.4f,0.4f,0.4f);

				d.alive += delta;
				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_TELEPORT_END:
		{
			//updateBonePoints(model->getCModel());
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive > d.life) continue;

				d.center += d.velocity*delta*0.75f;
				//d.center = bone_points[0] + d.original_pos;
				//d.center = bone_floor_points[d.bone_id];

				//if (d.alive <= d.life*0.2f) 
				//{
				//	d.color.w = d.alive/(d.life*0.2f);
				//}

				d.color.w = 0.8f - d.alive/d.life;
				
				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.4f,0.4f,0.4f) * (1 - d.alive/d.life) + D3DXVECTOR3(0.25,0.25f,0.25f);

				d.alive += delta;
				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_EXPLOSION:
		{
			//updateBonePoints(model->getCModel());
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive > d.life && is_emitting) 
				{
					d.randomRotation(false,false,true);
					d.randomVelocity(D3DXVECTOR3(-0.5f,-0.5f,-0.5f),D3DXVECTOR3(0.5f,0.5f,0.5f));
					d.randomRotationVelocity(D3DXVECTOR3(0,0,-0.75f),D3DXVECTOR3(0,0,0.75f));
					d.alive = 0;
					d.randomCenter(D3DXVECTOR3(-0.1f,-0.2f,-0.1f),D3DXVECTOR3(0.1f,0.2f,0.1f));
				}

				d.center += d.velocity*delta;

				if (d.alive <= d.life*0.1f) 
				{
					d.color.w = d.alive/(d.life*0.1f);
				}
				else if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f);
				}
				else d.color.w = 1.0f;
				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.75f,0.75f,0.75f) * (1 - d.alive/d.life);
				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_EXPLOSION_CROW:
		{
			//updateBonePoints(model->getCModel());
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive > d.life) continue;

				d.center += d.velocity*delta;

				if (d.alive <= d.life*0.1f) 
				{
					d.color.w = d.alive/(d.life*0.1f);
				}
				else if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f);
				}
				else d.color.w = 1.0f;
				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.12f,0.12f,0.12f) * (1 - d.alive/d.life);
				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_STATIC:
		{
			//updateBonePoints(model->getCModel());
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive > d.life && is_emitting) 
				{
					d.alive = 0;
					d.randomCenter(D3DXVECTOR3(-0.25f,0,-0.25f),D3DXVECTOR3(0.25f,0,0.25f));
					//d.randomVelocity(D3DXVECTOR3(0,0.2f,0.0f),D3DXVECTOR3(0,0.4f,0.0f));
					d.randomFrame(0,6);
					//d.randomRotation(false, false, true);
				}

				d.center += d.velocity*delta;

				if (d.alive <= d.life*0.2f) 
				{
					d.color.w = d.alive/(d.life*0.2f);
				}
				else if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f);
				}
				else d.color.w = 1.0f;

				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.75f,0.75f,0.75f) * (1 - d.alive/d.life);

				d.alive += delta;
				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_STATIC_FALL:
		{
			//updateBonePoints(model->getCModel());
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive > d.life) 
				{
					d.alive = 0;
					d.randomCenter(D3DXVECTOR3(-0.25f,0,-0.25f),D3DXVECTOR3(0.25f,0,0.25f));
					d.randomVelocity(D3DXVECTOR3(0,-0.4f,0.0f),D3DXVECTOR3(0,-0.2f,0.0f));
					d.randomFrame(0,6);
					d.randomRotation(false, false, true);
				}

				d.center += d.velocity*delta;

				if (d.alive <= d.life*0.2f) 
				{
					d.color.w = d.alive/(d.life*0.2f);
				}
				else if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f);
				}
				else d.color.w = 1.0f;

				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.75f,0.75f,0.75f) * (1 - d.alive/d.life);

				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_EXPLOSION_MODEL:
		{
			//updateBonePoints(model->getCModel());
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive > d.life && is_emitting) 
				{
					d.center = d.original_pos;
					d.alive = 0;
				}

				d.center += d.velocity*delta;

				if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f);
				}
				else d.color.w = 1.0f;
				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.4f,0.4f,0.4f) * (1 - d.alive/d.life);

				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_WALK:
		{
			ShadowActionsComponent * sh = EntityManager::get().getComponent<ShadowActionsComponent>(World::instance()->getPlayer());
			if (sh->isHidden() || sh->getVisibility() == ILLUMINATED || EntityManager::get().getComponent<PlayerControllerComponent>(World::instance()->getPlayer())->getLife() <= 1.0f) 
			{
				is_emitting = false;
			}
			else 
			{
				if (is_emitting == false)
				{
					for( size_t i=0; i<instances.size(); ++i ) {
						TInstanceData &d = instances[ i ];
						if (d.alive >= d.life)
						{
							d.alive = randomFloat(0,d.life);
							d.randomFrame(0,6);
							d.randomRotation(false, false, true);
							//d.bone_id = 0;
							d.randomBoneId(0,bone_points.size());
							d.original_pos = bone_points[d.bone_id];
							d.center = d.original_pos;
						}
					}
				}
				is_emitting = true;
			}
			updateBonePoints(model->getCModel());
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];
				//float percent = d.alive/d.life;
				if ((d.alive >= d.life) && is_emitting) 
				{
					////d.randomVelocity(D3DXVECTOR3(0.0f,-0.05f,0.0f),D3DXVECTOR3(0.0f,-0.25f,0.0f));
					//d.randomFrame(0,6);
					////d.center = getRandomBonePosition(cmodel);
					//d.randomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
					//d.randomRotation(false, false, true);
					//d.alive = 0;
					//d.randomBoneId(0,bone_points.size());
					////d.alive = randomFloat(0,d.life);
					////updateBonePoint(cmodel, d.bone_id);
					////updateBoneFloorPoint(d.bone_id);
					//d.original_pos = bone_points[d.bone_id];
					//d.center = d.original_pos;
					////d.dest_pos = bone_points[d.bone_id];
				}
				//d.center.y -= delta/8;
				//d.center = lerp(d.original_pos,original,percent);
				if (d.alive <= d.life*0.2f) 
				{
					d.color.w = d.alive/(d.life*0.2f);
				}
				else if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f);
				}
				else d.color.w = 1.0f;

				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.45f,0.45f,0.45f)  * (1 - d.alive/d.life);


				
				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_CROW:
		{
			updateBonePoints(model->getCModel());
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];
				//float percent = d.alive/d.life;
				if ((d.alive >= d.life) && is_emitting) 
				{
					d.randomVelocity(D3DXVECTOR3(-0.1f,-0.3f,-0.4f),D3DXVECTOR3(-0.2f,-0.15f,0.4f));
					d.randomRotation(true, true, true);
					d.alive = 0;
					d.randomBoneId(0,bone_points.size());
					updateBonePoint(model->getCModel(), d.bone_id);
					d.original_pos = bone_points[d.bone_id];
					d.center = d.original_pos;
				}

				d.center.x += (sin(d.alive/2)+1) * d.velocity.x * delta ;
				d.center.y += d.velocity.y * delta;
				d.center.z += sin(d.alive) * d.velocity.z * delta;
				if (d.alive <= d.life*0.1f) d.color.w = d.alive/(d.life*0.1f);
				else if (d.alive >= d.life*0.75f) d.color.w = ((d.life - d.alive)/(d.life*0.25f));
				d.angle.z += delta/2;
				d.angle.x += delta/4;
				d.angle.y += delta/6;

				//d.center += d.velocity*delta;
				////d.frame = 5;
				//if (d.alive <= d.life*0.2f) 
				//{
				//	d.color.w = d.alive/(d.life*0.2f);
				//}
				//else if (d.alive >= d.life*0.8f) 
				//{
				//	d.color.w = (d.life - d.alive)/(d.life*0.2f);
				//}
				//else d.color.w = 1.0f;

				//d.angle.z += delta * d.rot_velocity.z;
				//d.scale = D3DXVECTOR3(0.25f,0.25f,0.25f)  * (1 - d.alive/d.life);
				
				d.alive += delta;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
		case SHADOW_CREATION:
		{
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life && is_emitting) 
				{
					d.randomCenter(D3DXVECTOR3(-aabb_radius*0.8f,0.0f,-aabb_radius*0.8f),D3DXVECTOR3(aabb_radius*0.8f,0.0f,aabb_radius*0.8f));
					d.randomFrame(0,6);
					d.randomVelocity(D3DXVECTOR3(0,0.01f,0),D3DXVECTOR3(0,0.002f,0));
					d.randomRotationVelocity(D3DXVECTOR3(0,0,-0.25f),D3DXVECTOR3(0,0,0.25f));
					d.randomRotation(false, false, true);
					d.alive = 0;
				}

				d.center.y += delta * d.velocity.y;
				if (d.alive <= d.life*0.2f) 
				{
					d.color.w = d.alive/(d.life*0.2f);
				}
				else if (d.alive >= d.life*0.8f) 
				{
					d.color.w = (d.life - d.alive)/(d.life*0.2f);
				}
				else d.color.w = 1.0f;


				d.angle.z += delta * d.rot_velocity.z;
				d.scale = D3DXVECTOR3(0.55f,0.55f,0.55f) * (d.alive/d.life);

				d.alive += delta;
				if (dist_camera > 15) d.color.w *=  1 - (dist_camera-15)/5.0f;
				if (aabb_radius < 0.5f) d.color.w *= aabb_radius/0.5f;
				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
					
			}
			aabb.half = D3DXVECTOR3(aabb_radius+2.0f, aabb_radius+2.0f,aabb_radius+2.0f);

		}
		break;
	case FOG:
		{
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life && is_emitting) 
				{
					d.randomCenter(D3DXVECTOR3(-5,-0.5f,-5),D3DXVECTOR3(5,0.5f,5));
					d.randomVelocity(D3DXVECTOR3(-0.05f,-0.05f,-0.05f),D3DXVECTOR3(-0.25f,0.05f,-0.05f));
					d.alive = 0;
				}

				d.center.x += delta * d.velocity.x;
				d.center.y += delta * d.velocity.y;
				d.center.z += delta * d.velocity.z;
				if (d.alive <= d.life*0.2f) d.color.w = (d.alive/(d.life*0.2f)) * 0.25f;
				else if (d.alive >= d.life*0.8f) d.color.w = ((d.life - d.alive)/(d.life*0.2f)) * 0.25f;

				if (dist_camera > 15) d.color.w *=  1 - (dist_camera-15)/5.0f;
				//if (dist_camera < 3) d.color.w *= dist_camera/3.0f;

				d.alive += delta;
				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;
	case SMOKE_FIRE:
		{
			for( size_t i=0; i<instances.size(); ++i ) {
				TInstanceData &d = instances[ i ];

				if (d.alive >= d.life && is_emitting) 
				{
					d.randomVelocity(D3DXVECTOR3(-0.06f,0.15f,-0.06f),D3DXVECTOR3(0.12f,0.5f,0.12f));
					d.randomCenter(D3DXVECTOR3(-0.1f,0.15f,-0.1f), D3DXVECTOR3(0.1f,0.2f,0.1f));
					d.alive = 0;
					d.randomRotation(false, false, true);
				}

				d.center.x += delta * d.velocity.x;
				d.center.y += delta * d.velocity.y;
				d.center.z += delta * d.velocity.z;
				if (d.alive <= d.life*0.2f) d.color.w = (d.alive/(d.life*0.2f))*0.85f;
				else if (d.alive >= d.life*0.8f) d.color.w = ((d.life - d.alive)/(d.life*0.2f))*0.85f;

				d.color.x = 1.1f - d.alive/d.life;
				d.color.z = 1.1f - d.alive/d.life;
				d.color.y = 1.1f - d.alive/d.life;
				d.scale = D3DXVECTOR3(0.4f,0.4f,0.4f) * (d.alive/d.life) + D3DXVECTOR3(0.05f,0.05f,0.05f);
				d.angle.z += delta;

				d.alive += delta;
				if (dist_camera > 15) d.color.w *=  1 - (dist_camera-15)/5.0f;

				if (sort_particles)
				{
					btVector3 pos = btVector3(d.center.x, d.center.y, d.center.z);
					d.dist_camera = CameraSystem::get().getCurrentCamera().getPosition().distance(pos);
				}
			}
		}
		break;

	default:

		break;
	}

	timer_alive += delta;

}

bool ParticleEffectComponent::checkForDeletion(void)
{
	if (!destroy_on_finish) return false;
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		if (d.alive < d.life) // A particle is still alive, don't destroy
		{
			return false;
		}
	}
	// no particles alive, destroy
	EntityManager::get().removeEntity(this->entity);
	return true;
}


void ParticleEffectComponent::render()
{
	assert( mesh );
	HRESULT hr;

	if( gpu_instances.empty() )
		return;

	//if (!particles_decl) return;
	if (!vb) return;

	updateParticlesToGPU();

	// Especificar le tipo de datos que van a venir
	hr = g_App.GetDevice()->SetVertexDeclaration( particles_decl );
	assert( hr == D3D_OK );

	// Activat el stream 0 para la geometria
	hr = g_App.GetDevice()->SetStreamSource( 0, mesh->vb, 0, mesh->header.bytes_per_vertex );
	assert( hr == D3D_OK );

	hr = g_App.GetDevice()->SetStreamSourceFreq( 0, D3DSTREAMSOURCE_INDEXEDDATA | gpu_instances.size() );
	assert( hr == D3D_OK );

	// Activat el stream 1 para los datos de las instancias
	hr = g_App.GetDevice()->SetStreamSource( 1, vb, 0, sizeof( TInstanceGPUData ) );
	assert( hr == D3D_OK );

	hr = g_App.GetDevice()->SetStreamSourceFreq( 1, D3DSTREAMSOURCE_INSTANCEDATA | 1UL );
	assert( hr == D3D_OK );

	// Activar los indices de la geometria
	hr = g_App.GetDevice()->SetIndices( mesh->ib );
	assert( hr == D3D_OK );

	// Render geometry
	hr = g_App.GetDevice()->DrawIndexedPrimitive( 
		mesh->header.primitive_type
		, 0
		, 0
		, mesh->header.nvertexs
		, 0
		, mesh->header.nfaces
		);
	assert( hr == D3D_OK );

	// Disable hw instancing
	g_App.GetDevice()->SetStreamSourceFreq(0,1);
	g_App.GetDevice()->SetStreamSourceFreq(1,1);
}

bool ParticleEffectComponent::initParticles( size_t max_particles ) {

	gpu_instances.reserve( max_particles );
	gpu_instances.resize(max_particles );
	instances.reserve(max_particles);
	instances.resize(max_particles );

	//gpu_instances.reserve(max_particles);
	size_t total_bytes = sizeof( TInstanceGPUData ) * gpu_instances.capacity();

	if( FAILED( g_App.GetDevice()->CreateVertexBuffer( 
		total_bytes
		,	0
		,	0			// We are using a special decl
		,   D3DPOOL_DEFAULT
		,   &vb
		,   NULL ) ) )
		return false;

	// Create declaration 
	if( particles_decl == NULL ) {
		HRESULT hr = g_App.GetDevice()->CreateVertexDeclaration( particles_decl_elems, &particles_decl );
		assert( hr == D3D_OK );
	}

	//sortParticles();

	return true;
}

void ParticleEffectComponent::destroy() {
	if( vb )
	{
		vb->Release( ); vb = NULL;
	}

}

void ParticleEffectComponent::destroyStatic(void)
{
	if( particles_decl ) 
		particles_decl->Release(), particles_decl = NULL;
}

bool ParticleEffectComponent::updateParticlesToGPU( ) {
	assert( vb );
	size_t total_bytes = sizeof( TInstanceGPUData ) * gpu_instances.size();
	void* pVertices;
	if( FAILED( vb->Lock( 0, total_bytes, &pVertices, 0 ) ) )
		return false;
	memcpy( pVertices, &gpu_instances[0], total_bytes );
	vb->Unlock();
	return true;
}

void ParticleEffectComponent::setColor(D3DXVECTOR4 color ) 
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.color = color;
	}
}

void ParticleEffectComponent::setRandomVelocity(D3DXVECTOR3 from, D3DXVECTOR3 to)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.randomVelocity(from,to);
	}
}

void ParticleEffectComponent::setRandomFrame(int from, int to)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.randomFrame(from,to);
	}
}

void ParticleEffectComponent::setRandomRotationVelocity(D3DXVECTOR3 from, D3DXVECTOR3 to)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.randomRotationVelocity(from,to);
	}
}

void ParticleEffectComponent::setRandomCenter(D3DXVECTOR3 from, D3DXVECTOR3 to)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.randomCenter(from, to);
	}
}

void ParticleEffectComponent::setRandomStartLife()
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.alive = randomFloat(0,d.life);
	}
}

void ParticleEffectComponent::setMaxLife(D3DXVECTOR2 m)
{
	max_life = m;
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.life = randomFloat(m.x,m.y);
	}
}

void ParticleEffectComponent::setRandomStartFrame()
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.frame = (int) randomFloat(0,num_frames);
	}
}

void ParticleEffectComponent::setScale(D3DXVECTOR3 scale)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.scale = scale;
	}
}

void ParticleEffectComponent::setCenter(D3DXVECTOR3 center)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.center = center;
	}
}

void ParticleEffectComponent::setCenter(CModel * model)
{
	// Pedir las lineas como array de floats
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		int pos = randomFloat(0,bone_points.size());
		d.center = bone_points[pos];
	}
}

D3DXVECTOR3 ParticleEffectComponent::getRandomBonePosition(CModel * model)
{
	int pos = randomFloat(0,bone_points.size());
	return bone_points[pos];
}

void ParticleEffectComponent::updateBonePoints(CModel * model)
{
	bone_points.clear();
	size_t nbones = model->getSkeleton()->getCoreSkeleton( )->getVectorCoreBone().size();
	bone_points.resize( nbones );
	// Pedir las lineas como array de floats
	model->getSkeleton( )->getBonePoints( &bone_points[ 0 ].x );
}

void ParticleEffectComponent::updateBonePoint(CModel * model, int boneId)
{
	VPoints temp;
	temp.resize (bone_points.size());
	model->getSkeleton( )->getBonePoints( &temp[ 0 ].x );
	bone_points[boneId] = temp[boneId];
}

void ParticleEffectComponent::updateBoneFloorPoints(void)
{
	bone_floor_points.clear();
	bone_floor_points.resize( bone_points.size() );

	for (int i = 0; i < bone_points.size(); i++)
	{
		updateBoneFloorPoint(i);
	}
}

void ParticleEffectComponent::updateBoneFloorPoint(int boneId)
{
	bone_floor_points[boneId] = bone_points[boneId] + D3DXVECTOR3(randomFloat(-0.25f,0.25f),-1.0f,randomFloat(-0.25f,0.25f));
}

void ParticleEffectComponent::setRandomScale(D3DXVECTOR3 from, D3DXVECTOR3 to)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.randomScale(from,to);
	}
}

void ParticleEffectComponent::setRandomBoneId(int from, int to)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.randomBoneId(from,to);
	}
}

void ParticleEffectComponent::sortParticles()
{
	if (sort_particles) std::sort (instances.begin(), instances.end(),tinstanceCmp);   

	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		gpu_instances[i].center = instances[i].center;
		gpu_instances[i].color = instances[i].color;
		gpu_instances[i].scale = instances[i].scale;
		gpu_instances[i].frame = instances[i].frame;
		gpu_instances[i].angle = instances[i].angle;
	}
}

void ParticleEffectComponent::setRotation(D3DXVECTOR3 rot)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.angle = rot;
	}
}

void ParticleEffectComponent::setRandomRotation(bool x, bool y, bool z)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.randomRotation(x, y, z);
	}
}

void ParticleEffectComponent::setCenterAtBoneFloor(bool update_original)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.center = bone_floor_points[d.bone_id];
		if (update_original) d.original_pos = d.center;
	}
}

void ParticleEffectComponent::setBoneId(int id)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.bone_id = id;
	}
}

void ParticleEffectComponent::setCenterAtBone(bool update_original)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.center = bone_points[d.bone_id];
		if (update_original) d.original_pos = d.center;
	}
}

void ParticleEffectComponent::setDestPosAtBone()
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.center = bone_floor_points[d.bone_id];
		d.dest_pos = bone_points[d.bone_id];
	}
}

void ParticleEffectComponent::play()
{
	is_emitting = true;
}

void ParticleEffectComponent::stop()
{
	is_emitting = false;
}

void ParticleEffectComponent::setRandomCenterFromBones(bool update_original)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.center = randomBetweenPoints(d.original_pos,d.dest_pos);
		if (update_original) d.original_pos = d.center;
	}
}

void ParticleEffectComponent::setRandomDestiny(D3DXVECTOR3 a, D3DXVECTOR3 b)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.dest_pos = randomBetweenPoints(a,b);
	}
}

void ParticleEffectComponent::setVelocity(D3DXVECTOR3 a)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.velocity = a;
	}
}

void ParticleEffectComponent::setRandomOrigin(D3DXVECTOR3 a, D3DXVECTOR3 b)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.original_pos = randomBetweenPoints(a,b);
	}
}

void ParticleEffectComponent::setStartLife(float x)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.alive = x;
	}
}

void ParticleEffectComponent::setOffsetToCenter(D3DXVECTOR3 offset , bool update_original)
{
	for( size_t i=0; i<instances.size(); ++i ) {
		TInstanceData &d = instances[ i ];
		d.center += offset;
		if (update_original) d.original_pos = d.center;
	}
}
