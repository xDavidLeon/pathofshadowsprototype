#include "system_physics.h"
#include "entity.h"
#include "mesh.h"
#include "world.h"
#include "component_charcontroller.h"
#include "entity_manager.h"
#include "BulletCollision\CollisionShapes\btConvexHullShape.h"
#include "BulletMultiThreaded\SpuGatheringCollisionDispatcher.h"
#include "BulletMultiThreaded\PlatformDefinitions.h"
#include "BulletMultiThreaded/Win32ThreadSupport.h"
#include "BulletMultiThreaded/SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.h"
#include "BulletMultiThreaded\btThreadSupportInterface.h"
#include "BulletMultiThreaded\btParallelConstraintSolver.h"

#include "BulletCollision\CollisionShapes\btShapeHull.h"

const int maxProxies = 32766;
const int maxOverlap = 65535;

PhysicsSystem::PhysicsSystem(void)
{
	m_threadSupportSolver = 0;
	m_threadSupportCollision = 0;
	int maxNumOutstandingTasks = 4;
	btDefaultCollisionConstructionInfo cci;
	cci.m_defaultMaxPersistentManifoldPoolSize = 32768;
	_colConfig = new btDefaultCollisionConfiguration(cci);
	m_threadSupportCollision = new Win32ThreadSupport(Win32ThreadSupport::Win32ThreadConstructionInfo(
								"collision",
								processCollisionTask,
								createCollisionLocalStoreMemory,
								maxNumOutstandingTasks));
	_dispatcher = new	SpuGatheringCollisionDispatcher(m_threadSupportCollision,maxNumOutstandingTasks,_colConfig);

	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);

	//_dispatcher = new btCollisionDispatcher(_colConfig);
	_broadphase = new btAxisSweep3(worldMin,worldMax, maxProxies);

	m_threadSupportSolver = createSolverThreadSupport(maxNumOutstandingTasks);

	_solver = new btParallelConstraintSolver(m_threadSupportSolver);
	_dispatcher->setDispatcherFlags(btCollisionDispatcher::CD_DISABLE_CONTACTPOOL_DYNAMIC_ALLOCATION);


	_world = new btDiscreteDynamicsWorld(_dispatcher, _broadphase, _solver, _colConfig);
	_world->getDispatchInfo().m_allowedCcdPenetration=0.0004f;
	//_world->getDispatchInfo().m_enableSPU = true;
	_world->getSolverInfo().m_numIterations = 2;
	_world->getSolverInfo().m_solverMode = SOLVER_SIMD+SOLVER_USE_WARMSTARTING+SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;
	_world->getDispatchInfo().m_enableSPU = true;

	_world->setGravity(btVector3(0, 0.0000011, 0));
	
	_world->setDebugDrawer(new BulletDebugDrawer());

	_world->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_NoDebug);

	colMaskShadow = colisionTypes::CHARARTER | colisionTypes::INVISIBLE_GEOM;
	colMaskShadow = ~colMaskShadow; //Todo menos capsulas o geometria invisible
	colMaskVision = colisionTypes::CHARARTER | colisionTypes::INVISIBLE_GEOM; //| colisionTypes::FORBIDDEN;
	colMaskVision = ~colMaskVision; //Todo menos capsulas, geometria invisible //o prohibida.
	colMaskIllumination = colisionTypes::CHARARTER | colisionTypes::INVISIBLE_GEOM | colisionTypes::FORBIDDEN;
	colMaskIllumination = ~colMaskIllumination; //Todo menos capsulas, geometria invisible //o prohibida.
	colMaskNavigation = colisionTypes::CHARARTER;
	colMaskNavigation = ~colMaskNavigation; //Todo menos capsulas

}

btThreadSupportInterface* PhysicsSystem::createSolverThreadSupport(int maxNumThreads)
{
	Win32ThreadSupport::Win32ThreadConstructionInfo threadConstructionInfo("solverThreads",SolverThreadFunc,SolverlsMemoryFunc,maxNumThreads);
	Win32ThreadSupport* threadSupport = new Win32ThreadSupport(threadConstructionInfo);
	threadSupport->startSPU();

	return threadSupport;
}

