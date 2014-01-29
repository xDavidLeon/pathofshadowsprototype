#ifndef COMPONENT_LIGHT
#define COMPONENT_LIGHT

#include "component.h"
#include <d3dx9.h>
#include "component_transform.h"
#include "globals.h"
#include "texture_manager.h"
#include "component_transform.h"
#include "entity.h"
#include "system_light.h"
#include <cassert>

const static float maxRadiusMh = 10.0f;

enum LIGHT_TYPE 
{
	NONE,
	LIGHT_DIRECTIONAL,
	LIGHT_SPOT,
	LIGHT_POINT,
	LIGHT_BLOB
};

class LightComponent :
	public Component
{
	
public:
	LIGHT_TYPE light_type;
	LightComponent(Entity* e, LIGHT_TYPE type, D3DXCOLOR color, float intensity = 1.0f);
	~LightComponent(void);

	D3DXCOLOR	light_color;
	float		light_intensity;
	D3DXVECTOR3 light_direction;
	D3DXVECTOR3 look_at;

	float light_near;
	float light_far;
	float light_fov;
	bool isTorch;
	bool isShadow;
	bool generate_shadows;
	int shadowmap_resolution;
	float depth_bias;

	D3DXMATRIX light_view;
	D3DXMATRIX light_ortho;
	D3DXMATRIX light_projection;
	D3DXMATRIX light_view_projection;
	IDirect3DTexture9* rt_shadowmap;
	TTexture texture_attenuation;

	void setType(LIGHT_TYPE type)
	{
		light_type = type;
		LightSystem::get().addLight(entity, light_type);
	}

	LIGHT_TYPE getType() const{ return light_type; }

	void setLookAt(float x, float y, float z);
	void generateViewMatrix();
	void generateOrthoMatrix();
	float getRadius();
	float getRadiusSq();
	void setRadius(float r);
	void update(float delta);
	void setTargetRadius(float new_radius, float velocity_factor);
	void setTargetIntensity(float new_intensity, float velocity_factor);

private:
	float		light_radius, light_radiusSq; //La sq es para calcular distancias sin tener que hacer sqrt()
	float		target_radius;
	float		target_intensity;
	float		velocity_multiplier;
};

#endif
