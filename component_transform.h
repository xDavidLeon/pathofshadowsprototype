#pragma once
#include "component.h"
#include <btBulletCollisionCommon.h>
#include <list>

class TransformComponent :
	public Component
{
public:
	TransformComponent(Entity* e, const btTransform& t);
	~TransformComponent(void);
	void init();

	btTransform * transform;
	btTransform _initTransform;
	btTransform * worldTransform;
	TransformComponent * parent;
	std::list<TransformComponent*> children; 

	const btVector3 & getPosition() const;
	const btVector3 & getFront( ) const;
	const btVector3 & getLeft( ) const;
	const btVector3 & getUp( ) const;
	//Se devuelve vector con X invertida
	void getFrontXinv( btVector3& out_vec ) const;
	void getLeftXinv( btVector3& out_vec ) const;

	btTransform *		getWorldTransform(void) const;
	void	setPosition( const btVector3 &new_pos );
	void	setPosition( const float x, const float y, const float z);
	void	move( const btVector3 &delta );
	void	moveLocal( const btVector3 &delta );
	void	rotate( const btVector3& axis, float radians );
	void	rotateY( float radians );
	void	lookAt( const btVector3 &target, const btVector3 &aux_up);
	void	lookAtBillboard( const btVector3 &target, const btVector3 &aux_up);
	void	approximateFront_v(const btVector3& dir, float amountRotated);
	void	approximateFront_v(const btVector3& dir, float amountRotated, float& angle_out);
	void	approximateFront_p(const btVector3& point, float amountRotated);
	void	approximateFront_p(const btVector3& point, float amountRotated, float& angle_out);


	float getAngleToPoint( const btVector3& loc ) const;
	bool isInsideVisionCone( const btVector3& loc, float half_fov ) const;
	bool isInsideVisionCone( const btVector3& loc, float half_fov, float& angle_out ) const;

	void update(float delta);

	void setParent(TransformComponent * t);
	void setChild(TransformComponent * t);

	void setInitTransform(const btTransform& nT){ _initTransform = nT; }

	int numChildren(void);
	void destroyChildren(void);
};

