#pragma once
#include "component.h"
#include "d3ddefs.h"
//#include "magic_shadow.h"

static const float grow_radius = 0.01f; //Radio que se suma cada vez
static const float shrink_radius = 0.001f; //Radio que se resta cada vez
static const float max_radius = 3.0f; //radio m�ximo que pueden asimilar las sombras
static const float visible_radius = 0.7f; //dist m�nima al centro de la sombra para ser invisible
static const float shrink_factor_fast = 0.92f; //Factor r�pido de "encogimiento"

enum msStates {
		GROWING,
		SHRINKING,
		DYING
};

class ParticleEffectComponent;
class ShadowComponent :
	public Component
{
public:
	ShadowComponent(Entity* e, D3DXVECTOR3 normalVector, int sid);
	~ShadowComponent(void);

	void grow();
	void stopGrowing();

	void die();
	bool canDie();

	D3DXVECTOR3 normal;
	float radius;
	msStates state;
	int shadow_id;
	//ParticleEffectComponent * particle;
};