PhysicsSystem::~PhysicsSystem(void)
{
	releaseAll();

	delete _world;
	delete _solver;
	if (m_threadSupportSolver) delete m_threadSupportSolver;
	delete _broadphase;
	delete _dispatcher;
	delete _colConfig;
	deleteCollisionLocalStoreMemory();
	if (m_threadSupportCollision)
	{
		delete m_threadSupportCollision;
	}
}

void PhysicsSystem::releaseAll(void)
{
	for (int i = _world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject * obj = _world->getCollisionObjectArray()[i];
		btRigidBody * body = btRigidBody::upcast(obj);

		if (body && body->getMotionState())
			delete body->getMotionState();

		_world->removeCollisionObject(obj);

		delete obj;
	}

	for (int j=0;j< _collisionShapes.size();j++)
	{
		btCollisionShape* shape = _collisionShapes[j];
		delete shape;
	}

	_collisionShapes.clear();

	std::map<Entity*,Component*>* characters = EntityManager::get().getAllEntitiesPosessingComponent<CharacterControllerComponent>();
	if (characters != NULL)
	{
		std::map<Entity*,Component*>::iterator iter;
		for (iter = characters->begin(); iter != characters->end(); ++iter)
		{
			CharacterControllerComponent * c = (CharacterControllerComponent*) iter->second;
			_world->removeCharacter(c->controller);
		}
	}
}

void PhysicsSystem::removeCollisionObject(btCollisionObject* obj)
{
	_world->removeCollisionObject(obj);
}

btRigidBody * PhysicsSystem::addStaticPlane(btVector3 & position)
{
	btCollisionShape * groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(position);

	btDefaultMotionState * motionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, groundShape, btVector3(0, 0, 0));
	btRigidBody * body = new btRigidBody(rbInfo);

	addCollisionShape(groundShape);
	_world->addRigidBody(body);

	return body;
}

btRigidBody * PhysicsSystem::addBoundingBox(btVector3 & position, btVector3 & half, float mass)
{
	btCollisionShape * colShape = new btBoxShape(half);
	btTransform boxTransform;
	boxTransform.setIdentity();

	btVector3 localInertia(0, 0, 0);

	colShape->calculateLocalInertia(mass, localInertia);

	boxTransform.setOrigin(position);

	btDefaultMotionState * motionState = new btDefaultMotionState(boxTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, colShape, localInertia);
	btRigidBody * body = new btRigidBody(rbInfo);

	addCollisionShape(colShape);
	_world->addRigidBody(body);

	return body;
}

btRigidBody * PhysicsSystem::addBoundingBox(btTransform * transform, btVector3 & center, btVector3 & half, float mass)
{
	btBoxShape* myShape = new btBoxShape(half);

	btVector3 fallInertia(0,0,0);
	myShape->calculateLocalInertia(mass,fallInertia);

	btCollisionShape* finalShape = NULL;
	if (center.length() != 0) 
	{
		finalShape = new btCompoundShape();
		btTransform offset;
		offset.setIdentity();
		offset.setOrigin(center);
		((btCompoundShape*)finalShape)->addChildShape(offset,myShape);
	}
	else finalShape = myShape;

	btDefaultMotionState * motionState = new btDefaultMotionState(*transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,finalShape,fallInertia);
	btRigidBody * body = new btRigidBody(rbInfo);
	addCollisionShape(finalShape);

	_world->addRigidBody(body);

	return body;
}

btRigidBody * PhysicsSystem::addSphere(btTransform * transform, const btScalar radius, btVector3 & center, float mass)
{
	btSphereShape* myShape = new btSphereShape(radius);

	btVector3 fallInertia(0,0,0);
	myShape->calculateLocalInertia(mass,fallInertia);

	btCompoundShape* compound = new btCompoundShape();
	btTransform offset;
	offset.setIdentity();
	offset.setOrigin(center);
	compound->addChildShape(offset,myShape);

	btDefaultMotionState * motionState = new btDefaultMotionState(*transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,compound,fallInertia);
	btRigidBody * body = new btRigidBody(rbInfo);
	addCollisionShape(compound);
	_world->addRigidBody(body);

	return body;
}

