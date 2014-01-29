#include "component_charcontroller.h"
#include "globals.h"
#include "system_physics.h"

CharacterControllerComponent::CharacterControllerComponent(Entity* e, btKinematicCharacterController * c) : Component(e)
{
	controller = c;
	init();
}

void CharacterControllerComponent::init()
{
	velocity = btVector3(0,0,0);
	desiredVelocity = btVector3(0,0,0);
}

CharacterControllerComponent::~CharacterControllerComponent(void)
{

}

void CharacterControllerComponent::approximateVelocity(void)
{
	if (velocity.getX() != desiredVelocity.getX())
	{
		btScalar x = velocity.getX();
		btScalar desiredX = desiredVelocity.getX();
		float diff = abs(desiredX-x)*0.1;
		if (x < desiredVelocity.getX()) velocity.setX(x + diff);
		else velocity.setX(x - diff);
	}
	if (velocity.getY() != desiredVelocity.getY())
	{
		btScalar y = velocity.getY();
		btScalar desiredY = desiredVelocity.getY();
		float diff = abs(desiredY-y)*0.1;
		if (y < desiredVelocity.getY()) velocity.setY(y + diff);
		else velocity.setY(y - diff);
	}
	if (velocity.getZ() != desiredVelocity.getZ())
	{
		btScalar z = velocity.getZ();
		btScalar desiredZ = desiredVelocity.getZ();
		float diff = abs(desiredZ-z)*0.1;
		if (z < desiredVelocity.getZ()) velocity.setZ(z + diff);
		else velocity.setZ(z - diff);
	}
}

void CharacterControllerComponent::activateCollisions(void)
{
	//Volver a activar las colisiones del player (pillamos el controller y el body y los volvemos a meter en el mundo fisico)
	PhysicsSystem::get().getDynamicsWorld()->addCollisionObject(controller->getGhostObject(),colisionTypes::CHARARTER, -1);
	PhysicsSystem::get().getDynamicsWorld()->addAction(controller);
}

void CharacterControllerComponent::deactivateCollisions(void)
{
	//Desactivar colisiones del player (obtenemos el ghostObject(body) y el character controller y los sacamos del mundo fisico. Se queda asi hasta que se encuentre una forma mejor)
	PhysicsSystem::get().getDynamicsWorld()->removeCollisionObject(controller->getGhostObject());
	PhysicsSystem::get().getDynamicsWorld()->removeAction(controller);
}
