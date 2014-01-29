#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

class Entity;

class AnimationSystem
{
public:
	static AnimationSystem & get()
	{
		static AnimationSystem singleton;
		return singleton;
	}

	void update(float delta);
	void render();
	void renderDebug();
	bool isInFrustum(Entity* entity);

private:
	AnimationSystem(void);
	~AnimationSystem(void);
};

#endif