btRigidBody * PhysicsSystem::addCapsule(btTransform * transform, const btScalar radius, const btScalar height, btVector3 & center, float mass)
{
	btCapsuleShape* myShape = new btCapsuleShape(radius,height);

	btVector3 fallInertia(0,0,0);
	myShape->calculateLocalInertia(mass,fallInertia);

	btCompoundShape* compound = new btCompoundShape();
	btTransform offset;
	offset.setIdentity();
	offset.setOrigin(center);
	compound->addChildShape(offset,myShape);

	btDefaultMotionState * motionState = new btDefaultMotionState(*transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,compound,fallInertia);
	btRigidBody * body = new btRigidBody(rbInfo);
	addCollisionShape(compound);
	_world->addRigidBody(body);

	return body;
}

btRigidBody * PhysicsSystem::addConvexMeshCollider(btTransform * transform, TMesh* mesh, float mass)
{
	DWORD dwVertexSize = mesh->header.bytes_per_vertex;
    BYTE* pVertexData;
    mesh->vb->Lock(0, 0, (void**)&pVertexData, 0);

    BYTE* pIndexData;
	mesh->ib->Lock(0, 0, (void**)&pIndexData, 0);

	btTriangleIndexVertexArray * indexVertexArrays = new btTriangleIndexVertexArray(mesh->header.nfaces, (int*)pIndexData, 3 * sizeof(int), mesh->header.nvertexs, (btScalar*)pVertexData, dwVertexSize);
	btVector3 aabbMin(-10000,-10000,-10000),aabbMax(10000,10000,10000);
	btConvexShape * convexshape = new btConvexTriangleMeshShape(indexVertexArrays);
	mesh->ib->Unlock();
	mesh->vb->Unlock();   

    btShapeHull *hull = new btShapeHull(convexshape);
    btScalar margin = convexshape->getMargin();
    hull->buildHull(margin);
    convexshape->setUserPointer(hull);

	btConvexHullShape* convexShape  = new btConvexHullShape();
	bool updateLocalAabb = false;

    for (int i=0;i<hull->numVertices();i++)
    {
            convexShape->addPoint(hull->getVertexPointer()[i]);     
    }
    convexShape->recalcLocalAabb();

	//delete indexVertexArrays;

	btDefaultMotionState * motionState = new btDefaultMotionState(*transform);

	btVector3 fallInertia(0,0,0);
	convexShape->calculateLocalInertia(mass,fallInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass,motionState,convexShape,fallInertia);
	btRigidBody * body = new btRigidBody(rigidBodyCI);
	body->setSleepingThresholds(1.0f,1.0f);
	body->setActivationState(ISLAND_SLEEPING);
	addCollisionShape(convexShape);
	_world->addRigidBody(body);

	return body;
}

btRigidBody * PhysicsSystem::addConcaveMeshCollider(btTransform * transform, TMesh* mesh, float mass)
{
    DWORD dwVertexSize = mesh->header.bytes_per_vertex;
    BYTE* pVertexData;
    mesh->vb->Lock(0, 0, (void**)&pVertexData, 0);

    BYTE* pIndexData;
	mesh->ib->Lock(0, 0, (void**)&pIndexData, 0);

	btTriangleIndexVertexArray * indexVertexArrays = new btTriangleIndexVertexArray(mesh->header.nfaces, (int*)pIndexData, 3 * sizeof(int), mesh->header.nvertexs, (btScalar*)pVertexData, dwVertexSize);
	btVector3 aabbMin(-10000,-10000,-10000),aabbMax(10000,10000,10000);
	btBvhTriangleMeshShape * m_collisionShape = new btBvhTriangleMeshShape(indexVertexArrays, true,aabbMin,aabbMax);
	mesh->ib->Unlock();
	mesh->vb->Unlock();   

	m_collisionShape->buildOptimizedBvh();
	btDefaultMotionState * motionState = new btDefaultMotionState(*transform);

	btVector3 fallInertia(0,0,0);
	//m_collisionShape->calculateLocalInertia(mass,fallInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass,motionState,m_collisionShape,fallInertia);
	btRigidBody * body = new btRigidBody(rigidBodyCI);
	body->setSleepingThresholds(1.0f,1.0f);
	body->setActivationState(ISLAND_SLEEPING);
	addCollisionShape(m_collisionShape);
	_world->addRigidBody(body);
	//delete indexVertexArrays;

	return body;
}

