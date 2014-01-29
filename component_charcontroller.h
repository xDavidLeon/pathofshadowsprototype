#pragma once
#include "component.h"

#include "BulletDynamics\Character\btKinematicCharacterController.h"

class CharacterControllerComponent :
	public Component
{
public:
	CharacterControllerComponent(Entity* e, btKinematicCharacterController * c);
	~CharacterControllerComponent(void);

	void init();

	void approximateVelocity(void);
	void activateCollisions(void);
	void deactivateCollisions(void);

	btKinematicCharacterController * controller;
	btVector3 velocity;
	btVector3 desiredVelocity;
};

