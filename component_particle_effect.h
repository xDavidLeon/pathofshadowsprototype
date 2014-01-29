#ifndef PARTICLE_COMPONENT_H
#define PARTICLE_COMPONENT_H

#include "component.h"
#include <vector>
#include "mesh.h"
#include "texture_manager.h"
#include <list>

class CModel;
class ModelComponent;
class LightComponent;
class ParticleEffectComponent : public Component
{

public:

	// Contains minimal particle data to send to GPU
	struct TInstanceGPUData 
	{
		D3DXVECTOR3 center;
		D3DXVECTOR4 color;
		D3DXVECTOR3 scale;
		float frame;
		D3DXVECTOR3 angle;
		TInstanceGPUData () : center(0,0,0), color(1,1,1,1), scale( 1,1,1 ), frame(0), angle(0,0,0) {}
	} instance_gpu_data;

	// Complete data of particle for complex operations and effects
	class TInstanceData {
	public:
		D3DXVECTOR3 center;
		D3DXVECTOR4 color;
		D3DXVECTOR3 scale;
		float		frame;
		D3DXVECTOR3		angle;
		float		dist_camera;
		float		alive;
		float		life;
		D3DXVECTOR3 velocity;
		D3DXVECTOR3 rot_velocity;
		int			bone_id;
		D3DXVECTOR3 original_pos;
		D3DXVECTOR3 dest_pos;

		// Extras de logica
		// Velocidad
		// spin...

		TInstanceData () : center(0,0,0), color(1,1,1,1), scale( 1,1,1 ), frame(0), angle(0,0,0), dist_camera(0), alive(0), life(0), velocity( 0,0,0 ), rot_velocity( 0,0,0 ), original_pos(0,0,0), dest_pos(0,0,0)   {}

		void randomCenter(D3DXVECTOR3 from, D3DXVECTOR3 to)
		{
			float x = randomFloat(from.x,to.x);
			float y = randomFloat(from.y,to.y);
			float z = randomFloat(from.z,to.z);
			center = D3DXVECTOR3(x,y,z);
		}

		void randomVelocity(D3DXVECTOR3 from, D3DXVECTOR3 to)
		{
			float x = randomFloat(from.x,to.x);
			float y = randomFloat(from.y,to.y);
			float z = randomFloat(from.z,to.z);
			velocity = D3DXVECTOR3(x,y,z);
		}

		void randomRotationVelocity(D3DXVECTOR3 from, D3DXVECTOR3 to)
		{
			float x = randomFloat(from.x,to.x);
			float y = randomFloat(from.y,to.y);
			float z = randomFloat(from.z,to.z);
			rot_velocity = D3DXVECTOR3(x,y,z);
		}

		void randomScale(D3DXVECTOR3 from, D3DXVECTOR3 to)
		{
			float x = randomFloat(from.x,to.x);
			float y = randomFloat(from.y,to.y);
			float z = randomFloat(from.z,to.z);
			scale = D3DXVECTOR3(x,y,z);
		}

		void randomFrame(int from, int to)
		{
			int x = (int) randomFloat(from,to);
			frame = x;
		}

		void randomBoneId(int from, int to)
		{
			int x = (int) randomFloat(from,to);
			bone_id = x;
		}

		void randomRotation(bool x, bool y, bool z)
		{
			float _x = 0;
			float _y = 0;
			float _z = 0;
			if (x) _x = randomFloat(-360, 360);
			if (y) _y = randomFloat(-360, 360);
			if (z) _z = randomFloat(-360, 360);
			angle = D3DXVECTOR3(_x,_y,_z);
		}
	};

	struct TInstanceCmp {
		bool operator()(const TInstanceData &a,const TInstanceData &b) const{
			return a.dist_camera > b.dist_camera;
		}
	} tinstanceCmp;

