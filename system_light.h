#ifndef LIGHT_SYS
#define LIGHT_SYS

#include <list>
#include "btBulletDynamicsCommon.h"

class Entity;
enum LIGHT_TYPE;

class LightSystem
{
	std::list<Entity*> _dirLights;
	std::list<Entity*> _pointLights;
	std::list<Entity*> _spotLights;

	float _lightRaycastOffset;

	LightSystem(void);
	~LightSystem(void);

public:
	static LightSystem& get()
	{
		static LightSystem LS = LightSystem();
		return LS;
	}

	void update(float delta);
	void addLight(Entity* light_e, LIGHT_TYPE type);
	void removeLight(Entity* light_e, LIGHT_TYPE type);

	bool posInPointLight(const btVector3& pos, float offset=0.0f);
	bool posInDirectionalLight(const btVector3& pos);

	void renderDirLights(const btVector3& from_pos);

	void putOutTorch(const std::string& pointlight, const std::string& fireP, const std::string& smokeP, const std::string& billboard, float poTime);

	void releaseLights();
};

#endif
