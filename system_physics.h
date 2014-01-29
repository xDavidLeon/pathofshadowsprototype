#ifndef INC_PHYSICS_H_
#define INC_PHYSICS_H_

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include "bullet_debug_drawer.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "mesh.h"


#define BIT(x) (1<<(x))

enum colisionTypes {
	NOTHING = 0,
	VISIBLE_GEOM = BIT(1),	 //(para la geometr�a visible)
	INVISIBLE_GEOM = BIT(2), //(barreras para zonas donde no se puede estar)
	CHARARTER = BIT(3),      //(o capsule)
	FORBIDDEN = BIT(4)       //(para zonas en las que no se puede estar)
};

struct btCollisionAlgorithmCreateFunc;

class Entity;
class PhysicsSystem
{
private:
	btAlignedObjectArray<btCollisionShape*> _collisionShapes;
	btBroadphaseInterface * _broadphase;
	btCollisionDispatcher * _dispatcher;
	btConstraintSolver * _solver;
	btDefaultCollisionConfiguration * _colConfig;
	btDiscreteDynamicsWorld * _world;
	class	btThreadSupportInterface*		m_threadSupportCollision;
	class	btThreadSupportInterface*		m_threadSupportSolver;
	PhysicsSystem(void);
	~PhysicsSystem(void);
	btThreadSupportInterface* createSolverThreadSupport(int maxNumThreads);

public:
	static PhysicsSystem &get()
	{
		static PhysicsSystem singleton;
		return singleton;
	}

	void			releaseAll(void);
	void			removeCollisionObject(btCollisionObject* obj);

	btRigidBody *	addStaticPlane(btVector3 & position);
	btRigidBody *	addBoundingBox(btVector3 & position, btVector3 & half, float mass = 1.0f);
	btRigidBody *	addBoundingBox(btTransform * transform, btVector3 & center, btVector3 & half, float mass = 0.0f);
	btRigidBody *	addSphere(btTransform * transform, const btScalar radius, btVector3 & center, float mass = 0.0f);
	btRigidBody *	addCapsule(btTransform * transform, const btScalar radius, const btScalar height, btVector3 & center, float mass = 0.0f);

	btRigidBody *	addConvexMeshCollider(btTransform * transform, TMesh* mesh, float mass = 0.0f);
	btRigidBody *	addConcaveMeshCollider(btTransform * transform, TMesh* mesh, float mass = 0.0f);

	btKinematicCharacterController * addCharacterController(btTransform * transform, btVector3 & center, btVector3 & half, short colFilterGroup, short colFilterMask, float mass);

	//btRigidBody * addRigidBody(btTransform & transform, btCollisionShape * shape, btScalar & mass, GameObject * g = NULL);
	void addCollisionShape(btCollisionShape * colShape);
	btDiscreteDynamicsWorld * getDynamicsWorld();
	btCollisionWorld * getCollisionWorld();
	btBroadphaseInterface * getBroadphase();

	bool checkCollision(const btVector3& from, const btVector3& to, short colFilterMask=0) const;

	unsigned colMaskShadow, colMaskVision, colMaskNavigation, colMaskIllumination;
};

#endif