	enum PARTICLE_EFFECT
	{
		FIRE,
		FIREFLIES,
		FOG,
		DECOY,
		SHADOW,
		SHADOW_WALK,
		SHADOW_CROW,
		SHADOW_STATIC,
		SHADOW_STATIC_FALL,
		SHADOW_CREATION,
		SHADOW_TELEPORT,
		SHADOW_TELEPORT_END,
		SHADOW_EXPLOSION,
		SHADOW_EXPLOSION_CROW,
		SHADOW_EXPLOSION_MODEL,
		SHADOW_IMPLOSION,
		SHADOW_IMPLOSION_MODEL,
		SHADOW_HAND,
		BLOOD_SPLATTER,
		BLOOD_FLOOR,
		SMOKE_FIRE,
		LIGHT_DUST,
		LEAVES
	};

	ParticleEffectComponent(Entity* e, PARTICLE_EFFECT particle_effect, ModelComponent* model = NULL, D3DXVECTOR3 dest = D3DXVECTOR3(0,0,0), D3DXVECTOR3 velocity = D3DXVECTOR3(0,0,0));

	~ParticleEffectComponent(void);

	PARTICLE_EFFECT effect;

	typedef std::vector< D3DXVECTOR3 > VPoints;

	// Render Data
	typedef std::vector< TInstanceData > VParticles;
	typedef std::vector< TInstanceGPUData > VGPUParticles;

	LPDIRECT3DVERTEXBUFFER9 vb;

	VParticles instances;
	VGPUParticles gpu_instances;

	// Variables
	const TMesh      *mesh;
	ModelComponent * model;
	TTexture texture;
	int	num_frames;

	// Helper parameters
	VPoints bone_points;
	VPoints bone_floor_points;
	std::vector<Entity*> lights;
	D3DXVECTOR3 destiny;
	D3DXVECTOR3 velocity_original;
	float timer_alive;
	float timer_emission;
	bool ignore_outline;

	// Particle effect parameters
	D3DXVECTOR2 max_life;
	float aabb_radius;
	TAABB aabb;
	bool additive;
	std::string tech_name;
	bool destroy_on_finish;
	bool sort_particles;

	// Functions
	void update(float delta);
	void render();
	void initDefaultParams(void);
	bool initParticles( size_t max_particles );
	void destroy();
	static void destroyStatic(void);

	bool checkForDeletion(void);
	void setColor(D3DXVECTOR4 color);
	void setScale(D3DXVECTOR3 size);
	void setCenter(D3DXVECTOR3 size);
	void setCenter(CModel * model);
	void setRotation(D3DXVECTOR3 rot);
	void setRandomScale(D3DXVECTOR3 from, D3DXVECTOR3 to);
	D3DXVECTOR3 getRandomBonePosition(CModel * model);
	void setRandomVelocity(D3DXVECTOR3 from, D3DXVECTOR3 to);
	void setRandomRotationVelocity(D3DXVECTOR3 from, D3DXVECTOR3 to);
	void setRandomCenter(D3DXVECTOR3 from, D3DXVECTOR3 to);
	void setRandomCenterFromBones(bool update_original = true);
	void setRandomRotation(bool x, bool y, bool z);
	void setRandomStartLife();
	void setStartLife(float x);
	void setRandomStartFrame();
	void setMaxLife(D3DXVECTOR2 m);
	void updateBonePoints(CModel * model);
	void updateBonePoint(CModel * model, int boneId);
	void updateBoneFloorPoints(void);
	void updateBoneFloorPoint(int boneId);
	void setRandomFrame(int from, int to);
	void setRandomBoneId(int from, int to);
	void setCenterAtBoneFloor(bool update_original = true);
	void setCenterAtBone(bool update_original = true);
	void setDestPosAtBone();
	void setRandomDestiny(D3DXVECTOR3 a, D3DXVECTOR3 b);
	void setRandomOrigin(D3DXVECTOR3 a, D3DXVECTOR3 b);
	void setVelocity(D3DXVECTOR3 vel);
	void setBoneId(int id);
	void setOffsetToCenter(D3DXVECTOR3 offset , bool update_original = true);

	void wakeUp(void);
	void play(void);
	void stop(void);
	bool isEmitting(void) { return is_emitting; }

private:
	bool is_emitting;

	void sortParticles();
	bool updateParticlesToGPU();
	void initParams(ModelComponent * model = NULL, D3DXVECTOR3 dest = D3DXVECTOR3(0,0,0), D3DXVECTOR3 velocity = D3DXVECTOR3(0,0,0));
};

#endif
