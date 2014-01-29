#include "component_light.h"
#include "d3ddefs.h"
#include "globals.h"
#include "angular.h"
#include "world.h"

LightComponent::LightComponent(Entity* e, LIGHT_TYPE type, D3DXCOLOR color, float intensity) : Component(e)
{
	light_type = type;
	light_color = color;
	light_intensity = intensity;
	LightSystem::get().addLight(e, light_type);
	isTorch = true;
	isShadow = true;
	if (type == LIGHT_TYPE::LIGHT_BLOB) isShadow = false;
	target_radius = light_radius;
	target_intensity = light_intensity;
	velocity_multiplier = 1.0f;
}

LightComponent::~LightComponent(void)
{
	LightSystem::get().removeLight(entity, light_type);
}

void LightComponent::generateOrthoMatrix()
{

}

void LightComponent::generateViewMatrix()
{
	//D3DXVECTOR3 up;

	//// Setup the vector that points upwards.
	//up.x = 0.0f;
	//up.y = 1.0f;
	//up.z = 0.0f;

	//// Create the view matrix from the three vectors.
	//D3DXMatrixLookAtLH(&light_view, &pos, &look_at, &up);
	
	return;
}

void LightComponent::update(float delta)
{
	if (light_radius < target_radius) 
	{
		light_radius += delta*velocity_multiplier;
		if (light_radius > target_radius) light_radius = target_radius;
	}
	else if (light_radius > target_radius) 
	{
		light_radius -= delta*velocity_multiplier;
		if (light_radius < target_radius) light_radius = target_radius;
	}
	if (light_intensity < target_intensity) 
	{
		light_intensity += delta*velocity_multiplier;
		if (light_intensity > target_intensity) light_intensity = target_intensity;
	}
	else if (light_intensity > target_radius) 
	{
		light_intensity -= delta*velocity_multiplier;
		if (light_intensity < target_intensity) light_intensity = target_intensity;
	}
}

void LightComponent::setRadius(float r)
{
	light_radius = r;
	light_radiusSq = r*r;
	target_radius = r;
}

float LightComponent::getRadius(void)
{
	return light_radius;
}

float LightComponent::getRadiusSq(void)
{
	return light_radiusSq;
}

void LightComponent::setTargetRadius(float new_radius, float velocity_factor)
{
	target_radius = new_radius;
	velocity_multiplier = velocity_factor;
}

void LightComponent::setTargetIntensity(float new_intensity, float velocity_factor)
{
	target_intensity = new_intensity;
	velocity_multiplier = velocity_factor;
}
