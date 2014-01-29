#include "component_transform.h"
#include "d3ddefs.h"
#include <d3dx9.h>
#include "entity_manager.h"
#include "globals.h"
#include "BTDeadEnemy.h"
#include "component_bt.h"

TransformComponent::TransformComponent(Entity* e, const btTransform & t) : Component(e)
{
	transform = new btTransform();
	_initTransform = t;
	worldTransform = new btTransform();
	worldTransform->setIdentity();
	parent = NULL;
	init();
}

void TransformComponent::init()
{
	//situamos la entidad donde su init position... si no es un cadaver!!!
	BTComponent* btc = EntityManager::get().getComponent<BTComponent>(entity);
	if(btc)
	{
		if(dynamic_cast<BTDeadEnemy*>(btc->getBT())) return; //si es un dead enemy salimos sin hacer init()
	}

	transform->setBasis(_initTransform.getBasis());
	transform->setOrigin(_initTransform.getOrigin());
}

TransformComponent::~TransformComponent(void)
{
	//delete transform;
	//delete worldTransform;

	transform = NULL;
	worldTransform = NULL;
	parent = NULL;
}

btTransform *	TransformComponent::getWorldTransform(void) const
{
	if (parent == NULL) return transform;
	else if (parent->transform != NULL)
	{
		//worldTransform->mult(*parent->getWorldTransform(),*transform);
		worldTransform->mult(*transform, *parent->getWorldTransform());
		return worldTransform;
	}
}

const btVector3 &TransformComponent::getPosition() const {
	return getWorldTransform()->getOrigin();
}

const btVector3 &TransformComponent::getFront( ) const {
	return getWorldTransform()->getBasis()[2].normalize();
}

const btVector3 &TransformComponent::getLeft( ) const {
	return getWorldTransform()->getBasis()[0].normalize();
}

const btVector3 &TransformComponent::getUp( ) const {
	return getWorldTransform()->getBasis()[1].normalize();
}

void TransformComponent::getFrontXinv( btVector3& out_vec ) const
{
	out_vec = getFront();
	out_vec.setX(-out_vec.getX());
}

void TransformComponent::getLeftXinv( btVector3& out_vec ) const
{
	out_vec = getLeft();
	out_vec.setX(-out_vec.getX());
}

void TransformComponent::setPosition( const btVector3 &new_pos ) {
	transform->setOrigin(new_pos);
}

void TransformComponent::setPosition( const float x, const float y, const float z)
{
	transform->setOrigin(btVector3(x,y,z));
}

void TransformComponent::lookAt( const btVector3 &target, const btVector3 &aux_up)
{
	btVector3 dist = target-getPosition();
	float distance = dist.length();
	btVector3 front = dist.normalize();
	btVector3 left = aux_up.cross(front).normalize();
	btVector3 up = front.cross(left).normalize();
	float m00 = left.getX();
	float m01 = up.getX();
	float m02 = front.getX();
	float m10 = left.getY();
	float m11 = up.getY();
	float m12 = front.getY();
	float m20 = left.getZ();
	float m21 = up.getZ();
	float m22 = front.getZ();

	btQuaternion ret;
	ret.setW(sqrtf(1.0f + m00 + m11 + m22) * 0.5f);
	float w4_recip = 1.0f / (4.0f * ret.getW());
	ret.setX((m21 - m12) * w4_recip);
	ret.setY((m02 - m20) * w4_recip);
	ret.setZ((m10 - m01) * w4_recip);

	transform->setRotation(ret);
	//transform->setOrigin(target - (front*distance));
	
}

void TransformComponent::lookAtBillboard( const btVector3 &camera_front, const btVector3 &aux_up)
{
	btVector3 front = -camera_front;
	btVector3 left = aux_up.cross(front).normalize();
	btVector3 up = front.cross(left).normalize();
	float m00 = left.getX();
	float m01 = up.getX();
	float m02 = front.getX();
	float m10 = left.getY();
	float m11 = up.getY();
	float m12 = front.getY();
	float m20 = left.getZ();
	float m21 = up.getZ();
	float m22 = front.getZ();

	btQuaternion ret;
	ret.setW(sqrtf(1.0f + m00 + m11 + m22) * 0.5f);
	float w4_recip = 1.0f / (4.0f * ret.getW());
	ret.setX((m21 - m12) * w4_recip);
	ret.setY((m02 - m20) * w4_recip);
	ret.setZ((m10 - m01) * w4_recip);

	transform->setRotation(ret);
	//transform->setOrigin(target - (front*distance));
	
}