btKinematicCharacterController * PhysicsSystem::addCharacterController(btTransform * transform, btVector3 & center, btVector3 & half, short colFilterGroup, short colFilterMask, float mass)
{
btPairCachingGhostObject* m_ghostObject = new btPairCachingGhostObject();
	m_ghostObject->setWorldTransform(*transform);

	_broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	btScalar characterHeight=half.getY()*0.8f;
	btScalar radius =half.getX();
	btConvexShape* capsule = new btCapsuleShape(radius,characterHeight);

	m_ghostObject->setCollisionShape (capsule);
	//m_ghostObject->setCollisionFlags (btCollisionObject::CF_CHARACTER_OBJECT);

	btScalar stepHeight = btScalar(half.getY());
	btKinematicCharacterController * m_character = new btKinematicCharacterController (m_ghostObject,capsule,stepHeight);
	_world->addCollisionObject(m_ghostObject, colFilterGroup, colFilterMask);
	addCollisionShape(capsule);

	_world->addAction(m_character);
	return m_character;


	//// mario
	//Vector3 pivotPosRotated = pivotPosition;
	//Matrix4x4 rot;
	//D3DXMatrixRotationQuaternion(&rot, &graphicQuat);
	//pivotPosRotated = pivotPosition.multiplyByMatrix(rot);
	//Vector3 pos = graphicPosition + pivotPosRotated;

	//physicTransform.setOrigin( BulletUtils::toBt( pos ) );
	//physicTransform.setRotation( BulletUtils::toBt( graphicQuat ) );

	//ghostObject = new btPairCachingGhostObject();
	//ghostObject->setWorldTransform( physicTransform );



	//btPairCachingGhostObject* m_ghostObject = new btPairCachingGhostObject();
	//
	///**************** parte edu ********************/
	//btCompoundShape* compound = new btCompoundShape();
	//btTransform offset;
	//offset.setIdentity();
	//center.setX(0); center.setZ(0);
	//offset.setOrigin(center);
	//offset.mult(offset, *transform);
	///**************** parte edu ********************/
	//
	//m_ghostObject->setWorldTransform(offset);

	//_broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	//btScalar characterHeight=half.getY();
	//btScalar radius =half.getX();
	//btConvexShape* capsule = new btCapsuleShape(radius,characterHeight);


	///**************** parte edu ********************/
	//compound->addChildShape(offset,capsule);
	//

	//m_ghostObject->setCollisionShape (compound);
	//m_ghostObject->setCollisionFlags (btCollisionObject::CF_CHARACTER_OBJECT);

	//btScalar stepHeight = btScalar(0.35);
	//btKinematicCharacterController * m_character = new btKinematicCharacterController (m_ghostObject,capsule,stepHeight);
	//_world->addCollisionObject(m_ghostObject,btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);

	//_world->addAction(m_character);
	//return m_character;
}

void PhysicsSystem::addCollisionShape(btCollisionShape * colShape)
{
	_collisionShapes.push_back(colShape);
}

btDiscreteDynamicsWorld * PhysicsSystem::getDynamicsWorld()
{
	return _world;
}

btCollisionWorld * PhysicsSystem::getCollisionWorld()
{
	return _world->getCollisionWorld();
}

btBroadphaseInterface * PhysicsSystem::getBroadphase()
{
	return _broadphase;
}

//Devuelve true si colisiona
bool PhysicsSystem::checkCollision(const btVector3& from, const btVector3& to, short colFilterMask) const
{
	btCollisionWorld::ClosestRayResultCallback rayCallback(from,to);
	if(colFilterMask) rayCallback.m_collisionFilterMask = colFilterMask;
	_world->getCollisionWorld()->rayTest(from, to, rayCallback);
	return rayCallback.hasHit();
}
