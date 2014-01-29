#pragma once
#include "component.h"
#include "btBulletDynamicsCommon.h"

class RigidbodyComponent :
	public Component
{
public:
	RigidbodyComponent(Entity* e, btRigidBody * b);
	~RigidbodyComponent(void);

	void			setKinematic(bool k);
	void			setPosition( const btVector3 &new_pos );
	void			move( const btVector3 &delta );
	void			rotate( const btVector3& axis, float radians );
	void			rotateY( float radians );

	bool			isInsideVisionCone( const btVector3& loc, float half_fov ) const;
	float			getDistanceTo( const btVector3& loc ) const;
	float			getAngleToPoint( const btVector3& loc ) const;
	const btVector3 &getPosition() const;
	const btVector3 &getFront( ) const;
	const btVector3 &getLeft( ) const;
	const btVector3 &getUp( ) const;

	btRigidBody* body;
};