void TransformComponent::move( const btVector3 &delta ) {
	btVector3 delta_world = btVector3(delta);
	delta_world.setX(-delta_world.getX());	//Maldicion del -X
	btVector3 newPos = transform->getOrigin() + delta_world;
	setPosition(newPos);
}

void TransformComponent::moveLocal( const btVector3 &delta ) {
	setPosition( (*transform) * delta);
}

void TransformComponent::rotate( const btVector3& axis, float radians ) {
	btQuaternion bRotation = transform->getRotation();
	btQuaternion rot = btQuaternion();
	rot.setRotation(axis, radians);

	transform->setRotation(rot*bRotation);
}

void TransformComponent::rotateY( float radians ) {
	rotate( btVector3(0,1,0), radians );
}

// Angulo con signo, asumiendo que giro sobre el eje Y
float TransformComponent::getAngleToPoint( const btVector3& loc ) const {
	btVector3 delta = loc - getPosition();
	delta.setX(-delta.getX()); //(yeah)

	// Convert loc to local space
	float proj_in_front = getFront().dot(delta);
	float proj_in_left  = getLeft().dot(delta);

	// use atan2f, never acosf!!
	float angle = atan2f( proj_in_left, proj_in_front );
	//dbg("angle: %f\n", angle);
	return angle;
}

bool TransformComponent::isInsideVisionCone( const btVector3& loc, float half_fov ) const {
	float angle = fabsf( getAngleToPoint( loc ) );
	return angle < half_fov;
}

//Igual que la anterior pero devuelve por referencia el angulo
bool TransformComponent::isInsideVisionCone( const btVector3& loc, float half_fov, float& angle_out ) const
{
	angle_out = fabsf( getAngleToPoint( loc ) );
	return angle_out < half_fov;
}

void TransformComponent::approximateFront_v(const btVector3& dir, float amountRotated)
{
	float angle = getAngleToPoint( getPosition()+dir );
	// amountRotated should be always > 0
	if( amountRotated > fabsf( angle ) ) {
		amountRotated = fabsf( angle );
	}

	rotateY(amountRotated * sign(-angle));
}

void TransformComponent::approximateFront_v(const btVector3& dir, float amountRotated, float& angle_out)
{
	angle_out = getAngleToPoint( getPosition()+dir );
	// amountRotated should be always > 0
	if( amountRotated > fabsf( angle_out ) ) {
		amountRotated = fabsf( angle_out );
	}

	rotateY(amountRotated * sign(-angle_out));
}

void TransformComponent::approximateFront_p(const btVector3& point, float amountRotated)
{
	float angle = getAngleToPoint( point );
	// amountRotated should be always > 0
	if( amountRotated > fabsf( angle ) ) {
		amountRotated = fabsf( angle );
	}

	rotateY(amountRotated * sign(-angle));
}

void TransformComponent::approximateFront_p(const btVector3& point, float amountRotated, float& angle_out)
{
	angle_out = getAngleToPoint( point );
	// amountRotated should be always > 0
	if( amountRotated > fabsf( angle_out ) ) {
		amountRotated = fabsf( angle_out );
	}

	rotateY(amountRotated * sign(-angle_out));
}

void TransformComponent::update(float delta)
{
}

void TransformComponent::setParent(TransformComponent * t)
{
	parent = t;
	t->children.push_back(this);
}

void TransformComponent::setChild(TransformComponent * t)
{
	t->parent = this;
	children.push_back(t);
}

int TransformComponent::numChildren(void)
{
	return children.size();
}

void TransformComponent::destroyChildren(void)
{
	TransformComponent * child;
	std::list<TransformComponent*>::iterator it;
	while (children.size())
	{
		it = children.begin();
		child = *it;
		children.erase(it); //Sacamos el hijo del vector de hijos
		EntityManager::get().removeEntity(child->entity); //Eliminamos hijo
	}
}
