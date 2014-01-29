#include "component_rigidbody.h"
#include "system_physics.h"

RigidbodyComponent::RigidbodyComponent(Entity * e, btRigidBody* b) : Component(e)
{
	body = b;
}


RigidbodyComponent::~RigidbodyComponent(void)
{
}

void RigidbodyComponent::setKinematic(bool k)
{
	if (k)
	{
		PhysicsSystem::get().getDynamicsWorld()->removeRigidBody(body);
		//body->setMassProps(0, btVector3(0,0,0));
		body->setCollisionFlags(body->getCollisionFlags()|btCollisionObject::CF_KINEMATIC_OBJECT| btCollisionObject::CF_NO_CONTACT_RESPONSE);
		body->setLinearVelocity(btVector3(0,0,0));
		body->setAngularVelocity(btVector3(0,0,0));
		body->setActivationState(DISABLE_DEACTIVATION);
		PhysicsSystem::get().getDynamicsWorld()->addRigidBody(body);
		//body->activate(true);
	}
	else 
	{
		PhysicsSystem::get().getDynamicsWorld()->removeRigidBody(body);
		//btVector3 inertia(0,0,0);
		//body->calculateLocalInertia(mass, inertia);
		body->setCollisionFlags( btCollisionObject::CF_STATIC_OBJECT);
		body->updateInertiaTensor();
		body->setLinearVelocity(btVector3(0,0,0));
		body->setAngularVelocity(btVector3(0,0,0));
		body->setActivationState(WANTS_DEACTIVATION);
		PhysicsSystem::get().getDynamicsWorld()->addRigidBody(body);
		body->activate(true); // or try� body->forceActivationState(ACTIVE_FLAG)
	}
}

float RigidbodyComponent::getDistanceTo( const btVector3& loc ) const {
	btVector3 delta = getPosition() - loc;
	return delta.length();
}

const btVector3 &RigidbodyComponent::getPosition() const {
	return body->getWorldTransform().getOrigin();
}

const btVector3 &RigidbodyComponent::getFront( ) const {
	return body->getWorldTransform().getBasis()[2].normalize();
}

const btVector3 &RigidbodyComponent::getLeft( ) const {
	btVector3& v3 = body->getWorldTransform().getBasis()[0];
	v3.normalize();
	return v3;
}

const btVector3 &RigidbodyComponent::getUp( ) const {
	return body->getWorldTransform().getBasis()[1].normalize();
}

bool RigidbodyComponent::isInsideVisionCone( const btVector3& loc, float half_fov ) const {
	float angle = fabsf( getAngleToPoint( loc ) );
	return angle < half_fov;
}

// Angulo con signo, asumiendo que giro sobre el eje Y
float RigidbodyComponent::getAngleToPoint( const btVector3& loc ) const {
	btVector3 delta = loc - getPosition();

	// Convert loc to local space
	float proj_in_front = getFront().dot(delta);
	float proj_in_left  = getLeft().dot(delta);

	// use atan2f, never acosf!!
	float angle = atan2f( proj_in_left, proj_in_front );
	return angle;
}

void RigidbodyComponent::setPosition( const btVector3 &new_pos ) {
	body->getWorldTransform().setOrigin(new_pos);
}

void RigidbodyComponent::move( const btVector3 &delta ) {
	btVector3 newPos = body->getWorldTransform().getOrigin() + delta;
	setPosition(newPos);
}

void RigidbodyComponent::rotate( const btVector3& axis, float radians ) {
	btQuaternion bRotation = body->getWorldTransform().getRotation();
	btQuaternion rot = btQuaternion();
	rot.setRotation(axis, radians);

	body->getWorldTransform().setRotation(rot*bRotation);
}

void RigidbodyComponent::rotateY( float radians ) {
	rotate( btVector3(0,1,0), radians );
}
